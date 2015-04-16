/*
 * CryptoManager.h
 *
 *  Created on: Jun 4, 2014
 *      Author: zbierak
 */

#ifndef CRYPTOMANAGER_H_
#define CRYPTOMANAGER_H_

#include <vector>

#include "signing-thread.h"
#include "crypto-job.h"
#include "job-queue.h"

namespace bento {

class CryptoManager {
public:
	CryptoManager(const Topology& topology);
	virtual ~CryptoManager();

	void setMessageSigners(const std::vector<IMessageSigner*>& signers);
	std::vector<IMessageSigner*> getMessageSigners();

	void start();
	void stop();                                                                    // can be called by the node thread only

	inline zmq::socket_t* getNodeSocket() { return m_chanAtNode; }      // get the communication socket from the end of the node thread

	void requestSign(const std::string& target, const int32_t type, const std::string& msg);                                  // can be called by the node thread only
	void requestVerify(const std::string& from, const int32_t type, const std::string& msg, const std::string& signature);    // can be called by the node thread only
private:
	std::vector<IMessageSigner*> m_signers;

	const Topology& m_topology;

	zmq::socket_t* m_chanAtNode;            // channel at the node thread

	JobQueue<CryptoJob*>* m_jobQueue;
	SigningThread** m_workers;
};

} /* namespace bento */

#endif /* CRYPTOMANAGER_H_ */
