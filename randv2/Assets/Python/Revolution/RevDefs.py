# Definitions for Revolution Mod
#
# by jdog5000
# Version 0.5

from CvPythonExtensions import *
import CvUtil
import PyHelpers
import BugCore


# #globals
gc = CyGlobalContext()
# PyPlayer = PyHelpers.PyPlayer
# PyInfo = PyHelpers.PyInfo
game = CyGame()
localText = CyTranslator()
RevOpt = BugCore.game.Revolution

LOG_DEBUG = True
config = None


# Players
# List of player numbers that BarbarianCiv will never turn into a full civ.  Allows minor civs in scenarios to keep minor status.
alwaysMinorList = [] #[0,1,2]

## --------- XML variables used in the mod ----------- ##
# If your mod changes some of these XML variables, you'll need to find an appropriate replacement

# Civs
sXMLMinor = 'CIVILIZATION_MINOR'
sXMLBarbarian = 'CIVILIZATION_BARBARIAN'

# Units
sXMLSpy = 'UNIT_SPY'
sXMLScout = 'UNITCLASS_SCOUT'
sXMLSettler = 'UNITCLASS_SETTLER'
sXMLWarrior = 'UNITCLASS_WARRIOR'
sXMLWorker = 'UNITCLASS_WORKER'
sXMLGeneral = 'UNIT_GREAT_GENERAL'
sXMLProphet = 'UNIT_PROPHET'
sXMLEngineer = 'UNIT_ENGINEER'
sXMLArtist = 'UNIT_ARTIST'
sXMLMerchant = 'UNIT_MERCHANT'
sXMLScientist = 'UNIT_SCIENTIST'
# Rise of Mankind 2.61
sXMLArcher = 'UNITCLASS_ARCHER'
# Rise of Mankind 2.61

# Buildings
# Used by Rev when rebels capture a tiny city first
sXMLPalace = "BUILDINGCLASS_PALACE"
# Given to BarbCivs under various circumstances
sXMLLibrary = 'BUILDINGCLASS_LIBRARY'
sXMLGranary = 'BUILDINGCLASS_GRANARY'
sXMLBarracks = 'BUILDINGCLASS_BARRACKS'
sXMLMarket = 'BUILDINGCLASS_MARKET'
sXMLWalls = 'BUILDINGCLASS_WALLS'
sXMLLighthouse = 'BUILDINGCLASS_LIGHTHOUSE'
sXMLForge = 'BUILDINGCLASS_FORGE'
sXMLMonument = 'BUILDINGCLASS_OBELISK'
# Rise of Mankind 2.6 additions
sXMLBazaar = 'BUILDINGCLASS_BAZAAR'
sXMLScribes = 'BUILDINGCLASS_SCHOOL_OF_SCRIBES'
# Rise of Mankind 2.6

# Techs
# Used by Rev, weight of nationality effects increases after discovery
sXMLNationalism = 'TECH_NATIONALISM'
# Used by Rev, weight of religious effects decreases after each discovery
sXMLLiberalism = 'TECH_LIBERALISM'
sXMLSciMethod = 'TECH_SCIENTIFIC_METHOD'
# Given to BarbCivs under varying circumstances
sXMLSailing = 'TECH_SAILING'
sXMLWheel = 'TECH_THE_WHEEL'
sXMLAnimal = 'TECH_ANIMAL_HUSBANDRY'
sXMLHorseback = 'TECH_HORSEBACK_RIDING'
sXMLBronze = 'TECH_BRONZE_WORKING'
sXMLIron = 'TECH_IRON_WORKING'
sXMLGuilds = 'TECH_GUILDS'
# Rise of Mankind 2.6 additions
sXMLChariotry = 'TECH_CHARIOTRY'
sXMLMetalCasting = 'TECH_METAL_CASTING'
sXMLNavalWarfare = 'TECH_NAVAL_WARFARE'
sXMLWeaving = 'TECH_WEAVING'
sXMLMining = 'TECH_MINING'
# Rise of Mankind 2.6

# Promotions
# Given to rebel units
sXMLWoodsman3 = 'PROMOTION_WOODSMAN3'
sXMLSentry = 'PROMOTION_SENTRY'
sXMLDrill2 = 'PROMOTION_DRILL2'
sXMLGuerilla3 = 'PROMOTION_GUERILLA3'
sXMLCommando = 'PROMOTION_COMMANDO'
# Given to viking style BarbCivs
sXMLAmphibious = 'PROMOTION_AMPHIBIOUS'

# Traits
# Used by Rev for AI decisions, BarbCiv to determine type of settling
sXMLAggressive = 'TRAIT_AGGRESSIVE'
sXMLSpiritual = 'TRAIT_SPIRITUAL'
sXMLExpansive = 'TRAIT_EXPANSIVE'

# Goodies
sXMLGoodyMap = 'GOODY_MAP'

# Improvements
sXMLFort = 'IMPROVEMENT_FORT'

# Terrain
sXMLOcean = "TERRAIN_OCEAN"
sXMLCoast = "TERRAIN_COAST"

## ---------- Data structures for various objects ---------- ##

