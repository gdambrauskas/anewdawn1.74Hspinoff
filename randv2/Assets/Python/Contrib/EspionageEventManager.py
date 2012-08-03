## espionage event manager
## by ruff_hi
##
## Required to store the prior players EP Value against the other players
## This information is used in the Espionage Screen to show EP spending per turn

from CvPythonExtensions import *
import CvUtil
import Popup as PyPopup
import PyHelpers
#import autolog
#import time
#import BugCore
#import BugUtil
#import TradeUtil

gc = CyGlobalContext()
#PyPlayer = PyHelpers.PyPlayer
#PyInfo = PyHelpers.PyInfo

import SdToolKit
sdEcho			= SdToolKit.sdEcho
sdModInit		= SdToolKit.sdModInit
sdModLoad		= SdToolKit.sdModLoad
sdModSave		= SdToolKit.sdModSave
sdEntityInit	= SdToolKit.sdEntityInit
sdEntityExists	= SdToolKit.sdEntityExists
sdEntityWipe	= SdToolKit.sdEntityWipe
sdGetVal		= SdToolKit.sdGetVal
sdSetVal		= SdToolKit.sdSetVal
sdGroup			= "EspionagePoints"

class EspionageEventManager:

	def __init__(self, eventManager):

		EspionageEvent(eventManager)

class AbstractEspionageEvent(object):

	def __init__(self, eventManager, *args, **kwargs):
		super(AbstractEspionageEventEvent, self).__init__(*args, **kwargs)

class EspionageEventEvent(AbstractEspionageEventEvent):

	def __init__(self, eventManager, *args, **kwargs):
		super(AutoLogEvent, self).__init__(eventManager, *args, **kwargs)

		eventManager.addEventHandler("BeginPlayerTurn", self.onBeginPlayerTurn)

	def onBeginPlayerTurn(self, argsList):
		'Called at the beginning of a players turn'
		iGameTurn, iPlayer = argsList

		if iPlayer != CyGame().getActivePlayer():
			return

		# check to see if we have already done this turn ...
		iCurrentTurn = gc.getGame().getGameTurn()
		zsSDKey = "Turn"
		if (sdEntityExists(sdGroup, zsSDKey) == False): #create record if it doesn't exist
			zDic = {'Turn':0}
			sdEntityInit(sdGroup, zsSDKey, zDic)

		if sdGetVal(sdGroup, zsSDKey, "Turn") == iCurrentTurn:
			return

		# save the current turn
		sdSetVal(sdGroup, zsSDKey, "Turn", iCurrentTurn)

		# loop through all the players, recording their EP against the other players
		for iCurrentPlayerLoop in range(gc.getMAX_PLAYERS()):
			pCurrentPlayer = gc.getPlayer(iCurrentPlayerLoop)
			iCurrentTeamID = pCurrentPlayer.getTeam()
			pCurrentTeam = gc.getTeam(iCurrentTeamID)
			for iTargetPlayerLoop in range(gc.getMAX_PLAYERS()):
				# skip if the current player is the target player
				if iCurrentPlayerLoop == iTargetPlayerLoop:
					continue

				pTargetPlayer = gc.getPlayer(iTargetPlayerLoop)
				iTargetTeamID = pTargetPlayer.getTeam()
				pTargetTeam = gc.getTeam(iTargetTeamID)

				zsSDKey = "%i-%i" %(iCurrentPlayerLoop, iTargetPlayerLoop)
				if (sdEntityExists(sdGroup, zsSDKey) == False): #create record if it doesn't exist
					zDic = {'Prior':0}
					sdEntityInit(sdGroup, zsSDKey, zDic)

				iPrior = 0
				if (iCurrentTeamID != iTargetTeamID
				and not pCurrentPlayer.isBarbarian()
				and not pTargetPlayer.isBarbarian()):
					if (pCurrentPlayer.isAlive()
					and pTargetPlayer.isAlive()):
						if (pCurrentTeam.isHasMet(iTargetTeamID)):
							iPrior = pCurrentTeam.getEspionagePointsAgainstTeam(iTargetTeamID)

				sdSetVal(sdGroup, zsSDKey, "Prior", iPrior)








