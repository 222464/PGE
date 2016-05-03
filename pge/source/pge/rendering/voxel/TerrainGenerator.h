#pragma once

#include <pge/rendering/voxel/VoxelTerrain.h>
#include <pge/rendering/voxel/VoxelChunk.h>

namespace pge {
	void terrainGenerator0(pge::VoxelChunk &chunk, int seed);
	void terrainGeneratorFlatlands(pge::VoxelChunk &chunk, int seed);
}