#include "RenderScene.h"

#include "../rendering/material/Material.h"

using namespace pge;

RenderScene::RenderScene()
: shaderSwitchesEnabled(true), renderingShadows(false), clearColor(0.0f, 0.0f, 0.0f, 0.0f),
renderingEnabled(true)
{}

void RenderScene::createRenderScene(size_t numWorkers, const AABB3D &rootRegion, sf::Window* pWindow,
	const std::shared_ptr<Shader> &gBufferRender,
	const std::shared_ptr<Shader> &gBufferRenderNormal,
	const std::shared_ptr<Shader> &gBufferRenderHeightNormal,
	const std::shared_ptr<Texture2D> &whiteTexture)
{
	assert(threadPool.getNumWorkers() == 0);

	threadPool.create(numWorkers);

	octree.create(rootRegion);

	this->pWindow = pWindow;

	gBuffer.create(pWindow->getSize().x, pWindow->getSize().y);

	currentState.reset(new State());

	sceneUniformBuffer.create();

	gBufferRenderShaders[standard] = gBufferRender;
	gBufferRenderShaders[normal] = gBufferRenderNormal;
	gBufferRenderShaders[heightNormal] = gBufferRenderHeightNormal;

	this->whiteTexture = whiteTexture;

	createMaterialInterface(gBufferRenderShaderMaterialUBOInterfaces[standard], gBufferRenderShaders[standard].get());
	createMaterialInterface(gBufferRenderShaderMaterialUBOInterfaces[normal], gBufferRenderShaders[normal].get());
	createMaterialInterface(gBufferRenderShaderMaterialUBOInterfaces[heightNormal], gBufferRenderShaders[heightNormal].get());

	// Set up normalized quad, a shared resource for post render effects
	normalizedQuad.create(true);

	normalizedQuad.vertices.resize(4);
	normalizedQuad.vertices[0] = Vec3f(-1.0f, -1.0f, 0.0f);
	normalizedQuad.vertices[1] = Vec3f(1.0f, -1.0f, 0.0f);
	normalizedQuad.vertices[2] = Vec3f(1.0f, 1.0f, 0.0f);
	normalizedQuad.vertices[3] = Vec3f(-1.0f, 1.0f, 0.0f);

	normalizedQuad.indices.resize(6);
	normalizedQuad.indices[0] = 0;
	normalizedQuad.indices[1] = 1;
	normalizedQuad.indices[2] = 2;
	normalizedQuad.indices[3] = 0;
	normalizedQuad.indices[4] = 2;
	normalizedQuad.indices[5] = 3;

	normalizedQuad.updateBuffers();
}

void RenderScene::renderShadow() {
	shaderSwitchesEnabled = false;
	renderingShadows = true;

	std::vector<SceneObjectRef> visibleOld = visible;

	findVisible(renderCamera);

	for (SceneObjectRef &sceneObjectRef : visible)
		sceneObjectRef->deferredRender();

	glFlush();

	for (SceneObjectRef &sceneObjectRef : visible)
		sceneObjectRef->batchRender();

	glFlush();

	visible = visibleOld;

	shaderSwitchesEnabled = true;
	renderingShadows = false;
}

void RenderScene::render(TextureRT &target) {
	updateShaderUniforms();

	findVisible(renderCamera);

	for (size_t i = 0; i < visible.size(); i++)
		visible[i]->preRender();

	gBuffer.bindDraw();
	gBuffer.setViewport();
	gBuffer.setDrawGeom();

	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (SceneObjectRef &sceneObjectRef : visible)
		sceneObjectRef->deferredRender();

	glFlush();

	for (SceneObjectRef &sceneObjectRef : visible)
		sceneObjectRef->batchRender();

	glFlush();

	gBuffer.bindRead();
	gBuffer.setDrawEffect();
	gBuffer.setReadEffect();

	glClear(GL_COLOR_BUFFER_BIT);

	glDepthMask(GL_FALSE);

	for (SceneObjectRef &sceneObjectRef : visible)
		sceneObjectRef->postRender();

	glFlush();

	visible.clear();

	GBuffer::unbind();

	glDepthMask(GL_TRUE);

	// Copy GBuffer to render target
	gBuffer.copyEffectToRenderTarget(target);

	PGE_GL_ERROR_CHECK();
}

