#
#	FILE:	 Highlands.py
#	AUTHOR:  Bob Thomas (Sirian)
#	PURPOSE: Regional map script - mountainous terrain
#-----------------------------------------------------------------------------
#	Copyright (c) 2005 Firaxis Games, Inc. All rights reserved.
#-----------------------------------------------------------------------------
#

from CvPythonExtensions import *
import CvUtil
import CvMapGeneratorUtil
import random
import sys
from math import sqrt
from CvMapGeneratorUtil import FractalWorld
from CvMapGeneratorUtil import TerrainGenerator
from CvMapGeneratorUtil import FeatureGenerator

# Rise of Mankind start 2.5
from CvMapGeneratorUtil import BonusBalancer

balancer = BonusBalancer()

shiftMultiplier = 0.0
# Rise of Mankind end 2.5

# Rise of Mankind 2.53
#which map size was selected
sizeSelected = 0
# Rise of Mankind 2.53

def getDescription():
	return "TXT_KEY_MAP_SCRIPT_HIGHLANDS_DESCR"

def getNumCustomMapOptions():
	return 5
	
def getNumHiddenCustomMapOptions():
	return 2

def getCustomMapOptionName(argsList):
	[iOption] = argsList
	option_names = {
		0:	"TXT_KEY_MAP_SCRIPT_MOUNTAIN_PATTERN",
		1:	"TXT_KEY_MAP_SCRIPT_MOUNTAIN_DENSITY",
		2:	"TXT_KEY_MAP_SCRIPT_WATER_SETTING",
		3:  "TXT_KEY_MAP_WORLD_WRAP",
		4:  "TXT_KEY_CONCEPT_RESOURCES"
		}
	translated_text = unicode(CyTranslator().getText(option_names[iOption], ()))
	return translated_text
	
def getNumCustomMapOptionValues(argsList):
	[iOption] = argsList
	option_values = {
		0:	3,
		1:	3,
		2:	3,
		3:  3,
		4:  2
		}
	return option_values[iOption]
	
def getCustomMapOptionDescAt(argsList):
	[iOption, iSelection] = argsList
	selection_names = {
		0:	{
			0: "TXT_KEY_MAP_SCRIPT_SCATTERED",
			1: "TXT_KEY_MAP_SCRIPT_RIDGELINES",
			2: "TXT_KEY_MAP_SCRIPT_CLUSTERED"
			},
		1:	{
			0: "TXT_KEY_MAP_SCRIPT_DENSE_PEAKS",
			1: "TXT_KEY_MAP_SCRIPT_NORMAL_PEAKS",
			2: "TXT_KEY_MAP_SCRIPT_THIN_PEAKS"
			},
		2:	{
			0: "TXT_KEY_MAP_SCRIPT_SMALL_LAKES",
			1: "TXT_KEY_MAP_SCRIPT_LARGE_LAKES",
			2: "TXT_KEY_MAP_SCRIPT_SEAS"
			},
		3:	{
			0: "TXT_KEY_MAP_WRAP_FLAT",
			1: "TXT_KEY_MAP_WRAP_CYLINDER",
			2: "TXT_KEY_MAP_WRAP_TOROID"
			},
		4:	{
			0: "TXT_KEY_WORLD_STANDARD",
			1: "TXT_KEY_MAP_BALANCED"
			}
		}
	translated_text = unicode(CyTranslator().getText(selection_names[iOption][iSelection], ()))
	return translated_text
	
def getCustomMapOptionDefault(argsList):
	[iOption] = argsList
	option_defaults = {
		0:	1,
		1:	1,
		2:	0,
		3:  0,
		4:  0
		}
	return option_defaults[iOption]

def isRandomCustomMapOption(argsList):
	[iOption] = argsList
	option_random = {
		0:	true,
		1:	true,
		2:	true,
		3:	false,
		4:  false
		}
	return option_random[iOption]

def isAdvancedMap():
	"This map should show up in simple mode"
	return 0
	
def isClimateMap():
	return 0

def isSeaLevelMap():
	return 0

def beforeInit():
	# Roll a dice to determine if the cold region will be in north or south.
	gc = CyGlobalContext()
	dice = gc.getGame().getMapRand()
	shiftRoll = dice.get(2, "North or South climate shift - Highlands PYTHON")
	if shiftRoll == 0: # Cold in north
		shiftMultiplier = 0.0
	else: # Cold in south
		shiftMultiplier = 1.0
	return 0

