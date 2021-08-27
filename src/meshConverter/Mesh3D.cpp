
#include "Mesh3D.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

Mesh3D::Mesh3D(void)
{
	initialized = false;
}

Mesh3D::~Mesh3D(void)
{

}

void Mesh3D::readOBJ(const char* filename) {


	ifstream stream;
	stream.open(filename);

	string line;

	while (getline(stream, line)) {
		vector<string> c = splitString(line, ' ');

		if (c.size() == 0) continue;

		if (c[0].compare("v") == 0) {
			processPosition(c);
		}
		else if (c[0].compare("vt") == 0) {
			processTexCoord(c);
		}
		else if (c[0].compare("vn") == 0) {
			processNormal(c);
		}
		else if (c[0].compare("f") == 0) {
			processFace(c);
		}
	}

	calculateAllFaceNormals();

	cout << "Found " << positions.size() << " vertex positions" << endl;
	cout << "Found " << texCoords.size() << " texture coordinates" << endl;
	cout << "Found " << normals.size() << " normal vectors" << endl;
	cout << "Created " << vertices.size() << " unique vertices " << endl;
	cout << "Created " << faces.size() << " triangular faces" << endl;

	positions.clear();
	texCoords.clear();
	normals.clear();

}

vector<string> Mesh3D::splitString(string& str, char delimiter)
{
	vector<string> words;
	string word;
	stringstream stream(str);

	while (getline(stream, word, delimiter))
		words.push_back(word);

	return words;
}

void Mesh3D::processPosition(vector<string>& c) {
	//cout << "Position "<< positions.size() << ": " << c[1] << "," << c[2] << "," << c[3] << "                        \r";	
	positions.push_back(Position(
		(float)atof(c[1].c_str()),
		(float)atof(c[2].c_str()),
		(float)atof(c[3].c_str()))
	);
}

void Mesh3D::processTexCoord(vector<string>& c) {
	//cout << "TexCoord "<< texCoords.size() << ": " << c[1] << "," << c[2] << "                        \r";	
	texCoords.push_back(TexCoord(
		(float)atof(c[1].c_str()),
		(float)atof(c[2].c_str()))
	);
}

void Mesh3D::processNormal(vector<string>& c) {
	//cout << "Normal " << normals.size() << ": " << c[1] << "," << c[2] << "," << c[3] << "                        \r";	
	normals.push_back(Normal(
		(float)atof(c[1].c_str()),
		(float)atof(c[2].c_str()),
		(float)atof(c[3].c_str()))
	);
}

void Mesh3D::processFace(vector<string>& c) {

	int numVertices = c.size() - 1;
	vector<unsigned short> indices;

	for (int i = 0; i < numVertices; ++i) {
		indices.push_back(indexOfVertex(c[i + 1]));
	}

	unsigned short index0 = indices[0];
	unsigned short index1 = indices[1];

	for (int i = 2; i < numVertices; ++i) {
		unsigned short index2 = indices[i];
		faces.push_back(Triangle(index0, index1, index2));
		index1 = index2;
	}

}

unsigned short Mesh3D::indexOfVertex(string vs) {

	map<string, unsigned short> ::iterator it = vertexHashMap.find(vs);

	unsigned short vertexIndex;

	// check if the vertex already exists
	if (it == vertexHashMap.end()) { // it's a new vertex

		vector<string> indices = splitString(vs, '/');
		int positionIndex = atoi(indices[0].c_str()) - 1;
		int texCoordIndex = atoi(indices[1].c_str()) - 1;
		int normalIndex = atoi(indices[2].c_str()) - 1;

		Vertex newVertex(positions[positionIndex],
			texCoords[texCoordIndex],
			normals[normalIndex]);

		vertexIndex = vertices.size();
		vertices.push_back(newVertex);
		vertexHashMap[vs] = vertexIndex;


	}
	else {
		vertexIndex = vertexHashMap[vs];
	}

	return vertexIndex;
}

