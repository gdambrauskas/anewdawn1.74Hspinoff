# Rise of Mankind EventManager
# by Zappara

from CvPythonExtensions import *
import CvEventInterface
import CvUtil
import Popup as PyPopup
import SdToolKit
import PyHelpers
#try:
#	import cPickle as pickle
#except:
#	import pickle
import CvGameUtils
import BugOptions
import BugCore
import BugUtil
import OOSLogger
import PlayerUtil
#import autolog

# BUG - Mac Support - start
BugUtil.fixSets(globals())
# BUG - Mac Support - end

gc = CyGlobalContext()
localText = CyTranslator()
PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo
PyCity = PyHelpers.PyCity
PyGame = PyHelpers.PyGame

SD_MOD_ID = "RiseOfMankind"

g_modEventManager = None
g_eventMgr = None
g_autolog = None

class RoMEventManager:
	def __init__(self, eventManager):
		eventManager.addEventHandler("ModNetMessage", self.onModNetMessage)
		eventManager.addEventHandler("OnLoad", self.onLoadGame)
		eventManager.addEventHandler("GameStart", self.onGameStart)
		eventManager.addEventHandler("BeginPlayerTurn", self.onBeginPlayerTurn)
		eventManager.addEventHandler("buildingBuilt", self.onBuildingBuilt)
		eventManager.addEventHandler("projectBuilt", self.onProjectBuilt)
		eventManager.addEventHandler("unitBuilt", self.onUnitBuilt)
		eventManager.addEventHandler("cityBuilt", self.onCityBuilt)
		eventManager.addEventHandler("cityRazed", self.onCityRazed)
		eventManager.addEventHandler("cityLost", self.onCityLost)
		eventManager.addEventHandler("gameUpdate", self.onGameUpdate)
		
		global g_modEventManager
		g_modEventManager = self
		
		#global g_autolog
		#g_autolog = autolog.autologInstance()
		
		global g_eventMgr
		g_eventMgr = eventManager
		self.eventManager = eventManager
		
		# RoM start Next War tracks cities that have been razed
		self.iArcologyCityID = -1
		
		self.iBUILDING_CRUSADE = 0
		self.iBUILDING_DJENNE = 0
		self.iBUILDING_WORLD_BANK = 0
		
		self.iTECH_KNOWLEDGE_MANAGEMENT = 0
		self.iTECH_APPLIED_ECONOMICS = 0
		self.m_iNetMessage_Colonist = 410
		self.m_iNetMessage_Pioneer = 411
		self.iPROJECT_EDEN = 0
		self.iUNIT_CRUSADER = 0
		self.iBUILDING_NANITE_DEFUSER = 0
		self.iTECH_FUNDAMENTALISM = 0

	def onModNetMessage(self, argsList):
		'Called whenever CyMessageControl().sendModNetMessage() is called - this is all for you modders!'
		
		iData1, iData2, iData3, iData4, iData5 = argsList
		
		print("Modder's net message!")
		
		CvUtil.pyPrint( 'onModNetMessage' )
		
		# Rise of Mankind start
		iMessageID = iData1
		
