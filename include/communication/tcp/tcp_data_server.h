/*
 * File description: tcp_data_server.h
 * Author information: Mike Ranzinger mranzinger@alchemyapi.com
 * Copyright information: Copyright Orchestr8 LLC
 */

#ifndef TCP_DATA_SERVER_H_
#define TCP_DATA_SERVER_H_

#include "../i_data_server.h"

namespace axon { namespace communication { namespace tcp {

class CTcpDataServer
	: public virtual IDataServer
{
public:
	class Impl;

private:
	std::unique_ptr<Impl> m_impl;

public:
	typedef std::shared_ptr<CTcpDataServer> Ptr;

	CTcpDataServer();
	CTcpDataServer(const std::string &a_hostString);
	CTcpDataServer(int a_port);

	~CTcpDataServer();

	virtual void Startup(const std::string &a_hostString) override;
	void Startup(int a_port);

	virtual std::string HostString() const override;

	virtual void Shutdown() override;

	virtual size_t NumClients() const override;

	virtual void Broadcast(const util::CBuffer &buff) override;

	virtual void SetConnectedHandler(ConnectedHandler a_handler) override;
	virtual void SetDisconnectedHandler(DisconnectedHandler a_handler) override;
};

} } }

#endif

