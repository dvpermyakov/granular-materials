#pragma once

#define _USE_MATH_DEFINES

#include "Buffer.h"

#include <math.h>

#include <map>
#include <vector>
#include <algorithm>

using namespace std;

struct GeometryInfo {
	vector<glm::vec3> points;
	vector<UINT> indicies;
};

GeometryInfo trianlgeFragmentation(vector<glm::vec3> positions, vector<UINT> indicies);

class Octahedron {
public:
	static int vertexCount;
	static int indexCount;
	static float radius;
	static glm::vec3 points[6];   // 6 points
	static UINT indicies[24];     // 8 faces with 3 points
};

class Cube {
public:
	static glm::vec3 points[24];
};

glm::vec3 getTangentVector(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);

const float EPSILON = pow(0.1, 5);
const int RANDOM_PRECISION = pow(10, 3);

struct TrianglePoints {
	glm::vec3 pt1;
	glm::vec3 pt2;
	glm::vec3 pt3;
};

struct Triangle {
	float a;
	float b;
	float c;
};

struct RandomPoints {
	int granuleIndex;
	glm::vec3 positions;
};

float getFloatRandom();
glm::vec3 getRandomVector();
glm::vec3 getTriangleCenter(TrianglePoints points);
float getTriangleRadius(TrianglePoints points);
vector<RandomPoints> getRandomPointsInTriangle(vector<float> radiuses, vector<float> probabilities, TrianglePoints points, glm::vec3 normal, float minLength, float maxLength, float averageRadius);
float getTriangleSquare(TrianglePoints points);
glm::vec3 getMinCoords(TrianglePoints points);
glm::vec3 getMaxCoords(TrianglePoints points);
Triangle getTriangle(TrianglePoints points);
float getHalfPerimeter(Triangle triangle);
glm::vec3 getRandomPointInTriangle(TrianglePoints points);
bool checkPointInTriangle(TrianglePoints trianglePoints, glm::vec3 point, float radius);
float getSphereVolume(float radius);
float getSphereSquare(float radius);