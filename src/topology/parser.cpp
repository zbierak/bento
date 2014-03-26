/*
 * parser.cpp
 *
 *  Created on: Nov 13, 2013
 *      Author: zbierak
 */

#include "parser.h"

#include <fstream>
#include <vector>

//#define TOPOLOGYPARSER_DEBUG

#ifdef TOPOLOGYPARSER_DEBUG
#define BOOST_SPIRIT_DEBUG
#endif

#include <boost/spirit/include/classic.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>
#include <boost/bind.hpp>

using namespace std;
using namespace BOOST_SPIRIT_CLASSIC_NS;

namespace bento {

void commentsErase(std::string& text, const string& start, const string& stop)
{
	unsigned i = 0;
	while (i < text.length())
	{
		unsigned posStart = text.find(start,i);
		if (posStart == string::npos)
			return;
		else
		{
			unsigned posEnd = text.find(stop, posStart + start.size());
			if (posEnd == string::npos || posEnd + stop.size() >= text.length())
			{
				text = text.substr(0, posStart);
			}
			else
			{
				text = text.substr(0, posStart).append(text.substr(posEnd + stop.size()));
				i = posStart;
			}
		}
	}
}

struct TopologyGrammar : public grammar<TopologyGrammar>
{
public:
	TopologyGrammar()
	{
		m_this = this;
		m_error = false;
	}

    template <typename ScannerT>
    struct definition
    {
        rule<ScannerT> TopologyDocument, nodes, nodeElem, allToAll, a2aElem, connect, str;

		definition(TopologyGrammar const& self)
		{
			TopologyDocument = nodes >> *(allToAll | connect);
			nodes = as_lower_d["nodes"] >> ch_p('{') >> !(nodeElem >> *(ch_p(',') >> nodeElem)) >> ch_p('}');
			nodeElem = str[boost::bind(&TopologyGrammar::storeNodeName, self.m_this, _1, _2)] >> ch_p('(') >>
					   str[boost::bind(&TopologyGrammar::storeDefaultAddr, self.m_this, _1, _2)] >> ch_p(')') >>
					   !(as_lower_d["is"] >> str[boost::bind(&TopologyGrammar::storeRole, self.m_this, _1, _2)]);
			allToAll = as_lower_d["a2a"] >> ch_p('{') >> !(a2aElem >> *(ch_p(',') >> a2aElem))
						>> ch_p('}')[boost::bind(&TopologyGrammar::processA2A, self.m_this)];
			a2aElem = str[boost::bind(&TopologyGrammar::storeA2A, self.m_this, _1, _2)];
			connect = as_lower_d["connect"]
			                     >> str[boost::bind(&TopologyGrammar::storeConnectWhat, self.m_this, _1, _2)]
			                     >> as_lower_d["to"]
			                     >> str[boost::bind(&TopologyGrammar::storeConnectToWhat, self.m_this, _1, _2)]
			                     >> !(as_lower_d["on"]
			                     >> str[boost::bind(&TopologyGrammar::storeConnectOnAddr, self.m_this, _1, _2)])
			                     >> ch_p(';')[boost::bind(&TopologyGrammar::processConnect, self.m_this)];
			str = lexeme_d[ alnum_p >> *(alnum_p | ch_p('.') | ch_p('-') | ch_p('_') | ch_p(':')) ];

			BOOST_SPIRIT_DEBUG_RULE(TopologyDocument);
			BOOST_SPIRIT_DEBUG_RULE(nodes);
			BOOST_SPIRIT_DEBUG_RULE(nodeElem);
			BOOST_SPIRIT_DEBUG_RULE(allToAll);
			BOOST_SPIRIT_DEBUG_RULE(a2aElem);
			BOOST_SPIRIT_DEBUG_RULE(connect);
			BOOST_SPIRIT_DEBUG_RULE(str);
		}

        rule<ScannerT> const& start() const { return TopologyDocument; }
    };

    inline bool hasError() { return m_error; }
    inline std::string getErrorMsg() { return m_errorMsg; }

    inline const Topology::TopologyMap& getTopologyMap() const { return m_topology; }
    inline const Topology::NodeList& getNodeList() const { return m_nodes; }
    inline const Topology::AddressList& getDefaultAddresses() const { return m_defaultAddress; }
    inline const Topology::RoleList& getRoles() const { return m_roles; }
private:
    TopologyGrammar* m_this;

    Topology::NodeList m_nodes;
    Topology::AddressList m_defaultAddress;
    Topology::RoleList m_roles;
    vector<string> m_awaitingA2A;
    string m_connectWhat;
    string m_connectToWhat;
    string m_connectOnAddress;

    Topology::TopologyMap m_topology;

    bool m_error;
    string m_errorMsg;

    // nodes section
    void storeNodeName(char const* str, char const* end)
    {
    	if (m_error) return;

    	string node(str, end);
    	m_nodes.push_back(node);
    	m_topology.insert(make_pair(node, Topology::AddressList()));
    }

    void storeDefaultAddr(char const* str, char const* end)
    {
    	if (m_error) return;

    	string addr(str, end);
    	m_defaultAddress.insert(make_pair(m_nodes[m_nodes.size()-1], addr));
    }

