#
#	FILE:	 Ringworld3.py  v1.00
#  ===========================
#  NEEDS MapScriptTools
#
#	AUTHOR:  Temudjin (2008-10)
#   based on Ring World by Ruff_Hi
#		- Civilization Fanatics (http://forums.civfanatics.com/showthread.php?t=274181)
#
#   ###########################################################################################
#	 this map is also fully compatible with 'Planetfall' by Maniac
#		- Civilization Fanatics (http://forums.civfanatics.com/showthread.php?t=252829)
#		>>>>> first put MapScriptTools.py into the ..\Beyond the Sword\Assets\Python folder <<<<<
#		>>>>> then put this file into the PrivateMaps folder of that mod and select it      <<<<<
#   ###########################################################################################


#	PURPOSE: Civ4 Map Script
#  ========================
#
#  Ring World was initially done by Ruff_hi, I just ran away with it and morphed it first into
#  Ringworld2 and now into Ringworld3, where there really isn't much left from the original except
#  for the general shape. My goal was to learn about, adjust, change, expand, and hopfully enhance
#  the map by building it less extreme and somewhat more 'realistic'.
#
#	The user has five custom options:
#
#	Team Start
#	Choose whether your team-mates start in the neighborhood, far away or somewhere random
#  	*Team Neighbors
#  	 Team Separated
#		 Randomly Placed
#
#	Region Strength
#	There are 4 partly overlapping regions in the map: Hills - Flat, Grass - Desert
#	The user can designate how strongly regional the map is.
#		 Zero
#		 Weak
#		*Moderate
#		 Strong
#
#	Balancing Boni
#	To balance starting positions, you can choose to place one of each from a list
#	of mod-depending strategic boni into the vicinity of each starting-plot.
#		 Normal
#		*Balanced
#
#	Pole Position
#	Choose if the equator is in the middle, north or south of the map
#		*NorthPole to SouthPole
#		 Equator to SouthPole
#		 NorthPole to Equator
#		 No Poles
#
#	World Shape
#		 Solid Ring - One strip of continuous land, possibly divided by small channels ( ~ 55% Water )
#		*Continents - Several continents and a few islands with some ocean in between ( ~ 60% Water )
#		 Islands	  - Several islands and perhaps a couple of small continents ( ~ 70% Water )
#
#  This script constructs a long skinny map that is only 16 tiles high.
#  Mostly the top and bottom 4 tiles (with Islands 3 tiles) of the map are water.
#  The other tiles form a circle of land surface with some oceanic breaks according to the user selection
#  The width of the map depends upon the size selected by the user.
#
#  A Ringworld of course is supposed to be a fairly big world. Area enough for a whole lot of civs.
#    On 'Islands' though, especially with high sealevel, there's not much room for many civs.
#	  It's an artificial world built using random, not fractal, patterns.
#    This also means that weather and terrain patterns are sometimes not very logical.
#  The map has no restrictions on the number of possible civs, that number depends only on the mod.
#  There are few Mountains. (more for Planetfall)
#	Mountains on Ringworld are 'reverse craters' from incomming space debris hitting the other side.
#	On 'Continents' a few of those hits caused the breaking up of the landmass into several big chunks.
#	On 'Islands' even more hits have broken most of those continents and deformed the the whole structure.
#
# HINTS
#  Use either the FlavourMod import from line 1, or the 'Flavour Start' option from 'Fall Further' or 'Orbis'
#  Cold climate or using different pole positions, will probably make it impossible to sail around the world.
#  Usually you have only two neighbors; instruct your diplomats to act accordingly!
#	In Planetfall you really don't want to use 'Scattered Pods' with maps bigger than 'Standard'.

# NOTES
#  Fall from Heaven has difficulties to correctly display the grid near the poles, which is a problem for
#    'Ringworld3', since most of the tiles are near the poles. It doesn't seem to affect gameplay though.
#    To correct this, copy 'OceanGRIDS.dds' into 'Mods\Fall from Heaven 2\Assets\Art\Terrain\Textures'
#    - by OnmyojiOmn --> Civilization Fanatics (http://forums.civfanatics.com/showthread.php?t=303257)

#  ===================
#  Temudjin 15.July.10
#  ===================

#	Changelog
#	---------
#	1.00					initial release ( based on Ringworld2 )
#



def getVersion():
	return "1.00"

def getDescription():
	return "\nBased on Larry Niven's famous 'Ringworld'\nA long ring - 16 tiles high"


from CvPythonExtensions import *
from math import *
import CvMapGeneratorUtil


################################################################################
## MapScriptTools Interface by Temudjin
################################################################################
import MapScriptTools as mst
balancer = mst.bonusBalancer

"""
----------------------------------------------------------------
The Map Building Process according to Temudjin
----------------------------------------------------------------
0)     - Get Map-Options
0.1)     getNumHiddenCustomMapOptions()
0.2)     getNumCustomMapOptions()
0.3)     getCustomMapOptionDefault()
0.4)     isAdvancedMap()
0.5)     getCustomMapOptionName()
0.6)     getNumCustomMapOptionValues()
0.7)     isRandomCustomMapOption()
0.8)     getCustomMapOptionDescAt()
0.9)     - Get Map-Types
0.9.1)     isClimateMap()
0.9.2)     isSeaLevelMap()
1)     beforeInit()
2)     - Initialize Map
2.2)     getGridSize()
2.3.1)   getTopLatitude()					# always use both
2.3.2)   getBottomLatitude()				# always use both
2.4.1)   getWrapX()							# always use both
2.4.2)   getWrapY()							# always use both
3)     beforeGeneration()
4)     - Generate Map
4.1)     generatePlotTypes()
4.2)     generateTerrainTypes()
4.3)     addRivers()
4.4)     addLakes()
4.5)     addFeatures()
4.6)     addBonuses()
4.6.1)     isBonusIgnoreLatitude()*
4.7)     addGoodies()
5)     afterGeneration()
6)     - Select Starting-Plots
6.1)     minStartingDistanceModifier()
6.2)     assignStartingPlots()
7)     - Normalize Starting-Plots
7.1)     normalizeStartingPlotLocations()
7.2)     normalizeAddRiver()
7.3)     normalizeRemovePeaks()
7.4)     normalizeAddLakes()
7.5)     normalizeRemoveBadFeatures()
7.6)     normalizeRemoveBadTerrain()
7.7)     normalizeAddFoodBonuses()
7.7.1)     isBonusIgnoreLatitude()*
7.8)     normalizeGoodTerrain()
7.9)     normalizeAddExtras()
7.9.1)     isBonusIgnoreLatitude()*
8 )    startHumansOnSameTile()

* used by default 'CyPythonMgr().allowDefaultImpl()' in:
  addBonuses(), normalizeAddFoodBonuses(), normalizeAddExtras()
------------------------------------------------------------------
"""

# #######################################################################################
# ######## beforeGeneration() - Called from system after 'Initialize Map'
# ######## - define your latitude formula, get the map-version
# ######## - initialize the MapScriptTools and initialize MapScriptTools.BonusBalancer
# #######################################################################################
def beforeGeneration():
	print "[RingW] -- beforeGeneration()"
	# initialize Ringworld
	getRingworldParameter()								# only for Ringworld
	# create function for mst.evalLatitude;
	if mapOptions.Polar == 0:
		compGetLat = "int(round([0.95,0.80,0.64,0.60,0.52,0.40,0.24,0.04,0.04,0.24,0.40,0.52,0.60,0.64,0.80,0.95][y]*80))"
	elif mapOptions.Polar == 1:
		compGetLat = "int(round([0.98,0.86,0.75,0.65,0.56,0.48,0.41,0.34,0.27,0.21,0.16,0.12,0.09,0.06,0.03,0.00][y]*80))"
	elif mapOptions.Polar == 2:
		compGetLat = "int(round([0.00,0.03,0.06,0.09,0.12,0.16,0.21,0.27,0.34,0.41,0.48,0.56,0.65,0.75,0.86,0.98][y]*80))"
	elif mapOptions.Polar == 3:
		compGetLat = "int(round([0.70,0.57,0.47,0.37,0.28,0.20,0.13,0.08,0.03,0.00,0.03,0.08,0.13,0.20,0.28,0.37][y]*90))"
	# Create mapInfo string
	mapInfo = ""
	for opt in range( getNumCustomMapOptions() ):
		nam = getCustomMapOptionName( [opt] )
		sel = CyMap().getCustomMapOption( opt )
		txt = getCustomMapOptionDescAt( [opt,sel] )
		mapInfo += "%27s:   %s\n" % ( nam, txt )
	# Initialize MapScriptTools
	mst.getModInfo( getVersion(), compGetLat, mapInfo )
	# Initialize MapScriptTools.BonusBalancer
	balancer.initialize( True, True, False )		#	balance boni, place missing boni

# #######################################################################################
# ######## generateTerrainTypes() - Called from system
# ######## - SECOND STAGE in 'Generate Map'
# ######## - creates an array of terrains (desert,plains,grass,...) in the map dimensions
# #######################################################################################
def generateTerrainTypes():
	print "[RingW] -- generateTerrainTypes()"

	# build ringworld (part 2)
	ringWorld.buildLandscape()
	# Planetfall: more highlands and ridges
	mst.planetFallMap.buildPfallHighlands()
	# Prettify map: Connect small islands
	mst.mapPrettifier.bulkifyIslands( 75, 4 )
	if mapOptions.WorldShape == 2:						# bulkify some more for 'Islands'
		mst.mapPrettifier.bulkifyIslands( 75, 6 )
	# Prettify map: Change coastal peaks to hills
	mst.mapPrettifier.hillifyCoast( 60 )

	# If the active mod is 'Planetfall', we use a different terrainGenerator.
	if mst.bPfall:
		terrainGen = PlanetfallTerrainGenerator()		# Ringworld has it's own Planetfall TerrainGenerator
	elif mst.bMars:
		terrainGen = mst.MST_TerrainGenerator()		# 'Mars Now!' uses the default TerrainGenerator
	else:
		terrainGen = RingworldTerrainGenerator()		# Ringworld uses it's own TerrainGenerator

	# Regardless of the mod, create the terrain and return the result.
	terrainTypes = terrainGen.generateTerrain()
	return terrainTypes

# #######################################################################################
# ######## addRivers() - Called from system
# ######## - THIRD STAGE in 'Generate Map'
# ######## - puts rivers on the map
# #######################################################################################
def addRivers():
	print "[RingW] -- addRivers()"

	# kill off more spurious lakes in non pole to pole maps
	ringWorld.adjustLakes()
	# Generate DeepOcean-terrain if mod allows for it
	mst.deepOcean.buildDeepOcean( dist=3 )
	# Generate marsh-terrain
	mst.marshMaker.convertTerrain()
	# Solidify marsh before building bogs
	if not mst.bPfall:
		# change tundra singletons #1
		ringWorld.changeTerrainOrphans( (mst.etTundra,mst.etGrass,90), (mst.etTundra,mst.etPlains,90) )
		if mst.bMarsh:
			marshPer = 6 - getAridity()
			mst.mapPrettifier.percentifyTerrain( (mst.etMarsh,marshPer), (mst.etTundra,2), (mst.etGrass,3) )
	# Build between 0..3 mountain-ranges.
	mst.mapRegions.buildBigDents()
	# Build between 0..3 bog-regions.
	mst.mapRegions.buildBigBogs()
	# Solidify desert
	if not mst.bPfall:
		desertPer = 12 + getAridity()
		mst.mapPrettifier.percentifyTerrain( (mst.etDesert,desertPer), (mst.etPlains,5), (mst.etGrass,2) )
		# create connected deserts and plains
		mst.mapPrettifier.lumpifyTerrain( mst.etDesert, mst.etPlains, mst.etGrass )
		mst.mapPrettifier.lumpifyTerrain( mst.etGrass, mst.etTundra, mst.etPlains )
		mst.mapPrettifier.lumpifyTerrain( mst.etPlains, mst.etDesert, mst.etGrass )
		# change desert singletons
		ringWorld.changeTerrainOrphans( (mst.etDesert,mst.etPlains,80), (mst.etSnow,mst.etTundra,99) )
		# change tundra singletons #2
		ringWorld.changeTerrainOrphans( (mst.etTundra,mst.etGrass,90), (mst.etTundra,mst.etPlains,90) )
	# build mountains on edge
	ringWorld.worldEdgeLand()
	# access mountain valleys
	ringWorld.openMountains()

	# No rivers for 'Mars Now!'
	if not mst.bMars:
		# Kill of spurious lakes
		mst.mapPrettifier.connectifyLakes( 90 )
		# Sprout rivers from lakes
#		mst.mapPrint.buildRiverMap( True, "addRivers_1()" )
		mst.riverMaker.buildRiversFromLake( None, 20-2*getAridity(), 4, 2 )		# up to 4 rivers per lake>=2
#		mst.mapPrint.buildRiverMap( True, "addRivers_2()" )

		# Put rivers on the map.
#		CyPythonMgr().allowDefaultImpl()
		# Put rivers on small islands
		mst.riverMaker.islandRivers()

# #######################################################################################
# ######## addLakes() - Called from system
# ######## - FOURTH STAGE in 'Generate Map'
# ######## - puts lakes on the map
# #######################################################################################
# Ringworld3 already has enough lakes
def addLakes():
	print "[RingW] -- addLakes()"
	return

# #######################################################################################
# ######## addFeatures() - Called from system after addLakes()
# ######## - FIFTH STAGE in 'Generate Map'
# ######## - puts features on the map
# #######################################################################################
def addFeatures():
	print "[RingW] -- addFeatures()"

	featuregen = mst.MST_FeatureGenerator()		# standard feature generator with Ringworld-Latitudes
	featuregen.addFeatures()

