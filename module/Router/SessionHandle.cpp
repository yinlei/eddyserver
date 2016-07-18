﻿#include "SessionHandle.h"
#include <ProtobufCodec.h>
#include <proto/internal.pb.h>
#include "Logging.h"
#include "RouterManager.h"
#include "StatisticalTools.h"

SessionHandle::SessionHandle(RouterManager &router_manager)
	: is_logged_(false)
	, router_manager_(router_manager)
{
}

// 连接事件
void SessionHandle::OnConnect()
{
}

// 接收消息事件
void SessionHandle::OnMessage(network::NetMessage &message)
{
	// 统计上行流量
	StatisticalTools::GetInstance()->AccumulateUpVolume(message.Readable() + sizeof(network::DefaultMessageFilter::MessageHeader));

	// 处理请求
	auto request = ProtubufCodec::Decode(message);
	if (request == nullptr)
	{
		router_manager_.RespondErrorCode(*this, message, internal::kInvalidProtocol);
		return;
	}

	// 连接后必须登录
	if (!is_logged_)
	{
		if (dynamic_cast<internal::PingReq*>(request.get()) == nullptr)
		{
			if (dynamic_cast<internal::LoginRouterReq*>(request.get()) == nullptr)
			{
				router_manager_.RespondErrorCode(*this, message, internal::kNotLoggedIn);
				logger()->warn("操作前未发起登录请求，来自{}:{}", RemoteEndpoint().address().to_string(), RemoteEndpoint().port());
				return;
			}
			else
			{
				is_logged_ = router_manager_.HandleMessage(*this, request.get(), message);
				return;
			}
		}
	}

	// 处理心跳包
	if (dynamic_cast<internal::PingReq*>(request.get()) != nullptr)
	{
		message.Clear();
		internal::PongRsp response;
		ProtubufCodec::Encode(&response, message);
		Respond(message);
	}
	else
	{
		router_manager_.HandleMessage(*this, request.get(), message);
	}
}

// 关闭事件
void SessionHandle::OnClose()
{
	router_manager_.HandleServerOffline(*this);
}

// 回复消息
void SessionHandle::Respond(const network::NetMessage &message)
{
	StatisticalTools::GetInstance()->AccumulateDownVolume(message.Readable() + sizeof(network::DefaultMessageFilter::MessageHeader));
	Send(message);
}

/************************************************************************/
// 创建消息筛选器
network::MessageFilterPointer CreateMessageFilter()
{
	return std::make_shared<network::DefaultMessageFilter>();
}

network::SessionHandlePointer CreateSessionHandle(RouterManager &router_manager)
{
	return std::make_shared<SessionHandle>(router_manager);
}
/************************************************************************/