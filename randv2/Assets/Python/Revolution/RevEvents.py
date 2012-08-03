# Events to support Revolution mod
#
# by jdog5000
# Version 1.5

from CvPythonExtensions import *
import CvUtil
import PyHelpers
import Popup as PyPopup
try:
	import cPickle as pickle
except:
	import pickle
import math
# --------- Revolution mod -------------
import RevDefs
import RevData
import SdToolKitCustom
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

# Stores player id's human has reject assimilation overtures from
noAssimilateList = list()

revCultureModifier = 1.0
cityLostModifier = 1.0
cityAcquiredModifier = 1.0
acquiredTurns = 10

endWarsOnDeath = True
allowAssimilation = True
bSmallRevolts = False

LOG_DEBUG = True
centerPopups = False
RevOpt = None
customEM = None


########################## Event Handling ##########################################################

def init( newCustomEM, RevOptHandle ) :

		global revCultureModifier, cityLostModifier, cityAcquiredModifier, acquiredTurns, LOG_DEBUG, centerPopups, RevOpt, customEM
		global endWarsOnDeath, allowAssimilation, bSmallRevolts

		RevOpt = RevOptHandle
		customEM = newCustomEM
		
		print "Initializing RevEvents"
		
		LOG_DEBUG = RevOpt.isRevDebugMode()

		# Config settings
		revCultureModifier = RevOpt.getRevCultureModifier()
		cityLostModifier = RevOpt.getCityLostModifier()
		cityAcquiredModifier = RevOpt.getCityAcquiredModifier()
		acquiredTurns = RevOpt.getAcquiredTurns()
		
		# Controls for event handling
		endWarsOnDeath = RevOpt.isEndWarsOnDeath()
		allowAssimilation = RevOpt.isAllowAssimilation()
		bSmallRevolts = RevOpt.isAllowSmallBarbRevs()
		
		centerPopups = RevOpt.isCenterPopups()

		# Register events
		customEM.addEventHandler( 'EndGameTurn', onEndGameTurn )
		customEM.addEventHandler( 'BeginPlayerTurn', onBeginPlayerTurn )
		customEM.addEventHandler( 'EndPlayerTurn', onEndPlayerTurn )
		customEM.addEventHandler( "setPlayerAlive", onSetPlayerAlive )
		customEM.addEventHandler( "changeWar", onChangeWar )
		customEM.addEventHandler( "religionFounded", onReligionFounded )
		
		customEM.addEventHandler( 'cityBuilt', onCityBuilt )
		customEM.addEventHandler( 'cityAcquired', onCityAcquired )
		customEM.addEventHandler( "cityLost", onCityLost )
		customEM.addEventHandler( "buildingBuilt", onBuildingBuilt )

		customEM.setPopupHandler( RevDefs.assimilationPopup, ["assimilationPopup", assimilateHandler, blankHandler] )
		
		RevUtils.initCivicsList()
		

def removeEventHandlers( ) :
		print "Removing event handlers from RevEvents"
		
		customEM.removeEventHandler( 'EndGameTurn', onEndGameTurn )
		customEM.removeEventHandler( 'BeginPlayerTurn', onBeginPlayerTurn )
		customEM.removeEventHandler( 'EndPlayerTurn', onEndPlayerTurn )
		customEM.removeEventHandler( "setPlayerAlive", onSetPlayerAlive )
		customEM.removeEventHandler( "changeWar", onChangeWar )
		customEM.removeEventHandler( "religionFounded", onReligionFounded )
		
		customEM.removeEventHandler( 'cityBuilt', onCityBuilt )
		customEM.removeEventHandler( 'cityAcquired', onCityAcquired )
		customEM.removeEventHandler( "cityLost", onCityLost )
		customEM.removeEventHandler( "buildingBuilt", onBuildingBuilt )

		customEM.setPopupHandler( RevDefs.assimilationPopup, ["assimilationPopup", blankHandler, blankHandler] )


def blankHandler( playerID, netUserData, popupReturn ) :
		# Dummy handler to take the second event for popup
		return

########################## Turn-based events ###############################

def onEndGameTurn( argsList ) :

	if( game.getGameTurn()%int(RevUtils.getGameSpeedMod()*10) == 0 ) :
		updateAttitudeExtras()

	removeFloatingRebellions()
	
	if( allowAssimilation ) :
		checkForAssimilation( )
	
	for i in range(0,gc.getMAX_CIV_PLAYERS()) :
		playerI = gc.getPlayer(i)
		if( playerI.isRebel() ) :
			if( gc.getTeam(playerI.getTeam()).getAtWarCount(True) == 0 ) :
				playerI.setIsRebel( False )
				if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - %s (Player %d) is no longer a rebel due to no wars"%(playerI.getCivilizationDescription(0),i))
			elif( gc.getTeam(playerI.getTeam()).countRebelAgainst() == 0 ) :
				playerI.setIsRebel( False )
				if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - %s (Player %d) is no longer a rebel due to no rebel against"%(playerI.getCivilizationDescription(0),i))
			elif( playerI.getNumCities() > 3 and (game.getGameTurn() - playerI.getCapitalCity().getGameTurnAcquired() > 15) ) :
				playerI.setIsRebel( False )
				if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - %s (Player %d) is no longer a rebel by cities and capital ownership turns"%(playerI.getCivilizationDescription(0),i))
			elif( playerI.getNumCities() > 0 and (game.getGameTurn() - playerI.getCapitalCity().getGameTurnAcquired() > 30) ) :
				playerI.setIsRebel( False )
				if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - %s (Player %d) is no longer a rebel by capital ownership turns"%(playerI.getCivilizationDescription(0),i))
			elif( LOG_DEBUG ) :
				teamString = ""
				for j in range(0,gc.getMAX_CIV_TEAMS()) :
					if( gc.getTeam(playerI.getTeam()).isRebelAgainst(j) ) :
						teamString += "%d, "%(j)
				CvUtil.pyPrint("Rev - %s (%d) is a rebel against teams %s"%(playerI.getCivilizationDescription(0),i,teamString))

def onBeginPlayerTurn( argsList ) :
	iGameTurn, iPlayer = argsList

	#if( iPlayer == game.getActivePlayer() ) :
	#	CvUtil.pyPrint("Rev - Recording civics for active player")
	iNextPlayer = iPlayer + 1
	while( iNextPlayer <= gc.getBARBARIAN_PLAYER() and not gc.getPlayer(iNextPlayer).isAlive() ) :
		iNextPlayer += 1
	
	if( iNextPlayer > gc.getBARBARIAN_PLAYER() ) :
		iNextPlayer = 0
		while( iNextPlayer < iPlayer and not gc.getPlayer(iNextPlayer).isAlive() ) :
			iNextPlayer += 1


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

	# Stuff before next players turn 
	#CvUtil.pyPrint("Rev - Recording civics for player %d"%(iNextPlayer))
	recordCivics( iNextPlayer )
	
	if( bSmallRevolts and gc.getPlayer(iNextPlayer).isAlive() ) :
		doSmallRevolts( iNextPlayer )

########################## Diplomatic events ###############################

