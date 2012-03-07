
from CvPythonExtensions import *

import CvUtil
import CvEventManager
import sys
import PyHelpers
import CvConfigParser
import math
import time
import ScreenInput
import CvScreenEnums
			
gc = CyGlobalContext()	

PyPlayer = PyHelpers.PyPlayer
PyGame = PyHelpers.PyGame()

localText = CyTranslator()

g_iXResolution = 1024
g_iYResolution = 720


	
class ScreenResolutionSize:

	def setValues(self):
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE)
		global g_iXResolution
		global g_iYResolution
		g_iXResolution = screen.getXResolution()
		g_iYResolution = screen.getYResolution()

		gc.getGame().setXResolution(g_iXResolution)
		gc.getGame().setYResolution(g_iYResolution)

	def __init__(self, eventManager):

		eventManager.addEventHandler("GameStart", self.onGameStart)
		eventManager.addEventHandler("windowActivation", self.onWindowActivation)
		eventManager.addEventHandler("OnLoad", self.onLoadGame)

		self.setValues()
		
	def onWindowActivation(self, argsList):
		'Called when the game window activates or deactivates'
		bActive = argsList[0]
		
		if(bActive):
			self.setValues()
	
		
	def onGameStart(self, argsList):
		self.setValues()
		
	def onLoadGame(self, argsList):
		self.setValues()