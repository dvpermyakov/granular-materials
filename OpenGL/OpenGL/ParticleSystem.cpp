#include "ParticleSystem.h"

const int amountFeedbackAttributes = 6;
const char* feedbackAttributes[6] = {
	"positionOut",
	"velocityOut",
	"colorOut",
	"lifeTimeOut",
	"sizeOut",
	"typeOut"
};

ShaderProgram ParticleSystem::updateProgram;
ShaderProgram ParticleSystem::renderProgram;
Texture ParticleSystem::texture;

void ParticleSystem::loadShaders() {
	updateProgram.create();
	updateProgram.attachShader(Shader("Resources\\Shaders\\particle_system\\update\\particles_update.vert", GL_VERTEX_SHADER));
	updateProgram.attachShader(Shader("Resources\\Shaders\\particle_system\\update\\particles_update.geom", GL_GEOMETRY_SHADER));
	updateProgram.attachShader(Shader("Resources\\Shaders\\particle_system\\update\\particles_update.frag", GL_FRAGMENT_SHADER)); 
	for (int i = 0; i < amountFeedbackAttributes; i++) {
		glTransformFeedbackVaryings(updateProgram.getObject(), amountFeedbackAttributes, feedbackAttributes, GL_INTERLEAVED_ATTRIBS);
	}
	updateProgram.linkProgram();

	renderProgram.create();
	renderProgram.attachShader(Shader("Resources\\Shaders\\particle_system\\render\\particles_render.vert", GL_VERTEX_SHADER));
	renderProgram.attachShader(Shader("Resources\\Shaders\\particle_system\\render\\particles_render.geom", GL_GEOMETRY_SHADER));
	renderProgram.attachShader(Shader("Resources\\Shaders\\particle_system\\render\\particles_render.frag", GL_FRAGMENT_SHADER));
	renderProgram.linkProgram();

	texture.loadFromPath(PARTICLE_TEXTURE_LOCATION);
	texture.setSamplerParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	texture.setSamplerParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	texture.setSamplerParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	texture.setSamplerParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

ParticleSystem::ParticleSystem(ParticleSettings* particleSettings, SystemSettings* systemSettings) : particleSettings(particleSettings), systemSettings(systemSettings), alive(true), steps(0), lastGeneration(0.0f), initTime(0.0f) {
	initParticle = new Particle{
		particleSettings->position,
		glm::vec3(0.0f),
		glm::vec3(0.0f),
		0.0f,
		0.0f,
		CREATOR
	};
	currentParticlesNumber = 1;

	glGenVertexArrays(1, &vaoRead);
	glGenBuffers(1, &vboRead);
	glGenVertexArrays(1, &vaoWrite);
	glGenBuffers(1, &vboWrite);
	glGenTransformFeedbacks(1, &transformFeedbackBuffer);
	glGenQueries(1, &query);

	
	glBindVertexArray(vaoRead);
	glBindBuffer(GL_ARRAY_BUFFER, vboRead);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * MAX_PARTICLES_IN_BUFFER, NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle), &initParticle);
	for (int i = 0; i < amountFeedbackAttributes; i++) glEnableVertexAttribArray(i);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0);  // Position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)12); // Velocity
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)24); // Color
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)36); // Lifetime
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)40); // Size
	glVertexAttribPointer(5, 1, GL_INT,   GL_FALSE, sizeof(Particle), (const GLvoid*)44); // Type

	glBindVertexArray(vaoWrite);
	glBindBuffer(GL_ARRAY_BUFFER, vboWrite);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * MAX_PARTICLES_IN_BUFFER, NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle), &initParticle);
	for (int i = 0; i < amountFeedbackAttributes; i++) glEnableVertexAttribArray(i);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0);  // Position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)12); // Velocity
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)24); // Color
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)36); // Lifetime
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)40); // Size
	glVertexAttribPointer(5, 1, GL_INT,   GL_FALSE, sizeof(Particle), (const GLvoid*)44); // Type
}

float grandf(float min, float range) {
	float random = float(rand() % (RAND_MAX + 1)) / float(RAND_MAX);
	return min + range * random;
}

void ParticleSystem::update() {
	float current = clock() / CLOCKS_PER_SEC;
	if (steps == 0) {
		initTime = current;
	}
	if (current - initTime > systemSettings->systemLifeTimeMax || steps > systemSettings->particleStepsMax) {
		alive = false;
		return;
	}

	int generateParticleAmount = 0;
	if (current - lastGeneration > systemSettings->generationInterval) {
		steps++;
		generateParticleAmount = particleSettings->generateParticleAmount;
		updateProgram.setUniform("randomSeed", glm::vec3(grandf(-10.0f, 20.0f), grandf(-10.0f, 20.0f), grandf(-10.0f, 20.0f)));
		lastGeneration = current;
	}

	updateProgram.useProgram();
	updateProgram.setUniform("settings.position", particleSettings->position);
	updateProgram.setUniform("settings.velocityMin", particleSettings->velocityMin);
	updateProgram.setUniform("settings.velocityRange", particleSettings->velocityRange);
	updateProgram.setUniform("settings.gravity", particleSettings->gravity);
	updateProgram.setUniform("settings.color", particleSettings->color);
	updateProgram.setUniform("settings.lifeTimeMin", particleSettings->lifeTimeMin);
	updateProgram.setUniform("settings.lifeTimeRange", particleSettings->lifeTimeRange);
	updateProgram.setUniform("settings.size", particleSettings->size);
	updateProgram.setUniform("settings.generateParticleAmount", generateParticleAmount);
	updateProgram.setUniform("timePassed", 1.0f);

	glEnable(GL_RASTERIZER_DISCARD);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedbackBuffer);

	glBindVertexArray(vaoRead);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vboWrite);

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
	glBeginTransformFeedback(GL_POINTS);

	glDrawArrays(GL_POINTS, 0, currentParticlesNumber);

	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	glGetQueryObjectiv(query, GL_QUERY_RESULT, &currentParticlesNumber);

	glDisable(GL_RASTERIZER_DISCARD);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

	swap(vaoRead, vaoWrite);
	swap(vboRead, vboWrite);
}

void ParticleSystem::render(glm::mat4 projectionMatrix, Camera* camera) {
	glm::vec3 direction = glm::normalize(camera->getDirection());
	glm::vec3 quad1 = glm::normalize(glm::cross(direction, camera->getUpVector()));
	glm::vec3 quad2 = glm::normalize(glm::cross(direction, quad1));

	glBindVertexArray(vaoRead);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(0);

	renderProgram.useProgram();
	renderProgram.setUniform("matrices.projection", projectionMatrix);
	renderProgram.setUniform("matrices.view", camera->getViewMatrix());
	renderProgram.setUniform("quad1", quad1);
	renderProgram.setUniform("quad2", quad2);
	renderProgram.setUniform("gSampler", 0);
	texture.bindTexture();

	glDrawArrays(GL_POINTS, 0, currentParticlesNumber);

	glDepthMask(1);
	glDisable(GL_BLEND);
}

bool ParticleSystem::isAlive() {
	return alive;
}

int ParticleSystem::getParticleNumber() {
	return currentParticlesNumber;
}