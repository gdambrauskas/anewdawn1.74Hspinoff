##-------------------------------------------------------------------
## Modified from Abandon Raze City by unknown, Bhruic, Nemo, OrionVetran
##  by Dancing Hoskuld with a great deal of help from EmperorFool and Zappara
##  to work in, with and under BUG
##-------------------------------------------------------------------
##
## Things that could be done to improve this mod making it interact with the main mod in a more generic fashon.
## 1. replace the English language stuff with the correct language translations in the text file.
##-------------------------------------------------------------------
##
## To get this working in any mod you need to
## 1. put 	<load mod="Abandon City Mod"/>	in the init.xml in the Python/Config directory
##
## Modders:- If you want any special events when a city is abandoned add them to your OnCityRazed event.
##		This mod calls that event.
##  In RoM Holy Shrines are not world wonders a fix to reflect this is in the code it should have no effect
##    on other mods but the test can be commented out if needs be.
##
##-------------------------------------------------------------------
##
## Changes
##  April 2009
##	A moders section of this code now allows a single place for the setting of the "parameters"
##	 - produce just workers or settlers and workers
##	 - produce 1 worker for every X popuation in the city
##	 - produce 1 settler fo every X population in the city with 1 worker for every Y in the remainder
##   Sell buildings rather than demolish them.  Thanks Zappara
##   Unhappiness from selling religious buildings +1 unhappy for x turns.  Where x depends on game speed.
##   Can not abandon last city.
##   Can not sell free buildings however they need to appear in the list for this program to work.
##
##   Note. If you only have one city and it has no buildings then Ctrl-A will appear not to work.  This was
##   done to to stop a CDT which could be caused otherwise.  Besides there is nothing to sell and you can't
##   abandon your only city.
##   - I had made a typo in the check for this - it is now fixed - thanks Zappara
##
## Late April 2009
##  Civ specific unique units for workers and settlers are now produced when appropriate. - thanks Zappara
##  Percentage of building production cost is now in the parameter section so modders can change it
##    currently set at the Rise of mankind default of 20%.  Popup header text now reflects this percentage.
##  More of the text supports language even if I have not put the language stuff there yet.
##  Fixed to support modular buildings. - thanks Zappara
##
## September 2009
## 	Minor coding changes to make the code cleaner.
##		Includes removing excess coments, define globals once, and put parameter for obsolete reduction.
##	Stopped putting free buildings in the list.  
##		The way the popup works is it puts the items in the list in the order supplied (text and index).
##		It displays (text) and returns (index) for the selected item.
##
##-------------------------------------------------------------------
##
## Potential changes
## 1. has an .ini file (AbandonCity.INI) to hold parameters
##	 - mod is active/inactive
##     - only abandon city (no selling of buildings)
##
## 2. buildings in albhabetical order in the list.
##	
##
## ------------------------------------------
## Decided not to generate upgraded settler units as these could then be settled and the free buildings sold off.
##	- obsolete decision now that selling free buildings gives no money.
##	- further obsolete since now we do not present free buildings to be sold :)
##
## Decided to have the population parameters in the code for modders rather than letting the player change them ad hoc.
##
## Decided that you can sell obsolete buildings.  Just because they are obsolete does not mean that they are not having an effect.



from CvPythonExtensions import *
import CvUtil
import CvEventInterface
import PyHelpers
import Popup as PyPopup
import BugCore
import BugUtil
import SdToolKit
import autolog
import string

# BUG - Mac Support - start
BugUtil.fixSets(globals())
# BUG - Mac Support - end

# globals

SD_MOD_ID = "AbandonCity"

ABANDON_CITY_DEMOLISH_BUILDING_EVENT_ID = CvUtil.getNewEventID("AbandonCity.Active")

gc = CyGlobalContext()
localText = CyTranslator()
PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo

#AbandonCityOpt = BugCore.game.AbandonCity
g_modEventManager = None
g_eventMgr = None
g_autolog = None

