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
#define nPoly 50
//nPoly=4*n+2
int nump = 0;
double width = 0.5;
double divisionsize = 240;

class Cells
{
public:
	polygon boundary = polygon(nPoly);
	zVector cell_direction;
	zVector cell_center;
	zTransform vertexTM;
	zTransform normalTM;
	vector<zVector> gridPoint;
	vector<Particle> unit;
	int cell_id;
	bool boolMove = true;
	bool boolGrow;
	bool boolGridset;
	int density;


	int fit(int i)
	{
		int s = i % nPoly;
		return s;
	}

	void computTMatrix()
	{
		vertexTM.setIdentity();
		normalTM.setIdentity();

		zVector u, v, w, t;
		u = cell_direction;
		v = u; swap(v.x, v.y); v.y *= -1;
		w = zVector(0, 0, 1);
		u.normalize(); v.normalize(); w.normalize();
		zVector c = cell_center;
		t = v * 0.5;
		//assign the values to the matrix
		vertexTM.col(0) << u.x, u.y, u.z, 1;
		vertexTM.col(1) << v.x, v.y, v.z, 1;
		vertexTM.col(2) << w.x, w.y, w.z, 1;
		vertexTM.col(3) << c.x, c.y, c.z, 1;

		normalTM.col(0) << u.x, u.y, u.z, 1;
		normalTM.col(1) << t.x, t.y, t.z, 1;
		normalTM.col(2) << w.x, w.y, w.z, 1;
		normalTM.col(3) << 0, 0, 0, 1;
	}

	void setDefaultBox(double r)
	{
		computTMatrix();
		boundary.setdefultpolygon(r);
		boundary.computeNormals();

		for (auto& pt : boundary.vertex)
		{
			pt = pt * vertexTM;
		}
		for (auto& pt : boundary.normal)
		{
			pt = pt * normalTM;
		}
	}

	void setDefaultGrid()
	{
		int ngrid = 20;
		boolGrow = false;
		for (auto& boolean : boundary.boolExpand)
		{
			if (boolean == true) boolGrow = true;
		}
		if (boolGrow == false )
		{
			setDensity();
			cell_center = computecenter(boundary.vertex);
			computTMatrix();
			gridPoint.clear();

			float size;
			float height;
			if (density == 1) { size = 4; height = 11; }
			else if (density == 2) { size = 2; height = 6; }
			else if (density == 3) { size = 1; height = 3; }

			for (int i = -ngrid; i < ngrid; i++)
			{
				for (int j = -ngrid; j < ngrid; j++)
				{
					zVector gridpt(i* size,j* size, 0);
					gridpt = gridpt * vertexTM;
					if (!(boundary.outsidePolygon(boundary, gridpt))) gridPoint.push_back(gridpt);
				}
			}
			boolGridset = true; 
		}
	}

	void setDensity()
	{
		boundary.computeArea();
		if (boundary.area <= 108) density = 1;
		else if (boundary.area <= 200) density = 2;
		else density = 3;
	}

	void assignment()
	{
		if (boolGridset == true)
		{
			unit.clear();
			for (auto& pt : gridPoint)
			{
				float height;
				if (density == 1) height = 11;
				else if (density == 2) height = 6;
				else if (density == 3) height = 3;
				float d = boundary.polygondistance(pt);
				if (d<=4)
				{
					int ty = ofRandom(1.5, 3.5);
					unit.push_back(Particle(pt.x, pt.y, height, ty));
				}
				else unit.push_back(Particle(pt.x, pt.y, 0, 4));
			}
			setDensity();
		}
	}
	
	void move()
	{
		double movespeed = 1;
		for (int i = 0; i < boundary.vertexCount; i++)
		{
			if (boundary.boolExpand[i] == false)  boolMove = false; 
		}

		if (boolMove == true)
		{
			cell_direction.normalize();
			cell_center += cell_direction * movespeed;
			for (int i = 0; i < boundary.vertexCount; i++)
			{
				boundary.vertex[i] += cell_direction * movespeed;
			}
			cell_direction = fieldinfluence(cell_center);
		}
	}

