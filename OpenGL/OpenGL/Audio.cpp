#include "Audio.h"

ISoundEngine* SoundEngine::engine = NULL;

void SoundEngine::init() {
	engine = createIrrKlangDevice();
}

void SoundEngine::play2D(Sound type) {
	switch (type) {
	case SOUND_SHOT:
		engine->play2D(SHOT_AUDIO.c_str());
		break;
	case SOUND_COLLISION_BULLET:
		engine->play2D(RICOCHET_AUDIO.c_str());
		break;
	}
}

