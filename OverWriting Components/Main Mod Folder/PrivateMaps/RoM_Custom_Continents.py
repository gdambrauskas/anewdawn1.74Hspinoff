#
#	FILE:	 Custom_Continents.py
#	AUTHOR:  Bob Thomas (Sirian)
#	PURPOSE: Global map script - You choose the number of continents.
#-----------------------------------------------------------------------------
#	Copyright (c) 2005 Firaxis Games, Inc. All rights reserved.
#-----------------------------------------------------------------------------
#

from CvPythonExtensions import *
import CvUtil
import CvMapGeneratorUtil
from CvMapGeneratorUtil import FractalWorld
from CvMapGeneratorUtil import TerrainGenerator
from CvMapGeneratorUtil import FeatureGenerator
#from CvMapGeneratorUtil import BonusBalancer

#balancer = BonusBalancer()

def getDescription():
	return "TXT_KEY_MAP_SCRIPT_CUSTOM_CONTINENTS_DESCR"

def getNumCustomMapOptions():
	return 3

def getNumHiddenCustomMapOptions():
	return 2

def getCustomMapOptionName(argsList):
	[iOption] = argsList
	option_names = {
		0:	"TXT_KEY_MAP_SCRIPT_NUMBER_OF_CONTINENTS",
		1:	"TXT_KEY_MAP_WORLD_WRAP",
		2:  "TXT_KEY_CONCEPT_RESOURCES"
		}
	translated_text = unicode(CyTranslator().getText(option_names[iOption], ()))
	return translated_text
	
def getNumCustomMapOptionValues(argsList):
	[iOption] = argsList
	option_values = {
		0:	7,
		1:	3,
		2:  2
		}
	return option_values[iOption]
	

def getCustomMapOptionDescAt(argsList):
	[iOption, iSelection] = argsList
	selection_names = {
		0:	{
			0: "TXT_KEY_MAP_SCRIPT_RANDOM",
			1: "TXT_KEY_MAP_SCRIPT_ONE_PER_TEAM",
			2: "2",
			3: "3",
			4: "4",
			5: "5",
			6: "6"
			},
		1:	{
			0: "TXT_KEY_MAP_WRAP_FLAT",
			1: "TXT_KEY_MAP_WRAP_CYLINDER",
			2: "TXT_KEY_MAP_WRAP_TOROID"
			},
		2:	{
			0: "TXT_KEY_WORLD_STANDARD",
			1: "TXT_KEY_MAP_BALANCED"
			}
		}
	translated_text = unicode(CyTranslator().getText(selection_names[iOption][iSelection], ()))
	return translated_text
	
def getCustomMapOptionDefault(argsList):
	[iOption] = argsList
	option_defaults = {
		0:	0,
		1:	1,
		2:  0
		}
	return option_defaults[iOption]

def isRandomCustomMapOption(argsList):
	[iOption] = argsList
	option_random = {
		0:	false,
		1:	false,
		2:  false
		}
	return option_random[iOption]

def getWrapX():
	map = CyMap()
	return (map.getCustomMapOption(1) == 1 or map.getCustomMapOption(1) == 2)
	
def getWrapY():
	map = CyMap()
	return (map.getCustomMapOption(1) == 2)
	
def normalizeAddExtras():
	if (CyMap().getCustomMapOption(2) == 1):
		balancer.normalizeAddExtras()
	CyPythonMgr().allowDefaultImpl()	# do the rest of the usual normalizeStartingPlots stuff, don't overrride

def addBonusType(argsList):
	[iBonusType] = argsList
	gc = CyGlobalContext()
	type_string = gc.getBonusInfo(iBonusType).getType()

	if (CyMap().getCustomMapOption(2) == 1):
		if (type_string in balancer.resourcesToBalance) or (type_string in balancer.resourcesToEliminate):
			return None # don't place any of this bonus randomly
		
	CyPythonMgr().allowDefaultImpl() # pretend we didn't implement this method, and let C handle this bonus in the default way

def isAdvancedMap():
	"This map should not show up in simple mode"
	return 1

def minStartingDistanceModifier():
	return -12

