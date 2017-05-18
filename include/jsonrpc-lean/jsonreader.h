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

#ifndef JSONRPC_LEAN_JSONREADER_H
#define JSONRPC_LEAN_JSONREADER_H

#include "fault.h"
#include "json.h"
#include "request.h"
#include "response.h"
#include "value.h"

#include "json.hpp"

namespace jsonrpc {

    class JsonReader final {
    public:
	    explicit JsonReader(const std::string& data) {			
			myDocument = nlohmann::json::parse(data.c_str());
        }

		explicit JsonReader(const nlohmann::json & json) {
			myDocument = json;
		}

        // Reader
        Request GetRequest() {
            if (!myDocument.is_object()) {
                throw InvalidRequestFault();
            }

            ValidateJsonrpcVersion();

            auto method = myDocument.find(json::METHOD_NAME);
            if (method == myDocument.end() || !method->is_string()) {
                throw InvalidRequestFault();
            }

            Request::Parameters parameters;
            auto params = myDocument.find(json::PARAMS_NAME);
            if (params != myDocument.end()) {
                if (!params->is_array()) {
                    throw InvalidRequestFault();
                }

                for (auto param = params.value().begin(); param != params.value().end();
                    ++param) {
                    parameters.emplace_back(GetValue(*param));
                }
            }

            auto id = myDocument.find(json::ID_NAME);
            if (id == myDocument.end()) {
                // Notification
                return Request(method->get<std::string>(), std::move(parameters), false);
            }

            return Request(method.value().get<std::string>(), std::move(parameters), GetId(id.value()));
        }

        Response GetResponse() {
            if (!myDocument.is_object()) {
                throw InvalidRequestFault();
            }

            ValidateJsonrpcVersion();

            auto id = myDocument.find(json::ID_NAME);
            if (id == myDocument.end()) {
                throw InvalidRequestFault();
            }

            auto result = myDocument.find(json::RESULT_NAME);
            auto error = myDocument.find(json::ERROR_NAME);

            if (result != myDocument.end()) {
                if (error != myDocument.end()) {
                    throw InvalidRequestFault();
                }
                return Response(GetValue(result.value()), GetId(id.value()));
            } else if (error != myDocument.end()) {
                if (result != myDocument.end()) {
                    throw InvalidRequestFault();
                }
                if (!error.value().is_object()) {
                    throw InvalidRequestFault();
                }
                auto code = error.value().find(json::ERROR_CODE_NAME);
                if (code == error.value().end() || !code.value().is_number_integer()) {
                    throw InvalidRequestFault();
                }
                auto message = error.value().find(json::ERROR_MESSAGE_NAME);
                if (message == error.value().end() || !message.value().is_string()) {
                    throw InvalidRequestFault();
                }

                return Response(code.value().get<int>(), message.value().get<std::string>(),
                    GetId(id.value()));
            } else {
                throw InvalidRequestFault();
            }
        }

        Value GetValue() {
            return GetValue(myDocument);
        }

    private:
        void ValidateJsonrpcVersion() const {
            auto jsonrpc = myDocument.find(json::JSONRPC_NAME);
            if (jsonrpc == myDocument.end()
                || !jsonrpc.value().is_string()
                || (jsonrpc.value().get<std::string>() != json::JSONRPC_VERSION_2_0)) {
                throw InvalidRequestFault();
            }
        }

        Value GetValue(const nlohmann::json& value) const {
            switch (value.type()) {
			case nlohmann::json::value_t::null :
                return Value();
			case nlohmann::json::value_t::boolean :
                return Value(value.get<bool>());
            case  nlohmann::json::value_t::object : {
                Value::Struct data;
                for (auto it = value.begin(); it != value.end(); ++it) {                    
                    data.emplace(it.key(), GetValue(it.value()));
                }
                return Value(std::move(data));
            }
            case nlohmann::json::value_t::array : {
                Value::Array array;
                //array.reserve(value.Size()); //it is jut the vector<Value> underneath, it is gonna be OK
                for (auto & el : value) {
                    array.emplace_back(GetValue(el));
                }
                return Value(std::move(array));
            }
            case nlohmann::json::value_t::string : 
            {
                return Value(value.get<std::string>());
            }
			case nlohmann::json::value_t::number_float :
			case nlohmann::json::value_t::number_integer:
			case nlohmann::json::value_t::number_unsigned:
                if (value.is_number_float()) {
                    return Value(value.get<double>());
                } else if (value.is_number_integer()) {
                    return Value(value.get<int64_t>());
                } else if (value.is_number_unsigned()) {
                    return Value(static_cast<int64_t>(value.get<uint64_t>())); //why getting unsigned and then converting to signed?
                } else {
                    assert(value.is_number());
                    return Value(value.get<double>());
                }
                break;
            }

            throw InternalErrorFault();
        }

		int32_t GetId(const nlohmann::json& id) const {
            if (id.is_string()) {
                return std::stoi(id.get<std::string>());
            } else if (id.is_number_integer()) {
                return id.get<int32_t>();
            } else if (id.is_null()) {
                return {};
            }

            throw InvalidRequestFault();
        }

        std::string myData;
		nlohmann::json myDocument;
    };

} // namespace jsonrpc

#endif // JSONRPC_LEAN_JSONREADER_H
