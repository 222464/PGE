#pragma once

#include "SceneObject.h"
#include "../rendering/Camera.h"

#include "../system/ThreadPool.h"

#include <vector>

namespace pge {
    class SceneObjectWorkItem : public ThreadPool::WorkItem {
    public:
        SceneObject* pItem;
        float dt;

        // Inherited from ThreadPool::WorkItem
        void run(size_t threadIndex) {
            pItem->update(dt);
            pItem->updateWithThreadData(dt, threadIndex);
        }
    };

    class State {
    private:
        std::vector<std::shared_ptr<SceneObject>> sceneObjects;

        // New indices for scene object references
        std::vector<size_t> newIndices;

    public:
        // Runs threads on all scene objects
        void startProcessingNextState(ThreadPool &pool, State &next, unsigned short logicMask, float dt);

        // Waits for all threads to complete. Thread pool and next state must be the same used in startProcessingNextState
        void waitForNextState(ThreadPool &pool, State &next);

        friend class Scene;
        friend class RenderScene;
    };
}