#include "SceneObjectPropLOD.h"

#include "../rendering/model/SceneObjectStaticModelBatcher.h"

#include "../rendering/shader/Shader.h"

bool SceneObjectPropLOD::create(const std::vector<std::string> &fileNames) {
    assert(getScene() != nullptr);

    for (size_t i = 0; i < fileNames.size(); i++) {
        std::shared_ptr<pge::Asset> asset;

        if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset(fileNames[i], asset))
            return false;

        pge::StaticModelOBJ* pModelOBJ = static_cast<pge::StaticModelOBJ*>(asset.get());

        pModelOBJ->model.genMipMaps();

        pModelsOBJ.push_back(pModelOBJ);
    }

    transform = pge::Matrix4x4f::identityMatrix();

    return true;
}

void SceneObjectPropLOD::calculateAABB() {
    aabb = pModelsOBJ[0]->getAABB().getTransformedAABB(transform);

    if (getScene() != nullptr)
        updateAABB();
}

void SceneObjectPropLOD::onAdd() {
    batcherRef = getScene()->getNamed("smb");

    assert(batcherRef.isAlive());
}

void SceneObjectPropLOD::deferredRender() {
    pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(batcherRef.get());

    int lodIndex = std::min(static_cast<int>(pModelsOBJ.size()) - 1, static_cast<int>(((transform * pge::Vec3f(0.0f, 0.0f, 0.0f)) - getRenderScene()->logicCamera.position).magnitude() / lodSwitchDistance));

    pModelsOBJ[lodIndex]->render(pBatcher, transform);
}