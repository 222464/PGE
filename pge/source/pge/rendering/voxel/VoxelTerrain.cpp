#include "VoxelTerrain.h"

#include "../../util/Math.h"

using namespace pge;

VoxelTerrain::VoxelTerrain()
: size(0, 0, 0), center(0, 0, 0),
outsideMatrixVoxel(127),
numChunkLODs(1), LODSwitchDistance(60.0f),
voxelScalar(0.03125f), voxelSize(1.0f)
{
    // Generate normals
    normals[0] = Vec3f(1.0f, 0.0f, 0.0f);
    normals[1] = Vec3f(-1.0f, 0.0f, 0.0f);
    normals[2] = Vec3f(0.0f, 1.0f, 0.0f);
    normals[3] = Vec3f(0.0f, -1.0f, 0.0f);
    normals[4] = Vec3f(0.0f, 0.0f, 1.0f);
    normals[5] = Vec3f(0.0f, 0.0f, -1.0f);

    // Generate test offsets
    positionOffsets[0] = Point3i(1, 0, 0);
    positionOffsets[1] = Point3i(-1, 0, 0);
    positionOffsets[2] = Point3i(0, 1, 0);
    positionOffsets[3] = Point3i(0, -1, 0);
    positionOffsets[4] = Point3i(0, 0, 1);
    positionOffsets[5] = Point3i(0, 0, -1);

    // Generate corners
    const float cornerDist = 0.5f;

    corners[0][0] = Vec3f(cornerDist, -cornerDist, cornerDist);
    corners[0][1] = Vec3f(cornerDist, -cornerDist, -cornerDist);
    corners[0][2] = Vec3f(cornerDist, cornerDist, -cornerDist);
    corners[0][3] = Vec3f(cornerDist, cornerDist, cornerDist);

    corners[1][0] = Vec3f(-cornerDist, -cornerDist, -cornerDist);
    corners[1][1] = Vec3f(-cornerDist, -cornerDist, cornerDist);
    corners[1][2] = Vec3f(-cornerDist, cornerDist, cornerDist);
    corners[1][3] = Vec3f(-cornerDist, cornerDist, -cornerDist);

    corners[2][0] = Vec3f(-cornerDist, cornerDist, -cornerDist);
    corners[2][1] = Vec3f(-cornerDist, cornerDist, cornerDist);
    corners[2][2] = Vec3f(cornerDist, cornerDist, cornerDist);
    corners[2][3] = Vec3f(cornerDist, cornerDist, -cornerDist);

    corners[3][0] = Vec3f(-cornerDist, -cornerDist, cornerDist);
    corners[3][1] = Vec3f(-cornerDist, -cornerDist, -cornerDist);
    corners[3][2] = Vec3f(cornerDist, -cornerDist, -cornerDist);
    corners[3][3] = Vec3f(cornerDist, -cornerDist, cornerDist);

    corners[4][0] = Vec3f(-cornerDist, -cornerDist, cornerDist);
    corners[4][1] = Vec3f(cornerDist, -cornerDist, cornerDist);
    corners[4][2] = Vec3f(cornerDist, cornerDist, cornerDist);
    corners[4][3] = Vec3f(-cornerDist, cornerDist, cornerDist);

    corners[5][0] = Vec3f(cornerDist, -cornerDist, -cornerDist);
    corners[5][1] = Vec3f(-cornerDist, -cornerDist, -cornerDist);
    corners[5][2] = Vec3f(-cornerDist, cornerDist, -cornerDist);
    corners[5][3] = Vec3f(cornerDist, cornerDist, -cornerDist);
}

voxelType VoxelTerrain::getVoxel(const Point3i &centerRelativePosition) {
    Point3i pos((size / 2) * VoxelChunk::chunkSize + centerRelativePosition);

    if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= size.x * VoxelChunk::chunkSize || pos.y >= size.y * VoxelChunk::chunkSize || pos.z >= size.z * VoxelChunk::chunkSize)
        return outsideMatrixVoxel;

    Point3i chunkPos(pos.x / VoxelChunk::chunkSize, pos.y / VoxelChunk::chunkSize, pos.z / VoxelChunk::chunkSize);

    size_t index = chunkPos.x + chunkPos.y * size.x + chunkPos.z * size.x * size.y;

    Point3i matrixPos(pos.x % VoxelChunk::chunkSize, pos.y % VoxelChunk::chunkSize, pos.z % VoxelChunk::chunkSize);

    VoxelChunk* pChunk = static_cast<VoxelChunk*>((*chunks)[index].get());

    return pChunk->sharedData->matrix[matrixPos.x + matrixPos.y * VoxelChunk::chunkSize + matrixPos.z * VoxelChunk::chunkSize * VoxelChunk::chunkSize];
}

