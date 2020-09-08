#pragma once

#include "../../scene/SceneObject.h"

#include "../shader/Shader.h"
#include "../shader/UBOShaderInterface.h"
#include "../model/StaticPositionModel.h"

namespace pge {
	class SceneObjectLighting : public SceneObject {
	private:
		std::shared_ptr<Shader> ambientLightShader;

		std::shared_ptr<Shader> pointLightShader;
		std::shared_ptr<UBOShaderInterface> pointLightLightUBOShaderInterface;
	
		std::shared_ptr<Shader> pointLightShadowedShader;
		std::shared_ptr<UBOShaderInterface> pointLightShadowedLightUBOShaderInterface;
		
		std::shared_ptr<Shader> spotLightShader;
		std::shared_ptr<UBOShaderInterface> spotLightLightUBOShaderInterface;
	
		std::shared_ptr<Shader> directionalLightShader;
		std::shared_ptr<UBOShaderInterface> directionalLightLightUBOShaderInterface;
		std::shared_ptr<Shader> directionalLightShadowedShader;
		std::shared_ptr<UBOShaderInterface> directionalLightShadowedLightUBOShaderInterface;

		std::shared_ptr<Shader> emissiveRenderShader;

		std::shared_ptr<Shader> depthRenderShader;
		
		std::shared_ptr<StaticPositionModel> sphereModel;
		std::shared_ptr<StaticPositionModel> coneModel;

		std::shared_ptr<Texture2D> noiseMap;

		std::list<SceneObjectRef> pointLights;
		std::list<SceneObjectRef> spotLights;
		std::list<SceneObjectRef> directionalLights;
		std::list<SceneObjectRef> shadowedPointLights;
		//std::list<SceneObjectRef> shadowedSpotLights;
		std::list<SceneObjectRef> shadowedDirectionalLights;

		Vec3f attenuation;

		bool needsAttenuationUpdate;

	public:
		float pointLightRangeScalar;
		float spotLightRangeScalar;

		Vec3f ambientLight;

		SceneObjectLighting()
			: attenuation(0.5f, 0.001f, 0.0001f), needsAttenuationUpdate(true),
			pointLightRangeScalar(1.3f),
			spotLightRangeScalar(1.1f),
			ambientLight(0.0f, 0.0f, 0.0f)
		{
			renderMask = 0xffff;
		}

		void create(const std::shared_ptr<Shader> &ambientLightShader,
			const std::shared_ptr<Shader> &pointLightShader,
			const std::shared_ptr<Shader> &pointLightShadowedShader,
			const std::shared_ptr<Shader> &spotLightShader,
			const std::shared_ptr<Shader> &directionalLightShader,
			const std::shared_ptr<Shader> &directionalLightShadowedShader,
			const std::shared_ptr<Shader> &emissiveRenderShader,
			const std::shared_ptr<Shader> &depthRenderShader,
			const std::shared_ptr<StaticPositionModel> &sphereModel,
			const std::shared_ptr<StaticPositionModel> &coneModel,
			const std::shared_ptr<Texture2D> &noiseMap);

		void add(class SceneObjectPointLight &light);
		void add(class SceneObjectSpotLight &light);
		void add(class SceneObjectDirectionalLight &light);
		void add(class SceneObjectPointLightShadowed &light);
		//void add(class SceneObjectSpotLightShadowed &light);
		void add(class SceneObjectDirectionalLightShadowed &light);

		void setAttenuation(const Vec3f &attenuation);

		const Vec3f &getAttenuation() const {
			return attenuation;
		}

		// Inherited from SceneObject
		void postRender();

		std::list<SceneObjectRef>::const_iterator getPointLightBegin() const {
			return pointLights.begin();
		}

		std::list<SceneObjectRef>::const_iterator getPointLightEnd() const {
			return pointLights.end();
		}

		std::list<SceneObjectRef>::const_iterator getSpotLightBegin() const {
			return spotLights.begin();
		}

		std::list<SceneObjectRef>::const_iterator getSpotLightEnd() const {
			return spotLights.end();
		}

		std::list<SceneObjectRef>::const_iterator getDirectionalLightBegin() const {
			return directionalLights.begin();
		}

		std::list<SceneObjectRef>::const_iterator getDirectionalLightEnd() const {
			return directionalLights.end();
		}

		std::list<SceneObjectRef>::const_iterator getShadowedPointLightBegin() const {
			return shadowedPointLights.begin();
		}

		std::list<SceneObjectRef>::const_iterator getShadowedPointLightEnd() const {
			return shadowedPointLights.end();
		}

		std::list<SceneObjectRef>::const_iterator getShadowedDirectionalLightBegin() const {
			return shadowedDirectionalLights.begin();
		}

		std::list<SceneObjectRef>::const_iterator getShadowedDirectionalLightEnd() const {
			return shadowedDirectionalLights.end();
		}

		SceneObject* copyFactory() {
			return new SceneObjectLighting(*this);
		}

		friend class SceneObjectPointLight;
		friend class SceneObjectSpotLight;
		friend class SceneObjectDirectionalLight;
		friend class SceneObjectPointLightShadowed;
		friend class SceneObjectSpotLightShadowed;
		friend class SceneObjectDirectionalLightShadowed;
	};

	Matrix4x4f getBiasMatrix();
}