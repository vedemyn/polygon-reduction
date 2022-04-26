
#include "Mesh3D.h"

#include <Windows.h>
#include <GL/glew.h>
#include <GL\GL.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void Mesh3D::createVertexBuffer() {

	int size = vertices.size() * 8; 
	float *vertexBuffer = new float[size];

	std::vector<Vertex>::iterator it = vertices.begin();

	int vbIndex = 0;
	while(it != vertices.end()) {
		Vertex v = (*it);

		vertexBuffer[vbIndex++] = v.px;
		vertexBuffer[vbIndex++] = v.py;
		vertexBuffer[vbIndex++] = v.pz;
		vertexBuffer[vbIndex++] = v.u;
		vertexBuffer[vbIndex++] = v.v;
		vertexBuffer[vbIndex++] = v.nx;
		vertexBuffer[vbIndex++] = v.ny;
		vertexBuffer[vbIndex++] = v.nz;
		it++;
	}

	//vertices.clear();

	glGenBuffers(1, &vboID); // Create the buffer ID, this is basically the same as generating texture ID's
    glBindBuffer(GL_ARRAY_BUFFER, vboID); // Bind the buffer (vertex array data)

    glBufferData(GL_ARRAY_BUFFER, size*sizeof(float), vertexBuffer, GL_STATIC_DRAW);

    // When we get here, all the vertex data is effectively on the card
	delete [] vertexBuffer;


}

void Mesh3D::createIndexBuffer() {

	int size = faces.size() * 3;
	numPrimitives = size;
	unsigned short *indexBuffer = new unsigned short[size];
	std::vector<Triangle>::iterator it = faces.begin();

	int ibIndex = 0;
	while(it != faces.end()) {
		Triangle f = (*it);

		indexBuffer[ibIndex++] = f.i[0];
		indexBuffer[ibIndex++] = f.i[1];
		indexBuffer[ibIndex++] = f.i[2];
		it++;
	}
	//faces.clear();
	
    glGenBuffers(1, &iboID); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*sizeof(unsigned short), indexBuffer, GL_STATIC_DRAW);
	
	delete [] indexBuffer;
}

void Mesh3D::draw(int mode) {

	if (!initialized) {
		createVertexBuffer();
		createIndexBuffer();
		initialized = true;
	}

    // Bind our buffers much like we would for texturing
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
	
    glVertexPointer(  3, GL_FLOAT, 8*sizeof(float), BUFFER_OFFSET(0*sizeof(float)));
    glTexCoordPointer(2, GL_FLOAT, 8*sizeof(float), BUFFER_OFFSET(3*sizeof(float)));
    glNormalPointer(     GL_FLOAT, 8*sizeof(float), BUFFER_OFFSET(5*sizeof(float)));
	


	switch(mode) {
	case 0:
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3,      GL_FLOAT, 8*sizeof(float), BUFFER_OFFSET(5*sizeof(float)));
		break;
	case 1:
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_FLOAT, 8*sizeof(float), BUFFER_OFFSET(3*sizeof(float)));
		break;
	case 2:
		glDisableClientState(GL_COLOR_ARRAY);
		break;
	}

    // Actually do our drawing, parameters are Primative (Triangles, Quads, Triangle Fans etc), Elements to 
    // draw, Type of each element, Start Offset
    glDrawElements(GL_TRIANGLES, numPrimitives, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));

	

    // Disable our client state back to normal drawing
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);


}