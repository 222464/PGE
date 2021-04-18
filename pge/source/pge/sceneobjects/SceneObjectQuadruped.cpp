#include "SceneObjectQuadruped.h"

#include "../rendering/model/SceneObjectStaticModelBatcher.h"

#include "SceneObjectOrbitCamera.h"

#include "../util/Math.h"

#include <iostream>
#include <sstream>

const int constraintSolverIterations = 80;
const float maxLimbBend = pge::pi * 0.125f;
const float maxSpeed = 30.0f;
const float maxForce = 5000.0f;
const float interpFactor = 400.0f;

void SceneObjectQuadruped::Leg::create(pge::SceneObjectPhysicsWorld* pPhysicsWorld, btRigidBody* pBodyPart, const btVector3 &rootPos) {
    // Remove old
    if (lower.pConstraint != nullptr)
        pPhysicsWorld->pDynamicsWorld->removeConstraint(lower.pConstraint.get());

    if (lower.pRigidBody != nullptr)
        pPhysicsWorld->pDynamicsWorld->removeRigidBody(lower.pRigidBody.get());

    if (upper.pConstraint != nullptr)
        pPhysicsWorld->pDynamicsWorld->removeConstraint(upper.pConstraint.get());

    if (upper.pRigidBody != nullptr)
        pPhysicsWorld->pDynamicsWorld->removeRigidBody(upper.pRigidBody.get());

    if (pGhostLower != nullptr)
        pPhysicsWorld->pDynamicsWorld->removeCollisionObject(pGhostLower.get());

    // Physics
    const float height = 0.5f;
    const float radius = 0.12f;

    lower.pCollisionShape.reset(new btCapsuleShape(radius, height));
    upper.pCollisionShape.reset(new btCapsuleShape(radius, height));

    lower.pMotionState.reset(new btDefaultMotionState(btTransform(btQuaternion(btVector3(0.0f, 0.0f, 1.0f), pge::piOver2), rootPos + btVector3(-0.5f * 0.25f, 0.5f * 0.25f, 0.0f))));
    upper.pMotionState.reset(new btDefaultMotionState(btTransform(btQuaternion(btVector3(0.0f, 0.0f, 1.0f), -pge::piOver2), rootPos + btVector3(0.5f * 0.25f, 0.5f * 0.25f + 0.75f, 0.0f))));

    const float lowerMass = 6.0f;
    const float upperMass = 6.0f;

    btVector3 lowerInertia, upperInertia;

    lower.pCollisionShape->calculateLocalInertia(lowerMass, lowerInertia);
    upper.pCollisionShape->calculateLocalInertia(upperMass, upperInertia);

    btRigidBody::btRigidBodyConstructionInfo rigidBodyCILower(lowerMass, lower.pMotionState.get(), lower.pCollisionShape.get(), lowerInertia);
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCIUpper(upperMass, upper.pMotionState.get(), upper.pCollisionShape.get(), upperInertia);

    rigidBodyCILower.m_restitution = 0.05f;
    rigidBodyCILower.m_friction = 10.0f;

    rigidBodyCIUpper.m_restitution = 0.05f;
    rigidBodyCIUpper.m_friction = 0.5f;

    lower.pRigidBody.reset(new btRigidBody(rigidBodyCILower));
    upper.pRigidBody.reset(new btRigidBody(rigidBodyCIUpper));

    lower.pRigidBody->setActivationState(DISABLE_DEACTIVATION);
    upper.pRigidBody->setActivationState(DISABLE_DEACTIVATION);

    pPhysicsWorld->pDynamicsWorld->addRigidBody(lower.pRigidBody.get(), 1 << 2, 0x0001);
    pPhysicsWorld->pDynamicsWorld->addRigidBody(upper.pRigidBody.get(), 1 << 3, 0x0001);

    btTransform frameLowerUpper = btTransform::getIdentity();
    btTransform frameUpperLower = btTransform::getIdentity();
    btTransform frameUpperBody = btTransform::getIdentity();
    btTransform frameBodyUpper = btTransform::getIdentity();

    frameLowerUpper.setOrigin(btVector3(0.0f, 0.25f, 0.0f));
    frameLowerUpper.setRotation(btQuaternion(btVector3(0.0f, 0.0f, 1.0f), -pge::piOver2));

    frameUpperLower.setOrigin(btVector3(0.0f, -0.25f, 0.0f));

    frameUpperBody.setOrigin(btVector3(0.0f, 0.25f, 0.0f));
    frameUpperBody.setRotation(btQuaternion(btVector3(0.0f, 0.0f, 1.0f), pge::piOver4));

    frameBodyUpper.setOrigin(rootPos);
    
    lower.pConstraint.reset(new btGeneric6DofConstraint(*lower.pRigidBody, *upper.pRigidBody, frameLowerUpper, frameUpperLower, false));
    upper.pConstraint.reset(new btGeneric6DofConstraint(*upper.pRigidBody, *pBodyPart, frameUpperBody, frameBodyUpper, false));

    pPhysicsWorld->pDynamicsWorld->addConstraint(lower.pConstraint.get(), true);
    pPhysicsWorld->pDynamicsWorld->addConstraint(upper.pConstraint.get(), true);

    lower.pConstraint->setLimit(0, 0.0f, 0.0f);
    lower.pConstraint->setLimit(1, 0.0f, 0.0f);
    lower.pConstraint->setLimit(2, 0.0f, 0.0f);

    upper.pConstraint->setLimit(0, 0.0f, 0.0f);
    upper.pConstraint->setLimit(1, 0.0f, 0.0f);
    upper.pConstraint->setLimit(2, 0.0f, 0.0f);

    lower.pConstraint->getRotationalLimitMotor(0)->m_enableMotor = true;
    lower.pConstraint->getRotationalLimitMotor(0)->m_maxMotorForce = maxForce;
    lower.pConstraint->getRotationalLimitMotor(1)->m_enableMotor = true;
    lower.pConstraint->getRotationalLimitMotor(1)->m_maxMotorForce = maxForce;
    lower.pConstraint->getRotationalLimitMotor(2)->m_enableMotor = true;
    lower.pConstraint->getRotationalLimitMotor(2)->m_maxMotorForce = maxForce;

    upper.pConstraint->getRotationalLimitMotor(0)->m_enableMotor = true;
    upper.pConstraint->getRotationalLimitMotor(0)->m_maxMotorForce = maxForce;
    upper.pConstraint->getRotationalLimitMotor(1)->m_enableMotor = true;
    upper.pConstraint->getRotationalLimitMotor(1)->m_maxMotorForce = maxForce;
    upper.pConstraint->getRotationalLimitMotor(2)->m_enableMotor = true;
    upper.pConstraint->getRotationalLimitMotor(2)->m_maxMotorForce = maxForce;

    lower.pConstraint->setOverrideNumSolverIterations(constraintSolverIterations);
    upper.pConstraint->setOverrideNumSolverIterations(constraintSolverIterations);

    pGhostLower.reset(new btGhostObject());

    pGhostLower->setCollisionShape(lower.pCollisionShape.get());
    pGhostLower->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);

    pPhysicsWorld->pDynamicsWorld->addCollisionObject(pGhostLower.get());
}

