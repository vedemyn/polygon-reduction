
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

void Mesh3D::readOBJ(const char *filename) {

	
	ifstream stream;
	stream.open(filename);
	
    string line;

	while(getline(stream, line)) {
		vector<string> c = splitString(line,' ');

		if (c.size() == 0) continue;
		
		if(c[0].compare("v") == 0) {
			processPosition(c);
		} else if(c[0].compare("vt") == 0) {
			processTexCoord(c);
		} else if(c[0].compare("vn") == 0) {
			processNormal(c);
		} else if(c[0].compare("f") == 0) {
			processFace(c);
		}
	}
	
	cout << "Found " << positions.size() << " vertex positions" << endl;
	cout << "Found " << texCoords.size() << " texture coordinates" << endl;
	cout << "Found " << normals.size()   << " normal vectors" << endl;
	cout << "Created " << vertices.size()  << " unique vertices " << endl;
	cout << "Created " << faces.size()     << " triangular faces" << endl;
	
	positions.clear();
	texCoords.clear();
	normals.clear(); 

}

vector<string> Mesh3D::splitString(string &str, char delimiter)
{
	vector<string> words;
    string word;
    stringstream stream(str);

    while( getline(stream, word, delimiter) )
        words.push_back(word);

	return words;
}

void Mesh3D::processPosition(vector<string> &c) {
	//cout << "Position "<< positions.size() << ": " << c[1] << "," << c[2] << "," << c[3] << "                        \r";	
	positions.push_back(Position(
		(float)atof(c[1].c_str()),
		(float)atof(c[2].c_str()),
		(float)atof(c[3].c_str()))
	);
}

void Mesh3D::processTexCoord(vector<string> &c) {
	//cout << "TexCoord "<< texCoords.size() << ": " << c[1] << "," << c[2] << "                        \r";	
	texCoords.push_back(TexCoord(
		(float)atof(c[1].c_str()),
		(float)atof(c[2].c_str()))
	);
}

void Mesh3D::processNormal(vector<string> &c) {
	//cout << "Normal " << normals.size() << ": " << c[1] << "," << c[2] << "," << c[3] << "                        \r";	
	normals.push_back(Normal(
		(float)atof(c[1].c_str()),
		(float)atof(c[2].c_str()),
		(float)atof(c[3].c_str()))
	);
}

void Mesh3D::processFace(vector<string> &c) {
	
	int numVertices = c.size()-1;
	vector<unsigned short> indices;

	for(int i = 0; i < numVertices; ++i) {	
		indices.push_back(indexOfVertex(c[i+1]));
	}

	unsigned short index0 = indices[0];
	unsigned short index1 = indices[1];

	for(int i = 2; i < numVertices; ++i) {
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

		vector<string> indices = splitString(vs,'/');
		int positionIndex = atoi(indices[0].c_str())-1;
		int texCoordIndex = atoi(indices[1].c_str())-1;
		int normalIndex   = atoi(indices[2].c_str())-1;

		Vertex newVertex(positions[positionIndex], 
			texCoords[texCoordIndex],
			normals[normalIndex]);

		vertexIndex = vertices.size();
		vertices.push_back(newVertex);
		vertexHashMap[vs] = vertexIndex;


	} else {
		vertexIndex = vertexHashMap[vs];
	} 

	return vertexIndex;
}

