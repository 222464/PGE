#pragma once

#include "../scene/RenderScene.h"

#include "SceneObjectProp.h"
#include "physics/SceneObjectPhysicsWorld.h"

class SceneObjectBox : public pge::SceneObject {
private:
	// Rendering
	pge::StaticModelOBJ* pModelOBJ;

	// Physics
	pge::SceneObjectRef physicsWorld;

	std::shared_ptr<btCollisionShape> pCollisionShape;
	std::shared_ptr<btDefaultMotionState> pMotionState;
	std::shared_ptr<btRigidBody> pRigidBody;
public:
	SceneObjectBox() {
		renderMask = 0xffff;
	}

	bool create(const std::string &modelFileName, const pge::Vec3f &startPosition, const pge::Quaternion &startRotation, float mass, float restitution, float friction);

	// Inherited from SceneObject
	void onAdd();
	void update(float dt);
	void deferredRender();
	void onDestroy();

	void setPosition(const pge::Vec3f &position) {
		pRigidBody->getWorldTransform().setOrigin(bt(position));
	}

	SceneObjectBox* copyFactory() {
		return new SceneObjectBox(*this);
	}
};

