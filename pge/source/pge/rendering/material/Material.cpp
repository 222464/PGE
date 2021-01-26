#include "Material.h"

#include "../../util/Functions.h"

#include <fstream>
#include <sstream>
#include <assert.h>

using namespace pge;

Material::Material()
: diffuseColor(1.0f, 1.0f, 1.0f), specularColor(0.0f), shininess(60.0f), emissiveColor(0.0f),
heightMapScale(0.0625f),
pDiffuseMap(nullptr), pSpecularMap(nullptr), pShininessMap(nullptr), pEmissiveMap(nullptr),
pNormalMap(nullptr), pHeightMap(nullptr),
type(RenderScene::standard),
refreshUniformBufferFlag(true)
{}

void Material::createUniformBuffer(const UBOShaderInterface &uboShaderInterface) {
	uniformBuffer.create();
	uboShaderInterface.setUpBuffer(uniformBuffer);
}

void Material::setUniformsBuffer(UBOShaderInterface &uboShaderInterface) {
	uniformBuffer.bind(GL_UNIFORM_BUFFER);

	uboShaderInterface.setUniformv3f("pgeDiffuseColor", diffuseColor);
	uboShaderInterface.setUniformf("pgeSpecularColor", specularColor);
	uboShaderInterface.setUniformf("pgeShininess", shininess);
	uboShaderInterface.setUniformf("pgeEmissiveColor", emissiveColor);
	uboShaderInterface.setUniformf("pgeHeightMapScale", heightMapScale);

	VBO::unbind(GL_UNIFORM_BUFFER);
}

void Material::setUniforms(Shader* pShader, Texture2D* pWhiteTexture) {
	pShader->setUniformv3f("pgeDiffuseColor", diffuseColor);
	pShader->setUniformf("pgeSpecularColor", specularColor);
	pShader->setUniformf("pgeShininess", shininess);
	pShader->setUniformf("pgeEmissiveColor", emissiveColor);
	pShader->setUniformf("pgeHeightMapScale", heightMapScale);
}

void Material::setUniformsTextures(Shader* pShader, Texture2D* pWhiteTexture) {
	switch (type) {
	case RenderScene::standard:
		pShader->setShaderTexture("pgeDiffuseMap", pDiffuseMap == nullptr ? pWhiteTexture->getTextureID() : pDiffuseMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeSpecularMap", pSpecularMap == nullptr ? pWhiteTexture->getTextureID() : pSpecularMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeShininessMap", pShininessMap == nullptr ? pWhiteTexture->getTextureID() : pShininessMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeEmissiveMap", pEmissiveMap == nullptr ? pWhiteTexture->getTextureID() : pEmissiveMap->getTextureID(), GL_TEXTURE_2D);

		break;

	case RenderScene::normal:
		pShader->setShaderTexture("pgeDiffuseMap", pDiffuseMap == nullptr ? pWhiteTexture->getTextureID() : pDiffuseMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeSpecularMap", pSpecularMap == nullptr ? pWhiteTexture->getTextureID() : pSpecularMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeShininessMap", pShininessMap == nullptr ? pWhiteTexture->getTextureID() : pShininessMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeEmissiveMap", pEmissiveMap == nullptr ? pWhiteTexture->getTextureID() : pEmissiveMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeNormalMap", pNormalMap == nullptr ? pWhiteTexture->getTextureID() : pNormalMap->getTextureID(), GL_TEXTURE_2D);

		break;

	case RenderScene::heightNormal:
	default:
		pShader->setShaderTexture("pgeDiffuseMap", pDiffuseMap == nullptr ? pWhiteTexture->getTextureID() : pDiffuseMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeSpecularMap", pSpecularMap == nullptr ? pWhiteTexture->getTextureID() : pSpecularMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeShininessMap", pShininessMap == nullptr ? pWhiteTexture->getTextureID() : pShininessMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeEmissiveMap", pEmissiveMap == nullptr ? pWhiteTexture->getTextureID() : pEmissiveMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeNormalMap", pNormalMap == nullptr ? pWhiteTexture->getTextureID() : pNormalMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeHeightMap", pHeightMap == nullptr ? pWhiteTexture->getTextureID() : pHeightMap->getTextureID(), GL_TEXTURE_2D);

		break;
	}
}

void Material::bindUniformBuffer(UBOShaderInterface &uboShaderInterface) {
	if (refreshUniformBufferFlag) {
		if (!uniformBuffer.created())
			createUniformBuffer(uboShaderInterface);

		setUniformsBuffer(uboShaderInterface);

		refreshUniformBufferFlag = false;
	}

	uboShaderInterface.bindBufferToSetIndex(uniformBuffer);
}

void Material::updateShaderType() {
	if (pNormalMap != nullptr) {
		if (pHeightMap != nullptr)
			type = RenderScene::heightNormal;
		else
			type = RenderScene::normal;
	}
	else
		type = RenderScene::standard;
}

void Material::genMipMaps() {
	if (pDiffuseMap != nullptr)
		pDiffuseMap->genMipMaps();

	if (pSpecularMap != nullptr)
		pSpecularMap->genMipMaps();

	if (pShininessMap != nullptr)
		pShininessMap->genMipMaps();

	if (pEmissiveMap != nullptr)
		pEmissiveMap->genMipMaps();

	if (pNormalMap != nullptr)
		pNormalMap->genMipMaps();

	if (pHeightMap != nullptr)
		pHeightMap->genMipMaps();
}

bool Material::loadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::vector<Material> &materials) {
	std::string rootName(getRootName(fileName));

	std::ifstream fromFile(fileName);

	if(!fromFile.is_open()) {
#ifdef PGE_DEBUG
		std::cerr << "Could not load material file " << fileName << std::endl;
#endif
		return false;
	}

	Material* pCurrent = nullptr;

	while(!fromFile.eof()) {
		// Read line header
		std::string line;
		getline(fromFile, line);

		std::stringstream ss(line);

		std::string header;
		ss >> header;

		if(header == "newmtl") {
			materials.push_back(Material());

			pCurrent = &materials.back();
		}
		else if(header == "Ka") {
			if(pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}
			
			ss >> pCurrent->emissiveColor;
		}
		else if(header == "Kd") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}
			
			ss >> pCurrent->diffuseColor.x >> pCurrent->diffuseColor.y >> pCurrent->diffuseColor.z;
		}
		else if(header == "Ks") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}
			
			ss >> pCurrent->specularColor;
		}
		else if (header == "Ns") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->shininess;
		}
		else if (header == "map_Ka") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pEmissiveMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Kd") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pDiffuseMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Ks") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pSpecularMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Ki") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pShininessMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "bump" || header == "map_Bump") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pNormalMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "height" || header == "map_Height") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pHeightMap = static_cast<Texture2D*>(pAsset.get());
		}
	}

	for (Material &m : materials)
		m.updateShaderType();

	assert(!materials.empty());

	return true;
}

