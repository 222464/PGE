#include "SceneObjectRef.h"

#include "SceneObject.h"
#include "Scene.h"

using namespace pge;

SceneObjectRef::~SceneObjectRef() {
	if (pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*pSceneObject->mutex);
		pSceneObject->pReferences.erase(this);
	}
}

void SceneObjectRef::operator=(SceneObject &sceneObject) {
	if (pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*pSceneObject->mutex);
		pSceneObject->pReferences.erase(this);
	}

	pSceneObject = &sceneObject;

	if (pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*pSceneObject->mutex);
		pSceneObject->pReferences.insert(this);
	}
}

void SceneObjectRef::operator=(SceneObject* pSceneObject) {
	if (pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*pSceneObject->mutex);
		pSceneObject->pReferences.erase(this);
	}

	this->pSceneObject = pSceneObject;

	if (pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*pSceneObject->mutex);
		pSceneObject->pReferences.insert(this);
	}
}

void SceneObjectRef::operator=(const SceneObjectRef &other) {
	if (pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*pSceneObject->mutex);
		pSceneObject->pReferences.erase(this);
	}

	pSceneObject = other.pSceneObject;

	if (pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*pSceneObject->mutex);
		pSceneObject->pReferences.insert(this);
	}
}

void SceneObjectRef::operator=(std::nullptr_t) {
	if (pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*pSceneObject->mutex);
		pSceneObject->pReferences.erase(this);
	}

	pSceneObject = nullptr;
}

size_t SceneObjectRef::operator()(const SceneObjectRef &reference) const {
	return reinterpret_cast<size_t>(this) / sizeof(SceneObjectRef);
}