void RenderScene::renderToMainFramebuffer() {
	updateShaderUniforms();

	findVisible(renderCamera);

	for (size_t i = 0; i < visible.size(); i++)
		visible[i]->preRender();

	gBuffer.bindDraw();
	gBuffer.setViewport();
	gBuffer.setDrawGeom();

	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (SceneObjectRef &sceneObjectRef : visible)
		sceneObjectRef->deferredRender();

	glFlush();

	for (SceneObjectRef &sceneObjectRef : visible)
		sceneObjectRef->batchRender();

	glFlush();

	gBuffer.bindRead();
	gBuffer.setDrawEffect();
	gBuffer.setReadEffect();

	glClear(GL_COLOR_BUFFER_BIT);

	glDepthMask(GL_FALSE);

	for (SceneObjectRef &sceneObjectRef : visible)
		sceneObjectRef->postRender();

	glFlush();

	visible.clear();

	GBuffer::unbind();

	glDepthMask(GL_TRUE);

	// Copy GBuffer to render target
	gBuffer.copyEffectToMainFramebuffer();

	PGE_GL_ERROR_CHECK();
}

void RenderScene::frame(float dt) {
	for (size_t i = 0; i < currentState->sceneObjects.size(); i++)
	if (!currentState->sceneObjects[i]->shouldDestroy() && ((logicMask &currentState->sceneObjects[i]->logicMask) != 0)) {
		currentState->sceneObjects[i]->preSynchronousUpdate(dt);

		if (currentState->sceneObjects[i]->shouldDestroy()) {
			currentState->sceneObjects[i]->onDestroy();
			currentState->sceneObjects[i]->removeReferences();
		}
	}
	else {
		currentState->sceneObjects[i]->onDestroy();
		currentState->sceneObjects[i]->removeReferences();
	}

	// Create new state
	nextState.reset(new State());

	currentState->startProcessingNextState(threadPool, *nextState, logicMask, dt);

	if (renderingEnabled)
		renderToMainFramebuffer();

	currentState->waitForNextState(threadPool, *nextState);

	logicCamera.fullUpdate();
	renderCamera = logicCamera;

	currentState.reset(nextState.release());
	nextState = nullptr;

	for (size_t i = 0; i < currentState->sceneObjects.size(); i++)
	if (!currentState->sceneObjects[i]->shouldDestroy() && ((logicMask &currentState->sceneObjects[i]->logicMask) != 0)) {
		currentState->sceneObjects[i]->synchronousUpdate(dt);

		if (currentState->sceneObjects[i]->shouldDestroy()) {
			currentState->sceneObjects[i]->onDestroy();
			currentState->sceneObjects[i]->removeReferences();
		}
	}
	else {
		currentState->sceneObjects[i]->onDestroy();
		currentState->sceneObjects[i]->removeReferences();
	}

	octree.pruneDeadReferences();

	// Update all octree occupants if they are flagged for an update
	for (size_t i = 0; i < currentState->sceneObjects.size(); i++)
	if (currentState->sceneObjects[i]->needsTreeUpdate) {
		currentState->sceneObjects[i]->treeUpdate();

		currentState->sceneObjects[i]->needsTreeUpdate = false;
	}

	// Remove name references to objects that have been destroyed
	for (std::unordered_map<std::string, std::unordered_set<SceneObjectRef, SceneObjectRef>>::iterator itM = namedObjects.begin(); itM != namedObjects.end();) {
		for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator itS = itM->second.begin(); itS != itM->second.end();) {
			if (itS->isAlive())
				itS++;
			else
				itS = itM->second.erase(itS);
		}

		if (itM->second.empty())
			itM = namedObjects.erase(itM);
		else
			itM++;
	}

	// Add new objects generated in update
	for (size_t i = 0; i < objectsToAdd.size(); i++) {
		currentState->sceneObjects.push_back(objectsToAdd[i].object);

		objectsToAdd[i].object->indexPlusOne = currentState->sceneObjects.size();

		if (objectsToAdd[i].octreeManaged)
			octree.add(*objectsToAdd[i].object);
	}

	for (size_t i = 0; i < namedObjectsToAdd.size(); i++) {
		currentState->sceneObjects.push_back(namedObjectsToAdd[i].object);

		namedObjectsToAdd[i].object->indexPlusOne = currentState->sceneObjects.size();

		if (namedObjectsToAdd[i].octreeManaged)
			octree.add(*namedObjectsToAdd[i].object);

		namedObjects[namedObjectsToAdd[i].name].insert(*namedObjectsToAdd[i].object);
	}

	std::vector<NamedObjectAddData> currentNamedObjectAddData = namedObjectsToAdd;
	std::vector<ObjectAddData> currentObjectAddData = objectsToAdd;

	namedObjectsToAdd.clear();
	objectsToAdd.clear();

	for (size_t i = 0; i < currentObjectAddData.size(); i++)
		currentObjectAddData[i].object->onAdd();

	for (size_t i = 0; i < currentNamedObjectAddData.size(); i++)
		currentNamedObjectAddData[i].object->onAdd();
}

