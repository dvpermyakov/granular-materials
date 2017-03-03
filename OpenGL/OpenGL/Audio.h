#pragma once

#include "Resources.h"

#include <irrKlang.h>

#include <string>

using namespace irrklang;
using namespace std;

enum Sound {
	SOUND_SHOT = 0,
	SOUND_COLLISION_BULLET,
};

class SoundEngine {
public:
	static void init();
	static void play2D(Sound type);  // type is got from Sound enum
private:
	static ISoundEngine* engine;
};

extern SoundEngine sound;