#pragma once

#include "../../scene/SceneObject.h"

#include "VoxelChunk.h"

#include "../texture/Texture2DArray.h"

namespace pge {
	class VoxelTerrain : public SceneObject {
	private:
		std::shared_ptr<std::vector<SceneObjectRef>> chunks;

		Point3i size;
		Point3i center;

		std::array<Vec3f, 6> normals;
		std::array<Point3i, 6> positionOffsets;
		std::array<std::array<Vec3f, 4>, 6> corners;

		std::list<SceneObjectRef> renderChunks;

		std::shared_ptr<Shader> gBufferRenderShader;

		std::shared_ptr<Shader> depthRenderShader;

		std::shared_ptr<Texture2DArray> diffuse;
		std::shared_ptr<Texture2DArray> normal;

	public:
		voxelType outsideMatrixVoxel;
		float voxelScalar;
		float voxelSize;

		int numChunkLODs;
		float LODSwitchDistance;

		VoxelTerrain();

		void create(const Point3i &size,
			const std::shared_ptr<Shader> &gBufferRenderShader,
			const std::shared_ptr<Shader> &depthRenderShader,
			const std::shared_ptr<Texture2DArray> &diffuse,
			const std::shared_ptr<Texture2DArray> &normal);

		void generate(void(*generator)(pge::VoxelChunk &chunk, int seed), int seed, SceneObjectPhysicsWorld* pPhysicsWorld = nullptr, float restitution = 0.0f, float friction = 1.0f);

		// Inherited from SceneObject
		void batchRender();

		voxelType getVoxel(const Point3i &centerRelativePosition);
		void setVoxel(const Point3i &centerRelativePosition, voxelType voxel);

		void setChunkMatrixSize(const Point3i &size);

		const Point3i getSize() const {
			return size;
		}

		const Point3i getCenter() const {
			return center;
		}

		SceneObject* copyFactory() {
			return new VoxelTerrain(*this);
		}

		const Vec3f &getNormal(size_t index) const {
			return normals[index];
		}

		const Point3i &getPositionOffset(size_t index) const {
			return positionOffsets[index];
		}

		friend class VoxelChunk;
	};
}