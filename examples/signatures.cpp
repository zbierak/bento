/*
 * signatures.cpp
 *
 *  Created on: Aug 9, 2014
 *      Author: zbierak
 *
 * The demo for advanced messages signing and verification. It shows
 * signing and signature verification in multiple threads.
 *
 */

#include <iostream>
#include <vector>
#include <time.h>

#include <bento/node.h>
#include <bento/exceptions.h>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
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
	virtual void signMessage(const bento::Topology& topology, const std::string& to, const int32_t type, const std::string& msg, std::string& signature)
	{
		cout << "Signature of message " << msg << " is generated by signer " << m_signerId << endl;
		boost::this_thread::sleep(boost::posix_time::milliseconds(boost::lexical_cast<unsigned>(msg)));
		signature = msg;
	}

	virtual bool verifyMessage(const bento::Topology& topology, const std::string& from, const int32_t type, const std::string& msg, const std::string& signature)
	{
		cout << "Verification of message signature " << msg << " is performed by signer " << m_signerId << endl;
		boost::this_thread::sleep( boost::posix_time::milliseconds(2 * boost::lexical_cast<unsigned>(msg)));
		return signature == msg;
	}

	uint32_t m_signerId;

public:
	MessageSigner(uint32_t signerId): m_signerId(signerId) {}
};

class SignatureNode: public bento::Node
{
private:
	int respReceived;
public:
	SignatureNode(const std::string& name, const std::string& topologyFileName): Node(bento::Topology(name, topologyFileName))
	{
		cout << "Node role: " << this->getTopology().getOwnerRole() << endl;
		respReceived = 0;
		srand(time(NULL));
	}

    virtual void onConnect()
    {
    	cout << "Node has connected to all its neighbors, sending some messages." << endl;

    	for (unsigned i=0; i<1000; i++)
    		this->pass(boost::lexical_cast<string>(i));
    }

    void onMessage(const std::string& from, const int32_t type, const std::string& msg)
    {

    }
};

#include <bento/timers/timer.h>

const unsigned SIGNING_THREADS_COUNT = 10;

int main(int argc, char *argv[])
{
	// relative path, assuming the software is build in a subdirectory
	const string TOPOLOGY_FILE = "../signatures.top";

	if (argc < 2)
	{
		cout << "Missing arguments. The application should be called:" << endl;
		cout << "\tsignatures <node-name>" << endl;
		return -1;
	}

	vector<bento::IMessageSigner*> signers;
	for (unsigned i=0; i<SIGNING_THREADS_COUNT; i++)
		signers.push_back(new MessageSigner(i+1));

	SignatureNode node(std::string(argv[1]), TOPOLOGY_FILE);
	node.setMessageSigners(signers);
	node.start();

	cout << "Press ENTER to close application when ready..." << endl;
	cin.ignore();

	node.shutdown();

	for (unsigned i=0; i<signers.size(); i++)
		delete signers[i];

	return 0;
}
