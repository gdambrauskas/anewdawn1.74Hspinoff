# ChangePlayer Mod
#
# by jdog5000
# Version 0.71


from CvPythonExtensions import *
import CvUtil
import PyHelpers
import Popup as PyPopup
# --------- Revolution mod -------------
import RevDefs
import RevUtils
import BugCore

# globals
gc = CyGlobalContext()
PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo
game = CyGame()
localText = CyTranslator()
RevOpt = BugCore.game.Revolution

LOG_DEBUG = True

class ChangePlayer :

	def __init__(self, customEM, RevOpt ) :

		print "Initializing ChangePlayer Mod"

		global LOG_DEBUG

		self.LOG_DEBUG = RevOpt.isChangePlayerDebugMode()
		LOG_DEBUG = self.LOG_DEBUG

		self.RevOpt = RevOpt
		self.customEM = customEM

		self.customEM.addEventHandler( "kbdEvent", self.onKbdEvent )

		self.customEM.setPopupHandler( RevDefs.changeCivPopup, ["changeCivPopup",changeCivHandler,self.blankHandler] )
		self.customEM.setPopupHandler( RevDefs.changeHumanPopup, ["changeHumanPopup",changeHumanHandler,self.blankHandler] )
		self.customEM.setPopupHandler( RevDefs.updateGraphicsPopup, ["updateGraphicsPopup",updateGraphicsHandler,self.blankHandler] )

	def removeEventHandlers( self ) :
		print "Removing event handlers from ChangePlayer"

		self.customEM.removeEventHandler( "kbdEvent", self.onKbdEvent )

		self.customEM.setPopupHandler( RevDefs.changeCivPopup, ["changeCivPopup",self.blankHandler,self.blankHandler] )
		self.customEM.setPopupHandler( RevDefs.changeHumanPopup, ["changeHumanPopup",self.blankHandler,self.blankHandler] )
		self.customEM.setPopupHandler( RevDefs.updateGraphicsPopup, ["updateGraphicsPopup",self.blankHandler,self.blankHandler] )

	def blankHandler( self, playerID, netUserData, popupReturn ) :
		# Dummy handler to take the second event for popup
		return


	def onKbdEvent(self, argsList ):
		'keypress handler'
		eventType,key,mx,my,px,py = argsList

		if ( eventType == RevDefs.EventKeyDown and game.getChtLvl() > 0 ):
			theKey=int(key)

			if( theKey == int(InputTypes.KB_P) and self.customEM.bShift and self.customEM.bCtrl ) :
				changeCivPopup( )

			if( theKey == int(InputTypes.KB_L) and self.customEM.bShift and self.customEM.bCtrl ) :
				changeHumanPopup( )

			if( theKey == int(InputTypes.KB_U) and self.customEM.bShift and self.customEM.bCtrl ) :
				updateGraphics( )

