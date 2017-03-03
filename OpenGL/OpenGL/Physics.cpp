#include "Physics.h"

glm::vec3 toVec3(const btVector3 &vector) {
	glm::vec3 vec;
	vec.x = (float)vector.getX();
	vec.y = (float)vector.getY();
	vec.z = (float)vector.getZ();
	return vec;
}

btVector3 toVec3(glm::vec3 vector) {
	btVector3 vec;
	vec.setX(vector.x);
	vec.setY(vector.y);
	vec.setZ(vector.z);
	return vec;
}

ShaderProgram DebugDraw::program;
GLControl* DebugDraw::control = NULL;
Camera* DebugDraw::camera = NULL;
GLFont* DebugDraw::font = NULL;

void DebugDraw::loadShaders() {
	program.create();
	program.attachShader(Shader("Resources\\Shaders\\debug_draw\\debug.vert", GL_VERTEX_SHADER));
	program.attachShader(Shader("Resources\\Shaders\\debug_draw\\debug.frag", GL_FRAGMENT_SHADER));
	program.linkProgram();
}

void DebugDraw::setControl(GLControl* control) {
	DebugDraw::control = control;
}

void DebugDraw::setCamera(Camera* camera) {
	DebugDraw::camera = camera;
}

void DebugDraw::setFont(GLFont* font) {
	DebugDraw::font = font;
}

DebugDraw::DebugDraw() {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	vbo.generate();
	vbo.addData(&glm::vec3(0.0f), sizeof(glm::vec3));
	vbo.addData(&glm::vec3(0.0f), sizeof(glm::vec3));
	vbo.bind();
	vbo.upload(GL_DYNAMIC_DRAW, false);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}

void DebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
	glm::vec3 data[2] = { toVec3(from), toVec3(to) };
	vbo.changeData(&data[0], 2 * sizeof(glm::vec3), 0);

	glBindVertexArray(vao);
	program.useProgram();
	program.setUniform("projectionMatrix", *this->control->getProjectionMatrix());
	program.setUniform("viewMatrix", this->camera->getViewMatrix());
	program.setUniform("color", toVec3(color));
	glDrawArrays(GL_LINES, 0, 2);
}

void DebugDraw::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {}

void DebugDraw::reportErrorWarning(const char* warningString) {
	font->print(*control->getOrthoMatrix(), warningString, glm::vec3(1, 1, 1), 10, control->getViewPortHeight() - 60);
}

void DebugDraw::draw3dText(const btVector3& location, const char* textString) {
	font->print(*control->getOrthoMatrix(), textString, glm::vec3(1, 1, 1), 10, control->getViewPortHeight() - 90);
}

int	DebugDraw::getDebugMode() const {
	return mode;
}

void DebugDraw::setDebugMode(int debugMode) {
	mode = debugMode;
}

struct FilterCallback : public btOverlapFilterCallback {
	bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const {
		bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
		collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask) != 0;

		if (proxy0->m_collisionFilterMask == COL_GRANULE || proxy1->m_collisionFilterMask == COL_GRANULE) {
			return true;
		}

		if (proxy0->m_collisionFilterMask == COL_FORCER && proxy1->m_collisionFilterMask == COL_FORCER) {
			return false;
		}
		if (proxy0->m_collisionFilterMask == COL_FORCER && proxy1->m_collisionFilterMask == COL_SHELL) {
			return false;
		}
		if (proxy0->m_collisionFilterMask == COL_SHELL && proxy1->m_collisionFilterMask == COL_FORCER) {
			return false;
		}
		if (proxy0->m_collisionFilterMask == COL_SHELL && proxy1->m_collisionFilterMask == COL_SHELL) {
			return false;
		}

		return collides;
	}
};

short PhysicEngine::FORCER_MASK = COL_FORCER;
short PhysicEngine::SHELL_MASK = COL_SHELL;

