// This file is derived from xsonrpc Copyright (C) 2015 Erik Johansson <erik@ejohansson.se>
// This file is part of jsonrpc-lean, a c++11 JSON-RPC client/server library.
//
// Modifications and additions Copyright (C) 2015 Adriano Maia <tony@stark.im>
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation; either version 2.1 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#ifndef JSONRPC_LEAN_SERVER_H
#define JSONRPC_LEAN_SERVER_H

#include "request.h"
#include "value.h"
#include "fault.h"
#include "jsonreader.h"
#include "response.h"
#include "jsonwriter.h"
#include "jsonformatteddata.h"
#include "dispatcher.h"
#include "datagram_based.h"

#include <mutex>
#include <atomic>

namespace jsonrpc {

    class Server : public IDatagramConsumer {
    public:
		explicit Server(IDatagramProducerConsumer & transmitter) :
			m_transmitter(transmitter),
			m_enabled(false)
        {
			transmitter.registerService(*this);
        }

        ~Server() {}

        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        Server(Server&&) = delete;
        Server& operator=(Server&&) = delete;

        Dispatcher& GetDispatcher() { return myDispatcher; }

	    void datagramConsume(const std::string & message) override
		{		
			std::lock_guard<std::mutex> backlogLock(m_backlogMutex);

			m_backlog.emplace_back(message);
			processBacklog();
		}

		void enableService()
        {
			std::lock_guard<std::mutex> backlogLock(m_backlogMutex);

			m_enabled = true;
			processBacklog();
        }

        // aContentType is here to allow future implementation of other rpc formats with minimal code changes
        // Will return NULL if no FormatHandler is found, otherwise will return a FormatedData
        // If aRequestData is a Notification (the client doesn't expect a response), the returned FormattedData will have an empty ->GetData() buffer and ->GetSize() will be 0
        std::shared_ptr<jsonrpc::JsonFormattedData> HandleRequest(const std::string& aRequestData) const
        {                        
            auto writer = std::make_unique<JsonWriter>();

            try {
                auto reader = std::make_unique<JsonReader>(std::move(aRequestData));
                Request request = reader->GetRequest();
                reader.reset();

                auto response = myDispatcher.Invoke(request.GetMethodName(), request.GetParameters(), request.GetId());
                if (response.GetId()) // if Id is 0, this is a notification and we don't have to write a response
				{
                    
                    response.Write(*writer);
                }
            } catch (const Fault& ex) {
                Response(ex.GetCode(), ex.GetString(), 0).Write(*writer);
            }

            return writer->GetData();
        }
    private:
		void processBacklog() //needs to be called when holding m_backlogMutex
		{
			if (m_enabled)
			{
				for (const auto & message : m_backlog)
				{
					datagramConsumeImpl_(message);
				}

				m_backlog.clear();
			}
		}

		void datagramConsumeImpl_(const std::string & message) const
		{
			auto response = HandleRequest(message);
			m_transmitter.datagramTransmit(response->GetData());
		}

        Dispatcher myDispatcher;
		IDatagramTransmitter & m_transmitter;
		std::atomic_bool m_enabled;
		std::vector<std::string> m_backlog;
		std::mutex m_backlogMutex;
    };

} // namespace jsonrpc

#endif // JSONRPC_LEAN_SERVER_H