#	mst.mapPrint.buildAreaMap( True, "addFeatures" )
#	mst.mapPrint.buildPlotMap( True, "addFeatures" )
#	mst.mapPrint.buildTerrainMap( True, "addFeatures" )
#	mst.mapPrint.buildFeatureMap( True, "addFeatures" )
	planetFall.reduceSeaFungus()						# reduce sea-fungus					- Planetfall
	mst.planetFallMap.buildPfallOcean(40, 55)		# handle shelves and trenches		- Planetfall

	ringWorld.desertBloom()								# more oasis/floodplains			- not Planetfall
	ringWorld.buildScrub()								# make scrub if allowed				- not Planetfall

	ringWorld.adjustIceCaps()							# adjust Ice-caps						- all
	mst.mapPrettifier.beautifyVolcanos()			# transform some volcanos			- all

	mst.mapRegions.buildElementalQuarter( 66 )	# build ElementalQuarter			- FFH only

# ######################################################################################################
# ######## assignStartingPlots() - Called from system
# ######## - assign starting positions for each player after the map is generated
# ######## - Planetfall has GameOption 'SCATTERED_LANDING_PODS' - use default implementation
# ######################################################################################################
def assignStartingPlots():
	print "[RingW] -- assignStartingPlots()"
	if mst.bPfall:
		CyPythonMgr().allowDefaultImpl()
	else:
		ringworldAssignStartingPlots()

# ############################################################################################
# ######## normalizeStartingPlotLocations() - Called from system after starting-plot selection
# ######## - FIRST STAGE in 'Normalize Starting-Plots'
# ######## - change assignments to starting-plots
# ############################################################################################
def normalizeStartingPlotLocations():
	print "[RingW] -- normalizeStartingPlotLocations()"

	# build Lost Isle
	# - this region needs to be placed after starting-plots are first assigned
	if mapOptions.WorldShape > 0:
		mst.mapRegions.buildLostIsle( 66, minDist=5, bAliens=True )

	# shuffle starting-plots for teams
	if mapOptions.Teams == 0:
		CyPythonMgr().allowDefaultImpl()						# by default civ places teams near to each other
	elif mapOptions.Teams == 1:
		mst.teamStart.placeTeamsTogether( False, True )	# shuffle starting-plots to separate teams
	else:
		mst.teamStart.placeTeamsTogether( True, True )	# randomize starting-plots

# ############################################################################################
# ######## normalizeAddRiver() - Called from system after normalizeStartingPlotLocations()
# ######## - SECOND STAGE in 'Normalize Starting-Plots'
# ######## - add some rivers if needed
# ############################################################################################
def normalizeAddRiver():
	print "[RingW] -- normalizeAddRiver()"
	if not mst.bMars:
		CyPythonMgr().allowDefaultImpl()

#def normalizeRemovePeaks():
#	return None

# ############################################################################################
# ######## normalizeAddLakesRiver() - Called from system after normalizeRemovePeaks()
# ######## - FOURTH STAGE in 'Normalize Starting-Plots'
# ######## - add some lakes if needed
# ############################################################################################
def normalizeAddLakes():
	print "[RingW] -- normalizeAddLakes()"
	if not mst.bMars:
		CyPythonMgr().allowDefaultImpl()

#def normalizeRemoveBadFeatures():
#	return None

# ############################################################################################
# ######## normalizeRemoveBadTerrain() - Called from system after normalizeRemoveBadFeatures()
# ######## - SIXTH STAGE in 'Normalize Starting-Plots'
# ######## - change bad terrain if needed
# ############################################################################################
def normalizeRemoveBadTerrain():
	print "-- normalizeRemoveBadTerrain()"
	if not mst.bMars:
		CyPythonMgr().allowDefaultImpl()

#def normalizeAddFoodBonuses():
#	return None

# ############################################################################################
# ######## normalizeAddGoodTerrain() - Called from system after normalizeAddFoodBonuses()
# ######## - EIGHTH STAGE in 'Normalize Starting-Plots'
# ######## - add good terrain if needed
# ############################################################################################
def normalizeAddGoodTerrain():
	print "-- normalizeAddGoodTerrain()"
	if not mst.bMars:
		CyPythonMgr().allowDefaultImpl()

# ############################################################################################
# ######## normalizeAddExtras() - Called from system after normalizeAddGoodTerrain()
# ######## - NINTH and LAST STAGE in 'Normalize Starting-Plots'
# ######## - last chance to adjust starting-plots
# ######## - called before startHumansOnSameTile(), which is the last map-function so called
# ############################################################################################
def normalizeAddExtras():
	print "[RingW] -- normalizeAddExtras()"

	# Balance boni, place missing boni and move minerals
	balancer.normalizeAddExtras()
	# use standard mineral-list, and create hills if none are near
	balancer.moveMinerals( None, True )

	# Do the default housekeeping
	CyPythonMgr().allowDefaultImpl()

	# Make sure marshes are on flatlands
	mst.marshMaker.normalizeMarshes()

	# Give extras to special regions
	mst.mapRegions.addRegionExtras()

	# Place special features on map
	mst.featurePlacer.placeKelp()
	mst.featurePlacer.placeHauntedLands()
	mst.featurePlacer.placeCrystalPlains()

	# Kill ice on warm edges
	mst.mapPrettifier.deIcifyEdges()

	# Print maps and stats
	mst.mapPrint.buildAreaMap( True, "normalizeAddExtras()_1" )
#	mst.mapPrint.buildPlotMap( True, "normalizeAddExtras()_1" )
#	mst.mapPrint.buildTerrainMap( True, "normalizeAddExtras()" )
#	mst.mapPrint.buildFeatureMap( True, "normalizeAddExtras()" )
#	mst.mapPrint.buildBonusMap( False, "normalizeAddExtras()" )
	if mst.bFFH: mst.mapPrint.buildBonusMap( False, "Mana", None, mst.mapPrint.manaDict )
	mst.mapPrint.buildRiverMap( True, "normalizeAddExtras()" )
	stats = mst.mapStats.mapStatistics()

	# Save game-options
	mapOptions.saveGameOptions()
	mapOptions.showGameOptions()

# ############################################################################
# ######## minStartingDistanceModifier() - Called from system at various times
# ######## - FIRST STAGE in 'Select Starting-Plots'
# ######## - adjust starting-plot distances
# ############################################################################
def minStartingDistanceModifier():
#	print "[RingW] -- minStartingDistanceModifier()"
	if mst.bPfall: return -25
	return 20
################################################################################


################################################################
### Custom Map Options
################################################################

def getNumHiddenCustomMapOptions():
	""" Default is used for the last n custom-options in 'Play Now' mode. """
	print "[RingW] -- getNumHiddenCustomMapOptions()"

	#----------------------------------------------------
	# read game-options (this is the earliest opportunity)
	if len(mapOptions.mapDesc.keys()) > 0:
		mapOptions.reloadGameOptions()
		mapOptions.showGameOptions()
	#----------------------------------------------------

	return mst.iif( mst.bMars, 0, 1 )

def getNumCustomMapOptions():
#	print "[RingW] -- getNumCustomMapOptions()"
	return 4 + mst.iif( mst.bMars, 0, 1 )

def getCustomMapOptionName(argsList):
#	print "[RingW] -- getCustomMapOptionName()"
	[iOption] = argsList
	option_names =	{
						0:	"Balancing Boni",
						1:	"Region Strength",
						2:	"Pole Position",
						3:	"World Shape",
						4: "Team Start",
						}
	translated_text = unicode(CyTranslator().getText(option_names[iOption], ()))
	return translated_text

def getNumCustomMapOptionValues(argsList):
#	print "[RingW] -- getNumCustomMapOptionValues()"
	[iOption] = argsList
	option_values =	{
							0: 2,
							1:	4,
							2: 4,
							3: 3,
							4:	3,
							}
	return option_values[iOption]

def getCustomMapOptionDescAt(argsList):
#	print "[RingW] -- getCustomMapOptionDescAt()"
	[iOption, iSelection] = argsList

	selection_names =	{
							0:	{
								0: "Normal",
								1: "Balanced"
								},
							1:	{
								0: "None",
								1: "Weak",
								2: "Moderate",
								3: "Strong"
								},
							2:	{
								0: "NorthPole to SouthPole",
								1: "Equator to SouthPole",
								2: "NorthPole to Equator",
								3: "No Poles"
								},
							3:	{
								0: "Solid Ring",
								1: "Continents",
								2: "Islands"
								},
							4:	{
								0: "Team Neighbors",
								1: "Team Separated",
								2: "Randomly Placed"
								},
							}

	translated_text = unicode(CyTranslator().getText(selection_names[iOption][iSelection], ()))
	return translated_text

# reload Ringworld UserSettings from last session or default
def getCustomMapOptionDefault(argsList):
#	print "[RingW] -- getCustomMapOptionDefault()"
	[iOption] = argsList
	option_defaults = {
							0: mapOptions.Balanced,
							1:	mapOptions.RegionStrength,
							2: mapOptions.Polar,
							3: mapOptions.WorldShape,
							4:	mapOptions.Teams,
							}
	return option_defaults[iOption]

def isRandomCustomMapOption(argsList):
#	print "[RingW] -- isRandomCustomMapOption()"
	[iOption] = argsList
	option_random =	{
							0: False,
							1: False,
							2: True,
							3: True,
							4: False,
							}
	return option_random[iOption]

# reload GameOptions, if they were already saved by last session
def isAdvancedMap():
#	print "[RingW] -- isAdvancedMap()"
	"This map should show up in simple mode"
	return 0

################################################################

def isSeaLevelMap():
	return 1
def isClimateMap():
	return 1

def getWrapX():
	return True
def getWrapY():
	return False

def getTopLatitude():
	if CyMap().getCustomMapOption(2)==1: return 0			# south to equator
	return 80
def getBottomLatitude():
	if CyMap().getCustomMapOption(2)==2: return 0			# north to equator
	return -80

def getGridSize(argsList):
	print "[RingW] -- getGridSize()"
	gc = CyGlobalContext()
	if (argsList[0] == -1): # (-1,) is passed to function on loads
		return []

	grid_sizes = {	WorldSizeTypes.WORLDSIZE_DUEL:		(  8,4),
						WorldSizeTypes.WORLDSIZE_TINY:		( 20,4),
						WorldSizeTypes.WORLDSIZE_SMALL:		( 36,4),
						WorldSizeTypes.WORLDSIZE_STANDARD:	( 52,4),
						WorldSizeTypes.WORLDSIZE_LARGE:		( 68,4),
						WorldSizeTypes.WORLDSIZE_HUGE:		( 80,4)
	}

########## Temudjin START
	bigGrids = [ (92,4), (104,4) ]							# just change these, or even add some more
	maxWorld = CyGlobalContext().getNumWorldInfos()
	maxSize = min( 6 + len( bigGrids ), maxWorld )
	for i in range( 6, maxSize ):
		grid_sizes[ i ] = bigGrids[ i - 6 ]
########## Temudjin END

	[eWorldSize] = argsList
	return grid_sizes[eWorldSize]


# ###########################################################################################
# ######## getRingworldParameter()
# ######## - initialises region and map_menu globals, to be called before generatePlotTypes()
# ###########################################################################################
def getRingworldParameter():
	global xHills, xDesert, xGrass, dxRegion
	global iClimate, sClimate, iSeaLevel, sSeaLevel, iWorldSize, sWorldSize
	global iWorldShape, sWorldShape, iRegionStrength, sRegionStrength
	global iPolePosition, sPolePosition

	###########################
	### Ringworld constants
	###########################
	global gc, map, game, dice
	global iNumPlotsX, iNumPlotsY
	gc   = CyGlobalContext()
	map  = CyMap()
	game = CyGame()
	dice = gc.getGame().getMapRand()
	# MapSize
	iNumPlotsX = map.getGridWidth()
	iNumPlotsY = map.getGridHeight()

	# Map Climate Selection
	aClimate = []
	aClimate.append("Temperate")
	aClimate.append("Tropical")
	aClimate.append("Arid")
	aClimate.append("Rocky")
	aClimate.append("Cold")
	aClimate.append("? Other ?")
	iClimate = map.getClimate()
	if iClimate>4:
		sClimate = aClimate[5]
	else:
		sClimate = aClimate[iClimate]

	# Map Sealevel Selection
	aSeaLevel = []
	aSeaLevel.append("Low")
	aSeaLevel.append("Medium")
	aSeaLevel.append("High")
	aSeaLevel.append("? Other ?")
	iSeaLevel = map.getSeaLevel()
	if iSeaLevel>2:
		sSeaLevel = aSeaLevel[3]
	else:
		sSeaLevel = aSeaLevel[iSeaLevel]

	# Map Worldsize Selection
	aWorldSize = []
	aWorldSize.append("Duel")
	aWorldSize.append("Tiny")
	aWorldSize.append("Small")
	aWorldSize.append("Standard")
	aWorldSize.append("Large")
	aWorldSize.append("Huge")
	aWorldSize.append("Giant")
	aWorldSize.append("? Other ?")
	iWorldSize = map.getWorldSize()
	if iWorldSize>6:
		sWorldSize = aWorldSize[7]
	else:
		sWorldSize = aWorldSize[iWorldSize]

	# Map WorldShape Selection
	aWorldShape = []
	aWorldShape.append("Solid Ring")
	aWorldShape.append("Continents")
	aWorldShape.append("Islands")
	aWorldShape.append("? Other ?")
	iWorldShape = map.getCustomMapOption(3)
	if iWorldShape>2:
		sWorldShape = aWorldShape[3]
	else:
		sWorldShape = aWorldShape[iWorldShape]

	iPolePosition = map.getCustomMapOption(2)
	aPolePosition = [ "NorthPole to SouthPole", "Equator to SouthPole", "NorthPole to Equator", "No Poles" ]
	sPolePosition = aPolePosition[iPolePosition]

	# Map RegionStrength Selection
	aRegionStrength = []
	aRegionStrength.append("None")
	aRegionStrength.append("Weak")
	aRegionStrength.append("Moderate")
	aRegionStrength.append("Strong")
	aRegionStrength.append("? Other ?")
	iRegionStrength = map.getCustomMapOption(1)
	if iRegionStrength>3:
		sRegionStrength = aRegionStrength[4]
	else:
		sRegionStrength = aRegionStrength[iRegionStrength]

	sBalancingBoni = iif( map.getCustomMapOption(0)==0, "Normal", "Balanced" )
	sTeamStart = iif( map.getCustomMapOption(4)==0, "Neighbors", \
					 iif( map.getCustomMapOption(4)==1, "Separated", "Randomly Placed" ) )

	dxRegion = 0.40 + 0.1 * iRegionStrength			  # reach of Region: more strength -> more pronounced
	xHills = mst.chooseNumber( iNumPlotsX/2 )
	iRand = mst.chooseNumber( 4 )
	xDesert = (iRand * 2 + 1) * (iNumPlotsX/8) + xHills
	xDesert = xDesert % iNumPlotsX
	xGrass = xDesert + iNumPlotsX/2
	xGrass = xGrass % iNumPlotsX

	sprint  = "[RingW]   " + "="*45 + " Ringworld3 " + getVersion() + " ===== \n"
	sprint += "[RingW]   Map Options:    WorldShape: %s, RegionStrength: %s \n" % (sWorldShape, sRegionStrength)
	sprint += "[RingW]                   BalancingBoni: %s, TeamStart: %s \n" % (sBalancingBoni, sTeamStart)
	sprint += "[RingW]                   PolePosition: %s \n\n" % (sPolePosition)
	sprint += "[RingW]   Region Centers: Hills1, Hills2: %i, %i  -  Arid, Wet: %i, %i\n" % (xHills, xHills+iNumPlotsX/2, xDesert, xGrass)
	sprint += "[RingW]   " + "="*67
	print sprint

	mapOptions.Balanced		  = map.getCustomMapOption(0)				# remember between sessions
	mapOptions.RegionStrength = map.getCustomMapOption(1)				# remember between sessions
	mapOptions.Polar			  = map.getCustomMapOption(2)				# remember between sessions
	mapOptions.WorldShape	  = map.getCustomMapOption(3)				# remember between sessions
	mapOptions.Teams			  = map.getCustomMapOption(4)				# remember between sessions


