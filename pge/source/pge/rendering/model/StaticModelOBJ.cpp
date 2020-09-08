#include "StaticModelOBJ.h"

using namespace pge;

bool StaticModelOBJ::createAsset(const std::string &fileName) {
	textureManager.create(Texture2D::assetFactory);

	return model.loadFromOBJ(fileName, textureManager, aabb, true, false);
}