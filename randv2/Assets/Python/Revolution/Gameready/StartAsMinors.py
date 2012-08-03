# Start as Minors
#
# by jdog5000
# Version 1.00

from CvPythonExtensions import *
import CvUtil
import PyHelpers
try:
	import cPickle as pickle
except:
	import pickle
import math
# --------- Revolution mod -------------
import RevDefs
import RevData
import SdToolKitCustom as SDTK
import RevInstances
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
RevOpt = None
customEM = None


########################## Event Handling ##########################################################

def init( newCustomEM, RevOptHandle ) :

		global LOG_DEBUG, RevOpt, customEM

		RevOpt = RevOptHandle
		customEM = newCustomEM
		
		print "Initializing Start As Minors"
		
		LOG_DEBUG = RevOpt.isRevDebugMode()

		# Register events
		customEM.addEventHandler( 'EndPlayerTurn', onEndPlayerTurn )
		customEM.addEventHandler( "setPlayerAlive", onSetPlayerAlive )
		customEM.addEventHandler( "techAcquired", onTechAcquired )
		
		# Turn civs to minors at the beginning of the game
		if( gc.getGame().isOption(GameOptionTypes.GAMEOPTION_START_AS_MINORS) and game.getGameTurn() < 2 ) :
			for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
				playerI = gc.getPlayer(idx)
				if( playerI.isAlive() ) :
					onSetPlayerAlive( [idx, playerI.isAlive()] )
		

def removeEventHandlers( ) :
		print "Removing event handlers from StartAsMinors"
		
		customEM.removeEventHandler( 'EndPlayerTurn', onEndPlayerTurn )
		customEM.removeEventHandler( "setPlayerAlive", onSetPlayerAlive )
		customEM.removeEventHandler( "techAcquired", onTechAcquired )


########################## Turn-based events ###############################

def onEndPlayerTurn( argsList ) :
	
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


########################## Diplomatic events ###############################

def onSetPlayerAlive( argsList ) :

	iPlayerID = argsList[0]
	bNewValue = argsList[1]
	
	pPlayer = gc.getPlayer( iPlayerID )
	pTeam = gc.getTeam( pPlayer.getTeam() )
	
	if( bNewValue ) : 
		# Extra check required for exiting to main menu, starting new game
		if( gc.getGame().isOption(GameOptionTypes.GAMEOPTION_START_AS_MINORS) ) :

			if( not pPlayer.isMinorCiv() and not gc.getTeam(pPlayer.getTeam()).isOpenBordersTrading() ) :
				if( LOG_DEBUG ) : CvUtil.pyPrint("SAM - forcing new civ to minor before writing")
				pTeam.setIsMinorCiv(True, False)
				
			if( game.isFinalInitialized() and game.getGameTurn() < 1 and pPlayer.isMinorCiv() ) :
				if( pPlayer.isAlive() and not pPlayer.isBarbarian() ) :
					if( pPlayer.getNumUnits() > 0 and pPlayer.getNumMilitaryUnits() < 2 and not game.isInAdvancedStart() ) :
						warriorClass = CvUtil.findInfoTypeNum(gc.getUnitClassInfo,gc.getNumUnitClassInfos(),RevDefs.sXMLWarrior)
						iWarrior = gc.getCivilizationInfo( pPlayer.getCivilizationType() ).getCivilizationUnits(warriorClass)
						
						startPlot = pPlayer.getStartingPlot()
						
						if( LOG_DEBUG ) : CvUtil.pyPrint("SAM - Civ has %d units"%(pPlayer.getNumUnits()))
						
						if( not startPlot is None and not startPlot.isNone() ) :
							pPlayer.initUnit( iWarrior, startPlot.getX(), startPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH )
							if( LOG_DEBUG ) : CvUtil.pyPrint("SAM - Giving new civ extra warrior for defense")

def onTechAcquired(argsList):
	'Tech Acquired'
	iTechType, iTeam, iPlayer, bAnnounce = argsList
	# Note that iPlayer may be NULL (-1) and not a refer to a player object

	if( (iTeam >= 0) and (iTeam <= gc.getMAX_CIV_TEAMS()) ) :
		teamI = gc.getTeam(iTeam)

		if(teamI.isAlive()) :
			if( teamI.isMinorCiv() and gc.getTechInfo(iTechType).isOpenBordersTrading() ) :
		
				if( SDTK.sdObjectExists( 'BarbarianCiv', game ) ) :
					for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
						playerI = gc.getPlayer(idx)

						if( playerI.isAlive() and (playerI.getTeam() == iTeam ) ) :
							if( idx in SDTK.sdObjectGetVal( "BarbarianCiv", game, "AlwaysMinorList" ) ) :
								# Keep scenario minors defined in RevDefs as always minor
								return
		
				if( gc.getGame().isOption(GameOptionTypes.GAMEOPTION_START_AS_MINORS) ) :
			
					for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
						playerI = gc.getPlayer(idx)
						if( playerI.getTeam() == iTeam and playerI.isAlive() ) :
					
							if( SDTK.sdObjectExists( 'BarbarianCiv', playerI ) ) :
								if( SDTK.sdObjectGetVal( 'BarbarianCiv', playerI, 'SettleTurn' ) == None ) :
									# Use BarbarianCiv logic
									return
								if( LOG_DEBUG ) : CvUtil.pyPrint("SAM - Settled barb civ %s now has Writing"%(playerI.getCivilizationDescription(0)))
			
			
					if( LOG_DEBUG ) : CvUtil.pyPrint("SAM - clearing minor status for team %d with Writing"%(iTeam))
			
					teamI.setIsMinorCiv(False, False)
			
					for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
						playerI = gc.getPlayer(idx)
						if( gc.getPlayer(idx).getTeam() == iTeam and playerI.isAlive() ) :
							if( not RevInstances.DynamicCivNamesInst == None ) : RevInstances.DynamicCivNamesInst.setNewNameByCivics( iPlayer )
					
							CyInterface().addMessage(iPlayer, false, gc.getDefineINT("EVENT_MESSAGE_TIME"), localText.getText("TXT_KEY_BARBCIV_DISCOVER_WRITING", ()), None, InterfaceMessageTypes.MESSAGE_TYPE_MAJOR_EVENT, None, gc.getInfoTypeForString("COLOR_HIGHLIGHT_TEXT"), -1, -1, False, False)

			
					gc.getMap().verifyUnitValidPlot()

