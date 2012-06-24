# BARBARIAN_CIV_MOD
#
# by jdog5000
# Version 3.0
#

from CvPythonExtensions import *
import CvUtil
import PyHelpers
import Popup as PyPopup
# --------- Revolution mod -------------
import RevDefs
import RevUtils
import SdToolKitCustom as SDTK
import RevInstances
import BugCore

# globals
gc = CyGlobalContext()
PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo
game = CyGame()
localText = CyTranslator()
RevOpt = BugCore.game.Revolution

class BarbarianCiv :

	def __init__(self, customEM, RevOpt ) :

		print "Initializing BarbarianCiv Mod"

		self.RevOpt = RevOpt
		self.customEM = customEM

		self.newPlayerIdx = None

		# popup alert when new civ is formed from barb cities (forced true if offering control)
		self.LOG_BARB_SETTLE = self.RevOpt.isAnnounceBarbSettle() or self.offerControl
		self.bBarbSettlePopup = self.RevOpt.isUsePopup() or self.offerControl
		self.bNotifyOnlyClosePlayers = self.RevOpt.isOnlyNotifyNearbyPlayers()
		self.blockPopupInAuto = self.RevOpt.isBlockPopupInAuto()
		# enable debug log and functions
		self.LOG_DEBUG  = self.RevOpt.BarbCivDebugMode()

		# New world policy controls handling of barbcivs starting away from the original civs in the game:
		#	0:  New World treated basically like old world, New World will be basically full of full civs by the time explorers arrive
		#	1:  Reduce settling, bonuses of New World barb civs but can form both minors and full.  Will probably be a mixture of states, with some free space when explorers arrive
		#	2:  Like 1 but capped at minor civs in New World until the civs start to make contact with the old world.  Odds of settling into full increase with number of contacts.  Will be all minors when the first caravels pass by, full civs emerge as time goes on.
		#	3:  Like 2 this is capped at minor civs in the New World but the chance of settling into a full civ doesn't start until an old world civ lands units in the New World.
		#	4:  No barb civs of any kind settle on empty island/continent until another civ lands an explorer
		#	5:  No barb civs of any kind until another civ founds a city on island/continent
		self.iNewWorldPolicy = self.RevOpt.getNewWorldPolicy()
		self.iMinorHalfLife = self.RevOpt.getMinorHalfLife()
		self.bFierceNatives = self.RevOpt.isFierceNatives()

		self.maxCivs = RevOpt.getBarbCivMaxCivs()
		if( self.maxCivs <= 0 ) :
			self.maxCivs = gc.getMAX_CIV_PLAYERS()

		# Minimum population needed for a group of barb cities to settle
		self.minPopulation = max([1, self.RevOpt.getMinPopulation()])

		# Minimum number of contacts (any kind and full civ) to settle
		self.minContacts = self.RevOpt.getMinContacts()
		self.minFullContacts = self.RevOpt.getMinFullContacts()

		# Control odds for forming minor civs
		self.formMinorMod  = self.RevOpt.getFormMinorModifier()

		# Affects number of units given to militaristic barbs
		self.militaryStrength = self.RevOpt.getMilitaryStrength()

		# If city was founded by other than barbs, bring back a similar style civ (if possible)
		self.barbCivsByStyle = self.RevOpt.isBarbCivsByStyle()

		# Max number of turns after a militaristic barb civ starts alone will they declare war on first contact
		self.militaryWindow = self.RevOpt.getMilitaryWindow()

		# How close other civ's capital needs to be for Barb civ to have a chance of immeadiately declaring war
		self.warCloseDist = self.RevOpt.getWarCloseDist()

		# Offer control of new barb civs
		self.offerControl = self.RevOpt.isOfferControl()
		self.bCancelAutoForOffer = self.RevOpt.isCancelAutoForOffer()


		self.customEM.addEventHandler( "BeginGameTurn", self.onBeginGameTurn )
		self.customEM.addEventHandler( "EndPlayerTurn", self.onEndPlayerTurn )
		self.customEM.addEventHandler( "firstContact", self.onFirstContact )
		self.customEM.addEventHandler( "setPlayerAlive", self.onSetPlayerAlive )
		self.customEM.addEventHandler( "cityAcquired", self.onCityAcquired )
		self.customEM.setPopupHandler( RevDefs.barbSettlePopup, ["barbSettlePopup",self.barbSettleHandler,self.blankHandler] )

		if( self.LOG_DEBUG ) :
			self.customEM.addEventHandler( "kbdEvent", self.onKbdEvent )
			print "BarbCiv settings: iNewWorldPolicy: %d MilStr: %d ByStyle: %d MilWin: %d dbg: %d" \
			%(self.iNewWorldPolicy, self.militaryStrength, self.barbCivsByStyle, self.militaryWindow, self.LOG_DEBUG)

		if( not SDTK.sdObjectExists( "BarbarianCiv", game ) ) :
			SDTK.sdObjectInit( "BarbarianCiv", game, {} )
			SDTK.sdObjectSetVal( "BarbarianCiv", game, "AlwaysMinorList", [] )
		elif( SDTK.sdObjectGetVal( "BarbarianCiv", game, "AlwaysMinorList" ) == None ) :
			SDTK.sdObjectSetVal( "BarbarianCiv", game, "AlwaysMinorList", [] )

		# Add any players that should always be minors from RevDefs
		alwaysMinorList = SDTK.sdObjectGetVal( "BarbarianCiv", game, "AlwaysMinorList" )
		alwaysMinorList.extend( RevDefs.alwaysMinorList )
		SDTK.sdObjectSetVal( "BarbarianCiv", game, "AlwaysMinorList", alwaysMinorList )

	def removeEventHandlers( self ) :
		print "Removing event handlers from BarbarianCiv"

		self.customEM.removeEventHandler( "BeginGameTurn", self.onBeginGameTurn )
		self.customEM.removeEventHandler( "EndPlayerTurn", self.onEndPlayerTurn )
		self.customEM.removeEventHandler( "firstContact", self.onFirstContact )
		self.customEM.removeEventHandler( "setPlayerAlive", self.onSetPlayerAlive )
		self.customEM.removeEventHandler( "cityAcquired", self.onCityAcquired )
		self.customEM.setPopupHandler( RevDefs.barbSettlePopup, ["barbSettlePopup",self.blankHandler,self.blankHandler] )

		if( self.LOG_DEBUG ) :
			self.customEM.removeEventHandler( "kbdEvent", self.onKbdEvent )


	def blankHandler( self, playerID, netUserData, popupReturn ) :

			# Dummy handler to take the second event for popup
			return

	def barbSettleHandler( self, iPlayerID, netUserData, popupReturn ) :

			if( popupReturn.getButtonClicked() == 0 ) :
				if( self.LOG_DEBUG ) : print ("BarbCiv - Clicked option 0, no change")
##********************************
##   LEMMY 101 FIX
##********************************

			if( popupReturn.getButtonClicked() == 1 ) :
				RevUtils.changeHuman( self.newPlayerIdx, iPlayerID )
##********************************
##   LEMMY 101 FIX
##********************************


	def onBeginGameTurn( self, argsList ) :
		'check for worthy barb cities'
		iGameTurn = argsList[0]

		self.checkBarbCities()

		# print iGameTurn


	def onSetPlayerAlive( self, argsList ) :

		iPlayerID = argsList[0]
		bNewValue = argsList[1]


	def onEndPlayerTurn( self, argsList ) :

		iGameTurn, iPlayer = argsList
		bDoLaunchRev = False

		iNextPlayer = iPlayer + 1
		while( iNextPlayer <= gc.getBARBARIAN_PLAYER() ) :
			if( not gc.getPlayer(iNextPlayer).isAlive() ) :
				iNextPlayer += 1
			else :
				break

		if( iNextPlayer > gc.getBARBARIAN_PLAYER() ) :
			iGameTurn += 1
			iNextPlayer = 0
			while( iNextPlayer < iPlayer ) :
				if( not gc.getPlayer(iNextPlayer).isAlive() ) :
					iNextPlayer += 1
				else :
					break

		# Stuff at beginning of this players turn
		if( gc.getPlayer(iNextPlayer).isMinorCiv() and not gc.getPlayer(iNextPlayer).isBarbarian() ) :
			self.checkMinorCivs(iNextPlayer)


	def onKbdEvent(self, argsList ):
		'keypress handler'
		eventType,key,mx,my,px,py = argsList

		if ( eventType == RevDefs.EventKeyDown ):
			theKey=int(key)

			# For debug or trial only
			if( theKey == int(InputTypes.KB_B) and self.customEM.bShift and self.customEM.bCtrl ) :
				iNumMinors = 0
				iNumFormerBarbs = 0
				for i in range(0,gc.getMAX_CIV_PLAYERS()):
					if( gc.getPlayer(i).isMinorCiv() ) :
						iNumMinors += 1
					if( SDTK.sdObjectExists( 'BarbarianCiv', gc.getPlayer(i) ) ) :
						iNumFormerBarbs += 1
				mess = "Num barb cities: %d\n"%(PyPlayer( gc.getBARBARIAN_PLAYER() ).getNumCities())
				mess += "Num minor civs: %d\n"%(iNumMinors)
				mess += "Num former barbs: %d\n"%(iNumFormerBarbs)

				popup = PyPopup.PyPopup(RevDefs.barbSettlePopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
				popup.setBodyString(mess)
				popup.launch()

	def getScenario( self, pArea, bVerbose = True ) :

		bNonBarbCivCities = False
		bForeignCivCities = False
		bNonBarbCivUnits = False
		bMajorCivCities = False
		bNewWorldScenario = True

		if( pArea == None ) :
			print "Error!  Passed a None area!"
			game.setAIAutoPlay(0)
			return [True,True,True,False]

		for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
			if( pArea.getCitiesPerPlayer(idx) > 0 ) :
				if( not gc.getPlayer(idx).isMinorCiv() ) :
					bMajorCivCities = True
					if( not pArea.getID() == gc.getPlayer(idx).getCapitalCity().area().getID() ) :
						bNonBarbCivCities = True
						bForeignCivCities = True
						bNonBarbCivUnits = True
				if( not SDTK.sdObjectExists( 'BarbarianCiv', gc.getPlayer(idx) ) ) :
					bNonBarbCivCities = True
					bNonBarbCivUnits = True
					bNewWorldScenario = False

			elif( not bNonBarbCivUnits and pArea.getUnitsPerPlayer(idx) > 0 ) :
				if( not SDTK.sdObjectExists( 'BarbarianCiv', gc.getPlayer(idx) ) ) :
					bNonBarbCivUnits = True
				elif( not gc.getPlayer(idx).isMinorCiv() ) :
					if( gc.getPlayer(idx).getNumCities() > 0 and not pArea.getID() == gc.getPlayer(idx).getCapitalCity().area().getID() ) :
						bNonBarbCivUnits = True

		if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - bNewWorld: %d, bUnits: %d, bCities: %d, bForeignCities: %d"%(bNewWorldScenario, bNonBarbCivUnits, bNonBarbCivCities, bForeignCivCities))

		return [bNewWorldScenario, bNonBarbCivCities, bForeignCivCities, bMajorCivCities, bNonBarbCivUnits]

	def pickNewPlayerSlot( self, cityList, bVerbose = True ) :

		newPlayerIdx = None
		newTeamIdx = None

		# Check for dead civ with culture in the city
		for i in range(0,gc.getMAX_CIV_PLAYERS()) :
			if( (not gc.getPlayer(i).isAlive()) and cityList[0].getCulture(i) > 30 ) :
				# Rule out civs if they just lost city to barbs
				bPossible = True
				for pCity in cityList :
					if( pCity.getPreviousOwner() == i ) :
						bPossible = False
						break

				if( bPossible ) :
					newPlayerIdx = i
					newTeamIdx = gc.getPlayer(i).getTeam()
					if( self.LOG_DEBUG and bVerbose) : CvUtil.pyPrint("  BC - Reincarnating dead player %s"%(gc.getPlayer(i).getCivilizationDescription(0)))
					break

		if( newPlayerIdx == None ) :
			# Put in empty slot
			for i in range(0,gc.getMAX_CIV_PLAYERS()) :
				if( not gc.getPlayer(i).isEverAlive() and not gc.getPlayer(i).isAlive() ) :
					if( not SDTK.sdObjectExists('Revolution', gc.getPlayer(i)) ) :
						newPlayerIdx = i
						#if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - searching for team for never alive player %d, they think team is %d"%(i,gc.getPlayer(i).getTeam()))
						newTeamIdx = gc.getPlayer(i).getTeam()
						break

			# Overwrite a civ that never managed to found a city and hasn't lost any cities (maybe a rebel)
#			if( newPlayerIdx == None ) :
#				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - No empty slots, looking for 'fake' civs")
#				for i in range(0,gc.getMAX_CIV_PLAYERS()) :
#					if( (not gc.getPlayer(i).isAlive()) and (not gc.getPlayer(i).isFoundedFirstCity()) and gc.getPlayer(i).getCitiesLost() == 0 and (gc.getPlayer(i).getNumUnits() == 0) ) :
#						if( SDTK.sdObjectExists('Revolution', gc.getPlayer(i)) ) :
#							spawnList = SDTK.sdObjectGetVal('Revolution', gc.getPlayer(i), 'SpawnList')
#							if( not spawnList == None and len(spawnList) > 0 ) :
#								continue
#						newPlayerIdx = i
#						newTeamIdx = gc.getPlayer(i).getTeam()
#						break


		return [newPlayerIdx, newTeamIdx]

	def getCloseCivs( self, iPlayer, pArea, ix, iy, bVerbose = True ) :
		closePlayers = list()
		closeTeams = list()
		for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
			if( not idx == iPlayer ) :
				if( pArea.getCitiesPerPlayer(idx) > 0 and not gc.getPlayer(idx).isMinorCiv() ) :

					playerI = gc.getPlayer(idx)
					capitalI = playerI.getCapitalCity()
					if( plotDistance( ix, iy, capitalI.getX(), capitalI.getY() ) < self.warCloseDist/2 ) :
						if( self.LOG_DEBUG and bVerbose) : CvUtil.pyPrint("  BC - %s are very close to new barb civ"%(playerI.getCivilizationDescription(0)))
						closePlayers.append(idx)
					if( plotDistance( ix, iy, capitalI.getX(), capitalI.getY() ) < self.warCloseDist ) :
						if( self.LOG_DEBUG and bVerbose) : CvUtil.pyPrint("  BC - %s are close to new barb civ"%(playerI.getCivilizationDescription(0)))
						closePlayers.append(idx)

					if( plotDistance( ix, iy, capitalI.getX(), capitalI.getY() ) < self.warCloseDist + 4 ) :
						if( not playerI.getTeam() in closeTeams ) :
							# Used for local tech knowledge calculations, other non-war benefits
							closeTeams.append(playerI.getTeam())

		return [closePlayers,closeTeams]

	def countNonBarbCivContacts( self, iPlayer, bVerbose = True ) :

		iCount = 0
		pPlayer = gc.getPlayer(iPlayer)
		pTeam = gc.getTeam(pPlayer.getTeam())

		for idx in range(0,gc.getMAX_CIV_PLAYERS()):
			playerI = gc.getPlayer(idx)
			if( playerI.isAlive() and pTeam.isHasMet(playerI.getTeam()) ) :
				if( playerI.isMinorCiv() ) :
					continue
				if( SDTK.sdObjectExists( 'BarbarianCiv', playerI ) ) :
					try :
						if( playerI.getCapitalCity().area().getID() == pPlayer.getCapitalCity().area().getID() ) :
							continue
					except :
						pass

				iCount += 1

		if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Player %d can contact %d non local barb civs"%(iPlayer,iCount))

		return iCount

	def pickCivAndLeader( self, cityList, newPlayerIdx, cultPlayer = None, artStyle = None, bVerbose = True ) :
		# Pick the civ type and leader for the new civ

		newCivIdx = None
		newLeaderIdx = None

		# TODO: add preferred artstyle

		# Don't reincarnate as these types
		iMinor = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),RevDefs.sXMLMinor)
		iBarbarian = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),RevDefs.sXMLBarbarian)
		# Civs not currently in the game
		availableCivs = list()
		# Civs with similar style to cultPlayer, if they exist
		similarStyleCivs = list()
		for civType in range(0,gc.getNumCivilizationInfos()) :
			if( not civType == iBarbarian ) :
				if( not civType == iMinor ) :
					taken = False
					for i in range(0,gc.getMAX_CIV_PLAYERS()) :
						if( not i == newPlayerIdx ) :
							if( civType == gc.getPlayer(i).getCivilizationType() ) :
								if( gc.getPlayer(i).isEverAlive() or SDTK.sdObjectExists('Revolution', gc.getPlayer(i)) ) :
									taken = True
									break
					if( not taken ) :
						availableCivs.append(civType)
						if( not cultPlayer == None ) :
							if( gc.getCivilizationInfo( cultPlayer.getCivilizationType() ).getArtStyleType() == gc.getCivilizationInfo(civType).getArtStyleType() ) :
								if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Potential similar style civ: %s"%(gc.getCivilizationInfo(civType).getShortDescription(0)))
								similarStyleCivs.append(civType)

		if( len(availableCivs) < 1 ) :
			if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - ERROR: Unexpected lack of unused civ types")
			return [newCivIdx, newLeaderIdx]

		if( len(similarStyleCivs) > 0 and self.barbCivsByStyle ) :
			# Create a civ using this style
			if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Creating a similar style civ to %s"%(cultPlayer.getCivilizationDescription(0)))
			availableCivs = similarStyleCivs

		newCivIdx = availableCivs[game.getSorenRandNum(len(availableCivs),'BarbarianCiv: pick unused civ type')]

		# Choose a leader for the new civ
		leaderList = list()
		for leaderType in range(0,gc.getNumLeaderHeadInfos()) :
			if( gc.getCivilizationInfo(newCivIdx).isLeaders(leaderType) or game.isOption( GameOptionTypes.GAMEOPTION_LEAD_ANY_CIV ) ) :
				taken = False
				for jdx in range(0,gc.getMAX_PLAYERS()) :
					if( gc.getPlayer(jdx).getLeaderType() == leaderType and not newPlayerIdx == jdx ) :
						taken = True
						break
				if( not taken ) : leaderList.append(leaderType)

		if( len(leaderList) < 1 ) :
			if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - ERROR: Unexpected lack of possible leaders")
			return [newCivIdx, newLeaderIdx]

		newLeaderIdx = leaderList[game.getSorenRandNum(len(leaderList),'BarbarianCiv: pick leader')]

		return [newCivIdx,newLeaderIdx]

	def giveTechs( self, newPlayerIdx, closeTeams, bNewWorldScenario, bBuilderBonus = 0, militaryStyle = None, bVerbose = True ) :
		# Give techs to new player, with variables for extra techs for builders or military style
		newPlayer = gc.getPlayer( newPlayerIdx )
		newTeam = gc.getTeam(newPlayer.getTeam())

		barbTechFrac = self.RevOpt.getBarbTechFrac()
		newWorldFrac = self.RevOpt.getNewWorldReduction()
		barbTeam = gc.getTeam( gc.getBARBARIAN_TEAM() )
		if(self.iNewWorldPolicy == 0):
			bNewWorldScenario = false
		
		iPartials = 2

		for techID in range(0,gc.getNumTechInfos()) :
			numTeams = game.countCivTeamsAlive() + 1
			numTeamsWhoKnow = game.countKnownTechNumTeams(techID)
			fracKnow = numTeamsWhoKnow/(1.0*numTeams)

			if( barbTeam.isHasTech(techID) and not newTeam.isHasTech(techID) ) :
				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving %s   from barb"%(PyInfo.TechnologyInfo(techID).getDescription()))
				newTeam.setHasTech(techID,True,newPlayerIdx,False,False)
				continue

			elif( not bNewWorldScenario ) :

				if( len(closeTeams) > 0 ) :
					localNumTeamsWhoKnow = 0
					for idx in closeTeams :
						teamI = gc.getTeam(idx)
						if( teamI.isHasTech(techID) ) :
							localNumTeamsWhoKnow += 1

					modFracKnow = fracKnow/2.0 + localNumTeamsWhoKnow/(2.0*(len(closeTeams) + .5))
				else :
					modFracKnow = fracKnow

				if( modFracKnow >= barbTechFrac and newPlayer.canEverResearch(techID) ) :
					if( not newTeam.isHasTech(techID) ) :
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving %s   %f with %f loc"%(PyInfo.TechnologyInfo(techID).getDescription(),fracKnow,modFracKnow))
						newTeam.setHasTech(techID,True,newPlayerIdx,False,False)

		if( not bNewWorldScenario ) :
			if( not militaryStyle == None ) :
				iPartials += 2

				iHorseback = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLHorseback)
				iSailing = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLSailing)
				iWheel = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLWheel)
				iHusbandry = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLAnimal)
				iBronze = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLBronze)
				iIron = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLIron)
				iGuilds = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLGuilds)
