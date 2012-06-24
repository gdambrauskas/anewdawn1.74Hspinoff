from CvPythonExtensions import *
import CvEventInterface
import CvUtil
import Popup as PyPopup
import SdToolKit
import PyHelpers

import CvGameUtils
import BugOptions
import BugCore
import BugUtil
import OOSLogger
import PlayerUtil

BugUtil.fixSets(globals())

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

class NukeAfterEffects:
	def __init__(self, eventManager):

		self.__LOG_IMPROVEMENT =0

		eventManager.addEventHandler("nukeExplosion", self.onNukeExplosion)
		eventManager.addEventHandler("improvementBuilt", self.onImprovementBuilt)

		global g_modEventManager
		g_modEventManager = self
		
		#global g_autolog
		#g_autolog = autolog.autologInstance()
		
		global g_eventMgr
		g_eventMgr = eventManager
		self.eventManager = eventManager

	def onImprovementBuilt(self, argsList):
		'Improvement Built'
		iImprovement, iX, iY = argsList
###AdvancedNukesbegin###
		pPlot = CyMap().plot(iX,iY)
		if (gc.getInfoTypeForStringWithHiddenAssert("IMPROVEMENT_SANITIZE_WATER") > 0):
			if(iImprovement==gc.getInfoTypeForString('IMPROVEMENT_SANITIZE_WATER')):
				pPlot.setTerrainType(gc.getInfoTypeForString( "TERRAIN_COAST" ), 1, 1)
				pPlot.setImprovementType(-1)
###AdvancedNukesend###
			if (not self.__LOG_IMPROVEMENT):
				return
			CvUtil.pyPrint('Improvement %s was built at %d, %d'
				%(PyInfo.ImprovementInfo(iImprovement).getDescription(), iX, iY))

	def onNukeExplosion(self, argsList):
		'Nuke Explosion'
		pPlot, pNukeUnit = argsList
###AdvancedNukesbegin###
		if (gc.getInfoTypeForStringWithHiddenAssert("UNIT_TURN") > 0):
			if (pNukeUnit is not None and pNukeUnit.getUnitType() == gc.getInfoTypeForString('UNIT_TURN')):
				if (pPlot.isCity()==true):
					iPlayer = pNukeUnit.getOwner()
					pPlayer = gc.getPlayer(iPlayer)
					pCity = pPlot.getPlotCity()
					pPlayer.acquireCity(pCity,false,false)
					iX = pPlot.getX()
					iY = pPlot.getY()
					for iiX in range(iX-1, iX+2, 1):
						for iiY in range(iY-1, iY+2, 1):
							numUnits = pPlot.getNumUnits()
							for e in xrange(numUnits,0,-1):
									pUnit = pPlot.getUnit(e)
									pUnit.kill(false, -1)  
							pNukedPlot = CyMap().plot(iiX,iiY)
							if (pNukedPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FALLOUT')):
								pNukedPlot.setFeatureType(-1, -1)
	###AdvancedNukesend###
			CvUtil.pyPrint('Nuke detonated at %d, %d'
				%(pPlot.getX(), pPlot.getY()))
###AdvancedNukesbegin###
		if (gc.getInfoTypeForStringWithHiddenAssert("UNIT_FUSION_NUKE") > 0):
			if (pNukeUnit is not None and pNukeUnit.getUnitType() == gc.getInfoTypeForString('UNIT_FUSION_NUKE')):

							
				iX = pPlot.getX()
				iY = pPlot.getY()

				for iXLoop in range(iX - 0, iX + 1, 1):
					for iYLoop in range(iY - 0, iY + 1, 1):
							pPlot = CyMap().plot(iXLoop, iYLoop)
							if (( pPlot.isPeak()==true  ) or (pPlot.isHills()==true)):
								pPlot.setPlotType(PlotTypes.PLOT_LAND, True, True)
							pPlot.setTerrainType(gc.getInfoTypeForString( "TERRAIN_COAST" ), 1, 1)
							
		if (gc.getInfoTypeForStringWithHiddenAssert("UNIT_FUSION_NOVA") > 0):
			if (pNukeUnit is not None and pNukeUnit.getUnitType() == gc.getInfoTypeForString('UNIT_FUSION_NOVA')):

							
				iX = pPlot.getX()
				iY = pPlot.getY()

				for iXLoop in range(iX - 1, iX + 2, 1):
					for iYLoop in range(iY - 1, iY + 2, 1):
							pPlot = CyMap().plot(iXLoop, iYLoop)
							if (( pPlot.isPeak()==true  ) or (pPlot.isHills()==true)):
								pPlot.setPlotType(PlotTypes.PLOT_LAND, True, True)
							pPlot.setTerrainType(gc.getInfoTypeForString( "TERRAIN_COAST" ), 1, 1)
							
		if (gc.getInfoTypeForStringWithHiddenAssert("UNIT_POISON_NUKE") > 0):
			if (pNukeUnit is not None and pNukeUnit.getUnitType() == gc.getInfoTypeForString('UNIT_POISON_NUKE')):
				iX = pPlot.getX()
				iY = pPlot.getY()
				for iXLoop in range(iX - 1, iX + 2, 1):
					for iYLoop in range(iY - 1, iY + 2, 1):
							pPlot = CyMap().plot(iXLoop, iYLoop)
							pPlot.setFeatureType(gc.getInfoTypeForString( "FEATURE_BIOGAS" ), 1)
							if ( pPlot.isWater()==true  ):
								pPlot.setTerrainType(gc.getInfoTypeForString( "TERRAIN_SLIMY_COAST" ), 1, 1)
		
		if (gc.getInfoTypeForStringWithHiddenAssert("UNIT_POISON_NOVA") > 0):
			if (pNukeUnit is not None and pNukeUnit.getUnitType() == gc.getInfoTypeForString('UNIT_POISON_NOVA')):
				iX = pPlot.getX()
				iY = pPlot.getY()
				for iXLoop in range(iX - 5, iX + 6, 1):
					for iYLoop in range(iY - 5, iY + 6, 1):
							pPlot = CyMap().plot(iXLoop, iYLoop)
							pPlot.setFeatureType(gc.getInfoTypeForString( "FEATURE_PLAGUEGAS" ), 1)
							if ( pPlot.isWater()==true  ):
								pPlot.setTerrainType(gc.getInfoTypeForString( "TERRAIN_SLIMY_OCEAN" ), 1, 1)

###AdvancedNukesend###
