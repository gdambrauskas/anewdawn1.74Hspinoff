from __future__ import division
from CvPythonExtensions import *
from copy import copy, deepcopy
from sys import maxint
from math import *
import BugUtil

gc = CyGlobalContext()
game = gc.getGame()
dice = game.getSorenRand()

DMAX_EARTH = 20038	# max possible distance between two points on earth (WGS-84)

CoordinatesDictionary = {}

def InitCoordinatesDictionary():
	coords = []
	global CoordinatesDictionary
	
	#Ensure that the Civilization exists first
	
	##Afforess Note to Modders:
	#getInfoTypeForStringWithHiddenAssert is functionally identical to getInfoTypeForString, except that does not produce errors when it returns -1 (XML not found).
	#Since this merely checks whether the listed civilizations exists, producing error logs would be inconvient.
	
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ABORIGINES") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_ABORIGINES", 2.084, 49.014))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_AUSTRALIA") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_AUSTRALIA", 2.085, 49.015))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_NEWZEALAND") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_NEWZEALAND", 2.086, 49.016))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_POLYNESIA") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_POLYNESIA", 2.087, 49.015))

	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_DENE") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_DENE", 39.898, 5.040))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_UPAAJUT") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_UPAAJUT", 39.897, 5.039))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_CANADA") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_CANADA", 39.896, 5.038))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_AMERICA") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_AMERICA", 39.895, 5.037))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_CUBA") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_CUBA", 39.894, 5.036))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_HAITI") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_HAITI", 39.893, 5.035))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_MEXICO") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_MEXICO", 39.892, 5.034))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_WESTINDIES") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_WESTINDIES", 39.891, 5.033))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_CHILE") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_CHILE", 39.890, 5.032))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_VENEZUELA") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_VENEZUELA", 39.889, 5.031))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_BRAZIL") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_BRAZIL", 39.888, 5.030))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_HONDURAS") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_HONDURAS", 39.887, 5.029))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_BOLIVARIAN") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_BOLIVARIAN", 39.886, 5.028))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ARGENTINA") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_ARGENTINA", 39.885, 5.027))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_AZTEC") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_AZTEC", 39.884, 5.026))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_NATIVE_AMERICA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_NATIVE_AMERICA", 39.883, 5.025))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_IROQUOIS") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_IROQUOIS", 39.882, 5.024))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_INCA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_INCA", 39.881, 5.023))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_MAYA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_MAYA", 39.880, 5.023))

	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_FINLAND") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_FINLAND", 49.562, 20.143))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_GERMANY") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_GERMANY", 49.561, 20.142))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_AUSTRIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_AUSTRIA", 49.560, 20.141))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_POLAND") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_POLAND", 49.559, 20.140))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_NORWAY") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_NORWAY", 49.558, 20.139))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_DENMARK") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_DENMARK", 49.557, 20.138))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_NETHERLANDS") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_NETHERLANDS", 49.556, 20.137))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ICELAND") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_ICELAND", 49.554, 20.125))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_IRELAND") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_IRELAND", 49.553, 20.126))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_WALES") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_WALES", 49.552, 20.127))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_SCOTLAND") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_SCOTLAND", 49.551, 20.128))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ENGLAND") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_ENGLAND", 49.550, 20.129))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_BELGIUM") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_BELGIUM", 49.549, 20.130))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_BULGARIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_BULGARIA", 49.548, 20.131))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_SWEDEN") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_SWEDEN", 49.547, 20.132))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_SWISS_CONFEDERACY") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_SWISS_CONFEDERACY", 49.546, 20.133))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_LITHUANIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_LITHUANIA", 49.545, 20.134))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_LATVIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_LATVIA", 49.544, 20.135))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_MAGYAR") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_MAGYAR", 49.543, 20.136))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_FRANCE") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_FRANCE", 49.542, 20.124))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_SPAIN") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_SPAIN", 49.541, 20.123))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_PORTUGAL") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_PORTUGAL", 49.540, 20.122))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ITALY") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_ITALY", 49.539, 20.144))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_PAPAL") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_PAPAL", 49.538, 20.145))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_GREECE") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_GREECE", 49.537, 20.145))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_MICROSTATESEUR") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_MICROSTATESEUR", 49.536, 20.146))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ROMANIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_ROMANIA", 49.535, 20.147))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_CROATIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_CROATIA", 49.534, 20.148))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_SLOVENIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_SLOVENIA", 49.533, 20.149))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_CZECH") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_CZECH", 49.532, 20.150))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_SLOVAKIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_SLOVAKIA", 49.531, 20.150))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_BIH") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_BIH", 49.563, 20.151))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_SERBIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_SERBIA", 49.564, 20.152))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_KOSOVO") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_KOSOVO", 49.565, 20.153))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_UKRAINE") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_UKRAINE", 49.566, 20.154))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_RUSSIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_RUSSIA", 49.567, 20.155))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_GEORGIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_GEORGIA", 49.568, 20.156))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_CHECHNYA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_CHECHNYA", 49.569, 20.157))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ROME") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_ROME", 49.530, 20.158))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_BAKTRIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_BAKTRIA", 49.529, 20.159))

	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_KAZAKH") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_KAZAKH", 45.046, 45.752))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_UZBEK") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_UZBEK", 45.045, 45.753))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_AFGHANISTAN") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_AFGHANISTAN", 45.044, 45.754))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_CENTRALASUN") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_CENTRALASUN", 45.043, 45.755))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_UYGHUR") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_UYGHUR", 45.042, 45.756))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_MONGOL") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_MONGOL", 45.041, 45.757))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_JAPAN") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_JAPAN", 45.040, 45.772))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_KOREA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_KOREA", 45.039, 45.766))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_CHINA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_CHINA", 45.038, 45.765))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_TIBET") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_TIBET", 45.037, 45.764))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_VIETNAM") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_VIETNAM", 45.036, 45.763))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_KHMER") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_KHMER", 45.035, 45.762))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_BURMA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_BURMA", 45.034, 45.761))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_BANGLADESH") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_BANGLADESH", 45.033, 45.760))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_INDIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_INDIA", 45.032, 45.759))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_PAKISTAN") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_PAKISTAN", 45.031, 45.758))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_SIAM") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_SIAM", 45.030, 45.767))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_MALAYSIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_MALAYSIA", 45.029, 45.768))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_MICROSTATESA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_MICROSTATESA", 45.028, 45.769))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_INDONESIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_INDONESIA", 45.027, 45.770))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_PHILIPPINES") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_PHILIPPINES", 45.026, 45.771))

	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ASSYRIA") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_ASSYRIA", 21.434, 39.838))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ARMENIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_ARMENIA", 21.433, 39.837))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ALBANIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_ALBANIA", 21.432, 39.836))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_OTTOMAN") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_OTTOMAN", 21.431, 39.835))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_LEBANON") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_LEBANON", 21.430, 39.834))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ISRAEL") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_ISRAEL", 21.429, 39.833))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_PALMYRA") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_PALMYRA", 21.428, 39.832))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_IRAQ") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_IRAQ", 21.427, 39.831))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_IRAN") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_IRAN", 21.426, 39.830))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ARABIA") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_ARABIA", 21.425, 39.829))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_YEMEN") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_YEMEN", 21.424, 39.828))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_SOMALIA") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_SOMALIA", 21.423, 39.827))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ETHIOPIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_ETHIOPIA", 21.422, 39.826))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_EGYPT") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_EGYPT", 21.421, 39.825))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_SUDAN") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_SUDAN", 21.420, 39.824))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_GARAMANTES") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_GARAMANTES", 21.419, 39.823))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ABYSSINIANS") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_ABYSSINIANS", 21.418, 39.822))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_MORROCO") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_MORROCO", 21.417, 39.821))

	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_KANEMBORNU") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_KANEMBORNU", 10.418, 29.825))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_MALI") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_MALI", 10.417, 29.824))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_BENIN") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_BENIN", 10.416, 29.823))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_NIGERIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_NIGERIA", 10.415, 29.822))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_MANDE") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_MANDE", 10.414, 29.821))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_GHANA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_GHANA", 10.413, 29.820))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_CONGO") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_CONGO", 10.412, 29.819))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_TOGO") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_TOGO", 10.411, 29.818))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_KITARA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_KITARA", 10.410, 29.817))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_MAASAI") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_MAASAI", 10.409, 29.816))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_MUTAPA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_MUTAPA", 10.408, 29.815))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_SOUTH_AFRICA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_SOUTH_AFRICA", 10.407, 29.814))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_KHOISAN") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_KHOISAN", 10.406, 29.813))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_MALAGASY") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_MALAGASY", 10.405, 29.812))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_ZULU") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_ZULU", 10.404, 29.811))

	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_BABYLON") > 0):		coords.append(GeographicalCoordinate("CIVILIZATION_BABYLON", 32.536, 44.421))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_BYZANTIUM") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_BYZANTIUM", 41.009, 28.976))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_CARTHAGE") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_CARTHAGE", 36.887, 10.315))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_CELT") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_CELT", 46.923, 4.038))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_HITTITES") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_HITTITES", 41.016, 28.966))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_HOLY_ROMAN") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_HOLY_ROMAN", 50.775, 6.084))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_PERSIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_PERSIA", 29.934, 52.891))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_SUMERIA") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_SUMERIA", 31.322, 45.636))
	if (gc.getInfoTypeForStringWithHiddenAssert("CIVILIZATION_VIKING") > 0):
		coords.append(GeographicalCoordinate("CIVILIZATION_VIKING", 63.420, 10.393))
		
	CoordinatesDictionary = dict(zip([coord.getCivilization() for coord in coords], coords))


