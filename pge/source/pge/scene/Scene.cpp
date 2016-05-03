#include <pge/scene/Scene.h>

using namespace pge;

Scene::~Scene() {
	for (size_t i = 0; i < _currentState->_sceneObjects.size(); i++) {
		_currentState->_sceneObjects[i]->removeReferences();
		_currentState->_sceneObjects[i]->onDestroy();
	}

	_nextState = nullptr;
	_currentState = nullptr;
}

void Scene::createScene(size_t numWorkers, const AABB3D &rootRegion) {
	assert(_threadPool.getNumWorkers() == 0);

	_threadPool.create(numWorkers);
	_workerData.resize(numWorkers);

	_octree.create(rootRegion);

	_currentState.reset(new State());
}

void Scene::update(float dt) {
	for (size_t i = 0; i < _currentState->_sceneObjects.size(); i++)
	if (!_currentState->_sceneObjects[i]->_shouldDestroy && ((_logicMask &_currentState->_sceneObjects[i]->_logicMask) != 0)) {
		_currentState->_sceneObjects[i]->preSynchronousUpdate(dt);

		if (_currentState->_sceneObjects[i]->_shouldDestroy) {
			_currentState->_sceneObjects[i]->onDestroy();
			_currentState->_sceneObjects[i]->removeReferences();
		}
	}
	else {
		_currentState->_sceneObjects[i]->onDestroy();
		_currentState->_sceneObjects[i]->removeReferences();
	}

	// Create new state
	_nextState.reset(new State());

	_currentState->startProcessingNextState(_threadPool, *_nextState, _logicMask, dt);
	_currentState->waitForNextState(_threadPool, *_nextState);

	_currentState.reset(_nextState.release());
	_nextState = nullptr;

	for (size_t i = 0; i < _currentState->_sceneObjects.size(); i++)
	if (!_currentState->_sceneObjects[i]->_shouldDestroy && ((_logicMask &_currentState->_sceneObjects[i]->_logicMask) != 0)) {
		_currentState->_sceneObjects[i]->synchronousUpdate(dt);

		if (_currentState->_sceneObjects[i]->_shouldDestroy) {
			_currentState->_sceneObjects[i]->onDestroy();
			_currentState->_sceneObjects[i]->removeReferences();
		}
	}
	else {
		_currentState->_sceneObjects[i]->onDestroy();
		_currentState->_sceneObjects[i]->removeReferences();
	}

	_octree.pruneDeadReferences();

	// Update all octree occupants if they are flagged for an update
	for (size_t i = 0; i < _currentState->_sceneObjects.size(); i++)
	if (_currentState->_sceneObjects[i]->_needsTreeUpdate) {
		_currentState->_sceneObjects[i]->treeUpdate();

		_currentState->_sceneObjects[i]->_needsTreeUpdate = false;
	}

	// Remove name references to objects that have been destroyed
	for (std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>>::iterator itM = _namedObjects.begin(); itM != _namedObjects.end();) {
		for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator itS = itM->second.begin(); itS != itM->second.end();) {
			if (itS->isAlive())
				itS++;
			else
				itS = itM->second.erase(itS);
		}

		if (itM->second.empty())
			itM = _namedObjects.erase(itM);
		else
			itM++;
	}

	// Add new objects generated in update
	for (size_t i = 0; i < _objectsToAdd.size(); i++) {
		_currentState->_sceneObjects.push_back(_objectsToAdd[i]._object);

		_objectsToAdd[i]._object->_indexPlusOne = _currentState->_sceneObjects.size();

		if (_objectsToAdd[i]._octreeManaged)
			_octree.add(*_objectsToAdd[i]._object);
	}

	for (size_t i = 0; i < _namedObjectsToAdd.size(); i++) {
		_currentState->_sceneObjects.push_back(_namedObjectsToAdd[i]._object);

		_namedObjectsToAdd[i]._object->_indexPlusOne = _currentState->_sceneObjects.size();

		if (_namedObjectsToAdd[i]._octreeManaged)
			_octree.add(*_namedObjectsToAdd[i]._object);

		_namedObjects[_namedObjectsToAdd[i]._name].insert(*_namedObjectsToAdd[i]._object);
	}

	std::vector<NamedObjectAddData> currentNamedObjectAddData = _namedObjectsToAdd;
	std::vector<ObjectAddData> currentObjectAddData = _objectsToAdd;

	_namedObjectsToAdd.clear();
	_objectsToAdd.clear();

	for (size_t i = 0; i < currentObjectAddData.size(); i++)
		currentObjectAddData[i]._object->onAdd();

	for (size_t i = 0; i < currentNamedObjectAddData.size(); i++)
		currentNamedObjectAddData[i]._object->onAdd();
}

void Scene::findVisible(const Camera &camera) {
	_visible.clear();

	std::vector<SceneObjectRef> result;

	_octree.queryFrustum(result, camera._frustum, _logicMask, _renderMask);

	for (SceneObjectRef &oc : result)
	if (!oc->_shouldDestroy)
		_visible.push_back(oc);

	// Add all that don't have a tree
	for (std::shared_ptr<SceneObject> &sceneObject : _currentState->_sceneObjects)
	if (!sceneObject->_shouldDestroy && sceneObject->getTree() == nullptr && ((_renderMask & sceneObject->_renderMask) != 0))
		_visible.push_back(*sceneObject);

	// Sort visible by layer
	std::sort(_visible.begin(), _visible.end(), compare);
}

