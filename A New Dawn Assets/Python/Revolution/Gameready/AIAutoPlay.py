# AI_AUTO_PLAY_MOD
#
# by jdog5000
# version 2.0

from CvPythonExtensions import *
import CvScreenEnums
import CvTopCivs
import CvUtil
import PyHelpers
import Popup as PyPopup
# --------- Revolution mod -------------
import RevDefs
import SdToolKitCustom
import RevUtils
import BugCore

# globals
gc = CyGlobalContext()
PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo
game = CyGame()
localText = CyTranslator()
RevOpt = BugCore.game.Revolution

class AIAutoPlay :

	def __init__(self, customEM, RevOpt = None ) :

		print "Initializing AIAutoPlay Mod"

		self.LOG_DEBUG = False
		self.SHOW_NEW_LEADER_POPUP = True
		self.blockPopups = True
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
		self.bLaunchedChangeHumanPopup = False
		self.refortify = True
		self.bSaveAllDeaths = True
		self.bEnableNextTurnArray = list()
		self.iSwitchToNextTurn = list()
		self.iSwitchFromNextTurn = list()
		self.bDisableNextTurnArray = list()
		self.bDisableNextTurnArray2 = list()
		self.NumberOfTurns = list()
		self.Voluntary = list()
		self.TurnsToAuto = list()
		for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
			self.bEnableNextTurnArray.append(0)
			self.bDisableNextTurnArray.append(0)
			self.bDisableNextTurnArray2.append(0)
			self.iSwitchToNextTurn.append(-1)
			self.iSwitchFromNextTurn.append(-1)
			self.NumberOfTurns.append(0)
			self.Voluntary.append(0)
			self.TurnsToAuto.append(10)