def assignCulturallyLinkedStarts(): 
	print "CultureLink option enabled."
	BugUtil.debug("Culturally Linked Starts Enabled")
	InitCoordinatesDictionary()
	pCultureLink = CultureLink()
	pCultureLink.assignStartingPlots()
	del pCultureLink
	return None


class CultureLink:

	pStartingPlotsList = []
	pRWCoordinatesList = []

	def __init__(self):
		CultureLink.pStartingPlotsList = [] 
		self.__initStartingPlotsList()
		CultureLink.pRWCoordinatesList = [] 
		self.__initRWCoordinatesList()

	def __initStartingPlotsList(self):
		iNumPlayers = game.countCivPlayersEverAlive()
		for iPlayer in range(iNumPlayers):
			pPlayer = gc.getPlayer(iPlayer)
			pStartingPlot = pPlayer.getStartingPlot()
			CultureLink.pStartingPlotsList.append(pStartingPlot)

	def __initRWCoordinatesList(self):
		iNumPlayers = game.countCivPlayersEverAlive()
		for iPlayer in range(iNumPlayers):
			pPlayer = gc.getPlayer(iPlayer)
			eCivilization = pPlayer.getCivilizationType()
			pCoordinate = CoordinatesDictionary[eCivilization]
			CultureLink.pRWCoordinatesList.append(pCoordinate)

	def assignStartingPlots(self):
		
		iNumPlayers = game.countCivPlayersEverAlive()
		iPlayersList = range(iNumPlayers)
		
		iSPDistanceMatrix = ScaleMatrix(self.__computeSPDistanceMatrix())
		print FormatMatrix(iSPDistanceMatrix, "Starting Plots Distance Matrix:")
		iRWDistanceMatrix = ScaleMatrix(self.__computeRWDistanceMatrix())
		print FormatMatrix(iRWDistanceMatrix, "Real World Distance Matrix:")
		
		def runBruteForceSearch(permutation = [], depth = 0, best = (None, 'inf')):
			if depth < iNumPlayers:
				for i in iPlayersList:
					if not i in permutation: 
						permutation.append(i)
						best = runBruteForceSearch(permutation, depth + 1, best)
						permutation.pop()
			else:
				error = evaluatePermutation(permutation)
				if error < best[1]:
					best = (copy(permutation), error)
					print "%s %.4f" % (best[0], best[1])
			return best
		
		def runAntColonyOptimization():
			# constants
			# NUM_RUNS = 50
			# NUM_ANTS = 250
			# NUM_BEST_ANTS = 5
			# PHEROMON_UPDATE = 0.025
			NUM_ANTS = iNumPlayers
			NUM_BEST_ANTS = int(iNumPlayers / 10)
			NUM_RUNS = iNumPlayers * 25
			PHEROMON_UPDATE = 0.34 / iNumPlayers
			# the best ant (permutation, error) we know
			TheBestAnt = (None, 'inf')
			# uniformly distributed pheromon at the beginning
			fPheromonMatrix = SquareMatrix(iNumPlayers, 1 / iNumPlayers)
			# the actual ACO:
			for iRun in xrange(NUM_RUNS):
				ants = {}
				# get some random ants:
				for i in xrange(NUM_ANTS):
					permutation = randomList(iPlayersList, fPheromonMatrix)
					error = evaluatePermutation(permutation)
					ants[error] = permutation
				bestants = [] 
				# get the x best ants (smallest error):
				for error in sorted(ants)[:NUM_BEST_ANTS]:
					ant = (ants[error], error)
					bestants.append(ant)
				# check if we have a new TheBestAnt:
				if bestants[0][1] < TheBestAnt[1]:
					TheBestAnt = bestants[0]
					print "%s %.8f (%d)" % (TheBestAnt[0], TheBestAnt[1], iRun)
				# let the x best ants update the pheromon matrix:
				for i, pos in enumerate(fPheromonMatrix):
						for ant in bestants:
							value = ant[0][i]
							fPheromonMatrix[i][value] = pos[value] + PHEROMON_UPDATE
						# total probability for each pos has to be one:
						fPheromonMatrix[i] = ScaleList(fPheromonMatrix[i])
			return TheBestAnt
		
		def evaluatePermutation(permutation):
			fPermRWMatrix = []
			for i in permutation:
				row = [iRWDistanceMatrix[i][j] for j in permutation]
				fPermRWMatrix.append(row)
			fError = 0.0
			for i in iPlayersList:
				for j in iPlayersList:
					if i > j:
						# square to make it more robust against small errors
						fError += abs(iSPDistanceMatrix[i][j] - fPermRWMatrix[i][j]) ** 1.3
			return fError
		
		if iNumPlayers <= 9: # brute force 
			iBestPermutation = runBruteForceSearch()[0]
		else: # ants where brute force is impossible
			iBestPermutation = runAntColonyOptimization()[0]
			
		# assign the best found permutation:
		pStartingPlots = CultureLink.pStartingPlotsList
		for iPlayer, iStartingPlot in zip(iBestPermutation, range(len(pStartingPlots))):
			gc.getPlayer(iPlayer).setStartingPlot(pStartingPlots[iStartingPlot], True)

	def __computeRWDistanceMatrix(self):
		pCoordinates = CultureLink.pRWCoordinatesList
		fRWDistanceMatrix = SquareMatrix(len(pCoordinates), 0.0)
		for i, pCoordinateA in enumerate(pCoordinates):
			for j, pCoordinateB in enumerate(pCoordinates):
				if j > i:
					fRWDistance = RealWorldDistance(pCoordinateA, pCoordinateB) / DMAX_EARTH
					fRWDistanceMatrix[i][j] = fRWDistance
					fRWDistanceMatrix[j][i] = fRWDistance
		return fRWDistanceMatrix

	def __computeSPDistanceMatrix(self):
		fSPDistanceMatrix = SquareMatrix(len(CultureLink.pStartingPlotsList), 0.0)
		# fill the starting plot distance matrix with normalized step distances:
		for i, pStartingPlotA in enumerate(CultureLink.pStartingPlotsList):
			for j, pStartingPlotB in enumerate(CultureLink.pStartingPlotsList):
				if i > j:
					fSPDistance = StepDistance(pStartingPlotA, pStartingPlotB) / MaxPossibleStepDistance()
					if pStartingPlotA.getArea() != pStartingPlotB.getArea():
						#print "Area A: %s, Area B: %s" % (pStartingPlotA.getArea(), pStartingPlotB.getArea())
						fSPDistance = fSPDistance * 2
					fSPDistanceMatrix[i][j] = fSPDistance
					fSPDistanceMatrix[j][i] = fSPDistance
		return fSPDistanceMatrix


