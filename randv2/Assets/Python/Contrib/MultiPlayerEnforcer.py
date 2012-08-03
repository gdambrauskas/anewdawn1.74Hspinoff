# MultiPlayerEnforcer - Resets all RevDCM and Revolution Options to ensure game is in sync

from CvPythonExtensions import *
gc = CyGlobalContext()
game = gc.getGame()
import RevDCM


def onLoadGame(argsList):
	if(game.isNetworkMultiPlayer()):
		RevDCM.resetOptions()

def onGameStart(argsList):
	if(game.isNetworkMultiPlayer()):
		RevDCM.resetOptions()