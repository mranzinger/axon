/*
 * i_data_server.h
 *
 *  Created on: Feb 9, 2014
 *      Author: Mike
 */

#ifndef I_DATA_SERVER_H_
#define I_DATA_SERVER_H_

#include "i_data_connection.h"

namespace axon { namespace communication {

class AXON_COMMUNICATE_API IDataServer
{
public:
	typedef std::shared_ptr<IDataServer> Ptr;

	typedef std::function<void (IDataConnection::Ptr)> ConnectedHandler;
	typedef ConnectedHandler DisconnectedHandler;

	typedef std::function<Ptr (const std::string &)> DataServerFactory;

	virtual ~IDataServer() { }

	static Ptr Create(const std::string &hostString);
	static void RegisterFactory(const std::string &protocolId, const DataServerFactory &f);

	virtual void Startup(const std::string &hostString) = 0;

	virtual std::string HostString() const = 0;

	virtual void Shutdown() = 0;

	virtual size_t NumClients() const = 0;

	virtual void Broadcast(const util::CBuffer &a_buff) = 0;

	virtual void SetConnectedHandler(ConnectedHandler a_handler) = 0;
	virtual void SetDisconnectedHandler(DisconnectedHandler a_handler) = 0;
};

template<typename DataServerType>
struct register_server
{
	register_server(const std::string &protocolId)
	{
		IDataServer::RegisterFactory(protocolId,
			[] (const std::string &hStr) -> IDataServer::Ptr
			{
				auto ds = std::make_shared<DataServerType>();

				ds->Startup(hStr);

				return ds;
			});
	}
};

} }



#endif /* I_DATA_SERVER_H_ */
