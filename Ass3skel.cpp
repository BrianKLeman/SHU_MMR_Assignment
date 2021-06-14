// Ass3skel.cpp 11/03/08
#include <cmath>
#include <cassert>
#include <iostream>
#include <cstdlib>
using namespace std;
#include "glut.h"

// ********************************************************************
//    function prototypes
// ********************************************************************
// reflected vector
void reflectVector( const float Light[3],const float Normal[3], float Result[3] );
// Scalar product.  
void scalarProduct(const float first[3], const float second[3], float& scalProd);
//---------------------------------------------------------------------------
// Normalise vector.
void normalise(float vec[3]);
//---------------------------------------------------------------------------
// Add vectors
void addVec(const float first[3], const float second[3], float sum[3]);
//---------------------------------------------------------------------------
// Subtract vectors
void subtractVec(const float first[3], const float second[3], float diff[3]);
//---------------------------------------------------------------------------
// Subtract coordinates of points (first-second) to give vector
void subtractPoints(const float first[4], const float second[4], float diff[3]);
//---------------------------------------------------------------------------
// extract single column from Matrix to Vector
void extractColumn (const float matrix[4][3], int columnNo, float column[4]);
void extractColumn3(const float matrix[3][3], int columnNo, float column[3]);
//---------------------------------------------------------------------------
// Copy vector
void copyVec(const float source[3], float result[3]);
//---------------------------------------------------------------------------
// Multiply vector by scalar.
void scaleVec(const float source[3], float scalar, float result[3]);
//---------------------------------------------------------------------------
// Draw pixel at position start,with colour named in last parameter
void drawPixel (float startX, float startY, const float col);
//---------------------------------------------------------------------------
// Fill polygon
void fillPolygonPix 
	(int vertexCount, const float verticesX[], const float verticesY[]);
//---------------------------------------------------------------------------
void callbackDisplay(); // global prototype for a frequently-used function

// ******* Global variables and constants *******

const int clientHeight = 600; // Graphics window height (pixels)
const int clientWidth = 600; // Graphics window width (pixels)

enum RenderType {flat, Gouraud, Phong};
//RenderType currentRender = Gouraud;
RenderType currentRender = Phong;

// Points are columns, representing homogeneous coords, in this array
const float patch[4][3] = // array of 3 points
{
	{250, 100, 300},
	{100, 150, 300},
	{  0,   0,   0},
	{  1,   1,   1}
}; 
const float objectCentre[4] = {206,217,-3000,1};
const float lightZ = 300;
const float viewPoint[4] = {200,200,300,1};
float lightPosition[4] = {200,200,lightZ,1};

const float I_ambient (0.2); 
const float I_light (0.5); 
const float k_ambient (0.5); 
const float k_diffuse (0.3); 
const float k_specular (0.8); 
const float specular_exponent = 30;

//material
const float k_a[3] = { 0.25,0.25,0.25 };
const float k_d[3] = { 0.4,0.4,0.4 };
const float k_s[3] = { 0.774597,0.774597,0.774597};
const float k_e = 76.8;//12.8;

//light intensity
const float I_a[3] = {0.3,0.3,0.3};
const float I_d[3] = {0.5,0.5,0.5};
const float I_s[3] = {1.0,1.0,1.0};


// ********************************************************************
//         main function and user-interface
// ********************************************************************

int main(int argc, char* argv[])
{
	void showCommands();
	void setUpMenu();
	void callbackMouse(int btn, int state, int x, int y);
	void callbackMotion(int x, int y);
    showCommands();
	// initialise OpenGL
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); // must be before glutCreateWindow
	glutInitWindowSize(clientWidth,clientHeight);
	glutInitWindowPosition(50,50);
	glutCreateWindow("MMR - shading");
	glClearColor(0,0,0,1); // black
	glShadeModel(GL_SMOOTH);
	gluOrtho2D(0,clientWidth,0,clientHeight); 
    glutDisplayFunc(callbackDisplay);
    glutMouseFunc(callbackMouse);
	glutMotionFunc(callbackMotion);
    setUpMenu();
    glutMainLoop();
    return 0;
}
//---------------------------------------------------------------------------

// List the commands available
void showCommands()
{
    cout << "\nPress left mouse button for x- any y-coords of light\n" << endl;
    cout << "\nPress right mouse button for commands\n" << endl;
}

