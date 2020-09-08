#include "Map3DWS.h"

#include "../../rendering/model/SceneObjectStaticModelBatcher.h"

#include "../../rendering/lighting/SceneObjectPointLight.h"
#include "../../rendering/lighting/SceneObjectPointLightShadowed.h"

#include <fstream>
#include <sstream>

using namespace pge;

bool Map3DWS::createAsset(const std::string &name) {
	std::ifstream fromFile(name, std::ios::binary);

	if (!fromFile.is_open()) {
#ifdef PGE_DEBUG
		std::cerr << "Could not open file " << name << std::endl;
#endif

		return false;
	}

	fromFile.seekg(0);

	// Read header
	MapHeader header;

	fromFile.read(reinterpret_cast<char*>(&header.mapVersion), sizeof(Word));
	fromFile.read(reinterpret_cast<char*>(&header.mapFlags), sizeof(Byte));
	fromFile.read(reinterpret_cast<char*>(&header.nameCount), sizeof(Long));
	fromFile.read(reinterpret_cast<char*>(&header.nameOffset), sizeof(Long));
	fromFile.read(reinterpret_cast<char*>(&header.objectCount), sizeof(Long));
	fromFile.read(reinterpret_cast<char*>(&header.objectOffset), sizeof(Long));

	//fromFile.read(reinterpret_cast<char*>(&header), sizeof(MapHeader));

	// Read name table
	fromFile.seekg(header.nameOffset);

	nameTable.clear();
	nameTable.reserve(header.nameCount);

	for (Long i = 0; i < header.nameCount; i++) {
		std::string name;
		std::getline(fromFile, name, '\0');
		nameTable.push_back(name);
	}

	// ------------------------------------ Load Objects ------------------------------------

	// Storage
	std::vector<MapObjectTableEntry> objectTable;
	std::vector<MapMaterial> materials;
	std::vector<MapMeshReference> meshReferences;
	std::vector<MapBrush> brushes;
	std::vector<MapMesh> meshes;
	
	// For revisiting
	std::vector<size_t> revisitObjects;
	std::vector<size_t> revisitLocations;

	objectTable.reserve(header.objectCount);

	Long offset = 0;

	for (Long oi = 0; oi < header.objectCount; oi++) {
		fromFile.seekg(header.objectOffset + offset);

		MapObjectTableEntry entry;

		fromFile.read(reinterpret_cast<char*>(&entry.objectClass), sizeof(Name));
		fromFile.read(reinterpret_cast<char*>(&entry.dataSize), sizeof(Long));

		offset += entry.dataSize + sizeof(Name) + sizeof(Long);

		objectTable.push_back(entry);

		std::string nameEntry = nameTable[entry.objectClass - 1];

		if (nameEntry == "material") {
			// Load material
			MapMaterial material;

			fromFile.read(reinterpret_cast<char*>(&material.flags), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&material.groupName), sizeof(Name));
			fromFile.read(reinterpret_cast<char*>(&material.objectName), sizeof(Name));

			if (material.flags & 2)
				fromFile.read(reinterpret_cast<char*>(&material.extensionName), sizeof(Name));
			
			materials.push_back(material);
		}
		else if (nameEntry == "meshreference") {
			MapMeshReference meshReference;

			fromFile.read(reinterpret_cast<char*>(&meshReference.flags), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&meshReference.groupName), sizeof(Name));
			fromFile.read(reinterpret_cast<char*>(&meshReference.objectName), sizeof(Name));
			fromFile.read(reinterpret_cast<char*>(&meshReference.limbCount), sizeof(Byte));

			meshReferences.push_back(meshReference);
		}
		else if (nameEntry == "lightmap") {
			MapLightmap lightMap;

			fromFile.read(reinterpret_cast<char*>(&lightMap.flags), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&lightMap.resolution), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&lightMap.format), sizeof(Long));

			int fullResolution = std::pow(2, lightMap.resolution);
			fullResolution *= fullResolution; // Square

			lightMap.pixels.resize(fullResolution);

			for (int i = 0; i < fullResolution; i++)
				fromFile.read(reinterpret_cast<char*>(&lightMap.pixels[i]), sizeof(Color3));
		}
		else if (nameEntry == "brush") {
			MapBrush brush;
			
			fromFile.read(reinterpret_cast<char*>(&brush.flags), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&brush.keys), sizeof(Long));

			for (Long i = 0; i < brush.keys; i++) {
				Name key;
				Name value;

				fromFile.read(reinterpret_cast<char*>(&key), sizeof(Name));
				fromFile.read(reinterpret_cast<char*>(&value), sizeof(Name));

				brush.keyValue[key] = value;
			}

			Long longDummy;
			Byte byteDummy;

			fromFile.read(reinterpret_cast<char*>(&longDummy), sizeof(Long));
			fromFile.read(reinterpret_cast<char*>(&longDummy), sizeof(Long));
			fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));

			fromFile.read(reinterpret_cast<char*>(&brush.vertexCount), sizeof(Byte));

			brush.vertexPositions.resize(brush.vertexCount);

			for (Byte i = 0; i < brush.vertexCount; i++) {
				fromFile.read(reinterpret_cast<char*>(&brush.vertexPositions[i].x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&brush.vertexPositions[i].y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&brush.vertexPositions[i].z), sizeof(float));
			}

			fromFile.read(reinterpret_cast<char*>(&brush.faceCount), sizeof(Byte));

			brush.faces.reserve(brush.faceCount);

			for (Byte i = 0; i < brush.faceCount; i++) {
				MapFace face;

				fromFile.read(reinterpret_cast<char*>(&face.flags), sizeof(Byte));
				fromFile.read(reinterpret_cast<char*>(&face.planeEquation.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.planeEquation.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.planeEquation.z), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.planeEquation.w), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.texturePosition.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.texturePosition.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.textureScale.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.textureScale.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.textureRotation.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.textureRotation.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.UTextureMappingPlane.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.UTextureMappingPlane.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.UTextureMappingPlane.z), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.UTextureMappingPlane.w), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.VTextureMappingPlane.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.VTextureMappingPlane.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.VTextureMappingPlane.z), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.VTextureMappingPlane.w), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.lumelSize), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face.smoothGroupIndex), sizeof(Long));
				fromFile.read(reinterpret_cast<char*>(&face.materialIndex), sizeof(Long));
				
				if (face.flags & 16) {
					fromFile.read(reinterpret_cast<char*>(&face.lightmapIndex), sizeof(Long));

					fromFile.read(reinterpret_cast<char*>(&face.indexCount), sizeof(Byte));

					face.partialVerticesLightmapped.resize(face.indexCount);

					for (Byte j = 0; j < face.indexCount; j++) {
						MapPartialVertexLightmapped partialVertex;

						fromFile.read(reinterpret_cast<char*>(&partialVertex.vertex), sizeof(Byte));
						fromFile.read(reinterpret_cast<char*>(&partialVertex.texCoords.x), sizeof(float));
						fromFile.read(reinterpret_cast<char*>(&partialVertex.texCoords.y), sizeof(float));
						fromFile.read(reinterpret_cast<char*>(&partialVertex.lightmapCoords.x), sizeof(float));
						fromFile.read(reinterpret_cast<char*>(&partialVertex.lightmapCoords.y), sizeof(float));

						face.partialVerticesLightmapped[j] = partialVertex;
					}
				}
				else {
					fromFile.read(reinterpret_cast<char*>(&face.indexCount), sizeof(Byte));

					face.partialVertices.resize(face.indexCount);

					for (Byte j = 0; j < face.indexCount; j++) {
						MapPartialVertex partialVertex;

						fromFile.read(reinterpret_cast<char*>(&partialVertex.vertex), sizeof(Byte));
						fromFile.read(reinterpret_cast<char*>(&partialVertex.texCoords.x), sizeof(float));
						fromFile.read(reinterpret_cast<char*>(&partialVertex.texCoords.y), sizeof(float));
	
						face.partialVertices[j] = partialVertex;
					}
				}

				brush.faces.push_back(face);
			}
			
			brushes.push_back(brush);
		}
		else if (nameEntry == "mesh") {
			// Revisit
			revisitObjects.push_back(objectTable.size() - 1);
			revisitLocations.push_back(fromFile.tellg());
		}
		else if (nameEntry == "entity") {
			MapEntity entity;

			fromFile.read(reinterpret_cast<char*>(&entity.flags), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&entity.position.x), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&entity.position.y), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&entity.position.z), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&entity.keys), sizeof(Long));

			for (Long i = 0; i < entity.keys; i++) {
				Name key;
				Name value;

				fromFile.read(reinterpret_cast<char*>(&key), sizeof(Name));
				fromFile.read(reinterpret_cast<char*>(&value), sizeof(Name));

				entity.keyValue[key] = value;

				entity.properties[nameTable[key - 1]] = nameTable[value - 1];
			}

			// Read unecessary data away
			Long longDummy;
	
			fromFile.read(reinterpret_cast<char*>(&longDummy), sizeof(Long));
			fromFile.read(reinterpret_cast<char*>(&longDummy), sizeof(Long));

			entities.push_back(entity);
		}
	}

	// Revisit skipped locations (this was done since the object was likely dependent on others)
	for (size_t oi = 0; oi < revisitObjects.size(); oi++) {
		fromFile.seekg(revisitLocations[oi]);

		if (nameTable[objectTable[revisitObjects[oi]].objectClass - 1] == "mesh") {
			MapMesh mesh;

			fromFile.read(reinterpret_cast<char*>(&mesh.flags), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&mesh.keys), sizeof(Long));

			for (Long i = 0; i < mesh.keys; i++) {
				Name key;
				Name value;

				fromFile.read(reinterpret_cast<char*>(&key), sizeof(Name));
				fromFile.read(reinterpret_cast<char*>(&value), sizeof(Name));

				mesh.keyValue[key] = value;
			}

			// Read unecessary data away
			Long longDummy;
			Byte byteDummy;

			fromFile.read(reinterpret_cast<char*>(&longDummy), sizeof(Long));
			fromFile.read(reinterpret_cast<char*>(&longDummy), sizeof(Long));
			fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));

			fromFile.read(reinterpret_cast<char*>(&mesh.meshReferenceIndex), sizeof(Long));
			fromFile.read(reinterpret_cast<char*>(&mesh.position.x), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&mesh.position.y), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&mesh.position.z), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&mesh.rotation.x), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&mesh.rotation.y), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&mesh.rotation.z), sizeof(float));

			if (!(mesh.flags & 1)) {
				fromFile.read(reinterpret_cast<char*>(&mesh.scale.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&mesh.scale.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&mesh.scale.z), sizeof(float));
			}
			else
				mesh.scale = Vec3(1.0f, 1.0f, 1.0f);

			mesh.limbs.reserve(meshReferences[mesh.meshReferenceIndex].limbCount);

			for (Long i = 0; i < meshReferences[mesh.meshReferenceIndex].limbCount; i++) {
				MapLimb limb;

				fromFile.read(reinterpret_cast<char*>(&limb.materialIndex), sizeof(Long));

				if (mesh.flags & 16) {
					// Read unecessary data away
					Word wordDummy;

					fromFile.read(reinterpret_cast<char*>(&wordDummy), sizeof(Word));

					for (Word j = 0; j < wordDummy; j++) {
						fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
						fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
						fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
					}
				}

				mesh.limbs.push_back(limb);
			}

			meshes.push_back(mesh);
		}
	}

	fromFile.close();

	// -------------------------------- Generate Map ---------------------------------

	assert(settings.pScene != nullptr);
	
	if (settings.useGraphics) {
		RenderScene* pRenderScene = dynamic_cast<RenderScene*>(settings.pScene);

		assert(pRenderScene != nullptr);

		// Create chunk
		std::shared_ptr<Map3DWSChunk> chunk(new Map3DWSChunk());

		settings.pScene->add(chunk, false);

		// Load materials
		std::unordered_map<std::string, size_t> matNameToIndex;

		chunk->model.reset(new StaticModel());

		std::string materialFileName = name.substr(0, name.length() - 3) + "mtl";

		if (!Material::loadFromMTL(materialFileName, chunk->textureManager.get(), matNameToIndex, chunk->model->materials)) {
#ifdef PGE_DEBUG
			std::cerr << "Could not open file " << materialFileName << ". Generating the file." << std::endl;
#endif

			std::ofstream toFile(materialFileName);

			if (!toFile.is_open()) {
#ifdef PGE_DEBUG
				std::cerr << "Could not open file " << materialFileName << " for generating!" << std::endl;
#endif

				return false;
			}

			// Write materials, full color diffuse only
			toFile << "# Generated 3DWS Material File" << std::endl << std::endl;

			// Generate materials
			for (size_t i = 0; i < materials.size(); i++) {
				std::string materialName = nameTable[materials[i].objectName - 1];
				std::string materialExtension = (materials[i].flags & 2) ? nameTable[materials[i].extensionName - 1] : settings.defaultImageFileExtension;

				toFile << "newmtl " << materialName << std::endl;
				toFile << "Ns 96.0" << std::endl;
				toFile << "Ka 0.0 0.0 0.0" << std::endl;
				toFile << "Kd 1.0 1.0 1.0" << std::endl;
				toFile << "Ks 0.0 0.0 0.0" << std::endl;
				toFile << "Ni 1.0" << std::endl;
				toFile << "d 1.0" << std::endl;
				toFile << "illum 2" << std::endl;
				toFile << "map_Kd " << materialName << materialExtension << std::endl << std::endl;
			}

			toFile.close();

			if (!Material::loadFromMTL(materialFileName, chunk->textureManager.get(), matNameToIndex, chunk->model->materials)) {
#ifdef PGE_DEBUG
				std::cerr << "Could not open generated file" << materialFileName << std::endl;
#endif
				return false;
			}
		}

		for (size_t i = 0; i < chunk->model->materials.size(); i++)
			chunk->model->materials[i].genMipMaps();

		// Create a mesh for each material
		std::vector<std::shared_ptr<StaticMesh>> materialMeshes;

		materialMeshes.resize(chunk->model->materials.size());

		for (size_t i = 0; i < materialMeshes.size(); i++)
			materialMeshes[i].reset(new StaticMesh());

		for (size_t bi = 0; bi < brushes.size(); bi++) {
			MapBrush &b = brushes[bi];

			for (Long fi = 0; fi < b.faceCount; fi++) {
				MapFace &f = b.faces[fi];

				size_t materialIndex = matNameToIndex[nameTable[materials[f.materialIndex - 1].objectName - 1]];

				size_t prevNumVertices = materialMeshes[materialIndex]->vertices.size();

				if (!f.partialVerticesLightmapped.empty()) {
					// Add to the material mesh
					for (size_t vi = 0; vi < f.partialVerticesLightmapped.size(); vi++) {
						StaticMesh::Vertex v;

						v.position = settings.sizeScalar * b.vertexPositions[f.partialVerticesLightmapped[vi].vertex];
						v.texCoord = f.partialVerticesLightmapped[vi].texCoords;
						v.normal = Vec3f(1.0f, 0.0f, 0.0f);

						materialMeshes[materialIndex]->vertices.push_back(v);
					}
				}
				else {
					// Add to the material mesh
					for (size_t vi = 0; vi < f.partialVertices.size(); vi++) {
						StaticMesh::Vertex v;

						v.position = settings.sizeScalar * b.vertexPositions[f.partialVertices[vi].vertex];
						v.texCoord = f.partialVertices[vi].texCoords;
						v.normal = Vec3f(1.0f, 0.0f, 0.0f);

						materialMeshes[materialIndex]->vertices.push_back(v);
					}
				}

				// Add indices (triangle fan shape)
				Byte indexCount = f.indexCount;

				for (Byte t = indexCount - 2; t > 0; t--) {
					staticMeshIndexType index0 = prevNumVertices + indexCount - 1;
					staticMeshIndexType index1 = prevNumVertices + t;
					staticMeshIndexType index2 = prevNumVertices + t - 1;

					materialMeshes[materialIndex]->indices.push_back(index0);
					materialMeshes[materialIndex]->indices.push_back(index1);
					materialMeshes[materialIndex]->indices.push_back(index2);

					// Calculate normals
					Vec3f normal = (materialMeshes[materialIndex]->vertices[index1].position - materialMeshes[materialIndex]->vertices[index0].position).cross(materialMeshes[materialIndex]->vertices[index2].position - materialMeshes[materialIndex]->vertices[index0].position).normalized();
				
					materialMeshes[materialIndex]->vertices[index0].normal = normal;
					materialMeshes[materialIndex]->vertices[index1].normal = normal;
					materialMeshes[materialIndex]->vertices[index2].normal = normal;
				}
			}
		}

		for (size_t i = 0; i < materialMeshes.size(); i++) {
			if (materialMeshes[i]->vertices.empty())
				continue;

			chunk->model->meshes.push_back(StaticModel::StaticMeshAndMaterialIndex(materialMeshes[i], i));
			chunk->model->meshes.back().mesh->create(true);
			chunk->model->meshes.back().mesh->updateBuffers();
		}

		for (size_t i = 0; i < chunk->model->materials.size(); i++) {
			chunk->model->materials[i].createUniformBuffer(pRenderScene->getMaterialUBOShaderInterface(RenderScene::standard));
			chunk->model->materials[i].setUniformsBuffer(pRenderScene->getMaterialUBOShaderInterface(RenderScene::standard));
		}
	}

	return true;
}

