/*
 * timers.cpp
 *
 *  Created on: Nov 22, 2013
 *      Author: zbierak
 *
 * The demo for sample fiddling with timers. It randomly creates an array
 * and sorts it using timeouts (i.e. sleeps for the amount of ms contained
 * in every element of the array). Upon timeout it sends the results to its
 * neighbors.
 *
 */

#include <iostream>
#include <time.h>

#include <bento/node.h>
#include <bento/exceptions.h>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

const unsigned ARRAY_SIZE = 10;

class TimerNode: public bento::Node
{
private:
	int respReceived;
public:
	TimerNode(const std::string& name, const std::string& topologyFileName): Node(bento::Topology(name, topologyFileName))
	{
		cout << "Node role: " << this->getTopology().getOwnerRole() << endl;
		respReceived = 0;
		srand(time(NULL));
	}

    virtual void onConnect()
    {
    	cout << "Node has connected to all its neighbors, setting some timers." << endl;

    	for (unsigned i=0; i<ARRAY_SIZE; i++)
    	{
    		unsigned timeout = rand() % 10000;
    		this->setTimeout(timeout, boost::bind(&TimerNode::onTimeout, this, timeout));
    	}

    	// test canceling the timeout
    	int timeoutId = this->setTimeout(10000, boost::bind(&TimerNode::onCanceledTimeout, this));
    	bool cancelOk = this->cancelTimeout(timeoutId);
    	assert(cancelOk);
    }

    void onTimeout(unsigned timeoutValue)
    {
    	pass(boost::lexical_cast<string>(timeoutValue));
    }

    void onCanceledTimeout()
    {
    	assert("This function should never be called, due to a canceled timeout!" && false);
    }

    void onMessage(const std::string& from, const int32_t type, const std::string& msg)
    {
    	cout << "[" << ++respReceived << "/" << ARRAY_SIZE << "] " << msg << endl;

    	if (respReceived == ARRAY_SIZE)
    		respReceived = 0;
    }
};

#include <bento/timers/timer.h>

int main(int argc, char *argv[])
{
	// relative path, assuming the software is build in a subdirectory
	const string TOPOLOGY_FILE = "../timers.top";

	if (argc < 2)
	{
		cout << "Missing arguments. The application should be called:" << endl;
		cout << "\ttimers <node-name>" << endl;
		return -1;
	}

	TimerNode node(std::string(argv[1]), TOPOLOGY_FILE);
	node.start();

	node.connectTopology();

	cout << "Press ENTER to close application when ready..." << endl;
	cin.ignore();

	node.stop();

	return 0;
}