########################################################################
##### Class RingWorld - functions used to build Ringworld3
########################################################################
# -- after generatePlotTypes
# buildLandscape()
# buildHills()
# buildHighlands()
# worldEdge()
# -- after generateTerrainTypes
# adjustLakes()
# changeTerrainOrphans( terList )
# -- after addRivers
# -- after addLakes
# openMountains()
# -- after addFeatures
# desertBloom()
# buildScrub()
# adjustIceCaps()
########################################################################
class RingWorld:

	# build ringworld (part 2)
	def buildLandscape( self ):
		print "[RingW] -- RingWorld.buildLandscape()"
		# create hills and peaks
		self.buildHills()
#		mst.mapPrint.buildPlotMap( True, "RingWorld.buildLandscape()_buildHills()" )
		# build region: Hills vs. Land
		ringworldRegions.buildHills_Land()
#		mst.mapPrint.buildPlotMap( True, "RingWorld.buildLandscape()_buildHills_Land()" )
		# Make mountains in region centers
		ringworldRegions.regionCenters()
#		mst.mapPrint.buildPlotMap( True, "RingWorld.buildLandscape()_regionCenters()" )
		# Build Highlands and Foothills
		self.buildHighlands()
#		mst.mapPrint.buildPlotMap( True, "RingWorld.buildLandscape()_buildHighlands()" )

	# transform land to hills and peaks
	def buildHills( self ):
		print "[RingW] -- RingWorld.buildHills()"
		iMult = 1
		if mst.bPfall: iMult += 3
		if sClimate=="Rocky":
			if mst.bPfall:
				iMult += 1
			else:
				iMult += 2
		if sSeaLevel=="Low": iMult += 1
		nChance = (30 + 5*iWorldShape) * iMult / 10

		cntFH1 = 0
		cntFH2 = 0
		cntP = 0
		cntH = 0
		# initially 0.5% Peaks for Solid Ring, 0.8% for Continents, 1.1% for Islands - * 4 for Planetfall
		# -----------------------------------------------------------------------------------------------
		for x in range( iNumPlotsX ):
			for y in range( 2, iNumPlotsY-2 ):
				plot = map.plot( x, y )
				if not plot.isWater():
					if plot.isPeak() or ( mst.chooseNumber(1000) < (5 + 3*iWorldShape)*iMult ):
						if not plot.isPeak():
							plot.setPlotType( PlotTypes.PLOT_PEAK, True, True )
							cntP += 1
						# Add Foothills
						for fx in range(x-1,x+2):
							for fy in range(y-1,y+2):
								p = map.plot( fx, fy )
								if p.isNone(): continue
								if p.isFlatlands():
									plot.setPlotType( mst.choose(nChance, PlotTypes.PLOT_HILLS, PlotTypes.PLOT_LAND), True, True )
									cntFH1 += 1
								elif p.isHills():
									plot.setPlotType( mst.choose(nChance/2, PlotTypes.PLOT_PEAK, PlotTypes.PLOT_HILLS), True, True )
									cntFH2 += 1

		# initially 3% Hills for Solid Ring, 3.5% for Continents, 4% for Islands - * 4 for Planetfall
		# -------------------------------------------------------------------------------------------
		for x in range(iNumPlotsX):
			for y in range( 2, iNumPlotsY-2 ):
				plot = map.plot( x, y )
				if plot.isFlatlands():
					if mst.chooseNumber(1000) < (30 + 5*iWorldShape)*iMult:
						plot.setPlotType( PlotTypes.PLOT_HILLS, True, True )
						cntH += 1
						# Add Hillrange
						for fx in range(x-1,x+2):
							for fy in range(y-1,y+2):
								p = map.plot( fx, fy )
								if p.isNone(): continue
								if p.isFlatlands():
									plot.setPlotType( mst.choose(nChance, PlotTypes.PLOT_HILLS, PlotTypes.PLOT_LAND), True, True )
									cntFH1 += 1
								elif p.isHills():
									plot.setPlotType( mst.choose(nChance/2, PlotTypes.PLOT_PEAK, PlotTypes.PLOT_HILLS), True, True )
									cntFH2 += 1
		print "[RingW] Peaks %i, HighFoothills %i, Hills %i, LowFoothills %i" % (cntP, cntFH2, cntH, cntFH1)

	# build some highlands beside the existing peaks and hills
	def buildHighlands( self ):
		if mst.bPfall: return
		print "[RingW] -- RingWorld.buildHighlands()"

		iMult = 1
		if sClimate=="Rocky": iMult += 2
		if sSeaLevel=="Low": iMult += 1
		nChance = (27 + 4*iWorldShape) * iMult / 10

		sprint = ""
		sprint += "[RingW] baseChance 27, multiplier %i, chance %4.1f \n" % (iMult,nChance)

		cntFH1 = 0
		cntFH2 = 0
		# build highlands and foothills
		for x in range(iNumPlotsX):
			for y in range( iNumPlotsY ):
				plot = map.plot( x, y )
				if plot.isHills() or plot.isPeak():
					for iDir in range( DirectionTypes.NUM_DIRECTION_TYPES ):
						p = plotDirection( x, y, DirectionTypes(iDir) )
						if not p.isNone():
							if p.isFlatlands():
								p.setPlotType( mst.choose(nChance, PlotTypes.PLOT_HILLS, PlotTypes.PLOT_LAND), True, True )
								cntFH1 += 1
							elif p.isHills():
								p.setPlotType( mst.choose(nChance/2, PlotTypes.PLOT_PEAK, PlotTypes.PLOT_HILLS), True, True )
								cntFH2 += 1
		sprint += "[RingW] HighFoothills %i, LowFoothills %i" % ( cntFH2, cntFH1 )
		print sprint

	# place mountains on the equatorial edge of the world, create ringworld-exits
	def worldEdgeLand( self ):
		print "[RingW] -- RingWorld.worldEdgeLand()"
		# Create Ringworld Exits
		dx = mst.chooseNumber( iNumPlotsX )
		plotXY( dx, 0, 0, 0).setPlotType( PlotTypes.PLOT_PEAK, True, True )
		mst.mapSetSign( plotXY( dx, 0, 0, 0), "Ringworld Exit #1" )
		plotXY( dx, 1, 0, 0).setPlotType( PlotTypes.PLOT_PEAK, True, True )
		dx = mst.chooseNumber( iNumPlotsX )
		plotXY( dx, 15, 0, 0).setPlotType( PlotTypes.PLOT_PEAK, True, True )
		mst.mapSetSign( plotXY( dx, 15, 0, 0), "Ringworld Exit #2" )
		plotXY( dx, 14, 0, 0).setPlotType( PlotTypes.PLOT_PEAK, True, True )

		# check if needed
		if mapOptions.Polar in [0,3]: return				# 'NorthPole to SouthPole' or 'No Pole'

		# build wall at the edge of the world
		chLand = 20
		chHills = 25
		y0 = iif( mapOptions.Polar == 1, 15, 0 )
		y1 = abs( y0 - 1 )
		for x in range( iNumPlotsX ):
			plot = map.plot( x, y0 )			# edge
			pl   = map.plot( x, y1 )			# inland
			if not plot.isWater():
				pType = mst.choose( chHills, PlotTypes.PLOT_HILLS, PlotTypes.PLOT_PEAK )
				plot.setPlotType( pType, True, True )
			if not pl.isWater():
				if mst.choose( chLand, True, False ):
					pType = mst.chooseMore( (chLand, PlotTypes.PLOT_LAND), (chHills*2, PlotTypes.PLOT_HILLS), (100, PlotTypes.PLOT_PEAK) )
					pl.setPlotType( pType, True, True )

	# kill off some more spurious lakes
	def adjustLakes( self ):
		print "[RingW] -- RingWorld.adjustLakes()"
		if mapOptions.Polar == 0: return				# all is good
		chLake  = 36
		chHills = 10
		cnt = 0
		for x in range( iNumPlotsX ):
			for y in range( 1, 15 ):
				n = checkLake( x, y )
				pType = mst.choose( chHills, PlotTypes.PLOT_HILLS, PlotTypes.PLOT_LAND )
				# kill small lakes
				if n == 1:
					m = numWaterNeighbors( x, y, 2 )
					if mst.choose( 100 - chLake / (m+1), True, False ):
						map.plot( x, y ).setPlotType( pType, True, True )
						cnt += 1
				# kill edges from big lakes
				elif n == 3:
					m = numWaterNeighbors( x, y, 1 )
					if m == 1:
						if mst.choose( 100 - chLake, True, False ):
							map.plot( x, y ).setPlotType( pType, True, True )
							cnt += 1
		print "[RingW] %i small lakes and lake edges filled in." % ( cnt )

	# change isolated terrains; terList consists of 3-tuples: (oldTerrain, newTerrain, chances)
	def changeTerrainOrphans( self, *terList ):
		print "[RingW] -- RingWorld.changeTerrainOrphans()"
		cnt = 0
		for x in range( iNumPlotsX ):
			for y in range( iNumPlotsY ):
				plot = map.plot( x, y )
				for ter, newTer, chChange in terList:
					if plot.getTerrainType() == ter:
						terNum = mst.numTerrainNeighbors( x, y, ter, 2 )
						if terNum == 0:
							if mst.choose( chChange, True, False ):
								plot.setTerrainType( newTer, True, True )

	# Make Mountain-Centers accessible
	def openMountains( self ):
		print "[RingW] -- RingWorld.openMountains()"
		sprint = ""
		for x in range( iNumPlotsX ):
			for y in range( iNumPlotsY ):
				plot = plotXY(x, y, 0, 0)
				i = numPeakNeighbors( x, y )
				if i == 8:
					dx = mst.choose( 50, x+1, x-1 )
					dy = mst.choose( 50, y+1, y-1 )
					pl = plotXY( dx, dy, 0, 0 )
					if pl.isNone(): continue
					pl.setPlotType( PlotTypes.PLOT_HILLS, True, True )
					if not mst.bPfall:
						pl.setTerrainType( mst.choose(10,mst.etGrass,mst.etTundra), True, True )
						if not plot.isPeak():
							plot.setTerrainType( mst.choose(50,mst.etGrass,mst.etTundra), True, True )
					sprint += "[RingW] Made access for mountain-center at: %r %r \n" % (x,y)
				elif i == 7:
					dx = mst.choose( 50, x+1, x-1 )
					dy = mst.choose( 50, y+1, y-1 )
					pl = plotXY( dx, dy, 0, 0 )
					if pl.isNone(): continue
					pl.setPlotType( PlotTypes.PLOT_HILLS, True, True )
					if not mst.bPfall:
						pl.setTerrainType( mst.choose(20,mst.etGrass,mst.etTundra), True, True )
						if not plot.isPeak():
							plot.setTerrainType( mst.choose(10,mst.etGrass,mst.etTundra), True, True )
					sprint += "[RingW] Made access for mountain-center at: %r %r \n" % (x,y)
		if not sprint=="": print sprint

	# put more flootplains and oasis on deserts
	def desertBloom( self ):
		if mst.bPfall: return
		print "[RingW] -- RingWorld.desertBloom()"
		# chances for features
		chOasis = 30
		chFlood = 30
		sprint = ""
		for x in range( iNumPlotsX ):
			for y in range( iNumPlotsY ):
				plot = map.plot( x, y )
				if plot.isFlatlands():
					if (plot.getTerrainType() == mst.etDesert):
						if plot.getFeatureType() == FeatureTypes.NO_FEATURE:
							des = mst.numTerrainNeighbors( x, y, mst.etDesert )
							if des > 4:
								if ( mst.numFeatureNeighbors(x,y,[mst.efOasis,mst.efFloodPlains])==0 and
									  plot.isRiverSide() ):
									if mst.choose(chOasis+des, True, False):
										plot.setFeatureType( mst.efOasis, -1 )
										sprint += "[RingW] Oasis at: %r %r \n" % (x,y)
							if isBesideLake(x,y):
								if mst.numFeatureNeighbors(x,y,[mst.efOasis,mst.efFloodPlains]) == 0:
									if mst.choose(30, True, False):
										plot.setFeatureType( mst.efFloodPlains, -1 )
										sprint += "[RingW] Floodplains at: %r %r \n" % (x,y)
							if plot.isRiverSide():
								plot.setFeatureType( mst.efFloodPlains, -1 )
								sprint += "[RingW] Floodplains at: %r %r \n" % (x,y)
		if not sprint=="": print sprint

	# put scrub on deserts
	def buildScrub( self ):
		if mst.bPfall: return
		print "[RingW] -- RingWorld.buildScrub()"
		# test for scrub
		if not mst.bScrub:
			print "[RingW] No scrub in this mod"
			return
		# chances for features
		chScrub = 20
		sprint = ""
		for x in range( map.getGridWidth() ):
			for y in range(2,iNumPlotsY-2):
				plot = plotXY( x, y, 0, 0 )
				if plot.isFlatlands():
					if plot.getTerrainType() == mst.etDesert:
						if plot.getFeatureType() == FeatureTypes.NO_FEATURE:
							foundNonDesert = False
							for dx in [-1,0,1]:
								for dy in [-1,0,1]:
									p = plotXY( x, y, dx, dy )
									if p.isNone():	foundNonDesert = True
									if p.isWater() == True:
										foundNonDesert = True
									elif p.isRiverSide():
										foundNonDesert == True
									elif p.getTerrainType() != mst.etDesert:
										foundNonDesert = True
									elif p.getFeatureType() == mst.efOasis:
										foundNonDesert = True
							if foundNonDesert:
								if mst.choose( chScrub, True, False ):
									plot.setFeatureType( mst.efScrub, -1 )
									sprint += "[RingW] Scrub at: %r %r \n" % (x,y)
		if not sprint=="": print sprint

	# adjust ice-caps to different climates
	def adjustIceCaps( self ):
		print "[RingW] -- RingWorld.adjustIceCaps()"
		# check which poles are cold
		if mapOptions.Polar == 3: return
		bBot = (mapOptions.Polar == 0) or (mapOptions.Polar == 1)
		bTop = (mapOptions.Polar == 0) or (mapOptions.Polar == 2)
		# adjust icecap for 'hot' climate
		if sClimate=="Arid" or sClimate=="Tropical":
			for x in range( iNumPlotsX ):
				iceRange = []
				if bBot: iceRange.append(  1 )
				if bTop: iceRange.append( 14 )
				for y in iceRange:
					if mst.choose( 5, True, False ):
						plot = plotXY( x, y, 0, 0 )
						plot.setFeatureType( mst.efIce, -1 )
		# adjust icecap for 'cold' climate
		if sClimate=="Cold":
			for x in range( iNumPlotsX ):
				iceRange = []
				if bBot: iceRange.append(  1 )
				if bTop: iceRange.append( 14 )
				for y in iceRange:
					plot = plotXY( x, y, 0, 0 )
					if plot.getFeatureType() == mst.efIce:
						if mst.choose( 3, True, False ):
							plot.setFeatureType( FeatureTypes.NO_FEATURE, -1 )
							if mst.choose( 15, True, False ):
								plot = plotXY( x, y, -1, 0 )
								plot.setFeatureType( FeatureTypes.NO_FEATURE, -1 )
				iceRange = []
				if bBot: iceRange + [2, 3, 4]
				if bTop: iceRange + [13, 12, 11]
				for y in iceRange:
					plot = plotXY( x, y, 0, 0 )
					if not plot.getFeatureType() == mst.efIce: continue			# no ice
					# get chances
					if y in [2,13]:
						chNoIce = [11,30]
					elif y in [3,12]:
						chNoIce = [22,40]
					else:
						chNoIce = [33,50]
					# get foundation
					if y<8: dy = y - 1
					else:   dy = y + 1
					pl = plotXY(x, dy, 0, 0)
					if pl.getFeatureType() == mst.efIce:
						if mst.choose( chNoIce[0], True, False ):
							pl.setFeatureType( FeatureTypes.NO_FEATURE, -1 )
						elif mst.choose( chNoIce[1], True, False ):
							plot.setFeatureType( FeatureTypes.NO_FEATURE, -1 )

