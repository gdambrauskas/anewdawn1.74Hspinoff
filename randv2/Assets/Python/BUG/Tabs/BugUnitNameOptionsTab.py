## BugUnitNameOptionsTab
##
## Tab for the BUG Unit Name Options.
##
## Copyright (c) 2007-2008 The BUG Mod.
##
## Author: Ruff_Hi

import BugOptionsTab

class BugUnitNameOptionsTab(BugOptionsTab.BugOptionsTab):
	"BUG Unit Name Options Screen Tab"
	
	def __init__(self, screen):
		BugOptionsTab.BugOptionsTab.__init__(self, "UnitNaming", "Unit Naming")

	def create(self, screen):
		tab = self.createTab(screen)
		panel = self.createMainPanel(screen)
		column = self.addOneColumnLayout(screen, panel)
	
		left, center, right = self.addThreeColumnLayout(screen, column, "Options")
		
		self.addCheckbox(screen, left, "UnitNaming__Enabled")
		self.addCheckbox(screen, center, "MiscHover__UpdateUnitNameOnUpgrade")
		self.addCheckbox(screen, right, "UnitNaming__UseAdvanced")

		columnL, columnR = self.addTwoColumnLayout(screen, column, "UnitNaming")
# Rise of Mankind 2.71 start
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Default")
		# self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_AIR")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_ANIMAL")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_ARCHER")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_ASSAULT_MECH")
		# self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_ARMOR")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_BIOLOGICAL")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_BOMBERS")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_CLONES")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_DREADNOUGHT")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_DIESEL_SHIPS")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_DOOM")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_EARLY_BOMBERS")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_EARLY_FIGHTERS")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_SPY")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_GUN")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_HELICOPTER")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_HITECH")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_JET_FIGHTERS")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_MELEE")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_MISSILE")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_MOUNTED")
		# self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_NAVAL")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_None")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_NANITE")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_NUCLEAR_SHIPS")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_RECON")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_ROBOT")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_SETTLER")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_SIEGE")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_STEALTH")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_STEAM_SHIPS")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_SUPERSONIC_PLANES")
		# self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_SPY")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_TRACKED")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_WHEELED")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_WOODEN_SHIPS")
		self.addTextEdit(screen, columnL, columnR, "UnitNaming__Combat_WORKER")
# Rise of Mankind 2.71 end