# Network protocol header

		self.netAutoPlayPopupProtocol = 5656
		self.netAutoPlayChangePlayerProtocol = 5657
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------		
		if( not RevOpt == None ) :
			self.LOG_DEBUG = RevOpt.isRevDebugMode()
			self.SHOW_NEW_LEADER_POPUP = RevOpt.isShowNewLeaderPopup()
			self.blockPopups = RevOpt.isBlockPopups()
			self.refortify = RevOpt.isRefortify()
			self.bSaveAllDeaths = RevOpt.isSaveAllDeaths()

		self.playerID = 0
		self.AutoCounter = 0

		self.AutoTypes={
			0 : localText.getText("TXT_KEY_AIAUTOPLAY_NO", ()),
			1 : localText.getText("TXT_KEY_AIAUTOPLAY_FULLY", ()),
			#2 : localText.getText("TXT_KEY_AIAUTOPLAY_UNIT", ()),
			#3 : localText.getText("TXT_KEY_AIAUTOPLAY_DEBUG", ()),
			}

		self.customEM = customEM

		self.customEM.addEventHandler( "kbdEvent", self.onKbdEvent )
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
		self.customEM.addEventHandler( "ModNetMessage", self.onModNetMessage )
		self.customEM.addEventHandler( "PreEndGameTurn", self.onEndGameTurn ) 
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------		
		self.customEM.addEventHandler( 'BeginPlayerTurn', self.onBeginPlayerTurn )
		self.customEM.addEventHandler( 'EndPlayerTurn', self.onEndPlayerTurn )
		self.customEM.addEventHandler( 'OnLoad', self.onGameLoad )
		self.customEM.addEventHandler( 'GameStart', self.onGameStart )
		self.customEM.addEventHandler( 'victory', self.onVictory )

		self.customEM.setPopupHandler( RevDefs.toAIChooserPopup, ["toAIChooserPopup",self.AIChooserHandler,self.blankHandler] )
		self.customEM.setPopupHandler( RevDefs.abdicatePopup, ["abdicatePopup",self.abdicateHandler,self.blankHandler] )
		self.customEM.setPopupHandler( RevDefs.pickHumanPopup, ["pickHumanPopup",self.pickHumanHandler,self.blankHandler] )

		if( self.blockPopups ) :
			print "Removing some event handlers"
			try :
				self.customEM.removeEventHandler( "cityBuilt", customEM.onCityBuilt )
				self.customEM.addEventHandler( "cityBuilt", self.onCityBuilt )
			except ValueError :
				print "Failed to remove 'onCityBuilt', perhaps not registered"
				self.customEM.setEventHandler( "cityBuilt", self.onCityBuilt )
			
			try :
				self.customEM.removeEventHandler( "BeginGameTurn", customEM.onBeginGameTurn )
				self.customEM.addEventHandler( "BeginGameTurn", self.onBeginGameTurn )
			except ValueError :
				print "Failed to remove 'onBeginGameTurn', perhaps not registered"
				self.customEM.setEventHandler( "BeginGameTurn", self.onBeginGameTurn )


	def removeEventHandlers( self ) :
		print "Removing event handlers from AIAutoPlay"
		
		self.customEM.removeEventHandler( "kbdEvent", self.onKbdEvent )
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
		self.customEM.removeEventHandler( "PreEndGameTurn", self.onEndGameTurn )
		self.customEM.removeEventHandler( "ModNetMessage", self.onModNetMessage )
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------		
		self.customEM.removeEventHandler( 'BeginPlayerTurn', self.onBeginPlayerTurn )
		self.customEM.removeEventHandler( 'EndPlayerTurn', self.onEndPlayerTurn )
		self.customEM.removeEventHandler( 'OnLoad', self.onGameLoad )
		self.customEM.removeEventHandler( 'GameStart', self.onGameStart )
		self.customEM.removeEventHandler( 'victory', self.onVictory )

		self.customEM.setPopupHandler( RevDefs.toAIChooserPopup, ["toAIChooserPopup",self.blankHandler,self.blankHandler] )
		self.customEM.setPopupHandler( RevDefs.abdicatePopup, ["abdicatePopup",self.blankHandler,self.blankHandler] )
		self.customEM.setPopupHandler( RevDefs.pickHumanPopup, ["pickHumanPopup",self.blankHandler,self.blankHandler] )
		
		if( self.blockPopups ) :
			self.customEM.removeEventHandler( "cityBuilt", self.onCityBuilt )
			self.customEM.addEventHandler( "cityBuilt", self.customEM.onCityBuilt )
			
			self.customEM.removeEventHandler( "BeginGameTurn", self.onBeginGameTurn )
			self.customEM.addEventHandler( "BeginGameTurn", self.customEM.onBeginGameTurn )
	
	def blankHandler( self, playerID, netUserData, popupReturn ) :
		# Dummy handler to take the second event for popup
		return


	def onGameStart( self, argsList ) :
		self.onGameLoad([])

	def onGameLoad( self, argsList ) :
		# Init things which require a game object or other game data to exist

		if( not SdToolKitCustom.sdObjectExists( "AIAutoPlay", game ) ) :
			SdToolKitCustom.sdObjectInit( "AIAutoPlay", game, {} )
			SdToolKitCustom.sdObjectSetVal( "AIAutoPlay", game, "bCanCancelAuto", True )
		elif( SdToolKitCustom.sdObjectGetVal( "AIAutoPlay", game, "bCanCancelAuto" ) == None ) :
			SdToolKitCustom.sdObjectSetVal( "AIAutoPlay", game, "bCanCancelAuto", True )

	def onVictory( self, argsList ) :
		self.checkPlayer()
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
		for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
			if(gc.getPlayer(idx).isHuman()):
				game.setAIAutoPlay(idx, 0)
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------		
	def onEndGameTurn( self, argsList ) :
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
		if( game.getAIAutoPlay(game.getActivePlayer()) == 1 ) :
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------			# About to turn off automation
			#SdToolKitCustom.sdObjectSetVal( "AIAutoPlay", game, "bCanCancelAuto", False )
			pass

#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
	def doNewHuman( self, iNewPlayerID, iOldPlayerID ):
	# This still not working properly. Boo.... Close though.
		CyInterface().addImmediateMessage("doNewHuman")
		self.bLaunchedChangeHumanPopup = False
		gc.getPlayer(iOldPlayerID).setNewPlayerAlive( True )
		iSettler = CvUtil.findInfoTypeNum(gc.getUnitInfo,gc.getNumUnitInfos(),'UNIT_SETTLER')
		gc.getPlayer(iOldPlayerID).initUnit( iSettler, 0, 0, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH )
		gc.getPlayer(iOldPlayerID).setFoundedFirstCity( False )
		gc.getPlayer(iOldPlayerID).setIsHuman( True )
	
		toKillPlayer = gc.getPlayer(iOldPlayerID)
		CyInterface().addImmediateMessage("calling changeHuman")
		RevUtils.changeHuman( iNewPlayerID, iOldPlayerID )
	
		if( toKillPlayer.getNumCities() == 0 ) :
			# Kills off the lion in the ice field
			CvUtil.pyPrint("Killing off player %d"%(toKillPlayer.getID()))
			toKillPlayer.killUnits()
			toKillPlayer.setIsHuman(False)
			#success = game.changePlayer( toKillPlayer.getID(), toKillPlayer.getCivilizationType(), toKillPlayer.getLeaderType(), -1, False, False )
			toKillPlayer.setNewPlayerAlive(False)
			toKillPlayer.setFoundedFirstCity(True)
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------   
	def pickHumanHandler( self, iPlayerID, netUserData, popupReturn ) :

		CvUtil.pyPrint('Handling pick human popup')

		if( popupReturn.getButtonClicked() == 0 ):  # if you pressed cancel
			CyInterface().addImmediateMessage("Kill your remaining units if you'd like to see end game screens","")
			return

