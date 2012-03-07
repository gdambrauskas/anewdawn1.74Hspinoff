## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
##
## Implementaion of miscellaneous game functions

import CvUtil
from CvPythonExtensions import *
import CvEventInterface
import Popup as PyPopup
import PyHelpers
import zCivics
import BugOptions
import BugCore
import BugUtil
import RevInstances
import RevDCM
import PlayerUtil

# globals
gc = CyGlobalContext()
PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo
PyCity = PyHelpers.PyCity
PyGame = PyHelpers.PyGame
civics = zCivics.zCivics()

# BUG - Mac Support - start
BugUtil.fixSets(globals())
# BUG - Mac Support - end

class RoMGameUtils:
	"Rise of Mankind mod game functions"
	
	def __init__(self):
		#RevolutionDCM - Inquisition Mod
		self.revModEnabled = not gc.getGame().isOption(GameOptionTypes.GAMEOPTION_NO_REVOLUTION)

		self.iNationalMint = gc.getInfoTypeForString("BUILDING_NATIONAL_MINT")
		
		self.iHimejiCastle = gc.getInfoTypeForString("BUILDING_HIMEJI_CASTLE")
		self.iDjenne = gc.getInfoTypeForString("BUILDING_DJENNE")
		self.iTerrainDesert = gc.getInfoTypeForString("TERRAIN_DESERT")
		self.iTechCloning = gc.getInfoTypeForString("TECH_CLONING")
		self.iObelisk = gc.getInfoTypeForString("BUILDINGCLASS_OBELISK")
		self.iRapidPrototyping = gc.getInfoTypeForString("TECH_RAPID_PROTOTYPING")
		self.iReplicators = gc.getInfoTypeForString("BONUS_PERSONAL_REPLICATORS")
			
	def doPillageGold(self, argsList):
		"controls the gold result of pillaging"
		pPlot = argsList[0]
		pUnit = argsList[1]
		
		iPillageGold = 0
		iPillageGold = CyGame().getSorenRandNum(gc.getImprovementInfo(pPlot.getImprovementType()).getPillageGold(), "Pillage Gold 1")
		iPillageGold += CyGame().getSorenRandNum(gc.getImprovementInfo(pPlot.getImprovementType()).getPillageGold(), "Pillage Gold 2")

		iPillageGold += (pUnit.getPillageChange() * iPillageGold) / 100
		
		# Himeji Samurai Castle Start #

		pPlayer = gc.getPlayer( pUnit.getOwner( ) )
		pPlayer2 = gc.getPlayer( pPlot.getOwner( ) )
		obsoleteTech = gc.getBuildingInfo(self.iHimejiCastle).getObsoleteTech()
		if ( gc.getTeam(pPlayer.getTeam()).isHasTech(obsoleteTech) == false or obsoleteTech == -1 ):
			for iCity in range(pPlayer.getNumCities()):
				ppCity = pPlayer.getCity(iCity)
				if ppCity.getNumActiveBuilding(self.iHimejiCastle) == true:
					iPillageGold = ( ( 0 ) * 2 ) 
					iPillageGold = ( ( CyGame().getSorenRandNum(gc.getImprovementInfo(pPlot.getImprovementType()).getPillageGold(), "Pillage Gold 1") ) * 2 )
					iPillageGold += ( ( CyGame().getSorenRandNum(gc.getImprovementInfo(pPlot.getImprovementType()).getPillageGold(), "Pillage Gold 2") ) * 2 )

					iPillageGold += ( ( (pUnit.getPillageChange() * iPillageGold) / 100 ) * 2 )

				else:
					iPillageGold = 0
					iPillageGold = CyGame().getSorenRandNum(gc.getImprovementInfo(pPlot.getImprovementType()).getPillageGold(), "Pillage Gold 1")
					iPillageGold += CyGame().getSorenRandNum(gc.getImprovementInfo(pPlot.getImprovementType()).getPillageGold(), "Pillage Gold 2")

					iPillageGold += (pUnit.getPillageChange() * iPillageGold) / 100

		if pPlot.getOwner( ) >= 0:
			if ( gc.getTeam(pPlayer2.getTeam()).isHasTech(obsoleteTech) == false or obsoleteTech == -1 ):
				for iCity in range(pPlayer2.getNumCities()):
					ppCity = pPlayer2.getCity(iCity)
					if ppCity.getNumActiveBuilding(self.iHimejiCastle) == true:
						iPillageGold = 0
		# Himeji Samurai Castle End #
		
		return iPillageGold
	
	def doCityCaptureGold(self, argsList):
		"controls the gold result of capturing a city"
		
		pOldCity = argsList[0]
		pPlayer = gc.getPlayer( pOldCity.getOwner( ) )
		
		iCaptureGold = 0
		
		iCaptureGold += gc.getDefineINT("BASE_CAPTURE_GOLD")
		iCaptureGold += (pOldCity.getPopulation() * gc.getDefineINT("CAPTURE_GOLD_PER_POPULATION"))
		iCaptureGold += CyGame().getSorenRandNum(gc.getDefineINT("CAPTURE_GOLD_RAND1"), "Capture Gold 1")
		iCaptureGold += CyGame().getSorenRandNum(gc.getDefineINT("CAPTURE_GOLD_RAND2"), "Capture Gold 2")

		if (gc.getDefineINT("CAPTURE_GOLD_MAX_TURNS") > 0):
			iCaptureGold *= cyIntRange((CyGame().getGameTurn() - pOldCity.getGameTurnAcquired()), 0, gc.getDefineINT("CAPTURE_GOLD_MAX_TURNS"))
			iCaptureGold /= gc.getDefineINT("CAPTURE_GOLD_MAX_TURNS")
			
		# Himeji Samurai Castle Start 

		obsoleteTech = gc.getBuildingInfo(self.iHimejiCastle).getObsoleteTech()

		if ( gc.getTeam(pPlayer.getTeam()).isHasTech(obsoleteTech) == false or obsoleteTech == -1 ):
			for iCity in range(pPlayer.getNumCities()):
				ppCity = pPlayer.getCity(iCity)
				if ppCity.getNumActiveBuilding(self.iHimejiCastle) == true:
					iCaptureGold = 0
				if ppCity.getNumActiveBuilding(self.iNationalMint) == true:
					iCaptureGold *= 10
		
		# Himeji Samurai Castle End 
		
		return iCaptureGold


	
	def getUpgradePriceOverride(self, argsList):
		iPlayer, iUnitID, iUnitTypeUpgrade = argsList

		pPlayer = gc.getPlayer(iPlayer)
		pUnit = pPlayer.getUnit(iUnitID)

		if ( gc.getTeam(pPlayer.getTeam()).isHasTech(self.iRapidPrototyping)):
			if pPlayer.hasBonus(self.iReplicators):
				iPrice = ((gc.getDefineINT("BASE_UNIT_UPGRADE_COST"))/2)
				iPrice += (((max(0, (pPlayer.getUnitProductionNeeded(iUnitTypeUpgrade) - pPlayer.getUnitProductionNeeded(pUnit.getUnitType()))) * gc.getDefineINT("UNIT_UPGRADE_COST_PER_PRODUCTION")))/2)

				if ((not pPlayer.isHuman()) and (not pPlayer.isBarbarian())):
					pHandicapInfo = gc.getHandicapInfo(gc.getGame().getHandicapType())
					iPrice = ((iPrice * pHandicapInfo.getAIUnitUpgradePercent() / 100)/2)
					iPrice = ((iPrice * max(0, ((pHandicapInfo.getAIPerEraModifier() * pPlayer.getCurrentEra()) + 100)) / 100)/2)

					iPrice = ((iPrice - ((iPrice * pUnit.getUpgradeDiscount()) / 100))/2)
		
				return iPrice
		else:
			return -1	# Any value 0 or above will be used
			
	def getExperienceNeeded(self, argsList):
		# use this function to set how much experience a unit needs
		iLevel, iOwner = argsList
		
		iExperienceNeeded = 0

		# regular epic game experience
		iExperienceNeeded = iLevel * iLevel + 1
#		iExperienceNeeded = iLevel * 5
		
		iModifier = gc.getPlayer(iOwner).getLevelExperienceModifier()
		if (0 != iModifier):
			iExperienceNeeded += (iExperienceNeeded * iModifier + 99) / 100   # ROUND UP
			
		return iExperienceNeeded