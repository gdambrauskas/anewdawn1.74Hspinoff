#
#	FILE:	 Team_Battleground.py
#	AUTHOR:  Bob Thomas (Sirian)
#	CONTRIB: Andy Szybalski
#	PURPOSE: Regional map script - Ideal for quick MP team games.
#-----------------------------------------------------------------------------
#	Copyright (c) 2005 Firaxis Games, Inc. All rights reserved.
#-----------------------------------------------------------------------------
#
#
# Ruff - Desert Bridge & Round Start
#  Additional options added by Ruff_Hi from
#        - Civilization Fanatics (http://forums.civfanatics.com/member.php?u=64034)
#        - Deviant Minds (http://deviantminds.us/forum/memberlist.php?mode=viewprofile&u=8)
#        - Realms Beyond (http://realmsbeyond.net/forums/member.php?u=555)
#    - Left v Right with a desert land bridge
#    - round with starting locations uniformly spread around the circle
#    - donut - same as round but with water at the center


from CvPythonExtensions import *
import CvUtil
import CvMapGeneratorUtil
import sys
from CvMapGeneratorUtil import FractalWorld
from CvMapGeneratorUtil import HintedWorld
from CvMapGeneratorUtil import TerrainGenerator
from CvMapGeneratorUtil import FeatureGenerator

from math import sqrt
from math import cos
from math import sin
from math import radians

# Rise of Mankind 2.53
#which map size was selected
sizeSelected = 0
# Rise of Mankind 2.53

def getDescription():
	return "TXT_KEY_MAP_SCRIPT_TEAM_BATTLEGROUND_DESCR"

def getNumCustomMapOptions():
	return 3
	
def getNumHiddenCustomMapOptions():
	return 1
	
def getCustomMapOptionName(argsList):
	[iOption] = argsList
	option_names = {
		0:	"TXT_KEY_MAP_SCRIPT_TEAM_PLACEMENT",
		1:	"TXT_KEY_MAP_SCRIPT_TEAM_SETTING",
		2:  "TXT_KEY_MAP_WORLD_WRAP"
		}
	translated_text = unicode(CyTranslator().getText(option_names[iOption], ()))
	return translated_text
	
def getNumCustomMapOptionValues(argsList):
	[iOption] = argsList
	option_values = {
		0:	6,
		1:	3,
		2:	3
		}
	return option_values[iOption]
	
def getCustomMapOptionDescAt(argsList):
	[iOption, iSelection] = argsList

	selection_names = {
		0:	{
			0: "TXT_KEY_MAP_SCRIPT_LEFT_VS_RIGHT",
			1: "TXT_KEY_MAP_SCRIPT_TOP_VS_BOTTOM",
			2: "TXT_KEY_MAP_SCRIPT_FOUR_CORNERS",
			3: "TXT_KEY_MAP_SCRIPT_LEFT_VS_RIGHT_BRIDGE",
			4: "TXT_KEY_MAP_SCRIPT_ROUND",
			5: "TXT_KEY_MAP_SCRIPT_DONUT"
			},
		1:	{
			0: "TXT_KEY_MAP_SCRIPT_START_TOGETHER",
			1: "TXT_KEY_MAP_SCRIPT_START_SEPARATED",
			2: "TXT_KEY_MAP_SCRIPT_START_ANYWHERE"
			},
		2:	{
			0: "TXT_KEY_MAP_WRAP_FLAT",
			1: "TXT_KEY_MAP_WRAP_CYLINDER",
			2: "TXT_KEY_MAP_WRAP_TOROID"
			}
		}

	translated_text = unicode(CyTranslator().getText(selection_names[iOption][iSelection], ()))
	return translated_text
	
def getCustomMapOptionDefault(argsList):
	[iOption] = argsList
	option_defaults = {
		0:	1,
		1:	0,
		2:  0
		}
	return option_defaults[iOption]

def isRandomCustomMapOption(argsList):
	[iOption] = argsList
	option_random = {
		0:	true,
		1:	false,
		2:  false
		}
	return option_random[iOption]

def isAdvancedMap():
	"This map should not show up in simple mode"
	return 1

def isSeaLevelMap():
	return 0

def getWrapX():
	map = CyMap()
	return (map.getCustomMapOption(2) == 1 or map.getCustomMapOption(2) == 2)
	
