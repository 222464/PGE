#pragma once

#include "../../scene/RenderScene.h"

#include "../../rendering/model/StaticModel.h"

#include "../physics/SceneObjectPhysicsWorld.h"

namespace pge {
    class Map3DWS : public Asset {
    public:
        typedef uint8_t Byte;
        typedef uint16_t Word;
        typedef int32_t Long;
        typedef float Float;
        typedef uint32_t Name;
        typedef std::array<unsigned char, 3> Color3;
        typedef Vec2f Vec2;
        typedef Vec3f Vec3;
        typedef Vec4f Vec4;

        struct MapHeader {
            Word mapVersion;
            Byte mapFlags;
            Long nameCount;
            Long nameOffset;
            Long objectCount;
            Long objectOffset;
        };

        struct MapObjectTableEntry {
            Name objectClass;
            Long dataSize;
        };

        struct MapMaterial {
            Byte flags;
            Name groupName;
            Name objectName;

            // If flags & 2
            Name extensionName;
        };

        struct MapMeshReference {
            Byte flags;
            Name groupName;
            Name objectName;
            Byte limbCount;
        };

        struct MapLightmap {
            Byte flags;
            Byte resolution;
            Long format;

            // (2 ^ resolution) ^ 2 pixels
            std::vector<Color3> pixels;
        };

        struct MapPartialVertex {
            Byte vertex;
            Vec2 texCoords;
        };

        struct MapPartialVertexLightmapped {
            Byte vertex;
            Vec2 texCoords;
            Vec2 lightmapCoords;
        };

        struct MapFace {
            Byte flags;
            Vec4 planeEquation;
            Vec2 texturePosition;
            Vec2 textureScale;
            Vec2 textureRotation;
            Vec4 UTextureMappingPlane;
            Vec4 VTextureMappingPlane;
            Float lumelSize;
            Long smoothGroupIndex;
            Long materialIndex;

            // If flags & 16
            Long lightmapIndex;

            Byte indexCount;

            // If !(flags & 16)
            std::vector<MapPartialVertex> partialVertices;

            // If flags & 16
            std::vector<MapPartialVertexLightmapped> partialVerticesLightmapped;
        };

        struct MapBrush {
            Byte flags;
            Long keys;

            // keys number of keys
            std::unordered_map<Name, Name> keyValue;

            //Long group;
            //Long visGroup;
            //Color3 brushColor;
            Byte vertexCount;

            // vertexCount number of vertices
            std::vector<Vec3> vertexPositions;

            Byte faceCount;

            // faceCount number of faces
            std::vector<MapFace> faces;
        };

        struct MapLimb {
            Long materialIndex;

            // If flags & 16 (flags from MapMesh)
            //Word vertexCount;
            //std::vector<Color3> vertexColor;
        };

        struct MapMesh {
            Byte flags;
            Long keys;

            // keys number of keys
            std::unordered_map<Name, Name> keyValue;

            //Long group;
            //Long visGroup;
            //Color3 meshColor;
            Long meshReferenceIndex;
            Vec3 position;
            Vec3 rotation;

            // If !(flags & 1)
            Vec3 scale;

            // limbCount from meshreference number of limbs
            std::vector<MapLimb> limbs;
        };

        struct MapEntity {
            Byte flags;
            Vec3 position;
            Long keys;

            // keys number of keys
            std::unordered_map<Name, Name> keyValue;

            //Long group;
            //Long visGroup;

            // The following is not part of the spec but added for usability
            std::unordered_map<std::string, std::string> properties;
        };

        struct MapGenerationSettings {
            Scene* pScene;
            bool usePhysics;
            bool useGraphics;
            std::string defaultImageFileExtension;
            float sizeScalar;
            SceneObjectRef giRef;

            MapGenerationSettings()
                : pScene(nullptr),
                usePhysics(true),
                useGraphics(true),
                defaultImageFileExtension(".png"),
                sizeScalar(0.025f),
                giRef(nullptr)
            {}
        };

        MapGenerationSettings settings;

        std::vector<std::string> nameTable;
        std::vector<MapEntity> entities;

        // Inherited from Asset
        bool createAsset(const std::string &name);

        // Asset factory
        static Asset* assetFactory() {
            return new Map3DWS();
        }
    };

    class Map3DWSChunk : public SceneObject {
    private:
        std::shared_ptr<StaticModel> model;

        pge::SceneObjectRef batcherRef;

    public:
        std::shared_ptr<AssetManager> textureManager;

        Map3DWSChunk()
            : textureManager(new AssetManager())
        {
            renderMask = 0xffff;
            textureManager->create(Texture2D::assetFactory);
        }

        // Inherited from SceneObject
        void onAdd();
        void deferredRender();

        Map3DWSChunk* copyFactory() {
            return new Map3DWSChunk(*this);
        }

        friend class Map3DWS;
    };

    class Map3DWSPhysics : public SceneObject {
    private:
        std::shared_ptr<btTriangleMesh> pTriangleMesh;
        std::shared_ptr<btBvhTriangleMeshShape> pMeshShape;
        std::shared_ptr<btDefaultMotionState> pMotionState;
        std::shared_ptr<btRigidBody> pRigidBody;

    public:
        // Inherited from SceneObject
        void onAdd();
        void onDestroy();

        Map3DWSPhysics* copyFactory() {
            return new Map3DWSPhysics(*this);
        }

        friend class Map3DWS;
    };

    void addMapLights(const Map3DWS &map, Scene* pScene);
}