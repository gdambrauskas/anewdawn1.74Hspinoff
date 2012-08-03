## Copyright (c) 2009 The BUG Mod.
##
## Bug System: EmperorFool (thanks!)
## Author Glider1

from CvPythonExtensions import *
import BugUtil
import CvUtil
import BugCore
import Popup as PyPopup
import CvEventInterface
import PyHelpers

gc = CyGlobalContext()
RevDCMOpt = BugCore.game.RevDCM
PICK_RELIGION_EVENT = CvUtil.getNewEventID("PickReligion")
PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo
PyCity = PyHelpers.PyCity
PyGame = PyHelpers.PyGame
localText = CyTranslator()


#RevolutionDCM
#Class ReligionControl responds to "calls back" from the DLL via the BUG system. 
#This has been done by declaring the attachment in RevDCM.xml. BUG will then instanticiate 
#the class and attach it's DLL call backs.

class ReligionControl:
	def doHolyCity(self):
		if RevDCMOpt.isOC_LIMITED_RELIGIONS():
			#This algorithm has to deal with a PLETHORA of possibilities like multiple religions 
			#being founded by multiple players on the same turn.
			iPlayerQueue = []
			iSlotQueue = []
			# Sort through all religions and players and distinguish unique religions that want founding
			for iSlot in range(gc.getNumReligionInfos()):
				for iPlayer in range(gc.getMAX_PLAYERS()):
					PyPlayer = gc.getPlayer(iPlayer)
					if PyPlayer.isAlive():
						if not CvUtil.OwnsHolyCity(iPlayer):
							if gc.getTeam(PyPlayer.getTeam()).isHasTech(gc.getReligionInfo(iSlot).getTechPrereq()):
								if not CyGame().isReligionSlotTaken(iSlot):
									if not iSlot in iSlotQueue:
										iPlayerQueue.append(iPlayer)
										iSlotQueue.append(iSlot)
			if CyGame().isOption(GameOptionTypes.GAMEOPTION_PICK_RELIGION) or RevDCMOpt.isCHOOSE_RELIGION():
				#Throw the list to the popup manager that has to delegate multiple popups for each new religion
				pickReligion = PickReligionPopup(CvEventInterface.getEventManager())
				pickReligion.foundPickableReligions(iPlayerQueue, iSlotQueue)
			else:
				#Let the sdk handle the list as it normally would anyway
				for i in range(len(iPlayerQueue)):
					gc.getPlayer(iPlayerQueue[i]).foundReligion(iSlotQueue[i], iSlotQueue[i], True)
			return True
		if RevDCMOpt.isCHOOSE_RELIGION():
			#Stop the sdk founding a religion because all possible cases in vanilla BTS
			#have already been handled in doHolyCityTech.
			return true
		#false means allow the SDK to found a religion if needed
		return false
	
	def doHolyCityTech(self,argsList):
		eTeam = argsList[0]
		ePlayer = argsList[1]
		eTech = argsList[2]
		bFirst = argsList[3]
		if RevDCMOpt.isOC_LIMITED_RELIGIONS():
			#In limited religion, the assignment of religion cannot be done here
			#because there still could be more unknowable calls to this function
			#that affect the allocation of religions.
			return true
		if RevDCMOpt.isCHOOSE_RELIGION():
			pickReligion = PickReligionPopup(CvEventInterface.getEventManager())
			for iSlot in range(gc.getNumReligionInfos()):	
				prereq = gc.getReligionInfo(iSlot).getTechPrereq()
				if eTech == prereq and bFirst:
					pickReligion.initiatePopup(ePlayer, iSlot)
					break
			return true
		#false means allow the SDK to found a religion if needed
		return false
		
#####################################################
# ReligionControl Events
# Note: defined and attached from RevDCM.xml
#####################################################
		