// Menu system to be activated when right mouse button is pressed
void right_menu(int id)
{
   switch (id)
   {
     case 35: currentRender = flat; callbackDisplay(); break;
     case 36: currentRender = Gouraud; callbackDisplay(); break;
     case 38: currentRender = Phong; callbackDisplay(); break;
     case 99: exit(0); break;
   }
}

void setUpMenu()
{
	glutCreateMenu(right_menu);
	glutAddMenuEntry("Flat shading", 35);
	glutAddMenuEntry("Gouraud shading", 36);
	glutAddMenuEntry("Phong shading", 38);
	glutAddMenuEntry("Exit",99);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
  }


// Mouse button pressed
void callbackMouse(int btn, int state, int x, int y)
{
    if(btn==GLUT_LEFT_BUTTON && state==GLUT_DOWN) 
    {
		y = clientHeight - y; // Mouse coords are in window system, origin top left
		lightPosition[0] = x;
		lightPosition[1] = y;
		lightPosition[2] = lightZ;
		callbackDisplay();
	}
}
//---------------------------------------------------------------------------

// Called when mouse moved with button pressed.
void callbackMotion(int x, int y)
{
	y = clientHeight - y; // Mouse coords are in window system, origin top left
	lightPosition[0] = x;
	lightPosition[1] = y;
	lightPosition[2] = lightZ;
	callbackDisplay();
}

// ********************************************************************
//         Functions for shading
// ********************************************************************

// Draw the patch
void callbackDisplay()
{
	void setVertexColours3 (float vertexColour[3][3], const float normals[3][3], 
					   float vertexViewVec[3][3], float vertexLightVec[3][3]);

	void setUpNormals(float normals[3][3]);
	//this function sets up the view vectors
	void setUpViewVectors(float viewVecs[3][3]);
	void setVertexColours
		(float vertexColour[], const float normals[3][3], 
		 float vertexViewVec[3][3], float vertexLightVec[3][3]);
	void doInterpolationGouraud(const int pixelX[], const int pixelY[], 
		const float vertexColour[3][3]);
	void doInterpolationPhong(const int pixelX[], const int pixelY[], 
		const float normals[3][3], const float view[3][3], const float light[3][3]);
	void setUpLightVectors(float lightVec[3][3]);
	int pixelX[3], pixelY[3];
	// vectors, at each vertex of the path, are rows in these arrays.
	float normals[3][3];
	float vertexViewVec[3][3];
	float vertexLightVec[3][3];
	float vertexColour[3];
	float vertexGColour[3][3];
	float col[3];
	setUpNormals(normals);
	setUpLightVectors(vertexLightVec);
	setUpViewVectors(vertexViewVec);
	setVertexColours(vertexColour, normals, vertexViewVec,vertexLightVec);
	setVertexColours3(vertexGColour, normals, vertexViewVec,vertexLightVec);

	for (int i(0); i < 3; ++i)
	{
		pixelX[i] = patch[0][i] / patch[3][i];
		pixelY[i] = patch[1][i] / patch[3][i];
	}
	glClear(GL_COLOR_BUFFER_BIT); // Clear graphics window
	switch (currentRender)
	{
	case flat: 
	//  set average colour for each pixel inside the triangle
		
		
		float pixX[3], pixY[3];
		int i;
		for (i = 0; i < 3; ++i)
		{
			pixX[i] = pixelX[i];
			col[i] = (vertexGColour[0][i]+vertexGColour[1][i]+vertexGColour[2][i]) * (1.0/3);
			pixY[i] = pixelY[i];
		}
		glColor3f (col[0],col[1],col[2]);
		fillPolygonPix (3, pixX, pixY);
	break;
	case Gouraud: 
	// Interpolate to set colour for each pixel inside the triangle
		doInterpolationGouraud(pixelX, pixelY, vertexGColour);
	break;
	case Phong: 
		doInterpolationPhong(pixelX, pixelY, normals, vertexViewVec, vertexLightVec);
	break;
	}
	glutSwapBuffers();
	glFlush(); // ensure that results of graphics commands are actually displayed
}
//---------------------------------------------------------------------------

