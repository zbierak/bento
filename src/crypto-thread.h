/*
 * CryptoThread.h
 *
 *  Created on: Jun 4, 2014
 *      Author: zbierak
 */

#ifndef CRYPTOTHREAD_H_
#define CRYPTOTHREAD_H_

#include <boost/thread.hpp>

#include "zmq-wrappers/zmq-context.h"
#include "zmq-wrappers/zmq-inproc.h"

#include "message-signer.h"
#include "topology/topology.h"

namespace bento {

enum CRYPTO_MSG_TYPE { CMT_SIGN_REQUEST, CMT_SIGN_RESPONSE, CMT_VERIFY_REQUEST, CMT_VERIFY_RESPONSE, CMT_END_OF_THE_WORLD };

class CryptoThread {
public:
	CryptoThread(const Topology& topology);
	virtual ~CryptoThread();

	void setMessageSigner(IMessageSigner* signer);

	void start();
	void stop();                                                                    // can be called by the node thread only

	inline zmq::socket_t* getNodeSocket() { return m_chanAtNode; }      // get the communication socket from the end of the node thread

	void requestSign(const std::string& target, const int32_t type, const std::string& msg);                                  // can be called by the node thread only
	void requestVerify(const std::string& from, const int32_t type, const std::string& msg, const std::string& signature);    // can be called by the node thread only
private:
	void run();

	void signMessage();
	void verifyMessage();

	bool m_running;
	boost::thread* m_thread;

	IMessageSigner* m_signer;

	const Topology& m_topology;

	zmq::socket_t* m_chanAtCrypto;          // channel at the crypto thread
	zmq::socket_t* m_chanAtNode;            // channel at the node thread
};

} /* namespace bento */

#endif /* CRYPTOTHREAD_H_ */