def onBeginPlayerTurn(argsList):
	'Called at the beginning of a players turn'
	iGameTurn, iPlayer = argsList
	pPlayer = gc.getPlayer(iPlayer)
		
	# Code to determine if all cities have 6 "Religious Unity" Prerequisites by Orion Veteran
	# Special thanks to STO for providing the code strategy - Great Job! 
	# 1. A player must have an official State Religion
	# 2. A player must have the Holy City for the official State Religion	
	# 3. A player must have built the Shrine for the official State Religion			
	# 4. 80% a player's cities must have the official state religion established.
	# 5. All of a player's cities must not have any non-state religions established.
	# 6. Religious influence must be at least 80%	
	# RevolutionDCM NOTE - condition 3 has been temporarily disabled because the code is not yet compatible with mods that have greater than the default number of religions.	
	if (pPlayer.getNumCities() > 0) and (CyGame().getNumCivCities() > 2 * CyGame().countCivPlayersAlive()): # be sure the game is advanced enough to test a religious victory
		bUnity = True
		iStateReligion = pPlayer.getStateReligion()
		
		if iStateReligion == -1 :
			# No State Religion was Found - Prerequisite 1
			#CyInterface().addMessage(CyGame().getActivePlayer(),True,25,'No State Religion!','AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/TerrainFeatures/Forest.dds',ColorTypes(8),0,0,False,False)
			bUnity = False
			
		if bUnity :				
			if not pPlayer.hasHolyCity(iStateReligion) :
				# No Holy City was Found for the official state religion - Prerequisite 2
				#CyInterface().addMessage(CyGame().getActivePlayer(),True,25,'Holy City Not Found!','AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/TerrainFeatures/Forest.dds',ColorTypes(8),0,0,False,False)
				bUnity = False
		
		if bUnity :
			#RevolutionDCM - relaxation of prerequisite 4 because of the practical difficulty in spreading state religion to every satellite settlement
			maxNonStatePercentThreshold = 20
			nonStateReligionCount = 0	
			numCities = len(PyHelpers.PyPlayer(iPlayer).getCityList())

			for pyCity in PyHelpers.PyPlayer(iPlayer).getCityList() :
				if not bUnity :
					break
			
				for iReligionLoop in range(gc.getNumReligionInfos()):
					if iReligionLoop == iStateReligion :
						if not pyCity.hasReligion(iReligionLoop) :
							# City does not have the official state religion - Prerequisite 4
							#CyInterface().addMessage(CyGame().getActivePlayer(),True,25,'No State Religion founded in City!','AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/TerrainFeatures/Forest.dds',ColorTypes(8),0,0,False,False)
							nonStateReligionCount += 1
						nonStateReligionPercent = 100 * nonStateReligionCount / numCities
						if nonStateReligionPercent > maxNonStatePercentThreshold:
							bUnity = False
							break
					elif pyCity.hasReligion(iReligionLoop) :
						# City has a non-State religion - Prerequisite 5
						if not CvUtil.isHoly(pyCity):
							bUnity = False
							break
							
		if bUnity :
			if CyGame().calculateReligionPercent(iStateReligion) < 80 :
				# Religeous influence is less than 80% - Prerequisite 6
				#CyInterface().addMessage(CyGame().getActivePlayer(),True,25,'No Religion influence!','AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/TerrainFeatures/Forest.dds',ColorTypes(8),0,0,False,False)
				bUnity = False							

		if bUnity :
			if CyGame().isVictoryValid(gc.getInfoTypeForString("VICTORY_RELIGIOUS")) : 
				# The player has met all 5 prerequsites to achieve a valid religeous victory
				#CyInterface().addMessage(CyGame().getActivePlayer(),True,25,'Victory!','AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/TerrainFeatures/Forest.dds',ColorTypes(8),0,0,False,False)
				CyGame().setWinner(pPlayer.getTeam(), gc.getInfoTypeForString("VICTORY_RELIGIOUS"))
				
		
		
