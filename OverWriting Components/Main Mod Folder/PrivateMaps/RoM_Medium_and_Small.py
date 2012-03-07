#
#	FILE:	 Medium_and_Small.py
#	AUTHOR:  Bob Thomas (Sirian)
#	PURPOSE: Global map script - Mixed islands and continents.
#-----------------------------------------------------------------------------
#	Copyright (c) 2007 Firaxis Games, Inc. All rights reserved.
#-----------------------------------------------------------------------------
#	1.10	Temudjin 15.July.10 - MapScriptTools
#                            - compatibility with 'Planetfall'
#                            - compatibility with 'Mars Now!'
#                            - add Map Option: World Shape
#                            - add Map Option: TeamStart
#                            - add Marsh terrain, if supported by mod
#                            - add Deep Ocean terrain, if supported by mod
#                            - add rivers on islands and from lakes
#                            - add Map Regions ( BigDent, BigBog, ElementalQuarter, LostIsle )
#                            - add Map Features ( Kelp, HauntedLands, CrystalPlains )
#                            - better balanced resources
#                            - print stats of mod and map
#                            - add getVersion()

from CvPythonExtensions import *
import CvUtil
import CvMapGeneratorUtil
#from CvMapGeneratorUtil import FractalWorld
#from CvMapGeneratorUtil import TerrainGenerator
#from CvMapGeneratorUtil import FeatureGenerator


################################################################
## MapScriptTools by Temudjin
################################################################
import MapScriptTools as mst
balancer = mst.bonusBalancer

def getVersion():
	return "1.10"
def getDescription():
	return "TXT_KEY_MAP_SCRIPT_MEDIUM_AND_SMALL_DESCR"

def beforeGeneration():
	print "-- beforeGeneration()"
	# Create mapInfo string
	mapInfo = ""
	for opt in range( getNumCustomMapOptions() ):
		nam = getCustomMapOptionName( [opt] )
		sel = CyMap().getCustomMapOption( opt )
		txt = getCustomMapOptionDescAt( [opt,sel] )
		mapInfo += "%27s:   %s\n" % ( nam, txt )
	# Initialize MapScriptTools
	mst.getModInfo( getVersion(), None, mapInfo )
	# Initialize bonus balancing
	balancer.initialize()
	beforeGeneration2()							# call renamed script function

def generateTerrainTypes():
	print "-- generateTerrainTypes()"

	# Planetfall: more highlands
	mst.planetFallMap.buildPfallHighlands()
	# Prettify map: most coastal peaks -> hills
	mst.mapPrettifier.hillifyCoast()
	# Prettify map: Connect small islands
	mst.mapPrettifier.bulkifyIslands()
	# Generate terrain
	terrainGen = mst.MST_TerrainGenerator()
	terrainTypes = terrainGen.generateTerrain()
	return terrainTypes

def addRivers():
	print "-- addRivers()"
	# Generate DeepOcean-terrain if mod allows for it
	mst.deepOcean.buildDeepOcean()
	# Planetfall: handle shelves and trenches
	mst.planetFallMap.buildPfallOcean()
	# Generate marsh-terrain
	mst.marshMaker.convertTerrain()

	# Build between 0..3 mountain-ranges.
	mst.mapRegions.buildBigDents()
	# Build between 0..3 bog-regions.
	mst.mapRegions.buildBigBogs()
	# Build ElementalQuarter
	mst.mapRegions.buildElementalQuarter()

	# no rivers on Mars
	if not mst.bMars:
		# Put rivers on the map.
		CyPythonMgr().allowDefaultImpl()
		# Put rivers on small islands
		mst.riverMaker.islandRivers()

def addLakes():
	print "-- addLakes()"
	if not mst.bMars:
		CyPythonMgr().allowDefaultImpl()