	void growth(vector<Cells>& Ce)
	{
		for (int i = 0; i < boundary.vertexCount; i++)
		{
			for (auto& parcel : Ce)
			{
				if (parcel.cell_id != cell_id)
				{
					float dist;
					dist = parcel.boundary.polygondistance(boundary.vertex[i]);
					if (dist < 2.0) boundary.boolExpand[i] = false;
				}
			}

			if (boundary.boolExpand[i] == true)
			{
				double growspeed;
				boundary.computeArea();
				if (boolMove == true) growspeed = 0.1;
				else if (boundary.area < divisionsize) growspeed = 0.5;
				else growspeed = 0.3;
				boundary.vertex[i] += boundary.normal[i] * growspeed;
			}

			if (boundary.boolExpand[i] == false)
			{
				zVector smooth = ((boundary.vertex[boundary.prev(i)] + boundary.vertex[boundary.next(i)]) * 0.5 - boundary.vertex[i]);
				boundary.vertex[i] += smooth * 0.05;
			}
		}
	}

	void division(vector<Cells>& Ce)
	{
		boundary.computeArea();

		if (boundary.area >= divisionsize)
		{
			boundary.computeaxis(cell_center);
			int n1 = boundary.d_id[0];
			int n2 = boundary.d_id[1];
			vector<zVector> pt1;
			vector<zVector> pt2;
			double mindist1 = 100000;
			double mindist2 = 100000;
			Cells newcell;

			for (int i = 0; i < nPoly / 2; i++)
			{
				pt1.push_back(boundary.vertex[fit(n2 + i)]);
				pt2.push_back(boundary.vertex[fit(n1 + i)]);
			}

			cell_center = computecenter(pt1);
			cell_direction = fieldinfluence(cell_center);
			newcell.cell_center = computecenter(pt2);
			newcell.cell_direction = fieldinfluence(newcell.cell_center);

			for (auto& pt : pt1)
			{
				if (pt.distanceTo(cell_center) <= mindist1) mindist1 = pt.distanceTo(cell_center);
			}
			for (auto& pt : pt2)
			{
				if (pt.distanceTo(newcell.cell_direction) <= mindist2) mindist2 = pt.distanceTo(newcell.cell_direction);
			}
			setDefaultBox(mindist1);
			newcell.setDefaultBox(mindist2);
			newcell.cell_id = nump;
			newcell.boundary.id = newcell.cell_id;
			nump++;

			Ce.push_back(newcell);
		}
	}

	void drawCell()
	{
		glPointSize(5);
		glColor3ub(230, 57, 70);
		drawPoint(ZtoA(cell_center));
		glLineWidth(1);
		drawLine(ZtoA(cell_center), ZtoA(cell_center + cell_direction * 1));

		for (int i = 0; i < boundary.vertexCount; i++)
		{
			(boundary.boolExpand[i]) ? glColor3ub(10, 10, 10) : glColor3ub(230, 57, 70);
			glPointSize(5);
			drawPoint(ZtoA(boundary.vertex[i]));
			glLineWidth(1);
			drawLine(ZtoA(boundary.vertex[i]), ZtoA(boundary.vertex[boundary.next(i)]));

		}
		
		for (auto& ut : unit)
		{
			Alice::vec pt1(ut.x, ut.y, ut.z);
			Alice::vec pt2(ut.x, ut.y,0);
			//zVector v = fieldinfluence(pt1);
			//float l = v.length();
			ut.drawparticle();
			glLineWidth(5);
			drawLine(pt1, pt2);
		}
	}
};


//make an instance of the class Cells;

vector<Cells> cellgather;
zObjMesh oMesh;
zModel model;

void setup() // events // eventHandles
{
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
		AB.setDefaultBox(ofRandom(2, 4));
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
			element.assignment();
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

//int np = 0;
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
		for (int i = 0; i < nPoly; i++)
		{
			for (auto& element : cellgather)
			{
				element.boundary.boolExpand[i] = true;
			}
		}
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