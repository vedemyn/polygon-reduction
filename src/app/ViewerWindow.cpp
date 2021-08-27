#include "app/ViewerWindow.h"
#include "core/ArcBall.h"
#include <math.h>
#include <GL/glut.h>
#include <stdio.h>
#include <assert.h>
#include <sys/timeb.h>
#include "meshConverter\Mesh3D.h"


ViewerWindow::ViewerWindow()
{
	//const char *filename = "input\\Marsienne_base.obj";
	//const char *filename = "input\\Sphere.obj";
	const char *filename = "input\\suzanne.obj";

	mode = 0;
	mesh = new Mesh3D();
	mesh->readOBJ(filename);
}

ViewerWindow::~ViewerWindow(void)
{
	
}

void
ViewerWindow::renderGeometry() {
	
	if (mode == 0) {
		glEnable(GL_LIGHTING);
	} else {
		glDisable(GL_LIGHTING);
	}
	if (mode == 3) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	mesh->draw(mode);

}

void ViewerWindow::keyEvent(unsigned char key,int x,int y){
	switch(key) {
	case '1':
		mode = 0;
		break;
	case '2':
		mode = 1;
		break;
	case '3':
		mode = 2;
		break;
	case '4':
		mode = 3;
		break;
	case 'r':
		mesh->collapseEdges(1);
		mesh->draw(mode);
		break;

	}
}
