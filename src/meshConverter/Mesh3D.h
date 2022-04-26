#pragma once
#include "VertexStructs.h"
#include "../mathutil/CVector.h"
#include <vector>
#include <map>
#include <string>

class Mesh3D
{
public:
	Mesh3D(void);
	virtual ~Mesh3D(void);

	void readOBJ(const char *filename);
	void writeOBJ();
	void draw(int mode = 0);

	int minimumCostVertex();
	void removeTriangles(int howMany);
	void collapseEdge(unsigned short p1, unsigned short p2);
	void checkAndRemoveFromNeighbors(unsigned short source, unsigned short toRemove);
	void replaceVertexInFace(unsigned short faceIndex, unsigned short toReplaceIndex, unsigned short replaceWithIndex);
	double calculateCollapseCost(unsigned short p1, unsigned short p2);
	void calculateCostAtVertex(unsigned short vertex);
	void calculateAllCosts();
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
	void generateInterconnections(unsigned short triIndex, std::vector<unsigned short> vertIndices);
	bool checkIfVertexInFace(unsigned short vertIndex, unsigned short faceIndex);
	
	std::vector<std::string> splitString(std::string &str, char delimiter);
	unsigned short indexOfVertex(std::string vs);

	void createVertexBuffer();
	void createIndexBuffer();
	bool initialized;
	unsigned int vboID;
	unsigned int iboID;
	int numPrimitives;

	std::string fileaddress;
};

