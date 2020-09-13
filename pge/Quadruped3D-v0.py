import logging
import os

import numpy as np
import gym
import gym.wrappers
import pypge

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
    env = gym.wrappers.Monitor(env, outdir, force=True)

    episode_count = 400
    max_steps = 60 * 10
    reward = 0
    totalReward = 0
    done = False
    
    for i in range(episode_count):
        ob = env.reset()

        for j in range(max_steps):
            action = env.action_space.sample()

            ob, reward, done, _ = env.step(action)

            totalReward += reward

            reward = 0.0

            if done:
                print("Total reward: " + str(totalReward))
                totalReward = 0
                reward = -1.0
                break

    # Dump result info to disk
    env.monitor.close()

    # Upload to the scoreboard. We could also do this from another
    # process if we wanted.
    #logger.info("Successfully ran RandomAgent. Now trying to upload results to the scoreboard. If it breaks, you can always just try re-uploading the same results.")
    #gym.upload(outdir, algorithm_id='random')