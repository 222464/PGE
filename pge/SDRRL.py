import numpy as np
import heapq
from operator import itemgetter
from copy import copy

class SDRRL(object):
    def __init__(self, numState, numHidden, numAction, initMinWeight, initMaxWeight):
        self._numState = numState
        self._numHidden = numHidden
        self._numAction = numAction

        self._actionDelta = np.zeros((numAction, 1))
        self._hiddenStates = np.zeros((numHidden, 1))

        self._weightsFF = np.random.randn(numHidden, numState) * (initMaxWeight - initMinWeight) + initMinWeight
        self._weightsQ = np.random.randn(1, numHidden) * (initMaxWeight - initMinWeight) + initMinWeight
        self._tracesQ = np.zeros((1, numHidden))
        self._weightsAction = np.random.randn(numAction, numHidden) * (initMaxWeight - initMinWeight) + initMinWeight

        self._prevV = 0.0

        self._alphaFF = 0.01
        self._alphaAction = 0.1
        self._alphaQ = 0.05
        self._gamma = 0.95
        self._lambda = 0.9
        self._activeRatio = 0.04
        self._noise = 0.08

    def simStep(self, reward, state):
        numActive = int(self._activeRatio * self._numHidden)

        activations = np.zeros((self._numHidden, 1))

        for i in range(0, self._numHidden):
            activations[i] = -np.sum(np.square(state.T - self._weightsFF[i]))

        #activations = np.dot(self._weightsFF, state)

        # Generate tuples for sorting
        heap = [(activations.item(0), 0)]
        
        for i in range(1, self._numHidden):
            heapq.heappush(heap, (activations.item(i), i))

        # Use sorted information for inhibition
        hiddenStatesPrev = copy(self._hiddenStates)

        self._hiddenStates = np.zeros((self._numHidden, 1))
        
        nLargest = heapq.nlargest(numActive, heap, key=itemgetter(0))
        
        # Inhibition
        for i in range(0, numActive):
            self._hiddenStates[nLargest[i][1]] = 1.0

        # Q
        q = np.dot(self._weightsQ, self._hiddenStates).item(0)

        # Action
        action = np.tanh(np.dot(self._weightsAction, self._hiddenStates))

        actionExp = np.minimum(1.0, np.maximum(-1.0, action + np.random.randn(self._numAction, 1) * self._noise))

        # Reconstruction
        recon = np.dot(self._weightsFF.T, self._hiddenStates)

        delta = state - recon

        # Update
        self._weightsFF += self._alphaFF * np.dot(self._hiddenStates, delta.T)

        tdError = reward + self._gamma * q - self._prevV

        self._tracesQ = np.maximum(self._tracesQ * self._lambda, hiddenStatesPrev.T)

        self._weightsQ += self._alphaQ * tdError * self._tracesQ

        if tdError > 0.0:
            self._weightsAction += self._alphaAction * np.dot(self._actionDelta, hiddenStatesPrev.T)

        self._prevV = q
        
        self._actionDelta = actionExp - action
        
        return actionExp