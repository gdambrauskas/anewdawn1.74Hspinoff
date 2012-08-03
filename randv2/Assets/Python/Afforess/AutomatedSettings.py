## A New Dawn Mod Code
##

from CvPythonExtensions import *
gc = CyGlobalContext()
import BugOptions
import BugCore
import BugUtil
import Popup as PyPopup
import CvUtil
import PlayerUtil
AutomatedOpts = BugCore.game.AutomatedSettings
MainOpt = BugCore.game.MainInterface

MODDEROPTION_EVENT_ID = CvUtil.getNewEventID("Modder Options")
CANAUTOBUILD_EVENT_ID = CvUtil.getNewEventID("CanAutoBuild")
CANPLAYERAUTOBUILD_EVENT_ID = CvUtil.getNewEventID("CanPlayerAutoBuild")

class AutomatedSettings:
	def __init__(self, eventManager):
		eventManager.addEventHandler("OnLoad", self.onLoadGame)
		eventManager.addEventHandler("GameStart", self.onGameStart)
		eventManager.addEventHandler("ModNetMessage", self.onModNetMessage)

	def onLoadGame(self,argsList):
		self.optionUpdate()
		MainOpt.setShowOptionsKeyReminder(True)

	def onGameStart(self,argsList):
		self.optionUpdate()
		MainOpt.setShowOptionsKeyReminder(True)

	def optionUpdate(self):	
		if AutomatedOpts.isReset():
			resetOptions()
		else:
			setXMLOptionsfromIniFile()

	def onModNetMessage(self, argsList):
		protocol, data1, data2, data3, data4 = argsList
		#ModderOptions
		if (protocol == MODDEROPTION_EVENT_ID):
			pPlayer = gc.getPlayer(data1)
			pPlayer.setModderOption(data2, data3)
		elif (protocol == CANAUTOBUILD_EVENT_ID):
			pPlayer = gc.getPlayer(data1)
			pCity = pPlayer.getCity(data2)
			pCity.setAutomatedCanBuild(data3, data4)
		elif (protocol == CANPLAYERAUTOBUILD_EVENT_ID):
			pPlayer = gc.getPlayer(data1)
			pPlayer.setAutomatedCanBuild(data3, data4)

##################################################		
# Module level functions defined in RoMSettings.xml	
##################################################	

def getCanAutoBuildEventID():
	return CANAUTOBUILD_EVENT_ID
	
def getCanPlayerAutoBuildEventID():
	return CANPLAYERAUTOBUILD_EVENT_ID

def changedReset (option, value):
	resetOptions()
	return True

########################################################################
def changedAvoidEnemyUnits(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_PILLAGE_AVOID_ENEMY_UNITS, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_PILLAGE_AVOID_ENEMY_UNITS), int(value), 0)
	
def changedAvoidBarbarianCities(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_PILLAGE_AVOID_BARBARIAN_CITIES, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_PILLAGE_AVOID_BARBARIAN_CITIES), int(value), 0)
	
def changedHideAutomatePillage(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_PILLAGE, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_PILLAGE), int(value), 0)

def changedNoCapturingCities(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_HUNT_NO_CITY_CAPTURING, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_HUNT_NO_CITY_CAPTURING), int(value), 0)

def changedAllowUnitSuiciding(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_HUNT_ALLOW_UNIT_SUICIDING, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_HUNT_ALLOW_UNIT_SUICIDING), int(value), 0)
	
def changedHideAutomateHunt(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_HUNT, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_HUNT), int(value), 0)
	
def changedAutoHuntMinimumAttackOdds(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_HUNT_MIN_COMBAT_ODDS, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_HUNT_MIN_COMBAT_ODDS), int(value), 0)
		
def changedCanLeaveBorders(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_PATROL_CAN_LEAVE_BORDERS, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_PATROL_CAN_LEAVE_BORDERS), int(value), 0)

