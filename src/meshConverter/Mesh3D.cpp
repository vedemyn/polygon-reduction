
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

	fileaddress = filename;

	positions.clear();
	texCoords.clear();
	normals.clear();
	vertices.clear();
	faces.clear();

	vertexHashMap.clear();

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

	calculateAllCosts();

	cout << "Found " << positions.size() << " vertex positions" << endl;
	cout << "Found " << texCoords.size() << " texture coordinates" << endl;
	cout << "Found " << normals.size() << " normal vectors" << endl;
	cout << "Created " << vertices.size() << " unique vertices " << endl;
	cout << "Created " << faces.size() << " triangular faces" << endl;

	positions.clear();
	texCoords.clear();
	normals.clear();

	initialized = false;

}

void Mesh3D::writeOBJ() {
	std::cout << fileaddress << std::endl;

	ofstream stream;
	// -4 because the ending should be ".obj" 
	stream.open(fileaddress.insert(fileaddress.size()-4, "_reduced"));


	for (int i = 0; i < vertices.size(); ++i)
	{
		stream << "v " << vertices[i].px << " " << vertices[i].py << " " << vertices[i].pz << "\n";
		//order doesn't really matter so we could just write out these too, if they were saved
		//stream << "vn " << vertices[i].nx << " " << vertices[i].ny << " " << vertices[i].nz << "\n";
		//stream << "vn " << vertices[i].u << " " << vertices[i].u << "\n";
	}

	for (int i = 0; i < faces.size(); ++i)
	{
		stream << "f " << faces[i].i[0] + 1 << " " << faces[i].i[1] + 1 << " " << faces[i].i[2] + 1 << "\n";
		//normals and texture coordinates could be added, but i haven't cared about them so far so it's data loss
		// and i'd have to save the info about those in the first place
	}
	stream.close();
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
		auto tri = (Triangle(index0, index1, index2));
		calculateFaceNormal(tri);
		faces.push_back(tri);

		// save information on which face the indexes belong to
		generateInterconnections(faces.size() - 1, { index0, index1, index2 });

		index1 = index2;
	}

}

void Mesh3D::generateInterconnections(unsigned short triIndex, std::vector<unsigned short> vertIndices) {
	//vertindices is assumed to be size 3
	for (int i = 0; i < 3; i++)
	{
		vertices[vertIndices[i]].faces.push_back(triIndex);
		if (std::count(vertices[vertIndices[i]].neighbors.begin(), vertices[vertIndices[i]].neighbors.end(), vertIndices[(i+1)%3]) == 0)
			vertices[vertIndices[i]].neighbors.push_back(vertIndices[(i+1)%3]);
		if (std::count(vertices[vertIndices[i]].neighbors.begin(), vertices[vertIndices[i]].neighbors.end(), vertIndices[(i+2)%3]) == 0)
			vertices[vertIndices[i]].neighbors.push_back(vertIndices[(i+2)%3]);
	}
}

bool Mesh3D::checkIfVertexInFace(unsigned short vertIndex, unsigned short faceIndex) {
	return  faces[faceIndex].i[0] == vertIndex || faces[faceIndex].i[1] == vertIndex || faces[faceIndex].i[2] == vertIndex;
}

unsigned short Mesh3D::indexOfVertex(string vs) {

	map<string, unsigned short> ::iterator it = vertexHashMap.find(vs);

	unsigned short vertexIndex;

	// check if the vertex already exists
	if (it == vertexHashMap.end()) { // it's a new vertex

		vector<string> indices = splitString(vs, '/');
		int positionIndex = atoi(indices[0].c_str()) - 1;
		int texCoordIndex;
		if (indices.size() > 1) texCoordIndex = atoi(indices[1].c_str()) - 1;
		int normalIndex;
		if (indices.size() > 2) normalIndex = atoi(indices[2].c_str()) - 1;

		Vertex newVertex(positions[positionIndex]);
		if (indices.size() > 2)
			newVertex = Vertex(positions[positionIndex], texCoords[texCoordIndex], normals[normalIndex]);
		else if (indices.size() > 1)
			newVertex = Vertex(positions[positionIndex], texCoords[texCoordIndex]);
		vertexIndex = vertices.size();
		vertices.push_back(newVertex);
		vertexHashMap[vs] = vertexIndex;


	}
	else {
		vertexIndex = vertexHashMap[vs];
	}

	return vertexIndex;
}

