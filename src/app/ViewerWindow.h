#pragma once
#include "../core/GlutWindow.h"
#include "globaldefs.h"

class Mesh3D;

class ViewerWindow :
	public CGlutWindow
{
	

public:
	ViewerWindow();
	virtual ~ViewerWindow(void);

	//virtual void keyEvent(unsigned char key,int x,int y);
	void renderGeometry();

	void openFileGUI();

	void keyEvent(unsigned char key,int x,int y);

protected:
	Mesh3D *mesh;
	int mode;

};
