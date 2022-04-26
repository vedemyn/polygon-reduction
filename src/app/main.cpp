// TerraGen.cpp : Defines the entry point for the console application.
//

#include "ViewerWindow.h"
#include "../GL/glut.h"
#include "../meshConverter/Mesh3D.h"
#include <time.h>
#include <iostream>


CGlutWindow* g_pMainWindow = 0;

void display(void) { g_pMainWindow->renderFrame(); }
void idle(void) { g_pMainWindow->idle(); }
void reshape(int width, int height) { g_pMainWindow->resize(width, height); }
void keyboard(unsigned char key, int x, int y) { g_pMainWindow->keyEvent(key, x, y); }
void mouse(int button, int state, int x, int y) { g_pMainWindow->mouseButtonEvent(button, state, x, y); }
void move(int x, int y) { g_pMainWindow->mouseMoveEvent(x, y); }

int main(int argc, char* argv[])
{
	std::cout << argc << std::endl;
	std::cout << argv[0] << std::endl;
	if (argc <= 1) {
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

		g_pMainWindow = new ViewerWindow();

		glutDisplayFunc(display);
		glutReshapeFunc(reshape);
		glutKeyboardFunc(keyboard);
		glutMouseFunc(mouse);
		glutMotionFunc(move);
		//glutPassiveMotionFunc(move);
		glutIdleFunc(idle);
		glutMainLoop();
	}
	else if (argc > 2) {
		//just the mesh reduction here by reading the args, no gui
		auto mesh = new Mesh3D();
		//could put in checks if the first arg is a path...
		mesh->readOBJ(argv[1]);
		mesh->removeTriangles(atoi(argv[2]));
		mesh->writeOBJ();
	}


	return 0;
}