int Mesh3D::minimumCostVertex() {
	unsigned short minimumIndex = 0;
	for (int i = 0; i < vertices.size(); ++i)
	{
		if (vertices[i].collapseCost < vertices[minimumIndex].collapseCost) {
			minimumIndex = i;
		}
	}
	//the costs are usually smaller than 1, this should relatively safely be one of the cases where collapse is not allowed
	//there is certainly a better way to do this but i would have to change a lot for that
	if (vertices[minimumIndex].collapseCost > 900000)
		return -1;

	return minimumIndex;
}

void Mesh3D::removeTriangles(int howMany) {
	//actually removes edges until the desired number of triangles was removed 
	//each removed edge usually removes 1+ triangles, so we may overshoot by a little
	auto target = faces.size() - howMany;
	std::cout << target << std::endl;
	while (faces.size() > target)
	{
		auto eraseIndex = minimumCostVertex();
		if (eraseIndex == -1) {
			std::cout << "You have reduced the mesh to death. Are you proud of yourself?" << std::endl;
			break;
		}
		auto collapseToIndex = vertices[eraseIndex].collapseTarget;
		//GUIONLY
		std::cout << "eraseIndex: " << eraseIndex << std::endl;
		std::cout << "CollapseToIndex: " << collapseToIndex << std::endl;

		collapseEdge(eraseIndex, collapseToIndex);
	}


}

void Mesh3D::collapseEdge(unsigned short p1, unsigned short p2) {
if (p2 < 0) {
		if (vertices[p1].faces.size() == 0) {
			//remove this vertex from all neighbors
			while (vertices[p1].neighbors.size())
			{
				auto it = std::find(vertices[vertices[p1].neighbors[0]].neighbors.begin(), vertices[vertices[p1].neighbors[0]].neighbors.end(), p1);
				if (it != vertices[vertices[p1].neighbors[0]].neighbors.end())
				{
					vertices[vertices[p1].neighbors[0]].neighbors.erase(it);
				}
				auto it2 = std::find(vertices[p1].neighbors.begin(), vertices[p1].neighbors.end(), vertices[p1].neighbors[0]);
				if (it2 != vertices[p1].neighbors.end())
				{
					vertices[p1].neighbors.erase(it2);
				}
			}

			vertices.erase(vertices.begin() + p1);
		}
		return;
	}

	//temporary array of all the neighbors of p1
	std::vector<unsigned short> tmp;
	for (int i = 0; i < vertices[p1].neighbors.size(); ++i)
	{
		tmp.push_back(vertices[p1].neighbors[i]);
	}


	//remove the faces on the edge p1 p2
	auto i = vertices[p1].faces.size();
	int facesRemovedCounter = 0;
	while (i--) {
		if (checkIfVertexInFace(p2, vertices[p1].faces[i]))
		{
			//save some data for cleanup
			auto faceindex = vertices[p1].faces[i];
			std::vector<unsigned short> indexesOfAllFaceVertices(std::begin(faces[faceindex].i), std::end(faces[faceindex].i));

			//delete the face
			faces.erase(faces.begin() + faceindex);
			facesRemovedCounter++;

			//delete the face index from each of its vertices
			int deletedFaceRefFromVerticesCounter = 0;
			for (int n = 0; n < 3; ++n) {
				auto it = std::find(
					vertices[indexesOfAllFaceVertices[n]].faces.begin(),
					vertices[indexesOfAllFaceVertices[n]].faces.end(),
					faceindex);
				if (it != vertices[indexesOfAllFaceVertices[n]].faces.end()) {
					vertices[indexesOfAllFaceVertices[n]].faces.erase(it);
					deletedFaceRefFromVerticesCounter++;
				}
			}
			//now correct the remaining indices that have slipped down
			for (int n = 0; n < vertices.size(); ++n)
			{
				for (int m = 0; m < vertices[n].faces.size(); ++m)
				{
					if (vertices[n].faces[m] > faceindex)
						vertices[n].faces[m]--;
				}
			}

			//now remove each of its vertices from each other one of its vertices, if they're no longer neighbors
			for (int n = 0; n < 3; ++n) {
				auto n2 = (n + 1) % 3;

				checkAndRemoveFromNeighbors(indexesOfAllFaceVertices[n], indexesOfAllFaceVertices[n2]);
				checkAndRemoveFromNeighbors(indexesOfAllFaceVertices[n2], indexesOfAllFaceVertices[n]);
			}
		}
	}
	std::cout << "Removed: " << facesRemovedCounter << " faces." << std::endl;
	//replace the vertices on the remaining triangles
	auto j = vertices[p1].faces.size();
	while (j--) {
		replaceVertexInFace(vertices[p1].faces[j], p1, p2); 
	}
	while (vertices[p1].neighbors.size())
	{
			auto it = std::find(vertices[vertices[p1].neighbors[0]].neighbors.begin(), vertices[vertices[p1].neighbors[0]].neighbors.end(), p1);
			if (it != vertices[vertices[p1].neighbors[0]].neighbors.end())
			{
				vertices[vertices[p1].neighbors[0]].neighbors.erase(it);
			}
			auto it2 = std::find(vertices[p1].neighbors.begin(), vertices[p1].neighbors.end(), vertices[p1].neighbors[0]);
			if (it2 != vertices[p1].neighbors.end())
			{
				vertices[p1].neighbors.erase(it2);
			}
	}
	vertices.erase(vertices.begin() + p1);
	//correct the indices that slipped on the faces
	for (int n = 0; n < faces.size(); ++n)
	{
		for (int m = 0; m < 3; ++m) {
			if (faces[n].i[m] > p1)
				faces[n].i[m]--;
		}
	}
	//correct the indices that slipped in the neighbors
	for (int n = 0; n < vertices.size(); ++n)
	{
		if (vertices[n].collapseTarget > p1)
			vertices[n].collapseTarget--;

		for (int m = 0; m < vertices[n].neighbors.size(); ++m)
		{
			if (vertices[n].neighbors[m] > p1)
				vertices[n].neighbors[m]--;
		}
	}
	//correct the references to neighbors as well
	for (int n = 0; n < tmp.size(); ++n)
	{
		if (tmp[n] > p1) {
			tmp[n]--;
		}
	}


	for (int n = 0; n < tmp.size(); ++n)
	{
		calculateCostAtVertex(tmp[n]);
	}

	initialized = false;
}

