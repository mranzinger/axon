/*
 * i_data_connection.h
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#ifndef I_DATA_CONNECTION_H_
#define I_DATA_CONNECTION_H_

#include <memory>
#include <condition_variable>
#include <functional>
#include <string>

#include "util/buffer.h"
#include "messaging/data_buffer.h"
#include "timeout_exception.h"

#include "dll_export.h"

namespace axon { namespace communication {

class AXON_COMMUNICATE_API IDataConnection
{
public:
	typedef std::shared_ptr<IDataConnection> Ptr;
	typedef std::weak_ptr<IDataConnection> WeakPtr;

	// Callback that gets invoked when data is received from the connection.
	typedef std::function<void (CDataBuffer)> DataReceivedHandler;

	typedef std::function<Ptr (const std::string &)> DataConnectionFactory;

	virtual ~IDataConnection() { }

	/*
	 * Factory function that will resolve the connection string, and return
	 * back a connected instance of the specified type.
	 *
	 * The syntax is:
	 * [protocol id]://[protocol specific string]
	 *
	 * Ex: To connect to a remote computer using TCP/IP, the string would be:
	 * tcp://localhost:8080
	 *
	 * If you want to use TLS, then the string is similar, but looks like:
	 * secure://[protocol id]://[protocol specific string]
	 *
	 * Ex:
	 * secure://tcp://localhost:8080
	 *
	 * Refer to protocol specific documentation for what should be supplied
	 * in the latter part of the string
	 *
	 * NOTE: If no protocol specific string is supplied, then a default non-connected
	 * instance of that protocol handler will be supplied.
	 *
	 * Ex:
	 * tcp
	 * secure://tcp
	 */
	static Ptr Create(const std::string &a_connectionString);

	static void RegisterFactory(const std::string &a_protocolId, const DataConnectionFactory &a_factory);

	virtual std::string ConnectionString() const = 0;

	virtual bool Connect(const std::string &a_connectionString) = 0;
	virtual bool Reconnect() = 0;

	virtual void Close() = 0;

	virtual bool IsOpen() const = 0;
	virtual bool IsServerClient() const = 0;

	virtual void Send(const util::CBuffer &a_buff)
	{
		Send(a_buff, nullptr);
	}

	virtual void Send(const util::CBuffer &buff, std::condition_variable *finishEvt) = 0;

	virtual void SetReceiveHandler(DataReceivedHandler handler) = 0;
};

/*
 * Templated helper class that will automatically register a protocol type
 * with IDataConnection, which the Create function uses to resolve the connection
 * string.
 *
 * Usage: (.cpp)
 * register_protocol<TcpClient> s_tcpRegister("tcp");
 *
 * Protocols that use this registration mechanism must have a parameterless constructor
 */
template<typename DataConnectionType>
struct register_protocol
{
	register_protocol(const std::string &protocolId)
	{
		IDataConnection::RegisterFactory(protocolId,
			[] (const std::string &cStr) -> IDataConnection::Ptr
			{
				IDataConnection::Ptr ret = std::make_shared<DataConnectionType>();

				if (ret->Connect(cStr))
					return ret;
				else
					return IDataConnection::Ptr();
			});
	}
};

} }



#endif /* I_DATA_CONNECTION_H_ */
