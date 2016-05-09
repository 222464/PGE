from BlockGameInterface import BlockGameEnv
from CartPole3DInterface import CartPole3DEnv
from Quadruped3DInterface import Quadruped3DEnv
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

register(
    id='Quadruped3D-v0',
    entry_point='pypge.Quadruped3DInterface:Quadruped3DEnv',
    timestep_limit=600
)