void VoxelTerrain::setVoxel(const Point3i &centerRelativePosition, voxelType voxel) {
    Point3i pos((size / 2) * VoxelChunk::chunkSize + centerRelativePosition);

    if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= size.x * VoxelChunk::chunkSize || pos.y >= size.y * VoxelChunk::chunkSize || pos.z >= size.z * VoxelChunk::chunkSize)
        return;

    Point3i chunkPos(pos.x / VoxelChunk::chunkSize, pos.y / VoxelChunk::chunkSize, pos.z / VoxelChunk::chunkSize);

    size_t index = chunkPos.x + chunkPos.y * size.x + chunkPos.z * size.x * size.y;

    Point3i matrixPos(pos.x % VoxelChunk::chunkSize, pos.y % VoxelChunk::chunkSize, pos.z % VoxelChunk::chunkSize);

    VoxelChunk* pChunk = static_cast<VoxelChunk*>((*chunks)[index].get());

    pChunk->sharedData->matrix[matrixPos.x + matrixPos.y * VoxelChunk::chunkSize + matrixPos.z * VoxelChunk::chunkSize * VoxelChunk::chunkSize] = voxel;
}

void VoxelTerrain::setChunkMatrixSize(const Point3i &size) {
    Point3i oldSize = size;
    Point3i oldCenter = oldSize / 2;

    this->size = size;
    Point3i matrixCenter = size / 2;

    std::shared_ptr<std::vector<SceneObjectRef>> newChunks(new std::vector<SceneObjectRef>());

    newChunks->resize(size.x * size.y * size.z);

    for (int x = 0; x < size.x; x++)
    for (int y = 0; y < size.y; y++)
    for (int z = 0; z < size.z; z++) {
        Point3i pos(x, y, z);

        Point3i dCenter = pos - matrixCenter;

        if (chunks != nullptr &&
            (dCenter.x * 2 < oldSize.x ||
            dCenter.y * 2 < oldSize.y ||
            dCenter.z * 2 < oldSize.z)) {
            Point3i oldPos = oldCenter + dCenter;
            
            if (dCenter.x * 2 < size.x ||
                dCenter.y * 2 < size.y ||
                dCenter.z * 2 < size.z) {
                SceneObjectRef chunk = (*chunks)[oldPos.x + oldPos.y * size.x + oldPos.z * size.x * size.z];
                (*newChunks)[pos.x + pos.y * size.x + pos.z * size.x * size.y] = chunk;

                static_cast<VoxelChunk*>(chunk.get())->centerRelativePosition = dCenter;
            }
            else
                (*chunks)[oldPos.x + oldPos.y * size.x + oldPos.z * size.x * size.z]->destroy();
        }
        else {
            // Create new chunk
            std::shared_ptr<VoxelChunk> newChunk(new VoxelChunk());

            getRenderScene()->add(newChunk, true);

            (*newChunks)[pos.x + pos.y * size.x + pos.z * size.x * size.y] = newChunk.get();

            newChunk->create(*this, dCenter);
        }
    }

    chunks = newChunks;
}

void VoxelTerrain::create(const Point3i &size,
    const std::shared_ptr<Shader> &gBufferRenderShader,
    const std::shared_ptr<Shader> &depthRenderShader,
    const std::shared_ptr<Texture2DArray> &diffuse,
    const std::shared_ptr<Texture2DArray> &normal)
{
    assert(getScene() != nullptr);

    renderMask = 0xffff;

    setChunkMatrixSize(size);

    this->gBufferRenderShader = gBufferRenderShader;
    this->depthRenderShader = depthRenderShader;

    this->diffuse = diffuse;
    this->normal = normal;

    assert(diffuse->getLayers() == 3);
    assert(normal->getLayers() == 3);

    gBufferRenderShader->bind();

    gBufferRenderShader->setShaderTexture("pgeDiffuseMapArray", diffuse->getTextureID(), GL_TEXTURE_2D_ARRAY);
    gBufferRenderShader->setShaderTexture("pgeNormalMapArray", normal->getTextureID(), GL_TEXTURE_2D_ARRAY);
}

