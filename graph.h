#pragma once

#include <headers/zApp/include/zObjects.h>
#include <headers/zApp/include/zFnSets.h>
#include <headers/zApp/include/zViewer.h>

using namespace zSpace;

Alice::vec ZtoA(zVector& in)
{
	return Alice::vec(in.x, in.y, in.z);
}

zVector AtoZ(Alice::vec& in)
{
	return zVector(in.x, in.y, in.z);
}

double distance_line(zVector p, zVector a, zVector b)
{
	zVector ori(0, 0, 0);
	zVector v1 = b - a, v2 = p - a;
	return ori.distanceTo(v1 ^ v2) / a.distanceTo(b);
}

double distance_segment(zVector p, zVector a, zVector b)
{
	zVector v1 = b - a, v2 = p - a, v3 = p - b;
	//return p.distanceTo(a);
	if (a == b) { return p.distanceTo(a); }
	if ((v1 * v2) <= 0) { return p.distanceTo(a); }
	if ((v1 * v3) >= 0) { return p.distanceTo(b); }
	else return distance_line(p, a, b);
}

double area(zVector p, zVector a, zVector b)
{
	zVector ori(0, 0, 0);
	zVector v1 = b - a, v2 = p - a;
	return ori.distanceTo(v1 ^ v2) /2.0;
}



zVector offset(zVector p, zVector a , zVector b, zVector center,double r)
{
	zVector ori(0, 0, 0);
	zVector pi;
	zVector v1 = p - a, v2 = p - b;
	v1.normalize(); v2.normalize();
	zVector v3 = v1 + v2;
	v3.normalize();
	double d = r*(ori.distanceTo(v1)* ori.distanceTo(v3))/ ori.distanceTo(v1 ^ v3);
	pi = p + v3 * d;
	if (center.distanceTo(p) <= center.distanceTo(pi)) pi = p - v3 * d;
	return pi;
}

zVector remapvalue(zVector n, double max, double nmax = 1)
{
	zVector s;
	double t = n.length();
	n.normalize();
	s = n * t * (nmax / max);
	return s;
}

zVector fieldinfluence(zVector center)
{
	zVector dir;
	zVector attractor_1(-40, -40, 0);
	zVector attractor_2(33, 45, 0);
	zVector vec_1;
	zVector vec_2;

	vec_1 = attractor_1 - center;
	vec_2 = attractor_2 - center;
	float dis_1 = vec_1.length();
	float dis_2 = vec_2.length();

	//swap(vec_1.x, vec_1.y); vec_1.x *= -1.0;
	//swap(vec_2.x, vec_2.y); vec_2.x *= -1.0;
	vec_1.normalize();
	vec_2.normalize();
	vec_1 = vec_1 / sqrt(dis_1);
	vec_2 = vec_2 / sqrt(dis_2);
	dir = (vec_1 + vec_2)*5;
	//dir.normalize();
	//dir = dir*5;
	return dir;
}

#define nPoly 50 //nPoly=4*n+2
int nump = 0;
double width = 0.5;
double divisionsize = 240;

class polygon
{
public:
	int vertexCount;
	//double radius;
	vector<zVector> vertex;
	vector<zVector> normal;
	vector<array<int, 2>> edge;
	vector<bool> boolExpand;
	bool normalreset=false;
	double area ;
	int d_id[2];
	int id;
	double shortaxis;

	polygon(int n)
	{
		vertexCount = n;
	}

	int next(int i)
	{
		int s = (i + 1) % vertexCount;
		return s;
	}
	int prev(int i)
	{
		int s = (vertexCount - 1 + i) % vertexCount;
		return s;
	}
	int oppo(int i)
	{
		int s = (i + vertexCount / 2) % vertexCount;
		return s;
	}

	void setdefultpolygon(double r)
	{
		float inc = TWO_PI / float(vertexCount);
		vertex.clear();
		edge.clear();
		boolExpand.clear();
		for (int i = 0; i < vertexCount; i++)
		{
			float x = r * sin(float(i) * inc);
			float y = r * cos(float(i) * inc);

			zVector pt(x, y, 0);
			array<int, 2> index = { i, (i + 1) % vertexCount };
			vertex.push_back(pt);
			edge.push_back(index);
			boolExpand.push_back(true);
		}
	}

	void computeNormals(zVector center)
	{
		normal.clear();
		double max = -1;
		for (int i = 0; i < vertexCount; i++)
		{
			zVector nor = vertex[i] - center;
			nor.normalize();
			normal.push_back(nor);
		}
	}

	zVector computecenter()
	{
		zVector cen;
		float area;
		for (int i = 0; i < vertex.size(); i++)
		{
			int j = (i + 1) % vertex.size();
			double temp;
			temp = (vertex[i].x * vertex[j].y - vertex[j].x * vertex[i].y);
			area += temp;
			cen += (vertex[i] + vertex[j]) * temp / 3.0;
		}
		if (area != 0) cen = cen / area;
		return cen;
	}


