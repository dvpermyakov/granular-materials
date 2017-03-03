#include "Geometry.h"

int Octahedron::vertexCount = 6;
int Octahedron::indexCount = 24;
float Octahedron::radius = 1.0f;

glm::vec3 Octahedron::points[6] = {
	glm::vec3(1, 0, 0),
	glm::vec3(-1, 0, 0),
	glm::vec3(0, 1, 0),
	glm::vec3(0, -1, 0),
	glm::vec3(0, 0, 1),
	glm::vec3(0, 0, -1),
};

UINT Octahedron::indicies[24] {
	1, 2, 5, 0, 2, 5, 1, 3, 5, 0, 3, 5,
	1, 2, 4, 0, 2, 4, 1, 3, 4, 0, 3, 4
};

GeometryInfo trianlgeFragmentation(vector<glm::vec3> positions, vector<UINT> indicies) {
	vector<glm::vec3> points(positions.begin(), positions.end());
	vector<UINT> newIndicies;
	map<pair<UINT, UINT>, UINT> indexMap;
	UINT currentPosition = positions.size();
	for (UINT i = 0; i < indicies.size(); i += 3) {
		UINT i0, i1, i2;
		i0 = indicies[i]; i1 = indicies[i + 1]; i2 = indicies[i + 2];
		UINT p1, p2, p3;

		if (indexMap.find(pair<UINT, UINT>(i0, i1)) == indexMap.end()) {
			points.push_back((positions[i0] + positions[i1]) / 2.0f);
			p1 = currentPosition;
			currentPosition++;
			indexMap[pair<UINT, UINT>(i0, i1)] = p1;
		}
		else p1 = indexMap[pair<UINT, UINT>(i0, i1)];

		if (indexMap.find(pair<UINT, UINT>(i1, i2)) == indexMap.end()) {
			points.push_back((positions[i1] + positions[i2]) / 2.0f);
			p2 = currentPosition;
			currentPosition++;
			indexMap[pair<UINT, UINT>(i1, i2)] = p2;
		}
		else p2 = indexMap[pair<UINT, UINT>(i1, i2)];

		if (indexMap.find(pair<UINT, UINT>(i0, i2)) == indexMap.end()) {
			points.push_back((positions[i0] + positions[i2]) / 2.0f);
			p3 = currentPosition;
			currentPosition++;
			indexMap[pair<UINT, UINT>(i0, i2)] = p3;
		}
		else p3 = indexMap[pair<UINT, UINT>(i0, i2)];
		UINT currentIndicies[12] = {
			p1, p2, p3,
			i0, p1, p3,
			i1, p1, p2,
			i2, p2, p3 };
		newIndicies.insert(newIndicies.end(), currentIndicies, currentIndicies + 12);
	}
	GeometryInfo gi = { points, newIndicies };
	return gi;
}

glm::vec3 Cube::points[24] = {
	// Back face
	glm::vec3(-50.0f, 50.0f, -50.0f), glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, -50.0f), glm::vec3(50.0f, -50.0f, -50.0f),
	// Front face
	glm::vec3(-50.0f, -50.0f, 50.0f), glm::vec3(-50.0f, 50.0f, 50.0f), glm::vec3(50.0f, -50.0f, 50.0f), glm::vec3(50.0f, 50.0f, 50.0f),
	// Left face
	glm::vec3(-50.0f, 50.0f, 50.0f), glm::vec3(-50.0f, -50.0f, 50.0f), glm::vec3(-50.0f, 50.0f, -50.0f), glm::vec3(-50.0f, -50.0f, -50.0f),
	// Right face
	glm::vec3(50.0f, 50.0f, -50.0f), glm::vec3(50.0f, -50.0f, -50.0f), glm::vec3(50.0f, 50.0f, 50.0f), glm::vec3(50.0f, -50.0f, 50.0f),
	// Top face
	glm::vec3(-50.0f, 50.0f, -50.0f), glm::vec3(50.0f, 50.0f, -50.0f), glm::vec3(-50.0f, 50.0f, 50.0f), glm::vec3(50.0f, 50.0f, 50.0f),
	// Bottom face
	glm::vec3(50.0f, -50.0f, -50.0f), glm::vec3(-50.0f, -50.0f, -50.0f), glm::vec3(50.0f, -50.0f, 50.0f), glm::vec3(-50.0f, -50.0f, 50.0f),
};

