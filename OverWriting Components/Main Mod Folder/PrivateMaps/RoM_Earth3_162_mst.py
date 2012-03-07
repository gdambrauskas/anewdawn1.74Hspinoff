###########################################################
## This mapscript is modified to use the "FlavorMod".
## See "FFH_FlavourMod.py" in the same directory for details.
###########################################################
## from FFH_FlavourMod import *
###########################################################

#
#   FILE:    Earth3.py  1.62
#   AUTHORS: Sirian  (Terra.py)
#            GRM7584 (Earth2.py - adapted from Sirian's Terra script)
#            jkp1187 (Terra3.py - adapted from Earth2.py)
#   PURPOSE: Global map script - Simulates Randomized Earth
#-----------------------------------------------------------------------------
#   Copyright (c) 2005 Firaxis Games, Inc. All rights reserved.
#-----------------------------------------------------------------------------
#	 1.52		Temudjin 15.Jun.08	- cool starting plots, mapTools( Ringworld2 )
#	 1.60		Temudjin 10.Aug.09	- use MapScriptTools
#                                - compatibility with 'Planetfall'
#											- add Map Options: MapRegion, BalancedResources, TeamStart
#                                - add Marsh terrain, if supported by mod
#	 1.61 	Temudjin 15.Sep.09	- add New Zealand, Madagascar
#                                - change Japan
#											- add Himalayas (single BigDent at special place)
#											- add rivers from lakes, on islands
#	 1.62 	Temudjin 15.July.10  - allow more world sizes, if supported by mod
#                                - add Deep Ocean terrain, if supported by mod
#                                - add Map Features ( Kelp, HauntedLands, CrystalPlains )
#                                - compatibility with 'Mars Now!'

from CvPythonExtensions import *
import CvUtil
import CvMapGeneratorUtil
#from CvMapGeneratorUtil import MultilayeredFractal
#from CvMapGeneratorUtil import TerrainGenerator
#from CvMapGeneratorUtil import FeatureGenerator


################################################################
## MapScriptTools by Temudjin
################################################################
import MapScriptTools as mst
balancer = mst.bonusBalancer

def getVersion():
	return "1.62"

def getDescription():
    return "Derivative Mapscript Simulating a randomized Earth Ver." + getVersion()

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
	balancer.initialize( CyMap().getCustomMapOption(1)==1 )

def generateTerrainTypes():
	print "-- generateTerrainTypes()"

	# Planetfall: more highlands and ridges
	mst.planetFallMap.buildPfallHighlands( 8 )		# only a few more highlands
	# Prettify the map
	mst.mapPrettifier.hillifyCoast( 30 )
	# Connect some newfoundland and indonesia isles
	mst.mapPrettifier.bulkifyIslands( 85, 6 )
	# Choose terrainGenerator
	if mst.bPfall or mst.bMars:
		# Use with Earth3TerrainGenerator() defaults
		terrainGen = mst.MST_TerrainGenerator( 33, 26, 0.85, 0.80, 0.1, 0.1, 0.35, -1, -1, 3 )
	else:
		terrainGen = Earth3TerrainGenerator()
	# Generate terrain
	terrainTypes = terrainGen.generateTerrain()
	return terrainTypes

def addRivers():
	print "-- addRivers()"

	# Generate DeepOcean-terrain if mod allows for it
	mst.deepOcean.buildDeepOcean()
	# Generate marsh-terrain
	mst.marshMaker.convertTerrain()
	# build Himalayas
	buildHimalayas()
	if CyMap().getCustomMapOption(0) == 1:				# userOption: use mapRegions
		# Build between 0..3 mountain-ranges.
#		mst.mapRegions.buildBigDents( 1 )				# no more dents, we already have Himalayas
		# Build between 0..3 bog-regions.
		mst.mapRegions.buildBigBogs()
		# build ElementalQuarter
		mst.mapRegions.buildElementalQuarter()

	# Sprout rivers from lakes.
	mst.riverMaker.buildRiversFromLake( None, 66, 2, 1 )		# all lakes, 66% chance, 2 rivers, size>=1

	# no rivers on Mars
	if not mst.bMars:
		# Put rivers on the map.
		CyPythonMgr().allowDefaultImpl()

	# Put additional rivers on small islands
	mst.riverMaker.islandRivers()

def addLakes():
	print "-- addLakes()"
	if not mst.bMars:
		CyPythonMgr().allowDefaultImpl()

def addFeatures():
	print "-- addFeatures()"

	# kill of spurious lakes
	mst.mapPrettifier.connectifyLakes( 33 )
	# Use default featureGenerator
	featuregen = mst.MST_FeatureGenerator()
	featuregen.addFeatures()
	# transform coastal volcanos
	mst.mapPrettifier.beautifyVolcanos()
	# Planetfall: handle shelves and trenches
	mst.planetFallMap.buildPfallOcean()

# shuffle starting-plots for teams
def normalizeStartingPlotLocations():
	print "-- normalizeStartingPlotLocations()"

	# build Lost Isle
	# - this region needs to be placed after starting-plots are first assigned
	if CyMap().getCustomMapOption(0) == 1:
		mst.mapRegions.buildLostIsle( chance=25, minDist=7, bAliens=mst.choose(33,True,False) )

	if CyMap().getCustomMapOption(2) == 0:
		# by default civ places teams near to each other
		CyPythonMgr().allowDefaultImpl()
	elif CyMap().getCustomMapOption(2) == 1:
		# shuffle starting-plots to separate teams
		mst.teamStart.placeTeamsTogether( False, True )
	else:
		# randomize starting-plots to ignore teams
		mst.teamStart.placeTeamsTogether( True, True )

# prevent additional rivers on Mars
def normalizeAddRiver():
	print "-- normalizeAddRiver()"
	if not mst.bMars:
		CyPythonMgr().allowDefaultImpl()

# prevent additional lakes on Mars
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

	# Balance boni, place missing boni, move minerals
	balancer.normalizeAddExtras()

#	normalizeAddExtras2()									# call renamed script function
	CyPythonMgr().allowDefaultImpl()

	if CyMap().getCustomMapOption(0) == 1:				# userOption: use mapRegions
		# give extras to special regions
		mst.mapRegions.addRegionExtras()

	# Place special features on map
	mst.featurePlacer.placeKelp()
	mst.featurePlacer.placeHauntedLands()
	mst.featurePlacer.placeCrystalPlains()

	# Print maps
	mst.mapPrint.buildTerrainMap( False, "normalizeAddExtras()" )
	if mst.bFFH: mst.mapPrint.buildBonusMap( False, "normalizeAddExtras():Mana", None, mst.mapPrint.manaDict )
	mst.mapPrint.buildRiverMap( False, "normalizeAddExtras2()" )
	# Print stats
	mst.mapStats.mapStatistics()

def minStartingDistanceModifier():
	return mst.iif( mst.bPfall or mst.bMars, -25, 20 )

################################################################