void VoxelTerrain::generate(void(*generator)(pge::VoxelChunk &chunk, int seed), int seed, SceneObjectPhysicsWorld* pPhysicsWorld, float restitution, float friction) {
    assert(chunks != nullptr);

    for (size_t i = 0; i < chunks->size(); i++) {
        assert((*chunks)[i].isAlive());
        VoxelChunk* pChunk = static_cast<VoxelChunk*>((*chunks)[i].get());
        generator(*pChunk, seed);
    }

    for (size_t i = 0; i < chunks->size(); i++) {
        assert((*chunks)[i].isAlive());
        VoxelChunk* pChunk = static_cast<VoxelChunk*>((*chunks)[i].get());

        pChunk->generate(pPhysicsWorld, restitution, friction);
    }
}

void VoxelTerrain::batchRender() {
    if (getRenderScene()->shaderSwitchesEnabled) {
        gBufferRenderShader->bind();
        gBufferRenderShader->bindShaderTextures();

        glDisableVertexAttribArray(PGE_ATTRIB_TEXCOORD);

        getRenderScene()->setTransform(Matrix4x4f::identityMatrix());

        for (std::list<SceneObjectRef>::iterator it = renderChunks.begin(); it != renderChunks.end(); it++) {
            assert((*it).isAlive());

            VoxelChunk* pChunk = static_cast<VoxelChunk*>((*it).get());

            Point3i voxelPos = (center + pChunk->getCenterRelativePosition()) * VoxelChunk::chunkSize;

            Vec3f chunkPos = Vec3f(voxelPos.x, voxelPos.y, voxelPos.z) * voxelSize;

            int lodIndex = clamp(static_cast<int>((getRenderScene()->logicCamera.position - chunkPos).magnitude() / LODSwitchDistance), 0, numChunkLODs - 1);

            pChunk->sharedData->lods[lodIndex].vertices.bind(GL_ARRAY_BUFFER);

            glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelChunk::Vertex), 0);
            glVertexAttribPointer(PGE_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelChunk::Vertex), reinterpret_cast<const GLvoid*>(sizeof(Vec3f)));

            pChunk->sharedData->lods[lodIndex].indices.bind(GL_ELEMENT_ARRAY_BUFFER);

            glDrawElements(GL_TRIANGLES, pChunk->sharedData->lods[lodIndex].numIndices, PGE_VOXEL_CHUNK_INDEX_TYPE_ENUM, nullptr);
        }

        glEnableVertexAttribArray(PGE_ATTRIB_TEXCOORD);
    }
    else if (getRenderScene()->renderingShadows) {
        Shader* pPrevShader = Shader::getCurrentShader();

        depthRenderShader->bind();

        glDisableVertexAttribArray(PGE_ATTRIB_TEXCOORD);
        glDisableVertexAttribArray(PGE_ATTRIB_NORMAL);

        getRenderScene()->setTransform(Matrix4x4f::identityMatrix());

        for (std::list<SceneObjectRef>::iterator it = renderChunks.begin(); it != renderChunks.end(); it++) {
            assert((*it).isAlive());

            VoxelChunk* pChunk = static_cast<VoxelChunk*>((*it).get());

            Point3i voxelPos = (center + pChunk->getCenterRelativePosition()) * VoxelChunk::chunkSize;

            Vec3f chunkPos = Vec3f(voxelPos.x, voxelPos.y, voxelPos.z) * voxelSize;

            int lodIndex = clamp(static_cast<int>((getRenderScene()->logicCamera.position - chunkPos).magnitude() / LODSwitchDistance), 0, numChunkLODs - 1);

            pChunk->sharedData->lods[lodIndex].vertices.bind(GL_ARRAY_BUFFER);

            glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelChunk::Vertex), 0);

            pChunk->sharedData->lods[lodIndex].indices.bind(GL_ELEMENT_ARRAY_BUFFER);

            glDrawElements(GL_TRIANGLES, pChunk->sharedData->lods[lodIndex].numIndices, PGE_VOXEL_CHUNK_INDEX_TYPE_ENUM, nullptr);
        }

        glEnableVertexAttribArray(PGE_ATTRIB_NORMAL);
        glEnableVertexAttribArray(PGE_ATTRIB_TEXCOORD);

        pPrevShader->bind();
    }

    renderChunks.clear();
}