void Mesh3D::checkAndRemoveFromNeighbors(unsigned short source, unsigned short toRemove) {
	if (std::count(vertices[source].neighbors.begin(), vertices[source].neighbors.end(), toRemove) == 0)
		return;
	for (int i = 0; i < vertices[source].faces.size(); ++i) {
		if (checkIfVertexInFace(toRemove, vertices[source].faces[i]))
			return;
	}
	vertices[source].neighbors.erase(std::find(
		vertices[source].neighbors.begin(),
		vertices[source].neighbors.end(),
		toRemove
	));
}

void Mesh3D::replaceVertexInFace(unsigned short faceIndex, unsigned short toReplaceIndex, unsigned short replaceWithIndex)
{
	if (toReplaceIndex == faces[faceIndex].i[0])
		faces[faceIndex].i[0] = replaceWithIndex;
	else if (toReplaceIndex == faces[faceIndex].i[1])
		faces[faceIndex].i[1] = replaceWithIndex;
	else
		faces[faceIndex].i[2] = replaceWithIndex;

	auto it = std::find(vertices[toReplaceIndex].faces.begin(), vertices[toReplaceIndex].faces.end(), faceIndex);
	if (it != vertices[toReplaceIndex].faces.end())
		vertices[toReplaceIndex].faces.erase(it);

	vertices[replaceWithIndex].faces.push_back(faceIndex);

	for (int i = 0; i < 3; ++i)
	{
		checkAndRemoveFromNeighbors(toReplaceIndex, faces[faceIndex].i[i]);
		checkAndRemoveFromNeighbors(faces[faceIndex].i[i], toReplaceIndex);
	}
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j)
			if (i != j)
			{
				if (std::count(vertices[faces[faceIndex].i[i]].neighbors.begin(), vertices[faces[faceIndex].i[i]].neighbors.end(), faces[faceIndex].i[j]) == 0)
					vertices[faces[faceIndex].i[i]].neighbors.push_back(faces[faceIndex].i[j]);
			}
	}

	calculateFaceNormal(faces[faceIndex]);
}

