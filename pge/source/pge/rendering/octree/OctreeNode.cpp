#include "OctreeNode.h"

#include "Octree.h"

#include "../../scene/SceneObject.h"

#include <assert.h>

using namespace pge;

OctreeNode::OctreeNode(const AABB3D &region, int level, OctreeNode* pParent, Octree* pOctree)
: hasChildren(false),
region(region), level(level), pParent(pParent), pOctree(pOctree),
numOccupantsBelow(0)
{}

void OctreeNode::create(const AABB3D &region, int level, OctreeNode* pParent, Octree* pOctree) {
    hasChildren = false;

    this->region = region;
    this->level = level;
    this->pParent = pParent;
    this->pOctree = pOctree;
}

void OctreeNode::getPossibleOccupantPosition(const SceneObjectRef &oc, Point3i &point) {
    // Compare the center of the AABB of the occupant to that of this node to determine
    // which child it may (possibly, not certainly) fit in
    const Vec3f &occupantCenter(oc->aabb.getCenter());
    const Vec3f &nodeRegionCenter(region.getCenter());

    point.x = occupantCenter.x > nodeRegionCenter.x ? 1 : 0;
    point.y = occupantCenter.y > nodeRegionCenter.y ? 1 : 0;
    point.z = occupantCenter.z > nodeRegionCenter.z ? 1 : 0;
}

void OctreeNode::addToThisLevel(const SceneObjectRef &oc) {
    oc->pOctreeNode = this;

    if (occupants.find(oc) != occupants.end())
        return;

    occupants.insert(oc);
}

bool OctreeNode::addToChildren(const SceneObjectRef &oc) {
    assert(hasChildren);

    Point3i position;

    getPossibleOccupantPosition(oc, position);

    OctreeNode* pChild = children[position.x + position.y * 2 + position.z * 4].get();

    // See if the occupant fits in the child at the selected position
    if (pChild->region.contains(oc->aabb)) {
        // Fits, so can add to the child and finish
        pChild->add(oc);

        return true;
    }

    return false;
}

void OctreeNode::partition() {
    assert(!hasChildren);

    const Vec3f &halfRegionDims(region.getHalfDims());
    const Vec3f &regionLowerBound(region.getLowerBound());
    const Vec3f &regionCenter(region.getCenter());

    int nextLowerLevel = level - 1;

    for (int x = 0; x < 2; x++)
    for (int y = 0; y < 2; y++)
    for (int z = 0; z < 2; z++) {
        Vec3f offset(x * halfRegionDims.x, y * halfRegionDims.y, z * halfRegionDims.z);

        AABB3D childAABB(regionLowerBound + offset, regionCenter + offset);

        // Scale up AABB by the oversize multiplier
        childAABB.setHalfDims(childAABB.getHalfDims() * getTree()->oversizeMultiplier);

        children[x + y * 2 + z * 4].reset(new OctreeNode(childAABB, nextLowerLevel, this, pOctree));
    }

    hasChildren = true;
}

void OctreeNode::merge() {
    if (hasChildren) {
        // Place all occupants at lower levels into this node
        getOccupants(occupants);

        destroyChildren();
    }
}

void OctreeNode::getOccupants(std::unordered_set<SceneObjectRef, SceneObjectRef> &occupants) {
    // Iteratively parse subnodes in order to collect all occupants below this node
    std::list<OctreeNode*> open;

    open.push_back(this);

    while (!open.empty()) {
        // Depth-first (results in less memory usage), remove objects from open list
        OctreeNode* pCurrent = open.back();
        open.pop_back();

        // Get occupants
        for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->occupants.begin(); it != pCurrent->occupants.end(); it++)
        if ((*it) != nullptr) {
            // Assign new node
            (*it)->pOctreeNode = this;

            // Add to this node
            occupants.insert(*it);
        }

        // If the node has children, add them to the open list
        if (pCurrent->hasChildren)
        for (int i = 0; i < 8; i++)
            open.push_back(pCurrent->children[i].get());
    }
}