void setUpNormals(float normals[3][3])
{
	for (int p = 0; p < 3; ++p)
	{
		float oneVertex[4];
		extractColumn (patch, p, oneVertex);
		subtractPoints(oneVertex,objectCentre , normals[p]);
		normalise(normals[p]);
	}
			/*for(int i = 0;i<3;i++)
	cout<<"normal["<<i<<"]: "<<normals[0][i]<<endl;
	for(int i = 0;i<3;i++)
	cout<<"normal["<<i<<"]: "<<normals[1][i]<<endl;
	for(int i = 0;i<3;i++)
	cout<<"normal["<<i<<"]: "<<normals[2][i]<<endl;*/

}

//-----------------------------------------------------------------------------
//this function sets up the light vectors
void setUpLightVectors(float lightVecs[3][3])
{
	float absVertex[3];
	for (int p = 0; p < 3; ++p)
	{
		float oneVertex[4];
		extractColumn (patch, p, oneVertex);
		//store absolute position of vertex
		addVec(objectCentre,oneVertex,absVertex);
		subtractPoints(lightPosition,absVertex , lightVecs[p]);
		normalise(lightVecs[p]);
	}
	/*for(int i = 0;i<3;i++)
	cout<<"light["<<i<<"]: "<<lightVecs[0][i]<<endl;
	for(int i = 0;i<3;i++)
	cout<<"light["<<i<<"]: "<<lightVecs[1][i]<<endl;
	for(int i = 0;i<3;i++)
	cout<<"light["<<i<<"]: "<<lightVecs[2][i]<<endl;*/
}

//-----------------------------------------------------------------------------
//this function sets up the view vectors
void setUpViewVectors(float viewVecs[3][3])
{
	float absVertex[3];
	for (int p = 0; p < 3; ++p)
	{
		float oneVertex[4];
		extractColumn (patch, p, oneVertex);
		//store absolute position of vertex
		addVec(objectCentre,oneVertex,absVertex);
		subtractPoints(viewPoint, absVertex, viewVecs[p]);
		normalise(viewVecs[p]);
	}
	/*for(int i = 0;i<3;i++)
	cout<<"view["<<i<<"]: "<<viewVecs[0][i]<<endl;
	for(int i = 0;i<3;i++)
	cout<<"view["<<i<<"]: "<<viewVecs[1][i]<<endl;
	for(int i = 0;i<3;i++)
	cout<<"view["<<i<<"]: "<<viewVecs[2][i]<<endl;*/
}

//---------------------------------------------------------------------------

void setVertexColours (float vertexColour[], const float normals[3][3], 
					   float vertexViewVec[3][3], float vertexLightVec[3][3])
{
	float setColour
		(const float normal[3], const float viewVec[3], const float lightVec[3]);
	float viewVec[3], lightVec[3];
	for (int i = 0; i < 3; ++i)
	{
		// more code to be written
		//swapped these vectors around
		copyVec(vertexViewVec[i],viewVec );
		//normalise the view vector
		normalise(viewVec);
		//copy light vector into the lightVec array for calculations
		copyVec(vertexLightVec[i],lightVec );
		//normalise the light vector
		normalise(lightVec);
		vertexColour[i] = setColour(normals[i], viewVec, lightVec);
	}
}

//---------------------------------------------------------------------------

void setVertexColours3 (float vertexColour[3][3], const float normals[3][3], 
					   float vertexViewVec[3][3], float vertexLightVec[3][3])
{
	void setColourRGB
		(const float normal[3], const float viewVec[3], const float lightVec[3], float colour[3]);
	float viewVec[3], lightVec[3];
	for (int i = 0; i < 3; ++i)
	{
		// more code to be written
		//swapped these vectors around
		copyVec(vertexViewVec[i],viewVec );
		//normalise the view vector
		normalise(viewVec);
		//copy light vector into the lightVec array for calculations
		copyVec(vertexLightVec[i],lightVec );
		//normalise the light vector
		normalise(lightVec);
		
		setColourRGB(normals[i], viewVec, lightVec, vertexColour[i]);
	}
}

//---------------------------------------------------------------------------

// Calculate the colour at a point, 
// from normal, light direction and viewing direction
// See Section 6 of the notes
float setColour(const float normal[3], const float viewVec[3], const float lightVec[3])
{
	//calculate the reflected vector first
	float R[3];
	reflectVector(lightVec,normal,R);
	float dotRV;
	scalarProduct(R,viewVec, dotRV);
	float dotNL;
	scalarProduct(normal,lightVec,dotNL);
 	float col = (k_ambient * I_ambient) + (k_diffuse * I_light* dotNL) + I_light*k_specular*(powf(dotRV,specular_exponent));
	// more code to be written
	return col;
}