########################################################################
##### Class RingWorld END
########################################################################
ringWorld = RingWorld()


########################################################################
##### Class PlanetFall - functions only used in Planetfall mod
########################################################################
# reduceSeaFungus()
########################################################################
class PlanetFall:
	# Ringworld's oceans are a bit crowded anyway (no alternative routes)
	# Less of a problem with 'Islands'
	def reduceSeaFungus( self ):
		if not mst.bPfall: return
		print "[RingW] -- PlanetFall.removeSeaFungus()"
		sprint = ""
		for x in range( iNumPlotsX ):
			for y in range(1,15):
				plot = plotXY( x, y, 0, 0 )
				if plot.getFeatureType() == mst.efSeaFungus:
					iRand = (3 + ( 4 * (2-iWorldShape) ) )			# single-11, cont-7, isle-3
					if mst.choose( iRand, True, False ):
						plot.setFeatureType( FeatureTypes.NO_FEATURE, -1 )
						sprint += "[RingW] Kill sea-fungus @ %i,%i \n" % (x,y)
		if not sprint=="": print sprint

########################################################################
##### Class PlanetFall END
########################################################################
planetFall = PlanetFall()


# ###################################################################################
# ######## generatePlotTypes() - Called from system
# ######## - FIRST STAGE in building the map
# ######## - creates an array of plots (ocean,hills,peak,land) in the map dimensions
# ###################################################################################
def generatePlotTypes():
	print "[RingW] -- generatePlotTypes()"
	hinted_world = CvMapGeneratorUtil.HintedWorld()

	# Set the top and bottom three tiles to ocean, others to land
	for x in range( iNumPlotsX ):
		for y in range( iNumPlotsY ):
			hinted_world.setValue(x,y,0) # ocean
	hinted_world.buildAllContinents()
	plotTypes = hinted_world.generatePlotTypes(water_percent = 0)

	# fix the water and land mix, part 0
	# ----------------------------------
	plotTypes = [PlotTypes.PLOT_OCEAN] * map.numPlots()
	y0, y1 = 4, 12
	if sWorldShape == "Islands":
		y0, y1 = 3, 13
	elif mapOptions.Polar == 1:
		y1 = 16
	elif mapOptions.Polar == 2:
		y0 =  0
	elif mapOptions.Polar == 3:
		y0, y1 = 2, 14
	for x in range( iNumPlotsX ):
		for y in range( y0, y1 ):
			i = GetIndex(x, y)
			plotTypes[i] = PlotTypes.PLOT_LAND

	#--------------------------------------------------------------------
	# Template to determine the shape of the land
	#--------------------------------------------------------------------
	# Templates are nested by keys: [Land Density, Number of Land tiles]}
	# [LLLL, LLLW, LLWW, LWWW, WWWW]
	#--------------------------------------------------------------------
	Land_Template = {	0: [ 83, 90, 95, 98, 100],
							1: [ 76, 85, 92, 97, 100],
							2: [ 50, 70, 85, 95, 100]
						}

	# fix the water and land mix, part 1
	# ----------------------------------
	for x in range(iNumPlotsX):

		# Choose 1st tile-group
		iNumWater = mst.chooseMore( (Land_Template[iWorldShape][0],0), (Land_Template[iWorldShape][1],1),
											 (Land_Template[iWorldShape][2],2), (Land_Template[iWorldShape][3],3),
											 (Land_Template[iWorldShape][4],4) )
		if iNumWater == 1:
			# only 1 water tile
			# tiles can be WLLL or LLLW
			y = mst.choose( 33, 7, 4)														# 33% LLLW, 67% WLLL
			i = GetIndex( x, y )
			plotTypes[i] = PlotTypes.PLOT_OCEAN

		elif iNumWater == 2:
			# 2 water tiles
			# tiles can be WWLL, LLWW or WLLW
			y1,y2 = mst.chooseMore( (50,(4,5)), (67,(6,7)), (100,(4,7)) )  	# 50% WWLL, 17% LLWW, 33% WLLW
			i = GetIndex(x,y1)
			plotTypes[i] = PlotTypes.PLOT_OCEAN
			i = GetIndex(x,y2)
			plotTypes[i] = PlotTypes.PLOT_OCEAN

		elif iNumWater > 2:
			# either 3 or 4 water tiles
			# set all to water and then set 1 back to land as required
			i = GetIndex(x, 4)
			plotTypes[i] = PlotTypes.PLOT_OCEAN
			i = GetIndex(x, 5)
			plotTypes[i] = PlotTypes.PLOT_OCEAN
			i = GetIndex(x, 6)
			plotTypes[i] = PlotTypes.PLOT_OCEAN
			i = GetIndex(x, 7)
			plotTypes[i] = PlotTypes.PLOT_OCEAN

			if iNumWater == 3:
				# 3 water tiles
				# tiles can be WWWL, WWLW, WLWW or LWWW
				y = mst.chooseMore( (33,7),(66,6),(83,5),(100,4) )					# 33% WWWL, 33% WWLW, 17% WLWW, 17% LWWW
				i = GetIndex(x,y)
				plotTypes[i] = PlotTypes.PLOT_LAND

		# Choose 2nd tile-group
		iNumWater = mst.chooseMore( (Land_Template[iWorldShape][0],0), (Land_Template[iWorldShape][1],1),
											 (Land_Template[iWorldShape][2],2), (Land_Template[iWorldShape][3],3),
											 (Land_Template[iWorldShape][4],4) )

		if iNumWater == 1:
			# only 1 water tile
			# tiles can be WLLL or LLLW
			y = mst.choose( 33, 8, 11 )														# 33% WLLL, 67% LLLW
			i = GetIndex( x, y )
			plotTypes[i] = PlotTypes.PLOT_OCEAN

		elif iNumWater == 2:
			# 2 water tiles
			# tiles can be WWLL, LLWW or WLLW
			y1,y2 = mst.chooseMore( (50,(10,11)), (67,(8,9)), (100,(8,11)) )  	# 50% LLWW, 17% WWLL, 33% WLLW
			i = GetIndex(x,y1)
			plotTypes[i] = PlotTypes.PLOT_OCEAN
			i = GetIndex(x,y2)
			plotTypes[i] = PlotTypes.PLOT_OCEAN

		elif iNumWater > 2:
			# either 3 or 4 water tiles
			# set all to water and then set 1 back to land if required
			i = GetIndex(x, 8)
			plotTypes[i] = PlotTypes.PLOT_OCEAN
			i = GetIndex(x, 9)
			plotTypes[i] = PlotTypes.PLOT_OCEAN
			i = GetIndex(x,10)
			plotTypes[i] = PlotTypes.PLOT_OCEAN
			i = GetIndex(x,11)
			plotTypes[i] = PlotTypes.PLOT_OCEAN

			if iNumWater == 3:
				# 3 water tiles
				# tiles can be WWWL, WWLW, WLWW or LWWW
				y = mst.chooseMore( (33,8),(66,9),(83,10),(100,11) )				# 33% LWWW, 33% WLWW, 17% WWLW, 17% WWWL
				i = GetIndex(x, y)
				plotTypes[i] = PlotTypes.PLOT_LAND

		# a third land-stripe for the equator edge
		if mapOptions.Polar in [1,2]:
			# Choose 3rd tile-group
			iNumWater = mst.chooseMore( (Land_Template[iWorldShape][0],0), (Land_Template[iWorldShape][1],1),
												 (Land_Template[iWorldShape][2],2), (Land_Template[iWorldShape][3],3),
												 (Land_Template[iWorldShape][4],4) )
			# More chance for land on 3rd tile-group
			if mst.choose( 20, True, False ): iNumWater -= 1

			if iNumWater == 1:
				# only 1 water tile
				# tiles can be WLLL or LLLW
				if mapOptions.Polar == 1:
					y = mst.choose( 33, 12, 15 )														# 33% WLLL, 67% LLLW
				if mapOptions.Polar == 2:
					y = mst.choose( 33,  3,  0 )														# 33% WLLL, 67% LLLW
				i = GetIndex( x, y )
				plotTypes[i] = PlotTypes.PLOT_OCEAN

			elif iNumWater == 2:
				# 2 water tiles
				# tiles can be WWLL, LLWW or WLLW
				if mapOptions.Polar == 1:
					y1,y2 = mst.chooseMore( (66,(14,15)), (75,(12,13)), (100,(12,15)) )  	# 66% LLWW, 9% WWLL, 25% WLLW
				elif mapOptions.Polar == 2:
					y1,y2 = mst.chooseMore( (66,( 0, 1)), (75,( 2, 3)), (100,( 0, 3)) )  	# 66% LLWW, 9% WWLL, 25% WLLW
				i = GetIndex(x,y1)
				plotTypes[i] = PlotTypes.PLOT_OCEAN
				i = GetIndex(x,y2)
				plotTypes[i] = PlotTypes.PLOT_OCEAN

			elif iNumWater > 2:
				# either 3 or 4 water tiles
				# set all to water and then set 1 back to land if required
				if mapOptions.Polar == 1: rList = [12,13,14,15]
				if mapOptions.Polar == 2: rList = [3,2,1,0]
				for y in rList:
					i = GetIndex(x, y)
					plotTypes[i] = PlotTypes.PLOT_OCEAN

				if iNumWater == 3:
					# 3 water tiles
					# tiles can be WWWL, WWLW, WLWW or LWWW
					if mapOptions.Polar == 1:
						y = mst.chooseMore( (33,12),(66,13),(83,14),(100,15) )				# 33% LWWW, 33% WLWW, 17% WWLW, 17% WWWL
					elif mapOptions.Polar == 2:
						y = mst.chooseMore( (33, 3),(66, 2),(83, 1),(100, 0) )				# 33% LWWW, 33% WLWW, 17% WWLW, 17% WWWL
					i = GetIndex(x, y)
					plotTypes[i] = PlotTypes.PLOT_LAND

	# fix the water and land mix, part 2 - Make Edges
	# -----------------------------------------------
	if iWorldShape < 2:
		for x in range(iNumPlotsX):
			# 6% chance each for additional land north or south
			if mapOptions.Polar == 0:
				y = mst.chooseMore( (6,3), (12,12) )
			elif mapOptions.Polar == 1:
				y = mst.choose( 6, 3, None )
			elif mapOptions.Polar == 2:
				y = mst.choose( 6, 12, None )
			if mapOptions.Polar == 3:
				y = mst.chooseMore( (6,2), (12,13) )
			if not y == None:
				i = GetIndex(x,y)
				plotTypes[i] = PlotTypes.PLOT_LAND

		# build bays
		print "[RingW] Build Bays"
		for x in range(iNumPlotsX):
			if mapOptions.Polar == 0:   rList = [3,12]
			elif mapOptions.Polar == 1: rList = [3]
			elif mapOptions.Polar == 2: rList = [12]
			elif mapOptions.Polar == 3: rList = [2,3,12,13]
			for y in rList:
				i = GetIndex(x, y)
				if plotTypes[i] == PlotTypes.PLOT_OCEAN:
					if mst.choose( 12, True, False ):
						iOff = iif( y>7, -1, 1 )
						i = GetIndex(x, y + iOff)
						plotTypes[i] = PlotTypes.PLOT_OCEAN

		# build lakes
		print "[RingW] Build Lakes"
		for x in range(iNumPlotsX):
			if mapOptions.Polar == 0:   rList = [6,7,8,9]
			elif mapOptions.Polar == 1: rList = [6,7,8,9,10,11,12,13]
			elif mapOptions.Polar == 2: rList = [2,3,4,5,6,7,8,9]
			elif mapOptions.Polar == 3: rList = [5,6,7,8,9,10]
			for y in rList:
				i = GetIndex(x, y)
				if plotTypes[i] == PlotTypes.PLOT_OCEAN:
					if mapOptions.Polar > 0:
						if mst.choose( 25, True, False ): continue
						if mst.choose( 33, True, False ):
							plotTypes[i] = PlotTypes.PLOT_LAND
							continue
					if mst.choose( 25, True, False ):
						j = GetIndex(x-1,y)
						plotTypes[j] = PlotTypes.PLOT_OCEAN
						j = GetIndex(x-2,y)
						plotTypes[j] = PlotTypes.PLOT_OCEAN
						continue
					if mst.choose( 33, True, False ):
						fx,fy = mst.chooseMore( (15,(x,y-1)), (30,(x,y+1)), (100,(x-1,y)) )
						j = GetIndex(fx,fy)
						plotTypes[j] = PlotTypes.PLOT_OCEAN

	# rough up continent sides
	if mapOptions.Polar == 3:
		for x in range(iNumPlotsX):
			# build isles
			if mst.choose( 4, True, False ):
				y = mst.choose( 50, 0, 15 )
				j = GetIndex(x,y)
				plotTypes[j] = PlotTypes.PLOT_LAND
				if mst.choose( 20, True, False ):
					j = GetIndex(x-1,y)
					plotTypes[j] = PlotTypes.PLOT_LAND
					if mst.choose( 10, True, False ):
						j = GetIndex(x-2,y)
						plotTypes[j] = PlotTypes.PLOT_LAND
			# build edges and beys
			for y in [1,14]:
				iOff = iif( y>7, -1, +1 )
				i = GetIndex(x,y+iOff)
				if plotTypes[i] == PlotTypes.PLOT_LAND:
					if mst.choose( 10, True, False ):
						# build edge
						j = GetIndex(x,y)
						plotTypes[j] = PlotTypes.PLOT_LAND
						if mst.choose( 30, True, False ):
							# build 2nd edge
							j = GetIndex(x-1,y)
							plotTypes[j] = PlotTypes.PLOT_LAND
						if mst.choose( 20, True, False ):
							# build 3rd edge
							j = GetIndex(x-2,y)
							plotTypes[j] = PlotTypes.PLOT_LAND
					elif mst.choose( 12, True, False ):
						# build bay
						j = GetIndex(x,y+iOff)
						plotTypes[j] = PlotTypes.PLOT_OCEAN
						if mst.choose( 24, True, False ):
							# build 2nd bay
							j = GetIndex(x-1,y+iOff)
							plotTypes[j] = PlotTypes.PLOT_OCEAN
						if mst.choose( 16, True, False ):
							# build 3rd bay
							j = GetIndex(x-2,y+iOff)
							plotTypes[j] = PlotTypes.PLOT_OCEAN
							if mst.choose( 8, True, False ):
								# build deep bay
								j = GetIndex(x-1,y+iOff+iOff)
								plotTypes[j] = PlotTypes.PLOT_OCEAN

	# fix the water and land mix, part 3 - Make Isles
	# -----------------------------------------------
	if iWorldShape == 2:
		# break top and bottom stripes
		for x in range(iNumPlotsX):
			if mapOptions.Polar == 0:   rList = [4,11]
			elif mapOptions.Polar == 1: rList = [4]
			elif mapOptions.Polar == 2: rList = [11]
			elif mapOptions.Polar == 3: rList = [3,4,11,12]
			for y in rList:
				j = GetIndex(x, y)
				iOff = iif( y>7, -1, 1 )
				if plotTypes[j] == PlotTypes.PLOT_OCEAN:
					if mst.choose( 80, True, False ):
						j = GetIndex(x, y+iOff)
						plotTypes[j] = PlotTypes.PLOT_OCEAN
				elif mst.choose( 50, True, False ):
					j = GetIndex(x, y+iOff)
					plotTypes[j] = PlotTypes.PLOT_OCEAN

		# break continents
		for x in range(iNumPlotsX):
			i = GetIndex(x, 5)
			j = GetIndex(x, 10)
			if (((plotTypes[i] == PlotTypes.PLOT_OCEAN) and (plotTypes[j] == PlotTypes.PLOT_OCEAN))
			or ( mst.choose( 8, True, False ) ) ):
				if mst.choose( 85, True, False ):
					for y in range( iNumPlotsY ):
						j = GetIndex(x, y)
						plotTypes[j] = PlotTypes.PLOT_OCEAN
						if mst.choose( 85, True, False ):
							j = GetIndex(x-1, y)
							plotTypes[j] = PlotTypes.PLOT_OCEAN
							if mst.choose( 50, True, False ):
								j = GetIndex(x-2, y)
								plotTypes[j] = PlotTypes.PLOT_OCEAN

					j = GetIndex(x-3, 3)
					plotTypes[j] = PlotTypes.PLOT_OCEAN
					j = GetIndex(x+1, 3)
					plotTypes[j] = PlotTypes.PLOT_OCEAN
					j = GetIndex(x-3, 12)
					plotTypes[j] = PlotTypes.PLOT_OCEAN
					j = GetIndex(x+1, 12)
					plotTypes[j] = PlotTypes.PLOT_OCEAN
					x += 1

		# kill most land on Rows 2,13
		for x in range(iNumPlotsX):
			if mapOptions.Polar == 0:   rList = [2,13]
			elif mapOptions.Polar == 1: rList = [2]
			elif mapOptions.Polar == 2: rList = [13]
			elif mapOptions.Polar == 3: rList = []
			for y in rList:
				j = GetIndex(x, y)
				if mst.choose( 16, True, False ):
					plotTypes[j] = PlotTypes.PLOT_OCEAN

	# remove or improve most small islands
	ringworldContinents.removeSmallIslands( plotTypes )

	# fix the water and land mix, part 4 - Make Continents
	# ----------------------------------------------------
	# split continents
	ringworldContinents.buildDivides( plotTypes )
	# make coastlines curvy
	ringworldContinents.roughUpDivides( plotTypes )

	return plotTypes


