#pragma once

#include <AntTweakBar.h>

#include "GLControl.h"
#include "Granule.h"

#include <string>
#include <map>

using namespace std;

class ATBInterface {
public:
	static void init(GLControl* control);
	static void render(Granule* granule);
	static void render(GranuleModel* model);
	static void removeAllBars();
	static void clearMap();
	static TwBar* getBar(Granule* granule);
	static void release();
private:
	static string getBarId(Granule* granule);
	static string getBarId(GranuleModel* model);
	static GLControl* control;
	static map<Granule*, TwBar*> barMap;
	static TwBar* modelBar;
	static GranuleModel* model;

	static TwType TYPE_GEOMETRY;
};