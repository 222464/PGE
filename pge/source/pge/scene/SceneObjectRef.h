#pragma once

#include <assert.h>
#include <cstddef>

namespace pge {
    class SceneObjectRef {
    private:
        class SceneObject* pSceneObject;

    public:
        SceneObjectRef()
            : pSceneObject(nullptr)
        {}

        SceneObjectRef(const SceneObjectRef &other)
            : pSceneObject(nullptr)
        {
            *this = other;
        }

        SceneObjectRef(class SceneObject &sceneObject)
            : pSceneObject(nullptr)
        {
            *this = sceneObject;
        }

        SceneObjectRef(SceneObject* pSceneObject)
            : pSceneObject(nullptr)
        {
            *this = pSceneObject;
        }

        SceneObjectRef(std::nullptr_t)
            : pSceneObject(nullptr)
        {}

        ~SceneObjectRef();

        void operator=(class SceneObject &sceneObject);
        void operator=(class SceneObject* pSceneObject);

        void operator=(std::nullptr_t);

        void operator=(const SceneObjectRef &other);

        class SceneObject* operator->() const {
            return pSceneObject;
        }

        class SceneObject &operator*() const {
            return *pSceneObject;
        }

        class SceneObject* get() const {
            return pSceneObject;
        }

        bool operator==(std::nullptr_t) const {
            return pSceneObject == nullptr;
        }

        bool operator!=(std::nullptr_t) const {
            return pSceneObject != nullptr;
        }

        bool isAlive() const {
            return pSceneObject != nullptr;
        }

        // Custom hash
        size_t operator()(const SceneObjectRef &reference) const;

        bool operator==(const SceneObjectRef &reference) const {
            return pSceneObject == reference.pSceneObject;
        }

        friend class State;
        friend class Scene;
        friend class RenderScene;
        friend class SceneObject;
    };
}