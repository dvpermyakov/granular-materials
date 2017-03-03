#include "Buffer.h"
#include "WindowApp.h"
#include "ShaderProgram.h"
#include "Environment.h"

#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <cmath>

using namespace std;

struct PixelInfo {
	float modelIndex;
	float meshIndex;
	float primitiveIndex;
	float z;
};

class Picking {
public:
	static void loadShaders();
	void init();
	void generatePickingMap(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
	void setModels(vector<AbstractModel*> models);
	void addModel(AbstractModel* model);
	void removeModel(AbstractModel* model);
	AbstractModel* getPickedModel(int x, int y);
private:
	static ShaderProgram program;
	float getFloatIndex(int index);
	int getIntIndex(float index);
	PixelInfo getPixelInfo(int x, int y);
	FrameBufferObject fbo;
	vector<AbstractModel*> models;
};