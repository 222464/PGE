#include "TerrainGrassBatcher.h"

#include "../../scene/RenderScene.h"

#include "../../util/Math.h"

using namespace pge;

TerrainGrassBatcher::TerrainGrassBatcher()
: waveTimer(0.0f), completelyVisibleDistance(30.0f), completelyInvisibleDistance(50.0f), waveRate(2.0f),
renderShadows(false)
{
	renderMask = 0xffff;
}

void TerrainGrassBatcher::create(const std::shared_ptr<Texture2D> &grassTileSetDiffuse, const std::shared_ptr<Texture2D> &grassTileSetNormal, const std::shared_ptr<Texture2D> &noiseMap, const std::shared_ptr<Shader> &grassRenderShader, const std::shared_ptr<Shader> &depthRenderShader) {
	this->grassTileSetDiffuse = grassTileSetDiffuse;
	this->grassTileSetNormal = grassTileSetNormal;
	this->noiseMap = noiseMap;
	this->grassRenderShader = grassRenderShader;
	this->depthRenderShader = depthRenderShader;

	grassTileSetDiffuse->genMipMaps();
	grassTileSetNormal->genMipMaps();

	grassRenderShader->bind();

	grassRenderShader->setShaderTexture("pgeDiffuseMap", grassTileSetDiffuse->getTextureID(), GL_TEXTURE_2D);
	grassRenderShader->setShaderTexture("pgeNormalMap", grassTileSetNormal->getTextureID(), GL_TEXTURE_2D);
	grassRenderShader->setShaderTexture("pgeNoiseMap", noiseMap->getTextureID(), GL_TEXTURE_2D);

	depthRenderShader->bind();

	depthRenderShader->setShaderTexture("pgeDiffuseMap", grassTileSetDiffuse->getTextureID(), GL_TEXTURE_2D);
	depthRenderShader->setShaderTexture("pgeNoiseMap", noiseMap->getTextureID(), GL_TEXTURE_2D);
}

void TerrainGrassBatcher::update(float dt) {
	waveTimer = fmodf(waveTimer + waveRate * dt, piTimes2);
}

void TerrainGrassBatcher::batchRender() {
	if (getRenderScene()->shaderSwitchesEnabled) {
		grassRenderShader->bind();
		grassRenderShader->bindShaderTextures();

		grassRenderShader->setUniformf("pgeCompletelyVisibleDistance", completelyVisibleDistance);
		//_grassRenderShader->setUniformf("pgeCompletelyInvisibleDistance", completelyInvisibleDistance);
		grassRenderShader->setUniformf("pgeInvFadeRange", 1.0f / (completelyInvisibleDistance - completelyVisibleDistance));
		grassRenderShader->setUniformf("pgeWaveTime", waveTimer);

		getRenderScene()->setTransform(Matrix4x4f::identityMatrix());

		for (std::list<SceneObjectRef>::iterator it = grassObjects.begin(); it != grassObjects.end(); it++) {
			if ((getRenderScene()->logicCamera.position - (*it)->getAABB().getCenter()).magnitude() - (*it)->getAABB().getRadius() > completelyInvisibleDistance)
				continue;

			TerrainGrass* pGrass = static_cast<TerrainGrass*>(it->get());

			for (size_t i = 0; i < pGrass->staticMeshes.size(); i++)
				pGrass->staticMeshes[i]->render();
		}
	}
	else if (getRenderScene()->renderingShadows && renderShadows) {
		Shader* pPrevShader = Shader::getCurrentShader();

		depthRenderShader->bind();
		depthRenderShader->bindShaderTextures();

		depthRenderShader->setUniformf("pgeCompletelyVisibleDistance", completelyVisibleDistance);
		//_grassRenderShader->setUniformf("pgeCompletelyInvisibleDistance", completelyInvisibleDistance);
		depthRenderShader->setUniformf("pgeInvFadeRange", 1.0f / (completelyInvisibleDistance - completelyVisibleDistance));
		depthRenderShader->setUniformf("pgeWaveTime", waveTimer);

		getRenderScene()->setTransform(Matrix4x4f::identityMatrix());

		for (std::list<SceneObjectRef>::iterator it = grassObjects.begin(); it != grassObjects.end(); it++) {
			if ((getRenderScene()->logicCamera.position - (*it)->getAABB().getCenter()).magnitude() - (*it)->getAABB().getRadius() > completelyInvisibleDistance)
				continue;

			TerrainGrass* pGrass = static_cast<TerrainGrass*>(it->get());

			for (size_t i = 0; i < pGrass->staticMeshes.size(); i++)
				pGrass->staticMeshes[i]->render();
		}

		pPrevShader->bind();
	}

	grassObjects.clear();
}