#include "SceneObjectBox.h"

bool SceneObjectBox::create(const std::string &modelFileName, const pge::Vec3f &startPosition, const pge::Quaternion &startRotation, float mass, float restitution, float friction) {
	assert(getScene() != nullptr);

	// Rendering
	std::shared_ptr<pge::Asset> asset;

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset(modelFileName, asset))
		return false;

	pModelOBJ = static_cast<pge::StaticModelOBJ*>(asset.get());

	pModelOBJ->model.genMipMaps();

	// Get reference to physics world
	physicsWorld = getScene()->getNamedCheckQueue("physWrld");

	assert(physicsWorld.isAlive());

	pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get());

	// Physics
	pCollisionShape.reset(new btBoxShape(bt(pModelOBJ->getAABB().getHalfDims())));

	pMotionState.reset(new btDefaultMotionState(btTransform(bt(startRotation), bt(startPosition))));

	btVector3 inertia;
	pCollisionShape->calculateLocalInertia(mass, inertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, pMotionState.get(), pCollisionShape.get(), inertia);

	rigidBodyCI.m_restitution = restitution;
	rigidBodyCI.m_friction = friction;

	pRigidBody.reset(new btRigidBody(rigidBodyCI));

	pge::Matrix4x4f transform;

	pRigidBody->getWorldTransform().getOpenGLMatrix(&transform.elements[0]);

	aabb = pModelOBJ->getAABB().getTransformedAABB(transform);

	updateAABB();

	return true;
}

void SceneObjectBox::onAdd() {
	pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get());

	pPhysicsWorld->pDynamicsWorld->addRigidBody(pRigidBody.get());
}

void SceneObjectBox::update(float dt) {
	pge::Matrix4x4f transform;

	pRigidBody->getWorldTransform().getOpenGLMatrix(&transform.elements[0]);

	aabb = pModelOBJ->getAABB().getTransformedAABB(transform);

	updateAABB();
}

void SceneObjectBox::deferredRender() {
	pge::Matrix4x4f transform;

	pRigidBody->getWorldTransform().getOpenGLMatrix(&transform.elements[0]);

	getRenderScene()->setTransform(transform);

	pModelOBJ->render(getRenderScene());
}

void SceneObjectBox::onDestroy() {
	if (physicsWorld.isAlive() && pRigidBody != nullptr) {
		pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get());

		pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBody.get());
	}
}