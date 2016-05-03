#include <pge/rendering/model/StaticModel.h>

#include <pge/rendering/model/SceneObjectStaticModelBatcher.h>

#include <pge/util/Functions.h>

#include <fstream>
#include <sstream>

using namespace pge;

void StaticModel::render(RenderScene* pScene) {
	for (StaticMeshAndMaterialIndex &meshAndMaterial : _meshes) {
		pScene->useShader(_materials[meshAndMaterial._materialIndex]);
		meshAndMaterial._mesh->render();
	}
}

void StaticModel::render(SceneObjectStaticModelBatcher* pBatcher, const Matrix4x4f &transform) {
	pBatcher->_modelTransforms[this].push_back(transform);
}

void StaticModel::genMipMaps() {
	for (size_t i = 0; i < _materials.size(); i++)
		_materials[i].genMipMaps();
}

bool StaticModel::loadFromOBJ(const std::string &fileName, AssetManager &textureManager, AABB3D &aabb, bool useBuffers, bool clearArrays, RenderScene* pRenderScene) {
	std::ifstream fromFile(fileName);

	if (!fromFile.is_open()) {
#ifdef PGE_DEBUG
		std::cerr << "Could not load model file " << fileName << std::endl;
#endif
		return false;
	}

	std::string rootName(getRootName(fileName));

	std::vector<Vec3f> filePositions;
	std::vector<Vec2f> fileTexCoords;
	std::vector<Vec3f> fileNormals;

	// Hash map for linking indices to vertex array index for attributes
	std::unordered_map<StaticMeshIndexSet, size_t, StaticMeshIndexSet> indexToVertex;

	// Initial extremes
	aabb._lowerBound = Vec3f(999999.0f, 999999.0f, 999999.0f);
	aabb._upperBound = Vec3f(-999999.0f, -999999.0f, -999999.0f);

	std::unordered_map<std::string, size_t> matReferences;

	while (!fromFile.eof()) {
		// Read line header
		std::string line;
		getline(fromFile, line);

		std::stringstream ss(line);

		std::string header;
		ss >> header;

		if (header == "v") {
			// Add vertex
			float x, y, z;

			ss >> x >> y >> z;

			filePositions.push_back(Vec3f(x, y, z));

			aabb.expand(Vec3f(x, y, z));
		}
		else if (header == "vt") {
			// Add texture coordinate
			float s, t;

			ss >> s >> t;

#ifdef PGE_OBJ_FLIP_TEXCOORD_Y
			t = 1.0f - t;
#endif

			fileTexCoords.push_back(Vec2f(s, t));
		}
		else if (header == "vn") {
			// Add normal
			float nx, ny, nz;

			ss >> nx >> ny >> nz;

			fileNormals.push_back(Vec3f(nx, ny, nz));
		}
		else if (header == "f") {
			// Add a face
			std::vector<StaticMeshIndexSet> v;

			v.reserve(3);

			while (!ss.eof() && ss.good()) {
				StaticMeshIndexSet is;

				ss >> is._vi;
				ss.ignore(1, '/');
				ss >> is._ti;
				ss.ignore(1, '/');
				ss >> is._ni;

				v.push_back(is);
			}

			int vC = 0;
			staticMeshIndexType firstIndex;

			for (size_t i = 0; i < v.size(); i++) {
				// Search for index set 1
				std::unordered_map<StaticMeshIndexSet, size_t, StaticMeshIndexSet>::iterator it = indexToVertex.find(v[i]);

				if (it == indexToVertex.end()) {
					// Vertex attributes do not exist, create them

					// File indicies start at 1, so convert
					size_t vertIndex = v[i]._vi - 1;
					size_t texCoordIndex = v[i]._ti - 1;
					size_t normalIndex = v[i]._ni - 1;

					StaticMesh::Vertex vertex;
					vertex._position = filePositions[vertIndex];
					vertex._normal = fileNormals[normalIndex];
					vertex._texCoord = fileTexCoords[texCoordIndex];

					_meshes.back()._mesh->_vertices.push_back(vertex);
	
					// Index of vertex in vertex component array
					size_t realIndex = _meshes.back()._mesh->_vertices.size() - 1;

					// Add attribute set index to the map
					indexToVertex[v[i]] = realIndex;

					if (vC == 0)
						firstIndex = static_cast<staticMeshIndexType>(realIndex);
					else if (vC >= 3) {
						// Triangle fan
						staticMeshIndexType last = _meshes.back()._mesh->_indices.back();

						_meshes.back()._mesh->_indices.push_back(firstIndex);
						_meshes.back()._mesh->_indices.push_back(last);
					}

					vC++;

					_meshes.back()._mesh->_indices.push_back(static_cast<staticMeshIndexType>(realIndex));
				}
				else {
					if (vC == 0)
						firstIndex = static_cast<staticMeshIndexType>(it->second);
					else if (vC >= 3) {
						// Triangle fan
						staticMeshIndexType last = _meshes.back()._mesh->_indices.back();

						_meshes.back()._mesh->_indices.push_back(firstIndex);
						_meshes.back()._mesh->_indices.push_back(last);
					}

					vC++;

					_meshes.back()._mesh->_indices.push_back(static_cast<staticMeshIndexType>(it->second));

				}
			}
		}
		else if (header == "usemtl") {
			// Add new mesh
			_meshes.push_back(StaticMeshAndMaterialIndex());
			_meshes.back()._mesh.reset(new StaticMesh());

			// Get texture name and load it
			std::string matName;
			ss >> matName;

			// Link obj to material
			std::unordered_map<std::string, size_t>::iterator it = matReferences.find(matName);

			if (it == matReferences.end()) {
#ifdef PGE_DEBUG
				std::cerr << "Could not find material \"" << matName << "\"!" << std::endl;
#endif
				return false;
			}

			_meshes.back()._materialIndex = it->second;
		}
		else if (header == "mtllib") {
			std::string libName;
			ss >> libName;

			std::ostringstream fullMaterialLibraryName;

			fullMaterialLibraryName << rootName << libName;

			if (!Material::loadFromMTL(fullMaterialLibraryName.str(), &textureManager, matReferences, _materials)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}
		}
	}

	fromFile.close();

	aabb.calculateHalfDims();
	aabb.calculateCenter();

	for (StaticMeshAndMaterialIndex &meshAndMaterial : _meshes) {
		meshAndMaterial._mesh->create(useBuffers);

		if (useBuffers)
			meshAndMaterial._mesh->updateBuffers();

		if (clearArrays)
			meshAndMaterial._mesh->clearArrays();
	}

	if (pRenderScene != nullptr) {
		for (size_t i = 0; i < _materials.size(); i++) {
			_materials[i].createUniformBuffer(pRenderScene->getMaterialUBOShaderInterface(RenderScene::_standard));
			_materials[i].setUniformsBuffer(pRenderScene->getMaterialUBOShaderInterface(RenderScene::_standard));
		}
	}

	return true;
}