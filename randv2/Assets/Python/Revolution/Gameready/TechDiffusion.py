# TechDiffusion
#
# by jdog5000
# Version 1.2
#


from CvPythonExtensions import *
import CvUtil
import PyHelpers
import Popup as PyPopup
# --------- Revolution mod -------------
import RevDefs
import BugCore

# globals
gc = CyGlobalContext()
PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo
game = CyGame()
localText = CyTranslator()
RevOpt = BugCore.game.Revolution

class TechDiffusion :

	def __init__(self, customEM, RevOpt ) :

		self.RevOpt = RevOpt
		self.customEM = customEM

		print "Initializing TechDiffusion Mod"

		self.LOG_DEBUG = RevOpt.isTechDifDebugMode()
		self.techDict = None

		self.minTechsBehind = RevOpt.getMinTechsBehind()
		self.fullEffectTechsBehind = RevOpt.getFullEffectTechsBehind()
		self.bonusTechsBehind = RevOpt.getBonusTechsBehind()
		self.diffusionMod = RevOpt.getDiffusionMod()

		self.customEM.addEventHandler( "BeginGameTurn", self.onBeginGameTurn )
		self.customEM.addEventHandler( "techAcquired", self.onTechAcquired )

		if( self.LOG_DEBUG ) :
			#self.customEM.addEventHandler( "kbdEvent", self.onKbdEvent )
			pass

	def removeEventHandlers( self ) :
		print "Removing event handlers from TechDiffusion"

		self.customEM.removeEventHandler( "BeginGameTurn", self.onBeginGameTurn )
		self.customEM.removeEventHandler( "techAcquired", self.onTechAcquired )

	def blankHandler( self, playerID, netUserData, popupReturn ) :

		# Dummy handler to take the second event for popup
		return

	def onGameStart(self, argsList) :

		pass


	def onKbdEvent(self, argsList ):
		'keypress handler'
		eventType,key,mx,my,px,py = argsList

		if ( eventType == RevDefs.EventKeyDown ):
			theKey = int(key)

			# For debug or trial only
			if( theKey == int(InputTypes.KB_B) and self.customEM.bShift and self.customEM.bCtrl ) :
				pass


	def onBeginGameTurn( self, argsList ) :

		if( self.techDict == None ) :
			self.initializeTechDict()

		currentMaxEra = 0
		for i in range(0,gc.getMAX_CIV_PLAYERS()) :
			if( gc.getPlayer(i).getCurrentEra() > currentMaxEra ) :
				currentMaxEra = gc.getPlayer(i).getCurrentEra()

		curMaxTechCount = 0
		numTechsByTeam = dict()
		for techID in range(0,gc.getNumTechInfos()) :
			for teamID in self.techDict[techID] :
				if( teamID in numTechsByTeam ) :
					numTechsByTeam[teamID] += 1
				else :
					numTechsByTeam[teamID] = 1

		curMaxTechCount = max(numTechsByTeam.values())

		for i in range(0,gc.getMAX_CIV_TEAMS()) :

			teamI = gc.getTeam(i)

			# Start diffusing if team is behind and
			# either has alphabet or is really far behind
			if( teamI.isAlive() and teamI.getNumCities() > 0 ) :
				if( teamI.isTechTrading() or curMaxTechCount - numTechsByTeam.get(i,0) > self.bonusTechsBehind ) :
					if( numTechsByTeam.get(i,0) <= curMaxTechCount - self.minTechsBehind ) :
						self.diffuseTech( i, numTechsByTeam.get(i,0), currentMaxEra, curMaxTechCount )

	def diffuseTech( self, iTeamID, teamTechCount, currentMaxEra, curMaxTechCount ) :

		gameSpeedMod = (661.0)/(game.getEstimateEndTurn() + 1.0)
		if( gameSpeedMod > 50.0 ) : gameSpeedMod = 1.0

		team = gc.getTeam( iTeamID )
		teamLeader = gc.getPlayer( team.getLeaderID() )
		teamEra = teamLeader.getCurrentEra()

		if( self.LOG_DEBUG ) :
			CvUtil.pyPrint( " " )
			CvUtil.pyPrint( " TD : %s (Team %d) qualifies for tech welfare with %d of %d techs"%(teamLeader.getCivilizationDescription(0),iTeamID, teamTechCount, curMaxTechCount) )

		# Increases tech diffusion rate as time goes on
		curMaxTechMod = min([1.0, pow(curMaxTechCount/(1.0*gc.getNumTechInfos()), 0.5)] )

		for techID in range(0,gc.getNumTechInfos()) :

			if( team.isNoTradeTech(techID) ) :
				continue

			techTeamList = self.techDict[techID]

			if( not (len(techTeamList) == 0) and not iTeamID in techTeamList ) :
				if( team.isHasTech(techID) ) :
					CvUtil.pyPrint( " TD : ERROR team not in tech dictionary, re-tabulating")
					self.initializeTechDict()
					continue

				tech = gc.getTechInfo( techID )
				techCost = team.getResearchCost(techID)
				techPower = 1.0*tech.getPowerValue()

				iResearchProgress = team.getResearchProgress(techID)
				if( 1.0*iResearchProgress > curMaxTechMod*techCost and not teamTechCount < curMaxTechCount - 11 ) :
					# Already far along, no more diffusion
					if( self.LOG_DEBUG ) : CvUtil.pyPrint( " TD : Not diffusing %s to %s, already well along in research"%(tech.getDescription(),teamLeader.getCivilizationDescription(0)))
					continue

				if( tech.getEra() == gc.getNumEraInfos() ) :
					# Don't diffuse future tech
					continue

				anyAndFalse = False
				for k in range(gc.getDefineINT("NUM_AND_TECH_PREREQS")):
					iPrereqTech = tech.getPrereqAndTechs( k )
					if( iPrereqTech > 0 and iPrereqTech < gc.getNumTechInfos() ) :
						if( not team.isHasTech( iPrereqTech ) ) :
							anyAndFalse = True
							break

				if( anyAndFalse ) :
					# Missing a required pre-req, no diffusion
					if( self.LOG_DEBUG ) : CvUtil.pyPrint( " TD : Not diffusing %s to %s, don't have all and pre-reqs"%(tech.getDescription(),teamLeader.getCivilizationDescription(0)))
					continue

				allOrFalse = -1
				for k in range(gc.getDefineINT("NUM_OR_TECH_PREREQS")):
					iPrereqTech = tech.getPrereqOrTechs( k )
					if( iPrereqTech > 0 and iPrereqTech < gc.getNumTechInfos() ) :
						allOrFalse = 1
						if( team.isHasTech( iPrereqTech ) ) :
							allOrFalse = 0
							break

				if( allOrFalse > 0 ) :
					# Don't have any pre-reqs, no diffusion
					if( self.LOG_DEBUG ) : CvUtil.pyPrint( " TD : Not diffusing %s to %s, don't have any or pre-reqs"%(tech.getDescription(),teamLeader.getCivilizationDescription(0)))
					continue

				# Found a tech that perhaps could diffuse to this team
				if( self.LOG_DEBUG ) : CvUtil.pyPrint( " TD : %s (Team %d), can receive tech %s cost %1.0f, power %1.0f"%(teamLeader.getCivilizationDescription(0),iTeamID,tech.getDescription(),techCost,techPower) )

				diffPower = 0.0

				for iTechTeam in techTeamList :
					# Relations of teams determine amount of contact with that tech
					if( team.canContact(iTechTeam) and gc.getTeam(iTechTeam).isAlive() and gc.getTeam(iTechTeam).isTechTrading() ) :

						if( team.isAtWar(iTechTeam) ) :
							diffPower += 0.5
						elif( team.isDefensivePact(iTechTeam) ) :
							diffPower += 4
						elif( team.isOpenBorders(iTechTeam) ) :
							diffPower += 3
						else :
							diffPower += 1

						if( teamEra < gc.getNumEraInfos() - 1 ) :
							# Long distance diffusion limited in early eras
							# Perhaps handled sufficiently by contact, general era mod?
							pass

				if( diffPower > 16.0 ) :
					diffPower = 16.0

				if( diffPower <= 2.0 ) :
					if( self.LOG_DEBUG ) : CvUtil.pyPrint( " TD : Not diffusing do to lack of diff power")
					continue
				else :
					if( self.LOG_DEBUG ) : CvUtil.pyPrint( " TD : Diffusion power is %1.1f with %d teams alive"%(diffPower,game.countCivTeamsAlive()) )

				# Turn down diffusion power for powerful (usually military) techs nations would keep secret
				if( techPower > 20.0 ) :
					# Max is 12 for standard Civ techs
					techPower = 20.0
				diffPower = diffPower*(1.0 - pow(techPower,.5)/5.0)

				diffPower = pow( diffPower, 1.0 )

				# Turn diffusion power into a percentage of research cost
				researchPercent = min([curMaxTechMod*self.diffusionMod*(diffPower)/(2.0*100.0), 0.07])
				researchPercent = researchPercent

				if( teamTechCount > curMaxTechCount - self.fullEffectTechsBehind ) :
					researchPercent = researchPercent*(curMaxTechCount - teamTechCount)/(1.0*self.fullEffectTechsBehind)

				# Extra boost if really far back
				if( teamTechCount <= curMaxTechCount - self.bonusTechsBehind ) :
					if( self.LOG_DEBUG ) : CvUtil.pyPrint( " TD : Team is way behind")
					researchPercent = researchPercent*pow((curMaxTechCount-teamTechCount-self.bonusTechsBehind + 2)/2.0, 0.5 )

				if( self.LOG_DEBUG ) : CvUtil.pyPrint( " TD : Percent to add to research: %1.2f (%d points, rate would be %d)"%(100*researchPercent,int(researchPercent*techCost + 0.5),teamLeader.calculateResearchRate(techID)) )

				if( researchPercent*techCost > 0.5 ) :
					# Effect must round up to at least one beaker
					iResearchCredit = max([int(researchPercent*techCost + 0.5), 1])
					iNewProgress = min([iResearchProgress + iResearchCredit, techCost])
					if( 1.0*iNewProgress > curMaxTechMod*techCost and not teamTechCount <= curMaxTechCount - self.bonusTechsBehind  ) :
						iNewProgress =  int(curMaxTechMod*techCost + 0.5)

					team.setResearchProgress( techID, iNewProgress, teamLeader.getID() )
				else :
					# Not enough diffusion power, no research credit
					pass

	def initializeTechDict( self ) :

		if( self.LOG_DEBUG ) : CvUtil.pyPrint( " TD : Initializing tech dictionary" )

		# This dictionary contains a list of which teams have each tech
		self.techDict = dict()
		for techID in range(0,gc.getNumTechInfos()) :
			teamList = list()
			for teamID in range(0,gc.getMAX_CIV_TEAMS()) :
				if( gc.getTeam(teamID).isHasTech(techID) ) :
					teamList.append(teamID)

			self.techDict[techID] = teamList

	def onTechAcquired( self, argsList ) :
		'Tech Acquired'
		iTechType, iTeam, iPlayer, bAnnounce = argsList

		if( self.techDict == None ) :
			self.initializeTechDict( )

		techTeamList = self.techDict[iTechType]

		if( (iTeam >= 0) and (iTeam <= gc.getMAX_CIV_TEAMS()) ) :
			teamI = gc.getTeam(iTeam)
			if(teamI.isAlive()) :
				if( not iTeam in techTeamList ) :
					techTeamList.append( iTeam )
					self.techDict[iTechType] = techTeamList
				else :
					# Dictionary can become outdated when a rebel civ dies, is reborn ...
					if( self.LOG_DEBUG ) : CvUtil.pyPrint( " TD : WARNING: Team already in tech dictionary, re-tabulating")
					self.initializeTechDict()
