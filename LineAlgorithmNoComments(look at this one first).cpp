// Testing.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"  // For visual c++
#include <iostream>
#include <fstream>
#include "Point.h"
#include <vector>
#include <forward_list>
#include <chrono>


/*
This was my code that got fastest time in a internal competition on an algorithm in a course.
The task was:
			You get a list of points.
			With the points you should find all the straight lines
			that can be created by using atleast 3 points or more in a row.

			This had to be done by the given files:  Point.h, Point.c  and your function algorithm should take in a "vector of points"


*/

using namespace std;

void algorithm(std::vector<Point> &thePoints);

int main()
{
	// read points from file
	ifstream input;
	input.open("c:\\points_examples\\input12800.txt");  // just locate your own file in your computer^^
	vector<Point> points;
	int points_count, x, y;
	input >> points_count;
	cout << points_count << " points to check" << endl;
	for (int i = 0; i < points_count; ++i)
	{
		input >> x >> y;
		points.push_back(Point(x, y));
	}
	input.close();

	auto begin = chrono::high_resolution_clock::now();
	algorithm(points);
	auto end = chrono::high_resolution_clock::now();
	cout << "Computing line segments took "
		<< std::chrono::duration_cast<chrono::milliseconds>(end - begin).count()
		<< " milliseconds." << endl;
	return 0;
}

struct LineToDraw
{
	LineToDraw(Point *start, Point *end)
	{
		startPoint = start;
		endPoint = end;
	}

	Point *startPoint = nullptr;
	Point *endPoint = nullptr;
};

union double_hash
{
	double value; // 8 bytes
	struct
	{
		unsigned short a; // 2 bytes
		unsigned short b; // 2 bytes
		unsigned short c; // 2 bytes
		unsigned short d; // 2 bytes
	};
};

struct PointData
{
	Point *point = nullptr;
	double_hash slope;
	PointData *next = nullptr;
};

inline size_t hashing(const double_hash slope, const size_t tableSize)
{
	const unsigned short xoredSlope = slope.a^slope.b^slope.c^slope.d;
	return xoredSlope & (tableSize - 1);
}



void algorithm(std::vector<Point> &thePoints)
{
	const size_t POINTS_COUNT = thePoints.size();
	PointData *points = new PointData[POINTS_COUNT];
	forward_list<LineToDraw> linesToDraw;



	for (size_t index = 0; index < POINTS_COUNT; ++index)
	{
		points[index].point = &thePoints[index];
	}

	size_t biggestBitValue = 2;
	while (biggestBitValue < POINTS_COUNT)
	{
		biggestBitValue <<= 1;
	}
	const size_t TABLE_SIZE = biggestBitValue;


	PointData **hashTable = new PointData*[TABLE_SIZE];
	memset(hashTable, 0, TABLE_SIZE * sizeof(PointData*));

	//------------------ Finding the lines ------------------------//
	for (size_t pivotIndex = 0; pivotIndex < POINTS_COUNT; ++pivotIndex)
	{
		const Point &pivotPoint = *(points[pivotIndex].point);

		//----------- Calc Slope and insert in bucket -------------//
		for (size_t index = 0; index < POINTS_COUNT; ++index)
		{
			PointData &currPoint = points[index];
			currPoint.slope.value = currPoint.point->slopeTo(pivotPoint);
			const size_t hashValue = hashing(currPoint.slope, TABLE_SIZE);

			currPoint.next = hashTable[hashValue];
			hashTable[hashValue] = &currPoint;

		}

		//----------- Finding which buckets contains lines -------//
		for (size_t index = 0; index < TABLE_SIZE; ++index)
		{
			PointData *startOfLine = hashTable[index];
			// making sure that the hashTable filled with nullptr(0) next loopTurn
			hashTable[index] = nullptr;

			//----------- Finding Lines inside this bucket -----------//
			while (startOfLine != nullptr)
			{
				PointData *currPoint = startOfLine->next;
				PointData *previousPoint = startOfLine;
				PointData *endOfLine = NULL;
				int pointsInLineCount = 1;

				//--------- Taking out a single line -----------//
				while (currPoint != nullptr)
				{
					if (currPoint->slope.value == startOfLine->slope.value)
					{
						//removing the point from the list.
						previousPoint->next = currPoint->next;

						endOfLine = currPoint;
						++pointsInLineCount;
					}
					previousPoint = currPoint;
					currPoint = currPoint->next;
				}

				if (pointsInLineCount >= 3)
				{
					linesToDraw.push_front(LineToDraw(startOfLine->point, endOfLine->point));
				}

				startOfLine = startOfLine->next;
			}
		}
	}
	delete[] points;
	delete[] hashTable;
}