# Rise of Mankind 2.6 changes
				iChariotry = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLChariotry)
				iMetalCasting = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLMetalCasting)
				iNavalWarfare = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLNavalWarfare)
				iWeaving = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLWeaving)
				iMining = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLMining)
# Rise of Mankind 2.6 changes end
				if( militaryStyle == 'viking' ) :
					if( not newTeam.isHasTech(iWeaving) ) :
						newTeam.setHasTech(iWeaving,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iWeaving).getDescription()))
						iPartials -= 1
					elif( not newTeam.isHasTech(iSailing) ) :
						newTeam.setHasTech(iSailing,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iSailing).getDescription()))
						iPartials -= 1
						
					# give Mining or Metal Casting if the civ has Mining already
					if( not newTeam.isHasTech(iMining) ) :
						newTeam.setHasTech(iMining,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iMining).getDescription()))
						iPartials -= 1
					elif( not newTeam.isHasTech(iMetalCasting) ) :
						newTeam.setHasTech(iMetalCasting,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iMetalCasting).getDescription()))
						iPartials -= 1						
					elif( not newTeam.isHasTech(iBronze) ) :
						newTeam.setHasTech(iBronze,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iBronze).getDescription()))
						iPartials -= 1
					else :
						if( not newTeam.isHasTech(iIron) ) :
							newTeam.setHasTech(iIron,True,newPlayerIdx,False,False)
							if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iIron).getDescription()))
							iPartials -= 1
				elif( militaryStyle == 'chariot' ) :
					if( not newTeam.isHasTech(iWheel) ) :
						newTeam.setHasTech(iWheel,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iWheel).getDescription()))
						iPartials -= 1
					if( not newTeam.isHasTech(iHusbandry) ) :
						newTeam.setHasTech(iHusbandry,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iHusbandry).getDescription()))
						iPartials -= 1
					if( not newTeam.isHasTech(iChariotry) ) :
						newTeam.setHasTech(iChariotry,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iChariotry).getDescription()))
						iPartials -= 1
							
					if( not newTeam.isHasTech(iMining) ) :
						newTeam.setHasTech(iMining,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iMining).getDescription()))
						iPartials -= 1
					elif( not newTeam.isHasTech(iMetalCasting) ) :
						newTeam.setHasTech(iMetalCasting,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iMetalCasting).getDescription()))
						iPartials -= 1						
					elif( not newTeam.isHasTech(iBronze) ) :
						newTeam.setHasTech(iBronze,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iBronze).getDescription()))
						iPartials -= 1
					else :
						if( not newTeam.isHasTech(iIron) ) :
							newTeam.setHasTech(iIron,True,newPlayerIdx,False,False)
							if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iIron).getDescription()))
							iPartials -= 1

				elif( militaryStyle == 'horse' ) :
					if( not newTeam.isHasTech(iHusbandry) ) :
						newTeam.setHasTech(iHusbandry,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iHusbandry).getDescription()))
						iPartials -= 1
					elif( not newTeam.isHasTech(iChariotry) ) :
						newTeam.setHasTech(iChariotry,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iChariotry).getDescription()))
						iPartials -= 1
					elif( not newTeam.isHasTech(iHorseback) ) :
						newTeam.setHasTech(iHorseback,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iHorseback).getDescription()))
						iPartials -= 1
					else :
						if( game.countKnownTechNumTeams(iGuilds) > 1 ) :
							if( not newTeam.isHasTech(iGuilds) ) :
								newTeam.setHasTech(iGuilds,True,newPlayerIdx,False,False)
								if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iGuilds).getDescription()))
								iPartials -= 1

				else : #( militaryStyle == 'balance' ) :
					if( not newTeam.isHasTech(iMining) ) :
						newTeam.setHasTech(iMining,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iMining).getDescription()))
						iPartials -= 1
					elif( not newTeam.isHasTech(iMetalCasting) ) :
						newTeam.setHasTech(iMetalCasting,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iMetalCasting).getDescription()))
						iPartials -= 1		
					if( not newTeam.isHasTech(iBronze) ) :
						newTeam.setHasTech(iBronze,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iBronze).getDescription()))
						iPartials -= 1
					elif( not newTeam.isHasTech(iIron) ) :
							newTeam.setHasTech(iIron,True,newPlayerIdx,False,False)
							if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iIron).getDescription()))
							iPartials -= 1
					elif( not newTeam.isHasTech(iHorseback) ) :
						newTeam.setHasTech(iHorseback,True,newPlayerIdx,False,False)
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iHorseback).getDescription()))
						iPartials -= 1
					else :
						if( game.countKnownTechNumTeams(iGuilds) > 0 ) :
							if( not newTeam.isHasTech(iGuilds) ) :
								newTeam.setHasTech(iGuilds,True,newPlayerIdx,False,False)
								if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving military %s"%(PyInfo.TechnologyInfo(iGuilds).getDescription()))
								iPartials -= 1

			elif( bBuilderBonus > 0 ) :
				# Extra techs
				maxCount = self.RevOpt.getBuilderBonusTechs()
				iPartials += 3

				if( bNewWorldScenario and self.iNewWorldPolicy > 0 ) :
					maxCount -= 1

				newTechList = list()
				for techID in range(0,gc.getNumTechInfos()) :
					if( not newTeam.isHasTech(techID) and newPlayer.canResearch(techID,True) ) :
						newTechList.append(techID)

				for k in range(0,maxCount) :
					if( len(newTechList) > 0 ) :
						techID = newTechList.pop(game.getSorenRandNum(len(newTechList),'BC: Builder tech'))
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving builder %s"%(PyInfo.TechnologyInfo(techID).getDescription()))
						newTeam.setHasTech(techID,True,newPlayerIdx,False,False)

		if (self.iNewWorldPolicy <=1):
			bNewWorldScenario = false
		iPass = 0
		while (iPartials > 0 and iPass < 4) :
			if( self.LOG_DEBUG and bVerbose ) :
				CvUtil.pyPrint("  BC - Giving %d partial techs, pass %d"%(iPartials,iPass))
			for techID in range(0,gc.getNumTechInfos()) :
				if(bNewWorldScenario):
					iPass = 4
					break
	
				tech = gc.getTechInfo( techID )
				iTechPower = tech.getPowerValue()
				iTechEra = tech.getEra()

				if( not newTeam.isHasTech(techID) and iTechEra <= newPlayer.getCurrentEra() + 1 and tech.getEra() < gc.getNumEraInfos() ) :
					iTechCost = newTeam.getResearchCost(techID)
					iResearchProgress = newTeam.getResearchProgress(techID)
					iThreshold = iTechCost/2
					if( bBuilderBonus ) : iThreshold += iTechCost/5

					if( iResearchProgress < iThreshold ) :

						anyAndFalse = False
						for k in range(gc.getDefineINT("NUM_AND_TECH_PREREQS")):
							iPrereqTech = tech.getPrereqAndTechs( k )
							if( iPrereqTech > 0 and iPrereqTech < gc.getNumTechInfos() ) :
								if( not newTeam.isHasTech( iPrereqTech ) ) :
									anyAndFalse = True
									break

						allOrFalse = -1
						for k in range(gc.getDefineINT("NUM_OR_TECH_PREREQS")):
							iPrereqTech = tech.getPrereqOrTechs( k )
							if( iPrereqTech > 0 and iPrereqTech < gc.getNumTechInfos() ) :
								allOrFalse = 1
								if( newTeam.isHasTech( iPrereqTech ) ) :
									allOrFalse = 0
									break

						if( not anyAndFalse and allOrFalse <= 0 ) :

							numTeams = game.countCivTeamsAlive() + 1
							numTeamsWhoKnow = game.countKnownTechNumTeams(techID)
							fracKnow = numTeamsWhoKnow/(1.0*numTeams)

							fTechFrac = .35 + fracKnow/2.0 + game.getSorenRandNum(10 + 7*iPartials,"BarbCiv - give partial tech")/100.0
							iNewProgress = int(iResearchProgress + fTechFrac * iTechCost)
							iNewProgress = min([iNewProgress, (85*iTechCost)/100])

							iBase = (40*gc.getGameSpeedInfo(game.getGameSpeedType()).getResearchPercent())/100
							iRand = iTechCost + 4*iBase
							if( bBuilderBonus ) :
								iRand += max([0, iBase*(iTechPower - iPartials)])
							elif( not militaryStyle == None ) :
								iRand += 4*iBase - iBase*iTechPower
							else :
								iRand += 8*iBase + iBase*iTechPower

							if( iNewProgress > game.getSorenRandNum( iRand, "BarbCiv - give partial tech") ) :
								if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving partial %s: %2.1f perc (power %d)"%(tech.getDescription(),(100.0*iNewProgress)/iTechCost,iTechPower))
								newTeam.setResearchProgress( techID, iNewProgress, newPlayer.getID() )
								iPartials -= 1
								if( iPartials <= 0 ) :
									break

			iPass += 1


	def pickMilitaryStyle( self, iPlayer, iNumClosePlayers = -1, bVerbose = True, iCounter = -1, iAttack = -1, iMobile = -1 ) :
		# Pick militaristic flavor: viking, horse, chariot, balance etc
		newPlayer = gc.getPlayer(iPlayer)

		if( iAttack != -1 and iAttack != gc.getUnitClassInfo(gc.getUnitInfo(iAttack).getUnitClassType()).getDefaultUnitIndex() ) :
			if( game.getSorenRandNum(100,'BC: Balance style') < 40 ) :
				milStyle = 'balance'
				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - BALANCE style!")

		milStyle = None
		if( newPlayer.countNumCoastalCities() > newPlayer.getNumCities()/2 ) :
			if( iNumClosePlayers == 0 ) :
				vikingOdds = 75
			elif( iCounter != -1 and iCounter != gc.getUnitClassInfo(gc.getUnitInfo(iCounter).getUnitClassType()).getDefaultUnitIndex() ) :
				vikingOdds = 60
			else :
				vikingOdds = 40

			if( game.getSorenRandNum(100,'BC: Viking style') < vikingOdds ) :
				milStyle = 'viking'
				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - VIKING style!")

		if( milStyle == None ) :