def getWrapY():
	map = CyMap()
	return (map.getCustomMapOption(2) == 2)
	
def getTopLatitude():
	return 80
def getBottomLatitude():
	return -80
	
def minStartingDistanceModifier():
	return -65

def beforeGeneration():
	global equator
	global team_num
	team_num = []
	team_index = 0
	topVsBottomCheck = CyMap().getCustomMapOption(0)
	if topVsBottomCheck == 1:
		equator = CyGlobalContext().getInfoTypeForString("TERRAIN_DESERT")
	else:
		equator = CyGlobalContext().getInfoTypeForString("TERRAIN_GRASS")
	for teamCheckLoop in range(18):
		if CyGlobalContext().getTeam(teamCheckLoop).isEverAlive():
			team_num.append(team_index)
			team_index += 1
		else:
			team_num.append(-1)
	return None

def getGridSize(argsList):
	"Different grids, depending on the choice of Team Placement. Very small worlds."
	if (argsList[0] == -1): # (-1,) is passed to function on loads
		return []

	# Get user input.
	grid_choice = CyMap().getCustomMapOption(0)  # 0 = left v right, 1 = top v bottom, 2 = four corners, 3 = left v right with land bridge, 4 = round, 5 = donut
	if grid_choice == 2:
		grid_choice = 1

	if grid_choice == 3: # left v right with land bridge defaults to same as left v right
		grid_choice = 0
	
	if (grid_choice == 4   # round
	or  grid_choice == 5): # donut
		grid_choice = 1
	
	grid_sizes = [{WorldSizeTypes.WORLDSIZE_DUEL:		(5,3),
	               WorldSizeTypes.WORLDSIZE_TINY:		(6,4),
	               WorldSizeTypes.WORLDSIZE_SMALL:		(8,5),
	               WorldSizeTypes.WORLDSIZE_STANDARD:	(10,6),
	               WorldSizeTypes.WORLDSIZE_LARGE:		(13,8),
	               WorldSizeTypes.WORLDSIZE_HUGE:		(16,10)},
	              {WorldSizeTypes.WORLDSIZE_DUEL:		(4,4),
	               WorldSizeTypes.WORLDSIZE_TINY:		(5,5),
	               WorldSizeTypes.WORLDSIZE_SMALL:		(6,6),
	               WorldSizeTypes.WORLDSIZE_STANDARD:	(8,8),
	               WorldSizeTypes.WORLDSIZE_LARGE:		(10,10),
	               WorldSizeTypes.WORLDSIZE_HUGE:		(13,13)}
	]

	[eWorldSize] = argsList
# Rise of Mankind 2.53 - giant and gigantic mapsize fix	
	global sizeSelected
	sizeSelected = eWorldSize
	print "    sizeSelected",sizeSelected
	# Giant size
	if ( grid_choice == 0 and sizeSelected == 6 ):
		return (20, 12)
	elif ( grid_choice == 1 and sizeSelected == 6 ):
		return (16, 16)
	# Gigantic size
	elif ( grid_choice == 0 and sizeSelected == 7 ):
		return (24, 16)
	elif ( grid_choice == 1 and sizeSelected == 7 ):
		return (20, 20)
# Rise of Mankind 2.53 - giant and gigantic mapsize fix
	print "    grid_choice and eWorldSize",grid_sizes[grid_choice][eWorldSize]
	return grid_sizes[grid_choice][eWorldSize]

