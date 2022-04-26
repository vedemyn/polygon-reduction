#pragma once
#include "../mathutil/CVector.h"
#include <vector>

struct Position {
	Position(float x, float y, float z) { px = x; py = y; pz = z; }
	float px;
	float py; 
	float pz;
};

struct Normal {
	Normal(float x, float y, float z) { nx = x; ny = y; nz = z; }
	float nx;
	float ny; 
	float nz;
};

struct TexCoord {
	TexCoord(float x, float y) { u = x; v = y;}
	float u;
	float v; 
};

struct Vertex {

	Vertex(Position p) {
		px = p.px;	py = p.py;	pz = p.pz;
	}
	Vertex(Position p, TexCoord t) {
		px = p.px;	py = p.py;	pz = p.pz;
		u = t.u; v = t.v;
	}
	Vertex(Position p, TexCoord t, Normal n) {
		px = p.px;	py = p.py;	pz = p.pz;
		nx = n.nx;	ny = n.ny;	nz = n.nz;
		u = t.u; v = t.v;
	}
	float px;
	float py; 
	float pz;
	float u = 0.0;
	float v = 0.0; 
	float nx = 0.0;
	float ny = 0.0; 
	float nz = 0.0;
	std::vector<unsigned short> faces;
	std::vector<unsigned short> neighbors;
	int collapseTarget; // int because i need a negative number to somehow save that there's no valid target... maybe a better solution somewhere
	double collapseCost;
};

struct Triangle {
	Triangle(unsigned short a, unsigned short b, unsigned short c) { 
		i[0] = a; 
		i[1] = b; 
		i[2] = c;
	}
	unsigned short i[3];
	CVector faceNormal;
	
};

