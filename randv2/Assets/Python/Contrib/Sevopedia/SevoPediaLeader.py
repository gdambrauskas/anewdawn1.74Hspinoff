# Sid Meier's Civilization 4
# Copyright Firaxis Games 2005

#
# Sevopedia 2.3
#   sevotastic.blogspot.com
#   sevotastic@yahoo.com
#
# additional work by Gaurav, Progor, Ket, Vovan, Fitchn, LunarMongoose
# see ReadMe for details
#

from CvPythonExtensions import *
import CvUtil
import ScreenInput
import SevoScreenEnums
import random

gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

class SevoPediaLeader:

	def __init__(self, main):
		self.iLeader = -1
		self.top = main

		self.X_LEADERHEAD_PANE = self.top.X_PEDIA_PAGE
		self.Y_LEADERHEAD_PANE = self.top.Y_PEDIA_PAGE
		self.W_LEADERHEAD_PANE = 240
		self.H_LEADERHEAD_PANE = 290

		self.W_LEADERHEAD = self.W_LEADERHEAD_PANE - 30
		self.H_LEADERHEAD = self.H_LEADERHEAD_PANE - 34
		self.X_LEADERHEAD = self.X_LEADERHEAD_PANE + (self.W_LEADERHEAD_PANE - self.W_LEADERHEAD) / 2
		self.Y_LEADERHEAD = self.Y_LEADERHEAD_PANE + (self.H_LEADERHEAD_PANE - self.H_LEADERHEAD) / 2 + 3

		self.W_CIV = 64
		self.H_CIV = 64
		self.X_CIV = self.X_LEADERHEAD_PANE + (self.W_LEADERHEAD_PANE - self.W_CIV) / 2
		self.Y_CIV = self.Y_LEADERHEAD_PANE + self.H_LEADERHEAD_PANE + 10
		self.W_CIVS = 240
		self.H_CIVS = 110
		self.X_CIVS = self.X_LEADERHEAD_PANE + (self.W_LEADERHEAD_PANE - self.W_CIVS) / 2
		self.Y_CIVS = self.Y_LEADERHEAD_PANE + self.H_LEADERHEAD_PANE + 10

		self.X_CIVIC = self.X_LEADERHEAD_PANE + self.W_LEADERHEAD_PANE + 10
		self.Y_CIVIC = self.Y_LEADERHEAD_PANE
		self.W_CIVIC = self.top.R_PEDIA_PAGE - self.X_CIVIC
		self.H_CIVIC = 80

		self.X_RELIGION = self.X_CIVIC + self.W_CIVIC + 10
		self.Y_RELIGION = self.Y_LEADERHEAD_PANE
		self.W_RELIGION = self.top.R_PEDIA_PAGE - self.X_RELIGION
		self.H_RELIGION = 60

		self.X_TRAITS = self.X_LEADERHEAD_PANE + self.W_LEADERHEAD_PANE + 10
		self.Y_TRAITS = self.Y_CIVIC + self.H_CIVIC + 10
		self.W_TRAITS = self.top.R_PEDIA_PAGE - self.X_TRAITS
		self.H_TRAITS = self.Y_CIV + self.H_CIV - self.Y_TRAITS - 124#157 Roamty

		self.X_STATS = self.X_LEADERHEAD_PANE + self.W_LEADERHEAD_PANE + 10
		self.Y_STATS = self.Y_TRAITS + self.H_TRAITS + 10
		self.W_STATS = self.top.R_PEDIA_PAGE - self.X_STATS
		self.H_STATS = 169#self.Y_CIV + self.H_CIV - self.Y_STATS Roamty

		self.X_HISTORY = self.X_LEADERHEAD_PANE
		self.Y_HISTORY = self.Y_CIVS + self.H_CIVS + 10
		self.W_HISTORY = self.top.R_PEDIA_PAGE - self.X_HISTORY
		self.H_HISTORY = 230#self.top.B_PEDIA_PAGE - self.Y_HISTORY Roamty



	def interfaceScreen(self, iLeader):
		self.iLeader = iLeader
		screen = self.top.getScreen()

		leaderPanelWidget = self.top.getNextWidgetName()
		screen.addPanel(leaderPanelWidget, "", "", True, True, self.X_LEADERHEAD_PANE, self.Y_LEADERHEAD_PANE, self.W_LEADERHEAD_PANE, self.H_LEADERHEAD_PANE, PanelStyles.PANEL_STYLE_BLUE50)
		self.leaderWidget = self.top.getNextWidgetName()
		screen.addLeaderheadGFC(self.leaderWidget, self.iLeader, AttitudeTypes.ATTITUDE_PLEASED, self.X_LEADERHEAD, self.Y_LEADERHEAD, self.W_LEADERHEAD, self.H_LEADERHEAD, WidgetTypes.WIDGET_GENERAL, -1, -1)

		self.placeHistory()
		self.placeCivic()
		self.placeReligion()
		self.placeCiv()
		self.placeTraits()
		self.placeStats()



	def placeCiv(self):
		screen = self.top.getScreen()
		civLeaderList = []
		for iCiv in range(gc.getNumCivilizationInfos()):
			civ = gc.getCivilizationInfo(iCiv)
			if (civ.isLeaders(self.iLeader)):
				civLeaderList.append((iCiv, civ))
		if (len(civLeaderList) > 1):
			panelName = self.top.getNextWidgetName()
			screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_CATEGORY_CIV", ()), "", False, True, self.X_CIVS, self.Y_CIVS, self.W_CIVS, self.H_CIVS, PanelStyles.PANEL_STYLE_BLUE50 )
			screen.attachLabel(panelName, "", "  ")
			def placeCivIcon(iCiv, civ):
				screen.attachImageButton(panelName, "", civ.getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIV, iCiv, 1, False)
		else:
			def placeCivIcon(iCiv, civ):
				screen.setImageButton(self.top.getNextWidgetName(), civ.getButton(), self.X_CIV, self.Y_CIV, self.W_CIV, self.H_CIV, WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIV, iCiv, -1)
		for iCiv, civ in civLeaderList:
			placeCivIcon(iCiv, civ)


	def placeTraits(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_TRAITS", ()), "", True, False, self.X_TRAITS, self.Y_TRAITS, self.W_TRAITS, self.H_TRAITS, PanelStyles.PANEL_STYLE_BLUE50)
		listName = self.top.getNextWidgetName()
		iNumCivs = 0
		iLeaderCiv = -1
		for iCiv in range(gc.getNumCivilizationInfos()):
			civ = gc.getCivilizationInfo(iCiv)
			if civ.isLeaders(self.iLeader):
				iNumCivs += 1
				iLeaderCiv = iCiv
		if iNumCivs == 1:
			szSpecialText = CyGameTextMgr().parseLeaderTraits(self.iLeader, iLeaderCiv, False, True)
		else:		
			szSpecialText = CyGameTextMgr().parseLeaderTraits(self.iLeader, -1, False, True)
		szSpecialText = szSpecialText[1:]
		screen.addMultilineText(listName, szSpecialText, self.X_TRAITS+5, self.Y_TRAITS+30, self.W_TRAITS-5, self.H_TRAITS-35, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)



	def placeStats(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_STATS", ()), "", True, True, self.X_STATS, self.Y_STATS, self.W_STATS, self.H_STATS, PanelStyles.PANEL_STYLE_BLUE50)

		iWonder = gc.getLeaderHeadInfo(self.iLeader).getWonderConstructRand()
		iAttitude = gc.getLeaderHeadInfo(self.iLeader).getBaseAttitude()
		iWarmonger = gc.getLeaderHeadInfo(self.iLeader).getWarmongerRespect()

		iFull = gc.getLeaderHeadInfo(self.iLeader).getMaxWarRand()
		iNearby = gc.getLeaderHeadInfo(self.iLeader).getMaxWarNearbyPowerRatio()
		iDistant = gc.getLeaderHeadInfo(self.iLeader).getMaxWarDistantPowerRatio()
		iAdjacent = gc.getLeaderHeadInfo(self.iLeader).getMaxWarMinAdjacentLandPercent()

		iLimited = gc.getLeaderHeadInfo(self.iLeader).getLimitedWarRand()
		iRatio = gc.getLeaderHeadInfo(self.iLeader).getLimitedWarPowerRatio()

		iDogpile = gc.getLeaderHeadInfo(self.iLeader).getDogpileWarRand()

		szStatsText = ""
		szStatsText += localText.getText("TXT_KEY_PEDIA_WONDER", (iWonder,))
		szStatsText += localText.getText("TXT_KEY_PEDIA_ATTITUDE", (iAttitude, iWarmonger,))
		szStatsText += localText.getText("TXT_KEY_PEDIA_FULL_WAR", (iFull, iNearby, iDistant, iAdjacent,))
		szStatsText += localText.getText("TXT_KEY_PEDIA_LIMITED_WAR", (iLimited, iRatio,))
		szStatsText += localText.getText("TXT_KEY_PEDIA_DOGPILE", (iDogpile,))

		listName = self.top.getNextWidgetName()
		screen.addMultilineText(listName, szStatsText, self.X_STATS+5, self.Y_STATS+30, self.W_STATS-10, self.H_STATS-35, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)



	def placeCivic(self):		
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_FAV_CIVIC_AND_RELIGION", ()), "", True, True, self.X_CIVIC, self.Y_CIVIC, self.W_CIVIC, self.H_CIVIC, PanelStyles.PANEL_STYLE_BLUE50)
		iCivic = gc.getLeaderHeadInfo(self.iLeader).getFavoriteCivic()
		if (-1 != iCivic):
			szCivicText = u"<link=literal>" + gc.getCivicInfo(iCivic).getDescription() + u"</link>"
			listName = self.top.getNextWidgetName()
			screen.addMultilineText(listName, szCivicText, self.X_CIVIC+5, self.Y_CIVIC+30, self.W_CIVIC-10, self.H_CIVIC-10, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)


	def placeReligion(self):		
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		iReligion = gc.getLeaderHeadInfo(self.iLeader).getFavoriteReligion()
		if (-1 != iReligion):
			szReligionText = u"<link=literal>" + gc.getReligionInfo(iReligion).getDescription() + u"</link>"
			listName = self.top.getNextWidgetName()
			screen.addMultilineText(listName, szReligionText, self.X_CIVIC+5, self.Y_CIVIC+50, self.W_CIVIC-10, self.H_CIVIC-10, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)



	def placeHistory(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, "", "", True, True, self.X_HISTORY, self.Y_HISTORY, self.W_HISTORY, self.H_HISTORY, PanelStyles.PANEL_STYLE_BLUE50)
		historyTextName = self.top.getNextWidgetName()
		CivilopediaText = gc.getLeaderHeadInfo(self.iLeader).getCivilopedia()
		CivilopediaText = u"<font=2>" + CivilopediaText + u"</font>"
		screen.attachMultilineText(panelName, historyTextName, CivilopediaText, WidgetTypes.WIDGET_GENERAL,-1,-1, CvUtil.FONT_LEFT_JUSTIFY)



	def handleInput (self, inputClass):
		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CHARACTER):
			if (inputClass.getData() == int(InputTypes.KB_0)):
				self.top.getScreen().performLeaderheadAction(self.leaderWidget, LeaderheadAction.LEADERANIM_GREETING)
			elif (inputClass.getData() == int(InputTypes.KB_6)):
				self.top.getScreen().performLeaderheadAction(self.leaderWidget, LeaderheadAction.LEADERANIM_DISAGREE)
			elif (inputClass.getData() == int(InputTypes.KB_7)):
				self.top.getScreen().performLeaderheadAction(self.leaderWidget, LeaderheadAction.LEADERANIM_AGREE)
			elif (inputClass.getData() == int(InputTypes.KB_1)):
				self.top.getScreen().setLeaderheadMood(self.leaderWidget, AttitudeTypes.ATTITUDE_FRIENDLY)
				self.top.getScreen().performLeaderheadAction(self.leaderWidget, LeaderheadAction.NO_LEADERANIM)
			elif (inputClass.getData() == int(InputTypes.KB_2)):
				self.top.getScreen().setLeaderheadMood(self.leaderWidget, AttitudeTypes.ATTITUDE_PLEASED)
				self.top.getScreen().performLeaderheadAction(self.leaderWidget, LeaderheadAction.NO_LEADERANIM)
			elif (inputClass.getData() == int(InputTypes.KB_3)):
				self.top.getScreen().setLeaderheadMood(self.leaderWidget, AttitudeTypes.ATTITUDE_CAUTIOUS)
				self.top.getScreen().performLeaderheadAction(self.leaderWidget, LeaderheadAction.NO_LEADERANIM)
			elif (inputClass.getData() == int(InputTypes.KB_4)):
				self.top.getScreen().setLeaderheadMood(self.leaderWidget, AttitudeTypes.ATTITUDE_ANNOYED)
				self.top.getScreen().performLeaderheadAction(self.leaderWidget, LeaderheadAction.NO_LEADERANIM)
			elif (inputClass.getData() == int(InputTypes.KB_5)):
				self.top.getScreen().setLeaderheadMood(self.leaderWidget, AttitudeTypes.ATTITUDE_FURIOUS)
				self.top.getScreen().performLeaderheadAction(self.leaderWidget, LeaderheadAction.NO_LEADERANIM)
			else:
				self.top.getScreen().leaderheadKeyInput(self.leaderWidget, inputClass.getData())
		return 0
