import logging
import os

import numpy as np
import gym
import pypge
from SDRRL import SDRRL
import math

if __name__ == '__main__':
    # You can optionally set up the logger. Also fine to set the level
    # to logging.DEBUG or logging.WARN if you want to change the
    # amount of outut.
    logger = logging.getLogger()
    logger.setLevel(logging.INFO)

    env = gym.make('Quadruped3D-v0')

    # You provide the directory to write to (can be an existing
    # directory, but can't contain previous monitor results. You can
    # also dump to a tempdir if you'd like: tempfile.mkdtemp().
    outdir = '/tmp/random-agent-results'
    env.monitor.start(outdir, force=True)#video_callable=lambda i : False

    episode_count = 400
    max_steps = 40 * 10
    reward = 0
    totalReward = 0
    done = False
    
    numClocks = 20
    
    clockPhases = np.random.rand(numClocks, 1) * math.pi * 2.0
    clockFrequencies = np.random.randn(numClocks, 1) * 0.2
    
    agent = SDRRL(34 + numClocks, 64, 27, -0.1, 0.1)

    for i in xrange(episode_count):
        ob = env.reset()

        for j in xrange(max_steps):
            obsMat = np.matrix(ob).T
            
            clockInputs = np.ones((numClocks, 1)) * j
            
            clocks = np.sin(np.multiply(clockInputs, clockFrequencies) + clockPhases)
            
            obsMat = np.concatenate((obsMat, clocks), axis=0)

            action = agent.simStep(reward, obsMat).T[0]
            ob, reward, done, _ = env.step(action)
            totalReward += reward
            
            if done:
                print("Total reward: " + str(totalReward))
                totalReward = 0
                break

    # Dump result info to disk
    env.monitor.close()

    # Upload to the scoreboard. We could also do this from another
    # process if we wanted.
    #logger.info("Successfully ran RandomAgent. Now trying to upload results to the scoreboard. If it breaks, you can always just try re-uploading the same results.")
    #gym.upload(outdir, algorithm_id='random')