def generatePlotTypes():
	NiTextOut("Setting Plot Types (Python Team Battleground) ...")
	global hinted_world, mapRand
	global fractal_world
	gc = CyGlobalContext()
	map = CyMap()
	mapRand = gc.getGame().getMapRand()
	userInputPlots = map.getCustomMapOption(0)
	
	if userInputPlots == 2: # Four Corners
		hinted_world = HintedWorld()
		iNumPlotsX = map.getGridWidth()
		iNumPlotsY = map.getGridHeight()

		centery = (hinted_world.h - 1)//2
		centerx = (hinted_world.w - 1)//2
	
		iCenterXList = []
		iCenterXList.append(centerx-1)
		iCenterXList.append(centerx)
		iCenterXList.append(centerx+1)
	
		iCenterYList = []
		iCenterYList.append(centery-1)
		iCenterYList.append(centery)
		iCenterYList.append(centery+1)

		bridgey = centery

		# Set all blocks to land except a strip in the center
		for x in range(hinted_world.w):
			for y in range(hinted_world.h):
				if x == centerx:
					if y == bridgey:
						hinted_world.setValue(x,y,128) # coast
					else:
						hinted_world.setValue(x,y,0)
				else:
					hinted_world.setValue(x,y,255)
					if y in iCenterYList:
						hinted_world.setValue(x,y,128) # coast
					if y == centery:
						hinted_world.setValue(x,y,0) # ocean

		hinted_world.buildAllContinents()
		plotTypes = hinted_world.generatePlotTypes(20)
	
		# Remove any land bridge that exists
		centerplotx = (iNumPlotsX - 1)//2
		dx = 1
		for x in range(centerplotx-dx, centerplotx+dx+1):
			for y in range(iNumPlotsY):
				i = map.plotNum(x, y)
				if plotTypes[i] != PlotTypes.PLOT_OCEAN:
					plotTypes[i] = PlotTypes.PLOT_OCEAN
		centerploty = (iNumPlotsY - 1)//2
		dy = 1
		for y in range(centerploty-dy, centerploty+dy+1):
			for x in range(iNumPlotsX):
				i = map.plotNum(x, y)
				if plotTypes[i] != PlotTypes.PLOT_OCEAN:
					plotTypes[i] = PlotTypes.PLOT_OCEAN
		
		# Now add the bridge across the center!
		sizekey = map.getWorldSize()
		sizevalues = {
			WorldSizeTypes.WORLDSIZE_DUEL:		3,
			WorldSizeTypes.WORLDSIZE_TINY:		4,
			WorldSizeTypes.WORLDSIZE_SMALL:		5,
			WorldSizeTypes.WORLDSIZE_STANDARD:	6,
			WorldSizeTypes.WORLDSIZE_LARGE:		8,
			WorldSizeTypes.WORLDSIZE_HUGE:		10
			}
# Rise of Mankind 2.53
		if ( not sizekey in sizevalues ):
			shift = 12
		else:
			shift = sizevalues[sizekey]
			
# Rise of Mankind 2.53			
		linewidth = 3
		offsetstart = 0 - int(linewidth/2)
		offsetrange = range(offsetstart, offsetstart + linewidth)
		westX1, southY1, eastX1, northY1 = centerplotx - shift, centerploty - shift, centerplotx + shift, centerploty + shift
		westX2, southY2, eastX2, northY2 = centerplotx - shift, centerploty - shift, centerplotx + shift, centerploty + shift
		bridge_data = [[westX1, southY1, eastX1, northY1], [westX2, northY2, eastX2, southY2]]
		for bridge_loop in range(2):
			[startx, starty, endx, endy] = bridge_data[bridge_loop]

			if abs(endy-starty) < abs(endx-startx):
				# line is closer to horizontal
				if startx > endx:
					startx, starty, endx, endy = endx, endy, startx, starty # swap start and end
				dx = endx-startx
				dy = endy-starty
				if dx == 0 or dy == 0:
					slope = 0
				else:
					slope = float(dy)/float(dx)
				y = starty
				for x in range(startx, endx+1):
					for offset in offsetrange:
						if map.isPlot(x, int(round(y+offset))):
							i = map.plotNum(x, int(round(y+offset)))
							plotTypes[i] = PlotTypes.PLOT_LAND
					y += slope
			else:
				# line is closer to vertical
				if starty > endy:
					startx, starty, endx, endy = endx, endy, startx, starty # swap start and end
				dx, dy = endx-startx, endy-starty
				if dx == 0 or dy == 0:
					slope = 0
				else:
					slope = float(dx)/float(dy)
				x = startx
				for y in range(starty, endy+1):
					for offset in offsetrange:
						if map.isPlot(int(round(x+offset)), y):
							i = map.plotNum(int(round(x+offset)), y)
							plotTypes[i] = PlotTypes.PLOT_LAND
					x += slope
		
		return plotTypes

	if (userInputPlots == 4   # round
	or  userInputPlots == 5): # donut
		hinted_world = HintedWorld()
		iNumPlotsX = map.getGridWidth()
		iNumPlotsY = map.getGridHeight()

		centery = (iNumPlotsY - 1)//2
		centerx = (iNumPlotsX - 1)//2
		radii = centery - 1

		# Set all blocks to ocean except the inner circle
		for x in range(iNumPlotsX):
			for y in range(iNumPlotsY):
				dist_xy_c = sqrt( (x - centerx) ** 2 + (y - centery) ** 2)
				if dist_xy_c < radii:
					hinted_world.setValue(x,y,255)
				else:
					hinted_world.setValue(x,y,0) # ocean

		hinted_world.buildAllContinents()
		plotTypes = hinted_world.generatePlotTypes(water_percent = 0)

		if userInputPlots == 5: # donut
			# get the size of the hole
			map_size = map.getWorldSize()
			sizevalues = {
				WorldSizeTypes.WORLDSIZE_DUEL:		2,
				WorldSizeTypes.WORLDSIZE_TINY:		3,
				WorldSizeTypes.WORLDSIZE_SMALL:		4,
				WorldSizeTypes.WORLDSIZE_STANDARD:	5,
				WorldSizeTypes.WORLDSIZE_LARGE:		7,
				WorldSizeTypes.WORLDSIZE_HUGE:		8
				}