void PhysicEngine::load(glm::vec3 gravity) {
	objects = new ObjectsForRendering();

	broadphase = new btDbvtBroadphase();

	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);
	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	dynamicsWorld->setGravity(toVec3(gravity));

	debugDraw = new DebugDraw();
	debugDraw->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	dynamicsWorld->setDebugDrawer(debugDraw);
	renderDebug = false;

	btOverlapFilterCallback * filterCallback = new FilterCallback();
	dynamicsWorld->getPairCache()->setOverlapFilterCallback(filterCallback);
}

void PhysicEngine::addTerrain(Ground* ground) {
	vector<vector<glm::vec3>> vertecies(ground->getDataHeight(), vector<glm::vec3>(ground->getDataWidth()));
	for (int i = 0; i < ground->getDataHeight(); i++) {
		for (int j = 0; j < ground->getDataWidth(); j++) {
			float x = (i / float(ground->getDataHeight() - 1) - 0.5f) * ground->getScale().x;
			float y = float(ground->getRowHeight(j, i)) / 255.0f      * ground->getScale().y;
			float z = (j / float(ground->getDataWidth() - 1) - 0.5f)  * ground->getScale().z;
			vertecies[i][j] = { x, y, z };
		}
	}
	vector<vector<glm::vec3>> points;
	for (int i = 0; i < ground->getDataHeight() - 1; i++) {
		for (int j = 0; j < ground->getDataWidth() - 1; j++) {
			points.push_back(vector<glm::vec3>({ vertecies[i][j], vertecies[i][j + 1], vertecies[i + 1][j] }));
			points.push_back(vector<glm::vec3>({ vertecies[i][j + 1], vertecies[i + 1][j], vertecies[i + 1][j + 1] }));
		}
	}
	btRigidBody* body = addStaticTriangleBody(points, ground->getPosition());
	bodies.push_back({ ground, body, TERRAIN });
}
	
void PhysicEngine::removeBigGranules() {
	vector<int> indiciesToRemove;
	for (int i = 0; i < bodies.size(); i++) {
		if (bodies[i].type == BIG_GRANULE) {
			indiciesToRemove.push_back(i);
		}
	}
	while (!indiciesToRemove.empty()) {
		bodies.erase(bodies.begin() + indiciesToRemove.back());
		indiciesToRemove.pop_back();
	}
}

void PhysicEngine::addRigidBody(AbstractModel* model, float mass, RigidBodyType type) {
	if (type == TERRAIN) return;
	float radius = model->getScale().x;
	if (type == BULLET) radius *= 10;
	btCollisionShape* shape = new btSphereShape(radius);
	ModelMotionState* motionState = new ModelMotionState(model);
	btRigidBody* body = createRigidBody(mass, motionState, shape, btVector3(1, 1, 1), toVec3(model->getVelocity()));
	bodies.push_back({ model, body, type });
}