def onSetPlayerAlive( argsList ) :

	iPlayerID = argsList[0]
	bNewValue = argsList[1]
	
	pPlayer = gc.getPlayer( iPlayerID )
	
	if( bNewValue == False ) :
		
		print 'Rev - %s are dead, %d cities lost, %d founded a city'%(pPlayer.getCivilizationDescription(0),pPlayer.getCitiesLost(),pPlayer.isFoundedFirstCity())
		
		# Check if this was a put down revolution
		for i in range(0,gc.getMAX_CIV_PLAYERS()) :
			playerI = gc.getPlayer(i)
			if( playerI.isAlive() and playerI.getNumCities() > 0 ) :
				playerIPy = PyPlayer( i )
				cityList = playerIPy.getCityList()
				for city in cityList :
					pCity = city.GetCy()
					revCiv = RevData.getCityVal( pCity, "RevolutionCiv" )
					revTurn = RevData.getCityVal( pCity, "RevolutionTurn" )
					if( revCiv == pPlayer.getCivilizationType() and revTurn > 0 ) :
						if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - The dying %s are the rebel type for %s"%(pPlayer.getCivilizationDescription(0),pCity.getName()))
						if( gc.getTeam(pPlayer.getTeam()).isAtWar(pCity.getOwner()) ) :
							revIdx = pCity.getRevolutionIndex()
							localIdx = pCity.getLocalRevIndex()
							revCnt = pCity.getNumRevolts(pCity.getOwner())
							if( pCity.getReinforcementCounter() > 0 ) :
								# Put down while still fresh
								if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - Revolution put down while still actively revolting")
								frac = .3
								if( localIdx < 0 ) :
									frac += .1
								if( revIdx > RevOpt.getAlwaysViolentThreshold() ) :
									frac += .1
								if( revCnt > 2 ) :
									# Hardened, stubborn populace
									frac -= .08
								changeRevIdx = -int(frac*revIdx)
								pCity.changeRevolutionIndex( changeRevIdx )
								pCity.changeRevRequestAngerTimer( -pCity.getRevRequestAngerTimer() )
								pCity.setRevolutionIndex( min([pCity.getRevolutionIndex(),RevOpt.getAlwaysViolentThreshold()]) )
								revIdxHist = RevData.getCityVal(pCity,'RevIdxHistory')
								revIdxHist['Events'][0] += changeRevIdx
								RevData.updateCityVal( pCity, 'RevIdxHistory', revIdxHist )
								pCity.setReinforcementCounter( 0 )
								pCity.setOccupationTimer(0)
								if( LOG_DEBUG ) : CvUtil.pyPrint("Rev index in %s decreased to %d (from %d)"%(pCity.getName(),pCity.getRevolutionIndex(),revIdx))
							elif( game.getGameTurn() - revTurn < 30 ) :
								# Put down after a while
								if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - Revolution put down after going dormant")
								frac = .2
								if( localIdx < 0 ) :
									if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - Local conditions are improving")
									frac += .1
								if( revIdx > RevOpt.getAlwaysViolentThreshold() ) :
									frac += .1
								if( revCnt > 2 ) :
									# Hardened, stubborn populace
									frac -= .05
								changeRevIdx = -int(frac*revIdx)
								pCity.changeRevolutionIndex( changeRevIdx )
								pCity.changeRevRequestAngerTimer( -pCity.getRevRequestAngerTimer() )
								pCity.setRevolutionIndex( min([pCity.getRevolutionIndex(),RevOpt.getAlwaysViolentThreshold()]) )
								revIdxHist = RevData.getCityVal(pCity,'RevIdxHistory')
								revIdxHist['Events'][0] += changeRevIdx
								RevData.updateCityVal( pCity, 'RevIdxHistory', revIdxHist )
								pCity.setOccupationTimer(0)
								if( LOG_DEBUG ) : CvUtil.pyPrint("Rev index in %s decreased to %d (from %d)"%(pCity.getName(),pCity.getRevolutionIndex(),revIdx))


		if( not pPlayer.isFoundedFirstCity() ) :
			# Add +1 for this turn?
			for turnID in range(0,game.getGameTurn()) :
				if( pPlayer.getAgricultureHistory(turnID) > 0 ) :
					print 'Rev - Setting founded city to true for failed reincarnation of rebel player %d'%(iPlayerID)
					pPlayer.setFoundedFirstCity( True )
					break

		pTeam = gc.getTeam(pPlayer.getTeam())
		if( endWarsOnDeath ) :
			if( pTeam.getNumMembers() == 1 or not pTeam.isAlive() ) :
				for idx in range(0,gc.getMAX_CIV_TEAMS()) :
					if( not idx == pTeam.getID() and not gc.getTeam(idx).isMinorCiv() ) :
						if( pTeam.isAtWar(idx) ) :
							pTeam.makePeace(idx)
		
		if( pPlayer.isMinorCiv() ) :
			print 'Rev - %s were minor civ'%(pPlayer.getCivilizationDescription(0))
			pTeam.setIsMinorCiv(False, False)
		
		if( LOG_DEBUG and pPlayer.isRebel() ) : CvUtil.pyPrint("Rev - %s (%d) is no longer a rebel by death"%(pPlayer.getCivilizationDescription(0),pPlayer.getID()))
		pPlayer.setIsRebel( False )
#-------------------------------------------------------------------------------------------------
# Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------        
		
		# Appears to be too late, game is already ending before this popup can take effect
		if( iPlayerID == game.getActivePlayer() and game.getAIAutoPlay(iPlayerID) == 0 ) :
			try :
				game.setAIAutoPlay(iPlayerID, 1)
#-------------------------------------------------------------------------------------------------
# END Lemmy101 RevolutionMP edit
#-------------------------------------------------------------------------------------------------        
				#ChangePlayer.changeHumanPopup( True )
			except :
				pass

def onChangeWar( argsList ):
		'War Status Changes'
		bIsWar = argsList[0]
		iTeam = argsList[1]
		iRivalTeam = argsList[2]

		if( False and LOG_DEBUG ) :
			if (bIsWar):
					strStatus = "declared war"
			else:
					strStatus = "declared peace"
			CvUtil.pyPrint('Team %d has %s on Team %d'
					%(iTeam, strStatus, iRivalTeam))

		if( not bIsWar ) :
			# Check if this is peaceful end to a revolution
			onTeamList = list()
			onTeamCivs = list()
			onRivalTeamList = list()
			onRivalTeamCivs = list()
			for i in range(0,gc.getMAX_CIV_PLAYERS()) :
				pPlayer = gc.getPlayer(i)
				if( pPlayer.getTeam() == iTeam and pPlayer.isAlive() ) :
					# On team declaring peace
					onTeamList.append(pPlayer)
					onTeamCivs.append(pPlayer.getCivilizationType())

				elif( pPlayer.getTeam() == iRivalTeam and pPlayer.isAlive() ) :
					# On team accepting peace
					onRivalTeamList.append(pPlayer)
					onRivalTeamCivs.append(pPlayer.getCivilizationType())

			for pPlayer in onTeamList :

				playerPy = PyPlayer(pPlayer.getID())

				cityList = playerPy.getCityList()
				for city in cityList :
					pCity = city.GetCy()
					revCiv = RevData.getCityVal( pCity, "RevolutionCiv" )
					if( revCiv in onRivalTeamCivs ) :
						if( not RevData.getCityVal( pCity, "RevolutionTurn" ) == None ) :
							if( game.getGameTurn() - RevData.getCityVal( pCity, "RevolutionTurn" ) < 30 ) :
								# City recently rebelled for civ now at peace
								localIdx = pCity.getLocalRevIndex()
								revIdx = pCity.getRevolutionIndex()
								revCnt = pCity.getNumRevolts(pCity.getOwner())
								if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - Rebels in %s have agreed to peace (%d, %d, %d)"%(pCity.getName(),revIdx,localIdx,revCnt))
								frac = .2
								if( localIdx < 0 ) :
									frac += .1
								if( revIdx > RevOpt.getAlwaysViolentThreshold() ) :
									frac += .1
								if( revCnt > 2 ) :
									# Hardened, stubborn populace
									frac -= .05
								changeRevIdx = -int(frac*revIdx)
								pCity.changeRevolutionIndex( changeRevIdx )
								pCity.setRevolutionIndex( min([pCity.getRevolutionIndex(),RevOpt.getAlwaysViolentThreshold()]) )
								revIdxHist = RevData.getCityVal(pCity,'RevIdxHistory')
								revIdxHist['Events'][0] += changeRevIdx
								RevData.updateCityVal( pCity, 'RevIdxHistory', revIdxHist )
								pCity.setOccupationTimer(0)
								if( LOG_DEBUG ) : CvUtil.pyPrint("Rev index in %s decreased to %d (from %d)"%(pCity.getName(),pCity.getRevolutionIndex(),revIdx))

				gc.getTeam(pPlayer.getTeam()).setRebelAgainst( iRivalTeam, False )
				
			for pPlayer in onRivalTeamList :
				playerPy = PyPlayer(pPlayer.getID())

				cityList = playerPy.getCityList()
				for city in cityList :
					pCity = city.GetCy()
					revCiv = RevData.getCityVal( pCity, "RevolutionCiv" )
					if( revCiv in onTeamCivs ) :
						if( not RevData.getCityVal( pCity, "RevolutionTurn" ) == None ) :
							if( game.getGameTurn() - RevData.getCityVal( pCity, "RevolutionTurn" ) < 30 ) :
								# City recently rebelled for civ now at peace
								localIdx = pCity.getLocalRevIndex()
								revIdx = pCity.getRevolutionIndex()
								revCnt = pCity.getNumRevolts(pCity.getOwner())
								if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - Rebels in %s have sued for peace"%(pCity.getName()))
								frac = .2
								if( localIdx < 0 ) :
									frac += .1
								if( revIdx > RevOpt.getAlwaysViolentThreshold() ) :
									frac += .1
								if( revCnt > 2 ) :
									# Hardened, stubborn populace
									frac -= .05
								changeRevIdx = -int(frac*revIdx)
								pCity.changeRevolutionIndex( changeRevIdx )
								pCity.setRevolutionIndex( min([pCity.getRevolutionIndex(),RevOpt.getAlwaysViolentThreshold()]) )
								revIdxHist = RevData.getCityVal(pCity,'RevIdxHistory')
								revIdxHist['Events'][0] += changeRevIdx
								RevData.updateCityVal( pCity, 'RevIdxHistory', revIdxHist )
								pCity.setOccupationTimer(0)
								if( LOG_DEBUG ) : CvUtil.pyPrint("Rev index in %s decreased to %d (from %d)"%(pCity.getName(),pCity.getRevolutionIndex(),revIdx))

				gc.getTeam(pPlayer.getTeam()).setRebelAgainst( iTeam, False )


