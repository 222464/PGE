#include "Scene.h"

using namespace pge;

Scene::~Scene() {
	for (size_t i = 0; i < currentState->sceneObjects.size(); i++) {
		currentState->sceneObjects[i]->removeReferences();
		currentState->sceneObjects[i]->onDestroy();
	}

	nextState = nullptr;
	currentState = nullptr;
}

void Scene::createScene(size_t numWorkers, const AABB3D &rootRegion) {
	assert(threadPool.getNumWorkers() == 0);

	threadPool.create(numWorkers);
	workerData.resize(numWorkers);

	octree.create(rootRegion);

	currentState.reset(new State());
}

void Scene::update(float dt) {
	for (size_t i = 0; i < currentState->sceneObjects.size(); i++)
	if (!currentState->sceneObjects[i]->shouldDestroy() && ((logicMask &currentState->sceneObjects[i]->logicMask) != 0)) {
		currentState->sceneObjects[i]->preSynchronousUpdate(dt);

		if (currentState->sceneObjects[i]->shouldDestroy()) {
			currentState->sceneObjects[i]->onDestroy();
			currentState->sceneObjects[i]->removeReferences();
		}
	}
	else {
		currentState->sceneObjects[i]->onDestroy();
		currentState->sceneObjects[i]->removeReferences();
	}

	// Create new state
	nextState.reset(new State());

	currentState->startProcessingNextState(threadPool, *nextState, logicMask, dt);
	currentState->waitForNextState(threadPool, *nextState);

	currentState.reset(nextState.release());
	nextState = nullptr;

	for (size_t i = 0; i < currentState->sceneObjects.size(); i++)
	if (!currentState->sceneObjects[i]->shouldDestroy() && ((logicMask &currentState->sceneObjects[i]->logicMask) != 0)) {
		currentState->sceneObjects[i]->synchronousUpdate(dt);

		if (currentState->sceneObjects[i]->shouldDestroy()) {
			currentState->sceneObjects[i]->onDestroy();
			currentState->sceneObjects[i]->removeReferences();
		}
	}
	else {
		currentState->sceneObjects[i]->onDestroy();
		currentState->sceneObjects[i]->removeReferences();
	}

	octree.pruneDeadReferences();

	// Update all octree occupants if they are flagged for an update
	for (size_t i = 0; i < currentState->sceneObjects.size(); i++)
	if (currentState->sceneObjects[i]->needsTreeUpdate) {
		currentState->sceneObjects[i]->treeUpdate();

		currentState->sceneObjects[i]->needsTreeUpdate = false;
	}

	// Remove name references to objects that have been destroyed
	for (std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>>::iterator itM = namedObjects.begin(); itM != namedObjects.end();) {
		for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator itS = itM->second.begin(); itS != itM->second.end();) {
			if (itS->isAlive())
				itS++;
			else
				itS = itM->second.erase(itS);
		}

		if (itM->second.empty())
			itM = namedObjects.erase(itM);
		else
			itM++;
	}

	// Add new objects generated in update
	for (size_t i = 0; i < objectsToAdd.size(); i++) {
		currentState->sceneObjects.push_back(objectsToAdd[i].object);

		objectsToAdd[i].object->indexPlusOne = currentState->sceneObjects.size();

		if (objectsToAdd[i].octreeManaged)
			octree.add(*objectsToAdd[i].object);
	}

	for (size_t i = 0; i < namedObjectsToAdd.size(); i++) {
		currentState->sceneObjects.push_back(namedObjectsToAdd[i].object);

		namedObjectsToAdd[i].object->indexPlusOne = currentState->sceneObjects.size();

		if (namedObjectsToAdd[i].octreeManaged)
			octree.add(*namedObjectsToAdd[i].object);

		namedObjects[namedObjectsToAdd[i].name].insert(*namedObjectsToAdd[i].object);
	}

	std::vector<NamedObjectAddData> currentNamedObjectAddData = namedObjectsToAdd;
	std::vector<ObjectAddData> currentObjectAddData = objectsToAdd;

	namedObjectsToAdd.clear();
	objectsToAdd.clear();

	for (size_t i = 0; i < currentObjectAddData.size(); i++)
		currentObjectAddData[i].object->onAdd();

	for (size_t i = 0; i < currentNamedObjectAddData.size(); i++)
		currentNamedObjectAddData[i].object->onAdd();
}

void Scene::findVisible(const Camera &camera) {
	visible.clear();

	std::vector<SceneObjectRef> result;

	octree.queryFrustum(result, camera.frustum, logicMask, renderMask);

	for (SceneObjectRef &oc : result)
	if (!oc->shouldDestroy())
		visible.push_back(oc);

	// Add all that don't have a tree
	for (std::shared_ptr<SceneObject> &sceneObject : currentState->sceneObjects)
	if (!sceneObject->shouldDestroy() && sceneObject->getTree() == nullptr && ((renderMask & sceneObject->renderMask) != 0))
		visible.push_back(*sceneObject);

	// Sort visible by layer
	std::sort(visible.begin(), visible.end(), compare);
}

