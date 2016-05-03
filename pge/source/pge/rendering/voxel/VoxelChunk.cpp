#include <pge/rendering/voxel/VoxelChunk.h>

#include <pge/rendering/voxel/VoxelTerrain.h>

#include <pge/util/Math.h>

using namespace pge;

void VoxelChunk::create(const SceneObjectRef &voxelTerrain, const Point3i &centerRelativePosition) {
	_renderMask = 0xffff;
	
	_voxelTerrain = voxelTerrain;

	if (_sharedData == nullptr)
		_sharedData.reset(new SharedData());

	_centerRelativePosition = centerRelativePosition;

	updateChunkAABB();
}

void VoxelChunk::addVertex(const Point3i &center, int lod,
	unsigned char vertIndex, std::unordered_map<ChunkVertex, voxelChunkMeshIndexType, ChunkVertex> &positionToIndex,
	std::vector<Vertex> &vertices, std::vector<voxelChunkMeshIndexType> &faceIndices, std::unordered_map<voxelChunkMeshIndexType, bool> &isFlange, bool setAsFlange,
	VoxelTerrain* pVoxelTerrain)
{
	assert(vertIndex < 8);

	Point3i p0(center);

	std::array<std::array<int, 2>, 3> cornersi;

	cornersi[0][0] = center.x;
	cornersi[0][1] = center.x + lod;
	cornersi[1][0] = center.y;
	cornersi[1][1] = center.y + lod;
	cornersi[2][0] = center.z;
	cornersi[2][1] = center.z + lod;

	Point3i p1(cornersi[0][(vertIndex & 0x04) == 0 ? 0 : 1],
		cornersi[1][(vertIndex & 0x02) == 0 ? 0 : 1],
		cornersi[2][(vertIndex & 0x01) == 0 ? 0 : 1]);

	std::unordered_map<ChunkVertex, voxelChunkMeshIndexType, ChunkVertex>::iterator it = positionToIndex.find(ChunkVertex(p1));

	if (it == positionToIndex.end()) {
		std::array<Vec3f, 8> pa;
		std::array<float, 8> va;

		for (unsigned char i = 0; i < 8; i++) {
			Point3i p = p1 + Point3i((i & 0x04) == 0 ? 0 : -lod, (i & 0x02) == 0 ? 0 : -lod, (i & 0x01) == 0 ? 0 : -lod);
			Vec3f pf(static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z));
			float v = static_cast<float>(pVoxelTerrain->getVoxel(p)) * pVoxelTerrain->_voxelScalar;

			pa[i] = pf;
			va[i] = v;
		}

		Vec3f vertexPosition(0.0f, 0.0f, 0.0f);
		float vSum = 0.0f;

		if (sign(va[7]) != sign(va[3])) {
			vertexPosition += lerp(pa[7], pa[3], va[7] / (va[7] - va[3]));
			vSum++;
		}

		if (sign(va[3]) != sign(va[1])) {
			vertexPosition += lerp(pa[3], pa[1], va[3] / (va[3] - va[1]));
			vSum++;
		}

		if (sign(va[1]) != sign(va[5])) {
			vertexPosition += lerp(pa[1], pa[5], va[1] / (va[1] - va[5]));
			vSum++;
		}

		if (sign(va[5]) != sign(va[7])) {
			vertexPosition += lerp(pa[5], pa[7], va[5] / (va[5] - va[7]));
			vSum++;
		}

		if (sign(va[2]) != sign(va[6])) {
			vertexPosition += lerp(pa[2], pa[6], va[2] / (va[2] - va[6]));
			vSum++;
		}

		if (sign(va[6]) != sign(va[4])) {
			vertexPosition += lerp(pa[6], pa[4], va[6] / (va[6] - va[4]));
			vSum++;
		}

		if (sign(va[4]) != sign(va[0])) {
			vertexPosition += lerp(pa[4], pa[0], va[4] / (va[4] - va[0]));
			vSum++;
		}

		if (sign(va[0]) != sign(va[2])) {
			vertexPosition += lerp(pa[0], pa[2], va[0] / (va[0] - va[2]));
			vSum++;
		}

		if (sign(va[3]) != sign(va[2])) {
			vertexPosition += lerp(pa[3], pa[2], va[3] / (va[3] - va[2]));
			vSum++;
		}

		if (sign(va[1]) != sign(va[0])) {
			vertexPosition += lerp(pa[1], pa[0], va[1] / (va[1] - va[0]));
			vSum++;
		}

		if (sign(va[5]) != sign(va[4])) {
			vertexPosition += lerp(pa[5], pa[4], va[5] / (va[5] - va[4]));
			vSum++;
		}

		if (sign(va[7]) != sign(va[6])) {
			vertexPosition += lerp(pa[7], pa[6], va[7] / (va[7] - va[6]));
			vSum++;
		}

		assert(vSum != 0.0f);

		vertexPosition = vertexPosition / vSum * pVoxelTerrain->_voxelSize;

		// Create new vertex
		// Find position via weighted average of
		vertices.push_back(Vertex(vertexPosition));
		faceIndices.push_back(vertices.size() - 1);
		positionToIndex[ChunkVertex(p1)] = vertices.size() - 1;

		isFlange[faceIndices.back()] = setAsFlange;
	}
	else
		// Use existing vertex
		faceIndices.push_back(it->second);
}

