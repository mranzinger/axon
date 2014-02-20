/*
 * File description: tcp_data_connection.h
 * Author information: Mike Raninger mikeranzinger@gmail.com
 * Copyright information: Copyright Mike Ranzinger
 */

#ifndef TCP_DATA_CONNECTION_H_
#define TCP_DATA_CONNECTION_H_

#include <memory>

#include "../i_data_connection.h"


namespace axon { namespace communication { namespace tcp {

class AXON_COMMUNICATE_API CTcpDataConnection
	: public virtual IDataConnection
{
public:
	class Impl;

private:
	std::unique_ptr<Impl> m_impl;

public:
	typedef std::shared_ptr<CTcpDataConnection> Ptr;

	CTcpDataConnection();
	CTcpDataConnection(const std::string &a_connectionString);
	CTcpDataConnection(std::string a_hostName, int a_port);
	CTcpDataConnection(std::unique_ptr<Impl> a_impl);

	~CTcpDataConnection();

	virtual std::string ConnectionString() const;

	bool Connect(std::string a_hostName, int a_port);
	virtual bool Connect(const std::string &a_connectionString);

	virtual void Close();

	virtual bool IsOpen() const;
	virtual bool IsServerClient() const;

	virtual void Send(const util::CBuffer &buff, std::condition_variable *finishEvt);

	virtual void SetReceiveHandler(DataReceivedHandler handler);

	Impl *GetImpl() const { return m_impl.get(); }
};

}
}
}



#endif /* TCP_DATA_CONNECTION_H_ */