def changeCivPopup( ) :
	'Chooser window for switching a players civ'

	popup = PyPopup.PyPopup(RevDefs.changeCivPopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
	popup.setHeaderString( 'Change a civ?' )
	popup.setBodyString( 'Which civ to change, to which civ and leader and what team?' )
	popup.addSeparator()
	#popup.createPythonEditBox( '10', 'Number of turns to turn over to AI', 0)

	popup.createPythonPullDown( 'Switch this civ ...', 1 )
	for i in range(0,gc.getMAX_PLAYERS()) :
		player = PyPlayer(i)
		if( not player.isNone() ) :
			if( player.isAlive() ) :
				popup.addPullDownString( "%s of the %s"%(player.getName(),player.getCivilizationName()), i, 1 )

	activePlayer = gc.getActivePlayer()
	popup.popup.setSelectedPulldownID( activePlayer.getID(), 1 )

	popup.createPythonPullDown( ' ... to this civ', 2 )
	for i in range(0,gc.getNumCivilizationInfos()) :
		newCivInfo = gc.getCivilizationInfo(i)
		popup.addPullDownString( "%s Empire"%(newCivInfo.getAdjective(0)), i, 2 )

	popup.popup.setSelectedPulldownID( activePlayer.getCivilizationType(), 2 )

	popup.createPythonPullDown( ' ... with this leader', 3 )
	for i in range(0,gc.getNumLeaderHeadInfos()) :
		newLeaderInfo = gc.getLeaderHeadInfo(i)
		leaderName = newLeaderInfo.getDescription()
##			leaderName = newLeaderInfo.getLeaderHead()  # this is the full path of the head animation
##			if( leaderName.count('/') > 0 ) :
##				leaderName = leaderName.split('/')[-2]  # hack to get to just name
##				leaderName = leaderName.replace('_',' ')
##				leaderName = leaderName.title()
		#if( LOG_DEBUG ) : CvUtil.pyPrint( "   CP : Leadername %s"%(leaderName) )

		popup.addPullDownString( "%s"%(leaderName), i, 3 )

	popup.popup.setSelectedPulldownID( activePlayer.getLeaderType(), 3 )

	popup.createPythonPullDown( ' ... on this team', 4 )
	popup.addPullDownString( "Keep current team", -1, 4 )  # Team idx of -1 maintains current team setting
	for i in range(0,gc.getMAX_PLAYERS()) :
		player = PyPlayer(i)
		if( not player.isNone() ) :
			if( player.isAlive() ) :
				popup.addPullDownString( "Team with the %s"%(player.getCivilizationName()), i, 4 )

	popup.popup.setSelectedPulldownID( -1, 4 )

	popup.addSeparator()

	popup.addButton('Cancel')

	popup.launch()

def changeCivHandler( playerID, netUserData, popupReturn ) :
	'Handles changeCiv popup'

	if( popupReturn.getButtonClicked() == 0 ):  # if you pressed cancel
		return

	playerIdx = popupReturn.getSelectedPullDownValue( 1 )
	newCivType = popupReturn.getSelectedPullDownValue( 2 )
	newLeaderType = popupReturn.getSelectedPullDownValue( 3 )
	teamIdx = popupReturn.getSelectedPullDownValue( 4 )

	if( (iTeam >= 0) and (iTeam <= gc.getMAX_CIV_TEAMS()) ) :
		playerI = gc.getPlayer(playerIdx)
		if(playerI.isAlive) :
			if( LOG_DEBUG ) : CvUtil.pyPrint( "   CP : You have selected player %d, the %s, on team %d"%(playerIdx, playerI.getCivilizationDescription(0), playerI.getTeam()) )
	if( (iTeam >= 0) and (iTeam <= gc.getMAX_CIV_TEAMS()) ) :
		teamI =  gc.getTeam(teamIdx)
		if( LOG_DEBUG ) : CvUtil.pyPrint( "   CP : New team idx is %d"%(teamIdx) )


	#player = gc.getPlayer(playerIdx)
	#game.changePlayer( playerIdx, newCivType, newLeaderType, teamIdx, player.isHuman(), True )
	success = RevUtils.changeCiv( playerIdx, newCivType, newLeaderType, teamIdx )

	if( success ) :
		CyInterface().addImmediateMessage("Player %d has been changed"%(playerIdx),"")
		if( LOG_DEBUG ) : CvUtil.pyPrint( "   CP : Player change completed" )
	else :
		CyInterface().addImmediateMessage("An error occured in changeCiv.","")
		if( LOG_DEBUG ) : CvUtil.pyPrint( "   CP : Error on changeCiv" )
		return


	popup = PyPopup.PyPopup(RevDefs.updateGraphicsPopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
	popup.setBodyString("If you see any unit flags or other graphics which have not updated to the new civ type, press Ctrl+Shift+U to update them.  Note that this will end you turn and (in rare circumstances) flash other parts of the game map on the screen.\n\nYou can also do this now by clicking the top button below now.")
	popup.addButton( "Update now" )
	#popup.launch()

def updateGraphicsHandler( playerID, netUserData, popupReturn ) :

	if( popupReturn.getButtonClicked() == 0 ):  # if you pressed update now
		updateGraphics( )

def updateGraphics( ) :
	# Switch human player around to force a redraw of unit flags
	iHuman = game.getActivePlayer()

	iSwitchTo = -1
	for i in range(0,gc.getMAX_CIV_PLAYERS()) :
		player = PyPlayer(i)
		if( not player.isNone() ) :
			if( not player.isAlive() ) :
				iSwitchTo = i
				break

	if( iSwitchTo < 0 ) :
		iSwitchTo = 1 + game.getSorenRandNum( gc.getMAX_CIV_PLAYERS() - 1, 'ChangePlayer')

	game.setForcedAIAutoPlay(iHuman, 3, true )

	RevUtils.changeHuman( iSwitchTo, iHuman )

	RevUtils.changeHuman( iHuman, iSwitchTo )

	#game.setAIAutoPlay(0)

def changeHumanPopup( bDied = False ) :
	'Chooser window for switching human player'
	popup = PyPopup.PyPopup(RevDefs.changeHumanPopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
	popup.setHeaderString( 'Pick a new civ' )
	if( bDied ) :
		popup.setBodyString( 'Your civ has been eliminated.  Would you like to continue as another leader?\n\n(Note:  Switching to barbs is not recommended except as an experiment)' )
	else :
		popup.setBodyString( 'Which civ would you like to lead?\n\n(Note:  Switching to barbs is not recommended except as an experiment)' )
	popup.addSeparator()
	#popup.createPythonEditBox( '10', 'Number of turns to turn over to AI', 0)

	popup.createPythonPullDown( 'Take control of this civ ...', 1 )
	for i in range(0,gc.getMAX_PLAYERS()) :
		player = PyPlayer(i)
		if( not player.isNone() ) :
			if( player.isAlive() ) :
				popup.addPullDownString( "%s of the %s"%(player.getName(),player.getCivilizationName()), i, 1 )

	activePlayerIdx = gc.getActivePlayer().getID()
	popup.popup.setSelectedPulldownID( activePlayerIdx, 1 )

	popup.addSeparator()

	popup.addButton('Cancel')

	popup.launch()

def changeHumanHandler( playerID, netUserData, popupReturn ) :
	'Handles changeHuman popup'

	if( popupReturn.getButtonClicked() == 0 ):  # if you pressed cancel
		return

	newHumanIdx = popupReturn.getSelectedPullDownValue( 1 )
	newPlayer = gc.getPlayer(newHumanIdx)
	oldHumanIdx = playerID
	oldPlayer = gc.getPlayer(oldHumanIdx)

	if( newHumanIdx == oldHumanIdx ) :
		if( LOG_DEBUG ) : CvUtil.pyPrint( "   CP : You have selected the same civ, no change")
		CyInterface().addImmediateMessage("You retain control of the %s"%(oldPlayer.getCivilizationDescription(0)),"")
		return


	if( LOG_DEBUG ) : CvUtil.pyPrint( "   CP : You have selected player %d, the %s"%(newHumanIdx, newPlayer.getCivilizationDescription(0)) )

	success = RevUtils.changeHuman( newHumanIdx, oldHumanIdx )

	if( success ) :
		if( LOG_DEBUG ) : CvUtil.pyPrint( "   CP : Number of human players is now %d"%(game.getNumHumanPlayers()) )
		if( LOG_DEBUG ) : CvUtil.pyPrint( "   CP : Active player is now %d"%(game.getActivePlayer()) )
##			for i in range(0,gc.getMAX_CIV_PLAYERS()) :
##				if( LOG_DEBUG ) : CvUtil.pyPrint( "   CP : Player %d is human %d"%(i,gc.getPlayer(i).isHuman()))
		CyInterface().addImmediateMessage("You now control the %s"%(newPlayer.getCivilizationDescription(0)),"")
	else :
		if( LOG_DEBUG ) : CvUtil.pyPrint( "   CP : Error occured, number of human players is now %d"%(game.getNumHumanPlayers()) )
		CyInterface().addImmediateMessage("An error occured in changeHuman ...","")

##def changeCiv( playerIdx, newCivType, newLeaderType, teamIdx = -1 ) :
##	# Changes specified players civ, leader and/or team
##	# Does not change isHuman setting
##
##	player = gc.getPlayer(playerIdx)
##	success = game.changePlayer( playerIdx, newCivType, newLeaderType, teamIdx, player.isHuman(), True )
##
##	#game.convertUnits( playerIdx )
##
##	return success
##
##def changeHuman( newHumanIdx, oldHumanIdx ) :
##
##	newPlayer = gc.getPlayer(newHumanIdx)
##	oldPlayer = gc.getPlayer(oldHumanIdx)
##
##	game.setActivePlayer( newHumanIdx, False )
##	success = game.changePlayer( newHumanIdx, newPlayer.getCivilizationType(), newPlayer.getLeaderType(), -1, True, False )
##	if( success ) :
##		success = game.changePlayer( oldHumanIdx, oldPlayer.getCivilizationType(), oldPlayer.getLeaderType(), -1, False, False )
##
##	return success