# This function will be called by the system, after the map was generated, after the
# starting-plots have been choosen, at the start of the normalizing process.
# You will only need this function, if you want to use the teamStart options.
# In this example we assume that the script has a custom-option for team starts with
# 4 options: 'nearby', 'separated', 'randomize', 'ignore'.
def normalizeStartingPlotLocations():
	print "-- normalizeStartingPlotLocations()"

	# build Lost Isle
	# - this region needs to be placed after starting-plots are first assigned
	mst.mapRegions.buildLostIsle( chance=33, minDist=7, bAliens=mst.choose(33,True,False) )

	if CyMap().getCustomMapOption(3) == 0:
		CyPythonMgr().allowDefaultImpl()							# by default civ places teams near to each other
		# mst.teamStart.placeTeamsTogether( True, True )	# use teamStart to place teams near to each other
	elif CyMap().getCustomMapOption(3) == 1:
		mst.teamStart.placeTeamsTogether( False, True )		# shuffle starting-plots to separate teams
	elif CyMap().getCustomMapOption(3) == 2:
		mst.teamStart.placeTeamsTogether( True, True )		# randomize starting-plots (may be near or not)
	else:
		mst.teamStart.placeTeamsTogether( False, False )	# leave starting-plots alone

def normalizeAddRiver():
	print "-- normalizeAddRiver()"
	if not mst.bMars:
		CyPythonMgr().allowDefaultImpl()

def normalizeAddLakes():
	print "-- normalizeAddLakes()"
	if not mst.bMars:
		CyPythonMgr().allowDefaultImpl()

# prevent terrain changes on Mars
def normalizeRemoveBadTerrain():
	print "-- normalizeRemoveBadTerrain()"
	if not mst.bMars:
		CyPythonMgr().allowDefaultImpl()

# prevent terrain changes on Mars
def normalizeAddGoodTerrain():
	print "-- normalizeAddGoodTerrain()"
	if not mst.bMars:
		CyPythonMgr().allowDefaultImpl()

def normalizeAddExtras():
	print "-- normalizeAddExtras()"
	# Prettify map - connect some small adjacent lakes
	mst.mapPrettifier.connectifyLakes()
	# Sprout rivers from lakes.
	mst.riverMaker.buildRiversFromLake( None, 50, 1, 2 )
	# transform coastal volcanos
	mst.mapPrettifier.beautifyVolcanos()
	# Balance boni, place missing boni, move minerals
	balancer.normalizeAddExtras()

#	normalizeAddExtras2()						# call renamed script function
	CyPythonMgr().allowDefaultImpl()

	# Give extras (names and boni) to special regions
	# FFH does a lot housekeeping, but some special regions may want to overrule that.
	mst.mapRegions.addRegionExtras()

	# Place special features on map
	mst.featurePlacer.placeKelp()
	mst.featurePlacer.placeHauntedLands()
	mst.featurePlacer.placeCrystalPlains()

	# Print maps and stats
	mst.mapPrint.buildRiverMap( False, "normalizeAddExtras()" )
	mst.mapStats.mapStatistics()

def minStartingDistanceModifier():
	if mst.bPfall: return -25
	return 0

def getWrapX():
	return ( CyMap().getCustomMapOption(4) in [1,3] )
def getWrapY():
	return ( CyMap().getCustomMapOption(4) in [2,3] )

################################################################


def isAdvancedMap():
	"This map should show up in simple mode"
	return 0


def getNumCustomMapOptions():
	return 4 + mst.iif( mst.bMars, 0, 1 )

def getNumHiddenCustomMapOptions():
	return mst.iif( mst.bMars, 0, 1 )

def getCustomMapOptionName(argsList):
	[iOption] = argsList
	option_names = {
		0:	"TXT_KEY_MAP_SCRIPT_CONTINENTS_SIZE",
		1:	"TXT_KEY_MAP_SCRIPT_ISLANDS_SIZE",
		2:	"TXT_KEY_MAP_SCRIPT_ISLAND_OVERLAP",
		3: "World Shape",
		4: "Team Start",
		}
	translated_text = unicode(CyTranslator().getText(option_names[iOption], ()))
	return translated_text

