#pragma once

#include <pge/scene/RenderScene.h>
#include <pge/rendering/texture/Texture2D.h>
#include <pge/rendering/bufferobjects/VBO.h>

namespace pge {
	// Set texture pointers to NULL to indicate it is not used
	class Material {
	private:
		VBO _uniformBuffer;

		bool _refreshUniformBuffer;

	public:
		Vec3f _diffuseColor;
		float _specularColor;
		float _shininess;
		float _emissiveColor;
		
		// Special
		float _heightMapScale;

		Texture2D* _pDiffuseMap;
		Texture2D* _pSpecularMap;
		Texture2D* _pShininessMap;
		Texture2D* _pEmissiveMap;

		// Special
		Texture2D* _pNormalMap;
		Texture2D* _pHeightMap;

		RenderScene::GBufferRenderShaderType _type;

		Material();

		void createUniformBuffer(const UBOShaderInterface &uboShaderInterface);

		void setUniforms(Shader* pShader, Texture2D* pWhiteTexture);
		void setUniformsTextures(Shader* pShader, Texture2D* pWhiteTexture);
		void setUniformsBuffer(UBOShaderInterface &uboShaderInterface);
		void bindUniformBuffer(UBOShaderInterface &uboShaderInterface);

		const VBO &getUniformBuffer() const {
			return _uniformBuffer;
		}

		RenderScene::GBufferRenderShaderType getShaderType() const {
			return _type;
		}

		void updateShaderType();

		void refreshUniformBuffer() {
			_refreshUniformBuffer = true;
		}

		void genMipMaps();

		static bool loadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::vector<Material> &materials);
		static bool loadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::vector<std::string> &materialNames, std::vector<Material> &materials);
		static bool loadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::unordered_map<std::string, size_t> &materialNamesToIndicesMap, std::vector<Material> &materials);
	};
}