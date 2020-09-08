#include "SceneObjectProp.h"

#include "../rendering/model/SceneObjectStaticModelBatcher.h"

#include "../rendering/shader/Shader.h"

bool SceneObjectProp::create(const std::string &fileName) {
	assert(getScene() != nullptr);

	std::shared_ptr<pge::Asset> asset;

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset(fileName, asset))
		return false;

	pModelOBJ = static_cast<pge::StaticModelOBJ*>(asset.get());

	pModelOBJ->model.genMipMaps();

	transform = pge::Matrix4x4f::identityMatrix();

	return true;
}

void SceneObjectProp::calculateAABB() {
	aabb = pModelOBJ->getAABB().getTransformedAABB(transform);

	if (getScene() != nullptr)
		updateAABB();
}

void SceneObjectProp::onAdd() {
	batcherRef = getScene()->getNamed("smb");

	assert(batcherRef.isAlive());
}

void SceneObjectProp::deferredRender() {
	pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(batcherRef.get());

	pModelOBJ->render(pBatcher, transform);
}