void PhysicEngine::addGranularBody(GranuleModel* model) {
	vector<GranuleSettings> granules = *model->getGranules();
	typedef map<int, vector<GranuleVectors*>*>::iterator iterator;
	map<int, vector<GranuleVectors*>*> triangleMap = *model->getTriangelMap();

	appMain.createProgressBar(triangleMap.size());
	for (iterator it = triangleMap.begin(); it != triangleMap.end(); it++) {
		if (it->second->size() < 2) continue;
		ModelTriangle triangle = model->getTriangle(it->first);
		glm::vec3 offset = triangle.normal * model->getAvgScale();
		vector<btRigidBody*> bodies;

		/*vector<glm::vec3> points = { 
			triangle.positions[0] - offset * triangle.maxLength, 
			triangle.positions[1] - offset * triangle.maxLength,
			triangle.positions[2] - offset * triangle.maxLength };
		btRigidBody* forcer = addConvexTriangleBody(vector<vector<glm::vec3>>({ points }), glm::vec3(0.0f), offset * SUITABLE_ACCELERATION, 1000, &FORCER_MASK);
		forcer->setGravity(toVec3(glm::vec3(0)));
		bodies.push_back(forcer);*/

		/*vector<vector<glm::vec3>> shellPoints;
		shellPoints.push_back(vector<glm::vec3>({ triangle.positions[0], triangle.positions[1], triangle.positions[2] }));
		shellPoints.push_back(vector<glm::vec3>({ triangle.positions[1], triangle.positions[2], triangle.positions[1] - offset * triangle.maxLength }));
		shellPoints.push_back(vector<glm::vec3>({ triangle.positions[1], triangle.positions[2], triangle.positions[2] - offset * triangle.maxLength }));
		shellPoints.push_back(vector<glm::vec3>({ triangle.positions[0], triangle.positions[2], triangle.positions[0] - offset * triangle.maxLength }));
		shellPoints.push_back(vector<glm::vec3>({ triangle.positions[0], triangle.positions[2], triangle.positions[2] - offset * triangle.maxLength }));
		shellPoints.push_back(vector<glm::vec3>({ triangle.positions[0], triangle.positions[1], triangle.positions[0] - offset * triangle.maxLength }));
		shellPoints.push_back(vector<glm::vec3>({ triangle.positions[0], triangle.positions[1], triangle.positions[1] - offset * triangle.maxLength }));
		shellPoints.push_back(vector<glm::vec3>({ 
			triangle.positions[0] - offset * triangle.maxLength, 
			triangle.positions[1] - offset * triangle.maxLength, 
			triangle.positions[2] - offset * triangle.maxLength }));
		addStaticTriangleBody(shellPoints, glm::vec3(0.0f), &SHELL_MASK);*/

		bodies.push_back(addWall(getTriangleCenter({ triangle.positions[0], triangle.positions[1], triangle.positions[2] }), -triangle.normal, &SHELL_MASK));
		bodies.push_back(addWall((triangle.positions[0] + triangle.positions[2]) / 2.0f, glm::cross(triangle.positions[0] - triangle.positions[2], -triangle.normal), &SHELL_MASK));
		bodies.push_back(addWall((triangle.positions[1] + triangle.positions[0]) / 2.0f, glm::cross(triangle.positions[1] - triangle.positions[0], -triangle.normal), &SHELL_MASK));
		bodies.push_back(addWall((triangle.positions[2] + triangle.positions[1]) / 2.0f, glm::cross(triangle.positions[2] - triangle.positions[1], -triangle.normal), &SHELL_MASK));
		bodies.push_back(addWall(getTriangleCenter({ triangle.positions[0], triangle.positions[1], triangle.positions[2] }) - offset * triangle.minLength, triangle.normal, &SHELL_MASK));

		vector<btRigidBody*> granules;
		for (int j = 0; j < it->second->size(); j++) {
			btRigidBody* granule = addGranule(model, (*it->second)[j]->granuleIndex, (*it->second)[j]->exemplarIndex, getRandomVector() - 0.5f);
			granule->setGravity(toVec3(glm::vec3(0.0f)));
			bodies.push_back(granule);
			granules.push_back(granule);
		}

		for (int k = 0; k < 10; k++) {
			dynamicsWorld->stepSimulation(1 / 60.0f, 1);
		}

		for (int i = 0; i < granules.size(); i++) {
			btTransform transform;
			granules[i]->getMotionState()->getWorldTransform(transform);
			GranuleVectors vectors = { toVec3(transform.getOrigin()), toVec3(transform.getRotation().getAxis()) };
			model->setVectors((*it->second)[i]->granuleIndex, (*it->second)[i]->exemplarIndex, vectors);
		}

		/*
		btTransform transform;
		forcer->getMotionState()->getWorldTransform(transform);
		int forcerCount = 0;
		while (glm::length(toVec3(transform.getOrigin())) < glm::length(offset * (triangle.maxLength - triangle.minLength))) {
			dynamicsWorld->stepSimulation(1 / 60.f, 1);
			forcer->getMotionState()->getWorldTransform(transform);
			forcerCount++; 
			if (forcerCount > MAX_TICKS_IN_LOADING) break;
		}*/

		for (int k = 0; k < bodies.size(); k++) {
			dynamicsWorld->removeRigidBody(bodies[k]);
			delete bodies[k];
		}

		appMain.stepProgressBar();
	}
	appMain.removeProgressBar();

	model->updateVbos();
	model->setStatus(COMPRESSED);
}

