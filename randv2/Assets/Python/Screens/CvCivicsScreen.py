## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvUtil
import ScreenInput
import CvScreenEnums
import string
import CvScreensInterface

import BugCore
BugOpt = BugCore.game.Advisors

import BugUtil

# globals
gc = CyGlobalContext()
game = gc.getGame()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

import PyHelpers
PyPlayer = PyHelpers.PyPlayer

class CvCivicsScreen:
	"Civics Screen"

	def __init__(self):
		self.USER_ADJUSTMENT = 15 # default 100
		self.SCREEN_NAME = "CivicsScreen"
		self.CANCEL_NAME = "CivicsCancel"
		self.EXIT_NAME = "CivicsExit"
		self.TITLE_NAME = "CivicsTitleHeader"
		self.TITLE_TOP_PANEL = "CivicsTopPanel"
		self.TITLE_BOTTOM_PANEL = "CivicsBottomPanel"
		self.BUTTON_NAME = "CivicsScreenButton"
		self.TEXT_NAME = "CivicsScreenText"
		self.AREA_NAME = "CivicsScreenArea"
		self.HELP_AREA_NAME = "CivicsScreenHelpArea"
		self.HELP_IMAGE_NAME = "CivicsScreenCivicOptionImage"
		self.DEBUG_DROPDOWN_ID =  "CivicsDropdownWidget"
		self.BACKGROUND_ID = "CivicsBackground"
		self.HELP_HEADER_NAME = "CivicsScreenHeaderName"
		self.QUICKVIEW_ADD = "QuickView"

		self.CivicsScreenInputMap = {
			self.BUTTON_NAME	: self.CivicsButton,
			self.TEXT_NAME		: self.CivicsButton,
			self.EXIT_NAME		: self.Revolution,
			self.CANCEL_NAME	: self.Cancel}

		self.iActivePlayer = -1

		self.m_paeCurrentCivics = []
		self.m_paeDisplayCivics = []
		self.m_paeOriginalCivics = []
		self.m_paiCivicOptionHeight = []

		self.HEADINGS_TOP = 0
	#	self.HEADINGS_BOTTOM = 220 Never Used
		self.HEADINGS_LEFT = 0
		self.HEADINGS_RIGHT = 260
		self.HELP_TOP = 20
		self.HELP_BOTTOM = 610
		self.HELP_LEFT = 290
		self.HELP_RIGHT = 950
		self.BUTTON_SIZE = 24
		self.BIG_BUTTON_SIZE = 64
		self.BOTTOM_LINE_TOP = 630
		self.BOTTOM_LINE_HEIGHT = 60
		self.Y_EXIT = 726
		self.Y_CANCEL = 726
		self.Y_SCREEN = 396
		self.H_SCREEN = 768
		self.Z_SCREEN = -6.1
		self.Y_TITLE = 8
		self.Z_TEXT = self.Z_SCREEN - 0.2
		self.TEXT_MARGIN = 11			# original = 15



	def setValues(self):
		screen = CyGInterfaceScreen("MainInterface", CvScreenEnums.MAIN_INTERFACE)
		resolutionWidth = 1024
		resolutionHeigth = 768


		self.H_SCREEN = resolutionHeigth
		self.W_SCREEN = resolutionWidth
		
# Afforess - Tech Screen Resolution - start
		self.TEXT_SIZE_SCALE = 1
		if (screen.getXResolution() > 1024):
			self.W_SCREEN = screen.getXResolution()
			self.HELP_RIGHT = (screen.getXResolution() - 74) #950 + 250
		if (screen.getYResolution() > 768):
			self.H_SCREEN = screen.getYResolution()
# Afforess - Tech Screen Resolution - end
#DancinhHoskuld - max help characters per line - start
		self.MAX_HELP_CHARS_PER_LINE = int(screen.getXResolution() / 8.5) - 40