double Mesh3D::calculateCollapseCost(unsigned short p1, unsigned short p2) {

	auto p1Vec = CVector(vertices[p1].px, vertices[p1].py, vertices[p1].pz);
	auto p2Vec = CVector(vertices[p2].px, vertices[p2].py, vertices[p2].pz);

	auto length = (p2Vec - p1Vec).getLength();
	double edgeProminence = 0;


	std::vector<unsigned short> mutualNeighbors = vertices[p1].neighbors;
	mutualNeighbors.erase(std::remove_if(mutualNeighbors.begin(), mutualNeighbors.end(), [this, p2](unsigned short elem) {
		return vertices[p2].neighbors.end() == std::find(vertices[p2].neighbors.begin(), vertices[p2].neighbors.end(), elem);
		}));


	std::vector<unsigned short> sides;

	for (int i = 0; i < vertices[p1].faces.size(); ++i) {
		if (checkIfVertexInFace(p2, vertices[p1].faces[i]))
		{
			sides.push_back(vertices[p1].faces[i]);
		}
	}

	for (int i = 0; i < mutualNeighbors.size(); ++i)
	{
		// check the valence of the neighboring vertices
		if (vertices[mutualNeighbors[i]].neighbors.size() <= 3) {
			return 1000000.0;
		}


		//check that a triangle exists for each of the mutual neighbors
		bool found = false;
		for (int j = 0; j < sides.size(); ++j)
		{
			if (checkIfVertexInFace(mutualNeighbors[i], sides[j]))
				found = true;
		}
		if (!found) {
			return 1000000.0;
		}
	}

	//we only need to find out if p1 and p2 are border vertices if the edge is not a border edge (reversing the condition)
	if (sides.size() != 1) {
		if (vertices.size() <= 4)
		{
			return 1000000.0;
		}
		for (int i = 0; i < vertices[p1].neighbors.size(); ++i)
		{
			auto neighborSidesCounter = 0;
			for (int j = 0; j < vertices[vertices[p1].neighbors[i]].faces.size(); ++j) {
				if (checkIfVertexInFace(p1, vertices[vertices[p1].neighbors[i]].faces[j]))
				{
					neighborSidesCounter++;
				}
			}
			if (neighborSidesCounter == 1)
			{
				return 1000000.0;
			}
			
		}
		for (int i = 0; i < vertices[p2].neighbors.size(); ++i)
		{
			auto neighborSidesCounter = 0;
			for (int j = 0; j < vertices[vertices[p2].neighbors[i]].faces.size(); ++j) {
				if (checkIfVertexInFace(p2, vertices[vertices[p2].neighbors[i]].faces[j]))
				{
					neighborSidesCounter++;
				}
			}
			if (neighborSidesCounter == 1)
			{
				return 1000000.0;
			}
			
		}

	}
	else if (vertices.size() <= 3)
	{
		return 1000000.0;
	}


	//actual cost calculation, after considering the edge cases earlier
	for (int i = 0; i < vertices[p1].faces.size(); ++i)
	{
		double startingProminence = 1;
		for (int j = 0; j < sides.size(); ++j) {
			double dotprod = faces[vertices[p1].faces[i]].faceNormal * faces[sides[j]].faceNormal;
			startingProminence = std::min(startingProminence, (1 - dotprod) / 2.0f);
		}
		edgeProminence = std::max(edgeProminence, startingProminence);
	}

	return length * edgeProminence;

}

void Mesh3D::calculateCostAtVertex(unsigned short vertex) {
	//single vertex, free to remove
	if (vertices[vertex].neighbors.size() == 0) {
		vertices[vertex].collapseCost = -1.0;
		vertices[vertex].collapseTarget = -1;
		return;
	}

	vertices[vertex].collapseCost = 999999.0;
	vertices[vertex].collapseTarget = -1;
	for (int i = 0; i < vertices[vertex].neighbors.size(); ++i) {
		double cost = calculateCollapseCost(vertex, vertices[vertex].neighbors[i]);
		if (cost < vertices[vertex].collapseCost) {
			vertices[vertex].collapseTarget = vertices[vertex].neighbors[i];
			vertices[vertex].collapseCost = cost;
		}
	}
}

void Mesh3D::calculateAllCosts() {
	for (int i = 0; i < vertices.size(); ++i) {
		calculateCostAtVertex(i);
	}
}

void Mesh3D::calculateFaceNormal(Triangle& face) {
	
	auto p1 = CVector(vertices[face.i[0]].px, vertices[face.i[0]].py, vertices[face.i[0]].pz);
	auto p2 = CVector(vertices[face.i[1]].px, vertices[face.i[1]].py, vertices[face.i[1]].pz);
	auto p3 = CVector(vertices[face.i[2]].px, vertices[face.i[2]].py, vertices[face.i[2]].pz);
	auto normal = crossProduct(p2 - p1, p2 - p3);
	normal.normalize();

	face.faceNormal = normal;
}

