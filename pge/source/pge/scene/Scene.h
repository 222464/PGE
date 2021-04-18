#pragma once

#include "State.h"

#include "../assetmanager/AssetManager.h"

#include "../rendering/Camera.h"
#include "../rendering/octree/DynamicOctree.h"

#include "../system/ThreadPool.h"

#include <random>

namespace pge {
    // For sorting by object layer
    static bool compare(const SceneObjectRef &sceneObject1, const SceneObjectRef &sceneObject2) {
        return sceneObject1->layer < sceneObject2->layer;
    }

    class Scene : public Uncopyable {
    protected:
        struct ObjectAddData {
            bool octreeManaged;
            std::shared_ptr<SceneObject> object;

            ObjectAddData() {}
            ObjectAddData(bool octreeManaged, const std::shared_ptr<SceneObject> &object)
                : octreeManaged(octreeManaged), object(object)
            {}
        };

        struct NamedObjectAddData {
            std::string name;
            bool octreeManaged;
            std::shared_ptr<SceneObject> object;

            NamedObjectAddData() {}
            NamedObjectAddData(const std::string &name, bool octreeManaged, const std::shared_ptr<SceneObject> &object)
                : name(name), octreeManaged(octreeManaged), object(object)
            {}
        };

        std::unique_ptr<State> currentState;
        std::unique_ptr<State> nextState;

        std::vector<ObjectAddData> objectsToAdd;
        std::vector<NamedObjectAddData> namedObjectsToAdd;
    
        ThreadPool threadPool;
        std::vector<std::unique_ptr<void*>> workerData;

        std::unordered_map<std::string, std::unique_ptr<AssetManager>> assetManagers;

        std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>> namedObjects;

    public:
        unsigned short logicMask;
        unsigned short renderMask;

        bool close;

        Camera logicCamera;

        DynamicOctree octree;

        std::vector<SceneObjectRef> visible;

        std::mt19937 randomGenerator;

        Scene()
            : logicMask(0xffff), renderMask(0xffff), close(false)
        {}

        virtual ~Scene();

        void createScene(size_t numWorkers, const AABB3D &rootRegion);

        void update(float dt);

        void findVisible(const Camera &camera);
        void findVisible(const Camera &camera, float distance);
        void findVisible(const Camera &camera, std::vector<SceneObjectRef> &visible);
        void findVisible(const Camera &camera, float distance, std::vector<SceneObjectRef> &visible);

        void add(const std::shared_ptr<SceneObject> &sceneObject, bool octreeManaged = false);

        void addNamed(const std::shared_ptr<SceneObject> &sceneObject, const std::string &name, bool octreeManaged = false);

        SceneObjectRef getNamed(const std::string &name);
        std::unordered_set<SceneObjectRef, SceneObjectRef> getAllNamed(const std::string &name);

        SceneObjectRef getNamedCheckQueue(const std::string &name);
        std::unordered_set<SceneObjectRef, SceneObjectRef> getAllNamedCheckQueue(const std::string &name);

        AssetManager* getAssetManager(const std::string &name) {
            return assetManagers[name].get();
        }

        AssetManager* getAssetManager(const std::string &name, Asset* (*assetFactory)());

        SceneObject* getCurrentSceneObject(size_t index) {
            if (currentState == nullptr)
                return nullptr;

            return currentState->sceneObjects[index].get();
        }

        SceneObject* getNextSceneObject(size_t index) {
            if (nextState == nullptr)
                return nullptr;

            return nextState->sceneObjects[index].get();
        }

        size_t getNumWorkers() const {
            return threadPool.getNumWorkers();
        }

        std::unique_ptr<void*> &getWorkerData(size_t index) {
            return workerData[index];
        }
    };
}
