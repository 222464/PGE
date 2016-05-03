#pragma once

#include <pge/scene/SceneObject.h>

#include <pge/rendering/shader/Shader.h>
#include <pge/rendering/shader/UBOShaderInterface.h>
#include <pge/rendering/model/StaticPositionModel.h>

namespace pge {
	class SceneObjectLighting : public SceneObject {
	private:
		std::shared_ptr<Shader> _ambientLightShader;

		std::shared_ptr<Shader> _pointLightShader;
		std::shared_ptr<UBOShaderInterface> _pointLightLightUBOShaderInterface;
	
		std::shared_ptr<Shader> _pointLightShadowedShader;
		std::shared_ptr<UBOShaderInterface> _pointLightShadowedLightUBOShaderInterface;
		
		std::shared_ptr<Shader> _spotLightShader;
		std::shared_ptr<UBOShaderInterface> _spotLightLightUBOShaderInterface;
	
		std::shared_ptr<Shader> _directionalLightShader;
		std::shared_ptr<UBOShaderInterface> _directionalLightLightUBOShaderInterface;
		std::shared_ptr<Shader> _directionalLightShadowedShader;
		std::shared_ptr<UBOShaderInterface> _directionalLightShadowedLightUBOShaderInterface;

		std::shared_ptr<Shader> _emissiveRenderShader;

		std::shared_ptr<Shader> _depthRenderShader;
		
		std::shared_ptr<StaticPositionModel> _sphereModel;
		std::shared_ptr<StaticPositionModel> _coneModel;

		std::shared_ptr<Texture2D> _noiseMap;

		std::list<SceneObjectRef> _pointLights;
		std::list<SceneObjectRef> _spotLights;
		std::list<SceneObjectRef> _directionalLights;
		std::list<SceneObjectRef> _shadowedPointLights;
		//std::list<SceneObjectRef> _shadowedSpotLights;
		std::list<SceneObjectRef> _shadowedDirectionalLights;

		Vec3f _attenuation;

		bool _needsAttenuationUpdate;

	public:
		float _pointLightRangeScalar;
		float _spotLightRangeScalar;

		Vec3f _ambientLight;

		SceneObjectLighting()
			: _attenuation(0.5f, 0.001f, 0.0001f), _needsAttenuationUpdate(true),
			_pointLightRangeScalar(1.3f),
			_spotLightRangeScalar(1.1f),
			_ambientLight(0.0f, 0.0f, 0.0f)
		{
			_renderMask = 0xffff;
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
			return _attenuation;
		}

		// Inherited from SceneObject
		void postRender();

		std::list<SceneObjectRef>::const_iterator getPointLightBegin() const {
			return _pointLights.begin();
		}

		std::list<SceneObjectRef>::const_iterator getPointLightEnd() const {
			return _pointLights.end();
		}

		std::list<SceneObjectRef>::const_iterator getSpotLightBegin() const {
			return _spotLights.begin();
		}

		std::list<SceneObjectRef>::const_iterator getSpotLightEnd() const {
			return _spotLights.end();
		}

		std::list<SceneObjectRef>::const_iterator getDirectionalLightBegin() const {
			return _directionalLights.begin();
		}

		std::list<SceneObjectRef>::const_iterator getDirectionalLightEnd() const {
			return _directionalLights.end();
		}

		std::list<SceneObjectRef>::const_iterator getShadowedPointLightBegin() const {
			return _shadowedPointLights.begin();
		}

		std::list<SceneObjectRef>::const_iterator getShadowedPointLightEnd() const {
			return _shadowedPointLights.end();
		}

		std::list<SceneObjectRef>::const_iterator getShadowedDirectionalLightBegin() const {
			return _shadowedDirectionalLights.begin();
		}

		std::list<SceneObjectRef>::const_iterator getShadowedDirectionalLightEnd() const {
			return _shadowedDirectionalLights.end();
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