#DancinhHoskuld - max help characters per line


		self.X_CANCEL = 20
		self.X_POSITION = 0
		self.Y_POSITION = 0
		self.PANEL_HEIGHT = 165
		self.QUICKVIEW_WIDTH = 150
		self.QUICKVIEW_TOP = 25 # was 35
		self.PANEL_WIDTH = 0
		self.INITIAL_SPACING = 30
		self.HEADINGS_WIDTH = 320
		#RevolutionDCM start - revolutions screen adjustment
		#if ((not game.isOption(GameOptionTypes.GAMEOPTION_NO_REVOLUTION)) and BugCore.game.RoMSettings.isShowRevCivics()):
		#	for l in range(gc.getNumCivicInfos()):
		#		self.HEADINGS_HEIGHT = (((40 + self.BUTTON_SIZE + self.TEXT_MARGIN) * l/self.CIVICCATEGORIES)/5 * 2) + 260 + self.USER_ADJUSTMENT
		#else:
		#	for l in range(gc.getNumCivicInfos()):
		#		self.HEADINGS_HEIGHT = (((40 + self.BUTTON_SIZE + self.TEXT_MARGIN) * l/self.CIVICCATEGORIES)/5 * 2) + 160 + self.USER_ADJUSTMENT
		#RevolutionDCM end
		self.RIGHT_LINE_WIDTH = 0
		self.SCROLL_BAR_SPACING = 40
		self.BOTTOM_LINE_TEXT_SPACING = 150
		self.BOTTOM_LINE_WIDTH = self.W_SCREEN
		self.BOTTOM_LINE_HEIGHT = 70

		self.X_SCREEN = self.W_SCREEN / 2
		self.Y_CANCEL = self.H_SCREEN - 40
		self.Y_EXIT = self.H_SCREEN - 40
		self.HELP_BOTTOM = self.H_SCREEN - self.PANEL_HEIGHT - self.BOTTOM_LINE_HEIGHT - 10

		self.BOTTOM_LINE_TOP = self.H_SCREEN - self.BOTTOM_LINE_HEIGHT
		self.X_EXIT = self.W_SCREEN - 30

	def getScreen(self):
		return CyGInterfaceScreen(self.SCREEN_NAME, CvScreenEnums.CIVICS_SCREEN)

	def setActivePlayer(self, iPlayer):

		self.iActivePlayer = iPlayer
		activePlayer = gc.getPlayer(iPlayer)

		self.m_paeCurrentCivics = []
		self.m_paeDisplayCivics = []
		self.m_paeOriginalCivics = []
		self.m_paiCivicOptionHeight = []
		for i in range (gc.getNumCivicOptionInfos()):
			self.m_paeCurrentCivics.append(activePlayer.getCivics(i))
			self.m_paeDisplayCivics.append(activePlayer.getCivics(i))
			self.m_paeOriginalCivics.append(activePlayer.getCivics(i))
			self.m_paiCivicOptionHeight.append(self.getRealHeadingHeight(i))



	def interfaceScreen (self):
		self.setValues()
		screen = self.getScreen()
		if screen.isActive():
			return
# Afforess - Tech Screen Resolution - start
		if (screen.getXResolution() > 1024):
			self.W_SCREEN = screen.getXResolution()
			self.X_SCREEN = int(self.W_SCREEN * 1.5)
