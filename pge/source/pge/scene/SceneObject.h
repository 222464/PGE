#pragma once

#include <pge/rendering/SFMLOGL.h>

#include <pge/scene/SceneObjectRef.h>

#include <pge/constructs/AABB3D.h>

#include <unordered_set>
#include <thread>
#include <mutex>
#include <memory>

namespace pge {
	class SceneObject {
	private:
		class Scene* _pScene;

		size_t _indexPlusOne;

		std::unordered_set<SceneObjectRef*> _pReferences;

		SceneObjectRef _this;

		class OctreeNode* _pOctreeNode;
		class Octree* _pOctree;

		bool _needsTreeUpdate;

		std::shared_ptr<std::recursive_mutex> _mutex;

		void treeUpdate();

	protected:
		AABB3D _aabb;

		bool _syncable;

		bool _shouldDestroy;

	public:
		unsigned short _logicMask;
		unsigned short _renderMask;

		float _layer;

		std::string _tag;

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
			return _pScene;
		}

		class RenderScene* getRenderScene() const;

		size_t getIndexPlusOne() const {
			return _indexPlusOne;
		}

		void destroy() {
			_shouldDestroy = true;
		}

		bool shouldDestroy() const {
			return _shouldDestroy;
		}

		const AABB3D &getAABB() const {
			return _aabb;
		}

		Octree* getTree() const {
			return _pOctree;
		}

		const SceneObjectRef &getThis() const {
			return _this;
		}

		virtual SceneObject* copyFactory() = 0;

		void removeReferences();

		void updateAABB() {
			_needsTreeUpdate = true;
		}

		friend class State;
		friend class Scene;
		friend class RenderScene;
		friend SceneObjectRef;
		friend class OctreeNode;
		friend class Octree;
	};
}
