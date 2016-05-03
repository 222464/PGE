#include <pge/rendering/voxel/VoxelTerrain.h>

#include <pge/util/Math.h>

using namespace pge;

VoxelTerrain::VoxelTerrain()
: _size(0, 0, 0), _center(0, 0, 0),
_outsideMatrixVoxel(127),
_numChunkLODs(2), _LODSwitchDistance(60.0f),
_voxelScalar(0.03125f), _voxelSize(1.0f)
{
	// Generate normals
	_normals[0] = Vec3f(1.0f, 0.0f, 0.0f);
	_normals[1] = Vec3f(-1.0f, 0.0f, 0.0f);
	_normals[2] = Vec3f(0.0f, 1.0f, 0.0f);
	_normals[3] = Vec3f(0.0f, -1.0f, 0.0f);
	_normals[4] = Vec3f(0.0f, 0.0f, 1.0f);
	_normals[5] = Vec3f(0.0f, 0.0f, -1.0f);

	// Generate test offsets
	_positionOffsets[0] = Point3i(1, 0, 0);
	_positionOffsets[1] = Point3i(-1, 0, 0);
	_positionOffsets[2] = Point3i(0, 1, 0);
	_positionOffsets[3] = Point3i(0, -1, 0);
	_positionOffsets[4] = Point3i(0, 0, 1);
	_positionOffsets[5] = Point3i(0, 0, -1);

	// Generate corners
	const float cornerDist = 0.5f;

	_corners[0][0] = Vec3f(cornerDist, -cornerDist, cornerDist);
	_corners[0][1] = Vec3f(cornerDist, -cornerDist, -cornerDist);
	_corners[0][2] = Vec3f(cornerDist, cornerDist, -cornerDist);
	_corners[0][3] = Vec3f(cornerDist, cornerDist, cornerDist);

	_corners[1][0] = Vec3f(-cornerDist, -cornerDist, -cornerDist);
	_corners[1][1] = Vec3f(-cornerDist, -cornerDist, cornerDist);
	_corners[1][2] = Vec3f(-cornerDist, cornerDist, cornerDist);
	_corners[1][3] = Vec3f(-cornerDist, cornerDist, -cornerDist);

	_corners[2][0] = Vec3f(-cornerDist, cornerDist, -cornerDist);
	_corners[2][1] = Vec3f(-cornerDist, cornerDist, cornerDist);
	_corners[2][2] = Vec3f(cornerDist, cornerDist, cornerDist);
	_corners[2][3] = Vec3f(cornerDist, cornerDist, -cornerDist);

	_corners[3][0] = Vec3f(-cornerDist, -cornerDist, cornerDist);
	_corners[3][1] = Vec3f(-cornerDist, -cornerDist, -cornerDist);
	_corners[3][2] = Vec3f(cornerDist, -cornerDist, -cornerDist);
	_corners[3][3] = Vec3f(cornerDist, -cornerDist, cornerDist);

	_corners[4][0] = Vec3f(-cornerDist, -cornerDist, cornerDist);
	_corners[4][1] = Vec3f(cornerDist, -cornerDist, cornerDist);
	_corners[4][2] = Vec3f(cornerDist, cornerDist, cornerDist);
	_corners[4][3] = Vec3f(-cornerDist, cornerDist, cornerDist);

	_corners[5][0] = Vec3f(cornerDist, -cornerDist, -cornerDist);
	_corners[5][1] = Vec3f(-cornerDist, -cornerDist, -cornerDist);
	_corners[5][2] = Vec3f(-cornerDist, cornerDist, -cornerDist);
	_corners[5][3] = Vec3f(cornerDist, cornerDist, -cornerDist);
}

voxelType VoxelTerrain::getVoxel(const Point3i &centerRelativePosition) {
	Point3i pos((_size / 2) * VoxelChunk::_chunkSize + centerRelativePosition);

	if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= _size.x * VoxelChunk::_chunkSize || pos.y >= _size.y * VoxelChunk::_chunkSize || pos.z >= _size.z * VoxelChunk::_chunkSize)
		return _outsideMatrixVoxel;

	Point3i chunkPos(pos.x / VoxelChunk::_chunkSize, pos.y / VoxelChunk::_chunkSize, pos.z / VoxelChunk::_chunkSize);

	size_t index = chunkPos.x + chunkPos.y * _size.x + chunkPos.z * _size.x * _size.y;

	Point3i matrixPos(pos.x % VoxelChunk::_chunkSize, pos.y % VoxelChunk::_chunkSize, pos.z % VoxelChunk::_chunkSize);

	VoxelChunk* pChunk = static_cast<VoxelChunk*>((*_chunks)[index].get());

	return pChunk->_sharedData->_matrix[matrixPos.x + matrixPos.y * VoxelChunk::_chunkSize + matrixPos.z * VoxelChunk::_chunkSize * VoxelChunk::_chunkSize];
}