# Rise of Mankind 2.53
			if ( not map_size in sizevalues ):
				hole_radii = 9
			else:
				hole_radii = sizevalues[map_size]
# Rise of Mankind 2.53

		# Set all blocks to ocean except the inner circle
		for x in range(iNumPlotsX):
			for y in range(iNumPlotsY):
				i = map.plotNum(x, y)
				dist_xy_c = sqrt( (x - centerx) ** 2 + (y - centery) ** 2)
				if (dist_xy_c < radii):
					plotTypes[i] = PlotTypes.PLOT_LAND
				else:
					plotTypes[i] = PlotTypes.PLOT_OCEAN

				if (userInputPlots == 5 # donut
				and dist_xy_c < hole_radii):
					plotTypes[i] = PlotTypes.PLOT_OCEAN

		return plotTypes

	elif userInputPlots == 1: # Top vs Bottom
		fractal_world = FractalWorld(fracXExp=6, fracYExp=6)
		fractal_world.initFractal(continent_grain = 4, rift_grain = -1, has_center_rift = False, invert_heights = True)
		plot_types = fractal_world.generatePlotTypes(water_percent = 8)
		return plot_types

	else: # Left vs Right
		iNumPlotsX = map.getGridWidth()
		iNumPlotsY = map.getGridHeight()
	
		hinted_world = HintedWorld(4,2)
		centerx = (hinted_world.w - 1)//2	
		centery = (hinted_world.h - 1)//2
		bridgey = centery

		# set all blocks to land except a strip in the center
		for x in range(hinted_world.w):
			for y in range(hinted_world.h):
				if x == centerx:
					if y == bridgey:
						hinted_world.setValue(x,y,128) # coast
					else:
						hinted_world.setValue(x,y,0)
				else:
					hinted_world.setValue(x,y,255)
		
		hinted_world.buildAllContinents()
		plotTypes = hinted_world.generatePlotTypes(20)
	
		#fix any land bridge that exists
		centerplotx = (iNumPlotsX - 1)//2
		dx = 1
		for x in range(centerplotx-dx, centerplotx+dx+1):
			for y in range(iNumPlotsY):
				i = map.plotNum(x, y)
				if plotTypes[i] != PlotTypes.PLOT_OCEAN:
					plotTypes[i] = PlotTypes.PLOT_OCEAN

		if userInputPlots == 3: # Left v Right with bridge
			centerplotx = (iNumPlotsX)//2
			centerploty = (iNumPlotsY)//2
			dy = 1
			for x in range(iNumPlotsX):
				for y in range(centerploty-dy, centerploty+dy+1):
					i = map.plotNum(x, y)
					if plotTypes[i] == PlotTypes.PLOT_OCEAN:
						plotTypes[i] = PlotTypes.PLOT_LAND

		return plotTypes