########################## City-based events ###############################

def onCityBuilt( argsList ):
		'City Built'
		city = argsList[0]

		RevData.initCity( city )

		pPlayer = gc.getPlayer( city.getOwner() )

		if( pPlayer.isBarbarian() ) :
			city.setRevolutionIndex( int(.4*RevOpt.getAlwaysViolentThreshold()) )
			city.setRevIndexAverage(city.getRevolutionIndex())
			return

		if( not city.area().getID() == pPlayer.getCapitalCity().area().getID() ) :
			city.setRevolutionIndex( int(.35*RevOpt.getInstigateRevolutionThreshold()) )
		else :
			city.setRevolutionIndex( int(.25*RevOpt.getInstigateRevolutionThreshold()) )
		city.setRevIndexAverage(city.getRevolutionIndex())
		
		revTurn = RevData.revObjectGetVal( pPlayer, 'RevolutionTurn' )
		if( not revTurn == None ) :
			if( pPlayer.getNumCities() < 4 and game.getGameTurn() - revTurn < 25 ) :
				relID = pPlayer.getStateReligion()
				if( relID >= 0 ) :
					if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - New rebel city %s given rebel religion"%(city.getName()))
					city.setHasReligion( relID, True, False, False )

def onCityAcquired( argsList ):
		'City Acquired'

		owner,playerType,pCity,bConquest,bTrade = argsList

		checkRebelBonuses( argsList )
		updateRevolutionIndices( argsList )

		# Init city script data (unit spawn counter, rebel player)
		iRevCiv = RevData.getCityVal(pCity, 'RevolutionCiv')
		RevData.initCity(pCity)
		RevData.setCityVal( pCity, 'RevolutionCiv', iRevCiv )
		
		iTurns = pCity.getOccupationTimer()
		#if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - Occupations timer is currently %d"%(iTurns))
		pCity.setRevolutionCounter( max([int(1.5*iTurns),3]) )
		