void RenderScene::useShader(Shader* pShader) {
	if (shaderSwitchesEnabled) {
		pShader->bind();

		Shader::getCurrentShader()->setUniformmat4("pgeViewModel", viewModelMatrix);
		Shader::getCurrentShader()->setUniformmat4("pgeProjectionViewModel", projectionViewModelMatrix);
		Shader::getCurrentShader()->setUniformmat3("pgeNormal", normalMatrix);
	}

	PGE_GL_ERROR_CHECK();
}

void RenderScene::useShader(GBufferRenderShaderType type) {
	if (shaderSwitchesEnabled) {
		gBufferRenderShaders[type]->bind();

		Shader::getCurrentShader()->setUniformmat4("pgeViewModel", viewModelMatrix);
		Shader::getCurrentShader()->setUniformmat4("pgeProjectionViewModel", projectionViewModelMatrix);
		Shader::getCurrentShader()->setUniformmat3("pgeNormal", normalMatrix);
	}

	PGE_GL_ERROR_CHECK();
}

void RenderScene::useShader(Material &material) {
	if (shaderSwitchesEnabled) {
		gBufferRenderShaders[material.type]->bind();

		Shader::getCurrentShader()->setUniformmat4("pgeViewModel", viewModelMatrix);
		Shader::getCurrentShader()->setUniformmat4("pgeProjectionViewModel", projectionViewModelMatrix);
		Shader::getCurrentShader()->setUniformmat3("pgeNormal", normalMatrix);

		material.bindUniformBuffer(gBufferRenderShaderMaterialUBOInterfaces[material.type]);

		// Set material textures
		material.setUniformsTextures(Shader::getCurrentShader(), whiteTexture.get());

		Shader::getCurrentShader()->bindShaderTextures();
	}
	else if (renderingShadows) {
		assert(Shader::getCurrentShader() != nullptr);

		Shader::getCurrentShader()->setUniformmat4("pgeViewModel", viewModelMatrix);
		Shader::getCurrentShader()->setUniformmat4("pgeProjectionViewModel", projectionViewModelMatrix);
		Shader::getCurrentShader()->setUniformmat3("pgeNormal", normalMatrix);

		if (material.pDiffuseMap != nullptr)
			Shader::getCurrentShader()->setShaderTexture("pgeDiffuseMap", material.pDiffuseMap->getTextureID(), GL_TEXTURE_2D);
		else
			Shader::getCurrentShader()->setShaderTexture("pgeDiffuseMap", getWhiteTexture()->getTextureID(), GL_TEXTURE_2D);

		Shader::getCurrentShader()->bindShaderTextures();
	}

	PGE_GL_ERROR_CHECK();
}

void RenderScene::updateShaderUniforms() {
	sceneUniformBuffer.bind(GL_UNIFORM_BUFFER);

	gBufferRenderShaders[standard]->bind();
	gBufferRenderShaders[standard]->setUniformmat4("pgeViewModel", renderCamera.projectionMatrix);

	gBufferRenderShaders[normal]->bind();
	gBufferRenderShaders[normal]->setUniformmat4("pgeViewModel", renderCamera.projectionMatrix);

	gBufferRenderShaders[heightNormal]->bind();
	gBufferRenderShaders[heightNormal]->setUniformmat4("pgeViewModel", renderCamera.projectionMatrix);

	PGE_GL_ERROR_CHECK();
}

void RenderScene::setTransform(const Matrix4x4f &transform) {
	viewModelMatrix = renderCamera.getViewMatrix() * transform;

	Matrix3x3f upperLeftSubmatrixInverse;

	viewModelMatrix.getUpperLeftMatrix3x3f().inverse(upperLeftSubmatrixInverse);

	normalMatrix = upperLeftSubmatrixInverse.transpose();

	projectionViewModelMatrix = renderCamera.getProjectionViewMatrix() * transform;

	Shader::getCurrentShader()->setUniformmat4("pgeViewModel", viewModelMatrix);
	Shader::getCurrentShader()->setUniformmat4("pgeProjectionViewModel", projectionViewModelMatrix);
	Shader::getCurrentShader()->setUniformmat3("pgeNormal", normalMatrix);

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