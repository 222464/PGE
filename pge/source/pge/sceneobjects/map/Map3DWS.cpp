#include <pge/sceneobjects/map/Map3DWS.h>

#include <pge/rendering/model/SceneObjectStaticModelBatcher.h>

#include <pge/rendering/lighting/SceneObjectPointLight.h>
#include <pge/rendering/lighting/SceneObjectPointLightShadowed.h>

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

	fromFile.read(reinterpret_cast<char*>(&header._mapVersion), sizeof(Word));
	fromFile.read(reinterpret_cast<char*>(&header._mapFlags), sizeof(Byte));
	fromFile.read(reinterpret_cast<char*>(&header._nameCount), sizeof(Long));
	fromFile.read(reinterpret_cast<char*>(&header._nameOffset), sizeof(Long));
	fromFile.read(reinterpret_cast<char*>(&header._objectCount), sizeof(Long));
	fromFile.read(reinterpret_cast<char*>(&header._objectOffset), sizeof(Long));

	//fromFile.read(reinterpret_cast<char*>(&header), sizeof(MapHeader));

	// Read name table
	fromFile.seekg(header._nameOffset);

	_nameTable.clear();
	_nameTable.reserve(header._nameCount);

	for (Long i = 0; i < header._nameCount; i++) {
		std::string name;
		std::getline(fromFile, name, '\0');
		_nameTable.push_back(name);
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

	objectTable.reserve(header._objectCount);

	Long offset = 0;

	for (Long oi = 0; oi < header._objectCount; oi++) {
		fromFile.seekg(header._objectOffset + offset);

		MapObjectTableEntry entry;

		fromFile.read(reinterpret_cast<char*>(&entry._objectClass), sizeof(Name));
		fromFile.read(reinterpret_cast<char*>(&entry._dataSize), sizeof(Long));

		offset += entry._dataSize + sizeof(Name) + sizeof(Long);

		objectTable.push_back(entry);

		std::string nameEntry = _nameTable[entry._objectClass - 1];

		if (nameEntry == "material") {
			// Load material
			MapMaterial material;

			fromFile.read(reinterpret_cast<char*>(&material._flags), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&material._groupName), sizeof(Name));
			fromFile.read(reinterpret_cast<char*>(&material._objectName), sizeof(Name));

			if (material._flags & 2)
				fromFile.read(reinterpret_cast<char*>(&material._extensionName), sizeof(Name));
			
			materials.push_back(material);
		}
		else if (nameEntry == "meshreference") {
			MapMeshReference meshReference;

			fromFile.read(reinterpret_cast<char*>(&meshReference._flags), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&meshReference._groupName), sizeof(Name));
			fromFile.read(reinterpret_cast<char*>(&meshReference._objectName), sizeof(Name));
			fromFile.read(reinterpret_cast<char*>(&meshReference._limbCount), sizeof(Byte));

			meshReferences.push_back(meshReference);
		}
		else if (nameEntry == "lightmap") {
			MapLightmap lightMap;

			fromFile.read(reinterpret_cast<char*>(&lightMap._flags), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&lightMap._resolution), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&lightMap._format), sizeof(Long));

			int fullResolution = std::pow(2, lightMap._resolution);
			fullResolution *= fullResolution; // Square

			lightMap._pixels.resize(fullResolution);

			for (int i = 0; i < fullResolution; i++)
				fromFile.read(reinterpret_cast<char*>(&lightMap._pixels[i]), sizeof(Color3));
		}
		else if (nameEntry == "brush") {
			MapBrush brush;
			
			fromFile.read(reinterpret_cast<char*>(&brush._flags), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&brush._keys), sizeof(Long));

			for (Long i = 0; i < brush._keys; i++) {
				Name key;
				Name value;

				fromFile.read(reinterpret_cast<char*>(&key), sizeof(Name));
				fromFile.read(reinterpret_cast<char*>(&value), sizeof(Name));

				brush._keyValue[key] = value;
			}

			Long longDummy;
			Byte byteDummy;

			fromFile.read(reinterpret_cast<char*>(&longDummy), sizeof(Long));
			fromFile.read(reinterpret_cast<char*>(&longDummy), sizeof(Long));
			fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));

			fromFile.read(reinterpret_cast<char*>(&brush._vertexCount), sizeof(Byte));

			brush._vertexPositions.resize(brush._vertexCount);

			for (Byte i = 0; i < brush._vertexCount; i++) {
				fromFile.read(reinterpret_cast<char*>(&brush._vertexPositions[i].x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&brush._vertexPositions[i].y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&brush._vertexPositions[i].z), sizeof(float));
			}

			fromFile.read(reinterpret_cast<char*>(&brush._faceCount), sizeof(Byte));

			brush._faces.reserve(brush._faceCount);

			for (Byte i = 0; i < brush._faceCount; i++) {
				MapFace face;

				fromFile.read(reinterpret_cast<char*>(&face._flags), sizeof(Byte));
				fromFile.read(reinterpret_cast<char*>(&face._planeEquation.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._planeEquation.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._planeEquation.z), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._planeEquation.w), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._texturePosition.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._texturePosition.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._textureScale.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._textureScale.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._textureRotation.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._textureRotation.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._UTextureMappingPlane.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._UTextureMappingPlane.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._UTextureMappingPlane.z), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._UTextureMappingPlane.w), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._VTextureMappingPlane.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._VTextureMappingPlane.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._VTextureMappingPlane.z), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._VTextureMappingPlane.w), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._lumelSize), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&face._smoothGroupIndex), sizeof(Long));
				fromFile.read(reinterpret_cast<char*>(&face._materialIndex), sizeof(Long));
				
				if (face._flags & 16) {
					fromFile.read(reinterpret_cast<char*>(&face._lightmapIndex), sizeof(Long));

					fromFile.read(reinterpret_cast<char*>(&face._indexCount), sizeof(Byte));

					face._partialVerticesLightmapped.resize(face._indexCount);

					for (Byte j = 0; j < face._indexCount; j++) {
						MapPartialVertexLightmapped partialVertex;

						fromFile.read(reinterpret_cast<char*>(&partialVertex._vertex), sizeof(Byte));
						fromFile.read(reinterpret_cast<char*>(&partialVertex._texCoords.x), sizeof(float));
						fromFile.read(reinterpret_cast<char*>(&partialVertex._texCoords.y), sizeof(float));
						fromFile.read(reinterpret_cast<char*>(&partialVertex._lightmapCoords.x), sizeof(float));
						fromFile.read(reinterpret_cast<char*>(&partialVertex._lightmapCoords.y), sizeof(float));

						face._partialVerticesLightmapped[j] = partialVertex;
					}
				}
				else {
					fromFile.read(reinterpret_cast<char*>(&face._indexCount), sizeof(Byte));

					face._partialVertices.resize(face._indexCount);

					for (Byte j = 0; j < face._indexCount; j++) {
						MapPartialVertex partialVertex;

						fromFile.read(reinterpret_cast<char*>(&partialVertex._vertex), sizeof(Byte));
						fromFile.read(reinterpret_cast<char*>(&partialVertex._texCoords.x), sizeof(float));
						fromFile.read(reinterpret_cast<char*>(&partialVertex._texCoords.y), sizeof(float));
	
						face._partialVertices[j] = partialVertex;
					}
				}

				brush._faces.push_back(face);
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

			fromFile.read(reinterpret_cast<char*>(&entity._flags), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&entity._position.x), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&entity._position.y), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&entity._position.z), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&entity._keys), sizeof(Long));

			for (Long i = 0; i < entity._keys; i++) {
				Name key;
				Name value;

				fromFile.read(reinterpret_cast<char*>(&key), sizeof(Name));
				fromFile.read(reinterpret_cast<char*>(&value), sizeof(Name));

				entity._keyValue[key] = value;

				entity._properties[_nameTable[key - 1]] = _nameTable[value - 1];
			}

			// Read unecessary data away
			Long longDummy;
	
			fromFile.read(reinterpret_cast<char*>(&longDummy), sizeof(Long));
			fromFile.read(reinterpret_cast<char*>(&longDummy), sizeof(Long));

			_entities.push_back(entity);
		}
	}

	// Revisit skipped locations (this was done since the object was likely dependent on others)
	for (size_t oi = 0; oi < revisitObjects.size(); oi++) {
		fromFile.seekg(revisitLocations[oi]);

		if (_nameTable[objectTable[revisitObjects[oi]]._objectClass - 1] == "mesh") {
			MapMesh mesh;

			fromFile.read(reinterpret_cast<char*>(&mesh._flags), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&mesh._keys), sizeof(Long));

			for (Long i = 0; i < mesh._keys; i++) {
				Name key;
				Name value;

				fromFile.read(reinterpret_cast<char*>(&key), sizeof(Name));
				fromFile.read(reinterpret_cast<char*>(&value), sizeof(Name));

				mesh._keyValue[key] = value;
			}

			// Read unecessary data away
			Long longDummy;
			Byte byteDummy;

			fromFile.read(reinterpret_cast<char*>(&longDummy), sizeof(Long));
			fromFile.read(reinterpret_cast<char*>(&longDummy), sizeof(Long));
			fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
			fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));

			fromFile.read(reinterpret_cast<char*>(&mesh._meshReferenceIndex), sizeof(Long));
			fromFile.read(reinterpret_cast<char*>(&mesh._position.x), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&mesh._position.y), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&mesh._position.z), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&mesh._rotation.x), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&mesh._rotation.y), sizeof(float));
			fromFile.read(reinterpret_cast<char*>(&mesh._rotation.z), sizeof(float));

			if (!(mesh._flags & 1)) {
				fromFile.read(reinterpret_cast<char*>(&mesh._scale.x), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&mesh._scale.y), sizeof(float));
				fromFile.read(reinterpret_cast<char*>(&mesh._scale.z), sizeof(float));
			}
			else
				mesh._scale = Vec3(1.0f, 1.0f, 1.0f);

			mesh._limbs.reserve(meshReferences[mesh._meshReferenceIndex]._limbCount);

			for (Long i = 0; i < meshReferences[mesh._meshReferenceIndex]._limbCount; i++) {
				MapLimb limb;

				fromFile.read(reinterpret_cast<char*>(&limb._materialIndex), sizeof(Long));

				if (mesh._flags & 16) {
					// Read unecessary data away
					Word wordDummy;

					fromFile.read(reinterpret_cast<char*>(&wordDummy), sizeof(Word));

					for (Word j = 0; j < wordDummy; j++) {
						fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
						fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
						fromFile.read(reinterpret_cast<char*>(&byteDummy), sizeof(Byte));
					}
				}

				mesh._limbs.push_back(limb);
			}

			meshes.push_back(mesh);
		}
	}

	fromFile.close();

	// -------------------------------- Generate Map ---------------------------------

	assert(_settings._pScene != nullptr);
	
	if (_settings._useGraphics) {
		RenderScene* pRenderScene = dynamic_cast<RenderScene*>(_settings._pScene);

		assert(pRenderScene != nullptr);

		// Create chunk
		std::shared_ptr<Map3DWSChunk> chunk(new Map3DWSChunk());

		_settings._pScene->add(chunk, false);

		// Load materials
		std::unordered_map<std::string, size_t> matNameToIndex;

		chunk->_model.reset(new StaticModel());

		std::string materialFileName = name.substr(0, name.length() - 3) + "mtl";

		if (!Material::loadFromMTL(materialFileName, chunk->_textureManager.get(), matNameToIndex, chunk->_model->_materials)) {
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
				std::string materialName = _nameTable[materials[i]._objectName - 1];
				std::string materialExtension = (materials[i]._flags & 2) ? _nameTable[materials[i]._extensionName - 1] : _settings._defaultImageFileExtension;

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

			if (!Material::loadFromMTL(materialFileName, chunk->_textureManager.get(), matNameToIndex, chunk->_model->_materials)) {
#ifdef PGE_DEBUG
				std::cerr << "Could not open generated file" << materialFileName << std::endl;
#endif
				return false;
			}
		}

		for (size_t i = 0; i < chunk->_model->_materials.size(); i++)
			chunk->_model->_materials[i].genMipMaps();

		// Create a mesh for each material
		std::vector<std::shared_ptr<StaticMesh>> materialMeshes;

		materialMeshes.resize(chunk->_model->_materials.size());

		for (size_t i = 0; i < materialMeshes.size(); i++)
			materialMeshes[i].reset(new StaticMesh());

		for (size_t bi = 0; bi < brushes.size(); bi++) {
			MapBrush &b = brushes[bi];

			for (Long fi = 0; fi < b._faceCount; fi++) {
				MapFace &f = b._faces[fi];

				size_t materialIndex = matNameToIndex[_nameTable[materials[f._materialIndex - 1]._objectName - 1]];

				size_t prevNumVertices = materialMeshes[materialIndex]->_vertices.size();

				if (!f._partialVerticesLightmapped.empty()) {
					// Add to the material mesh
					for (size_t vi = 0; vi < f._partialVerticesLightmapped.size(); vi++) {
						StaticMesh::Vertex v;

						v._position = _settings._sizeScalar * b._vertexPositions[f._partialVerticesLightmapped[vi]._vertex];
						v._texCoord = f._partialVerticesLightmapped[vi]._texCoords;
						v._normal = Vec3f(1.0f, 0.0f, 0.0f);

						materialMeshes[materialIndex]->_vertices.push_back(v);
					}
				}
				else {
					// Add to the material mesh
					for (size_t vi = 0; vi < f._partialVertices.size(); vi++) {
						StaticMesh::Vertex v;

						v._position = _settings._sizeScalar * b._vertexPositions[f._partialVertices[vi]._vertex];
						v._texCoord = f._partialVertices[vi]._texCoords;
						v._normal = Vec3f(1.0f, 0.0f, 0.0f);

						materialMeshes[materialIndex]->_vertices.push_back(v);
					}
				}

				// Add indices (triangle fan shape)
				Byte indexCount = f._indexCount;

				for (Byte t = indexCount - 2; t > 0; t--) {
					staticMeshIndexType index0 = prevNumVertices + indexCount - 1;
					staticMeshIndexType index1 = prevNumVertices + t;
					staticMeshIndexType index2 = prevNumVertices + t - 1;

					materialMeshes[materialIndex]->_indices.push_back(index0);
					materialMeshes[materialIndex]->_indices.push_back(index1);
					materialMeshes[materialIndex]->_indices.push_back(index2);

					// Calculate normals
					Vec3f normal = (materialMeshes[materialIndex]->_vertices[index1]._position - materialMeshes[materialIndex]->_vertices[index0]._position).cross(materialMeshes[materialIndex]->_vertices[index2]._position - materialMeshes[materialIndex]->_vertices[index0]._position).normalized();
				
					materialMeshes[materialIndex]->_vertices[index0]._normal = normal;
					materialMeshes[materialIndex]->_vertices[index1]._normal = normal;
					materialMeshes[materialIndex]->_vertices[index2]._normal = normal;
				}
			}
		}

		for (size_t i = 0; i < materialMeshes.size(); i++) {
			if (materialMeshes[i]->_vertices.empty())
				continue;

			chunk->_model->_meshes.push_back(StaticModel::StaticMeshAndMaterialIndex(materialMeshes[i], i));
			chunk->_model->_meshes.back()._mesh->create(true);
			chunk->_model->_meshes.back()._mesh->updateBuffers();
		}

		for (size_t i = 0; i < chunk->_model->_materials.size(); i++) {
			chunk->_model->_materials[i].createUniformBuffer(pRenderScene->getMaterialUBOShaderInterface(RenderScene::_standard));
			chunk->_model->_materials[i].setUniformsBuffer(pRenderScene->getMaterialUBOShaderInterface(RenderScene::_standard));
		}
	}

	return true;
}

