#pragma once

#include <pge/scene/RenderScene.h>

#include <pge/rendering/model/StaticModel.h>

#include <pge/sceneobjects/physics/SceneObjectPhysicsWorld.h>

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
			Word _mapVersion;
			Byte _mapFlags;
			Long _nameCount;
			Long _nameOffset;
			Long _objectCount;
			Long _objectOffset;
		};

		struct MapObjectTableEntry {
			Name _objectClass;
			Long _dataSize;
		};

		struct MapMaterial {
			Byte _flags;
			Name _groupName;
			Name _objectName;

			// If _flags & 2
			Name _extensionName;
		};

		struct MapMeshReference {
			Byte _flags;
			Name _groupName;
			Name _objectName;
			Byte _limbCount;
		};

		struct MapLightmap {
			Byte _flags;
			Byte _resolution;
			Long _format;

			// (2 ^ _resolution) ^ 2 pixels
			std::vector<Color3> _pixels;
		};

		struct MapPartialVertex {
			Byte _vertex;
			Vec2 _texCoords;
		};

		struct MapPartialVertexLightmapped {
			Byte _vertex;
			Vec2 _texCoords;
			Vec2 _lightmapCoords;
		};

		struct MapFace {
			Byte _flags;
			Vec4 _planeEquation;
			Vec2 _texturePosition;
			Vec2 _textureScale;
			Vec2 _textureRotation;
			Vec4 _UTextureMappingPlane;
			Vec4 _VTextureMappingPlane;
			Float _lumelSize;
			Long _smoothGroupIndex;
			Long _materialIndex;

			// If _flags & 16
			Long _lightmapIndex;

			Byte _indexCount;

			// If !(_flags & 16)
			std::vector<MapPartialVertex> _partialVertices;

			// If _flags & 16
			std::vector<MapPartialVertexLightmapped> _partialVerticesLightmapped;
		};

		struct MapBrush {
			Byte _flags;
			Long _keys;

			// _keys number of keys
			std::unordered_map<Name, Name> _keyValue;

			//Long _group;
			//Long _visGroup;
			//Color3 _brushColor;
			Byte _vertexCount;

			// _vertexCount number of vertices
			std::vector<Vec3> _vertexPositions;

			Byte _faceCount;

			// _faceCount number of faces
			std::vector<MapFace> _faces;
		};

		struct MapLimb {
			Long _materialIndex;

			// If _flags & 16 (_flags from MapMesh)
			//Word _vertexCount;
			//std::vector<Color3> _vertexColor;
		};

		struct MapMesh {
			Byte _flags;
			Long _keys;

			// _keys number of keys
			std::unordered_map<Name, Name> _keyValue;

			//Long _group;
			//Long _visGroup;
			//Color3 _meshColor;
			Long _meshReferenceIndex;
			Vec3 _position;
			Vec3 _rotation;

			// If !(_flags & 1)
			Vec3 _scale;

			// _limbCount from meshreference number of limbs
			std::vector<MapLimb> _limbs;
		};

		struct MapEntity {
			Byte _flags;
			Vec3 _position;
			Long _keys;

			// _keys number of keys
			std::unordered_map<Name, Name> _keyValue;

			//Long _group;
			//Long _visGroup;

			// The following is not part of the spec but added for usability
			std::unordered_map<std::string, std::string> _properties;
		};

		struct MapGenerationSettings {
			Scene* _pScene;
			bool _usePhysics;
			bool _useGraphics;
			std::string _defaultImageFileExtension;
			float _sizeScalar;
			SceneObjectRef _giRef;

			MapGenerationSettings()
				: _pScene(nullptr),
				_usePhysics(true),
				_useGraphics(true),
				_defaultImageFileExtension(".png"),
				_sizeScalar(0.025f),
				_giRef(nullptr)
			{}
		};

		MapGenerationSettings _settings;

		std::vector<std::string> _nameTable;
		std::vector<MapEntity> _entities;

		// Inherited from Asset
		bool createAsset(const std::string &name);

		// Asset factory
		static Asset* assetFactory() {
			return new Map3DWS();
		}
	};

	class Map3DWSChunk : public SceneObject {
	private:
		std::shared_ptr<StaticModel> _model;

		pge::SceneObjectRef _batcherRef;

	public:
		std::shared_ptr<AssetManager> _textureManager;

		Map3DWSChunk()
			: _textureManager(new AssetManager())
		{
			_renderMask = 0xffff;
			_textureManager->create(Texture2D::assetFactory);
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
		std::shared_ptr<btTriangleMesh> _pTriangleMesh;
		std::shared_ptr<btBvhTriangleMeshShape> _pMeshShape;
		std::shared_ptr<btDefaultMotionState> _pMotionState;
		std::shared_ptr<btRigidBody> _pRigidBody;

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