def getNumCustomMapOptionValues(argsList):
	[iOption] = argsList
	option_values = {
		0:	4,
		1:	2,
		2:	2,
		3:	4,
		4:	3
		}
	return option_values[iOption]

def getCustomMapOptionDescAt(argsList):
	[iOption, iSelection] = argsList
	selection_names = {
		0:	{
			0: "TXT_KEY_MAP_SCRIPT_NORMAL_CONTINENTS",
			1: "TXT_KEY_MAP_SCRIPT_UNPREDICTABLE",
			2: "TXT_KEY_MAP_SCRIPT_SNAKY_CONTINENTS",
			3: "TXT_KEY_MAP_SCRIPT_ISLANDS",
			},
		1:	{
			0: "TXT_KEY_MAP_SCRIPT_ISLANDS",
			1: "TXT_KEY_MAP_SCRIPT_TINY_ISLANDS"
			},
		2:	{
			0: "TXT_KEY_MAP_SCRIPT_ISLAND_REGION_SEPARATE",
			1: "TXT_KEY_MAP_SCRIPT_ISLANDS_MIXED_IN"
			},
		3:	{
			0: "Flat",
			1: "Cylinder (x-wrap)",
			2: "Tube (y-wrap)",
			3: "Toroid (x,y-wrap)"
			},
		4:	{
			0: "Team Neighbors",
			1: "Team Separated",
			2: "Randomly Placed"
			},
		}
	translated_text = unicode(CyTranslator().getText(selection_names[iOption][iSelection], ()))
	return translated_text

def getCustomMapOptionDefault(argsList):
	[iOption] = argsList
	option_defaults = {
		0:	0,
		1:	0,
		2:	1,
		3:	1,
		4:	0
		}
	return option_defaults[iOption]

def isRandomCustomMapOption(argsList):
	[iOption] = argsList
	option_random = {
		0:	true,
		1:	true,
		2:	true,
		3:	true,
		4:	false
		}
	return option_random[iOption]

def beforeGeneration2():
	global yShiftRoll1
	global yShiftRoll2
	gc = CyGlobalContext()
	dice = gc.getGame().getMapRand()

	# Binary shift roll (for horizontal shifting if Island Region Separate).
	yShiftRoll1 = dice.get(2, "West Region Shift, Vertical - Medium and Small PYTHON")
	print yShiftRoll1
	yShiftRoll2 = dice.get(2, "East Region Shift, Vertical - Medium and Small PYTHON")
	print yShiftRoll2

