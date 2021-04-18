#pragma once

#include "OctreeNode.h"
#include "../culling/Frustum.h"

#include <memory>

#include <unordered_set>
#include <list>

#include <mutex>
#include <thread>

namespace pge {
    // Callback function for ray cast
    typedef bool(*OctreeRayCallBack)(const SceneObjectRef &oc, float t0, float t1);

    // Base class for dynamic and static octree types
    class Octree {
    protected:
        std::unordered_set<SceneObjectRef, SceneObjectRef> outsideRoot;

        std::unique_ptr<OctreeNode> pRootNode;

        // Called whenever something is removed, an action can be defined by derived classes
        // Defaults to doing nothing
        virtual void onRemoval() {}

        void setOctree(const SceneObjectRef &oc);

        void recursiveCopy(OctreeNode* pThisNode, OctreeNode* pOtherNode, OctreeNode* pThisParent);

        std::recursive_mutex mutex;

    public:
        size_t minNumNodeOccupants;
        size_t maxNumNodeOccupants;
        size_t maxLevels;

        float oversizeMultiplier;

        Octree();
        Octree(const Octree &other) {
            *this = other;
        }

        virtual ~Octree() {}

        void operator=(const Octree &other);

        virtual void add(const SceneObjectRef &oc) = 0;

        void pruneDeadReferences();

        void queryRegion(const AABB3D &region, std::vector<SceneObjectRef> &result, unsigned short logicMask = 0xffff, unsigned short renderMask = 0xffff);
        void queryFrustum(std::vector<SceneObjectRef> &result, const Frustum &frustum, unsigned short logicMask = 0xffff, unsigned short renderMask = 0xffff);
        void querySegment(std::vector<SceneObjectRef> &result, const Vec3f &p1, const Vec3f &p2, unsigned short logicMask = 0xffff, unsigned short renderMask = 0xffff);
        void queryPoint(std::vector<SceneObjectRef> &result, const Vec3f &p, unsigned short logicMask = 0xffff, unsigned short renderMask = 0xffff);

        // Get closest
        void queryRay(SceneObjectRef &result, float &t, const Vec3f &start, const Vec3f &direction, unsigned short logicMask = 0xffff, unsigned short renderMask = 0xffff);
        void queryRay(SceneObjectRef &result, float &t, const Vec3f &start, const Vec3f &direction, OctreeRayCallBack callBack, unsigned short logicMask = 0xffff, unsigned short renderMask = 0xffff);

        friend class OctreeNode;
        friend class SceneObject;
    };
}