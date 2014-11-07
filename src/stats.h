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

	/**
	 * Assign current timestamp as a start of event eventId
	 */
	void eventStart(int64_t eventId);

	/**
	 * Assign current timestamp as a stop of event eventId
	 */
	void eventStop(int64_t eventId);

	/**
	 * Create a new event with an already known duration
	 */
	void createEvent(double duration);

	/**
	 * Start new experiment and be ready to record incoming events
	 */
	void experimentBegin(const std::string& experimentName = "");

	/**
	 * End an experiment and return its statistics
	 */
	void experimentEnd(StatsResult& result);
protected:
	typedef std::map<int64_t, double> SortedMap;
	typedef boost::unordered_map<int64_t, double> FastAccessMap;
	SortedMap m_eventStart;
	FastAccessMap m_eventStop;

	// events duration added with createEvent()
	std::vector<double> m_additionalEvents;

	std::string m_experimentName;
	bool m_duringExperiment;
	double m_experimentStart;

	// record event ???
	// event start / event stop
};

} /* namespace bento */

#endif /* STATS_NODE_H_ */