#-----------------------
#             dx
#              !
#        .......|......
# dy+4   .....hh|h.....
# dy+3   ...hhHH|Hhh...
# dy+2   ..hhHH^|HHhh..
# dy+1   ..hH^^^|^^Hh..
# dy     .hH^^MM|M^^Hh.
# dy-1   ..hH^^^|^^Hh..
# dy-2   ..hhHH^|HHhh..
# dy-3   ...hhHH|Hhh...
# dy-4   .....hh|h.....
#        .......|......
#         !    !     !
#       dx-5  dx  dx+5+s
#-----------------------
def buildHimalayas():
	print "-- buildHimalayas()"

	HimalayaLon = 0.77
	HimalayaLat = 0.61
	x = int(HimalayaLon * mst.iNumPlotsX) + mst.chooseNumber( 3 + CyMap().getWorldSize()/2 )
	y = int(HimalayaLat * mst.iNumPlotsY) + mst.chooseNumber( 1 + CyMap().getWorldSize()/2 )

	plot = CyMap().plot( x, y )
	mst.mapRegions.adjustBigDentsTemplate()
	mst.mapRegions.theBigDent( plot, False, 66 + 2 * CyMap().getWorldSize() )	# Horizontal, 66% base access chance
	# get landmark bonus
	if mst.bPfall:
		eFlat = mst.choose( 66, mst.ebGrenadeFruits, mst.ebGold )
		eHills = mst.choose( 66, mst.ebSilver, mst.ebFungalGin )
		eTerr = mst.etCoast
		if plot.isFlatlands(): eTerr = mst.etFlatArid
		else: eTerr = mst.etRockyArid
	else:
		eFlat = mst.choose( 66, mst.ebCow, mst.ebMarble )
		eHills = mst.choose( 66, mst.ebSilver, mst.ebFur )
		eTerr = mst.etCoast
		if plot.isFlatlands(): eTerr = mst.etPlains
		else: eTerr = mst.etTundra
	# set landmark
#	mst.mapSetSign( plot, "Himalayas" )
	# set landmark terrain / bonus
	if plot.isHills():
		plot.setTerrainType( eTerr, True, True )
		plot.setBonusType( eHills )
	elif plot.isFlatlands():
		plot.setTerrainType( eTerr, True, True )
		plot.setBonusType( eFlat )
	print " Himalayas built @ %i, %i" % ( x, y )

################################################################
### Custom Map Options
################################################################

def getNumCustomMapOptions():
#	print "===== getNumCustomMapOptions()"
	return 2 + mst.iif( mst.bMars, 0, 1 )

def getCustomMapOptionName(argsList):
#	print "===== getCustomMapOptionName()"
	[iOption] = argsList
	option_names =	{ 0: "Map Regions",
						  1: "Resources",
						  2: "Team Start"
						}
	translated_text = unicode(CyTranslator().getText(option_names[iOption], ()))
	return translated_text

def getNumCustomMapOptionValues(argsList):
#	print "===== getNumCustomMapOptionValues()"
	[iOption] = argsList
	option_values =	{ 0: 2,
							  1: 2,
							  2: 3
							}
	return option_values[iOption]

def getCustomMapOptionDescAt(argsList):
#	print "===== getCustomMapOptionDescAt()"
	[iOption, iSelection] = argsList

	selection_names =	{
							0:	{
								0: "Normal Earth",
								1: "With Special Regions"
								},
							1:	{
								0: "Normal Resources",
								1: "Balanced Resources"
								},
							2:	{
								0: "Team Neighbors",
								1: "Team Separated",
								2: "Randomly Placed"
								}
							}

	translated_text = unicode(CyTranslator().getText(selection_names[iOption][iSelection], ()))
	return translated_text

# reload Ringworld2 UserSettings from last session or default
def getCustomMapOptionDefault(argsList):
#	print "===== getCustomMapOptionDefault()"
	[iOption] = argsList
	option_defaults =	{ 0: 0,
							  1: 1,
							  2: 0
							}
	return option_defaults[iOption]

def isRandomCustomMapOption(argsList):
	[iOption] = argsList
	option_random =	{ 0: True,
							  1: False,
							  2: False
							}
	return option_random[iOption]

def getWrapX():
	return True

def getWrapY():
	return False

################################################################


'''
MULTILAYERED FRACTAL NOTES

The MultilayeredFractal class was created for use with this script.

I worked to make it adaptable to other scripts, though, and eventually it
migrated in to the MapUtil file along with the other primary map classes.

- Bob Thomas   July 13, 2005


TERRA NOTES

Terra turns out to be our largest size map. This is the only map script
in the original release of Civ4 where the grids are this large!

This script is also the one that got me started in to map scripting. I had
this idea early in the development cycle and just kept pestering until Soren
turned me loose on it, finally. Once I got going, I just kept on going!

- Bob Thomas   September 20, 2005

Earth2 NOTES

This is based purely on the Terra script, albeit with a lot more similarity
to Earth in terms of landmasses. Rocky Climate and Normal Sea Levels strongly
recommended for maximum earthiness.

Earth3 NOTES

This script is identical to the Earth2 script, except that the original
civ placement script from Terra/Earth2 has been removed, and replaced with
the civ placement script from LiDiCesare's "Tectonics" script.  Civs will
now be placed anywhere in the New and Old World -- although the new placement script
should have removed the chance of them being located on one or two tile
islands.

Because the Earth3 map is significantly larger than comparable maps of its size,
it is recommended that the players use at least 2-3 more active civilizations
than the default recommendation for that size.  (E.g., use 9 or 10 civs on a
STANDARD sized map instead of the default 7.)

- John Palchak   December 9, 2007

Incredibly huge thanks to Sto from CivFanatics.com, who COMPLETELY re-worked the
code on his own when the placement script started throwing an error!

- John Palchak    January 21, 2008

Tried to make 'Earth3' compatible to 'Planetfall' and introduced user options.
Now uses MapScriptTools.

- Temudjin    August 10, 2009
'''

def isAdvancedMap():
    "This map should show up in simple mode"
    return 0

def getGridSize(argsList):
	"Enlarge the grids! According to Soren, Earth-type maps are usually huge anyway."
	if (argsList[0] == -1): return []			# (-1,) is passed to function on loads

	grid_sizes =	{
						WorldSizeTypes.WORLDSIZE_DUEL:      (10,6),
						WorldSizeTypes.WORLDSIZE_TINY:      (15,9),
						WorldSizeTypes.WORLDSIZE_SMALL:     (20,12),
						WorldSizeTypes.WORLDSIZE_STANDARD:  (25,15),
						WorldSizeTypes.WORLDSIZE_LARGE:     (30,18),
						WorldSizeTypes.WORLDSIZE_HUGE:      (40,24)
						}

########## Temudjin START
	bigGrids = [ (50,30), (60,36) ]							# just change these, or even add some more
	maxWorld = CyGlobalContext().getNumWorldInfos()
	maxSize = min( 6 + len( bigGrids ), maxWorld )
	for i in range( 6, maxSize ):
		grid_sizes[ i ] = bigGrids[ i - 6 ]
########## Temudjin END

	[eWorldSize] = argsList
	return grid_sizes[eWorldSize]


# ------------------------------
# Temudjins Cool Starting Plots
# ------------------------------
# ensures that there are more than 3 land/hill plots within the central 3x3 grid
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