########################################################################
##### Class RingworldContinents - functions used to divide the world
########################################################################
# removeSmallIslands( aPlots )
# buildDivides( aPlots )
# divideContinent( aPlots, x, iDivide )
# roughUpDivides( aPlots )
# eIslandType = checkIsland( xCoord, yCoord, aPlots)
# iDivide = calcDivide()
########################################################################
class RingworldContinents:

	# remove or improve most 1- or 2-plot islands
	def removeSmallIslands( self, aPlots ):
		print "[RingW] -- RingworldContinents.removeSmallIslands()"
		for x in range( map.getGridWidth() ):
			for y in range( iNumPlotsY ):
				i = GetIndex(x,y)
				if aPlots[i] == PlotTypes.PLOT_OCEAN:
					continue
				iLand = self.checkIsland(x, y, aPlots)
				if iLand == 1:
					if mst.choose( 75, True, False ):
						fx,fy = mst.chooseMore( (20,(x,y+1)), (40,(x,y-1)), (70,(x+1,y)), (100,(x-1,y)) )
						if (fy>2) and (fy<13):
#							print "[RingW] Single Island Doubled: x,y %i,%i - fx,fy %i,%i" % (x,y,fx,fy)
							i = GetIndex(fx,fy)
							aPlots[i] = PlotTypes.PLOT_LAND
							iLand = self.checkIsland(x, y, aPlots)							# maybe it's no island anymore
					else:
						if aPlots[i] == PlotTypes.PLOT_PEAK:
							aPlots[i] = PlotTypes.PLOT_OCEAN									# single mountains stamped out
							continue
				if (iLand == 1) or (iLand == 2):
					if mst.choose( 85 - 10 * iLand, True, False ):
						# There is more than one way to kill small islands
						if mst.choose( 30 + 3 * iSeaLevel, True, False ):
							if iLand==1:
#								print "[RingW] Single Island Stamped Out: x,y %i,%i" % (x,y)
								j = GetIndex(x,y)
								aPlots[j] = PlotTypes.PLOT_OCEAN
							else:
#								print "[RingW] Double Island Stamped Out: x,y %i,%i" % (x,y)
								for fx in range(x-1, x+2):
									for fy in range(y-1, y+2):
										i = GetIndex(fx,fy)
										aPlots[i] = PlotTypes.PLOT_OCEAN
						else:
#							print "[RingW] Island Enhanced: x,y %i,%i" % (x,y)
							for fx in range(x-1, x+2):
								for fy in range(y-1, y+2):
									if (fx==x) and (fy==y):
										continue
									i = GetIndex(fx,fy)
									if (fy<=1) or (fy>=14):
										aPlots[i] = PlotTypes.PLOT_OCEAN
									if (fy==2) or (fy==13):
										aPlots[i] = mst.choose( 50, PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN )
									else:
										aPlots[i] = mst.choose( 90, PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN )
					elif mst.choose( 90, True, False ):
#						print "[RingW] Island Enlarged: x,y %i,%i" % (x,y)
						for fx in range(x, x+2):
							for fy in range(y, y+2):
								if (fx==x) and (fy==y):
									continue
								i = GetIndex(fx,fy)
								if (fy<=1) or (fy>=14):
									aPlots[i] = PlotTypes.PLOT_OCEAN
								if (fy==2) or (fy==13):
									aPlots[i] = mst.choose( 50, PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN )
								else:
									aPlots[i] = mst.choose( 70, PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN )

	# build divides depending on map type, width and sealevel
	def buildDivides( self, aPlots ):
		print "[RingW] -- RingworldContinents.buildDivides()"
		xDivides = []
		if iWorldShape == 0:								# 'Solid Ring' - build channel, small chance
			for x in range(iNumPlotsX):
				iRand = mst.chooseNumber( 1000 )
				if iRand < 4:
					iDivide = 0
					if sSeaLevel=="High": iDivide += 1
					if mapOptions.Polar > 0: iDivide += 1
					xDivides.append( (x,iDivide) )
					x += 40 + 5 * iWorldSize
		else:													# 'Continents' or 'Islands'
			# make sure Isles and Continents get a very good chance at 2 Continental Divides
			iRand1 = mst.chooseNumber( 2*iNumPlotsX/6 )
			iDiv1 = self.calcDivide()
			iRand2 = mst.chooseNumber( 3*iNumPlotsX/6, 5*iNumPlotsX/6 )
			iDiv2 = self.calcDivide()

			# bonus chance for additional continents
			xSizeMod = iWorldSize * 3 + iSeaLevel
			x = 0
			while x < iNumPlotsX:
				iCnt = 0
				for y in [4,5,6,7,8,9,10,11]:
					j = GetIndex(x,y)
					if aPlots[j]==PlotTypes.PLOT_OCEAN:
						iCnt += 1
				if ((iWorldShape == 1) and (iCnt>3)) or ((iWorldShape == 2) and (iCnt>5)):
					iRand0 = mst.chooseNumber( 100 )
					if (x > (iRand1 - 15 - 2*iDiv1 - xSizeMod)) and (x < (iRand1 + 15 + 2*iDiv1 + xSizeMod)):
						xDivides.append( (x,iDiv1) )
						x += 15 + 2*iDiv1 + xSizeMod
					elif (x > (iRand2 - 15 - 2*iDiv2 - xSizeMod)) and (x < (iRand2 + 15 + 2*iDiv2 + xSizeMod)):
						xDivides.append( (x,iDiv2) )
						x += 15 + 2*iDiv2 + xSizeMod
					elif  ( ((iWorldShape == 1) and (iCnt > 5) and (iRand0 > (20-xSizeMod) ))
						or (  (iWorldShape == 1) and (iCnt > 4) and (iRand0 > (75-xSizeMod) ))
						or (  (iWorldShape == 2) and (iCnt > 7) and (iRand0 > 25))
						or (  (iWorldShape == 2) and (iCnt > 6) and (iRand0 > 80)) ):
						iDivide = self.calcDivide()
						xDivides.append( (x,iDivide) )
						x += 15 + 2*iDivide + xSizeMod
				x += 1

		# do continental divisions
		for i in range(len(xDivides)):
			x,iDivide = xDivides[i]
			self.divideContinent(aPlots, x, iDivide)

	#--------------------------------
	# Divide Continent
	#
	#                      x   iSkip
	#           !     !  !!!    !
	# 0,1,2     .........ooo.....
	# 3,4       ......3oooooo3...
	# 5,6       .......79ooo5....
	# 7,8       .......79ooo5....
	# 9,10      .......79ooo5....
	# 11,12     ......3oooooo3...
	# 13,14,15  .........ooo.....
	#           !     !  !!! !  !
	#                 !    ! ! iSkip
	# x-iDivide+     -2    x 2
	# iDivide = 3
	#--------------------------------
	def divideContinent( self, aPlots, x, iDivide ):
