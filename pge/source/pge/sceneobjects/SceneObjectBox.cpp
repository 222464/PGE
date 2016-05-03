#include <pge/sceneobjects/SceneObjectBox.h>

bool SceneObjectBox::create(const std::string &modelFileName, const pge::Vec3f &startPosition, const pge::Quaternion &startRotation, float mass, float restitution, float friction) {
	assert(getScene() != nullptr);

	// Rendering
	std::shared_ptr<pge::Asset> asset;

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset(modelFileName, asset))
		return false;

	_pModelOBJ = static_cast<pge::StaticModelOBJ*>(asset.get());

	_pModelOBJ->_model.genMipMaps();

	// Get reference to physics world
	_physicsWorld = getScene()->getNamedCheckQueue("physWrld");

	assert(_physicsWorld.isAlive());

	pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(_physicsWorld.get());

	// Physics
	_pCollisionShape.reset(new btBoxShape(bt(_pModelOBJ->getAABB().getHalfDims())));

	_pMotionState.reset(new btDefaultMotionState(btTransform(bt(startRotation), bt(startPosition))));

	btVector3 inertia;
	_pCollisionShape->calculateLocalInertia(mass, inertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, _pMotionState.get(), _pCollisionShape.get(), inertia);

	rigidBodyCI.m_restitution = restitution;
	rigidBodyCI.m_friction = friction;

	_pRigidBody.reset(new btRigidBody(rigidBodyCI));

	pge::Matrix4x4f transform;

	_pRigidBody->getWorldTransform().getOpenGLMatrix(&transform._elements[0]);

	_aabb = _pModelOBJ->getAABB().getTransformedAABB(transform);

	updateAABB();

	return true;
}

void SceneObjectBox::onAdd() {
	pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(_physicsWorld.get());

	pPhysicsWorld->_pDynamicsWorld->addRigidBody(_pRigidBody.get());
}

void SceneObjectBox::update(float dt) {
	pge::Matrix4x4f transform;

	_pRigidBody->getWorldTransform().getOpenGLMatrix(&transform._elements[0]);

	_aabb = _pModelOBJ->getAABB().getTransformedAABB(transform);

	updateAABB();
}

void SceneObjectBox::deferredRender() {
	pge::Matrix4x4f transform;

	_pRigidBody->getWorldTransform().getOpenGLMatrix(&transform._elements[0]);

	getRenderScene()->setTransform(transform);

	_pModelOBJ->render(getRenderScene());
}

void SceneObjectBox::onDestroy() {
	if (_physicsWorld.isAlive() && _pRigidBody != nullptr) {
		pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(_physicsWorld.get());

		pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBody.get());
	}
}