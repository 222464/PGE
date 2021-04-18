#include "BVHNode.h"

#include "BVHTree.h"

using namespace pge;

const float BVHNode::traverseCost = 8.0f;
const float BVHNode::intersectCost = 1.0f;

BVHNode::BVHNode(class BVHTree* pTree, BVHNode* pParent)
: pTree(pTree), pParent(pParent), numOccupantsBelow(0), collapseMarker(false), pLeft(nullptr), pRight(nullptr)
{}

void BVHNode::add(const FormTriangle &triangle, const AABB3D &triangleAABB) {
    numOccupantsBelow++;

    // Add here
    occupants.push_back(triangle);
}

float BVHNode::getCostSAH(const Vec3f &splitPos, int axis) {
    Vec3f dims = aabb.getDims();
    float a = 2.0f * (dims.x * dims.y + dims.y * dims.z + dims.x * dims.z);
    float aInv = 1.0f / a;

    switch (axis) {
    case 0:
        {
            AABB3D aabb0(aabb.lowerBound, Vec3f(splitPos.x, aabb.upperBound.y, aabb.upperBound.z));
            AABB3D aabb1(Vec3f(splitPos.x, aabb.lowerBound.y, aabb.lowerBound.z), aabb.upperBound);

            Vec3f dims0 = aabb0.getDims();
            Vec3f dims1 = aabb1.getDims();

            float a0 = 2.0f * (dims0.x * dims0.y + dims0.y * dims0.z + dims0.x * dims0.z);
            float a1 = 2.0f * (dims1.x * dims1.y + dims1.y * dims1.z + dims1.x * dims1.z);

            float n0 = 0.0f;
            float n1 = 0.0f;

            for (size_t i = 0, size = occupants.size(); i < size; i++) {
                AABB3D aabb = occupants[i].getAABB();

                if (aabb0.intersects(aabb))
                    n0++;
                if (aabb1.intersects(aabb))
                    n1++;
            }

            return traverseCost + intersectCost * (a0 * n0 + a1 * n1) * aInv;
        }

    case 1:
        {
            AABB3D aabb0(aabb.lowerBound, Vec3f(aabb.upperBound.x, splitPos.y, aabb.upperBound.z));
            AABB3D aabb1(Vec3f(aabb.lowerBound.x, splitPos.y, aabb.lowerBound.z), aabb.upperBound);

            Vec3f dims0 = aabb0.getDims();
            Vec3f dims1 = aabb1.getDims();

            float a0 = 2.0f * (dims0.x * dims0.y + dims0.y * dims0.z + dims0.x * dims0.z);
            float a1 = 2.0f * (dims1.x * dims1.y + dims1.y * dims1.z + dims1.x * dims1.z);

            float n0 = 0.0f;
            float n1 = 0.0f;

            for (size_t i = 0, size = occupants.size(); i < size; i++) {
                AABB3D aabb = occupants[i].getAABB();

                if (aabb0.intersects(aabb))
                    n0++;
                if (aabb1.intersects(aabb))
                    n1++;
            }

            return traverseCost + intersectCost * (a0 * n0 + a1 * n1) * aInv;
        }

    case 2:
        {
            AABB3D aabb0(aabb.lowerBound, Vec3f(aabb.upperBound.x, aabb.upperBound.y, splitPos.z));
            AABB3D aabb1(Vec3f(aabb.lowerBound.x, aabb.lowerBound.y, splitPos.z), aabb.upperBound);

            Vec3f dims0 = aabb0.getDims();
            Vec3f dims1 = aabb1.getDims();

            float a0 = 2.0f * (dims0.x * dims0.y + dims0.y * dims0.z + dims0.x * dims0.z);
            float a1 = 2.0f * (dims1.x * dims1.y + dims1.y * dims1.z + dims1.x * dims1.z);

            float n0 = 0.0f;
            float n1 = 0.0f;

            for (size_t i = 0, size = occupants.size(); i < size; i++) {
                AABB3D aabb = occupants[i].getAABB();

                if (aabb0.intersects(aabb))
                    n0++;
                if (aabb1.intersects(aabb))
                    n1++;
            }

            return traverseCost + intersectCost * (a0 * n0 + a1 * n1) * aInv;
        }
    }

    return 0.0f;
}

