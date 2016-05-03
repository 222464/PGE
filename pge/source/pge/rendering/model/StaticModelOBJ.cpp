#include <pge/rendering/model/StaticModelOBJ.h>

using namespace pge;

bool StaticModelOBJ::createAsset(const std::string &fileName) {
	_textureManager.create(Texture2D::assetFactory);

	return _model.loadFromOBJ(fileName, _textureManager, _aabb, true, false);
}