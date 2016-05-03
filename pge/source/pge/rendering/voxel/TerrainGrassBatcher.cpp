#include <pge/rendering/voxel/TerrainGrassBatcher.h>

#include <pge/scene/RenderScene.h>

#include <pge/util/Math.h>

using namespace pge;

TerrainGrassBatcher::TerrainGrassBatcher()
: _waveTimer(0.0f), _completelyVisibleDistance(30.0f), _completelyInvisibleDistance(50.0f), _waveRate(2.0f),
_renderShadows(false)
{
	_renderMask = 0xffff;
}

void TerrainGrassBatcher::create(const std::shared_ptr<Texture2D> &grassTileSetDiffuse, const std::shared_ptr<Texture2D> &grassTileSetNormal, const std::shared_ptr<Texture2D> &noiseMap, const std::shared_ptr<Shader> &grassRenderShader, const std::shared_ptr<Shader> &depthRenderShader) {
	_grassTileSetDiffuse = grassTileSetDiffuse;
	_grassTileSetNormal = grassTileSetNormal;
	_noiseMap = noiseMap;
	_grassRenderShader = grassRenderShader;
	_depthRenderShader = depthRenderShader;

	_grassTileSetDiffuse->genMipMaps();
	_grassTileSetNormal->genMipMaps();

	_grassRenderShader->bind();

	_grassRenderShader->setShaderTexture("pgeDiffuseMap", _grassTileSetDiffuse->getTextureID(), GL_TEXTURE_2D);
	_grassRenderShader->setShaderTexture("pgeNormalMap", _grassTileSetNormal->getTextureID(), GL_TEXTURE_2D);
	_grassRenderShader->setShaderTexture("pgeNoiseMap", _noiseMap->getTextureID(), GL_TEXTURE_2D);

	_depthRenderShader->bind();

	_depthRenderShader->setShaderTexture("pgeDiffuseMap", _grassTileSetDiffuse->getTextureID(), GL_TEXTURE_2D);
	_depthRenderShader->setShaderTexture("pgeNoiseMap", _noiseMap->getTextureID(), GL_TEXTURE_2D);
}

void TerrainGrassBatcher::update(float dt) {
	_waveTimer = std::fmodf(_waveTimer + _waveRate * dt, _piTimes2);
}

void TerrainGrassBatcher::batchRender() {
	if (getRenderScene()->_shaderSwitchesEnabled) {
		_grassRenderShader->bind();
		_grassRenderShader->bindShaderTextures();

		_grassRenderShader->setUniformf("pgeCompletelyVisibleDistance", _completelyVisibleDistance);
		//_grassRenderShader->setUniformf("pgeCompletelyInvisibleDistance", _completelyInvisibleDistance);
		_grassRenderShader->setUniformf("pgeInvFadeRange", 1.0f / (_completelyInvisibleDistance - _completelyVisibleDistance));
		_grassRenderShader->setUniformf("pgeWaveTime", _waveTimer);

		getRenderScene()->setTransform(Matrix4x4f::identityMatrix());

		for (std::list<SceneObjectRef>::iterator it = _grassObjects.begin(); it != _grassObjects.end(); it++) {
			if ((getRenderScene()->_logicCamera._position - (*it)->getAABB().getCenter()).magnitude() - (*it)->getAABB().getRadius() > _completelyInvisibleDistance)
				continue;

			TerrainGrass* pGrass = static_cast<TerrainGrass*>(it->get());

			for (size_t i = 0; i < pGrass->_staticMeshes.size(); i++)
				pGrass->_staticMeshes[i]->render();
		}
	}
	else if (getRenderScene()->_renderingShadows && _renderShadows) {
		Shader* pPrevShader = Shader::getCurrentShader();

		_depthRenderShader->bind();
		_depthRenderShader->bindShaderTextures();

		_depthRenderShader->setUniformf("pgeCompletelyVisibleDistance", _completelyVisibleDistance);
		//_grassRenderShader->setUniformf("pgeCompletelyInvisibleDistance", _completelyInvisibleDistance);
		_depthRenderShader->setUniformf("pgeInvFadeRange", 1.0f / (_completelyInvisibleDistance - _completelyVisibleDistance));
		_depthRenderShader->setUniformf("pgeWaveTime", _waveTimer);

		getRenderScene()->setTransform(Matrix4x4f::identityMatrix());

		for (std::list<SceneObjectRef>::iterator it = _grassObjects.begin(); it != _grassObjects.end(); it++) {
			if ((getRenderScene()->_logicCamera._position - (*it)->getAABB().getCenter()).magnitude() - (*it)->getAABB().getRadius() > _completelyInvisibleDistance)
				continue;

			TerrainGrass* pGrass = static_cast<TerrainGrass*>(it->get());

			for (size_t i = 0; i < pGrass->_staticMeshes.size(); i++)
				pGrass->_staticMeshes[i]->render();
		}

		pPrevShader->bind();
	}

	_grassObjects.clear();
}