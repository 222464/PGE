#include <pge/sceneobjects/SceneObjectPropLOD.h>

#include <pge/rendering/model/SceneObjectStaticModelBatcher.h>

#include <pge/rendering/shader/Shader.h>

bool SceneObjectPropLOD::create(const std::vector<std::string> &fileNames) {
	assert(getScene() != nullptr);

	for (size_t i = 0; i < fileNames.size(); i++) {
		std::shared_ptr<pge::Asset> asset;

		if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset(fileNames[i], asset))
			return false;

		pge::StaticModelOBJ* pModelOBJ = static_cast<pge::StaticModelOBJ*>(asset.get());

		pModelOBJ->_model.genMipMaps();

		_pModelsOBJ.push_back(pModelOBJ);
	}

	_transform = pge::Matrix4x4f::identityMatrix();

	return true;
}

void SceneObjectPropLOD::calculateAABB() {
	_aabb = _pModelsOBJ[0]->getAABB().getTransformedAABB(_transform);

	if (getScene() != nullptr)
		updateAABB();
}

void SceneObjectPropLOD::onAdd() {
	_batcherRef = getScene()->getNamed("smb");

	assert(_batcherRef.isAlive());
}

void SceneObjectPropLOD::deferredRender() {
	pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(_batcherRef.get());

	int lodIndex = std::min(static_cast<int>(_pModelsOBJ.size()) - 1, static_cast<int>(((_transform * pge::Vec3f(0.0f, 0.0f, 0.0f)) - getRenderScene()->_logicCamera._position).magnitude() / _lodSwitchDistance));

	_pModelsOBJ[lodIndex]->render(pBatcher, _transform);
}