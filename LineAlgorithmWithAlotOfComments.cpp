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


/**
* The hashTable keys will depend on slopes which is doubles
* so to help the hashFunction we store the doubles/slopes in a way that it's
* easy to split up the bytes of the double and use XOR on them.
* @brief The double_hash union
*/
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


/**
* The hashTable for the algorithm will store
* a PointData as value, where the slope is used
* to create keys, and also later used to compare points slopes.
* Each bucket/slot in the hashTable will be a linked list.
* That's why we need a PointData *next pointer. And the Point
* itself to know which points creates the lines later.
* @brief The PointData struct
*/
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

/**
* The Method asumes the vector<Point>&thePoints are sorted in x and y order(Very IMPORTANT).
* This method will take in points and draw the lines that are 4 points in a straight line or more.
* Will do it by:
* Foreach point in the vector<Point>&thePoints we find the lines created from that point(PivotPoint).
* To find the lines for specific points we will use kinda the same idea as "radix sort"
* because for a line with 3 points to be created means that the slop from the started point to
* the other points has to have same slope, this is done by:
*
* Foreach point:
* 1.) calculating the slope between our PivotPoint and any other point
* 2.) Use the slope to create a hashValue(key) and insert it into
*     the corresponding bucket/slot in the hashTable
*
* After we will have all the points with same slope to pivotPoint in the same bucket
* so we loop through the hashTable and find the lines in each bucket.
* A bucket can contain more then one line so we have to handle that to.
*
* This is done for all the points in the vector and we clear the hashTable for each pivotpoint.
*
* @brief algorithm
* @param thePoints
*/
void algorithm(std::vector<Point> &thePoints)
{
	const size_t POINTS_COUNT = thePoints.size();
	PointData *points = new PointData[POINTS_COUNT];
	forward_list<LineToDraw> linesToDraw;


	//---------- Moving Points to PointData Array -------------//
	/*
	* We going to use points in the hash table to PointData and moving goes quite fast compare to what actually takes times,
	* This could of course be optimized also but we where given in the task to get in a vector and use the exisitng Point class.
	* Also is looping through arrays much faster then vector when you do it many times.
	*/
	for (size_t index = 0; index < POINTS_COUNT; ++index)
	{
		points[index].point = &thePoints[index];
	}
	//---------------------------------------------------------//


	//---------- Finding hashTable Size that is 2^n ----------//
	/*
	* To be able to use faster hashing by just XORING and use Bitwise modulus(shift)
	* we make the HashTable size a 2^n size.
	*/
	size_t biggestBitValue = 2;
	while (biggestBitValue < POINTS_COUNT)
	{
		biggestBitValue <<= 1;
	}
	const size_t TABLE_SIZE = biggestBitValue;
	//--------------------------------------------------------//




	PointData **hashTable = new PointData*[TABLE_SIZE];
	memset(hashTable, 0, TABLE_SIZE * sizeof(PointData*));

	//------------------ Finding the lines ------------------------//
	for (size_t pivotIndex = 0; pivotIndex < POINTS_COUNT; ++pivotIndex)
	{
		const Point &pivotPoint = *(points[pivotIndex].point);

		//----------- Calc Slope and insert in bucket -------------//
		/*
		* for each point calculate slope to pivotPoint and insert to corresponding bucket
		* in the HashTable(array. The buckets contains of pointers(PointData) that creates a linked list
		*
		*/
		for (size_t index = 0; index < POINTS_COUNT; ++index)
		{
			PointData &currPoint = points[index];
			currPoint.slope.value = currPoint.point->slopeTo(pivotPoint);
			const size_t hashValue = hashing(currPoint.slope, TABLE_SIZE);

			currPoint.next = hashTable[hashValue];
			hashTable[hashValue] = &currPoint;

		}
		//--------------------------------------------------------//




		//----------- Finding which buckets contains lines -------//
		/*
		* Because 2 different slopes can give same hashValue we might have
		* more then one line inside a bucket, therefore we got to pick them up out
		* one by one. If there are 2 or more Points(PointData) with same slope value
		* they will create a line. And after we check a bucket we will set it to nullptr
		* so all buckets will be nullptr and we can use the HashTable again
		*/
		for (size_t index = 0; index < TABLE_SIZE; ++index)
		{
			PointData *startOfLine = hashTable[index];
			// making sure that the hashTable filled with nullptr(0) next loopTurn
			hashTable[index] = nullptr;

			//----------- Finding Lines inside this bucket -----------//
			/*
			* 1.) Setting the first(0) point in the bucket(linkedList) as
			*     start of the line.
			* 2.) Finding points that has same slope if 3 or more create a line
			* 3.) At same time we find the points we delete them from list.
			* These 3 steps are the main steps and are made to find a line
			* the step are done over and over again until the list is empty
			*/
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
				//----------------------------------------------//

				if (pointsInLineCount >= 3)
				{
					linesToDraw.push_front(LineToDraw(startOfLine->point, endOfLine->point));
				}

				startOfLine = startOfLine->next;
			}
			//-------------------------------------------------------//
		}
		//--------------------------------------------------------//
	}
	//------------------------------------------------------------//
	delete[] points;
	delete[] hashTable;
}