def beforeGeneration():
	global iNumConts
	global cont_data
	global xShiftRoll, yShiftRoll
	gc = CyGlobalContext()
	map = CyMap()
	dice = gc.getGame().getMapRand()
	iW = map.getGridWidth()
	iH = map.getGridHeight()
	iPlayers = gc.getGame().countCivPlayersEverAlive()
	iTeams = gc.getGame().countCivTeamsEverAlive()
	bThisMapBalanced = False # Balanced means use only "fair" templates where all continents are roughly the same value. The "fair" template is always to be ID# 0 in the template data list.
	
	# If binary shift is employed for any layers, use these values to decide whether 
	# or not to shift all such layers in this map instance.
	#
	# The binary shift allows a single templateID to be able to reverse itself 
	# (north-south, or east-west) to be able to contain up to four "versions" of one pattern.
	#
	# If binary shift is not employed, the alternative is random shift within a range.
	# Ideally I would have included both options concurrently, but the line on more 
	# and more sophistication has to be drawn somewhere!
	#
	# Binary shift rolls (one each for horizontal or vertical shifting):
	xShiftRoll = dice.get(2, "Region Shift, Horizontal - Custom Continents PYTHON")
	yShiftRoll = dice.get(2, "Region Shift, Vertical - Custom Continents PYTHON")
	#print xShiftRoll, yShiftRoll
	
	# Determine the number of continents.
	userInputContinents = map.getCustomMapOption(0)
	if userInputContinents > 1:
		iNumConts = userInputContinents
	elif userInputContinents == 1:
		bThisMapBalanced = True
		if iTeams > 1 and iTeams < 7:
			iNumConts = iTeams
		elif iTeams < 2:
			iNumConts = 2
		elif iTeams <= 12:
			iNumConts = int(iTeams / 2)
		elif iTeams <= 18:
			iNumConts = int(iTeams / 3)
		else:
			iNumConts = 6
	else: # Random, weighted to number of players in the game.
		numContsRoll = dice.get(100, "Number of Continents - Custom Continents PYTHON")
		byPlayerIndex = {2: [60, 90, 100, 111],
		                 3: [55, 85, 96, 100],
		                 4: [50, 80, 94, 99],
		                 5: [48, 78, 92, 98],
		                 6: [45, 75, 90, 96],
		                 7: [40, 70, 85, 95],
		                 8: [38, 68, 83, 94],
		                 9: [35, 65, 80, 92],
		                 10: [33, 60, 75, 90],
		                 11: [30, 55, 70, 88],
		                 12: [28, 53, 68, 86],
		                 13: [25, 50, 65, 85],
		                 14: [25, 45, 63, 85],
		                 15: [20, 40, 60, 80],
		                 16: [17, 35, 55, 75],
		                 17: [13, 30, 50, 70],
		                 18: [10, 25, 45, 65]
		}
		if numContsRoll < byPlayerIndex[iPlayers][0]:
			iNumConts = 2
		elif numContsRoll < byPlayerIndex[iPlayers][1]:
			iNumConts = 3
		elif numContsRoll < byPlayerIndex[iPlayers][2]:
			iNumConts = 4
		elif numContsRoll < byPlayerIndex[iPlayers][3]:
			iNumConts = 5
		else:
			iNumConts = 6
	#print("Continents: ", iNumConts)
	
	# List of number of template instances, indexed by number of continents selected.
	configs = [0, 0, 10, 9, 7, 6, 6]
	
	# Choose a template for this game.
	if bThisMapBalanced:
		templateID = 0
		# For "One Per Team" setting, ensure (reasonably) fair continents for each 
		# team by using the zero template. If you want to be able to play team matches 
		# on unbalanced templates, then manually set the number of continents on 
		# startup. For instance, for four teams, set it to 4 instead of to One Per Team.
	else:
		templateID = dice.get(configs[iNumConts], "Template - Custom Continents PYTHON")
	#print("Template: ", templateID)

	# Templates are nested by keys: {(NumContinents, TemplateID): {ContinentID: [#Layers, [Layer1], [Layer2], etc]}}
	# Each Layer: [fWestLon, fEastLon, fSouthLat, fNorthLat, xVar, yVar, bFlagX, bFlagY, iWater, iGrain, iFracFlags, xExp, yExp, iShift]
	#
	# Water: -1 = Use Sirian's default water setting for the chosen map grain.
	# iGrain: 12 = random 1 or 2; 13 = random 1 to 3; 14 = random 1 to 4; iGrain: 23 = random 2 or 3; 24 = random 2 to 4; 34 = random 3 or 4; 35 = random 3 to 5
	# iFracFlags: 0 = 0; 1 = Horz; 2 = Vert; 3 = Round
	#                                                                    Wtr, Grn, Flg, Exps, Shift
	#            Key          Longitude   Latitude   Variance  Var Flags     Fractal Settings
	#
	templates = {(2,0): {0: [2,
	                         [0.03, 0.47, 0.05, 0.65, 0, 0.3, False, True, 55, 23, 3, -1, -1, 11],
	                         [0.14, 0.36, 0.2, 0.5, 0, 0.3, False, True, 60, 1, 1, -1, -1, 7]],
	                     1: [2,
	                         [0.53, 0.97, 0.05, 0.65, 0, 0.3, False, True, 55, 23, 3, -1, -1, 11],
	                         [0.64, 0.86, 0.2, 0.5, 0, 0.3, False, True, 60, 1, 1, -1, -1, 7]]},
	             (2,1): {0: [7,
	                         [0.04, 0.6, 0.74, 0.9, 0.36, -0.64, False, False, 55, 2, 2, -1, -1, 7],
	                         [0.23, 0.41, 0.78, 0.86, 0.36, -0.64, False, False, 60, 1, 1, -1, -1, 5],
	                         [0.1, 0.3, 0.5, 0.9, 0.6, -0.4, False, False, 55, 1, 2, 6, 7, 7],
	                         [0.05, 0.35, 0.45, 0.95, 0.6, -0.4, False, False, 70, 3, 3, 6, 7, 7],
	                         [0.15, 0.5, 0.4, 0.6, 0.35, 0, False, False, 55, 1, 1, -1, -1, 7],
	                         [0.35, 0.5, 0.2, 0.55, 0.15, 0.25, False, False, 60, 2, 3, 6, 6, 5],
	                         [0.2, 0.4, 0.25, 0.4, 0.4, 0.35, False, False, 80, 4, 3, 6, 6, 5]],
	                     1: [5,
	                         [0.6, 0.9, 0.35, 0.45, -0.5, 0.2, False, False, 60, 1, 1, -1, -1, 7],
	                         [0.55, 0.95, 0.3, 0.5, -0.5, 0.2, False, False, 65, 3, 1, -1, -1, 7],
	                         [0.72, 0.85, 0.25, 0.85, -0.57, -0.1, False, False, 50, 1, 2, 6, 7, 7],
	                         [0.7, 0.9, 0.65, 0.85, -0.6, -0.5, False, False, 70, 3, 3, 6, 6, 7],
	                         [0.7, 0.95, 0.1, 0.3, -0.65, 0.6, False, False, 80, 4, 3, 6, 5, 5]]},
	             (2,2): {0: [7,
	                         [0.1, 0.9, 0.2, 0.3, 0, 0.5, False, False, 45, 1, 1, -1, -1, 3],
	                         [0.1, 0.9, 0.1, 0.4, 0, 0.5, False, False, 60, 2, 1, -1, -1, 5],
	                         [0.05, 0.3, 0.15, 0.6, 0, 0.25, False, False, 70, 2, 3, 6, 6, 5],
	                         [0.7, 0.95, 0.15, 0.6, 0, 0.25, False, False, 70, 2, 3, 6, 6, 5],
	                         [0.05, 0.3, 0.15, 0.6, 0, 0.25, False, False, 60, 4, 3, 6, 5, 5],
	                         [0.7, 0.95, 0.15, 0.6, 0, 0.25, False, False, 60, 4, 3, 6, 5, 5],
	                         [0.15, 0.5, 0.05, 0.2, 0.35, 0.75, True, False, 75, 3, 1, 6, 5, 3]],
	                     1: [4,
	                         [0.1, 0.9, 0.65, 0.95, 0, -0.6, False, False, 55, 2, 1, -1, -1, 5],
	                         [0.15, 0.85, 0.75, 0.85, 0, -0.6, False, False, 55, 1, 1, -1, -1, 3],
	                         [0.4, 0.6, 0.45, 0.85, 0, -0.3, False, False, 55, 2, 2, 6, 6, 5],
	                         [0.4, 0.6, 0.45, 0.95, 0, -0.4, False, False, 70, 4, 3, 6, 6, 5]]},
	             (2,3): {0: [3,
	                         [0.03, 0.57, 0.05, 0.75, 0.4, 0.2, False, True, 65, 2, 2, -1, -1, 11],
	                         [0.13, 0.47, 0.2, 0.7, 0.4, 0.1, False, True, 75, 4, 3, -1, -1, 7],
	                         [0.22, 0.38, 0.2, 0.6, 0.4, 0.2, False, True, 60, 1, 1, -1, -1, 7]],
	                     1: [3,
	                         [0.63, 0.97, 0.05, 0.75, -0.6, 0.2, False, True, 60, 2, 2, -1, -1, 9],
	                         [0.68, 0.92, 0.15, 0.65, -0.6, 0.2, False, True, 70, 4, 3, -1, -1, 7],
	                         [0.74, 0.86, 0.2, 0.6, -0.6, 0.2, False, True, 55, 1, 1, -1, -1, 5]]},
	             (2,4): {0: [7,
	                         [0.03, 0.47, 0.6, 0.95, 0.5, -0.55, False, False, 60, 2, 1, -1, -1, 9],
	                         [0.03, 0.37, 0.5, 0.95, 0.6, -0.45, False, False, 60, 2, 1, -1, -1, 7],
	                         [0.03, 0.27, 0.4, 0.85, 0.7, -0.25, False, False, 60, 2, 2, -1, -1, 7],
	                         [0.03, 0.17, 0.3, 0.75, 0.8, -0.05, False, False, 60, 2, 3, -1, -1, 5],
	                         [0.13, 0.57, 0.7, 0.95, 0.5, -0.65, False, False, 60, 1, 1, -1, -1, 7],
	                         [0.23, 0.67, 0.8, 0.95, 0.5, -0.75, False, False, 60, 1, 1, -1, -1, 5],
	                         [0.4, 0.6, 0.5, 0.65, 0, -0.15, False, False, 82, 4, 1, 6, 5, 5]],
	                     1: [8,
	                         [0.53, 0.97, 0.2, 0.55, -0.5, 0.25, False, False, 55, 2, 1, -1, -1, 9],
	                         [0.63, 0.97, 0.3, 0.65, -0.6, 0.05, False, False, 55, 2, 1, -1, -1, 7],
	                         [0.73, 0.97, 0.4, 0.75, -0.7, -0.15, False, False, 55, 2, 1, -1, -1, 7],
	                         [0.83, 0.97, 0.5, 0.85, -0.8, -0.35, False, False, 55, 2, 1, -1, -1, 5],
	                         [0.43, 0.87, 0.1, 0.45, -0.3, 0.45, False, False, 60, 2, 1, -1, -1, 7],
	                         [0.33, 0.77, 0.05, 0.35, -0.1, 0.6, False, False, 60, 2, 1, -1, -1, 7],
	                         [0.23, 0.67, 0.05, 0.25, 0.1, 0.7, False, False, 60, 2, 1, -1, -1, 5],
	                         [0.13, 0.57, 0.05, 0.15, 0.3, 0.8, False, False, 60, 1, 1, -1, -1, 3]]},
	             (2,5): {0: [4,
	                         [0.6, 0.95, 0.2, 0.65, -0.55, 0.15, False, False, 70, 2, 1, -1, -1, 9],
	                         [0.6, 0.95, 0.2, 0.65, -0.55, 0.15, False, False, 65, 2, 2, -1, -1, 9],
	                         [0.4, 0.9, 0.05, 0.45, -0.3, 0.5, False, False, 70, 3, 1, -1, -1, 7],
	                         [0.45, 0.85, 0.15, 0.35, -0.3, 0.5, False, False, 50, 1, 1, -1, -1, 3]],
	                     1: [5,
	                         [0.1, 0.5, 0.5, 0.9, 0.4, -0.4, False, False, 60, 3, 1, -1, -1, 9],
	                         [0.15, 0.45, 0.55, 0.85, 0.4, -0.4, False, False, 65, 1, 1, -1, -1, 7],
	                         [0.3, 0.8, 0.75, 0.95, -0.1, -0.7, False, False, 55, 1, 1, -1, -1, 3],
	                         [0.15, 0.25, 0.2, 0.7, 0.6, 0.1, False, False, 45, 1, 2, 6, 7, 3],
	                         [0.05, 0.35, 0.25, 0.65, 0.6, 0.1, False, False, 75, 4, 3, 6, 6, 5]]},
	             (2,6): {0: [3,
	                         [0.03, 0.77, 0.05, 0.45, 0.2, 0, True, False, 55, 2, 2, -1, -1, 7],
	                         [0.21, 0.59, 0.15, 0.35, 0.2, 0, True, False, 60, 1, 1, -1, -1, 5],
	                         [0.33, 0.47, 0.28, 0.52, 0, 0.2, False, False, 70, 3, 3, 6, 6, 3]],
	                     1: [3,
	                         [0.03, 0.77, 0.55, 0.95, 0.2, 0, True, False, 55, 2, 2, -1, -1, 7],
	                         [0.21, 0.59, 0.65, 0.85, 0.2, 0, True, False, 60, 1, 1, -1, -1, 5],
	                         [0.53, 0.67, 0.48, 0.72, 0, -0.2, False, False, 70, 3, 3, 6, 6, 3]]},
	             (2,7): {0: [8,
	                         [0.3, 0.4, 0.1, 0.9, 0.3, 0, False, True, 40, 1, 2, 6, 7, 5],
	                         [0.15, 0.45, 0.1, 0.3, 0.4, 0.6, False, True, 70, 3, 1, 6, 5, 3],
	                         [0.05, 0.4, 0.15, 0.3, 0.55, 0.2, False, True, 60, 1, 1, 6, 5, 3],
	                         [0.3, 0.65, 0.15, 0.3, 0.05, 0.2, False, True, 60, 1, 1, 6, 5, 3],
	                         [0.05, 0.4, 0.5, 0.65, 0.55, 0.2, False, True, 60, 1, 1, 6, 5, 3],
	                         [0.1, 0.25, 0.18, 0.42, 0.65, 0, False, True, 70, 14, 2, 5, 5, 3],
	                         [0.3, 0.65, 0.5, 0.65, 0.05, 0.2, False, True, 60, 1, 1, 6, 5, 3],
	                         [0.35, 0.6, 0.05, 0.35, 0.05, 0.6, False, True, 80, 34, 0, 6, 6, 5]],
	                     1: [3,
	                         [0.7, 0.95, 0.1, 0.7, -0.6, 0.2, False, True, 60, 24, 2, 6, 6, 5],
	                         [0.75, 0.9, 0.2, 0.6, -0.6, 0.2, False, True, 60, 1, 3, 6, 7, 5],
	                         [0.7, 0.95, 0.1, 0.4, -0.6, 0.5, False, True, 80, 34, 2, 6, 6, 3]]},
	             (2,8): {0: [5,
	                         [0.05, 0.6, 0.1, 0.25, 0.35, 0, False, False, 50, 1, 1, -1, -1, 3],
	                         [0.05, 0.6, 0.4, 0.6, 0.35, 0, False, False, 50, 1, 1, -1, -1, 3],
	                         [0.05, 0.6, 0.75, 0.9, 0.35, 0, False, False, 50, 1, 1, -1, -1, 3],
	                         [0.1, 0.25, 0.45, 0.9, 0.65, -0.35, False, False, 45, 1, 2, 5, 6, 3],
	                         [0.4, 0.55, 0.1, 0.55, 0.05, 0.35, False, False, 45, 1, 2, 5, 6, 3]],
	                     1: [4,
	                         [0.7, 0.95, 0.15, 0.45, -0.65, 0, False, False, 65, 24, 3, 6, 6, 3],
	                         [0.67, 0.98, 0.4, 0.6, -0.65, 0, False, False, 50, 1, 1, -1, -1, 3],
	                         [0.67, 0.8, 0.45, 0.9, -0.47, -0.35, False, False, 45, 1, 2, 5, 6, 3],
	                         [0.85, 0.98, 0.1, 0.55, -0.83, 0.35, False, False, 45, 1, 2, 5, 6, 3]]},
	             (2,9): {0: [3,
	                         [0.03, 0.77, 0.05, 0.7, 0.2, 0.25, True, False, 65, 3, 1, -1, -1, 11],
	                         [0.23, 0.57, 0.2, 0.55, 0.2, 0.25, True, False, 55, 12, 1, -1, -1, 7],
	                         [0.13, 0.67, 0.1, 0.65, 0.2, 0.25, True, False, 80, 34, 1, -1, -1, 9]],
	                     1: [3,
	                         [0.05, 0.65, 0.75, 0.95, 0.3, -0.7, False, False, 55, 2, 1, -1, -1, 5],
	                         [0.15, 0.55, 0.8, 0.9, 0.3, -0.7, True, False, 55, 12, 1, -1, -1, 3],
	                         [0.55, 0.85, 0.75, 0.95, -0.3, -0.7, False, False, 75, 34, 1, 6, 5, 3]]},
	             (3,0): {0: [2,
	                         [0.03, 0.48, 0.05, 0.56, 0, 0.39, False, False, 55, 23, 3, -1, -1, 9],
	                         [0.14, 0.36, 0.18, 0.43, 0, 0.39, False, False, 60, 1, 1, -1, -1, 5]],
	                     1: [2,
	                         [0.52, 0.97, 0.05, 0.56, 0, 0.39, False, False, 55, 23, 3, -1, -1, 9],
	                         [0.64, 0.86, 0.18, 0.43, 0, 0.39, False, False, 60, 1, 1, -1, -1, 5]],
	                     2: [2,
	                         [0.03, 0.83, 0.64, 0.93, 0.14, -0.57, True, False, 55, 23, 1, -1, -1, 9],
	                         [0.23, 0.63, 0.71, 0.86, 0.14, -0.57, True, False, 60, 1, 1, -1, -1, 5]]},
	             (3,1): {0: [2,
	                         [0.03, 0.57, 0.05, 0.46, 0.4, 0, False, False, 55, 13, 3, -1, -1, 9],
	                         [0.21, 0.39, 0.15, 0.36, 0.4, 0, False, False, 60, 1, 1, -1, -1, 5]],
	                     1: [2,
	                         [0.03, 0.57, 0.54, 0.95, 0.4, 0, False, False, 55, 13, 3, -1, -1, 9],
	                         [0.21, 0.39, 0.15, 0.36, 0.4, 0, False, False, 60, 1, 1, -1, -1, 5]],
	                     2: [3,
	                         [0.63, 0.97, 0.05, 0.7, -0.6, 0.25, False, False, 55, 13, 3, 6, 6, 9],
	                         [0.72, 0.88, 0.21, 0.54, -0.6, 0.25, False, False, 60, 1, 1, -1, -1, 5],
	                         [0.65, 0.95, 0.75, 0.9, -0.6, -0.65, False, False, 60, 34, 1, 5, 5, 3]]},
	             (3,2): {0: [3,
	                         [0.03, 0.57, 0.05, 0.75, 0.4, 0.2, False, True, 65, 2, 2, -1, -1, 11],
	                         [0.13, 0.47, 0.2, 0.7, 0.4, 0.1, False, True, 75, 4, 3, -1, -1, 7],
	                         [0.22, 0.38, 0.2, 0.6, 0.4, 0.2, False, True, 60, 1, 1, -1, -1, 7]],
	                     1: [3,
	                         [0.63, 0.97, 0.05, 0.5, -0.6, 0.45, False, False, 60, 2, 2, -1, -1, 5],
	                         [0.68, 0.92, 0.15, 0.4, -0.6, 0.45, False, False, 70, 4, 3, -1, -1, 5],
	                         [0.74, 0.86, 0.2, 0.35, -0.6, 0.45, False, False, 55, 1, 1, 6, 5, 3]],
	                     2: [2,
	                         [0.63, 0.97, 0.55, 0.9, -0.6, -0.45, False, False, 60, 23, 3, -1, -1, 5],
	                         [0.74, 0.86, 0.66, 0.88, -0.6, -0.45, False, False, 55, 1, 1, 6, 5, 3]]},
	             (3,3): {0: [5,
	                         [0.05, 0.45, 0.2, 0.3, 0, 0.5, False, False, 45, 1, 1, -1, -1, 3],
	                         [0.05, 0.45, 0.1, 0.4, 0, 0.5, False, False, 60, 23, 1, -1, -1, 5],
	                         [0.05, 0.3, 0.15, 0.6, 0, 0.25, False, False, 70, 2, 3, 6, 6, 5],
	                         [0.05, 0.3, 0.15, 0.6, 0, 0.25, False, False, 60, 4, 3, 6, 5, 5],
	                         [0.05, 0.25, 0.05, 0.2, 0.2, 0.75, True, False, 75, 3, 1, 6, 5, 3]],
	                     1: [5,
	                         [0.55, 0.95, 0.2, 0.3, 0, 0.5, False, False, 45, 1, 1, -1, -1, 3],
	                         [0.55, 0.95, 0.1, 0.4, 0, 0.5, False, False, 60, 23, 1, -1, -1, 5],
	                         [0.7, 0.95, 0.15, 0.6, 0, 0.25, False, False, 70, 2, 3, 6, 6, 5],
	                         [0.7, 0.95, 0.15, 0.6, 0, 0.25, False, False, 60, 4, 3, 6, 5, 5],
	                         [0.55, 0.75, 0.05, 0.2, 0.2, 0.75, True, False, 75, 3, 1, 6, 5, 3]],
	                     2: [4,
	                         [0.1, 0.9, 0.65, 0.95, 0, -0.6, False, False, 55, 2, 1, -1, -1, 5],
	                         [0.15, 0.85, 0.75, 0.85, 0, -0.6, False, False, 55, 1, 1, -1, -1, 3],
	                         [0.4, 0.6, 0.45, 0.85, 0, -0.3, False, False, 55, 2, 2, 6, 6, 5],
	                         [0.4, 0.6, 0.45, 0.95, 0, -0.4, False, False, 70, 4, 3, 6, 6, 5]]},
	             (3,4): {0: [7,
	                         [0.03, 0.47, 0.6, 0.95, 0.5, -0.55, False, False, 60, 2, 1, -1, -1, 9],
	                         [0.03, 0.37, 0.5, 0.95, 0.6, -0.45, False, False, 60, 2, 1, -1, -1, 7],
	                         [0.03, 0.27, 0.4, 0.85, 0.7, -0.25, False, False, 60, 2, 2, -1, -1, 7],
	                         [0.03, 0.17, 0.3, 0.75, 0.8, -0.05, False, False, 60, 2, 3, -1, -1, 5],
	                         [0.13, 0.57, 0.7, 0.95, 0.3, -0.65, False, False, 60, 1, 1, -1, -1, 7],
	                         [0.23, 0.67, 0.8, 0.95, 0.1, -0.75, False, False, 60, 1, 1, -1, -1, 5],
	                         [0.4, 0.6, 0.5, 0.65, 0, -0.15, False, False, 82, 4, 1, 6, 5, 5]],
	                     1: [5,
	                         [0.53, 0.97, 0.45, 0.55, -0.5, 0, False, False, 45, 1, 1, -1, -1, 3],
	                         [0.63, 0.97, 0.35, 0.65, -0.6, 0, False, False, 55, 23, 3, -1, -1, 5],
	                         [0.73, 0.97, 0.25, 0.75, -0.7, 0, False, False, 55, 2, 3, -1, -1, 7],
	                         [0.83, 0.97, 0.15, 0.85, -0.8, 0, False, False, 55, 2, 3, -1, -1, 5],
	                         [0.9, 0.97, 0.05, 0.95, -0.87, 0, False, False, 50, 1, 2, 5, 6, 3]],
	                     2: [5,
	                         [0.43, 0.47, 0.05, 0.5, 0.1, 0.45, False, False, 45, 1, 2, 5, 6, 1],
	                         [0.33, 0.57, 0.05, 0.4, 0.1, 0.55, False, False, 55, 2, 1, -1, -1, 3],
	                         [0.23, 0.67, 0.05, 0.3, 0.1, 0.65, False, False, 55, 2, 1, -1, -1, 5],
	                         [0.13, 0.77, 0.05, 0.2, 0.1, 0.75, False, False, 55, 2, 1, -1, -1, 3],
	                         [0.03, 0.87, 0.05, 0.1, 0.1, 0.85, False, False, 50, 1, 1, 7, 6, 1]]},
	             (3,5): {0: [4,
	                         [0.6, 0.95, 0.2, 0.65, -0.55, 0.15, False, False, 70, 2, 1, -1, -1, 9],
	                         [0.6, 0.95, 0.2, 0.65, -0.55, 0.15, False, False, 65, 23, 3, -1, -1, 9],
	                         [0.4, 0.9, 0.05, 0.45, -0.3, 0.5, False, False, 70, 3, 1, -1, -1, 7],
	                         [0.45, 0.85, 0.15, 0.35, -0.3, 0.5, False, False, 50, 1, 1, -1, -1, 3]],
	                     1: [2,
	                         [0.55, 0.95, 0.7, 0.9, -0.5, -0.6, False, False, 65, 34, 1, -1, -1, 3],
	                         [0.3, 0.9, 0.75, 0.95, -0.2, -0.7, False, False, 50, 1, 1, -1, -1, 3]],
	                     2: [4,
	                         [0.05, 0.25, 0.4, 0.9, 0.7, -0.3, False, False, 60, 13, 3, -1, -1, 5],
	                         [0.15, 0.45, 0.5, 0.65, 0.4, -0.15, False, False, 65, 1, 1, -1, -1, 3],
	                         [0.15, 0.25, 0.2, 0.6, 0.6, 0.2, False, False, 45, 1, 2, 5, 6, 3],
	                         [0.05, 0.35, 0.25, 0.65, 0.6, 0.1, False, False, 75, 4, 3, 6, 6, 5]]},
	             (3,6): {0: [6,
	                         [0.1, 0.2, 0.1, 0.9, 0, 0, False, False, 40, 1, 2, 5, 6, 3],
	                         [0.05, 0.47, 0.05, 0.35, 0, 0, False, False, 65, 23, 1, -1, -1, 5],
	                         [0.05, 0.47, 0.1, 0.25, 0, 0, False, False, 60, 1, 1, -1, -1, 3],
	                         [0.05, 0.47, 0.65, 0.95, 0, 0, False, False, 65, 23, 1, -1, -1, 5],
	                         [0.05, 0.47, 0.7, 0.95, 0, 0, False, False, 60, 1, 1, -1, -1, 3],
	                         [0.05, 0.25, 0.15, 0.85, 0, 0, False, False, 70, 4, 3, 5, 6, 3]],
	                     1: [2,
	                         [0.3, 0.7, 0.4, 0.6, 0, 0, False, False, 55, 14, 3, -1, -1, 5],
	                         [0.35, 0.65, 0.45, 0.55, 0, 0, False, False, 45, 1, 1, -1, -1, 3]],
	                     2: [6,
	                         [0.8, 0.9, 0.1, 0.9, 0, 0, False, False, 40, 1, 2, 5, 6, 3],
	                         [0.53, 0.95, 0.05, 0.35, 0, 0, False, False, 65, 23, 1, -1, -1, 5],
	                         [0.53, 0.95, 0.1, 0.25, 0, 0, False, False, 60, 1, 1, -1, -1, 3],
	                         [0.53, 0.95, 0.65, 0.95, 0, 0, False, False, 65, 23, 1, -1, -1, 5],
	                         [0.53, 0.95, 0.7, 0.95, 0, 0, False, False, 60, 1, 1, -1, -1, 3],
	                         [0.75, 0.95, 0.15, 0.85, 0, 0, False, False, 70, 4, 3, 5, 6, 3]]},
	             (3,7): {0: [3,
	                         [0.03, 0.77, 0.05, 0.7, 0.2, 0.25, True, False, 65, 3, 1, -1, -1, 11],
	                         [0.23, 0.57, 0.2, 0.55, 0.2, 0.25, True, False, 55, 12, 1, -1, -1, 7],
	                         [0.13, 0.67, 0.1, 0.65, 0.2, 0.25, True, False, 80, 34, 1, -1, -1, 9]],
	                     1: [3,
	                         [0.03, 0.47, 0.75, 0.95, 0, -0.7, False, False, 55, 2, 1, -1, -1, 5],
	                         [0.1, 0.4, 0.8, 0.9, 0, -0.7, False, False, 55, 12, 1, -1, -1, 3],
	                         [0.15, 0.35, 0.75, 0.95, 0, -0.7, False, False, 75, 34, 1, 6, 5, 3]],
	                     2: [3,
	                         [0.53, 0.97, 0.75, 0.95, 0, -0.7, False, False, 55, 2, 1, -1, -1, 5],
	                         [0.6, 0.9, 0.8, 0.9, 0, -0.7, False, False, 55, 12, 1, -1, -1, 3],
	                         [0.65, 0.85, 0.75, 0.95, 0, -0.7, False, False, 75, 34, 1, 6, 5, 3]]},
	             (3,8): {0: [6,
	                         [0.03, 0.57, 0.7, 0.95, 0, -0.65, False, False, 55, 23, 1, -1, -1, 3],
	                         [0.15, 0.55, 0.8, 0.85, 0, -0.65, False, False, 35, 1, 1, -1, -1, 2],
	                         [0.43, 0.97, 0.05, 0.3, 0, 0.65, False, False, 55, 23, 1, -1, -1, 3],
	                         [0.45, 0.85, 0.15, 0.2, 0, 0.65, False, False, 35, 1, 1, -1, -1, 2],
	                         [0.43, 0.57, 0.05, 0.95, 0, 0, False, False, 50, 24, 2, 6, 6, 3],
	                         [0.47, 0.53, 0.1, 0.9, 0, 0, False, False, 20, 1, 2, 5, 6, 1]],
	                     1: [4,
	                         [0.03, 0.37, 0.2, 0.6, 0, 0.3, False, False, 70, 34, 1, -1, -1, 5],
	                         [0.03, 0.17, 0.15, 0.55, 0.2, 0.3, True, False, 45, 12, 2, 5, 6, 3],
	                         [0.03, 0.37, 0.05, 0.2, 0, 0.75, False, False, 55, 1, 1, 6, 5, 3],
	                         [0.03, 0.37, 0.5, 0.65, 0, -0.15, False, False, 55, 1, 1, 6, 5, 3]],
	                     	 2: [4,
	                         [0.63, 0.97, 0.4, 0.8, 0, -0.3, False, False, 70, 34, 1, -1, -1, 5],
	                         [0.63, 0.77, 0.45, 0.85, 0.2, -0.3, True, False, 45, 12, 2, 5, 6, 3],
	                         [0.63, 0.97, 0.8, 0.95, 0, -0.75, False, False, 55, 1, 1, 6, 5, 3],
	                         [0.63, 0.97, 0.35, 0.5, 0, 0.15, False, False, 55, 1, 1, 6, 5, 3]]},
	             (4,0): {0: [2,
	                         [0.03, 0.47, 0.05, 0.45, 0, 0, False, False, 55, 23, 3, -1, -1, 9],
	                         [0.14, 0.36, 0.15, 0.35, 0, 0, False, False, 60, 1, 1, -1, -1, 5]],
	                     1: [2,
	                         [0.53, 0.97, 0.05, 0.45, 0, 0, False, False, 55, 23, 3, -1, -1, 9],
	                         [0.64, 0.86, 0.15, 0.35, 0, 0, False, False, 60, 1, 1, -1, -1, 5]],
	                     2: [2,
	                         [0.03, 0.47, 0.55, 0.95, 0, 0, False, False, 55, 23, 3, -1, -1, 9],
	                         [0.14, 0.36, 0.65, 0.85, 0, 0, False, False, 60, 1, 1, -1, -1, 5]],
	                     3: [2,
	                         [0.53, 0.97, 0.55, 0.95, 0, 0, False, False, 55, 23, 3, -1, -1, 9],
	                         [0.64, 0.86, 0.65, 0.85, 0, 0, False, False, 60, 1, 1, -1, -1, 5]]},
	             (4,1): {0: [3,
	                         [0.03, 0.47, 0.05, 0.65, 0, 0.3, False, False, 70, 34, 3, -1, -1, 9],
	                         [0.07, 0.43, 0.15, 0.55, 0, 0.3, False, False, 65, 23, 3, -1, -1, 7],
	                         [0.14, 0.36, 0.2, 0.5, 0, 0.3, False, False, 60, 1, 1, -1, -1, 5]],
	                     1: [2,
	                         [0.53, 0.97, 0.05, 0.3, 0, 0.65, False, False, 55, 24, 3, -1, -1, 5],
	                         [0.64, 0.86, 0.1, 0.25, 0, 0.65, False, False, 60, 1, 1, -1, -1, 3]],
	                     2: [2,
	                         [0.03, 0.47, 0.70, 0.95, 0, -0.65, False, False, 55, 24, 3, -1, -1, 5],
	                         [0.14, 0.36, 0.75, 0.9, 0, -0.65, False, False, 60, 1, 1, -1, -1, 3]],
	                     3: [3,
	                         [0.53, 0.97, 0.35, 0.95, 0, -0.3, False, False, 70, 34, 3, -1, -1, 9],
	                         [0.57, 0.93, 0.45, 0.85, 0, -0.3, False, False, 65, 23, 3, -1, -1, 7],
	                         [0.64, 0.86, 0.5, 0.8, 0, -0.3, False, False, 60, 1, 1, -1, -1, 5]]},
	             (4,2): {0: [4,
	                         [0.6, 0.95, 0.2, 0.65, -0.55, 0.15, False, False, 70, 2, 1, -1, -1, 9],
	                         [0.6, 0.95, 0.2, 0.65, -0.55, 0.15, False, False, 65, 23, 3, -1, -1, 9],
	                         [0.4, 0.9, 0.05, 0.45, -0.3, 0.5, False, False, 70, 3, 1, -1, -1, 7],
	                         [0.45, 0.85, 0.15, 0.35, -0.3, 0.5, False, False, 50, 1, 1, -1, -1, 3]],
	                     1: [2,
	                         [0.6, 0.95, 0.75, 0.9, -0.55, -0.65, False, False, 55, 1, 1, -1, -1, 3],
	                         [0.6, 0.95, 0.7, 0.95, -0.55, -0.65, False, False, 50, 23, 3, -1, -1, 3]],
	                     2: [2,
	                         [0.1, 0.55, 0.5, 0.9, 0.35, -0.4, False, False, 60, 14, 3, -1, -1, 7],
	                         [0.2, 0.45, 0.6, 0.8, 0.35, -0.4, False, False, 55, 1, 1, -1, -1, 3]],
	                     3: [3,
	                         [0.05, 0.35, 0.1, 0.4, 0.6, 0.5, False, False, 60, 13, 3, 6, 6, 5],
	                         [0.11, 0.28, 0.15, 0.35, 0.61, 0.5, False, False, 50, 1, 3, 6, 6, 3],
	                         [0.03, 0.36, 0.05, 0.45, 0.61, 0.5, False, False, 75, 35, 3, 6, 6, 5]]},
	             (4,3): {0: [5,
	                         [0.05, 0.45, 0.2, 0.3, 0, 0.5, False, False, 45, 1, 1, -1, -1, 3],
	                         [0.05, 0.45, 0.1, 0.4, 0, 0.5, False, False, 60, 23, 1, -1, -1, 5],
	                         [0.05, 0.3, 0.15, 0.6, 0, 0.25, False, False, 70, 2, 3, 6, 6, 5],
	                         [0.05, 0.3, 0.15, 0.6, 0, 0.25, False, False, 60, 4, 3, 6, 5, 5],
	                         [0.15, 0.35, 0.05, 0.2, 0, 0.75, False, False, 75, 3, 1, 6, 5, 3]],
	                     1: [5,
	                         [0.55, 0.95, 0.2, 0.3, 0, 0.5, False, False, 45, 1, 1, -1, -1, 3],
	                         [0.55, 0.95, 0.1, 0.4, 0, 0.5, False, False, 60, 23, 1, -1, -1, 5],
	                         [0.7, 0.95, 0.15, 0.6, 0, 0.25, False, False, 70, 2, 3, 6, 6, 5],
	                         [0.7, 0.95, 0.15, 0.6, 0, 0.25, False, False, 60, 4, 3, 6, 5, 5],
	                         [0.65, 0.85, 0.05, 0.2, 0, 0.75, False, False, 75, 3, 1, 6, 5, 3]],
	                     2: [3,
	                         [0.55, 0.95, 0.8, 0.95, -0.5, -0.75, False, False, 45, 34, 1, -1, -1, 3],
	                         [0.7, 0.95, 0.65, 0.9, -0.65, -0.55, False, False, 55, 2, 3, -1, -1, 3],
	                         [0.75, 0.9, 0.75, 0.85, -0.65, -0.6, False, False, 60, 1, 1, -1, -1, 3]],
	                     3: [5,
	                         [0.05, 0.45, 0.65, 0.95, 0.5, -0.6, False, False, 55, 3, 1, -1, -1, 5],
	                         [0.15, 0.5, 0.7, 0.9, 0.35, -0.6, False, False, 55, 2, 1, -1, -1, 3],
	                         [0.35, 0.5, 0.68, 0.75, 0.15, -0.43, False, False, 35, 1, 1, -1, -1, 3],
	                         [0.4, 0.6, 0.45, 0.75, 0, -0.2, False, False, 55, 2, 3, 6, 6, 5],
	                         [0.35, 0.65, 0.45, 0.75, 0, -0.2, False, False, 70, 4, 3, 6, 6, 5]]},
	             (4,4): {0: [4,
	                         [0.33, 0.47, 0.55, 0.95, 0.2, -0.5, False, False, 50, 23, 1, -1, -1, 3],
	                         [0.23, 0.57, 0.65, 0.95, 0.2, -0.6, False, False, 55, 12, 1, -1, -1, 5],
	                         [0.13, 0.67, 0.75, 0.95, 0.2, -0.7, False, False, 55, 23, 1, -1, -1, 3],
	                         [0.03, 0.77, 0.85, 0.95, 0.2, -0.8, False, False, 50, 1, 1, -1, -1, 3]],
	                     1: [4,
	                         [0.03, 0.37, 0.45, 0.5, 0.6, 0.05, False, False, 40, 1, 1, 6, 5, 1],
	                         [0.03, 0.27, 0.35, 0.6, 0.7, 0.05, False, False, 45, 13, 3, 6, 6, 3],
	                         [0.03, 0.17, 0.25, 0.7, 0.8, 0.05, False, False, 45, 13, 3, 6, 6, 3],
	                         [0.03, 0.12, 0.2, 0.75, 0.85, 0.05, False, False, 40, 1, 2, 5, 6, 1]],
	                     2: [5,
	                         [0.53, 0.97, 0.45, 0.55, -0.5, 0, False, False, 45, 1, 1, -1, -1, 3],
	                         [0.63, 0.97, 0.35, 0.65, -0.6, 0, False, False, 55, 23, 3, -1, -1, 5],
	                         [0.73, 0.97, 0.25, 0.75, -0.7, 0, False, False, 55, 2, 3, -1, -1, 7],
	                         [0.83, 0.97, 0.15, 0.85, -0.8, 0, False, False, 55, 2, 3, -1, -1, 5],
	                         [0.9, 0.97, 0.1, 0.9, -0.87, 0, False, False, 50, 1, 2, 5, 6, 3]],
	                     3: [5,
	                         [0.38, 0.52, 0.05, 0.45, 0.1, 0.5, False, False, 45, 1, 2, 5, 6, 1],
	                         [0.33, 0.57, 0.05, 0.4, 0.1, 0.55, False, False, 55, 2, 1, -1, -1, 3],
	                         [0.23, 0.67, 0.05, 0.3, 0.1, 0.65, False, False, 55, 2, 1, -1, -1, 5],
	                         [0.13, 0.77, 0.05, 0.2, 0.1, 0.75, False, False, 55, 2, 1, -1, -1, 3],
	                         [0.03, 0.87, 0.05, 0.1, 0.1, 0.85, False, False, 50, 1, 1, 7, 6, 1]]},
	             (4,5): {0: [3,
	                         [0.03, 0.77, 0.05, 0.7, 0.2, 0.25, False, False, 65, 3, 1, -1, -1, 11],
	                         [0.23, 0.57, 0.2, 0.55, 0.2, 0.25, False, False, 55, 12, 1, -1, -1, 7],
	                         [0.13, 0.67, 0.1, 0.65, 0.2, 0.25, False, False, 80, 34, 1, -1, -1, 9]],
	                     1: [3,
	                         [0.83, 0.97, 0.1, 0.7, -0.8, 0.2, False, False, 70, 34, 2, 6, 6, 3],
	                         [0.83, 0.97, 0.1, 0.7, -0.8, 0.2, False, False, 55, 23, 3, 6, 6, 3],
	                         [0.88, 0.93, 0.2, 0.6, -0.8, 0.2, False, False, 45, 1, 2, 5, 6, 1]],
	                     2: [3,
	                         [0.03, 0.47, 0.75, 0.95, 0, -0.7, False, False, 55, 2, 1, -1, -1, 5],
	                         [0.1, 0.4, 0.8, 0.9, 0, -0.7, False, False, 55, 12, 1, -1, -1, 3],
	                         [0.15, 0.35, 0.75, 0.95, 0, -0.7, False, False, 75, 34, 1, 6, 5, 3]],
	                     3: [3,
	                         [0.53, 0.97, 0.75, 0.95, 0, -0.7, False, False, 55, 2, 1, -1, -1, 5],
	                         [0.6, 0.9, 0.8, 0.9, 0, -0.7, False, False, 55, 12, 1, -1, -1, 3],
	                         [0.65, 0.85, 0.75, 0.95, 0, -0.7, False, False, 75, 34, 1, 6, 5, 3]]},
	             (4,6): {0: [2,
	                         [0.03, 0.37, 0.05, 0.55, 0.6, 0, False, False, 55, 23, 3, -1, -1, 9],
	                         [0.12, 0.28, 0.15, 0.45, 0.6, 0, False, False, 60, 1, 1, -1, -1, 5]],
	                     1: [2,
	                         [0.03, 0.37, 0.6, 0.95, 0.6, 0, False, False, 55, 23, 3, -1, -1, 7],
	                         [0.12, 0.28, 0.7, 0.85, 0.6, 0, False, False, 50, 1, 1, -1, -1, 3]],
	                     2: [4,
	                         [0.43, 0.67, 0.25, 0.65, -0.1, 0.1, False, False, 70, 24, 3, -1, -1, 5],
	                         [0.45, 0.65, 0.2, 0.6, -0.1, 0.2, False, False, 55, 2, 3, -1, -1, 5],
	                         [0.43, 0.97, 0.05, 0.35, -0.4, 0.6, False, False, 55, 23, 3, -1, -1, 5],
	                         [0.55, 0.85, 0.1, 0.3, -0.4, 0.6, False, False, 60, 1, 1, -1, -1, 3]],
	                     3: [4,
	                         [0.73, 0.97, 0.35, 0.75, -0.7, -0.1, False, False, 70, 24, 3, -1, -1, 5],
	                         [0.73, 0.97, 0.4, 0.8, -0.7, -0.2, False, False, 55, 2, 3, -1, -1, 5],
	                         [0.43, 0.97, 0.65, 0.95, -0.4, -0.6, False, False, 55, 23, 3, -1, -1, 5],
	                         [0.55, 0.85, 0.7, 0.9, -0.4, -0.6, False, False, 60, 1, 1, -1, -1, 3]]},
	             (5,0): {0: [2,
	                         [0.02, 0.32, 0.05, 0.55, 0, 0.4, False, False, 55, 23, 3, -1, -1, 7],
	                         [0.09, 0.25, 0.18, 0.43, 0, 0.4, False, False, 60, 1, 1, -1, -1, 3]],
	                     1: [2,
	                         [0.35, 0.65, 0.05, 0.55, 0, 0.4, False, False, 55, 23, 3, -1, -1, 7],
	                         [0.42, 0.58, 0.18, 0.43, 0, 0.4, False, False, 60, 1, 1, -1, -1, 3]],
	                     2: [2,
	                         [0.68, 0.98, 0.05, 0.55, 0, 0.4, False, False, 55, 23, 3, -1, -1, 7],
	                         [0.75, 0.86, 0.18, 0.43, 0, 0.4, False, False, 60, 1, 1, -1, -1, 3]],
	                     3: [2,
	                         [0.02, 0.48, 0.62, 0.95, 0, -0.57, False, False, 55, 23, 3, -1, -1, 7],
	                         [0.13, 0.37, 0.7, 0.87, 0, -0.57, False, False, 60, 1, 1, -1, -1, 3]],
	                     4: [2,
	                         [0.52, 0.98, 0.62, 0.95, 0, -0.57, False, False, 55, 23, 3, -1, -1, 7],
	                         [0.63, 0.87, 0.7, 0.87, 0, -0.57, False, False, 60, 1, 1, -1, -1, 3]]},
	             (5,1): {0: [2,
	                         [0.03, 0.47, 0.05, 0.4, 0, 0, False, False, 55, 23, 3, -1, -1, 9],
	                         [0.14, 0.36, 0.15, 0.3, 0, 0, False, False, 60, 1, 1, -1, -1, 5]],
	                     1: [2,
	                         [0.53, 0.97, 0.05, 0.4, 0, 0, False, False, 55, 23, 3, -1, -1, 9],
	                         [0.64, 0.86, 0.15, 0.3, 0, 0, False, False, 60, 1, 1, -1, -1, 5]],
	                     2: [2,
	                         [0.03, 0.47, 0.6, 0.95, 0, 0, False, False, 55, 23, 3, -1, -1, 9],
	                         [0.14, 0.36, 0.7, 0.85, 0, 0, False, False, 60, 1, 1, -1, -1, 5]],
	                     3: [2,
	                         [0.53, 0.97, 0.6, 0.95, 0, 0, False, False, 55, 23, 3, -1, -1, 9],
	                         [0.64, 0.86, 0.7, 0.85, 0, 0, False, False, 60, 1, 1, -1, -1, 5]],
	                     4: [3,
	                         [0.05, 0.95, 0.45, 0.55, 0, 0, False, False, 70, 24, 1, -1, -1, 3],
	                         [0.1, 0.9, 0.45, 0.55, 0, 0, False, False, 70, 3, 1, -1, -1, 3],
	                         [0.15, 0.85, 0.48, 0.52, 0, 0, False, False, 0, 1, 1, 5, 5, 1]]},
	             (5,2): {0: [2,
	                         [0.03, 0.57, 0.7, 0.95, 0, -0.65, False, False, 55, 23, 1, -1, -1, 3],
	                         [0.15, 0.55, 0.8, 0.85, 0, -0.65, False, False, 35, 1, 1, -1, -1, 2]],
	                     1: [2,
	                         [0.43, 0.97, 0.05, 0.3, 0, 0.65, False, False, 55, 23, 1, -1, -1, 3],
	                         [0.45, 0.85, 0.15, 0.2, 0, 0.65, False, False, 35, 1, 1, -1, -1, 2]],
	                     2: [2,
	                         [0.43, 0.57, 0.35, 0.65, 0, 0, False, False, 50, 24, 2, 6, 6, 3],
	                         [0.47, 0.53, 0.35, 0.65, 0, 0, False, False, 20, 1, 2, 5, 6, 1]],
	                     3: [4,
	                         [0.03, 0.37, 0.2, 0.6, 0, 0.3, False, False, 70, 34, 1, -1, -1, 5],
	                         [0.03, 0.17, 0.15, 0.55, 0.2, 0.3, True, False, 45, 12, 2, 5, 6, 3],
	                         [0.03, 0.37, 0.05, 0.2, 0, 0.75, False, False, 55, 1, 1, 6, 5, 3],
	                         [0.03, 0.37, 0.5, 0.65, 0, -0.15, False, False, 55, 1, 1, 6, 5, 3]],
	                     	 4: [4,
	                         [0.63, 0.97, 0.4, 0.8, 0, -0.3, False, False, 70, 34, 1, -1, -1, 5],
	                         [0.63, 0.77, 0.45, 0.85, 0.2, -0.3, True, False, 45, 12, 2, 5, 6, 3],
	                         [0.63, 0.97, 0.8, 0.95, 0, -0.75, False, False, 55, 1, 1, 6, 5, 3],
	                         [0.63, 0.97, 0.35, 0.5, 0, 0.15, False, False, 55, 1, 1, 6, 5, 3]]},
	             (5,3): {0: [5,
	                         [0.03, 0.47, 0.2, 0.3, 0, 0.5, False, False, 45, 1, 1, -1, -1, 3],
	                         [0.03, 0.47, 0.1, 0.4, 0, 0.5, False, False, 60, 23, 1, -1, -1, 5],
	                         [0.03, 0.32, 0.15, 0.6, 0.65, 0.25, False, False, 70, 2, 3, 6, 6, 5],
	                         [0.03, 0.32, 0.15, 0.6, 0.65, 0.25, False, False, 60, 4, 3, 6, 5, 5],
	                         [0.13, 0.37, 0.05, 0.2, 0, 0.75, False, False, 75, 3, 1, 6, 5, 3]],
	                     1: [5,
	                         [0.53, 0.97, 0.2, 0.3, 0, 0.5, False, False, 45, 1, 1, -1, -1, 3],
	                         [0.53, 0.97, 0.1, 0.4, 0, 0.5, False, False, 60, 23, 1, -1, -1, 5],
	                         [0.53, 0.82, 0.15, 0.6, -0.35, 0.25, False, False, 70, 2, 3, 6, 6, 5],
	                         [0.53, 0.82, 0.15, 0.6, -0.35, 0.25, False, False, 60, 3, 3, 6, 5, 5],
	                         [0.63, 0.85, 0.05, 0.2, 0, 0.75, False, False, 75, 3, 1, 6, 5, 3]],
	                     2: [5,
	                         [0.38, 0.47, 0.45, 0.75, 0.15, -0.2, False, False, 50, 2, 3, 6, 5, 3],
	                         [0.4, 0.45, 0.5, 0.75, 0.15, -0.25, False, False, 50, 1, 2, 5, 6, 1],
	                         [0.03, 0.32, 0.65, 0.95, 0.65, -0.6, False, False, 60, 24, 3, 6, 5, 3],
	                         [0.08, 0.27, 0.7, 0.9, 0.65, -0.6, False, False, 50, 2, 3, 6, 5, 1],
	                         [0.03, 0.47, 0.65, 0.75, 0.5, -0.4, False, False, 50, 1, 1, 6, 5, 2]],
	                     3: [4,
	                         [0.53, 0.82, 0.65, 0.95, -0.35, -0.6, False, False, 60, 24, 3, 6, 5, 3],
	                         [0.53, 0.82, 0.7, 0.9, -0.35, -0.6, False, False, 50, 2, 3, 6, 5, 1],
	                         [0.37, 0.82, 0.8, 0.95, -0.19, -0.75, False, False, 55, 24, 3, -1, -1, 3],
	                         [0.42, 0.77, 0.85, 0.9, -0.19, -0.75, False, False, 40, 1, 1, -1, -1, 1]],
	                     4: [2,
	                         [0.87, 0.98, 0.45, 0.95, -0.85, -0.4, False, False, 55, 23, 3, -1, -1, 3],
	                         [0.87, 0.98, 0.5, 0.9, -0.85, -0.4, False, False, 60, 1, 1, -1, -1, 3]]},
	             (5,4): {0: [4,
	                         [0.03, 0.37, 0.05, 0.25, 0, 0, False, False, 55, 23, 1, 6, 5, 3],
	                         [0.1, 0.3, 0.1, 0.2, 0, 0, False, False, 50, 1, 1, 6, 5, 2],
	                         [0.03, 0.17, 0.1, 0.47, 0, 0, False, False, 60, 24, 1, 5, 5, 3],
	                         [0.05, 0.15, 0.05, 0.45, 0, 0, False, False, 50, 2, 1, 5, 5, 3]],
	                     1: [4,
	                         [0.03, 0.37, 0.75, 0.95, 0, 0, False, False, 55, 23, 1, 6, 5, 3],
	                         [0.1, 0.3, 0.8, 0.9, 0, 0, False, False, 50, 1, 1, 6, 5, 2],
	                         [0.03, 0.17, 0.53, 0.9, 0, 0, False, False, 60, 24, 1, 5, 5, 3],
	                         [0.05, 0.15, 0.55, 0.95, 0, 0, False, False, 50, 2, 1, 5, 5, 3]],
	                     2: [4,
	                         [0.63, 0.97, 0.05, 0.25, 0, 0, False, False, 55, 23, 1, 6, 5, 3],
	                         [0.7, 0.9, 0.1, 0.2, 0, 0, False, False, 50, 1, 1, 6, 5, 2],
	                         [0.83, 0.97, 0.1, 0.47, 0, 0, False, False, 60, 24, 1, 5, 5, 3],
	                         [0.85, 0.95, 0.05, 0.45, 0, 0, False, False, 50, 2, 1, 5, 5, 3]],
	                     3: [4,
	                         [0.63, 0.97, 0.75, 0.95, 0, 0, False, False, 55, 23, 1, 6, 5, 3],
	                         [0.7, 0.9, 0.8, 0.9, 0, 0, False, False, 50, 1, 1, 6, 5, 2],
	                         [0.83, 0.97, 0.53, 0.9, 0, 0, False, False, 60, 24, 1, 5, 5, 3],
	                         [0.85, 0.95, 0.55, 0.95, 0, 0, False, False, 50, 2, 1, 5, 5, 3]],
	                     4: [7,
	                         [0.43, 0.57, 0.1, 0.4, 0, 0, False, False, 70, 34, 1, 6, 5, 3],
	                         [0.43, 0.57, 0.6, 0.85, 0, 0, False, False, 70, 34, 1, 6, 5, 3],
	                         [0.43, 0.57, 0.1, 0.4, 0, 0, False, False, 55, 2, 3, 6, 5, 3],
	                         [0.43, 0.57, 0.6, 0.85, 0, 0, False, False, 55, 2, 3, 6, 5, 3],
	                         [0.23, 0.77, 0.3, 0.7, 0, 0, False, False, 60, 3, 3, -1, -1, 7],
	                         [0.33, 0.67, 0.4, 0.6, 0, 0, False, False, 50, 5, 3, 6, 5, 5],
	                         [0.28, 0.72, 0.35, 0.65, 0, 0, False, False, 50, 2, 1, -1, -1, 5]]},
	             (5,5): {0: [3,
	                         [0.43, 0.97, 0.05, 0.45, -0.4, 0.5, False, False, 55, 2, 2, -1, -1, 7],
	                         [0.55, 0.85, 0.15, 0.35, -0.4, 0.5, False, False, 60, 1, 1, -1, -1, 5],
	                         [0.5, 0.9, 0.1, 0.4, -0.4, 0.5, False, False, 70, 3, 3, 6, 6, 5]],
	                     1: [3,
	                         [0.23, 0.77, 0.55, 0.95, 0, -0.5, False, False, 55, 2, 2, -1, -1, 7],
	                         [0.35, 0.65, 0.65, 0.85, 0, -0.5, False, False, 60, 1, 1, -1, -1, 5],
	                         [0.3, 0.7, 0.6, 0.9, 0, -0.5, False, False, 70, 3, 3, 6, 6, 5]],
	                     2: [2,
	                         [0.03, 0.37, 0.05, 0.45, 0.6, 0.5, False, False, 55, 23, 3, 6, 5, 5],
	                         [0.1, 0.3, 0.15, 0.35, 0.6, 0.5, False, False, 50, 1, 1, 6, 5, 3]],
	                     3: [2,
	                         [0.03, 0.17, 0.5, 0.95, 0, -0.45, False, False, 55, 23, 3, 6, 5, 5],
	                         [0.05, 0.15, 0.55, 0.9, 0, -0.45, False, False, 50, 1, 3, 6, 5, 3]],
	                     4: [2,
	                         [0.83, 0.97, 0.5, 0.95, 0, -0.45, False, False, 55, 23, 3, 6, 5, 5],
	                         [0.85, 0.95, 0.55, 0.9, 0, -0.45, False, False, 50, 1, 3, 6, 5, 3]]},
	             (6,0): {0: [2,
	                         [0.02, 0.32, 0.05, 0.47, 0, 0, False, False, 55, 23, 3, 6, 6, 7],
	                         [0.09, 0.25, 0.15, 0.35, 0, 0, False, False, 60, 1, 1, 5, 5, 3]],
	                     1: [2,
	                         [0.36, 0.64, 0.05, 0.47, 0, 0, False, False, 55, 23, 3, 6, 6, 7],
	                         [0.42, 0.58, 0.15, 0.35, 0, 0, False, False, 60, 1, 1, 5, 5, 3]],
	                     2: [2,
	                         [0.68, 0.98, 0.05, 0.47, 0, 0, False, False, 55, 23, 3, 6, 6, 7],
	                         [0.75, 0.91, 0.15, 0.35, 0, 0, False, False, 60, 1, 1, 5, 5, 3]],
	                     3: [2,
	                         [0.02, 0.32, 0.53, 0.95, 0, 0, False, False, 55, 23, 3, 6, 6, 7],
	                         [0.09, 0.25, 0.65, 0.85, 0, 0, False, False, 60, 1, 1, 5, 5, 3]],
	                     4: [2,
	                         [0.36, 0.64, 0.53, 0.95, 0, 0, False, False, 55, 23, 3, 6, 6, 7],
	                         [0.42, 0.58, 0.65, 0.85, 0, 0, False, False, 60, 1, 1, 5, 5, 3]],
	                     5: [2,
	                         [0.68, 0.98, 0.53, 0.95, 0, 0, False, False, 55, 23, 3, 6, 6, 7],
	                         [0.75, 0.91, 0.65, 0.85, 0, 0, False, False, 60, 1, 1, 5, 5, 3]]},
	             (6,1): {0: [2,
	                         [0.02, 0.32, 0.05, 0.37, 0, 0.58, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.09, 0.25, 0.15, 0.25, 0, 0.58, False, False, 60, 1, 1, 5, 5, 3]],
	                     1: [2,
	                         [0.36, 0.64, 0.05, 0.57, 0, 0.38, False, False, 55, 23, 3, 6, 6, 7],
	                         [0.42, 0.58, 0.15, 0.45, 0, 0.38, False, False, 60, 1, 1, 5, 5, 3]],
	                     2: [2,
	                         [0.68, 0.98, 0.05, 0.37, 0, 0.58, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.75, 0.91, 0.15, 0.25, 0, 0.58, False, False, 60, 1, 1, 5, 5, 3]],
	                     3: [2,
	                         [0.02, 0.32, 0.43, 0.95, 0, -0.38, False, False, 55, 23, 3, 6, 6, 7],
	                         [0.09, 0.25, 0.55, 0.85, 0, -0.38, False, False, 60, 1, 1, 5, 5, 3]],
	                     4: [2,
	                         [0.36, 0.64, 0.63, 0.95, 0, -0.58, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.42, 0.58, 0.75, 0.85, 0, -0.58, False, False, 60, 1, 1, 5, 5, 3]],
	                     5: [2,
	                         [0.68, 0.98, 0.43, 0.95, 0, -0.38, False, False, 55, 23, 3, 6, 6, 7],
	                         [0.75, 0.91, 0.55, 0.85, 0, -0.38, False, False, 60, 1, 1, 5, 5, 3]]},
	             (6,2): {0: [2,
	                         [0.03, 0.47, 0.05, 0.53, 0, 0.42, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.1, 0.4, 0.17, 0.41, 0, 0.42, False, False, 60, 1, 1, 5, 5, 3]],
	                     1: [2,
	                         [0.53, 0.72, 0.05, 0.53, 0, 0.42, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.57, 0.67, 0.17, 0.41, 0, 0.42, False, False, 60, 1, 1, 5, 5, 3]],
	                     2: [2,
	                         [0.77, 0.97, 0.05, 0.53, 0, 0.42, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.82, 0.92, 0.17, 0.41, 0, 0.42, False, False, 60, 1, 1, 5, 5, 3]],
	                     3: [2,
	                         [0.02, 0.32, 0.58, 0.95, 0, -0.53, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.1, 0.25, 0.67, 0.84, 0, -0.53, False, False, 60, 1, 1, 5, 5, 3]],
	                     4: [2,
	                         [0.35, 0.65, 0.58, 0.95, 0, -0.53, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.42, 0.58, 0.67, 0.84, 0, -0.53, False, False, 60, 1, 1, 5, 5, 3]],
	                     5: [2,
	                         [0.68, 0.98, 0.58, 0.95, 0, -0.53, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.75, 0.9, 0.67, 0.84, 0, -0.53, False, False, 60, 1, 1, 5, 5, 3]]},
	             (6,3): {0: [2,
	                         [0.02, 0.35, 0.06, 0.62, 0.63, 0.32, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.1, 0.27, 0.2, 0.5, 0.63, 0.32, False, False, 60, 1, 1, 5, 5, 3]],
	                     1: [2,
	                         [0.02, 0.35, 0.66, 0.94, 0.63, -0.6, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.1, 0.27, 0.73, 0.87, 0.63, -0.6, False, False, 60, 1, 1, 5, 5, 3]],
	                     2: [2,
	                         [0.39, 0.61, 0.05, 0.48, 0, 0, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.45, 0.55, 0.16, 0.37, 0, 0, False, False, 60, 1, 1, 5, 5, 3]],
	                     3: [2,
	                         [0.39, 0.61, 0.52, 0.95, 0, 0, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.45, 0.55, 0.63, 0.84, 0, 0, False, False, 60, 1, 1, 5, 5, 3]],
	                     4: [2,
	                         [0.65, 0.98, 0.06, 0.62, -0.63, 0, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.73, 0.9, 0.2, 0.5, 0, -0.63, False, False, 60, 1, 1, 5, 5, 3]],
	                     5: [2,
	                         [0.65, 0.98, 0.66, 0.94, -0.63, 0, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.73, 0.9, 0.73, 0.87, -0.63, 0, False, False, 60, 1, 1, 5, 5, 3]]},
	             (6,4): {0: [2,
	                         [0.03, 0.27, 0.05, 0.53, 0, 0.42, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.08, 0.22, 0.17, 0.41, 0, 0.42, False, False, 60, 1, 1, 5, 5, 3]],
	                     1: [2,
	                         [0.33, 0.67, 0.05, 0.53, 0, 0.42, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.4, 0.6, 0.17, 0.41, 0, 0.42, False, False, 60, 1, 1, 5, 5, 3]],
	                     2: [2,
	                         [0.73, 0.97, 0.05, 0.53, 0, 0.42, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.78, 0.92, 0.17, 0.41, 0, 0.42, False, False, 60, 1, 1, 5, 5, 3]],
	                     3: [2,
	                         [0.02, 0.32, 0.58, 0.95, 0, -0.53, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.1, 0.25, 0.67, 0.84, 0, -0.53, False, False, 60, 1, 1, 5, 5, 3]],
	                     4: [2,
	                         [0.35, 0.65, 0.58, 0.95, 0, -0.53, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.42, 0.58, 0.67, 0.84, 0, -0.53, False, False, 60, 1, 1, 5, 5, 3]],
	                     5: [2,
	                         [0.68, 0.98, 0.58, 0.95, 0, -0.53, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.75, 0.9, 0.67, 0.84, 0, -0.53, False, False, 60, 1, 1, 5, 5, 3]]},
	             (6,5): {0: [2,
	                         [0.02, 0.6, 0.06, 0.34, 0.38, 0.6, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.1, 0.53, 0.13, 0.27, 0.38, 0.6, False, False, 60, 1, 1, 5, 5, 3]],
	                     1: [2,
	                         [0.02, 0.35, 0.38, 0.62, 0, 0, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.1, 0.27, 0.44, 0.56, 0, 0, False, False, 60, 1, 1, 5, 5, 3]],
	                     2: [4,
	                         [0.02, 0.35, 0.66, 0.94, 0.62, -0.6, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.1, 0.27, 0.73, 0.87, 0.62, -0.6, False, False, 60, 1, 1, 5, 5, 3],
	                         [0.39, 0.61, 0.05, 0.48, 0, 0, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.45, 0.55, 0.16, 0.37, 0, 0, False, False, 60, 1, 1, 5, 5, 3]],
	                     3: [4,
	                         [0.39, 0.61, 0.52, 0.95, 0, 0, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.45, 0.55, 0.63, 0.84, 0, 0, False, False, 60, 1, 1, 5, 5, 3],
	                         [0.4, 0.98, 0.06, 0.34, -0.38, 0, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.47, 0.9, 0.13, 0.27, -0.38, 0, False, False, 60, 1, 1, 5, 5, 3]],
	                     4: [2,
	                         [0.65, 0.98, 0.38, 0.62, 0, 0, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.73, 0.9, 0.44, 0.56, 0, 0, False, False, 60, 1, 1, 5, 5, 3]],
	                     5: [2,
	                         [0.65, 0.98, 0.66, 0.94, -0.62, 0, False, False, 55, 23, 3, 6, 6, 5],
	                         [0.73, 0.9, 0.73, 0.87, -0.62, 0, False, False, 60, 1, 1, 5, 5, 3]]}
	}
	# End of template data.

	# List region_coords: [WestLon, EastLon, SouthLat, NorthLat]
	cont_data = templates[(iNumConts, templateID)]
	#print cont_data
		