def getWrapX():
	map = CyMap()
	return (map.getCustomMapOption(3) == 1 or map.getCustomMapOption(3) == 2)
	
def getWrapY():
	map = CyMap()
	return (map.getCustomMapOption(3) == 2)

def normalizeAddExtras():
	if (CyMap().getCustomMapOption(4) == 1):
		balancer.normalizeAddExtras()
	CyPythonMgr().allowDefaultImpl()	# do the rest of the usual normalizeStartingPlots stuff, don't overrride

def addBonusType(argsList):
	[iBonusType] = argsList
	gc = CyGlobalContext()
	type_string = gc.getBonusInfo(iBonusType).getType()

	if (CyMap().getCustomMapOption(4) == 1):
		if (type_string in balancer.resourcesToBalance) or (type_string in balancer.resourcesToEliminate):
			return None # don't place any of this bonus randomly
		
	CyPythonMgr().allowDefaultImpl() # pretend we didn't implement this method, and let C handle this bonus in the default way

def getTopLatitude():
	if shiftMultiplier == 0.0:
		return 85
	else:
		return 10
	
def getBottomLatitude():
	if shiftMultiplier == 0.0:
		return -10
	else:
		return -85

def getGridSize(argsList):
	"Reduce grid sizes by one level."
	grid_sizes = {
		WorldSizeTypes.WORLDSIZE_DUEL:		(8,5),
		WorldSizeTypes.WORLDSIZE_TINY:		(10,6),
		WorldSizeTypes.WORLDSIZE_SMALL:		(13,8),
		WorldSizeTypes.WORLDSIZE_STANDARD:	(16,10),
		WorldSizeTypes.WORLDSIZE_LARGE:		(21,13),
		WorldSizeTypes.WORLDSIZE_HUGE:		(26,16)
	}

	if (argsList[0] == -1): # (-1,) is passed to function on loads
		return []
	[eWorldSize] = argsList
# Rise of Mankind 2.53 - giant and gigantic mapsize fix	
	global sizeSelected
	sizeSelected = eWorldSize
	print "    sizeSelected",sizeSelected
	# Giant size
	if ( sizeSelected == 6 ):
		return (32, 20)
	# Gigantic size
	elif ( sizeSelected == 7 ):
		return (40, 25)
# Rise of Mankind 2.53 - giant and gigantic mapsize fix			
	return grid_sizes[eWorldSize]

def minStartingDistanceModifier():
	return -35

def generatePlotTypes():
	NiTextOut("Setting Plot Types (Python Highlands) ...")
	gc = CyGlobalContext()
	map = CyMap()
	dice = gc.getGame().getMapRand()
	iFlags = 0  # Disallow FRAC_POLAR flag, to prevent "zero row" problems.
	iW = map.getGridWidth()
	iH = map.getGridHeight()
	plotTypes = [PlotTypes.PLOT_LAND] * (iW*iH)
	terrainFrac = CyFractal()
	lakesFrac = CyFractal()
	
	# Get custom map user inputs.
	userInputGrain = map.getCustomMapOption(0)
	userInputPeaks = map.getCustomMapOption(1)
	userInputLakes = map.getCustomMapOption(2)

	# Varying grains for hills/peaks per map size and Mountain Ranges setting.
	# [clustered_grain, ridgelines_grain, scattered_grain]
	worldsizes = {
		WorldSizeTypes.WORLDSIZE_DUEL:      [3,4,5],
		WorldSizeTypes.WORLDSIZE_TINY:      [3,4,5],
		WorldSizeTypes.WORLDSIZE_SMALL:     [4,5,6],
		WorldSizeTypes.WORLDSIZE_STANDARD:  [4,5,6],
		WorldSizeTypes.WORLDSIZE_LARGE:     [4,5,6],
		WorldSizeTypes.WORLDSIZE_HUGE:      [4,5,6]
		}
# Rise of Mankind 2.53
	map_size = map.getWorldSize()
	if ( not map_size in worldsizes ):
		grain_list = [4,5,6]
	else:
		grain_list = worldsizes[map_size]
