#pragma once

#include <pge/scene/RenderScene.h>

#include <pge/sceneobjects/SceneObjectProp.h>
#include <pge/sceneobjects/physics/SceneObjectPhysicsWorld.h>

class SceneObjectBox : public pge::SceneObject {
private:
	// Rendering
	pge::StaticModelOBJ* _pModelOBJ;

	// Physics
	pge::SceneObjectRef _physicsWorld;

	std::shared_ptr<btCollisionShape> _pCollisionShape;
	std::shared_ptr<btDefaultMotionState> _pMotionState;
	std::shared_ptr<btRigidBody> _pRigidBody;
public:
	SceneObjectBox() {
		_renderMask = 0xffff;
	}

	bool create(const std::string &modelFileName, const pge::Vec3f &startPosition, const pge::Quaternion &startRotation, float mass, float restitution, float friction);

	// Inherited from SceneObject
	void onAdd();
	void update(float dt);
	void deferredRender();
	void onDestroy();

	void setPosition(const pge::Vec3f &position) {
		_pRigidBody->getWorldTransform().setOrigin(bt(position));
	}

	SceneObjectBox* copyFactory() {
		return new SceneObjectBox(*this);
	}
};