# Each city records its revolution status and history
cityData = dict()
# RevolutionIndex is the main measure of rebelliousness in a city, local records effects from factors in the city (as opposed to national factors)
cityData['PrevRevIndex'] = 0
cityData['RevIdxHistory'] = None
# Data about past revolutions
# TODO: change to list
cityData['RevolutionCiv'] = None
cityData['RevolutionTurn'] = None
# Counters to control timing for various features
cityData['WarningCounter'] = 0
cityData['SmallRevoltCounter'] = 0
# Bribe info
cityData['BribeTurn'] = None
cityData['TurnBribeCosts'] = None

revIdxHistKeyList = ['Happiness', 'Location', 'Colony', 'Nationality', 'Religion', 'Health', 'Garrison', 'Disorder', 'RevoltEffects', 'Events']
revIdxHistLen = 5

def initRevIdxHistory( ) :
	global revIdxHistKeyList

	revIdxHist = dict()
	for key in revIdxHistKeyList :
		revIdxHist[key] = [0]

	return revIdxHist


# Each player records its relation to other civs
# TODO: record national factors here?  create a civ wide measure of stability that interoperates with current city based approach?
playerData = dict()
# List of [iPlayer, iRevoltIdx] from which to spawn revolutionaries of this player on the next turn
playerData['SpawnList'] = list()
# Dictionary of revolts, held by index
playerData['RevoltDict'] = dict()
# Store civics to notice changes
playerData['CivicList'] = None
# Information about revolutions
playerData['RevolutionTurn'] = None
playerData['MotherlandID'] = None
playerData['JoinPlayerID'] = None
playerData['CapitalName'] = None


# Need this at all?
unitData = dict()


# Container for data passed by revolution popups

class RevoltData :

	def __init__( self, iPlayer, iRevTurn, cityList, revType, bPeaceful, specialDataDict = dict() ) :

		# Player whose cities are in revolt
		self.iPlayer = iPlayer
		self.iRevTurn = iRevTurn
		# List of cities revolting
		self.cityList = cityList
		# String describing revolution type
		self.revType = revType
		# Bool describing whether revolt is peaceful
		self.bPeaceful = bPeaceful

		# Includes special info for this revolution type (from list below, where self.___ would be specialDataDict['___'])
		self.specialDataDict = specialDataDict
		# self.iRevPlayer = iRevPlayer
		# self.bIsJoinWar = bIsJoinWar
		# self.iJoinPlayer = iJoinPlayer
		# self.iNewLeaderType = iNewLeaderType
		# self.newLeaderName = newLeaderName
		# self.bIsElection = bIsElection
		# self.iNewCivic = iNewCivic
		# self.iNewReligion = iNewReligion
		# self.iHappiness = iHappiness
		# self.iBuyOffCost = iBuyOffCost
		# self.vassalStyle = vassalStyle
		# self.bOfferPeace = bOfferPeace
		# self.bSwitchToRevs

		self.dict = self.toDict()


	def toDict( self ) :

		dataDict = dict()
		dataDict['iPlayer'] = self.iPlayer
		dataDict['cityList'] = self.cityList
		dataDict['revType'] = self.revType
		dataDict['bPeaceful'] = self.bPeaceful

		dataDict.update( self.specialDataDict )

		return dataDict

	def fromDict( self, sourceDict ) :
		# To use, pass Nones to RevoltData() then call this func with a full dict

		self.specialDataDict = dict()

		for [key,value] in sourceDict.iteritems() :
			if( key == 'iPlayer' ) :
				self.iPlayer = value
			elif( key == 'cityList' ) :
				self.cityList = value
			elif( key == 'revType' ) :
				self.revType = value
			elif( key == 'bPeaceful' ) :
				self.bPeaceful = value
			else :
				self.specialDataDict[key] = value

		self.dict = self.toDict()

## ---------- Revolution constants ---------- ##
# Changing these values is not recommended
revReadyFrac = .6
revInstigatorThreshold = 1000
alwaysViolentThreshold = 1700
badLocalThreshold = 10


## ---------- Popup number defines ---------- ##

# Revolution
revolutionPopup = 7000
revWatchPopup = 7001
joinHumanPopup = 7002
controlLostPopup = 7003
assimilationPopup = 7004
pickCityPopup = 7005
bribeCityPopup = 7006

# AIAutoPlay
toAIChooserPopup = 7050
abdicatePopup = 7051
pickHumanPopup = 7052

# ChangePlayer
changeCivPopup = 7060
changeHumanPopup = 7061
updateGraphicsPopup = 7062

# BarbarianCiv
barbSettlePopup = 7070

# Tester
testerPopup = 8000
waitingForPopup = 8001
setNamePopup = 8002
newNamePopup = 8003
#civicsPopup = 8004
formConfedPopup = 8005
dissolveConfedPopup = 8006
scorePopup = 8007
#recreateUnitsPopup = 8008
specialMovePopup = 8009

# Keep game from showing messages about handling these popups
CvUtil.SilentEvents.extend([toAIChooserPopup,revolutionPopup,revWatchPopup,joinHumanPopup,controlLostPopup,assimilationPopup,pickCityPopup,bribeCityPopup,abdicatePopup,pickHumanPopup])
CvUtil.SilentEvents.extend([changeCivPopup,changeHumanPopup,barbSettlePopup])

## ---------- Misc defines ---------- ##

# Popup ids
EventKeyDown=6

#RevolutionDCM
## ---------- RevWatch defines ---------- ##
showTrend = 5