def checkRebelBonuses( argsList ) :
		# Give bonuses to a rebel player who successfully captures one of their rebellious cities
		owner,playerType,pCity,bConquest,bTrade = argsList

		newOwnerID = pCity.getOwner()
		newOwner = gc.getPlayer(newOwnerID)
		newOwnerCiv = newOwner.getCivilizationType()
		oldOwnerID = pCity.getPreviousOwner()
		orgOwnerID = pCity.getOriginalOwner()
		
		# TODO: Handle case where city is acquired by disorganized rebels
		if( newOwnerID == gc.getBARBARIAN_PLAYER() and pCity.getRevolutionCounter() > 0 ) :
			if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - City %s captured by barb rebels!"%(pCity.getName()))
			oldOwner = gc.getPlayer(oldOwnerID)

			if( not oldOwnerID == orgOwnerID ) :
				orgOwner = gc.getPlayer(orgOwnerID)
				#if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - City %s originally owned by %s!"%(pCity.getName(),orgOwner.getCivilizationDescription(0)))

			if( pCity.countTotalCultureTimes100() > 100*100 ) :
				if( not (oldOwnerID == pCity.findHighestCulture()) ) :
					cultOwner = gc.getPlayer( pCity.findHighestCulture() )
					#if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - City %s culturally dominated by %s!"%(pCity.getName(),cultOwner.getCivilizationDescription(0)))

		elif( newOwnerCiv == RevData.getCityVal(pCity, 'RevolutionCiv') ) :

			# TODO: Check whether revolt is active in RevoltData
			if( (pCity.getReinforcementCounter() > 0 or (pCity.unhappyLevel(0) - pCity.happyLevel()) > 0) ) :
				if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - Rebellious pCity %s is captured by rebel identity %s (%d)!!!"%(pCity.getName(),newOwner.getCivilizationDescription(0),newOwnerCiv))

				newOwnerTeam = gc.getTeam(newOwner.getTeam())
				oldOwner = gc.getPlayer(oldOwnerID)
				oldOwnerTeam = gc.getTeam(oldOwner.getTeam())
				if( oldOwnerTeam.isAVassal() ) :
					for teamID in range(0,gc.getMAX_CIV_TEAMS()) :
						if( oldOwnerTeam.isVassal(teamID) ) :
							oldOwnerTeam = gc.getTeam( teamID )
				
				ix = pCity.getX()
				iy = pCity.getY()
				
				[iWorker,iBestDefender,iCounter,iAttack] = RevUtils.getHandoverUnitTypes( pCity, newOwner, newOwner )
				
				newUnitList = list()

				# Couple units regardless of rebel status
				newUnitList.append(newOwner.initUnit( iBestDefender, ix, iy, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH ))
				if( pCity.getPopulation() > 4 ) :
					newUnitList.append(newOwner.initUnit( iCounter, ix, iy, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH ))
				
				if( newOwner.isRebel() ) :
					# Extra benefits if still considered a rebel
					mess = localText.getText("TXT_KEY_REV_MESS_YOUR_CAPTURE",())%(pCity.getName())
					CyInterface().addMessage(newOwnerID, false, gc.getDefineINT("EVENT_MESSAGE_TIME"), mess, "AS2D_CITY_REVOLT", InterfaceMessageTypes.MESSAGE_TYPE_MINOR_EVENT, CyArtFileMgr().getInterfaceArtInfo("INTERFACE_RESISTANCE").getPath(), ColorTypes(8), ix, iy, True, True)
					mess = localText.getText("TXT_KEY_REV_MESS_REBEL_CONTROL",())%(newOwner.getCivilizationDescription(0),pCity.getName())
					CyInterface().addMessage(oldOwnerID, false, gc.getDefineINT("EVENT_MESSAGE_TIME"), mess, None, InterfaceMessageTypes.MESSAGE_TYPE_MINOR_EVENT, None, ColorTypes(7), -1, -1, False, False)
					
					# Gold
					iGold = game.getSorenRandNum(min([80,8*pCity.getPopulation()]),'Rev') + 8
					mess = localText.getText("TXT_KEY_REV_MESS_YOUR_CAPTURE_GOLD",())%(pCity.getName(),iGold)
					CyInterface().addMessage(newOwnerID, false, gc.getDefineINT("EVENT_MESSAGE_TIME"), mess, "AS2D_CITY_REVOLT", InterfaceMessageTypes.MESSAGE_TYPE_MINOR_EVENT, CyArtFileMgr().getInterfaceArtInfo("INTERFACE_RESISTANCE").getPath(), ColorTypes(8), ix, iy, False, False)
					newOwner.changeGold( iGold )
					
					# Culture
					newCulVal = int( revCultureModifier*max([pCity.getCulture(oldOwnerID),pCity.countTotalCultureTimes100()/200]) )
					newPlotVal = int( revCultureModifier*max([pCity.plot().getCulture(oldOwnerID),pCity.plot().countTotalCulture()/2]) )
					RevUtils.giveCityCulture( pCity, newOwnerID, newCulVal, newPlotVal, overwriteHigher = False )
					
					# Extra units
					if(iWorker != -1):
						newUnitList.append(newOwner.initUnit( iWorker, ix, iy, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH ))
					if( pCity.getPopulation() > 7 ) :
						newUnitList.append(newOwner.initUnit( iBestDefender, ix, iy, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH ))
					if( pCity.getPopulation() > 4 and newOwnerTeam.getPower(True) < oldOwnerTeam.getPower(True)/4 ) :
						newUnitList.append(newOwner.initUnit( iAttack, ix, iy, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH ))
					
					if( newOwner.getNumCities() <= 1 ) :
						# Extra units for first city captured
						newUnitList.append(newOwner.initUnit( iCounter, ix, iy, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH ))
						if( newOwnerTeam.getPower(True) < oldOwnerTeam.getPower(True)/2 ) :
							newUnitList.append(newOwner.initUnit( iBestDefender, ix, iy, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH ))
							newUnitList.append(newOwner.initUnit( iAttack, ix, iy, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH ))
						elif( newOwnerTeam.getPower(True) < oldOwnerTeam.getPower(True) ) :
							newUnitList.append(newOwner.initUnit( iAttack, ix, iy, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH ))
					
					# Give a boat to island rebels
					if( pCity.isCoastal(10) and pCity.area().getNumCities() < 3 and pCity.area().getNumTiles() < 25 ) :
						iAssaultShip = UnitTypes.NO_UNIT
						for unitID in range(0,gc.getNumUnitInfos()) :
							unitClass = gc.getUnitInfo(unitID).getUnitClassType()
							if( unitID == gc.getCivilizationInfo( newOwner.getCivilizationType() ).getCivilizationUnits(unitClass) ) :
								if( gc.getUnitInfo(unitID).getDomainType() == DomainTypes.DOMAIN_SEA and newOwner.canTrain(unitID,False,False) ):
									# Military unit transport ship
									if( gc.getUnitInfo(unitID).getUnitAIType( UnitAITypes.UNITAI_ASSAULT_SEA ) ):
										if( iAssaultShip == UnitTypes.NO_UNIT or gc.getUnitInfo(unitID).getCombat() >= gc.getUnitInfo(iAssaultShip).getCombat() ) :
											iAssaultShip = unitID
						if( not iAssaultShip == UnitTypes.NO_UNIT ) :
							newOwner.initUnit( iAssaultShip, ix, iy, UnitAITypes.UNITAI_ASSAULT_SEA, DirectionTypes.DIRECTION_SOUTH )
							if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - Rebels get a %s to raid motherland"%(PyInfo.UnitInfo(iAssaultShip).getDescription()))
					
					# Change city disorder timer to favor new player
					iTurns = pCity.getOccupationTimer()
					iTurns = iTurns/4 + 1
					pCity.setOccupationTimer(iTurns)
					
					# Temporary happiness boost
					pCity.changeRevSuccessTimer( int(iTurns + RevUtils.getGameSpeedMod()*15) )
					
					# Trigger golden age for rebel civ under certain circumstances
					revTurn = RevData.revObjectGetVal( newOwner, 'RevolutionTurn' )
					if( not revTurn == None and game.getGameTurn() - revTurn < 4*game.goldenAgeLength() ) :
						if( newOwner.getNumCities() == 3 ) :
							if( newOwner.getCitiesLost() == 0 ) :
								# By verifying they've never lost a city, gaurantee it doesn't happen multiple times
								mess = localText.getText("TXT_KEY_REV_MESS_GOLDEN_AGE",())
								CyInterface().addMessage(newOwnerID, false, gc.getDefineINT("EVENT_MESSAGE_TIME"), mess, "AS2D_CITY_REVOLT", InterfaceMessageTypes.MESSAGE_TYPE_MINOR_EVENT, CyArtFileMgr().getInterfaceArtInfo("INTERFACE_RESISTANCE").getPath(), ColorTypes(8), ix, iy, False, False)
								if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - Free Golden Age for %s rebels who have never lost a city"%(newOwner.getCivilizationDescription(0)))
								newOwner.changeGoldenAgeTurns( int(1.5*game.goldenAgeLength()) )
				
				else :
					# Conqueror not considered a rebel, fewer benefits
					# Culture
					newCulVal = int( revCultureModifier*max([pCity.getCulture(oldOwnerID)/2,pCity.countTotalCultureTimes100()/400]) )
					newPlotVal = int( revCultureModifier*max([pCity.plot().getCulture(oldOwnerID)/2,pCity.plot().countTotalCulture()/4]) )
					RevUtils.giveCityCulture( pCity, newOwnerID, newCulVal, newPlotVal, overwriteHigher = False )
					
					# Change city disorder timer to favor new player
					iTurns = pCity.getOccupationTimer()
					iTurns = min([iTurns, iTurns/3 + 1])
					pCity.setOccupationTimer(iTurns)
					
					# Temporary happiness boost
					pCity.changeRevSuccessTimer( int(iTurns + RevUtils.getGameSpeedMod()*6) )
				
				# Injure free units
				for unit in newUnitList :
					if( unit.canFight() ) :
						iDamage = 20 + game.getSorenRandNum(20,'Rev - Injure unit')
						unit.setDamage( iDamage, oldOwnerID )
				
			# City once rebelled as this civ type, but not currently rebellious
			else :
				if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - %s, captured by former rebel identity: %s (%d)!"%(pCity.getName(),newOwner.getCivilizationDescription(0),newOwnerCiv))
				newCulVal = int( revCultureModifier*max([pCity.getCulture(oldOwnerID)/2,pCity.countTotalCultureTimes100()/400]) )
				newPlotVal = int( revCultureModifier*max([pCity.plot().getCulture(oldOwnerID)/2,pCity.plot().countTotalCulture()/4]) )
				RevUtils.giveCityCulture( pCity, newOwnerID, newCulVal, newPlotVal, overwriteHigher = False )
				
				iTurns = pCity.getOccupationTimer()
				iTurns = iTurns/2 + 1
				pCity.setOccupationTimer(iTurns)

