from BlockGameInterface import BlockGameEnv
from gym.envs.registration import registry, register, make, spec

# Games
# ----------------------------------------

register(
    id='BlockGame-v0',
    entry_point='pypge.BlockGameInterface:BlockGameEnv',
    timestep_limit=200,
    reward_threshold=25.0,
)