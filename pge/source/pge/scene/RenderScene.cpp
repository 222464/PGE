#include <pge/scene/RenderScene.h>

#include <pge/rendering/material/Material.h>

using namespace pge;

RenderScene::RenderScene()
: _shaderSwitchesEnabled(true), _renderingShadows(false), _clearColor(0.0f, 0.0f, 0.0f, 0.0f),
_renderingEnabled(true)
{}

void RenderScene::createRenderScene(size_t numWorkers, const AABB3D &rootRegion, sf::RenderWindow* pWindow,
	const std::shared_ptr<Shader> &gBufferRender,
	const std::shared_ptr<Shader> &gBufferRenderNormal,
	const std::shared_ptr<Shader> &gBufferRenderHeightNormal,
	const std::shared_ptr<Texture2D> &whiteTexture)
{
	assert(_threadPool.getNumWorkers() == 0);

	_threadPool.create(numWorkers);

	_octree.create(rootRegion);

	_pWindow = pWindow;

	_gBuffer.create(_pWindow->getSize().x, _pWindow->getSize().y);

	_currentState.reset(new State());

	_sceneUniformBuffer.create();

	_gBufferRenderShaders[_standard] = gBufferRender;
	_gBufferRenderShaders[_normal] = gBufferRenderNormal;
	_gBufferRenderShaders[_heightNormal] = gBufferRenderHeightNormal;

	_whiteTexture = whiteTexture;

	createMaterialInterface(_gBufferRenderShaderMaterialUBOInterfaces[_standard], _gBufferRenderShaders[_standard].get());
	createMaterialInterface(_gBufferRenderShaderMaterialUBOInterfaces[_normal], _gBufferRenderShaders[_normal].get());
	createMaterialInterface(_gBufferRenderShaderMaterialUBOInterfaces[_heightNormal], _gBufferRenderShaders[_heightNormal].get());

	// Set up normalized quad, a shared resource for post render effects
	_normalizedQuad.create(true);

	_normalizedQuad._vertices.resize(4);
	_normalizedQuad._vertices[0] = Vec3f(-1.0f, -1.0f, 0.0f);
	_normalizedQuad._vertices[1] = Vec3f(1.0f, -1.0f, 0.0f);
	_normalizedQuad._vertices[2] = Vec3f(1.0f, 1.0f, 0.0f);
	_normalizedQuad._vertices[3] = Vec3f(-1.0f, 1.0f, 0.0f);

	_normalizedQuad._indices.resize(6);
	_normalizedQuad._indices[0] = 0;
	_normalizedQuad._indices[1] = 1;
	_normalizedQuad._indices[2] = 2;
	_normalizedQuad._indices[3] = 0;
	_normalizedQuad._indices[4] = 2;
	_normalizedQuad._indices[5] = 3;

	_normalizedQuad.updateBuffers();
}

void RenderScene::renderShadow() {
	_shaderSwitchesEnabled = false;
	_renderingShadows = true;

	std::vector<SceneObjectRef> visibleOld = _visible;

	findVisible(_renderCamera);

	for (SceneObjectRef &sceneObjectRef : _visible)
		sceneObjectRef->deferredRender();

	glFlush();

	for (SceneObjectRef &sceneObjectRef : _visible)
		sceneObjectRef->batchRender();

	glFlush();

	_visible = visibleOld;

	_shaderSwitchesEnabled = true;
	_renderingShadows = false;
}

void RenderScene::render(TextureRT &target) {
	updateShaderUniforms();

	findVisible(_renderCamera);

	for (size_t i = 0; i < _visible.size(); i++)
		_visible[i]->preRender();

	_gBuffer.bindDraw();
	_gBuffer.setViewport();
	_gBuffer.setDrawGeom();

	glClearColor(_clearColor.x, _clearColor.y, _clearColor.z, _clearColor.w);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (SceneObjectRef &sceneObjectRef : _visible)
		sceneObjectRef->deferredRender();

	glFlush();

	for (SceneObjectRef &sceneObjectRef : _visible)
		sceneObjectRef->batchRender();

	glFlush();

	_gBuffer.bindRead();
	_gBuffer.setDrawEffect();
	_gBuffer.setReadEffect();

	glClear(GL_COLOR_BUFFER_BIT);

	glDepthMask(GL_FALSE);

	for (SceneObjectRef &sceneObjectRef : _visible)
		sceneObjectRef->postRender();

	glFlush();

	_visible.clear();

	GBuffer::unbind();

	glDepthMask(GL_TRUE);

	// Copy GBuffer to render target
	_gBuffer.copyEffectToRenderTarget(target);

	PGE_GL_ERROR_CHECK();
}

