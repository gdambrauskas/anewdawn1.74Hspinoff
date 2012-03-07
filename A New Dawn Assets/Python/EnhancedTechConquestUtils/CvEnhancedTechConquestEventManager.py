## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005

from CvPythonExtensions import *
import CvUtil
import CvEventManager
import sys
import EnhancedTechConquest
	
gc = CyGlobalContext()

enhancedTechConquest = EnhancedTechConquest.EnhancedTechConquest()

# globals
###################################################
class CvEnhancedTechConquestEventManager:
	def __init__(self, eventManager):
		eventManager.addEventHandler("cityAcquired", self.onCityAcquired)
		eventManager.addEventHandler("windowActivation", self.onWindowActivation)
	
		EnhancedTechConquest.loadConfigurationData()

	def onCityAcquired(self, argsList):
		enhancedTechConquest.onCityAcquired(argsList)
		
		
	def onWindowActivation(self, argsList):
		'Called when the game window activates or deactivates'
		bActive = argsList[0]
		
		if(bActive):
			EnhancedTechConquest.loadConfigurationData()