void Scene::findVisible(const Camera &camera, float distance) {
	_visible.clear();

	std::vector<SceneObjectRef> result;

	_octree.queryFrustum(result, camera._frustum);

	for (SceneObjectRef &oc : result)
	if ((oc->getAABB().getCenter() - camera._position).magnitude() - oc->getAABB().getRadius() < distance)
	if (!oc->_shouldDestroy)
		_visible.push_back(*oc);

	// Add all that don't have a tree
	for (std::shared_ptr<SceneObject> &sceneObject : _currentState->_sceneObjects)
	if (!sceneObject->_shouldDestroy && sceneObject->getTree() == nullptr)
		_visible.push_back(*sceneObject);

	// Sort visible by layer
	std::sort(_visible.begin(), _visible.end(), compare);
}

void Scene::findVisible(const Camera &camera, std::vector<SceneObjectRef> &visible) {
	visible.clear();

	std::vector<SceneObjectRef> result;

	_octree.queryFrustum(result, camera._frustum, _logicMask, _renderMask);

	for (SceneObjectRef &oc : result)
	if (!oc->_shouldDestroy)
		visible.push_back(oc);

	// Add all that don't have a tree
	for (std::shared_ptr<SceneObject> &sceneObject : _currentState->_sceneObjects)
	if (!sceneObject->_shouldDestroy && sceneObject->getTree() == nullptr && ((_renderMask & sceneObject->_renderMask) != 0))
		visible.push_back(*sceneObject);

	// Sort visible by layer
	std::sort(visible.begin(), visible.end(), compare);
}

void Scene::findVisible(const Camera &camera, float distance, std::vector<SceneObjectRef> &visible) {
	visible.clear();

	std::vector<SceneObjectRef> result;

	_octree.queryFrustum(result, camera._frustum);

	for (SceneObjectRef &oc : result)
	if ((oc->getAABB().getCenter() - camera._position).magnitude() - oc->getAABB().getRadius() < distance)
	if (!oc->_shouldDestroy)
		visible.push_back(*oc);

	// Add all that don't have a tree
	for (std::shared_ptr<SceneObject> &sceneObject : _currentState->_sceneObjects)
	if (!sceneObject->_shouldDestroy && sceneObject->getTree() == nullptr && ((_renderMask & sceneObject->_renderMask) != 0))
		visible.push_back(*sceneObject);

	// Sort visible by layer
	std::sort(visible.begin(), visible.end(), compare);
}

void Scene::add(const std::shared_ptr<SceneObject> &sceneObject, bool octreeManaged) {
	_objectsToAdd.push_back(ObjectAddData(octreeManaged, sceneObject));
	sceneObject->_pScene = this;
	sceneObject->onQueue();
	sceneObject->_this = *sceneObject;
}

void Scene::addNamed(const std::shared_ptr<SceneObject> &sceneObject, const std::string &name, bool octreeManaged) {
	_namedObjectsToAdd.push_back(NamedObjectAddData(name, octreeManaged, sceneObject));
	sceneObject->_pScene = this;
	sceneObject->onQueue();
	sceneObject->_this = *sceneObject;
}

SceneObjectRef Scene::getNamed(const std::string &name) {
	std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>>::iterator it = _namedObjects.find(name);

	if (it == _namedObjects.end())
		return nullptr;

	assert(!it->second.empty());

	return *it->second.begin();
}

std::unordered_set<SceneObjectRef, SceneObjectRef> Scene::getAllNamed(const std::string &name) {
	std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>>::iterator it = _namedObjects.find(name);

	if (it == _namedObjects.end())
		return std::unordered_set<SceneObjectRef, SceneObjectRef>();

	return it->second;
}

SceneObjectRef Scene::getNamedCheckQueue(const std::string &name) {
	std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>>::iterator it = _namedObjects.find(name);

	if (it == _namedObjects.end()) {
		// Check the queue
		for (size_t i = 0; i < _namedObjectsToAdd.size(); i++)
		if (_namedObjectsToAdd[i]._name == name)
			return _namedObjectsToAdd[i]._object.get();

		return nullptr;
	}

	assert(!it->second.empty());

	return *it->second.begin();
}

std::unordered_set<SceneObjectRef, SceneObjectRef> Scene::getAllNamedCheckQueue(const std::string &name) {
	std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>>::iterator it = _namedObjects.find(name);

	if (it == _namedObjects.end()) {
		std::unordered_set<SceneObjectRef, SceneObjectRef> set;

		for (size_t i = 0; i < _namedObjectsToAdd.size(); i++)
		if (_namedObjectsToAdd[i]._name == name)
			set.insert(_namedObjectsToAdd[i]._object.get());

		return set;
	}

	return it->second;
}

AssetManager* Scene::getAssetManager(const std::string &name, Asset* (*assetFactory)()) {
	std::unordered_map<std::string, std::unique_ptr<AssetManager>>::iterator it = _assetManagers.find(name);

	if (it == _assetManagers.end()) {
		AssetManager* pAssetManager = new AssetManager();
		_assetManagers[name].reset(pAssetManager);

		pAssetManager->create(assetFactory);

		return pAssetManager;
	}
	
	return it->second.get();
}