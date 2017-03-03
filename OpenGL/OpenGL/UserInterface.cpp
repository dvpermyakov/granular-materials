#include "UserInterface.h"

const string BAR_ID = "bar";

GLControl* ATBInterface::control = NULL;
map<Granule*, TwBar*> ATBInterface::barMap;
TwBar* ATBInterface::modelBar = NULL;
GranuleModel* ATBInterface::model = NULL;

TwType ATBInterface::TYPE_GEOMETRY;

void TW_CALL setExemplarsButtonCallback(void* model) {
	((GranuleModel*)model)->setGranulesExemplars();
}

void TW_CALL changeTextureButtonCallback(void* granule) {
	((Granule*)granule)->setTexture();
}

void TW_CALL okButtonCallback(void* granule) {
	((Granule*)granule)->setRenderingSettings(false);
}

void TW_CALL ok2ButtonCallback(void* model) {
	((GranuleModel*)model)->setRenderingSettings(false);
}

void TW_CALL generateButtonCallback(void* granule) {
	((Granule*)granule)->generateGeometry();
}

void ATBInterface::init(GLControl* control) {
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(control->getViewPortWidth(), control->getViewPortHeight());
	ATBInterface::control = control;
	TwStructMember geometryMembers[] = {
		{ "fragmentation", TW_TYPE_UINT32, offsetof(GranuleGeometrySettings, fragCount), "min=1 max=5" },
		{ "smoothness", TW_TYPE_UINT32, offsetof(GranuleGeometrySettings, smoothCount), "min=1 max=5" },
		{ "sharpness", TW_TYPE_FLOAT, offsetof(GranuleGeometrySettings, sharpDegree), "min=0 max=1" },
	};
	TYPE_GEOMETRY = TwDefineStruct("Geometry", geometryMembers, 3, sizeof(GranuleGeometrySettings), NULL, NULL);
}

void ATBInterface::render(Granule* granule) {
	TwBar* bar = getBar(granule);
	string labelDeifne = getBarId(granule) + " visible=true ";
	TwDefine(labelDeifne.c_str());
	TwDraw();
}

void ATBInterface::render(GranuleModel* model) {
	if (modelBar == NULL || ATBInterface::model == NULL || model->getName() != ATBInterface::model->getName()) {
		ATBInterface::model = model;
		string barId = getBarId(model);
		modelBar = TwNewBar(barId.c_str());
		string labelDeifne = barId + " label='" + model->getName() + "'";
		TwDefine(labelDeifne.c_str());
		string textDefine = barId + " text=light";
		TwDefine(textDefine.c_str());
		string colorDefine = barId + " color='0 0 0' alpha=128";
		TwDefine(colorDefine.c_str());
		string positionDefine = barId + " position='" + to_string(control->getViewPortWidth() / 4) + " " + to_string(control->getViewPortHeight() / 4) + "'";
		TwDefine(positionDefine.c_str());
		string sizeDefine = barId + " size='" + to_string(control->getViewPortWidth() / 2) + " " + to_string(control->getViewPortHeight() / 2) + "'";
		TwDefine(sizeDefine.c_str());

		TwAddVarRW(modelBar, "MaxTriangles", TW_TYPE_UINT32, (void*)model->getMaxTrianglesRef(), "min=0");
		TwAddVarRW(modelBar, "VolumeLength", TW_TYPE_FLOAT, (void*)model->getVolumeLengthRef(), "min=0 max=100");
		TwAddVarRW(modelBar, "MinLength", TW_TYPE_FLOAT, (void*)model->getMinLengthRef(), "min=0 max=100");
		TwAddVarRW(modelBar, "MaxLength", TW_TYPE_FLOAT, (void*)model->getMaxLengthRef(), "min=0 max=100");
		TwAddVarRW(modelBar, "ScaleIndex", TW_TYPE_FLOAT, (void*)model->getScaleIndexRef(), "min=0 max=1");
		TwAddButton(modelBar, "Generate", setExemplarsButtonCallback, (void*)model, "label='Generate'");
		TwAddButton(modelBar, "Ok", ok2ButtonCallback, (void*)model, "label='Ok'");
	}
	removeAllBars();

	string labelDeifne = getBarId(model) + " visible=true ";
	TwDefine(labelDeifne.c_str());

	TwDraw();
}

TwBar* ATBInterface::getBar(Granule* granule) {
	if (barMap.find(granule) != barMap.end()) return barMap[granule];

	string barId = getBarId(granule);
	TwBar* bar = TwNewBar(barId.c_str());

	string labelDeifne = barId + " label='" + granule->getName() + "'";
	TwDefine(labelDeifne.c_str());
	string textDefine = barId + " text=light";
	TwDefine(textDefine.c_str());
	string colorDefine = barId + " color='0 0 0' alpha=128";
	TwDefine(colorDefine.c_str());
	string positionDefine = barId + " position='" + to_string(control->getViewPortWidth() / 4) + " " + to_string(control->getViewPortHeight() / 4) + "'";
	TwDefine(positionDefine.c_str());
	string sizeDefine = barId + " size='" + to_string(control->getViewPortWidth() / 2) + " " + to_string(control->getViewPortHeight() / 2) + "'";
	TwDefine(sizeDefine.c_str());

	// Array of drop down items
	TwEnumVal Textures[] = { { CLAY, "Clay" }, { MOSS, "Moss" }, { PEBBLE, "Pebble" }, { ROCK, "Rock" }, { SAND, "Sand" } };

	// ATB identifier for the array
	TwType TextureTwType = TwDefineEnum("Texture", Textures, 5);

	// Link it to the tweak bar
	TwAddVarRW(bar, "Texture", TextureTwType, granule->getTextureTypeRef(), NULL);
	TwAddButton(bar, "Change", changeTextureButtonCallback, (void*)granule, "label='ChangeTexture'");
	TwAddVarRW(bar, "Shininess", TW_TYPE_FLOAT, (void*)granule->getShininessRef(), "min=0");
	TwAddVarRW(bar, "Geometry", TYPE_GEOMETRY, (void*)granule->getGranuleGeometrySettings(), NULL);
	TwAddButton(bar, "Generate", generateButtonCallback, (void*)granule, "label='Generate'");
	TwAddVarRW(bar, "Probability", TW_TYPE_FLOAT, (void*)granule->getProbabilityRef(), "min=0");
	TwAddVarRW(bar, "Color", TW_TYPE_COLOR3F, (void*)granule->getColorRef(), NULL);
	TwAddButton(bar, "Ok", okButtonCallback, (void*)granule, "label='Ok'");

	barMap[granule] = bar;
	return bar;
}

void ATBInterface::removeAllBars() {
	typedef map<Granule*, TwBar*>::iterator iterator;
	for (iterator it = barMap.begin(); it != barMap.end(); it++) {
		string labelDeifne = getBarId(it->first) + " visible=false ";
		TwDefine(labelDeifne.c_str());
	}
	if (model != NULL) {
		string labelDeifne = getBarId(model) + " visible=false ";
		TwDefine(labelDeifne.c_str());
	}
}

void ATBInterface::clearMap() {
	barMap.clear();
}

string ATBInterface::getBarId(Granule* granule) {
	return BAR_ID + granule->getName();
}

string ATBInterface::getBarId(GranuleModel* model) {
	return BAR_ID + model->getName();
}

void ATBInterface::release() {
	TwTerminate();
}