// Calculate the colour at a point, 
// from normal, light direction and viewing direction
// See Section 6 of the notes
void setColourRGB(const float normal[3], const float viewVec[3], const float lightVec[3],float colours[3])
{
	//calculate the reflected vector first
	float R[3];
	reflectVector(lightVec,normal,R);
	float dotRV;
	scalarProduct(R,viewVec, dotRV);
	float dotNL;
	scalarProduct(normal,lightVec,dotNL);
 
	

 	for( int i = 0;i<3;i++)
	{
 		colours[i] = (k_a[i] * I_a[i]) + (k_d[i] * I_d[i]* dotNL) + (I_s[i]*k_s[i]*(powf(dotRV,k_e)));
	}
	// more code to be written
	
}



//---------------------------------------------------------------------------

void doInterpolationGouraud(const int pixelX[], const int pixelY[], 
				  const float vertexColour[3][3]) 
{
	void interpolateScanLineGouraud
		(int xleft, int xright, int yQ, float Ileft[3], float Iright[3]);
	// Use notation in Section 7.3 of the notes
	const int x2 = pixelX[0];
	const int y2 = pixelY[0];
	const int x1 = pixelX[1];
	const int y1 = pixelY[1];
	const int x3 = pixelX[2];
	const int y3 = pixelY[2];
	float I1[3], I2[3], I3[3], I4[3], I5[3];
	//holds red, green blue components of intensity
	float t;
	int yQ, x4, x5;
	copyVec(vertexColour[0],I2);
	copyVec(vertexColour[1],I1);
	copyVec(vertexColour[2],I3);

	//I2 = (vertexColour[0][0] + vertexColour[0][1] + vertexColour[0][2])/3.0;
	//I1 = (vertexColour[1][0] + vertexColour[1][1] + vertexColour[1][2])/3.0;
	//I3 = (vertexColour[2][0] + vertexColour[2][1] + vertexColour[2][2])/3.0;
	// for each scan line from y2 to y1-1
	
	for (yQ = y2; yQ < y1; ++yQ)
	{
		// interpolate to set x4 and I4 (code to be written)
		//calculate t
		t = (float)(yQ-y2)/(float)(y1-y2);
		//calculate I4
		for(int i = 0; i<3;i++)
			I4[i] = I2[i]*(1-t) + I1[i]*t;
		//calculate x4 in terms of y
		//y -y1= m(x -x1) there for x = ((y-y1)/m) + x1 | m = x2-x1/y2-y1
		float m = (float)(y2-y1)/(float)(x2-x1);
		//
		x4 = ((yQ-y1)/m) + x1;
		// interpolate to set x5 and I5 (code to be written)
		t = (float)(yQ-y2)/(float)(y3-y2);
		//calculate I5
		for(int i = 0; i<3;i++)
			I5[i] = I2[i]*(1-t) + I3[i]*t;
		//calculate x5M 
		//y -y1= m(x -x1) there for x = ((y-y1)/m) + x1 | m = x2-x1/y2-y1
		m = (float)(y3-y2)/(float)(x3-x2);
		x5 = ((yQ-y2)/m) + x2;
		// now interpolate for each pixel between x4 and x5 on scan line yQ
		if (x4 < x5)
			interpolateScanLineGouraud(x4, x5, yQ, I4, I5);
		else
			interpolateScanLineGouraud(x5, x4, yQ, I5, I4);
	}
	// for each scan line from y1 to y3-1
	for (yQ = y1; yQ < y3; ++yQ)
	{
				// interpolate to set x4 and I4 (code to be written)
		//calculate t
		t = (float)(yQ-y1)/(float)(y3-y1);
		//calculate I4
		for(int i = 0; i<3;i++)
			I4[i] = I1[i]*(1-t) + I3[i]*t;
		//calculate x4 in terms of y
		//y -y1= m(x -x1) there for x = ((y-y1)/m) + x1 | m = x2-x1/y2-y1
		float m = (float)(y3-y1)/(float)(x3-x1);
		//
		x4 = ((yQ-y1)/m) + x1;
		// interpolate to set x5 and I5 (code to be written)
		t = (float)(yQ-y2)/(float)(y3-y2);
		//calculate I5
		for(int i = 0; i<3;i++)
			I5[i] = I2[i]*(1-t) + I3[i]*t;
		//calculate x5
		//y -y1= m(x -x1) there for x = ((y-y1)/m) + x1 | m = x2-x1/y2-y1
		m = (float)(y3-y2)/(float)(x3-x2);
		x5 = ((yQ-y2)/m) + x2;
		// now interpolate for each pixel between x4 and x5 on scan line yQ
		if (x4 < x5)
			interpolateScanLineGouraud(x4, x5, yQ, I4, I5);
		else
			interpolateScanLineGouraud(x5, x4, yQ, I5, I4);
	}
}
//---------------------------------------------------------------------------