btRigidBody* PhysicEngine::addGranule(GranuleModel* model, int granuleIndex, int exemplarIndex, glm::vec3 velocity) {
	GranuleVectors vectors = model->getVectors(granuleIndex, exemplarIndex);
	btQuaternion quaternion;
	if (vectors.rotation == glm::vec3(0.0f)) {
		quaternion = btQuaternion(0, 0, 0);
	}
	else {
		quaternion = btQuaternion(toVec3(vectors.rotation), workingRotationAngleRadians);
	}
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(quaternion, toVec3(vectors.position)));
	btCollisionShape* shape = new btSphereShape(vectors.scale.x);
	short granuleMask = COL_GRANULE;
	return createRigidBody(1, motionState, shape, btVector3(1, 1, 1), toVec3(velocity), &granuleMask);
}

btRigidBody* PhysicEngine::addWall(glm::vec3 position, glm::vec3 normal, short* mask) {
	btStaticPlaneShape* shape = new btStaticPlaneShape(toVec3(normal), 0);
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), toVec3(position)));
	return createRigidBody(0, motionState, shape, btVector3(0, 0, 0), btVector3(0, 0, 0), mask);
}

btRigidBody* PhysicEngine::addConvexBody(vector<glm::vec3> points, glm::vec3 velocity, float mass, short* mask) {
	btConvexHullShape* shape = new btConvexHullShape();
	for (int i = 0; i < points.size(); i++) {
		shape->addPoint(toVec3(points[i]));
	}
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
	return createRigidBody(mass, motionState, shape, btVector3(0, 0, 0), toVec3(velocity), mask);
}

btRigidBody* PhysicEngine::addConvexTriangleBody(vector<vector<glm::vec3>> points, glm::vec3 position, glm::vec3 velocity, float mass, short* mask) {
	btTriangleMesh* triangles = new btTriangleMesh();
	for (int i = 0; i < points.size(); i++) {
		triangles->addTriangle(toVec3(points[i][0]), toVec3(points[i][1]), toVec3(points[i][2]));
	}
	btConvexTriangleMeshShape* shape = new btConvexTriangleMeshShape(triangles);
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), toVec3(position)));
	return createRigidBody(mass, motionState, shape, btVector3(0, 0, 0), toVec3(velocity), mask);
}

btRigidBody* PhysicEngine::addStaticTriangleBody(vector<vector<glm::vec3>> points, glm::vec3 position, short* mask) {
	btTriangleMesh* triangles = new btTriangleMesh();
	for (int i = 0; i < points.size(); i++) {
		triangles->addTriangle(toVec3(points[i][0]), toVec3(points[i][1]), toVec3(points[i][2]));
	}
	btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(triangles, false);
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), toVec3(position)));
	return createRigidBody(0, motionState, shape, btVector3(0, 0, 0), btVector3(0, 0, 0), mask);
}

btRigidBody* PhysicEngine::createRigidBody(btScalar mass, btMotionState* motionState, btCollisionShape* shape, const btVector3 &inertia, const btVector3 &velocity, short* mask) {
	if (mass > 0) {
		shape->calculateLocalInertia(mass, btVector3(inertia));
	}
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, shape, inertia);
	btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);
	if (velocity != btVector3(0, 0, 0)) {
		rigidBody->setLinearVelocity(velocity);
	}
	if (mask != NULL) {
		dynamicsWorld->addRigidBody(rigidBody, *mask, *mask);
	}
	else {
		dynamicsWorld->addRigidBody(rigidBody);
	}
	return rigidBody;
}

void PhysicEngine::update() {
	dynamicsWorld->stepSimulation(1 / 60.f, 10);
	vector<int> indiciesToRemove;
	for (int i = 0; i < bodies.size(); i++) {
		if (bodies[i].type == BULLET) {
			if (!((Bullet*)bodies[i].model)->isAlive()) {
				indiciesToRemove.push_back(i);
				dynamicsWorld->removeRigidBody(bodies[i].body);
			}
			else {
				for (int j = 0; j < bodies.size(); j++) {
					if (bodies[j].type == TERRAIN || bodies[j].type == COMMON || bodies[j].type == BIG_GRANULE) {
						ModelContactResultCallback callback(&bodies[i], &bodies[j], this);
						dynamicsWorld->contactPairTest(bodies[i].body, bodies[j].body, callback);
					}
				}
			}
		}
	}
	while (!indiciesToRemove.empty()) {
		bodies.erase(bodies.begin() + indiciesToRemove.back());
		indiciesToRemove.pop_back();
	}
	if (renderDebug) {
		dynamicsWorld->debugDrawWorld();
	}
}

