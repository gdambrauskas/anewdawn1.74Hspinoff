# Revolt class for launching and servicing revolts
#
# by jdog5000
# Version 1.5

from CvPythonExtensions import *
import CvUtil
import PyHelpers
import Popup as PyPopup
import math
# --------- Revolution mod -------------
import RevDefs
import RevData
import RevUtils
import SdToolKitCustom
import BugCore

# globals
gc = CyGlobalContext()
PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo
game = CyGame()
localText = CyTranslator()
RevOpt = BugCore.game.Revolution

class Revolt :
	
	def __init__( self, customEM, RevOpt ) :
		
		self.RevOpt = RevOpt
		self.customEM = customEM

		self.LOG_DEBUG = RevOpt.isRevDebugMode()
		
		self.revReadyFrac = RevDefs.revReadyFrac
		self.revInstigatorThreshold = RevDefs.revInstigatorThreshold
		self.alwaysViolentThreshold = RevDefs.alwaysViolentThreshold
		self.badLocalThreshold = RevDefs.badLocalThreshold
		
		self.againstPlayerList = list()
		self.iRevPlayer = -1
		self.cityList = list()
		
		self.iRevTurn = game.getGameTurn()
		
		
	
	def blankHandler( self, playerID, netUserData, popupReturn ) :
		# Dummy handler to take the second event for popup
		return
		
	def onCityLost(self, argsList):
		'City Lost'
		city = argsList[0]
		
		# If city lost by rebels, spawn a partisan or two
	
	def onCityAcquired( self, argsList):
		'City Acquired'
		owner,playerType,city,bConquest,bTrade = argsList
		
		# If rebels conquer a city off cityList, give them a big boost
		
		# If rebels conquer another city, give them some bonuses and defenders anyway
		
	
	
	def onSetPlayerAlive( self, argsList ) :
		# If rebel player has died, adjust revolt feelings in cityList
		
		iPlayerID = argsList[0]
		bNewValue = argsList[1]
		
		if( not bNewValue and iPlayerID == self.iRevPlayer ) :
			
			pPlayer = gc.getPlayer( iPlayerID )
			
			
		elif( not bNewValue and iPlayerID in self.againstPlayerList ) :
			pPlayer = gc.getPlayer( iPlayerID )
			