// Returns if should split 
bool BVHNode::findSplitPosSAH(float &pos, int &axis, size_t &splitIndex) {
    // Get sorted arrays in all directions
    // Get min and max for all axes
    std::vector<CoordAndIsMin> sortedX(occupants.size() * 2);
    std::vector<CoordAndIsMin> sortedY(occupants.size() * 2);
    std::vector<CoordAndIsMin> sortedZ(occupants.size() * 2);

    for (size_t i = 0; i < occupants.size(); i++) {
        AABB3D aabb = occupants[i].getAABB();

        sortedX[i * 2].coord = aabb.lowerBound.x;
        sortedX[i * 2].isMin = true;
        sortedX[i * 2 + 1].coord = aabb.upperBound.x;
        sortedX[i * 2 + 1].isMin = false;

        sortedY[i * 2].coord = aabb.lowerBound.y;
        sortedY[i * 2].isMin = true;
        sortedY[i * 2 + 1].coord = aabb.upperBound.y;
        sortedY[i * 2 + 1].isMin = false;

        sortedZ[i * 2].coord = aabb.lowerBound.z;
        sortedZ[i * 2].isMin = true;
        sortedZ[i * 2 + 1].coord = aabb.upperBound.z;
        sortedZ[i * 2 + 1].isMin = false;
    }

    std::sort(sortedX.begin(), sortedX.end());
    std::sort(sortedY.begin(), sortedY.end());
    std::sort(sortedZ.begin(), sortedZ.end());

    float currentCost = intersectCost * occupants.size();

    float bestCost = 999999.0f;

    // X axis
    int tL = 1;
    int tR = occupants.size();

    bool maxEvent = false;

    Vec3f dims = aabb.getDims();

    float aInv = 0.5f / (dims.x * dims.y + dims.y * dims.z + dims.x * dims.z);

    for (size_t i = 0; i < sortedX.size(); i++) {
        if (sortedX[i].coord <= aabb.lowerBound.x || sortedX[i].coord >= aabb.upperBound.x)
            continue;

        AABB3D aabbL(aabb.lowerBound, Vec3f(sortedX[i].coord, aabb.upperBound.y, aabb.upperBound.z));
        AABB3D aabbR(Vec3f(sortedX[i].coord, aabb.lowerBound.y, aabb.lowerBound.z), aabb.upperBound);

        Vec3f dimsL = aabbL.getDims();
        Vec3f dimsR = aabbR.getDims();

        float aL = 2.0f * (dimsL.x * dimsL.y + dimsL.y * dimsL.z + dimsL.x * dimsL.z);
        float aR = 2.0f * (dimsR.x * dimsR.y + dimsR.y * dimsR.z + dimsR.x * dimsR.z);

        float cost = traverseCost + intersectCost * (aL * tL + aR * tR) * aInv;

        if (cost < bestCost) {
            bestCost = cost;
            pos = sortedX[i].coord;
            axis = 0;
            splitIndex = i;
        }

        if (maxEvent) {
            tR--;
            maxEvent = false;
        }

        if (sortedX[i].isMin)
            tL++;
        else
            maxEvent = true;
    }

    tL = 1;
    tR = occupants.size();

    maxEvent = false;

    for (size_t i = 0; i < sortedY.size(); i++) {
        if (sortedY[i].coord <= aabb.lowerBound.y || sortedY[i].coord >= aabb.upperBound.y)
            continue;

        AABB3D aabbL(aabb.lowerBound, Vec3f(aabb.upperBound.x, sortedY[i].coord, aabb.upperBound.z));
        AABB3D aabbR(Vec3f(aabb.lowerBound.x, sortedY[i].coord, aabb.lowerBound.z), aabb.upperBound);

        Vec3f dimsL = aabbL.getDims();
        Vec3f dimsR = aabbR.getDims();

        float aL = 2.0f * (dimsL.x * dimsL.y + dimsL.y * dimsL.z + dimsL.x * dimsL.z);
        float aR = 2.0f * (dimsR.x * dimsR.y + dimsR.y * dimsR.z + dimsR.x * dimsR.z);

        float cost = traverseCost + intersectCost * (aL * tL + aR * tR) * aInv;

        if (cost < bestCost) {
            bestCost = cost;
            pos = sortedY[i].coord;
            axis = 1;
            splitIndex = i;
        }

        if (maxEvent) {
            tR--;
            maxEvent = false;
        }

        if (sortedY[i].isMin)
            tL++;
        else
            maxEvent = true;
    }

    tL = 1;
    tR = occupants.size();

    maxEvent = false;

    for (size_t i = 0; i < sortedZ.size(); i++) {
        if (sortedZ[i].coord <= aabb.lowerBound.z || sortedZ[i].coord >= aabb.upperBound.z)
            continue;

        AABB3D aabbL(aabb.lowerBound, Vec3f(aabb.upperBound.x, aabb.upperBound.y, sortedZ[i].coord));
        AABB3D aabbR(Vec3f(aabb.lowerBound.x, aabb.lowerBound.y, sortedZ[i].coord), aabb.upperBound);

        Vec3f dimsL = aabbL.getDims();
        Vec3f dimsR = aabbR.getDims();

        float aL = 2.0f * (dimsL.x * dimsL.y + dimsL.y * dimsL.z + dimsL.x * dimsL.z);
        float aR = 2.0f * (dimsR.x * dimsR.y + dimsR.y * dimsR.z + dimsR.x * dimsR.z);

        float cost = traverseCost + intersectCost * (aL * tL + aR * tR) * aInv;

        if (cost < bestCost) {
            bestCost = cost;
            pos = sortedZ[i].coord;
            axis = 2;
            splitIndex = i;
        }

        if (maxEvent) {
            tR--;
            maxEvent = false;
        }

        if (sortedZ[i].isMin)
            tL++;
        else
            maxEvent = true;
    }

    if (currentCost <= bestCost)
        return false;

    return true;
}