# Rise of Mankind 2.53		
	grain_list.reverse()
	grain = grain_list[userInputGrain]
	
	# Peak density
	peak_list = [70, 77, 83]
	hill_list = [40, 45, 50]
	peaks = peak_list[userInputPeaks]
	hills = hill_list[userInputPeaks]
	
	# Lake density
	lake_list = [5, 10, 15]
	lake_grains = [5, 4, 3]
	lakes = lake_list[userInputLakes]
	lake_grain = lake_grains[userInputLakes]

	terrainFrac.fracInit(iW, iH, grain, dice, 0, -1, -1)
	lakesFrac.fracInit(iW, iH, lake_grain, dice, 0, -1, -1)

	iLakesThreshold = lakesFrac.getHeightFromPercent(lakes)
	iHillsThreshold = terrainFrac.getHeightFromPercent(hills)
	iPeaksThreshold = terrainFrac.getHeightFromPercent(peaks)

	# Now the main loop, which will assign the plot types.
	for x in range(iW):
		for y in range(iH):
			i = y*iW + x
			lakeVal = lakesFrac.getHeight(x,y)
			val = terrainFrac.getHeight(x,y)
			if lakeVal <= iLakesThreshold:
				plotTypes[i] = PlotTypes.PLOT_OCEAN
			elif val >= iPeaksThreshold:
				plotTypes[i] = PlotTypes.PLOT_PEAK
			elif val >= iHillsThreshold and val < iPeaksThreshold:
				plotTypes[i] = PlotTypes.PLOT_HILLS
			else:
				plotTypes[i] = PlotTypes.PLOT_LAND

	return plotTypes

