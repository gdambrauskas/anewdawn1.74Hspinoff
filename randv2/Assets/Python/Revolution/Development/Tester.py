# Tester.py
#
# by jdog5000
#
# Provides event debug output and other testing faculties

from CvPythonExtensions import *
import CvUtil
import sys
import Popup as PyPopup
from PyHelpers import PyPlayer, PyInfo
import CvEventManager
# --------- Revolution mod -------------
import RevDefs
import CvUtil
import RevUtils
import RevInstances
import RevolutionInit
import SdToolKitCustom as SDTK
import RevData
import BugCore

# globals
gc = CyGlobalContext()
#PyPlayer = PyHelpers.PyPlayer
#PyInfo = PyHelpers.PyInfo
game = CyGame()
localText = CyTranslator()
RevOpt = BugCore.game.Revolution

class Tester :

	def __init__(self, customEM, RevOpt):

		print "Initializing Tester"

		self.LOG_DEBUG = RevOpt.isRevConfigDebugMode()

		self.iPrevTurn = None
		self.turnPlayers = list()

		if( self.LOG_DEBUG ) : customEM.addEventHandler( 'greatPersonBorn', self.onGreatPersonBorn )
		#if( self.LOG_DEBUG ) : customEM.addEventHandler( 'techAcquired', self.onTechAcquired )
		if( self.LOG_DEBUG ) : customEM.addEventHandler( 'religionFounded', self.onReligionFounded )
		if( self.LOG_DEBUG ) : customEM.addEventHandler( 'changeWar', self.onChangeWar )
		if( self.LOG_DEBUG ) : customEM.addEventHandler( 'cityLost', self.onCityLost )
		customEM.addEventHandler( "kbdEvent", self.onKbdEvent )
		#customEM.addEventHandler( "EndPlayerTurn", self.endTurnPopup )
		customEM.addEventHandler( "BeginPlayerTurn", self.onBeginPlayerTurn )
		customEM.addEventHandler( "OnLoad", self.onGameLoad )

		customEM.setPopupHandler( RevDefs.testerPopup, ["testerPopup",self.testerPopupHandler,self.blankHandler] )
		customEM.setPopupHandler( RevDefs.waitingForPopup, ["waitingForPopup",self.waitingForHandler,self.blankHandler] )
		customEM.setPopupHandler( RevDefs.setNamePopup, ["setNamePopup",self.setNameHandler,self.blankHandler] )
		#customEM.setPopupHandler( RevDefs.newNamePopup, ["newNamePopup",self.newNameHandler,self.blankHandler] )
		#customEM.setPopupHandler( RevDefs.formConfedPopup, ["formConfedPopup", self.formConfedHandler, self.blankHandler] )
		#customEM.setPopupHandler( RevDefs.dissolveConfedPopup, ["dissolveConfedPopup", self.dissolveConfedHandler, self.blankHandler] )
		#customEM.setPopupHandler( RevDefs.civicsPopup, ["civicsPopup", self.civicsHandler, self.blankHandler] )
		customEM.setPopupHandler( RevDefs.scorePopup, ["showScorePopup", self.showScoreHandler, self.blankHandler] )
		#customEM.setPopupHandler( RevDefs.recreateUnitsPopup, ["recreateUnitsPopup", self.recreateUnitsHandler, self.blankHandler] )
		customEM.setPopupHandler( RevDefs.specialMovePopup, ["specialMovePopup", self.specialMoveHandler, self.blankHandler] )

		self.customEM = customEM
		self.RevOpt = RevOpt

	def removeEventHandlers( self ) :
		print "Removing event handlers from Tester"
		
		if( self.LOG_DEBUG ) : self.customEM.removeEventHandler( 'greatPersonBorn', self.onGreatPersonBorn )
		#if( self.LOG_DEBUG ) : self.customEM.removeEventHandler( 'techAcquired', self.onTechAcquired )
		if( self.LOG_DEBUG ) : self.customEM.removeEventHandler( 'religionFounded', self.onReligionFounded )
		if( self.LOG_DEBUG ) : self.customEM.removeEventHandler( 'changeWar', self.onChangeWar )
		if( self.LOG_DEBUG ) : self.customEM.removeEventHandler( 'cityLost', self.onCityLost )
		self.customEM.removeEventHandler( "kbdEvent", self.onKbdEvent )
		#self.customEM.removeEventHandler( "EndPlayerTurn", self.endTurnPopup )
		self.customEM.removeEventHandler( "BeginPlayerTurn", self.onBeginPlayerTurn )
		self.customEM.removeEventHandler( "OnLoad", self.onGameLoad )

		self.customEM.setPopupHandler( RevDefs.testerPopup, ["testerPopup",self.blankHandler,self.blankHandler] )
		self.customEM.setPopupHandler( RevDefs.waitingForPopup, ["waitingForPopup",self.blankHandler,self.blankHandler] )
		self.customEM.setPopupHandler( RevDefs.setNamePopup, ["setNamePopup",self.blankHandler,self.blankHandler] )
		#self.customEM.setPopupHandler( RevDefs.newNamePopup, ["newNamePopup",self.blankHandler,self.blankHandler] )
		#self.customEM.setPopupHandler( RevDefs.formConfedPopup, ["formConfedPopup", self.blankHandler, self.blankHandler] )
		#self.customEM.setPopupHandler( RevDefs.dissolveConfedPopup, ["dissolveConfedPopup", self.blankHandler, self.blankHandler] )
		#self.customEM.setPopupHandler( RevDefs.civicsPopup, ["civicsPopup", self.blankHandler, self.blankHandler] )
		self.customEM.setPopupHandler( RevDefs.scorePopup, ["showScorePopup", self.showScoreHandler, self.blankHandler] )
		#self.customEM.setPopupHandler( RevDefs.recreateUnitsPopup, ["recreateUnitsPopup", self.blankHandler, self.blankHandler] )
		self.customEM.setPopupHandler( RevDefs.specialMovePopup, ["specialMovePopup", self.blankHandler, self.blankHandler] )
	
	def blankHandler( self, playerID, netUserData, popupReturn ) :
		# Dummy handler to take the second event for popup
		return


	def onGreatPersonBorn(self, argsList):
		'Unit Promoted'
		pUnit, iPlayer, pCity = argsList
		player = PyPlayer(iPlayer)
		if pUnit.isNone() or pCity.isNone():
				return

		CvUtil.pyPrint('A %s was born for %s in %s' %(pUnit.getName(), player.getCivilizationName(), pCity.getName()))


	def onTechAcquired(self, argsList):
		'Tech Acquired'

		iTechType, iTeam, iPlayer, bAnnounce = argsList
		# Note that iPlayer may be NULL (-1) and not a refer to a player object

		CvUtil.pyPrint('%s was finished by Team %d'
			%(PyInfo.TechnologyInfo(iTechType).getDescription(), iTeam))

	def onReligionFounded(self, argsList):
		'Religion Founded'

		iReligion, iFounder = argsList
		player = PyPlayer(iFounder)

		CvUtil.pyPrint('Player %d Civilization %s has founded %s'
			%(iFounder, player.getCivilizationName(), gc.getReligionInfo(iReligion).getDescription()))


	def onChangeWar(self, argsList):
		'War Status Changes'
		bIsWar = argsList[0]
		iPlayer = argsList[1]

		iRivalTeam = argsList[2]
		if (bIsWar):
			strStatus = "declared war"
		else:
			strStatus = "declared peace"

		#if( not gc.getPlayer(iPlayer).isMinorCiv() and not gc.getPlayer(iPlayer).isBarbarian() ) :
		#	if( not gc.getTeam(iRivalTeam).isMinorCiv() and not gc.getTeam(iRivalTeam).isBarbarian() ) :
		#CvUtil.pyPrint('Player %d Civilization %s has %s on Team %d'%(iPlayer, gc.getPlayer(iPlayer).getCivilizationDescription(0), strStatus, iRivalTeam))

	def onCityLost(self, argsList):
		'City Lost'
		city = argsList[0]
		player = PyPlayer(city.getOwner())

		CvUtil.pyPrint('City %s was lost by Player %d Civilization %s'
			%(city.getName(), player.getID(), player.getCivilizationName()))

	def onKbdEvent(self, argsList ):
		'keypress handler'
		eventType,key,mx,my,px,py = argsList

		if ( eventType == RevDefs.EventKeyDown ):
			theKey=int(key)

			if( theKey == int(InputTypes.KB_N) and self.customEM.bShift and self.customEM.bCtrl ) :
				self.setName()

			if( theKey == int(InputTypes.KB_E) and self.customEM.bShift and self.customEM.bCtrl ) :
				if (gc.getGame().cheatCodesEnabled()):
					iPlayer = gc.getGame().getActivePlayer() 
					popupInfo = CyPopupInfo() 
					popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON) 
					popupInfo.setText(CyTranslator().getText("TXT_KEY_POPUP_SELECT_EVENT",())) 
					popupInfo.setData1(iPlayer) 
					popupInfo.setOnClickedPythonCallback("selectOneEvent") 
					for i in range(gc.getNumEventTriggerInfos()): 
						trigger = gc.getEventTriggerInfo(i) 
						popupInfo.addPythonButton(str(trigger.getType()), "") 
					popupInfo.addPythonButton(CyTranslator().getText("TXT_KEY_POPUP_SELECT_NEVER_MIND", ()), "") 
					popupInfo.addPopup(iPlayer) 

			if( theKey == int(InputTypes.KB_D) and self.customEM.bShift and self.customEM.bCtrl ) :
				self.showSpawnListPopup()
				pass

			if( theKey == int(InputTypes.KB_C) and self.customEM.bShift and self.customEM.bCtrl ) :
				self.showRevoltDictPopup()
				pass
				
			if( theKey == int(InputTypes.KB_S) and self.customEM.bShift and self.customEM.bCtrl ) :
				#self.showScorePopup()
				self.showRevSuccessPopup()
				
			#if( theKey == int(InputTypes.KB_E) and self.customEM.bShift and self.customEM.bCtrl ) :
			#	self.showSpecialMovePopup()


	def printUnitCity( self ) :

		unit = CyInterface().getHeadSelectedUnit()
		pPlayer = gc.getPlayer(unit.getOwner())
		#homeCity = gc.getCity( unit.getHomeCityID() )

		mess = "%s was built in %s, %d."%(unit.getName(),unit.getHomeCityName(),unit.getHomeCityID())
		CyInterface().addImmediateMessage(mess,"")
		mess = "%s was built in %s."%(unit.getName(),pPlayer.getCity(unit.getHomeCityID()).getName())
		CyInterface().addImmediateMessage(mess,"")


	def testerPopupHandler( self, iPlayerID, netUserData, popupReturn ) :

		CvUtil.pyPrint('Handling tester popup')
		if( game.isPaused() ) :
			CvUtil.pyPrint('Unpausing')
			game.setPausePlayer(-1)


	def endTurnPopup( self, argsList ) :

		iGameTurn, iPlayer = argsList

		playerI = gc.getPlayer(iPlayer)

		if( playerI.isHuman() ) :
			popup = PyPopup.PyPopup(RevDefs.testerPopup, contextType = EventContextTypes.EVENTCONTEXT_ALL)
			popup.setBodyString( "Attempting to pause" )
			#CyInterface().setPausedPopups( True )
			game.setPausePlayer(game.getActivePlayer())
			popup.launch()


	def onBeginPlayerTurn( self, argsList ) :

		iGameTurn, iPlayer = argsList

		#CvUtil.pyPrint("  Turn %d - Player %d's turn"%(iGameTurn,iPlayer))
		
		if( not gc.getTeam(gc.getPlayer(iPlayer).getTeam()).isAtWar(gc.getBARBARIAN_TEAM()) and not gc.getPlayer(iPlayer).isBarbarian() ) :
			CvUtil.pyPrint("  ERROR! Player %d is no longer at war with barbs!"%(iPlayer))
			gc.getTeam(gc.getBARBARIAN_TEAM()).declareWar(gc.getPlayer(iPlayer).getTeam(), False, WarPlanTypes.NO_WARPLAN )
		
		if( self.iPrevTurn == None ) :
			# First turn after loading
			self.iPrevTurn = iGameTurn
			self.turnPlayers = list()
		
		if( iGameTurn > self.iPrevTurn ) :
			
			if( iPlayer > 0 ) :
				# Check if any lower players were skipped
				found = False
				for i in range(0,iPlayer) :
					if( gc.getPlayer(i).isAlive() ) :
						CvUtil.pyPrint("  Error!  Early turn %d - Player %d's turn"%(iGameTurn,iPlayer))
						found = True
						break

				if( not found ) :
					self.iPrevTurn = iGameTurn
					self.turnPlayers = list()
					self.turnPlayers.append(iPlayer)
				else :
					if( iPlayer in self.turnPlayers ) :
						CvUtil.pyPrint("  Error!  Repeated turn %d - Player %d's turn"%(iGameTurn,iPlayer))
					else :
						self.turnPlayers.append(iPlayer)

			else :
				self.iPrevTurn = iGameTurn
				self.turnPlayers = list()
				self.turnPlayers.append(iPlayer)

		else :
			if( iPlayer in self.turnPlayers ) :
				CvUtil.pyPrint("  Error!  Repeated turn %d - Player %d's turn"%(iGameTurn,iPlayer))
			else :
				self.turnPlayers.append(iPlayer)


	def onGameLoad( self, argsList ) :
		# Note, only runs when game is reloaded
		self.iPrevTurn = None
		self.iTurnPlayers = list()
		
	def waitingFor( self ) :

		# This is an attempt to automatically fix the 'waiting for other ...' problem

		popup = PyPopup.PyPopup(RevDefs.waitingForPopup, contextType = EventContextTypes.EVENTCONTEXT_ALL, bDynamic = False)

		humanStr = 'Num humans alive: %d  ever: %d\nHuman civs:\n'%(game.countHumanPlayersAlive(),game.countHumanPlayersEverAlive())
		activeStr = '\nActive civs:\n'
		for i in range(0,gc.getMAX_CIV_PLAYERS()) :
			playerI = gc.getPlayer(i)
			if( playerI.isHuman() ) :
			   humanStr += playerI.getCivilizationDescription(0) + '\n'

			if( playerI.isTurnActive() ) :
			   activeStr += playerI.getCivilizationDescription(0) + '\n'

		bodStr = humanStr + activeStr
		if( game.isPaused() ) :
			bodStr += '\nGame is paused'
		bodStr += '\nAutoPlay set to %d'%(game.getAIAutoPlay(game.getActivePlayer()))
		popup.setBodyString( bodStr )
		

		if( game.countHumanPlayersAlive()==0 ) :
			popup.addButton( 'Set player %d to human'%(game.getActivePlayer()))
			if( game.getAIAutoPlay(game.getActivePlayer()) > 0 ) :
				popup.addButton( 'Set AIAutoPlay to 0' )


		#popup.popup.setTimer( 10000 )
		popup.launch()


	def waitingForHandler( self, iPlayerID, netUserData, popupReturn ) :

		if( popupReturn.getButtonClicked() == 0 ):

			pPlayer = gc.getActivePlayer()
			RevUtils.changeHuman( pPlayer.getID(), -1 )
			self.waitingFor()
		
		elif( popupReturn.getButtonClicked() == 1 ):
			game.setForcedAIAutoPlay(gc.getActivePlayer(), 1, true)

	def setName( self ) :

		popup = PyPopup.PyPopup(RevDefs.setNamePopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
		popup.setBodyString( 'Change name of a civ' )
		popup.addSeparator()

		popup.createPythonEditBox( 'Name', 'New name for player, eg jdog5000', 0)
		popup.createPythonEditBox( 'Desc', 'New name for civ, eg American Empire', 1)
		popup.createPythonEditBox( 'Short', 'New name for civ, eg America', 2)
		popup.createPythonEditBox( 'Adj', 'New name for civ, eg American', 3)

		popup.createPythonPullDown( 'This AI Player', 1 )
		for i in range(0,gc.getMAX_CIV_PLAYERS()) :
			player = PyPlayer(i)
			if( not player.isNone() ) :
				if( player.isAlive() ) :
					popup.addPullDownString( "%s of the %s"%(player.getName(),player.getCivilizationName()), i, 1 )

		popup.addButton('None')
		CvUtil.pyPrint('Launching pick human popup')
		popup.launch()

	def setNameHandler( self, iPlayerID, netUserData, popupReturn ) :

		if( popupReturn.getButtonClicked() == 0 ):
			return

		newName = popupReturn.getEditBoxString(0)
		newCivDesc  = popupReturn.getEditBoxString(1)
		newCivShort = popupReturn.getEditBoxString(2)
		newCivAdj   = popupReturn.getEditBoxString(3)

		iPlayer = popupReturn.getSelectedPullDownValue( 1 )

		pPlayer = gc.getPlayer( iPlayer )

		CvUtil.pyPrint('Name before: %s, key: %s'%(pPlayer.getName(),pPlayer.getNameKey()))

		newStr = newName.encode('latin-1')
		CvUtil.pyPrint('New name: %s'%(newStr))
		pPlayer.setName( newStr )
		CvUtil.pyPrint('Key after: %s, name: %s'%(pPlayer.getNameKey(),pPlayer.getName()))

		CvUtil.pyPrint('Civ before: %s, key: %s'%(pPlayer.getCivilizationDescription(0),pPlayer.getCivilizationDescriptionKey()))

		newDesc  = newCivDesc.encode('latin-1')
		newShort = newCivShort.encode('latin-1')
		newAdj   = newCivAdj.encode('latin-1')
		CvUtil.pyPrint('New name: %s'%(newDesc))
		pPlayer.setCivName( newDesc, newShort, newAdj )
		CvUtil.pyPrint('Civ after: %s, key: %s'%(pPlayer.getCivilizationDescription(0),pPlayer.getCivilizationDescriptionKey()))


	def showCivicsPopup( self ) :

		pPlayer = gc.getActivePlayer()
		bodStr = "%s's civics:\n\n"%(pPlayer.getCivilizationDescription(0))

		for iOption in range(0,gc.getNumCivicOptionInfos()) :

			iCivic = pPlayer.getCivics(iOption)
			civicInfo = gc.getCivicInfo( iCivic )

			bodStr += civicInfo.getDescription() + ':  loc %d, nat %d, switch %d'%(civicInfo.getRevIdxLocal(),civicInfo.getRevIdxNational(),civicInfo.getRevIdxSwitchTo())
			bodStr += '\n'

		popup = PyPopup.PyPopup()
		popup.setBodyString( bodStr )
		popup.launch()

	# def showFormConfedPopup( self ) :

		# popup = PyPopup.PyPopup(RevDefs.formConfedPopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
		# popup.setBodyString( 'Join which teams in confederation?' )
		# popup.addSeparator()

		# popup.createPythonPullDown( 'Team 1', 1 )
		# for i in range(0,gc.getMAX_CIV_TEAMS()) :
			# team = gc.getTeam(i)
			# if( not team.isNone() ) :
				# if( team.isAlive() ) :
					# player = gc.getPlayer( team.getLeaderID() )
					# popup.addPullDownString( "Team with %s"%(player.getCivilizationDescription(0)), i, 1 )

		# popup.createPythonPullDown( 'Team 2', 2 )
		# for i in range(0,gc.getMAX_CIV_TEAMS()) :
			# team = gc.getTeam(i)
			# if( not team.isNone() ) :
				# if( team.isAlive() ) :
					# player = gc.getPlayer( team.getLeaderID() )
					# popup.addPullDownString( "Team with %s"%(player.getCivilizationDescription(0)), i, 2 )


		# popup.addButton('None')
		# popup.addButton('Oops! Go to dissolve confed')
		# popup.launch()

	# def formConfedHandler( self, iPlayerID, netUserData, popupReturn ) :

		# if( popupReturn.getButtonClicked() == 0 ):
			# return

		# if( popupReturn.getButtonClicked() == 1 ):
			# self.showDissolveConfedPopup()
			# return

		# iTeam1 = popupReturn.getSelectedPullDownValue( 1 )
		# iTeam2 = popupReturn.getSelectedPullDownValue( 2 )

		# CvUtil.pyPrint('*** Combining teams %d and %d ...'%(iTeam1,iTeam2))

		# if( iTeam2 == game.getActiveTeam() ) :
			# temp = iTeam1
			# iTeam1 = iTeam2
			# iTeam2 = temp

		# team1 = gc.getTeam(iTeam1)

		# team1.addConfedTeam( iTeam2 )

	# def showDissolveConfedPopup( self ) :

		# popup = PyPopup.PyPopup(RevDefs.dissolveConfedPopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
		# popup.setBodyString( 'Dissolve which confederation?' )
		# popup.addSeparator()

		# popup.createPythonPullDown( 'Team 1', 1 )
		# numConfeds = 0
		# for i in range(0,gc.getMAX_CIV_TEAMS()) :
			# team = gc.getTeam(i)
			# if( not team.isNone() ) :
				# if( team.isAlive() and team.getNumMembers() > 1 ) :
					# player = gc.getPlayer( team.getLeaderID() )
					# popup.addPullDownString( "Team with %s"%(player.getCivilizationDescription(0)), i, 1 )
					# numConfeds += 1

		# if( numConfeds == 0 ) :
			# popup = PyPopup.PyPopup(RevDefs.dissolveConfedPopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
			# popup.setBodyString( 'No confederations to dissolve ...' )

		# popup.addButton('None')
		# popup.addButton('Oops! Go to form confed')
		# popup.launch()


	# def dissolveConfedHandler( self, iPlayerID, netUserData, popupReturn ) :

		# if( popupReturn.getButtonClicked() == 0 ):
			# return

		# if( popupReturn.getButtonClicked() == 1 ):
			# self.showFormConfedPopup()
			# return

		# iTeam = popupReturn.getSelectedPullDownValue( 1 )
		# if( iTeam == None or iTeam < 0 ) :
			# return
		# pTeam = gc.getTeam( iTeam )

		# CvUtil.pyPrint('*** Breaking up team %d'%(iTeam))

		# for i in range(0,gc.getMAX_CIV_PLAYERS()) :
			# pPlayer = gc.getPlayer(i)
			# # WARNING: Not multiplayer compatible
			# if( pTeam.getNumMembers() > 1 and pPlayer.getTeam() == iTeam and not i == game.getActivePlayer() ) :
				# for j in range(0,gc.getMAX_CIV_TEAMS()) :
					# if( gc.getTeam(j).getNumMembers() < 1 ) :
						# pTeam.removeConfedPlayer( i, j )
						# CvUtil.pyPrint('*** Player %d moved to team %d ...'%(i,j))
						# break

		# CvUtil.pyPrint('*** Team %d now has %d members'%(iTeam, pTeam.getNumMembers()))
		
	def showScorePopup( self ) :
	
		popup = PyPopup.PyPopup(RevDefs.scorePopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
		popup.setBodyString( 'Show score for which player?' )
		popup.addSeparator()
		
		popup.createPythonPullDown( 'Player 1', 1 )
		for i in range(0,gc.getMAX_CIV_PLAYERS()) :
			player = gc.getPlayer(i)
			if( not player.isNone() ) :
				if( player.isAlive() ) :
					popup.addPullDownString( "%s"%(player.getCivilizationDescription(0)), i, 1 )

		popup.addButton('None')
		popup.launch()
		
	def showScoreHandler( self, iPlayerID, netUserData, popupReturn ) :

		if( popupReturn.getButtonClicked() == 0 ):
			return
			
		iPlayer = popupReturn.getSelectedPullDownValue( 1 )
		pPlayer = gc.getPlayer( iPlayer )
		
		bodStr = "Score for %s:\n\n"%(pPlayer.getCivilizationDescription(0))
		bodStr += "Land: %d\n"%(pPlayer.getLandScore())
		bodStr += "Pop:  %d\n"%(pPlayer.getPopScore())
		bodStr += "Tech: %d\n"%(pPlayer.getTechScore())
		bodStr += "Wonders: %d\n"%(pPlayer.getWondersScore())
		bodStr += "Total: %d\n"%(game.getPlayerScore(iPlayer))
			
		popup = PyPopup.PyPopup()
		popup.setBodyString( bodStr )
		popup.launch()
		
	# def showRecreateUnitsPopup( self ) :
	
		# popup = PyPopup.PyPopup(RevDefs.recreateUnitsPopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
		# popup.setBodyString( 'Recreate Units for which player?' )
		# popup.addSeparator()
		
		# popup.createPythonPullDown( 'Player 1', 1 )
		# for i in range(0,gc.getMAX_CIV_PLAYERS()) :
			# player = gc.getPlayer(i)
			# if( not player.isNone() ) :
				# if( player.isAlive() ) :
					# popup.addPullDownString( "%s"%(player.getCivilizationDescription(0)), i, 1 )

		# popup.addButton('None')
		# popup.launch()
		
	# def recreateUnitsHandler( self, iPlayerID, netUserData, popupReturn ) :

		# if( popupReturn.getButtonClicked() == 0 ):
			# return
			
		# iPlayer = popupReturn.getSelectedPullDownValue( 1 )
		# pPlayer = gc.getPlayer( iPlayer )
		# pyPlayer = PyPlayer(iPlayer) 
		
		# preUnitList = pyPlayer.getUnitList()
		# recreateList = list()
		
		# for pUnit in preUnitList :
			# recreateList.append([pUnit.getUnitType(),pUnit.getX(),pUnit.getY(),pUnit.getUnitAIType()])
			# pUnit.kill(False,0)
		
		# for [i,unitInfo] in enumerate(recreateList) :
			
			# [type,ix,iy,AItype] = unitInfo
			# CvUtil.pyPrint('  Do recreate for type %d at %d,%d'%(type,ix,iy))
			# pUnit = pPlayer.initUnit( type, ix, iy, UnitAITypes.NO_UNITAI )
			# pUnit.convert( preUnitList[i] )
			# CvUtil.pyPrint('  Created %d at %d,%d'%(pUnit.getUnitType(),pUnit.getX(),pUnit.getY()))
			
	def showSpecialMovePopup( self ) :
	
		popup = PyPopup.PyPopup(RevDefs.specialMovePopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
		popup.setBodyString( 'Special Move Units for which player?' )
		popup.addSeparator()
		
		popup.createPythonPullDown( 'Player 1', 1 )
		for i in range(0,gc.getMAX_CIV_PLAYERS()) :
			player = gc.getPlayer(i)
			if( not player.isNone() ) :
				if( player.isAlive() ) :
					popup.addPullDownString( "%s"%(player.getCivilizationDescription(0)), i, 1 )

		popup.addButton('None')
		popup.launch()
		
	def specialMoveHandler( self, iPlayerID, netUserData, popupReturn ) :

		if( popupReturn.getButtonClicked() == 0 ):
			return
			
		iPlayer = popupReturn.getSelectedPullDownValue( 1 )
		pPlayer = gc.getPlayer( iPlayer )
		
		pPlayer.specialMoveUnits( True )
	
	def showPeaceWithBarbPopup( self ) :
	
		bodStr = "Following players at peace with barbs:\n"
		for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
			playerI = gc.getPlayer(idx)
			if( playerI.isAlive() and not gc.getTeam(playerI.getTeam()).isAtWar(gc.getBARBARIAN_TEAM()) ) :
				bodStr += "%s\n"%(playerI.getCivilizationDescription(0))
		popup = PyPopup.PyPopup()
		popup.setBodyString( bodStr )
		popup.launch()
		
	def showInstancePopup( self ) :
		
		bodStr = ""
		if( RevInstances.RevolutionInst == None ) :
			bodStr += "RevInstances.RevolutionInst in NONE\n"
		else :
			bodStr += "RevInstances.RevolutionInst initialized\n"
		# if( RevolutionInit.RevolutionInst == None ) :
			# bodStr += "RevolutionInit.RevInst is None\n"
		# else :
			# bodStr += "RevolutionInit.RevInst is initialized\n"
		
		popup = PyPopup.PyPopup()
		popup.setBodyString( bodStr )
		popup.launch()

	def showWarPlans( self ) :
		
		bodStr = ""
		for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
			pass
		
		popup = PyPopup.PyPopup()
		popup.setBodyString( bodStr )
		popup.launch()
	
	def showGameOptions( self ) :
		
		bodStr = ""
		if( gc.getGame().isOption(GameOptionTypes.GAMEOPTION_BARBARIAN_CIV) ) :
			bodStr += "BarbCiv is enabled"
			
	def showSpawnListPopup( self ) :
		
		bodStr = ""
		for idx in range(0,gc.getMAX_PLAYERS()):
			bodStr += "%d"%(idx)
			bodStr += ":  "
			for [iPlayer,iRevoltIdx] in RevData.revObjectGetVal( gc.getPlayer(idx), 'SpawnList' ) :
				bodStr += "%d,%d ; "%(iPlayer,iRevoltIdx)
			bodStr += "\n"
		# if( RevolutionInit.RevolutionInst == None ) :
			# bodStr += "RevolutionInit.RevInst is None\n"
		# else :
			# bodStr += "RevolutionInit.RevInst is initialized\n"
		
		popup = PyPopup.PyPopup()
		popup.setBodyString( bodStr )
		popup.launch()
	
	
	def showRevoltDictPopup( self ) :
		
		bodStr = ""
		for idx in range(0,gc.getMAX_PLAYERS()):
			if( gc.getPlayer(idx).isEverAlive() ) :
				ePlayer = idx
				bodStr += "%d "%(idx)
				bodStr += "<color=%d,%d,%d,%d>" %(gc.getPlayer(ePlayer).getPlayerTextColorR(), gc.getPlayer(ePlayer).getPlayerTextColorG(), gc.getPlayer(ePlayer).getPlayerTextColorB(), gc.getPlayer(ePlayer).getPlayerTextColorA())
				bodStr += "%s"%(gc.getPlayer(idx).getCivilizationShortDescription(0))
				if( SDTK.sdObjectExists( 'BarbarianCiv', gc.getPlayer(idx) ) ) :
					bodStr += " BARB (%d)"%(SDTK.sdObjectGetVal( 'BarbarianCiv', gc.getPlayer(idx), "SpawnTurn" ))
				elif( not RevData.revObjectGetVal( gc.getPlayer(idx), 'RevolutionTurn' ) == None ):
					bodStr += " REB (%d)"%(RevData.revObjectGetVal( gc.getPlayer(idx), 'RevolutionTurn' ))
				bodStr += ":  "
				for revData in RevData.revObjectGetVal( gc.getPlayer(idx), 'RevoltDict' ).values() :
					bodStr += "%d,%d ; "%(revData.iRevTurn,revData.dict.get('iRevPlayer',-1))
				bodStr += "</color>"
				bodStr += "\n"
		# if( RevolutionInit.RevolutionInst == None ) :
			# bodStr += "RevolutionInit.RevInst is None\n"
		# else :
			# bodStr += "RevolutionInit.RevInst is initialized\n"
		
		popup = PyPopup.PyPopup()
		popup.setBodyString( bodStr )
		popup.launch()
	
	def showRevSuccessPopup( self ) :
		
		bodStr = ""
		pPlayer = gc.getActivePlayer()
		
		pPyPlayer = PyPlayer( pPlayer.getID() )
		
		for city in pPyPlayer.getCityList() :
			pCity = city.GetCy()
			bodStr += "%s: %d"%(pCity.getName(),pCity.getRevSuccessTimer())
			bodStr += "\n"
		 
		popup = PyPopup.PyPopup()
		popup.setBodyString( bodStr )
		popup.launch()