// Interpolate to set intensity for, and plot, each pixel between xleft and xright
// on scan line yQ
void interpolateScanLineGouraud(int xleft, int xright, int yQ, float Ileft[3], float Iright[3])
{
	void drawPixelRGB (float, float, const float [3]);
	float s;
	for (int xQ = xleft; xQ < xright; ++xQ)
	{
		float IQ[3];
		// more code to be written
		s = (float)(xQ - xleft)/(float)( xright - xleft);
		for(int i = 0; i<3;i++)
			IQ[i] = (float)Ileft[i]*(1.0-s) + (float)Iright[i]*s;
		
		drawPixelRGB (xQ, yQ, IQ);
	}
}

//---------------------------------------------------------------------------

void doInterpolationPhong(const int pixelX[], const int pixelY[], 
		const float normal[3][3], const float view[3][3], const float light[3][3])
{
	// to be written
	void interpolateScanLinePhong (int xleft, int xright, int yQ, float Nleft[3], 
	   float Nright[3], float Vleft[3], float Vright[3], float Lleft[3], float Lright[3]);

	const int x2 = pixelX[0];
	const int y2 = pixelY[0];
	const int x1 = pixelX[1];
	const int y1 = pixelY[1];
	const int x3 = pixelX[2];
	const int y3 = pixelY[2];
	int yQ, x4, x5;
	float NL[3],NR[3],VL[3],VR[3],LL[3],LR[3];
	float N1[4],  N2[4],N3[4];
	/*for(int i = 0;i<3;i++)
	cout<<"normal["<<i<<"]: "<<normal[i][0]<<endl;
	for(int i = 0;i<3;i++)
	cout<<"normal["<<i<<"]: "<<normal[i][1]<<endl;
	for(int i = 0;i<3;i++)
	cout<<"normal["<<i<<"]: "<<normal[i][2]<<endl;*/
	copyVec(normal[1],N1);
	copyVec(normal[0],N2);
	copyVec(normal[2],N3);
	float V1[3],  V2[3],V3[3];
	copyVec(view[1],V1);
	copyVec(view[0],V2);
	copyVec(view[2],V3);
	float L1[3],  L2[3],L3[3];
	copyVec(light[1],L1);
	copyVec(light[0],L2);
	copyVec(light[2],L3);
	/*for(int i = 0;i<3;i++)
	cout<<"normal["<<i<<"]: "<<normal[i][0]<<endl;
	for(int i = 0;i<3;i++)
	cout<<"normal["<<i<<"]: "<<normal[i][1]<<endl;
	for(int i = 0;i<3;i++)
	cout<<"normal["<<i<<"]: "<<normal[i][2]<<endl;*/

	// for each scan line from y2 to y1-1
	float t;
	
	for (yQ = y2; yQ < y1; ++yQ)
	{
		t = (float)(yQ - y2)/(float)(y1-y2);
		for(int i = 0;i<3;i++)
		{
			NL[i] = (N2[i])*(1.0-t) + t*N1[i];
		}
		normalise(NL);
		for(int i = 0;i<3;i++)
		{
			VL[i] = (V2[i])*(1.0-t) + t*V1[i];
		}
		normalise(VL);
		for(int i = 0;i<3;i++)
		{
			LL[i] = (L2[i])*(1.0-t) + t*L1[i];
		}
		normalise(LL);
	

		t = (float)(yQ - y2)/(float)(y3-y2);
		for(int i = 0;i<3;i++)
		{
			NR[i] = (N2[i])*(1.0-t) + t*N3[i];
		}
		normalise(NR);
		for(int i = 0;i<3;i++)
		{
			VR[i] = (V2[i])*(1.0-t) + t*V3[i];
		}
		normalise(VR);
		for(int i = 0;i<3;i++)
		{
			LR[i] = (L2[i])*(1.0-t) + t*L3[i];
		}
		normalise(LR);
		//assert(t>0);
		//calculate x4 in terms of y
		//y -y1= m(x -x1) there for x = ((y-y1)/m) + x1 | m = x2-x1/y2-y1
		float m = (float)(y2-y1)/(float)(x2-x1);
		//
		x4 = ((yQ-y1)/m) + x1;
		
		//calculate x5
		//y -y1= m(x -x1) there for x = ((y-y1)/m) + x1 | m = x2-x1/y2-y1
		m = (float)(y3-y2)/(float)(x3-x2);
		x5 = ((yQ-y2)/m) + x2;
		// now interpolate for each pixel between x4 and x5 on scan line yQ
 		if (x4 < x5)
			interpolateScanLinePhong(x4, x5, yQ, NL,NR,VL,VR,LL,LR);
		else
			interpolateScanLinePhong(x5, x4, yQ, NR,NL,VR,VL,LR,LL);
	}
	
	
	// for each scan line from y1 to y3-1
	for (yQ = y1; yQ < y3; ++yQ)
	{
		t = (float)(yQ - y1)/(float)(y3-y1);
		for(int i = 0;i<3;++i)
		{
			NL[i] = (N1[i])*(1-t) + t*N3[i];
		}
		normalise(NL);
		for(int i = 0;i<3;++i)
		{
			VL[i] = (V1[i])*(1-t) + t*V3[i];
		}
		normalise(VL);
		for(int i = 0;i<3;++i)
		{
			LL[i] = (L1[i])*(1-t) + t*L3[i];
		}
		normalise(LL);
	

		t = (float)(yQ - y2)/(float)(y3-y2);
		for(int i = 0;i<3;++i)
		{
			NR[i] = (N2[i])*(1-t) + t*N3[i];
		}
		normalise(NR);
		for(int i = 0;i<3;++i)
		{
			VR[i] = (V2[i])*(1-t) + t*V3[i];
		}
		normalise(VR);
		for(int i = 0;i<3;++i)
		{
			LR[i] = (L2[i])*(1-t) + t*L3[i];
		}
		normalise(LR);
				// interpolate to set x4 and I4 (code to be written)

		//calculate x4 in terms of y
		//y -y1= m(x -x1) there for x = ((y-y1)/m) + x1 | m = x2-x1/y2-y1
		float m = (float)(y3-y1)/(float)(x3-x1);
		//
		x4 = ((yQ-y1)/m) + x1;
		// interpolate to set x5 and I5 (code to be written)

		//calculate x5
		//y -y1= m(x -x1) there for x = ((y-y1)/m) + x1 | m = x2-x1/y2-y1
		m = (float)(y3-y2)/(float)(x3-x2);
		x5 = ((yQ-y2)/m) + x2;
		// now interpolate for each pixel between x4 and x5 on scan line yQ
		if (x4 < x5)
			interpolateScanLinePhong(x4, x5, yQ, NL,NR,VL,VR,LL,LR);
		else
			interpolateScanLinePhong(x5, x4, yQ, NR,NL,VR,VL,LR,LL);
	}
	
}
//---------------------------------------------------------------------------