class GeographicalCoordinate:

	def __init__(self, sCivilizationType, dLatitude, dLongitude):
		self.civ = GetInfoType(sCivilizationType)
		self.lat = dLatitude
		self.lon = dLongitude

	def __str__(self):
		args = (self.getCityName(), self.lat, self.lon)
		return "%s:\t%8.4f   %8.4f" % args

	def getCivilization(self):
		return self.civ

	def getCityName(self):
		pCivilization = gc.getCivilizationInfo(self.civ)
		if pCivilization.getNumCityNames() > 0:
			return pCivilization.getCityNames(0)
		return "unknown city name"

	def getLatitude(self):
		return self.lat

	def getLongitude(self):
		return self.lon


## GLOBAL HELPER FUNCTIONS:

def GetInfoType(sInfoType, bIgnoreTypos = False):
	iInfoType = gc.getInfoTypeForString(sInfoType)
	if iInfoType == -1 and not bIgnoreTypos:
		arg = ("InfoType %s unknown! Probably just a Typing Error." % sInfoType)
		raise ValueError, arg
	return iInfoType

def RealWorldDistance(pCoordA, pCoordB):
	# equator radius and earth flattening (WGS-84)
	SEMI_MAJOR_AXIS = 6378.137
	INVERSE_FLATTENING = 1/298.257223563
	# some variables to reduce redundancy
	F = pi * (pCoordA.getLatitude() + pCoordB.getLatitude()) / 360
	G = pi * (pCoordA.getLatitude() - pCoordB.getLatitude()) / 360
	l = pi * (pCoordA.getLongitude() - pCoordB.getLongitude()) / 360
	# calculate the rough distance
	S = sin(G)**2 * cos(l)**2 + cos(F)**2 * sin(l)**2
	C = cos(G)**2 * cos(l)**2 + sin(F)**2 * sin(l)**2
	w = atan(sqrt(S/C))
	D = 2 * w * SEMI_MAJOR_AXIS
	# flattening correction
	R = sqrt(S * C) / w	
	H1 = INVERSE_FLATTENING * (3 * R - 1) / (2 * C)
	H2 = INVERSE_FLATTENING * (3 * R + 1) / (2 * S)
	return D * (1 + H1 * sin(F)**2 * cos(G)**2 - H2 * cos(F)**2 * sin(G)**2)