class TeamBGTerrainGenerator(CvMapGeneratorUtil.TerrainGenerator):
	def generateTerrainAtPlot(self, iX, iY):
		global equator
		lat = 0.8 * self.getLatitudeAtPlot(iX,iY)

		if (self.map.plot(iX, iY).isWater()):
			return self.map.plot(iX, iY).getTerrainType()

		terrainVal = self.terrainGrass

		if lat >= self.fSnowLatitude:
			terrainVal = self.terrainIce
		elif lat >= self.fTundraLatitude:
			terrainVal = self.terrainTundra
		elif lat < self.fGrassLatitude:
			terrainVal = equator # Equator is grass usually, but desert for TvB
		else:
			desertVal = self.deserts.getHeight(iX, iY)
			plainsVal = self.plains.getHeight(iX, iY)
			if ((desertVal >= self.iDesertBottom) and (desertVal <= self.iDesertTop) and (lat >= self.fDesertBottomLatitude) and (lat < self.fDesertTopLatitude)):
				terrainVal = self.terrainDesert
			elif ((plainsVal >= self.iPlainsBottom) and (plainsVal <= self.iPlainsTop)):
				terrainVal = self.terrainPlains
			else:
				terrainVal =self.terrainGrass

		map = CyMap()
		userInputPlots = map.getCustomMapOption(0)
		if userInputPlots == 3: # Left v Right with bridge
			if (iY - 3 > 0
			and iY + 3 < map.getGridHeight()):
				if (self.map.plot(iX, iY - 3).isWater()
				and self.map.plot(iX, iY + 3).isWater()):
					terrainVal = self.terrainDesert
				elif (self.map.plot(iX, iY - 3).isWater()
				or    self.map.plot(iX, iY + 3).isWater()):
					terrainVal = self.terrainPlains

		if (terrainVal == TerrainTypes.NO_TERRAIN):
			return self.map.plot(iX, iY).getTerrainType()

		return terrainVal

def generateTerrainTypes():
	NiTextOut("Generating Terrain (Python Team Battleground) ...")
	terraingen = TeamBGTerrainGenerator()
	terraingen.__init__(iDesertPercent=15)
	terrainTypes = terraingen.generateTerrain()
	return terrainTypes

class TeamBGFeatureGenerator(CvMapGeneratorUtil.FeatureGenerator):
	def getLatitudeAtPlot(self, iX, iY):
		"returns a value in the range of 0.0 (tropical) to 1.0 (polar)"
		return 0.8 * (abs((self.iGridH/2) - iY)/float(self.iGridH/2))

def addFeatures():
	NiTextOut("Adding Features (Python Team Battleground) ...")
	featuregen = TeamBGFeatureGenerator()
	featuregen.addFeatures()
	return 0

def assignStartingPlots():
	gc = CyGlobalContext()
	dice = gc.getGame().getMapRand()
	global shuffle
	global shuffledTeams
	global assignedPlayers
	assignedPlayers = [0] * gc.getGame().countCivTeamsEverAlive()

	print assignedPlayers

	shuffle = gc.getGame().getMapRand().get(2, "Start Location Shuffle - PYTHON")

	global shuffledPlayers
	global player_num

	map = CyMap()
	userInputPlots = map.getCustomMapOption(0)
	if (userInputPlots == 4   # round
	or  userInputPlots == 5): # donut

# this block of code takes the players and shuffles them
# if the 'teams start together' is selected (ie map.getCustomMapOption(1) = 0)
# then the shuffled players are sorted into teams
# then each player in the shuffled player array is assigned a starting plot 1 in from the coast
# and uniformly spread around the circle

# shuffle the players
		player_num = gc.getGame().countCivPlayersEverAlive()
		player_list = [0] * player_num
		shuffledPlayers = []

		for playerLoop in range(player_num):
			player_list[playerLoop] = playerLoop

		for playerLoop in range(player_num):
			iChoosePlayer = dice.get(len(player_list), "Shuffling Players - TBG PYTHON")
			shuffledPlayers.append(player_list[iChoosePlayer])
			del player_list[iChoosePlayer]


# sort by team if required
		userInputProximity = map.getCustomMapOption(1)
		if userInputProximity == 0: # Start Together
			shuffledPlayers_Team = [0] * player_num
			for playerLoop in range(player_num):
				shuffledPlayers_Team[playerLoop] = CyGlobalContext().getPlayer(playerLoop).getTeam()

			player_Start = 0
			player_Out = player_Start + 1
			player_In = player_Start + 2

			while (player_Out < player_num
			and    player_In  < player_num):
				if shuffledPlayers_Team[player_Start] == shuffledPlayers_Team[player_Out]:
					player_Out = player_Out + 1
					player_In = player_In + 1

				elif shuffledPlayers_Team[player_Start] == shuffledPlayers_Team[player_In]:
					temp = shuffledPlayers[player_In]
					shuffledPlayers[player_In] = shuffledPlayers[player_Out]
					shuffledPlayers[player_Out] = temp

					temp = shuffledPlayers_Team[player_In]
					shuffledPlayers_Team[player_In] = shuffledPlayers_Team[player_Out]
					shuffledPlayers_Team[player_Out] = temp

					player_Start = player_Start + 1
					player_Out = player_Start + 1
					player_In = player_Start + 2

				else:
					player_In = player_In + 1

					if player_In > player_num:
						player_Out = player_Out + 1
						player_Start = player_Out - 1
						player_In = player_Out + 1