class PickReligionPopup:
	def __init__(self, eventManager):
		moreEvents = {
			PICK_RELIGION_EVENT : ('', self.__eventPickReligionApply,  self.__eventPickReligionBegin),
		}
		eventManager.Events.update(moreEvents)
		self.eventMgr = eventManager
		self.iSlotReligion = 0
		self.iPlayer = 0
		self.iReligionList = []
		self.foundCustomReligion = true
		self.iPlayerQueue = []
		self.iSlotQueue = []
		self.lock = false

	def foundPickableReligions(self, iPlayerQueue, iUniqueSlotQueue):
		self.iPlayerQueue = iPlayerQueue
		self.iSlotQueue = iUniqueSlotQueue
		if len(self.iPlayerQueue):
			self.initiatePopup(self.iPlayerQueue.pop(), self.iSlotQueue.pop())
			
	def initiatePopup(self, iPlayer, iSlot):
		self.iPlayer = iPlayer
		self.iSlotReligion = iSlot
		self.foundCustomReligion = false
		if CyGame().isOption(GameOptionTypes.GAMEOPTION_PICK_RELIGION):
			if gc.getPlayer(self.iPlayer).isHuman():
				self.foundCustomReligion = true
		if RevDCMOpt.isCHOOSE_RELIGION():
			self.foundCustomReligion = true
		if self.foundCustomReligion:
			self.eventMgr.beginEvent(PICK_RELIGION_EVENT)
		
	def __eventPickReligionBegin(self, argsList):
		if not self.lock:
			self.lock = true
			self.launchPopup()
		
	def launchPopup(self):
		activePlayer = gc.getActivePlayer().getID()
		CyGame().setAIAutoPlay(activePlayer, 0)
		activePlayerTeam = gc.getActivePlayer().getTeam()
		PY_activePlayerTeam = gc.getTeam(activePlayerTeam)
		thisPlayer = self.iPlayer
		PY_thisPlayer = gc.getPlayer(thisPlayer)
		thisPlayerTeam = PY_thisPlayer.getTeam()
		playerText = localText.getText("TXT_KEY_UNIT_REVDCM_RELIGION_DISTANT_CIV",())
		#~ playerText = "A distant civ"
		if thisPlayer == activePlayer:
			playerText = "You"
			#~ playerText = "You"
		elif PY_activePlayerTeam.isHasMet(thisPlayerTeam):
			playerText = PY_thisPlayer.getName()
		
		popup = PyPopup.PyPopup(PICK_RELIGION_EVENT, EventContextTypes.EVENTCONTEXT_SELF)
		size = 300
		rightCornerOffset = 110
		#popup.setSize(size, size)
		screenRes = CyUserProfile().getResolutionString(CyUserProfile().getResolution())
		textRes = screenRes.split('x')
		resX = int(textRes[0])
		popup.setPosition(resX-(size+rightCornerOffset), rightCornerOffset)
		popup.setHeaderString(localText.getText("TXT_KEY_UNIT_REVDCM_RELIGION_HEADER",()))
		#~ popup.setHeaderString("Choose religion option:")
		popup.setBodyString(playerText + localText.getText("TXT_KEY_UNIT_REVDCM_RELIGION_PART_2",()))
		#~ popup.setBodyString(playerText + " can found a religion! Choose it:")
		religionList = self.getRemainingReligions()
		for religion in religionList:
			religionText = gc.getReligionInfo(religion).getDescription()
			popup.addButton(religionText)
		if len(religionList):
			popup.launch(false)
			
	def __eventPickReligionApply(self, playerID, userData, popupReturn):
		iReligion = 0
		religionsRemaining = self.getRemainingReligions()
		if len(religionsRemaining):
			iReligion = self.getRemainingReligions()[popupReturn.getButtonClicked()]
		self.iReligionList.append(iReligion)
		if self.foundCustomReligion:
			gc.getPlayer(self.iPlayer).foundReligion(iReligion, self.iSlotReligion, True)
		else:
			#identical slots triggers the SDK to automatically resolve a religion
			gc.getPlayer(self.iPlayer).foundReligion(self.iSlotReligion, self.iSlotReligion, True)
		#Recursively call more popups
		self.lock = false
		if len(self.iPlayerQueue):
			self.foundPickableReligions(self.iPlayerQueue, self.iSlotQueue)		

	def getRemainingReligions(self):
		religionsRemaining = []
		for i in range(gc.getNumReligionInfos()):
			if not i in self.iReligionList and not CvUtil.isReligionExist(i):
				religionsRemaining.append(i)
		return religionsRemaining
