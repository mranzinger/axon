/*
 * proxy_server.cpp
 *
 *  Created on: Dec 12, 2014
 *      Author: mranzinger
 */

#include <functional>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>

#include <communication/messaging/axon_proxy_server.h>
#include <communication/tcp/tcp_data_server.h>
#include <communication/tcp/tcp_data_connection.h>

using namespace std;

namespace po = boost::program_options;
namespace comm = axon::communication;

const comm::CContract<void (string)> ADD_CLIENT_CONTRACT("CAxonProxyServer.AddClient");
const comm::CContract<void (string)> REMOVE_CLIENT_CONTRACT("CAxonProxyServer.RemoveClient");

void AddClient(const comm::CAxonProxyServer::Ptr &a_server, const string &a_info);
void RemoveClient(const comm::CAxonProxyServer::Ptr &a_server, const string &a_info);

int main(int argc, char *argv[])
{
    unsigned int l_port = 0;

    vector<string> l_clients;

    po::options_description desc("Allowed Options");
    desc.add_options()
            ("help,h", "produce help message")
            ;

    {
        po::options_description mand("Mandatory");
        mand.add_options()
                ("port,p", po::value(&l_port),
                        "Server Port.")
                ;
        desc.add(mand);
    }
    {
        po::options_description opt("Optional");
        opt.add_options()
                ("clients,c", po::value(&l_clients),
                        "List of initial clients")
                ;
        desc.add(opt);
    }

    po::variables_map varMap;
    po::store(po::parse_command_line(argc, argv, desc), varMap);
    po::notify(varMap);

    if (varMap.count("help"))
    {
        cout << desc << endl;
        return EXIT_SUCCESS;
    }

    comm::CAxonProxyServer::Ptr l_server = comm::CAxonProxyServer::Create(
            make_shared<comm::tcp::CTcpDataServer>(l_port)
    );

    l_server->HostContract(ADD_CLIENT_CONTRACT,
                    bind(&AddClient,
                         l_server,
                         placeholders::_1
                         ));
    l_server->HostContract(REMOVE_CLIENT_CONTRACT,
                    bind(&RemoveClient,
                         l_server,
                         placeholders::_1
                         ));

    for (const string &l_c : l_clients)
    {
        AddClient(l_server, l_c);
    }

    while (true)
    {
        cout << "Enter command:" << endl
             << "add [host name]:[port]" << endl
             << "remove [host name](:[port])" << endl
             << "q[uit]" << endl
             << endl << "cmd: " << flush;

        string l_line;
        getline(cin, l_line);

        size_t l_pos = l_line.find("add ");

        if (l_pos == 0)
        {
            string l_sub = l_line.substr(4);

            AddClient(l_server, l_sub);
        }
        else
        {
            l_pos = l_line.find("remove ");

            if (l_pos == 0)
            {
                string l_str = l_line.substr(7);

                RemoveClient(l_server, l_str);
            }
            else
            {
                l_pos = l_line.find("q");

                if (l_pos == 0)
                {
                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

void AddClient(const comm::CAxonProxyServer::Ptr &a_server, const string& a_info)
{
    size_t l_pos = a_info.find(':');

    if (l_pos == string::npos)
    {
        cout << "Invalid add command. Requires [host name]:[port]" << endl;
        return;
    }

    string l_host = a_info.substr(0, l_pos);
    string l_sPort = a_info.substr(l_pos + 1);
    auto l_port = boost::lexical_cast<int>(l_sPort);

    vector<comm::IDataConnection::Ptr> l_conns;

    for (size_t i = 0; i < 2; ++i)
    {
        auto l_conn = comm::tcp::CTcpDataConnection::Create(l_host, l_port);

        l_conns.push_back(move(l_conn));
    }

    a_server->AddProxies(l_conns);
}

void RemoveClient(const comm::CAxonProxyServer::Ptr &a_server, const string& a_info)
{
    a_server->RemoveProxy(a_info);
}