def StepDistance(pPlotA, pPlotB): 
	return stepDistance(pPlotA.getX(), pPlotA.getY(), pPlotB.getX(), pPlotB.getY())

def MaxPossibleStepDistance():
	if CyMap().getGridWidth() > CyMap().getGridHeight():
		if CyMap().isWrapX(): 
			return (CyMap().getGridWidth() + 1) // 2
		return CyMap().getGridWidth()
	if CyMap().isWrapY(): 
		return (CyMap().getGridHeigth() + 1) // 2
	return CyMap().getGridHeight()

def ScaleList(xList):
	fSum = sum(xList)
	return [xElement / fSum for xElement in xList]

def SquareMatrix(iSize, xInitWith = None):
	return [[xInitWith] * iSize for i in range(iSize)]

def ScaleMatrix(matrix, absmin = None, absmax = None):
	minValue = absmin 
	maxValue = absmax
	if minValue == None:
		minValue = min([min(row) for row in matrix])
	if maxValue == None:
		maxValue = max([max(row) for row in matrix])
	if minValue == maxValue:
		return "geht nicht"
	return [map(lambda x: (x - minValue) / (maxValue - minValue), row) for row in matrix]

def FormatMatrix(matrix, description = None, rownames = None):
	if len(matrix) > 0:
		s = ""
		if description != None:
			s += description + 2 * "\n"
		s += "["
		for r in xrange(len(matrix)):
			if r > 0:
				s += " "
			s += "["
			for c in xrange(len(matrix[0])):
				if matrix[r][c] != None:
					s += "%8.4f" % matrix[r][c]
				else:
					s += "%8s" % "None"
				if c != len(matrix[0]) - 1:
					s+= ","
			if r == len(matrix) - 1:
				s += "]]"
			else:
				s += "],"
			if rownames != None:
				s += 3 * " " + rownames[r]
			s += "\n"
		return s
	return("Error while creating formated matrix string.")