void VoxelTerrain::setVoxel(const Point3i &centerRelativePosition, voxelType voxel) {
	Point3i pos((_size / 2) * VoxelChunk::_chunkSize + centerRelativePosition);

	if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= _size.x * VoxelChunk::_chunkSize || pos.y >= _size.y * VoxelChunk::_chunkSize || pos.z >= _size.z * VoxelChunk::_chunkSize)
		return;

	Point3i chunkPos(pos.x / VoxelChunk::_chunkSize, pos.y / VoxelChunk::_chunkSize, pos.z / VoxelChunk::_chunkSize);

	size_t index = chunkPos.x + chunkPos.y * _size.x + chunkPos.z * _size.x * _size.y;

	Point3i matrixPos(pos.x % VoxelChunk::_chunkSize, pos.y % VoxelChunk::_chunkSize, pos.z % VoxelChunk::_chunkSize);

	VoxelChunk* pChunk = static_cast<VoxelChunk*>((*_chunks)[index].get());

	pChunk->_sharedData->_matrix[matrixPos.x + matrixPos.y * VoxelChunk::_chunkSize + matrixPos.z * VoxelChunk::_chunkSize * VoxelChunk::_chunkSize] = voxel;
}

void VoxelTerrain::setChunkMatrixSize(const Point3i &size) {
	Point3i oldSize = _size;
	Point3i oldCenter = oldSize / 2;

	_size = size;
	Point3i matrixCenter = _size / 2;

	std::shared_ptr<std::vector<SceneObjectRef>> newChunks(new std::vector<SceneObjectRef>());

	newChunks->resize(_size.x * _size.y * _size.z);

	for (int x = 0; x < _size.x; x++)
	for (int y = 0; y < _size.y; y++)
	for (int z = 0; z < _size.z; z++) {
		Point3i pos(x, y, z);

		Point3i dCenter = pos - matrixCenter;

		if (_chunks != nullptr &&
			(dCenter.x * 2 < oldSize.x ||
			dCenter.y * 2 < oldSize.y ||
			dCenter.z * 2 < oldSize.z)) {
			Point3i oldPos = oldCenter + dCenter;
			
			if (dCenter.x * 2 < _size.x ||
				dCenter.y * 2 < _size.y ||
				dCenter.z * 2 < _size.z) {
				SceneObjectRef chunk = (*_chunks)[oldPos.x + oldPos.y * _size.x + oldPos.z * _size.x * _size.z];
				(*newChunks)[pos.x + pos.y * _size.x + pos.z * _size.x * _size.y] = chunk;

				static_cast<VoxelChunk*>(chunk.get())->_centerRelativePosition = dCenter;
			}
			else
				(*_chunks)[oldPos.x + oldPos.y * _size.x + oldPos.z * _size.x * _size.z]->destroy();
		}
		else {
			// Create new chunk
			std::shared_ptr<VoxelChunk> newChunk(new VoxelChunk());

			getRenderScene()->add(newChunk, true);

			(*newChunks)[pos.x + pos.y * _size.x + pos.z * _size.x * _size.y] = newChunk.get();

			newChunk->create(*this, dCenter);
		}
	}

	_chunks = newChunks;
}

void VoxelTerrain::create(const Point3i &size,
	const std::shared_ptr<Shader> &gBufferRenderShader,
	const std::shared_ptr<Shader> &depthRenderShader,
	const std::shared_ptr<Texture2DArray> &diffuse,
	const std::shared_ptr<Texture2DArray> &normal)
{
	assert(getScene() != nullptr);

	_renderMask = 0xffff;

	setChunkMatrixSize(size);

	_gBufferRenderShader = gBufferRenderShader;
	_depthRenderShader = depthRenderShader;

	_diffuse = diffuse;
	_normal = normal;

	assert(_diffuse->getLayers() == 3);
	assert(_normal->getLayers() == 3);

	_gBufferRenderShader->bind();

	_gBufferRenderShader->setShaderTexture("pgeDiffuseMapArray", _diffuse->getTextureID(), GL_TEXTURE_2D_ARRAY);
	_gBufferRenderShader->setShaderTexture("pgeNormalMapArray", _normal->getTextureID(), GL_TEXTURE_2D_ARRAY);
}

