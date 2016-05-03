#include <pge/rendering/material/Material.h>

#include <pge/util/Functions.h>

#include <fstream>
#include <sstream>
#include <assert.h>

using namespace pge;

Material::Material()
: _diffuseColor(1.0f, 1.0f, 1.0f), _specularColor(0.0f), _shininess(60.0f), _emissiveColor(0.0f),
_heightMapScale(0.0625f),
_pDiffuseMap(nullptr), _pSpecularMap(nullptr), _pShininessMap(nullptr), _pEmissiveMap(nullptr),
_pNormalMap(nullptr), _pHeightMap(nullptr),
_type(RenderScene::_standard),
_refreshUniformBuffer(true)
{}

void Material::createUniformBuffer(const UBOShaderInterface &uboShaderInterface) {
	_uniformBuffer.create();
	uboShaderInterface.setUpBuffer(_uniformBuffer);
}

void Material::setUniformsBuffer(UBOShaderInterface &uboShaderInterface) {
	_uniformBuffer.bind(GL_UNIFORM_BUFFER);

	uboShaderInterface.setUniformv3f("pgeDiffuseColor", _diffuseColor);
	uboShaderInterface.setUniformf("pgeSpecularColor", _specularColor);
	uboShaderInterface.setUniformf("pgeShininess", _shininess);
	uboShaderInterface.setUniformf("pgeEmissiveColor", _emissiveColor);
	uboShaderInterface.setUniformf("pgeHeightMapScale", _heightMapScale);

	VBO::unbind(GL_UNIFORM_BUFFER);
}

void Material::setUniforms(Shader* pShader, Texture2D* pWhiteTexture) {
	pShader->setUniformv3f("pgeDiffuseColor", _diffuseColor);
	pShader->setUniformf("pgeSpecularColor", _specularColor);
	pShader->setUniformf("pgeShininess", _shininess);
	pShader->setUniformf("pgeEmissiveColor", _emissiveColor);
	pShader->setUniformf("pgeHeightMapScale", _heightMapScale);
}

