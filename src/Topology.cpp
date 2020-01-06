#include "Topology.h"
#include "Utility.h"
//-----------------------------------------------------------------------------------------------------------------------

NetworkTopology::NetworkTopology(Society* s) {
	nodes.reserve(s->people.size());
	edges.reserve(s->links.size());

	for (t_int i = 0; i < s->people.size(); ++i) nodes.push_back(NetworkNode(s->people[i].x, s->people[i].y));
	for (LinkIterator l = s->links.begin(); l != s->links.end(); ++l)
		edges.push_back(NetworkEdge(l->second.source + 1, l->second.target + 1, l->second.listenChance + 1, l->second.trust.expectation()));
}

//-----------------------------------------------------------------------------------------------------------------------

string NetworkTopology::description(float minListenChance, t_int weights) {
	
	// get boundary
	t_float xMin = INFINITY, yMin = INFINITY, xMax = -INFINITY, yMax = -INFINITY;
	for (t_int i = 0; i < nodes.size(); ++i) {
		if (nodes[i].x < xMin) xMin = nodes[i].x;
		if (nodes[i].y < yMin) yMin = nodes[i].y;
		if (nodes[i].x > xMax) xMax = nodes[i].x;
		if (nodes[i].y > yMax) yMax = nodes[i].y;


	}
	t_float dx = (xMax - xMin) / .9, dy = (yMax - yMin) / .9;

	// vertices
	string s = string("*Vertices ") + string(IntToString(nodes.size())) + string("\r\n");
	for (t_int i = 0; i < nodes.size(); ++i) {
		s += string(IntToString(i + 1)) + " \"" + IntToString(i + 1) + "\" " + DoubleToString((nodes[i].x - xMin) / dx + .05, 3);
		s += string(" ") + DoubleToString((nodes[i].y - yMin) / dy + .05, 3) + "\r\n";
	}

	// edges
	s += string("*Arcs ") + string(IntToString(edges.size())) + string("\r\n");
	for (t_int i = 0; i < edges.size(); ++i) {
		if (edges[i].listenChance > minListenChance) {
			s += string(IntToString(edges[i].src)) + string(" ") + string(IntToString(edges[i].dst)) + string(" ");
			if (weights == WTS_NONE) s += string("1.0") + string("\r\n");
			else if (weights == WTS_LISTEN_CHANCE) s += string(DoubleToString(edges[i].listenChance, 3)) + string("\r\n");
			else if (weights == WTS_TRUST) s += string(DoubleToString(edges[i].expTrust, 3)) + string("\r\n");
		}
	}
	
	return s;
}