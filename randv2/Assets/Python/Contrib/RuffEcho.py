##-------------------------------------------------------------------
## Ruff Debug Echo
##-------------------------------------------------------------------

from CvPythonExtensions import *
import CvUtil

def RuffEcho(szMessage, printToScr, printToLog):
	printToScr = False
	printToLog = False

	if (printToScr):
		CyInterface().addMessage(CyGame().getActivePlayer(), True, 10, szMessage, "", 2, None, ColorTypes(8), 0, 0, False, False)
	if (printToLog):
		CvUtil.pyPrint(szMessage)
	return 0