def shuffle(xList):
	xShuffledList = []
	xListCopy = copy(xList)
	for x in range(len(xList)):
		r = dice.get(len(xListCopy), "CultureLink: shuffle")
		xShuffledList.append(xListCopy.pop(r))
	return  xShuffledList

'''
RANDOMLIST:
Returns a permutation of the items in xList. The probability for each element to appear at position x of
the permutation is defined by row x of the probabilities matrix. Element y in row x is the probability 
for element y in xList to appear at position x in the permutation. 
In other words: randmomList is a more general form of shuffling with non-uniform probabilities.
'''

def randomList(xList, fProbabilitiesMatrix):
		# make a copy so we can change the values
		fPrMxCopy = deepcopy(fProbabilitiesMatrix)
		iNumElements = len(fPrMxCopy)
		xRandomList = [None] * iNumElements
		# fill xRandomList in random order to prevent bias
		xShuffledList = shuffle(range(iNumElements))   
		for i in xShuffledList:
				for j in xRandomList:
						if j != None:
								fPrMxCopy[i][j] = 0.0
				fPrMxCopy[i] = ScaleList(fPrMxCopy[i])
				# get a random element from xList not in xRandomList
				xRandomList[i] = randomListElement(xList, fPrMxCopy[i])
		return xRandomList

def randomListElement(xList, fProbabilitiesList):
		maxsoren = 2 ** 16 - 1
		#for i in range(32):
		#	print i
		#	dice.get(2**i, "test")
		
		r = dice.get(maxsoren, "CultureLink: randomListElement") / maxsoren
		fCumulatedProbabilities = 0.0
		for i, fProbability in enumerate(fProbabilitiesList):
				fCumulatedProbabilities += fProbability
				if r < fCumulatedProbabilities:
						return xList[i]
		return xList[-1:][0]
