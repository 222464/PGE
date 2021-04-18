#include "StaticModelOBJArray.h"

using namespace pge;

bool StaticModelOBJArray::createAsset(const std::string &fileName) {
    textureManager.create(Texture2D::assetFactory);

    return model.loadFromOBJ(fileName, textureManager, aabb, false, false);
}