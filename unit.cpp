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

Cells mycell;
zVector v1(0, 0, 0);
zTransform TM1;
zTransform TM2;
int mark =4;

void computMatrix()
{
	TM1.setIdentity();

	zVector u, v, w;
	u = zVector (1, 0, 0);
	v = u; swap(v.x, v.y); v.y *= -1;
	w = zVector(0, 0, 1);
	u.normalize(); v.normalize(); w.normalize();
	zVector c=v1;

	TM2.col(0) << u.x, u.y, u.z, 1;
	TM2.col(1) << v.x, v.y, v.z, 1;
	TM2.col(2) << w.x, w.y, w.z, 1;
	TM2.col(3) << c.x, c.y, c.z, 1;

	//v *= 0.5;
	TM1.col(0) << u.x, u.y, u.z, 1;
	TM1.col(1) << v.x, v.y, v.z, 1;
	TM1.col(2) << w.x, w.y, w.z, 1;
	TM1.col(3) << c.x, c.y, c.z, 1;
}
vector<zVector> grid;
void setgrid()
{
	for (int i = -30; i < 30; i++)
	{
		for (int j = -30; j < 30; j++)
		{
			zVector GP(i , j , 0);
			GP = GP * TM2;
			if (!(mycell.boundary.outsidePolygon(GP)))
			{
				for(int w = 0; w < 1; w++)
				{
					zVector GP2(GP.x, GP.y, w);
					grid.push_back(GP2);
				}
			}
		}
	}
}

vector<Particle> unit;
int npar=0;

int remaptype(float n)
{
	int m;
	if (n <= 4) m = 0;
	else if (n <= 6) m = 1;
	else if (n <= 8) m = 2;
	else if (n <= 10)  m = 3;
	return m;
}

void setunit()
{
	for (auto& pt : grid)
	{
		int ty = remaptype(ofRandom(0, 10));
		Particle pa(pt.x, pt.y, pt.z, ty);
		pa.index = npar;
		npar++;
		unit.push_back(pa);
	}
}

void getscore(vector<Particle>& UN)
{
	for (auto& pt1 : unit)
	{
		pt1.score = 0;
		for (auto& pt2 : UN)
		{
			pt1.score+=pt1.computescore(pt2);
		}
		cout << pt1.score << endl;
	}
}
void moveonce(vector<Particle>& UN)
{
	for (auto& pt1 : UN)
	{
		if (pt1.type != 0 && pt1.score < mark)
		{
			float max = 0;

			int maxid;
			for (int i = 0; i <= UN.size(); i++)
			{
				float v = ofRandom(1, 20);
				if (UN[i].type == 0 && v >= max)
				{
					maxid = i;
					max = v;
				}
			}
			pt1.switchunit(UN[maxid]);
		}
	}
}


void setup() // events // eventHandles
{
	setCamera(10,-30,-30,0,0);
	//setCamera(25, -30, -30, 0, 0);
	setCamera(10, 0, 0, 0, 0);
	computMatrix();

	mycell.cell_center = v1;
	mycell.cell_direction = fieldinfluence(mycell.cell_center);
	mycell.setDefaultBox(30);
	for (auto& pt : mycell.boundary.vertex)
	{
		//pt = pt* TM1;
	}
	mycell.cell_id = nump;
	nump++;
	for(auto& pt : mycell.boundary.boolExpand)
	{
		pt=false;
	}

	grid.clear();
	unit.clear();
	setgrid();
	setunit();
	
}

bool compute = false;


void update(int value) // an event that run 100 times a second, automatically, without any user input
{
	if (compute)
	{
		getscore(unit);
		moveonce(unit);
	}
}

void draw() // an event that run 100 times a second, automatically, without any user input
{
	backGround(1);
	//drawGrid(50);

	mycell.drawCell();
	for (auto& pt : unit)
	{
		pt.drawbox();
	}
}

void keyPress(unsigned char k, int xm, int ym) // events
{
	if (k == 'r') setup();
	if (k == ' ') compute = !compute;
	if (k == 's')
	{
		getscore(unit);
		moveonce(unit);
	}
	if (k == 'm')
	{
		
	}

	if (k == 'd')
	{
		
	}
}

void mousePress(int b, int state, int x, int y)
{

}

void mouseMotion(int x, int y)
{

}
#endif // _MAIN_