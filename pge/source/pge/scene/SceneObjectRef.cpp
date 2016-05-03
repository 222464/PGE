#include <pge/scene/SceneObjectRef.h>

#include <pge/scene/SceneObject.h>
#include <pge/scene/Scene.h>

using namespace pge;

SceneObjectRef::~SceneObjectRef() {
	if (_pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*_pSceneObject->_mutex);
		_pSceneObject->_pReferences.erase(this);
	}
}

void SceneObjectRef::operator=(SceneObject &sceneObject) {
	if (_pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*_pSceneObject->_mutex);
		_pSceneObject->_pReferences.erase(this);
	}

	_pSceneObject = &sceneObject;

	if (_pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*_pSceneObject->_mutex);
		_pSceneObject->_pReferences.insert(this);
	}
}

void SceneObjectRef::operator=(SceneObject* pSceneObject) {
	if (_pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*_pSceneObject->_mutex);
		_pSceneObject->_pReferences.erase(this);
	}

	_pSceneObject = pSceneObject;

	if (_pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*_pSceneObject->_mutex);
		_pSceneObject->_pReferences.insert(this);
	}
}

void SceneObjectRef::operator=(const SceneObjectRef &other) {
	if (_pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*_pSceneObject->_mutex);
		_pSceneObject->_pReferences.erase(this);
	}

	_pSceneObject = other._pSceneObject;

	if (_pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*_pSceneObject->_mutex);
		_pSceneObject->_pReferences.insert(this);
	}
}

void SceneObjectRef::operator=(std::nullptr_t) {
	if (_pSceneObject != nullptr) {
		std::lock_guard<std::recursive_mutex> lock(*_pSceneObject->_mutex);
		_pSceneObject->_pReferences.erase(this);
	}

	_pSceneObject = nullptr;
}

size_t SceneObjectRef::operator()(const SceneObjectRef &reference) const {
	return reinterpret_cast<size_t>(this) / sizeof(SceneObjectRef);
}