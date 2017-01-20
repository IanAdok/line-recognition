
#ifndef POINT_H
#define POINT_H

#include <iostream>
using namespace std;

static const int COORD_MAX = 32767; // max value of x and y coordinates

class Point
{
public:
	Point() = delete;
	Point(unsigned int _x, unsigned int _y) : x(_x), y(_y) {}

	double slopeTo(const Point& p) const;

	bool operator<(const Point&) const;
	bool operator>(const Point&) const;

	friend ostream& operator<<(ostream&, const Point&);


private:
	unsigned int x, y; // position
};

#endif // POINT_H