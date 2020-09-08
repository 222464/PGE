#include "VoxelChunk.h"

#include "VoxelTerrain.h"

#include "../../util/Math.h"

using namespace pge;

void VoxelChunk::create(const SceneObjectRef &voxelTerrain, const Point3i &centerRelativePosition) {
	renderMask = 0xffff;
	
	this->voxelTerrain = voxelTerrain;

	if (sharedData == nullptr)
		sharedData.reset(new SharedData());

	this->centerRelativePosition = centerRelativePosition;

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
			float v = static_cast<float>(pVoxelTerrain->getVoxel(p)) * pVoxelTerrain->voxelScalar;

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

		vertexPosition = vertexPosition / vSum * pVoxelTerrain->voxelSize;

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
	assert(voxelTerrain.isAlive());

	VoxelTerrain* pVoxelTerrain = static_cast<VoxelTerrain*>(voxelTerrain.get());

	if (sharedData->empty)
		return;

	// Set empty to true, will remain unless set false when comes across full voxel in generation
	sharedData->empty = false;

	sharedData->lods.resize(pVoxelTerrain->numChunkLODs);

	for (int lod = 1, lodIndex = 0; lodIndex < pVoxelTerrain->numChunkLODs; lod *= 2, lodIndex++) {
		std::unordered_map<ChunkVertex, voxelChunkMeshIndexType, ChunkVertex> positionToIndex;
		std::unordered_map<voxelChunkMeshIndexType, bool> isFlange;

		std::vector<Vertex> vertices;
		std::vector<voxelChunkMeshIndexType> faceIndices;

		// Go through voxels (not differentiating between border voxels and inside voxels)
		int lodOffset = lod;

		for (int x = -lodOffset; x < chunkSize + lodOffset; x += lod)
		for (int y = -lodOffset; y < chunkSize + lodOffset; y += lod)
		for (int z = -lodOffset; z < chunkSize + lodOffset; z += lod) {
			// If voxel is not empty
			voxelType currentVoxel;
			
			bool setAsFlange;

			if (x < 0 || x >= chunkSize ||
				y < 0 || y >= chunkSize ||
				z < 0 || z >= chunkSize)
			{
				currentVoxel = pVoxelTerrain->getVoxel(Point3i(
					centerRelativePosition.x * chunkSize + x,
					centerRelativePosition.y * chunkSize + y,
					centerRelativePosition.z * chunkSize + z));

				setAsFlange = true;
			}
			else {
				currentVoxel = sharedData->matrix[x + y * chunkSize + z * chunkSize * chunkSize];
				setAsFlange = false;
			}

			if (currentVoxel < 0)
				continue;

			// Check surrounding voxels to see if they are empty
			for (int side = 0; side < 6; side++) {
				Point3i offset(pVoxelTerrain->positionOffsets[side] * lod);

				int newX = x + offset.x;
				int newY = y + offset.y;
				int newZ = z + offset.z;

				voxelType voxel;

				// If check location is outside this chunk, get the neighboring chunk
				if (newX < 0 || newX >= chunkSize ||
					newY < 0 || newY >= chunkSize ||
					newZ < 0 || newZ >= chunkSize)
					voxel = pVoxelTerrain->getVoxel(Point3i(
					centerRelativePosition.x * chunkSize + newX,
					centerRelativePosition.y * chunkSize + newY,
					centerRelativePosition.z * chunkSize + newZ));
				else
					voxel = sharedData->matrix[newX + newY * chunkSize + newZ * chunkSize * chunkSize];

				// If empty
				if (voxel < 0)
					addGeometry(Point3i(centerRelativePosition.x * chunkSize + x, centerRelativePosition.y * chunkSize + y, centerRelativePosition.z * chunkSize + z),
						lod, side, positionToIndex, vertices, faceIndices, isFlange, setAsFlange, pVoxelTerrain);
			}
		}

		sharedData->lods[lodIndex].numVertices = vertices.size();
		sharedData->lods[lodIndex].numFaceIndices = faceIndices.size();

		assert(faceIndices.size() % 4 == 0);

		sharedData->lods[lodIndex].empty = sharedData->lods[lodIndex].numVertices == 0;

		if (lodIndex == 0 && sharedData->lods[lodIndex].empty) {
			sharedData->empty = true;

			return;
		}

		if (sharedData->lods[lodIndex].empty)
			continue;

		// Generate real indices from face indices (triangulate the quads)
		sharedData->lods[lodIndex].numIndices = 3 * sharedData->lods[lodIndex].numFaceIndices / 2;

		std::vector<ChunkFace> indices;
		indices.reserve(sharedData->lods[lodIndex].numIndices / 6);

		// Calculate normals and generate real indices
		for (size_t i = 0; i < sharedData->lods[lodIndex].numFaceIndices; i += 4) {
			// Triangulate quad
			Vertex &vertex0 = vertices[faceIndices[i]];
			Vertex &vertex1 = vertices[faceIndices[i + 1]];
			Vertex &vertex2 = vertices[faceIndices[i + 2]];
			Vertex &vertex3 = vertices[faceIndices[i + 3]];

			Vec3f normal0;
			Vec3f normal1;

			bool hasNormal0 = false;
			bool hasNormal1 = false;

			if (vertex1.position != vertex2.position && vertex1.position != vertex0.position && vertex2.position != vertex0.position) {
				normal0 = ((vertex1.position - vertex0.position).cross(vertex2.position - vertex0.position)).normalized();

				hasNormal0 = true;
			}

			if (vertex2.position != vertex0.position && vertex3.position != vertex2.position && vertex0.position != vertex3.position) {
				normal1 = ((vertex3.position - vertex2.position).cross(vertex0.position - vertex3.position)).normalized();

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

			vertex0.normal += normalSum;
			vertex1.normal += normal0;
			vertex2.normal += normalSum;
			vertex3.normal += normal1;

			//assert(normalSum != Vec3f(0.0f, 0.0f, 0.0f) && normal0 != Vec3f(0.0f, 0.0f, 0.0f) && normal1 != Vec3f(0.0f, 0.0f, 0.0f));

			// Generate real indices
			indices.push_back(ChunkFace(faceIndices[i], faceIndices[i + 1], faceIndices[i + 2], faceIndices[i + 3]));
		}

		for (size_t i = 0; i < vertices.size(); i++) {
			if (vertices[i].normal.magnitude() < 0.00001f)
				vertices[i].normal = Vec3f(0.0f, 1.0f, 0.0f);

			vertices[i].normal.normalize();
		}

		// Move flanges along normals
		for (size_t i = 0; i < sharedData->lods[lodIndex].numFaceIndices; i++) {
			if (isFlange[faceIndices[i]])
				vertices[faceIndices[i]].position -= vertices[faceIndices[i]].normal * (lod - 1) * 0.125f;
		}

		// Destroy any existing buffers
		if (sharedData->lods[lodIndex].vertices.created())
			sharedData->lods[lodIndex].vertices.destroy();

		if (sharedData->lods[lodIndex].indices.created())
			sharedData->lods[lodIndex].indices.destroy();

		// Create (or re-create) VBOs
		sharedData->lods[lodIndex].vertices.create();
		sharedData->lods[lodIndex].indices.create();

		// Vertex VBO
		sharedData->lods[lodIndex].vertices.bind(GL_ARRAY_BUFFER);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * sharedData->lods[lodIndex].numVertices, &vertices[0], GL_STATIC_DRAW);
		sharedData->lods[lodIndex].vertices.unbind();

		// Index VBO
		sharedData->lods[lodIndex].indices.bind(GL_ELEMENT_ARRAY_BUFFER);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(voxelChunkMeshIndexType) * sharedData->lods[lodIndex].numIndices, &indices[0], GL_STATIC_DRAW);
		sharedData->lods[lodIndex].indices.unbind();

		PGE_GL_ERROR_CHECK();

		// ---------------------------------------- Generate Physics Mesh ----------------------------------------

		if (pPhysicsWorld != nullptr && lod == 1) {
			physicsWorld = pPhysicsWorld;

			pTriangleMesh.reset(new btTriangleMesh());

			// Add all vertices
			for (size_t i = 0; i < indices.size(); i++) {
				const Vec3f &vertex0 = vertices[indices[i].i0].position;
				const Vec3f &vertex1 = vertices[indices[i].i1].position;
				const Vec3f &vertex2 = vertices[indices[i].i2].position;

				const Vec3f &vertex3 = vertices[indices[i].i3].position;
				const Vec3f &vertex4 = vertices[indices[i].i4].position;
				const Vec3f &vertex5 = vertices[indices[i].i5].position;

				pTriangleMesh->addTriangle(bt(vertex0), bt(vertex1), bt(vertex2), false);
				pTriangleMesh->addTriangle(bt(vertex3), bt(vertex4), bt(vertex5), false);
			}

			pMeshShape.reset(new btBvhTriangleMeshShape(pTriangleMesh.get(), true, true));

			pMotionState.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), btVector3(0.0f, 0.0f, 0.0f))));

			btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0.0f, pMotionState.get(), pMeshShape.get(), btVector3(0.0f, 0.0f, 0.0f));

			rigidBodyCI.m_restitution = restitution;
			rigidBodyCI.m_friction = friction;

			pRigidBody.reset(new btRigidBody(rigidBodyCI));

			pPhysicsWorld->pDynamicsWorld->addRigidBody(pRigidBody.get());
		}
	}
}

