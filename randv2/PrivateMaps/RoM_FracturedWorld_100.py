#
#	FILE:	 FracturedWorld_100_mst.py
#  ================================
#
#	AUTHOR:  Temudjin (2010)
#  ###########################################################################################
#	this map is also fully compatible with 'Planetfall' by Maniac
#	- Civilization Fanatics (http://forums.civfanatics.com/showthread.php?t=252829)
#	>>>>> first put MapScriptTools.py into the ..\Beyond the Sword\Assets\Python folder <<<<<
#	>>>>> then put this file into the PrivateMaps folder of that mod and select it      <<<<<
#  ###########################################################################################


#	PURPOSE: Civ4 Map Script
#  ========================
#
#	FracturedWorld is a random map script.
#  Initially the plots are very randomly distributed.
#  When the eons come and go, the world slowly changes as the continents drift,
#  break apart, new isles come to be and are driven into the continents.
#  You choose the number of eons to wait for the world to grow older and the rest is left to fate.
#  (Well not quite, as the target plot and terrain distrubution needs constant fiddling.)
#
#	Note: This script saves the map options in the log directory in 'FracturedWorld.cfg'
#
#	This script uses 'MapScriptTools' by Temudjin --> ???

#  ===================
#  Temudjin 15.July.10
#  ===================

#	Changelog
#	---------
#	1.00					initial release
#

def getVersion():
	return "1.00"

def getDescription():
	return "\nEvolving Map"

from CvPythonExtensions import *
import CvMapGeneratorUtil
import pickle

# universal constants
gc		  = CyGlobalContext()
map	  = CyMap()
game	  = CyGame()
cfgFile = 'FracturedWorld.cfg'