#		print "[RingW] -- RingworldContinents.divideContinent()"

		# iDivide tiles water: x, x-1...
		for y in range(iNumPlotsY):
			for i in range(iDivide):
				j = GetIndex(x-i,y)
				aPlots[j] = PlotTypes.PLOT_OCEAN

		# kurve edges
		rList = [3,4,11,12]
		if mapOptions.Polar == 3: rList = [2,3,12,13]
		for y in rList:
			j = GetIndex(x+1,y)
			aPlots[j] = PlotTypes.PLOT_OCEAN
			j = GetIndex(x-iDivide,y)
			aPlots[j] = PlotTypes.PLOT_OCEAN
			j = GetIndex(x-iDivide-1,y)
			aPlots[j] = PlotTypes.PLOT_OCEAN
			if mst.choose( 30, True, False ):
				j = GetIndex(x+2,y)
				aPlots[j] = PlotTypes.PLOT_OCEAN
			if mst.choose( 30, True, False ):
				j = GetIndex(x-iDivide-2,y)
				aPlots[j] = PlotTypes.PLOT_OCEAN
		rList = [5,6,7,8,9,10]
		if mapOptions.Polar == 3: rList = [4,5,6,7,8,9,10,11]
		for y in rList:
			if mst.choose( 90, True, False ):
				j = GetIndex(x-iDivide,y)
				aPlots[j] = PlotTypes.PLOT_OCEAN
			if mst.choose( 70, True, False ):
				j = GetIndex(x-iDivide-1,y)
				aPlots[j] = PlotTypes.PLOT_OCEAN
			if mst.choose( 50, True, False ):
				j = GetIndex(x+1,y)
				aPlots[j] = PlotTypes.PLOT_OCEAN

	# roughen up the edges of continents, kill straight lines
	def roughUpDivides( self, aPlots ):
		print "[RingW] -- RingworldContinents.roughUpDivides()"
		for x in range(iNumPlotsX):
			iWater = 0
			rList = [3,4,5,6,7,8,9,10,11,12]
			if mapOptions.Polar == 3: rList = [2,3,4,5,6,7,8,9,10,11,12,13]
			for y in rList:
				j = GetIndex(x, y)
				if aPlots[j] == PlotTypes.PLOT_OCEAN:
					iWater += 1

			NO_OCEAN  = -1
			NO_CHANGE = -2
			if iWater == 10:
	#			print "[RingW] roughUpDivides(), Straight Channel Found at x = %i" % (x)

				# western coast
				iLand = 0
				bCoast = False
				for y in rList:
					j = GetIndex(x-1, y)
					if aPlots[j] != PlotTypes.PLOT_OCEAN:
						iLand += 1
					else:
						iLand = 0
					if iLand	>= 5:
						# found stretch of 5 land plots
						bCoast = True
						continue
				if bCoast:
	#				print "[RingW] roughUpDivides(), Rough Up Coast - West of %i" % (x)
					for y in rList:
						j = GetIndex(x-1, y)
						if aPlots[j] != PlotTypes.PLOT_OCEAN:
							c = mst.chooseMore( (20,PlotTypes.PLOT_OCEAN), (90,NO_OCEAN), (100,NO_CHANGE) )
							if c==PlotTypes.PLOT_OCEAN:
								aPlots[j] = PlotTypes.PLOT_OCEAN
							elif c==NO_OCEAN:
								j = GetIndex(x, y)
								aPlots[j] = mst.choose( 90, PlotTypes.PLOT_LAND, PlotTypes.PLOT_HILLS )

				# eastern coast
				iLand = 0
				bCoast = False
				for y in rList:
					j = GetIndex(x+1, y)
					if aPlots[j] != PlotTypes.PLOT_OCEAN:
						iLand += 1
					else:
						iLand = 0
					if iLand	>= 5:
						# found stretch of 5 land plots
						bCoast = True
						continue
				if bCoast:
	#				print "[RingW] roughUpDivides(), Rough Up Coast - East of %i" % (x)
					for y in rList:
						j = GetIndex(x+1, y)
						if aPlots[j] != PlotTypes.PLOT_OCEAN:
							c = mst.chooseMore( (20,PlotTypes.PLOT_OCEAN), (90,NO_OCEAN), (100,NO_CHANGE) )
							if c==PlotTypes.PLOT_OCEAN:
								aPlots[j] = PlotTypes.PLOT_OCEAN
							elif c==NO_OCEAN:
								j = GetIndex(x, y)
								aPlots[j] = mst.choose( 90, PlotTypes.PLOT_LAND, PlotTypes.PLOT_HILLS )

	# test if plot is part of island
	# -1-ocean/lake, 1-single-tile, 2-double-tile, 3-big island
	def checkIsland( self, xCoord, yCoord, aPlots):
		j = GetIndex( xCoord, yCoord )
		if aPlots[j] == PlotTypes.PLOT_OCEAN:
			return -1														# ocean or lake

		land = []
		iLand = 0
		for x in range(xCoord-1, xCoord+2):
			for y in range(yCoord-1, yCoord+2):
				i = GetIndex(x,y)
				if not (aPlots[i] == PlotTypes.PLOT_OCEAN):
					iLand += 1
					land.append( (x,y) )
		if iLand == 1:
	#		print "[RingW] Single Island Found: x,y %i,%i" % (xCoord,yCoord)
			return 1															# single-tile island
		if iLand == 2:
			fx,fy = land[0]
			if (fx==xCoord) and (fy==yCoord):
				fx,fy = land[1]
			iLand = 0
			for x in range(fx-1, fx+2):
				for y in range(fy-1, fy+2):
					i = GetIndex(x,y)
					if not (aPlots[i] == PlotTypes.PLOT_OCEAN):
						iLand += 1
			if iLand == 2:
	#			print "[RingW] Double Island Found: x,y %i,%i" % (xCoord,yCoord)
				return 2														# double-tile island
		return 3																# big island

	# calculate width of continental divide
	def calcDivide( self ):
#		print "[RingW] -- RingworldContinents.calcDivides()"
		iDivMax = iWorldSize
		if mst.bPfall: iDivMax += 1			# 'Planetfall' has bigger divides (give shelf a chance!)
		if iWorldShape==1: iDivMax += 2		# Continents have bigger divides
		if iWorldShape==2: iDivMax -= 1		# Islands have smaller divides
		if iClimate==1: iDivMax += 1			# Tropical Climate
		if iClimate==2: iDivMax -= 1			# Arid Climate
		if iSeaLevel==0 and iWorldSize<3:
			iDivMax -= 1							# Low Sealevel: Duel, Tiny, Small
		elif iSeaLevel==0:
			iDivMax -= 2							# Low Sealevel: Standard, Large, Huge
		if iSeaLevel==2 and iWorldSize<2:
			iDivMax += 1							# High Sealevel: Duel, Tiny
		elif iSeaLevel==2 and iWorldSize<4:
			iDivMax += 2							# High Sealevel: Small, Standard
		elif iSeaLevel==2 and iWorldSize<6:
			iDivMax += 3							# High Sealevel: Large, Huge
		elif iSeaLevel==2:
			iDivMax += 4							# High Sealevel: Giant

		iDivide = mst.chooseNumber( 3 + iDivMax )

		# normalize for worldsize
		if iWorldSize==0:							# 0, 7  -->  2, 5  (2,5)
			iDivide = iDivide *  4/ 8 + 2
		elif iWorldSize==1:						# 0, 8  -->  3, 7  (3,6)
			iDivide = iDivide *  5/ 9 + 3
		elif iWorldSize==2:						# 0,10  -->  3, 8  (3,7)
			iDivide = iDivide *  6/11 + 3
		elif iWorldSize==3:						# 0,11  -->  3, 9  (3,8)
			iDivide = iDivide *  7/12 + 3
		elif iWorldSize==4:						# 0,13  -->  4,11  (4,10)
			iDivide = iDivide *  8/14 + 4
		elif iWorldSize==5:						# 0,14  -->  4,12  (4,11)
			iDivide = iDivide *  9/15 + 4
		else:											# 0,16  -->  5,14  (5,13)
			iDivide = iDivide * 10/17 + 5

		if mapOptions.Polar > 0: iDivide += 1
		return iDivide

########################################################################
##### Class RingworldContinents END
########################################################################
ringworldContinents = RingworldContinents()


########################################################################
##### Class RingworldRegions - functions used to make regions
########################################################################
# buildHills_Land()
# buildDesert_Grass( aTerrain )
# regionCenters()
# bChange = latitudeAdjust( y, dx, chLatitude )
# fDist = regionDiff( x, xMarker )
########################################################################
class RingworldRegions:

	#----------------------------------------------------------------------------------------
	# define a region with more hills and a flat one with less hills
	#
	# 0    5    10   15   20   25   30   35   40   45   50   55   60   65   70   75   80
	# !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !
	# ..............no more hills...........................no more hills..............  3-4
	# ..........+...................+...................+...................+..........   6
	# ..........+......HILLS1.......+...................+......HILLS2.......+..........   7
	# ..........+...................+...................+...................+..........   8
	# ..........+....Land->Hills....+...................+....Land->Hills....+..........   9
	# ..........+...................+...................+...................+..........  10
	# ..............no more hills...........................no more hills.............. 11-12
	# !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !    !
	#                     20                                      60
	#                     !                     !                 !
	#                    xHills  <-(dxHills1)-> x <-(dxHills2)-> xHills+iWidth/2
	#----------------------------------------------------------------------------------------
	def buildHills_Land( self ):
		print "[RingW] -- RingworldRegions.buildHills_Land()"
		if iRegionStrength==0: return
		iWidth = map.getGridWidth()
		probHillMin = 0.01 * iRegionStrength
		probHill    = 0.15
		chLatitude  = 90

	#	print "[RingW] xHills1, xHills2: %i, %i - dxRegion %5.3f" % (xHills,xHills+iWidth/2,dxRegion)
		for x in range(iWidth):
			dxHills1 = self.regionDiff( x, xHills )
			dxHills2 = self.regionDiff( x, xHills + iWidth/2 )

			fChance1 = probHillMin + probHill * (dxHills1 - dxRegion)/ (1 - dxRegion) * (iRegionStrength / 2.0)
			fChance2 = probHillMin + probHill * (dxHills2 - dxRegion)/ (1 - dxRegion) * (iRegionStrength / 2.0)
	#		print "[RingW] x: %3i - dxH1, chance: %5.3f, %7.5f  -  dxH2, chance: %5.3f, %7.5f" % (x,dxHills1,fChance1,dxHills2,fChance2)
			rList = [3,4,5,6,7,8,9,10,11,12]
			if mapOptions.Polar == 3: rList = [2,3,4,5,6,7,8,9,10,11,12,13]
			for y in rList:
				plot = plotXY( x, y, 0, 0 )
				if plot.isFlatlands() and (dxHills1>dxRegion):
					iRand = mst.chooseNumber( 1000 )
					if iRand < (1000.0 * fChance1):
						if self.latitudeAdjust( y, dxHills1, chLatitude ):
	#						print "[RingW] -> Changed Land to Hills1 @ %i,%i - %5.3f chance, %5.3f distance" % (x,y,fChance1*100,dxHills1)
							plot.setPlotType( PlotTypes.PLOT_HILLS, True, True )
				elif plot.isFlatlands() and (dxHills2>dxRegion):
					iRand = mst.chooseNumber( 1000 )
					if iRand < (1000.0 * fChance2):
						if self.latitudeAdjust( y, dxHills2, chLatitude ):
	#						print "[RingW] -> Changed Land to Hills2 @ %i,%i - %5.3f chance, %5.3f distance" % (x,y,fChance2*100,dxHills2)
							plot.setPlotType( PlotTypes.PLOT_HILLS, True, True )

	#---------------------------------------------------------------
	# PLOT-MAP
	# 0    5    10   15   20   25   30   35   40   45   50   55   60
	# !    !    !    !    !    !    !    !    !    !    !    !    !
	# ..............no more hills.................no more hills....
	# ............+..............+..............+..............+...
	# ............+..............+..............+..............+...
	# ............+....HILLS1....+..............+....HILLS2....+...
	# ............+..............+..............+..............+...
	# ............+.Land->Hills..+..............+.Land->Hills..+...
	# ............+..............+..............+..............+...
	# ..............no more hills.................no more hills....
	# !    !    !    !    !    !    !    !    !    !    !    !    !
	#                     20                            50
	#                     !                             !
	#                    xHills1                       xHills2
	#
	# TERRAIN-MAP
	# 0    5    10   15   20   25   30   35   40   45   50   55   60
	# !    !    !    !    !    !    !    !    !    !    !    !    !
	# .............................................................
	# .....+..............+..............+..............+..........
	# .....+...DESERT.....+..............+...GRASS......+..........
	# .....+..............+..............+..............+..........
	# .....+Tundra->Grass +..............+Desert->Plains+..........
	# .....+Grass->Plains.+..............+Plains->Grass.+..........
	# .....+Plains->Desert+..............+..............+..........
	# .............................................................
	# !    !    ! !  !    !    ! !  !    !    ! !  !    !    ! !  !
	#             12             27             42             57
	#             !              !              !              !
	# Scenario1:  !             xDesert         !             xGrass        xD = xH + 1/8
	#             !              !              !              !
	# Scenario2: xGrass          !             xDesert         !            xD = xH + 3/8
	#             !              !              !              !
	# Scenario3:  !             xGrass          !             xDesert       xD = xH + 5/8
	#             !                             !
	# Scenario4: xDesert                       xGrass                       xD = xH + 7/8
	#----------------------------------------------------------------
	def buildDesert_Grass( self, aTerrain ):
		print "[RingW] -- RingworldRegions.buildDesert_Grass()"
		if iRegionStrength==0: return
		ChanceDesert = 50
		ChanceGrass  = 60
		ChanceLat    = 10

		if mst.bPfall:
			iAridR  = mst.etRockyArid
			iAridF  = mst.etFlatArid
			iMoistR = mst.etRockyMoist
			iMoistF = mst.etFlatMoist
			iRainyR = mst.etRockyRainy
			iRainyF = mst.etFlatRainy
			iPolarR = mst.etRockyPolar
			iPolarF = mst.etFlatPolar
		else:
			iDesert   = mst.etDesert
			iPlains   = mst.etPlains
			iGrass    = mst.etGrass
			iTundra   = mst.etTundra
			iSnow     = mst.etSnow

		iWidth = map.getGridWidth()
		for x in range(iWidth):
			dxDesert = self.regionDiff(x, xDesert)
			fChance1 = ChanceDesert * (iRegionStrength / 2.0) * dxDesert
			dxGrass = 1.0 - dxDesert
			fChance2 = ChanceGrass  * (iRegionStrength / 2.0) * dxGrass
			for y in range(1,15):
				if plotXY( x, y, 0, 0 ).isWater():
					continue
				j = GetIndex(x,y)

				# changes in region grass --> desert
				if dxDesert>dxRegion:
					if mst.bPfall:
						if aTerrain[j]==iPolarR:
							aTerrain[j] = mst.choose(fChance1/2,iRainyR,iPolarR)
						elif aTerrain[j]==iPolarF:
							aTerrain[j] = mst.choose(fChance1/2,iRainyF,iPolarF)
						elif aTerrain[j]==iRainyR:
							aTerrain[j] = mst.choose(fChance1,iMoistR,iRainyR)
						elif aTerrain[j]==iRainyF:
							aTerrain[j] = mst.choose(fChance1,iMoistF,iRainyF)
						elif aTerrain[j]==iMoistR:
							aTerrain[j] = mst.choose(fChance1,iAridR,iMoistR)
						elif aTerrain[j]==iMoistF:
							aTerrain[j] = mst.choose(fChance1,iAridF,iMoistF)
					else:
						if aTerrain[j]==iSnow:
							aTerrain[j] = mst.choose(fChance1,iTundra,iSnow)
						elif aTerrain[j]==iTundra:
							aTerrain[j] = mst.choose(fChance1,iGrass,iTundra)
						elif aTerrain[j]==iGrass:
							aTerrain[j] = mst.choose(fChance1,iPlains,iGrass)
						elif aTerrain[j]==iPlains:
							aTerrain[j] = mst.choose(fChance1,iDesert,iPlains)

				# changes in region desert --> grass
				if dxGrass>dxRegion:
					if mst.bPfall:
						if aTerrain[j]==iAridR:
							aTerrain[j] = mst.choose(fChance2,iMoistR,iAridR)
						elif aTerrain[j]==iAridF:
							aTerrain[j] = mst.choose(fChance2,iMoistF,iAridF)
						elif aTerrain[j]==iMoistR:
							aTerrain[j] = mst.choose(fChance2,iRainyR,iMoistR)
						elif aTerrain[j]==iMoistF:
							aTerrain[j] = mst.choose(fChance2,iRainyF,iMoistF)
					else:
						if aTerrain[j]==iDesert:
							aTerrain[j] = mst.choose(fChance2,iPlains,iDesert)
						elif aTerrain[j]==iPlains:
							aTerrain[j] = mst.choose(fChance2,iGrass,iPlains)
						elif aTerrain[j]==iGrass:
							aTerrain[j] = mst.choose(fChance2/2,iTundra,iGrass)
						elif aTerrain[j]==iTundra:
							aTerrain[j] = mst.choose(fChance2/2,iSnow,iTundra)

	# Make mountains in region centers
	def regionCenters( self ):
		# hills get 2 peaks each
		x = xHills + mst.chooseNumber( 5 ) - 2
		y = 7 + mst.chooseNumber( 4 )
		plotXY(x, y, 0, 0).setPlotType( PlotTypes.PLOT_PEAK, True, True )
		x = xHills + mst.chooseNumber( 5 ) - 2
		y = 6 + mst.chooseNumber( 6 )
		plotXY(x, y, 0, 0).setPlotType( PlotTypes.PLOT_PEAK, True, True )
		x = xHills + mst.chooseNumber( 5 ) - 2 + iNumPlotsX/2
		y = 7 + mst.chooseNumber( 4 )
		plotXY(x, y, 0, 0).setPlotType( PlotTypes.PLOT_PEAK, True, True )
		x = xHills + mst.chooseNumber( 5 ) - 2 + iNumPlotsX/2
		y = 6 + mst.chooseNumber( 6 )
		plotXY(x, y, 0, 0).setPlotType( PlotTypes.PLOT_PEAK, True, True )
		# desert and grass get only one peak
		x = xDesert + mst.chooseNumber( 5 ) - 2
		y = 7 + mst.chooseNumber( 4 )
		plotXY(x, y, 0, 0).setPlotType( PlotTypes.PLOT_PEAK, True, True )
		x = xGrass + mst.chooseNumber( 5 ) - 2
		y = 7 + mst.chooseNumber( 4 )
		plotXY(x, y, 0, 0).setPlotType( PlotTypes.PLOT_PEAK, True, True )

	# 2nd chance 'no more hills'
	# at y: 3-4,11-12 in central region
	def latitudeAdjust( self, y, dx, chLatitude ):
