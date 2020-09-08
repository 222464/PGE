#pragma once

#include "../../scene/SceneObject.h"

#include "SceneObjectLighting.h"

#include "../../constructs/Vec3f.h"

namespace pge {
	class SceneObjectPointLight : public SceneObject {
	private:
		SceneObjectRef lighting;

		Vec3f position;
		Vec3f color;

		float range;

		std::shared_ptr<VBO> uniformBuffer;

		bool needsUniformBufferUpdate;

	public:
		bool enabled;

		SceneObjectPointLight();

		void create(SceneObjectLighting* pLighting);

		// Inherited from SceneObject
		void deferredRender();

		void setPosition(const Vec3f &position);
		void setColor(const Vec3f &color);
		void setRange(float range);

		const Vec3f &getPosition() const {
			return position;
		}
		
		const Vec3f &getColor() const {
			return color;
		}

		float getRange() const {
			return range;
		}

		void updateUniformBuffer();

		void setTransform(RenderScene* pRenderScene) {
			pRenderScene->setTransform(Matrix4x4f::translateMatrix(position) * Matrix4x4f::scaleMatrix(Vec3f(range, range, range)));
		}

		SceneObject* copyFactory() {
			return new SceneObjectPointLight(*this);
		}

		friend SceneObjectLighting;
	};
}