void Material::setUniformsTextures(Shader* pShader, Texture2D* pWhiteTexture) {
	switch (_type) {
	case RenderScene::_standard:
		pShader->setShaderTexture("pgeDiffuseMap", _pDiffuseMap == nullptr ? pWhiteTexture->getTextureID() : _pDiffuseMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeSpecularMap", _pSpecularMap == nullptr ? pWhiteTexture->getTextureID() : _pSpecularMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeShininessMap", _pShininessMap == nullptr ? pWhiteTexture->getTextureID() : _pShininessMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeEmissiveMap", _pEmissiveMap == nullptr ? pWhiteTexture->getTextureID() : _pEmissiveMap->getTextureID(), GL_TEXTURE_2D);

		break;

	case RenderScene::_normal:
		pShader->setShaderTexture("pgeDiffuseMap", _pDiffuseMap == nullptr ? pWhiteTexture->getTextureID() : _pDiffuseMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeSpecularMap", _pSpecularMap == nullptr ? pWhiteTexture->getTextureID() : _pSpecularMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeShininessMap", _pShininessMap == nullptr ? pWhiteTexture->getTextureID() : _pShininessMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeEmissiveMap", _pEmissiveMap == nullptr ? pWhiteTexture->getTextureID() : _pEmissiveMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeNormalMap", _pNormalMap == nullptr ? pWhiteTexture->getTextureID() : _pNormalMap->getTextureID(), GL_TEXTURE_2D);

		break;

	case RenderScene::_heightNormal:
	default:
		pShader->setShaderTexture("pgeDiffuseMap", _pDiffuseMap == nullptr ? pWhiteTexture->getTextureID() : _pDiffuseMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeSpecularMap", _pSpecularMap == nullptr ? pWhiteTexture->getTextureID() : _pSpecularMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeShininessMap", _pShininessMap == nullptr ? pWhiteTexture->getTextureID() : _pShininessMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeEmissiveMap", _pEmissiveMap == nullptr ? pWhiteTexture->getTextureID() : _pEmissiveMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeNormalMap", _pNormalMap == nullptr ? pWhiteTexture->getTextureID() : _pNormalMap->getTextureID(), GL_TEXTURE_2D);
		pShader->setShaderTexture("pgeHeightMap", _pHeightMap == nullptr ? pWhiteTexture->getTextureID() : _pHeightMap->getTextureID(), GL_TEXTURE_2D);

		break;
	}
}

void Material::bindUniformBuffer(UBOShaderInterface &uboShaderInterface) {
	if (_refreshUniformBuffer) {
		if (!_uniformBuffer.created())
			createUniformBuffer(uboShaderInterface);

		setUniformsBuffer(uboShaderInterface);

		_refreshUniformBuffer = false;
	}

	uboShaderInterface.bindBufferToSetIndex(_uniformBuffer);
}

void Material::updateShaderType() {
	if (_pNormalMap != nullptr) {
		if (_pHeightMap != nullptr)
			_type = RenderScene::_heightNormal;
		else
			_type = RenderScene::_normal;
	}
	else
		_type = RenderScene::_standard;
}

void Material::genMipMaps() {
	if (_pDiffuseMap != nullptr)
		_pDiffuseMap->genMipMaps();

	if (_pSpecularMap != nullptr)
		_pSpecularMap->genMipMaps();

	if (_pShininessMap != nullptr)
		_pShininessMap->genMipMaps();

	if (_pEmissiveMap != nullptr)
		_pEmissiveMap->genMipMaps();

	if (_pNormalMap != nullptr)
		_pNormalMap->genMipMaps();

	if (_pHeightMap != nullptr)
		_pHeightMap->genMipMaps();
}

bool Material::loadFromMTL(const std::string &fileName, AssetManager* pTextureManager, std::vector<Material> &materials) {
	std::string rootName(getRootName(fileName));

	std::ifstream fromFile(fileName);

	if(!fromFile.is_open()) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}
			
			ss >> pCurrent->_emissiveColor;
		}
		else if(header == "Kd") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}
			
			ss >> pCurrent->_diffuseColor.x >> pCurrent->_diffuseColor.y >> pCurrent->_diffuseColor.z;
		}
		else if(header == "Ks") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}
			
			ss >> pCurrent->_specularColor;
		}
		else if (header == "Ns") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->_shininess;
		}
		else if (header == "map_Ka") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pEmissiveMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Kd") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pDiffuseMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Ks") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pSpecularMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Ki") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pShininessMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "bump" || header == "map_Bump") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pNormalMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "height" || header == "map_Height") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pHeightMap = static_cast<Texture2D*>(pAsset.get());
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
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->_emissiveColor;
		}
		else if (header == "Kd") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->_diffuseColor.x >> pCurrent->_diffuseColor.y >> pCurrent->_diffuseColor.z;
		}
		else if (header == "Ks") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->_specularColor;
		}
		else if (header == "Ns") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->_shininess;
		}
		else if (header == "map_Ka") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pEmissiveMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Kd") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pDiffuseMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Ks") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pSpecularMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Ki") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pShininessMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "bump" || header == "map_Bump") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pNormalMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "height" || header == "map_Height") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pHeightMap = static_cast<Texture2D*>(pAsset.get());
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
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->_emissiveColor;
		}
		else if (header == "Kd") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->_diffuseColor.x >> pCurrent->_diffuseColor.y >> pCurrent->_diffuseColor.z;
		}
		else if (header == "Ks") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->_specularColor;
		}
		else if (header == "Ns") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
				std::cerr << "Attempted to set material parameter before defining material in \"" << fileName << "\"!" << std::endl;
#endif
				return false;
			}

			ss >> pCurrent->_shininess;
		}
		else if (header == "map_Ka") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pEmissiveMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Kd") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pDiffuseMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Ks") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pSpecularMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "map_Ki") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pShininessMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "bump" || header == "map_Bump") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pNormalMap = static_cast<Texture2D*>(pAsset.get());
		}
		else if (header == "height" || header == "map_Height") {
			if (pCurrent == nullptr) {
#ifdef D3D_DEBUG
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
#ifdef D3D_DEBUG
				std::cerr << "- in " << fileName << std::endl;
#endif
				return false;
			}

			pCurrent->_pHeightMap = static_cast<Texture2D*>(pAsset.get());
		}
	}

	for (Material &m : materials)
		m.updateShaderType();

	assert(!materials.empty());

	return true;
}