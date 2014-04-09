/*
 * stats-node.cpp
 *
 *  Created on: Nov 26, 2013
 *      Author: zbierak
 */

#include "stats.h"
#include "exceptions.h"
#include "timers/timer.h"

#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

namespace bento {

void StatsResult::printSummary()
{
	cout << "Summary of experiment " << experimentName << endl;
	cout << "* Min: " << minValue << endl;
	cout << "* Avg: " << avgValue << endl;
	cout << "* Med: " << medValue << endl;
	cout << "* Max: " << maxValue << endl;
	cout << "Overall experiment duration [s]: " << experimentDuration << endl;
	cout << endl;
}

void StatsResult::printEvents()
{
	if (!experimentName.empty())
		cout << experimentName << " ";

	for (unsigned i=0; i<eventsDuration.size(); i++)
	{
		if (i>0) cout << " ";
		cout << eventsDuration[i];
	}

	cout << endl;
}

Stats::Stats() : m_duringExperiment(false), m_experimentStart(-1) {}

Stats::~Stats() {}

void Stats::eventStart(int64_t eventId)
{
	if (!m_duringExperiment)
		throw GeneralException("Events can be only recorded during experiments (you need to call experimentBegin first)");

	m_eventStart[eventId] = Timer::Now::seconds();
}

void Stats::eventStop(int64_t eventId)
{
	if (!m_duringExperiment)
		throw GeneralException("Events can be only recorded during experiments (you need to call experimentBegin first)");

	m_eventStop[eventId] = Timer::Now::seconds();
}

void Stats::experimentBegin(const std::string& experimentName)
{
	if (m_duringExperiment)
		return;

	m_eventStart.clear();
	m_eventStop.clear();
	m_experimentName = experimentName;
	m_duringExperiment = true;
	m_experimentStart = Timer::Now::seconds();
}

void Stats::experimentEnd(StatsResult& result)
{
	if (!m_duringExperiment)
		return;

	result.experimentName = m_experimentName;
	result.experimentDuration = Timer::Now::seconds() - m_experimentStart;

	vector<double> elapsed;
	for (SortedMap::const_iterator startIt = m_eventStart.begin(); startIt != m_eventStart.end(); ++startIt)
	{
		FastAccessMap::const_iterator stopIt = m_eventStop.find(startIt->first);
		if (stopIt != m_eventStop.end())
		{
			elapsed.push_back(max(stopIt->second - startIt->second, 0.0));
		}
	}

	if (!elapsed.empty())
	{
		result.eventsDuration = elapsed;

		sort(elapsed.begin(), elapsed.end());
		result.minValue = elapsed.front();
		result.maxValue = elapsed.back();

		if (elapsed.size() % 2 == 1)
		{
			result.medValue = elapsed[(elapsed.size()-1)/2];
		}
		else
		{
			result.medValue = (elapsed[elapsed.size()/2] + elapsed[elapsed.size()/2 - 1]) / 2.0;
		}

		double avg = 0;
		for (unsigned i=0; i<elapsed.size(); i++)
			avg += elapsed[i];
		result.avgValue = avg / elapsed.size();
	}

	m_duringExperiment = false;

}

} /* namespace bento */


