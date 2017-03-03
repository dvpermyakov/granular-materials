#pragma once

#include "GLControl.h"
#include "Environment.h"
#include "Ground.h"
#include "Bullet.h"
#include "ParticleSystem.h"
#include "Granule.h"
#include "Buffer.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Text.h"
#include "Geometry.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <LinearMath/btIDebugDraw.h>

#include <vector>

using namespace std;

const int FREEBUFFER = 0;
const int MAX_TICKS_IN_LOADING = 1000;
const float SUITABLE_ACCELERATION = 10.0f;

enum RigidBodyType {
	TERRAIN = 0,
	BULLET,
	COMMON, 
	BIG_GRANULE,
	SMALL_GRANULE,
};

#define BIT(x) (1<<(x))
enum RigidBodyMask {
	COL_NOTHING = 0,       // Default, Collide with nothing
	COL_SHIP = BIT(0),     // Default, Collide with ships
	COL_WALL = BIT(1),     // Default, Collide with walls
	COL_POWERUP = BIT(2),  // Default, Collide with powerups
	COL_FORCER = BIT(3),
	COL_SHELL = BIT(4),
	COL_GRANULE = BIT(5),
};

struct PhysicModel {
	AbstractModel* model;
	btRigidBody* body;
	int type;
};

struct ObjectsForRendering {
	vector<ParticleSystem*> particleSystems;
};

class DebugDraw : public btIDebugDraw {
public:
	static void loadShaders();
	static void setControl(GLControl* control);
	static void setCamera(Camera* camera);
	static void setFont(GLFont* font);
	DebugDraw();
	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	void reportErrorWarning(const char* warningString);
	void draw3dText(const btVector3& location, const char* textString);
	int getDebugMode() const;
	void setDebugMode(int debugMode);
private:
	static ShaderProgram program;
	static GLControl* control;
	static Camera* camera;
	static GLFont* font;
	GLuint vao;
	VertexBufferObject vbo;
	int mode;
};

class PhysicEngine {
public:
	void load(glm::vec3 gravity = glm::vec3(0, -1, 0));
	void update();
	void addRigidBody(AbstractModel* model, float mass, RigidBodyType type);
	void addGranularBody(GranuleModel* model);
	void addTerrain(Ground* ground);
	void removeBigGranules();
	ObjectsForRendering* getObjectsForRendering();
	void resetObjectsForRendering();
	void setRenderDebug(bool renderDebug);
	void release();
private:
	static short FORCER_MASK;
	static short SHELL_MASK;
	btRigidBody* addWall(glm::vec3 position, glm::vec3 normal, short* mask = NULL);
	btRigidBody* addConvexBody(vector<glm::vec3> points, glm::vec3 velocity, float mass, short* mask = NULL);
	btRigidBody* addConvexTriangleBody(vector<vector<glm::vec3>> points, glm::vec3 position, glm::vec3 velocity, float mass, short* mask = NULL);
	btRigidBody* addStaticTriangleBody(vector<vector<glm::vec3>> points, glm::vec3 position, short* mask = NULL);
	btRigidBody* addGranule(GranuleModel* model, int granuleIndex, int exemplarIndex, glm::vec3 velocity);
	btRigidBody* createRigidBody(btScalar mass, btMotionState* motionState, btCollisionShape* shape, const btVector3 &inertia, const btVector3 &velocity = btVector3(0, 0, 0), short* mask = NULL);

	DebugDraw* debugDraw;
	btBroadphaseInterface* broadphase;
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

	vector<PhysicModel> bodies;
	ObjectsForRendering* objects;
	bool renderDebug;
};

class ModelMotionState : public btMotionState {
public:
	ModelMotionState(AbstractModel* model);
	void getWorldTransform(btTransform &worldTrans) const;
	void setWorldTransform(const btTransform &worldTrans);
private:
	AbstractModel* model;
};

class GranuleMotionState : public btMotionState {
public:
	GranuleMotionState(GranuleModel* model, int granuleIndex, int exemplarIndex);
	void getWorldTransform(btTransform &worldTrans) const;
	void setWorldTransform(const btTransform &worldTrans);
private:
	GranuleModel* model;
	int granuleIndex;
	int exemplarIndex;
};

struct ModelContactResultCallback : public btCollisionWorld::ContactResultCallback {
	ModelContactResultCallback(PhysicModel* invoker, PhysicModel* caught, PhysicEngine* engine);
	btScalar addSingleResult(btManifoldPoint& cp,
		const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0,
		const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1);
	PhysicEngine* engine;
	PhysicModel* invoker;
	PhysicModel* caught;
};