# allocate starting plot by player
		iNumPlotsX = map.getGridWidth()
		iNumPlotsY = map.getGridHeight()

		centery = (iNumPlotsY - 1)//2
		centerx = (iNumPlotsX - 1)//2
		radii = centery - 2

		base_theta = 360 / player_num * dice.get(1000, "Starting Plot - base theta") / 1000

		for playerLoop in range(player_num):
			pPlayer = shuffledPlayers[playerLoop]
			theta = base_theta + playerLoop * 360 / player_num
			x = int(centerx + round(radii * cos(radians(theta)),0))
			y = int(centery + round(radii * sin(radians(theta)),0))
			pPlot = map.plot(x, y)
			CyGlobalContext().getPlayer(pPlayer).setStartingPlot(pPlot,True)

	if gc.getGame().countCivTeamsEverAlive() < 5:
		team_list = [0, 1, 2, 3]
		shuffledTeams = []
		for teamLoop in range(gc.getGame().countCivTeamsEverAlive()):
			iChooseTeam = dice.get(len(team_list), "Shuffling Regions - TBG PYTHON")
			shuffledTeams.append(team_list[iChooseTeam])
			del team_list[iChooseTeam]

	CyPythonMgr().allowDefaultImpl()
	
def findStartingPlot(argsList):
	[playerID] = argsList
	global assignedPlayers
	global team_num

	map = CyMap()
	userInputPlots = map.getCustomMapOption(0)
	if (userInputPlots == 4   # round
	or  userInputPlots == 5): # donut ... starting position already set - return plotnum
		pPlot = CyGlobalContext().getPlayer(playerID).getStartingPlot()
		return map.plotNum(pPlot.getX(), pPlot.getY())

	thisTeamID = CyGlobalContext().getPlayer(playerID).getTeam()
	teamID = team_num[thisTeamID]
	
	assignedPlayers[teamID] += 1

	def isValid(playerID, x, y):
		map = CyMap()
		numTeams = CyGlobalContext().getGame().countCivTeamsAlive()
		if numTeams > 4 or numTeams < 2: # Put em anywhere, and let the normalizer sort em out.
			return true
		userInputProximity = map.getCustomMapOption(1)
		if userInputProximity == 2: # Start anywhere!
			return true

		global shuffle
		global shuffledTeams
		global team_num
		global shuffledPlayers
		global player_num

		thisTeamID = CyGlobalContext().getPlayer(playerID).getTeam()
		teamID = team_num[thisTeamID]
		userInputPlots = map.getCustomMapOption(0)
		iW = map.getGridWidth()
		iH = map.getGridHeight()

		# Two Teams, Start Together
		if numTeams == 2 and userInputProximity == 0: # Two teams, Start Together
			if userInputPlots == 1: # TvB
				if teamID == 0 and shuffle and y >= iH * 0.6:
					return true
				if teamID == 1 and not shuffle and y >= iH * 0.6:
					return true
				if teamID == 0 and not shuffle and y <= iH * 0.4:
					return true
				if teamID == 1 and shuffle and y <= iH * 0.4:
					return true
				return false

			elif (userInputPlots == 0   # LvR
			or    userInputPlots == 3): # LvR with land bridge
				if teamID == 0 and shuffle and x >= iW * 0.6:
					return true
				if teamID == 1 and not shuffle and x >= iW * 0.6:
					return true
				if teamID == 0 and not shuffle and x <= iW * 0.4:
					return true
				if teamID == 1 and shuffle and x <= iW * 0.4:
					return true
				return false

			else: # 4C
				corner = shuffledTeams[teamID]
				if corner == 0 and x <= iW * 0.4 and y <= iH * 0.4:
					return true
				if corner == 1 and x >= iW * 0.6 and y <= iH * 0.4:
					return true
				if corner == 2 and x <= iW * 0.4 and y >= iH * 0.6:
					return true
				if corner == 3 and x >= iW * 0.6 and y >= iH * 0.6:
					return true
				return false

		# Three or Four Teams
		elif (numTeams == 3 or numTeams == 4) and userInputProximity == 0: # 3 or 4 teams, Start Together
			corner = shuffledTeams[teamID]
			if corner == 0 and x <= iW * 0.4 and y <= iH * 0.4:
				return true
			if corner == 1 and x >= iW * 0.6 and y <= iH * 0.4:
				return true
			if corner == 2 and x <= iW * 0.4 and y >= iH * 0.6:
				return true
			if corner == 3 and x >= iW * 0.6 and y >= iH * 0.6:
				return true
			return false
		elif (numTeams == 3 or numTeams == 4) and userInputProximity == 1: # 3 or 4 teams, Start Separated
			corner = shuffledTeams[teamID] + assignedPlayers[teamID]
			while corner >= 4:
				corner -= 4
			if corner == 0 and x <= iW * 0.4 and y <= iH * 0.4:
				return true
			if corner == 1 and x >= iW * 0.6 and y <= iH * 0.4:
				return true
			if corner == 2 and x <= iW * 0.4 and y >= iH * 0.6:
				return true
			if corner == 3 and x >= iW * 0.6 and y >= iH * 0.6:
				return true
			return false

		# Two Teams, Start Separated
		elif numTeams == 2 and userInputProximity == 1: # Two teams, Start Separated
			if (shuffle and teamID == 0) or (not shuffle and teamID == 1):
				side = assignedPlayers[teamID]
			else:
				side = 1 + assignedPlayers[teamID]
			while side >= 2:
				side -= 2
			if userInputPlots == 1: # TvB
				if teamID == 0 and side and y >= iH * 0.6:
					return true
				if teamID == 1 and not side and y >= iH * 0.6:
					return true
				if teamID == 0 and not side and y <= iH * 0.4:
					return true
				if teamID == 1 and side and y <= iH * 0.4:
					return true
				return false

			elif (userInputPlots == 0   # LvR
			or    userInputPlots == 3): # LvR with land bridge
				if teamID == 0 and side and x >= iW * 0.6:
					return true
				if teamID == 1 and not side and x >= iW * 0.6:
					return true
				if teamID == 0 and not side and x <= iW * 0.4:
					return true
				if teamID == 1 and side and x <= iW * 0.4:
					return true
				return false

			else: # 4C
				corner = shuffledTeams[side]
				if corner == 0 and x <= iW * 0.4 and y <= iH * 0.4:
					return true
				if corner == 1 and x >= iW * 0.6 and y <= iH * 0.4:
					return true
				if corner == 2 and x <= iW * 0.4 and y >= iH * 0.6:
					return true
				if corner == 3 and x >= iW * 0.6 and y >= iH * 0.6:
					return true
				return false

		# All conditions have failed? Wow. Is that even possible? :)
		return true
	
	return CvMapGeneratorUtil.findStartingPlot(playerID, isValid)

def normalizeStartingPlotLocations():
	numTeams = CyGlobalContext().getGame().countCivTeamsAlive()
	userInputProximity = CyMap().getCustomMapOption(1)
	if (numTeams > 4 or numTeams < 2) and userInputProximity == 0:
		CyPythonMgr().allowDefaultImpl()
	else:
		return None

def getRiverStartCardinalDirection(argsList):
	"Returns the cardinal direction of the first river segment."
	pPlot = argsList[0]
	print pPlot
	map = CyMap()
	x, y = pPlot.getX(), pPlot.getY()
	iW = map.getGridWidth()
	iH = map.getGridHeight()
	userInputPlots = CyMap().getCustomMapOption(0)

	if (userInputPlots == 0 # LvR
	or  userInputPlots == 3): # LvR with land bridge
		if x < iW/2:
			return CardinalDirectionTypes.CARDINALDIRECTION_EAST
		else:
			return CardinalDirectionTypes.CARDINALDIRECTION_WEST

	elif userInputPlots == 2:
		if y < iH/2:
			return CardinalDirectionTypes.CARDINALDIRECTION_NORTH
		else:
			return CardinalDirectionTypes.CARDINALDIRECTION_SOUTH

	else:
		CyPythonMgr().allowDefaultImpl()