class MnSMultilayeredFractal(CvMapGeneratorUtil.MultilayeredFractal):
	def generatePlotsByRegion(self):
		# Sirian's MultilayeredFractal class, controlling function.
		# You -MUST- customize this function for each use of the class.
		global yShiftRoll1
		global yShiftRoll2
		if self.map.getCustomMapOption(0) != 4:
			iContinentsGrainWest = 1 + self.map.getCustomMapOption(0)
			iContinentsGrainEast = 1 + self.map.getCustomMapOption(0)
		else:
			iContinentsGrainWest = 1 + self.dice.get(4, "West Continent Grain - Medium and Small PYTHON")
			iContinentsGrainEast = 1 + self.dice.get(4, "East Continent Grain - Medium and Small PYTHON")
		iIslandsGrain = 4 + self.map.getCustomMapOption(1)
		userInputOverlap = self.map.getCustomMapOption(2)

		# Add a few random patches of Tiny Islands first.
		numTinies = 1 + self.dice.get(4, "Tiny Islands - Medium and Small PYTHON")
		print("Patches of Tiny Islands: ", numTinies)
		if numTinies:
			for tiny_loop in range(numTinies):
				tinyWestLon = 0.01 * self.dice.get(85, "Tiny Longitude - Medium and Small PYTHON")
				tinyWestX = int(self.iW * tinyWestLon)
				tinySouthLat = 0.01 * self.dice.get(85, "Tiny Latitude - Medium and Small PYTHON")
				tinySouthY = int(self.iH * tinyWestLon)
				tinyWidth = int(self.iW * 0.15)
				tinyHeight = int(self.iH * 0.15)

				self.generatePlotsInRegion(80,
				                           tinyWidth, tinyHeight,
				                           tinyWestX, tinySouthY,
				                           4, 3,
				                           0, self.iTerrainFlags,
				                           6, 5,
				                           True, 3,
				                           -1, False,
				                           False
				                           )

		# Add the contental regions.

		# Add the Western Continent(s).
		iSouthY = 0
		iNorthY = self.iH - 1
		iHeight = iNorthY - iSouthY + 1
		iWestX = int(self.iW / 20)
		iEastX = int(self.iW * 0.45) - 1
		iWidth = iEastX - iWestX + 1
		print("Cont West: ", iWestX, "Cont East: ", iEastX, "Cont Width: ", iWidth)

		# Vertical dimensions may be affected by overlap and/or shift.
		if userInputOverlap: # Then both layers fill the entire region and overlap each other.
			# The north and south boundaries are already set (to max values).
			# Set Y exponent:
			yExp = 5
			# Also need to reduce amount of land plots, since there will be two layers in all areas.
			iWater = 80
		else: # The regions are separate, with continents only in one part, islands only in the other.
			iWater = 74
			# Set X exponent to square setting:
			yExp = 6
			# Handle horizontal shift for the Continents layer.
			# (This will choose one side or the other for this region then fit it properly in its space).
			if yShiftRoll1:
				southShift = int(0.4 * self.iH)
				northShift = 0
			else:
				southShift = 0
				northShift = int(0.4 * self.iH)

			iSouthY += southShift
			iNorthY -= northShift
			iHeight = iNorthY - iSouthY + 1
		print("Cont South: ", iSouthY, "Cont North: ", iNorthY, "Cont Height: ", iHeight)

		self.generatePlotsInRegion(iWater,
		                           iWidth, iHeight,
		                           iWestX, iSouthY,
		                           iContinentsGrainWest, 4,
		                           self.iRoundFlags, self.iTerrainFlags,
		                           6, yExp,
		                           True, 15,
		                           -1, False,
		                           False
		                           )

		# Add the Western Islands.
		iWestX = int(self.iW * 0.12)
		iEastX = int(self.iW * 0.38) - 1
		iWidth = iEastX - iWestX + 1
		iSouthY = 0
		iNorthY = self.iH - 1
		iHeight = iNorthY - iSouthY + 1

		# Vertical dimensions may be affected by overlap and/or shift.
		if userInputOverlap: # Then both layers fill the entire region and overlap each other.
			# The north and south boundaries are already set (to max values).
			# Set Y exponent:
			yExp = 5
			# Also need to reduce amount of land plots, since there will be two layers in all areas.
			iWater = 80
		else: # The regions are separate, with continents only in one part, islands only in the other.
			iWater = 74
			# Set X exponent to square setting:
			yExp = 6
			# Handle horizontal shift for the Continents layer.
			# (This will choose one side or the other for this region then fit it properly in its space).
			if yShiftRoll1:
				southShift = 0
				northShift = int(0.4 * self.iH)
			else:
				southShift = int(0.4 * self.iH)
				northShift = 0

			iSouthY += southShift
			iNorthY -= northShift
			iHeight = iNorthY - iSouthY + 1
		print("Cont South: ", iSouthY, "Cont North: ", iNorthY, "Cont Height: ", iHeight)


		self.generatePlotsInRegion(iWater,
		                           iWidth, iHeight,
		                           iWestX, iSouthY,
		                           iIslandsGrain, 5,
		                           self.iRoundFlags, self.iTerrainFlags,
		                           6, yExp,
		                           True, 15,
		                           -1, False,
		                           False
		                           )

		# Add the Eastern Continent(s).
		iSouthY = 0
		iNorthY = self.iH - 1
		iHeight = iNorthY - iSouthY + 1
		iWestX = int(self.iW * 0.55)
		iEastX = int(self.iW * 0.95) - 1
		iWidth = iEastX - iWestX + 1
		print("Cont West: ", iWestX, "Cont East: ", iEastX, "Cont Width: ", iWidth)

		# Vertical dimensions may be affected by overlap and/or shift.
		if userInputOverlap: # Then both layers fill the entire region and overlap each other.
			# The north and south boundaries are already set (to max values).
			# Set Y exponent:
			yExp = 5
			# Also need to reduce amount of land plots, since there will be two layers in all areas.
			iWater = 80
		else: # The regions are separate, with continents only in one part, islands only in the other.
			iWater = 74
			# Set X exponent to square setting:
			yExp = 6
			# Handle horizontal shift for the Continents layer.
			# (This will choose one side or the other for this region then fit it properly in its space).
			if yShiftRoll2:
				southShift = int(0.4 * self.iH)
				northShift = 0
			else:
				southShift = 0
				northShift = int(0.4 * self.iH)

			iSouthY += southShift
			iNorthY -= northShift
			iHeight = iNorthY - iSouthY + 1
		print("Cont South: ", iSouthY, "Cont North: ", iNorthY, "Cont Height: ", iHeight)

		self.generatePlotsInRegion(iWater,
		                           iWidth, iHeight,
		                           iWestX, iSouthY,
		                           iContinentsGrainWest, 4,
		                           self.iRoundFlags, self.iTerrainFlags,
		                           6, yExp,
		                           True, 15,
		                           -1, False,
		                           False
		                           )

		# Add the Eastern Islands.
		iWestX = int(self.iW * 0.62)
		iEastX = int(self.iW * 0.88) - 1
		iWidth = iEastX - iWestX + 1
		iSouthY = 0
		iNorthY = self.iH - 1
		iHeight = iNorthY - iSouthY + 1

		# Vertical dimensions may be affected by overlap and/or shift.
		if userInputOverlap: # Then both layers fill the entire region and overlap each other.
			# The north and south boundaries are already set (to max values).
			# Set Y exponent:
			yExp = 5
			# Also need to reduce amount of land plots, since there will be two layers in all areas.
			iWater = 80
		else: # The regions are separate, with continents only in one part, islands only in the other.
			iWater = 74
			# Set X exponent to square setting:
			yExp = 6
			# Handle horizontal shift for the Continents layer.
			# (This will choose one side or the other for this region then fit it properly in its space).
			if yShiftRoll2:
				southShift = 0
				northShift = int(0.4 * self.iH)
			else:
				southShift = int(0.4 * self.iH)
				northShift = 0

			iSouthY += southShift
			iNorthY -= northShift
			iHeight = iNorthY - iSouthY + 1
		print("Cont South: ", iSouthY, "Cont North: ", iNorthY, "Cont Height: ", iHeight)


		self.generatePlotsInRegion(iWater,
		                           iWidth, iHeight,
		                           iWestX, iSouthY,
		                           iIslandsGrain, 5,
		                           self.iRoundFlags, self.iTerrainFlags,
		                           6, yExp,
		                           True, 15,
		                           -1, False,
		                           False
		                           )

		# All regions have been processed. Plot Type generation completed.
		print "Done"
		return self.wholeworldPlotTypes

'''
Regional Variables Key:

iWaterPercent,
iRegionWidth, iRegionHeight,
iRegionWestX, iRegionSouthY,
iRegionGrain, iRegionHillsGrain,
iRegionPlotFlags, iRegionTerrainFlags,
iRegionFracXExp, iRegionFracYExp,
bShift, iStrip,
rift_grain, has_center_rift,
invert_heights
'''

def generatePlotTypes():
	NiTextOut("Setting Plot Types (Python PF Medium and Small) ...")
	fractal_world = MnSMultilayeredFractal()
	plotTypes = fractal_world.generatePlotsByRegion()
	return plotTypes

def addFeatures():
	NiTextOut("Adding Features (Python PF Medium and Small) ...")
	featuregen = mst.MST_FeatureGenerator()		# Temudjin
	featuregen.addFeatures()
	return 0
