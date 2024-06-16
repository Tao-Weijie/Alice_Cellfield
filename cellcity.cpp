#define _MAIN_
#ifdef _MAIN_

#include "main.h"
#include "graph.h"
//#include "alice/spatialBin.h"
//// zSpace Core Headers
#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

using namespace zSpace;

////////////////////////////////////////////////////////// 
//GLOBAL VARIABLE --> is defined outside of setup(), update(), keyPress(), mousePress() and mouseMotion();

vector<Cells> cellgather;
zObjMesh oMesh;
zModel model;

void setup() // events // eventHandles
{
	setCamera(180, -0, -0, 0, 0);
	//S.addSlider(&width, "wd");// make a slider control for the variable called width;
	//S.sliders[0].maxVal = 10;
	zFnMesh fnMesh(oMesh);
	fnMesh.from("data/1.obj", zOBJ, false);

	zColorArray clrAr;
	fnMesh.getFaceColors(clrAr);

	zPointArray faceCenters;
	fnMesh.getCenters(zFaceData, faceCenters);

	cellgather.clear();
	nump = 0;

	for (auto& P : faceCenters)
	{
		Cells AB;
		zVector gridPt = P * 10;

		AB.cell_center = gridPt;
		AB.cell_direction = fieldinfluence(gridPt);
		AB.setDefaultBox(3);
		AB.cell_id = nump;
		AB.boundary.id = AB.cell_id;
		cellgather.push_back(AB);
		nump++;
	}
}

bool compute = false;

void update(int value) // an event that run 100 times a second, automatically, without any user input
{
	if (compute)
	{
		for (auto& element : cellgather)
		{
			element.growth(cellgather);
			element.move();
			//element.division(cellgather);
			element.setDefaultGrid();
			//element.assignment();
		}
	}
}

void draw() // an event that run 100 times a second, automatically, without any user input
{
	backGround(0);
	//drawGrid(50);
	for (auto& element : cellgather) element.drawCell();

	wireFrameOn();
	model.draw();
	wireFrameOff();
}

void keyPress(unsigned char k, int xm, int ym) // events
{
	if (k == 'r') setup();
	if (k == ' ') compute = !compute;
	if (k == 's')
	{
		for (int i = 0; i < nPoly; i++)
		{
			for (auto& element : cellgather)
			{
				element.boundary.boolExpand[i] = false;
			}
		}
	}
	if (k == 'm')
	{
		cellgather[50].computerscore(cellgather);
		//for (int i = 0; i < nPoly; i++)
		//{
		//		//element.boundary.boolExpand[i] = true;
		//}
	}

	if (k == 'd')
	{
		for (auto& element : cellgather)
		{
			element.division(cellgather);
		}
	}
}

void mousePress(int b, int state, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}
#endif // _MAIN_