def updateRevolutionIndices( argsList ) :
		owner,playerType,pCity,bConquest,bTrade = argsList

		newOwnerID = pCity.getOwner()
		newOwner = gc.getPlayer(newOwnerID)
		newOwnerCiv = newOwner.getCivilizationType()
		oldOwnerID = pCity.getPreviousOwner()
		orgOwnerID = pCity.getOriginalOwner()
		
		if( newOwner.isBarbarian() ) :
			return

		newRevIdx = 400
		changeRevIdx = -40

		if( bConquest ) :
			# Occupied cities also rack up rev points each turn
			newRevIdx += pCity.getRevolutionIndex()/4
			newRevIdx = min( [newRevIdx, 600] )

			if( pCity.plot().calculateCulturePercent( newOwnerID ) > 90 ) :
				changeRevIdx -= 75
				newRevIdx -= 100
			elif( pCity.plot().calculateCulturePercent( newOwnerID ) > 40 ) :
				changeRevIdx -= 35
				newRevIdx -= 60
			elif( pCity.plot().calculateCulturePercent( newOwnerID ) > 20 ) :
				changeRevIdx -= 30

		elif( bTrade ) :
			newRevIdx += pCity.getRevolutionIndex()/3
			newRevIdx = min( [newRevIdx, 650] )

			if( pCity.plot().calculateCulturePercent( newOwnerID ) > 90 ) :
				newRevIdx -= 50

		else :
			# Probably cultural conversion
			newRevIdx -= 100
			if( pCity.plot().calculateCulturePercent( newOwnerID ) > 50 ) :
				changeRevIdx -= 25
		
		
		if( newOwner.isRebel() and newOwnerCiv == RevData.getCityVal(pCity, 'RevolutionCiv') ) :
			changeRevIdx -= 50
			newRevIdx -= 200
		elif( newOwnerID == pCity.getOriginalOwner() ) :
			changeRevIdx -= 25
			newRevIdx -= 100

		if( pCity.getHighestPopulation() < 6 ) :
			changeRevIdx += 20
			newRevIdx -= 50		
		
		changeRevIdx = int(math.floor( cityAcquiredModifier*changeRevIdx + .5 ))
		
		if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Acquisition of %s by %s reduces rev indices by %d"%(pCity.getName(),newOwner.getCivilizationDescription(0),changeRevIdx))
		
		for listCity in PyPlayer( newOwnerID ).getCityList() :
			pListCity = listCity.GetCy()
			if( not pListCity.getID() == pCity.getID() ) :
				pListCity.changeRevolutionIndex( changeRevIdx )
				revIdxHist = RevData.getCityVal(pListCity,'RevIdxHistory')
				revIdxHist['Events'][0] += changeRevIdx
				RevData.updateCityVal( pListCity, 'RevIdxHistory', revIdxHist )

		if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - New rev idx for %s is %d"%(pCity.getName(),newRevIdx))
		
		pCity.setRevolutionIndex( newRevIdx )
		pCity.setRevIndexAverage( newRevIdx )
		pCity.setRevolutionCounter( acquiredTurns )
		pCity.setReinforcementCounter( 0 )
		RevData.updateCityVal( pCity, 'RevIdxHistory', RevDefs.initRevIdxHistory() )
		
		if( newOwner.isRebel() ) :
			if( newOwner.getNumCities() > 1 and RevData.revObjectGetVal( newOwner, 'CapitalName' ) == CvUtil.convertToStr(pCity.getName()) ) :
				# Rebel has captured their instigator city, make this their capital
				if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Rebel %s have captured their instigator city, %s!  Moving capital."%(newOwner.getCivilizationDescription(0),pCity.getName()))
				if( newOwner.isHuman() ) :
					# TODO: support this with a popup question
					pass
				else :
					capitalClass = CvUtil.findInfoTypeNum(gc.getBuildingClassInfo,gc.getNumBuildingClassInfos(),RevDefs.sXMLPalace)
					eCapitalBuilding = gc.getCivilizationInfo(newOwner.getCivilizationType()).getCivilizationBuildings(capitalClass)
					oldCapital = newOwner.getCapitalCity()
					oldCapital.setNumRealBuilding(eCapitalBuilding, 0)
					pCity.setNumRealBuilding(eCapitalBuilding, 1)
			
			# Ripple effects through other rebellious cities
			for listCity in PyPlayer( oldOwnerID ).getCityList() :
				pListCity = listCity.GetCy()
				reinfCount = pListCity.getReinforcementCounter()
				if( reinfCount > 2 and RevData.getCityVal(pListCity, 'RevolutionCiv') == newOwner.getCivilizationType() ) :
					if( reinfCount < 5 ) :
						reinfCount = 2
					else :
						reinfCount = reinfCount-2
					
					if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Accelerating reinforcement in %s"%(pListCity.getName()))
					# Setting below two will turn off reinforcement
					pListCity.setReinforcementCounter( max([reinfCount,2]) )


def onCityLost(argsList):
		'City Lost'
		pCity = argsList[0]
		player = gc.getPlayer(pCity.getOwner())

		playerCityLost( player, pCity, bConquest = (pCity.plot().getNumDefenders(player.getID()) == 0) )

def playerCityLost( player, pCity, bConquest = True ) :

		if( player.getNumCities() < 1 or player.isBarbarian() ) :
			return

		iPlayer = player.getID()
		capital = player.getCapitalCity()
		capitalArea = capital.area().getID()

		iRevIdxChange = 0

		if( game.getGameTurn() - pCity.getGameTurnAcquired() < 2 ) :
			iRevIdxChange = 0
			#if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Loss of %s by %s: very recently acquired, %d rev idx change"%(pCity.getName(),player.getCivilizationDescription(0),iRevIdxChange))
		elif( game.getGameTurn() - pCity.getGameTurnAcquired() < 20 ) :
			iRevIdxChange = 50
			#if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Loss of %s by %s: recently acquired, %d rev idx change"%(pCity.getName(),player.getCivilizationDescription(0),iRevIdxChange))
		elif( pCity.isCapital() ) :
			iRevIdxChange = 400
			#if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Loss of %s by %s: capital, %d rev idx change"%(pCity.getName(),player.getCivilizationDescription(0),iRevIdxChange))
		elif( pCity.getHighestPopulation() < 4 ) :
			iRevIdxChange = 50
			#if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Loss of %s by %s: small city, %d rev idx change"%(pCity.getName(),player.getCivilizationDescription(0),iRevIdxChange))
		elif( pCity.plot().calculateCulturePercent(iPlayer) > 60 and pCity.countTotalCultureTimes100() > 100*100 ) :
			iRevIdxChange = 150
			#if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Loss of %s by %s: majority nationality, %d rev idx change"%(pCity.getName(),player.getCivilizationDescription(0),iRevIdxChange))
		elif( pCity.getOriginalOwner() == iPlayer ) :
			iRevIdxChange = 125
			#if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Loss of %s by %s: original founder, %d rev idx change"%(pCity.getName(),player.getCivilizationDescription(0),iRevIdxChange))
		else :
			iRevIdxChange = 100
			#if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Loss of %s by %s: default, %d rev idx change"%(pCity.getName(),player.getCivilizationDescription(0),iRevIdxChange))

		if( not pCity.area().getID() == capitalArea and iRevIdxChange > 25 ) :
			iRevIdxChange -= 25
			#if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - %s is a colony, reducing effect to %d"%(pCity.getName(),iRevIdxChange))

		if( not bConquest ) :
			if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Not conquest, reducing rev index increase from %d"%(iRevIdxChange))
			iRevIdxChange = iRevIdxChange/3.0

		iRevIdxChange = int(math.floor( cityLostModifier*iRevIdxChange + .5))
		
		if( player.isRebel() ) :
			iRevIdxChange /= 2

		if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Loss of %s by %s (%d bConq): %d rev idx change"%(pCity.getName(),player.getCivilizationDescription(0),bConquest,iRevIdxChange))

		for city in PyPlayer(iPlayer).getCityList() :
			pCity = city.GetCy()
			pCity.changeRevolutionIndex( iRevIdxChange )
			revIdxHist = RevData.getCityVal(pCity,'RevIdxHistory')
			revIdxHist['Events'][0] += iRevIdxChange
			RevData.updateCityVal( pCity, 'RevIdxHistory', revIdxHist )

