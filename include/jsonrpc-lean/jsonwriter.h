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

#ifndef JSONRPC_LEAN_JSONWRITER_H
#define JSONRPC_LEAN_JSONWRITER_H

#include "json.h"
#include "value.h"
#include "jsonformatteddata.h"

#include "nlohmann/json.hpp"

namespace jsonrpc {

    class JsonWriter final {
    public:
        JsonWriter()
    	{
        }

		//TODO: this is breaking encapuslation of the class
        // Writer result - formatted json message
        std::shared_ptr<JsonFormattedData> GetData() 
    	{
            return myRequestData;
        }

		// Request
        nlohmann::json & StartRequest(const std::string& methodName, const int32_t id) 
    	{
			myRequestData = std::make_unique<JsonFormattedData>(id);
			myRequestData->Writer[json::JSONRPC_NAME] = jsonrpc::json::JSONRPC_VERSION_2_0;
            myRequestData->Writer[json::METHOD_NAME] = methodName;
			myRequestData->Writer[json::ID_NAME] = id;

			myRequestData->Writer[json::PARAMS_NAME] = nlohmann::json::array(); // param array started and returned here...
			return myRequestData->Writer[json::PARAMS_NAME];
        }

		// Response when successful
        void WriteResponse(const int32_t id, const jsonrpc::Value & value)
    	{
			myRequestData = std::make_unique<JsonFormattedData>(id);
			myRequestData->Writer[json::JSONRPC_NAME] = json::JSONRPC_VERSION_2_0;
			myRequestData->Writer[json::ID_NAME] = id;
			value.Write(myRequestData->Writer[json::RESULT_NAME]);
        }

		// Response when failed
		void WriteFault(int32_t code, const int32_t id, const std::string& string)
    	{
			myRequestData = std::make_unique<JsonFormattedData>(id);
			myRequestData->Writer[json::JSONRPC_NAME] = json::JSONRPC_VERSION_2_0;
			myRequestData->Writer[json::ID_NAME] = id;
            myRequestData->Writer[json::ERROR_NAME][json::ERROR_CODE_NAME] = code;
			myRequestData->Writer[json::ERROR_NAME][json::ERROR_MESSAGE_NAME] = string;
        }

    private:    

        std::shared_ptr<JsonFormattedData> myRequestData;
    };

} // namespace jsonrpc

#endif // JSONRPC_LEAN_JSONWRITER_H