bool SceneObjectQuadruped::create() {
    assert(getScene() != nullptr);

    // Rendering
    std::shared_ptr<pge::Asset> asset;

    if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/limb1.obj", asset))
        return false;

    pLimbModel = static_cast<pge::StaticModelOBJ*>(asset.get());

    if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/body.obj", asset))
        return false;

    pBodyPartModel = static_cast<pge::StaticModelOBJ*>(asset.get());

    // Get reference to physics world
    physicsWorld = getScene()->getNamedCheckQueue("physWrld");

    orbCam = getScene()->getNamedCheckQueue("orbCam");

    reset();

    doneLastFrame = false;

    action.fill(0.0f);

    return true;
}

void SceneObjectQuadruped::onAdd() {
    batcherRef = getScene()->getNamed("smb");

    assert(batcherRef.isAlive());
}

void SceneObjectQuadruped::reset() {
    // Slightly random angle
    std::uniform_real_distribution<float> pertDist(-0.05f, 0.05f);

    assert(physicsWorld.isAlive());

    pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get());

    // Remove old
    if (pConstraintForwardBackward != nullptr)
        pPhysicsWorld->pDynamicsWorld->removeConstraint(pConstraintForwardBackward.get());

    if (pRigidBodyFloor != nullptr)
        pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyFloor.get());

    if (pRigidBodyBackward != nullptr)
        pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyBackward.get());

    if (pRigidBodyForward != nullptr)
        pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyForward.get());

    // Physics
    const btVector3 bodyPartSize(0.5f, 0.25f, 0.5f);

    pCollisionShapeFloor.reset(new btBoxShape(btVector3(1000.0f, 0.5f, 1000.0f)));
    pCollisionShapeBodyForward.reset(new btBoxShape(bodyPartSize));
    pCollisionShapeBodyBackward.reset(new btBoxShape(bodyPartSize));
    
    pMotionStateFloor.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, -0.5f, 0.0f))));
    pMotionStateBodyForward.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, 1.0f, 0.0f))));
    pMotionStateBodyBackward.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, 1.0f, 0.0f))));
    
    const float floorMass = 0.0f;
    const float forwardMass = 20.0f;
    const float backwardMass = 20.0f;

    btVector3 floorInertia, forwardInertia, backwardInertia;

    pCollisionShapeBodyForward->calculateLocalInertia(floorMass, floorInertia);
    pCollisionShapeBodyForward->calculateLocalInertia(forwardMass, forwardInertia);
    pCollisionShapeBodyBackward->calculateLocalInertia(backwardMass, backwardInertia);

    btRigidBody::btRigidBodyConstructionInfo rigidBodyCIFloor(floorMass, pMotionStateFloor.get(), pCollisionShapeFloor.get(), floorInertia);
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCIForward(forwardMass, pMotionStateBodyForward.get(), pCollisionShapeBodyForward.get(), forwardInertia);
    btRigidBody::btRigidBodyConstructionInfo rigidBodyCIBackward(backwardMass, pMotionStateBodyBackward.get(), pCollisionShapeBodyBackward.get(), backwardInertia);

    rigidBodyCIFloor.m_restitution = 0.05f;
    rigidBodyCIFloor.m_friction = 0.5f;

    rigidBodyCIForward.m_restitution = 0.05f;
    rigidBodyCIForward.m_friction = 0.5f;

    rigidBodyCIBackward.m_restitution = 0.05f;
    rigidBodyCIBackward.m_friction = 0.5f;

    pRigidBodyFloor.reset(new btRigidBody(rigidBodyCIFloor));
    pRigidBodyForward.reset(new btRigidBody(rigidBodyCIForward));
    pRigidBodyBackward.reset(new btRigidBody(rigidBodyCIBackward));

    pRigidBodyForward->setActivationState(DISABLE_DEACTIVATION);
    pRigidBodyBackward->setActivationState(DISABLE_DEACTIVATION);

    pPhysicsWorld->pDynamicsWorld->addRigidBody(pRigidBodyFloor.get(), 0xffff, 0xffff);
    pPhysicsWorld->pDynamicsWorld->addRigidBody(pRigidBodyForward.get(), 1 << 4, 0x0001);
    pPhysicsWorld->pDynamicsWorld->addRigidBody(pRigidBodyBackward.get(), 1 << 5, 0x0001);

    btTransform frameA = btTransform::getIdentity();
    btTransform frameB = btTransform::getIdentity();

    frameA.setOrigin(btVector3(-0.5f, 0.0f, 0.0f));
    frameB.setOrigin(btVector3(0.5f, 0.0f, 0.0f));
    pConstraintForwardBackward.reset(new btGeneric6DofConstraint(*pRigidBodyForward, *pRigidBodyBackward, frameA, frameB, false));

    pPhysicsWorld->pDynamicsWorld->addConstraint(pConstraintForwardBackward.get(), true);

    pConstraintForwardBackward->setLimit(0, 0.0f, 0.0f);
    pConstraintForwardBackward->setLimit(1, 0.0f, 0.0f);
    pConstraintForwardBackward->setLimit(2, 0.0f, 0.0f);

    pConstraintForwardBackward->getRotationalLimitMotor(0)->m_enableMotor = true;
    pConstraintForwardBackward->getRotationalLimitMotor(0)->m_maxMotorForce = maxForce;
    pConstraintForwardBackward->getRotationalLimitMotor(1)->m_enableMotor = true;
    pConstraintForwardBackward->getRotationalLimitMotor(1)->m_maxMotorForce = maxForce;
    pConstraintForwardBackward->getRotationalLimitMotor(2)->m_enableMotor = true;
    pConstraintForwardBackward->getRotationalLimitMotor(2)->m_maxMotorForce = maxForce;

    pConstraintForwardBackward->setOverrideNumSolverIterations(constraintSolverIterations);

    // Create limbs
    legs[0].create(pPhysicsWorld, pRigidBodyBackward.get(), btVector3(0.0f, 0.0f, -0.333f));
    legs[1].create(pPhysicsWorld, pRigidBodyForward.get(), btVector3(0.0f, 0.0f, -0.333f));
    legs[2].create(pPhysicsWorld, pRigidBodyForward.get(), btVector3(0.0f, 0.0f, 0.333f));
    legs[3].create(pPhysicsWorld, pRigidBodyBackward.get(), btVector3(0.0f, 0.0f, 0.333f));

    ticksPerAction = 0;
    ticks = 0;
    reward = 0.0f;
    prevDist = 0.0f;
}