void VoxelChunk::addGeometry(const Point3i &center, int lod, int side,
	std::unordered_map<ChunkVertex, voxelChunkMeshIndexType, ChunkVertex> &positionToIndex,
	std::vector<Vertex> &vertices, std::vector<voxelChunkMeshIndexType> &faceIndices, std::unordered_map<voxelChunkMeshIndexType, bool> &isFlange, bool setAsFlange,
	VoxelTerrain* pVoxelTerrain)
{
	// Surrounding indices
	switch (side) {
	case 0: // +X
		addVertex(center, lod, 4, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 5, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 7, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 6, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);

		break;
	case 1: // -X
		addVertex(center, lod, 1, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 0, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 2, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 3, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);

		break;
	case 2: // +Y
		addVertex(center, lod, 3, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 2, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 6, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 7, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);

		break;
	case 3: // -Y
		addVertex(center, lod, 0, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 1, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 5, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 4, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);

		break;
	case 4: // +Z
		addVertex(center, lod, 5, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 1, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 3, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 7, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);

		break;
	case 5: // -Z
		addVertex(center, lod, 0, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 4, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 6, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
		addVertex(center, lod, 2, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);

		break;
	}
}

void VoxelChunk::generate(SceneObjectPhysicsWorld* pPhysicsWorld, float restitution, float friction) {
	assert(_voxelTerrain.isAlive());

	VoxelTerrain* pVoxelTerrain = static_cast<VoxelTerrain*>(_voxelTerrain.get());

	if (_sharedData->_empty)
		return;

	// Set empty to true, will remain unless set false when comes across full voxel in generation
	_sharedData->_empty = false;

	_sharedData->_lods.resize(pVoxelTerrain->_numChunkLODs);

	for (int lod = 1, lodIndex = 0; lodIndex < pVoxelTerrain->_numChunkLODs; lod *= 2, lodIndex++) {
		std::unordered_map<ChunkVertex, voxelChunkMeshIndexType, ChunkVertex> positionToIndex;
		std::unordered_map<voxelChunkMeshIndexType, bool> isFlange;

		std::vector<Vertex> vertices;
		std::vector<voxelChunkMeshIndexType> faceIndices;

		// Go through voxels (not differentiating between border voxels and inside voxels)
		int lodOffset = lod;

		for (int x = -lodOffset; x < _chunkSize + lodOffset; x += lod)
		for (int y = -lodOffset; y < _chunkSize + lodOffset; y += lod)
		for (int z = -lodOffset; z < _chunkSize + lodOffset; z += lod) {
			// If voxel is not empty
			voxelType currentVoxel;
			
			bool setAsFlange;

			if (x < 0 || x >= _chunkSize ||
				y < 0 || y >= _chunkSize ||
				z < 0 || z >= _chunkSize)
			{
				currentVoxel = pVoxelTerrain->getVoxel(Point3i(
					_centerRelativePosition.x * _chunkSize + x,
					_centerRelativePosition.y * _chunkSize + y,
					_centerRelativePosition.z * _chunkSize + z));

				setAsFlange = true;
			}
			else {
				currentVoxel = _sharedData->_matrix[x + y * _chunkSize + z * _chunkSize * _chunkSize];
				setAsFlange = false;
			}

			if (currentVoxel < 0)
				continue;

			// Check surrounding voxels to see if they are empty
			for (int side = 0; side < 6; side++) {
				Point3i offset(pVoxelTerrain->_positionOffsets[side] * lod);

				int newX = x + offset.x;
				int newY = y + offset.y;
				int newZ = z + offset.z;

				voxelType voxel;

				// If check location is outside this chunk, get the neighboring chunk
				if (newX < 0 || newX >= _chunkSize ||
					newY < 0 || newY >= _chunkSize ||
					newZ < 0 || newZ >= _chunkSize)
					voxel = pVoxelTerrain->getVoxel(Point3i(
					_centerRelativePosition.x * _chunkSize + newX,
					_centerRelativePosition.y * _chunkSize + newY,
					_centerRelativePosition.z * _chunkSize + newZ));
				else
					voxel = _sharedData->_matrix[newX + newY * _chunkSize + newZ * _chunkSize * _chunkSize];

				// If empty
				if (voxel < 0)
					addGeometry(Point3i(_centerRelativePosition.x * _chunkSize + x, _centerRelativePosition.y * _chunkSize + y, _centerRelativePosition.z * _chunkSize + z),
						lod, side, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
			}
		}

		_sharedData->_lods[lodIndex]._numVertices = vertices.size();
		_sharedData->_lods[lodIndex]._numFaceIndices = faceIndices.size();

		assert(faceIndices.size() % 4 == 0);

		_sharedData->_lods[lodIndex]._empty = _sharedData->_lods[lodIndex]._numVertices == 0;

		if (lodIndex == 0 && _sharedData->_lods[lodIndex]._empty) {
			_sharedData->_empty = true;

			return;
		}

		if (_sharedData->_lods[lodIndex]._empty)
			continue;

		// Generate real indices from face indices (triangulate the quads)
		_sharedData->_lods[lodIndex]._numIndices = 3 * _sharedData->_lods[lodIndex]._numFaceIndices / 2;

		std::vector<ChunkFace> indices;
		indices.reserve(_sharedData->_lods[lodIndex]._numIndices / 6);

		// Calculate normals and generate real indices
		for (size_t i = 0; i < _sharedData->_lods[lodIndex]._numFaceIndices; i += 4) {
			// Triangulate quad
			Vertex &vertex0 = vertices[faceIndices[i]];
			Vertex &vertex1 = vertices[faceIndices[i + 1]];
			Vertex &vertex2 = vertices[faceIndices[i + 2]];
			Vertex &vertex3 = vertices[faceIndices[i + 3]];

			Vec3f normal0;
			Vec3f normal1;

			bool hasNormal0 = false;
			bool hasNormal1 = false;

			if (vertex1._position != vertex2._position && vertex1._position != vertex0._position && vertex2._position != vertex0._position) {
				normal0 = ((vertex1._position - vertex0._position).cross(vertex2._position - vertex0._position)).normalized();

				hasNormal0 = true;
			}

			if (vertex2._position != vertex0._position && vertex3._position != vertex2._position && vertex0._position != vertex3._position) {
				normal1 = ((vertex3._position - vertex2._position).cross(vertex0._position - vertex3._position)).normalized();

				hasNormal1 = true;
			}

			if (!hasNormal0 || !hasNormal1) {
				if (hasNormal1)
					normal0 = normal1;
				else if (hasNormal0)
					normal1 = normal0;
				else 
					normal1 = normal0 = Vec3f(0.0f, 1.0f, 0.0f);
			}

			//assert(normal0.magnitude() < 1000.0f && normal1.magnitude() < 1000.0f);

			Vec3f normalSum = (normal0 + normal1) * 0.5f;

			vertex0._normal += normalSum;
			vertex1._normal += normal0;
			vertex2._normal += normalSum;
			vertex3._normal += normal1;

			//assert(normalSum != Vec3f(0.0f, 0.0f, 0.0f) && normal0 != Vec3f(0.0f, 0.0f, 0.0f) && normal1 != Vec3f(0.0f, 0.0f, 0.0f));

			// Generate real indices
			indices.push_back(ChunkFace(faceIndices[i], faceIndices[i + 1], faceIndices[i + 2], faceIndices[i + 3]));
		}

		for (size_t i = 0; i < vertices.size(); i++) {
			if (vertices[i]._normal.magnitude() < 0.00001f)
				vertices[i]._normal = Vec3f(0.0f, 1.0f, 0.0f);

			vertices[i]._normal.normalize();
		}

		// Move flanges along normals
		for (size_t i = 0; i < _sharedData->_lods[lodIndex]._numFaceIndices; i++) {
			if (isFlange[faceIndices[i]])
				vertices[faceIndices[i]]._position -= vertices[faceIndices[i]]._normal * (lod - 1) * 0.125f;
		}

		// Destroy any existing buffers
		if (_sharedData->_lods[lodIndex]._vertices.created())
			_sharedData->_lods[lodIndex]._vertices.destroy();

		if (_sharedData->_lods[lodIndex]._indices.created())
			_sharedData->_lods[lodIndex]._indices.destroy();

		// Create (or re-create) VBOs
		_sharedData->_lods[lodIndex]._vertices.create();
		_sharedData->_lods[lodIndex]._indices.create();

		// Vertex VBO
		_sharedData->_lods[lodIndex]._vertices.bind(GL_ARRAY_BUFFER);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _sharedData->_lods[lodIndex]._numVertices, &vertices[0], GL_STATIC_DRAW);
		_sharedData->_lods[lodIndex]._vertices.unbind();

		// Index VBO
		_sharedData->_lods[lodIndex]._indices.bind(GL_ELEMENT_ARRAY_BUFFER);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(voxelChunkMeshIndexType) * _sharedData->_lods[lodIndex]._numIndices, &indices[0], GL_STATIC_DRAW);
		_sharedData->_lods[lodIndex]._indices.unbind();

		D3D_GL_ERROR_CHECK();

		// ---------------------------------------- Generate Physics Mesh ----------------------------------------

		if (pPhysicsWorld != nullptr && lod == 1) {
			_physicsWorld = pPhysicsWorld;

			_pTriangleMesh.reset(new btTriangleMesh());

			// Add all vertices
			for (size_t i = 0; i < indices.size(); i++) {
				const Vec3f &vertex0 = vertices[indices[i]._i0]._position;
				const Vec3f &vertex1 = vertices[indices[i]._i1]._position;
				const Vec3f &vertex2 = vertices[indices[i]._i2]._position;

				const Vec3f &vertex3 = vertices[indices[i]._i3]._position;
				const Vec3f &vertex4 = vertices[indices[i]._i4]._position;
				const Vec3f &vertex5 = vertices[indices[i]._i5]._position;

				_pTriangleMesh->addTriangle(bt(vertex0), bt(vertex1), bt(vertex2), false);
				_pTriangleMesh->addTriangle(bt(vertex3), bt(vertex4), bt(vertex5), false);
			}

			_pMeshShape.reset(new btBvhTriangleMeshShape(_pTriangleMesh.get(), true, true));

			_pMotionState.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), btVector3(0.0f, 0.0f, 0.0f))));

			btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0.0f, _pMotionState.get(), _pMeshShape.get(), btVector3(0.0f, 0.0f, 0.0f));

			rigidBodyCI.m_restitution = restitution;
			rigidBodyCI.m_friction = friction;

			_pRigidBody.reset(new btRigidBody(rigidBodyCI));

			pPhysicsWorld->_pDynamicsWorld->addRigidBody(_pRigidBody.get());
		}
	}
}