# Rise of Mankind 2.71
		# NetModMessage 410
		# SettlersEventManager.py / Colonist
		# Add additional buildings and change city size
		if ( gc.getDefineINT("ROM_MULTIPLAYER_FIX") <= 0 ):
			if ( iMessageID == self.m_iNetMessage_Colonist ):
				iPlotX = iData2
				iPlotY = iData3
				iOwner = iData4
				iUnitID = iData5
			
				pPlot = CyMap( ).plot( iPlotX, iPlotY )
				pCity = pPlot.getPlotCity( )
				pPlayer = gc.getPlayer( iOwner )
				
				pCity.setPopulation(3)
				self.addCityBuildings(pCity, "BUILDINGCLASS_BARRACKS")
				self.addCityBuildings(pCity, "BUILDINGCLASS_GRANARY")
				self.addCityBuildings(pCity, "BUILDINGCLASS_FORGE")
				self.addCityBuildings(pCity, "BUILDINGCLASS_MARKET")
				if pCity.plot().isCoastalLand():
					self.addCityBuildings(pCity, "BUILDINGCLASS_HARBOR")
					self.addCityBuildings(pCity, "BUILDINGCLASS_LIGHTHOUSE")
					self.addCityBuildings(pCity, "BUILDINGCLASS_FISHERMAN_HUT")
			
			# NetModMessage 411
			# SettlersEventManager.py / Pioneer
			# Add additional buildings and change city size
			if ( iMessageID == self.m_iNetMessage_Pioneer ):
				iPlotX = iData2
				iPlotY = iData3
				iOwner = iData4
				iUnitID = iData5
			
				pPlot = CyMap( ).plot( iPlotX, iPlotY )
				pCity = pPlot.getPlotCity( )
				pPlayer = gc.getPlayer( iOwner )
				
				pCity.setPopulation(4)
				self.addCityBuildings(pCity, "BUILDINGCLASS_GARRISON")
				self.addCityBuildings(pCity, "BUILDINGCLASS_GRANARY")
				self.addCityBuildings(pCity, "BUILDINGCLASS_FORGE")
				self.addCityBuildings(pCity, "BUILDINGCLASS_COURTHOUSE")
				self.addCityBuildings(pCity, "BUILDINGCLASS_MARKET")
				self.addCityBuildings(pCity, "BUILDINGCLASS_STABLE")
				self.addCityBuildings(pCity, "BUILDINGCLASS_GROCER")
				self.addCityBuildings(pCity, "BUILDINGCLASS_DOCTOR")
				self.addCityBuildings(pCity, "BUILDINGCLASS_BANK")
				self.addCityBuildings(pCity, "BUILDINGCLASS_LIBRARY")
				if pCity.plot().isCoastalLand():
					self.addCityBuildings(pCity, "BUILDINGCLASS_PORT")
					self.addCityBuildings(pCity, "BUILDINGCLASS_LIGHTHOUSE")
					self.addCityBuildings(pCity, "BUILDINGCLASS_FISHERMAN_HUT")
					
	def addCityBuildings(self, pCity, szBuilding):
		iBuilding = gc.getInfoTypeForString(szBuilding)
		iUniqueBuilding = gc.getCivilizationInfo(gc.getPlayer(pCity.getOwner()).getCivilizationType()).getCivilizationBuildings(iBuilding)
		if iUniqueBuilding > -1:
			if pCity.canConstruct(iUniqueBuilding, False, False, False):
				pCity.setNumRealBuilding(iUniqueBuilding, 1)
				
	def onLoadGame(self, argsList):
		self.initProperties()

	def onGameStart(self, argsList):
		'Called at the start of the game'
		# def onBuildingBuilt / Additional tests variable
		self.initProperties()
	
	def initProperties(self):
		self.iBUILDING_CRUSADE = gc.getInfoTypeForString("BUILDING_CRUSADE")
		self.iBUILDING_DJENNE = gc.getInfoTypeForString("BUILDING_DJENNE")
		self.iBUILDING_WORLD_BANK = gc.getInfoTypeForString("BUILDING_WORLD_BANK")
				
		self.iTECH_KNOWLEDGE_MANAGEMENT = gc.getInfoTypeForString("TECH_KNOWLEDGE_MANAGEMENT")
		self.iTECH_APPLIED_ECONOMICS = gc.getInfoTypeForString("TECH_APPLIED_ECONOMICS")
		self.iPROJECT_EDEN = gc.getInfoTypeForString("PROJECT_EDEN")
		self.iUNIT_CRUSADER = gc.getInfoTypeForString("UNIT_CRUSADER")
		self.iBUILDING_NANITE_DEFUSER = gc.getInfoTypeForString("BUILDING_NANITE_DEFUSER")
		self.iTECH_FUNDAMENTALISM = gc.getInfoTypeForString("TECH_FUNDAMENTALISM")

	def onBeginPlayerTurn(self, argsList):
		'Called at the beginning of a players turn'
		iGameTurn, iPlayer = argsList
		pPlayer = gc.getPlayer(iPlayer)
		
		pPlayer = gc.getPlayer(iPlayer)
		if gc.getTeam(pPlayer.getTeam()).isHasTech(self.iTECH_APPLIED_ECONOMICS):
			obsoleteTech = gc.getBuildingInfo(self.iBUILDING_WORLD_BANK).getObsoleteTech()
			if ( gc.getTeam(pPlayer.getTeam()).isHasTech(obsoleteTech) == false or obsoleteTech == -1 ):
				for iCity in range(pPlayer.getNumCities()):
					ppCity = pPlayer.getCity(iCity)
					if ppCity.getNumActiveBuilding(self.iBUILDING_WORLD_BANK) == true:
						#pPID = pPlayer.getID()
						iGold = pPlayer.getGold( )
						pPlayer.changeGold( iGold//100 )
		# Crusade Start ##
#		test = ( iGameTurn % 14 )
#		BugUtil.info ( "crusade test: %d", test) 
		self.spawnCrusader(iGameTurn, pPlayer, self.iUNIT_CRUSADER, self.iBUILDING_CRUSADE, self.iTECH_FUNDAMENTALISM)
		# Crusade End #
	
	def spawnCrusader(self, currentGameTurn, player, unit, building, tech):
		if not gc.getTeam(player.getTeam()).isHasTech(tech):
			return
		obsoleteTech = gc.getBuildingInfo(building).getObsoleteTech()
		if (obsoleteTech != -1 and gc.getTeam(player.getTeam()).isHasTech(obsoleteTech) == true):
			return
		# BugUtil.info("unit spawn, technology check passed")
		self.spawnUnitInEachEligibleCity(currentGameTurn, player, unit, [building])
		# Crusade End #
	
	def spawnUnitInEachEligibleCity(self, currentGameTurn, player, unit, enabling_buildings):
		for iCity in range(player.getNumCities()):
			ppCity = player.getCity(iCity)
			can_build_unit = false
			for enabling_building in enabling_buildings:
				if ppCity.getNumActiveBuilding(enabling_building) == true:
					can_build_unit = true
					break
			if not can_build_unit:
				continue
			iX = ppCity.getX()
			iY = ppCity.getY()
			estiEnd = CyGame().getEstimateEndTurn()
			if ( estiEnd >= 1800 ):
				if ( currentGameTurn % 16 ) == 0:
					pNewUnit = player.initUnit(unit, iX, iY, UnitAITypes.UNITAI_ATTACK_CITY, DirectionTypes.NO_DIRECTION )
			elif ( estiEnd >= 1400 ):
				if ( currentGameTurn % 14 ) == 0:
					pNewUnit = player.initUnit(unit, iX, iY, UnitAITypes.UNITAI_ATTACK_CITY, DirectionTypes.NO_DIRECTION )
			elif ( estiEnd >= 1000 ):
				if ( currentGameTurn % 12 ) == 0:
					pNewUnit = player.initUnit(unit, iX, iY, UnitAITypes.UNITAI_ATTACK_CITY, DirectionTypes.NO_DIRECTION )
			elif ( estiEnd >= 700 ):
				if ( currentGameTurn % 8 ) == 0:
					pNewUnit = player.initUnit(unit, iX, iY, UnitAITypes.UNITAI_ATTACK_CITY, DirectionTypes.NO_DIRECTION )
			elif ( estiEnd >= 500 ):
				if ( currentGameTurn % 6 ) == 0:
					pNewUnit = player.initUnit(unit, iX, iY, UnitAITypes.UNITAI_ATTACK_CITY, DirectionTypes.NO_DIRECTION )
			elif ( estiEnd >= 300 ):
				if ( currentGameTurn % 4 ) == 0:
					pNewUnit = player.initUnit(unit, iX, iY, UnitAITypes.UNITAI_ATTACK_CITY, DirectionTypes.NO_DIRECTION )
			else:
				if ( currentGameTurn % 4 ) == 0:
					pNewUnit = player.initUnit(unit, iX, iY, UnitAITypes.UNITAI_ATTACK_CITY, DirectionTypes.NO_DIRECTION )
	
	def onBuildingBuilt(self, argsList):
		'Building Completed'
		pCity, iBuildingType = argsList
		game = gc.getGame()

		# Djenne
		if (iBuildingType == self.iBUILDING_DJENNE):
			pPlayer = gc.getPlayer(pCity.plot().getOwner())
			pPID = pPlayer.getID()
			pTID = pPlayer.getTeam()
			iX = pCity.getX()
			iY = pCity.getY()
			tt_desert = gc.getInfoTypeForString( 'TERRAIN_DESERT' )

			for iXLoop in range(iX - 2, iX + 3, 1):
				for iYLoop in range(iY - 2, iY + 3, 1):
					pPlot = CyMap().plot(iXLoop, iYLoop)
					if ( pPlot.isPlayerCityRadius(pPID)==true ):
						if ( pPlot.getTeam()==pTID ):
							if ( pPlot.getTerrainType()==tt_desert ):
								CyGame().setPlotExtraYield(iXLoop, iYLoop, YieldTypes.YIELD_COMMERCE, 8)
			
			CyInterface().addMessage(pPID,false,15,CyTranslator().getText("TXT_KEY_DJENNE_PYTHON",()),'',0,'Art/Interface/Buttons/Buildings/Djenne.dds',ColorTypes(44), iX, iY, True,True)

		
		# world bank national wonder
		if iBuildingType == self.iBUILDING_WORLD_BANK:

			pPlayer = gc.getPlayer(pCity.plot().getOwner())
			pPID = pPlayer.getID()
			iGold = pPlayer.getGold( )
			pPlayer.changeGold( iGold//2 )
		
		# Crusade check
		if iBuildingType == self.iBUILDING_CRUSADE:
			pPlayer = gc.getPlayer(pCity.plot().getOwner())
			iPID = pPlayer.getID()
			iTID = pPlayer.getTeam()
			iX = pCity.getX()
			iY = pCity.getY()

			for i in range(1):
				pNewUnit = pPlayer.initUnit( self.iUNIT_CRUSADER, iX, iY, UnitAITypes.UNITAI_ATTACK_CITY, DirectionTypes.NO_DIRECTION )
		
		# NANITE DEFUSER - destroyes all nukes from all players
		if (iBuildingType == self.iBUILDING_NANITE_DEFUSER):
			pPlayer = gc.getPlayer(pCity.plot().getOwner())
			iX = pCity.getX()
			iY = pCity.getY()
			for player in PlayerUtil.players(alive=True):
				pPID = player.getID()
				for unit in PlayerUtil.playerUnits(player):
					if (unit.getUnitCombatType() == gc.getInfoTypeForString("UNITCOMBAT_DOOM") or unit.nukeRange() > 0 or unit.getUnitAIType() == UnitAITypes.UNITAI_ICBM and not unit.isNone()):
						unit.kill( 0, -1 )
				CyInterface().addMessage(pPID,false,15,CyTranslator().getText("TXT_KEY_NANITE_DEFUSER_PYTHON",()),'',0,'Art/Interface/Buttons/Buildings/Ascension_Gate.dds',ColorTypes(44), iX, iY, True,True)


	def onProjectBuilt(self, argsList):
		'Project Completed'
		pCity, iProjectType = argsList
		game = gc.getGame()
		
		# Eden project
		if iProjectType == self.iPROJECT_EDEN:
			pPlayer = gc.getPlayer(pCity.plot().getOwner())
			pPID = pPlayer.getID()
			pTID = pPlayer.getTeam()
			iX = pCity.getX()
			iY = pCity.getY()
			tt_desert = gc.getInfoTypeForString( 'TERRAIN_DESERT' )
			tt_plain = gc.getInfoTypeForString( 'TERRAIN_PLAINS' )
			tt_grass = gc.getInfoTypeForString( 'TERRAIN_GRASS' )
			tt_tundra = gc.getInfoTypeForString( 'TERRAIN_TUNDRA' )
			tt_snow = gc.getInfoTypeForString( 'TERRAIN_SNOW' )
			tt_ocean = gc.getInfoTypeForString( 'TERRAIN_OCEAN' )
			tt_marsh = gc.getInfoTypeForString( 'TERRAIN_MARSH' )
			tt_coast = gc.getInfoTypeForString( "TERRAIN_COAST" )

			for iXLoop in range(iX - 50, iX + 50, 1):
				for iYLoop in range(iY - 50, iY + 50, 1):
					pPlot = CyMap().plot(iXLoop, iYLoop)
					#if ( pPlot.isPlayerCityRadius(pPID)==true ):
					if ( pPlot.getTeam()==pTID ):
						if ( pPlot.getTerrainType()==tt_grass ):
							if ( pPlot.getImprovementType()!=gc.getInfoTypeForString( 'IMPROVEMENT_FARM' )) and ( pPlot.getImprovementType()!=gc.getInfoTypeForString( 'IMPROVEMENT_VERTICAL_FARM' )) and ( pPlot.getImprovementType()!=gc.getInfoTypeForString( 'IMPROVEMENT_WINDMILL' )) and ( pPlot.getImprovementType()!=gc.getInfoTypeForString( 'IMPROVEMENT_PLANTATION' )) and ( pPlot.getImprovementType()!=gc.getInfoTypeForString( 'IMPROVEMENT_OLIVE_FARM' )) and ( pPlot.getImprovementType()!=gc.getInfoTypeForString( 'IMPROVEMENT_APPLE_FARM' )) and ( pPlot.getImprovementType()!=gc.getInfoTypeForString( 'IMPROVEMENT_WINERY' )) and ( pPlot.getImprovementType()!=gc.getInfoTypeForString( 'IMPROVEMENT_COTTAGE' )) and ( pPlot.getImprovementType()!=gc.getInfoTypeForString( 'IMPROVEMENT_HAMLET' )) and ( pPlot.getImprovementType()!=gc.getInfoTypeForString( 'IMPROVEMENT_VILLAGE' )) and ( pPlot.getImprovementType()!=gc.getInfoTypeForString( 'IMPROVEMENT_TOWN' )) and ( pPlot.getImprovementType()!=gc.getInfoTypeForString( 'IMPROVEMENT_FOREST_PRESERVE' )):
								if ( pPlot.getFeatureType()!=gc.getInfoTypeForString( 'FEATURE_JUNGLE' )):
									pPlot.setFeatureType(gc.getInfoTypeForString( "FEATURE_FOREST" ), 1)
						elif ( pPlot.getTerrainType()==tt_plain ):
							pPlot.setTerrainType(tt_grass, 1, 1)
						elif ( pPlot.getTerrainType()==tt_marsh ):
							pPlot.setTerrainType(tt_grass, 1, 1)
						elif ( pPlot.getTerrainType()==tt_tundra ):
							pPlot.setTerrainType(tt_plain, 1, 1)
						elif ( pPlot.getTerrainType()==tt_snow ):
							pPlot.setTerrainType(tt_tundra, 1, 1)
						elif ( pPlot.getTerrainType()==tt_ocean ):
							pPlot.setTerrainType(tt_coast, 1, 1)
						elif ( pPlot.getTerrainType()==tt_desert ):
							pPlot.setTerrainType(tt_plain, 1, 1)
			
			CyInterface().addMessage(pPID,false,15,CyTranslator().getText("TXT_KEY_EDEN_PYTHON",()),'',0,'Art/Interface/Buttons/Buildings/Eden.dds',ColorTypes(44), iX, iY, True,True)

	def onUnitBuilt(self, argsList):
		'Unit Completed'
		city = argsList[0]
		unit = argsList[1]
		player = PyPlayer(city.getOwner())

		# if player has Technocracy civic active, give free promotion to unit
		iPlayer = gc.getPlayer(city.getOwner())
		if gc.getTeam(iPlayer.getTeam()).isHasTech(self.iTECH_KNOWLEDGE_MANAGEMENT):
			iFutureCivicOption = CvUtil.findInfoTypeNum(gc.getCivicOptionInfo,gc.getNumCivicOptionInfos(),"CIVICOPTION_FUTURE")
			iTechnocracy = CvUtil.findInfoTypeNum(gc.getCivicInfo,gc.getNumCivicInfos(),"CIVIC_TECHNOCRACY")

			iFutureCivic = iPlayer.getCivics(iFutureCivicOption)

			if (iFutureCivic == iTechnocracy):
				if ( gc.getTeam(iPlayer.getTeam()).isHasTech(gc.getInfoTypeForString("TECH_SMART_DUST")) == true ):
					iSensors =  CvUtil.findInfoTypeNum(gc.getPromotionInfo, gc.getNumPromotionInfos(),'PROMOTION_SENSORS')
					if (unit.isPromotionValid(iSensors)):
						unit.setHasPromotion(iSensors, True)


	def onCityBuilt(self, argsList):
		'City Built'
		city = argsList[0]

		pUnit = CyInterface().getHeadSelectedUnit()
		if pUnit:
			if ( gc.getDefineINT("ROM_MULTIPLAYER_FIX") <= 0 ):
			# if multiplayer fix for Colonist and Pioneer unit isn't enabled, allow them to work 
				#pAdvancedSettlers=CvEventInterface.getEventManager()
			
				if pUnit.getUnitClassType() == gc.getInfoTypeForString("UNITCLASS_COLONIST"):

					pPlot = CyMap( ).plot( pUnit.getX( ), pUnit.getY( ) )
					iMessageID = 410
					BugUtil.info("RoM - Colonist messageID: %d", iMessageID)
					iPlotX = pPlot.getX()
					iPlotY = pPlot.getY()
					iOwner = pUnit.getOwner()
					iUnitID = pUnit.getID()
					CyMessageControl( ).sendModNetMessage( iMessageID, iPlotX, iPlotY, iOwner, iUnitID )
				
				elif pUnit.getUnitClassType() == gc.getInfoTypeForString("UNITCLASS_PIONEER"):
					pPlot = CyMap( ).plot( pUnit.getX( ), pUnit.getY( ) )
					iMessageID = 411
					BugUtil.info("RoM - Pioneer messageID: %d", iMessageID)
					iPlotX = pPlot.getX()
					iPlotY = pPlot.getY()
					iOwner = pUnit.getOwner()
					iUnitID = pUnit.getID()
					CyMessageControl( ).sendModNetMessage( iMessageID, iPlotX, iPlotY, iOwner, iUnitID )
	
	def onCityRazed(self, argsList):
		'City Razed'
		city, iPlayer = argsList
		
		self.iArcologyCityID = -1
		
		if city.getNumRealBuilding(gc.getInfoTypeForString("BUILDING_ARCOLOGY")) or city.getNumRealBuilding(gc.getInfoTypeForString("BUILDING_ARCOLOGY_SHIELDING")) or city.getNumRealBuilding(gc.getInfoTypeForString("BUILDING_ADVANCED_SHIELDING")):
			self.iArcologyCityID = city.getID()


	def onCityLost(self, argsList):
		'City Lost'
		city = argsList[0]
		player = PyPlayer(city.getOwner())
		
		if city.getID() == self.iArcologyCityID:
			city.plot().setImprovementType(gc.getInfoTypeForString("IMPROVEMENT_CITY_RUINS_ARCOLOGY"))
	
	def onGameUpdate(self, argsList):
		'sample generic event, called on each game turn slice'
		genericArgs = argsList[0][0]	# tuple of tuple of my args
		OOSLogger.doGameUpdate()