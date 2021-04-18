#pragma once

#include "VoxelTerrain.h"
#include "VoxelChunk.h"

namespace pge {
    void terrainGenerator0(pge::VoxelChunk &chunk, int seed);
    void terrainGeneratorFlatlands(pge::VoxelChunk &chunk, int seed);
}