void VoxelChunk::deferredRender() {
	if (_voxelTerrain.isAlive() && !_sharedData->_empty)
		static_cast<VoxelTerrain*>(_voxelTerrain.get())->_renderChunks.push_back(*this);
}

VoxelTerrain* VoxelChunk::getTerrain() {
	assert(_voxelTerrain.isAlive());

	return static_cast<VoxelTerrain*>(_voxelTerrain.get());
}

void VoxelChunk::updateChunkAABB() {
	VoxelTerrain* pVoxelTerrain = static_cast<VoxelTerrain*>(_voxelTerrain.get());

	_aabb._lowerBound = Vec3f((pVoxelTerrain->getCenter().x + _centerRelativePosition.x) * _chunkSize,
		(pVoxelTerrain->getCenter().y + _centerRelativePosition.y) * _chunkSize,
		(pVoxelTerrain->getCenter().z + _centerRelativePosition.z) * _chunkSize) * pVoxelTerrain->_voxelSize;
	_aabb._upperBound = _aabb._lowerBound + Vec3f(_chunkSize, _chunkSize, _chunkSize) * pVoxelTerrain->_voxelSize;

	_aabb.calculateHalfDims();
	_aabb.calculateCenter();

	updateAABB();
}

void VoxelChunk::onDestroy() {
	if (_pRigidBody != nullptr && _physicsWorld.isAlive()) {
		// Remove body from physics world
		SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<SceneObjectPhysicsWorld*>(_physicsWorld.get());

		pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBody.get());
	}
}