ObjectsForRendering* PhysicEngine::getObjectsForRendering() {
	return objects;
}

void PhysicEngine::resetObjectsForRendering() {
	delete objects;
	objects = new ObjectsForRendering();
}

void PhysicEngine::setRenderDebug(bool renderDebug) {
	this->renderDebug = renderDebug;
}

void PhysicEngine::release() {
	delete dynamicsWorld;
	delete solver;
	delete collisionConfiguration;
	delete dispatcher;
	delete broadphase;

	for (int i = 0; i < bodies.size(); i++) {
		delete bodies[i].body;
	}
}

ModelMotionState::ModelMotionState(AbstractModel* model) : model(model) {}

void ModelMotionState::getWorldTransform(btTransform &worldTrans) const {
	btQuaternion quaternion;
	if (model->getRotation() == glm::vec3(0.0f)) {
		quaternion = btQuaternion(0, 0, 0);
	}
	else {
		quaternion = btQuaternion(toVec3(model->getRotation()), workingRotationAngleRadians);
	}
	worldTrans = btTransform(quaternion, toVec3(model->getPosition()));
}

void ModelMotionState::setWorldTransform(const btTransform &worldTrans) {
	model->setModelMatrix(&toVec3(worldTrans.getOrigin()), &toVec3(worldTrans.getRotation().getAxis()));
}

GranuleMotionState::GranuleMotionState(GranuleModel* model, int granuleIndex, int exemplarIndex) : model(model), granuleIndex(granuleIndex), exemplarIndex(exemplarIndex) {}

void GranuleMotionState::getWorldTransform(btTransform &worldTrans) const {
	GranuleVectors vectors = model->getVectors(granuleIndex, exemplarIndex);
	btQuaternion quaternion;
	if (vectors.rotation == glm::vec3(0.0f)) {
		quaternion = btQuaternion(0, 0, 0);
	}
	else {
		quaternion = btQuaternion(toVec3(vectors.rotation), workingRotationAngleRadians);
	}
	worldTrans = btTransform(quaternion, toVec3(vectors.position));
}

void GranuleMotionState::setWorldTransform(const btTransform &worldTrans) {
	GranuleVectors vectors = { toVec3(worldTrans.getOrigin()), toVec3(worldTrans.getRotation().getAxis()) };
	model->setVectors(granuleIndex, exemplarIndex, vectors);
	//model->updateVbo(granuleIndex, exemplarIndex);
}

ModelContactResultCallback::ModelContactResultCallback(PhysicModel* invoker, PhysicModel* caught, PhysicEngine* engine) : btCollisionWorld::ContactResultCallback(), invoker(invoker), caught(caught), engine(engine) {}

btScalar ModelContactResultCallback::addSingleResult(btManifoldPoint& cp,
	const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
	const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) {

	if (caught->type == TERRAIN) {
		((Bullet*)invoker->model)->remove();
	}
	if (caught->type == COMMON) {
		invoker->model->contactAction();
	}
	if (caught->type == BIG_GRANULE) {
		ParticleSettings* pSettings = new ParticleSettings({
			invoker->model->getPosition(),
			glm::vec3(20.0f, 20.0f, 20.0f),
			glm::vec3(-40.0f, -40.0f, -40.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.5f),
			5.0f,
			10.0f,
			0.1f,
			30,
		});
		SystemSettings* sSettings = new SystemSettings({
			5.0f,
			10.0f,
			15,
		});
		engine->getObjectsForRendering()->particleSystems.push_back(new ParticleSystem(pSettings, sSettings));
	}
	return 0;
}