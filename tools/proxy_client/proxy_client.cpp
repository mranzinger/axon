/*
 * proxy_client.cpp
 *
 *  Created on: Dec 12, 2014
 *      Author: mranzinger
 */

#include <functional>
#include <iostream>

#include <boost/program_options.hpp>

#include <communication/messaging/axon_client.h>
#include <communication/tcp/tcp_data_connection.h>

using namespace std;

namespace po = boost::program_options;
namespace comm = axon::communication;

const comm::CContract<void (string)> ADD_CLIENT_CONTRACT("CAxonProxyServer.AddClient");
const comm::CContract<void (string)> REMOVE_CLIENT_CONTRACT("CAxonProxyServer.RemoveClient");

int main(int argc, char *argv[])
{
    string l_proxyStr;

    vector<string> l_addList;
    vector<string> l_removeList;

    po::options_description desc("Allowed Options");
    desc.add_options()
            ("help,h", "produce help message")
            ("add,a", po::value(&l_addList),
                    "Add client(s) '[host name]:[port]'")
            ("remove,r", po::value(&l_removeList),
                    "Remove client(s) '[host name](:[port])'")
            ;

    po::variables_map varMap;
    po::store(po::parse_command_line(argc, argv, desc), varMap);
    po::notify(varMap);

    if (varMap.count("help"))
    {
        cout << desc << endl;
        return EXIT_SUCCESS;
    }

    auto l_client = comm::CAxonClient::Create("tcp://" + l_proxyStr);

    for (const string &l_add : l_addList)
    {
        cout << "Adding " << l_add << endl;
        l_client->VSend(ADD_CLIENT_CONTRACT, l_add);
    }
    for (const string &l_remove : l_removeList)
    {
        cout << "Removing " << l_remove << endl;
        l_client->VSend(REMOVE_CLIENT_CONTRACT, l_remove);
    }

    return EXIT_SUCCESS;
}


