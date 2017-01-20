#include "stdafx.h"  // For visual c++
#include "Point.h"


double Point::slopeTo(const Point& p) const
{
	if (x == p.x && y == p.y)
		return  -std::numeric_limits<double>::infinity();
	else if (y == p.y) // horizontal line segment
		return 0.0;
	else if (x == p.x) // vertical line segment
		return  std::numeric_limits<double>::infinity();
	else
		return (static_cast<double>(p.y) - static_cast<double>(y)) /
		(static_cast<double>(p.x) - static_cast<double>(x));
}

bool Point::operator<(const Point& other) const
{
	if (x == other.x)
		return y < other.y;
	else
		return x < other.x;
}
bool Point::operator>(const Point& other) const
{
	return other < *this;
}
ostream& operator<<(std::ostream& out, const Point& p)
{
	out << "(" << p.x << ", " << p.y << ")";
	return out;
}