// Interpolate to set intensity for, and plot, each pixel between xleft and xright
// on scan line yQ
void interpolateScanLinePhong (int xleft, int xright, int yQ, float Nleft[3], 
	   float Nright[3], float Vleft[3], float Vright[3], float Lleft[3], float Lright[3])
{
	void drawPixelRGB (float startX, float startY, const float colour[3]);
	void setColourRGB(const float normal[3], const float viewVec[3], const float lightVec[3], float col[3]);
	// to be written
	float N[3], V[3], L[3];
	float t(0);
	for (int xQ = xleft; xQ < xright; ++xQ)
	{
		// more code to be written
		t = (float)(xQ-xleft)/(float)(xright-xleft);
		for(int i = 0;i <3;i++)
		{
			N[i] = (Nleft[i]*(1.0 - t) + Nright[i]*t);
		}
		normalise(N);
		for(int i = 0; i<3;i++)
		{
			V[i] = (Vleft[i]*(1.0 - t) + Vright[i]*t);
		}
		normalise(V);
		for(int i = 0; i<3;i++)
		{
			L[i] = (Lleft[i]*(1.0 - t) + Lright[i]*t);
		}
		normalise(L);
		float colour[3];
		setColourRGB(N,V,L,colour);
		drawPixelRGB (xQ, yQ,colour);
	}
}