void Map3DWSChunk::onAdd() {
	batcherRef = getScene()->getNamed("smb");

	assert(batcherRef.isAlive());
}

void Map3DWSChunk::deferredRender() {
	pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(batcherRef.get());

	model->render(pBatcher, Matrix4x4f::identityMatrix());
}

void Map3DWSPhysics::onAdd() {

}

void Map3DWSPhysics::onDestroy() {

}

void pge::addMapLights(const Map3DWS &map, Scene* pScene) {
	SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(pScene->getNamed("lighting").get());

	const float colorInv = 1.0f / 255.0f;

	for (size_t i = 0; i < map.entities.size(); i++) {
		const Map3DWS::MapEntity &entity = map.entities[i];

		if (entity.properties.find("classname")->second == "light") {
			// Add a light
			std::shared_ptr<SceneObjectPointLightShadowed> pointLightShadowed(new SceneObjectPointLightShadowed());

			pScene->add(pointLightShadowed, true);

			pointLightShadowed->create(pLighting, 256);

			float intensity = 1.0f;
			Vec3f color = Vec3f(1.0f, 1.0f, 1.0f);
			float range = 800.0f;

			{
				std::string intensityS = entity.properties.find("intensity")->second;

				std::istringstream reader(intensityS);

				float intensity;

				reader >> intensity;
			}

			{
				std::string rangeS = entity.properties.find("range")->second;

				std::istringstream reader(rangeS);

				reader >> range;
			}

			{
				std::string colorS = entity.properties.find("color")->second;

				std::istringstream reader(colorS);

				int r, g, b;

				reader >> r >> g >> b;

				color = Vec3f(r * colorInv, g * colorInv, b * colorInv);
			}

			pointLightShadowed->setColor(color * intensity * 0.5f);
			pointLightShadowed->setRange(range * map.settings.sizeScalar); // * map.settings.sizeScalar
			pointLightShadowed->setPosition(entity.position * map.settings.sizeScalar);
		}
	}
}