from BlockGameInterface import BlockGameEnv
from gym.envs.registration import registry, register, make, spec

# Games
# ----------------------------------------

register(
    id='BlockGame-v0',
    entry_point='pypge.BlockGameInterface:BlockGameEnv',
    timestep_limit=1000
)

register(
    id='CartPole3D-v0',
    entry_point='pypge.CartPole3DInterface:CartPole3DEnv',
    timestep_limit=600
)