void Map3DWSChunk::onAdd() {
	_batcherRef = getScene()->getNamed("smb");

	assert(_batcherRef.isAlive());
}

void Map3DWSChunk::deferredRender() {
	pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(_batcherRef.get());

	_model->render(pBatcher, Matrix4x4f::identityMatrix());
}

void Map3DWSPhysics::onAdd() {

}

void Map3DWSPhysics::onDestroy() {

}

void pge::addMapLights(const Map3DWS &map, Scene* pScene) {
	SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(pScene->getNamed("lighting").get());

	const float colorInv = 1.0f / 255.0f;

	for (size_t i = 0; i < map._entities.size(); i++) {
		const Map3DWS::MapEntity &entity = map._entities[i];

		if (entity._properties.find("classname")->second == "light") {
			// Add a light
			std::shared_ptr<SceneObjectPointLightShadowed> pointLightShadowed(new SceneObjectPointLightShadowed());

			pScene->add(pointLightShadowed, true);

			pointLightShadowed->create(pLighting, 256);

			float intensity = 1.0f;
			Vec3f color = Vec3f(1.0f, 1.0f, 1.0f);
			float range = 800.0f;

			{
				std::string intensityS = entity._properties.find("intensity")->second;

				std::istringstream reader(intensityS);

				float intensity;

				reader >> intensity;
			}

			{
				std::string rangeS = entity._properties.find("range")->second;

				std::istringstream reader(rangeS);

				reader >> range;
			}

			{
				std::string colorS = entity._properties.find("color")->second;

				std::istringstream reader(colorS);

				int r, g, b;

				reader >> r >> g >> b;

				color = Vec3f(r * colorInv, g * colorInv, b * colorInv);
			}

			pointLightShadowed->setColor(color * intensity * 0.5f);
			pointLightShadowed->setRange(range * map._settings._sizeScalar); // * map._settings._sizeScalar
			pointLightShadowed->setPosition(entity._position * map._settings._sizeScalar);
		}
	}
}