void VoxelTerrain::generate(void(*generator)(pge::VoxelChunk &chunk, int seed), int seed, SceneObjectPhysicsWorld* pPhysicsWorld, float restitution, float friction) {
	assert(_chunks != nullptr);

	for (size_t i = 0; i < _chunks->size(); i++) {
		assert((*_chunks)[i].isAlive());
		VoxelChunk* pChunk = static_cast<VoxelChunk*>((*_chunks)[i].get());
		generator(*pChunk, seed);
	}

	for (size_t i = 0; i < _chunks->size(); i++) {
		assert((*_chunks)[i].isAlive());
		VoxelChunk* pChunk = static_cast<VoxelChunk*>((*_chunks)[i].get());

		pChunk->generate(pPhysicsWorld, restitution, friction);
	}
}

void VoxelTerrain::batchRender() {
	if (getRenderScene()->_shaderSwitchesEnabled) {
		_gBufferRenderShader->bind();
		_gBufferRenderShader->bindShaderTextures();

		glDisableVertexAttribArray(PGE_ATTRIB_TEXCOORD);

		getRenderScene()->setTransform(Matrix4x4f::identityMatrix());

		for (std::list<SceneObjectRef>::iterator it = _renderChunks.begin(); it != _renderChunks.end(); it++) {
			assert((*it).isAlive());

			VoxelChunk* pChunk = static_cast<VoxelChunk*>((*it).get());

			Point3i voxelPos = (_center + pChunk->getCenterRelativePosition()) * VoxelChunk::_chunkSize;

			Vec3f chunkPos = Vec3f(voxelPos.x, voxelPos.y, voxelPos.z) * _voxelSize;

			int lodIndex = clamp(static_cast<int>((getRenderScene()->_logicCamera._position - chunkPos).magnitude() / _LODSwitchDistance), 0, _numChunkLODs - 1);

			pChunk->_sharedData->_lods[lodIndex]._vertices.bind(GL_ARRAY_BUFFER);

			glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelChunk::Vertex), 0);
			glVertexAttribPointer(PGE_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelChunk::Vertex), reinterpret_cast<const GLvoid*>(sizeof(Vec3f)));

			pChunk->_sharedData->_lods[lodIndex]._indices.bind(GL_ELEMENT_ARRAY_BUFFER);

			glDrawElements(GL_TRIANGLES, pChunk->_sharedData->_lods[lodIndex]._numIndices, PGE_VOXEL_CHUNK_INDEX_TYPE_ENUM, nullptr);
		}

		glEnableVertexAttribArray(PGE_ATTRIB_TEXCOORD);
	}
	else if (getRenderScene()->_renderingShadows) {
		Shader* pPrevShader = Shader::getCurrentShader();

		_depthRenderShader->bind();

		glDisableVertexAttribArray(PGE_ATTRIB_TEXCOORD);
		glDisableVertexAttribArray(PGE_ATTRIB_NORMAL);

		getRenderScene()->setTransform(Matrix4x4f::identityMatrix());

		for (std::list<SceneObjectRef>::iterator it = _renderChunks.begin(); it != _renderChunks.end(); it++) {
			assert((*it).isAlive());

			VoxelChunk* pChunk = static_cast<VoxelChunk*>((*it).get());

			Point3i voxelPos = (_center + pChunk->getCenterRelativePosition()) * VoxelChunk::_chunkSize;

			Vec3f chunkPos = Vec3f(voxelPos.x, voxelPos.y, voxelPos.z) * _voxelSize;

			int lodIndex = clamp(static_cast<int>((getRenderScene()->_logicCamera._position - chunkPos).magnitude() / _LODSwitchDistance), 0, _numChunkLODs - 1);

			pChunk->_sharedData->_lods[lodIndex]._vertices.bind(GL_ARRAY_BUFFER);

			glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelChunk::Vertex), 0);

			pChunk->_sharedData->_lods[lodIndex]._indices.bind(GL_ELEMENT_ARRAY_BUFFER);

			glDrawElements(GL_TRIANGLES, pChunk->_sharedData->_lods[lodIndex]._numIndices, PGE_VOXEL_CHUNK_INDEX_TYPE_ENUM, nullptr);
		}

		glEnableVertexAttribArray(PGE_ATTRIB_NORMAL);
		glEnableVertexAttribArray(PGE_ATTRIB_TEXCOORD);

		pPrevShader->bind();
	}

	_renderChunks.clear();
}