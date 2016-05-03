#pragma once

#include <pge/scene/State.h>

#include <pge/assetmanager/AssetManager.h>

#include <pge/rendering/Camera.h>
#include <pge/rendering/octree/DynamicOctree.h>

#include <pge/system/ThreadPool.h>

#include <random>

namespace pge {
	// For sorting by object layer
	static bool compare(const SceneObjectRef &sceneObject1, const SceneObjectRef &sceneObject2) {
		return sceneObject1->_layer < sceneObject2->_layer;
	}

	class Scene : public Uncopyable {
	protected:
		struct ObjectAddData {
			bool _octreeManaged;
			std::shared_ptr<SceneObject> _object;

			ObjectAddData() {}
			ObjectAddData(bool octreeManaged, const std::shared_ptr<SceneObject> &object)
				: _octreeManaged(octreeManaged), _object(object)
			{}
		};

		struct NamedObjectAddData {
			std::string _name;
			bool _octreeManaged;
			std::shared_ptr<SceneObject> _object;

			NamedObjectAddData() {}
			NamedObjectAddData(const std::string &name, bool octreeManaged, const std::shared_ptr<SceneObject> &object)
				: _name(name), _octreeManaged(octreeManaged), _object(object)
			{}
		};

		std::unique_ptr<State> _currentState;
		std::unique_ptr<State> _nextState;

		std::vector<ObjectAddData> _objectsToAdd;
		std::vector<NamedObjectAddData> _namedObjectsToAdd;
	
		ThreadPool _threadPool;
		std::vector<std::unique_ptr<void*>> _workerData;

		std::unordered_map<std::string, std::unique_ptr<AssetManager>> _assetManagers;

		std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>> _namedObjects;

	public:
		unsigned short _logicMask;
		unsigned short _renderMask;

		bool _close;

		Camera _logicCamera;

		DynamicOctree _octree;

		std::vector<SceneObjectRef> _visible;

		std::mt19937 _randomGenerator;

		Scene()
			: _logicMask(0xffff), _renderMask(0xffff), _close(false)
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
			return _assetManagers[name].get();
		}

		AssetManager* getAssetManager(const std::string &name, Asset* (*assetFactory)());

		SceneObject* getCurrentSceneObject(size_t index) {
			if (_currentState == nullptr)
				return nullptr;

			return _currentState->_sceneObjects[index].get();
		}

		SceneObject* getNextSceneObject(size_t index) {
			if (_nextState == nullptr)
				return nullptr;

			return _nextState->_sceneObjects[index].get();
		}

		size_t getNumWorkers() const {
			return _threadPool.getNumWorkers();
		}

		std::unique_ptr<void*> &getWorkerData(size_t index) {
			return _workerData[index];
		}
	};
}
