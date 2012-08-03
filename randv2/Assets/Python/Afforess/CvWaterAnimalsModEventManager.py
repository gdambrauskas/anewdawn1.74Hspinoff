#
# Water Animals Mod
# CvWaterAnimalsModEventManager
# 

from CvPythonExtensions import *

import CvUtil
import CvEventManager
import sys
import PyHelpers
import CvConfigParser
import math
import time
			
gc = CyGlobalContext()	

PyPlayer = PyHelpers.PyPlayer
PyGame = PyHelpers.PyGame()

localText = CyTranslator()

# Increase or decrease the value to change the chance that a water animal will 
# be spawned.
# Default value is 25
g_iWaterAnimalSpawnChance = 0

def loadConfigurationData():
	global g_iWaterAnimalSpawnChance
		
	config = CvConfigParser.CvConfigParser("Water Animals Mod Config.ini")

	if(config != None):
		g_iWaterAnimalSpawnChance = config.getint("Water Animals Mod", "Water Animal Spawn Chance", 25)

	gc.getGame().setWaterAnimalSpawnChance(g_iWaterAnimalSpawnChance)
	
	
class CvWaterAnimalsModEventManager:
	
	def __init__(self, eventManager):

		eventManager.addEventHandler("GameStart", self.onGameStart)
		eventManager.addEventHandler("windowActivation", self.onWindowActivation)
		#eventManager.addEventHandler("mouseEvent", self.onMouseEvent)

		loadConfigurationData()

	def onMouseEvent(self, argsList):
	
		#gc.getPlayer(18).killUnits()	
		CyInterface().addImmediateMessage("%s" %(gc.getGame().getWaterAnimalSpawnChance()),"")
		
	def onWindowActivation(self, argsList):
		'Called when the game window activates or deactivates'
		bActive = argsList[0]
		
		if(bActive):
			loadConfigurationData()	
	
		
	def onGameStart(self, argsList):
		loadConfigurationData()
