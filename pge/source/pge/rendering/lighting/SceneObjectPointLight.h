#pragma once

#include <pge/scene/SceneObject.h>

#include <pge/rendering/lighting/SceneObjectLighting.h>

#include <pge/constructs/Vec3f.h>

namespace pge {
	class SceneObjectPointLight : public SceneObject {
	private:
		SceneObjectRef _lighting;

		Vec3f _position;
		Vec3f _color;

		float _range;

		std::shared_ptr<VBO> _uniformBuffer;

		bool _needsUniformBufferUpdate;

	public:
		bool _enabled;

		SceneObjectPointLight();

		void create(SceneObjectLighting* pLighting);

		// Inherited from SceneObject
		void deferredRender();

		void setPosition(const Vec3f &position);
		void setColor(const Vec3f &color);
		void setRange(float range);

		const Vec3f &getPosition() const {
			return _position;
		}
		
		const Vec3f &getColor() const {
			return _color;
		}

		float getRange() const {
			return _range;
		}

		void updateUniformBuffer();

		void setTransform(RenderScene* pRenderScene) {
			pRenderScene->setTransform(Matrix4x4f::translateMatrix(_position) * Matrix4x4f::scaleMatrix(Vec3f(_range, _range, _range)));
		}

		SceneObject* copyFactory() {
			return new SceneObjectPointLight(*this);
		}

		friend SceneObjectLighting;
	};
}