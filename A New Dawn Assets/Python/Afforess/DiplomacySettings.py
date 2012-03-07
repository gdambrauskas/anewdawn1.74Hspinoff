## A New Dawn Mod Code

from CvPythonExtensions import *
gc = CyGlobalContext()
import BugOptions
import BugCore
import BugUtil
import Popup as PyPopup
import CvUtil
import PlayerUtil
ANewDawnOpt = BugCore.game.DiplomacySettings

class DiplomacySettings:
	def __init__(self, eventManager):
		eventManager.addEventHandler("OnLoad", self.onLoadGame)
		eventManager.addEventHandler("GameStart", self.onGameStart)

	def onLoadGame(self,argsList):
		self.optionUpdate()

	def onGameStart(self,argsList):
		self.optionUpdate()

	def optionUpdate(self):
		if (BugCore.game.RoMSettings.isRoMReset()):
			resetOptions()
		else:
			setXMLOptionsfromIniFile()

def changedCanTradeTechs(option, value):
	gc.getGame().setOption(GameOptionTypes.GAMEOPTION_NO_TECH_TRADING, not ANewDawnOpt.isCanTradeTechs())
def changedCanBrokerTechs(option, value):
	gc.getGame().setOption(GameOptionTypes.GAMEOPTION_NO_TECH_BROKERING, not ANewDawnOpt.isCanBrokerTechs())
def changedCanTradeResources(option, value):
	gc.setDefineINT("CAN_TRADE_RESOURCES", ANewDawnOpt.isCanTradeResources())
def changedCanTradeCities(option, value):
	gc.setDefineINT("CAN_TRADE_CITIES", ANewDawnOpt.isCanTradeCities())
def changedCanTradeWorkers(option, value):
	gc.setDefineINT("CAN_TRADE_WORKERS", ANewDawnOpt.isCanTradeWorkers())
def changedCanTradeMilitary(option, value):
	gc.setDefineINT("NO_MILITARY_UNIT_TRADING", not ANewDawnOpt.isCanTradeMilitary())
def changedCanTradeGold(option, value):
	gc.setDefineINT("CAN_TRADE_GOLD", ANewDawnOpt.isCanTradeGold())
def changedCanTradeGoldPerTurn(option, value):
	gc.setDefineINT("CAN_TRADE_GOLD_PER_TURN", ANewDawnOpt.isCanTradeGoldPerTurn())
def changedCanTradeMaps(option, value):
	gc.setDefineINT("CAN_TRADE_MAPS", ANewDawnOpt.isCanTradeMaps())
def changedCanTradeVassals(option, value):
	gc.getGame().setOption(GameOptionTypes.GAMEOPTION_NO_VASSAL_STATES, not ANewDawnOpt.isCanTradeVassals())
def changedCanTradeEmbassies(option, value):
	gc.setDefineINT("CAN_TRADE_EMBASSIES", ANewDawnOpt.isCanTradeEmbassies())
def changedCanTradeContact(option, value):
	gc.setDefineINT("CAN_TRADE_CONTACT", ANewDawnOpt.isCanTradeContact())
def changedCanTradeCorporations(option, value):
	gc.setDefineINT("CAN_TRADE_CORPORATIONS", ANewDawnOpt.isCanTradeCorporations())
def changedCanTradePeace(option, value):
	gc.setDefineINT("CAN_TRADE_PEACE", ANewDawnOpt.isCanTradePeace())
def changedCanTradeWar(option, value):
	gc.setDefineINT("CAN_TRADE_WAR", ANewDawnOpt.isCanTradeWar())
def changedCanTradeEmbargo(option, value):
	gc.setDefineINT("CAN_TRADE_EMBARGO", ANewDawnOpt.isCanTradeEmbargo())	
def changedCanTradeCivics(option, value):
	gc.setDefineINT("CAN_TRADE_CIVICS", ANewDawnOpt.isCanTradeCivics())		
def changedCanTradeReligions(option, value):
	gc.setDefineINT("CAN_TRADE_RELIGIONS", ANewDawnOpt.isCanTradeReligions())		
def changedCanTradeOpenBorders(option, value):
	gc.setDefineINT("CAN_TRADE_OPEN_BORDERS", ANewDawnOpt.isCanTradeOpenBorders())	