	void computeArea()
	{
		double AR;
	
		for (int i = 0; i < vertex.size(); i++)
		{
			int j = (i + 1) % vertex.size();
			double temp;
			temp = (vertex[i].x * vertex[j].y - vertex[j].x * vertex[i].y)*0.5;
			AR += temp;
		}
		AR = abs(AR);
		area=AR;
	}

	void computeaxis(zVector center)
	{
		double min = 10000000;
		for (int i = 0; i < vertexCount; i++)
		{
			if (vertex[i].distanceTo(center)  <= min)
			{
				min = vertex[i].distanceTo(center);
				d_id[0] = i;
				d_id[1] = oppo(i);
			}
		}
		shortaxis = min;
	}

	double polygondistance(zVector pt)
	{
		double min = 10000;
		for (int i = 0; i < vertexCount; i++)
		{
			double distance = distance_segment(pt, vertex[i], vertex[next(i)]);
			if (distance <= min) min = distance;
		}
		return min;
	}

	int outsidePolygon(zVector& p)
	{
		//cross points count of x
		int __count = 0;
		int N = vertexCount;
		//neighbour bound vertices
		zVector p1, p2;
		//left vertex
		p1 = vertex[0];
		//check all rays
		for (int i = 1; i <= N; ++i)
		{
			//point is an vertex
			if (p == p1) return (0);
			//right vertex
			p2 = vertex[i % N];
			//ray is outside of our interests
			if (p.y < MIN(p1.y, p2.y) || p.y > MAX(p1.y, p2.y))
			{
				//next ray left point
				p1 = p2; continue;
			}
			//ray is crossing over by the algorithm (common part of)
			if (p.y > MIN(p1.y, p2.y) && p.y < MAX(p1.y, p2.y))
			{
				//x is before of ray
				if (p.x <= MAX(p1.x, p2.x))
				{
					//overlies on a horizontal ray
					if (p1.y == p2.y && p.x >= MIN(p1.x, p2.x)) return (0);
					//ray is vertical
					if (p1.x == p2.x)
					{
						//overlies on a ray
						if (p1.x == p.x) return (0);
						//before ray
						else ++__count;
					}
					//cross point on the left side
					else
					{
						//cross point of x
						double xinters = (p.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
						//overlies on a ray
						if (fabs(p.x - xinters) < EPS) return (0);
						//before ray
						if (p.x < xinters) ++__count;
					}
				}
			}
			//special case when ray is crossing through the vertex
			else
			{
				//p crossing over p2
				if (p.y == p2.y && p.x <= p2.x)
				{
					//next vertex
					const zVector& p3 = vertex[(i + 1) % N];
					//p.y lies between p1.y & p3.y
					if (p.y >= MIN(p1.y, p3.y) && p.y <= MAX(p1.y, p3.y)) ++__count;
					else __count += 2;
				}
			}
			p1 = p2;
		}
		if (__count % 2 == 0) return(1);
		else return(0);
	}

	void draw()
	{
		for (auto& v : vertex)
		{
			glPointSize(5);
			drawPoint(ZtoA(v));
		}
		for (auto& e : edge)
		{
			drawLine(ZtoA(vertex[e[0]]), ZtoA(vertex[e[1]]));
		}
	}
};

class Particle
{
public:
	float x;
	float y;
	float z;
	int index;
	int type;
	int score=0;

	Particle(zVector v1,int t)
	{
		x = v1.x;
		y = v1.y;
		z = v1.z;
		type = t;
	}

	Particle(float _x, float _y, float _z,int t)
	{
		x = _x;
		y = _y;
		z = _z;
		type = t;
	}

	double distance(Particle B)
	{
		zVector v1(x, y, z);
		zVector v2(B.x, B.y, B.z);
		double d = v1.distanceTo(v2);
		return d;
	}

	int computescore(Particle B)
	{
		int count=0;
		if(type!=0 && index!=B.index && distance(B)<=1.5)
		{
			if(type==1)
			{
				if (B.type == 1) count = 1;
				//else if (B.type != 0) count = -1;
				//else if (B.type == 0) count = 1;
				//else if (B.type == 3) count = 1;
			}

			if (type == 2)
			{
				if (B.type == 2) count = 1;
				//else if (B.type != 0) count = -1;
				//else if (B.type == 0) count = 1;
				//else if (B.type == 3) count = 1;
			}

			if (type == 3)
			{
				if (B.type == 3) count = 1;
				//else if (B.type != 0) count = -1;
				//else if (B.type == 0) count = 1;
				//else if (B.type == 3) count = 1;
			}
		}
		return count;
	}

