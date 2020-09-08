#include "AssetManager.h"

#include <assert.h>

using namespace pge;

bool AssetManager::getAsset(const std::string &name) {
	std::lock_guard<std::mutex> lock(mutex);

	assert(assetFactory != nullptr);

	std::unordered_map<std::string, std::shared_ptr<Asset>>::iterator it = assets.find(name);

	if(it == assets.end()) {
		std::shared_ptr<Asset> asset(assetFactory());
		
		if (!asset->createAsset(name))
			return false;

		assets[name] = asset;

		return true;
	}

	return true;
}

bool AssetManager::getAsset(const std::string &name, std::shared_ptr<Asset> &asset) {
	std::lock_guard<std::mutex> lock(mutex);

	assert(assetFactory != nullptr);

	std::unordered_map<std::string, std::shared_ptr<Asset>>::iterator it = assets.find(name);

	if(it == assets.end()) {
		asset.reset(assetFactory());

		if (!asset->createAsset(name))
			return false;

		assets[name] = asset;
	}
	else
		asset = it->second;

	return true;
}

bool AssetManager::getAsset(const std::string &name, void* pData) {
	std::lock_guard<std::mutex> lock(mutex);

	assert(assetFactory != nullptr);

	std::unordered_map<std::string, std::shared_ptr<Asset>>::iterator it = assets.find(name);

	if(it == assets.end()) {
		std::shared_ptr<Asset> asset(assetFactory());
		
		if (!asset->createAsset(name, pData))
			return false;

		assets[name] = asset;

		return true;
	}

	return true;
}

bool AssetManager::getAsset(const std::string &name, std::shared_ptr<Asset> &asset, void* pData) {
	std::lock_guard<std::mutex> lock(mutex);

	assert(assetFactory != nullptr);

	std::unordered_map<std::string, std::shared_ptr<Asset>>::iterator it = assets.find(name);

	if(it == assets.end()) {
		asset.reset(assetFactory());

		if (!asset->createAsset(name, pData))
			return false;

		assets[name] = asset;
	}
	else
		asset = it->second;

	return true;
}

void AssetManager::destroyAsset(const std::string &name) {
	std::lock_guard<std::mutex> lock(mutex);

	std::unordered_map<std::string, std::shared_ptr<Asset>>::iterator it = assets.find(name);

	if(it != assets.end())
		assets.erase(it);
}