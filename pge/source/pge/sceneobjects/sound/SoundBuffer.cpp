#include "SoundBuffer.h"

using namespace pge;

bool SoundBuffer::createAsset(const std::string &name) {
    if (!buffer.loadFromFile(name))
        return false;

    return true;
}