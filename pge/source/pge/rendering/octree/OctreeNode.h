#pragma once

#include <pge/system/Uncopyable.h>
#include <pge/constructs/AABB3D.h>
#include <pge/constructs/Point3i.h>

#include <pge/scene/SceneObjectRef.h>

#include <memory>
#include <array>
#include <unordered_set>

#include <mutex>
#include <thread>

namespace pge {
	class OctreeNode : public Uncopyable {
	private:
		OctreeNode* _pParent;
		class Octree* _pOctree;

		bool _hasChildren;

		std::array<std::unique_ptr<OctreeNode>, 8> _children;

		std::unordered_set<SceneObjectRef, SceneObjectRef> _occupants;

		AABB3D _region;

		int _level;

		int _numOccupantsBelow;

		void getPossibleOccupantPosition(const SceneObjectRef &oc, Point3i &point);

		void addToThisLevel(const SceneObjectRef &oc);

		// Returns true if occupant was added to children
		bool addToChildren(const SceneObjectRef &oc);

		void destroyChildren() {
			for (int i = 0; i < 8; i++)
				_children[i].reset();

			_hasChildren = false;
		}

		void getOccupants(std::unordered_set<SceneObjectRef, SceneObjectRef> &occupants);

		void partition();

		void merge();

		void update(const SceneObjectRef &oc);
		void remove(const SceneObjectRef &oc);

		void removeForDeletion(std::unordered_set<SceneObjectRef, SceneObjectRef> &occupants);

	public:
		OctreeNode()
			: _hasChildren(false), _numOccupantsBelow(0)
		{}

		OctreeNode(const AABB3D &region, int level, OctreeNode* pParent, class Octree* pOctree);

		// For use after using default constructor
		void create(const AABB3D &region, int level, OctreeNode* pParent, class Octree* pOctree);

		class Octree* getTree() const {
			return _pOctree;
		}

		void add(const SceneObjectRef &oc);

		const AABB3D &getRegion() const {
			return _region;
		}

		void getAllOccupantsBelow(std::vector<SceneObjectRef> &occupants);
		void getAllOccupantsBelow(std::unordered_set<SceneObjectRef, SceneObjectRef> &occupants);

		int getNumOccupantsBelow() const {
			return _numOccupantsBelow;
		}

		void pruneDeadReferences();

		friend class SceneObject;
		friend class Octree;
		friend class DynamicOctree;
	};
}