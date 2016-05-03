#pragma once

#include <string>

#include <pge/system/Uncopyable.h>

namespace pge {
	class Asset : public Uncopyable {
	public:
		virtual ~Asset() {}
		virtual bool createAsset(const std::string &name) { return true; }
		virtual bool createAsset(const std::string &name, void* pData) { return true; }
	};
}