# Rise of Mankind 2.7
#			iHorseback = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLHorseback)
			iChariotry = CvUtil.findInfoTypeNum(gc.getTechInfo,gc.getNumTechInfos(),RevDefs.sXMLChariotry)
			if( not gc.getTeam(newPlayer.getTeam()).isHasTech(iChariotry) ) :
# Rise of Mankind 2.7
				iOdds = 40
				if( iMobile != -1 and iMobile != gc.getUnitClassInfo(gc.getUnitInfo(iMobile).getUnitClassType()).getDefaultUnitIndex() ) :
					iOdds += 25
				if( game.getSorenRandNum(100,'BC: CHARIOT style') < iOdds ) :
					milStyle = 'chariot'
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - CHARIOT style!")

			else :
				iOdds = 40
				if( iMobile != -1 and iMobile != gc.getUnitClassInfo(gc.getUnitInfo(iMobile).getUnitClassType()).getDefaultUnitIndex() ) :
					iOdds += 25
				if( game.getSorenRandNum(100,'BC: Horse style') < iOdds ) :
					milStyle = 'horse'
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - HORSE style!")

		if( milStyle == None ) :
			milStyle = 'balance'
			if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - BALANCE style!")

		return milStyle

	def getUnitsForPlayer( self, iPlayer, bVerbose = False, bSilent = False ) :

		newPlayer = gc.getPlayer( iPlayer )

		# Basic units, find the (potentially unique) unit type for the class for this civ
		workerClass = CvUtil.findInfoTypeNum(gc.getUnitClassInfo,gc.getNumUnitClassInfos(),RevDefs.sXMLWorker)
		iWorker = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationUnits(workerClass)
		settlerClass = CvUtil.findInfoTypeNum(gc.getUnitClassInfo,gc.getNumUnitClassInfos(),RevDefs.sXMLSettler)
		iSettler = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationUnits(settlerClass)
		scoutClass = CvUtil.findInfoTypeNum(gc.getUnitClassInfo,gc.getNumUnitClassInfos(),RevDefs.sXMLScout)
		iScout = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationUnits(scoutClass)

		# Pick military units civ will get, have to vary depending on available techs
		iBestDefender = UnitTypes.NO_UNIT
		iCounter = UnitTypes.NO_UNIT
		iMobile = UnitTypes.NO_UNIT
		iAttack = UnitTypes.NO_UNIT
		iAttackCity = UnitTypes.NO_UNIT
		iAssaultShip = UnitTypes.NO_UNIT
		for unitClass in range(0,gc.getNumUnitClassInfos()) :
			unitID = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationUnits(unitClass)
			unitInfo = gc.getUnitInfo(unitID)

			if( unitInfo == None ) :
				continue
			
			if( gc.getUnitClassInfo(unitClass).getMaxGlobalInstances() > 0 or gc.getUnitClassInfo(unitClass).getMaxPlayerInstances() > 0 or gc.getUnitClassInfo(unitClass).getMaxTeamInstances() > 0 ) :
				continue
				
			#Afforess
			if (gc.getUnitInfo(unitID).getExtraCost() > 0):
				continue
			if (gc.getUnitInfo(unitID).isHiddenNationality()):
				continue
			if (gc.getUnitInfo(unitID).isAlwaysHostile()):
				continue
			#Afforess End
				
			if( newPlayer.canTrain(unitID,False,False) ) :

				if( gc.getUnitInfo(unitID).getDomainType() == DomainTypes.DOMAIN_LAND ): # canTrain only does technology check
					# Defender (Archer,Longbow)
					if( gc.getUnitInfo(unitID).getDefaultUnitAIType() == UnitAITypes.UNITAI_CITY_DEFENSE ):
						if( (iBestDefender == UnitTypes.NO_UNIT) or gc.getUnitInfo(unitID).getCombat() >= gc.getUnitInfo(iBestDefender).getCombat() ) :
							#if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Civ def units return value: %d from %d, comp to %d"%(gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationUnits(unitClass),unitClass,unitID))
							iBestDefender = unitID
							if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Best defender set to %s"%(PyInfo.UnitInfo(iBestDefender).getDescription()))
					# Counter (Axemen,Phalanx)
					if( gc.getUnitInfo(unitID).getDefaultUnitAIType() == UnitAITypes.UNITAI_COUNTER ):
						if( (iCounter == UnitTypes.NO_UNIT) or gc.getUnitInfo(unitID).getCombat() >= gc.getUnitInfo(iCounter).getCombat() ) :
							#if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Civ counter units return value: %d from %d, comp to %d"%(gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationUnits(unitClass),unitClass,unitID))
							iCounter = unitID
							if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Best counter unit set to %s"%(PyInfo.UnitInfo(iCounter).getDescription()))
					# Assault units
					if( gc.getUnitInfo(unitID).getUnitAIType(UnitAITypes.UNITAI_ATTACK) ):
						if( (iAttack == UnitTypes.NO_UNIT) or gc.getUnitInfo(unitID).getCombat() > gc.getUnitInfo(iAttack).getCombat() ) :
							#if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Civ attack units return value: %d from %d, comp to %d"%(gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationUnits(unitClass),unitClass,unitID))
							iAttack = unitID
							if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Best attack set to %s"%(PyInfo.UnitInfo(iAttack).getDescription()))
					if( gc.getUnitInfo(unitID).getUnitAIType(UnitAITypes.UNITAI_ATTACK_CITY) ):
						if( (iAttackCity == UnitTypes.NO_UNIT) or gc.getUnitInfo(unitID).getBombardRate() > gc.getUnitInfo(iAttackCity).getBombardRate() or (gc.getUnitInfo(iAttackCity).getBombardRate() == 0 and gc.getUnitInfo(unitID).getCombat() > gc.getUnitInfo(iAttackCity).getCombat()) ) :
							#if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Civ attack units return value: %d from %d, comp to %d"%(gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationUnits(unitClass),unitClass,unitID))
							iAttack = unitID
							if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Best attack set to %s"%(PyInfo.UnitInfo(iAttackCity).getDescription()))
					# Fast Attack (Horse archer)
					if( gc.getUnitInfo(unitID).getUnitAIType( UnitAITypes.UNITAI_ATTACK ) and gc.getUnitInfo(unitID).getMoves() > 1 ):
						if( (iMobile == UnitTypes.NO_UNIT) or gc.getUnitInfo(unitID).getCombat() > gc.getUnitInfo(iMobile).getCombat() ) :
							#if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Civ mob units return value: %d from %d, comp to %d"%(gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationUnits(unitClass),unitClass,unitID))
							iMobile = unitID
							if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Best mobile set to %s"%(PyInfo.UnitInfo(iMobile).getDescription()))

				elif( gc.getUnitInfo(unitID).getDomainType() == DomainTypes.DOMAIN_SEA ):
					# Military unit transport ship
					if( gc.getUnitInfo(unitID).getUnitAIType( UnitAITypes.UNITAI_ASSAULT_SEA ) ):
						iAssaultShip = unitID
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Best assault ship set to %s"%(PyInfo.UnitInfo(iAssaultShip).getDescription()))

		if( iCounter == UnitTypes.NO_UNIT ) :
			iCounter = iAttack
			if( self.LOG_DEBUG and (bVerbose or not bSilent) ) : CvUtil.pyPrint("  BC - Counter unit not set, using %s"%(PyInfo.UnitInfo(iCounter).getDescription()))
		if( iBestDefender == UnitTypes.NO_UNIT ) :
			iBestDefender = iCounter
			if( self.LOG_DEBUG and (bVerbose or not bSilent) ) : CvUtil.pyPrint("  BC - Defender not set, using %s"%(PyInfo.UnitInfo(iBestDefender).getDescription()))
		if( iMobile == UnitTypes.NO_UNIT )  :
			iMobile  = iAttack
			if( self.LOG_DEBUG and (bVerbose or not bSilent) ) : CvUtil.pyPrint("  BC - Mobile not set, using %s"%(PyInfo.UnitInfo(iMobile).getDescription()))
		if( iAttackCity == UnitTypes.NO_UNIT ) :
			iAttackCity = iAttack
			if( self.LOG_DEBUG and (bVerbose or not bSilent) ) : CvUtil.pyPrint("  BC - Attack city not set, using %s"%(PyInfo.UnitInfo(iAttackCity).getDescription()))

		if( self.LOG_DEBUG and (bVerbose or not bSilent) ) : CvUtil.pyPrint("  BC - Best defender is %s"%(PyInfo.UnitInfo(iBestDefender).getDescription()))
		if( self.LOG_DEBUG and (bVerbose or not bSilent) ) : CvUtil.pyPrint("  BC - Best counter unit is %s"%(PyInfo.UnitInfo(iCounter).getDescription()))
		if( self.LOG_DEBUG and (bVerbose or not bSilent) ) : CvUtil.pyPrint("  BC - Best attack is %s"%(PyInfo.UnitInfo(iAttack).getDescription()))
		if( self.LOG_DEBUG and (bVerbose or not bSilent) ) : CvUtil.pyPrint("  BC - Best mobile is %s"%(PyInfo.UnitInfo(iMobile).getDescription()))
		if( self.LOG_DEBUG and (bVerbose or not bSilent) ) : CvUtil.pyPrint("  BC - Best attack city is %s"%(PyInfo.UnitInfo(iAttackCity).getDescription()))
		if( not iAssaultShip == UnitTypes.NO_UNIT and self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Best assault ship set to %s"%(PyInfo.UnitInfo(iAssaultShip).getDescription()))

		return [iWorker, iSettler, iScout, iBestDefender, iCounter, iAttack, iMobile, iAttackCity, iAssaultShip]

	def getBuildingsForPlayer( self, iPlayer, bVerbose = True ) :

		newPlayer = gc.getPlayer( iPlayer )

		# Use classes to give unique buidlings
		libraryClass = CvUtil.findInfoTypeNum(gc.getBuildingClassInfo,gc.getNumBuildingClassInfos(),RevDefs.sXMLLibrary)
		iLibrary = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationBuildings( libraryClass )
		granaryClass = CvUtil.findInfoTypeNum(gc.getBuildingClassInfo,gc.getNumBuildingClassInfos(),RevDefs.sXMLGranary)
		iGranary = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationBuildings( granaryClass )
		barracksClass = CvUtil.findInfoTypeNum(gc.getBuildingClassInfo,gc.getNumBuildingClassInfos(),RevDefs.sXMLBarracks)
		iBarracks = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationBuildings( barracksClass )
		marketClass = CvUtil.findInfoTypeNum(gc.getBuildingClassInfo,gc.getNumBuildingClassInfos(),RevDefs.sXMLMarket)
		iMarket = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationBuildings( marketClass )
		wallsClass = CvUtil.findInfoTypeNum(gc.getBuildingClassInfo,gc.getNumBuildingClassInfos(),RevDefs.sXMLWalls)
		iWalls = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationBuildings( wallsClass )
		lighthouseClass = CvUtil.findInfoTypeNum(gc.getBuildingClassInfo,gc.getNumBuildingClassInfos(),RevDefs.sXMLLighthouse)
		iLighthouse = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationBuildings( lighthouseClass )
		forgeClass = CvUtil.findInfoTypeNum(gc.getBuildingClassInfo,gc.getNumBuildingClassInfos(),RevDefs.sXMLForge)
		iForge = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationBuildings( forgeClass )
		monumentClass = CvUtil.findInfoTypeNum(gc.getBuildingClassInfo,gc.getNumBuildingClassInfos(),RevDefs.sXMLMonument )
		iMonument = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationBuildings( monumentClass )
# Rise of Mankind 2.6 building adjustments
		scribesClass = CvUtil.findInfoTypeNum(gc.getBuildingClassInfo,gc.getNumBuildingClassInfos(),RevDefs.sXMLScribes )
		iSchoolOfScribes = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationBuildings( scribesClass )
		bazaarClass = CvUtil.findInfoTypeNum(gc.getBuildingClassInfo,gc.getNumBuildingClassInfos(),RevDefs.sXMLBazaar )
		iBazaar = gc.getCivilizationInfo( newPlayer.getCivilizationType() ).getCivilizationBuildings( bazaarClass )

#        return [iLibrary, iGranary, iBarracks, iMarket, iWalls, iLighthouse, iForge, iMonument]
		return [iBazaar, iSchoolOfScribes, iLibrary, iGranary, iBarracks, iMarket, iWalls, iLighthouse, iForge, iMonument]
# Rise of Mankind 2.6 end

	def setupFormerBarbCity( self, pCity, iToPlayer, iBestDefender = None, iNumFreeDefenders = 1 ) :

		newPlayer = gc.getPlayer(iToPlayer)
		pyNewPlayer = PyPlayer(iToPlayer)
		cityX = pCity.getX()
		cityY = pCity.getY()

		defaultUnits = RevUtils.getPlayerUnits( cityX, cityY, iToPlayer )
		# Remove default given units
		for unit in defaultUnits :
			unit.kill( False, -1 )

		# Change name
		pCity.setName( newPlayer.getNewCityName(), True )

		# Culture
		iTotCul  = pCity.countTotalCultureTimes100()/100 + pCity.getCulture( gc.getBARBARIAN_PLAYER() )
		iPlotCul = pCity.plot().getCulture( gc.getBARBARIAN_PLAYER() )
		iPlotCul += pCity.plot().getCulture(pCity.getOriginalOwner())
		RevUtils.giveCityCulture( pCity, newPlayer.getID(), iTotCul, iPlotCul, False, True )

		# Change nearby barb units
		for [radius,pPlot] in RevUtils.plotGenerator( pCity.plot(), 3 ) :
			unitTypes = list()
			barbUnits = RevUtils.getPlayerUnits( pPlot.getX(), pPlot.getY(), gc.getBARBARIAN_PLAYER(), domain = DomainTypes.DOMAIN_LAND )
			for unit in barbUnits :
				if( not unit.isAnimal() and not unit.isCargo() and len(unitTypes) < (4 + newPlayer.getCurrentEra()) ) :
					unitTypes.append( unit.getUnitType() )
					unit.kill(False, -1)

			for type in unitTypes :
				pyNewPlayer.initUnit( type, pPlot.getX(), pPlot.getY(), 1 )

		if( not iBestDefender == None ) :
			pyNewPlayer.initUnit(iBestDefender,cityX,cityY,iNumFreeDefenders)

		return

	def setupSavedData( self, iPlayer, numCities = 0, capitalName = None, barbStyle = None, bSetupComplete = 0 ) :

		newPlayer = gc.getPlayer(iPlayer)

		playerDict = {}
		playerDict['SpawnTurn'] = game.getGameTurn()
		playerDict['NumCities'] = numCities
		playerDict['CapitalName'] = capitalName
		playerDict['SetupComplete'] = bSetupComplete
		playerDict['BarbStyle'] = barbStyle
		SDTK.sdObjectSetDict( 'BarbarianCiv', newPlayer, playerDict )


	def checkBarbCities( self, bVerbose = True ) :
		# Check Barbarian cities for any which should settle into a minor civ
		pyBarbPlayer = PyPlayer( gc.getBARBARIAN_PLAYER() )
		barbCityList = pyBarbPlayer.getCityList()

		if( len(barbCityList) == 0 ) :
			return

		if( game.countCivPlayersAlive() >= self.maxCivs ) :
			return

		# Odds modifier from config file
		mod = self.RevOpt.getFormMinorModifier()
		# If lots of barb cities, increase odds
		numCitiesMod = int(pow(len(barbCityList),.5))
		# Change by gamespeed, within limits
		gsm = RevUtils.getGameSpeedMod()
		# Odds of barb city settling down go up by era
		eraMod = game.getCurrentEra() - game.getStartEra()/2.0
		if( eraMod < .5 ) :
			eraMod = .5

		minPopForMinor = int(self.minPopulation + min([2,eraMod/2]))
		
		if( gc.getGame().isOption(GameOptionTypes.GAMEOPTION_BARBARIAN_WORLD) ) :
			if( (game.getGameTurn() - game.getStartTurn()) < 65/RevUtils.getGameSpeedMod() and game.getCurrentEra() < 2 ) :
				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Skipping barb city check in early BARBARIAN_WORLD game")
				return

		for city in barbCityList :
			pCity = city.GetCy()

			if( pCity.getPopulation() >= minPopForMinor ) :

				pArea = pCity.area()

				[bNewWorldScenario, bNonBarbCivCities, bForeignCivCities, bMajorCivCities, bNonBarbCivUnits] = self.getScenario( pArea, bVerbose = False )

				if( bNewWorldScenario ) :
					if( not bNonBarbCivUnits and not bMajorCivCities and self.iNewWorldPolicy >= 4 ) :
						continue
					elif( not bForeignCivCities and not bMajorCivCities and self.iNewWorldPolicy > 4 ) :
						continue

				odds = 3

				odds += 5*(pCity.getPopulation() - minPopForMinor)

				odds += 10*max([0,(pCity.getCultureLevel() - 1)])

				if( not pCity.getOriginalOwner() == gc.getBARBARIAN_PLAYER() ) :
					odds += 5

				if( pArea.getNumCities() == 1 ) :
					odds -= 6

				if( bNewWorldScenario and self.iNewWorldPolicy > 0 ) :

					if( not bForeignCivCities and not bMajorCivCities and self.iNewWorldPolicy > 4 ) :
						continue
					elif( not bNonBarbCivUnits and not bMajorCivCities and self.iNewWorldPolicy > 3 ) :
						continue

					iBarbAreaCities = pArea.getCitiesPerPlayer(gc.getBARBARIAN_PLAYER())
					if( iBarbAreaCities < 4 and pArea.getNumCities() - iBarbAreaCities > 3 ) :
						iPlayerCount = 0
						for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
							if( pArea.getCitiesPerPlayer(idx) > 0 ) :
								iPlayerCount += 1

						if( iPlayerCount > 1 ) :
							if( not bMajorCivCities ) :
								odds -= (9 - 2*iBarbAreaCities)
							else :
								odds -= (5 - iBarbAreaCities)

					if( bNonBarbCivUnits ) :
						odds *= 3
						odds /= 4
					else :
						if( eraMod < 2 ) :
							odds /= 3
						else :
							odds /= 2

				odds = int( self.formMinorMod*odds + 0.5 )

				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Odds of forming minor in %s are %d (%d pop, %d cult level with eraMod %f)"%(pCity.getName(),odds,pCity.getPopulation(),pCity.getCultureLevel(),eraMod))

				if( game.getSorenRandNum( int(100*RevUtils.getGameSpeedMod()), 'BC: Settle minor') < odds ) :
					self.createMinorCiv( [pCity] )
					return


	def createMinorCiv( self, cityList, bVerbose = True ) :
		# Turn city in list into a minor civ with a fighting chance
		try :
			cityString = u""
			for pCity in cityList :
				cityString += pCity.getName() + u", "
			cityString = cityString.strip(u", ")
		except :
			cityString = "1 %s"%localText.getText("TXT_KEY_REV_CITY", ())
		if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Creating minor civ in %s"%(cityString))

		[bNewWorldScenario, bNonBarbCivCities, bForeignCivCities, bMajorCivCities, bNonBarbCivUnits] = self.getScenario( cityList[0].area() )
		iEra = game.getCurrentEra() - game.getStartEra()/2
		pArea = cityList[0].area()

		# Pick a slot
		[iNewPlayer,iNewTeam] = self.pickNewPlayerSlot( cityList )

		if( iNewPlayer == None or iNewTeam == None ) :
			if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - no slots available, aborting barb civ creation")
			return

		# Check if barb city has culture from a non-barb player
		cultPlayer = None
		cultPlayerID = cityList[0].findHighestCulture()
		if( cultPlayerID >= 0 and cultPlayerID < gc.getBARBARIAN_PLAYER() ) :
			cultPlayer = gc.getPlayer( cultPlayerID )
			if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Civs new capital was culturally influenced by %s"%(cultPlayer.getCivilizationDescription(0)))

		[closePlayers,closeTeams] = self.getCloseCivs( iNewPlayer, cityList[0].area(), cityList[0].getX(), cityList[0].getY() )

		# Determine any artstyles to use preferencially

		# Pick civ and leader
		[newCivType,newLeaderType] = self.pickCivAndLeader( cityList, iNewPlayer, cultPlayer = cultPlayer )

		if( newCivType == None or newLeaderType == None ) :
			if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - no players or leaders available, aborting barb civ creation")
			return

		if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Adding new player in slot %d"%(iNewPlayer))
		
		# Add player to game
		game.addPlayer( iNewPlayer, newLeaderType, newCivType, false )

		newPlayer = gc.getPlayer(iNewPlayer)
		newTeam = gc.getTeam(newPlayer.getTeam())
		pyNewPlayer = PyPlayer(iNewPlayer)

		self.setupSavedData( iNewPlayer, numCities = len(cityList), capitalName = CvUtil.convertToStr(cityList[0].getName()) )
		
		if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - New player and team set up")

		newTeam.setIsMinorCiv( True, False )

		if( not newPlayer.isAlive() ) :
			if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Setting new player alive")
			newPlayer.setNewPlayerAlive(True)

		civName = newPlayer.getCivilizationDescription(0)
		leadName = newPlayer.getName()

		if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Minor civ %s (lead by %s) has formed in %s!  Turn: %d, Year: %d"%(civName,leadName,cityString,game.getElapsedGameTurns(),game.getTurnYear(game.getElapsedGameTurns())))

		# Add replay message
		mess = localText.getText("TXT_KEY_BARBCIV_FORM_MINOR", ())%(civName,cityString)
		mess = mess[0].capitalize() + mess[1:]
		game.addReplayMessage( ReplayMessageTypes.REPLAY_MESSAGE_MAJOR_EVENT, newPlayer.getID(), mess, cityList[0].getX(), cityList[0].getY(), gc.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"))

		for i,giftCity in enumerate(cityList) :
			#giftCity.changePopulation(1)
			# Using following method to acquire city produces 'revolted and joined' replay messages
			giftCity.plot().setOwner( newPlayer.getID() )

		# Note: city acquisition may invalidate previous city pointer, so have to create new list of cities
		cityList = pyNewPlayer.getCityList()
		capital = newPlayer.getCapitalCity()

		# Now setup generic new civ, no special types for minors
		# Gold
		newPlayer.changeGold( 70 + game.getSorenRandNum(100,'BarbarianCiv: give money') )

		# Tech
		self.giveTechs( iNewPlayer, closeTeams, bNewWorldScenario )

		# Units
		iNumBarbDefenders = gc.getHandicapInfo( game.getHandicapType() ).getBarbarianInitialDefenders()

		[iWorker, iSettler, iScout, iBestDefender, iCounter, iAttack, iMobile, iAttackCity, iAssaultShip] = self.getUnitsForPlayer( iNewPlayer )

		# Buildings
		#[iLibrary, iGranary, iBarracks, iMarket, iWalls, iLighthouse, iForge, iMonument] = self.getBuildingsForPlayer( iNewPlayer )
# Rise of Mankind 2.6 new buildings added to list
		[iBazaar, iSchoolOfScribes, iLibrary, iGranary, iBarracks, iMarket, iWalls, iLighthouse, iForge, iMonument] = self.getBuildingsForPlayer( iNewPlayer )
# Rise of Mankind 2.6 end
		# Put stuff in cities
		for i,city in enumerate(cityList) :
			cityX = city.getX()
			cityY = city.getY()
			pCity = city.GetCy()

			self.setupFormerBarbCity(pCity, newPlayer.getID(), iBestDefender, int(iNumBarbDefenders*self.militaryStrength + 0.5))

			# Adjust Population
			iPop = pCity.getPopulation()
			pCity.setPopulation( max([self.minPopulation, min([iPop, self.minPopulation + 1 + 2*newPlayer.getCurrentEra() - i])]) )
			if( bNewWorldScenario and self.iNewWorldPolicy > 0 ) :
				if( self.iNewWorldPolicy > 1 and not bNonBarbCivUnits ) :
					if( len(closeTeams) < 2 and iEra < 2 ) :
						iNewPop = max([min([self.minPopulation,iPop]),iPop - 1])
						pCity.setPopulation(max([iNewPop,self.minPopulation]))
			if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - %s has population %d (adj from %d)"%(pCity.getName(),pCity.getPopulation(),iPop))

			# Extra units
			pyNewPlayer.initUnit(iWorker,cityX,cityY,1)

			attackUnits = list()

			if( iEra > 0 ) :
				if( iCounter == iBestDefender ) :
					attackUnits.append( newPlayer.initUnit(iAttack,cityX,cityY,UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH ) )
				else :
					attackUnits.append( newPlayer.initUnit(iCounter,cityX,cityY,UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH ) )

			# Buildings
			if( i == 0 ) :

				if( not bNewWorldScenario or bForeignCivCities or self.iNewWorldPolicy == 0 ) :
					if( city.canConstruct( iBarracks ) ) :
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iBarracks).getDescription(),city.getName()))
						city.setNumRealBuildingIdx(iBarracks,1)

					if( iEra > 0 and city.canConstruct( iGranary ) ) :
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iGranary).getDescription(),city.getName()))
						city.setNumRealBuildingIdx(iGranary,1)

					if( iEra > 1 and city.canConstruct( iWalls ) ) :
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iWalls).getDescription(),city.getName()))
						city.setNumRealBuildingIdx(iWalls,1)

					if( iEra > 1 and city.canConstruct( iForge ) ) :
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iForge).getDescription(),city.getName()))
						city.setNumRealBuildingIdx(iForge,1)
