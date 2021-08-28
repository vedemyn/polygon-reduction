
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

	calculateAllCosts();

	//number of neighbors and faces on each index seems correct

	for (int i = 0; i < vertices.size(); ++i)
	{
		std::cout << vertices[i].neighbors.size() << "  ";
	}
	std::cout << std::endl;
	std::cout << std::endl;

	for (int i = 0; i < vertices.size(); ++i)
	{
		std::cout << vertices[i].faces.size() << "  ";
	}


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
		auto tri = (Triangle(index0, index1, index2));
		calculateFaceNormal(tri);
		faces.push_back(tri);
		// save information on which face the indexes belong to
		//maaaaaaybe pull these out into a function lol
		vertices[index0].faces.push_back(faces.size() - 1);
		if (std::count(vertices[index0].neighbors.begin(), vertices[index0].neighbors.end(), index1) == 0)
			vertices[index0].neighbors.push_back(index1);
		if (std::count(vertices[index0].neighbors.begin(), vertices[index0].neighbors.end(), index2) == 0)
			vertices[index0].neighbors.push_back(index2);

		vertices[index1].faces.push_back(faces.size() - 1);
		if (std::count(vertices[index1].neighbors.begin(), vertices[index1].neighbors.end(), index0) == 0)
			vertices[index1].neighbors.push_back(index0);
		if (std::count(vertices[index1].neighbors.begin(), vertices[index1].neighbors.end(), index2) == 0)
			vertices[index1].neighbors.push_back(index2);

		vertices[index2].faces.push_back(faces.size() - 1);
		if (std::count(vertices[index2].neighbors.begin(), vertices[index2].neighbors.end(), index0) == 0)
			vertices[index2].neighbors.push_back(index0);
		if (std::count(vertices[index2].neighbors.begin(), vertices[index2].neighbors.end(), index1) == 0)
			vertices[index2].neighbors.push_back(index1);
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

		Vertex newVertex(positions[positionIndex]//);
			,
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

unsigned short Mesh3D::minimumCostVertex() {
	unsigned short minimumIndex = 0;
	for (int i = 0; i < vertices.size(); ++i)
	{
		if (vertices[i].collapseCost < vertices[minimumIndex].collapseCost) {
			minimumIndex = i;
		}
	}
	return minimumIndex;
}

void Mesh3D::collapseEdges(int howMany) {

	for (int n = 0; n < howMany; ++n)
	{
		auto eraseIndex = minimumCostVertex();
		auto collapseToIndex = vertices[eraseIndex].collapseTarget;
		std::cout << "eraseIndex: " << eraseIndex << std::endl;
		std::cout << "CollapseToIndex: " << collapseToIndex << std::endl;

		collapseEdge(eraseIndex, collapseToIndex);

		/*
		//find triangles with both vertices of the edge and remove them
		//need an iterator first
		std::vector<unsigned short> indicesOfTrianglesContainingEdge;
		for (int n = 0; n < faces.size(); ++n) {
			for (int i = 0; i < 3; i++) {
				//unnecessary checking of some edges here but whatevs
				if ((faces[n].i[i] == eraseIndex) && (faces[n].i[(i + 1) % 3] == collapseToIndex || faces[n].i[(i + 2) % 3] == collapseToIndex)) {
					indicesOfTrianglesContainingEdge.push_back(n);
					break;
				}
			}
		}

		for (int i = 0; i < indicesOfTrianglesContainingEdge.size(); i++) {
			std::cout << "erasing " << indicesOfTrianglesContainingEdge[i] << std::endl;
			faces.erase(faces.begin() + indicesOfTrianglesContainingEdge[i]);
		}

		//update triangles with v1 of the edge to use v2 instead

		for (int n = 0; n < faces.size(); ++n) {
			for (int i = 0; i < 3; i++) {
				if (faces[n].i[i] == eraseIndex) {
					std::cout << "replacing vertex in " << n << std::endl;
					faces[n].i[i] = collapseToIndex;
					//have to recalculate the face normals of these faces, since they have been changed
					//this assumes that there's no malformed face where the same vertex is in twice......
					// maybe rewrite so that assumption isn't true later
					calculateFaceNormal(faces[n]);
					break;
				}
			}

		}
		//remove v1
		vertices.erase(vertices.begin() + eraseIndex);



		//update indices of points in triangles
		for (int i = 0; i < faces.size(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				if (faces[i].i[j] > eraseIndex)
					faces[i].i[j]--;
			}
		}
		//update indices of triangles in points, and neighbors in points...
		for (int i = 0; i < vertices.size(); ++i)
		{
			for (int j = 0; j < vertices[i].faces.size(); ++j)
			{
				for (int k = 0; k < indicesOfTrianglesContainingEdge.size(); ++k)
					if (vertices[i].faces[j] > indicesOfTrianglesContainingEdge[k])
						vertices[i].faces[j]--;
			}
			for (int j = 0; j < vertices[i].neighbors.size(); ++j)
			{
				if (vertices[i].neighbors[j] > eraseIndex)
					vertices[i].neighbors[j]--;
			}
		}
		*/

	}

}

void Mesh3D::collapseEdge(unsigned short p1, unsigned short p2) {
if (p2 < 0) {
		std::cout << "A single vertex somehow" << std::endl;
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
	std::cout<< i << std::endl;
	int facesRemovedCounter = 0;
	while (i--) {
		auto indexesOfFaceVertices = faces[vertices[p1].faces[i]].i;
		if (indexesOfFaceVertices[0] == p2 || indexesOfFaceVertices[1] == p2 || indexesOfFaceVertices[2] == p2) {
			
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
			std::cout << "Deleted the " << facesRemovedCounter << ". face from " << deletedFaceRefFromVerticesCounter << " vertices" << std::endl;
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

				RemoveIfNonNeighbor(indexesOfAllFaceVertices[n], indexesOfAllFaceVertices[n2]);
				RemoveIfNonNeighbor(indexesOfAllFaceVertices[n2], indexesOfAllFaceVertices[n]);
			}
		}
	}
	std::cout << "Removed: " << facesRemovedCounter << " faces." << std::endl;
	//replace the vertices on the remaining triangles
	//416 removed at this point
	auto j = vertices[p1].faces.size();
	std::cout<< j << std::endl;
	while (j--) {
		replaceVertexInFace(vertices[p1].faces[j], p1, p2); //the offender that adds a vertex to its own neighbors
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

void Mesh3D::RemoveIfNonNeighbor(unsigned short source, unsigned short toRemove) {
	if (std::count(vertices[source].neighbors.begin(), vertices[source].neighbors.end(), toRemove) == 0)
		return;
	for (int i = 0; i < vertices[source].faces.size(); ++i) {
		if (faces[vertices[source].faces[i]].i[0] == toRemove || faces[vertices[source].faces[i]].i[1] == toRemove || faces[vertices[source].faces[i]].i[2] == toRemove)
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
	//assert both vertices exist
	//assert one of the vertices in the face is  toReplaceIndex
	//assert none of the vertices is already replaceWithIndex
	if (toReplaceIndex == faces[faceIndex].i[0])
		faces[faceIndex].i[0] = replaceWithIndex;
	else if (toReplaceIndex == faces[faceIndex].i[1])
		faces[faceIndex].i[1] = replaceWithIndex;
	else
		//assert this vertex has to be toreplaceindex
		faces[faceIndex].i[2] = replaceWithIndex;

	auto it = std::find(vertices[toReplaceIndex].faces.begin(), vertices[toReplaceIndex].faces.end(), faceIndex);
	if (it != vertices[toReplaceIndex].faces.end())
		vertices[toReplaceIndex].faces.erase(it);

	vertices[replaceWithIndex].faces.push_back(faceIndex);

	for (int i = 0; i < 3; ++i)
	{
		RemoveIfNonNeighbor(toReplaceIndex, faces[faceIndex].i[i]);
		RemoveIfNonNeighbor(faces[faceIndex].i[i], toReplaceIndex);
	}
	for (int i = 0; i < 3; ++i) {
		//assert that each of the faces vertices' face arrays contains this face exactly once
		for (int j = 0; j < 3; ++j)
			if (i != j)
			{
				//CHECK THIS WHEN SLEPT
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
	double curvature = 0;

	std::vector<unsigned short> sides;

	for (int i = 0; i < vertices[p1].faces.size(); ++i) {
		if (faces[vertices[p1].faces[i]].i[0] == p2 || faces[vertices[p1].faces[i]].i[1] == p2 || faces[vertices[p1].faces[i]].i[2] == p2)
		{
			sides.push_back(vertices[p1].faces[i]);
		}
	}

	for (int i = 0; i < vertices[p1].faces.size(); ++i)
	{
		double mincurv = 1;
		for (int j = 0; j < sides.size(); ++j) {
			double dotprod = faces[vertices[p1].faces[i]].faceNormal * faces[sides[j]].faceNormal;
			mincurv = std::min(mincurv, (1 - dotprod) / 2.0f);
		}
		curvature = std::max(curvature, mincurv);
	}

	return length * curvature;

	/*
	//that's not a valid edge...
	if (p1 == p2) return 999999.0;

	auto p1Vec = CVector(vertices[p1].px, vertices[p1].py, vertices[p1].pz);
	auto p2Vec = CVector(vertices[p2].px, vertices[p2].py, vertices[p2].pz);

	auto length = (p2Vec - p1Vec).getLength();
	auto curvature = 0.0;

	//get all triangles containing p1
	std::vector<unsigned short> p1Triangles = vertices[p1].faces;

	//get all triangles containing p1 and p2	
	std::vector<unsigned short> p1p2Triangles = p1Triangles;
	p1p2Triangles.erase(std::remove_if(p1p2Triangles.begin(), p1p2Triangles.end(),
		[this, p2](unsigned short i) {return (faces[i].i[0] != p2) && (faces[i].i[1] != p2) && (faces[i].i[2] != p2); }),
		p1p2Triangles.end());

	//do not collapse border edges
	// ??? unless both vertices are also border vertices
		std::vector<unsigned short> indicesOfTrianglesContainingEdge;
		auto triangleCounter = 0;
		for (int n = 0; n < faces.size(); ++n) {
			for (int i = 0; i < 3; i++) {
				//unnecessary checking of some edges here but whatevs
				if ((faces[n].i[i] == p1) && (faces[n].i[(i + 1) % 3] == p2 || faces[n].i[(i + 2) % 3] == p2)) {
					indicesOfTrianglesContainingEdge.push_back(n);
					triangleCounter++;
					break;
				}
			}
			if (triangleCounter > 2) break;
		}
		if (triangleCounter < 2) return 999999.0;


	// check whether all the triangles exist for the neighbors both points have in common
	std::vector<unsigned short> commonNeighbors = vertices[p1].neighbors;
	commonNeighbors.erase(std::remove_if(commonNeighbors.begin(), commonNeighbors.end(),
		[this, p2](unsigned short i) {
			bool result = true;
			for (int n = 0; n < vertices[p2].neighbors.size(); ++n)
			{
				if (vertices[p2].neighbors[n] == i)
					result = false;
			}
			return result;
		}),
		commonNeighbors.end());

	for (int i = 0; i < commonNeighbors.size(); ++i) {

		// check for valence of point. But I don't have a quick way to access that,
		// so i check if it's part of at least 4 triangles instead
		// PROBABLY NOT CORRECT BUT CLOSE ENOUGH FOR NOW 
		if (vertices[commonNeighbors[i]].faces.size() < 4) {
			//std::cout << "Valence smaller 3" << std::endl;
			return 999999.0;
		}
		bool triangleExists = false;
		for (int j = 0; j < vertices[i].faces.size(); ++j) {
			triangleExists |=  (faces[vertices[i].faces[j]].i[0] == p1 ||
								faces[vertices[i].faces[j]].i[1] == p1 ||
								faces[vertices[i].faces[j]].i[2] == p1)
								&&
							   (faces[vertices[i].faces[j]].i[0] == p2 ||
								faces[vertices[i].faces[j]].i[1] == p2 ||
								faces[vertices[i].faces[j]].i[2] == p2);

		}
		if (!triangleExists) {
			//std::cout << "FOUND A NON TRIANGLE" << std::endl;
			return 999999.0;
		}
	}



	for (int i = 0; i < p1Triangles.size(); ++i) {
		auto startingCurvature = 1.0;

		for (int j = 0; j < p1p2Triangles.size(); ++j) {

			auto normal1 = faces[p1Triangles[i]].faceNormal;
			auto normal2 = faces[p1p2Triangles[j]].faceNormal;
			auto dotproduct = normal1 * normal2;

			startingCurvature = std::min(startingCurvature, (1.0f - dotproduct) / 2.0f);
		}

		curvature = std::max(curvature, startingCurvature);
	}

	auto cost = length * curvature;

	//std::cout << "cost: " << cost << std::endl;
	return cost;

	*/
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

std::pair<unsigned short, unsigned short> Mesh3D::calculateLowestCostPair() {
	std::pair <unsigned short, unsigned short> result;
	double collapseCost = 999999.0;
	//for each face...
	for (int n = 0; n < faces.size(); ++n) {
		//for all 6 edge variations on that face (3 * 2 for swapped direction)
		for (int i = 0; i < 3; ++i) {
			if (faces[n].i[i] != faces[n].i[(i + 1) % 3]) {
				auto cost = calculateCollapseCost(faces[n].i[i], faces[n].i[(i + 1) % 3]);
				if (cost < collapseCost) {
					collapseCost = cost;
					result.first = faces[n].i[i];
					result.second = faces[n].i[(i + 1) % 3];
					std::cout << "Lower cost: " << cost << "  at vertices " << result.first << " and " << result.second << std::endl;
				}

			}
			if (faces[n].i[i] != faces[n].i[(i + 2) % 3]) {
				auto cost = calculateCollapseCost(faces[n].i[i], faces[n].i[(i + 2) % 3]);
				if (cost < collapseCost) {
					collapseCost = cost;
					result.first = faces[n].i[i];
					result.second = faces[n].i[(i + 2) % 3];
					std::cout << "Lower cost: " << cost << "  at vertices " << result.first << " and " << result.second << std::endl;
				}
			}
		}
	}
	//std::cout << "Lowest Cost: " << collapseCost << std::endl;

	return result;
}


void Mesh3D::calculateFaceNormal(Triangle& face) {
	
	auto p1 = CVector(vertices[face.i[0]].px, vertices[face.i[0]].py, vertices[face.i[0]].pz);
	auto p2 = CVector(vertices[face.i[1]].px, vertices[face.i[1]].py, vertices[face.i[1]].pz);
	auto p3 = CVector(vertices[face.i[2]].px, vertices[face.i[2]].py, vertices[face.i[2]].pz);
	auto normal = crossProduct(p2 - p1, p2 - p3);
	normal.normalize();

	face.faceNormal = normal;
}

