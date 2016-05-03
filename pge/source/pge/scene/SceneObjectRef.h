#pragma once

#include <assert.h>

namespace pge {
	class SceneObjectRef {
	private:
		class SceneObject* _pSceneObject;

	public:
		SceneObjectRef()
			: _pSceneObject(nullptr)
		{}

		SceneObjectRef(const SceneObjectRef &other)
			: _pSceneObject(nullptr)
		{
			*this = other;
		}

		SceneObjectRef(class SceneObject &sceneObject)
			: _pSceneObject(nullptr)
		{
			*this = sceneObject;
		}

		SceneObjectRef(SceneObject* pSceneObject)
			: _pSceneObject(nullptr)
		{
			*this = pSceneObject;
		}

		SceneObjectRef(std::nullptr_t)
			: _pSceneObject(nullptr)
		{}

		~SceneObjectRef();

		void operator=(class SceneObject &sceneObject);
		void operator=(class SceneObject* pSceneObject);

		void operator=(std::nullptr_t);

		void operator=(const SceneObjectRef &other);

		class SceneObject* operator->() const {
			return _pSceneObject;
		}

		class SceneObject &operator*() const {
			return *_pSceneObject;
		}

		class SceneObject* get() const {
			return _pSceneObject;
		}

		bool operator==(std::nullptr_t) const {
			return _pSceneObject == nullptr;
		}

		bool operator!=(std::nullptr_t) const {
			return _pSceneObject != nullptr;
		}

		bool isAlive() const {
			return _pSceneObject != nullptr;
		}

		// Custom hash
		size_t operator()(const SceneObjectRef &reference) const;

		bool operator==(const SceneObjectRef &reference) const {
			return _pSceneObject == reference._pSceneObject;
		}

		friend class State;
		friend class Scene;
		friend class RenderScene;
		friend class SceneObject;
	};
}