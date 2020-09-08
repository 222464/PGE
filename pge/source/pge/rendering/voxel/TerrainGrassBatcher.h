#pragma once

#include "../../scene/SceneObject.h"
#include "../mesh/StaticMesh.h"

#include "TerrainGrass.h"

namespace pge {
	class TerrainGrassBatcher : public SceneObject {
	private:
		std::list<SceneObjectRef> grassObjects;

		std::shared_ptr<Shader> grassRenderShader;

		std::shared_ptr<Shader> depthRenderShader;

		std::shared_ptr<Texture2D> grassTileSetDiffuse;
		std::shared_ptr<Texture2D> grassTileSetNormal;
		std::shared_ptr<Texture2D> noiseMap;

		float waveTimer;

	public:
		float completelyVisibleDistance;
		float completelyInvisibleDistance;

		float waveRate;

		bool renderShadows;

		TerrainGrassBatcher();

		void create(const std::shared_ptr<Texture2D> &grassTileSetDiffuse, const std::shared_ptr<Texture2D> &grassTileSetNormal, const std::shared_ptr<Texture2D> &noiseMap, const std::shared_ptr<Shader> &grassRenderShader, const std::shared_ptr<Shader> &depthRenderShader);

		// Inherited from SceneObject
		void update(float dt);
		void batchRender();

		SceneObject* copyFactory() {
			return new TerrainGrassBatcher(*this);
		}

		friend TerrainGrass;
	};
}