	void switchunit(Particle &B)
	{
		zVector v1(x, y, z);
		x = B.x; y = B.y; z = B.z;
		B.x = v1.x; B.y = v1.y; B.z = v1.z;
	}

	void drawparticle()
	{
		glPointSize(10);
		if (type == 0) return;
		else if (type == 1) glColor3ub(245, 134, 54);
		else if (type == 2) glColor3ub(157, 132, 188);
		else if (type == 3) glColor3ub(79, 130, 163);
		drawPoint(Alice::vec(x, y, z));
	}
	void drawbox()
	{
		float r = 0.35;
		::vec4 color;
		if (type == 0) return;
		else if (type == 1) color = ::vec4(245/255.0, 134 / 255.0, 54 / 255.0, 0);
		else if (type == 2) color = ::vec4(157 / 255.0, 132 / 255.0, 188 / 255.0,1);
		else if (type == 3) color = ::vec4(79 / 255.0, 130 / 255.0, 163 / 255.0,1);
		drawCube(Alice::vec(x - r, y - r, z - r), Alice::vec(x + r, y + r, z + r), Alice::vec(x, y, z), false, color);
		//drawCube(Alice::vec(x - r, y - r, z - r), Alice::vec(x + r, y + r, z + r), Alice::vec(x, y, z), true, color);
	}
};

class Cells
{
public:
	polygon boundary = polygon(nPoly);
	zVector cell_direction;
	zVector cell_center;
	//zTransform vertexTM;
	//zTransform normalTM;
	vector<zVector> gridbox;
	vector<Particle> unit;
	int cell_id;
	int density;
	bool boolMove = true;
	bool boolGrow = true;
	bool boolGridset = false;
	


	int fit(int i)
	{
		int s = i % nPoly;
		return s;
	}

	zTransform RM;
	zTransform TM;
	zTransform SM;

	void computRMatrix()
	{
		RM.setIdentity();

		zVector u, v, w;
		u = cell_direction;
		v = u; swap(v.x, v.y); v.y *= -1;
		w = zVector(0, 0, 1);
		u.normalize(); v.normalize(); w.normalize();
		zVector c = cell_center;
		//assign the values to the matrix
		RM.col(0) << u.x, u.y, u.z, 1;
		RM.col(1) << v.x, v.y, v.z, 1;
		RM.col(2) << w.x, w.y, w.z, 1;
		RM.col(3) << c.x, c.y, c.z, 1;
	}

	void computTMatrix()
	{
		TM.setIdentity();

		zVector c = cell_center;
		//assign the values to the matrix
		TM.col(0) << 1, 0, 0, 1;
		TM.col(1) << 0, 1, 0, 1;
		TM.col(2) << 0, 0, 1, 1;
		TM.col(3) << c.x, c.y, c.z, 1;
	}

	void computSMatrix(float r)
	{
		SM.setIdentity();

		zVector u;
		u = cell_direction;
		u.normalize();
		swap(u.x, u.y); u.y *= -1;

		SM.col(0) << (1+(r-1)*u.x* u.x), ((r - 1) * u.x * u.y), ((r - 1) * u.x * u.z), 1;
		SM.col(1) << ((r - 1) * u.x * u.y), (1 + (r - 1) * u.y * u.y), ((r - 1) * u.y * u.z), 1;
		SM.col(2) << ((r - 1) * u.x * u.z), ((r - 1) * u.y * u.z), (1 + (r - 1) * u.z * u.z), 1;
		SM.col(3) << 0, 0, 0, 1;
	}

	void setDefaultBox(double r)
	{
		computTMatrix();
		computSMatrix(0.5);
		boundary.setdefultpolygon(r);
		for (auto& pt : boundary.vertex)
		{
			pt = pt*SM*TM;
		}
		boundary.computeNormals(cell_center);
	}

	void setDensity()
	{
		boundary.computeArea();
		if (boundary.area <= 150) { density = 1; }
		else if (boundary.area <= 240) { density = 2; }
		else { density = 3; }
	}

	void setDefaultGrid()
	{
		int ngrid = 20;
		boolGrow = false;
		for (auto& boolean : boundary.boolExpand)
		{
			if (boolean == true) boolGrow = true;
		}
		if (boolGrow == false && boolGridset == false)
		{
			cell_center = boundary.computecenter();
			cell_direction = fieldinfluence(cell_center);
			computTMatrix();
			computRMatrix();
			setDensity();

			float size;
			float height;
			if (density == 1) { size = 4; height = 12; }
			else if (density == 2) { size = 2; height = 8; }
			else if (density == 3) { size = 1; height = 4; }

			for (int i = -ngrid; i < ngrid; i++)
			{
				for (int j = -ngrid; j < ngrid; j++)
				{
					zVector GP(i * size, j * size, height);
					GP = GP * RM;
					if (!(boundary.outsidePolygon( GP))) { gridbox.push_back(GP); }
				}
			}
			unit.clear();
			for (auto& pt : gridbox)
			{
				zVector foot(pt.x, pt.y, 0);
				float d = boundary.polygondistance(foot);
				if (d <= 4)
				{
					for (int i = 0; i <= pt.z; i++)
					{
						int ty = ofRandom(1.5, 3.5);
						unit.push_back(Particle(pt.x, pt.y, i, ty));
					}
				}
			}
			boolGridset = true;
		}
	}

