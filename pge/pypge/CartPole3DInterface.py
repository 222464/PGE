import math
import subprocess
import gym
from gym import spaces
import numpy as np
import socket
import struct

pgeExePath = './pge'
IP = 'localhost'
PORT = 54003
MAX_CHUNK = 16384 # Must match pge value
TIMEOUT = 5.0

class CartPole3DEnv(gym.Env):
	metadata = {
		'render.modes': ['human', 'rgb_array'],
		'video.frames_per_second' : 50
	}
	
	def __init__(self):
		
		self.width = 640
		self.height = 480
		self.show = 'show'

		self.p = subprocess.Popen([pgeExePath, 'CartPole3D-v0', self.show, str(self.width), str(self.height)])
		
		self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.s.bind((IP, PORT))
		self.s.listen(1)
		
		conn, addr = self.s.accept()
		conn.settimeout(TIMEOUT)
		
		print("Connection from " + addr[0])
		
		self.connection = conn

		self.reset()
		self.viewer = None

		self.img = np.zeros((self.height, self.width, 3), dtype=np.uint8)

		low = np.array([-np.inf, -np.inf, -np.inf, -np.inf, -np.inf, -np.inf, -np.inf, -np.inf])
		high = np.array([np.inf, np.inf, np.inf, np.inf, np.inf, np.inf, np.inf, np.inf])

		self.action_space = spaces.Box(np.array([-1.0, -1.0]), np.array([1.0, 1.0]))
		self.observation_space = spaces.Box(low, high)

		self.state = np.zeros((8))

		self.r = False
		self.close = False
		self.capturePrev = False
		self.capture = False
		
	def _step(self, action):
		assert action[0]>=-1 and action[0]<=1 and action[1]>=-1 and action[1]<=1, "%r (%s) invalid"%(action, type(action))

		actionf = action.astype(np.float32)
		
		out = bytes()
		
		willCapture = self.capture

		if self.r: 
			out = b'R'
			out += struct.pack('ff', actionf[0], actionf[1]) # Dummy data
			self.r = False
			willCapture = False
		elif self.capture:# and not self.capturePrev:
			out = b'C'
			out += struct.pack('ff', actionf[0], actionf[1])
		elif not self.capture:# and self.capturePrev:
			out = b'S'
			out += struct.pack('ff', actionf[0], actionf[1])
		elif self.close:
			willCapture = False
			out = b'X'
			out += struct.pack('ff', actionf[0], actionf[1]) # Dummy data
		else:
			out = b'A'
			out += struct.pack('ff', actionf[0], actionf[1])
			
		# Write action
		self.connection.send(out)

		# Receive new state
		
		# Read
		sizeR = 4 + 8 * 4 + 4 + 4

		data = self.connection.recv(sizeR)

		# Read reward
		reward = struct.unpack('f', data[0:4])[0]
			
		for i in range(0, 8):
			self.state[i] = struct.unpack('f', data[4 + i * 4:4 + i * 4 + 4])[0]
		
		done = struct.unpack('i', data[sizeR-8:sizeR-4])[0] != 0
		
		# If capturing, expect additional data	
		numChunks = struct.unpack('i', data[sizeR-4:sizeR])[0]
		
		# Check if capture is included
		if numChunks != 0:	
			imgData = bytearray()
			numSoFar = 0
			
			while numSoFar < self.width*self.height*3:
				numLeft = self.width*self.height*3 - numSoFar
				
				numRead = min(numLeft, MAX_CHUNK)
				
				d = self.connection.recv(numRead)
				
				imgData += d
				numSoFar += len(d)
				
			assert(numSoFar == self.width*self.height*3)

			self.img = np.array(imgData).reshape((self.height, self.width, 3))
			
		done = bool(done)

		self.capturePrev = self.capture

		return self.state, reward, done, {}

	def _reset(self):
		self.r = True
		self.state = np.zeros((8))
		self.capture = False
		return self.state

	def _render(self, mode='human', close=False):
		self.capture = True
		if close:
			if self.viewer is not None:
				self.viewer.close()
			return

		if mode == 'rgb_array':
			return self.img
		elif mode is 'human':
			pass
			
	def __del__(self):
		self.connection.send(b'X')
		self.connection.close()
