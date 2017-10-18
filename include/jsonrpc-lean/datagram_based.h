#ifndef JSONRPC_LEAN_DATAGRAM_BASED_H
#define JSONRPC_LEAN_DATAGRAM_BASED_H

#include <string>

namespace jsonrpc 
{

	class IDatagramTransmitter
	{
	public:
		virtual ~IDatagramTransmitter() = default;

		virtual void datagramTransmit(const std::string & request) = 0;
	};

	class IDatagramConsumer
	{
	public:
		virtual ~IDatagramConsumer() = default;

		virtual void datagramConsume(const std::string & response) = 0;

		virtual void transportDisconnect(const std::string & error) = 0;
	};

	// This is an adaptation component. Capable of connecting client and/or service (or both at once) to the single communication channel.
	class IDatagramProducerConsumer : public IDatagramTransmitter, public IDatagramConsumer
	{
	public:
		virtual ~IDatagramProducerConsumer() = default;

		void datagramConsume(const std::string & message) override = 0;

		void datagramTransmit(const std::string & request) override = 0;

		// register client into this adapter (client is a consumer when it comes to processing a response)
		virtual void registerClient(IDatagramConsumer & client) = 0;

		// register service into this adapter (service is a consumer when it receives a method call)
		virtual void registerService(IDatagramConsumer& service) = 0;
	};

}

#endif //JSONRPC_LEAN_DATAGRAM_BASED_H