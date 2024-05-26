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
double divisionsize = 90;

class Cells
{
public:
	//declare class variables.

	polygon boundary;

	zVector cell_direction;
	zVector cell_center;
	int cell_id;
	double cell_area;
	double cell_short;
	int  short_id[2];
	double cell_growspeed;

	zVector cell_boundary_points[nPoly];
	zVector cell_boundary_normals[nPoly];
	bool boolGrow[nPoly];
	bool boolMove;

	int next(int i)
	{
		int s = (i + 1) % nPoly;
		return s;
	}
	int prev(int i)
	{
		int s = (nPoly - 1 + i) % nPoly;
		return s;
	}
	int oppo(int i)
	{
		int s = (i + nPoly / 2) % nPoly;
		return s;
	}
	int fit(int i)
	{
		int s = i % nPoly;
		return s;
	}
	void computeboxarea()
	{
		cell_area = 0;
		for (int i = 0; i < nPoly; i++)
		{
			double ar = area(cell_center, cell_boundary_points[i], cell_boundary_points[next(i)]);
			cell_area += ar;
		}
	}
	
	void computeaxis()
	{
		double s = 1000;
		for(int i = 0; i < nPoly; i++)
		{
			if (cell_boundary_points[i].distanceTo(cell_boundary_points[oppo(i)]) <= s)
			{
				s = cell_boundary_points[i].distanceTo(cell_boundary_points[oppo(i)]);
				short_id[0] = i;
				short_id[1] = oppo(i);
			}
		}
		cell_short = s;
	}

	void computcenter()
	{
		zVector c;
		for (int i = 0; i < nPoly; i++)
		{
			c += cell_boundary_points[i];
		}
		cell_center = c / nPoly;
		cell_direction = fieldinfluence(cell_center);
	}
	void computeNormals()
	{
		double max = -1;
		for (int i = 0; i < nPoly; i++)
		{
			zVector e1 = (cell_boundary_points[prev(i)] - cell_boundary_points[i]);
			zVector e2 = (cell_boundary_points[i] - cell_boundary_points[next(i)]);
			//e1.normalize(); e2.normalize();
			if ((e1^e2).z>0) cell_boundary_normals[i] = e2-e1;
			else cell_boundary_normals[i] = e1-e2;
			//cell_boundary_normals[i].normalize();

			if (cell_boundary_normals[i].length() >= max) max = cell_boundary_normals[i].length();
		}
		for (int i = 0; i < nPoly; i++)
		{
			cell_boundary_normals[i] = remapvalue(cell_boundary_normals[i], max);
		}
	}

	void setDefaultBox()
	{
		polygon boundary(nPoly, ofRandom(2, 4));
		polygon boundary1(10, 10);
		//float inc = TWO_PI / float(nPoly);
		//float r = ofRandom(2, 4);
		////float r = 3;

		//for (int i = 0; i < nPoly; i++)
		//{
		//	float x = r * sin(float(i) * inc);
		//	float y = r * cos(float(i) * inc);

		//	cell_boundary_points[i] = zVector(x, y, 0);
		//	boolGrow[i] = true;
		//}
		//boolDivision = true;
		
		boolMove = true;
		transformBox();
	}

	void transformBox()
	{
		zTransform TM;
		TM.setIdentity();

		zVector u, v, w;
		u = cell_direction;
		v = u; swap(v.x, v.y); v.y *= -1;
		w = zVector(0, 0, 1);
		u.normalize(); v.normalize(); w.normalize();
		zVector c = cell_center;
		v *= width;

		//assign the values to the matrix
		TM.col(0) << u.x, u.y, u.z, 1;
		TM.col(1) << v.x, v.y, v.z, 1;
		TM.col(2) << w.x, w.y, w.z, 1;
		TM.col(3) << c.x, c.y, c.z, 1;

		for (int i = 0; i < nPoly; i++) cell_boundary_points[i] = cell_boundary_points[i] * TM;
		computeNormals();
	}

	void move()
	{
		double speed = 0.2;

		for(int i = 0; i < nPoly; i++)
		{
			if (boolGrow[i] == false) { boolMove = false; }
		}

		if(boolMove == true)
		{
			cell_center += cell_direction * speed;
			cell_direction = fieldinfluence(cell_center);
			for (int i = 0; i < nPoly; i++)
			{
				cell_boundary_points[i] += cell_direction * speed;
			}
		}
	}

	void growth(vector<Cells>& Ce)
	{
		zVector np;
		for (int i = 0; i < nPoly; i++)
		{
			for (auto& element : Ce)
			{
				if (element.cell_id != cell_id)
				{
					for (int j = 0; j < nPoly; j++)
					{
						float dist = distance_segment( cell_boundary_points[i],element.cell_boundary_points[j], element.cell_boundary_points[next(j)]);
						if (dist < 0.5) boolGrow[i] = false;
					}
				}
			}

			if (boolGrow[i] == true)
			{
				computeboxarea();
				if (cell_area < divisionsize)
				{
					cell_growspeed =0.5;
				}
				else cell_growspeed = 0.05;
				cell_boundary_points[i] += cell_boundary_normals[i] * cell_growspeed;
			}

			if (boolGrow[i] == false)
			{
				zVector center = (cell_boundary_points[prev(i)] + cell_boundary_points[next(i)]) * 0.5;
				zVector smooth = (center - cell_boundary_points[i]);
				cell_boundary_points[i] += smooth * 0.05;
			}
		}
	}