def onBuildingBuilt( argsList):
		'Building Completed'
		pCity, iBuildingType = argsList

		buildingInfo = gc.getBuildingInfo(iBuildingType)
		buildingClassInfo = gc.getBuildingClassInfo(buildingInfo.getBuildingClassType())

		if( buildingClassInfo.getMaxGlobalInstances() == 1 and buildingInfo.getPrereqReligion() < 0 and buildingInfo.getProductionCost() > 10 ) :
			if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - World wonder %s build in %s"%(buildingInfo.getDescription(),pCity.getName()))
			curRevIdx = pCity.getRevolutionIndex()
			pCity.changeRevolutionIndex( -max([150,int(0.25*curRevIdx)]) )
			for city in PyPlayer(pCity.getOwner()).getCityList() :
				listCity = city.GetCy()
				curRevIdx = listCity.getRevolutionIndex()
				iRevIdxChange = -max([75,int(0.12*curRevIdx)])
				listCity.changeRevolutionIndex( iRevIdxChange )
				revIdxHist = RevData.getCityVal(pCity,'RevIdxHistory')
				revIdxHist['Events'][0] += iRevIdxChange
				RevData.updateCityVal( pCity, 'RevIdxHistory', revIdxHist )

		elif( buildingClassInfo.getMaxPlayerInstances() == 1 and buildingInfo.getPrereqReligion() < 0 and buildingInfo.getProductionCost() > 10 ) :
			if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - National wonder %s build in %s"%(buildingInfo.getDescription(),pCity.getName()))
			curRevIdx = pCity.getRevolutionIndex()
			pCity.changeRevolutionIndex( -max([80,int(0.12*curRevIdx)]) )
			for city in PyPlayer(pCity.getOwner()).getCityList() :
				listCity = city.GetCy()
				curRevIdx = listCity.getRevolutionIndex()
				iRevIdxChange = -max([50,int(0.07*curRevIdx)])
				listCity.changeRevolutionIndex( iRevIdxChange )
				revIdxHist = RevData.getCityVal(pCity,'RevIdxHistory')
				revIdxHist['Events'][0] += iRevIdxChange
				RevData.updateCityVal( pCity, 'RevIdxHistory', revIdxHist )


########################## Religious events ###############################

def onReligionFounded(argsList):
		'Religion Founded'
		iReligion, iFounder = argsList
		pPlayer = gc.getPlayer(iFounder)
		
		#print "Player %d has founded religion %d"%(iFounder,iReligion)

		if( pPlayer.getStateReligion() >= 0 and iReligion >= 0 ) :
			if( not (pPlayer.getStateReligion() == iReligion) and not pPlayer.isAnarchy() ) :
				pCity = gc.getGame().getHolyCity(iReligion)
				if( pCity.getOwner() == iFounder ) :
					curRevIdx = pCity.getRevolutionIndex()
					pCity.setRevolutionIndex( max([int(.35*RevDefs.revInstigatorThreshold),curRevIdx+100]) )
					if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - %s founded non-state religion, index of %s now %d ... state %d, new %d"%(pCity.getName(),pCity.getName(),pCity.getRevolutionIndex(),pPlayer.getStateReligion(),iReligion))



def recordCivics( iPlayer ) :

	pPlayer = gc.getPlayer( iPlayer )

	if( not pPlayer.isAlive() or pPlayer.isBarbarian() ) :
		return

	curCivics = list()
	for i in range(0,gc.getNumCivicOptionInfos()):
		curCivics.append( pPlayer.getCivics(i) )

	RevData.revObjectSetVal( pPlayer, "CivicList", curCivics )


