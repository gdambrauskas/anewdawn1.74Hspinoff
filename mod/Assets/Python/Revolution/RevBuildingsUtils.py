# Traits functions for Revolution Mod
#
# by jdog5000
# Version 1.5

from CvPythonExtensions import *
import CvUtil
import PyHelpers
import pickle
# --------- Revolution mod -------------
import RevDefs
import RevData
import SdToolKitCustom
import RevInstances


# globals
gc = CyGlobalContext()
PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo
game = CyGame()
localText = CyTranslator()


########################## Traits effect helper functions #####################

def getBuildingsRevIdxLocal( pCity ) :

	localRevIdx = 0
	posList = list()
	negList = list()

	for iBuilding in range(gc.getNumBuildingInfos()):
		if ( pCity.getNumRealBuilding(iBuilding) > 0):
			kBuilding = gc.getBuildingInfo(iBuilding)
			buildingEffect = kBuilding.getRevIdxLocal()
			if( buildingEffect > 0 ) :
				negList.append( (buildingEffect, kBuilding.getDescription()) )
			elif( buildingEffect < 0 ) :
				posList.append( (buildingEffect, kBuilding.getDescription()) )

			#CvUtil.pyPrint("  Rev - %s local effect: %d"%(traitInfo.getDescription(),traitEffect))

			localRevIdx += buildingEffect

	return [localRevIdx,posList,negList]


def getBuildingsCivStabilityIndex( iPlayer ) :

	pPlayer = gc.getPlayer(iPlayer)

	civStabilityIdx = 0
	posList = list()
	negList = list()

	if( pPlayer.isNone() ) :
		return [civStabilityIdx,posList,negList]
		
	for iBuilding in range(0,gc.getNumBuildingInfos()):
		kBuilding = gc.getBuildingInfo(iBuilding)
		buildingEffect = -kBuilding.getRevIdxNational()

		if ( buildingEffect != 0 ):
			iBuildingClass = kBuilding.getBuildingClassType()
			numBuildings = pPlayer.getBuildingClassCount(iBuildingClass)

			buildingEffect *= numBuildings
			if( buildingEffect > 0 ) :
				posList.append( (buildingEffect, kBuilding.getDescription()) )
			elif( buildingEffect < 0 ) :
				negList.append( (buildingEffect, kBuilding.getDescription()) )

			#CvUtil.pyPrint("  Rev - %s local effect: %d"%(kBuilding.getDescription(),buildingsEffect))

			civStabilityIdx += buildingEffect

	return [civStabilityIdx,posList,negList]


def getBuildingsDistanceMod( pCity ) :

	distModifier = 0
	posList = list()
	negList = list()

	for iBuilding in range(gc.getNumBuildingInfos()):
		kBuilding = gc.getBuildingInfo(iBuilding)
		if ( kBuilding.getRevIdxDistanceModifier() != 0 ):
			if ( pCity.getNumRealBuilding(iBuilding) > 0):
				distModifier += kBuilding.getRevIdxDistanceModifier()

	return distModifier