#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
		toKillPlayer = gc.getPlayer(iPlayerID)
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------   

		newHumanIdx = popupReturn.getSelectedPullDownValue( 1 )
		newPlayer = gc.getPlayer(newHumanIdx)

		# game.setActivePlayer( newHumanIdx, False )
		# newPlayer.setIsHuman(True)

		# CvUtil.pyPrint("You now control the %s"%(newPlayer.getCivilizationDescription(0)))
		# CyInterface().addImmediateMessage("You now control the %s"%(newPlayer.getCivilizationDescription(0)),"")
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
		self.doNewHuman(newHumanIdx, toKillPlayer.getID())
		self.bLaunchedChangeHumanPopup = False
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------   

	def onBeginPlayerTurn( self, argsList ) :
		iGameTurn, iPlayer = argsList
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
		if( game.getAIAutoPlay(iPlayer) == 1 and not gc.getActivePlayer() is None and iPlayer > game.getActivePlayer() and gc.getActivePlayer().isAlive() ) :
			# Forces isHuman checks to come through positive for everything after human players turn

			self.checkPlayer()
			#SdToolKitCustom.sdObjectSetVal( "AIAutoPlay", game, "bCanCancelAuto", False )
			#game.setAIAutoPlay(iPlayer, 0)
		
		elif( self.bSaveAllDeaths ) :
		   if( game.getAIAutoPlay(game.getActivePlayer()) == 0 and not gc.getActivePlayer() is None and not gc.getActivePlayer().isAlive() and iPlayer > game.getActivePlayer() ) :
				self.checkPlayer()
				#SdToolKitCustom.sdObjectSetVal( "AIAutoPlay", game, "bCanCancelAuto", False )
				#game.setAIAutoPlay(iPlayer, 0)
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------   

	def onEndPlayerTurn( self, argsList ) :
		iGameTurn, iPlayer = argsList

		# Can't use isHuman as isHuman has been deactivated by automation
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
		if( self.refortify and iPlayer == game.getActivePlayer() and game.getAIAutoPlay(iPlayer) == 1 ) :
			RevUtils.doRefortify( game.getActivePlayer() )
		
		if( iPlayer == gc.getBARBARIAN_PLAYER() and game.getAIAutoPlay(iPlayer) == 1 ) :
			# About to turn off automation
			#SdToolKitCustom.sdObjectSetVal( "AIAutoPlay", game, "bCanCancelAuto", False )
			#self.checkPlayer()
			pass
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------   

	def checkPlayer( self ) :
		
		pPlayer = gc.getActivePlayer()

