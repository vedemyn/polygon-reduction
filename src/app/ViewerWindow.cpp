#include "ViewerWindow.h"
#include "../core/ArcBall.h"
#include "../meshConverter/Mesh3D.h"
#include "../GL/glut.h"
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <sys/timeb.h>
#include <ShObjIdl_core.h>
#include <atlstr.h>

ViewerWindow::ViewerWindow()
{
	mode = 2;
	mesh = new Mesh3D();
}

ViewerWindow::~ViewerWindow(void)
{
}

void
ViewerWindow::renderGeometry() {

	glDisable(GL_LIGHTING);
	glShadeModel(GL_FLAT); //shows the polygons more
	
	if (mode == 2) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	mesh->draw(mode);

}

void ViewerWindow::openFileGUI() {
	IFileDialog* pfd = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	hr = pfd->Show(NULL);
	if (SUCCEEDED(hr)) {
		IShellItem* result; 
		hr = pfd->GetResult(&result);
		if (SUCCEEDED(hr)) {
			PWSTR filename;
			result->GetDisplayName(SIGDN_FILESYSPATH, &filename);
			std::string filenamestring = CW2A(filename);
			mesh->readOBJ(filenamestring.c_str());
			mesh->draw();
		}
	}
}

void ViewerWindow::keyEvent(unsigned char key,int x,int y){
	switch(key) {
	case '1':
		//color based on vertex normals
		mode = 0;
		break;
	case '2':
		//color based on uv coords
		mode = 1;
		break;
	case '3':
		//only edges
		mode = 2;
		break;
	case 'r':
		mesh->removeTriangles(1);
		renderGeometry();
		break;
	case 'o':
		openFileGUI();
		break;
	case 's':
		mesh->writeOBJ();
		break;
	}

}
