#pragma once

#include "ShaderProgram.h"
#include "Camera.h"
#include "Texture.h"
#include "Resources.h"

#include <GL/glew.h>
#include <gl/wglew.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <time.h>
#include <vector>
#include <algorithm>

using namespace std;

#define MAX_PARTICLES_IN_BUFFER 100000

enum ParticleType {
	CREATOR = 0,
	CREATED,
};

struct SystemSettings {
	float generationInterval;  // in seconds
	float systemLifeTimeMax;   // in seconds
	int particleStepsMax;
};

struct ParticleSettings {
	glm::vec3 position;
	glm::vec3 velocityMin;
	glm::vec3 velocityRange;
	glm::vec3 gravity;
	glm::vec3 color;
	float lifeTimeMin;       // in seconds
	float lifeTimeRange;     // in seconds
	float size;
	int generateParticleAmount;
};

struct Particle {
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 color;
	float lifeTime;
	float size;
	ParticleType type;
};

class ParticleSystem {
public:
	static void loadShaders();
	ParticleSystem(ParticleSettings* particleSettings, SystemSettings* systemSettings);
	void update();
	void render(glm::mat4 projectionMatrix, Camera* camera);
	bool isAlive();
	int getParticleNumber();
private:
	static ShaderProgram updateProgram;
	static ShaderProgram renderProgram;
	static Texture texture;

	GLuint vaoRead;
	GLuint vboRead;
	GLuint vaoWrite;
	GLuint vboWrite;
	GLuint transformFeedbackBuffer;
	GLuint query;
	ParticleSettings* particleSettings;
	SystemSettings* systemSettings;
	Particle* initParticle;

	int steps;
	int currentParticlesNumber;
	float initTime;             // in seconds
	float lastGeneration;       // in seconds
	bool alive;
};