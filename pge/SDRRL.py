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
        self._stimuli = np.zeros((numHidden, 1))
        self._sparsities = np.zeros((numHidden, 1))
        self._biases = np.zeros((numHidden, 1))

        self._weightsFF = np.random.randn(numHidden, numState) * (initMaxWeight - initMinWeight) + initMinWeight
        self._weightsQ = np.random.randn(1, numHidden) * (initMaxWeight - initMinWeight) + initMinWeight
        self._tracesQ = np.zeros((1, numHidden))
        self._weightsAction = np.random.randn(numAction, numHidden) * (initMaxWeight - initMinWeight) + initMinWeight

        self._prevV = 0.0

        self._alphaFF = 0.0
        self._alphaAction = 0.1
        self._alphaQ = 0.001
        self._alphaBias = 0.0
        self._gamma = 0.97
        self._lambda = 0.92
        self._activeRatio = 0.1
        self._noise = 0.05

    def simStep(self, reward, state):
        numActive = int(self._activeRatio * self._numHidden)

        self._stimuli = np.dot(self._weightsFF, state)
        activations = self._stimuli - self._biases

        # Generate tuples for sorting
        heap = [(activations.item(0), 0)]
        
        for i in range(1, self._numHidden):
            heapq.heappush(heap, (activations.item(i), i))

        # Use sorted information for inhibition
        hiddenStatesPrev = copy(self._hiddenStates)

        self._sparsities = np.zeros((self._numHidden, 1))
        
        nLargest = heapq.nlargest(numActive, heap, key=itemgetter(0))
        
        # Inhibition
        for i in range(0, numActive):
            self._sparsities[nLargest[i][1]] = 1.0
            
        self._hiddenStates = np.multiply(self._sparsities, activations)

        # Q
        q = np.dot(self._weightsQ, self._hiddenStates).item(0)

        # Action
        action = np.tanh(np.dot(self._weightsAction, self._hiddenStates))

        actionExp = copy(action)
        
        for i in range(0, self._numAction):
            if np.random.rand() < self._noise:
                actionExp[i] = np.random.rand() * 2.0 - 1.0

        #actionExp = np.minimum(1.0, np.maximum(-1.0, action + np.random.randn(self._numAction, 1) * self._noise))

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

        self._biases += self._alphaBias * (self._stimuli - self._biases)

        self._prevV = q
        
        self._actionDelta = actionExp - action
        
        return actionExp