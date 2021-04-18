#pragma once

#include "../../scene/RenderScene.h"
#include "../texture/Texture2D.h"
#include "../bufferobjects/VBO.h"

namespace pge {
    // Set texture pointers to NULL to indicate it is not used
    class Material {
    private:
        VBO uniformBuffer;

        bool refreshUniformBufferFlag;

    public:
        Vec3f diffuseColor;
        float specularColor;
        float shininess;
        float emissiveColor;
        
        // Special
        float heightMapScale;

        Texture2D* pDiffuseMap;
        Texture2D* pSpecularMap;
        Texture2D* pShininessMap;
        Texture2D* pEmissiveMap;

        // Special
        Texture2D* pNormalMap;
        Texture2D* pHeightMap;

        RenderScene::GBufferRenderShaderType type;

        Material();

        void createUniformBuffer(const UBOShaderInterface &uboShaderInterface);

        void setUniforms(Shader* pShader, Texture2D* pWhiteTexture);
        void setUniformsTextures(Shader* pShader, Texture2D* pWhiteTexture);
        void setUniformsBuffer(UBOShaderInterface &uboShaderInterface);
        void bindUniformBuffer(UBOShaderInterface &uboShaderInterface);

        const VBO &getUniformBuffer() const {
            return uniformBuffer;
        }

        RenderScene::GBufferRenderShaderType getShaderType() const {
            return type;
        }

        void updateShaderType();

        void refreshUniformBuffer() {
            refreshUniformBufferFlag = true;
        }

        void genMipMaps();

        static bool loadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::vector<Material> &materials);
        static bool loadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::vector<std::string> &materialNames, std::vector<Material> &materials);
        static bool loadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::unordered_map<std::string, size_t> &materialNamesToIndicesMap, std::vector<Material> &materials);
    };
}