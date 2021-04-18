#include "StaticPositionModel.h"

#include "../../util/Functions.h"

#include <fstream>
#include <sstream>

using namespace pge;

void StaticPositionModel::render() {
    for (std::shared_ptr<StaticPositionMesh> &mesh : meshes)
        mesh->render();
}

bool StaticPositionModel::loadFromOBJ(const std::string &fileName, AABB3D &aabb, bool useBuffers, bool clearArrays) {
    std::ifstream fromFile(fileName);

    if (!fromFile.is_open()) {
#ifdef PGE_DEBUG
        std::cerr << "Could not load model file " << fileName << std::endl;
#endif
        return false;
    }

    std::string rootName(getRootName(fileName));

    std::vector<Vec3f> filePositions;

    // Hash map for linking indices to vertex array index for attributes
    std::unordered_map<staticPositionMeshIndexType, size_t> indexToVertex;

    // Initial extremes
    aabb.lowerBound = Vec3f(999999.0f, 999999.0f, 999999.0f);
    aabb.upperBound = Vec3f(-999999.0f, -999999.0f, -999999.0f);

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
        else if (header == "f") {
            if (meshes.empty())
                meshes.push_back(std::shared_ptr<StaticPositionMesh>(new StaticPositionMesh()));

            // Add a face
            std::array<staticPositionMeshIndexType, 3> v;

            // Ignore non-position indices
            ss >> v[0];
            ss >> v[1];
            ss >> v[2];

            for (int i = 0; i < 3; i++) {
                // Search for index set 1
                std::unordered_map<staticPositionMeshIndexType, size_t>::iterator it = indexToVertex.find(v[i]);

                if (it == indexToVertex.end()) {
                    // Vertex attributes do not exist, create them

                    // File indicies start at 1, so convert
                    size_t vertIndex = v[i] - 1;

                    meshes.back()->vertices.push_back(filePositions[vertIndex]);

                    // Index of vertex in vertex component array
                    size_t realIndex = meshes.back()->vertices.size() - 1;

                    // Add attribute set index to the map
                    indexToVertex[v[i]] = realIndex;

                    meshes.back()->indices.push_back(static_cast<staticPositionMeshIndexType>(realIndex));
                }
                else
                    meshes.back()->indices.push_back(static_cast<staticPositionMeshIndexType>(it->second));
            }
        }
        else if (header == "usemtl")
            // Add new mesh
            meshes.push_back(std::shared_ptr<StaticPositionMesh>(new StaticPositionMesh()));
    }

    fromFile.close();

    aabb.calculateHalfDims();
    aabb.calculateCenter();

    for (std::shared_ptr<StaticPositionMesh> &meshAndMaterial : meshes) {
        meshAndMaterial->create(useBuffers);

        if (useBuffers)
            meshAndMaterial->updateBuffers();

        if (clearArrays)
            meshAndMaterial->clearArrays();
    }

    return true;
}