﻿#ifndef __IO_SERVICE_THREAD_H__
#define __IO_SERVICE_THREAD_H__

#include <thread>
#include <asio/io_service.hpp>
#include <asio/steady_timer.hpp>
#include "TCPSessionQueue.h"

namespace network
{
	class IOServiceThreadManager;

	class IOServiceThread : public std::enable_shared_from_this < IOServiceThread >
	{
		friend class IOServiceThreadManager;
		IOServiceThread(const IOServiceThread&) = delete;
		IOServiceThread& operator=(const IOServiceThread&) = delete;

	public:
		IOServiceThread(IOThreadID id, IOServiceThreadManager &manager);
		~IOServiceThread() = default;

	public:
		void RunThread();

		void Join();

		void Stop();

		template <typename CompletionHandler>
		void Post(ASIO_MOVE_ARG(CompletionHandler) handler)
		{
			io_service_.post(handler);
		}

	public:
		IOThreadID ID() const
		{
			return thread_id_;
		}

		asio::io_service& IOService()
		{
			return io_service_;
		}

		IOServiceThreadManager& ThreadManager()
		{
			return manager_;
		}

		TCPSessionQueue& SessionQueue()
		{
			return session_queue_;
		}

	private:
		void Run();

		void CheckSessionKeepAliveTime(asio::error_code error);

	private:
		IOThreadID								thread_id_;
		IOServiceThreadManager&					manager_;
		asio::io_service						io_service_;
		asio::steady_timer						timer_;
		std::unique_ptr<std::thread>			thread_;
		std::unique_ptr<asio::io_service::work>	work_;
		TCPSessionQueue							session_queue_;
	};
}

#endif