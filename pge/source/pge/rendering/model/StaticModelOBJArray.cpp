#include <pge/rendering/model/StaticModelOBJArray.h>

using namespace pge;

bool StaticModelOBJArray::createAsset(const std::string &fileName) {
	_textureManager.create(Texture2D::assetFactory);

	return _model.loadFromOBJ(fileName, _textureManager, _aabb, false, false);
}