# Rise of Mankind 2.6 new building types added
					if( iEra > 1 and city.canConstruct( iBazaar ) ) :
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iBazaar).getDescription(),city.getName()))
						city.setNumRealBuildingIdx(iBazaar,1)
					if( iEra > 1 and city.canConstruct( iSchoolOfScribes ) ) :
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iSchoolOfScribes).getDescription(),city.getName()))
						city.setNumRealBuildingIdx(iSchoolOfScribes,1)
# Rise of Mankind 2.6 end

				# TODO:  Mobile depending on bonuses?
				offensiveUnitList = [iCounter,iAttack,iMobile]
				if( iCounter != gc.getUnitClassInfo(gc.getUnitInfo(iCounter).getUnitClassType()).getDefaultUnitIndex() ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Extra weight for %s unique units"%(PyInfo.UnitInfo(iCounter).getDescription()))
					offensiveUnitList.append(iCounter)
				if( iAttack != gc.getUnitClassInfo(gc.getUnitInfo(iAttack).getUnitClassType()).getDefaultUnitIndex() ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Extra weight for %s unique units"%(PyInfo.UnitInfo(iAttack).getDescription()))
					offensiveUnitList.append(iAttack)
				if( iMobile != gc.getUnitClassInfo(gc.getUnitInfo(iMobile).getUnitClassType()).getDefaultUnitIndex() ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Extra weight for %s unique units"%(PyInfo.UnitInfo(iMobile).getDescription()))
					offensiveUnitList.append(iMobile)

				iBaseOffensiveUnits = (iNumBarbDefenders + 2)/2.0
				
				if( iEra > 1 ) :
					iBaseOffensiveUnits += 1.7*min([iEra,4])
				else :
					iBaseOffensiveUnits -= 2 - iEra

				if( bNewWorldScenario and not bForeignCivCities and self.iNewWorldPolicy > 0 ) :
					iBaseOffensiveUnits -= 3
				else :
					pyNewPlayer.initUnit(iScout,cityX,cityY,1)

				iOffensiveUnits = iBaseOffensiveUnits*self.militaryStrength
				iOffensiveUnits *= (4 + game.getSorenRandNum(6 + iEra, 'BC minor offense'))
				iOffensiveUnits /= 6
				iOffensiveUnits = max([iOffensiveUnits, 1])
				iOffensiveUnits = int( iOffensiveUnits + 0.5 )

				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving %d offensive units (base %d)"%(iOffensiveUnits,iBaseOffensiveUnits))
				for j in range(0,iOffensiveUnits) :
					iType = offensiveUnitList[game.getSorenRandNum( len(offensiveUnitList), 'BC give offensive')]
					attackUnits.append( newPlayer.initUnit(iType,cityX,cityY,UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH ) )

			for attackUnit in attackUnits :
				# Check AI settings
				iniAI = attackUnit.getUnitAIType()
				unitInfo = gc.getUnitInfo(attackUnit.getUnitType())
				if( iniAI == UnitAITypes.UNITAI_CITY_DEFENSE and unitInfo.getUnitAIType(UnitAITypes.UNITAI_ATTACK) ) :
					attackUnit.setUnitAIType( UnitAITypes.UNITAI_ATTACK )
				
				iRand = game.getSorenRandNum( 10, 'BC give offensive')
				if( iRand > 7 and unitInfo.getUnitAIType(UnitAITypes.UNITAI_ATTACK_CITY) ) :
					attackUnit.setUnitAIType( UnitAITypes.UNITAI_ATTACK_CITY )
				elif( iRand > 6 and unitInfo.getUnitAIType(UnitAITypes.UNITAI_ATTACK) ) :
					attackUnit.setUnitAIType( UnitAITypes.UNITAI_ATTACK )

				if( iEra > 0 ) :
					attackUnit.changeExperience(iEra + game.getSorenRandNum(2 + iEra,'BarbarianCiv: unit experience'), -1, False, False, False)

		if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - new minor starting with %d military units"%(newPlayer.getNumMilitaryUnits()))
		
		if( self.LOG_BARB_SETTLE ) :
			bodStr1 = localText.getText("TXT_KEY_BARBCIV_WORD_SPREADS", ())
			bodStr2 = localText.getText("TXT_KEY_BARBCIV_FORM_MINOR", ())%(newPlayer.getCivilizationDescription(0),cityString)

			bLaunchPopup = self.bBarbSettlePopup and (not self.blockPopupInAuto or game.getAIAutoPlay(newPlayer.getID()) == 0)

			if( self.bNotifyOnlyClosePlayers and not game.isDebugMode() ) :

				bSentActiveMess = False

				for iPlayer in range(0,gc.getMAX_CIV_PLAYERS()) :
					bSendMess = False
					if( gc.getPlayer(iPlayer).isAlive() ) :
						if( gc.getPlayer(iPlayer).getTeam() in closeTeams ) :
							bSendMess = True
						elif( capital.plot().isRevealed(gc.getPlayer(iPlayer).getTeam(),False) ) :
							bSendMess = True

						if( bSendMess ) :
							CyInterface().addMessage(iPlayer, false, gc.getDefineINT("EVENT_MESSAGE_TIME"), bodStr1 + " " + bodStr2, None, InterfaceMessageTypes.MESSAGE_TYPE_MAJOR_EVENT, None, gc.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), -1, -1, False, False)
							if( iPlayer == game.getActivePlayer() ) :
								bSentActiveMess = True

				bLaunchPopup = (bLaunchPopup and bSentActiveMess)

			else :
				for iPlayer in range(0,gc.getMAX_CIV_PLAYERS()) :
					if( gc.getPlayer(iPlayer).isAlive() ) :
						CyInterface().addMessage(iPlayer, false, gc.getDefineINT("EVENT_MESSAGE_TIME"), bodStr1 + " " + bodStr2, None, InterfaceMessageTypes.MESSAGE_TYPE_MAJOR_EVENT, None, gc.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), -1, -1, False, False)

			if( bLaunchPopup ) :
				popup = PyPopup.PyPopup(RevDefs.barbSettlePopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
				bodStr = bodStr1 + "\n\n" + bodStr2
				if( self.offerControl ) :
					bodStr += localText.getText("TXT_KEY_BARBCIV_SPREAD_OPT", ())%(leadName)

				popup.setBodyString( bodStr )

				popup.addButton( localText.getText("TXT_KEY_BARBCIV_OK", ()) )
				if( self.offerControl ) :
					popup.addButton(localText.getText("TXT_KEY_BARBCIV_OPT_AM", ())%(leadName))
					self.newPlayerIdx = newPlayer.getID()
					if( self.bCancelAutoForOffer ) :
						game.setAIAutoPlay( 0 )
				popup.launch(bCreateOkButton = False)
				

	def checkMinorCivs( self, iPlayer, bVerbose = True ) :
		# Check minor civs for accomplishments which warrant settling into full civ
		playerI = gc.getPlayer(iPlayer)
		teamI = gc.getTeam(playerI.getTeam())
		if( playerI.isMinorCiv() and playerI.getNumCities() > 0 ) :

			if( not SDTK.sdObjectGetVal( "BarbarianCiv", game, "AlwaysMinorList" ) == None ) :
				if( iPlayer in SDTK.sdObjectGetVal( "BarbarianCiv", game, "AlwaysMinorList" ) ) :
					# Keep scenario minors defined in RevDefs as always minor
					return

			if( SDTK.sdObjectExists( 'BarbarianCiv', playerI ) and SDTK.sdObjectGetVal( "BarbarianCiv", playerI, "SetupComplete" ) == 0 ) :
				# Check for accomplishments
				#if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Checking minor civ %s for accomplishments"%(playerI.getCivilizationDescription(0)))

				hasMetList = list()
				hasMetWithMinorsList = list()
				for idx in range(0,gc.getMAX_CIV_TEAMS()) :
					if( teamI.isAlive() and teamI.isHasMet(idx) ) :
						hasMetWithMinorsList.append(idx)
						if( not gc.getTeam(idx).isMinorCiv() ) :
							hasMetList.append(idx)
						elif( gc.getGame().isOption(GameOptionTypes.GAMEOPTION_START_AS_MINORS) ) :
							otherPlayer = gc.getPlayer(gc.getTeam(idx).getLeaderID())
							otherCapital = otherPlayer.getCapitalCity()
							
							if( otherCapital != None and not otherCapital.isNone() ) :
								if( otherCapital.getOriginalOwner() == otherPlayer.getID() ) :
									if( otherCapital.getGameTurnFounded() == game.getStartTurn() ) :
										hasMetList.append(idx)
						

				if( len(hasMetList) < self.minFullContacts ) :
					return
				if( len(hasMetWithMinorsList) < self.minContacts ) :
					return

				if( playerI.getNumCities() > 1 ) :
					otherFounder = -1
					for city in PyPlayer(iPlayer).getCityList() :
						if( city.GetCy().getGameTurnAcquired() > SDTK.sdObjectGetVal('BarbarianCiv',playerI,'SpawnTurn') and city.GetCy().getPreviousOwner() >= 0 and not city.GetCy().getPreviousOwner() == iPlayer ) :
							otherFounder = city.GetCy().getPreviousOwner()
							break
					if( otherFounder >= 0 and not otherFounder == iPlayer ) :
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Minor civ %s has two cities, conquered one from %d"%(playerI.getCivilizationDescription(0),otherFounder))
						if( otherFounder == gc.getBARBARIAN_PLAYER() ) :
							otherFounder = None
						self.settleMinorCiv( iPlayer, style = "Military", iAttackPlayer = otherFounder )
						return

				if( playerI.getCitiesLost() > 0 ) :
					attacker = None
					for jdx in range(0,gc.getMAX_CIV_PLAYERS()) :
						for city in PyPlayer(jdx).getCityList() :
							if( city.GetCy().getPreviousOwner() == iPlayer ) :
								attacker = jdx
								break
						if( not attacker == None ) :
							break

					if( self.LOG_DEBUG and bVerbose ) :
						mess = ""
						if( not attacker == None ) : mess += "to Player %d"%(attacker)
						CvUtil.pyPrint("  BC - Minor civ %s lost a city %s"%(playerI.getCivilizationDescription(0),mess))
					self.settleMinorCiv( iPlayer, style = "Military", iAttackPlayer = attacker )
					return

				if( game.getGameTurn() - SDTK.sdObjectGetVal('BarbarianCiv',playerI,'SpawnTurn') < 10 ) :
					return

				if( playerI.getNumCities() > 3 ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Minor civ %s has four cities"%(playerI.getCivilizationDescription(0)))
					self.settleMinorCiv( iPlayer )
					return

				iSettleMilUnits = int( self.militaryStrength*( playerI.getNumCities()*gc.getHandicapInfo(game.getHandicapType()).getBarbarianInitialDefenders() + 2.0*(3 + min([3,playerI.getCurrentEra()])) ) )
				if( playerI.getNumMilitaryUnits() > iSettleMilUnits ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Minor civ %s has huge miltary, %d units"%(playerI.getCivilizationDescription(0),playerI.getNumMilitaryUnits()))
					self.settleMinorCiv( iPlayer, style = "Military" )
					return

				if( playerI.getTotalPopulation() > min([self.minPopulation + game.getCurrentEra() + 1,5]) + playerI.getNumCities() ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Minor civ %s has enough pop %d from %d cities"%(playerI.getCivilizationDescription(0),playerI.getTotalPopulation(),playerI.getNumCities()))
					self.settleMinorCiv( iPlayer )
					return

				if( playerI.getWondersScore() > 9 ) : # Palace seems to be worth 5
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Minor civ %s has built a wonder"%(playerI.getCivilizationDescription(0)))
					self.settleMinorCiv( iPlayer, style = "Builder" )
					return

				if( playerI.countTotalCulture() > gc.getCultureLevelInfo(4).getSpeedThreshold(game.getGameSpeedType()) ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Minor civ %s has reached refined total culture of %d"%(playerI.getCivilizationDescription(0),playerI.countTotalCulture()))
					self.settleMinorCiv( iPlayer, style = "Builder" )
					return

				for relIdx in range(0,gc.getNumReligionInfos()) :
					if( playerI.hasHolyCity(relIdx) ) :
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Minor civ %s has founded a religion"%(playerI.getCivilizationDescription(0)))
						self.settleMinorCiv( iPlayer )
						return

	def settleMinorCiv( self, iPlayer, style = None, iAttackPlayer = None, bVerbose = True ) :
		# Turn a minor BarbCiv into a full civ, give more bonuses to launch into the world
		newPlayer = gc.getPlayer(iPlayer)
		pyNewPlayer = PyPlayer(iPlayer)

		cityList = pyNewPlayer.getCityList()
		capital = newPlayer.getCapitalCity()
		pyCapital = pyNewPlayer.getCapitalCity()

		[bNewWorldScenario, bNonBarbCivCities, bForeignCivCities, bMajorCivCities, bNonBarbCivUnits] = self.getScenario( capital.area() )

		if( bNewWorldScenario ) :
			if( not bForeignCivCities and self.iNewWorldPolicy > 4 ) :
				if( not bMajorCivCities ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Cancel settle due to New World policy")
					return

			if( self.iNewWorldPolicy > 2 ) :
				if( not bNonBarbCivUnits and not bMajorCivCities ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Cancel settle due to New World policy")
					return
				iOdds = 1000*(1.0 - pow(0.5,1.0/self.iMinorHalfLife))
				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Odds of settling minor are %.2f"%(iOdds/10))
				if( not (iOdds > game.getSorenRandNum(1000,"BC - settle minor")) ) :
					return


			if( self.iNewWorldPolicy == 2 ) :
				iNumNonBarbCivContacts = self.countNonBarbCivContacts( iPlayer )
				iOdds = 1000*(1.0 - pow(0.5,1.0/self.iMinorHalfLife))*pow(iNumNonBarbCivContacts,0.5)
				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Odds of settling minor by contacts are %.2f"%(iOdds/10))
				if( not (iOdds > game.getSorenRandNum(1000,"BC - settle minor")) ) :
					return

		if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Settling minor civ %s into full civ"%(newPlayer.getCivilizationDescription(0)))

		[closePlayers,closeTeams] = self.getCloseCivs( iPlayer, capital.area(), capital.getX(), capital.getY() )

		# Check for defined style
		if( style == None ) :
			# Use pre-determined style
			style = SDTK.sdObjectGetVal('BarbarianCiv', newPlayer, 'BarbStyle')

			if( style == None ) :
				# Pick a style for barbarian organization
				militaryOdds = int( 100*self.RevOpt.getBaseMilitaryOdds() )
				iAgg = CvUtil.findInfoTypeNum(gc.getTraitInfo,gc.getNumTraitInfos(),RevDefs.sXMLAggressive)
				iExp = CvUtil.findInfoTypeNum(gc.getTraitInfo,gc.getNumTraitInfos(),RevDefs.sXMLExpansive)
				if( gc.getLeaderHeadInfo( newPlayer.getLeaderType() ).hasTrait( iAgg ) ) :
					if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - %s is aggressive"%(newPlayer.getName()))
					militaryOdds += 25
				elif( gc.getLeaderHeadInfo( newPlayer.getLeaderType() ).hasTrait( iExp ) ) :
					if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - %s is expansionist"%(newPlayer.getName()))
					militaryOdds += 15
				if( len(closePlayers) == 0 ) :
					if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - New civ is all alone")
					militaryOdds -= 15
				elif( bForeignCivCities ) :
					if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Nearby foreign colonies")
					militaryOdds += 10
				if( not cityList[0].GetCy().getOriginalOwner() == gc.getBARBARIAN_PLAYER() ) :
					if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - New civ's capital not built by barbs")
					militaryOdds += 20
				elif( cityList[0].getCulture() > 100 ) :
					if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - New civ's capital has high barb culture")
					militaryOdds -= 20

				iRand = game.getSorenRandNum(100,'BarbarianCiv: Check barbs')
				if( militaryOdds > iRand ) :
					# Military consolidation
					if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Military buildup, roll: %d > %d"%(militaryOdds,iRand))
					style = 'Military'
					# Pick a more specific military style

				else :
					# Builder consolidation
					if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Builder buildup, roll: %d <= %d"%(militaryOdds,iRand))
					style = 'Builder'

		else :
			if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Builup style forced to %s"%(style))

		if( style == "Military" ) :
			militaryStyle = self.pickMilitaryStyle( iPlayer )
			SDTK.sdObjectSetVal( 'BarbarianCiv', newPlayer, 'BarbStyle', 'Military' )
		elif( style == "Builder" ) :
			SDTK.sdObjectSetVal( 'BarbarianCiv', newPlayer, 'BarbStyle', 'Builder' )

		newTeam = gc.getTeam(newPlayer.getTeam())

		oldCivName = newPlayer.getCivilizationDescription(0)
		leadName = newPlayer.getName()

		if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - %s has organized the %s!  Turn: %d, Year: %d (after %d turns as minor)"%(leadName,oldCivName,game.getElapsedGameTurns(),game.getTurnYear(game.getElapsedGameTurns()),game.getGameTurn()-SDTK.sdObjectGetVal('BarbarianCiv',newPlayer,'SpawnTurn')))

		SDTK.sdObjectSetVal( 'BarbarianCiv', newPlayer, 'SettleTurn', game.getGameTurn() )
		SDTK.sdObjectSetVal( 'BarbarianCiv', newPlayer, 'SetupComplete', 0 )
		SDTK.sdObjectSetVal( 'BarbarianCiv', newPlayer, 'DoSurpriseAttack', False )

		# Tech
		if( style == "Military" ) :
			self.giveTechs( iPlayer, closeTeams, bNewWorldScenario, militaryStyle = militaryStyle )
		elif( style == "Builder" ) :
			self.giveTechs( iPlayer, closeTeams, bNewWorldScenario, bBuilderBonus = True )
		else :
			self.giveTechs( iPlayer, closeTeams, bNewWorldScenario )

		# Units
		[iWorker, iSettler, iScout, iBestDefender, iCounter, iAttack, iMobile, iAttackCity, iAssaultShip] = self.getUnitsForPlayer( iPlayer )
		
		if( style == "Military" ) :
			newPlayer.setFreeUnitCountdown( 20 )
		elif( style == "Builder" ) :
			newPlayer.setFreeUnitCountdown( 12 )

		# Buildings
# Rise of Mankind 2.61 building additions		
		#[iLibrary, iGranary, iBarracks, iMarket, iWalls, iLighthouse, iForge, iMonument] = self.getBuildingsForPlayer( iPlayer )
		[iBazaar, iSchoolOfScribes, iLibrary, iGranary, iBarracks, iMarket, iWalls, iLighthouse, iForge, iMonument] = self.getBuildingsForPlayer( iPlayer )
# Rise of Mankind 2.61 end
		
		# Pickup nearby barb cities
		iNumBarbDefenders = gc.getHandicapInfo( game.getHandicapType() ).getBarbarianInitialDefenders()
		pyBarbPlayer = PyPlayer( gc.getBARBARIAN_PLAYER() )
		barbCityList = pyBarbPlayer.getCityList()

		for barbCity in barbCityList :
			pBarbCity = barbCity.GetCy()

			if( pBarbCity.area().getID() == capital.area().getID() ) :
				if( pBarbCity.getPreviousOwner() == -1 ) :
					iDist = plotDistance( capital.getX(), capital.getY(), pBarbCity.getX(), pBarbCity.getY())
					iDist2 = gc.getMap().calculatePathDistance( capital.plot(), pBarbCity.plot() )
					iBarbCityRand = game.getSorenRandNum(iDist*iDist, "Give barb city")
					if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Distance to barb city %s is %d (path %d, rand %d"%(pBarbCity.getName(),iDist,iDist2,iBarbCityRand))

					if( iDist < 10 ) :
						if( 20 > iBarbCityRand ) :
							pBarbCityPlot = pBarbCity.plot()
							if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Adding extra barb city %s at %d, %d"%(pBarbCity.getName(),pBarbCity.getX(),pBarbCity.getY()))
							pBarbCityPlot.setOwner( newPlayer.getID() )
							self.setupFormerBarbCity(pBarbCityPlot.getPlotCity(), newPlayer.getID(), iBestDefender, int(iNumBarbDefenders*self.militaryStrength + 0.5))

		hasMetList = list()
		for idx in range(0,gc.getMAX_CIV_TEAMS()) :
			if( newTeam.isHasMet(idx) and not gc.getTeam(idx).isMinorCiv() ) :
				hasMetList.append(idx)

		if( style == "Military" ) :

			# Set probable army composition by type
			if( militaryStyle == "viking" ) :
				offensiveUnitList = [iCounter,iCounter,iCounter,iCounter,iAttack,iAttack,iAttack,iAttackCity]
				iReinforcementType = iCounter
			elif( militaryStyle == "horse" ) :
				offensiveUnitList = [iCounter,iAttack,iMobile,iMobile,iMobile,iMobile]
				iReinforcementType = iMobile
			elif( militaryStyle == "chariot" ) :
				offensiveUnitList = [iCounter,iCounter,iAttack,iMobile,iMobile,iMobile]
				iReinforcementType = iMobile
			else : # "balance"
				offensiveUnitList = [iCounter,iCounter,iAttack,iAttack,iAttack,iMobile,iAttackCity]
				iReinforcementType = iAttack

			SDTK.sdObjectSetVal( 'BarbarianCiv', newPlayer, 'iReinforcementType', iReinforcementType )

		iEra = newPlayer.getCurrentEra()
		
		# Great person
		iProphet = CvUtil.findInfoTypeNum(gc.getUnitInfo,gc.getNumUnitInfos(),RevDefs.sXMLProphet)
		iArtist = CvUtil.findInfoTypeNum(gc.getUnitInfo,gc.getNumUnitInfos(),RevDefs.sXMLArtist)
		iMerchant = CvUtil.findInfoTypeNum(gc.getUnitInfo,gc.getNumUnitInfos(),RevDefs.sXMLMerchant)
		iScientist = CvUtil.findInfoTypeNum(gc.getUnitInfo,gc.getNumUnitInfos(),RevDefs.sXMLScientist)
		iEngineer = CvUtil.findInfoTypeNum(gc.getUnitInfo,gc.getNumUnitInfos(),RevDefs.sXMLEngineer)
		iGeneral = CvUtil.findInfoTypeNum(gc.getUnitInfo,gc.getNumUnitInfos(),RevDefs.sXMLGeneral)

		if( style == "Military" ) :
			gpList = [iProphet,iProphet,iScientist,iGeneral,iGeneral,iGeneral]
		else :
			gpList = [iProphet,iProphet,iArtist,iMerchant,iScientist,iScientist,iEngineer,iEngineer]

		gpType = gpList[game.getSorenRandNum(len(gpList),'BarbarianCiv: pick leader')]
		newPlayer.initUnit(gpType, capital.getX(), capital.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH )
		if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - GP type %s"%(PyInfo.UnitInfo(gpType).getDescription()))

		# Gold
		newPlayer.changeGold( 80 + 30*iEra + game.getSorenRandNum(100,'BarbarianCiv: give money') )

		# Put stuff in cities
		cityList = pyNewPlayer.getCityList()
		for i,city in enumerate(cityList) :
			pCity = city.GetCy()
			cityX = city.getX()
			cityY = city.getY()

			if (not bNewWorldScenario or self.iNewWorldPolicy == 0 ) :
				pCity.setPopulation(min([pCity.getPopulation()+2,pCity.getHighestPopulation()]))
			if( pCity.getPopulation() < 3 and (bNonBarbCivUnits or self.iNewWorldPolicy == 0) ) :
				pCity.setPopulation(3)
			

			# Units
			if( city.getPopulation() > 3 and (bNonBarbCivUnits or self.iNewWorldPolicy == 0) ) :
				if( not iBestDefender == UnitTypes.NO_UNIT ) : pyNewPlayer.initUnit(iBestDefender,cityX,cityY,int(1*self.militaryStrength + 0.5))

			if( style == "Military" ) :
				baseUnits = max([iNumBarbDefenders - i + iEra - 1,0])
				baseUnits -= newPlayer.getNumMilitaryUnits()/(4.0 + iNumBarbDefenders)
				extraUnits = 1 + 2.5*iEra
				if( not bNonBarbCivUnits and self.iNewWorldPolicy > 0 ) :
					extraUnits -= 1
				if( (len(cityList) == 1 and bNonBarbCivUnits) or (i == 0 and bForeignCivCities) ) :
					extraUnits += 2
				extraUnits += max([-1,(game.getHandicapType() - 3)/2])
				extraUnits = int(max([0,extraUnits]))

				iOffensiveUnits = max([0,baseUnits]) + game.getSorenRandNum(extraUnits, "BC - Num off units")
				iOffensiveUnits = int( self.militaryStrength*iOffensiveUnits + 0.5 )

				attackUnits = list()
				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving minor %d offensive units in %s"%(iOffensiveUnits,pCity.getName()))
				for j in range(0,iOffensiveUnits) :
					iType = offensiveUnitList[game.getSorenRandNum( len(offensiveUnitList), 'BC give offensive')]
					attackUnits.append( newPlayer.initUnit(iType,cityX,cityY,UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH) )

				for attackUnit in attackUnits :
					# Check AI settings
					iniAI = attackUnit.getUnitAIType()
					unitInfo = gc.getUnitInfo(attackUnit.getUnitType())
					if( iniAI == UnitAITypes.UNITAI_CITY_DEFENSE and unitInfo.getUnitAIType(UnitAITypes.UNITAI_ATTACK) ) :
						attackUnit.setUnitAIType( UnitAITypes.UNITAI_ATTACK )
					
					iRand = game.getSorenRandNum( 10, 'BC give offensive')
					if( iRand > 5 and unitInfo.getUnitAIType(UnitAITypes.UNITAI_ATTACK_CITY) ) :
						if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Setting unit AI for %s (%d,%d) to ATTACK_CITY"%(attackUnit.getName(),attackUnit.getX(),attackUnit.getY()))
						attackUnit.setUnitAIType( UnitAITypes.UNITAI_ATTACK_CITY )
					elif( iRand > 6 and unitInfo.getUnitAIType(UnitAITypes.UNITAI_ATTACK) ) :
						attackUnit.setUnitAIType( UnitAITypes.UNITAI_ATTACK )

				if( pCity.isCoastal(10) and not iAssaultShip == UnitTypes.NO_UNIT ) :
					if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Placing boat in %s"%(city.getName()))
					pyNewPlayer.initUnit(iAssaultShip,city.getX(),city.getY(),1)

			elif( style == "Builder" ) :
				if( city.getPopulation() > 4 and (bNonBarbCivUnits or self.iNewWorldPolicy == 0) ) :
					pyNewPlayer.initUnit(iCounter,cityX,cityY,int(2*self.militaryStrength + 0.5))

			# Buildings
			if( city.canConstruct( iGranary ) and (not bNewWorldScenario or bForeignCivCities or self.iNewWorldPolicy == 0) ) :
				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iGranary).getDescription(),city.getName()))
				city.setNumRealBuildingIdx(iGranary,1)

			if( city.canConstruct( iMonument ) and (not bNewWorldScenario or self.iNewWorldPolicy == 0) ) :
				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iMonument).getDescription(),city.getName()))
				city.setNumRealBuildingIdx(iMonument,1)
# Rise of Mankind 2.6 building additions
			if( city.canConstruct( iSchoolOfScribes ) and (not bNewWorldScenario or bForeignCivCities or self.iNewWorldPolicy == 0) ) :
				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iSchoolOfScribes).getDescription(),city.getName()))
				city.setNumRealBuildingIdx(iSchoolOfScribes,1)
