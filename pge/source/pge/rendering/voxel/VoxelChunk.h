#pragma once

#include <pge/scene/SceneObject.h>

#include <pge/rendering/mesh/StaticMesh.h>

#include <pge/sceneobjects/physics/SceneObjectPhysicsWorld.h>

#define PGE_VOXEL_CHUNK_INDEX_TYPE_ENUM GL_UNSIGNED_SHORT

namespace pge {
	typedef GLushort voxelChunkMeshIndexType;
	typedef char voxelType;

	class VoxelChunk : public SceneObject {
	public:
		static const int _chunkSize = 32;
		static const int _voxelsPerChunk = _chunkSize * _chunkSize * _chunkSize;

		struct ChunkVertex {
			Point3i _voxelPosition;

			ChunkVertex() {}
			ChunkVertex(const Point3i &p) {
				_voxelPosition = p;
			}

			size_t operator()(const ChunkVertex &set) const {
				return static_cast<size_t>(_voxelPosition.x ^ _voxelPosition.y ^ _voxelPosition.z);
			}

			bool operator==(const ChunkVertex &other) const {
				return _voxelPosition == other._voxelPosition;
			}
		};

		struct ChunkFace {
			voxelChunkMeshIndexType _i0, _i1, _i2, _i3, _i4, _i5;

			ChunkFace() {}
			ChunkFace(voxelChunkMeshIndexType qi0, voxelChunkMeshIndexType qi1, voxelChunkMeshIndexType qi2, voxelChunkMeshIndexType qi3)
				: _i0(qi0), _i1(qi1), _i2(qi2), _i3(qi0), _i4(qi2), _i5(qi3)
			{}

			void indexQuad(voxelChunkMeshIndexType qi0, voxelChunkMeshIndexType qi1, voxelChunkMeshIndexType qi2, voxelChunkMeshIndexType qi3) {
				_i0 = qi0;
				_i1 = qi1;
				_i2 = qi2;
				_i3 = qi0;
				_i4 = qi2;
				_i5 = qi3;
			}
		};

		struct Vertex {
			Vec3f _position;
			Vec3f _normal;

			Vertex()
				: _normal(0.0f, 0.0f, 0.0f)
			{}

			Vertex(const Vec3f &position)
				: _position(position), _normal(0.0f, 0.0f, 0.0f)
			{}

			Vertex(const Vec3f &position, const Vec3f &normal)
				: _position(position), _normal(normal)
			{}
		};

		struct LOD {
			VBO _vertices;
			VBO _indices;

			size_t _numVertices;
			size_t _numFaceIndices;
			size_t _numIndices;

			bool _empty;
		};
		
		struct SharedData {
			bool _empty;

			std::vector<LOD> _lods;

			std::array<voxelType, _voxelsPerChunk> _matrix;

			SharedData()
				: _empty(false)
			{}
		};

	private:
		// Physics
		std::shared_ptr<btTriangleMesh> _pTriangleMesh;
		std::shared_ptr<btBvhTriangleMeshShape> _pMeshShape;
		std::shared_ptr<btDefaultMotionState> _pMotionState;
		std::shared_ptr<btRigidBody> _pRigidBody;

		std::shared_ptr<SharedData> _sharedData;

		SceneObjectRef _voxelTerrain;
		SceneObjectRef _physicsWorld;

		Point3i _centerRelativePosition;

		void addVertex(const Point3i &center, int lod,
			unsigned char vertIndex, std::unordered_map<ChunkVertex, voxelChunkMeshIndexType, ChunkVertex> &positionToIndex,
			std::vector<Vertex> &vertices, std::vector<voxelChunkMeshIndexType> &faceIndices, std::unordered_map<voxelChunkMeshIndexType, bool> &isFlange, bool setAsFlange,
			class VoxelTerrain* pVoxelTerrain);

		void addGeometry(const Point3i &center, int lod, int side,
			std::unordered_map<ChunkVertex, voxelChunkMeshIndexType, ChunkVertex> &positionToIndex,
			std::vector<Vertex> &vertices, std::vector<voxelChunkMeshIndexType> &faceIndices, std::unordered_map<voxelChunkMeshIndexType, bool> &isFlange, bool setAsFlange,
			class VoxelTerrain* pVoxelTerrain);

	public:
		void create(const SceneObjectRef &voxelTerrain, const Point3i &centerRelativePosition);

		void generate(SceneObjectPhysicsWorld* pPhysicsWorld = nullptr, float restitution = 0.0f, float friction = 1.0f);

		// Inherited from SceneObject
		void deferredRender();

		void onDestroy();

		SceneObject* copyFactory() {
			return new VoxelChunk(*this);
		}

		const Point3i &getCenterRelativePosition() const {
			return _centerRelativePosition;
		}

		voxelType getVoxel(const Point3i &position) {
			assert(_sharedData != nullptr);

			return _sharedData->_matrix[position.x + position.y * _chunkSize + position.z * _chunkSize * _chunkSize];
		}

		void setVoxel(const Point3i &position, voxelType value) {
			assert(_sharedData != nullptr);

			_sharedData->_matrix[position.x + position.y * _chunkSize + position.z * _chunkSize * _chunkSize] = value;
		}

		void updateChunkAABB();

		class VoxelTerrain* getTerrain();

		friend class VoxelTerrain;
	};
}