void RenderScene::renderToMainFramebuffer() {
	updateShaderUniforms();

	findVisible(_renderCamera);

	for (size_t i = 0; i < _visible.size(); i++)
		_visible[i]->preRender();

	_gBuffer.bindDraw();
	_gBuffer.setViewport();
	_gBuffer.setDrawGeom();

	glClearColor(_clearColor.x, _clearColor.y, _clearColor.z, _clearColor.w);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (SceneObjectRef &sceneObjectRef : _visible)
		sceneObjectRef->deferredRender();

	glFlush();

	for (SceneObjectRef &sceneObjectRef : _visible)
		sceneObjectRef->batchRender();

	glFlush();

	_gBuffer.bindRead();
	_gBuffer.setDrawEffect();
	_gBuffer.setReadEffect();

	glClear(GL_COLOR_BUFFER_BIT);

	glDepthMask(GL_FALSE);

	for (SceneObjectRef &sceneObjectRef : _visible)
		sceneObjectRef->postRender();

	glFlush();

	_visible.clear();

	GBuffer::unbind();

	glDepthMask(GL_TRUE);

	// Copy GBuffer to render target
	_gBuffer.copyEffectToMainFramebuffer();

	PGE_GL_ERROR_CHECK();
}

void RenderScene::frame(float dt) {
	for (size_t i = 0; i < _currentState->_sceneObjects.size(); i++)
	if (!_currentState->_sceneObjects[i]->_shouldDestroy && ((_logicMask &_currentState->_sceneObjects[i]->_logicMask) != 0)) {
		_currentState->_sceneObjects[i]->preSynchronousUpdate(dt);

		if (_currentState->_sceneObjects[i]->_shouldDestroy) {
			_currentState->_sceneObjects[i]->onDestroy();
			_currentState->_sceneObjects[i]->removeReferences();
		}
	}
	else {
		_currentState->_sceneObjects[i]->onDestroy();
		_currentState->_sceneObjects[i]->removeReferences();
	}

	// Create new state
	_nextState.reset(new State());

	_currentState->startProcessingNextState(_threadPool, *_nextState, _logicMask, dt);

	if (_renderingEnabled)
		renderToMainFramebuffer();

	_currentState->waitForNextState(_threadPool, *_nextState);

	_logicCamera.fullUpdate();
	_renderCamera = _logicCamera;

	_currentState.reset(_nextState.release());
	_nextState = nullptr;

	for (size_t i = 0; i < _currentState->_sceneObjects.size(); i++)
	if (!_currentState->_sceneObjects[i]->_shouldDestroy && ((_logicMask &_currentState->_sceneObjects[i]->_logicMask) != 0)) {
		_currentState->_sceneObjects[i]->synchronousUpdate(dt);

		if (_currentState->_sceneObjects[i]->_shouldDestroy) {
			_currentState->_sceneObjects[i]->onDestroy();
			_currentState->_sceneObjects[i]->removeReferences();
		}
	}
	else {
		_currentState->_sceneObjects[i]->onDestroy();
		_currentState->_sceneObjects[i]->removeReferences();
	}

	_octree.pruneDeadReferences();

	// Update all octree occupants if they are flagged for an update
	for (size_t i = 0; i < _currentState->_sceneObjects.size(); i++)
	if (_currentState->_sceneObjects[i]->_needsTreeUpdate) {
		_currentState->_sceneObjects[i]->treeUpdate();

		_currentState->_sceneObjects[i]->_needsTreeUpdate = false;
	}

	// Remove name references to objects that have been destroyed
	for (std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>>::iterator itM = _namedObjects.begin(); itM != _namedObjects.end();) {
		for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator itS = itM->second.begin(); itS != itM->second.end();) {
			if (itS->isAlive())
				itS++;
			else
				itS = itM->second.erase(itS);
		}

		if (itM->second.empty())
			itM = _namedObjects.erase(itM);
		else
			itM++;
	}

	// Add new objects generated in update
	for (size_t i = 0; i < _objectsToAdd.size(); i++) {
		_currentState->_sceneObjects.push_back(_objectsToAdd[i]._object);

		_objectsToAdd[i]._object->_indexPlusOne = _currentState->_sceneObjects.size();

		if (_objectsToAdd[i]._octreeManaged)
			_octree.add(*_objectsToAdd[i]._object);
	}

	for (size_t i = 0; i < _namedObjectsToAdd.size(); i++) {
		_currentState->_sceneObjects.push_back(_namedObjectsToAdd[i]._object);

		_namedObjectsToAdd[i]._object->_indexPlusOne = _currentState->_sceneObjects.size();

		if (_namedObjectsToAdd[i]._octreeManaged)
			_octree.add(*_namedObjectsToAdd[i]._object);

		_namedObjects[_namedObjectsToAdd[i]._name].insert(*_namedObjectsToAdd[i]._object);
	}

	std::vector<NamedObjectAddData> currentNamedObjectAddData = _namedObjectsToAdd;
	std::vector<ObjectAddData> currentObjectAddData = _objectsToAdd;

	_namedObjectsToAdd.clear();
	_objectsToAdd.clear();

	for (size_t i = 0; i < currentObjectAddData.size(); i++)
		currentObjectAddData[i]._object->onAdd();

	for (size_t i = 0; i < currentNamedObjectAddData.size(); i++)
		currentNamedObjectAddData[i]._object->onAdd();
}