def changedPatrolAllowUnitSuiciding(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_PATROL_ALLOW_UNIT_SUICIDING, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_PATROL_ALLOW_UNIT_SUICIDING), int(value), 0)
	
def changedNoPatrolCapturingCities(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_PATROL_NO_CITY_CAPTURING, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_PATROL_NO_CITY_CAPTURING), int(value), 0)
	
def changedHideAutomatePatrol(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_PATROL, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_PATROL), int(value), 0)
	
def changedAutoPatrolMinimumAttackOdds(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_PATROL_MIN_COMBAT_ODDS, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_PATROL_MIN_COMBAT_ODDS), int(value), 0)

def changedCanLeaveCity(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_DEFENSE_CAN_LEAVE_CITY, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_DEFENSE_CAN_LEAVE_CITY), int(value), 0)
	
def changedHideAutomateDefense(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_DEFENSE, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_DEFENSE), int(value), 0)
	
def changedAutoDefenseMinimumAttackOdds(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_DEFENSE_MIN_COMBAT_ODDS, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_DEFENSE_MIN_COMBAT_ODDS), int(value), 0)
		
def changedAirUnitCanDefend(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_AIR_CAN_DEFEND, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_AIR_CAN_DEFEND), int(value), 0)
	
def changedAirUnitCanRebase(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_AIR_CAN_REBASE, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_AIR_CAN_REBASE), int(value), 0)
	
def changedHideAirAutomations(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_AIR, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_AIR), int(value), 0)
			
def changedHideAutoExplore(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_EXPLORE, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_EXPLORE), int(value), 0)
	
def changedHideAutoSpread(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_SPREAD, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_SPREAD), int(value), 0)
	
def changedHideAutoCaravan(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_CARAVAN, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_CARAVAN), int(value), 0)
		
def changedHideAutoPirate(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_PIRATE, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_PIRATE), int(value), 0)
	
def changedAutoPirateMinimumAttackOdds(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_AUTO_PIRATE_MIN_COMBAT_ODDS, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_AUTO_PIRATE_MIN_COMBAT_ODDS), int(value), 0)
	
def changedHideAutoProtect(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_PROTECT, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_PROTECT), int(value), 0)
	
def changedHideAutoUpgrade(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_UPGRADE, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_UPGRADE), int(value), 0)

def changedMostExpensive(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_UPGRADE_MOST_EXPENSIVE, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_UPGRADE_MOST_EXPENSIVE), int(value), 0)
	
def changedMostExpierenced(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_UPGRADE_MOST_EXPERIENCED, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_UPGRADE_MOST_EXPERIENCED), int(value), 0)
	
def changedMinimumUpgradeGold(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_UPGRADE_MIN_GOLD, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_UPGRADE_MIN_GOLD), int(value), 0)
	
def changedHideAutoPromote(option, value):
	gc.getActivePlayer().setModderOption(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_PROMOTE, value)
	CyMessageControl().sendModNetMessage(MODDEROPTION_EVENT_ID, gc.getGame().getActivePlayer(), int(ModderOptionTypes.MODDEROPTION_HIDE_AUTO_PROMOTE), int(value), 0)	
	
	
	
def setXMLOptionsfromIniFile():
	BugUtil.debug("Initializing Automated Settings")

	AutomatedOptions = BugOptions.getOptions("AutomatedSettings").options
	for i in range(len(AutomatedOptions)):
		if (AutomatedOptions[i].getKey() != "Reset"):
			AutomatedOptions[i].doDirties()
		#changedDefenderWithdraw(ANewDawnOpt, ANewDawnOpt.isDefenderWithdraw())

	
def resetOptions():
	AutomatedOptions = BugOptions.getOptions("AutomatedSettings").options
	for i in range(len(AutomatedOptions)):
		AutomatedOptions[i].resetValue()
	setXMLOptionsfromIniFile()
	AutomatedOptions.setReset(False)