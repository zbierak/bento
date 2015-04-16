/*
 * crypto-job.h
 *
 *  Created on: Apr 16, 2015
 *      Author: z
 */

#ifndef CRYPTO_JOB_H_
#define CRYPTO_JOB_H_

#include <string>

namespace bento {

/**
 * This is a structure that holds the request and result for a crypto job.
 *
 * - For a message sign, replica should use initSign() and the crypto thread will fill signature field
 * - For a signature verify, replica should use initVerify() and the crypto thread will fill correct field
 */
struct CryptoJob
{
	enum CryptoJobType { CJ_SIGN, CJ_VERIFY };

	CryptoJobType jobType;
	std::string node;
	int32_t msgType;
	std::string msg;
	std::string signature;
	bool correct;

	inline void initSign(const std::string& node, const int32_t type, const std::string& msg)
	{
		this->jobType = CJ_SIGN;
		this->node = node;
		this->msgType = type;
		this->msg = msg;
	}

	inline void initVerify(const std::string& node, const int32_t type, const std::string& msg, const std::string& signature)
	{
		this->jobType = CJ_VERIFY;
		this->node = node;
		this->msgType = type;
		this->msg = msg;
		this->signature = signature;
	}
};

}

#endif /* CRYPTO_JOB_H_ */
