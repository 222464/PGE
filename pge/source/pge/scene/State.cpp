#include <pge/scene/State.h>

using namespace pge;

void State::startProcessingNextState(ThreadPool &pool, State &next, unsigned short logicMask, float dt) {
	// Add all non-destroyed objects to the new state and keep track of reference changes
	_newIndices.resize(_sceneObjects.size());

	for (size_t i = 0; i < _sceneObjects.size(); i++)
		_newIndices[i] = i;

	size_t numLess = 0;

	for (size_t i = 0; i < _sceneObjects.size(); i++) {
		_newIndices[i] -= numLess;

		if (!_sceneObjects[i]->_shouldDestroy) {
			next._sceneObjects.push_back(std::shared_ptr<SceneObject>(_sceneObjects[i]->copyFactory()));
			next._sceneObjects.back()->_indexPlusOne = next._sceneObjects.size();
			next._sceneObjects.back()->_pReferences.clear();
		}
		else
			numLess++;
	}

	// Update all new scene objects in parallel
	for (size_t i = 0; i < next._sceneObjects.size(); i++)
	if ((logicMask & next._sceneObjects[i]->_logicMask) != 0) {
		std::shared_ptr<SceneObjectWorkItem> workItem(new SceneObjectWorkItem());

		workItem->_pItem = next._sceneObjects[i].get();
		workItem->_dt = dt;

		pool.addItem(workItem);
	}

	// Update all new scene objects in parallel
	/*for (size_t i = 0; i < next._sceneObjects.size(); i++)
	if ((logicMask & next._sceneObjects[i]->_logicMask) != 0) {
		next._sceneObjects[i]->update(dt);
	}*/
}

void State::waitForNextState(ThreadPool &pool, State &next) {
	pool.wait();

	// Check for more destruction
	for (size_t i = 0; i < next._sceneObjects.size(); i++)
	if (next._sceneObjects[i]->getThis() == nullptr || next._sceneObjects[i]->getThis()->_shouldDestroy)
		next._sceneObjects[i]->destroy();

	// Update all references with new indicies
	for (size_t i = 0; i < _sceneObjects.size(); i++) {
		for (SceneObjectRef* pRef : _sceneObjects[i]->_pReferences) {
			assert(pRef->_pSceneObject != nullptr);
			pRef->_pSceneObject = next._sceneObjects[_newIndices[i]].get();

			next._sceneObjects[_newIndices[i]]->_pReferences.insert(pRef);
		}

		_sceneObjects[i]->_pReferences.clear();
	}
}