void SceneObjectQuadruped::act(float dt) {
    // Set to target angles
    int actIndex = 0;

    for (int i = 0; i < 4; i++) {
        legs[i].lower.pConstraint->getRotationalLimitMotor(0)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (action[actIndex++] * maxLimbBend - legs[i].lower.pConstraint->getAngle(0))));
        legs[i].lower.pConstraint->getRotationalLimitMotor(1)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (action[actIndex++] * maxLimbBend - legs[i].lower.pConstraint->getAngle(1))));
        legs[i].lower.pConstraint->getRotationalLimitMotor(2)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (action[actIndex++] * maxLimbBend - legs[i].lower.pConstraint->getAngle(2))));
    
        legs[i].upper.pConstraint->getRotationalLimitMotor(0)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (action[actIndex++] * maxLimbBend - legs[i].upper.pConstraint->getAngle(0))));
        legs[i].upper.pConstraint->getRotationalLimitMotor(1)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (action[actIndex++] * maxLimbBend - legs[i].upper.pConstraint->getAngle(1))));
        legs[i].upper.pConstraint->getRotationalLimitMotor(2)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (action[actIndex++] * maxLimbBend - legs[i].upper.pConstraint->getAngle(2))));
    }

    pConstraintForwardBackward->getRotationalLimitMotor(0)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (action[actIndex++] * maxLimbBend - pConstraintForwardBackward->getAngle(0))));
    pConstraintForwardBackward->getRotationalLimitMotor(1)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (action[actIndex++] * maxLimbBend - pConstraintForwardBackward->getAngle(1))));
    pConstraintForwardBackward->getRotationalLimitMotor(2)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (action[actIndex++] * maxLimbBend - pConstraintForwardBackward->getAngle(2))));

    float dist = getPosition().magnitude();

    reward = dist - prevDist;

    // If fell over
    if (pRigidBodyForward->getWorldTransform().getRotation().angleShortestPath(btQuaternion::getIdentity()) > pge::piOver2 * 0.9f) {
        doneLastFrame = true;

        reward = -1.0f;

        reset();
    }

    prevDist = dist;
}

