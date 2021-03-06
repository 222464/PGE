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

class Quadruped3DEnv(gym.Env):
	metadata = {
		'render.modes': ['human', 'rgb_array'],
		'video.frames_per_second' : 50
	}
	
	def __init__(self):
		
		self.width = 640
		self.height = 480
		self.show = 'show'
		self.connectionReset = False

		self.p = subprocess.Popen([pgeExePath, 'Quadruped3D-v0', self.show, str(self.width), str(self.height)])
		
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

		low = np.array([-np.inf for i in range(0, 34)])
		high = np.array([np.inf for i in range(0, 34)])
		
		self.action_space = spaces.Box(np.array([-1.0 for i in range(0, 27)]), np.array([1.0 for i in range(0, 27)]))
		self.observation_space = spaces.Box(low, high)

		self.state = np.zeros((34))

		self.r = False
		self.close = False
		self.capturePrev = False
		self.capture = False
		
	def step(self, action):
		for i in range(0, 27):
			assert action[i]>=-1 and action[i]<=1, "%r (%s) invalid"%(action[i], type(action[i]))

		actionf = action.astype(np.float32)
		
		out = bytes()
		
		willCapture = self.capture

		if self.r: 
			out = b'R'
			for i in range(0, 27):
				out += struct.pack('f', actionf[i])
			self.r = False
			willCapture = False
		elif self.capture:# and not self.capturePrev:
			out = b'C'
			for i in range(0, 27):
				out += struct.pack('f', actionf[i])
		elif not self.capture:# and self.capturePrev:
			out = b'S'
			for i in range(0, 27):
				out += struct.pack('f', actionf[i])
		elif self.close:
			willCapture = False
			out = b'X'
			for i in range(0, 27):
				out += struct.pack('f', actionf[i])
		else:
			out = b'A'
			for i in range(0, 27):
				out += struct.pack('f', actionf[i])
				
		# Write action
		assert(len(out) == 1 + 27 * 4)
		
		self.connection.send(out)

		# Receive new state
		
		# Read
		sizeR = 4 + 34 * 4 + 4 + 4

		data = bytes()
		
		while len(data) < sizeR:
			try:
				data += self.connection.recv(sizeR - len(data))
			except ConnectionResetError:
				self.connectionReset = True
				exit(0)
			
		# Read reward
		reward = struct.unpack('f', data[0:4])[0]
			
		for i in range(0, 34):
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

	def reset(self):
		self.r = True
		self.state = np.zeros((34))
		self.capture = False
		return self.state

	def render(self, mode='human', close=False):
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
		if not self.connectionReset:
			self.connection.send(b'X')
			self.connection.close()