void BVHNode::split(int numSplitsAfterNoTriangleReduction) {
    float splitPos;
    int axis;
    size_t splitIndex;

    if (!findSplitPosSAH(splitPos, axis, splitIndex))
        return;

    /*if (aabb.GetHalfDims().x > aabb.GetHalfDims().y)
    {
        if (aabb.GetHalfDims().x > aabb.GetHalfDims().z)
        {
            axis = 0;
            splitPos = aabb.GetCenter().x;
        }
        else
        {
            axis = 2;
            splitPos = aabb.GetCenter().z;
        }
    }
    else
    {
        if (aabb.GetHalfDims().y > aabb.GetHalfDims().z)
        {
            axis = 1;
            splitPos = aabb.GetCenter().y;
        }
        else
        {
            axis = 2;
            splitPos = aabb.GetCenter().z;
        }
    }*/

    pLeft.reset(new BVHNode(pTree, this));
    pRight.reset(new BVHNode(pTree, this));

    switch (axis)
    {
    case 0:
        {
            AABB3D aabb0(aabb.lowerBound, Vec3f(splitPos, aabb.upperBound.y, aabb.upperBound.z));
            AABB3D aabb1(Vec3f(splitPos, aabb.lowerBound.y, aabb.lowerBound.z), aabb.upperBound);

            pLeft->aabb = aabb0;
            pRight->aabb = aabb1;

            // Add occupants to the new nodes if they fit
            for (size_t i = 0, size = occupants.size(); i < size; i++) {
                AABB3D aabb = occupants[i].getAABB();

                if (pLeft->aabb.intersects(aabb))
                    pLeft->add(occupants[i], aabb);
                if (pRight->aabb.intersects(aabb))
                    pRight->add(occupants[i], aabb);
            }

            if (numSplitsAfterNoTriangleReduction < pTree->maxSplitsAfterNoTriangleReduction) {
                // Left split
                pLeft->split(pLeft->occupants.size() == occupants.size() ? (numSplitsAfterNoTriangleReduction + 1) : 0);

                // Right split
                pRight->split(pRight->occupants.size() == occupants.size() ? (numSplitsAfterNoTriangleReduction + 1) : 0);
            }

            break;
        }
    case 1:
        {
            AABB3D aabb0(aabb.lowerBound, Vec3f(aabb.upperBound.x, splitPos, aabb.upperBound.z));
            AABB3D aabb1(Vec3f(aabb.lowerBound.x, splitPos, aabb.lowerBound.z), aabb.upperBound);

            pLeft->aabb = aabb0;
            pRight->aabb = aabb1;

            // Add occupants to the new nodes if they fit
            for (size_t i = 0, size = occupants.size(); i < size; i++) {
                AABB3D aabb = occupants[i].getAABB();

                if (pLeft->aabb.intersects(aabb))
                    pLeft->add(occupants[i], aabb);
                if (pRight->aabb.intersects(aabb))
                    pRight->add(occupants[i], aabb);
            }

            if (numSplitsAfterNoTriangleReduction < pTree->maxSplitsAfterNoTriangleReduction) {
                // Left split
                pLeft->split(pLeft->occupants.size() == occupants.size() ? (numSplitsAfterNoTriangleReduction + 1) : 0);

                // Right split
                pRight->split(pRight->occupants.size() == occupants.size() ? (numSplitsAfterNoTriangleReduction + 1) : 0);
            }

            break;
        }
    case 2:
        {
            AABB3D aabb0(aabb.lowerBound, Vec3f(aabb.upperBound.x, aabb.upperBound.y, splitPos));
            AABB3D aabb1(Vec3f(aabb.lowerBound.x, aabb.lowerBound.y, splitPos), aabb.upperBound);

            pLeft->aabb = aabb0;
            pRight->aabb = aabb1;

            // Add occupants to the new nodes if they fit
            for (size_t i = 0, size = occupants.size(); i < size; i++) {
                AABB3D aabb = occupants[i].getAABB();

                if (pLeft->aabb.intersects(aabb))
                    pLeft->add(occupants[i], aabb);
                if (pRight->aabb.intersects(aabb))
                    pRight->add(occupants[i], aabb);
            }

            if (numSplitsAfterNoTriangleReduction < pTree->maxSplitsAfterNoTriangleReduction) {
                // Left split
                pLeft->split(pLeft->occupants.size() == occupants.size() ? (numSplitsAfterNoTriangleReduction + 1) : 0);

                // Right split
                pRight->split(pRight->occupants.size() == occupants.size() ? (numSplitsAfterNoTriangleReduction + 1) : 0);
            }

            break;
        }
    }

    occupants.clear();
}