# Rise of Mankind 2.6 end

			if( style == "Military" ) :
				if( city.canConstruct( iBarracks ) ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iBarracks).getDescription(),city.getName()))
					city.setNumRealBuildingIdx(iBarracks,1)
				if( iEra > 1 and city.canConstruct( iLibrary ) ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iLibrary).getDescription(),city.getName()))
					city.setNumRealBuildingIdx(iLibrary,1)
				if( iEra > 2 and city.canConstruct( iForge ) ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iForge).getDescription(),city.getName()))
					city.setNumRealBuildingIdx(iForge,1)

			elif( style == "Builder" ) :
				if( city.canConstruct( iLibrary ) ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iLibrary).getDescription(),city.getName()))
					city.setNumRealBuildingIdx(iLibrary,1)
				if( city.canConstruct( iLighthouse ) ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iLighthouse).getDescription(),city.getName()))
					city.setNumRealBuildingIdx(iLighthouse,1)
				if( city.canConstruct( iGranary ) and (not bNewWorldScenario or bNonBarbCivUnits or self.iNewWorldPolicy == 0) ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iGranary).getDescription(),city.getName()))
					city.setNumRealBuildingIdx(iGranary,1)
				if( city.canConstruct( iForge ) ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iForge).getDescription(),city.getName()))
					city.setNumRealBuildingIdx(iForge,1)