    void storeRole(char const* str, char const* end)
        {
        	if (m_error) return;

        	string role(str, end);
        	m_roles.insert(make_pair(m_nodes[m_nodes.size()-1], role));
        }

    // a2a section
    void storeA2A(char const* str, char const* end)
    {
    	if (m_error) return;

    	string node(str, end);
    	if (std::find(m_nodes.begin(), m_nodes.end(), node) == m_nodes.end())
    	{
    		m_error = true;
    		m_errorMsg = "Undefined node name '" + node + "' inside the A2A clause. You need to define this name first inside the nodes section.";
    		return;
    	}

    	m_awaitingA2A.push_back(node);
    }

    void processA2A()
    {
    	if (m_error) return;

    	for (unsigned i=0; i<m_awaitingA2A.size(); i++)
    	{
    		Topology::TopologyMap::iterator targetIt = m_topology.find(m_awaitingA2A[i]);
  			assert(targetIt != m_topology.end());

    		for (unsigned j=0; j<m_awaitingA2A.size(); j++)
    		{
    			if (i == j)
    				continue;

        		boost::unordered_map<string, string>::iterator defaultIt = m_defaultAddress.find(m_awaitingA2A[j]);
        		assert(defaultIt != m_defaultAddress.end());

        		// does not override previous address (by another a2a or connect)
        		targetIt->second.insert(make_pair(defaultIt->first, defaultIt->second));
    		}
    	}

    	m_awaitingA2A.clear();
    }

    // connect section
    void storeConnectWhat(char const* str, char const* end)
    {
      	if (m_error) return;
       	m_connectWhat = string(str, end);

       	if (std::find(m_nodes.begin(), m_nodes.end(), m_connectWhat) == m_nodes.end())
       	{
       		m_error = true;
       	    m_errorMsg = "Undefined node name '" + m_connectWhat + "' inside the CONNECT clause. You need to define this name first inside the nodes section.";
       	    return;
       	}
    }

    void storeConnectToWhat(char const* str, char const* end)
    {
      	if (m_error) return;
      	m_connectToWhat = string(str, end);

      	if (std::find(m_nodes.begin(), m_nodes.end(), m_connectToWhat) == m_nodes.end())
      	{
      		m_error = true;
      	    m_errorMsg = "Undefined node name '" + m_connectToWhat + "' inside the CONNECT clause. You need to define this name first inside the nodes section.";
      	    return;
      	}

      	boost::unordered_map<string, string>::const_iterator it = m_defaultAddress.find(m_connectToWhat);
      	if (it == m_defaultAddress.end())
      	{
      		m_error = true;
      		m_errorMsg = "Default address not specified for node '" + m_connectToWhat + "'. You need to define this name properly inside the nodes section.";
      		return;
      	}

      	m_connectOnAddress = it->second;
    }

    void storeConnectOnAddr(char const* str, char const* end)
    {
    	if (m_error) return;
        m_connectOnAddress = string(str, end);
    }

    void processConnect()
    {
    	if (m_error) return;

    	Topology::TopologyMap::iterator targetIt = m_topology.find(m_connectWhat);
    	assert(targetIt != m_topology.end());

    	// overrides previous address, if any (by a2a or another connect)
    	targetIt->second[m_connectToWhat] = m_connectOnAddress;
    }
public:
    void debugPrint()
    {
        cout << "Parsed topology:" << endl;
        for (Topology::TopologyMap::const_iterator it = m_topology.begin(); it != m_topology.end(); ++it)
        {
        	cout << "\t" << "Neighbours of " << it->first << endl;
        	for (Topology::AddressList::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        	{
        		cout << "\t\t" << it2->first << " with address " << it2->second << endl;
        	}
        }
    }
};

bool parseTopologyFile(const std::string& file, Topology& result, std::string& error)
{
	ifstream input;
	input.open(file.c_str());
	if (!input.is_open()) {
		error = "Unable to open file " + file;
		return false;
	}

	string target = "";
	TopologyGrammar gram;

	while (input.good() && !input.eof())
	{
		char a = input.get();
		if (a == '\r')	a = ' ';
		if (a != EOF) target += a;
	}

	input.close();

	commentsErase(target, "/*", "*/");
	commentsErase(target, "//", "\n");

	boost::replace_all(target, "\n", " ");

	parse_info<> info = parse(target.c_str(), gram, space_p);

	if (gram.hasError())
	{
		error = gram.getErrorMsg();
		return false;
	}

	bool success = true;
	if (!info.full) {
		string remaining = info.stop;
		boost::algorithm::trim(remaining);
		if (!remaining.empty()) {
			success = false;
		}
	}

	if (success)
	{
		result.updateTopologyMap(gram.getNodeList(), gram.getTopologyMap(), gram.getDefaultAddresses(), gram.getRoles());
		return true;
	}
    else
    {
    	error = "Topology parse error on: \n\t" + std::string(info.stop).substr(0, 255);
		return false;
    }
}

} /* namespace bento */
