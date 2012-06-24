## RevolutionDCM Options Tab
##
## Tab to configure all RevolutionDCM options.
##
## Copyright (c) 2007-2008 The BUG Mod.
##
## Author: Glider1

from CvPythonExtensions import *
import BugOptionsTab
gc = CyGlobalContext()

class RevDCMOptionsTab(BugOptionsTab.BugOptionsTab):

	def __init__(self, screen):
		BugOptionsTab.BugOptionsTab.__init__(self, "RevDCM", "RevDCM")

	def create(self, screen):
		game = gc.getGame()
		networkGame =  game.isGameMultiPlayer()
		tab = self.createTab(screen)
		panel = self.createMainPanel(screen)
		column = self.addOneColumnLayout(screen, panel)
		left, right = self.addTwoColumnLayout(screen, column, "Options", False)
	
		if not networkGame:
			#Interface options
			interfaceOptionsTitle = "Interface Options:"
			#localText.changeTextColor (generalOptionsTitle, gc.getInfoTypeForString("Blue"))
			self.addLabel(screen, left, "RevDCM__RevDCMInterfaceOptions")
			col1, col2, col3, col4 = self.addMultiColumnLayout(screen, right, 4, "HiddenAttitude")
			self.addCheckbox(screen, col1, "RevDCM__RevDCMHiddenAttitude")

			screen.attachHSeparator(left, left + "SepInterface1")
			screen.attachHSeparator(right, right + "SepInterface2")

			#DCM options
			self.addLabel(screen, left, "RevDCM__RevDCM_seige")
			col1, col2, col3 = self.addMultiColumnLayout(screen, right, 3, "DCM_Seige_Events")
			self.addCheckbox(screen, col1, "RevDCM__DCM_RANGE_BOMBARD")
			self.addCheckbox(screen, col2, "RevDCM__DCM_OPP_FIRE")
			self.addLabel(screen, left, "RevDCM__RevDCM_general")
			col1, col2, col3, col4 = self.addMultiColumnLayout(screen, right, 4, "DCM_Events")
			self.addCheckbox(screen, col1, "RevDCM__DCM_BATTLE_EFFECTS")
			self.addCheckbox(screen, col2, "RevDCM__DCM_ARCHER_BOMBARD")
			self.addCheckbox(screen, col3, "RevDCM__DCM_STACK_ATTACK")
			self.addCheckbox(screen, col4, "RevDCM__DCM_ATTACK_SUPPORT")
			self.addLabel(screen, left, "RevDCM__RevDCM_air")
			col1, col2, col3 = self.addMultiColumnLayout(screen, right, 3, "DCM_Air_Events")
			self.addCheckbox(screen, col1, "RevDCM__DCM_ACTIVE_DEFENSE")
			self.addCheckbox(screen, col2, "RevDCM__DCM_FIGHTER_ENGAGE")
			self.addCheckbox(screen, col3, "RevDCM__DCM_AIR_BOMBING")
			self.addLabel(screen, left, "RevDCM__RevDCM_naval")
			col1, col2, col3 = self.addMultiColumnLayout(screen, right, 3, "DCM_Naval_Events")
			self.addCheckbox(screen, col1, "RevDCM__DCM_NAVAL_BOMBARD")

			screen.attachHSeparator(left, left + "SepDCM1")
			screen.attachHSeparator(right, right + "SepDCM2")

			#IDW options
			self.addLabel(screen, left, "RevDCM__RevDCMIDW")
			col1, col2, col3 = self.addMultiColumnLayout(screen, right, 3, "IDW_Events1")
			self.addCheckbox(screen, col1, "RevDCM__IDW_ENABLED")
			self.addCheckbox(screen, col2, "RevDCM__IDW_EMERGENCY_DRAFT_ENABLED")
			self.addLabel(screen, left, "RevDCM__RevDCMIDWINFLUENCE")
			col1, col2, col3 = self.addMultiColumnLayout(screen, right, 3, "IDW_Events2")
			self.addCheckbox(screen, col1, "RevDCM__IDW_PILLAGE_INFLUENCE_ENABLED")
			self.addCheckbox(screen, col2, "RevDCM__IDW_NO_NAVAL_INFLUENCE")
			self.addCheckbox(screen, col3, "RevDCM__IDW_NO_BARBARIAN_INFLUENCE")

			screen.attachHSeparator(left, left + "SepIDW1")
			screen.attachHSeparator(right, right + "SepIDW2")

			#Super Spies options
			if (not game.isOption(GameOptionTypes.GAMEOPTION_NO_ESPIONAGE)):
				self.addLabel(screen, left, "RevDCM__RevDCMSS")
				col1, col2, col3 = self.addMultiColumnLayout(screen, right, 3, "SS_Events1")
				self.addCheckbox(screen, col1, "RevDCM__SS_ENABLED")
				self.addCheckbox(screen, col2, "RevDCM__SS_BRIBE")
				self.addCheckbox(screen, col3, "RevDCM__SS_ASSASSINATE")

			screen.attachHSeparator(left, left + "SepInq1")
			screen.attachHSeparator(right, right + "SepInq2")

			#Inquisitions options
			if (not game.isOption(GameOptionTypes.GAMEOPTION_NO_INQUISITIONS)):
				self.addLabel(screen, left, "RevDCM__RevDCMINQ")
				col1, col2, col3 = self.addMultiColumnLayout(screen, right, 3, "INQ_Events1")
				self.addCheckbox(screen, col2, "RevDCM__OC_RESPAWN_HOLY_CITIES")
			
				screen.attachHSeparator(left, left + "SepRel1")
				screen.attachHSeparator(right, right + "SepRel2")

			#Religion options
			self.addLabel(screen, left, "RevDCM__RevDCMReligion")
			col1, col2, col3 = self.addMultiColumnLayout(screen, right, 3, "Rel_Events1")
			self.addCheckbox(screen, col1, "RevDCM__OC_LIMITED_RELIGIONS")
			self.addCheckbox(screen, col2, "RevDCM__CHOOSE_RELIGION")

			screen.attachHSeparator(left, left + "SepEnd1")
			screen.attachHSeparator(right, right + "SepEnd2")

			#General options
			generalOptionsTitle = "General Options:"
			#localText.changeTextColor (generalOptionsTitle, gc.getInfoTypeForString("Blue"))
			self.addLabel(screen, left, "RevDCM__RevDCMGeneralOptions")
			col1, col2, col3, col4 = self.addMultiColumnLayout(screen, right, 4, "General_Events")
			self.addCheckbox(screen, col1, "RevDCM__RevDCMReset")

			screen.attachHSeparator(left, left + "SepGeneral1")
			screen.attachHSeparator(right, right + "SepGeneral2")
			
			
		else:
			self.addLabel(screen, left, "RevDCM_network_game")
			self.addLabel(screen, right, "RevDCM_network_game1")
			
		
		#On screen information
		self.addLabel(screen, left, "RevDCM_info", "Notes:")
		self.addLabel(screen, right, "RevDCM_info1")
		self.addLabel(screen, right, "RevDCM_info2")
		self.addLabel(screen, right, "RevDCM_info3")
		self.addLabel(screen, right, "RevolutionDCMHelp")