#		print "[RingW] -- RingworldRegions.latitudeAdjust()"
		if dx<0.8:
			rList = [3,4,11,12]
			if mapOptions.Polar == 3: rList = [2,3,12,13]
			if y in rList:
				return mst.choose( chLatitude, True, False )
		return True

	# normalize 'nearness' squared to region center: 1 on region center, 0 max distance (= width/2)
	def regionDiff( self, x, xMarker ):
#		print "[RingW] -- RingworldRegions.regionDiff()"
		iWidth = map.getGridWidth()
		maxDiff = iWidth / 2.0
		xCenter = xMarker % iWidth
		minX = min( abs(xCenter-x), iWidth-abs(xCenter-x) )
		rDiff = float(1.0 - minX / maxDiff)
		return (rDiff * rDiff)

########################################################################
##### Class RingworldRegions - functions used to make regions
########################################################################
ringworldRegions = RingworldRegions()


########################################################################
##### Class RingworldTerrainGenerator - make Ringworld3 terrain
########################################################################
# generateTerrain()
# adjustTerrain( x, y, ter, data):
# generateTerrainAtPlot( iX, iY):
# fLat = getLatitudeAtPlot( iX, iY):
########################################################################
class RingworldTerrainGenerator(CvMapGeneratorUtil.TerrainGenerator):
	def generateTerrain(self):
		print "[RingW] -- RingworldTerrainGenerator:generateTerrain()"

		# build default terrain
		terrainData = [0]*(self.iWidth*self.iHeight)
		for x in range(self.iWidth):
			for y in range(self.iHeight):
				iI = y*self.iWidth + x
				terrain = self.generateTerrainAtPlot(x, y)
				terrainData[iI] = terrain

		# build Region: Desert vs. Grass
		ringworldRegions.buildDesert_Grass( terrainData )

		# build local groupings
		for x in range( iNumPlotsX ):
			for y in range( iNumPlotsY ):
				j = GetIndex(x,y)
				if map.plot( x, y ).isWater(): continue
				elif map.plot( x, y ).isPeak(): continue
				if terrainData[j]==self.terrainDesert:
					self.adjustTerrain(x,y,[0,-1,-1,-1,-2],terrainData)
				elif terrainData[j]==self.terrainPlains:
					self.adjustTerrain(x,y,[1, 0,-1,-1,-1],terrainData)
				elif terrainData[j]==self.terrainGrass:
					self.adjustTerrain(x,y,[1, 1, 0,-1,-1],terrainData)
				elif terrainData[j]==self.terrainTundra:
					self.adjustTerrain(x,y,[1, 1, 1, 0,-1],terrainData)
				elif terrainData[j]==self.terrainIce:
					self.adjustTerrain(x,y,[2, 1, 1, 1, 0],terrainData)

		# icy mountains
		for x in range( iNumPlotsX ):
			for y in range( iNumPlotsY ):
				j = GetIndex(x,y)
				if self.map.plot(x,y).isPeak():
					if y<4 or y>11:
						terrainData[j] = self.terrainIce
					else:
						terrainData[j] = mst.choose( 80, self.terrainTundra, self.terrainIce )
				elif not self.map.plot(x,y).isWater():
					# make Mountain-Highlands
					i = numPeakNeighbors(x,y)
					if i > 6:
						terrainData[j] = self.terrainIce
					elif i > 4:
						terrainData[j] = self.terrainTundra

		# change desert surrounded by water to plains!
		for x in range( iNumPlotsX ):
			for y in range( iNumPlotsY ):
				j = GetIndex(x,y)
				if terrainData[j]==mst.etDesert:
					coast = numWaterNeighbors( x, y )
					if coast > 3:
						if mst.choose( 66, True, False):
							terrainData[j] = self.terrainPlains
							print "[RingW] Desert->Plains at: %r,%r" % (x,y)

		return terrainData

	# reduce radical terrain differences
	def adjustTerrain(self, x, y, ter, data):
		iChance = 33
		tList = [self.terrainDesert,self.terrainPlains,self.terrainGrass,self.terrainTundra,self.terrainIce]
		for eCard in range( CardinalDirectionTypes.NUM_CARDINALDIRECTION_TYPES ):
			p = plotCardinalDirection( x, y, CardinalDirectionTypes(eCard) )
			if not p.isNone():
				dx,dy = p.getX(), p.getY()
				i = GetIndex( dx, dy )
				for j in range( len(tList) ):
					if data[i]==tList[j]:
						if mst.choose(iChance, True, False): data[i] = tList[j+ter[j]]

	def generateTerrainAtPlot(self, iX, iY):
		plot = map.plot( iX, iY )
		if plot.isWater(): return plot.getTerrainType()

		# Default terrain        ice,tun,gra,pln,des
		terrain_Template = {	0: [ 50, 80,100,100,100],
									1: [ 20, 40, 85,100,100],
									2: [  0,  5, 70, 95,100],
									3: [  0,  0, 25, 75,100],
									4: [  0,  0, 30, 60,100],
									5: [  0,  0, 66,100,100]
									}
		clima_Template = [ self.terrainIce, self.terrainTundra, self.terrainGrass, self.terrainPlains, self.terrainDesert ]

		if mst.evalLatitude(plot, False) < 0.1:
			clima = 5		# tropics
		elif (mst.evalLatitude(plot, False) >= 0.1) and (mst.evalLatitude(plot, False) < 0.2):
			clima = 4		# desert
		elif (mst.evalLatitude(plot, False) >= 0.2) and (mst.evalLatitude(plot, False) < 0.4):
			clima = 3		# plains
		elif (mst.evalLatitude(plot, False) >= 0.4) and (mst.evalLatitude(plot, False) < 0.6):
			clima = 2		# grass
		elif (mst.evalLatitude(plot, False) >= 0.6) and (mst.evalLatitude(plot, False) < 0.8):
			clima = 1		# tundra
		else:
			clima = 0		# ice

		terrainVal = mst.chooseMore(	(terrain_Template[clima][0], clima_Template[0]),
												(terrain_Template[clima][1], clima_Template[1]),
												(terrain_Template[clima][2], clima_Template[2]),
												(terrain_Template[clima][3], clima_Template[3]),
												(terrain_Template[clima][4], clima_Template[4])
											)

		# Arid Climate: Ice<-Tundra<-Grass, Grass->Plains->Desert
		if (sClimate == "Arid") and mst.choose( 10, True, False ):
			if terrainVal==self.terrainPlains    and mst.choose( 90, True, False ):
					terrainVal = self.terrainDesert		# 9%
			elif terrainVal==self.terrainGrass   and mst.choose( 45, True, False ):
					terrainVal = self.terrainPlains		# 4.5%
			elif terrainVal==self.terrainGrass   and mst.choose(  5, True, False ):
					terrainVal = self.terrainTundra		# 0.5%
			elif terrainVal==self.terrainTundra  and mst.choose( 10, True, False ):
					terrainVal = self.terrainIce			# 1%
		# Tropical Climate: Ice->Tundra->Grass, Grass<-Plains<-Desert
		elif (sClimate == "Tropical") and mst.choose( 15, True, False ):
			if terrainVal==self.terrainDesert    and mst.choose( 60, True, False ):
					terrainVal = self.terrainPlains		# 6%
			elif terrainVal==self.terrainPlains  and mst.choose( 30, True, False ):
					terrainVal = self.terrainGrass		# 3%
			elif terrainVal==self.terrainTundra  and mst.choose( 30, True, False ):
					terrainVal = self.terrainGrass		# 3%
			elif terrainVal==self.terrainIce     and mst.choose( 40, True, False ):
					terrainVal = self.terrainTundra		# 4%
		# Cold Climate: Ice<-Tundra<-Grass<-Plains<-Desert
		if (sClimate == "Cold") and mst.choose( 10, True, False ):
			if terrainVal==self.terrainDesert    and mst.choose( 50, True, False ):
					terrainVal = self.terrainPlains		# 6%
			elif terrainVal==self.terrainPlains  and mst.choose( 25, True, False ):
					terrainVal = self.terrainGrass		# 3%
			elif terrainVal==self.terrainGrass   and mst.choose( 10, True, False ):
					terrainVal = self.terrainTundra		# 0.5%
			elif terrainVal==self.terrainTundra  and mst.choose( 20, True, False ):
					terrainVal = self.terrainIce			# 2%

		if (terrainVal == TerrainTypes.NO_TERRAIN):
			return plot.getTerrainType()
		return terrainVal

	def getLatitudeAtPlot(self, iX, iY):
		"returns a value in the range of 0.0 (tropical) to 1.0 (polar)"
		return Ringworld_getLatitudeAtPlot(iX, iY)

########################################################################
##### Class RingworldTerrainGenerator END
########################################################################


########################################################################
##### Class PlanetfallTerrainGenerator - make terrain for Planetfall
########################################################################
# generateTerrain()
# fLat = getLatitudeAtPlot( iX, iY):
########################################################################
class PlanetfallTerrainGenerator(CvMapGeneratorUtil.TerrainGenerator):
	def generateTerrain(self):
		print "[RingW] -- PlanetfallTerrainGenerator.generateTerrain()"

		# build default terrain
		terrainData = [0]*(self.iWidth*self.iHeight)
		for x in range(self.iWidth):
			for y in range(self.iHeight):
				j = y*self.iWidth + x
				terrain = self.generateTerrainAtPlot(x, y)
				terrainData[j] = terrain
		# build Region: desert vs. grass (in this case arid vs rainy)
		ringworldRegions.buildDesert_Grass( terrainData )
		return terrainData

	def getLatitudeAtPlot(self, iX, iY):
		"returns a value in the range of 0.0 (tropical) to 1.0 (polar)"
		return Ringworld_getLatitudeAtPlot(iX, iY)

