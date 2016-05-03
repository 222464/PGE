#pragma once

#include <pge/scene/SceneObject.h>
#include <pge/rendering/Camera.h>

#include <pge/system/ThreadPool.h>

#include <vector>

namespace pge {
	class SceneObjectWorkItem : public ThreadPool::WorkItem {
	public:
		SceneObject* _pItem;
		float _dt;

		// Inherited from ThreadPool::WorkItem
		void run(size_t threadIndex) {
			_pItem->update(_dt);
			_pItem->updateWithThreadData(_dt, threadIndex);
		}
	};

	class State {
	private:
		std::vector<std::shared_ptr<SceneObject>> _sceneObjects;

		// New indices for scene object references
		std::vector<size_t> _newIndices;

	public:
		// Runs threads on all scene objects
		void startProcessingNextState(ThreadPool &pool, State &next, unsigned short logicMask, float dt);

		// Waits for all threads to complete. Thread pool and next state must be the same used in startProcessingNextState
		void waitForNextState(ThreadPool &pool, State &next);

		friend class Scene;
		friend class RenderScene;
	};
}