void VoxelChunk::deferredRender() {
	if (voxelTerrain.isAlive() && !sharedData->empty)
		static_cast<VoxelTerrain*>(voxelTerrain.get())->renderChunks.push_back(*this);
}

VoxelTerrain* VoxelChunk::getTerrain() {
	assert(voxelTerrain.isAlive());

	return static_cast<VoxelTerrain*>(voxelTerrain.get());
}

void VoxelChunk::updateChunkAABB() {
	VoxelTerrain* pVoxelTerrain = static_cast<VoxelTerrain*>(voxelTerrain.get());

	aabb.lowerBound = Vec3f((pVoxelTerrain->getCenter().x + centerRelativePosition.x) * chunkSize,
		(pVoxelTerrain->getCenter().y + centerRelativePosition.y) * chunkSize,
		(pVoxelTerrain->getCenter().z + centerRelativePosition.z) * chunkSize) * pVoxelTerrain->voxelSize;
	aabb.upperBound = aabb.lowerBound + Vec3f(chunkSize, chunkSize, chunkSize) * pVoxelTerrain->voxelSize;

	aabb.calculateHalfDims();
	aabb.calculateCenter();

	updateAABB();
}

void VoxelChunk::onDestroy() {
	if (pRigidBody != nullptr && physicsWorld.isAlive()) {
		// Remove body from physics world
		SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<SceneObjectPhysicsWorld*>(physicsWorld.get());

		pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBody.get());
	}
}