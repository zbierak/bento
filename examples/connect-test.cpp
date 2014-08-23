/*
 * connect-test.cpp
 *
 *  Created on: Nov 06, 2013
 *      Author: zbierak
 *
 * This demo presents a simple network of nodes that connect to each other.
 * When all connections are established, the program closes and displays a message.
 */

#include <iostream>

#include <bento/node.h>
#include <bento/exceptions.h>

#include <boost/lexical_cast.hpp>

using namespace std;

int32_t MESSAGE_TYPE_GATHER_SAMPLE = 1;

class ConnectNode: public bento::Node
{
public:
	ConnectNode(const std::string& name, const std::string& topologyFileName): Node(bento::Topology(name, topologyFileName))
	{
		this->registerGatherMessage(MESSAGE_TYPE_GATHER_SAMPLE, 2);
	}

    virtual void onConnect()
    {
    	cout << "Node has connected to all its neighbors, passing hello messages." << endl;

    	pass("HELLO!");
    	pass(MESSAGE_TYPE_GATHER_SAMPLE, "SAMPLE GATHER MESSAGE");
    }

    void onMessage(const std::string& from, const int32_t type, const std::string& msg)
    {
    	if (type < 0)
    		cout << "Node " << getName() << " obtained message " << msg << " from " << from << endl;
    	else
    		cout << "Node " << getName() << " obtained message of type " << type << " with contents " << msg << " from " << from << endl;
    }
};

int main(int argc, char *argv[])
{
	// relative path, assuming the software is build in a subdirectory
	const string TOPOLOGY_FILE = "../connect-test.top";

	if (argc < 2)
	{
		cout << "Missing arguments. The application should be called:" << endl;
		cout << "\tconnect-test <node-name>" << endl;
		return -1;
	}

	ConnectNode node(std::string(argv[1]), TOPOLOGY_FILE);
	node.start();

	cout << "Press ENTER to close application when ready..." << endl;
	cin.ignore();

	node.shutdown();

	return 0;
}