//---------------------------------------------------------------------------


// Draw pixel at position start, 
// with RGB colour components in col
void drawPixelRGB (float startX, float startY, const float colour[3])
{
	float localColour[3];
	for(int i = 0;i<3;i++)
	{
		if (colour[i] > 1)
			localColour[i] = 1;
		else
			localColour[i] = colour[i];
	}
	glColor3f (localColour[0], localColour[1], localColour[2]);
	glDisable(GL_BLEND);
	glBegin(GL_POINTS);
		glVertex2i(startX, startY);
	glEnd();
}

// Draw pixel at position start, 
// with RGB colour components in col
void drawPixel (float startX, float startY, const float col)
{
	float localColour;
	if (col > 1)
		localColour = 1;
	else
		localColour = col;
	glColor3f (localColour, localColour, localColour);
	glDisable(GL_BLEND);
	glBegin(GL_POINTS);
		glVertex2i(startX, startY);
	glEnd();
}
//---------------------------------------------------------------------------
// Fill polygon
void fillPolygonPix 
	(int vertexCount, const float verticesX[], const float verticesY[])
{
	glBegin(GL_POLYGON);
	for (int i(0); i < vertexCount; ++i)
		glVertex2f(verticesX[i], verticesY[i]);
	glEnd();
}

// ********************************************************************
//         General-purpose vector functions
// ********************************************************************

// Scalar product.  
void scalarProduct(const float first[3], const float second[3], float& scalProd)
{
    //a.b = ax*bx + ay*by + az*bz
  scalProd = first[0]*second[0] 
		   + first[1]*second[1]
		   + first[2]*second[2];
}
//---------------------------------------------------------------------------
// Normalise vector.
void normalise(float vec[3])
{
	float length = sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]* vec[2]);
	vec[0] /= length;
	vec[1] /= length;
	vec[2] /= length;
}
//---------------------------------------------------------------------------
// Add vectors
void addVec(const float first[3], const float second[3], float sum[3])
{
  for (int i = 0; i < 3; i++)
    sum[i] = first[i] + second[i];
}
//---------------------------------------------------------------------------
// Subtract vectors
void subtractVec(const float first[3], const float second[3], float diff[3])
{
	// code needed here
	  for (int i = 0; i < 3; i++)
    diff[i] = first[i] - second[i];
}
//---------------------------------------------------------------------------
// Subtract coordinates of points (first-second) to give vector
void subtractPoints(const float first[4], const float second[4], float diff[3])
{
  for (int i = 0; i < 3; i++)
    diff[i] = first[i] - second[i]; // not the correct value
}
//---------------------------------------------------------------------------
// extract single column from Matrix to Vector
void extractColumn (const float matrix[4][3], int columnNo, float column[4])
{
  for (int i = 0; i < 4; i++)
    column[i] = matrix[i][columnNo];
}

//extract column second function
void extractColumn3(const float matrix[4][3], int columnNo, float column[4])
{
	for (int i = 0; i < 3; i++)
    column[i] = matrix[i][columnNo];
}
//---------------------------------------------------------------------------
// Copy vector.
void copyVec(const float source[3], float result[3])
{
    for (int i = 0; i < 3; ++i)
    	result[i] = source[i];
}
//---------------------------------------------------------------------------
// Multiply vector by scalar.
void scaleVec(const float source[3], float scalar, float result[3])
{
    for (int i = 0; i < 3; ++i)
    	result[i] = source[i]*scalar;
}

// reflected vector
void reflectVector( const float Light[3],const float Normal[3], float Result[3] )
{
	//R = 2(L.N)N - L
	float LdotN;
		float unitNormal[3];
	float unitLight[3];
	copyVec(Normal,unitNormal);
	normalise(unitNormal);
	copyVec(Light,unitLight);
	normalise(unitLight);
	scalarProduct(unitLight,unitNormal, LdotN);
	//multiply the dot product by 2
	LdotN *= 2;

	//create a new vector and make it equal to the first part of the formula
	float N[3];
	scaleVec(Normal,LdotN,N);
	//subtract L from N and then normalise the result
	subtractVec(N,Light,Result);
	normalise(Result);
	//finished
}
