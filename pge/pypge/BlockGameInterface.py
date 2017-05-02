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

class BlockGameEnv(gym.Env):
	metadata = {
		'render.modes': ['human', 'rgb_array'],
		'video.frames_per_second' : 50
	}
	
	def __init__(self):
		
		self.width = 640
		self.height = 480
		self.show = 'show'

		self.p = subprocess.Popen([pgeExePath, 'BlockGame-v0', self.show, str(self.width), str(self.height)])
		
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

		low = np.array([0,0,0,0,0,0,0,0,0])
		high = np.array([np.inf, np.inf, np.inf, np.inf, np.inf, np.inf, np.inf, np.inf, np.inf])

		self.action_space = spaces.Discrete(8)
		self.observation_space = spaces.Box(low, high)

		self.state = np.zeros((9))

		self.r = False
		self.close = False
		self.capturePrev = False
		self.capture = False
		
	def _step(self, action):
		assert action>=0 and action<8, "%r (%s) invalid"%(action, type(action))

		out = bytes()
		
		willCapture = self.capture

		if self.r: 
			out = b'R'
			out += struct.pack('i', action)
			self.r = False
			willCapture = False
		elif self.capture:# and not self.capturePrev:
			out = b'C'
			out += struct.pack('i', action)
		elif not self.capture:# and self.capturePrev:
			out = b'S'
			out += struct.pack('i', action)
		elif self.close:
			willCapture = False
			out = b'X'
			out += struct.pack('i', action)
		else:
			out = b'A'
			out += struct.pack('i', action)
			
		# Write action
		self.connection.send(out)

		# Receive new state
		
		# Read
		sizeR = 4 + 9 * 4 + 4

		data = self.connection.recv(sizeR)

		# Read reward
		reward = struct.unpack('f', data[0:4])[0]
			
		for i in range(0, 9):
			self.state[i] = struct.unpack('i', data[4 + i * 4:4 + i * 4 + 4])[0]
			
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
			
		done = False

		done = bool(done)

		self.capturePrev = self.capture

		return self.state, reward, done, {}

	def _reset(self):
		self.r = True
		self.capture = False
		self.state = np.zeros((9))
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
