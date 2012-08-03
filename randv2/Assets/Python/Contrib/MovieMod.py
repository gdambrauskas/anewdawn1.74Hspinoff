# MovieMod -- Play religion and Nat Wonder movies - 

from CvPythonExtensions import *

gc = CyGlobalContext()

def onBuildingBuilt(argsList): # Took out arg 'self' - not required

	'Building Completed'
	pCity, iBuildingType = argsList
	game = gc.getGame()

	if ( (not game.isNetworkMultiPlayer()) and (pCity.getOwner() == game.getActivePlayer()) ):
		if ( gc.getBuildingInfo(iBuildingType).getMovie() and not isWorldWonderClass(gc.getBuildingInfo(iBuildingType).getBuildingClassType()) ):
			if (gc.getTeam(pCity.getTeam()).getBuildingClassCount(gc.getBuildingInfo(iBuildingType).getBuildingClassType()) <= 1):

				popupInfo = CyPopupInfo()
				popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON_SCREEN)
				popupInfo.setData1(iBuildingType)
				popupInfo.setData2(pCity.getID())
				popupInfo.setData3(0)
				popupInfo.setText(u"showWonderMovie")
				popupInfo.addPopup(pCity.getOwner())