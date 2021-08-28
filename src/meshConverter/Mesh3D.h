#pragma once
#include "VertexStructs.h"
#include <vector>
#include <map>
#include <string>
#include "mathutil/CVector.h"

class Mesh3D
{
public:
	Mesh3D(void);
	virtual ~Mesh3D(void);

	void readOBJ(const char *filename);
	void draw(int mode = 0);

	unsigned short minimumCostVertex();
	void collapseEdges(int howMany);
	void collapseEdge(unsigned short p1, unsigned short p2);
	void RemoveIfNonNeighbor(unsigned short source, unsigned short toRemove);
	void replaceVertexInFace(unsigned short faceIndex, unsigned short toReplaceIndex, unsigned short replaceWithIndex);
	double calculateCollapseCost(unsigned short p1, unsigned short p2);
	void calculateCostAtVertex(unsigned short vertex);
	void calculateAllCosts();
	std::pair<unsigned short, unsigned short> calculateLowestCostPair();
	void calculateFaceNormal(Triangle& face);


protected:
	std::vector<Position> positions;
	std::vector<Normal> normals;
	std::vector<TexCoord> texCoords;
	std::vector<Vertex> vertices;
	std::vector<Triangle> faces;
	
	std::map<std::string, unsigned short> vertexHashMap;

protected:
	
	void processPosition(std::vector<std::string> &c);
	void processTexCoord(std::vector<std::string> &c);
	void processNormal(std::vector<std::string> &c);
	void processFace(std::vector<std::string> &c);
	
	std::vector<std::string> splitString(std::string &str, char delimiter);
	unsigned short indexOfVertex(std::string vs);

	void createVertexBuffer();
	void createIndexBuffer();
	bool initialized;
	unsigned int vboID;
	unsigned int iboID;
	int numPrimitives;
};