#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
		if( not pPlayer is None and not pPlayer.isAlive()) :
			popup = PyPopup.PyPopup(RevDefs.pickHumanPopup,contextType = EventContextTypes.EVENTCONTEXT_ALL, bDynamic = False)
			popup.setHeaderString( localText.getText("TXT_KEY_AIAUTOPLAY_PICK_CIV", ()) )
			popup.setBodyString( localText.getText("TXT_KEY_AIAUTOPLAY_CIV_DIED", ()) )
			popup.addSeparator()

			popup.createPythonPullDown( localText.getText("TXT_KEY_AIAUTOPLAY_TAKE_CONTROL_CIV", ()), 1 )
			for i in range(0,gc.getMAX_CIV_PLAYERS()) :
				player = PyPlayer(i)
				if( not player.isNone() and not i == pPlayer.getID() ) :
					if( player.isAlive() ) :
						popup.addPullDownString( localText.getText("TXT_KEY_AIAUTOPLAY_OF_THE", ())%(player.getName(),player.getCivilizationName()), i, 1 )

			activePlayerIdx = gc.getActivePlayer().getID()
			popup.popup.setSelectedPulldownID( activePlayerIdx, 1 )

			popup.addSeparator()
			self.bLaunchedChangeHumanPopup = True
			popup.addButton( localText.getText("TXT_KEY_AIAUTOPLAY_NONE", ()) )
			CvUtil.pyPrint('Launching pick human popup')
			popup.launch()

		#if( not pPlayer.isHuman() ) :
		#	CvUtil.pyPrint('Returning human player to control of %s'%(pPlayer.getCivilizationDescription(0)))
		#	game.setActivePlayer( pPlayer.getID(), False )
		#	pPlayer.setIsHuman( True )
		#	game.setAIAutoPlay(pPlayer.getID(), 0)

		for idx in range(0,gc.getMAX_CIV_TEAMS()) :
			pPlayer.setEspionageSpendingWeightAgainstTeam(idx, pPlayer.getEspionageSpendingWeightAgainstTeam(idx)/10)
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------   


	def onKbdEvent( self, argsList ) :
		'keypress handler'
		eventType,key,mx,my,px,py = argsList

		if ( eventType == RevDefs.EventKeyDown ):
			theKey=int(key)

			if( theKey == int(InputTypes.KB_X) and self.customEM.bShift and self.customEM.bCtrl ) :
				# Get it?  Shift ... control ... to the AI
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
				if( game.getAIAutoPlay(game.getActivePlayer()) > 0 ) :
					try :
						bCanCancelAuto = SdToolKitCustom.sdObjectGetVal( "AIAutoPlay", game, "bCanCancelAuto" )
						if( bCanCancelAuto is None ) :
							bCanCancelAuto = True
							SdToolKitCustom.sdObjectSetVal( "AIAutoPlay", game, "bCanCancelAuto", True )
					except :
						print "Error!  AIAutoPlay: Can't find bCanCancelAuto, assuming it would be True"
						bCanCancelAuto = True

					if( bCanCancelAuto ) :
						if( self.refortify ) :
							RevUtils.doRefortify( game.getActivePlayer() )
							self.disableMultiCheck(game.getActivePlayer())
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------   
					  
						self.checkPlayer()
				else :
					self.toAIChooser()

			if( theKey == int(InputTypes.KB_M) and self.customEM.bShift and self.customEM.bCtrl ) :
				# Toggle auto moves
				if( self.LOG_DEBUG ) : CyInterface().addImmediateMessage("Moving your units...","")
				#self.playerID = gc.getActivePlayer().getID()
				game.setAIAutoPlay( game.getActivePlayer(), 1 )

			if( theKey == int(InputTypes.KB_O) and self.customEM.bShift and self.customEM.bCtrl ) :
				RevUtils.doRefortify( game.getActivePlayer() )
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
	def doCheckMultiplayerUpdate( self ):
		if( not game.isMultiplayer()):
			return

		for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
			if(self.bEnableNextTurnArray[idx]==1):
				self.bEnableNextTurnArray[idx]=0
				self.abdicate(idx, self.NumberOfTurns[idx], self.Voluntary[idx])
			elif(self.bDisableNextTurnArray[idx]==1):
				self.bDisableNextTurnArray[idx]=0
				self.bDisableNextTurnArray2[idx]=1
			elif(self.bDisableNextTurnArray2[idx]==1):
				self.bDisableNextTurnArray2[idx]=0
				game.setAIAutoPlay(idx, 0)
			if(self.iSwitchToNextTurn[idx]!=-1):
				#self.doNewHuman(self.iSwitchToNextTurn[idx], self.iSwitchFromNextTurn[idx])
				self.iSwitchToNextTurn[idx] = -1
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------   

	def onBeginGameTurn( self, argsList):
		'Called at the beginning of the end of each turn'
		iGameTurn = argsList[0]
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
		if( game.isMultiplayer()):
			self.doCheckMultiplayerUpdate()
		if( game.getAIAutoPlay(game.getActivePlayer()) == 0 ) :
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------   
			CvTopCivs.CvTopCivs().turnChecker(iGameTurn)

	def onCityBuilt(self, argsList):
		'City Built'
		city = argsList[0]
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
		if (city.getOwner() == CyGame().getActivePlayer() and game.getAIAutoPlay(game.getActivePlayer()) == 0 ):
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------   
				self.customEM.onCityBuilt(argsList)
		else :
				try :
					CvUtil.pyPrint('City Built Event: %s' %(city.getName()))
				except :
					CvUtil.pyPrint('City Built Event: Error processing city name' )


	def toAIChooser( self ) :
		'Chooser window for when user switches to AI auto play'

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()
		popupSizeX = 400
		popupSizeY = 250

		popup = PyPopup.PyPopup(RevDefs.toAIChooserPopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
		popup.setPosition((xResolution - popupSizeX )/2, (yResolution-popupSizeY)/2-50)
		popup.setSize(popupSizeX,popupSizeY)
		popup.setHeaderString( localText.getText("TXT_KEY_AIAUTOPLAY_TURN_ON", ()) )
		popup.setBodyString( localText.getText("TXT_KEY_AIAUTOPLAY_TURNS", ()) )
		popup.addSeparator()
		popup.createPythonEditBox( '10', 'Number of turns to turn over to AI', 0)
		popup.setEditBoxMaxCharCount( 4, 2, 0 )

		

		popup.createPythonPullDown( localText.getText("TXT_KEY_AIAUTOPLAY_AUTOMATION", ()), 2 )
		for i in range(0,len(self.AutoTypes)) :
			popup.addPullDownString( self.AutoTypes[i], i, 2 )

		popup.popup.setSelectedPulldownID( 1, 2 )

		#popup.createPythonCheckBoxes( 4, 3 )
		#popup.setPythonCheckBoxText( 0, 'Pause at turn end', 'Pauses automation at the end of the turn, so you can inspect things', 3 )
		#popup.setPythonCheckBoxText( 1, 'Pause every 10', 'Pauses automation every 10 turns', 3 )
		#popup.setPythonCheckBoxText( 2, 'Cancel on war declared', 'Cancel automation if your civ becomes involved in a war', 3 )
		#popup.setPythonCheckBoxText( 3, 'Wake-up on game end', 'If someone (your civ?) wins, automation cancelled', 3 )

		popup.addSeparator()
		popup.addButton("OK")
		popup.addButton(localText.getText("TXT_KEY_AIAUTOPLAY_CANCEL", ()))

		popup.launch(False, PopupStates.POPUPSTATE_IMMEDIATE)

	def AIChooserHandler( self, playerID, netUserData, popupReturn ) :
		'Handles AIChooser popup'
		if( popupReturn.getButtonClicked() == 1 ):  # if you pressed cancel
			return
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
		if(not self.isLocalHumanPlayer(playerID)):
			return
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------   
		numTurns = 0
		self.playerID = playerID
		if( popupReturn.getEditBoxString(0) != '' ) :
			numTurns = int( popupReturn.getEditBoxString(0) )

		autoIdx = popupReturn.getSelectedPullDownValue( 2 )

		if( autoIdx == 0 ) :
			if( self.LOG_DEBUG ) : CyInterface().addImmediateMessage("Clearing all automation","")
			#self.clearAllAutomation( )
		elif( autoIdx == 1 ) :
			if( numTurns > 0 ) :
				if( self.LOG_DEBUG ) : CyInterface().addImmediateMessage("Fully automating for %d turns"%(numTurns),"")
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
				self.abdicateMultiCheck( self.playerID, numTurns = numTurns, voluntary = True )
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------   
		elif( autoIdx == 2 and numTurns > 0 ) :
			if( self.LOG_DEBUG ) : CyInterface().addImmediateMessage("Auto Move","")
			self.setAutoMoves( numTurns )
		elif( autoIdx == 3 and numTurns > 0 ) :
			if( self.LOG_DEBUG ) : CyInterface().addImmediateMessage("Debug Mode","")
			self.setDebugMode( numTurns )

		# How to read out checkboxes?  popupReturn doesn't seem to have a function
		#bitField = popupReturn.getCheckBoxBitfield( 3 )
		#self.bPause =
		#CyInterface().addImmediateMessage("bitfieldsize: %d"%(len(bitField)),"")

#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------
	def isLocalHumanPlayer( self, playerID ) :
		# Determines whether to show popup to active player
		return (gc.getPlayer(playerID).isHuman() or gc.getPlayer(playerID).isHumanDisabled()) and game.getActivePlayer() == playerID

	def disableMultiCheck (self, playerID) :
		if( game.isMultiplayer()):
			self.bDisableNextTurnArray[playerID] = 1
			if(self.isLocalHumanPlayer(playerID)):
				CyMessageControl().sendModNetMessage(self.netAutoPlayPopupProtocol, 0, playerID, gc.getGame().getGameTurn(), 0) 
		else:
			game.setAIAutoPlay(playerID, 0)
	  
	def abdicateMultiCheck (self, playerID, numTurns = -1, voluntary = False) :  
		if( game.isMultiplayer()):
			self.bEnableNextTurnArray[playerID] = 1
			self.bDisableNextTurnArray[playerID] = 0
			self.bDisableNextTurnArray2[playerID] = 0
			
			self.NumberOfTurns[playerID] = numTurns
			self.Voluntary[playerID] = voluntary
			if(playerID == game.getActivePlayer()):
				CyMessageControl().sendModNetMessage(self.netAutoPlayPopupProtocol, 1, playerID, numTurns, voluntary) 
		else:
			self.abdicate(playerID, numTurns, voluntary)

	def abdicateMultiCheckNoMessage (self, playerID, numTurns = -1, voluntary = False) :  
		if( game.isMultiplayer()):
			self.bEnableNextTurnArray[playerID] = 1
			self.bDisableNextTurnArray[playerID] = 0
			self.bDisableNextTurnArray2[playerID] = 0
			
			self.NumberOfTurns[playerID] = numTurns
			self.Voluntary[playerID] = voluntary
		else:
			self.abdicate(playerID, numTurns, voluntary)
	 
	def onModNetMessage( self, argsList) :
		protocol, data1, data2, data3, data4 = argsList
		if protocol == self.netAutoPlayPopupProtocol :
			if(data1==0 and data2 != game.getActivePlayer()):
				if(data3==gc.getGame().getGameTurn()):
					self.bDisableNextTurnArray[data2] = 1
				elif(data3<gc.getGame().getGameTurn()):
					self.bDisableNextTurnArray2[data2] = 1
			elif(data1==1 and data2 != game.getActivePlayer()):
				self.bEnableNextTurnArray[data2] = 1
				self.NumberOfTurns[data2] = data3
				self.TurnsToAuto[data2] = data3
				self.Voluntary[data2] = data4
				
		if protocol == self.netAutoPlayChangePlayerProtocol:
			#if(data2 != game.getActivePlayer() and data3 != game.getActivePlayer()):
			self.doNewHuman(data2, data3)
			#self.iSwitchToNextTurn[data1] = data2
			#self.iSwitchFromNextTurn[data1] = data3
		  
	def abdicate( self, playerID, numTurns = -1, voluntary = False ) :
		'Turn over control to the AI'
		if( numTurns > 0 ) :
			self.TurnsToAuto[playerID] = numTurns

		if( self.TurnsToAuto[playerID] < 1 ) :
			return

		#if( voluntary and self.SHOW_NEW_LEADER_POPUP and game.getActivePlayer()==playerID ) :
		#	popup = PyPopup.PyPopup(RevDefs.abdicatePopup,contextType = EventContextTypes.EVENTCONTEXT_ALL)
		#	popup.setHeaderString( localText.getText("TXT_KEY_AIAUTOPLAY_NEW_LEADER", ()) )
		#	if( voluntary ) :
		#		bodStr = localText.getText("TXT_KEY_AIAUTOPLAY_ABDICATE", ())
		#	else :
		#		bodStr = localText.getText("TXT_KEY_AIAUTOPLAY_USURPATOR", ())
		#	bodStr = bodStr +  localText.getText("TXT_KEY_AIAUTOPLAY_GOOD_NEWS", ()) %(self.TurnsToAuto[playerID])
		#	popup.setBodyString( bodStr )
		#	popup.launch()
		#else :
			if( voluntary ) : SdToolKitCustom.sdObjectSetVal( "AIAutoPlay", game, "bCanCancelAuto", True )
		game.setAIAutoPlay( playerID, self.TurnsToAuto[playerID] )

	def abdicateHandler( self, playerID, netUserData, popupReturn ) :
		# Took all this out and handled it elsewhere
		return

## --------------- All functions below this line are not currently used, may not perform as expected -------------------


	def onChangeWar( self, argsList ) :
		bIsWar = argsList[0]
		iPlayer = argsList[1]
		iRivalTeam = argsList[2]
		if( game.getAIAutoPlay( game.getActivePlayer() ) > 0 ) :
			if( self.bWakeOnWar and bIsWar ) :
				if( iPlayer == self.playerID or iRivalTeam == gc.getPlayer(playerID).getTeam() ) :
					CyInterface().addImmediateMessage("Wake on war","")
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------   
  