	void division(vector<Cells>& Ce)
	{
		for (auto& element : Ce)
		{
			computeaxis();
			if (cell_short >= 30)
			{
				int n1 = short_id[0];
				int n2 = short_id[1];
				zVector newpt1[nPoly / 2];
				zVector newpt2[nPoly / 2];
				for (int i = 0; i < nPoly / 2; i++)
				{
					newpt1[i] = cell_boundary_points[n1] + (cell_boundary_points[n2] - cell_boundary_points[n1]) * ((i + 1.0) / (nPoly / 2 + 1.0));
					//newpt2[i] = cell_boundary_points[n3] + (cell_boundary_points[n2] - cell_boundary_points[n3]) * ((i + 1.0) / (nPoly / 2 + 1.0));
				}

				zVector pt1[nPoly];
				zVector pt2[nPoly];
				for (int i = 0; i < nPoly / 2; i++)
				{
					pt1[i] = cell_boundary_points[fit(n2 + i)];
					pt1[i + (nPoly / 2)] = newpt1[i];
				}

				Cells oldcell;
				for (int i = 0; i < nPoly; i++)
				{
					oldcell.cell_boundary_points[i] = pt1[i];
					oldcell.boolGrow[i] = true;
				}
				oldcell.computcenter();
				oldcell.computeNormals();
				
				element = oldcell;
				/*Cells newcell;
				newcell.cell_center = cell_boundary_points[short_id[0]] + (cell_boundary_points[short_id[1]] - cell_boundary_points[short_id[0]]) * (3.0 / 4.0);
				newcell.cell_direction = fieldinfluence(newcell.cell_center);
				for (int i = 0; i < nPoly; i++)
				{
					newcell.cell_boundary_points[i] = pt2[i];
					newcell.boolGrow[i] = true;
				}
				newcell.computeNormals();
				newcell.cell_id = nump;
				Ce.push_back(newcell);
				nump++;*/
			}
		}
	}

	zVector norm;
	void drawCell()
	{
		glPointSize(5);
		glColor3ub(230, 57, 70);
		drawPoint(ZtoA(cell_center));
		drawLine(ZtoA(cell_center), ZtoA(cell_center + cell_direction * 1));
		computeboxarea();

		for (int i = 0; i < nPoly; i++)
		{
			(boolGrow[i]) ? glColor3ub(230, 57, 70) : glColor3ub(29,53,87);
			glPointSize(5);
			drawPoint(ZtoA(cell_boundary_points[i]));
			glLineWidth(1);
			drawLine(ZtoA(cell_boundary_points[i]), ZtoA(cell_boundary_points[next(i)]));

			//drawString(to_string(cell_id), ZtoA(cell_center));
			//drawString(to_string(i), ZtoA(cell_boundary_points[i]));

			/*if(cell_area<=50)glColor3ub(241, 250, 238);
			else if (cell_area <= divisionsize)glColor3ub(168, 218, 220);
			else glColor3ub(69, 123, 157);
			glPointSize(5);
			drawPoint(ZtoA(offset(cell_boundary_points[i], cell_boundary_points[prev(i)], cell_boundary_points[next(i)], cell_center, -0.5)));*/

			norm = cell_boundary_normals[i];
			//norm.normalize();
			//drawLine(ZtoA(cell_boundary_points[i]), ZtoA(cell_boundary_points[i] + norm * 2.0));
		}
	}
};
//make an instance of the class Cells;

vector<Cells> cellgather;
zObjMesh oMesh;
zModel model;

void setup() // events // eventHandles
{
	S.addSlider(&width, "wd");// make a slider control for the variable called width;
	//S.sliders[0].maxVal = 10;

	zFnMesh fnMesh(oMesh);
	fnMesh.from("data/2.obj", zOBJ, false);

	zColorArray clrAr;
	fnMesh.getFaceColors(clrAr);

	zPointArray faceCenters;
	fnMesh.getCenters(zFaceData, faceCenters);
	
	cellgather.clear();
	nump = 0;

	for (auto& P : faceCenters)
	{
		Cells AB;
		zVector gridPt = P * 4.5;

		AB.cell_center = gridPt;
		AB.cell_direction = fieldinfluence(gridPt);
		AB.setDefaultBox();
		AB.cell_id = nump;
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
		}
	}
}

void draw() // an event that run 100 times a second, automatically, without any user input
{
	backGround(1);
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
				element.boolGrow[i] = false;
			}
		}
	}
	if (k == 'm')
	{
		for (int i = 0; i < nPoly; i++)
		{
			for (auto& element : cellgather)
			{
				element.boolGrow[i] = true;
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