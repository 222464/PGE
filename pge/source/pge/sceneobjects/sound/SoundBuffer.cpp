#include <pge/sceneobjects/sound/SoundBuffer.h>

using namespace pge;

bool SoundBuffer::createAsset(const std::string &name) {
	if (!_buffer.loadFromFile(name))
		return false;

	return true;
}