# ensures that plot isn't within ok plots from edge of the world
def okMapEdge( x, y, ok=3 ):
	if not CyMap().isWrapX():
		if ( x < ok ) or ( x > (mst.iNumPlotsX-1-ok) ):
			return False
	if not CyMap().isWrapY():
		if ( y < ok ) or ( y > (mst.iNumPlotsY-1-ok) ):
			return False
	return True
# ----------------------------------
# END Temudjin's Cool Starting Plots
# ----------------------------------


#
# Starting position generation.
# BEGIN STO'S CODE

def assignStartingPlots():

	global bestArea
	gc = CyGlobalContext()
	map = CyMap()
	map.recalculateAreas()
	dice = gc.getGame().getMapRand()
	iPlayers = gc.getGame().countCivPlayersEverAlive()
	areas = CvMapGeneratorUtil.getAreas()
	areaValue = {}

	if iPlayers < 2:
		CyPythonMgr().allowDefaultImpl()
		return

	for area in areas:
		if area.isWater(): continue
		areaValue[area.getID()] = area.calculateTotalBestNatureYield() + area.getNumRiverEdges() + 2 * area.countCoastalLand() + 3 * area.countNumUniqueBonusTypes()
	print " areaValue : %r" %areaValue

	# Shuffle players so the same player doesn't always get the first pick.
	######### Temudjin START
	shuffledPlayers = []
	player_list = mst.mapStats.getCivPlayerList()
	for ply in player_list: shuffledPlayers.append( ply.getID() )
	mst.randomList.shuffle( shuffledPlayers )
#	player_list = []
#	for plrCheckLoop in range(18):
#		if gc.getPlayer(plrCheckLoop).isEverAlive():
#			player_list.append(plrCheckLoop)
#	shuffledPlayers = []
#	for playerLoop in range(iPlayers):
#		iChoosePlayer = dice.get(len(player_list), "Shuffling Players - Highlands PYTHON")
#		shuffledPlayers.append(player_list[iChoosePlayer])
#		del player_list[iChoosePlayer]
	######### Temudjin END

	# Loop through players, assigning starts for each.
	assignedPlots = {}
	bSucceed = True
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
				print " bestAreaValue : %r , bestAreaID : %r " %(bestAreaValue, bestArea)

                def isValid(iPlayer, x, y):
                        plot = map.plot(x,y)
                        if (plot.getArea() != bestArea):
                                return False
                        if plot.getLatitude() >= 75 :
                                return False
                        # Also check for Temudjin's cool starting plots
                        return ( okLandPlots(x,y,10) and okMapEdge(x,y,3) )

                findstart = earth3FindStartingPlot(playerID, isValid)
                if findstart != -1 :
                        sPlot = map.plotByIndex(findstart)
                        if not sPlot.isNone() :
                                assignedPlots[playerID] = findstart
                                sPlot.setStartingPlot(True)
                                player.setStartingPlot(sPlot,True)
                                continue

                # a player can't have a plot so will run a default assgnment
                bSucceed = False
                break

        if not bSucceed :
                print " can't assgin a plot for each player : run default implementation "
                for playerID in assignedPlots.keys() :
                        findstart = assignedPlots[playerID]
                        sPlot = map.plotByIndex(findstart)
                        sPlot.setStartingPlot(False)
                        player.setStartingPlot(sPlot,False)
                CyPythonMgr().allowDefaultImpl()

def earth3FindStartingPlot(playerID, validFn = None):
	gc = CyGlobalContext()
	map = CyMap()
	player = gc.getPlayer(playerID)

	player.AI_updateFoundValues(True)

	iRange = player.startingPlotRange()
	iPass = 0

	while (true):
		iBestValue = 0
		pBestPlot = None

		for iX in range(map.getGridWidth()):
			for iY in range(map.getGridHeight()):
				if validFn != None and not validFn(playerID, iX, iY):
					continue

				pLoopPlot = map.plot(iX, iY)

				val = pLoopPlot.getFoundValue(playerID)

				if val > iBestValue:

					valid = True

					for iI in range(gc.getMAX_CIV_PLAYERS()):
						if (gc.getPlayer(iI).isAlive()):
							if (iI != playerID):
								if gc.getPlayer(iI).startingPlotWithinRange(pLoopPlot, playerID, iRange, iPass):
									valid = False
									break

					if valid:
							iBestValue = val
							pBestPlot = pLoopPlot

		if pBestPlot != None:
			return map.plotNum(pBestPlot.getX(), pBestPlot.getY())

		print "player", playerID, "pass", iPass, "failed"

		iPass += 1
		if iPass > 50 : break

	return -1

# END STO'S CODE