# parameters for modders.
bSettlersAndWorkers = True 	# "Treu means settlers and workers are produced.  False to just produce workers.
iPopulationForSettlers = 10 	# If producing settlers then produce 1 per each full city population of iPopulationForSettlers
iPopulationForWorkers = 3 	# produce 1 per each full city population of iPopulationForWorkers. 
					# Note: this is left over population after producing settlers if any.
g_CostFraction = 0.2 	# Percentage of building cost converted to money. 20% is the default from Rise of mankind
g_ReductionForObsolete = 2	# Amount the gold being returned is divided by if the building is obsolete.

g_ConstructModifier = 1	# used to scale gold for gamespeed
g_iAbandonTrigger = gc.getNumBuildingClassInfos()


def onStartAbandonCity(argsList):
	g_modEventManager.onStartAbandonCity(argsList)

class AbandonCityEventManager:
	def __init__(self, eventManager):
		eventManager.addEventHandler("StartAbandonCity", self.onStartAbandonCity)
		eventManager.addEventHandler("ModNetMessage", self.onModNetMessage)

		global g_modEventManager
		g_modEventManager = self

		global g_autolog
		g_autolog = autolog.autologInstance()
		
		global g_eventMgr
		g_eventMgr = eventManager
		self.eventManager = eventManager
		

		eventManager.setPopupHandler(ABANDON_CITY_DEMOLISH_BUILDING_EVENT_ID, ("AbandonCity.Active", self.__eventAbandonCityDestroyBuildingApply, self.__eventAbandonCityDestroyBuildingBegin))

	def onModNetMessage(self, argsList):
		protocol, data1, data2, data3, data4 = argsList
		if (protocol == 900):
			pPlayer = gc.getPlayer(data4)
			pCity = pPlayer.getCity(data1)
			iX = pCity.getX()
			iY = pCity.getY()
			pPlayer.initUnit(data2, iX, iY, UnitAITypes.NO_UNITAI, DirectionTypes.NO_DIRECTION)
		elif (protocol == 901):
			pPlayer = gc.getPlayer(data4)
			pCity = pPlayer.getCity(data1)
			pCity.kill()
		elif (protocol == 902):
			pPlayer = gc.getPlayer(data4)
			pCity = pPlayer.getCity(data1)
			pCity.setNumRealBuilding(data2, 0)
			pPlayer.changeGold(data3)
			CyInterface().setDirty(InterfaceDirtyBits.CityScreen_DIRTY_BIT, True)
			CyInterface().setDirty(InterfaceDirtyBits.SelectionButtons_DIRTY_BIT, True)
			if (gc.getBuildingInfo(data2).getReligionType() >= 0):
				pCity.changeHurryAngerTimer(pCity.flatHurryAngerLength())
				if (not gc.getGame().isOption(GameOptionTypes.GAMEOPTION_NO_REVOLUTION)):
					pCity.changeRevolutionIndex(100)

	def onStartAbandonCity(self, argsList):
		# Have keypress from handler - return 1 if the event was consumed
		# Get the player details and game options.
		
		#CyInterface().addImmediateMessage("Abandon City Demolish Building - __StartAbandonCity called", None)

		game = gc.getGame()
		
		if ( CyInterface().isCityScreenUp() ):
			if (CyInterface().getHeadSelectedCity().getOwner() == gc.getGame().getActivePlayer()):
				pHeadSelectedCity = CyInterface().getHeadSelectedCity()
				pPlayer = gc.getPlayer(pHeadSelectedCity.getOwner())
				## if player only has one city and that city has no buildings then do not display popup as it can cause a CDT
				# gdam start
				# Since I turned off city abandonment, now should never display popup if city has no buildings to sell. Empty popup clicked->CTD.
				if (pHeadSelectedCity.getNumBuildings() == 0):
					return 0
				# gdam end
				if (pHeadSelectedCity.getNumBuildings() == 0) and (pPlayer.getNumCities() == 1) :
					return 0
				g_eventMgr.beginEvent(ABANDON_CITY_DEMOLISH_BUILDING_EVENT_ID)
				return 1
		return 0

	def __eventAbandonCityDestroyBuildingBegin(self, argsList):
		# Get player and city details. Set up headings etc.
		# Display appropriate dialogue.
		
		#CyInterface().addImmediateMessage("Abandon City Demolish Building - __eventAbandonCityDestroyBuildingBegin called", None)
		pHeadSelectedCity = CyInterface().getHeadSelectedCity()
		pPlayer = gc.getPlayer(pHeadSelectedCity.getOwner())
		civ = gc.getCivilizationInfo(pPlayer.getCivilizationType())
		
		szheader = BugUtil.getPlainText("TXT_KEY_ABANDON_CITY_HEADER1")
		ipercentageForHeader = g_CostFraction * 100
		szheader += " " + u"%d" %(ipercentageForHeader) + BugUtil.getPlainText("TXT_KEY_ABANDON_CITY_HEADER2")

		abandoncity = BugUtil.getPlainText("TXT_KEY_ABANDON_CITY")
		ok = BugUtil.getPlainText("TXT_KEY_MAIN_MENU_OK")
		cancel = BugUtil.getPlainText("TXT_KEY_POPUP_CANCEL")
		popup = PyPopup.PyPopup(ABANDON_CITY_DEMOLISH_BUILDING_EVENT_ID, EventContextTypes.EVENTCONTEXT_ALL)
		popup.setHeaderString(szheader)
		popup.createPullDown()

		# find out what the gamespeed building cost modifier is
		# and adjust construct modifier also with building production percent game define
		iGameSpeed = gc.getGame().getGameSpeedType()
		iBuildProdPercent = gc.getDefineINT("BUILDING_PRODUCTION_PERCENT")/100
		g_ConstructModifier = iBuildProdPercent*gc.getGameSpeedInfo(iGameSpeed).getConstructPercent()/100
		
		# Populate list box with non-free buildings
		for i in range(0,gc.getNumBuildingClassInfos()):
			iType = civ.getCivilizationBuildings(i)
			#BugUtil.debug("for loop num %d, building %s", i, gc.getBuildingInfo(i).getDescription())
			
			# RoM 2.7 added check for Holy Shrines so that player can't demolish them, those aren't Great Wonders in RoM
			if (iType > 0 and pHeadSelectedCity.getNumRealBuilding(iType) > 0 and not isLimitedWonderClass(gc.getBuildingInfo(iType).getBuildingClassType()) and not gc.getBuildingInfo(iType).getGlobalReligionCommerce() > 0):
				# Only put non-free buildings in the list
				if (not pPlayer.isBuildingFree(iType)):
					# Figure out the gold to be paid for the building.
					# Note that this code is repeated when the building is actually sold.
					iGoldBack = int(gc.getBuildingInfo(iType).getProductionCost() * g_ConstructModifier * g_CostFraction)

					# If building is obsolete, give only half sum back
					obsoleteTech = gc.getBuildingInfo(iType).getObsoleteTech()
					if ( gc.getTeam(pPlayer.getTeam()).isHasTech(obsoleteTech) == true ):
						iGoldBack = int(iGoldBack/g_ReductionForObsolete)

					# Build up text to display in the list box
					szBuilding = gc.getBuildingInfo(iType).getDescription()
					szBuilding = gc.getBuildingInfo(iType).getDescription()
					szBuilding += " (" + u"%d " %(iGoldBack) + localText.getText("TXT_KEY_COMMERCE_GOLD", ())
					# Add unhappiness text here
					if gc.getBuildingInfo(iType).getReligionType() >= 0 : # religious building
						szBuilding += ", " + localText.getText("TXT_KEY_ABANDON_UNHAPPINESS", ()) #+  u"%c" % CyGame().getSymbolID(FontSymbols.UNHAPPY_CHAR)
					szBuilding += ")"

					popup.addPullDownString(szBuilding, iType)

		# Only allow abandonment of city if there is more than one city
	    # gdam start
	    # Do not allow abandonment of the city. Maybe allow it later with a code check that there's no enemy within certain radius of the city.
	    # Player would always choose to abandon the city, because that prevents tech from being captured, money obtained etc Cheesy tactics.
		#if pPlayer.getNumCities() > 1 :
		#	popup.addPullDownString(abandoncity, g_iAbandonTrigger)
		# gdam end
		popup.addButton(ok)
		popup.addButton(cancel)
		popup.launch(False, PopupStates.POPUPSTATE_IMMEDIATE)
		return

	def __eventAbandonCityDestroyBuildingApply(self, playerID, userData, popupReturn):
		#CyInterface().addImmediateMessage("Abandon City Demolish Building - __eventAbandonCityDestroyBuildingApply called", None)
		pPlayer = gc.getPlayer(playerID)
		pHeadSelectedCity = CyInterface().getHeadSelectedCity()
		civ = gc.getCivilizationInfo(pHeadSelectedCity.getCivilizationType())
		
		if (popupReturn.getButtonClicked() == 0):
			if (popupReturn.getSelectedPullDownValue(0) == g_iAbandonTrigger ):
				ix = pHeadSelectedCity.getX()
				iy = pHeadSelectedCity.getY()
				iPopulation = CyInterface().getHeadSelectedCity().getPopulation()

				# Generate the units
				iWorker = gc.getInfoTypeForString("UNIT_WORKER")
				iSettler = gc.getInfoTypeForString("UNIT_SETTLER")

				iUniqueWorkerUnit = civ.getCivilizationUnits(iWorker)
				iUniqueSettlerUnit = civ.getCivilizationUnits(iSettler)

				# if settlers and workers
				if bSettlersAndWorkers:
					isettlers = iPopulation/iPopulationForSettlers
					iworkers = (iPopulation - iPopulationForSettlers * isettlers)/iPopulationForWorkers
				else:
					isettlers = 0
					iworkers = iPopulation/iPopulationForWorkers
					
				for i in range(0,iworkers):
					#pPlayer.initUnit(iUniqueWorkerUnit, ix, iy, UnitAITypes.NO_UNITAI, DirectionTypes.NO_DIRECTION)
					CyMessageControl().sendModNetMessage(900, pHeadSelectedCity.getID(), iUniqueWorkerUnit, 0, playerID)
				for i in range(0,isettlers):
					#pPlayer.initUnit(iUniqueSettlerUnit, ix, iy, UnitAITypes.NO_UNITAI, DirectionTypes.NO_DIRECTION)
					CyMessageControl().sendModNetMessage(900, pHeadSelectedCity.getID(), iUniqueSettlerUnit, 0, playerID)
				
				# Abandon the City
				CyMessageControl().sendModNetMessage(901, pHeadSelectedCity.getID(), 0, 0, playerID)
				#pHeadSelectedCity.kill()
				CyAudioGame().Play2DSound("AS2D_DISCOVERBONUS")

			else:
				iBuildingType = popupReturn.getSelectedPullDownValue(0)
				iType = civ.getCivilizationBuildings(gc.getBuildingInfo(iBuildingType).getBuildingClassType())

				iGoldBack = int(gc.getBuildingInfo(iType).getProductionCost() * g_ConstructModifier * g_CostFraction)

				obsoleteTech = gc.getBuildingInfo(iType).getObsoleteTech()
				if ( gc.getTeam(pPlayer.getTeam()).isHasTech(obsoleteTech) == True):
					iGoldBack = int(iGoldBack/g_ReductionForObsolete)

				#pPlayer.changeGold(iGoldBack)

				# unhappiness penalty when demolishing any religion buildings
				#if gc.getBuildingInfo(iType).getReligionType() >= 0 : # religious building
				#	pHeadSelectedCity.changeHurryAngerTimer(pHeadSelectedCity.flatHurryAngerLength())
				#	if (not gc.getGame().isOption(GameOptionTypes.GAMEOPTION_NO_REVOLUTION)):
				#		pHeadSelectedCity.changeRevolutionIndex(100)
						
				#pHeadSelectedCity.setNumRealBuilding(iType, 0)
				CyMessageControl().sendModNetMessage(902, pHeadSelectedCity.getID(), iType, iGoldBack, playerID)
				CyAudioGame().Play2DSound("AS2D_DISCOVERBONUS")
				#CyInterface().setDirty(InterfaceDirtyBits.CityScreen_DIRTY_BIT, True)
				#CyInterface().setDirty(InterfaceDirtyBits.SelectionButtons_DIRTY_BIT, True)
