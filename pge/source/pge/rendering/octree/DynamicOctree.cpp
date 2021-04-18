#include "DynamicOctree.h"

#include "../../scene/SceneObjectRef.h"
#include "../../scene/SceneObject.h"

#include "../../util/Math.h"

#include <assert.h>

using namespace pge;

void DynamicOctree::operator=(const DynamicOctree &other) {
    Octree::operator=(other);

    minOutsideRoot = other.minOutsideRoot;
    maxOutsideRoot = other.maxOutsideRoot;
}

void DynamicOctree::add(const SceneObjectRef &oc) {
    std::unique_lock<std::recursive_mutex> lock(mutex);

    assert(created());

    // If the occupant fits in the root node
    if(pRootNode->getRegion().contains(oc->getAABB()))
        pRootNode->add(oc);
    else
        outsideRoot.insert(oc);

    setOctree(oc);
}

void DynamicOctree::expand() {
    std::unique_lock<std::recursive_mutex> lock(mutex);

    // Find direction with most occupants
    Vec3f averageDir(0.0f, 0.0f, 0.0f);

    for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = outsideRoot.begin(); it != outsideRoot.end(); it++)
        averageDir += ((*it)->getAABB().getCenter() - pRootNode->getRegion().getCenter()).normalized();

    Vec3f centerOffsetDist(pRootNode->getRegion().getHalfDims() / oversizeMultiplier);

    Vec3f centerOffset(sign(averageDir.x) * centerOffsetDist.x, sign(averageDir.y) * centerOffsetDist.y, sign(averageDir.z) * centerOffsetDist.z);

    // Child node position of current root node
    int rX = centerOffset.x > 0.0f ? 0 : 1;
    int rY = centerOffset.y > 0.0f ? 0 : 1;
    int rZ = centerOffset.z > 0.0f ? 0 : 1;

    AABB3D newRootAABB(Vec3f(0.0f, 0.0f, 0.0f), centerOffsetDist * 4.0f);
    newRootAABB.setCenter(centerOffset + pRootNode->getRegion().getCenter());

    OctreeNode* pNewRoot = new OctreeNode(newRootAABB,  pRootNode->level + 1, nullptr, this);

    // ----------------------- Manual Children Creation for New Root -------------------------

    const Vec3f &halfRegionDims(pNewRoot->region.getHalfDims());
    const Vec3f &regionLowerBound(pNewRoot->region.getLowerBound());
    const Vec3f &regionCenter(pNewRoot->region.getCenter());

    // Create the children nodes
    for(int x = 0; x < 2; x++)
        for(int y = 0; y < 2; y++)
            for(int z = 0; z < 2; z++) {
                if(x == rX && y == rY && z == rZ)
                    pNewRoot->children[x + y * 2 + z * 4].reset(pRootNode.release());
                else {
                    Vec3f offset(x * halfRegionDims.x, y * halfRegionDims.y, z * halfRegionDims.z);

                    AABB3D childAABB(regionLowerBound + offset, regionCenter + offset);

                    // Scale up AABB by the oversize multiplier
                    childAABB.setHalfDims(childAABB.getHalfDims() * oversizeMultiplier);
                    childAABB.calculateCenter();
    
                    pNewRoot->children[x + y * 2 + z * 4].reset(new OctreeNode(childAABB, pRootNode->level, pNewRoot, this));
                }
            }

    pNewRoot->hasChildren = true;
    pNewRoot->numOccupantsBelow = pRootNode->numOccupantsBelow;
    pRootNode->pParent = pNewRoot;

    // Transfer ownership
    pRootNode.release();
    pRootNode.reset(pNewRoot);

    // ----------------------- Try to Add Previously Outside Root -------------------------

    // Make copy so don't try to re-add ones just added
    std::unordered_set<SceneObjectRef, SceneObjectRef> outsideRootCopy(outsideRoot);
    outsideRoot.clear();

    for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = outsideRootCopy.begin(); it != outsideRootCopy.end(); it++)
        add(*it);
}

void DynamicOctree::contract() {
    std::unique_lock<std::recursive_mutex> lock(mutex);

    assert(pRootNode->hasChildren);

    // Find child with the most occupants and shrink to that
    int maxIndex = 0;

    for (int i = 1; i < 8; i++)
    if (pRootNode->children[i]->getNumOccupantsBelow() >
        pRootNode->children[maxIndex]->getNumOccupantsBelow())
        maxIndex = i;

    // Reorganize
    for (int i = 0; i < 8; i++) {
        if (i == maxIndex)
            continue;

        pRootNode->children[i]->removeForDeletion(outsideRoot);
    }

    OctreeNode* pNewRoot = pRootNode->children[maxIndex].release();

    pRootNode->destroyChildren();

    pRootNode->removeForDeletion(outsideRoot);

    pRootNode.reset(pNewRoot);

    pRootNode->pParent = nullptr;
}

void DynamicOctree::trim() {
    std::unique_lock<std::recursive_mutex> lock(mutex);

    if(pRootNode.get() == nullptr)
        return;

    // Check if should grow
    if(outsideRoot.size() > maxOutsideRoot)
        expand();
    else if(outsideRoot.size() < minOutsideRoot && pRootNode->hasChildren)
        contract();
}