# Afforess - Tech Screen Resolution - end
		screen.setRenderInterfaceOnly(True);
		screen.showScreen( PopupStates.POPUPSTATE_IMMEDIATE, False)
		screen.setDimensions(self.X_POSITION, self.Y_POSITION, self.W_SCREEN, self.H_SCREEN * 2)
		screen.addDDSGFC(self.BACKGROUND_ID, ArtFileMgr.getInterfaceArtInfo("SCREEN_BG_OPAQUE").getPath(), 0, 0, self.W_SCREEN, self.H_SCREEN, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		# Panels on the Top(name of screen) and bottom(Cancel, Exit, Revolution buttons)
		screen.addPanel( self.TITLE_TOP_PANEL, u"", u"", True, False, 0, 0, self.W_SCREEN, self.PANEL_HEIGHT, PanelStyles.PANEL_STYLE_TOPBAR )
		screen.addPanel( self.TITLE_BOTTOM_PANEL, u"", u"", True, False, 0, self.BOTTOM_LINE_TOP, self.W_SCREEN * 2, self.H_SCREEN - self.BOTTOM_LINE_TOP, PanelStyles.PANEL_STYLE_BOTTOMBAR )
		screen.addScrollPanel( self.QUICKVIEW_ADD, u"", 0, 0, self.W_SCREEN, self.PANEL_HEIGHT-20, PanelStyles.PANEL_STYLE_EXTERNAL )
		screen.setActivation( self.QUICKVIEW_ADD, ActivationTypes.ACTIVATE_NORMAL )
		screen.showWindowBackground(False)

		# Set the background and exit button, and show the screen
		screen.setDimensions((screen.getXResolution() - self.W_SCREEN), (screen.getYResolution() - self.H_SCREEN), self.W_SCREEN, self.H_SCREEN)

		# set the standard "exit" text, other text "cancel, revolution" are handled at the bottom in this python file
		screen.setText(self.CANCEL_NAME, "Background", u"<font=4>" + localText.getText("TXT_KEY_SCREEN_CANCEL", ()).upper() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, self.X_CANCEL, self.Y_CANCEL, self.Z_TEXT, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, 1, 0)

	# Header...
		screen.setText(self.TITLE_NAME, "Background", u"<font=4b>" + localText.getText("TXT_KEY_CIVICS_SCREEN_TITLE", ()).upper() + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, self.X_SCREEN, self.Y_TITLE, self.Z_TEXT, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)

		self.setActivePlayer(game.getActivePlayer())

		if (CyGame().cheatCodesEnabled() or gc.getTeam(gc.getGame().getActiveTeam()).getNumMembers() > 1):
			self.szDropdownName = self.DEBUG_DROPDOWN_ID
			screen.addDropDownBoxGFC(self.szDropdownName, 22, 12, 300, WidgetTypes.WIDGET_GENERAL, -1, -1, FontTypes.GAME_FONT)
			for j in range(gc.getMAX_PLAYERS()):
				if (gc.getPlayer(j).isAlive()):
					if (CyGame().cheatCodesEnabled() or gc.getPlayer(j).getTeam() == gc.getGame().getActiveTeam()):
						screen.addPullDownString(self.szDropdownName, gc.getPlayer(j).getName(), j, j, False )

		# Make the scrollable area for the civics list...
		screen.addScrollPanel( "CivicList", u"", self.PANEL_WIDTH, self.PANEL_HEIGHT - 15, self.W_SCREEN, self.HELP_BOTTOM, PanelStyles.PANEL_STYLE_EXTERNAL )
		screen.setActivation( "CivicList", ActivationTypes.ACTIVATE_NORMAL )


		# Draw Contents
		self.drawContents()

		return 0

	# Draw the contents...
	def drawContents(self):

		# Draw the radio buttons
		self.drawAllButtons()

		# Draw Help Text
		self.drawAllHelpText()

		# Update Maintenance/anarchy/etc.
		self.updateAnarchy()

	def drawCivicOptionButtons(self, iCivicOption):

		activePlayer = gc.getPlayer(self.iActivePlayer)
		screen = self.getScreen()

		for j in range(gc.getNumCivicInfos()):
			
			#Skip unresearchable techs
			if (self.canEverDoCivic(j) and gc.getCivicInfo(j).getCivicOptionType() == iCivicOption):
				screen.setState(self.getCivicsButtonName(j), self.m_paeCurrentCivics[iCivicOption] == j)
				screen.setState(self.getCivicsButtonName(j)+self.QUICKVIEW_ADD, self.m_paeCurrentCivics[iCivicOption] == j)

				if (self.m_paeDisplayCivics[iCivicOption] == j):
					screen.setState(self.getCivicsButtonName(j), True)
					screen.setState(self.getCivicsButtonName(j)+self.QUICKVIEW_ADD, True)
					screen.show(self.getCivicsButtonName(j))
					screen.show(self.getCivicsButtonName(j)+self.QUICKVIEW_ADD)
				elif (activePlayer.canDoCivics(j)):
					#screen.setState(self.getCivicsButtonName(j), False)
					screen.show(self.getCivicsButtonName(j))
					screen.show(self.getCivicsButtonName(j)+self.QUICKVIEW_ADD)
				else:
					screen.hide(self.getCivicsButtonName(j))
					screen.hide(self.getCivicsButtonName(j)+self.QUICKVIEW_ADD)

	# Will draw the radio buttons (and revolution)
	def drawAllButtons(self):

		for i in range(gc.getNumCivicOptionInfos()):

			screen = self.getScreen()

			#fY = self.HEADINGS_HEIGHT * i
			height = self.getHeadingHeight(i)
			fY = self.getHeadingHeightAt(i) - height
			fX = self.HEADINGS_LEFT

			ffX = 3 + self.QUICKVIEW_WIDTH * i
			ffY = self.QUICKVIEW_TOP + self.BIG_BUTTON_SIZE

			# draw the Top Screen panels for the civics
			szAreaID = self.AREA_NAME + str(i)
			screen.attachPanelAt( "CivicList", szAreaID, u"", u"", True, True, PanelStyles.PANEL_STYLE_MAIN, fX, fY + self.HEADINGS_TOP, self.HEADINGS_RIGHT - self.HEADINGS_LEFT, height, WidgetTypes.WIDGET_GENERAL, i, -1 )


			# draw the civic category(government, labor, ...)

			szCivicID = "CivicID" + str(i)
			screen.setTextAt( szCivicID, "CivicList", u"<font=4>" + gc.getCivicOptionInfo(i).getDescription() + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, fX + self.HEADINGS_WIDTH / 6, fY + 6, 0, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, i, -1 )
			screen.setActivation( szCivicID, ActivationTypes.ACTIVATE_MIMICPARENTFOCUS)
			fY += 3 * self.TEXT_MARGIN

			iCount = 0
			for j in range(gc.getNumCivicInfos()):
				if (self.canEverDoCivic(j)):
					if (gc.getCivicInfo(j).getCivicOptionType() == i):

						iCount += 1
						#Clone these to get the smaller checkboxes.  But you need to set up the clicking behavior manually, as well as enabling Mouseover Help information displays.
						screen.addCheckBoxGFCAt("CivicList", self.getCivicsButtonName(j), gc.getCivicInfo(j).getButton(), ArtFileMgr.getInterfaceArtInfo("BUTTON_HILITE_SQUARE").getPath(), fX + 12, fY, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_GENERAL, j, 1, ButtonStyles.BUTTON_STYLE_LABEL, False)
						screen.setActivation( self.getCivicsButtonName(j), ActivationTypes.ACTIVATE_NORMAL )
						screen.hide (self.getCivicsButtonName(j) )

						screen.addCheckBoxGFCAt(self.QUICKVIEW_ADD, self.getCivicsButtonName(j)+self.QUICKVIEW_ADD, gc.getCivicInfo(j).getButton(), ArtFileMgr.getInterfaceArtInfo("BUTTON_HILITE_SQUARE").getPath(), ffX, ffY, self.BUTTON_SIZE, self.BUTTON_SIZE, WidgetTypes.WIDGET_GENERAL, j, 1, ButtonStyles.BUTTON_STYLE_LABEL, False)
						screen.setActivation( self.getCivicsButtonName(j)+self.QUICKVIEW_ADD, ActivationTypes.ACTIVATE_NORMAL )
						screen.hide (self.getCivicsButtonName(j)+self.QUICKVIEW_ADD)

						# draw the civic names
						screen.setTextAt( self.getCivicsTextName(j), "CivicList", u"<font=3>   -   " + gc.getCivicInfo(j).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, fX + 12 + self.BUTTON_SIZE + 10 , fY, 0, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )

						# make sure the next line is put down under the previous
						fY += 2 * self.TEXT_MARGIN + 1
						ffX += 3 + self.BUTTON_SIZE
						if(iCount%5==0):
							ffX -= 5 * (3 + self.BUTTON_SIZE)
							ffY += 3 + self.BUTTON_SIZE

				self.drawCivicOptionButtons(i)
			
	def canEverDoCivic(self, iCivic):
		activePlayer = gc.getPlayer(self.iActivePlayer)
		if (gc.getCivicInfo(iCivic).getTechPrereq() != -1):
			if (not activePlayer.canEverResearch(gc.getCivicInfo(iCivic).getTechPrereq())):
				return False
		return True

	def highlight(self, iCivic):
		iCivicOption = gc.getCivicInfo(iCivic).getCivicOptionType()
		if self.m_paeDisplayCivics[iCivicOption] != iCivic:
			self.m_paeDisplayCivics[iCivicOption] = iCivic
			self.drawCivicOptionButtons(iCivicOption)
			return True
		return False

	def unHighlight(self, iCivic):
		iCivicOption = gc.getCivicInfo(iCivic).getCivicOptionType()
		if self.m_paeDisplayCivics[iCivicOption] != self.m_paeCurrentCivics[iCivicOption]:
			self.m_paeDisplayCivics[iCivicOption] = self.m_paeCurrentCivics[iCivicOption]
			self.drawCivicOptionButtons(iCivicOption)
			return True
		return False

	def select(self, iCivic):
		activePlayer = gc.getPlayer(self.iActivePlayer)
		if (not activePlayer.canDoCivics(iCivic)):
			# If you can't even do this, get out....
			return 0

		iCivicOption = gc.getCivicInfo(iCivic).getCivicOptionType()

		# Set the previous widget
		iCivicPrev = self.m_paeCurrentCivics[iCivicOption]

		# Switch the widgets
		self.m_paeCurrentCivics[iCivicOption] = iCivic

		# Unighlight the previous widget
		self.unHighlight(iCivicPrev)
		self.getScreen().setState(self.getCivicsButtonName(iCivicPrev), False)
		self.getScreen().setState(self.getCivicsButtonName(iCivicPrev)+self.QUICKVIEW_ADD, False)

		# highlight the new widget
		self.highlight(iCivic)
		self.getScreen().setState(self.getCivicsButtonName(iCivic), True)
		self.getScreen().setState(self.getCivicsButtonName(iCivic)+self.QUICKVIEW_ADD, True)

		return 0

	def CivicsButton(self, inputClass):

		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED) :
			if (inputClass.getFlags() & MouseFlags.MOUSE_RBUTTONUP):
				CvScreensInterface.pediaJumpToCivic((inputClass.getID(), ))
			else:
				# Select button
				self.select(inputClass.getID())
				self.drawHelpText(gc.getCivicInfo(inputClass.getID()).getCivicOptionType())
				self.updateAnarchy()
		elif (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_ON) :
			# Highlight this button
			if self.highlight(inputClass.getID()):
				self.drawHelpText(gc.getCivicInfo(inputClass.getID()).getCivicOptionType())
				self.updateAnarchy()
		elif (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_OFF) :
			if self.unHighlight(inputClass.getID()):
				self.drawHelpText(gc.getCivicInfo(inputClass.getID()).getCivicOptionType())
				self.updateAnarchy()

		return 0

	def drawHelpText(self, iCivicOption):

		activePlayer = gc.getPlayer(self.iActivePlayer)
		iCivic = self.m_paeDisplayCivics[iCivicOption]

		screen = self.getScreen()
		# make sure the string is empty, not needed really, but to be sure
		szHelpText = u""

		# spacing for the blue background help panel
		#fY = self.HEADINGS_HEIGHT * iCivicOption
		height = self.getHeadingHeight(iCivicOption)
		heightAt = self.getHeadingHeightAt(iCivicOption) - height
		
		fY = heightAt
		fX = self.INITIAL_SPACING + self.HELP_LEFT - self.PANEL_WIDTH + 2 * self.TEXT_MARGIN
		
		# draw the blue panels
		szHelpAreaID = self.HELP_AREA_NAME + str(iCivicOption)
		screen.attachPanelAt( "CivicList", szHelpAreaID, u"", u"", True, True, PanelStyles.PANEL_STYLE_MAIN, self.HELP_LEFT - self.PANEL_WIDTH + self.INITIAL_SPACING, fY, self.HELP_RIGHT - self.HELP_LEFT, height, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		# spacing for the help strings
		fY = self.HELP_TOP + heightAt
		fX = self.HELP_LEFT + self.HEADINGS_WIDTH/2 - 100		
		
		#upkeep
		szPaneIDUpkeep = "Upkeep" + str(iCivicOption)
		if ((gc.getCivicInfo(iCivic).getUpkeep() != -1) and not activePlayer.isNoCivicUpkeep(iCivicOption)):
			szHelpText = gc.getUpkeepInfo(gc.getCivicInfo(iCivic).getUpkeep()).getDescription() + u" - %d%c" %(activePlayer.getSingleCivicUpkeep(iCivic, true), gc.getCommerceInfo(CommerceTypes.COMMERCE_GOLD).getChar())
			screen.setTextAt( szPaneIDUpkeep, "CivicList", u"<font=4>" + szHelpText + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, fX + self.HEADINGS_WIDTH / 4 + 150, fY - self.TEXT_MARGIN , 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
			fY += 1.2 * self.TEXT_MARGIN
		else:
			szHelpText = localText.getText("TXT_KEY_CIVICS_SCREEN_NO_UPKEEP", ())
			screen.setTextAt( szPaneIDUpkeep, "CivicList", u"<font=4>" + szHelpText + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, fX + self.HEADINGS_WIDTH / 4  + 150, fY - self.TEXT_MARGIN, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
			fY += 1.2 * self.TEXT_MARGIN
			
		#Add Strategy text:
		szStrategy = gc.getCivicInfo(iCivic).getStrategy()
		if (len(szStrategy) > 2):
			#~ self.MAX_HELP_CHARS_PER_LINE = int(screen.getXResolution() / 8.5) - 40
			szStrategyList = szStrategy.split(' ')
			szFormattedStrategyList = []
			i = 0
			chars = 0
			szFormattedStrategyList.append("")
			for item in szStrategyList:
				if (chars + len(item) > self.MAX_HELP_CHARS_PER_LINE):
					szFormattedStrategyList.append(item)
					i += 1
					chars = len(item)
				else:
					if (len(item.splitlines()) > 1):
						#This doesn't work...
						szList = item.splitlines()
						for item2 in szList:
							szFormattedStrategyList.append(item2)
							i += 1
							chars = len(item2)
					else:
						szFormattedStrategyList[i] += " " + item
						chars += len(item)
				
			for item in szFormattedStrategyList:	
				item = BugUtil.colorText(item, "COLOR_DARK_GREY")
				szPanelIDHelpItem = "CivicStrategyText" + str(iCivicOption) + str(i)
				screen.setTextAt( szPanelIDHelpItem, "CivicList", u"<font=3>" + item + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, fX , fY + 6, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 ) #Afforess
				fY += 2 * self.TEXT_MARGIN #Afforess was 1.5
				i += 1

			
		szHelpText = CyGameTextMgr().parseCivicInfo(iCivic, False, True, True)
		HelpList = szHelpText.splitlines()
		i = 0
		for item in HelpList:
			szPanelIDHelpItem = "CivicListHelp" + str(iCivicOption) + str(i)
			screen.setTextAt( szPanelIDHelpItem, "CivicList", u"<font=3>" + item + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, fX , fY + 6, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 ) #Afforess
			fY += 2 * self.TEXT_MARGIN #Afforess was 1.5
			i += 1


		# johny smith ScreenTweaks LINE:
		fY = heightAt
		fX = self.INITIAL_SPACING + self.HELP_LEFT - self.BIG_BUTTON_SIZE
		ffX = (self.QUICKVIEW_WIDTH) * iCivicOption + (self.QUICKVIEW_WIDTH-self.BIG_BUTTON_SIZE)/2

		szHelpImageID = self.HELP_IMAGE_NAME + str(iCivicOption)
		screen.setImageButtonAt(szHelpImageID, "CivicList", gc.getCivicInfo(iCivic).getButton(), fX - self.PANEL_WIDTH + self.INITIAL_SPACING/17 * 2, fY + height/2 - self.BIG_BUTTON_SIZE/2, self.BIG_BUTTON_SIZE, self.BIG_BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIVIC, iCivic, 1)
		screen.setImageButtonAt(szHelpImageID+self.QUICKVIEW_ADD, self.QUICKVIEW_ADD, gc.getCivicInfo(iCivic).getButton(), ffX, self.QUICKVIEW_TOP, self.BIG_BUTTON_SIZE, self.BIG_BUTTON_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIVIC, iCivic, 1)

	
	def getHeadingHeight(self, iCivicOption):
		return self.m_paiCivicOptionHeight[iCivicOption]
		
	def getLinesOfHelpText(self, iCivicOption):
		lines = 0
		mostLines = -1
		for iCivic in range (gc.getNumCivicInfos()):
			if (gc.getCivicInfo(iCivic).getCivicOptionType() == iCivicOption):
				lines = 0
				szStrategy = gc.getCivicInfo(iCivic).getStrategy()
				#~ if (len(szStrategy) > 2):
					#~ szStrategyList = szStrategy.splitlines()
					#~ lines += len(szStrategyList)
#DancinhHoskuld - max help characters per line - start
				szStrategyList = szStrategy.split(' ')
				i = 0
				chars = 0
				for item in szStrategyList:
					if (chars + len(item) > self.MAX_HELP_CHARS_PER_LINE):
						i += 1
						chars = len(item)
				lines += i
#DancinhHoskuld - max help characters per line - end

				szHelpText = CyGameTextMgr().parseCivicInfo(iCivic, False, True, True)
				HelpList = szHelpText.splitlines()
				lines += len(HelpList)
				
				lines += 1
				
				if (lines > mostLines):
					mostLines = lines
		return mostLines
		
	def getRealHeadingHeight(self, iCivicOption):
		return 80 + self.getLinesOfHelpText(iCivicOption) * 25
		
	def getHeadingHeightAt(self, iCivicOption):
		height = self.getHeadingHeight(iCivicOption)
		for i in range (gc.getNumCivicOptionInfos()):
			if (i < iCivicOption):
				height += self.getHeadingHeight(i)
				
		return height
		
	# Will draw the help text
	def drawAllHelpText(self):
		for i in range (gc.getNumCivicOptionInfos()):
			if (self.canEverDoCivic(i)):
				screen = self.getScreen()
				self.drawHelpText(i)

	# Will Update the maintenance/anarchy/etc
	def updateAnarchy(self):

		screen = self.getScreen()

		activePlayer = gc.getPlayer(self.iActivePlayer)

		bChange = False
		i = 0
		while (i  < gc.getNumCivicOptionInfos() and not bChange):
			if (self.m_paeCurrentCivics[i] != self.m_paeOriginalCivics[i]):
				bChange = True
			i += 1

		# Make the revolution button
		screen.deleteWidget(self.EXIT_NAME)
		if (activePlayer.canRevolution(0) and bChange):
			screen.setText(self.EXIT_NAME, "Background", u"<font=4>" + localText.getText("TXT_KEY_CONCEPT_REVOLUTION", ( )).upper() + u"</font>", CvUtil.FONT_RIGHT_JUSTIFY, self.X_EXIT, self.Y_EXIT, self.Z_TEXT, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_REVOLUTION, 1, 0)
			screen.show(self.CANCEL_NAME)
			screen.moveToFront(self.CANCEL_NAME)
		else:
			screen.setText(self.EXIT_NAME, "Background", u"<font=4>" + localText.getText("TXT_KEY_PEDIA_SCREEN_EXIT", ( )).upper() + u"</font>", CvUtil.FONT_RIGHT_JUSTIFY, self.X_EXIT, self.Y_EXIT, self.Z_TEXT, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, 1, -1)
			screen.hide(self.CANCEL_NAME)

		# Anarchy
		iTurns = activePlayer.getCivicAnarchyLength(self.m_paeDisplayCivics);

		if (activePlayer.canRevolution(0)):
			szText = localText.getText("TXT_KEY_ANARCHY_TURNS", (iTurns, ))
		else:
			szText = CyGameTextMgr().setRevolutionHelp(self.iActivePlayer)

		screen.setLabel("CivicsRevText", "Background", u"<font=3>" + szText + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, (self.X_SCREEN / 2.5) + self.BOTTOM_LINE_TEXT_SPACING, self.Y_EXIT + 2, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)

		# Maintenance
		szText = localText.getText("TXT_KEY_CIVIC_SCREEN_UPKEEP", (activePlayer.getCivicUpkeep(self.m_paeDisplayCivics, True), ))
		screen.setLabel("CivicsUpkeepText", "Background", u"<font=3>" + szText + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, self.X_SCREEN / 5 + self.BOTTOM_LINE_TEXT_SPACING * 1 / 4, self.Y_CANCEL + 1, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
		


	# Revolution!!!
	def Revolution(self, inputClass):

		activePlayer = gc.getPlayer(self.iActivePlayer)

		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED) :
			if (activePlayer.canRevolution(0)):
				messageControl = CyMessageControl()
				messageControl.sendUpdateCivics(self.m_paeDisplayCivics)
			screen = self.getScreen()
			screen.hideScreen()


	def Cancel(self, inputClass):
		screen = self.getScreen()
		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED) :
			for i in range (gc.getNumCivicOptionInfos()):
				self.m_paeCurrentCivics[i] = self.m_paeOriginalCivics[i]
				self.m_paeDisplayCivics[i] = self.m_paeOriginalCivics[i]

			self.drawContents()

	def getCivicsButtonName(self, iCivic):
		szName = self.BUTTON_NAME + str(iCivic)
		return szName

	def getCivicsTextName(self, iCivic):
		szName = self.TEXT_NAME + str(iCivic)
		return szName

	# Will handle the input for this screen...
	def handleInput(self, inputClass):
		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_LISTBOX_ITEM_SELECTED):
			screen = self.getScreen()
			iIndex = screen.getSelectedPullDownID(self.DEBUG_DROPDOWN_ID)
			self.setActivePlayer(screen.getPullDownData(self.DEBUG_DROPDOWN_ID, iIndex))
			self.drawContents()
			return 1
		elif (self.CivicsScreenInputMap.has_key(inputClass.getFunctionName())):
			'Calls function mapped in CvCivicsScreen'
			# only get from the map if it has the key

			# get bound function from map and call it
			self.CivicsScreenInputMap.get(inputClass.getFunctionName())(inputClass)
			return 1
		return 0

	def update(self, fDelta):
		return