# Rise of Mankind 2.6 building additions
				if( city.canConstruct( iBazaar ) ) :
					if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iBazaar).getDescription(),city.getName()))
					city.setNumRealBuildingIdx(iBazaar,1)
# Rise of Mankind 2.6 end

			# Culture
			iTotCul = pCity.countTotalCultureTimes100()/100
			if( style == "Builder" ) :
				iTotCul += 25

			if( iTotCul > 0 ) :
				iTotCul += pCity.getCulture( gc.getBARBARIAN_PLAYER() )
				iPlotCul = pCity.plot().getCulture( gc.getBARBARIAN_PLAYER() )
				iPlotCul += pCity.plot().getCulture(pCity.getOriginalOwner())
				RevUtils.giveCityCulture( pCity, newPlayer.getID(), iTotCul, iPlotCul, False, True )
				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Culture of %s set to %d"%(city.getName(),city.getCulture()))

		# Extras for capital
		if( not bNewWorldScenario or bForeignCivCities or self.iNewWorldPolicy == 0 ) :
			pyNewPlayer.initUnit(iWorker,capital.getX(),capital.getY(),1)
		if( iEra > 1 ) :
			pyNewPlayer.initUnit(iWorker,capital.getX(),capital.getY(),1)
		if( pyCapital.canConstruct( iWalls ) and (len(closeTeams) > 0 or bNonBarbCivUnits) ) :
			if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iWalls).getDescription(),capital.getName()))
			pyCapital.setNumRealBuildingIdx(iWalls,1)
		if( pyCapital.canConstruct( iBarracks ) ) :
			if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iBarracks).getDescription(),capital.getName()))
			pyCapital.setNumRealBuildingIdx(iBarracks,1)
		if( pyCapital.canConstruct( iForge ) ) :
			if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iForge).getDescription(),capital.getName()))
			pyCapital.setNumRealBuildingIdx(iForge,1)
		if( pyCapital.canConstruct( iGranary ) and (not bNewWorldScenario or bForeignCivCities or self.iNewWorldPolicy == 0) ) :
			if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iGranary).getDescription(),capital.getName()))
			pyCapital.setNumRealBuildingIdx(iGranary,1)
		if( iEra > 0 and pyCapital.canConstruct( iLibrary ) ) :
			if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iLibrary).getDescription(),capital.getName()))
			pyCapital.setNumRealBuildingIdx(iLibrary,1)
		if( style == "Builder" ) : # Builder
			pyNewPlayer.initUnit(iWorker,capital.getX(),capital.getY(),1)
			if( len(closeTeams) > 1 or self.militaryStrength > 1.5 ) :
				pyNewPlayer.initUnit(iBestDefender,capital.getX(),capital.getY(),1)
				pyNewPlayer.initUnit(iCounter,capital.getX(),capital.getY(),int(1*self.militaryStrength + 0.5))
				if( not bNewWorldScenario or bForeignCivCities or self.iNewWorldPolicy == 0 ) :
					pyNewPlayer.initUnit(iSettler,capital.getX(),capital.getY(),1)
					pyNewPlayer.initUnit(iCounter,capital.getX(),capital.getY(),1)
			if( pyCapital.canConstruct( iMarket ) ) :
				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Constructing %s in %s"%(PyInfo.BuildingInfo(iMarket).getDescription(),capital.getName()))
				pyCapital.setNumRealBuildingIdx(iMarket,1)
			if( newPlayer.getCurrentEra() > 1 and (not bNewWorldScenario or bForeignCivCities or self.iNewWorldPolicy == 0) ) :
				pyNewPlayer.initUnit(iWorker,capital.getX(),capital.getY(),1)
				pyNewPlayer.initUnit(iBestDefender,capital.getX(),capital.getY(),1)
		elif( style == "Military" ) :
			if( (len(cityList) == 1 or newPlayer.getCurrentEra() > 2) ) : # Military extra stuff if small or late
				pyNewPlayer.initUnit(iCounter,cityX,cityY,1)
				if( bNonBarbCivUnits or len(closeTeams) > 0 ) :
					pyNewPlayer.initUnit(iMobile,cityX,cityY,1)
					if( not bNewWorldScenario or bForeignCivCities or self.iNewWorldPolicy == 0 ) :
						pyNewPlayer.initUnit(iAttack,cityX,cityY,1)
						if( capital.isCoastal(10) ) :
								if( not iAssaultShip == UnitTypes.NO_UNIT ) :
									if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving new civ a boat")
									pyNewPlayer.initUnit(iAssaultShip,capital.getX(),capital.getY(),1)

			if( militaryStyle == 'horse' or militaryStyle == 'chariot' ) :
				iExtraType = iMobile
			elif( militaryStyle == 'viking' ) :
				iExtraType = iAttack
			else :
				iExtraType = iAttackCity
			if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving extra %s in capital with ATTACK_CITY AI"%(gc.getUnitInfo(iExtraType).getDescription()))
			newPlayer.initUnit(iExtraType,capital.getX(),capital.getY(),UnitAITypes.UNITAI_ATTACK_CITY,DirectionTypes.DIRECTION_SOUTH)
			if( iEra > 1 ) :
				if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - Giving extra %s in capital with ATTACK_CITY AI"%(gc.getUnitInfo(iExtraType).getDescription()))
				newPlayer.initUnit(iExtraType,capital.getX(),capital.getY(),UnitAITypes.UNITAI_ATTACK_CITY,DirectionTypes.DIRECTION_SOUTH)

		# Experience for units
		if( style == "Military" ) :
			bGeneralSet = False

			# Bonuses for mobile units
			mobileUnits = pyNewPlayer.getUnitsOfType( iMobile )
			for unit in mobileUnits :
				if( unit.getGameTurnCreated() == game.getGameTurn() ) :

					if( not bGeneralSet and iMobile == iReinforcementType ) :
						generalUnit = newPlayer.initUnit(iGeneral, unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH )
						if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint( "   BC - General will lead %s %d"%(unit.getName(), unit.getID()))
						generalUnit.lead(unit.getID())
						unit.changeExperience(10, -1, False, False, False)
						if( gc.getUnitInfo(unit.getUnitType()).getUnitAIType(UnitAITypes.UNITAI_ATTACK_CITY) ) :
							unit.setUnitAIType(UnitAITypes.UNITAI_ATTACK_CITY)
						else :
							unit.setUnitAIType(UnitAITypes.UNITAI_ATTACK)
						bGeneralSet = True
					else :
						unit.changeExperience(2 + game.getSorenRandNum(7,'BarbarianCiv: unit experience'), -1, False, False, False)

			# Bonuses for counter units
			if( not iCounter == iMobile ) :
				counterUnits = pyNewPlayer.getUnitsOfType( iCounter )
				iAmphib = None
				if( militaryStyle == 'viking' ) :
					if( self.LOG_DEBUG ) : CvUtil.pyPrint( "   BC - Giving viking units amphibious promotion" )
					try :
						iAmphib = CvUtil.findInfoTypeNum(gc.getPromotionInfo,gc.getNumPromotionInfos(),RevDefs.sXMLAmphibious)
					except :
						if( self.LOG_DEBUG ) : CvUtil.pyPrint( "Error:  couldn't give viking units amphibious promotion" )

				for unit in counterUnits :
					if( unit.getGameTurnCreated() == game.getGameTurn() ) :

						if( not bGeneralSet and iCounter == iReinforcementType ) :
							generalUnit = newPlayer.initUnit(iGeneral, unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH )
							if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint( "   BC - General will lead %s %d"%(unit.getName(), unit.getID()))
							generalUnit.lead(unit.getID())
							unit.changeExperience(10, -1, False, False, False)
							if( gc.getUnitInfo(unit.getUnitType()).getUnitAIType(UnitAITypes.UNITAI_ATTACK_CITY) ) :
								unit.setUnitAIType(UnitAITypes.UNITAI_ATTACK_CITY)
							else :
								unit.setUnitAIType(UnitAITypes.UNITAI_ATTACK)
							bGeneralSet = True
							if( not iAmphib == None and unit.isPromotionValid(iAmphib) ) :
								unit.setHasPromotion(iAmphib, True)

						elif( not iAmphib == None and unit.isPromotionValid(iAmphib) ) :
							unit.setHasPromotion(iAmphib, True)
							unit.changeExperience(2 + game.getSorenRandNum(5,'BarbarianCiv: unit experience'), -1, False, False, False)
						else :
							unit.changeExperience(2 + game.getSorenRandNum(7,'BarbarianCiv: unit experience'), -1, False, False, False)

			# Bonuses for attack units
			if( not iAttack == iMobile and not iAttack == iCounter ) :
				attackUnits = pyNewPlayer.getUnitsOfType( iAttack )
				iPromoRand = 7
				if( bForeignCivCities and len(closeTeams) > 1 ) :
					iPromoRand = 10

				for unit in attackUnits :
					if( unit.getGameTurnCreated() == game.getGameTurn() ) :
						if( not bGeneralSet and iAttack == iReinforcementType ) :
							generalUnit = newPlayer.initUnit(iGeneral, unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH )
							if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint( "   BC - General will lead %s %d"%(unit.getName(), unit.getID()))
							generalUnit.lead(unit.getID())
							unit.changeExperience(iPromoRand, -1, False, False, False)
							if( gc.getUnitInfo(unit.getUnitType()).getUnitAIType(UnitAITypes.UNITAI_ATTACK_CITY) ) :
								unit.setUnitAIType(UnitAITypes.UNITAI_ATTACK_CITY)
							else :
								unit.setUnitAIType(UnitAITypes.UNITAI_ATTACK)
							bGeneralSet = True
						else :
							unit.changeExperience( 2 + game.getSorenRandNum(iPromoRand,'BarbarianCiv: unit experience'), -1, False, False, False )

			# Bonuses for attack city units
			if( not iAttackCity == iAttack and not iAttackCity == iMobile and not iAttackCity == iCounter ) :
				attackCityUnits = pyNewPlayer.getUnitsOfType( iAttackCity )
				for unit in attackCityUnits :
					if( unit.getGameTurnCreated() == game.getGameTurn() ) :
						unit.changeExperience(2 + game.getSorenRandNum(12,'BarbarianCiv: unit experience'), -1, False, False, False)

		# Golden age
		if( style == "Builder" ) :
			newPlayer.changeGoldenAgeTurns( int(1.5*game.goldenAgeLength()) )
		elif( style == "Military" ) :
			newPlayer.changeGoldenAgeTurns( int(1.0*game.goldenAgeLength()) )

		# Convert team
		if( gc.getTeam(newPlayer.getTeam()).isOpenBordersTrading() or not gc.getGame().isOption(GameOptionTypes.GAMEOPTION_START_AS_MINORS) ) :
			newTeam.setIsMinorCiv(False, (style == "Military" and iAttackPlayer == None) )

		# Force new name for this player
		if( not RevInstances.DynamicCivNamesInst == None ) : RevInstances.DynamicCivNamesInst.setNewNameByCivics(iPlayer)
		civName = newPlayer.getCivilizationDescription(0)

		# Spread effects to nearby enemy cities
		for [radius,pPlot] in RevUtils.plotGenerator( capital.plot(), 9 ) :
			if( not pPlot == None and not pPlot.isNone() ) :
				if( pPlot.area().getID() == capital.area().getID() ) :
					pPlotCity = pPlot.getPlotCity()
					if( not pPlotCity == None and not pPlotCity.isNone() ) :
						if( not pPlotCity.getTeam() == newTeam.getID() and not pPlotCity.isBarbarian() ) :
							radius = plotDistance( capital.getX(), capital.getY(), pPlot.getX(), pPlot.getY())
							#radius = gc.getMap().calculatePathDistance(capital.plot(), pPlot)
							if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint( "   BC - Checking city %s for spread at radius %d"%(pPlotCity.getName(),radius))
							if( radius < 9 ) :
								if( pPlotCity.getCultureLevel() < 3 ) :
									if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint( "   BC - Spreading effects to %s"%(pPlotCity.getName()))
									factor = ((4 - pPlotCity.getCultureLevel())*50)/radius
									iGivePlotCult = (factor*pPlot.countTotalCulture())/100
									RevUtils.giveCityCulture( pPlotCity, newPlayer.getID(), 0, iGivePlotCult, overwriteHigher = False, bSilent = False, iPlotBase = 50 )

									if( not RevInstances.RevolutionInst == None ) :
										iRevIndexChange = 200 + pPlotCity.getRevolutionIndex()/4
										iRevIndexChange = (10*iRevIndexChange)/(radius*radius)
										pPlotCity.changeRevolutionIndex( iRevIndexChange )
										if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint( "   BC - Increasing rev index in %s by %d"%(pPlotCity.getName(),iRevIndexChange))


		# Diplomacy
		if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint( "   BC - Doing diplomacy for new civ")

		contactList = list()
		for i in range(0, gc.getMAX_CIV_PLAYERS()) :
			if( newPlayer.canContact( i ) ) :
				contactList.append( i )

		if( style == "Military" and not newPlayer.isMinorCiv() and not newPlayer.isHuman() ) :

			# Declare war on prechosen target
			if( (not iAttackPlayer == None) and iAttackPlayer >= 0 and iAttackPlayer < gc.getMAX_CIV_PLAYERS() ) :
				newPlayer.AI_changeAttitudeExtra(iAttackPlayer, -5)
				pyNewPlayer.getTeam().declareWar(gc.getPlayer(iAttackPlayer).getTeam(), True, WarPlanTypes.WARPLAN_TOTAL)
				if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - New civ declared war on the prechosen %s"%(PyPlayer(iAttackPlayer).getCivilizationName()))

			else :
				localWarChosen = (gc.getTeam(newPlayer.getTeam()).getAnyWarPlanCount(True) > 0)

				area = newPlayer.getCapitalCity().area()

				if( bNewWorldScenario and self.bFierceNatives ) :
					# Fight any foreigners we can declare war on
					for id in contactList :
						playerI = gc.getPlayer(id)
						capitalI = playerI.getCapitalCity()
						if( (not area == None) and (not capitalI == None) ) :
							if( not area.getID() == capitalI.area().getID() and playerI.getNumCities() > 3 ) :
								if( area.getUnitsPerPlayer(id) > 2 or area.getCitiesPerPlayer(id) > 0 ) :
									if( 50 > game.getSorenRandNum(100,'BarbarianCiv: attack colonists') ) :
										newPlayer.AI_changeAttitudeExtra(id, -5)
										pyNewPlayer.getTeam().declareWar(gc.getPlayer(id).getTeam(),True, WarPlanTypes.NO_WARPLAN)
										if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - New civ declared war on the foreign %s"%(PyPlayer(id).getCivilizationName()))
										if( area.getCitiesPerPlayer(id) > 0 ) :
											localWarChosen = True
										if( pyNewPlayer.getTeam().getAnyWarPlanCount(True) > min([newPlayer.getNumCities()+1,4]) ) :
											break

				if( not localWarChosen ) :
					# Give them some boats to go raiding!
					if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - No potential victim civ")
					if( not iAssaultShip == UnitTypes.NO_UNIT ) :
						for i,city in enumerate(cityList) :
							if( city.GetCy().waterArea().getID() == gc.getMap().findBiggestArea(True).getID() ) :
								if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Giving new civ some boats for raiding")
								pyNewPlayer.initUnit(iAssaultShip,city.getX(),city.getY(),3)
								newPlayer.initUnit(iAssaultShip,city.getX(),city.getY(),UnitAITypes.UNITAI_EXPLORE_SEA,DirectionTypes.DIRECTION_SOUTH )
								break

					SDTK.sdObjectSetVal( 'BarbarianCiv', newPlayer, 'DoSurpriseAttack', True )

		warString = ""
		for teamIdx in range(0,gc.getMAX_CIV_TEAMS()) :
			if( gc.getTeam(teamIdx).isAlive() ) :
				if( not gc.getTeam(teamIdx).isMinorCiv() and pyNewPlayer.getTeam().isAtWar(teamIdx) ) :
					warString += "%s, "%(gc.getTeam(teamIdx).getName())
		if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - New civ starting at war with (full only): " + warString)

		if( self.LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("  BC - new civ starting with %d military units"%(newPlayer.getNumMilitaryUnits()))

		SDTK.sdObjectSetVal( 'BarbarianCiv', newPlayer, 'SetupComplete', 1 )

		# Add replay message
		mess = localText.getText("TXT_KEY_BARBCIV_MINOR_SETTLE", ())%(leadName,newPlayer.getCivilizationAdjective(1),civName)
		game.addReplayMessage( ReplayMessageTypes.REPLAY_MESSAGE_MAJOR_EVENT, newPlayer.getID(), mess, capital.getX(), capital.getY(), gc.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"))

		# Announce the barb civ settling
		if( self.LOG_BARB_SETTLE ) :
			bodStr1 = localText.getText("TXT_KEY_BARBCIV_WORD_SPREADS", ())
			bodStr2 = localText.getText("TXT_KEY_BARBCIV_MINOR_SETTLE", ())%(leadName,newPlayer.getCivilizationAdjective(1),civName)

			bLaunchPopup = self.bBarbSettlePopup and (not self.blockPopupInAuto or game.getAIAutoPlay(newPlayer.getID()) == 0)

			if( self.bNotifyOnlyClosePlayers and not game.isDebugMode() ) :

				bSentActiveMess = False

				for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
					if( idx == newPlayer.getID() ) :
						CyInterface().addMessage(idx, false, gc.getDefineINT("EVENT_MESSAGE_TIME"), localText.getText("TXT_KEY_BARBCIV_FULL_CIV", ()), None, InterfaceMessageTypes.MESSAGE_TYPE_MAJOR_EVENT, None, gc.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), -1, -1, False, False)
						continue

					bSendMess = False
					if( gc.getPlayer(idx).isAlive() ) :
						if( gc.getPlayer(idx).getTeam() in closeTeams or idx in contactList ) :
							bSendMess = True
						elif( capital.plot().isRevealed(gc.getPlayer(idx).getTeam(),False) ) :
							bSendMess = True

						if( bSendMess ) :
							CyInterface().addMessage(idx, false, gc.getDefineINT("EVENT_MESSAGE_TIME"), bodStr1 + " " + bodStr2, None, InterfaceMessageTypes.MESSAGE_TYPE_MAJOR_EVENT, None, gc.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), -1, -1, False, False)
							if( idx == game.getActivePlayer() ) :
								bSentActiveMess = True

				bLaunchPopup = (bLaunchPopup and bSentActiveMess)

			else :
				for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
					if( gc.getPlayer(idx).isAlive() ) :
						if( idx == newPlayer.getID() ) :
							CyInterface().addMessage(idx, false, gc.getDefineINT("EVENT_MESSAGE_TIME"), localText.getText("TXT_KEY_BARBCIV_FULL_CIV", ()), None, InterfaceMessageTypes.MESSAGE_TYPE_MAJOR_EVENT, None, gc.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), -1, -1, False, False)
						else :
							CyInterface().addMessage(idx, false, gc.getDefineINT("EVENT_MESSAGE_TIME"), bodStr1 + " " + bodStr2, None, InterfaceMessageTypes.MESSAGE_TYPE_MAJOR_EVENT, None, gc.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), -1, -1, False, False)

			if( bLaunchPopup ) :
				popup = PyPopup.PyPopup(RevDefs.barbSettlePopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
				bodStr = bodStr1 + "\n\n" + bodStr2
				if( self.offerControl ) :
					bodStr += localText.getText("TXT_KEY_BARBCIV_SPREAD_OPT", ())%(leadName)

				if( newPlayer.getID() == game.getActivePlayer() ) :
					popup.setBodyString( localText.getText("TXT_KEY_BARBCIV_FULL_CIV", ()) )
					popup.addButton( localText.getText("TXT_KEY_BARBCIV_OK", ()) )
				else :
					popup.setBodyString( bodStr )
					popup.addButton( localText.getText("TXT_KEY_BARBCIV_OK", ()) )
					if( self.offerControl ) :
						popup.addButton(localText.getText("TXT_KEY_BARBCIV_OPT_AM", ())%(leadName))
						self.newPlayerIdx = newPlayer.getID()
						if( self.bCancelAutoForOffer ) :
							game.setAIAutoPlay( 0 )

				popup.launch(bCreateOkButton = False)

		gc.getMap().verifyUnitValidPlot()

		# Enable only for testing settling of barbcivs
		if( False ) :
			game.setAIAutoPlay( 0 )
			iPrevHuman = game.getActivePlayer()
			RevUtils.changeHuman( newPlayer.getID(), iPrevHuman )


	def onFirstContact( self, argsList, bVerbose = True ) :
		'Contact'
		iTeamX,iHasMetTeamY = argsList

		#if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Team %d has just met team %d"%(iTeamX,iHasMetTeamY))

		teamX = gc.getTeam(iTeamX)

		if( teamX.isMinorCiv() or gc.getTeam(iHasMetTeamY).isMinorCiv() ) :
			return

		if( (teamX.getLeaderID() < 0) or (teamX.getLeaderID() > gc.getMAX_PLAYERS()) ):
			return
		if( (gc.getTeam(iHasMetTeamY).getLeaderID() < 0) or (gc.getTeam(iHasMetTeamY).getLeaderID() > gc.getMAX_PLAYERS()) ):
			return
		playerX = gc.getPlayer(teamX.getLeaderID())
		playerY = gc.getPlayer(gc.getTeam(iHasMetTeamY).getLeaderID())

		if( playerX == None or playerY == None ) :
			#if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Error: didn't find any players")
			return

		if( SDTK.sdObjectExists( 'BarbarianCiv', playerX ) and teamX.AI_getWarPlan(iHasMetTeamY) == WarPlanTypes.NO_WARPLAN ) :
			if( SDTK.sdObjectGetVal( 'BarbarianCiv', playerX, 'BarbStyle' ) == 'Military' and SDTK.sdObjectGetVal( 'BarbarianCiv', playerX, 'SetupComplete' ) > 0 ) :

				iSettleTurn = SDTK.sdObjectGetVal( 'BarbarianCiv', playerX, 'SettleTurn' )

				bNoOtherContacts = True
				for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
					if( playerX.canContact( idx ) and not idx == playerY.getID() ) :
						#if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Barb civ %d can also contact %d, no Viking action"%(iTeamX,idx))
						bNoOtherContacts = False

				if( bNoOtherContacts and SDTK.sdObjectGetVal( 'BarbarianCiv', playerX, 'DoSurpriseAttack' )) :
					# Invade first contact
					if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - The %s (militaristic barb) has just met its first civ, %d"%(playerX.getCivilizationDescription(0),iHasMetTeamY))

					if( not iSettleTurn == None and game.getGameTurn() < (iSettleTurn + self.militaryWindow) ) :
						if( playerX.isHuman() ) :
							CyInterface().addMessage(playerX.getID(), false, gc.getDefineINT("EVENT_MESSAGE_TIME"), localText.getText("TXT_KEY_BARBCIV_HUMAN_DECLARE", ())%(playerY.getCivilizationDescription(0)), None, InterfaceMessageTypes.MESSAGE_TYPE_INFO, None, gc.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), -1, -1, False, False)
							return

						if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - The %s is surprise attacking the %s!!!"%(playerX.getCivilizationDescription(0),playerY.getCivilizationDescription(0)))
						playerX.AI_changeAttitudeExtra(playerY.getID(), -5)
						teamX.AI_setWarPlan(iHasMetTeamY,WarPlanTypes.WARPLAN_TOTAL)
						unitList = PyPlayer(playerX.getID()).getUnitList()