void RenderScene::useShader(Shader* pShader) {
	if (_shaderSwitchesEnabled) {
		pShader->bind();

		Shader::getCurrentShader()->setUniformmat4("pgeViewModel", _viewModelMatrix);
		Shader::getCurrentShader()->setUniformmat4("pgeProjectionViewModel", _projectionViewModelMatrix);
		Shader::getCurrentShader()->setUniformmat3("pgeNormal", _normalMatrix);
	}

	PGE_GL_ERROR_CHECK();
}

void RenderScene::useShader(GBufferRenderShaderType type) {
	if (_shaderSwitchesEnabled) {
		_gBufferRenderShaders[type]->bind();

		Shader::getCurrentShader()->setUniformmat4("pgeViewModel", _viewModelMatrix);
		Shader::getCurrentShader()->setUniformmat4("pgeProjectionViewModel", _projectionViewModelMatrix);
		Shader::getCurrentShader()->setUniformmat3("pgeNormal", _normalMatrix);
	}

	PGE_GL_ERROR_CHECK();
}

void RenderScene::useShader(Material &material) {
	if (_shaderSwitchesEnabled) {
		_gBufferRenderShaders[material._type]->bind();

		Shader::getCurrentShader()->setUniformmat4("pgeViewModel", _viewModelMatrix);
		Shader::getCurrentShader()->setUniformmat4("pgeProjectionViewModel", _projectionViewModelMatrix);
		Shader::getCurrentShader()->setUniformmat3("pgeNormal", _normalMatrix);

		material.bindUniformBuffer(_gBufferRenderShaderMaterialUBOInterfaces[material._type]);

		// Set material textures
		material.setUniformsTextures(Shader::getCurrentShader(), _whiteTexture.get());

		Shader::getCurrentShader()->bindShaderTextures();
	}
	else if (_renderingShadows) {
		assert(Shader::getCurrentShader() != nullptr);

		Shader::getCurrentShader()->setUniformmat4("pgeViewModel", _viewModelMatrix);
		Shader::getCurrentShader()->setUniformmat4("pgeProjectionViewModel", _projectionViewModelMatrix);
		Shader::getCurrentShader()->setUniformmat3("pgeNormal", _normalMatrix);

		if (material._pDiffuseMap != nullptr)
			Shader::getCurrentShader()->setShaderTexture("pgeDiffuseMap", material._pDiffuseMap->getTextureID(), GL_TEXTURE_2D);
		else
			Shader::getCurrentShader()->setShaderTexture("pgeDiffuseMap", getWhiteTexture()->getTextureID(), GL_TEXTURE_2D);

		Shader::getCurrentShader()->bindShaderTextures();
	}

	PGE_GL_ERROR_CHECK();
}

void RenderScene::updateShaderUniforms() {
	_sceneUniformBuffer.bind(GL_UNIFORM_BUFFER);

	_gBufferRenderShaders[_standard]->bind();
	_gBufferRenderShaders[_standard]->setUniformmat4("pgeViewModel", _renderCamera._projectionMatrix);

	_gBufferRenderShaders[_normal]->bind();
	_gBufferRenderShaders[_normal]->setUniformmat4("pgeViewModel", _renderCamera._projectionMatrix);

	_gBufferRenderShaders[_heightNormal]->bind();
	_gBufferRenderShaders[_heightNormal]->setUniformmat4("pgeViewModel", _renderCamera._projectionMatrix);

	PGE_GL_ERROR_CHECK();
}

void RenderScene::setTransform(const Matrix4x4f &transform) {
	_viewModelMatrix = _renderCamera.getViewMatrix() * transform;

	Matrix3x3f upperLeftSubmatrixInverse;

	_viewModelMatrix.getUpperLeftMatrix3x3f().inverse(upperLeftSubmatrixInverse);

	_normalMatrix = upperLeftSubmatrixInverse.transpose();

	_projectionViewModelMatrix = _renderCamera.getProjectionViewMatrix() * transform;

	Shader::getCurrentShader()->setUniformmat4("pgeViewModel", _viewModelMatrix);
	Shader::getCurrentShader()->setUniformmat4("pgeProjectionViewModel", _projectionViewModelMatrix);
	Shader::getCurrentShader()->setUniformmat3("pgeNormal", _normalMatrix);

	PGE_GL_ERROR_CHECK();
}

void RenderScene::createMaterialInterface(UBOShaderInterface &materialInterface, Shader* pShader) {
	const GLchar* materialNames[5] = {
		"pgeDiffuseColor",
		"pgeSpecularColor",
		"pgeShininess",
		"pgeEmissiveColor",
		"pgeHeightMapScale"
	};

	materialInterface.create("pgeMaterial", pShader, materialNames, 5);
}