# subclass TerrainGenerator to redefine everything. This is a regional map.
class HighlandsTerrainGenerator(CvMapGeneratorUtil.TerrainGenerator):
	def __init__(self, fracXExp=-1, fracYExp=-1):
		# Note: If you change longitude values here, then you will...
		# ...need to change them elsewhere in the script, as well.
		self.gc = CyGlobalContext()
		self.map = CyMap()

		self.grain_amount = 4 + self.gc.getWorldInfo(self.map.getWorldSize()).getTerrainGrainChange()

		self.iWidth = self.map.getGridWidth()
		self.iHeight = self.map.getGridHeight()

		self.mapRand = self.gc.getGame().getMapRand()

		self.iFlags = 0  # Disallow FRAC_POLAR flag, to prevent "zero row" problems.

		self.cold=CyFractal()
		self.cool=CyFractal()
		self.temp=CyFractal()
		self.hot=CyFractal()
		self.variation=CyFractal()

		self.iColdIBottomPercent = 75
		self.iColdTBottomPercent = 20
		self.iCoolTBottomPercent = 85
		self.iCoolPBottomPercent = 45
		self.iTempDBottomPercent = 90
		self.iTempPBottomPercent = 65
		self.iHotDBottomPercent = 70
		self.iHotPBottomPercent = 60

		self.fColdLatitude = 0.8
		self.fCoolLatitude = 0.6
		self.fHotLatitude = 0.2

		self.fracXExp = fracXExp
		self.fracYExp = fracYExp

		self.initFractals()
		
	def initFractals(self):
		self.cold.fracInit(self.iWidth, self.iHeight, self.grain_amount, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		self.iColdIBottom = self.cold.getHeightFromPercent(self.iColdIBottomPercent)
		self.iColdTBottom = self.cold.getHeightFromPercent(self.iColdTBottomPercent)

		self.cool.fracInit(self.iWidth, self.iHeight, self.grain_amount, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		self.iCoolTBottom = self.cool.getHeightFromPercent(self.iCoolTBottomPercent)
		self.iCoolPBottom = self.cool.getHeightFromPercent(self.iCoolPBottomPercent)

		self.temp.fracInit(self.iWidth, self.iHeight, self.grain_amount, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		self.iTempDBottom = self.temp.getHeightFromPercent(self.iTempDBottomPercent)
		self.iTempPBottom = self.temp.getHeightFromPercent(self.iTempPBottomPercent)

		self.hot.fracInit(self.iWidth, self.iHeight, self.grain_amount, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		self.iHotDBottom = self.hot.getHeightFromPercent(self.iHotDBottomPercent)
		self.iHotPBottom = self.hot.getHeightFromPercent(self.iHotPBottomPercent)

		self.variation.fracInit(self.iWidth, self.iHeight, self.grain_amount, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)

		self.terrainDesert = self.gc.getInfoTypeForString("TERRAIN_DESERT")
		self.terrainPlains = self.gc.getInfoTypeForString("TERRAIN_PLAINS")
		self.terrainGrass = self.gc.getInfoTypeForString("TERRAIN_GRASS")
		self.terrainIce = self.gc.getInfoTypeForString("TERRAIN_SNOW")
		self.terrainTundra = self.gc.getInfoTypeForString("TERRAIN_TUNDRA")
# Rise of Mankind start 2.5
		self.terrainMarsh = self.gc.getInfoTypeForString("TERRAIN_MARSH")
# Rise of Mankind end 2.5

	def getLatitudeAtPlot(self, iX, iY):
		lat = iY/float(self.iHeight) # 0.0 = south

		# Adjust latitude using self.variation fractal, to mix things up:
		lat += (128 - self.variation.getHeight(iX, iY))/(255.0 * 5.0)

		# Limit to the range [0, 1]:
		if lat < 0:
			lat = 0.0
		if lat > 1:
			lat = 1.0
		
		# Flip terrain if southward shift was rolled.
		fLatitude = abs(lat - shiftMultiplier)

		return fLatitude

	def generateTerrainAtPlot(self,iX,iY):
		lat = self.getLatitudeAtPlot(iX,iY)

		if (self.map.plot(iX, iY).isWater()):
			return self.map.plot(iX, iY).getTerrainType()

		if lat >= self.fColdLatitude:
			val = self.cold.getHeight(iX, iY)
			if val >= self.iColdIBottom:
				terrainVal = self.terrainIce
			elif val >= self.iColdTBottom and val < self.iColdIBottom:
				terrainVal = self.terrainTundra
			else:
# Rise of Mankind start 2.5
				terrainVal = self.terrainMarsh
				# terrainVal = self.terrainPlains
# Rise of Mankind end 2.5
		elif lat < self.fColdLatitude and lat >= self.fCoolLatitude:
			val = self.cool.getHeight(iX, iY)
			if val >= self.iCoolTBottom:
				terrainVal = self.terrainTundra
			elif val >= self.iCoolPBottom and val < self.iCoolTBottom:
				terrainVal = self.terrainPlains
			else:
				terrainVal = self.terrainGrass
		elif lat < self.fHotLatitude:
			val = self.hot.getHeight(iX, iY)
			if val >= self.iHotDBottom:
				terrainVal = self.terrainDesert
			elif val >= self.iHotPBottom and val < self.iHotDBottom:
				terrainVal = self.terrainPlains
			else:
				terrainVal = self.terrainGrass
		else:
			val = self.temp.getHeight(iX, iY)
			if val >= self.iTempDBottom:
				terrainVal = self.terrainDesert
			elif val < self.iTempDBottom and val >= self.iTempPBottom:
				terrainVal = self.terrainPlains
			else:
				terrainVal = self.terrainGrass

		if (terrainVal == TerrainTypes.NO_TERRAIN):
			return self.map.plot(iX, iY).getTerrainType()

		return terrainVal

def generateTerrainTypes():
	NiTextOut("Generating Terrain (Python Highlands) ...")
	terraingen = HighlandsTerrainGenerator()
	terrainTypes = terraingen.generateTerrain()
	return terrainTypes

class HighlandsFeatureGenerator(CvMapGeneratorUtil.FeatureGenerator):
	def __init__(self, iJunglePercent=60, iForestPercent=45, iHotForestPercent = 25,
	             forest_grain=6, fracXExp=-1, fracYExp=-1):
		self.gc = CyGlobalContext()
		self.map = CyMap()
		self.mapRand = self.gc.getGame().getMapRand()
		self.forests = CyFractal()
		
		self.iFlags = 0  # Disallow FRAC_POLAR flag, to prevent "zero row" problems.

		self.iGridW = self.map.getGridWidth()
		self.iGridH = self.map.getGridHeight()
		
		self.iJunglePercent = iJunglePercent
		self.iForestPercent = iForestPercent
		self.iHotForestPercent = iHotForestPercent
		
		self.forest_grain = forest_grain + self.gc.getWorldInfo(self.map.getWorldSize()).getFeatureGrainChange()

		self.fracXExp = fracXExp
		self.fracYExp = fracYExp

		self.__initFractals()
		self.__initFeatureTypes()
	
	def __initFractals(self):
		self.forests.fracInit(self.iGridW, self.iGridH, self.forest_grain, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		
		self.iJungleLevel = self.forests.getHeightFromPercent(100 - self.iJunglePercent)
		self.iForestLevel = self.forests.getHeightFromPercent(self.iForestPercent)
		self.iHotForestLevel = self.forests.getHeightFromPercent(self.iHotForestPercent)
		
	def __initFeatureTypes(self):
		self.featureJungle = self.gc.getInfoTypeForString("FEATURE_JUNGLE")
		self.featureForest = self.gc.getInfoTypeForString("FEATURE_FOREST")
		self.featureOasis = self.gc.getInfoTypeForString("FEATURE_OASIS")

	def getLatitudeAtPlot(self, iX, iY):
		lat = iY/float(self.iGridH) # 0.0 = south
		# Flip terrain if southward shift was rolled.
		return abs(lat - shiftMultiplier)

	def addFeaturesAtPlot(self, iX, iY):
		lat = self.getLatitudeAtPlot(iX, iY)
		pPlot = self.map.sPlot(iX, iY)

		for iI in range(self.gc.getNumFeatureInfos()):
			if pPlot.canHaveFeature(iI):
				if self.mapRand.get(10000, "Add Feature PYTHON") < self.gc.getFeatureInfo(iI).getAppearanceProbability():
					pPlot.setFeatureType(iI, -1)

		if (pPlot.getFeatureType() == FeatureTypes.NO_FEATURE):
			self.addJunglesAtPlot(pPlot, iX, iY, lat)
			
		if (pPlot.getFeatureType() == FeatureTypes.NO_FEATURE):
			self.addForestsAtPlot(pPlot, iX, iY, lat)
		
	def addIceAtPlot(self, pPlot, iX, iY, lat):
		# We don' need no steeking ice. M'kay? Alrighty then.
		ice = 0
	
	def addJunglesAtPlot(self, pPlot, iX, iY, lat):
		# Warning: this version of JunglesAtPlot is using the forest fractal!
		if lat < 0.17 and pPlot.canHaveFeature(self.featureJungle):
			if (self.forests.getHeight(iX, iY) >= self.iJungleLevel):
				pPlot.setFeatureType(self.featureJungle, -1)

	def addForestsAtPlot(self, pPlot, iX, iY, lat):
		if lat > 0.2:
			if pPlot.canHaveFeature(self.featureForest):
				if self.forests.getHeight(iX, iY) <= self.iForestLevel:
					pPlot.setFeatureType(self.featureForest, -1)
		else:
			if pPlot.canHaveFeature(self.featureForest):
				if self.forests.getHeight(iX, iY) <= self.iHotForestLevel:
					pPlot.setFeatureType(self.featureForest, -1)

def addFeatures():
	global featuregen
	NiTextOut("Adding Features (Python Highlands) ...")
	featuregen = HighlandsFeatureGenerator()
	featuregen.addFeatures()
	return 0

def assignStartingPlots():
	# In order to prevent "pockets" from forming, where civs can be blocked in 
	# by Peaks or lakes, causing a "dud" map, pathing must be checked for each 
	# new start plot before it hits the map. Any pockets that are detected must 
	# be opened. The following process takes care of this need. Soren created a 
	# useful function that already lets you know how far a given plot is from
	# the closest nearest civ already on the board. MinOriginalStartDist is that 
	# function. You can get-- or setMinoriginalStartDist() as a value attached 
	# to each plot. Any value of -1 means no valid land-hills-only path exists to
	# a civ already placed. For Highlands, that means we have found a pocket 
	# and it must be opened. A valid legal path from all civs to all other civs 
	# is required for this map to deliver reliable, fun games every time.
	#
	# - Sirian
	#
	gc = CyGlobalContext()
	map = CyMap()
	dice = gc.getGame().getMapRand()
	iW = map.getGridWidth()
	iH = map.getGridHeight()
	iPlayers = gc.getGame().countCivPlayersEverAlive()
	iNumStartsAllocated = 0
	start_plots = []
	print "==="
	print "Number of players:", iPlayers
	print "==="

	terrainPlains = gc.getInfoTypeForString("TERRAIN_PLAINS")

	# Obtain player numbers. (Account for possibility of Open slots!)
	player_list = []
	for plrCheckLoop in range(18):
		if CyGlobalContext().getPlayer(plrCheckLoop).isEverAlive():
			player_list.append(plrCheckLoop)
	# Shuffle players so that who goes first (and gets the best start location) is randomized.
	shuffledPlayers = []
	for playerLoopTwo in range(gc.getGame().countCivPlayersEverAlive()):
		iChoosePlayer = dice.get(len(player_list), "Shuffling Players - Highlands PYTHON")
		shuffledPlayers.append(player_list[iChoosePlayer])
		del player_list[iChoosePlayer]

	# Loop through players, assigning starts for each.
	for assign_loop in range(iPlayers):
		playerID = shuffledPlayers[assign_loop]
		player = gc.getPlayer(playerID)
		
		# Use the absolute approach for findStart from CvMapGeneratorUtil, which 
		# ignores areaID quality and finds the best local situation on the board.
		findstart = CvMapGeneratorUtil.findStartingPlot(playerID)
		sPlot = map.plotByIndex(findstart)
		
		# Record the plot number to the data array for use if needed to open a "pocket".
		iStartX = sPlot.getX()
		iStartY = sPlot.getY()
		
		# If first player placed, no need to check for pathing yet.
		if assign_loop == 0:
			start_plots.append([iStartX, iStartY])
			player.setStartingPlot(sPlot, true) # True flag causes data to be refreshed for MinOriginalStartDist data cells in plots on the same land mass.
			print "-+-+-"
			print "Player"
			print playerID
			print "First player assigned."
			print "-+-+-"
			continue
		
		# Check the pathing in the start plot.
		if sPlot.getMinOriginalStartDist() != -1:
			start_plots.append([iStartX, iStartY])
			player.setStartingPlot(sPlot, true)
			print "-+-+-"
			print "Player"
			print playerID
			print "Open Path, no problems."
			print "-+-+-"
			continue
		
		# If the process has reached this point, then this player is stuck 
		# in a "pocket". This could be an island, a valley surrounded by peaks, 
		# or an area blocked off by peaks. Could even be that a major line 
		# of peaks and lakes combined is bisecting the entire map.
		print "-----"
		print "Player"
		print playerID
		print "Pocket detected, attempting to resolve..."
		print "-----"
		#
		# First step is to identify which existing start plot is closest.
		print "Pocket Plot"
		print iStartX, iStartY
		print "---"
		[iEndX, iEndY] = start_plots[0]
		fMinDistance = sqrt(((iStartX - iEndX) ** 2) + ((iStartY - iEndY) ** 2))
		for check_loop in range(1, len(start_plots)):
			[iX, iY] = start_plots[check_loop]
			if fMinDistance > sqrt(((iStartX - iX) ** 2) + ((iStartY - iY) ** 2)):
				# Closer start plot found!
				[iEndX, iEndY] = start_plots[check_loop]
				fMinDistance = sqrt(((iStartX - iX) ** 2) + ((iStartY - iY) ** 2))
		print "Nearest player (path destination)"
		print iEndX, iEndY
		print "---"
		print "Absolute distance:"
		print fMinDistance
		print "-----"
		
		# Now we draw an invisible line, plot by plot, one plot wide, from 
		# the current start to the nearest start, converting peaks along the 
		# way in to hills, and lakes in to flatlands, until a path opens.
		
		# Bulldoze the path until it opens!
		startPlot = map.plot(iStartX, iStartY)
		endPlot = map.plot(iEndX, iEndY)
		if abs(iEndY-iStartY) < abs(iEndX-iStartX):
			# line is closer to horizontal
			if iStartX > iEndX:
				startX, startY, endX, endY = iEndX, iEndY, iStartX, iStartY # swap start and end
				bReverseFlag = True
				print "Path reversed, working from the end plot."
			else: # don't swap
				startX, startY, endX, endY = iStartX, iStartY, iEndX, iEndY
				bReverseFlag = False
				print "Path not reversed."
			dx = endX-startX
			dy = endY-startY
			if dx == 0 or dy == 0:
				slope = 0
			else:
				slope = float(dy)/float(dx)
			print("Slope: ", slope)
			y = startY
			for x in range(startX, endX):
				print "Checking plot"
				print x, int(round(y))
				print "---"
				if map.isPlot(x, int(round(y))):
					i = map.plotNum(x, int(round(y)))
					pPlot = map.plotByIndex(i)
					y += slope
					print("y plus slope: ", y)
					if pPlot.isHills() or pPlot.isFlatlands(): continue # on to next plot!
					if pPlot.isPeak():
						print "Peak found! Bulldozing this plot."
						print "---"
						pPlot.setPlotType(PlotTypes.PLOT_HILLS, true, true)
						if bReverseFlag:
							currentDistance = map.calculatePathDistance(pPlot, startPlot)
						else:
							currentDistance = map.calculatePathDistance(pPlot, endPlot)
						if currentDistance != -1: # The path has been opened!
							print "Pocket successfully opened!"
							print "-----"
							break
					elif pPlot.isWater():
						print "Lake found! Filling in this plot."
						print "---"
						pPlot.setPlotType(PlotTypes.PLOT_LAND, true, true)
						pPlot.setTerrainType(terrainPlains, true, true)
						if pPlot.getBonusType(-1) != -1:
							print "########################"
							print "A sea-based Bonus is now present on the land! EEK!"
							print "########################"
							pPlot.setBonusType(-1)
							print "OK, nevermind. The resource has been removed."
							print "########################"
						if bReverseFlag:
							currentDistance = map.calculatePathDistance(pPlot, startPlot)
						else:
							currentDistance = map.calculatePathDistance(pPlot, endPlot)
						if currentDistance != -1: # The path has been opened!
							print "Pocket successfully opened!"
							print "-----"
							break

		else:
			# line is closer to vertical
			if iStartY > iEndY:
				startX, startY, endX, endY = iEndX, iEndY, iStartX, iStartY # swap start and end
				bReverseFlag = True
				print "Path reversed, working from the end plot."
			else: # don't swap
				startX, startY, endX, endY = iStartX, iStartY, iEndX, iEndY
				bReverseFlag = False
				print "Path not reversed."
			dx, dy = endX-startX, endY-startY
			if dx == 0 or dy == 0:
				slope = 0
			else:
				slope = float(dx)/float(dy)
			print("Slope: ", slope)
			x = startX
			for y in range(startY, endY+1):
				print "Checking plot"
				print int(round(x)), y
				print "---"
				if map.isPlot(int(round(x)), y):
					i = map.plotNum(int(round(x)), y)
					pPlot = map.plotByIndex(i)
					x += slope
					print("x plus slope: ", x)
					if pPlot.isHills() or pPlot.isFlatlands(): continue # on to next plot!
					if pPlot.isPeak():
						print "Peak found! Bulldozing this plot."
						print "---"
						pPlot.setPlotType(PlotTypes.PLOT_HILLS, true, true)
						if bReverseFlag:
							currentDistance = map.calculatePathDistance(pPlot, startPlot)
						else:
							currentDistance = map.calculatePathDistance(pPlot, endPlot)
						if currentDistance != -1: # The path has been opened!
							print "Pocket successfully opened!"
							print "-----"
							break
					elif pPlot.isWater():
						print "Lake found! Filling in this plot."
						print "---"
						pPlot.setPlotType(PlotTypes.PLOT_LAND, true, true)
						pPlot.setTerrainType(terrainPlains, true, true)
						if pPlot.getBonusType(-1) != -1:
							print "########################"
							print "A sea-based Bonus is now present on the land! EEK!"
							print "########################"
							pPlot.setBonusType(-1)
							print "OK, nevermind. The resource has been removed."
							print "########################"
						if bReverseFlag:
							currentDistance = map.calculatePathDistance(pPlot, startPlot)
						else:
							currentDistance = map.calculatePathDistance(pPlot, endPlot)
						if currentDistance != -1: # The path has been opened!
							print "Pocket successfully opened!"
							print "-----"
							break
			
		# Now that all the pathing for this player is resolved, set the start plot.
		start_plots.append([iStartX, iStartY])
		player.setStartingPlot(sPlot, true)

	# All done!
	print "**********"
	print "All start plots assigned!"
	print "**********"
	return None
	
def normalizeRemovePeaks():
	return None

def normalizeRemoveBadTerrain():
	return None

def normalizeAddGoodTerrain():
	return None