class EarthMultilayeredFractal(CvMapGeneratorUtil.MultilayeredFractal):
    # Subclass. Only the controlling function overridden in this case.
    def generatePlotsByRegion(self):
        # Sirian's MultilayeredFractal class, controlling function.
        # You -MUST- customize this function for each use of the class.
        #
        # The following grain matrix is specific to Earth3.py
        sizekey = self.map.getWorldSize()
        sizevalues = {
            WorldSizeTypes.WORLDSIZE_DUEL:      (3,2,1),
            WorldSizeTypes.WORLDSIZE_TINY:      (3,2,1),
            WorldSizeTypes.WORLDSIZE_SMALL:     (4,2,1),
            WorldSizeTypes.WORLDSIZE_STANDARD:  (4,2,1),
            WorldSizeTypes.WORLDSIZE_LARGE:     (4,2,1),
            WorldSizeTypes.WORLDSIZE_HUGE:      (5,2,1)
            }

        ########## Temudjin START
        if mst.isWorldSize( "GIANT" ):
           sizevalues[6] = (5,2,1)
        if mst.isWorldSize( "GIGANTIC" ):
           sizevalues[7] = (5,2,1)
        ########## Temudjin END

        (ScatterGrain, BalanceGrain, GatherGrain) = sizevalues[sizekey]

        # Sea Level adjustment (from user input), limited to value of 5%.
        sea = self.gc.getSeaLevelInfo(self.map.getSeaLevel()).getSeaLevelChange()
        sea = min(sea, 5)
        sea = max(sea, -5)

        # The following regions are specific to Earth3.py

        NATundraWestLon = 0.05
        NATundraEastLon = 0.21
        GreenlandWestLon = 0.26
        GreenlandEastLon = 0.39
        NAmericasWestLon = 0.10
        NAmericasEastLon = 0.29
        FloridaWestLon = 0.28
        FloridaEastLon = 0.30
        MexicoWestLon = 0.12
        MexicoEastLon = 0.20
        CAmericasWestLon = 0.13
        CAmericasEastLon = 0.25
        PanamaWestLon = 0.21
        PanamaEastLon = 0.25
        CaribWestLon = 0.17
        CaribEastLon = 0.35
        SAmericasWestLon = 0.19
        SAmericasEastLon = 0.33
        BrazilWestLon = 0.21
        BrazilEastLon = 0.37
        AndesWestLon = 0.23
        AndesEastLon = 0.26
        EmeraldWestLon = 0.54
        EmeraldEastLon = 0.58
        NEuropeWestLon = 0.60
        NEuropeEastLon = 0.68
        CEuropeWestLon = 0.54
        CEuropeEastLon = 0.68
        IberiaWestLon = 0.51
        IberiaEastLon = 0.55
        MediWestLon = 0.54
        MediEastLon = 0.68
        NumidiaWestLon = 0.50
        NumidiaEastLon = 0.58
        AfricaWestLon = 0.50
        AfricaEastLon = 0.68
        CAfricaWestLon = 0.58
        CAfricaEastLon = 0.68
        SAfricaWestLon = 0.60
        SAfricaEastLon = 0.66
        MadagascarWestLon = 0.69					# Temudjin NEW
        MadagascarEastLon = 0.72					# Temudjin NEW
        SiberiaWestLon = 0.67
        SiberiaEastLon = 0.95
        SteppeWestLon = 0.67
        SteppeEastLon = 0.92
        NearEastWestLon = 0.67
        NearEastEastLon = 0.75
        ArabiaWestLon = 0.68
        ArabiaEastLon = 0.73
        IndiaWestLon = 0.73
        IndiaEastLon = 0.81
        ChinaWestLon = 0.80
        ChinaEastLon = 0.89
        IndoChinaWestLon = 0.80
        IndoChinaEastLon = 0.92					# Temudjin was 0.94
        JapanWestLon = 0.93						# Temudjin was 0.91
        JapanEastLon = 0.95						# Temudjin was 0.94
        AustraliaWestLon = 0.82					# Temudjin was 0.84
        AustraliaEastLon = 0.96
        NewZealandWestLon = 0.97					# Temudjin NEW
        NewZealandEastLon = 0.99					# Temudjin NEW
        SouthPacificWestLon = 0.01
        SouthPacificEastLon = 0.20
        NATundraNorthLat = 0.94
        NATundraSouthLat = 0.80
        GreenlandNorthLat = 0.90
        GreenlandSouthLat = 0.78
        NAmericasNorthLat = 0.82
        NAmericasSouthLat = 0.65
        FloridaNorthLat = 0.66
        FloridaSouthLat = 0.64
        MexicoNorthLat = 0.70
        MexicoSouthLat = 0.60
        CAmericasNorthLat = 0.65
        CAmericasSouthLat = 0.54
        PanamaNorthLat = 0.55
        PanamaSouthLat = 0.54
        CaribNorthLat = 0.66
        CaribSouthLat = 0.50
        SAmericasNorthLat = 0.55
        SAmericasSouthLat = 0.25
        BrazilNorthLat = 0.50
        BrazilSouthLat = 0.40
        AndesNorthLat = 0.56
        AndesSouthLat = 0.14
        EmeraldNorthLat = 0.86
        EmeraldSouthLat = 0.81
        NEuropeNorthLat = 0.92
        NEuropeSouthLat = 0.80
        CEuropeNorthLat = 0.80
        CEuropeSouthLat = 0.72
        IberiaNorthLat = 0.74
        IberiaSouthLat = 0.68
        MediNorthLat = 0.75
        MediSouthLat = 0.60
        AfricaNorthLat = 0.60
        AfricaSouthLat = 0.44
        NumidiaNorthLat = 0.64
        NumidiaSouthLat = 0.55
        CAfricaNorthLat = 0.46
        CAfricaSouthLat = 0.25
        SAfricaNorthLat = 0.30
        SAfricaSouthLat = 0.18
        MadagascarNorthLat = 0.31				# Temudjin NEW
        MadagascarSouthLat = 0.21				# Temudjin NEW
        SiberiaNorthLat = 0.94
        SiberiaSouthLat = 0.82
        SteppeNorthLat = 0.85
        SteppeSouthLat = 0.70
        NearEastNorthLat = 0.75
        NearEastSouthLat = 0.59
        ArabiaNorthLat = 0.60
        ArabiaSouthLat = 0.55
        IndiaNorthLat = 0.75
        IndiaSouthLat = 0.47
        ChinaNorthLat = 0.75
        ChinaSouthLat = 0.50
        IndoChinaNorthLat = 0.55
        IndoChinaSouthLat = 0.32
        JapanNorthLat = 0.73
        JapanSouthLat = 0.66
        AustraliaNorthLat = 0.30
        AustraliaSouthLat = 0.15
        NewZealandNorthLat = 0.19				# Temudjin NEW
        NewZealandSouthLat = 0.13				# Temudjin NEW
        SouthPacificNorthLat = 0.45
        SouthPacificSouthLat = 0.15

        # Simulate the Western Hemisphere - North American Tundra.
        NiTextOut("Generating North America (Python Earth3) ...")
        # Set dimensions of North American Tundra
        NATundraWestX = int(self.iW * NATundraWestLon)
        NATundraEastX = int(self.iW * NATundraEastLon)
        NATundraNorthY = int(self.iH * NATundraNorthLat)
        NATundraSouthY = int(self.iH * NATundraSouthLat)
        NATundraWidth = NATundraEastX - NATundraWestX + 1
        NATundraHeight = NATundraNorthY - NATundraSouthY + 1

        NATundraWater = 35+sea

        self.generatePlotsInRegion(NATundraWater,
				   NATundraWidth, NATundraHeight,
				   NATundraWestX, NATundraSouthY,
				   BalanceGrain, GatherGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Western Hemisphere - Greenland.
        NiTextOut("Generating North America (Python Earth3) ...")
        # Set dimensions of Greenland
        GreenlandWestX = int(self.iW * GreenlandWestLon)
        GreenlandEastX = int(self.iW * GreenlandEastLon)
        GreenlandNorthY = int(self.iH * GreenlandNorthLat)
        GreenlandSouthY = int(self.iH * GreenlandSouthLat)
        GreenlandWidth = GreenlandEastX - GreenlandWestX + 1
        GreenlandHeight = GreenlandNorthY - GreenlandSouthY + 1

        GreenlandWater = 70+sea

        self.generatePlotsInRegion(GreenlandWater,
				   GreenlandWidth, GreenlandHeight,
				   GreenlandWestX, GreenlandSouthY,
				   ScatterGrain, GatherGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   1, True,
				   False
				   )

        self.generatePlotsInRegion(GreenlandWater,
				   GreenlandWidth, GreenlandHeight,
				   GreenlandWestX, GreenlandSouthY,
				   ScatterGrain, GatherGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   1, True,
				   False
				   )

        # Simulate the Western Hemisphere - North America Mainland.
        NiTextOut("Generating North America (Python Earth3) ...")
        # Set dimensions of North America Mainland
        NAmericasWestX = int(self.iW * NAmericasWestLon)
        NAmericasEastX = int(self.iW * NAmericasEastLon)
        NAmericasNorthY = int(self.iH * NAmericasNorthLat)
        NAmericasSouthY = int(self.iH * NAmericasSouthLat)
        NAmericasWidth = NAmericasEastX - NAmericasWestX + 1
        NAmericasHeight = NAmericasNorthY - NAmericasSouthY + 1

        NAmericasWater = 40+sea

        self.generatePlotsInRegion(NAmericasWater,
				   NAmericasWidth, NAmericasHeight,
				   NAmericasWestX, NAmericasSouthY,
				   GatherGrain, BalanceGrain,
				   self.iVertFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(NAmericasWater,
				   NAmericasWidth, NAmericasHeight,
				   NAmericasWestX, NAmericasSouthY,
				   GatherGrain, BalanceGrain,
				   self.iVertFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Western Hemisphere - Florida.
        NiTextOut("Generating North America (Python Earth3) ...")
        # Set dimensions of Florida
        FloridaWestX = int(self.iW * FloridaWestLon)
        FloridaEastX = int(self.iW * FloridaEastLon)
        FloridaNorthY = int(self.iH * FloridaNorthLat)
        FloridaSouthY = int(self.iH * FloridaSouthLat)
        FloridaWidth = FloridaEastX - FloridaWestX + 1
        FloridaHeight = FloridaNorthY - FloridaSouthY + 1

        FloridaWater = 40+sea

        self.generatePlotsInRegion(FloridaWater,
				   FloridaWidth, FloridaHeight,
				   FloridaWestX, FloridaSouthY,
				   GatherGrain, ScatterGrain,
				   self.iVertFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Western Hemisphere - Mexico.
        NiTextOut("Generating Central America (Python Earth3) ...")
        # Set dimensions of Mexico
        MexicoWestX = int(self.iW * MexicoWestLon)
        MexicoEastX = int(self.iW * MexicoEastLon)
        MexicoNorthY = int(self.iH * MexicoNorthLat)
        MexicoSouthY = int(self.iH * MexicoSouthLat)
        MexicoWidth = MexicoEastX - MexicoWestX + 1
        MexicoHeight = MexicoNorthY - MexicoSouthY + 1

        MexicoWater = 30+sea

        self.generatePlotsInRegion(MexicoWater,
				   MexicoWidth, MexicoHeight,
				   MexicoWestX, MexicoSouthY,
				   GatherGrain, GatherGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Western Hemisphere - Central America.
        NiTextOut("Generating Central America (Python Earth3) ...")
        # Set dimensions of Central America
        CAmericasWestX = int(self.iW * CAmericasWestLon)
        CAmericasEastX = int(self.iW * CAmericasEastLon)
        CAmericasNorthY = int(self.iH * CAmericasNorthLat)
        CAmericasSouthY = int(self.iH * CAmericasSouthLat)
        CAmericasWidth = CAmericasEastX - CAmericasWestX + 1
        CAmericasHeight = CAmericasNorthY - CAmericasSouthY + 1

        CAmericasWater = 80+sea

        self.generatePlotsInRegion(CAmericasWater,
				   CAmericasWidth, CAmericasHeight,
				   CAmericasWestX, CAmericasSouthY,
				   GatherGrain, GatherGrain,
				   self.iVertFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Western Hemisphere - Panama.
        NiTextOut("Generating Central America (Python Earth3) ...")
        # Set dimensions of Panama
        PanamaWestX = int(self.iW * PanamaWestLon)
        PanamaEastX = int(self.iW * PanamaEastLon)
        PanamaNorthY = int(self.iH * PanamaNorthLat)
        PanamaSouthY = int(self.iH * PanamaSouthLat)
        PanamaWidth = PanamaEastX - PanamaWestX + 1
        PanamaHeight = PanamaNorthY - PanamaSouthY + 1

        PanamaWater = 85+sea

        self.generatePlotsInRegion(PanamaWater,
				   PanamaWidth, PanamaHeight,
				   PanamaWestX, PanamaSouthY,
				   GatherGrain, GatherGrain,
				   self.iHorzFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Western Hemisphere - The Caribbean.
        NiTextOut("Generating Central America (Python Earth3) ...")
        # Set dimensions of The Caribbean
        CaribWestX = int(self.iW * CaribWestLon)
        CaribEastX = int(self.iW * CaribEastLon)
        CaribNorthY = int(self.iH * CaribNorthLat)
        CaribSouthY = int(self.iH * CaribSouthLat)
        CaribWidth = CaribEastX - CaribWestX + 1
        CaribHeight = CaribNorthY - CaribSouthY + 1

        CaribWater = 90+sea

        self.generatePlotsInRegion(CaribWater,
				   CaribWidth, CaribHeight,
				   CaribWestX, CaribSouthY,
				   ScatterGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Western Hemisphere - South America.
        NiTextOut("Generating South America (Python Earth3) ...")
        # Set dimensions of South America
        SAmericasWestX = int(self.iW * SAmericasWestLon)
        SAmericasEastX = int(self.iW * SAmericasEastLon)
        SAmericasNorthY = int(self.iH * SAmericasNorthLat)
        SAmericasSouthY = int(self.iH * SAmericasSouthLat)
        SAmericasWidth = SAmericasEastX - SAmericasWestX + 1
        SAmericasHeight = SAmericasNorthY - SAmericasSouthY + 1

        SAmericasWater = 65+sea

        self.generatePlotsInRegion(SAmericasWater,
				   SAmericasWidth, SAmericasHeight,
				   SAmericasWestX, SAmericasSouthY,
				   GatherGrain, GatherGrain,
				   self.iVertFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(SAmericasWater,
				   SAmericasWidth, SAmericasHeight,
				   SAmericasWestX, SAmericasSouthY,
				   GatherGrain, GatherGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Western Hemisphere - Brazil.
        NiTextOut("Generating South America (Python Earth3) ...")
        # Set dimensions of Brazil
        BrazilWestX = int(self.iW * BrazilWestLon)
        BrazilEastX = int(self.iW * BrazilEastLon)
        BrazilNorthY = int(self.iH * BrazilNorthLat)
        BrazilSouthY = int(self.iH * BrazilSouthLat)
        BrazilWidth = BrazilEastX - BrazilWestX + 1
        BrazilHeight = BrazilNorthY - BrazilSouthY + 1

        BrazilWater = 45+sea

        self.generatePlotsInRegion(BrazilWater,
				   BrazilWidth, BrazilHeight,
				   BrazilWestX, BrazilSouthY,
				   GatherGrain, GatherGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(BrazilWater,
				   BrazilWidth, BrazilHeight,
				   BrazilWestX, BrazilSouthY,
				   GatherGrain, GatherGrain,
				   self.iHorzFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Western Hemisphere - Andes.
        NiTextOut("Generating South America (Python Earth3) ...")
        # Set dimensions of Andes
        AndesWestX = int(self.iW * AndesWestLon)
        AndesEastX = int(self.iW * AndesEastLon)
        AndesNorthY = int(self.iH * AndesNorthLat)
        AndesSouthY = int(self.iH * AndesSouthLat)
        AndesWidth = AndesEastX - AndesWestX + 1
        AndesHeight = AndesNorthY - AndesSouthY + 1

        AndesWater = 35+sea

        self.generatePlotsInRegion(AndesWater,
				   AndesWidth, AndesHeight,
				   AndesWestX, AndesSouthY,
				   GatherGrain, GatherGrain,
				   self.iVertFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - British Isles.
        NiTextOut("Generating Europe (Python Earth3) ...")
        # Set dimensions of British Isles
        EmeraldWestX = int(self.iW * EmeraldWestLon)
        EmeraldEastX = int(self.iW * EmeraldEastLon)
        EmeraldNorthY = int(self.iH * EmeraldNorthLat)
        EmeraldSouthY = int(self.iH * EmeraldSouthLat)
        EmeraldWidth = EmeraldEastX - EmeraldWestX + 1
        EmeraldHeight = EmeraldNorthY - EmeraldSouthY + 1

        EmeraldWater = 80+sea

        self.generatePlotsInRegion(EmeraldWater,
				   EmeraldWidth, EmeraldHeight,
				   EmeraldWestX, EmeraldSouthY,
				   BalanceGrain, ScatterGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(EmeraldWater,
				   EmeraldWidth, EmeraldHeight,
				   EmeraldWestX, EmeraldSouthY,
				   BalanceGrain, ScatterGrain,
				   self.iVertFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - Northern Europe.
        NiTextOut("Generating Europe (Python Earth3) ...")
        # Set dimensions of Northern Europe
        NEuropeWestX = int(self.iW * NEuropeWestLon)
        NEuropeEastX = int(self.iW * NEuropeEastLon)
        NEuropeNorthY = int(self.iH * NEuropeNorthLat)
        NEuropeSouthY = int(self.iH * NEuropeSouthLat)
        NEuropeWidth = NEuropeEastX - NEuropeWestX + 1
        NEuropeHeight = NEuropeNorthY - NEuropeSouthY + 1

        NEuropeWater = 50+sea

        self.generatePlotsInRegion(NEuropeWater,
				   NEuropeWidth, NEuropeHeight,
				   NEuropeWestX, NEuropeSouthY,
				   BalanceGrain, GatherGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - Central Europe.
        NiTextOut("Generating Europe (Python Earth3) ...")
        # Set dimensions of Central Europe
        CEuropeWestX = int(self.iW * CEuropeWestLon)
        CEuropeEastX = int(self.iW * CEuropeEastLon)
        CEuropeNorthY = int(self.iH * CEuropeNorthLat)
        CEuropeSouthY = int(self.iH * CEuropeSouthLat)
        CEuropeWidth = CEuropeEastX - CEuropeWestX + 1
        CEuropeHeight = CEuropeNorthY - CEuropeSouthY + 1

        CEuropeWater = 35+sea

        self.generatePlotsInRegion(CEuropeWater,
				   CEuropeWidth, CEuropeHeight,
				   CEuropeWestX, CEuropeSouthY,
				   GatherGrain, GatherGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(CEuropeWater,
				   CEuropeWidth, CEuropeHeight,
				   CEuropeWestX, CEuropeSouthY,
				   GatherGrain, GatherGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - Iberia.
        NiTextOut("Generating Europe (Python Earth3) ...")
        # Set dimensions of Iberia
        IberiaWestX = int(self.iW * IberiaWestLon)
        IberiaEastX = int(self.iW * IberiaEastLon)
        IberiaNorthY = int(self.iH * IberiaNorthLat)
        IberiaSouthY = int(self.iH * IberiaSouthLat)
        IberiaWidth = IberiaEastX - IberiaWestX + 1
        IberiaHeight = IberiaNorthY - IberiaSouthY + 1

        IberiaWater = 20+sea

        self.generatePlotsInRegion(IberiaWater,
				   IberiaWidth, IberiaHeight,
				   IberiaWestX, IberiaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - Mediterranean.
        NiTextOut("Generating Europe (Python Earth3) ...")
        # Set dimensions of Mediterranean
        MediWestX = int(self.iW * MediWestLon)
        MediEastX = int(self.iW * MediEastLon)
        MediNorthY = int(self.iH * MediNorthLat)
        MediSouthY = int(self.iH * MediSouthLat)
        MediWidth = MediEastX - MediWestX + 1
        MediHeight = MediNorthY - MediSouthY + 1

        MediWater = 80+sea

        self.generatePlotsInRegion(MediWater,
				   MediWidth, MediHeight,
				   MediWestX, MediSouthY,
				   ScatterGrain, ScatterGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - North Africa.
        NiTextOut("Generating Africa (Python Earth3) ...")
        # Set dimensions of North Africa
        AfricaWestX = int(self.iW * AfricaWestLon)
        AfricaEastX = int(self.iW * AfricaEastLon)
        AfricaNorthY = int(self.iH * AfricaNorthLat)
        AfricaSouthY = int(self.iH * AfricaSouthLat)
        AfricaWidth = AfricaEastX - AfricaWestX + 1
        AfricaHeight = AfricaNorthY - AfricaSouthY + 1

        AfricaWater = 50+sea

        self.generatePlotsInRegion(AfricaWater,
				   AfricaWidth, AfricaHeight,
				   AfricaWestX, AfricaSouthY,
				   GatherGrain, ScatterGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(AfricaWater,
				   AfricaWidth, AfricaHeight,
				   AfricaWestX, AfricaSouthY,
				   GatherGrain, ScatterGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(AfricaWater,
				   AfricaWidth, AfricaHeight,
				   AfricaWestX, AfricaSouthY,
				   GatherGrain, ScatterGrain,
				   self.iVertFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - Numidia.
        NiTextOut("Generating Africa (Python Earth3) ...")
        # Set dimensions of Numidia
        NumidiaWestX = int(self.iW * NumidiaWestLon)
        NumidiaEastX = int(self.iW * NumidiaEastLon)
        NumidiaNorthY = int(self.iH * NumidiaNorthLat)
        NumidiaSouthY = int(self.iH * NumidiaSouthLat)
        NumidiaWidth = NumidiaEastX - NumidiaWestX + 1
        NumidiaHeight = NumidiaNorthY - NumidiaSouthY + 1

        NumidiaWater = 20+sea

        self.generatePlotsInRegion(NumidiaWater,
				   NumidiaWidth, NumidiaHeight,
				   NumidiaWestX, NumidiaSouthY,
				   GatherGrain, GatherGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - Central Africa.
        NiTextOut("Generating Africa (Python Earth3) ...")
        # Set dimensions of Central Africa
        CAfricaWestX = int(self.iW * CAfricaWestLon)
        CAfricaEastX = int(self.iW * CAfricaEastLon)
        CAfricaNorthY = int(self.iH * CAfricaNorthLat)
        CAfricaSouthY = int(self.iH * CAfricaSouthLat)
        CAfricaWidth = CAfricaEastX - CAfricaWestX + 1
        CAfricaHeight = CAfricaNorthY - CAfricaSouthY + 1

        CAfricaWater = 35+sea

        self.generatePlotsInRegion(CAfricaWater,
				   CAfricaWidth, CAfricaHeight,
				   CAfricaWestX, CAfricaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(CAfricaWater,
				   CAfricaWidth, CAfricaHeight,
				   CAfricaWestX, CAfricaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - South Africa.
        NiTextOut("Generating Africa (Python Earth3) ...")
        # Set dimensions of South Africa
        SAfricaWestX = int(self.iW * SAfricaWestLon)
        SAfricaEastX = int(self.iW * SAfricaEastLon)
        SAfricaNorthY = int(self.iH * SAfricaNorthLat)
        SAfricaSouthY = int(self.iH * SAfricaSouthLat)
        SAfricaWidth = SAfricaEastX - SAfricaWestX + 1
        SAfricaHeight = SAfricaNorthY - SAfricaSouthY + 1

        SAfricaWater = 45+sea

        self.generatePlotsInRegion(SAfricaWater,
				   SAfricaWidth, SAfricaHeight,
				   SAfricaWestX, SAfricaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(SAfricaWater,
				   SAfricaWidth, SAfricaHeight,
				   SAfricaWestX, SAfricaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

###### Temudjin START
        # Simulate the Eastern Hemisphere - Madagascar.
        NiTextOut("Generating Madagascar (Python Earth3) ...")
        # Set dimensions of Madagascar
        MadagascarWestX = int(self.iW * MadagascarWestLon)
        MadagascarEastX = int(self.iW * MadagascarEastLon)
        MadagascarNorthY = int(self.iH * MadagascarNorthLat)
        MadagascarSouthY = int(self.iH * MadagascarSouthLat)
        MadagascarWidth = MadagascarEastX - MadagascarWestX + 1
        MadagascarHeight = MadagascarNorthY - MadagascarSouthY + 1

        MadagascarWater = 70+sea

        self.generatePlotsInRegion(MadagascarWater,
				   MadagascarWidth, MadagascarHeight,
				   MadagascarWestX, MadagascarSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(MadagascarWater,
				   MadagascarWidth, MadagascarHeight,
				   MadagascarWestX, MadagascarSouthY-1,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )
###### Temudjin END

        # Simulate the Eastern Hemisphere - Siberia.
        NiTextOut("Generating Asia (Python Earth3) ...")
        # Set dimensions of Siberia
        SiberiaWestX = int(self.iW * SiberiaWestLon)
        SiberiaEastX = int(self.iW * SiberiaEastLon)
        SiberiaNorthY = int(self.iH * SiberiaNorthLat)
        SiberiaSouthY = int(self.iH * SiberiaSouthLat)
        SiberiaWidth = SiberiaEastX - SiberiaWestX + 1
        SiberiaHeight = SiberiaNorthY - SiberiaSouthY + 1

        SiberiaWater = 25+sea

        self.generatePlotsInRegion(SiberiaWater,
				   SiberiaWidth, SiberiaHeight,
				   SiberiaWestX, SiberiaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - Steppe.
        NiTextOut("Generating Asia (Python Earth3) ...")
        # Set dimensions of Steppe
        SteppeWestX = int(self.iW * SteppeWestLon)
        SteppeEastX = int(self.iW * SteppeEastLon)
        SteppeNorthY = int(self.iH * SteppeNorthLat)
        SteppeSouthY = int(self.iH * SteppeSouthLat)
        SteppeWidth = SteppeEastX - SteppeWestX + 1
        SteppeHeight = SteppeNorthY - SteppeSouthY + 1

        SteppeWater = 6+sea

        self.generatePlotsInRegion(SteppeWater,
				   SteppeWidth, SteppeHeight,
				   SteppeWestX, SteppeSouthY,
				   GatherGrain, GatherGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - Near East.
        NiTextOut("Generating Asia (Python Earth3) ...")
        # Set dimensions of Near East
        NearEastWestX = int(self.iW * NearEastWestLon)
        NearEastEastX = int(self.iW * NearEastEastLon)
        NearEastNorthY = int(self.iH * NearEastNorthLat)
        NearEastSouthY = int(self.iH * NearEastSouthLat)
        NearEastWidth = NearEastEastX - NearEastWestX + 1
        NearEastHeight = NearEastNorthY - NearEastSouthY + 1

        NearEastWater = 50+sea

        self.generatePlotsInRegion(NearEastWater,
				   NearEastWidth, NearEastHeight,
				   NearEastWestX, NearEastSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(NearEastWater,
				   NearEastWidth, NearEastHeight,
				   NearEastWestX, NearEastSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - Arabia.
        NiTextOut("Generating Asia (Python Earth3) ...")
        # Set dimensions of Arabia
        ArabiaWestX = int(self.iW * ArabiaWestLon)
        ArabiaEastX = int(self.iW * ArabiaEastLon)
        ArabiaNorthY = int(self.iH * ArabiaNorthLat)
        ArabiaSouthY = int(self.iH * ArabiaSouthLat)
        ArabiaWidth = ArabiaEastX - ArabiaWestX + 1
        ArabiaHeight = ArabiaNorthY - ArabiaSouthY + 1

        ArabiaWater = 50+sea

        self.generatePlotsInRegion(ArabiaWater,
				   ArabiaWidth, ArabiaHeight,
				   ArabiaWestX, ArabiaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iVertFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - India.
        NiTextOut("Generating Asia (Python Earth3) ...")
        # Set dimensions of India
        IndiaWestX = int(self.iW * IndiaWestLon)
        IndiaEastX = int(self.iW * IndiaEastLon)
        IndiaNorthY = int(self.iH * IndiaNorthLat)
        IndiaSouthY = int(self.iH * IndiaSouthLat)
        IndiaWidth = IndiaEastX - IndiaWestX + 1
        IndiaHeight = IndiaNorthY - IndiaSouthY + 1

        IndiaWater = 33+sea

        self.generatePlotsInRegion(IndiaWater,
				   IndiaWidth, IndiaHeight,
				   IndiaWestX, IndiaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(IndiaWater,
				   IndiaWidth, IndiaHeight,
				   IndiaWestX, IndiaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - China.
        NiTextOut("Generating Asia (Python Earth3) ...")
        # Set dimensions of China
        ChinaWestX = int(self.iW * ChinaWestLon)
        ChinaEastX = int(self.iW * ChinaEastLon)
        ChinaNorthY = int(self.iH * ChinaNorthLat)
        ChinaSouthY = int(self.iH * ChinaSouthLat)
        ChinaWidth = ChinaEastX - ChinaWestX + 1
        ChinaHeight = ChinaNorthY - ChinaSouthY + 1

        ChinaWater = 65+sea

        self.generatePlotsInRegion(ChinaWater,
				   ChinaWidth, ChinaHeight,
				   ChinaWestX, ChinaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(ChinaWater,
				   ChinaWidth, ChinaHeight,
				   ChinaWestX, ChinaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iHorzFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(ChinaWater,
				   ChinaWidth, ChinaHeight,
				   ChinaWestX, ChinaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iHorzFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - IndoChina.
        NiTextOut("Generating Asia (Python Earth3) ...")
        # Set dimensions of IndoChina
        IndoChinaWestX = int(self.iW * IndoChinaWestLon)
        IndoChinaEastX = int(self.iW * IndoChinaEastLon)
        IndoChinaNorthY = int(self.iH * IndoChinaNorthLat)
        IndoChinaSouthY = int(self.iH * IndoChinaSouthLat)
        IndoChinaWidth = IndoChinaEastX - IndoChinaWestX + 1
        IndoChinaHeight = IndoChinaNorthY - IndoChinaSouthY + 1

        IndoChinaWater = 75+sea			# Temudjin was 82

        self.generatePlotsInRegion(IndoChinaWater,
				   IndoChinaWidth, IndoChinaHeight,
				   IndoChinaWestX, IndoChinaSouthY,
				   ScatterGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(IndoChinaWater,
				   IndoChinaWidth, IndoChinaHeight,
				   IndoChinaWestX, IndoChinaSouthY,
				   ScatterGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - Japan.
        NiTextOut("Generating Asia (Python Earth3) ...")
        # Set dimensions of Japan
        JapanWestX = int(self.iW * JapanWestLon)
        JapanEastX = int(self.iW * JapanEastLon)
        JapanNorthY = int(self.iH * JapanNorthLat)
        JapanSouthY = int(self.iH * JapanSouthLat)
        JapanWidth = JapanEastX - JapanWestX + 1
        JapanHeight = JapanNorthY - JapanSouthY + 1

        JapanWater = 82+sea										# Temudjin was 92

        self.generatePlotsInRegion(JapanWater,
				   JapanWidth, JapanHeight,
				   JapanWestX+1, JapanSouthY+1,					# Temudjin ..X+1, ..Y+1
				   BalanceGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(JapanWater,
				   JapanWidth, JapanHeight,
				   JapanWestX, JapanSouthY,
				   BalanceGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # Simulate the Eastern Hemisphere - Australia.
        NiTextOut("Generating Australia (Python Earth3) ...")
        # Set dimensions of Australia
        AustraliaWestX = int(self.iW * AustraliaWestLon)
        AustraliaEastX = int(self.iW * AustraliaEastLon)
        AustraliaNorthY = int(self.iH * AustraliaNorthLat)
        AustraliaSouthY = int(self.iH * AustraliaSouthLat)
        AustraliaWidth = AustraliaEastX - AustraliaWestX + 1
        AustraliaHeight = AustraliaNorthY - AustraliaSouthY + 1

        AustraliaWater = 45+sea

        self.generatePlotsInRegion(AustraliaWater,
				   AustraliaWidth, AustraliaHeight,
				   AustraliaWestX, AustraliaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(AustraliaWater,
				   AustraliaWidth, AustraliaHeight,
				   AustraliaWestX, AustraliaSouthY,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

###### Temudjin START
        # Simulate the Eastern Hemisphere - New Zealand.
        NiTextOut("Generating New Zealand (Python Earth3) ...")
        # Set dimensions of New Zealand
        NewZealandWestX = int(self.iW * NewZealandWestLon)
        NewZealandEastX = int(self.iW * NewZealandEastLon)
        NewZealandNorthY = int(self.iH * NewZealandNorthLat)
        NewZealandSouthY = int(self.iH * NewZealandSouthLat)
        NewZealandWidth = NewZealandEastX - NewZealandWestX + 1
        NewZealandHeight = NewZealandNorthY - NewZealandSouthY + 1

        NewZealandWater = 77+sea

        self.generatePlotsInRegion(NewZealandWater,
				   NewZealandWidth, NewZealandHeight,
				   NewZealandWestX+1, NewZealandSouthY+1,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        self.generatePlotsInRegion(NewZealandWater,
				   NewZealandWidth, NewZealandHeight,
				   NewZealandWestX-1, NewZealandSouthY-1,
				   GatherGrain, BalanceGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )
###### Temudjin END

        # Simulate the South Pacific - South Pacific.
        NiTextOut("Generating Pacific (Python Earth3) ...")
        # Set dimensions of South Pacific
        SouthPacificWestX = int(self.iW * SouthPacificWestLon)
        SouthPacificEastX = int(self.iW * SouthPacificEastLon)
        SouthPacificNorthY = int(self.iH * SouthPacificNorthLat)
        SouthPacificSouthY = int(self.iH * SouthPacificSouthLat)
        SouthPacificWidth = SouthPacificEastX - SouthPacificWestX + 1
        SouthPacificHeight = SouthPacificNorthY - SouthPacificSouthY + 1

        SouthPacificWater = 94+sea

        self.generatePlotsInRegion(SouthPacificWater,
				   SouthPacificWidth, SouthPacificHeight,
				   SouthPacificWestX, SouthPacificSouthY,
				   ScatterGrain, ScatterGrain,
				   self.iRoundFlags, self.iTerrainFlags,
				   5, 5,
				   True, 5,
				   -1, False,
				   False
				   )

        # All regions have been processed. Plot Type generation completed.
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
	NiTextOut("Setting Plot Types (Python Earth3) ...")
	global plotTypes
	gc = CyGlobalContext()
	mmap = gc.getMap()
	MapWidth = mmap.getGridWidth()
	MapHeight = mmap.getGridHeight()
	plotTypes = [PlotTypes.PLOT_OCEAN] * (MapWidth*MapHeight)

	# Call generatePlotsByRegion() function, from TerraMultilayeredFractal subclass.
	plotgen = EarthMultilayeredFractal()
	plotTypes = plotgen.generatePlotsByRegion()
	return plotTypes

# :Temudjin: removed getLatitudeAtPlot(), to enable auto-loading of corrected version
#  - error in function getLatitudeAtPlot() corrected in bts 3.19 unofficial patch
class Earth3TerrainGenerator(CvMapGeneratorUtil.TerrainGenerator):
	def __init__(self, iDesertPercent=33, iPlainsPercent=26,
					 fSnowLatitude=0.85, fTundraLatitude=0.80,
					 fGrassLatitude=0.1, fDesertBottomLatitude=0.1,
					 fDesertTopLatitude=0.35, fracXExp=-1,
					 fracYExp=-1, grain_amount=3):
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
		self.variation=CyFractal()

		iDesertPercent += self.gc.getClimateInfo(self.map.getClimate()).getDesertPercentChange()
		iDesertPercent = min(iDesertPercent, 100)
		iDesertPercent = max(iDesertPercent, 0)

		self.iDesertPercent = iDesertPercent
		self.iPlainsPercent = iPlainsPercent

		self.iDesertTopPercent = 100
		self.iDesertBottomPercent = max(0,int(100-iDesertPercent))
		self.iPlainsTopPercent = 100
		self.iPlainsBottomPercent = max(0,int(100-iDesertPercent-iPlainsPercent))
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

		self.variation.fracInit(self.iWidth, self.iHeight, self.grain_amount, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)

		self.terrainDesert = self.gc.getInfoTypeForString("TERRAIN_DESERT")
		self.terrainPlains = self.gc.getInfoTypeForString("TERRAIN_PLAINS")
		self.terrainIce = self.gc.getInfoTypeForString("TERRAIN_SNOW")
		self.terrainTundra = self.gc.getInfoTypeForString("TERRAIN_TUNDRA")
		self.terrainGrass = self.gc.getInfoTypeForString("TERRAIN_GRASS")

	def generateTerrain(self):
		terrainData = [0]*(self.iWidth*self.iHeight)
		for x in range(self.iWidth):
			for y in range(self.iHeight):
				iI = y*self.iWidth + x
				terrain = self.generateTerrainAtPlot(x, y)
				terrainData[iI] = terrain
		return terrainData

	def generateTerrainAtPlot(self,iX,iY):
		lat = self.getLatitudeAtPlot(iX,iY)

		if (self.map.plot(iX, iY).isWater()):
			return self.map.plot(iX, iY).getTerrainType()

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
			if ((desertVal >= self.iDesertBottom) and (desertVal <= self.iDesertTop) and (lat >= self.fDesertBottomLatitude) and (lat < self.fDesertTopLatitude)):
				terrainVal = self.terrainDesert
			elif ((plainsVal >= self.iPlainsBottom) and (plainsVal <= self.iPlainsTop)):
				terrainVal = self.terrainPlains

		if (terrainVal == TerrainTypes.NO_TERRAIN):
			return self.map.plot(iX, iY).getTerrainType()

		return terrainVal
