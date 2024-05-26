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
	return distance_line(p, a, b);
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

zVector fieldinfluence(zVector center)
{
	zVector dir;
	zVector attractor_1(0, 0, 0);
	zVector attractor_2(0, 0, 0);
	zVector vec_1;
	zVector vec_2;
	vec_1 = attractor_1-center;
	vec_2 = attractor_2-center;
	swap(vec_1.x, vec_1.y); vec_1.x *= -1.0;
	dir = vec_1 + vec_2;
	dir.normalize();
	return dir;
}

zVector remapvalue(zVector n,double max,double nmax=1)
{
	zVector s;
	double t = n.length();
	n.normalize();
	s = n * t * ( nmax  / max ) ;
	return s;
}

class polygon
{
	double radius;
public:
	int vertexCount;
	vector<zVector> vertex;
	vector<array<int, 2>> edge;
	vector<bool> grow;
	vector<zVector> normals;
	double area = computerArea();

	polygon(int n,double r)
	{
		vertexCount = n;
		radius = r;
	}

	void setdefultpolygon()
	{
		float inc = TWO_PI / float(vertexCount);
		
		for (int i = 0; i < vertexCount; i++)
		{
			float x = radius * sin(float(i) * inc);
			float y = radius * cos(float(i) * inc);

			zVector pt(x, y, 0);
			array<int, 2> index = { i,(i + 1) % vertexCount };
			vertex.push_back(pt);
			edge.push_back(index);
		}
	}
	
	double computerArea()
	{
		zVector center;
		zVector ori(0, 0, 0);
		double AR;
		for (int i = 0; i < vertexCount; i++)
		{
			center += vertex[i];
		}
		center = center / vertexCount;

		for (int i = 0; i < vertexCount; i++)
		{
			zVector v1 = vertex[(i + 1) % vertexCount] - vertex[i], v2 = center - vertex[i];
			AR += ori.distanceTo(v1 ^ v2) / 2.0;
		}
		return AR;
	}

	void draw()
	{
		for (auto& v : vertex)
		{
			glColor3ub(230, 57, 70);
			glPointSize(5);
			drawPoint(ZtoA(v));
		}
		for (auto& e : edge)
		{
			drawLine(ZtoA(vertex[e[0]]), ZtoA(vertex[e[1]]));
		}
	}
};
