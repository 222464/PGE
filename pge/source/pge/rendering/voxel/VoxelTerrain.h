#pragma once

#include <pge/scene/SceneObject.h>

#include <pge/rendering/voxel/VoxelChunk.h>

#include <pge/rendering/texture/Texture2DArray.h>

namespace pge {
	class VoxelTerrain : public SceneObject {
	private:
		std::shared_ptr<std::vector<SceneObjectRef>> _chunks;

		Point3i _size;
		Point3i _center;

		std::array<Vec3f, 6> _normals;
		std::array<Point3i, 6> _positionOffsets;
		std::array<std::array<Vec3f, 4>, 6> _corners;

		std::list<SceneObjectRef> _renderChunks;

		std::shared_ptr<Shader> _gBufferRenderShader;

		std::shared_ptr<Shader> _depthRenderShader;

		std::shared_ptr<Texture2DArray> _diffuse;
		std::shared_ptr<Texture2DArray> _normal;

	public:
		voxelType _outsideMatrixVoxel;
		float _voxelScalar;
		float _voxelSize;

		int _numChunkLODs;
		float _LODSwitchDistance;

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
			return _size;
		}

		const Point3i getCenter() const {
			return _center;
		}

		SceneObject* copyFactory() {
			return new VoxelTerrain(*this);
		}

		const Vec3f &getNormal(size_t index) const {
			return _normals[index];
		}

		const Point3i &getPositionOffset(size_t index) const {
			return _positionOffsets[index];
		}

		friend class VoxelChunk;
	};
}