#ifndef __TOPOLOGY_H__
#define __TOPOLOGY_H__

#include "Society.h"

#define WTS_NONE 0
#define WTS_LISTEN_CHANCE 1
#define WTS_TRUST 2

class NetworkNode {
public:
	NetworkNode(t_float xPos, t_float yPos) { 
		x = xPos; 
		y = yPos; 
	}

	t_float x, y;
};

class NetworkEdge{
public:
	NetworkEdge(t_int from, t_int to, t_float chance, t_float trust) {
		src = from;
		dst = to;
		listenChance = chance;
		expTrust = trust;
	}

	t_int src, dst;
	t_float listenChance, expTrust;
};

class NetworkTopology {
public:
    NetworkTopology() {}
	NetworkTopology(Society* s);
	string description(float minListenChance, t_int weights);

	vector<NetworkNode> nodes;
	vector<NetworkEdge> edges;
};


#endif