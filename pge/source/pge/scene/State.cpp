#include "State.h"

using namespace pge;

void State::startProcessingNextState(ThreadPool &pool, State &next, unsigned short logicMask, float dt) {
    // Add all non-destroyed objects to the new state and keep track of reference changes
    newIndices.resize(sceneObjects.size());

    for (size_t i = 0; i < sceneObjects.size(); i++)
        newIndices[i] = i;

    size_t numLess = 0;

    for (size_t i = 0; i < sceneObjects.size(); i++) {
        newIndices[i] -= numLess;

        if (!sceneObjects[i]->shouldDestroy()) {
            next.sceneObjects.push_back(std::shared_ptr<SceneObject>(sceneObjects[i]->copyFactory()));
            next.sceneObjects.back()->indexPlusOne = next.sceneObjects.size();
            next.sceneObjects.back()->pReferences.clear();
        }
        else
            numLess++;
    }

    // Update all new scene objects in parallel
    for (size_t i = 0; i < next.sceneObjects.size(); i++)
    if ((logicMask & next.sceneObjects[i]->logicMask) != 0) {
        std::shared_ptr<SceneObjectWorkItem> workItem(new SceneObjectWorkItem());

        workItem->pItem = next.sceneObjects[i].get();
        workItem->dt = dt;

        pool.addItem(workItem);
    }

    // Update all new scene objects in parallel
    /*for (size_t i = 0; i < next.sceneObjects.size(); i++)
    if ((logicMask & next.sceneObjects[i]->logicMask) != 0) {
        next.sceneObjects[i]->update(dt);
    }*/
}

void State::waitForNextState(ThreadPool &pool, State &next) {
    pool.wait();

    // Check for more destruction
    for (size_t i = 0; i < next.sceneObjects.size(); i++)
    if (next.sceneObjects[i]->getThis() == nullptr || next.sceneObjects[i]->getThis()->shouldDestroy())
        next.sceneObjects[i]->destroy();

    // Update all references with new indicies
    for (size_t i = 0; i < sceneObjects.size(); i++) {
        for (SceneObjectRef* pRef : sceneObjects[i]->pReferences) {
            assert(pRef->pSceneObject != nullptr);
            pRef->pSceneObject = next.sceneObjects[newIndices[i]].get();

            next.sceneObjects[newIndices[i]]->pReferences.insert(pRef);
        }

        sceneObjects[i]->pReferences.clear();
    }
}