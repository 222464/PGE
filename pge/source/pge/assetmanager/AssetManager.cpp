#include <pge/assetmanager/AssetManager.h>

#include <assert.h>

using namespace pge;

bool AssetManager::getAsset(const std::string &name) {
	std::lock_guard<std::mutex> lock(_mutex);

	assert(_assetFactory != nullptr);

	std::unordered_map<std::string, std::shared_ptr<Asset>>::iterator it = _assets.find(name);

	if(it == _assets.end()) {
		std::shared_ptr<Asset> asset(_assetFactory());
		
		if (!asset->createAsset(name))
			return false;

		_assets[name] = asset;

		return true;
	}

	return true;
}

bool AssetManager::getAsset(const std::string &name, std::shared_ptr<Asset> &asset) {
	std::lock_guard<std::mutex> lock(_mutex);

	assert(_assetFactory != nullptr);

	std::unordered_map<std::string, std::shared_ptr<Asset>>::iterator it = _assets.find(name);

	if(it == _assets.end()) {
		asset.reset(_assetFactory());

		if (!asset->createAsset(name))
			return false;

		_assets[name] = asset;
	}
	else
		asset = it->second;

	return true;
}

bool AssetManager::getAsset(const std::string &name, void* pData) {
	std::lock_guard<std::mutex> lock(_mutex);

	assert(_assetFactory != nullptr);

	std::unordered_map<std::string, std::shared_ptr<Asset>>::iterator it = _assets.find(name);

	if(it == _assets.end()) {
		std::shared_ptr<Asset> asset(_assetFactory());
		
		if (!asset->createAsset(name, pData))
			return false;

		_assets[name] = asset;

		return true;
	}

	return true;
}

bool AssetManager::getAsset(const std::string &name, std::shared_ptr<Asset> &asset, void* pData) {
	std::lock_guard<std::mutex> lock(_mutex);

	assert(_assetFactory != nullptr);

	std::unordered_map<std::string, std::shared_ptr<Asset>>::iterator it = _assets.find(name);

	if(it == _assets.end()) {
		asset.reset(_assetFactory());

		if (!asset->createAsset(name, pData))
			return false;

		_assets[name] = asset;
	}
	else
		asset = it->second;

	return true;
}

void AssetManager::destroyAsset(const std::string &name) {
	std::lock_guard<std::mutex> lock(_mutex);

	std::unordered_map<std::string, std::shared_ptr<Asset>>::iterator it = _assets.find(name);

	if(it != _assets.end())
		_assets.erase(it);
}