	//void assignment()
	//{
	//	if (boolGridset == true)
	//	{
	//		unit.clear();
	//		for (auto& pt : gridbox)
	//		{
	//			zVector foot(pt.x, pt.y, 0);
	//			float d = boundary.polygondistance(foot);
	//			if (d <= 4)
	//			{
	//				for (int i = 0; i <= pt.z; i++)
	//				{
	//					int ty = ofRandom(1.5, 3.5);
	//					unit.push_back(Particle(pt.x, pt.y, i, ty));
	//				}
	//			}
	//			//else unit.push_back(Particle(pt.x, pt.y, 0, 0));
	//		}
	//	}
	//}

	void move()
	{
		double movespeed = 0.6;
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
		boundary.computeArea();

		for (int i = 0; i < boundary.vertexCount; i++)
		{
			if (boolMove == false && boundary.normalreset == false)
			{
				computSMatrix(0.6);
				for (auto& pt : boundary.normal)
				{
					pt = pt * SM;
				}
				boundary.normalreset = true;
			}

			for (auto& parcel : Ce)
			{
				if (parcel.cell_id != cell_id)
				{
					float cell_dist=cell_center.distanceTo(parcel.cell_center);
					if(cell_dist<50)
					{
						float dist;
						dist = parcel.boundary.polygondistance(boundary.vertex[i]);
						if (dist < 2.0) boundary.boolExpand[i] = false;
					}
				}
			}

			if (boundary.boolExpand[i] == true)
			{
				double growspeed;
				
				if (boolMove == true) growspeed = 0.4;
				else if(boundary.area<400)growspeed = 0.05;
				else growspeed = 0;
				
				boundary.vertex[i] += boundary.normal[i] * growspeed;
			}

			if (boundary.boolExpand[i] == false)
			{
				zVector smooth = ((boundary.vertex[boundary.prev(i)] + boundary.vertex[boundary.next(i)]) * 0.5 - boundary.vertex[i]);
				boundary.vertex[i] += smooth * 0.01;
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

			cell_center = boundary.computecenter();
			cell_direction = fieldinfluence(cell_center);
			newcell.cell_center = newcell.boundary.computecenter();
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

	void computerscore(vector<Cells>& Ce)
	{
		for(auto& parcel : Ce)
		{
			float cell_dist = cell_center.distanceTo(parcel.cell_center);
			if(cell_dist<=50)
			{
				for (auto& PA : unit)
				{
					int m;
					Alice::vec pt(PA.x, PA.y, PA.z);
					for (auto& PB : parcel.unit)
					{
						m +=PA.computescore(PB);
					}
					PA.score = m;
				}
			}
			
		}
	}
	void drawCell()
	{
		/*glPointSize(5);
		glColor3ub(230, 57, 70);
		drawPoint(ZtoA(cell_center));
		glLineWidth(1);*/
		//drawLine(ZtoA(cell_center), ZtoA(cell_center + cell_direction * 10));

		for (int i = 0; i < boundary.vertexCount; i++)
		{
			(boundary.boolExpand[i]) ? glColor3ub(29, 53, 87) : glColor3ub(230, 57, 70);
			glPointSize(5);
			drawPoint(ZtoA(cell_center));
			//drawPoint(ZtoA(boundary.vertex[i]));
			glLineWidth(3);
			drawLine(ZtoA(boundary.vertex[i]), ZtoA(boundary.vertex[boundary.next(i)]));

			zVector offsetpt = offset(boundary.vertex[i], boundary.vertex[boundary.prev(i)], boundary.vertex[boundary.next(i)], cell_center, 1);
			if (boundary.boolExpand[i]==true) glColor3ub(0, 0, 0);
			else if (boundary.area <= 120) glColor3ub(40, 40, 40);
			else if (boundary.area <= 240) glColor3ub(100, 100, 100);
			else  glColor3ub(200, 200, 200);
			drawPoint(ZtoA(offsetpt));
		}

		/*for (auto& ut : gridbox)
		{
			glColor3ub(200, 200, 200);
			drawPoint(ZtoA(ut));
		}*/

		//for (auto& ut : unit)
		//{
		//	//ut.drawparticle();
		//	Alice::vec pt(ut.x, ut.y, ut.z);
		//	drawString(to_string(ut.score), pt);
		//}
	}
};
