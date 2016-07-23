﻿#include "SessionHandle.h"
#include <ProtobufCodec.h>
#include <proto/public_struct.pb.h>
#include <proto/server_internal.pb.h>
#include "LoginManager.h"

SessionHandle::SessionHandle(LoginManager &login_manager)
	: login_manager_(login_manager)
	, type_(RoleType::kUser)
{
}

// 获取角色类型
RoleType SessionHandle::GetRoleType() const
{
	return type_;
}

// 连接事件
void SessionHandle::OnConnect()
{
	login_manager_.HandleAcceptConnection(*this);
}

// 接收消息事件
void SessionHandle::OnMessage(network::NetMessage &message)
{
	// 处理请求
	auto request = ProtubufCodec::Decode(message);
	if (request == nullptr)
	{
		login_manager_.RespondErrorCode(*this, message, pub::kInvalidProtocol);
		return;
	}

	// 注册Linker
	if (dynamic_cast<svr::LinkerLoginReq*>(request.get()) != nullptr)
	{
		if (login_manager_.HandleLinkerMessage(*this, request.get(), message))
		{
			type_ = RoleType::kLinker;
		}
		return;
	}

	// 处理心跳
	if (dynamic_cast<pub::PingReq*>(request.get()) != nullptr)
	{
		message.Clear();
		pub::PongRsp response;
		ProtubufCodec::Encode(&response, message);
		Send(message);
		return;
	}

	// 处理消息
	if (type_ == RoleType::kUser)
	{
		login_manager_.HandleUserMessage(*this, request.get(), message);
	}
	else
	{
		login_manager_.HandleLinkerMessage(*this, request.get(), message);
	}
}

// 关闭事件
void SessionHandle::OnClose()
{
	if (type_ == RoleType::kUser)
	{
		login_manager_.HandleUserOffline(*this);
	}
	else
	{
		login_manager_.HandleLinkerOffline(*this);
	}
}

/************************************************************************/
// 创建消息筛选器
network::MessageFilterPointer CreateMessageFilter()
{
	return std::make_shared<network::DefaultMessageFilter>();
}

network::SessionHandlePointer CreateSessionHandle(LoginManager &login_manager)
{
	return std::make_shared<SessionHandle>(login_manager);
}
/************************************************************************/