########################################################################
##### Class PlanetfallTerrainGenerator END
########################################################################


# #######################################################################################
# ######## define latitudes
# ######## - for use in .getLatitudeAtPlot() within ringworld generator classes
# #######################################################################################
def Ringworld_getLatitudeAtPlot(x, y):
	"returns a value in the range of 0.0 (tropical) to 1.0 (polar)"
	if sPolePosition   == "NorthPole to SouthPole":
		#       86   72   58   54   47   36   22   4    4    22   36   47   54   58   72   86
		lat = [0.95,0.80,0.64,0.60,0.52,0.40,0.24,0.04,0.04,0.24,0.40,0.52,0.60,0.64,0.80,0.95]
	elif sPolePosition == "Equator to SouthPole":
		#       88   77   68   59   50   43   37   31   24   19   14   11   8    5    3    0
		lat = [0.98,0.86,0.75,0.65,0.56,0.48,0.41,0.34,0.27,0.21,0.16,0.12,0.09,0.06,0.03,0.00]
	elif sPolePosition == "NorthPole to Equator":
		#       0    3    5    8    11   14   19   24   31   37   43   50   59   68   77   88
		lat = [0.00,0.03,0.06,0.09,0.12,0.16,0.21,0.27,0.34,0.41,0.48,0.56,0.65,0.75,0.86,0.98]
	elif sPolePosition == "No Poles":
		#       63   52   42   33   25   18   12    7    3    0    3    7   12   18   25   33
		lat = [0.70,0.57,0.47,0.37,0.28,0.20,0.13,0.08,0.03,0.00,0.03,0.08,0.13,0.20,0.28,0.37]
	return lat[y]


# NOT USED!
# #######################################################################################
# ######## addBonus() - Called from system
# ######## - SIXTH STAGE in building the map
# #######################################################################################
#def addBonus():
#	print "[RingW] -- addBonus()"
#	CyPythonMgr().allowDefaultImpl()


######################################################################################################
########## Starting-Plot Functions
######################################################################################################
# ok = okLandPlots( xCoord, yCoord, ok=10 )
# ringworldAssignStartingPlots()
# plot = ringworldFindStartingPlot( playerID, validFn = None, startList = [] )
######################################################################################################

# Ensures that there are more than 3 land/hill plots within the central 3x3 grid
# and more than ok land/hill plots in the 5x5 grid around the starting-plot
def okLandPlots(xCoord, yCoord, ok=10):
	land1 = 0
	if ok >= 0:
		for x in range( -2, 3 ):
			for y in range( -2, 3 ):
				plot = plotXY( xCoord, yCoord, x, y )
				if not plot.isNone():
					if plot.isHills() or plot.isFlatlands():
						land1 += 1
		if land1 > ok:
			land2 = 0
			for x in range( -1, 2 ):
				for y in range( -1, 2 ):
					plot = plotXY( xCoord, yCoord, x, y )
					if not plot.isNone():
						if plot.isHills() or plot.isFlatlands():
							land2 += 1
			if land2 > 3:
				return True
	return False

# Assign starting-plots for all players
def ringworldAssignStartingPlots():
	global bestArea

	map.recalculateAreas()
	iPlayers = gc.getGame().countCivPlayersEverAlive()
	areas = CvMapGeneratorUtil.getAreas()
	areaValue = {}
	startDict = {}

	# show continent info
	sprint  = "[RingW] Area ID , Total Plots, Area Value, Start-Plots, Best Yield, Boni Total/Unique, River Edges, Coastal Land\n"
	sprint += "[RingW] --------,------------,-----------,------------,-----------,------------------,------------,-------------\n"
	sprint2 = []
	for area in areas:
		if area.isWater(): continue
		# build continent list
		areaValue[area.getID()] = ( 2 * area.calculateTotalBestNatureYield() +
			                         2 * area.getNumRiverEdges() + 2 * area.countCoastalLand() +
			                         5 * area.countNumUniqueBonusTypes() + area.getNumTotalBonuses() )
		aTotalPlots = area.getNumTiles()
		aStartPlots = area.getNumStartingPlots()
		aBestYield  = area.calculateTotalBestNatureYield()
		aTotalBoni  = area.getNumTotalBonuses()
		aUniqueBoni = area.countNumUniqueBonusTypes()
		aRiverEdges = area.getNumRiverEdges()
		aCoastLand  = area.countCoastalLand()
		sprint2.append( "[RingW] %9i,     %4i   ,  %5i    ,    %4i    ,   %5i   ,       %3i / %3i  ,    %4i    ,    %4i\n" % \
							 ( area.getID(), aTotalPlots, areaValue[area.getID()], aStartPlots, aBestYield, aTotalBoni, aUniqueBoni, aRiverEdges, aCoastLand ) )
	sprint2.sort( key = lambda test: test[20:30] )
	sprint2.reverse()
	for s in sprint2: sprint += s
	print sprint

	# Shuffle players so the same player doesn't always get the first pick.
	shuffledPlayers = mst.randomList.randomCountList( iPlayers )

	# Loop through players, assigning starts for each.
	assignedPlots = {}
	bSucceed = True
	print "[RingW] Find Starting Plots"
	for assign_loop in range(iPlayers):
		playerID = shuffledPlayers[assign_loop]
		player = gc.getPlayer(playerID)
		bestAreaValue = 0
		bestArea = -1
		for area in areas:
			if area.isWater(): continue
			value = areaValue[area.getID()] / (1 + 2*area.getNumStartingPlots() ) + 1
			if (value > bestAreaValue):
				bestAreaValue = value;
				bestArea = area.getID()

		sprint = "[RingW] bestAreaValue: %r , bestAreaID: %r\n" %( bestAreaValue-1, bestArea )

		#-----
		def isValid(iPlayer, x, y):
			plot = mst.GetPlot(x,y)
			if (plot.getArea() != bestArea):
				return False
			return True
		#-----

		sprint += "[RingW] FindStart: playerID %i" % (playerID)
		print sprint

		startList = [ (x,y) for x,y in startDict.values() ]
		sPlot = ringworldFindStartingPlot(playerID, isValid, startList)
		if sPlot != None:
			assignedPlots[playerID] = sPlot
			sPlot.setStartingPlot( True)
			player.setStartingPlot(sPlot, True)
			startDict[ playerID ] = ( sPlot.getX(), sPlot.getY() )
			continue
		# a player can't have a plot so will run a default assignment
		bSucceed = False
		break

	if not bSucceed :
		print "[RingW] WARNING! - Can't assgin a plot for each player : run default implementation "
		for playerID in assignedPlots.keys() :
			sPlot = assignedPlots[playerID]
			sPlot.setStartingPlot(False)
			player.setStartingPlot(sPlot,False)
		CyPythonMgr().allowDefaultImpl()
	else:
		sprint = ""
		for i in range(iPlayers):
			sprint += "[RingW] StartingPlot Player# %2i - @ (%i,%i)\n" % (i,startDict[i][0],startDict[i][1])
		print sprint

# Find starting-plot for player
def ringworldFindStartingPlot(playerID, validFn = None, startList = []):

	player = gc.getPlayer( playerID )
	player.AI_updateFoundValues( True )
	maxPlayers = gc.getMAX_CIV_PLAYERS()
	iRange = player.startingPlotRange()

	# Ringworld edge is only 2 (usually 3)
	xEdge = mst.iif( map.isWrapX(), 0, 2 )
	yEdge = mst.iif( map.isWrapY(), 0, 2 )

	# get scores for possible plots and sort them
	pList = [ [mst.GetPlot(x,y).getFoundValue(playerID), x, y]
					for x in range( xEdge, map.getGridWidth() - xEdge )
					for y in range( yEdge, map.getGridHeight() - yEdge )
					if ((x,y) not in startList) and
						not (validFn != None and not validFn(playerID, x, y))	]
	pList.sort()
	pList.reverse()

	# make passes with decreasing range
	sprint = ""
	for passes in range(40):
		landList = [ [val,mst.GetPlot(x,y)] for val,x,y in pList
													   if okLandPlots(x, y, 11-min(11,passes/12)) ]
		plotList = []
		for val,pl in landList:
			ok = True
			for i in range( maxPlayers ):
				if (i != playerID):
					if gc.getPlayer(i).startingPlotWithinRange(pl, playerID, iRange, passes):
						ok = False
						break
			if ok:
				plotList.append( [val, pl] )
		if len( plotList ) == 0:
			sprint += "[RingW] Player #%i - pass# %2i: FAILED \n" % (playerID, passes+1)
			continue
		elif len( plotList ) < 5:
			ch = 0
		else:
			ch = mst.chooseNumber( 2 )
		score = plotList[ch][0]
		plot  = plotList[ch][1]

		sprint += "[RingW] Player #%i, plotValue %i: SUCCESS at pass %2i - (%i,%i)" % (playerID, score, passes+1, plot.getX(), plot.getY())
		print sprint
		return plot

	# failed to find suitable plot
	sprint += "[RingW] Player #%i: FAILED to find starting-plot" % (playerID)
	print sprint
	return None

######################################
###  Helper Functions - check map  ###
######################################

# test if plot is part of lake
# -1-land/hills/peak, 0-no plot, 1-single-tile, 2-double-tile, 3-big lake/ocean
def checkLake( x, y ):
	plot = map.plot( x, y )
	if plot.isNone(): return 0							# no plot
	if not plot.isWater(): return -1					# no water
	lakeArea = map.getArea( plot.getArea() )
	lakeNum = lakeArea.getNumTiles()
	if lakeNum > 2: return 3							# big lake/ocean
	return lakeNum

# test if plot is Land beside Lake in W,E,S,N
def isBesideLake( x, y ):
	plot = map.plot( x, y )
	if plot.isWater(): return False
	for eCard in range( CardinalDirectionTypes.NUM_CARDINALDIRECTION_TYPES ):
		pl = plotCardinalDirection( x, y, CardinalDirectionTypes(eCard) )
		if pl.isNone(): continue
		if pl.isLake(): return True
	return False

# test how many peaks are in the neighborhood
# excluding the center plot
def numPeakNeighbors( x, y, data=None ):
	cnt = 0
	for dx in range(-1, 2):
		for dy in range(-1, 2):
			if (dx == 0) and (dy == 0): continue
			if data == None:
				plot = plotXY( x, y, dx, dy )
				if plot.isNone(): continue
				if plot.isPeak():
					cnt += 1
			else:
				i = GetIndex( x+dx, y+dy )
				if data[i] == PlotTypes.PLOT_PEAK:
					cnt += 1
	return cnt

# test how much water is in the neighborhood
# including the center plot
def numWaterNeighbors( x, y, dist=1, data=None ):
	cnt = 0
	for dx in range( -dist, dist+1 ):
		for dy in range( -dist, dist+1 ):
			if (dx == 0) and (dy == 0): continue
			if data == None:
				plot = plotXY( x, y, dx, dy )
				if plot.isNone(): continue
				if plot.isPeak():
					cnt += 1
			else:
				i = GetIndex( x+dx, y+dy )
				if data[i] == PlotTypes.PLOT_PEAK:
					cnt += 1
	return cnt

########################################
###  Helper Functions - general use  ###
########################################

# return aridity -4 .. 4
def getAridity():
	arid  = mst.marshMaker.getAridity()
	if mapOptions.Polar == 0: arid += -1
	elif mapOptions.Polar == 3: arid += 0
	else: arid += 1
	return arid

# convert x and y to an index (Auto-Wrap)
def GetIndex(x,y):
	while x < 0:
		x += CyMap().getGridWidth()
	xx = x % CyMap().getGridWidth()
	while y < 0:
		y += CyMap().getGridHeight()
	yy = y % CyMap().getGridHeight()
	return ( yy * CyMap().getGridWidth() + xx )

# test and pick
def iif( test, a, b ):
	if test: return a
	return b


############################################################
########## CLASS MapOptions - save and restore game-options
############################################################
# initialize()
# reloadGameOptions()
# saveGameOptions()
# showGameOptions()
############################################################
class MapOptions:
	def __init__(self):
		print "[RingW] ===== initialize Class::MapOptions"
		self.Balanced = 1					# default: Balanced
		self.RegionStrength = 2			# default: Moderate
		self.Polar = 0						# default: NorthPole to SouthPole
		self.WorldShape = 1				# default: Continents
		self.Teams = 0						# default: Team Neighbors
		self.mapDesc = {}					# description of map options in use

	# reload saved game options
	def reloadGameOptions(self):
		print "[RingW] -- MapOptions.reloadGameOptions()"
		gc = CyGlobalContext()
		for gam_string, bOpt in self.mapDesc.items():
			xStr = "gc.getGame().setOption(GameOptionTypes." + gam_string + "," + bOpt + ")"
			exec xStr

	# save game options for reloading
	def saveGameOptions(self):
		print "[RingW] -- MapOptions.saveGameOptions()"
		gc = CyGlobalContext()
		self.mapDesc = {}
		for gam in range(gc.getNumGameOptionInfos()):
			gam_string = gc.getGameOptionInfo(gam).getType()
			xStr = "gc.getGame().isOption(GameOptionTypes." + gam_string + ")"
			bOpt = "%r" % ( eval( xStr ) )
			self.mapDesc[gam_string] = bOpt
		#mst.printDict( self.mapDesc, "Game Options found:", prefix="[RingW] ")

	# print active game options
	def showGameOptions(self):
		print "[RingW] -- MapOptions.showGameOptions()"
		gc = CyGlobalContext()
		sprint = ""
		for gam in range(gc.getNumGameOptionInfos()):
			gam_string = gc.getGameOptionInfo(gam).getType()
			xStr = "gc.getGame().isOption(GameOptionTypes." + gam_string + ")"
			bOpt = eval( xStr )
			sVisible = "-"
			if mst.bBTS:
				if not gc.getGameOptionInfo(gam).getVisible():
					sVisible = "*"
			sprint += "[RingW] GameOption: #%2i %s [ %5r ] %s\n" % (gam,sVisible,bOpt,gam_string)
		print sprint

############################################################
########## CLASS MapOptions END
############################################################
mapOptions = MapOptions()
