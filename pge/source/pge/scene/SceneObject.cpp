#include "SceneObject.h"

#include "RenderScene.h"

using namespace pge;

SceneObject::SceneObject()
: aabb(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(1.0f, 1.0f, 1.0f)),
pOctreeNode(nullptr), pOctree(nullptr),
pScene(nullptr), indexPlusOne(0), layer(0.0f),
logicMask(0xffff), renderMask(0x0000),
tag(""), needsTreeUpdate(false),
syncable(false), shouldDestroyFlag(false),
mutex(new std::recursive_mutex())
{}

RenderScene* SceneObject::getRenderScene() const {
    return static_cast<RenderScene*>(pScene);
}

void SceneObject::treeUpdate() {
    if (pOctree == nullptr)
        return;

    std::unique_lock<std::recursive_mutex> lock(pOctree->mutex);

    if (pOctreeNode == nullptr) {
        // Not in a node, should be outside root then

        // If fits in the root now, add it
        OctreeNode* pRootNode = pOctree->pRootNode.get();

        if (pRootNode->region.contains(aabb))
            pRootNode->add(SceneObjectRef(this));
    }
    else
        pOctreeNode->update(SceneObjectRef(this));
}

void SceneObject::removeReferences() {
    std::unique_lock<std::recursive_mutex> lock(*mutex);

    // Nullify all references to this. Must copy since the references list will be modified when removing
    std::unordered_set<SceneObjectRef*> pReferencesCopy = pReferences;

    for (SceneObjectRef* pRef : pReferencesCopy)
        *pRef = nullptr;

    pReferences.clear();
}