class CCMultilayeredFractal(CvMapGeneratorUtil.MultilayeredFractal):
	def generatePlotsByRegion(self):
		# Sirian's MultilayeredFractal class, controlling function.
		# You -MUST- customize this function for each use of the class.
		global xShiftRoll, yShiftRoll
		userInputContinents = self.map.getCustomMapOption(0)
		defaultWater = [55, 60, 65, 70, 75, 80, 85] # Default values if iWater is set to -1
		
		# Add a few random patches of Tiny Islands first.
		numTinies = 1 + self.dice.get(4, "Tiny Islands - Custom Continents PYTHON")
		#print("Patches of Tiny Islands: ", numTinies)
		if numTinies:
			for tiny_loop in range(numTinies):
				tinyWestLon = 0.01 * self.dice.get(85, "Tiny Longitude - Custom Continents PYTHON")
				tinyWestX = int(self.iW * tinyWestLon)
				tinySouthLat = 0.01 * self.dice.get(85, "Tiny Latitude - Custom Continents PYTHON")
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

		# Add the Continents.
		global iNumConts
		global cont_data
		for continent_loop in range(iNumConts):
			# Each layer of this continent needs its own pass through the fractal generator.
			iNumLayers = cont_data[continent_loop][0]
			for region_loop in range(1, (iNumLayers + 1)):
				# Now read in the variables for this layer.
				#print "--------"
				#print("Data for Continent #: ", continent_loop, " Layer #: ", region_loop)
				#print cont_data[continent_loop][region_loop]
				#print "--------"
				[fWestLon, fEastLon, fSouthLat, fNorthLat, horzVar, vertVar, bFlagX, bFlagY, iWater, iGrain, flagID, xExp, yExp, iShift] = cont_data[continent_loop][region_loop]

				# Region dimensions
				iWestX = int(self.iW * fWestLon)
				iEastX = int(self.iW * fEastLon) - 1
				iSouthY = int(self.iH * fSouthLat)
				iNorthY = int(self.iH * fNorthLat) -1
				iWidth = iEastX - iWestX + 1
				iHeight = iNorthY - iSouthY + 1

				# Handle horizontal shift of this layer, if applicable.
				if horzVar != 0:
					xVar = int(self.iW * horzVar)
					if bFlagX: # The shift is meant to be random within a range.
						xShift = self.dice.get(xVar, "Region Shift, Horizontal - Custom Continents PYTHON")
					else: # The shift is meant to be absolute and binary: shift, or don't, at random.
						if xShiftRoll:
							xShift = xVar
						else:
							xShift = 0
					iWestX += xShift

				# Handle vertical shift of this layer, if applicable.
				if vertVar != 0:
					yVar = int(self.iH * vertVar)
					if bFlagY: # The shift is meant to be random within a range.
						yShift = self.dice.get(yVar, "Region Shift, Vertical - Custom Continents PYTHON")
					else: # The shift is meant to be absolute and binary: shift, or don't, at random.
						if yShiftRoll:
							yShift = yVar
						else:
							yShift = 0
					iSouthY += yShift

				# Handle the map grain.
				if iGrain > 0 and iGrain < 7: pass
				elif iGrain == 12:
					grainRoll = self.dice.get(2, "Random Grain - Custom Continents PYTHON")
					iGrain = grainRoll + 1
				elif iGrain == 13:
					grainRoll = self.dice.get(3, "Random Grain - Custom Continents PYTHON")
					iGrain = grainRoll + 1
				elif iGrain == 14:
					grainRoll = self.dice.get(4, "Random Grain - Custom Continents PYTHON")
					iGrain = grainRoll + 1
				elif iGrain == 23:
					grainRoll = self.dice.get(2, "Random Grain - Custom Continents PYTHON")
					iGrain = grainRoll + 2
				elif iGrain == 24:
					grainRoll = self.dice.get(3, "Random Grain - Custom Continents PYTHON")
					iGrain = grainRoll + 2
				elif iGrain == 34:
					grainRoll = self.dice.get(2, "Random Grain - Custom Continents PYTHON")
					iGrain = grainRoll + 3
				elif iGrain == 35:
					grainRoll = self.dice.get(3, "Random Grain - Custom Continents PYTHON")
					iGrain = grainRoll + 3
				else: # Unexpected data value, transforming to Grain 2.
					iGrain = 2
				
				# Handle the water value.
				if iWater == -1:
					iWater = defaultWater[iGrain]
				
				# Handle the map fractal flags.
				if flagID == 0:
					iFlags = 0
				elif flagID == 1:
					iFlags = self.iHorzFlags
				elif flagID == 2:
					iFlags = self.iVertFlags
				elif flagID == 3:
					iFlags = self.iRoundFlags
				else: # Unexpected data value, transforming to empty flags.
					iFlags = 0

				self.generatePlotsInRegion(iWater,
				                           iWidth, iHeight,
				                           iWestX, iSouthY,
				                           iGrain, 4,
				                           iFlags, self.iTerrainFlags,
				                           xExp, yExp,
				                           True, iShift,
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
	NiTextOut("Setting Plot Types (Python Custom Continents) ...")
	fractal_world = CCMultilayeredFractal()
	plotTypes = fractal_world.generatePlotsByRegion()
	return plotTypes

def generateTerrainTypes():
	NiTextOut("Generating Terrain (Python Custom Continents) ...")
	terraingen = TerrainGenerator()
	terrainTypes = terraingen.generateTerrain()
	return terrainTypes

def addFeatures():
	NiTextOut("Adding Features (Python Custom Continents) ...")
	featuregen = FeatureGenerator()
	featuregen.addFeatures()
	return 0

def assignStartingPlots():
	# Unless "One Per Team", use default.
	userInputContinents = CyMap().getCustomMapOption(0)
	if userInputContinents != 1:
		CyPythonMgr().allowDefaultImpl()
		return

	# One Per Team
	global team_num
	global shuffledTeams
	gc = CyGlobalContext()
	dice = gc.getGame().getMapRand()
	iTeams = gc.getGame().countCivTeamsEverAlive()

	team_num = []
	team_index = 0
	for teamCheckLoop in range(18):
		if CyGlobalContext().getTeam(teamCheckLoop).isEverAlive():
			team_num.append(team_index)
			team_index += 1
		else:
			team_num.append(-1)

	if iTeams < 7:
		team_list = range(iTeams)
		shuffledTeams = []
		for teamLoop in range(iTeams):
			iChooseTeam = dice.get(len(team_list), "Shuffling Regions - Custom Continents PYTHON")
			shuffledTeams.append(team_list[iChooseTeam])
			del team_list[iChooseTeam]
	CyPythonMgr().allowDefaultImpl()
	return
	
def findStartingPlot(argsList):
	# Unless "One Per Team", use default.
	userInputContinents = CyMap().getCustomMapOption(0)
	numTeams = CyGlobalContext().getGame().countCivTeamsAlive()
	if userInputContinents != 1 or numTeams > 6:
		CyPythonMgr().allowDefaultImpl()
		return

	# One Per Team
	[playerID] = argsList
	global yShiftRoll
	global shuffledTeams
	global team_num
	global cont_data
	global iWestX, iEastX, iSouthY, iNorthY
	map = CyMap()
	iW = map.getGridWidth()
	iH = map.getGridHeight()
	thisTeamID = CyGlobalContext().getPlayer(playerID).getTeam()
	teamID = team_num[thisTeamID]
	contNum = shuffledTeams[teamID]
	[fWestLon, fEastLon, fSouthLat, fNorthLat] = cont_data[contNum][1][0:4]
	vertVar = cont_data[contNum][1][5]
	yVar = int(iH * vertVar)
	#print cont_data[contNum][1][0:4]
	#print "Shift: ", yShiftRoll, "Amount: ", yVar
	iWestX = int(iW * fWestLon)
	iEastX = int(iW * fEastLon) - 1
	if numTeams == 2:
		iSouthY = 1
		iNorthY = iH - 2
	elif (numTeams == 3 or numTeams == 5) and yShiftRoll:
		iSouthY = int(iH * fSouthLat) + yVar
		iNorthY = int(iH * fNorthLat) + yVar - 1
	else:
		iSouthY = int(iH * fSouthLat)
		iNorthY = int(iH * fNorthLat) - 1
	#print "West X: ", iWestX
	#print "East X: ", iEastX
	#print "South Y: ", iSouthY
	#print "North Y: ", iNorthY

	def isValid(playerID, x, y):
		global iWestX, iEastX, iSouthY, iNorthY
		if x >= iWestX and x <= iEastX and y >= iSouthY and y <= iNorthY:
			return true
		return false
	
	return CvMapGeneratorUtil.findStartingPlot(playerID, isValid)

def normalizeStartingPlotLocations():
	# Unless "One Per Team", use default.
	userInputContinents = CyMap().getCustomMapOption(0)
	numTeams = CyGlobalContext().getGame().countCivTeamsAlive()
	if userInputContinents != 1 or numTeams > 6:
		CyPythonMgr().allowDefaultImpl()
		return
	return None