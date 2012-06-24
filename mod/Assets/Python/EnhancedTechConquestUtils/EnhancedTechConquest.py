## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
##
## TechConquest by Bhruic
## Updated by TheLopez
## Updated by Dancing Hoskuld to allow language support 
##	see also EnhancedTechConquest game text xml file.
##

from CvPythonExtensions import *
import CvUtil
import PyHelpers
import Popup as PyPopup
import CvConfigParser

# globals
gc = CyGlobalContext()
localText = CyTranslator()

# Change the value to true if technology should be handed completely over from
# the conquered city to their new owners.
# Default value is False
g_bCompleteTechnologyDiscovery = False

# Increase or decrease the value to change the number of or part of 
# technologies conquered cities will hand over to their new owners.
# Default value is 1
g_iTechnologyTransferCount = 1

# Change the value to true if the amount of technologies conquered cities will
# hand over to their new owners should be random.
# Default value is False
g_bRandomTechnologyTransferAmount = False

# Change the value to true if the conquering civilization can receive 
# technology without the appropriate prerequisites or ignore their civilization
# technology restrictions.
# Default value is False
g_bTechnologyTransferIgnorePrereq = False

# Change the value to False if full technology transfer should be allowed. By 
# setting the value to true this will force players to spend at least one turn
# researching pillaged technology.
# Default value is true
g_bDisableFullTechnologyTransfer = true

# Increase or decrease the value to change the base technology transfer 
# percentage amount.
# Default value is 25
g_iBaseTechnologyTransferPercent = 25

# Increase or decrease the value to change the percent amount per city 
# population that will be used to transfer technology to the new owners of
# the conquered city.
# Default value is 5
g_iPercentagePerCityPopulation = 5


def loadConfigurationData():
	global g_bCompleteTechnologyDiscovery
	global g_iTechnologyTransferCount
	global g_bRandomTechnologyTransferAmount
	global g_bTechnologyTransferIgnorePrereq
	global g_bDisableFullTechnologyTransfer
	global g_iBaseTechnologyTransferPercent
	global g_iPercentagePerCityPopulation 
	
	config = CvConfigParser.CvConfigParser("Rise of Mankind Config.ini")

	if(config != None):
		g_bCompleteTechnologyDiscovery = config.getboolean("Enhanced Tech Conquest", "Complete Technology Discovery", False)
		g_iTechnologyTransferCount = config.getint("Enhanced Tech Conquest", "Technology Transfer Count", 1)
		g_bRandomTechnologyTransferAmount = config.getboolean("Enhanced Tech Conquest", "Random Technology Transfer Amount", False)
		g_bTechnologyTransferIgnorePrereq = config.getboolean("Enhanced Tech Conquest", "Technology Transfer Ignore Prereq", False)
		g_bDisableFullTechnologyTransfer = config.getboolean("Enhanced Tech Conquest", "Disable Full Technology Transfer", True)
		g_iBaseTechnologyTransferPercent = config.getint("Enhanced Tech Conquest", "Base Technology Transfer Percent", 25)
		g_iPercentagePerCityPopulation = config.getint("Enhanced Tech Conquest", "Percentage Per City Population", 5)

	