def changedCanTradeLimitedBorders(option, value):
	gc.setDefineINT("CAN_TRADE_LIMITED_BORDERS", ANewDawnOpt.isCanTradeLimitedBorders())	
def changedCanTradeDefensivePact(option, value):
	gc.setDefineINT("CAN_TRADE_DEFENSIVE_PACT", ANewDawnOpt.isCanTradeDefensivePact())	
def changedCanTradeAlliance(option, value):
	gc.getGame().setOption(GameOptionTypes.GAMEOPTION_PERMANENT_ALLIANCES, ANewDawnOpt.isCanTradeAlliance())
def changedAdvancedDiplomacy(option, value):
	gc.getGame().setOption(GameOptionTypes.GAMEOPTION_ADVANCED_DIPLOMACY, ANewDawnOpt.isAdvancedDiplomacy())
		

	
def setXMLOptionsfromIniFile():
	BugUtil.debug("Initializing A New Dawn Diplomacy Settings")

	ANewDawnOpt.setCanTradeTechs(not gc.getGame().isOption(GameOptionTypes.GAMEOPTION_NO_TECH_TRADING))
	ANewDawnOpt.setCanBrokerTechs(not gc.getGame().isOption(GameOptionTypes.GAMEOPTION_NO_TECH_BROKERING))
	gc.setDefineINT("CAN_TRADE_RESOURCES", ANewDawnOpt.isCanTradeResources())
	gc.setDefineINT("CAN_TRADE_CITIES", ANewDawnOpt.isCanTradeCities())
	gc.setDefineINT("CAN_TRADE_WORKERS", ANewDawnOpt.isCanTradeWorkers())
	gc.setDefineINT("NO_MILITARY_UNIT_TRADING", not ANewDawnOpt.isCanTradeMilitary())
	gc.setDefineINT("CAN_TRADE_GOLD", ANewDawnOpt.isCanTradeGold())
	gc.setDefineINT("CAN_TRADE_GOLD_PER_TURN", ANewDawnOpt.isCanTradeGoldPerTurn())
	gc.setDefineINT("CAN_TRADE_MAPS", ANewDawnOpt.isCanTradeMaps())
	ANewDawnOpt.setCanTradeVassals(not gc.getGame().isOption(GameOptionTypes.GAMEOPTION_NO_VASSAL_STATES))
	gc.setDefineINT("CAN_TRADE_EMBASSIES", ANewDawnOpt.isCanTradeEmbassies())
	gc.setDefineINT("CAN_TRADE_CONTACT", ANewDawnOpt.isCanTradeContact())
	gc.setDefineINT("CAN_TRADE_CORPORATIONS", ANewDawnOpt.isCanTradeCorporations())
	gc.setDefineINT("CAN_TRADE_PEACE", ANewDawnOpt.isCanTradePeace())
	gc.setDefineINT("CAN_TRADE_WAR", ANewDawnOpt.isCanTradeWar())
	gc.setDefineINT("CAN_TRADE_EMBARGO", ANewDawnOpt.isCanTradeEmbargo())	
	gc.setDefineINT("CAN_TRADE_CIVICS", ANewDawnOpt.isCanTradeCivics())		
	gc.setDefineINT("CAN_TRADE_RELIGIONS", ANewDawnOpt.isCanTradeReligions())		
	gc.setDefineINT("CAN_TRADE_OPEN_BORDERS", ANewDawnOpt.isCanTradeOpenBorders())	
	gc.setDefineINT("CAN_TRADE_LIMITED_BORDERS", ANewDawnOpt.isCanTradeLimitedBorders())	
	gc.setDefineINT("CAN_TRADE_DEFENSIVE_PACT", ANewDawnOpt.isCanTradeDefensivePact())	
	ANewDawnOpt.setCanTradeAlliance(gc.getGame().isOption(GameOptionTypes.GAMEOPTION_PERMANENT_ALLIANCES))
	ANewDawnOpt.setAdvancedDiplomacy(gc.getGame().isOption(GameOptionTypes.GAMEOPTION_ADVANCED_DIPLOMACY))
	
def resetOptions():
	ANewDawnoptions = BugOptions.getOptions("DiplomacySettings").options
	for i in range(len(ANewDawnoptions)):
		ANewDawnoptions[i].resetValue()
	setXMLOptionsfromIniFile()
	BugCore.game.RoMSettings.setRoMReset(false)