void Mesh3D::collapseEdges(int howMany) {

	for (int n = 0; n < howMany; ++n)
	{
		auto target = calculateLowestCostPair();
		auto eraseIndex = target.first;
		auto collapseToIndex = target.second;
		std::cout << "CollapseToIndex: " <<  collapseToIndex << std::endl;

		//find triangles with both vertices of the edge and remove them
		//need an iterator first
		auto it = faces.begin();
		std::vector<unsigned short> indicesOfTrianglesContainingEdge;
		while (it != faces.end()) {
			for (int i = 0; i < 3; i++) {
				//unnecessary checking of some edges here but whatevs
				if ((it->i[i] == eraseIndex) && (it->i[(i + 1) % 3] == collapseToIndex || it->i[(i + 2) % 3] == collapseToIndex)) {
					indicesOfTrianglesContainingEdge.push_back(it - faces.begin());
					break;
				}
			}
			it++;
		}

		for (int i = 0; i < indicesOfTrianglesContainingEdge.size(); i++) {
			std::cout << "erasing " << indicesOfTrianglesContainingEdge[i] << std::endl;
			faces.erase(faces.begin() + indicesOfTrianglesContainingEdge[i]);
		}

		//update triangles with v1 of the edge to use v2 instead
		auto it2 = faces.begin();
		while (it2 != faces.end()) {
			for (int i = 0; i < 3; i++) {
				if (it2->i[i] == eraseIndex) {
					std::cout << "replacing vertex in " << it2 - faces.begin() << std::endl;
					it2->i[i] = collapseToIndex;
					//have to recalculate the face normals of these faces, since they have been changed
					//this assumes that there's no malformed face where the same vertex is in twice......
					// maybe rewrite so that assumption isn't true later
					calculateFaceNormal(*it);
					break;
				}
			}
			it2++;
		}

		//remove v1
		vertices.erase(vertices.begin() + eraseIndex);



		//updated indices
		for (size_t i = 0; i < faces.size(); i++)
		{
			for (size_t j = 0; j < 3; j++)
			{
				if (faces[i].i[j] > eraseIndex)
					faces[i].i[j]--;
			}
		}

	}
		initialized = false;

}


double Mesh3D::calculateCollapseCost(unsigned short p1, unsigned short p2) {

	auto p1Vec = CVector(vertices[p1].px, vertices[p1].py, vertices[p1].pz);
	auto p2Vec = CVector(vertices[p2].px, vertices[p2].py, vertices[p2].pz);

	auto length = (p2Vec - p1Vec).getLength();
	auto curvature = 0.0;
	auto startingCurvature = 1.0;

	//get all triangles containing p1
	std::vector<unsigned short> p1Triangles;
	auto it1 = faces.begin();
	while (it1 != faces.end()) {
		for (int i = 0; i < 3; i++) {
			if (it1->i[i] == p1) {
				p1Triangles.push_back(it1 - faces.begin());
				break;
			}
		}
		it1++;
	}

	//get all triangles containing p1 and p2
	std::vector<unsigned short> p1p2Triangles;
	auto it2 = faces.begin();
	while (it2 != faces.end()) {
		for (int i = 0; i < 3; i++) {
			//unnecessary checking of some edges here but whatevs
			if ((it2->i[i] == p1) && (it2->i[(i + 1) % 3] == p2 || it2->i[(i + 2) % 3] == p2)) {
				p1p2Triangles.push_back(it2 - faces.begin());
				break;
			}
		}
		it2++;
	}

	for (int i = 0; i < p1Triangles.size(); ++i) {
		auto normal1 = faces[p1Triangles[i]].faceNormal;

		for (int j = 0; j < p1p2Triangles.size(); ++j) {
			auto normal2 = faces[p1p2Triangles[j]].faceNormal;

			auto dotproduct = normal1 * normal2;

			startingCurvature = std::min(startingCurvature, (1.0f - dotproduct) / 2.0f);
		}

		curvature = std::max(curvature, startingCurvature);
	}

	auto cost = length * curvature;

	//std::cout << "cost: " << cost << std::endl;
	return cost;

}

std::pair<unsigned short, unsigned short> Mesh3D::calculateLowestCostPair() {
	std::pair <unsigned short, unsigned short> result;
	double collapseCost = DBL_MAX;

	auto it = faces.begin();
	while (it != faces.end()) {
		for (int i = 0; i < 3; ++i) {
			auto cost = calculateCollapseCost(it->i[i], it->i[(i + 1) % 3]);
			if (cost < collapseCost) {
				collapseCost = cost;
				result.first = it->i[i];
				result.second = it->i[(i + 1) % 3];
			}
			cost = calculateCollapseCost(it->i[i], it->i[(i + 2) % 3]);
			if (cost < collapseCost) {
				collapseCost = cost;
				result.first = it->i[i];
				result.second = it->i[(i + 2) % 3];
			}
		}
		it++;
	}
	std::cout << "Lowest Cost: " << collapseCost << std::endl;

	return result;
}


void Mesh3D::calculateFaceNormal(Triangle face) {
	auto p1 = CVector(vertices[face.i[0]].px, vertices[face.i[0]].py, vertices[face.i[0]].pz);
	auto p2 = CVector(vertices[face.i[1]].px, vertices[face.i[1]].py, vertices[face.i[1]].pz);
	auto p3 = CVector(vertices[face.i[2]].px, vertices[face.i[2]].py, vertices[face.i[2]].pz);
	auto normal = crossProduct(p2 - p1, p2 - p3);
	normal.normalize();

	face.faceNormal = normal;
}

void Mesh3D::calculateAllFaceNormals() {
	for (int i = 0; i < faces.size(); ++i) {
		calculateFaceNormal(faces[i]);
	}
}
