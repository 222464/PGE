#include <pge/sceneobjects/SceneObjectProp.h>

#include <pge/rendering/model/SceneObjectStaticModelBatcher.h>

#include <pge/rendering/shader/Shader.h>

bool SceneObjectProp::create(const std::string &fileName) {
	assert(getScene() != nullptr);

	std::shared_ptr<pge::Asset> asset;

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset(fileName, asset))
		return false;

	_pModelOBJ = static_cast<pge::StaticModelOBJ*>(asset.get());

	_pModelOBJ->_model.genMipMaps();

	_transform = pge::Matrix4x4f::identityMatrix();

	return true;
}

void SceneObjectProp::calculateAABB() {
	_aabb = _pModelOBJ->getAABB().getTransformedAABB(_transform);

	if (getScene() != nullptr)
		updateAABB();
}

void SceneObjectProp::onAdd() {
	_batcherRef = getScene()->getNamed("smb");

	assert(_batcherRef.isAlive());
}

void SceneObjectProp::deferredRender() {
	pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(_batcherRef.get());

	_pModelOBJ->render(pBatcher, _transform);
}