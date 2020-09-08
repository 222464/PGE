#include "TerrainGenerator.h"

#include "TerrainGrass.h"
#include "TerrainGrassBatcher.h"

#include "../lighting/SceneObjectPointLight.h"

#include "../../sceneobjects/SceneObjectProp.h"
#include "../../sceneobjects/SceneObjectPropLOD.h"

#include "../../constructs/Planef.h"

#include "../../util/NoiseGenerator.h"

#include "../../util/Math.h"

using namespace pge;

void pge::terrainGenerator0(VoxelChunk &chunk, int seed) {
	NoiseGenerator generator;
	generator.seed = seed;

	Point3i start((chunk.getTerrain()->getCenter() + chunk.getCenterRelativePosition()) * VoxelChunk::chunkSize);
	Point3i centerOffset(chunk.getCenterRelativePosition() * VoxelChunk::chunkSize);
	Vec3f startf(static_cast<float>(start.x), static_cast<float>(start.y), static_cast<float>(start.z));
	Vec3f centerf(0.0f, 0.0f, 0.0f);

	for (int x = 0; x < VoxelChunk::chunkSize; x++)
	for (int y = 0; y < VoxelChunk::chunkSize; y++)
	for (int z = 0; z < VoxelChunk::chunkSize; z++) {
		Vec3f worldPos(static_cast<float>(x + start.x), static_cast<float>(y + start.y), static_cast<float>(z + start.z));

		//float value = clamp((worldPos - centerf).magnitude() - 20.0f, -1.0f, 1.0f);
		float value = clamp(1.0f * ((generator.perlinNoise3D(worldPos.x * 0.01f, worldPos.y * 0.01f, worldPos.z * 0.01f, 6, 0.6f, 0.7f)) + 12.0f * worldPos.y / static_cast<float>(chunk.getTerrain()->getSize().y * VoxelChunk::chunkSize)), -1.0f, 1.0f);

		chunk.setVoxel(Point3i(x, y, z), static_cast<voxelType>(value * 127.0f));
	}

	std::vector<Vec3f> grassPositions;

	const int minGrassPerCell = 4;
	const int maxGrassPerCell = 7;

	const float grassYOffset = -0.2f;

	const float grassAngleCos = cosf(pi * 0.3f);

	//const float lightChance = 0.02f;

	//pge::SceneObjectRef lighting = chunk.getScene()->getNamed("lighting");

	int lowestVoxelY = -chunk.getTerrain()->getSize().y * VoxelChunk::chunkSize / 2;

	std::uniform_int_distribution<int> distGrassAmount(minGrassPerCell, maxGrassPerCell);
	std::uniform_real_distribution<float> voxelDist(0.0f, chunk.getTerrain()->voxelSize);
	std::uniform_real_distribution<float> uniformDist(0.0f, 1.0f);

	//std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

	// Find surface, add grass
	for (int x = 0; x < VoxelChunk::chunkSize; x++)
	for (int y = 0; y < VoxelChunk::chunkSize; y++)
	for (int z = 0; z < VoxelChunk::chunkSize; z++) {
		Point3i worldPosi(x + centerOffset.x, y + centerOffset.y, z + centerOffset.z);

		if (worldPosi.y <= lowestVoxelY)
			continue;

		//if (chunk.getVoxel(Point3i(x, y, z)) < 0 && chunk.getTerrain()->getVoxel(worldPosi + Point3i(0, 1, 0)) >= 0) {
		// Found surface. Add grass to this cell
		int grassAmount = distGrassAmount(chunk.getScene()->randomGenerator);

		Vec3f worldPosf(static_cast<float>(worldPosi.x), static_cast<float>(worldPosi.y), static_cast<float>(worldPosi.z));

		worldPosf *= chunk.getTerrain()->voxelSize;

		// ------------------------------- Extract plane from voxels -----------------------------------

		Point3i p1(worldPosi.x - 1, worldPosi.y - 1, worldPosi.z - 1);

		std::array<Vec3f, 8> pa;
		std::array<float, 8> va;

		for (unsigned char i = 0; i < 8; i++) {
			Point3i p = p1 + Point3i((i & 0x04) == 0 ? 0 : 1, (i & 0x02) == 0 ? 0 : 1, (i & 0x01) == 0 ? 0 : 1);
			Vec3f pf(static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z));
			float v = static_cast<float>(chunk.getTerrain()->getVoxel(p)) * chunk.getTerrain()->voxelScalar;

			pa[i] = pf;
			va[i] = v;
		}

		std::array<Vec3f, 4> intersections;

		size_t iIndex = 0;

		if (sign(va[7]) != sign(va[3])) {
			intersections[iIndex] = lerp(pa[7], pa[3], va[7] / (va[7] - va[3]));
			iIndex++;
		}

		if (sign(va[3]) != sign(va[1])) {
			intersections[iIndex] = lerp(pa[3], pa[1], va[3] / (va[3] - va[1]));
			iIndex++;
		}

		if (sign(va[1]) != sign(va[5])) {
			intersections[iIndex] = lerp(pa[1], pa[5], va[1] / (va[1] - va[5]));
			iIndex++;
		}

		if (iIndex < 3) {
			if (sign(va[5]) != sign(va[7])) {
				intersections[iIndex] = lerp(pa[5], pa[7], va[5] / (va[5] - va[7]));
				iIndex++;
			}

			if (iIndex < 3) {
				if (sign(va[2]) != sign(va[6])) {
					intersections[iIndex] = lerp(pa[2], pa[6], va[2] / (va[2] - va[6]));
					iIndex++;
				}

				if (iIndex < 3) {
					if (sign(va[6]) != sign(va[4])) {
						intersections[iIndex] = lerp(pa[6], pa[4], va[6] / (va[6] - va[4]));
						iIndex++;
					}

					if (iIndex < 3) {
						if (sign(va[4]) != sign(va[0])) {
							intersections[iIndex] = lerp(pa[4], pa[0], va[4] / (va[4] - va[0]));
							iIndex++;
						}

						if (iIndex < 3) {
							if (sign(va[0]) != sign(va[2])) {
								intersections[iIndex] = lerp(pa[0], pa[2], va[0] / (va[0] - va[2]));
								iIndex++;
							}

							if (iIndex < 3) {
								if (sign(va[3]) != sign(va[2])) {
									intersections[iIndex] = lerp(pa[3], pa[2], va[3] / (va[3] - va[2]));
									iIndex++;
								}

								if (iIndex < 3) {
									if (sign(va[1]) != sign(va[0])) {
										intersections[iIndex] = lerp(pa[1], pa[0], va[1] / (va[1] - va[0]));
										iIndex++;
									}

									if (iIndex < 3) {
										if (sign(va[5]) != sign(va[4])) {
											intersections[iIndex] = lerp(pa[5], pa[4], va[5] / (va[5] - va[4]));
											iIndex++;
										}

										if (iIndex < 3) {
											if (sign(va[7]) != sign(va[6])) {
												intersections[iIndex] = lerp(pa[7], pa[6], va[7] / (va[7] - va[6]));
												iIndex++;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		// Could have used a single goto instead of massive if system...

		if (iIndex < 3)
			continue;

		Vec3f normal = ((intersections[1] - intersections[0]).cross(intersections[2] - intersections[0])).normalized();

		Vec3f rD(0.0f, 1.0f, 0.0f);

		float denom = normal.dot(rD);

		if (denom > grassAngleCos) {
			for (int i = 0; i < grassAmount; i++) {
				Vec3f p;

				p.x = worldPosf.x + voxelDist(chunk.getScene()->randomGenerator) - chunk.getTerrain()->voxelSize;
				p.z = worldPosf.z + voxelDist(chunk.getScene()->randomGenerator) - chunk.getTerrain()->voxelSize;

				// Ray cast on plane
				Vec3f rO(p.x, 0.0f, p.z);

				Vec3f pL = intersections[1] - rO;
				float d = pL.dot(normal) / denom;

				p.y = d;

				grassPositions.push_back(p + Vec3f(0.0f, grassYOffset, 0.0f));
			}

			// Randomly spawn tree
			if (uniformDist(chunk.getScene()->randomGenerator) < 0.015f) {
				std::shared_ptr<SceneObjectPropLOD> tree(new SceneObjectPropLOD());

				chunk.getScene()->add(tree, true);

				std::vector<std::string> fileNames;

				fileNames.push_back("resources/models/trees/tree1ab/tree1a_lod0.obj");
				fileNames.push_back("resources/models/trees/tree1ab/tree1a_lod1.obj");
				fileNames.push_back("resources/models/trees/tree1ab/tree1a_lod2.obj");

				tree->create(fileNames);

				Vec3f p;

				p.x = worldPosf.x + voxelDist(chunk.getScene()->randomGenerator) - chunk.getTerrain()->voxelSize;
				p.z = worldPosf.z + voxelDist(chunk.getScene()->randomGenerator) - chunk.getTerrain()->voxelSize;

				// Ray cast on plane
				Vec3f rO(p.x, 0.0f, p.z);

				Vec3f pL = intersections[1] - rO;
				float d = pL.dot(normal) / denom;

				p.y = d;

				tree->transform = Matrix4x4f::translateMatrix(p);

				tree->calculateAABB();
			}
		}
	}

	if (!grassPositions.empty()) {
		std::shared_ptr<TerrainGrass> grass(new TerrainGrass());

		chunk.getScene()->add(grass, true);

		SceneObjectRef grassBatcher = chunk.getScene()->getNamedCheckQueue("grassBatcher");

		// Make sure the grass batcher was added to the scene
		if (grassBatcher == nullptr) {
			std::shared_ptr<TerrainGrassBatcher> batcher(new TerrainGrassBatcher());

			chunk.getScene()->addNamed(batcher, "grassBatcher");

			std::shared_ptr<Asset> assetGrassDiffuse;

			chunk.getScene()->getAssetManager("tex2D", Texture2D::assetFactory)->getAsset("resources/textures/grassTileSetDiffuse.png", assetGrassDiffuse);

			std::shared_ptr<Asset> assetGrassNormal;

			chunk.getScene()->getAssetManager("tex2D", Texture2D::assetFactory)->getAsset("resources/textures/grassTileSetNormal.png", assetGrassNormal);

			std::shared_ptr<Asset> assetNoise;

			chunk.getScene()->getAssetManager("tex2D", Texture2D::assetFactory)->getAsset("resources/textures/noise.bmp", assetNoise);

			std::shared_ptr<Shader> grassRenderShader(new Shader());

			grassRenderShader->createAsset("NONE resources/shaders/voxel/gBufferRenderGrass.vert resources/shaders/voxel/gBufferRenderGrass.frag");

			std::shared_ptr<Shader> depthRenderShader(new Shader());

			depthRenderShader->createAsset("NONE resources/shaders/voxel/depthRenderGrass.vert resources/shaders/voxel/depthRenderGrass.frag");

			batcher->create(std::static_pointer_cast<Texture2D>(assetGrassDiffuse), std::static_pointer_cast<Texture2D>(assetGrassNormal), std::static_pointer_cast<Texture2D>(assetNoise), grassRenderShader, depthRenderShader);

			grassBatcher = batcher.get();
		}

		grass->create(grassPositions, startf, grassBatcher, 4, 1, 4, 0.7f);
	}
}

void pge::terrainGeneratorFlatlands(VoxelChunk &chunk, int seed) {
	NoiseGenerator generator;
	generator.seed = seed;

	Point3i start((chunk.getTerrain()->getCenter() + chunk.getCenterRelativePosition()) * VoxelChunk::chunkSize);
	Point3i centerOffset(chunk.getCenterRelativePosition() * VoxelChunk::chunkSize);
	Vec3f startf(static_cast<float>(start.x), static_cast<float>(start.y), static_cast<float>(start.z));
	Vec3f centerf(0.0f, 0.0f, 0.0f);

	for (int x = 0; x < VoxelChunk::chunkSize; x++)
	for (int y = 0; y < VoxelChunk::chunkSize; y++)
	for (int z = 0; z < VoxelChunk::chunkSize; z++) {
		Vec3f worldPos(static_cast<float>(x + start.x), static_cast<float>(y + start.y), static_cast<float>(z + start.z));

		//float value = clamp((worldPos - centerf).magnitude() - 20.0f, -1.0f, 1.0f);
		float value = clamp(6.0f * worldPos.y / static_cast<float>(chunk.getTerrain()->getSize().y * VoxelChunk::chunkSize), -1.0f, 1.0f);

		chunk.setVoxel(Point3i(x, y, z), static_cast<voxelType>(value * 127.0f));
	}
}