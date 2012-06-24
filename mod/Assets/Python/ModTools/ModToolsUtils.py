# Console Functions -- Adds various functions for modders to access in the console

from CvPythonExtensions import *
import CvUtil

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
