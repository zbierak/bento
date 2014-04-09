/*
 * stats-node.h
 *
 *  Created on: Nov 26, 2013
 *      Author: zbierak
 */

#ifndef STATS_NODE_H_
#define STATS_NODE_H_

#include <vector>
#include <map>
#include <boost/unordered_map.hpp>

namespace bento {

struct StatsResult
{
	std::string experimentName;
	double experimentDuration;

	double minValue;
	double avgValue;
	double medValue;
	double maxValue;

	std::vector<double> eventsDuration;

	void printSummary();
	void printEvents();
};

class Stats {
public:
	Stats();
	virtual ~Stats();

	void eventStart(int64_t eventId);
	void eventStop(int64_t eventId);

	void experimentBegin(const std::string& experimentName = "");
	void experimentEnd(StatsResult& result);
protected:
	typedef std::map<int64_t, double> SortedMap;
	typedef boost::unordered_map<int64_t, double> FastAccessMap;
	SortedMap m_eventStart;
	FastAccessMap m_eventStop;

	std::string m_experimentName;
	bool m_duringExperiment;
	double m_experimentStart;

	// record event ???
	// event start / event stop
};

} /* namespace bento */

#endif /* STATS_NODE_H_ */