#						for unit in unitList :
#							if( not unit.isNone() ) :
#								#if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - checking out unit %s, id %d"%(unit.getName(),unit.getID()))
#								if( gc.getUnitInfo(unit.getUnitType()).getUnitAIType(UnitAITypes.UNITAI_ASSAULT_SEA) ) :
#									if( unit.isCargo() and not unit.hasCargo() ) :
#										unit.setUnitAIType( UnitAITypes.UNITAI_ASSAULT_SEA )
#										if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Setting %s to assault sea"%(unit.getName()))

				else :
					# Attack colonists or other outsiders
					if( self.bFierceNatives ) :
						if( not iSettleTurn == None and game.getGameTurn() < (iSettleTurn + self.militaryWindow) ) :
							if( (not playerX.getCapitalCity() == None) and (not playerY.getCapitalCity() == None) ) :
								area1 = playerX.getCapitalCity().area()
								area2 = playerY.getCapitalCity().area()
								if( not area1 == None and not area2 == None ) :
									if( playerY.getNumCities() > 3 and not area1.getID() == area2.getID() and area1.getUnitsPerPlayer(playerY.getID()) > 1 ) :
										# PlayerY is not from here, attack the colonizers
										if( playerX.isHuman() ) :
											CyInterface().addMessage(playerX.getID(), false, gc.getDefineINT("EVENT_MESSAGE_TIME"), localText.getText("TXT_KEY_BARBCIV_HUMAN_DECLARE", ())%(playerY.getCivilizationDescription(0)), None, InterfaceMessageTypes.MESSAGE_TYPE_INFO, None, gc.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), -1, -1, False, False)
											return

										playerX.AI_changeAttitudeExtra(playerY.getID(), -5)

										if( teamX.getAnyWarPlanCount(True) < min([playerX.getNumCities()+1,4]) ) :
											if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - The %s attack the foreign %s on contact"%(playerX.getCivilizationDescription(0),playerY.getCivilizationDescription(0)))
											teamX.AI_setWarPlan(iHasMetTeamY,WarPlanTypes.NO_WARPLAN)

	def onCityAcquired( self, argsList ):
		'City Acquired'

		owner,playerType,pCity,bConquest,bTrade = argsList

		if( bConquest ) :
			newOwnerID = pCity.getOwner()
			newOwner = gc.getPlayer(newOwnerID)

			iCityCount = newOwner.getNumCities()
			if( iCityCount < 6 ) :
				if( SDTK.sdObjectExists( 'BarbarianCiv', newOwner ) ) :
					if( SDTK.sdObjectGetVal( 'BarbarianCiv', newOwner, 'BarbStyle' ) == 'Military' and SDTK.sdObjectGetVal( 'BarbarianCiv', newOwner, 'SetupComplete' ) > 0 ) :
						iSettleTurn = SDTK.sdObjectGetVal( 'BarbarianCiv', newOwner, 'SettleTurn' )
						if( not iSettleTurn == None ) :
							if( game.getGameTurn() - iSettleTurn < self.militaryWindow ) :
								
								if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Barbciv Reinforcement: %s capture %s %d turns after settling"%(newOwner.getCivilizationDescription(0),pCity.getName(),game.getGameTurn() - iSettleTurn))
								iReinforcementType = SDTK.sdObjectGetVal( 'BarbarianCiv', newOwner, 'iReinforcementType' )
								
								if( not iReinforcementType == None ) :
									capital = newOwner.getCapitalCity()
									iNumBarbDefenders = gc.getHandicapInfo( game.getHandicapType() ).getBarbarianInitialDefenders()

									for i in range(max([1,2*iNumBarbDefenders - iCityCount/2 + newOwner.getCurrentEra()])) :
										if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - Barbciv Reinforcement: Giving %s an attack city %s in their capital"%(newOwner.getCivilizationDescription(0),PyInfo.UnitInfo(iReinforcementType).getDescription()))
										unit = newOwner.initUnit(iReinforcementType,capital.getX(),capital.getY(),UnitAITypes.UNITAI_ATTACK_CITY,DirectionTypes.DIRECTION_SOUTH)
										unit.changeExperience(2 + game.getSorenRandNum(6,'BarbarianCiv: unit experience'), -1, False, False, False)


########################## Debug functions ##########################################################

def forceBarbCityToMinor( cityName ) :

	pyBarbPlayer = PyPlayer( gc.getBARBARIAN_PLAYER() )
	barbCityList = pyBarbPlayer.getCityList()

	for city in barbCityList :
		pCity = city.GetCy()
		if( cityName in pCity.getName() ) :
			print "Found city %s"%(cityName)

			if( not RevInstances.BarbarianCivInst == None ) : RevInstances.BarbarianCivInst.createMinorCiv( [pCity], true )

			return

	print "Did not find city %s"%(cityName)