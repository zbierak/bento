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
 * Additionally, we use this example to show how message signing works.
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

/*
 * Simple message "signer" (just to illustrate the general concept).
 * Every message sent will be passed to signMessage. Every received
 * message will be passed to verifyMessage. If the latter returns
 * false, such message is regarded as invalid and discarded.
 */
class MessageSigner: public bento::IMessageSigner
{
	virtual void signMessage(const bento::Topology* topology, const std::string& to, const int32_t type, const std::string& msg, std::string& signature)
	{
		signature = msg;
	}

	virtual bool verifyMessage(const bento::Topology* topology, const std::string& from, const int32_t type, const std::string& msg, const std::string& signature)
	{
		return signature == msg;
	}
};

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

	MessageSigner signer;

	TimerNode node(std::string(argv[1]), TOPOLOGY_FILE);
	node.setMessageSigner(&signer);
	node.start();

	cout << "Press ENTER to close application when ready..." << endl;
	cin.ignore();

	node.shutdown();

	return 0;
}