def updateAttitudeExtras( bVerbose = False ) :

	for i in range(0,gc.getMAX_CIV_PLAYERS()) :
		playerI = gc.getPlayer( i )

		for j in range(0,gc.getMAX_CIV_PLAYERS()) :
			playerJ = gc.getPlayer( j )
			attEx = playerI.AI_getAttitudeExtra(j)
			# Odds and rate partially determined by current attitude value
			if( attEx > 0 and game.getSorenRandNum(100,'Rev: Attitude') < attEx*15 ) :
				playerI.AI_changeAttitudeExtra(j, -(1+attEx/10))
				if( LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("Rev - Extra Attitude for %s of %s now %d"%(playerI.getCivilizationDescription(0),playerJ.getCivilizationDescription(0),playerI.AI_getAttitudeExtra(j)))
			elif( attEx < 0 and game.getSorenRandNum(100,'Rev: Attitude') < -attEx*20 ) :
				teamI = gc.getTeam( playerI.getTeam() )
				if( not teamI.isAtWar( playerJ.getTeam() ) ) :
					playerI.AI_changeAttitudeExtra(j, -(attEx/10))
					if( LOG_DEBUG and bVerbose ) : CvUtil.pyPrint("Rev - Extra Attitude for %s of %s now %d"%(playerI.getCivilizationDescription(0),playerJ.getCivilizationDescription(0),playerI.AI_getAttitudeExtra(j)))

def removeFloatingRebellions( ) :
	# Destroy all units for a rebel who is at peace, yet has no cities

	for i in range(0,gc.getMAX_CIV_PLAYERS()) :
		player = gc.getPlayer(i)
		
		if( player.isAlive() and player.getNumCities() == 0 and player.getNumUnits() > 0 ) :
			if( player.isRebel() or not player.isFoundedFirstCity() ) :
			
				bNoSettler = True
				bOnlySpy = True

				playerPy = PyPlayer(i)
				for unit in playerPy.getUnitList() :
					if( unit.isFound() ) :
						bNoSettler = False
						bOnlySpy = False
						break
					if( not unit.getUnitAIType() == UnitAITypes.UNITAI_SPY ) :
						bOnlySpy = False
				
				if( bNoSettler ) :
					if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - Player %d (%s) is a homeless rebel"%(player.getID(),player.getCivilizationDescription(0)))
					
					if( gc.getTeam( player.getTeam() ).getAtWarCount( True ) == 0 ) :
						if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - Rebel player %d has lost their cause, terminating rebel"%(player.getID()))
						player.killUnits()
					
					elif( bOnlySpy ) :
						if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - Rebel player %d has only spies"%(player.getID()))
						
						if( game.getSorenRandNum(100, "Rev - Only spy death") < 10 ) :
							for unit in playerPy.getUnitList() :
								unit.kill( False, -1 )
								break
							if( LOG_DEBUG ) : CvUtil.pyPrint("Rev - Killed one spy of Rebel player %d"%(player.getID()))


########################## Assimilation ###############################

def checkForAssimilation(  ) :

		iWorldCities = game.getNumCivCities()
		iPlayersAlive = game.countCivPlayersAlive()
		## Stop divide by zero
		if iPlayersAlive == 0:
			return

		iWorldPlots = gc.getMap().getLandPlots()

		avgNumCities = iWorldCities/iPlayersAlive
		avgNumPlots = int(math.floor(iWorldPlots/(1.0*iPlayersAlive) +.5))
		minNumPlots = min([avgNumPlots/3+1, 15])
		minNumPlots = max([minNumPlots,9])

		maxEra = 0
		for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
			iEra = gc.getPlayer( idx ).getCurrentEra()
			if( iEra > maxEra ) :
				maxEra = iEra

		for idx in range(0,gc.getMAX_CIV_PLAYERS()) :
			playerI = gc.getPlayer( idx )

			if( not playerI.isAlive() or playerI.isBarbarian() or playerI.isMinorCiv() ) :
				continue

			if( playerI.isHuman() ) :
				continue

			teamI = gc.getTeam(playerI.getTeam())
			capital = playerI.getCapitalCity()
			assimPlayer = None
			#Afforess Scale Assimilation
			iMinimumCities = 2 + (int)(gc.getMap().getWorldSize() * 1.5)
			
			joinPlayerID = RevData.revObjectGetVal( playerI, 'JoinPlayerID' )
			# TODO: should this be extended to all players the assimilatee is at war with?  Vassal owner?
			bRiskWar = False
			motherLandID = RevData.revObjectGetVal( playerI, 'MotherlandID' )
			if( not motherLandID == None ) :
				revTurn = RevData.revObjectGetVal( playerI, 'RevolutionTurn' )
				if( not revTurn == None and game.getGameTurn() - revTurn < 40 and gc.getTeam(playerI.getTeam()).isAtWar(gc.getPlayer(motherLandID).getTeam()) ) :
					bRiskWar = True
			
			if( not joinPlayerID == None and game.getGameTurn() - capital.getGameTurnAcquired() < 30 and not teamI.isAVassal() ) :
				if( playerI.getNumCities() > 0 and playerI.getNumCities() < iMinimumCities ) :
					#Afforess Tweak: Small Civs should give up (Raised 10 to 20)
					iOdds = 20
					
					if( not motherLandID == None and gc.getTeam(playerI.getTeam()).isAtWar(gc.getPlayer(motherLandID).getTeam()) ) :
						for city in PyPlayer( motherLandID ).getCityList() :
							pCity = city.GetCy()
							if( RevData.getCityVal(pCity, 'RevolutionCiv') == playerI.getCivilizationType() ) :
								revTurn = RevData.getCityVal(pCity, 'RevolutionTurn')
								if( not revTurn == None and game.getGameTurn() - revTurn < 25 ) :
									# Odds decrease if more rebel cities are uncaptured
									iOdds -= 2
					
					if( iOdds > game.getSorenRandNum( 100, 'Revolution: Assimilate' ) ) :
						if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Assimilation! The rebel %s are requesting again to join the %s now that they've captured %d cities"%(playerI.getCivilizationDescription(0),gc.getPlayer(joinPlayerID).getCivilizationDescription(0),playerI.getNumCities()))
						assimPlayer = gc.getPlayer( joinPlayerID )

			elif( game.getGameTurn() > 100 ) :
				if( (game.getGameTurn() - capital.getGameTurnAcquired()) > 15 and playerI.getNumCities() > 0 and playerI.getNumCities() < iMinimumCities ) :
					if( playerI.getTotalLand() < minNumPlots or playerI.getTotalPopulation() < 5 ) :
						#if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - The %s is very small"%(playerI.getCivilizationDescription(0)))

						if( capital.area().getNumCities() < playerI.getNumCities() + 2 ) :
							# On own island
							continue

						if( teamI.getNumMembers() > 1 ) :
							# In confederation or alliance
							continue

						iOdds = 0
						

						if( capital.getPopulation() < 4 ) :
							iOdds += 2 + maxEra/2
						if(  playerI.getTotalLand() < minNumPlots/2 ) :
							iOdds += 2
						if( capital.getOccupationTimer() > 0 ) :
							iOdds += iOdds*3

						### Special cases
						if( teamI.isAVassal() ) :
							### Is a vassal
							if( capital.getRevolutionIndex() > RevUtils.alwaysViolentThreshold ) :
								iOdds += 7
							elif( capital.getRevolutionIndex() > RevUtils.revInstigatorThreshold ) :
								iOdds += 3

							#Afforess Tweak, Raising Max from 4 to 15
							iOdds = max([iOdds,15])

							if( game.getSorenRandNum( 100, 'Revolution: Assimilate' ) < iOdds ) :
								
								# If player is a Vassal, should only be allowed to assimilate with master
								masterPlayer = None
								for teamIdx in range(0,gc.getMAX_CIV_TEAMS()) :
									if( teamI.isVassal(teamIdx) ) :
										masterPlayer = gc.getPlayer( gc.getTeam(teamIdx).getLeaderID() )
										break

								if( not masterPlayer == None and masterPlayer.isAlive() ) :
									if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Assimilation!  Vassal %s considering assimilation to master %s"%(playerI.getCivilizationDescription(0),masterPlayer.getCivilizationDescription(0)))

									relations = playerI.AI_getAttitude( masterPlayer.getID() )
									if( capital.plot().getCulture(masterPlayer.getID())/(1.0*capital.plot().countTotalCulture()) > .3 ) :
										# Assimilate with master with large culture in city
										if( not relations == AttitudeTypes.ATTITUDE_FURIOUS ) :
											if( masterPlayer.isHuman() ) :
												if( not idx in noAssimilateList ) :
													assimPlayer = masterPlayer
											else :
												assimPlayer = masterPlayer
											if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Assimilation to master based on culture")
									elif( relations == AttitudeTypes.ATTITUDE_PLEASED or relations == AttitudeTypes.ATTITUDE_FRIENDLY ) :
										# Assimilate with friendly, powerful master
										masterPower = masterPlayer.getPower()
										vassalPower = playerI.getPower()

										if( masterPower > 2*vassalPower and masterPlayer.getTechScore() > playerI.getTechScore() + 5 ) :
											if( masterPlayer.isHuman() ) :
												if( not idx in noAssimilateList ) :
													assimPlayer = masterPlayer
											else :
												assimPlayer = masterPlayer
											if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Assimilation to friendly and powerful master")


						elif( capital.plot().calculateCulturePercent(idx) < 90 ) :
							### Capital has foreign influence
							iCultPlayer = capital.plot().calculateCulturalOwner()	   # iCultPlayer guaranteed to be alive
							if( not iCultPlayer == playerI.getID() ) :
								iOdds += 3

							if( capital.getRevolutionIndex() > RevUtils.alwaysViolentThreshold ) :
								iOdds += 10
							elif( capital.getRevolutionIndex() > RevUtils.revInstigatorThreshold ) :
								iOdds += 3

							if( game.getSorenRandNum( 100, 'Revolution: Assimilate' ) < iOdds ) :
								if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Assimilation!  %s considering assimilation by culture"%(playerI.getCivilizationDescription(0)))

								if( iCultPlayer >= 0 and not iCultPlayer == idx and not playerI.AI_getAttitude(iCultPlayer) == AttitudeTypes.ATTITUDE_FURIOUS ) :
									## Assimilate with cultural owner
									cultPlayer = gc.getPlayer(iCultPlayer)
									if( cultPlayer.isAlive() ) :
										if( cultPlayer.isHuman() ) :
											if( not idx in noAssimilateList ) :
												assimPlayer = cultPlayer
										else :
											assimPlayer = cultPlayer
										if( not assimPlayer == None and LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Assimilation culture owner; %s"%(assimPlayer.getCivilizationDescription(0)))

								if( assimPlayer == None ) :
									## Check for good relations with second place culture
									maxSecondCult = 0
									for j in range(0,gc.getMAX_CIV_PLAYERS()) :
										if( not j == idx ) :
											if( capital.plot().getCulture(j) > maxSecondCult and gc.getPlayer(j).isAlive() ) :
												secCultPlayer = gc.getPlayer(j)
												maxSecondCult = capital.plot().getCulture(j)

									if( maxSecondCult/(1.0*capital.plot().countTotalCulture()) > .2 ) :
										relations = playerI.AI_getAttitude( secCultPlayer.getID() )
										if( relations == AttitudeTypes.ATTITUDE_PLEASED or relations == AttitudeTypes.ATTITUDE_FRIENDLY ) :
											if( secCultPlayer.isHuman() ) :
												if( not idx in noAssimilateList ) :
													assimPlayer = secCultPlayer
											else :
												assimPlayer = secCultPlayer
											if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Assimilation to friendly, 2nd culture player")
										else :
											if( maxSecondCult/(1.0*capital.plot().countTotalCulture()) > .35 ) :
												if( relations == AttitudeTypes.ATTITUDE_CAUTIOUS ) :
													if( secCultPlayer.isHuman() ) :
														if( not idx in noAssimilateList ) :
															assimPlayer = secCultPlayer
													else :
														assimPlayer = secCultPlayer
													if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Assimilation to high 2nd culture player")
										

						elif( playerI.getCurrentEra() + 2 < maxEra ) :
							### Far, far behind technologically
							if( playerI.getCurrentEra() + 3 < maxEra ) :
								iOdds += 5

							if( capital.getRevolutionIndex() > RevUtils.alwaysViolentThreshold ) :
								iOdds += 5
							elif( capital.getRevolutionIndex() > RevUtils.revInstigatorThreshold ) :
								iOdds += 2

							if( game.getSorenRandNum( 100, 'Revolution: Assimilate' ) < iOdds ) :
								if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Assimilation!  %s considering assimilation due to tech"%(playerI.getCivilizationDescription(0)))

								## Check for good relations with second highest culture
								maxSecondCult = 0
								for j in range(0,gc.getMAX_CIV_PLAYERS()) :
									if( not j == idx ) :
										if( capital.plot().getCulture(j) > maxSecondCult and gc.getPlayer(j) ) :
											secCultPlayer = gc.getPlayer(j)
											maxSecondCult = capital.plot().getCulture(j)

								if( maxSecondCult >= 0 and secCultPlayer.isAlive() ) :
									if( secCultPlayer.getCurrentEra() > playerI.getCurrentEra() ) :
										relations = playerI.AI_getAttitude( secCultPlayer.getID() )
										if( relations == AttitudeTypes.ATTITUDE_PLEASED or relations == AttitudeTypes.ATTITUDE_FRIENDLY ) :
											if( secCultPlayer.isHuman() ) :
												if( not idx in noAssimilateList ) :
													assimPlayer = secCultPlayer
											else :
												assimPlayer = secCultPlayer
											if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Assimilation to friendly, 2nd culture player")

						else :
							continue

			if( not assimPlayer == None ) :
				# Assimilate!
				# Enable only for debugging assimilation popup
				if( False ) :
					game.setForcedAIAutoPlay(assimPlayer.getID(), 1, true )
					iPrevHuman = game.getActivePlayer()
					RevUtils.changeHuman( assimPlayer.getID(), iPrevHuman )
				
				if( assimPlayer.isHuman() ) :
					# Zoom to city
					CyCamera().JustLookAt( capital.plot().getPoint() )
					
					# Additions by Caesium et al
					caesiumtR = CyUserProfile().getResolutionString(CyUserProfile().getResolution())
					caesiumtextResolution = caesiumtR.split('x')
					caesiumpasx = int(caesiumtextResolution[0])/10
					caesiumpasy = int(caesiumtextResolution[1])/10
					popup = PyPopup.PyPopup(RevDefs.assimilationPopup, contextType = EventContextTypes.EVENTCONTEXT_ALL, bDynamic = False)
					if( centerPopups ) : popup.setPosition(3*caesiumpasx,3*caesiumpasy)
					# Additions by Caesium et al
					#popup.setHeaderString( 'Assimilation' )
					bodStr = localText.getText("TXT_KEY_REV_ASSIM_POPUP", ())%(playerI.getCivilizationDescription(0),playerI.getCivilizationDescription(0))
					if( bRiskWar ) :
						bodStr += '\n\n' + localText.getText("TXT_KEY_REV_ASSIM_POPUP_REBEL", ())%(gc.getPlayer(motherLandID).getCivilizationDescription(0))
					popup.setBodyString( bodStr )
					popup.addSeparator()
					popup.addButton( localText.getText("TXT_KEY_REV_BUTTON_ACCEPT",()) )
					popup.addButton( localText.getText("TXT_KEY_REV_BUTTON_MAYBE_LATER",()) )
					popup.addButton( localText.getText("TXT_KEY_REV_BUTTON_NEVER",()) )
					popup.setUserData( (playerI.getID(),assimPlayer.getID(),bRiskWar) )
					popup.launch(bCreateOkButton = False)

				else :
					if( bRiskWar ) :
						# Assimilating a rebel involves potential war declaration, attitude issues
						gc.getPlayer(motherLandID).AI_changeAttitudeExtra( assimPlayer.getID(), gc.getPlayer(motherLandID).AI_getAttitudeExtra(playerI.getID()) )
						if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - The %s (motherland of the rebel %s) is considering attacking the %s over the assimilation"%(gc.getPlayer(motherLandID).getCivilizationDescription(0),playerI.getCivilizationDescription(0),assimPlayer.getCivilizationDescription(0)))
						[iOdds,attackerTeam,victimTeam] = RevUtils.computeWarOdds( gc.getPlayer(motherLandID), assimPlayer, capital.area(), False, True, True )
						if( attackerTeam.canDeclareWar(victimTeam.getID()) and iOdds > game.getSorenRandNum(100, 'Revolution: War') ) :
							if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Rebel motherland takes exception to assimilation, team %d declare war on team %d"%(attackerTeam.getID(),victimTeam.getID()))
							attackerTeam.declareWar( victimTeam.getID(), True, WarPlanTypes.NO_WARPLAN )
						
					assimPlayer.assimilatePlayer(playerI.getID())

def assimilateHandler( iPlayerID, netUserData, popupReturn ) :

		global noAssimilateList
		
		if( popupReturn.getButtonClicked() == 0 ) :
			if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Assimilation accepted!")
			if( netUserData[2] ) :
				pMotherland = gc.getPlayer( RevData.revObjectGetVal( gc.getPlayer(netUserData[0]), 'MotherlandID' ) )
				pMotherland.AI_changeAttitudeExtra( netUserData[1], pMotherland.AI_getAttitudeExtra(netUserData[0]) )
				if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Rebel motherland %s extra attidude to %s now %d"%(pMotherland.getCivilizationDescription(0), gc.getPlayer(netUserData[1]).getCivilizationDescription(0), pMotherland.AI_getAttitudeExtra(netUserData[0])))
				[iOdds,attackerTeam,victimTeam] = RevUtils.computeWarOdds( pMotherland, gc.getPlayer(netUserData[1]), gc.getPlayer(netUserData[0]).getCapitalCity().area(), False, True, True )
				if( attackerTeam.canDeclareWar(victimTeam.getID()) and iOdds > game.getSorenRandNum(100, 'Revolution: War') ) :
					if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Rebel motherland takes exception, team %d declare war on team %d"%(attackerTeam.getID(),victimTeam.getID()))
					attackerTeam.declareWar( victimTeam.getID(), True, WarPlanTypes.NO_WARPLAN )
			
			gc.getPlayer(netUserData[1]).assimilatePlayer(netUserData[0])
		elif( popupReturn.getButtonClicked() == 1 ) :
			if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Assimilation postponed")
			pass
		else :
			if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Assimilation rejected!")
			noAssimilateList.append( netUserData[0] )

def doSmallRevolts( iPlayer ) :
		# Small revolts are short duration disorder striking a city, shutting
		# down production and culture, etc

		playerPy = PyPlayer( iPlayer )
		cityList = playerPy.getCityList()

		for city in cityList :
			pCity = city.GetCy()

			revIdx = pCity.getRevolutionIndex()

			if( revIdx > int(1.25*RevDefs.revReadyFrac*RevDefs.revInstigatorThreshold) ) :
				if( pCity.getOccupationTimer() > 0 ) :
					# Already in a revolt
					continue
					
				if( pCity.getRevolutionCounter() > 0 ) :
					continue
					
				if( RevData.getCityVal(pCity, 'SmallRevoltCounter') > 0 ) :
					continue

				localRevIdx = pCity.getLocalRevIndex()
				if( localRevIdx > 0 ) :
					localFactor = min([1+localRevIdx/3,10])
				else :
					localFactor = max([localRevIdx-1,-15])

				iOdds = int( 100.0*revIdx/(8.0*RevDefs.alwaysViolentThreshold) )
				iOdds += localFactor
				iOdds = min([iOdds,15])

				if( game.getSorenRandNum( 100, "Rev: Small Revolt" ) < iOdds ) :
					if( LOG_DEBUG ) : CvUtil.pyPrint("  Revolt - Small revolt in %s with odds %d (%d idx, %d loc)"%(pCity.getName(),iOdds,revIdx,localRevIdx))
					pCity.setOccupationTimer(2)
					
					RevData.setCityVal(pCity, 'SmallRevoltCounter', 6)

					mess = localText.getText("TXT_KEY_REV_MESS_SMALL_REVOLT",())%(pCity.getName())
					CyInterface().addMessage(iPlayer, false, gc.getDefineINT("EVENT_MESSAGE_TIME"), mess, "AS2D_CITY_REVOLT", InterfaceMessageTypes.MESSAGE_TYPE_MINOR_EVENT, CyArtFileMgr().getInterfaceArtInfo("INTERFACE_RESISTANCE").getPath(), ColorTypes(7), pCity.getX(), pCity.getY(), true, true)