void Scene::findVisible(const Camera &camera, float distance) {
	visible.clear();

	std::vector<SceneObjectRef> result;

	octree.queryFrustum(result, camera.frustum);

	for (SceneObjectRef &oc : result)
	if ((oc->getAABB().getCenter() - camera.position).magnitude() - oc->getAABB().getRadius() < distance)
	if (!oc->shouldDestroy())
		visible.push_back(*oc);

	// Add all that don't have a tree
	for (std::shared_ptr<SceneObject> &sceneObject : currentState->sceneObjects)
	if (!sceneObject->shouldDestroy() && sceneObject->getTree() == nullptr)
		visible.push_back(*sceneObject);

	// Sort visible by layer
	std::sort(visible.begin(), visible.end(), compare);
}

void Scene::findVisible(const Camera &camera, std::vector<SceneObjectRef> &visible) {
	visible.clear();

	std::vector<SceneObjectRef> result;

	octree.queryFrustum(result, camera.frustum, logicMask, renderMask);

	for (SceneObjectRef &oc : result)
	if (!oc->shouldDestroy())
		visible.push_back(oc);

	// Add all that don't have a tree
	for (std::shared_ptr<SceneObject> &sceneObject : currentState->sceneObjects)
	if (!sceneObject->shouldDestroy() && sceneObject->getTree() == nullptr && ((renderMask & sceneObject->renderMask) != 0))
		visible.push_back(*sceneObject);

	// Sort visible by layer
	std::sort(visible.begin(), visible.end(), compare);
}

void Scene::findVisible(const Camera &camera, float distance, std::vector<SceneObjectRef> &visible) {
	visible.clear();

	std::vector<SceneObjectRef> result;

	octree.queryFrustum(result, camera.frustum);

	for (SceneObjectRef &oc : result)
	if ((oc->getAABB().getCenter() - camera.position).magnitude() - oc->getAABB().getRadius() < distance)
	if (!oc->shouldDestroy())
		visible.push_back(*oc);

	// Add all that don't have a tree
	for (std::shared_ptr<SceneObject> &sceneObject : currentState->sceneObjects)
	if (!sceneObject->shouldDestroy() && sceneObject->getTree() == nullptr && ((renderMask & sceneObject->renderMask) != 0))
		visible.push_back(*sceneObject);

	// Sort visible by layer
	std::sort(visible.begin(), visible.end(), compare);
}

void Scene::add(const std::shared_ptr<SceneObject> &sceneObject, bool octreeManaged) {
	objectsToAdd.push_back(ObjectAddData(octreeManaged, sceneObject));
	sceneObject->pScene = this;
	sceneObject->onQueue();
	sceneObject->thisRef = *sceneObject;
}

void Scene::addNamed(const std::shared_ptr<SceneObject> &sceneObject, const std::string &name, bool octreeManaged) {
	namedObjectsToAdd.push_back(NamedObjectAddData(name, octreeManaged, sceneObject));
	sceneObject->pScene = this;
	sceneObject->onQueue();
	sceneObject->thisRef = *sceneObject;
}

SceneObjectRef Scene::getNamed(const std::string &name) {
	std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>>::iterator it = namedObjects.find(name);

	if (it == namedObjects.end())
		return nullptr;

	assert(!it->second.empty());

	return *it->second.begin();
}

std::unordered_set<SceneObjectRef, SceneObjectRef> Scene::getAllNamed(const std::string &name) {
	std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>>::iterator it = namedObjects.find(name);

	if (it == namedObjects.end())
		return std::unordered_set<SceneObjectRef, SceneObjectRef>();

	return it->second;
}

SceneObjectRef Scene::getNamedCheckQueue(const std::string &name) {
	std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>>::iterator it = namedObjects.find(name);

	if (it == namedObjects.end()) {
		// Check the queue
		for (size_t i = 0; i < namedObjectsToAdd.size(); i++)
		if (namedObjectsToAdd[i].name == name)
			return namedObjectsToAdd[i].object.get();

		return nullptr;
	}

	assert(!it->second.empty());

	return *it->second.begin();
}

std::unordered_set<SceneObjectRef, SceneObjectRef> Scene::getAllNamedCheckQueue(const std::string &name) {
	std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>>::iterator it = namedObjects.find(name);

	if (it == namedObjects.end()) {
		std::unordered_set<SceneObjectRef, SceneObjectRef> set;

		for (size_t i = 0; i < namedObjectsToAdd.size(); i++)
		if (namedObjectsToAdd[i].name == name)
			set.insert(namedObjectsToAdd[i].object.get());

		return set;
	}

	return it->second;
}

AssetManager* Scene::getAssetManager(const std::string &name, Asset* (*assetFactory)()) {
	std::unordered_map<std::string, std::unique_ptr<AssetManager>>::iterator it = assetManagers.find(name);

	if (it == assetManagers.end()) {
		AssetManager* pAssetManager = new AssetManager();
		assetManagers[name].reset(pAssetManager);

		pAssetManager->create(assetFactory);

		return pAssetManager;
	}
	
	return it->second.get();
}