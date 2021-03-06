﻿#include "LinkerManager.h"
#include <GWClient.h>
#include <ProtobufCodec.h>
#include <proto/client_link.pb.h>
#include <proto/client_logic.pb.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>
#include "Logging.h"
#include "GlobalObject.h"
#include "ServerConfig.h"
#include "SessionHandle.h"
#include "LoginConnector.h"

LinkerManager::LinkerManager(network::IOServiceThreadManager &threads)
	: threads_(threads)
	, counter_(ServerConfig::GetInstance()->GetReportInterval())
	, timer_(threads.MainThread()->IOService(), std::chrono::seconds(1))
	, wait_handler_(std::bind(&LinkerManager::OnUpdateTimer, this, std::placeholders::_1))
{
	timer_.async_wait(wait_handler_);
}

// 更新定时器
void LinkerManager::OnUpdateTimer(asio::error_code error_code)
{
	if (error_code)
	{
		logger()->error("{}:{} {}", __FUNCTION__, __LINE__, error_code.message());
		return;
	}

	// 删除超时Token
	for (auto iter = user_auth_.begin(); iter != user_auth_.end();)
	{
		if (std::chrono::steady_clock::now() - iter->second.time >= std::chrono::seconds(60))
		{
			logger()->debug("删除超时的Token {}", iter->first);
			iter = user_auth_.erase(iter);
		}
		{
			++iter;
		}
	}

	// 关闭长时间未验证的连接
	for (auto iter = unauth_user_session_.begin(); iter != unauth_user_session_.end();)
	{
		auto session = static_cast<SessionHandle*>(threads_.SessionHandler(*iter).get());
		if (session != nullptr && session->IsAuthTimeout())
		{
			logger()->debug("关闭长时间未验证连接，{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
			session->Close();
			iter = unauth_user_session_.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	// 上报Linker在线人数
	if (counter_ > 0 && --counter_ == 0)
	{
		svr::ReportLinkerReq request;
		request.set_load(user_session_.size());
		GlobalLoginConnector()->Send(&request);
		counter_ = ServerConfig::GetInstance()->GetReportInterval();
		logger()->info("上报当前在线人数: {}", user_session_.size());
	}

	timer_.expires_from_now(std::chrono::seconds(1));
	timer_.async_wait(wait_handler_);
}

// 用户消息
void LinkerManager::OnUserMessage(SessionHandle *session, google::protobuf::Message *messsage, network::NetMessage &buffer)
{
	// 用户是否验证
	auto found = reverse_user_session_.find(session->SessionID());
	if (found == reverse_user_session_.end())
	{		
		if (messsage->GetDescriptor() != cli::UserAuthReq::descriptor())
		{
			logger()->debug("用户未验证，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
			return SenddErrorCodeToUser(session, buffer, pub::kUserUnverified, messsage->GetTypeName().c_str());
		}

		auto request = static_cast<cli::UserAuthReq*>(messsage);
		auto auth_iter = user_auth_.find(request->token());
		if (auth_iter == user_auth_.end())
		{
			logger()->debug("用户验证失败，来自{}:{}", session->RemoteEndpoint().address().to_string(), session->RemoteEndpoint().port());
			return SenddErrorCodeToUser(session, buffer, pub::kAuthenticationFailure, messsage->GetTypeName().c_str());
		}

		// 是否已验证
		auto session_iter = user_session_.find(auth_iter->second.user_id);
		if (session_iter != user_session_.end())
		{
			// 踢掉上一个会话
			auto session_id = session_iter->second.session_id;
			user_session_.erase(session_iter);
			reverse_user_session_.erase(session_id);
			auto session_ptr = threads_.SessionHandler(session_id);
			if (session_ptr != nullptr)
			{
				session_ptr->Close();
				logger()->warn("用户{}:{}被踢掉线!", session_ptr->RemoteEndpoint().address().to_string(), session_ptr->RemoteEndpoint().port());
			}
		}

		// 更新数据
		SUserSession session_info;
		session_info.token = request->token();
		session_info.session_id = session->SessionID();
		unauth_user_session_.erase(session->SessionID());
		user_session_.insert(std::make_pair(auth_iter->second.user_id, session_info));
		reverse_user_session_.insert(std::make_pair(session->SessionID(), auth_iter->second.user_id));

		// 响应请求
		buffer.Clear();
		cli::UserAuthRsp response;
		response.set_user_id(auth_iter->second.user_id);
		ProtubufCodec::Encode(&response, buffer);
		session->Send(buffer);

		// 用户进入通知
		OnBroadcastUserEnter(auth_iter->second.user_id, buffer);

		logger()->info("用户[{}]验证成功!", auth_iter->second.user_id);
	}
	else
	{
		// 消息转发
	}
}

// 回复错误码
void LinkerManager::SenddErrorCodeToUser(SessionHandle *session, network::NetMessage &buffer, int error_code, const char *what)
{
	buffer.Clear();
	pub::ErrorRsp response;
	response.set_error_code(static_cast<pub::ErrorCode>(error_code));
	if (what != nullptr)
	{
		response.set_what(what);
	}
	ProtubufCodec::Encode(&response, buffer);
	session->Send(buffer);
}

// 用户连接
void LinkerManager::OnUserConnect(SessionHandle *session)
{
	unauth_user_session_.insert(session->SessionID());
}

// 用户关闭连接
void LinkerManager::OnUserClose(SessionHandle *session)
{
	unauth_user_session_.erase(session->SessionID());
	auto found = reverse_user_session_.find(session->SessionID());
	if (found != reverse_user_session_.end())
	{
		auto iter = user_session_.find(found->second);
		assert(iter != user_session_.end());
		if (iter != user_session_.end())
		{
			// 用户离开通知
			OnBroadcastUserLeave(found->first);

			// 断线后Token在一定时间内依然有效
			SUserAuth auth;
			auth.user_id = found->first;
			user_auth_.insert(std::make_pair(iter->second.token, auth));
			user_session_.erase(iter);
		}

		logger()->info("用户[{}]关闭连接!", found->second);
		reverse_user_session_.erase(found);
	}
}

// 广播用户进入
void LinkerManager::OnBroadcastUserEnter(uint32_t user_id, network::NetMessage &buffer)
{
	svr::UserEnterRsp msg;
	msg.set_user_id(user_id);
	std::vector<int> dst_type_lists = { svr::NodeType::kLogicSever, svr::NodeType::kChatServer };
	GlobalGWClient()->Broadcast(dst_type_lists, &msg, buffer);
}

// 广播用户离开
void LinkerManager::OnBroadcastUserLeave(uint32_t user_id)
{
	svr::UserEnterRsp msg;
	msg.set_user_id(user_id);
	std::vector<int> dst_type_lists = { svr::NodeType::kLogicSever, svr::NodeType::kChatServer };
	GlobalGWClient()->Broadcast(dst_type_lists, &msg);
}

// 登录服务器消息
void LinkerManager::OnLoginServerMessage(LoginConnector *connector, google::protobuf::Message *messsage, network::NetMessage &buffer)
{
	if (messsage->GetDescriptor() == svr::UpdateTokenReq::descriptor())
	{
		auto request = static_cast<svr::UpdateTokenReq*>(messsage);

		// 更新Token
		SUserAuth auth;
		auth.user_id = request->user_id();
		user_auth_.insert(std::make_pair(request->token(), auth));
		logger()->info("用户[{}]更新Token[{}]", request->user_id(), request->token());
	}
	else
	{
		logger()->warn("已忽略来自登录服务器的请求，{}", messsage->GetTypeName());
	}
}

// 网关服务器消息
void LinkerManager::OnGatewayServerMessage(gw::GWClient *connector, google::protobuf::Message *messsage, network::NetMessage &buffer)
{
	if (messsage->GetDescriptor() == svr::LinkerForward::descriptor())
	{
		OnForwardMessageToUser(messsage, buffer);
	}
	else if (messsage->GetDescriptor() == svr::LinkerBroadcast::descriptor())
	{
		OnBroadcastMessageToUser(messsage, buffer);
	}
	else if (messsage->GetDescriptor() == svr::CloseUserReq::descriptor())
	{
		OnCloseUserConnection(messsage, buffer);
	}
	else
	{
		logger()->warn("已忽略来自网关服务器的请求，{}", messsage->GetTypeName());
	}
}

// 获取用户Session
bool LinkerManager::GetUserSession(uint32_t user_id, network::SessionHandlePointer &session)
{
	auto found = user_session_.find(user_id);
	if (found == user_session_.end())
	{
		return false;
	}
	session = threads_.SessionHandler(found->second.session_id);
	return session != nullptr;
}

// 向用户转发消息
void LinkerManager::OnForwardMessageToUser(google::protobuf::Message *messsage, network::NetMessage &buffer)
{
	network::SessionHandlePointer session;
	auto request = static_cast<svr::LinkerForward*>(messsage);
	if (GetUserSession(request->user_id(), session))
	{
		buffer.Clear();
		buffer.Write(request->user_data().data(), request->user_data().size());	
		session->Send(buffer);
	}
}

// 向用户广播消息
void LinkerManager::OnBroadcastMessageToUser(google::protobuf::Message *messsage, network::NetMessage &buffer)
{
	auto request = static_cast<svr::LinkerBroadcast*>(messsage);
	if (!user_session_.empty())
	{
		buffer.Clear();
		buffer.Write(request->user_data().data(), request->user_data().size());
		for (const auto &pair : user_session_)
		{
			network::SessionHandlePointer session = threads_.SessionHandler(pair.first);
			if (session != nullptr)
			{
				session->Send(buffer);
			}
		}
	}
}

// 关闭用户连接
void LinkerManager::OnCloseUserConnection(google::protobuf::Message *messsage, network::NetMessage &buffer)
{
	network::SessionHandlePointer session;
	auto request = static_cast<svr::CloseUserReq*>(messsage);
	if (GetUserSession(request->user_id(), session))
	{
		user_session_.erase(request->user_id());
		reverse_user_session_.erase(session->SessionID());

		buffer.Clear();
		cli::CloseConnection msg;
		msg.set_error_code(0);
		ProtubufCodec::Encode(&msg, buffer);
		session->Send(buffer);
		session->Close();
	}
}