glm::vec3 getTangentVector(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
	glm::vec3 edge1 = v2 - v1;
	glm::vec3 edge2 = v3 - v1;
	glm::vec2 delta1;
	glm::vec2 delta2;
	if (v1.x != 0 && v2.x != 0 && v3.x != 0) {
		v1.x = 1.0f; v1.y /= v1.x; v1.z /= v1.x;
		v2.x = 1.0f; v2.y /= v2.x; v2.z /= v2.x;
		v3.x = 1.0f; v3.y /= v3.x; v3.z /= v3.x;

		delta1 = glm::vec2(v2.y, v2.z) - glm::vec2(v1.y, v1.z);
		delta2 = glm::vec2(v3.y, v3.z) - glm::vec2(v1.y, v1.z);
	}
	else if (v1.y != 0 && v2.y != 0 && v3.y != 0){
		v1.y = 1.0f; v1.x /= v1.y; v1.z /= v1.y;
		v2.y = 1.0f; v2.x /= v2.y; v2.z /= v2.y;
		v3.y = 1.0f; v3.x /= v3.y; v3.z /= v3.y;

		delta1 = glm::vec2(v2.x, v2.z) - glm::vec2(v1.x, v1.z);
		delta2 = glm::vec2(v3.x, v3.z) - glm::vec2(v1.x, v1.z);
	}
	else {
		v1.z = 1.0f; v1.x /= v1.z; v1.y /= v1.z;
		v2.z = 1.0f; v2.x /= v2.z; v2.y /= v2.z;
		v3.z = 1.0f; v3.x /= v3.z; v3.y /= v3.z;

		delta1 = glm::vec2(v2.x, v2.y) - glm::vec2(v1.x, v1.y);
		delta2 = glm::vec2(v3.x, v3.y) - glm::vec2(v1.x, v1.y);
	}

	delta1 = glm::normalize(delta1);
	delta2 = glm::normalize(delta2);
	float factor = 1.0f / (delta1.x * delta2.y - delta2.x * delta1.y);

	glm::vec3 tangent;
	tangent.x = factor * (delta2.y * edge1.x - delta1.y * edge2.x);
	tangent.y = factor * (delta2.y * edge1.y - delta1.y * edge2.y);
	tangent.z = factor * (delta2.y * edge1.z - delta1.y * edge2.z);

	return glm::normalize(tangent);
}
float getFloatRandom() {
	return rand() % RANDOM_PRECISION / (float)RANDOM_PRECISION;
}

glm::vec3 getRandomVector() {
	return glm::vec3(
		getFloatRandom(),
		getFloatRandom(),
		getFloatRandom());
}

glm::vec3 getTriangleCenter(TrianglePoints points) {
	return glm::vec3(
		(points.pt1.x + points.pt2.x + points.pt3.x) / 3.0f,
		(points.pt1.y + points.pt2.y + points.pt3.y) / 3.0f,
		(points.pt1.z + points.pt2.z + points.pt3.z) / 3.0f);
}

float getTriangleRadius(TrianglePoints points) {
	Triangle triangle = getTriangle(points);
	float p = getHalfPerimeter(triangle);
	return glm::sqrt((p - triangle.a) * (p - triangle.b) * (p - triangle.c) / p);
}

