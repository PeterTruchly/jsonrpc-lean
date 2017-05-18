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

#ifndef JSONRPC_LEAN_RESPONSE_H
#define JSONRPC_LEAN_RESPONSE_H

#include "value.h"

#include "jsonwriter.h"

namespace jsonrpc {

	class Response {
	public:
		Response(Value value, int32_t id) :
			myResult(std::move(value)),
			myIsFault(false),
			myFaultCode(0),
			myId(id)
		{
        }

        Response(int32_t faultCode, const std::string & faultString, int32_t id) :
    		myIsFault(true),
            myFaultCode(faultCode),
            myFaultString(std::move(faultString)),
			myId(id)
    	{
        }

        void Write(JsonWriter& writer) const {
            if (myIsFault) {
                writer.WriteFault(myFaultCode, myId, myFaultString);
            } else {
                writer.WriteResponse(myId, myResult);                
            }
        }

        Value& GetResult() { return myResult; }
        bool IsFault() const { return myIsFault; }

        void ThrowIfFault() const {
            if (!IsFault()) {
                return;
            }

            switch (static_cast<Fault::ReservedCodes>(myFaultCode)) {
            case Fault::RESERVED_CODE_MIN:
            case Fault::RESERVED_CODE_MAX:
            case Fault::SERVER_ERROR_CODE_MIN:
                break;
            case Fault::PARSE_ERROR:
                throw ParseErrorFault(myFaultString);
            case Fault::INVALID_REQUEST:
                throw InvalidRequestFault(myFaultString);
            case Fault::METHOD_NOT_FOUND:
                throw MethodNotFoundFault(myFaultString);
            case Fault::INVALID_PARAMETERS:
                throw InvalidParametersFault(myFaultString);
            case Fault::INTERNAL_ERROR:
                throw InternalErrorFault(myFaultString);
            }

            if (myFaultCode >= Fault::SERVER_ERROR_CODE_MIN
                && myFaultCode <= Fault::SERVER_ERROR_CODE_MAX) {
                throw ServerErrorFault(myFaultCode, myFaultString);
            }

            if (myFaultCode >= Fault::RESERVED_CODE_MIN
                && myFaultCode <= Fault::RESERVED_CODE_MAX) {
                throw PreDefinedFault(myFaultCode, myFaultString);
            }

            throw Fault(myFaultString, myFaultCode);
        }

        int32_t GetId() const { return myId; }

    private:
        Value myResult;
        bool myIsFault;
        int32_t myFaultCode;
        std::string myFaultString;
		int32_t myId;
    };

} // namespace jsonrpc

#endif // JSONRPC_LEAN_RESPONSE_H