void SceneObjectQuadruped::synchronousUpdate(float dt) {
    if (ticks >= ticksPerAction || !getRenderScene()->renderingEnabled) {
        ticks = 0;

        act(dt);

        // Give state and reward (+ capture if is on)

        // Observation (34 values)
        std::vector<float> obs(34);

        int obsIndex = 0;

        for (int i = 0; i < 4; i++) {
            pge::Vec3f eulerLower = cons(legs[i].lower.pConstraint->getRigidBodyB().getWorldTransform().getRotation().inverse() * legs[i].lower.pConstraint->getRigidBodyA().getWorldTransform().getRotation()).getEulerAngles();
            pge::Vec3f eulerUpper = cons(legs[i].upper.pConstraint->getRigidBodyB().getWorldTransform().getRotation().inverse() * legs[i].upper.pConstraint->getRigidBodyA().getWorldTransform().getRotation()).getEulerAngles();

            obs[obsIndex++] = eulerLower.x;
            obs[obsIndex++] = eulerLower.y;
            obs[obsIndex++] = eulerLower.z;

            obs[obsIndex++] = eulerUpper.x;
            obs[obsIndex++] = eulerUpper.y;
            obs[obsIndex++] = eulerUpper.z;
        }

        pge::Vec3f eulerBody = cons(pConstraintForwardBackward->getRigidBodyB().getWorldTransform().getRotation().inverse() * pConstraintForwardBackward->getRigidBodyA().getWorldTransform().getRotation()).getEulerAngles();

        obs[obsIndex++] = eulerBody.x;
        obs[obsIndex++] = eulerBody.y;
        obs[obsIndex++] = eulerBody.z;

        // Gravity sensor
        pge::Vec3f eulerGrav = cons(pRigidBodyForward->getWorldTransform().getRotation()).getEulerAngles();

        obs[obsIndex++] = eulerGrav.x;
        obs[obsIndex++] = eulerGrav.y;
        obs[obsIndex++] = eulerGrav.z;

        // Touch sensors (x4)
        for (int i = 0; i < 4; i++) {
            int num = legs[i].pGhostLower->getNumOverlappingObjects();

            bool hit = num > 0;

            obs[obsIndex++] = hit ? 1.0f : 0.0f;
        }

        // Update ghosts
        for (int i = 0; i < 4; i++)
            legs[i].pGhostLower->setWorldTransform(legs[i].lower.pRigidBody->getWorldTransform());

        doneLastFrame = false;
    }
    else
        ticks++;

    // Update camera
    if (orbCam.isAlive()) {
        SceneObjectOrbitCamera* pCam = static_cast<SceneObjectOrbitCamera*>(orbCam.get());

        pCam->focusPoint = getPosition();
    }
}