vector<RandomPoints> getRandomPointsInTriangle(vector<float> radiuses, vector<float> probabilities, TrianglePoints points, glm::vec3 normal, float minLength, float maxLength, float averageRadius) {
	float sum = 0;
	for (int i = 0; i < probabilities.size(); i++) {
		float probability = probabilities[i];
		probabilities[i] += sum;
		sum += probability;
	}
	vector<RandomPoints> result;
	int maxFailuresInRow;
	while (result.size() < getTriangleSquare(points) * minLength * averageRadius / getSphereVolume(averageRadius)) {
		float probRand = fmod(rand(), sum);
		int granuleIndex = 0;
		for (int i = 0; i < probabilities.size(); i++) {
			if (probabilities[i] >= probRand) break;
			granuleIndex++;
		}
		glm::vec3 point = getRandomPointInTriangle(points);
		if (!checkPointInTriangle(points, point, radiuses[granuleIndex])) {
			continue;
		}
		point += normal * radiuses[granuleIndex] * (1.0f + (maxLength - 2.0f) * getFloatRandom());
		bool findCollision = false;
		for (int i = 0; i < result.size(); i++) {
			float distance = glm::length(result[i].positions - point);
			if (distance - (radiuses[granuleIndex] + radiuses[result[i].granuleIndex]) < EPSILON) findCollision = true;
		}
		if (!findCollision) {
			result.push_back({ granuleIndex, point });
			maxFailuresInRow = 0;
		}
		else {
			maxFailuresInRow++;
			if (maxFailuresInRow > 300) break;
		}
	}
	return result;
}

float getTriangleSquare(TrianglePoints points) {
	Triangle triangle = getTriangle(points); 
	float p = getHalfPerimeter(triangle);
	return glm::sqrt((p - triangle.a) * (p - triangle.b) * (p - triangle.c) * p);
}

glm::vec3 getMinCoords(TrianglePoints points) {
	return glm::vec3(std::min(points.pt1.x, min(points.pt2.x, points.pt3.x)), min(points.pt1.y, min(points.pt2.y, points.pt3.y)), min(points.pt1.z, min(points.pt2.z, points.pt3.z)));
}

glm::vec3 getMaxCoords(TrianglePoints points) {
	return glm::vec3(std::max(points.pt1.x, max(points.pt2.x, points.pt3.x)), max(points.pt1.y, max(points.pt2.y, points.pt3.y)), max(points.pt1.z, max(points.pt2.z, points.pt3.z)));
}

Triangle getTriangle(TrianglePoints points) {
	Triangle triangle;
	triangle.a = glm::length(points.pt1 - points.pt2);
	triangle.b = glm::length(points.pt2 - points.pt3);
	triangle.c = glm::length(points.pt3 - points.pt1);
	return triangle;
}

float getHalfPerimeter(Triangle triangle) {
	return (triangle.a + triangle.b + triangle.c) / 2.0f;
}

glm::vec3 getRandomPointInTriangle(TrianglePoints points) {
	float sqrtR1 = glm::sqrt(getFloatRandom());
	float r2 = getFloatRandom();
	return (1 - sqrtR1) * points.pt1 + sqrtR1 * (1 - r2) * points.pt2 + r2 * sqrtR1 * points.pt3;
}

bool checkPointInTriangle(TrianglePoints trianglePoints, glm::vec3 point, float radius) {
	float sq1 = getTriangleSquare({ point, trianglePoints.pt1, trianglePoints.pt2 });
	float sq2 = getTriangleSquare({ point, trianglePoints.pt2, trianglePoints.pt3 });
	float sq3 = getTriangleSquare({ point, trianglePoints.pt1, trianglePoints.pt3 });
	float sqt = getTriangleSquare(trianglePoints);
	bool checkIn = abs(sqt - (sq1 + sq2 + sq3)) < EPSILON;
	bool checkSq1 = abs(radius - sq1 / glm::length(trianglePoints.pt1 - trianglePoints.pt2)) > EPSILON;
	bool checkSq2 = abs(radius - sq2 / glm::length(trianglePoints.pt2 - trianglePoints.pt3)) > EPSILON;
	bool checkSq3 = abs(radius - sq3 / glm::length(trianglePoints.pt1 - trianglePoints.pt3)) > EPSILON;
	return checkIn && checkSq1 && checkSq2 && checkSq3;
}

float getSphereVolume(float radius) {
	return 4 / 3 * M_PI * pow(radius, 3);
}

float getSphereSquare(float radius) {
	return M_PI * pow(radius, 2);
}