#pragma once

#include "../../system/Uncopyable.h"
#include "../../constructs/AABB3D.h"
#include "../../constructs/Point3i.h"

#include "../../scene/SceneObjectRef.h"

#include <memory>
#include <array>
#include <unordered_set>

#include <mutex>
#include <thread>

namespace pge {
    class OctreeNode : public Uncopyable {
    private:
        OctreeNode* pParent;
        class Octree* pOctree;

        bool hasChildren;

        std::array<std::unique_ptr<OctreeNode>, 8> children;

        std::unordered_set<SceneObjectRef, SceneObjectRef> occupants;

        AABB3D region;

        int level;

        int numOccupantsBelow;

        void getPossibleOccupantPosition(const SceneObjectRef &oc, Point3i &point);

        void addToThisLevel(const SceneObjectRef &oc);

        // Returns true if occupant was added to children
        bool addToChildren(const SceneObjectRef &oc);

        void destroyChildren() {
            for (int i = 0; i < 8; i++)
                children[i].reset();

            hasChildren = false;
        }

        void getOccupants(std::unordered_set<SceneObjectRef, SceneObjectRef> &occupants);

        void partition();

        void merge();

        void update(const SceneObjectRef &oc);
        void remove(const SceneObjectRef &oc);

        void removeForDeletion(std::unordered_set<SceneObjectRef, SceneObjectRef> &occupants);

    public:
        OctreeNode()
            : hasChildren(false), numOccupantsBelow(0)
        {}

        OctreeNode(const AABB3D &region, int level, OctreeNode* pParent, class Octree* pOctree);

        // For use after using default constructor
        void create(const AABB3D &region, int level, OctreeNode* pParent, class Octree* pOctree);

        class Octree* getTree() const {
            return pOctree;
        }

        void add(const SceneObjectRef &oc);

        const AABB3D &getRegion() const {
            return region;
        }

        void getAllOccupantsBelow(std::vector<SceneObjectRef> &occupants);
        void getAllOccupantsBelow(std::unordered_set<SceneObjectRef, SceneObjectRef> &occupants);

        int getNumOccupantsBelow() const {
            return numOccupantsBelow;
        }

        void pruneDeadReferences();

        friend class SceneObject;
        friend class Octree;
        friend class DynamicOctree;
    };
}