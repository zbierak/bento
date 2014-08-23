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

#include <bento/services/topology-status.h>

#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

using namespace std;


int32_t MESSAGE_TYPE_TOPOLOGY_STATUS = -100;

class ConnectNode: public bento::Node
{
public:
	ConnectNode(const std::string& name, const std::string& topologyFileName):
		Node(bento::Topology(name, topologyFileName)),
		m_ts(new bento::TopologyStatus(this, MESSAGE_TYPE_TOPOLOGY_STATUS))
	{
		addMessageIntercepter(m_ts);
	}

	void onTopologyStatus(const bento::Topology::NodeList& found, const bento::Topology::NodeList& notFound)
	{
		cout << "------------------------" << endl;
		cout << "Topology Status summary:" << endl;
		cout << "------------------------" << endl;
		cout << "    nodes found: " << endl;

		for (unsigned i=0; i<found.size(); i++)
			cout << "      " << found[i] << endl;

		cout << "------------------------" << endl;
		cout << "    nodes not found: " << endl;

		for (unsigned i=0; i<notFound.size(); i++)
			cout << "      " << notFound[i] << endl;
		cout << "------------------------" << endl;
	}

    virtual void onConnect()
    {
    	cout << "Node has connected to all its neighbors." << endl;

    	if (this->getTopology().getOwnerOrderNumber() == 0)
    	{
    		cout << "Checking the state of the topology" << endl;
    		m_ts->query(boost::bind(&ConnectNode::onTopologyStatus, this, _1, _2), 1000);
    	}
    }

    void onMessage(const std::string& from, const int32_t type, const std::string& msg)
    {
   		cout << "Node " << getName() << " obtained message " << msg << " from " << from << ". Curiously enough, this should never have happened." << endl;
    }
private:
    bento::TopologyStatusPtr m_ts;
};

int main(int argc, char *argv[])
{
	// relative path, assuming the software is build in a subdirectory
	const string TOPOLOGY_FILE = "../check-topology-status.top";

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

	node.stop();

	return 0;
}