void OctreeNode::removeForDeletion(std::unordered_set<SceneObjectRef, SceneObjectRef> &occupants) {
    // Iteratively parse subnodes in order to collect all occupants below this node
    std::list<OctreeNode*> open;

    open.push_back(this);

    while (!open.empty()) {
        // Depth-first (results in less memory usage), remove objects from open list
        OctreeNode* pCurrent = open.back();
        open.pop_back();

        // Get occupants
        for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->occupants.begin(); it != pCurrent->occupants.end(); it++)
        if ((*it) != nullptr) {
            // Since will be deleted, remove the reference
            (*it)->pOctreeNode = nullptr;

            // Add to this node
            occupants.insert(*it);
        }

        // If the node has children, add them to the open list
        if (pCurrent->hasChildren)
        for (int i = 0; i < 8; i++)
            open.push_back(pCurrent->children[i].get());
    }
}

void OctreeNode::getAllOccupantsBelow(std::vector<SceneObjectRef> &occupants) {
    // Iteratively parse subnodes in order to collect all occupants below this node
    std::list<OctreeNode*> open;

    open.push_back(this);

    while (!open.empty()) {
        // Depth-first (results in less memory usage), remove objects from open list
        OctreeNode* pCurrent = open.back();
        open.pop_back();

        // Get occupants
        for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->occupants.begin(); it != pCurrent->occupants.end(); it++)
        if ((*it) != nullptr)
            // Add to this node
            occupants.push_back(*it);

        // If the node has children, add them to the open list
        if (pCurrent->hasChildren)
        for (int i = 0; i < 8; i++)
            open.push_back(pCurrent->children[i].get());
    }
}

void OctreeNode::getAllOccupantsBelow(std::unordered_set<SceneObjectRef, SceneObjectRef> &occupants) {
    // Iteratively parse subnodes in order to collect all occupants below this node
    std::list<OctreeNode*> open;

    open.push_back(this);

    while (!open.empty()) {
        // Depth-first (results in less memory usage), remove objects from open list
        OctreeNode* pCurrent = open.back();
        open.pop_back();

        // Get occupants
        for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->occupants.begin(); it != pCurrent->occupants.end(); it++)
        if ((*it) == nullptr)
            // Add to this node
            occupants.insert(*it);

        // If the node has children, add them to the open list
        if (pCurrent->hasChildren)
        for (int i = 0; i < 8; i++)
            open.push_back(pCurrent->children[i].get());
    }
}

void OctreeNode::update(const SceneObjectRef &oc) {
    if (oc == nullptr)
        return;

    if(!occupants.empty());
        // Remove, may be re-added to this node later
        occupants.erase(oc);

    // Propogate upwards, looking for a node that has room (the current one may still have room)
    OctreeNode* pNode = this;

    while (pNode != nullptr) {
        pNode->numOccupantsBelow--;

        // If has room for 1 more, found a spot
        if (pNode->region.contains(oc->aabb))
            break;

        pNode = pNode->pParent;
    }

    // If no node that could contain the occupant was found, add to outside root set
    if (pNode == nullptr) {
        assert(pOctree != nullptr);

        if (pOctree->outsideRoot.find(oc) != pOctree->outsideRoot.end())
            return;

        pOctree->outsideRoot.insert(oc);

        oc->pOctreeNode = nullptr;
    }
    else // Add to the selected node
        pNode->add(oc);
}

void OctreeNode::remove(const SceneObjectRef &oc) {
    assert(!occupants.empty());

    // Remove from node
    occupants.erase(oc);

    if (oc == nullptr)
        return;

    // Propogate upwards, merging if there are enough occupants in the node
    OctreeNode* pNode = this;

    while (pNode != nullptr) {
        pNode->numOccupantsBelow--;

        if (pNode->numOccupantsBelow >= pOctree->minNumNodeOccupants) {
            pNode->merge();

            break;
        }

        pNode = pNode->pParent;
    }
}

void OctreeNode::add(const SceneObjectRef &oc) {
    assert(oc != nullptr);

    numOccupantsBelow++;

    // See if the occupant fits into any children (if there are any)
    if (hasChildren) {
        if (addToChildren(oc))
            return; // Fit, can stop
    }
    else {
        // Check if we need a new partition
        if (static_cast<signed>(occupants.size()) >= pOctree->maxNumNodeOccupants && level < pOctree->maxLevels) {
            partition();

            if (addToChildren(oc))
                return;
        }
    }

    // Did not fit in anywhere, add to this level, even if it goes over the maximum size
    addToThisLevel(oc);
}

void OctreeNode::pruneDeadReferences() {
    for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = occupants.begin(); it != occupants.end();) {
        if (it->isAlive())
            it++;
        else
            it = occupants.erase(it);
    }

    if (hasChildren)
    for (int i = 0; i < 8; i++)
        children[i]->pruneDeadReferences();
}