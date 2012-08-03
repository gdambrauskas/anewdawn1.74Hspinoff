# Console Functions -- Adds various functions for modders to access in the console

from CvPythonExtensions import *
import CvUtil
import ModToolsUtils

gc = CyGlobalContext()
map = CyMap()


#clears the map
def clearMap():

	for iPlayer in range(0,int(gc.getMAX_PLAYERS())):
		pPlayer = gc.getPlayer(iPlayer)
		if pPlayer.isAlive() :
			if not map.plot(iPlayer, 0).isCity() :
				map.plot(iPlayer, 0).erase()
				map.plot(iPlayer, 0).setTerrainType (CvUtil.findInfoTypeNum(gc.getTerrainInfo, gc.getNumTerrainInfos(), "TERRAIN_SNOW"), True, True)
				pPlayer.initCity(iPlayer, 0)
			pPlayer.killUnits()


#spawns a unit of every unitclass for all civs that are alive in a game
def showUnits():

	ModToolsUtils.clearMap()
	playersPerRow = (map.getGridWidth() - 2) / 12
	Height = 1 + gc.getNumUnitClassInfos() / 10
	playersCompleted = 0

	for iPlayer in range(0,int(gc.getMAX_PLAYERS())):
		pPlayer = gc.getPlayer(iPlayer)
		if (pPlayer.isAlive() and not pPlayer.isBarbarian()):
			if ( (Height * (playersCompleted / playersPerRow)) + (Height + 3 + (playersCompleted / playersPerRow) )  > map.getGridHeight() ):
				print "Unit Column height excedes map height, aborting"
				break
			for iUnitClass in range( gc.getNumUnitClassInfos() ):
				eLoopUnit = gc.getCivilizationInfo(pPlayer.getCivilizationType()).getCivilizationUnits(iUnitClass)
				if eLoopUnit != -1:
					if playersCompleted == 0:
						x = iUnitClass%10 + 1 + iPlayer*12
						y = (iUnitClass/10) + 3
						map.plot( x, y ).erase()
						map.plot( x, y ).setTerrainType(CvUtil.findInfoTypeNum(gc.getTerrainInfo, gc.getNumTerrainInfos(), "TERRAIN_OCEAN"), 1, 1)
						pPlayer.initUnit( eLoopUnit, x, y, UnitAITypes.UNITAI_UNKNOWN, DirectionTypes.NO_DIRECTION )
					else:
						row = playersCompleted / playersPerRow
						x = iUnitClass%10 + 1 + (iPlayer % playersPerRow)*12
						y = (iUnitClass/10) + (Height * row) + (3 + row)
						map.plot( x, y ).erase()
						map.plot( x, y ).setTerrainType(CvUtil.findInfoTypeNum(gc.getTerrainInfo, gc.getNumTerrainInfos(), "TERRAIN_OCEAN"), 1, 1)
						pPlayer.initUnit( eLoopUnit, x, y, UnitAITypes.UNITAI_UNKNOWN, DirectionTypes.NO_DIRECTION )
			playersCompleted += 1


#spawns a unit of every unitclass for all civs that are alive in a game
def showUnitsTerrain(TerrainType):

	ModToolsUtils.clearMap()
	playersPerRow = (map.getGridWidth() - 2) / 12
	Height = 1 + gc.getNumUnitClassInfos() / 10
	playersCompleted = 0

	for iPlayer in range(0,int(gc.getMAX_PLAYERS())):
		pPlayer = gc.getPlayer(iPlayer)
		if (pPlayer.isAlive() and not pPlayer.isBarbarian()):
			if ( (Height * (playersCompleted / playersPerRow)) + (Height + 3 + (playersCompleted / playersPerRow) )  > map.getGridHeight() ):
				print "Unit Column height excedes map height, aborting"
				break
			for iUnitClass in range( gc.getNumUnitClassInfos() ):
				eLoopUnit = gc.getCivilizationInfo(pPlayer.getCivilizationType()).getCivilizationUnits(iUnitClass)
				if eLoopUnit != -1:
					if playersCompleted == 0:
						x = iUnitClass%10 + 1 + iPlayer*12
						y = (iUnitClass/10) + 3
						map.plot( x, y ).erase()
						map.plot( x, y ).setTerrainType(CvUtil.findInfoTypeNum(gc.getTerrainInfo, gc.getNumTerrainInfos(), TerrainType), 1, 1)
						pPlayer.initUnit( eLoopUnit, x, y, UnitAITypes.UNITAI_UNKNOWN, DirectionTypes.NO_DIRECTION )
					else:
						row = playersCompleted / playersPerRow
						x = iUnitClass%10 + 1 + (iPlayer % playersPerRow)*12
						y = (iUnitClass/10) + (Height * row) + (3 + row)
						map.plot( x, y ).erase()
						map.plot( x, y ).setTerrainType(CvUtil.findInfoTypeNum(gc.getTerrainInfo, gc.getNumTerrainInfos(), TerrainType), 1, 1)
						pPlayer.initUnit( eLoopUnit, x, y, UnitAITypes.UNITAI_UNKNOWN, DirectionTypes.NO_DIRECTION )
			playersCompleted += 1


def showUnitClass(UnitClass):

	iUnitClass = gc.getInfoTypeForString(UnitClass)
	startingY = map.getGridHeight() / 3

	for iPlayer in range(0,int(gc.getMAX_PLAYERS())):
		if (gc.getPlayer(iPlayer).isAlive() and not gc.getPlayer(iPlayer).isBarbarian()):
			eUnit = gc.getCivilizationInfo(gc.getPlayer(iPlayer).getCivilizationType()).getCivilizationUnits(iUnitClass)
			if (eUnit != -1):
				x = 1 + iPlayer
				y = startingY
				while ( ( not map.plot( x, y ).isNone()) and (map.plot( x, y ).getNumUnits() > 0) ):
					y += 1
				if( y > map.getGridHeight()):
					print "Map is too full of units to populate, please execute clearMap()"
					break
				map.plot( x, y ).setTerrainType(CvUtil.findInfoTypeNum(gc.getTerrainInfo, gc.getNumTerrainInfos(), "TERRAIN_OCEAN"), 1, 1)
				gc.getPlayer(iPlayer).initUnit( eUnit, x, y, UnitAITypes.UNITAI_UNKNOWN, DirectionTypes.NO_DIRECTION )


def advanceEra():

	gameEra = 0
	for iPlayer in range(0,int(gc.getMAX_PLAYERS())):
		if (gc.getPlayer(iPlayer).isAlive() and not gc.getPlayer(iPlayer).isBarbarian()):
			teamEra = 0
			pTeam = gc.getTeam(gc.getPlayer(iPlayer).getTeam())
			for iTech in range( 0,int(gc.getNumTechInfos()) ):
				if pTeam.isHasTech(iTech) :
					techEra = gc.getTechInfo(iTech).getEra()
					if techEra > teamEra:
						teamEra = techEra
			if teamEra > gameEra:
				gameEra = teamEra
	targetEra = gameEra + 1

	for iPlayer in range(0,int(gc.getMAX_PLAYERS())):
		if (gc.getPlayer(iPlayer).isAlive() and not gc.getPlayer(iPlayer).isBarbarian()):
			pTeam = gc.getTeam(gc.getPlayer(iPlayer).getTeam())
		if targetEra <= gc.getNumEraInfos():
			for iTech in range( 0,int(gc.getNumTechInfos()) ):
				if gc.getTechInfo(iTech).getEra() <= targetEra:
					pTeam.setHasTech(iTech,True,iPlayer,False,False)
