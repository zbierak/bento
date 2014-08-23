/*
 * main-thread.cpp
 *
 *  Created on: Aug 23, 2014
 *      Author: zbierak
 *
 * This demo shows how to run bento in the main thread of the application. One node
 * sends some messages and quits, while the other quits after receiving those messages.
 *
 * Also, illustrates the usage of getOwnerOrderNumber() to select a leader from connected nodes.
 */

#include <iostream>

#include <bento/node.h>
#include <bento/exceptions.h>

#include <boost/lexical_cast.hpp>

using namespace std;

class MainThreadNode: public bento::Node
{
public:
	MainThreadNode(const std::string& name, const std::string& topologyFileName): Node(bento::Topology(name, topologyFileName))
	{
	}

    virtual void onConnect()
    {
    	// only the first node (leader) sends the hellp message
    	if (this->getTopology().getOwnerOrderNumber() == 0)
    	{
    		cout << "I have been selected as a leader, send hello message and quit." << endl;
    		pass("HELLO!");
    		shutdown();
    	}
    }

    void onMessage(const std::string& from, const int32_t type, const std::string& msg)
    {
    	cout << "Received message " << msg << " from " << from << ". Quitting." << endl;
    	shutdown();
    }
};

int main(int argc, char *argv[])
{
	// relative path, assuming the software is build in a subdirectory
	const string TOPOLOGY_FILE = "../main-thread.top";

	if (argc < 2)
	{
		cout << "Missing arguments. The application should be called:" << endl;
		cout << "\tmain-thread <node-name>" << endl;
		return -1;
	}

	MainThreadNode node(std::string(argv[1]), TOPOLOGY_FILE);
	node.run();

	return 0;
}
