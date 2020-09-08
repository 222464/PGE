#pragma once

#include "Scene.h"

#include "../rendering/bufferobjects/TextureRT.h"
#include "../rendering/GBuffer.h"
#include "../rendering/shader/Shader.h"
#include "../rendering/shader/UBOShaderInterface.h"
#include "../rendering/texture/Texture2D.h"
#include "../rendering/mesh/StaticPositionMesh.h"

namespace pge {
	class RenderScene : public Scene {
	public:
		enum GBufferRenderShaderType {
			standard = 0, normal, heightNormal
		};

	private:
		sf::Window* pWindow;

		VBO sceneUniformBuffer;

		std::array<std::shared_ptr<Shader>, 3> gBufferRenderShaders;
		std::array<UBOShaderInterface, 3> gBufferRenderShaderMaterialUBOInterfaces;

		std::shared_ptr<Texture2D> whiteTexture;

		StaticPositionMesh normalizedQuad;

		Matrix4x4f viewModelMatrix;
		Matrix4x4f projectionViewModelMatrix;
		Matrix3x3f normalMatrix;

	public:
		GBuffer gBuffer;

		Vec4f clearColor;

		Camera renderCamera;

		bool shaderSwitchesEnabled;

		bool renderingShadows;

		bool renderingEnabled;

		RenderScene();

		void createRenderScene(size_t numWorkers, const AABB3D &rootRegion, sf::Window* pWindow,
			const std::shared_ptr<Shader> &gBufferRender,
			const std::shared_ptr<Shader> &gBufferRenderNormal,
			const std::shared_ptr<Shader> &gBufferRenderHeightNormal,
			const std::shared_ptr<Texture2D> &whiteTexture);

		// Renders scene to the currently bound buffer
		void renderShadow();

		// Render scene to a render target
		void render(TextureRT &target);
		void renderToMainFramebuffer();

		void frame(float dt);

		void useShader(Shader* pShader);
		void useShader(GBufferRenderShaderType type);
		void useShader(class Material &material);

		void updateShaderUniforms();

		void setTransform(const Matrix4x4f &transform);

		static void createMaterialInterface(UBOShaderInterface &materialInterface, Shader* pShader);

		void renderNormalizedQuad() {
			normalizedQuad.render();
		}

		sf::Window* getRenderWindow() const {
			return pWindow;
		}

		const std::shared_ptr<Texture2D> &getWhiteTexture() const {
			return whiteTexture;
		}

		UBOShaderInterface &getMaterialUBOShaderInterface(GBufferRenderShaderType type) {
			return gBufferRenderShaderMaterialUBOInterfaces[type];
		}

		VBO &getSceneUniformBuffer() {
			return sceneUniformBuffer;
		}

		const Matrix4x4f &getCurrentViewModelMatrix() const {
			return viewModelMatrix;
		}

		const Matrix4x4f &getCurrentProjectionViewModelMatrix() const {
			return projectionViewModelMatrix;
		}

		const Matrix3x3f &getCurrentNormalMatrix() const {
			return normalMatrix;
		}
	};
}