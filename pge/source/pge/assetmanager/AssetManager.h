#pragma once

#include "Asset.h"

#include "../system/Uncopyable.h"

#include <unordered_map>
#include <memory>
#include <mutex>

/* 
Asset manager requires a factory function for a particular derivative of the Asset class.
This can be provided in the ctor or later using Create(...)
Stuff returned by GetAsset functions may be casted to the derived class
*/

namespace pge {
    class AssetManager : public Uncopyable {
    private:
        std::unordered_map<std::string, std::shared_ptr<Asset>> assets;

        // Factory function pointer
        Asset* (*assetFactory)();

        std::mutex mutex;

    public:
        AssetManager()
            : assetFactory(nullptr)
        {}

        void create(Asset* (*assetFactory)()) {
            this->assetFactory = assetFactory;
        }

        bool created() const {
            return assetFactory != nullptr;
        }

        bool getAsset(const std::string &name);
        bool getAsset(const std::string &name, std::shared_ptr<Asset> &asset);
        bool getAsset(const std::string &name, void* pData);
        bool getAsset(const std::string &name, std::shared_ptr<Asset> &asset, void* pData);
        void destroyAsset(const std::string &name);

        void clearAssets() {
            assets.clear();
        }
    };
}