void SceneObjectQuadruped::deferredRender() {
    pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(batcherRef.get());
    
    {
        // Render body
        pge::Matrix4x4f transform;

        pRigidBodyForward->getWorldTransform().getOpenGLMatrix(&transform.elements[0]);

        pBodyPartModel->render(pBatcher, transform * pge::Matrix4x4f::rotateMatrixY(pge::pi));
    }
    
    {
        // Render body
        pge::Matrix4x4f transform;

        pRigidBodyBackward->getWorldTransform().getOpenGLMatrix(&transform.elements[0]);

        pBodyPartModel->render(pBatcher, transform);
    }
    
    for (int i = 0; i < 4; i++) {
        {
            // Render limb
            pge::Matrix4x4f transform;

            legs[i].lower.pRigidBody->getWorldTransform().getOpenGLMatrix(&transform.elements[0]);

            pLimbModel->render(pBatcher, transform);
        }

        {
            // Render limb
            pge::Matrix4x4f transform;

            legs[i].upper.pRigidBody->getWorldTransform().getOpenGLMatrix(&transform.elements[0]);

            pLimbModel->render(pBatcher, transform);
        }
    }
}

void SceneObjectQuadruped::postRender() {
    // Get data from effect buffer
    //glReadPixels(0, 0, getRenderScene()->gBuffer.getWidth(), getRenderScene()->gBuffer.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, capBytes->data());
}

void SceneObjectQuadruped::onDestroy() {
    if (physicsWorld.isAlive()) {
        pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get());

        if (pConstraintForwardBackward != nullptr)
            pPhysicsWorld->pDynamicsWorld->removeConstraint(pConstraintForwardBackward.get());

        if (pRigidBodyFloor != nullptr)
            pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyFloor.get());

        if (pRigidBodyBackward != nullptr)
            pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyBackward.get());

        if (pRigidBodyForward != nullptr)
            pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyForward.get());

        for (int i = 0; i < 4; i++) {
            if (legs[i].lower.pConstraint != nullptr)
                pPhysicsWorld->pDynamicsWorld->removeConstraint(legs[i].lower.pConstraint.get());

            if (legs[i].lower.pRigidBody != nullptr)
                pPhysicsWorld->pDynamicsWorld->removeRigidBody(legs[i].lower.pRigidBody.get());

            if (legs[i].upper.pConstraint != nullptr)
                pPhysicsWorld->pDynamicsWorld->removeConstraint(legs[i].upper.pConstraint.get());

            if (legs[i].upper.pRigidBody != nullptr)
                pPhysicsWorld->pDynamicsWorld->removeRigidBody(legs[i].upper.pRigidBody.get());

            if (legs[i].pGhostLower != nullptr)
                pPhysicsWorld->pDynamicsWorld->removeCollisionObject(legs[i].pGhostLower.get());
        }
    }
}
