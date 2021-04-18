#pragma once

#include "../rendering/SFMLOGL.h"

#include "SceneObjectRef.h"

#include "../constructs/AABB3D.h"

#include <unordered_set>
#include <thread>
#include <mutex>
#include <memory>

namespace pge {
    class SceneObject {
    private:
        class Scene* pScene;

        size_t indexPlusOne;

        std::unordered_set<SceneObjectRef*> pReferences;

        SceneObjectRef thisRef;

        class OctreeNode* pOctreeNode;
        class Octree* pOctree;

        bool needsTreeUpdate;

        std::shared_ptr<std::recursive_mutex> mutex;

        void treeUpdate();

    protected:
        AABB3D aabb;

        bool syncable;

        bool shouldDestroyFlag;

    public:
        unsigned short logicMask;
        unsigned short renderMask;

        float layer;

        std::string tag;

        SceneObject();
        virtual ~SceneObject() {
            removeReferences();
        }

        virtual void onQueue() {}
        virtual void onAdd() {}
        virtual void preSynchronousUpdate(float dt) {}
        virtual void update(float dt) {}
        virtual void updateWithThreadData(float dt, size_t threadIndex) {}
        virtual void synchronousUpdate(float dt) {}
        virtual void onDestroy() {}

        // Render functions
        virtual void preRender() {}
        virtual void deferredRender() {}
        virtual void batchRender() {}
        virtual void postRender() {}

        class Scene* getScene() const {
            return pScene;
        }

        class RenderScene* getRenderScene() const;

        size_t getIndexPlusOne() const {
            return indexPlusOne;
        }

        void destroy() {
            shouldDestroyFlag = true;
        }

        bool shouldDestroy() const {
            return shouldDestroyFlag;
        }

        const AABB3D &getAABB() const {
            return aabb;
        }

        Octree* getTree() const {
            return pOctree;
        }

        const SceneObjectRef &getThis() const {
            return thisRef;
        }

        virtual SceneObject* copyFactory() = 0;

        void removeReferences();

        void updateAABB() {
            needsTreeUpdate = true;
        }

        friend class State;
        friend class Scene;
        friend class RenderScene;
        friend SceneObjectRef;
        friend class OctreeNode;
        friend class Octree;
    };
}
