#pragma once

#include "../../scene/SceneObject.h"

#include "../mesh/StaticMesh.h"

#include "../../sceneobjects/physics/SceneObjectPhysicsWorld.h"

#define PGE_VOXEL_CHUNK_INDEX_TYPE_ENUM GL_UNSIGNED_SHORT

namespace pge {
	typedef GLushort voxelChunkMeshIndexType;
	typedef char voxelType;

	class VoxelChunk : public SceneObject {
	public:
		static const int chunkSize = 32;
		static const int voxelsPerChunk = chunkSize * chunkSize * chunkSize;

		struct ChunkVertex {
			Point3i voxelPosition;

			ChunkVertex() {}
			ChunkVertex(const Point3i &p) {
				voxelPosition = p;
			}

			size_t operator()(const ChunkVertex &set) const {
				return static_cast<size_t>(voxelPosition.x ^ voxelPosition.y ^ voxelPosition.z);
			}

			bool operator==(const ChunkVertex &other) const {
				return voxelPosition == other.voxelPosition;
			}
		};

		struct ChunkFace {
			voxelChunkMeshIndexType i0, i1, i2, i3, i4, i5;

			ChunkFace() {}
			ChunkFace(voxelChunkMeshIndexType qi0, voxelChunkMeshIndexType qi1, voxelChunkMeshIndexType qi2, voxelChunkMeshIndexType qi3)
				: i0(qi0), i1(qi1), i2(qi2), i3(qi0), i4(qi2), i5(qi3)
			{}

			void indexQuad(voxelChunkMeshIndexType qi0, voxelChunkMeshIndexType qi1, voxelChunkMeshIndexType qi2, voxelChunkMeshIndexType qi3) {
				i0 = qi0;
				i1 = qi1;
				i2 = qi2;
				i3 = qi0;
				i4 = qi2;
				i5 = qi3;
			}
		};

		struct Vertex {
			Vec3f position;
			Vec3f normal;

			Vertex()
				: normal(0.0f, 0.0f, 0.0f)
			{}

			Vertex(const Vec3f &position)
				: position(position), normal(0.0f, 0.0f, 0.0f)
			{}

			Vertex(const Vec3f &position, const Vec3f &normal)
				: position(position), normal(normal)
			{}
		};

		struct LOD {
			VBO vertices;
			VBO indices;

			size_t numVertices;
			size_t numFaceIndices;
			size_t numIndices;

			bool empty;
		};
		
		struct SharedData {
			bool empty;

			std::vector<LOD> lods;

			std::array<voxelType, voxelsPerChunk> matrix;

			SharedData()
				: empty(false)
			{}
		};

	private:
		// Physics
		std::shared_ptr<btTriangleMesh> pTriangleMesh;
		std::shared_ptr<btBvhTriangleMeshShape> pMeshShape;
		std::shared_ptr<btDefaultMotionState> pMotionState;
		std::shared_ptr<btRigidBody> pRigidBody;

		std::shared_ptr<SharedData> sharedData;

		SceneObjectRef voxelTerrain;
		SceneObjectRef physicsWorld;

		Point3i centerRelativePosition;

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
			return centerRelativePosition;
		}

		voxelType getVoxel(const Point3i &position) {
			assert(sharedData != nullptr);

			return sharedData->matrix[position.x + position.y * chunkSize + position.z * chunkSize * chunkSize];
		}

		void setVoxel(const Point3i &position, voxelType value) {
			assert(sharedData != nullptr);

			sharedData->matrix[position.x + position.y * chunkSize + position.z * chunkSize * chunkSize] = value;
		}

		void updateChunkAABB();

		class VoxelTerrain* getTerrain();

		friend class VoxelTerrain;
	};
}