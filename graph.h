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

zVector computecenter(vector<zVector>& point)
{
	zVector cen;
	float area;
	for (int i=0; i < point.size(); i++)
	{
		int j = (i + 1) % point.size();
		double temp;
		temp = (point[i].x * point[j].y - point[j].x * point[i].y);
		area += temp;
		cen += (point[i]+point[j])*temp/3.0;
	}
	if(area!=0) cen = cen / area;
	return cen;
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
	zVector attractor_2(30, 50, 0);
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


class polygon
{
public:
	int vertexCount;
	//double radius;
	vector<zVector> vertex;
	vector<zVector> normal;
	vector<array<int, 2>> edge;
	vector<bool> boolExpand;
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

	void computeNormals()
	{
		normal.clear();
		double max = -1;
		for (int i = 0; i < vertexCount; i++)
		{
			normal.push_back(vertex[i]);
			if (normal[i].length() >= max) max = normal[i].length();
		}
		for (int i = 0; i < vertexCount; i++)
		{
			normal[i] = remapvalue(normal[i], max);
		}
	}

	void computeArea()
	{
		zVector ori(0, 0, 0);
		double AR;
		zVector center=computecenter(vertex);

		for (int i = 0; i < vertexCount; i++)
		{
			zVector v1 = vertex[(i + 1) % vertexCount] - vertex[i], v2 = center - vertex[i];
			AR += ori.distanceTo(v1 ^ v2) / 2.0;
		}
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

	int outsidePolygon(polygon b, zVector& p)
	{
		//cross points count of x
		int __count = 0;
		int N = vertexCount;
		//neighbour bound vertices
		zVector p1, p2;
		//left vertex
		p1 = b.vertex[0];
		//check all rays
		for (int i = 1; i <= N; ++i)
		{
			//point is an vertex
			if (p == p1) return (0);
			//right vertex
			p2 = b.vertex[i % N];
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
					const zVector& p3 = b.vertex[(i + 1) % N];
					//p.y lies between p1.y & p3.y
					if (p.y >= MIN(p1.y, p3.y) && p.y <= MAX(p1.y, p3.y))
					{
						++__count;
					}
					else
					{
						__count += 2;
					}
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
	int type;
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

	void drawparticle()
	{
		glPointSize(10);
		if (type == 1) glColor3ub(245, 134, 54);
		else if (type == 2) glColor3ub(157, 132, 188);
		else if (type == 3) glColor3ub(79, 130, 163);
		else if (type == 4) glColor3ub(0, 130, 0), glPointSize(20);
		drawPoint(Alice::vec(x, y, z));
	}
};