################################################################################
## MapScriptTools Interface by Temudjin
################################################################################
"""
For Reference:
----------------------------------------------------------------
The Map Building Process according to Temudjin
    --> also see Bob Thomas in "CvMapScriptInterface.py"
    (in ..\Assets\Python\EntryPoints)
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

import MapScriptTools as mst
balancer = mst.bonusBalancer

# #######################################################################################
# ######## beforeGeneration() - Called from system after 'Initialize Map'
# ######## - define your latitude formula, get the map-version
# ######## - initialize the MapScriptTools and initialize MapScriptTools.BonusBalancer
# #######################################################################################
def beforeGeneration():
	print "[FracW] -- beforeGeneration()"

	# save map-options
	mapOptions.Shape = map.getCustomMapOption(0)
	mapOptions.Water = map.getCustomMapOption(1)
	mapOptions.Peaks = map.getCustomMapOption(2)
	mapOptions.Terrain = map.getCustomMapOption(3)
	mapOptions.Age = map.getCustomMapOption(4)
	mapOptions.Teams = map.getCustomMapOption(5)
	if mst.bMars:
		mapOptions.Theme = map.getCustomMapOption(5)
		mapOptions.Teams = map.getCustomMapOption(6)

	mapOptions.Desert = int( mapOptions.descTerrList[mapOptions.Terrain][0:2] )
	mapOptions.Plains = int( mapOptions.descTerrList[mapOptions.Terrain][6:8] )
	if bMarsh:
		mapOptions.Marsh = int( mapOptions.descTerrList[mapOptions.Terrain][13:14] )
	else:
		mapOptions.Marsh = 0

	# Create mapInfo string
	mapInfo = ""
	for opt in range( getNumCustomMapOptions() ):
		nam = getCustomMapOptionName( [opt] )
		sel = CyMap().getCustomMapOption( opt )
		txt = getCustomMapOptionDescAt( [opt,sel] )
		mapInfo += "%27s:   %s\n" % ( nam, txt )
	# Initialize MapScriptTools
	mst.getModInfo( getVersion(), None, mapInfo )
	# Initialize MapScriptTools.BonusBalancer
	balancer.initialize( True, True, True )		#	balance boni, place missing boni, move minerals
	# Initialize MapScriptTools.MapRegions, don't use landmarks for FFH
#	mst.mapRegions.initialize( noSigns = mst.bFFH )

	# Save game-options
	mapOptions.saveGameOptions()
	mapOptions.showGameOptions()
	# Save map-options to file
	mapOptions.writeConfig()

# #######################################################################################
# ######## generateTerrainTypes() - Called from system after generatePlotTypes()
# ######## - SECOND STAGE in 'Generate Map'
# ######## - creates an array of terrains (desert,plains,grass,...) in the map dimensions
# #######################################################################################
def generateTerrainTypes():
	print "[FracW] -- generateTerrainTypes()"
	mst.mapPrint.buildTerrainMap( True, "generateTerrainTypes()" )

	# Evolve Planet
	#timer.t("evolvePlanet(),0")
	eons = mapOptions.ageEons[ mapOptions.Age ]
	plateTectonics.evolvePlanet( eons )
	#timer.t("evolvePlanet()")

	# Buils mountain ranges
	rangifyMountains( passes = iif(map.getClimate() == 3, 4, 2) )		# rocky climate

	# Build highlands if 'Hilly' or 'Peaky'
	if ( map.getClimate() == 3 ) or ( mapOptions.peakPlots[mapOptions.Peaks] >= 10 ):		# rocky, scenic, hilly or peaky
		buildHighlands()
	# Planetfall: more highlands and ridges
	mst.planetFallMap.buildPfallHighlands()

	# Generate terrain
	if mst.bPfall:
		rebuildCoast()
		mapOptions.terGen = mst.MST_TerrainGenerator( mapOptions.Desert, mapOptions.Plains )
	elif mst.bMars and (mapOptions.Theme == 0):
		mapOptions.terGen = mst.MST_TerrainGenerator( mapOptions.Desert / 2, mapOptions.Plains )
	else:
		mapOptions.terGen = FracturedWorldTerrainGenerator(mapOptions.Desert, mapOptions.Plains, mapOptions.Marsh)
	terrainTypes = mapOptions.terGen.generateTerrain()
	return terrainTypes

# #######################################################################################
# ######## addRivers() - Called from system after generateTerrainTypes()
# ######## - THIRD STAGE in 'Generate Map'
# ######## - puts rivers on the map
# #######################################################################################
def addRivers():
	print "[FracW] -- addRivers()"

	# Generate DeepOcean-terrain if mod allows for it
	mst.deepOcean.buildDeepOcean()
	# Planetfall: handle shelves and trenches
	mst.planetFallMap.buildPfallOcean()
	# Generate marsh-terrain
	mst.marshMaker.convertTerrain()
	# Solidify marsh before building bogs
	if bMarsh:
		marshPer = mapOptions.Marsh
		mst.mapPrettifier.percentifyTerrain( (mst.etMarsh,marshPer), (mst.etTundra,2), (mst.etGrass,3) )

	# Build between 0..3 mountain-ranges.
	mst.mapRegions.buildBigDents()
	# Build between 0..3 bog-regions.
	mst.mapRegions.buildBigBogs()
	# Adjust peak plots
	fPeaks = mapOptions.peakPlots[mapOptions.Peaks] * ( 100.0 - mapOptions.waterPlots[mapOptions.Water] ) / 95.0
	mst.mapPrettifier.percentifyPlots( PlotTypes.PLOT_PEAK, fPeaks, None, mapOptions.terGen )

	# Solidify desert
	if not mst.bPfall:
		desertPer = mapOptions.Desert
		mst.mapPrettifier.percentifyTerrain( (mst.etDesert,desertPer), (mst.etPlains,5), (mst.etGrass,2) )
		plainsPer = mapOptions.Plains
		mst.mapPrettifier.percentifyTerrain( (mst.etPlains,plainsPer), (mst.etGrass,1) )
		# create connected deserts and plains
		mst.mapPrettifier.lumpifyTerrain( mst.etDesert, mst.etPlains, mst.etGrass )
		mst.mapPrettifier.lumpifyTerrain( mst.etGrass, mst.etTundra, mst.etPlains )
		mst.mapPrettifier.lumpifyTerrain( mst.etPlains, mst.etDesert, mst.etGrass )

	# No rivers for 'Mars Now!' (usually)
	if (not mst.bMars) or (mapOptions.Theme == 1):
		# Kill of spurious lakes
		mst.mapPrettifier.connectifyLakes( 90 )
		# Sprout rivers from lakes (less ocean more rivers)
		mst.riverMaker.buildRiversFromLake( chRiver = 60 + 5 * mapOptions.Water )
		# Put rivers on the map.
		CyPythonMgr().allowDefaultImpl()
		# Put rivers on small islands
		mst.riverMaker.islandRivers()

# #######################################################################################
# ######## addLakes() - Called from system after addRivers()
# ######## - FOURTH STAGE in 'Generate Map'
# ######## - puts lakes on the map
# #######################################################################################
# No additional lakes for FracturedWorld
def addLakes():
	print "[FracW] -- addLakes()"
	return

# #######################################################################################
# ######## addFeatures() - Called from system after addLakes()
# ######## - FIFTH STAGE in 'Generate Map'
# ######## - puts features on the map
# #######################################################################################
def addFeatures():
	print "[FracW] -- addFeatures()"

	featuregen = mst.MST_FeatureGenerator()
	featuregen.addFeatures()

	mst.mapPrettifier.beautifyVolcanos()			# transform some volcanos	- all
	mst.mapRegions.buildElementalQuarter()			# build ElementalQuarter	- FFH only

# #######################################################################################
# ######## addBonuses() - Called from system after addFeatures()
# ######## - SIXTH STAGE in 'Generate Map'
# ######## - puts boni on the map
# #######################################################################################
#def addBonuses():
#	print "[FracW] -- addBonuses()"
#	CyPythonMgr().allowDefaultImpl()

# ######################################################################################################
# ######## assignStartingPlots() - Called from system
# ######## - assign starting positions for each player after the map is generated
# ######## - Planetfall has GameOption 'SCATTERED_LANDING_PODS' - use default implementation
# ######################################################################################################
def assignStartingPlots():
	print "[FracW] -- assignStartingPlots()"
	if mst.bPfall:
		CyPythonMgr().allowDefaultImpl()
	else:
		fracworldAssignStartingPlots()

# ############################################################################################
# ######## normalizeStartingPlotLocations() - Called from system after starting-plot selection
# ######## - FIRST STAGE in 'Normalize Starting-Plots'
# ######## - change assignments to starting-plots
# ############################################################################################
def normalizeStartingPlotLocations():
	print "[FracW] -- normalizeStartingPlotLocations()"

	# build Lost Isle
	# - this region needs to be placed after starting-plots are first assigned
	mst.mapRegions.buildLostIsle( 60, minDist=7, bAliens=mst.choose(33,True,False) )

	# shuffle starting-plots for teams
	if mapOptions.Teams == 0:
		CyPythonMgr().allowDefaultImpl()						# by default civ places teams near to each other
	elif mapOptions.Teams == 1:
		mst.teamStart.placeTeamsTogether( False, True )	# shuffle starting-plots to separate teams
	else:
		mst.teamStart.placeTeamsTogether( True, True )	# randomize starting-plots

	# Adjust starting-plots for 'One City Challange'
	occStart.occHumanStart()

# ############################################################################################
# ######## normalizeAddRiver() - Called from system after normalizeStartingPlotLocations()
# ######## - SECOND STAGE in 'Normalize Starting-Plots'
# ######## - add some rivers if needed
# ############################################################################################
def normalizeAddRiver():
	print "[FracW] -- normalizeAddRiver()"
	if (not mst.bMars) or (mapOptions.Theme == 1):
		CyPythonMgr().allowDefaultImpl()

#def normalizeRemovePeaks():
#	return None

# ############################################################################################
# ######## normalizeAddLakes() - Called from system after normalizeRemovePeaks()
# ######## - FOURTH STAGE in 'Normalize Starting-Plots'
# ######## - add some lakes if needed
# ############################################################################################
def normalizeAddLakes():
	print "[FracW] -- normalizeAddLakes()"
	if (not mst.bMars) or (mapOptions.Theme == 1):
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

# ############################################################################################
# ######## normalizeAddFoodBonuses() - Called from system after normalizeRemoveBadTerrain()
# ######## - SEVENTH STAGE in 'Normalize Starting-Plots'
# ######## - make sure everyone has enough to eat
# ############################################################################################
def normalizeAddFoodBonuses():
	print "[FracW] -- normalizeAddFoodBonuses()"
	CyPythonMgr().allowDefaultImpl()

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
	print "[FracW] -- normalizeAddExtras()"

	# Balance boni, place missing boni and move minerals
	balancer.normalizeAddExtras()

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
	mst.mapStats.statPlotCount( "" )
	mst.mapPrint.buildAreaMap( True, "normalizeAddExtras()" )
#	mst.mapPrint.buildPlotMap( True, "normalizeAddExtras()" )
	mst.mapPrint.buildTerrainMap( False, "normalizeAddExtras()" )
#	mst.mapPrint.buildFeatureMap( True, "normalizeAddExtras()" )
#	mst.mapPrint.buildBonusMap( False, "normalizeAddExtras()" )
#	if mst.bFFH: mst.mapPrint.buildBonusMap( False, "Mana", None, mst.mapPrint.manaDict )
	mst.mapPrint.buildRiverMap( False, "normalizeAddExtras()" )
	mst.mapStats.mapStatistics()

# ############################################################################
# ######## minStartingDistanceModifier() - Called from system at various times
# ######## - FIRST STAGE in 'Select Starting-Plots'
# ######## - adjust starting-plot distances
# ############################################################################
def minStartingDistanceModifier():
#	print "[FracW] -- minStartingDistanceModifier()"
	if mst.bPfall: return -25
	return 10
################################################################################


################################################################
### Custom Map Options
################################################################
def isBonusIgnoreLatitude():
#	print "[FracW] -- isBonusIgnoreLatitude()"
	return False

def getNumHiddenCustomMapOptions():
	""" Default is used for the last n custom-options in 'Play Now' mode. """
#	print "[FracW] -- getNumHiddenCustomMapOptions()"

	# ---------------------------------------
	# first opportunity to getcustom options
	if not mapOptions.hasReadConfig:
		# read map options
		mapOptions.readConfig()
		# read game options if available
		if len(mapOptions.mapDesc.keys()) > 0:
			mapOptions.reloadGameOptions()
			mapOptions.showGameOptions()
	# ---------------------------------------

	return  mst.iif( mst.bMars, 0, 1 )

def getNumCustomMapOptions():
#	print "[FracW] -- getNumCustomMapOptions()"
	return 6

def getCustomMapOptionName(argsList):
#	print "[FracW] -- getCustomMapOptionName()"
	[iOption] = argsList
	option_names =	{
						0: "World Shape",
						1:	iif(mst.bMars,"Desert","Water") + " (% of Plots)",
						2:	"Peaks (% of Land)",
						3:	mapOptions.descTerrain,
						4:	"World Age",
						5:	"Team Start"
						}
	if mst.bMars:
		option_names[5] = "Marsian Theme"
		option_names[6] = "Team Start"
	translated_text = unicode(CyTranslator().getText(option_names[iOption], ()))
	return translated_text

def getNumCustomMapOptionValues(argsList):
#	print "[FracW] -- getNumCustomMapOptionValues()"
	[iOption] = argsList
	option_values =	{
							0:	 4,
							1:	10,
							2:  5,
							3:  5,
							4: 10,
							5:  3
							}
	if mst.bMars:
		option_values[5] = 2
		option_values[6] = 3
	return option_values[iOption]

def getCustomMapOptionDescAt(argsList):
#	print "[FracW] -- getCustomMapOptionDescAt()"
	[iOption, iSelection] = argsList
	selection_names =	{
							0:	{
								0: "Flat",
								1: "Cylinder (x-wrap)",
								2: "Tube (y-wrap)",
								3: "Toroid (x,y-wrap)"
								},
							1:	{
								0: " 85% Islandworld",
								1: " 80% Waterworld",
								2: " 75% Oceanworld",
								3: " 70% Earthlike",
								4: " 65% Continental",
								5: " 60% Seaworld",
								6: " 50% Spiderworld",
								7: " 40% Pangaea",
								8: " 30% Lakeworld",
								9: " 20% Pondworld"
								},
							2:	{
								0: "  3% Flat",
								1: "  6% Uneven",
								2: " 10% Scenic",
								3: " 15% Hilly",
								4: " 20% Peaky"
								},
							3:	{
								0: mapOptions.descTerrList[0],
								1: mapOptions.descTerrList[1],
								2: mapOptions.descTerrList[2],
								3: mapOptions.descTerrList[3],
								4: mapOptions.descTerrList[4]
								},
							4:	{
								0: "Beginning",
								1: "  3 Eons",
								2: "  6 Eons",
								3: " 12 Eons",
								4: " 18 Eons",
								5: " 24 Eons",
								6: " 36 Eons",
								7: " 48 Eons",
								8: " 72 Eons",
								9: " 96 Eons"
								},
							5:	{
								0: "Team Neighbors",
								1: "Team Separated",
								2: "Randomly Placed"
								}
							}
	if mst.bMars:
		selection_names[5] = { 0: "Sands of Mars",
									  1: "Terraformed Mars" }
		selection_names[6] = { 0: "Team Neighbors",
									  1: "Team Separated",
									  2: "Randomly Placed" }
	translated_text = unicode(CyTranslator().getText(selection_names[iOption][iSelection], ()))
	return translated_text

# reload FracturedWorld UserSettings from last session or default
def getCustomMapOptionDefault(argsList):
#	print "[FracW] -- getCustomMapOptionDefault()"
	[iOption] = argsList
	option_defaults = {
							0:	mapOptions.Shape,
							1:	mapOptions.Water,
							2: mapOptions.Peaks,
							3: mapOptions.Terrain,
							4: mapOptions.Age,
							5: mapOptions.Teams
							}
	if mst.bMars:
		option_defaults[5] = mapOptions.Theme
		option_defaults[6] = mapOptions.Teams
	return option_defaults[iOption]

def isRandomCustomMapOption(argsList):
#	print "[FracW] -- isRandomCustomMapOption()"
	[iOption] = argsList
	option_random =	{
							0: False,
							1: True,
							2: True,
							3: True,
							4: True,
							5: False
							}
	if mst.bMars:
		option_random[6] = False
	return option_random[iOption]

def isAdvancedMap():
#	print "[FracW] -- isAdvancedMap()"
	"This map should show up in simple mode"
	return 0

def isClimateMap():
#	print "[FracW] -- isClimateMap()"
	return 1
def isSeaLevelMap():
#	print "[FracW] -- isSeaLevelMap()"
	return 0

def beforeInit():
	print "[FracW] -- beforeInit()"
	# allow default pre initialization
	CyPythonMgr().allowDefaultImpl()

def getGridSize(argsList):
	print "[FracW] -- getGridSize()"
	if (argsList[0] == -1): # (-1,) is passed to function on loads
		return []

	grid_sizes =	{
						WorldSizeTypes.WORLDSIZE_DUEL:      ( 8, 8),
						WorldSizeTypes.WORLDSIZE_TINY:      (12,10),
						WorldSizeTypes.WORLDSIZE_SMALL:     (16,12),
						WorldSizeTypes.WORLDSIZE_STANDARD:  (22,16),
						WorldSizeTypes.WORLDSIZE_LARGE:     (28,20),
						WorldSizeTypes.WORLDSIZE_HUGE:      (34,24)
						}

	bigGrids = [ (40,28), (44,32), (48,36) ]				# just change these, or even add some more
	maxWorld = CyGlobalContext().getNumWorldInfos()
	maxSize = min( 6 + len( bigGrids ), maxWorld )
	for i in range( 6, maxSize ):
		grid_sizes[ i ] = bigGrids[ i - 6 ]
	mst.printDict( grid_sizes, "GridSizes:", prefix = "[FracW] " )

	[eWorldSize] = argsList
	return grid_sizes[eWorldSize]

def getTopLatitude():
#	print "[FracW] -- getTopLatitude()"
	lat = 85
	if mapOptions.Shape == 3: lat -= 5					# Torodial
	if CyMap().getClimate() == 1: lat -= 10			# Tropical
	elif CyMap().getClimate() == 4: lat += 5			# Cold
	if mst.bMars: lat = max( 75, lat )
	return lat
def getBottomLatitude():
#	print "[FracW] -- getBottomLatitude()"
	lat = 85
	if mapOptions.Shape == 3: lat -= 5					# Torodial
	if CyMap().getClimate() == 1: lat -= 10			# Tropical
	elif CyMap().getClimate() == 4: lat += 5			# Cold
	if mst.bMars: lat = max( 75, lat )
	return -lat

def getWrapX():
#	print "[FracW] -- getWrapX()"
	return ( mapOptions.Shape in [1,3] )
def getWrapY():
#	print "[FracW] -- getWrapY()"
	return ( mapOptions.Shape in [2,3] )


# ###################################################################################
# ######## generatePlotTypes() - Called from system
# ######## - FIRST STAGE in building the map
# ######## - creates an array of plots (ocean,hills,peak,land) in the map dimensions
# ###################################################################################
def generatePlotTypes():
	print "[FracW] -- generatePlotTypes()"

	# fix the water and land mix
	# --------------------------
	plotTypes = []
	global chWater, chPeak, chFlat, chHills
	# global nPeaks, nHills, nFlat, nWater, nLand
	chWater = mapOptions.waterPlots[ mapOptions.Water ]
	chPeak = mapOptions.peakPlots[ mapOptions.Peaks ]
	nWater = map.numPlots() * chWater / 100
	nPeaks = ( map.numPlots() - nWater ) * chPeak / 100
	nFlat  = ( map.numPlots() - nWater - nPeaks ) * 3 / 4
	nHills = map.numPlots() - nWater - nPeaks - nFlat
	nLand = nPeaks + nHills + nFlat
	chFlat = nFlat * 100 / nLand
	chHills = chFlat + nHills * 100 / nLand

	# generate initial map
	plotList = [PlotTypes.PLOT_LAND] * nLand
	plotList.extend( [PlotTypes.PLOT_OCEAN] * nWater )
	plotTypes = mst.randomList.xshuffle( plotList )
	plateTectonics.inverted = 0
	chActualWater = chWater
	if chWater < 45:
		plateTectonics.invertMap( plotTypes )
		chActualWater = 100 - chWater
		print "[FracW] Map inverted"
#	mst.mapPrint.buildPlotMap( False, "generatePlotTypes()_1", None, plotTypes )
	w,f,h,p = plotCounter( plotTypes )

	lLump = int( round( w * 10 / map.numPlots() ) ) - 5
	wLump = 2 * ( 8 - int( round( w * 10 / map.numPlots() ) ) )
	plotTypes = lumpifyLands( 1, plotTypes )
	plotTypes = lumpifyOcean( 1, plotTypes )
	plotTypes = lumpifyLands( lLump, plotTypes )
	plotTypes = lumpifyOcean( wLump, plotTypes )
#	mst.mapPrint.buildPlotMap( False, "generatePlotTypes()_2", None, plotTypes )
	plotCounter( plotTypes )

	plotTypes = waterEdges( plotTypes )
	plotTypes = mst.mapPrettifier.percentifyPlots( PlotTypes.PLOT_OCEAN, chActualWater, plotTypes )
	plotTypes = lumpifyLands( lLump, plotTypes, 5 )
#	mst.mapPrint.buildPlotMap( False, "generatePlotTypes()_3", None, plotTypes )
	plotCounter( plotTypes )

	plotTypes = orphanPlots.moveOrphanPlots( plotTypes, 6, 75 )
	plotTypes = orphanPlots.moveOrphanPlots( plotTypes, 2, 50, True )		# move lakes
	plotTypes = removeSmallIslands( plotTypes )
#	mst.mapPrint.buildPlotMap( False, "generatePlotTypes()_4", None, plotTypes )
	plotCounter( plotTypes )
	return plotTypes

def pickLandType():
	# global chFlat, chHills
	return mst.chooseMore( (chFlat, PlotTypes.PLOT_LAND), (chHills, PlotTypes.PLOT_HILLS), (100, PlotTypes.PLOT_PEAK) )

# print plot-mix
def plotCounter( data=None, noPrint=False ):
	iWater = 0
	iFlat  = 0
	iHills = 0
	iPeaks = 0
	if data == None:
		for i in range( map.numPlots() ):
			if map.plotByIndex( i ).isPeak(): iPeaks += 1
			elif map.plotByIndex( i ).isHills(): iHills += 1
			elif map.plotByIndex( i ).isFlatlands(): iFlat += 1
			elif map.plotByIndex( i ).isWater(): iWater += 1
	else:
		for pl in data:
			if pl == PlotTypes.PLOT_PEAK: iPeaks += 1
			elif pl == PlotTypes.PLOT_HILLS: iHills += 1
			elif pl == PlotTypes.PLOT_LAND: iFlat += 1
			elif pl == PlotTypes.PLOT_OCEAN: iWater += 1
	pW = iWater * 100.0 / map.numPlots()
	pF = iFlat  * 100.0 / map.numPlots()
	pH = iHills * 100.0 / map.numPlots()
	pP = iPeaks * 100.0 / map.numPlots()
	sprint  = "[FracW] Plot-Mix: Water %i(%4.2f%s), Flat %i(%4.2f%s), Hills %i(%4.2f%s), Peaks %i(%4.2f%s)\n" % (iWater,pW,'%%', iFlat,pF,'%%', iHills,pH,'%%', iPeaks,pP,'%%')
	nL = iFlat + iHills + iPeaks
	if nL > 0:
		lF = iFlat * 100.0 / nL
		lH = iHills * 100.0 / nL
		lP = iPeaks * 100.0 / nL
		sprint += "[FracW]           Flat %4.2f%s of Land, Hills %4.2f%s of Land, Peaks %4.2f%s of Land" % ( lF,'%%', lH,'%%', lP,'%%' )
	if not noPrint: print sprint
	return ( iWater, iFlat, iHills, iPeaks )

# recalculate coast/ocean terrain
def rebuildCoast():
	for inx in range( map.numPlots() ):
		pl = map.plotByIndex( inx )
		if pl.isWater():
			x,y = mst.coordByIndex( inx )
			n = mst.numWaterNeighbors(x, y)
			ter = mst.iif( n == 8, mst.etOcean, mst.etCoast )
			if pl.getTerrainType() != ter:
				pl.setTerrainType(ter, False, False)

# make water-edges
def waterEdges( data ):
	print "[FracW] -- waterEdges()"
	for x in range( mst.iNumPlotsX ):
		for y in [ 0, mst.iNumPlotsY-1 ]:
			data[ map.plotNum(x,y) ] = PlotTypes.PLOT_OCEAN
	for x in [ 0, mst.iNumPlotsX-1 ]:
		for y in range( mst.iNumPlotsY ):
			data[ map.plotNum(x,y) ] = PlotTypes.PLOT_OCEAN
	return data

# change flatland/hills surrounded by ocean to ocean
def lumpifyOcean( passes, data=None, chChange=66 ):
	print "[FracW] -- lumpifyOcean()"
	cnt = 0
	if chChange < 0: chChange = 0
	elif chChange > 80: chChange = 80
	for loop in range( passes ):
		for inx in range( map.numPlots() ):
			x, y = mst.coordByIndex( inx )
			if data == None:
				plot = map.plotByIndex( inx )
				if not plot.isWater():
					tarNum = mst.numWaterNeighbors( x, y, 1 )
					if tarNum > 4:
						if mst.choose( chChange + 5*(tarNum-5), True, False):
							plot.setPlotType( PlotTypes.PLOT_OCEAN, False, False )
							cnt += 1
			else:
				if data[inx] == PlotTypes.PLOT_LAND:
					tarNum = mst.numWaterNeighbors( x, y, 1, data )
					if tarNum > 4:
						if mst.choose( chChange + 5*(tarNum-5), True, False):
							data[inx] = PlotTypes.PLOT_OCEAN
							cnt += 1
	if cnt > 0: print "[FracW] %i water-plots lumpified" % ( cnt )
	return data

# change ocean surrounded by land to land
def lumpifyLands( passes, data=None, chChange=60 ):
	print "[FracW] -- lumpifyLands()"
	cnt = 0
	if chChange < 0: chChange = 0
	elif chChange > 75: chChange = 75
	for loop in range( passes ):
		for inx in range( map.numPlots() ):
			x, y = mst.coordByIndex( inx )
			if data == None:
				plot = map.plotByIndex( inx )
				if plot.isWater():
					tarNum = mst.numWaterNeighbors( x, y, 1 )
					if tarNum < 5:
						if mst.choose( chChange + 5*(4-tarNum), True, False):
							plot.setPlotType( PlotTypes.PLOT_LAND, False, False )
							cnt += 1
			else:
				if data[inx] == PlotTypes.PLOT_OCEAN:
					tarNum = mst.numWaterNeighbors( x, y, 1, data )
					if tarNum < 5:
						if mst.choose( chChange + 5*(4-tarNum), True, False):
							data[inx] = PlotTypes.PLOT_LAND
							cnt += 1
	if cnt > 0:  print "[FracW] %i land-plots lumpified" % ( cnt )
	return data

# move lonly peaks toward other peaks
def rangifyMountains( chMove=33, passes=1 ):
	cnt = 0
	for loop in range( passes ):
		for inx in range( map.numPlots() ):
			plot = map.plotByIndex( inx )
			if plot.isPeak():
				if mst.numPeakNeighbors( plot.getX(), plot.getY() ) == 0:
					if mst.choose( chMove, True, False ):
						pl = findLandPlotNearMountain( plot.getX(), plot.getY() )
						if pl:
							plot, pl = pl, plot
							cnt += 1
	print "[FracW] %i Mountains moved in %i passes" % (cnt, passes)

# find plot near mountain
def findLandPlotNearMountain( x, y, ringDist=2 ):
	pHeight = []
	for dx in range(-ringDist, 1+ringDist):
		for dy in range(-ringDist, 1+ringDist):
			pDist = plotDistance( x, y, x+dx, y+dy )
			if pDist == ringDist:
				plot = plotXY( x, y, dx, dy)
				if not plot.isNone():
					if plot.isFlatlands() or plot.isHills():
						n = mst.numPeakNeighbors( x+dx, y+dy )
						for i in range(n):
							pHeight.append( plot )
	# return random plot from list or None if empty
	return mst.chooseListElement( pHeight )

# build some highlands beside the existing peaks and hills
def buildHighlands():
	print "[FracW] -- buildHighlands()"
	nChance = 25 * mst.iif( map.getClimate()==3, 3, 1 ) / 10
	# build highlands and foothills
	for inx in range( map.numPlots() ):
		plot = map.plotByIndex( inx )
		if plot.isHills() or plot.isPeak():
			x,y = mst.coordByIndex( inx )
			for iDir in range( DirectionTypes.NUM_DIRECTION_TYPES ):
				p = savePlotDirection( x, y, iDir )
				if p.isFlatlands():
					p.setPlotType( mst.choose(nChance, PlotTypes.PLOT_HILLS, PlotTypes.PLOT_LAND), True, True )
				elif p.isHills():
					p.setPlotType( mst.choose(nChance/2, PlotTypes.PLOT_PEAK, PlotTypes.PLOT_HILLS), True, True )

# remove or improve most 1- or 2-plot islands
def removeSmallIslands( aPlots ):
	print "[FracW] -- removeSmallIslands()"
	for x in range( mst.iNumPlotsX ):
		for y in range( mst.iNumPlotsY ):
			i = mst.GetIndex(x,y)
			if aPlots[i] == PlotTypes.PLOT_OCEAN: continue
			iLand = checkIsland(x, y, aPlots)
			if iLand == 1:
				fx,fy = mst.chooseMore( (20,(x,y+1)), (40,(x,y-1)), (70,(x+1,y)), (100,(x-1,y)) )
				if (fy>2) and (fy<13):
					if mst.choose( 75, True, False ):
#						print "[FracW] Single Island Doubled: x,y %i,%i - fx,fy %i,%i" % (x,y,fx,fy)
						i = mst.GetIndex(fx,fy)
						aPlots[i] = PlotTypes.PLOT_LAND
						iLand = checkIsland(x, y, aPlots)								# maybe it's no island anymore
			if (iLand == 1) or (iLand == 2):
				if mst.choose( 15 + 10 * iLand, False, True ):
					# There is more than one way to kill small islands
					if mst.choose( 33, True, False ):
						if iLand==1:
#							print "[FracW] Single Island Stamped Out: x,y %i,%i" % (x,y)
							j = mst.GetIndex(x,y)
							aPlots[j] = PlotTypes.PLOT_OCEAN
						else:
#							print "[FracW] Double Island Stamped Out: x,y %i,%i" % (x,y)
							for fx in range(x-1, x+2):
								for fy in range(y-1, y+2):
									i = mst.GetIndex(fx,fy)
									aPlots[i] = PlotTypes.PLOT_OCEAN
					else:
#						print "[FracW] Island Enhanced: x,y %i,%i" % (x,y)
						for fx in range(x-1, x+2):
							for fy in range(y-1, y+2):
								if (fx==x) and (fy==y): continue
								i = mst.GetIndex(fx,fy)
								if (fy<=1) or (fy>=(mst.iNumPlotsY-2)):
									aPlots[i] = PlotTypes.PLOT_OCEAN
								if (fy==2) or (fy==(mst.iNumPlotsY-3)):
									aPlots[i] = mst.choose( 50, PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN )
								else:
									aPlots[i] = mst.choose( 90, PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN )
				elif mst.choose( 10, True, False ):
#					print "[FracW] Island Enlarged: x,y %i,%i" % (x,y)
					for fx in range(x, x+2):
						for fy in range(y, y+2):
							if (fx==x) and (fy==y): continue
							i = mst.GetIndex(fx,fy)
							if (fy<=1) or (fy>=(mst.iNumPlotsY-2)):
								aPlots[i] = PlotTypes.PLOT_OCEAN
							if (fy==2) or (fy==(mst.iNumPlotsY-3)):
								aPlots[i] = mst.choose( 50, PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN )
							else:
								aPlots[i] = mst.choose( 70, PlotTypes.PLOT_LAND, PlotTypes.PLOT_OCEAN )
	return aPlots

# test if plot is part of island
# -1-ocean/lake, 1-single-tile, 2-double-tile, 3-big island
def checkIsland(xCoord, yCoord, aPlots):
	j = mst.GetIndex(xCoord,yCoord)
	if aPlots[j] == PlotTypes.PLOT_OCEAN:
		return -1														# ocean or lake

	land = []
	iLand = 0
	for x in range(xCoord-1, xCoord+2):
		for y in range(yCoord-1, yCoord+2):
			i = mst.GetIndex(x,y)
			if not (aPlots[i] == PlotTypes.PLOT_OCEAN):
				iLand += 1
				land.append( (x,y) )
	if iLand == 1:
#		print "[FracW] Single Island Found: x,y %i,%i" % (xCoord,yCoord)
		return 1															# single-tile island
	if iLand == 2:
		fx,fy = land[0]
		if (fx==xCoord) and (fy==yCoord):
			fx,fy = land[1]
		iLand = 0
		for x in range(fx-1, fx+2):
			for y in range(fy-1, fy+2):
				i = mst.GetIndex(x,y)
				if not (aPlots[i] == PlotTypes.PLOT_OCEAN):
					iLand += 1
		if iLand == 2:
#			print "[FracW] Double Island Found: x,y %i,%i" % (xCoord,yCoord)
			return 2														# double-tile island
	return 3																# big island

##########################
###  Helper Functions  ###
##########################

# test and pick
def iif( test, a, b ):
	if test: return a
	return b

# give plot in given direction; Autowrap
def savePlotDirection( x0, y0, eDir ):
	dx, dy = mst.xyDirection(eDir % 8)
	return mst.GetPlot(x0 + dx, y0 + dy)

# give plot in given direction; Autowrap
def savePlotCardinalDirection( x0, y0, eCard ):
	dx, dy = mst.xyCardinalDirection(eCard % 4)
	return mst.GetPlot(x0 + dx, y0 + dy)

# ---------------------------------------------------------------------------------------------------------
# TerrainGenerator from 'Thomas' War' by tsentom1 --> http://forums.civfanatics.com/showthread.php?t=281603
# defaults were slightly changed
# ---------------------------------------------------------------------------------------------------------
class FracturedWorldTerrainGenerator:
	"If iDesertPercent=35, then about 35% of all land will be desert. Plains is similar. \
	Note that all percentages are approximate, as values have to be roughened to achieve a natural look."

	def __init__(self, iDesertPercent=31, iPlainsPercent=17, iMarshPercent=3,
	             fSnowLatitude=0.72, fTundraLatitude=0.64,
	             fGrassLatitude=0.12, fDesertBottomLatitude=0.24,
	             fDesertTopLatitude=0.54, fracXExp=-1,
	             fracYExp=-1, grain_amount=4):

		print "[FracW] ===== initialize Class::FracturedWorldTerrainGenerator"

		self.gc = CyGlobalContext()
		self.map = CyMap()

		grain_amount += self.gc.getWorldInfo(self.map.getWorldSize()).getTerrainGrainChange()

		self.grain_amount = grain_amount

		self.iWidth = self.map.getGridWidth()
		self.iHeight = self.map.getGridHeight()

		self.mapRand = self.gc.getGame().getMapRand()

		self.iFlags = 0  # Disallow FRAC_POLAR flag, to prevent "zero row" problems.
		if self.map.isWrapX(): self.iFlags += CyFractal.FracVals.FRAC_WRAP_X
		if self.map.isWrapY(): self.iFlags += CyFractal.FracVals.FRAC_WRAP_Y

		self.deserts=CyFractal()
		self.plains=CyFractal()
		self.marsh=CyFractal()
		self.variation=CyFractal()

		iDesertPercent += self.gc.getClimateInfo(self.map.getClimate()).getDesertPercentChange()
		iDesertPercent = min(iDesertPercent, 100)
		iDesertPercent = max(iDesertPercent, 0)

		self.iDesertPercent = iDesertPercent
		self.iPlainsPercent = iPlainsPercent
		self.iMarshPercent = iMarshPercent

		self.iDesertTopPercent = 100
		self.iDesertBottomPercent = max(0,int(100-iDesertPercent))
		self.iMarshTopPercent = 100
		self.iMarshBottomPercent = max(0,int(100-iDesertPercent-iMarshPercent))
		self.iPlainsTopPercent = 100
		self.iPlainsBottomPercent = max(0,int(100-iDesertPercent-iMarshPercent-iPlainsPercent))

		self.iMountainTopPercent = 75
		self.iMountainBottomPercent = 60

		fSnowLatitude += self.gc.getClimateInfo(self.map.getClimate()).getSnowLatitudeChange()
		fSnowLatitude = min(fSnowLatitude, 1.0)
		fSnowLatitude = max(fSnowLatitude, 0.0)
		self.fSnowLatitude = fSnowLatitude

		fTundraLatitude += self.gc.getClimateInfo(self.map.getClimate()).getTundraLatitudeChange()
		fTundraLatitude = min(fTundraLatitude, 1.0)
		fTundraLatitude = max(fTundraLatitude, 0.0)
		self.fTundraLatitude = fTundraLatitude

		fGrassLatitude += self.gc.getClimateInfo(self.map.getClimate()).getGrassLatitudeChange()
		fGrassLatitude = min(fGrassLatitude, 1.0)
		fGrassLatitude = max(fGrassLatitude, 0.0)
		self.fGrassLatitude = fGrassLatitude

		fDesertBottomLatitude += self.gc.getClimateInfo(self.map.getClimate()).getDesertBottomLatitudeChange()
		fDesertBottomLatitude = min(fDesertBottomLatitude, 1.0)
		fDesertBottomLatitude = max(fDesertBottomLatitude, 0.0)
		self.fDesertBottomLatitude = fDesertBottomLatitude

		fDesertTopLatitude += self.gc.getClimateInfo(self.map.getClimate()).getDesertTopLatitudeChange()
		fDesertTopLatitude = min(fDesertTopLatitude, 1.0)
		fDesertTopLatitude = max(fDesertTopLatitude, 0.0)
		self.fDesertTopLatitude = fDesertTopLatitude

		self.fracXExp = fracXExp
		self.fracYExp = fracYExp

		self.initFractals()

	def initFractals(self):
		self.deserts.fracInit(self.iWidth, self.iHeight, self.grain_amount, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		self.iDesertTop = self.deserts.getHeightFromPercent(self.iDesertTopPercent)
		self.iDesertBottom = self.deserts.getHeightFromPercent(self.iDesertBottomPercent)

		self.plains.fracInit(self.iWidth, self.iHeight, self.grain_amount+1, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		self.iPlainsTop = self.plains.getHeightFromPercent(self.iPlainsTopPercent)
		self.iPlainsBottom = self.plains.getHeightFromPercent(self.iPlainsBottomPercent)

		self.marsh.fracInit(self.iWidth, self.iHeight, self.grain_amount, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		self.iMarshTop = self.marsh.getHeightFromPercent(self.iMarshTopPercent)
		self.iMarshBottom = self.marsh.getHeightFromPercent(self.iMarshBottomPercent)

		self.variation.fracInit(self.iWidth, self.iHeight, self.grain_amount, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)

		self.terrainDesert = self.gc.getInfoTypeForString("TERRAIN_DESERT")
		self.terrainPlains = self.gc.getInfoTypeForString("TERRAIN_PLAINS")
		self.terrainIce = self.gc.getInfoTypeForString("TERRAIN_SNOW")
		self.terrainTundra = self.gc.getInfoTypeForString("TERRAIN_TUNDRA")
		self.terrainGrass = self.gc.getInfoTypeForString("TERRAIN_GRASS")
		self.terrainMarsh = self.gc.getInfoTypeForString("TERRAIN_MARSH")

	# changed to account for tubes (y-wrap)
	def getLatitudeAtPlot(self, iX, iY):
		"returns a value in the range of 0.0 (tropical) to 1.0 (polar)"
		lat = mst.evalLatitude( map.plot(iX,iY), False )

		# Adjust latitude using self.variation fractal, to mix things up:
		lat += (128 - self.variation.getHeight(iX, iY))/(255.0 * 5.0)

		# Limit to the range [0, 1]:
		if lat < 0: lat = 0.0
		if lat > 1:	lat = 1.0
		return lat

	def generateTerrain(self):
		terrainData = [0]*(self.iWidth*self.iHeight)
		for x in range(self.iWidth):
			for y in range(self.iHeight):
				iI = y*self.iWidth + x
				terrain = self.generateTerrainAtPlot(x, y)
				terrainData[iI] = terrain

		#remove marsh next to desert
		for x in range(self.iWidth):
			for y in range(self.iHeight):
				iIndex = y * self.iWidth + x

				for iDirection in range(CardinalDirectionTypes.NUM_CARDINALDIRECTION_TYPES):
					pPlot = plotCardinalDirection(x, y, CardinalDirectionTypes(iDirection))

					if not pPlot.isNone():
						iOtherIndex = pPlot.getY() * self.iWidth + pPlot.getX()

						if ((terrainData[iIndex] == self.terrainDesert) and (terrainData[iOtherIndex] == self.terrainMarsh)) or ((terrainData[iIndex] == self.terrainMarsh) and (terrainData[iOtherIndex] == self.terrainDesert)):
							terrainData[iIndex] = self.terrainPlains
							break

		return terrainData

	def generateTerrainAtPlot(self,iX,iY):
		lat = self.getLatitudeAtPlot(iX,iY)

		plot = self.map.plot(iX, iY)

		if (plot.isWater()):
########## Temudjin START
#			return self.map.plot(iX, iY).getTerrainType()
			n = mst.numWaterNeighbors( iX, iY )
			return mst.iif( n == 8, mst.etOcean, mst.etCoast )
########## Temudjin END
		terrainVal = self.terrainGrass

		if lat >= self.fSnowLatitude:
			terrainVal = self.terrainIce
		elif lat >= self.fTundraLatitude:
			terrainVal = self.terrainTundra
		elif lat < self.fGrassLatitude:
			terrainVal = self.terrainGrass
		else:
			desertVal = self.deserts.getHeight(iX, iY)
			plainsVal = self.plains.getHeight(iX, iY)
			marshVal = self.marsh.getHeight(iX, iY)
			if ((desertVal >= self.iDesertBottom) and (desertVal <= self.iDesertTop) and (lat >= self.fDesertBottomLatitude) and (lat < self.fDesertTopLatitude)):
				terrainVal = self.terrainDesert
			elif ((marshVal >= self.iMarshBottom) and (marshVal <= self.iMarshTop) and plot.isFlatlands() and (lat >= self.fDesertBottomLatitude) and (lat < self.fDesertTopLatitude)):
				terrainVal = self.terrainMarsh
			elif ((plainsVal >= self.iPlainsBottom) and (plainsVal <= self.iPlainsTop)):
				terrainVal = self.terrainPlains

		if (terrainVal == TerrainTypes.NO_TERRAIN):
			return self.map.plot(iX, iY).getTerrainType()

		return terrainVal


######################################################################################################
########## Starting-Plot Functions
######################################################################################################
# ok = okLandPlots( xCoord, yCoord, ok=10 )
# fracworldAssignStartingPlots()
# plot = fracworldFindStartingPlot( playerID, validFn = None, startList = [] )
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
def fracworldAssignStartingPlots():
	global bestArea

	map.recalculateAreas()
	iPlayers = gc.getGame().countCivPlayersEverAlive()
	areas = CvMapGeneratorUtil.getAreas()
	areaValue = {}
	startDict = {}

	# show continent info
	sprint  = "[FracW] Area ID , Total Plots, Area Value, Start-Plots, Best Yield, Boni Total/Unique, River Edges, Coastal Land\n"
	sprint += "[FracW] --------,------------,-----------,------------,-----------,------------------,------------,-------------\n"
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
		sprint2.append( "[FracW] %9i,     %4i   ,  %5i    ,    %4i    ,   %5i   ,       %3i / %3i  ,    %4i    ,    %4i\n" % \
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
	print "[FracW] Find Starting Plots"
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

		sprint = "[FracW] bestAreaValue: %r , bestAreaID: %r\n" %( bestAreaValue-1, bestArea )

		#-----
		def isValid(iPlayer, x, y):
			plot = mst.GetPlot(x,y)
			if (plot.getArea() != bestArea):
				return False
			return True
		#-----

		sprint += "[FracW] FindStart: playerID %i" % (playerID)
		print sprint

		startList = [ (x,y) for x,y in startDict.values() ]
		sPlot = fracworldFindStartingPlot(playerID, isValid, startList)
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
		print "[FracW] WARNING! - Can't assgin a plot for each player : run default implementation "
		for playerID in assignedPlots.keys() :
			sPlot = assignedPlots[playerID]
			sPlot.setStartingPlot(False)
			player.setStartingPlot(sPlot,False)
		CyPythonMgr().allowDefaultImpl()
	else:
		sprint = ""
		for i in range(iPlayers):
			sprint += "[FracW] StartingPlot Player# %2i - @ (%i,%i)\n" % (i,startDict[i][0],startDict[i][1])
		print sprint

# Find starting-plot for player
def fracworldFindStartingPlot(playerID, validFn = None, startList = []):

	player = gc.getPlayer( playerID )
	player.AI_updateFoundValues( True )
	maxPlayers = gc.getMAX_CIV_PLAYERS()
	iRange = player.startingPlotRange()

	# FracturedWorld world edge is 4
	xEdge = mst.iif( map.isWrapX(), 0, 4 )
	yEdge = mst.iif( map.isWrapY(), 0, 4 )

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
			sprint += "[FracW] Player #%i - pass# %2i: FAILED \n" % (playerID, passes+1)
			continue
		elif len( plotList ) < 5:
			ch = 0
		else:
			ch = mst.chooseNumber( 2 )
		score = plotList[ch][0]
		plot  = plotList[ch][1]

		sprint += "[FracW] Player #%i, plotValue %i: SUCCESS at pass %2i - (%i,%i)" % (playerID, score, passes+1, plot.getX(), plot.getY())
		print sprint
		return plot

	# failed to find suitable plot
	sprint += "[FracW] Player #%i: FAILED to find starting-plot" % (playerID)
	print sprint
	return None


##################################################################################
########## CLASS PlateTectonics
##################################################################################
### Move small island areas, or break up continents by
###   moving part(s) into different directions.
### Independent of map-options the continents move as if
###   the planet has a toroid shape, wrapping on both axis.
### While some land sinks into the ocean, every third eon
###   new lands are created on the coasts or new islands arise.
##################################################################################
# evolvePlanet( eons=12, waterGoal=None, data=None, noInvertRange=range(45,56) )
# raiseNewLand( iNewLand, noIsland=False )
# raiseSeaLevel( iNewSea )
# breakContinent( areaID, data=None )
# moveTectonicPlate( areaID=None, mapSector=None, eDir=None, iSteps=1, data=None )
# moveTileList( areaList, eDir, iSteps=1, data=None )
# compactifyContinent( continentID=None, chCompact=33, chSink=66, chMove=66 )
# invertMap( data=None, noInvertRange=range(45,56) )
##################################################################################
class PlateTectonics:

	# class variable
	inverted = 0

	# Move about a third of the tectonic plates per eon for about 3 tiles.
	# Small islands move faster and more often, while big continents move
	# slower, less often and have a chance to break.
	def evolvePlanet( self, eons=12, waterGoal=None, data=None, noInvertRange=range(45,56) ):
		print "[FracW] -- PlateTectonics:evolvePlanet()"
		# global chWater

		noNewIslands = False
		if ( chWater >= 67 ) or ( chWater <= 33 ):
			noNewIslands = True

		print "[FracW] WaterChance: %r, Watergoal: %r - Inverted %r" % ( chWater, waterGoal, mst.odd(self.inverted)  )
		if waterGoal == None:
			waterGoal = map.numPlots() * chWater / 100
			if mst.odd( self.inverted ): waterGoal = map.numPlots() - waterGoal
		print "[FracW] WaterChance: %r, Watergoal: %r - Inverted %r" % ( chWater, waterGoal, mst.odd(self.inverted)  )
		mst.mapPrint.buildAreaMap( False, "PlateTectonics:evolvePlanet()" )
		plotCounter()
		for loop in range( eons ):
			# if biggest area is land then invert map and evolve lakes
			map.recalculateAreas()

			# compactify all big continents
			if ((loop+2) % 5) == 0:
				print "[FracW] Eon %i: compactify continents" % ( loop )
				self.compactifyContinent( None )
				map.recalculateAreas()
#				mst.mapPrint.buildAreaMap( False, "PlateTectonics:evolvePlanet()" )

			# get list of about a third of the land-areas but at least three
			areas = CvMapGeneratorUtil.getAreas()
			areaList = []
			for area in areas:
				areaList.append( ( area.getNumTiles(), area.isWater(), area.getID() ) )
			continentList = [ (num, id) for num,bWater,id in areaList
			                            if (not bWater) and mst.choose(35-(num/10), True, False) ]
			if len( continentList ) < 3:
				continentList = [ (num, id) for bWater,num,id in areaList if (not bWater) ]

			# move continents
			print "[FracW] Eon %i: moving continents" % ( loop )
			# mst.printList( continentList, prefix = "[FracW] " )
			for cLoop in range( len( continentList ) ):
				tileNum, areaID = mst.chooseListPop( continentList )
				if ( tileNum * 100.0 / map.numPlots() ) > 1.2:
					print "[FracW] Break Continent: %i" % (areaID)
					self.breakContinent( areaID, data )
				else:
					iSteps = 5 + ( map.getWorldSize() / 2 ) - len( str( tileNum ) )
					self.moveTectonicPlate( areaID, None, None, iSteps, data )

			# create some new land or sea
			if (loop < 12) or (( (loop % 3) == 0 ) and ( loop < (eons - 3) )):
				w0,f,h,p = plotCounter( None, noPrint = True )
				iLand = w0 - waterGoal
				iNewLand = ( (abs(iLand) / 2) + mst.chooseNumber( abs(iLand) ) )
				if iLand > 5:
					self.raiseNewLand( iNewLand, noNewIslands )
					print "[FracW] Watergoal: %i - Inverted %r" % ( waterGoal, mst.odd(self.inverted)  )
					w1,f,h,p = plotCounter()
					print "[FracW] Eon %i: raising continents create %i land-tiles" % ( loop, w0 - w1 )
				elif iLand < -5:
					self.raiseSeaLevel( iNewLand )
					print "[FracW] Watergoal: %i - Inverted %r" % ( waterGoal, mst.odd(self.inverted)  )
					w1,f,h,p = plotCounter()
					print "[FracW] Eon %i: raising sea-levels create %i water-tiles" % ( loop, w1 - w0 )
				# check if map should be inverted
				nBigWater = map.findBiggestArea( True ).getNumTiles()
				nBigLand = map.findBiggestArea( False ).getNumTiles()
				if (nBigLand > nBigWater) and (mapOptions.waterPlots[ mapOptions.Water ] not in noInvertRange):
					print "[FracW] Eon %i: inverting map" % ( loop )
					self.invertMap()
					waterGoal = map.numPlots() * chWater / 100
					if mst.odd( self.inverted ): waterGoal = map.numPlots() - waterGoal
					print "[FracW] Watergoal: %i - Inverted %r" % ( waterGoal, mst.odd(self.inverted)  )
					plotCounter()

		# revert map if necessary
		if mst.odd( self.inverted ):
			print "[FracW] Eon %i: revert map" % ( eons )
			self.invertMap()
			self.inverted = 0
			waterGoal = map.numPlots() * chWater / 100 + 20

		# last lumpifications
		mst.mapPrint.buildAreaMap( False, "PlateTectonics:evolvePlanet()" )
		print "[FracW] Last lumpifications"
		print "[FracW] Watergoal: %i - Inverted %r" % ( waterGoal, mst.odd(self.inverted)  )
		if mapOptions.waterPlots[ mapOptions.Water ] not in noInvertRange:
			plotCounter()
			lumpifyOcean( 1, None, chWater / 2 )
			w0,f,h,p = plotCounter( noPrint = True )
			self.compactifyContinent( None, 15, 80 )
			w1,f,h,p = plotCounter( noPrint = True )
			print "[FracW] %i additional land tiles through compactification" % (w0-w1)
			lumpifyLands( 1, None, (100 - chWater) / 2 )

		# adjust to given water-percentage
		cnt = 0
		while cnt < 3:
			cnt += 1
			w0,f,h,p = plotCounter()
			iLand = w0 - waterGoal
			iNewLand = ( (abs(iLand) / 2) + mst.chooseNumber( abs(iLand) ) )
			if iLand < -5 and ( (-iLand * 100.0 / map.numPlots()) > 1.1 ):
				self.raiseSeaLevel( iNewLand )
			elif iLand > 5 and ( (iLand * 100.0 / map.numPlots()) > 2.5 ):
				self.raiseNewLand( iNewLand, noNewIslands )
			else:
				break

		# raise hills and peaks from flatlands
		for inx in range( map.numPlots() ):
			plot = map.plotByIndex( inx )
			if not plot.isWater():
				plot.setPlotType( pickLandType(), False, False )
		map.recalculateAreas()
		plotCounter()

	# raise new coast and new islands from the sea
	def raiseNewLand( self, iNewLand, noIsland=False ):
#		print "[FracW] -- PlateTectonics:raiseNewLand()"
		oceanPlots = [ ( x, y, mst.numWaterNeighbors(x, y) ) for x in range( mst.iNumPlotsX )
		                                                     for y in range( mst.iNumPlotsY )
		                                                     if map.plot(x, y).isWater() ]
		coastPlots = [ ( x, y, n ) for x,y,n in oceanPlots if n < 6 ]
		chCoast = 75
		chOcean = 80
		while iNewLand > 0:
			iNewLand -= 1
			# good chance to raise the new land on coast
			if (len( coastPlots ) > 0):
				x,y,n = mst.chooseListPop( coastPlots )
				if mst.choose( chCoast + ( (5-n)*4 ), True, False ):
					if (x,y,n) in oceanPlots: oceanPlots.remove( (x,y,n) )
					plot = map.plot(x, y)
					plot.setPlotType(pickLandType(), False, False)
					continue
			# othewise raise land from ocean and build small islands
			if not noIsland:
				if len( oceanPlots ) > 0:
					x,y,n = mst.chooseListPop( oceanPlots )
					if (x,y,n) in coastPlots: coastPlots.remove( (x,y,n) )
					plot = map.plot(x, y)
					plot.setPlotType(pickLandType(), False, False)
					antiLast = -1
					while mst.choose(chOcean, True, False):
						eDir = mst.chooseNumber( 4 )
						if eDir == antiLast: eDir = (eDir + 2) % 4
						antiLast = (eDir + 2) % 4
						pl = savePlotCardinalDirection( x, y, eDir )
						x,y = mst.coordByPlot( pl )
						if (x,y,n) in coastPlots: break
						if pl.isWater():
							pl.setPlotType(pickLandType(), False, False)
							iNewLand -= 1
						if (x,y,n) in oceanPlots: oceanPlots.remove( (x,y,n) )
				else:
					break

	# raise new coast and new islands from the sea
	def raiseSeaLevel( self, iNewSea ):
#		print "[FracW] -- PlateTectonics:raiseSeaLevel()"
		landPlots = [ ( x, y, mst.numWaterNeighbors(x, y) ) for x in range( mst.iNumPlotsX )
		                                                    for y in range( mst.iNumPlotsY )
		                                                    if not map.plot(x, y).isWater() ]
		coastalPlots = [ ( x, y, n ) for x,y,n in landPlots if n > 0 ]
		chCoastal = 40
		while iNewSea > 0:
			iNewSea -= 1
			if len( coastalPlots ) > 0:
				x,y,n = mst.chooseListPop( coastalPlots )
				if mst.choose( chCoastal + ( (8-n)*5 ), True, False ):
					plot = map.plot(x, y)
					plot.setPlotType(PlotTypes.PLOT_OCEAN, False, False)
					continue

	# split part of continent
	def breakContinent( self, areaID, data=None ):
#		print "[FracW] -- PlateTectonics:breakContinent()"

		# sort plots
		x0, x1, y0, y1 = mst.getRegion(areaID, 0)
		print "[FracW] >>>>>>>>>>>>>>>>>>>>>>> mapRegion: breakContinent"
		mst.printList( [x0,x1,y0,y1], "mapRegion: breakContinent %r" % (areaID,), prefix = "[FracW] " )
		areaList = mst.getAreaPlotsXY( areaID )

		xList = list( set( [ x for x,y in areaList ] ) )
		xList.sort()
		xList = xList[4:-5]
		yList = list( set( [ y for x,y in areaList ] ) )
		yList.sort()
		yList = yList[4:-5]

		# find direction and plots to move
		fx = None
		fy = None
		if (len( xList ) == 0) and (len( yList ) == 0):
			# no break found - no move
			return
		elif len( xList ) >= len( yList ):
			fx = mst.chooseListElement( xList )
			areaList0 = [ (x, y) for x,y in areaList if (x < fx) or ( (x == fx) and mst.choose(66, True, False) )]
			areaList1 = [ (x, y) for x,y in areaList if (x, y) not in areaList0 ]
			if mst.choose(33, True, False):
				eDir = DirectionTypes.DIRECTION_EAST
			elif mst.choose(50, True, False):
				eDir = DirectionTypes.DIRECTION_NORTHEAST
			else:
				eDir = DirectionTypes.DIRECTION_SOUTHEAST
		else:
			fy = mst.chooseListElement( yList )
			areaList0 = [ (x, y) for x,y in areaList if (y < fy) or ( (y == fy) and mst.choose(66, True, False) )]
			areaList1 = [ (x, y) for x,y in areaList if (x, y) not in areaList0 ]
			if mst.choose(33, True, False):
				eDir = DirectionTypes.DIRECTION_SOUTH
			elif mst.choose(50, True, False):
				eDir = DirectionTypes.DIRECTION_SOUTHWEST
			else:
				eDir = DirectionTypes.DIRECTION_SOUTHEAST
		mst.printList( [x0,x1,y0,y1],
		               "breakContinent %r,fx:%r, fy:%r --> %s" % (areaID, fx, fy, mst.directionName(eDir)),
		               prefix = "[FracW] ")
		if mst.choose( 50, True, False ):
			# move randomly continent
			self.moveTileList( areaList0, mst.chooseNumber( 4 ), 3, data )
		elif mst.choose( 33, True, False ):
			# move into continent
			self.moveTileList( areaList1, eDir, 2, data )
		else:
			# move away from each other
			self.moveTileList( areaList0, eDir, 2, data )
			self.moveTileList( areaList1, mst.getOppositeDirection( eDir ), 1, data )
		#mst.mapPrint.buildPlotMap( True, "PlateTectonics:breakContinent()", (x0-4,x1+4,y0-4,y1+4) )

	# move rectangle or an area within, on the map
	# Note: areas can only be used after generatePlotTypes() has finished
	def moveTectonicPlate( self, areaID=None, mapSector=None, eDir=None, iSteps=1, data=None ):
		if eDir == None:
			eDir = DirectionTypes( mst.chooseNumber(8) )
			if eDir == DirectionTypes.DIRECTION_NORTHWEST:
				eDir = DirectionTypes.DIRECTION_NORTHEAST
			elif eDir == DirectionTypes.DIRECTION_WEST:
				eDir = DirectionTypes.DIRECTION_EAST
			elif eDir == DirectionTypes.DIRECTION_SOUTHWEST:
				eDir = DirectionTypes.DIRECTION_SOUTHEAST
		if areaID != None:
			areaList = mst.getAreaPlotsXY( areaID )
		elif mapSector != None:
			x0, x1, y0, y1 = mapSector
			if x0 > x1: x0,x1 = (x1, x0)
			if y0 > y1: y0,y1 = (y1, y0)
			# build list of tiles to move
			areaList = []
			for fx in range( x0, x1+1 ):
				for fy in range( y0, y1+1 ):
					areaList.append( (fx, fy) )
		else:
			return
		# move tiles
		self.moveTileList( areaList, eDir, iSteps, data )

	# move tiles indicated within list of coordinates one step into given direction
	# aList may be a list of plots or coordinate tuples
	# returns number of changes from water to land
	def moveTileList( self, aList, eDir, iSteps=1, data=None ):
		if len( aList ) == 0	: return
		if type( aList[0] ) != type( () ):
			areaList = [ ( pl.getX(), pl.getY() ) for pl in aList ]
		else:
			areaList = aList
		x0,y0 = areaList[0]
		virtualMap = [-1] * map.numPlots()
		# move plots to virtual map
		for x,y in areaList:
			inx = mst.GetIndex( x, y )
			if data == None:
				plot = mst.GetPlot( x, y )
				virtualMap[ inx ] = plot.getPlotType()
			else:
				virtualMap[ inx ] = data[ inx ]
		# erase original plots
		cnt = 0
		for x,y in areaList:
			inx = mst.GetIndex( x, y )
			if data == None:
				plot = mst.GetPlot( x, y )
				plot.setPlotType(PlotTypes.PLOT_OCEAN, False, False)
			else:
				data[ inx ] = PlotTypes.PLOT_OCEAN
		# move virtual plots to real map
		xyDir = mst.xyDirection( eDir )
		dx, dy = [ x * iSteps for x in xyDir ]
		for x,y in areaList:
			inx = mst.GetIndex( x, y )
			plType = virtualMap[ inx ]
			if plType != -1:
				if data==None:
					plot = mst.GetPlot(x + dx, y + dy)
					plot.setPlotType( plType, False, False )
				else:
					i = mst.GetIndex(x + dx, y + dy)
					data[ i ] = plType

	# erode loosely connected continent-tiles,
	# fill-in coast-tiles surrounded by continent,
	# sink diagonal land-bridges
	# - note that areas are not updated even as plots changed
	def compactifyContinent( self, continentID=None, chCompact=33, chSink=66, chMove=75 ):
		minContinent = 40

		if continentID == None:
			# compactify all continents with at least minContinent plots
			map.recalculateAreas()
			areas = CvMapGeneratorUtil.getAreas()
			# order areas
			areaList = []
			for area in areas:
				areaList.append( ( area.isWater(), area.getNumTiles(), area.getID() ) )
			# get list of big land-areas
			continentList = [ (num, id) for bWater,num,id in areaList
												 if (not bWater) and (num >= minContinent) ]
			continentList.sort()
			contSinglePlotList = []
			# make list of one plot for each continent
			for num, continentID in continentList:
				for inx in range( map.numPlots() ):
					plot = map.plotByIndex( inx )
					if plot.getArea() == continentID:
						contSinglePlotList.append( plot )
						break
			# compactify the continent each plot represents
			# - ID's may have been changed between calls of compactifyContinent()
			for pl in contSinglePlotList:
				continentID = plot.getArea()
				self.compactifyContinent( continentID, chCompact, chSink, chMove )

		else:
			# compactify single continent
			area = map.getArea( continentID )
			if area.getID == -1: return								# incorrect (probably outdated) continentID
			if area.isWater(): return									# land!
			if area.getNumTiles() < minContinent: return			# at least minContinent plots
			aPlots = mst.getAreaPlotsXY( continentID )			# get coord-list
			x0,y0 = aPlots[0]
			# fill surrounded coast
			cPlots = mst.getContinentCoastXY( aPlots, True )	# don't ignore lakes
			coordList = [ ( x, y, mst.numWaterNeighbors(x, y) ) for x,y in cPlots
			                                                    if mst.numWaterNeighbors(x,y) < 4 ]
			if len( coordList ) > 0:
				for x,y,n in coordList:
					if mst.choose( chCompact - 3 + ( (3-n)*5 ), True, False ):
						# add plot to continent
						plot = map.plot(x, y)
						plot.setPlotType( pickLandType(), False, False )
						if (x,y) not in aPlots:
							aPlots.append( (x, y) )		# keep aPlots current
			# flood surrounded land
			coordList = [ (x, y, mst.numWaterNeighbors(x, y) ) for x,y in aPlots
			                                                   if mst.numWaterNeighbors(x, y) >= 6 ]
			if len( coordList ) > 0:
				for x,y,n in coordList:
					if mst.choose( 90 - ( (7-n) * 15 ), True, False ):
						# flood land plot
						plot = map.plot(x, y)
						plot.setPlotType( PlotTypes.PLOT_OCEAN, False, False )
						if (x, y) in aPlots:
							aPlots.remove( (x, y) )		# keep aPlots current
			# sink single land-bridge
			bridgeList = [ (x, y) for x,y in aPlots if mst.checkLandBridge(x, y) != None ]
			theList = [ (x, y) for x,y in bridgeList if mst.checkDiagonalLandBridge(x, y) != None ]
			if len( theList ) > 0:
				if mst.choose( chSink, True, False ):
					# sink diagonal land-bridge
					inx = mst.chooseListIndex( theList )
					x,y = theList[ inx ]
					eDir0, eDir1 = mst.checkDiagonalLandBridge( x, y )
					plot = map.plot(x, y)
					plot.setPlotType(PlotTypes.PLOT_OCEAN, False, False)
					if (x, y) in aPlots:
						aPlots.remove( (x, y) )
					if mst.choose( 50, True, False ): eDir0, eDir1 = [ eDir1, eDir0 ]
					p0 = savePlotDirection( x, y, eDir0 )
					p1 = savePlotDirection( x, y, eDir1 )
					if mst.choose( chMove, True, False ):
						# sink by moving over
						self.moveTileList( aPlots, eDir1, iSteps=2 )
					else:
						# just sink another pillar of the bridge
						p1.setPlotType(PlotTypes.PLOT_OCEAN, False, False)
						x1, y1 = mst.coordByPlot( p1 )
						if (x1, y1) in aPlots:
							aPlots.remove( (x1, y1) )
			else:
				theList = bridgeList
				if len( theList ) > 0:
					if mst.choose( chSink, True, False ):
						# sink normal land-bridge
						inx = mst.chooseListIndex( theList )
						x,y = theList[ inx ]
						dDir = mst.checkDiagonalLandBridge( x, y )
						if dDir != None:
							eDir0, eDir1 = dDir
							plot = map.plot(x, y)
							plot.setPlotType(PlotTypes.PLOT_OCEAN, False, False)
							if (x, y) in aPlots:
								aPlots.remove( (x, y) )
							if mst.choose( 50, True, False ): eDir0, eDir1 = [ eDir1, eDir0 ]
							p1 = savePlotDirection( x, y, eDir1 )
							p1.setPlotType(PlotTypes.PLOT_OCEAN, False, False)
							x1, y1 = mst.coordByPlot( p1 )
							if (x1, y1) in aPlots:
								aPlots.remove( (x1, y1) )

	# change ocean to land and land to ocean
	def invertMap( self, data=None, noInvertRange=range(45,56) ):
		if mapOptions.waterPlots[ mapOptions.Water ] in noInvertRange: return
		self.inverted += 1
		for inx in range( map.numPlots() ):
			if data == None:
				plot = map.plotByIndex( inx )
				if plot.isWater():
					plot.setPlotType( PlotTypes.PLOT_LAND, False, False )
				else:
					plot.setPlotType( PlotTypes.PLOT_OCEAN, False, False )
			else:
				if data[ inx ] == PlotTypes.PLOT_OCEAN:
					data[ inx ] = PlotTypes.PLOT_LAND
				else:
					data[ inx ] = PlotTypes.PLOT_OCEAN
		if data == None: map.recalculateAreas()

###############################################################
########## CLASS PlateTectonics END
###############################################################
plateTectonics = PlateTectonics()


###############################################################
########## CLASS OrphanPlots
###############################################################
# initialize( bLakes=False )
# data = moveOrphanPlots( data, passes=1, chMove=66, bLakes=False )
# data = moveSingleOrphanPlot( x, y, data, eDir )
# dirList = checkOrphanNeighbors( x, y, data )
###############################################################
class OrphanPlots:

	bMoveLakes = False
	orphList = [ PlotTypes.PLOT_LAND, PlotTypes.PLOT_HILLS, PlotTypes.PLOT_PEAK ]

	# initialize if the orphans are islands in the ocean, or lakes on continents
	def initialize( self, bLakes=False ):
		self.bMoveLakes = bLakes
		if bLakes:
			self.orphList = [ PlotTypes.PLOT_OCEAN ]
		else:
			self.orphList = [ PlotTypes.PLOT_LAND, PlotTypes.PLOT_HILLS, PlotTypes.PLOT_PEAK ]

	# move singleconnected- or unconnected island plots toward a spot with
	# potentially more than one connection
	def moveOrphanPlots( self, data, passes=1, chMove=66, bLakes=False ):
		print "[FracW] -- OrphanPlots:moveOrphanPlots()"
		self.initialize( bLakes )
		for loop in range( passes ):
			mapList = mst.randomList.randomCountList( map.numPlots() )
			for inx in mapList:
				x, y = mst.coordByIndex( inx )
				if self.bMoveLakes:
					if map.plot(x,y).isWater(): dirList = self.checkOrphanNeighbors( x, y, data )
					else: continue
				else:
					if not map.plot(x,y).isWater(): dirList = self.checkOrphanNeighbors( x, y, data )
					else: continue
				if len( dirList ) == 0:
					eDir = DirectionTypes( mst.chooseNumber(8) )
				elif len( dirList ) == 1:
					eDir1 = (dirList[0] + 7) % 8
					eDir2 = (dirList[0] + 1) % 8
					if (dirList[0] % 2) == 0:
						eDir = DirectionTypes( mst.chooseMore( (33,eDir1), (66,eDir2), (100,-1) ) )
					else:
						eDir = DirectionTypes( mst.choose( 50, eDir1, eDir2 ) )
				else:
					eDir = DirectionTypes.NO_DIRECTION
				if not ( eDir == DirectionTypes.NO_DIRECTION ):
					if mst.choose( chMove, True, False ):
						data = self.moveSingleOrphanPlot( x, y, data, eDir )
		return data

	# move single orphan into given direction
	def moveSingleOrphanPlot( self, x, y, data, eDir ):
		plot = map.plot(x,y)
		index = mst.indexByPlot( plot )
		pl = savePlotDirection( x, y, eDir )
		inx = mst.indexByPlot( pl )
		if self.bMoveLakes:
			plotData = data[ inx ]
			data[ inx ] = PlotTypes.PLOT_OCEAN
			data[ index ] = plotData
		else:
			plotData = data[ index ]
			data[ index ] = PlotTypes.PLOT_OCEAN
			data[ inx ] = plotData
		return data

	# collect directions of orphan neighbors
	def checkOrphanNeighbors( self, x, y, data ):
		dirList = []
		for eDir in range( DirectionTypes.NUM_DIRECTION_TYPES ):
			p = savePlotDirection( x, y, eDir )
			if p.getPlotType() in self.orphList:
				dirList.append( eDir )
		return dirList

###############################################################
########## CLASS OrphanPlots END
###############################################################
orphanPlots = OrphanPlots()


############################################################
########## CLASS StopWatch - create timers
############################################################
# t( timerName, mode )
# clear( timerName )
# timerData()
############################################################
from time import *
class StopWatch:

	timerDict = {}

	# display time between calls
	# Modus = -1: restart timer; clear if first start
	# Modus =  0: clear and start timer
	# Modus =  1: don't add previous time to stoptime
	# Modus =  2: elapsed time since first start
	def t( self, tim=0, nModus=-1 ):
		tNow = clock()
		if nModus == 0:
			# modus 0 - clear timer
			tLast = 0
			tDiffSum = 0
			tStart = tNow
			self.timerDict[ tim ] = ( tLast, tDiffSum, tStart )

		# get timer data
		try:
			tLast, tDiffSum, tStart = self.timerDict[ tim ]
		except:
			# first start - clear timer
			nModus = 0
			tLast = 0
			tDiffSum = 0
			tStart = tNow
			self.timerDict[ tim ] = ( tLast, tDiffSum, tStart )

		# calc time difference and cummulate
		dt = tNow - tLast
		if (nModus % 2) >= 1:
			# modus 1 - don't add time
			tDiffSum += dt

		# display
		if nModus == 0:
			# timer was cleared
			print "[FracW] TIMER %r - START" % ( tim )
		elif (nModus > 0) and ((nModus % 2) >= 1):
			# modus 1
			print "[FracW] TIMER %r - RESTART - STOPTIME: %-10.5f" % ( tim, tDiffSum )
		elif (tDiffSum - dt) < 0.000001:
			# timer first stop
			print "[FracW] TIMER %r - TIMEDIFF: %-10.5f" % ( tim, dt )
		else:
			# timer second stop and later
			print "[FracW] TIMER %r - TIMEDIFF: %-10.5f - STOPTIME: %-10.5f" % ( tim, dt, tDiffSum )
		if (nModus > 0) and ((nModus % 4) >= 2):
			print "[FracW] TIMER %r - ELAPSED TIME: %-10.5f" % ( tim, tNow - tStart )

		# restart timer
		self.timerDict[ tim ] = ( clock(), tDiffSum, tStart )

	# clear counters
	def clear( self, tim=None ):
		if tim == None:
			self.timerDict = {}
			print "[FracW] TIMER - ALL CLEAR"
		elif tim in timerDict.keys():
			del self.timerDict[ tim ]
			print "[FracW] TIMER %r - CLEAR" % ( tim )
		else:
			print "[FracW] TIMER %r is not known" % ( tim )

	# list all timers
	def timerData( self ):
		mst.printDict( self.timerDict, "TIMER Data: (Laststart, Timediff, Firststart)", prefix = "[FracW] " )

############################################################
########## CLASS StopWatch END
############################################################
timer = StopWatch()


#######################################################################################
########## CLASS OccStart - make sure all players are reachable for One City Challenge
#######################################################################################
### - humans are supposed to have either a starting-plot near the biggest ocean
###   or all players should inhabit the same continent
### - team options are ignored
### - on failure there should either be a recalculation of starting-points or a pop-up,
###   but as yet there is only a report, if no solution was possible
#######################################################################################
# ok = occHumanStart()
# makeGameOptionDict( bPrint=True )
# bOpt = isGAMEOPTION_ONE_CITY_CHALLENGE()
# --- private ---
# bSingleStartContinent = isSingleStartContinent()
# makeStartingPlotDict()
# playerNeighborAreasDict = makePlayerNeighborAreasDict()
######################################################################################
class OccStart:

	gameOptionDict = {}		# gameOptionDict = { opt_string = bOption }
	startingPlotDict = {}	# startingPlotDict = { playerID = [ (startingPlot, startAreaID), .. ] }

	# make sure player has ocean access, if game-option 'One City Challange' is selected
	def occHumanStart( self ):
		# check game-option
		if not self.isGAMEOPTION_ONE_CITY_CHALLENGE(): return True

		# check for single start area for all players
		map.recalculateAreas()
		if self.isSingleStartContinent(): return True

		# humans need to have starting-plots at biggest ocean
		# - if there are starting plots on an isle within the
		#   second biggest ocean, we are out of luck!
		# - may want to check for this later
		ocean = map.findBiggestArea( True )				# biggest Ocean
		oceanID = ocean.getID()
		# list of human playerID's
		humanList = [ playerID for playerID in range( gc.getMAX_CIV_PLAYERS() )
		                       if gc.getPlayer( playerID ).isAlive()
		                       if gc.getPlayer( playerID ).isHuman() ]
		# dict of neighbor-areaID's for each playerID
		playerNeighborAreasDict = self.makePlayerNeighborAreasDict()	# { playerID = [ neighborAreaID, .. ] }
		# list of possible human occ-starts
		occStartList = [ playerID for playerID in playerNeighborAreasDict.keys()
		                          if oceanID in playerNeighborAreasDict[ playerID ] ]

		# check if solution is possible
		if len( humanList ) > len( occStartList ):
			print "[FracW] ##############################################"
			print "[FracW] ERROR! - Not all Humans can start near the ocean"
			print "[FracW] ##############################################"
			mst.printList( humanList, " List of Human players:", prefix = "[FracW] " )
			mst.printList( occStartList, " List of players with ocean starting-plot:", prefix = "[FracW] " )
			print "[FracW] " + mst.mapStats.sprintActiveCivs( True, False, True )		# show teams, no traits but humanity
			print "[FracW] ##############################################"
			# a popup might be useful here
			# basically with OCC you want to choose human startin-plots
			# from the coastal lands around the biggest ocean.
			return False

		# ignore humans already near oceans
		for humanID in humanList:
			if humanID in occStartList:
				humanList.remove( humanID )
				occStartList.remove( humanID )

		# switch starting-plots until all humans are near the ocean
		# - team-options are ignored
		for humanID in humanList:
			human = gc.getPlayer( humanID )
			playerID = mst.chooseListElement( occStartList )
			player = gc.getPlayer( playerID )
			hPlot = human.getStartingPlot()
			pPlot = player.getStartingPlot()
			human.setStartingPlot( pPlot, False )
			player.setStartingPlot( hPlot, False )
			occStartList.remove( playerID )
		return True

	# build dictionary: self.gameOptionDict = { opt_string = bOption }
	def makeGameOptionDict( self, bPrint=True ):
		self.gameOptionDict = {}
		for opt in range( gc.getNumGameOptionInfos() ):
			opt_string = gc.getGameOptionInfo( opt ).getType()
			xStr = "gc.getGame().isOption(GameOptionTypes." + opt_string + ")"
			try:
				bOption = eval( xStr )
			except:
				bOption = False
			self.gameOptionDict[ opt_string ] = bOption
		if bPrint:
			mst.printDict( self.gameOptionDict, "Game Options", prefix = "[FracW] " )

	# test if game-option 'One City Challenge' is checked
	def isGAMEOPTION_ONE_CITY_CHALLENGE( self ):
		self.makeGameOptionDict()							# no assumptions here!
		try:
			bOpt = self.gameOptionDict[ "GAMEOPTION_ONE_CITY_CHALLENGE" ]
		except:
			bOpt = False
		return bOpt

	###############
	### Helpers ###
	###############

	# ----------------------------------------
	# map.recalculateAreas()
	# areaID = plot.getArea()
	# areaID = area.getID()
	# area = plot.area()
	# area = map.getArea( areaID )
	# area = map.findBiggestArea( bWater )
	# areaList = CvMapGeneratorUtil.getAreas()
	# ----------------------------------------

	# test if all starting-plots are on the same continent
	def isSingleStartContinent( self ):
		if self.startingPlotDict == {}: self.makeStartingPlotDict()
		startAreaList = []
		for playerID in self.startingPlotDict.keys():
			startingPlot, startAreaID = self.startingPlotDict[ playerID ]
			if startAreaID not in startAreaList:
				startAreaList.append( startAreaID )
		return iif( len(startAreaList) == 1, True, False )

	# build dictionary: self.startingPlotDict = { playerID = [ (startingPlot, startAreaID), .. ] }
	def makeStartingPlotDict( self ):
		self.startingPlotDict = {}
		for playerID in range( gc.getMAX_CIV_PLAYERS() ):
			player = gc.getPlayer( playerID )
			if player.isAlive():
				startingPlot = player.getStartingPlot()
				startAreaID = startingPlot.getArea()
				self.startingPlotDict[ playerID ] = ( startingPlot, startAreaID )
#		mst.printDict( self.startingPlotDict, "self.startingPlotDict = { playerID = [ (startingPlot, startAreaID), .. ] }", prefix = "[FracW] " )

	# build dictionary: self.playerNeighborAreasDict = { playerID = [ neighborAreaID, .. ] }
	def makePlayerNeighborAreasDict( self ):
		if self.startingPlotDict == {}: self.makeStartingPlotDict()
		playerNeighborAreasDict = {}
		for playerID in self.startingPlotDict.keys():
			playerNeighborAreasDict[ playerID ] = self.getPlayerNeighborAreas( playerID )
#		mst.printDict( playerNeighborAreasDict, "playerNeighborAreasDict = { playerID = [ neighborAreaID, .. ] }", prefix = "[FracW] " )
		return playerNeighborAreasDict

	# get areas surrounding the starting-plot of the player
	def getPlayerNeighborAreas( self, playerID ):
		if self.startingPlotDict == {}: self.makeStartingPlotDict()
		startingPlot, startAreaID = self.startingPlotDict[ playerID ]
		x, y = startingPlot.getX(), startingPlot.getY()
		playerNeighborAreas = []
		for eDir in range( DirectionTypes.NUM_DIRECTION_TYPES ):
			pl = savePlotDirection( x, y, eDir )
			areaID = pl.getArea()
			if areaID == startAreaID: continue
			if areaID in playerNeighborAreas: continue
			playerNeighborAreas.append( areaID )
		return playerNeighborAreas

#######################################################################################
### CLASS OccStart END
#######################################################################################
occStart = OccStart()


############################################################
########## CLASS MapOptions - store map defaults
############################################################
class MapOptions:
	def __init__(self):
		print "[FracW] ===== initialize Class::MapOptions"

		global bMarsh
		bMarsh = self.isMarsh()
		self.Marsh = 0
		self.terGen = None					# terrainGenerator choosen

		# set defaults
		self.Shape = 3							# default: Toroid
		self.Water = 2							# default: 75% Oceanworld
		self.Peaks = 1							# default: 6% Uneven (no idea how earthlike that is)
		self.Terrain = 2						# default: Earthlike
		self.Age = 2							# default: 6 eons
		self.Theme = 0							# default: Sands of Mars
		self.Teams = 0							# default: Team Neighbors

		self.waterPlots = [ 85, 80, 75, 70, 65, 60, 50, 40, 30, 20 ]
		self.peakPlots  = [ 3, 6, 10, 15, 20 ]
		if bMarsh:
			self.descTerrain = "Terrain (Des,Plns,Mrsh)"
			self.descTerrList = [
											" 10% , 25% , 9% Wet",
											" 20% , 22% , 6% Damp",
											" 30% , 20% , 4% Earthlike",
											" 35% , 25% , 3% Dry",
											" 40% , 30% , 2% Parched"
										]
		else:
			self.descTerrain = "Terrain (Desert,Plains)"
			self.descTerrList = [
											" 10% , 30% Humid",
											" 20% , 25% Damp",
											" 30% , 20% Earthlike",
											" 35% , 25% Dry",
											" 40% , 30% Parched"
										]
		self.ageEons = [ 0, 3, 6, 12, 18, 24, 36, 48, 72, 96 ]
		self.Desert = 30
		self.Plains = 20

		# collect custom options
		self.hasReadConfig = False
		self.mapDesc = {}

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
		print "[FracW] -- MapOptions.showGameOptions()"
		sprint = ""
		for gam in range(gc.getNumGameOptionInfos()):
			gam_string = gc.getGameOptionInfo(gam).getType()
			xStr = "gc.getGame().isOption(GameOptionTypes." + gam_string + ")"
			bOpt = eval( xStr )
			sVisible = "-"
			if mst.bBTS:
				if not gc.getGameOptionInfo(gam).getVisible():
					sVisible = "*"
			sprint += "[FracW] GameOption: #%2i %s [ %5r ] %s\n" % (gam,sVisible,bOpt,gam_string)
		print sprint

	# check if mod has marsh
	def isMarsh( self ):
		bMarsh = ( gc.getInfoTypeForString("TERRAIN_MARSH") >= 0 )
		return bMarsh

	# read game and map options from 'FracturedWorld.cfg'
	def readConfig( self ):
		print "[FracW] -- readConfig() - already called? %r" % (self.hasReadConfig)
		if self.hasReadConfig: return
		self.hasReadConfig = True
		fileName = mst.civFolders.logDir + '\\' + cfgFile
		try:
			settings = open(fileName, 'r')
			config = pickle.load(settings)
			settings.close()
			print "[FracW] Success - read %r" % (fileName)

			# get map options
			self.Shape, self.Water, self.Peaks, self.Terrain, self.Age, self.Theme, self.Teams = config
			mst.printList( config, "Map Options:", prefix = "[FracW] " )
		except IOError:
			print "[FracW] ERROR! - Couldn't find %r" % (fileName)
		except EOFError:
			print "[FracW] ERROR! - Bad contents in %r" % (fileName)
		except:
			print "[FracW] ERROR! - Unexpected problem reading %r" % (fileName)

	# write custom options to 'FracturedWorld.cfg'
	def writeConfig( self ):
		print "[FracW] -- writeConfig()"

		# prepare map options
		self.userOptions = [ int(self.Shape), int(self.Water), int(self.Peaks),
		                     int(self.Terrain), int(self.Age), int(self.Theme), int(self.Teams) ]
		mst.printList( self.userOptions, "User Options:", prefix = "[FracW] " )

		# try to write custom options
		config = self.userOptions
		fileName = mst.civFolders.logDir + '\\' + cfgFile
		try:
			settings = open(fileName, 'w')
			try:
				pickle.dump( config, settings)
				print "[FracW] Success - written %r" % fileName
			except Exception, inst:
				print "[FracW] ERROR! - Pickling Error ( %r ) - failed to save 'FracturedWorld' settings to:\n   %r" % (inst,fileName)
		except IOError:
			print "[FracW] ERROR! - Couldn't create %r" % fileName
		except EOFError:
			print "[FracW] ERROR! - EOF writing %r" % fileName
		except:
			print "[FracW] ERROR! - unexpected problem writing %r" % fileName
		settings.close()
		self.hasReadConfig = False

############################################################
########## CLASS MapValues END
############################################################
mapOptions = MapOptions()