class EnhancedTechConquest:

	def onCityAcquired(self, argsList):
		iPreviousOwner, iNewOwner, pCity, bConquest, bTrade = argsList
		# debug("onCityAcquired")

		# Return immediately if the city was not conquered		
		if (not bConquest):
			return None
		# debug("isConquest")
			
		# Get the map random object
		pMapRand = gc.getGame().getMapRand()

		# Get the conquering player
		pPlayer = gc.getPlayer(iNewOwner)
		
		# Get the conquerer's team
		pNewTeam = gc.getTeam(pPlayer.getTeam())

		# Get the old city owner
		pOldCityOwner = gc.getPlayer(iPreviousOwner)

		lDiscoveredTechs = []
		lDiscoveredTechStrs = ""
		iTotalTechPoints = 0
		
		# Go through and try to get as many technologies as allowed
		iCountTechs = 0
		for i in range(gc.getNumTechInfos()):

			# If we have enough techs then break
			if (iCountTechs >= g_iTechnologyTransferCount):
				break

			# Get the possible technologies that could be transfered
			lPossibleTechnology = self.getPossibleTechnologyList(pOldCityOwner, pCity, lDiscoveredTechs)
			# debug("lPossibleTechnology", lPossibleTechnology)
			
			# Break if there are no possible technologies that can be transfered
			if (len(lPossibleTechnology) < 1):
				break

			# Pick a technology randomly from the list
			iRandTechPos = pMapRand.get(len(lPossibleTechnology), "TechConquest")
			# debug("iRandTechPos", iRandTechPos)

			# Get the tech type from the list
			iRandTech = lPossibleTechnology[iRandTechPos]
			# debug("iRandTech", iRandTech)

			# Get the total number of technology points that will be transfered 
			# to the new city owner
			iTotalTechPoints = self.getTechnologyTransferAmount(iRandTech, pNewTeam, pCity)
                        # debug("iTotalTechPoints", iTotalTechPoints)

                        # Don't need to do anything if no points are to be transfered
			if (iTotalTechPoints < 1):
                                continue

			iCountTechs += 1

			lDiscoveredTechs.append(iRandTech)

			# Increase the research progress for the new city owner
			pNewTeam.changeResearchProgress(iRandTech, iTotalTechPoints, iNewOwner)

			if (iCountTechs == 1):
				#lDiscoveredTechStrs += gc.getTechInfo(iRandTech).getDescription()
				lDiscoveredTechStrs += "\n\t\t" + gc.getTechInfo(iRandTech).getDescription()
			else:
				#lDiscoveredTechStrs += ", " + gc.getTechInfo(iRandTech).getDescription()
				lDiscoveredTechStrs += "\n\t\t" + gc.getTechInfo(iRandTech).getDescription()

		strMessage = ""
		# Inform the player they didn't get any new technologies
		if (iCountTechs < 1):
			strMessage = localText.getText("TXT_KEY_ENHANCED_TECH_CONQUEST_FAIL", ()) + " %s" %(pCity.getName())
		# Inform the player they got some new technology points
		else:
			strMessage = localText.getText("TXT_KEY_ENHANCED_TECH_CONQUEST_SUCESS", ()) %(pCity.getName(), lDiscoveredTechStrs)
		
		CyInterface().addMessage(pPlayer.getID(), True, 20, strMessage, "", 0, gc.getCivilizationInfo(pOldCityOwner.getCivilizationType()).getButton(), ColorTypes(0), pCity.getX(), pCity.getY(), True, True) 


	# Returns the amount of technology points of that should be transfered from 
	# the old owners of the city to the new owners of the city.
	def getTechnologyTransferAmount(self, iTechType, pNewTeam, pCity):

		# Return zero immediately if the tech info passed in is invalid
		if (iTechType < 0):
			return 0

		# Return zero immediately if the new team passed in is invalid
		elif (pNewTeam == None) or (pNewTeam.isNone()):
			return 0

		# Return zero immediately if the city passed in is invalid
		elif (pCity == None) or (pCity.isNone()) or (pCity.isBarbarian()):
			return 0
						
		pMapRand = gc.getGame().getMapRand()
		iTechCost = pNewTeam.getResearchCost(iTechType)
		iCurrentTechPoints = pNewTeam.getResearchProgress(iTechType)
		iTempCost = (iTechCost - iCurrentTechPoints)

		# Return the full tech cost if the mod has been configured to do so
		if (g_bCompleteTechnologyDiscovery):
			if(g_bDisableFullTechnologyTransfer) and (iTempCost > 1):
				return iTempCost - 1
			return iTempCost


		# Get the percentage amount of tech points that is given by the population
		iTempPercent = pCity.getPopulation() * g_iPercentagePerCityPopulation
		iPercent = min(max(0, iTempPercent), 100)

		iExtraTechPoints = 0

		# Get the base percentage amount of tech points
		iBaseTechPoints = int(iTechCost * (g_iBaseTechnologyTransferPercent/100.0))

		if (g_bRandomTechnologyTransferAmount):
			iExtraTechPoints = pMapRand.get(iPercent, "TechConquest")
		else :
			iExtraTechPoints = int(iTechCost * (iPercent/100.0))

		# Get the total amount of tech points to be transfered
		iTotalTechPoints = iBaseTechPoints + iExtraTechPoints

		if (iTotalTechPoints >= iTempCost):
			if (g_bDisableFullTechnologyTransfer) and (iTempCost > 1):
				return iTempCost - 1
			return iTempCost

		return iTotalTechPoints
		
		
	# Returns the list of technologies the conquering player could get from 
	# their newly conquered city.
	def getPossibleTechnologyList(self, pOldCityOwner, pCity, lDiscoveredTechs):

		lPossibleTechnology = []

		# Return an empty list if the old city owner passed in is invalid
		if (pOldCityOwner == None) or (pOldCityOwner.isNone()):
			return []

		# Return an empty list if the city passed in is invalid
		elif (pCity == None) or (pCity.isNone()) or (pCity.isBarbarian()):
			return []
		
		# Get the team from the old owner of the city
		pOldCityOwnerTeam = gc.getTeam(pOldCityOwner.getTeam())

		# Get the conquering player of the city
		pConquerer = gc.getPlayer(pCity.getOwner())

		# Get the conquering team of the city
		pConquererTeam = gc.getTeam(pConquerer.getTeam())
		
		# Go through each technology in the game
		for iTechType in range(gc.getNumTechInfos()):
			
			# CvUtil.pyPrint("%s %s %s" %(gc.getTechInfo(iTechType).getType(), pConquerer.canResearch(iTechType, False), g_bTechnologyTransferIgnorePrereq) ) 
			# Continue if the conquerer cannot research the technology
			if (not pConquerer.canResearch(iTechType, False) and (not g_bTechnologyTransferIgnorePrereq)):
				continue

			# Continue if the old team doesn't have the tech
			elif (not pOldCityOwnerTeam.isHasTech(iTechType)):
				continue

			# Continue if the conquering team does have the tech
			elif (pConquererTeam.isHasTech(iTechType)):
				continue

			# Continue if the tech is in the discovered tech list
			elif (iTechType in lDiscoveredTechs):
				continue

			# Append the technology to the possible technology list
			elif(g_bDisableFullTechnologyTransfer):
				iConquererProgress = pConquererTeam.getResearchProgress(iTechType)
				if (iConquererProgress >= (pConquererTeam.getResearchCost(iTechType) - 1)):
					continue

			# Append the technology to the possible technology list
			lPossibleTechnology.append(iTechType)

		return lPossibleTechnology