bool Material::loadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::vector<std::string> &materialNames, std::vector<Material> &materials) {
	std::string rootName(getRootName(fileName));

	std::ifstream fromFile(fileName);

	if (!fromFile.is_open()) {
#ifdef PGE_DEBUG
		std::cerr << "Could not load material file " << fileName << std::endl;
#endif
		return false;
	}

	Material* pCurrent = nullptr;

	while (!fromFile.eof()) {
		// Read line header
		std::string line;
		getline(fromFile, line);

		std::stringstream ss(line);

		std::string header;
		ss >> header;

		if (header == "newmtl") {
			materials.push_back(Material());

			std::string matName;
			ss >> matName;

			materialNames.push_back(matName);

			pCurrent = &materials.back();
		}
		else if (header == "Ka") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->emissiveColor;
		}
		else if (header == "Kd") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->diffuseColor.x >> pCurrent->diffuseColor.y >> pCurrent->diffuseColor.z;
		}
		else if (header == "Ks") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->specularColor;
		}
		else if (header == "Ns") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->shininess;
		}
		else if (header == "map_Ka") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pEmissiveMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Kd") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pDiffuseMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Ks") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pSpecularMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Ki") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pShininessMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "bump" || header == "map_Bump") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pNormalMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "height" || header == "map_Height") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pHeightMap = static_cast<Texture2D*>(pAsset.get());
		}
	}

	for (Material &m : materials)
		m.updateShaderType();

	assert(!materials.empty());

	return true;
}

bool Material::loadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::unordered_map<std::string, size_t> &materialNamesToIndicesMap, std::vector<Material> &materials) {
	std::string rootName(getRootName(fileName));

	std::ifstream fromFile(fileName);

	if (!fromFile.is_open()) {
#ifdef PGE_DEBUG
		std::cerr << "Could not load material file " << fileName << std::endl;
#endif
		return false;
	}

	Material* pCurrent = nullptr;

	while (!fromFile.eof()) {
		// Read line header
		std::string line;
		getline(fromFile, line);

		std::stringstream ss(line);

		std::string header;
		ss >> header;

		if (header == "newmtl") {
			materials.push_back(Material());

			std::string matName;
			ss >> matName;

			materialNamesToIndicesMap[matName] = materials.size() - 1;

			pCurrent = &materials.back();
		}
		else if (header == "Ka") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->emissiveColor;
		}
		else if (header == "Kd") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->diffuseColor.x >> pCurrent->diffuseColor.y >> pCurrent->diffuseColor.z;
		}
		else if (header == "Ks") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->specularColor;
		}
		else if (header == "Ns") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->shininess;
		}
		else if (header == "map_Ka") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pEmissiveMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Kd") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pDiffuseMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Ks") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pSpecularMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Ki") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pShininessMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "bump" || header == "map_Bump") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pNormalMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "height" || header == "map_Height") {
			if (pCurrent == nullptr) {
#ifdef PGE_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			std::string mapName;
			ss >> mapName;

			std::string relativeDir(rootName + mapName);
			std::ifstream existanceTester(relativeDir);

			if (existanceTester.good())
				mapName = relativeDir;

			std::shared_ptr<Asset> pAsset;

			if (!pTextureManager->getAsset(mapName, pAsset)) {
#ifdef PGE_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->pHeightMap = static_cast<Texture2D*>(pAsset.get());
		}
	}

	for (Material &m : materials)
		m.updateShaderType();

	assert(!materials.empty());

	return true;
}
