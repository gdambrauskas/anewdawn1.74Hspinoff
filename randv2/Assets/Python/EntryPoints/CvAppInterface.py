# Sid Meier's Civilization 4
# Copyright Firaxis Games 2005
#
# CvAppInterface.py
#
# These functions are App Entry Points from C++
# WARNING: These function names should not be changed
# WARNING: These functions can not be placed into a class
#
# No other modules should import this
#
# DONT ADD ANY MORE IMPORTS HERE - Moose
import sys
import os
import CvUtil
#

from CvPythonExtensions import *

# globals
gc = CyGlobalContext()

#Afforess
def AddSign(argsList):
	#import BugUtil
	import EventSigns
	#BugUtil.debug("Adding sign in CvAppInterface. Message is %s. Player is %d. Plot X is %d, Plot Y is %d", argsList[2], argsList[1], argsList[0].getX(), argsList[0].getY())
	EventSigns.addSign(argsList[0], argsList[1], argsList[2])
	#CyEngine().addSign(argsList[0], argsList[1], argsList[2])

def RemoveSign(argsList):
	import BugUtil
	import EventSigns
	BugUtil.debug("Removing sign in CvAppInterface. Player is %d. Plot X is %d, Plot Y is %d", argsList[1], argsList[0].getX(), argsList[0].getY())
	#EventSigns.hideSign(argsList[0], argsList[1])
	
	CyEngine().removeSign(argsList[0], argsList[1])
	EventSigns.gSavedSigns.removeSign(argsList[0], argsList[1])
	BugUtil.debug("Removing sign in CvAppInterface. Player is %d. Plot X is %d, Plot Y is %d", argsList[1], argsList[0].getX(), argsList[0].getY())
#Afforess End	
	
# don't make this an event - Moose
def init():
	# for PythonExtensions Help File
	PythonHelp = 0		# doesn't work on systems which haven't installed Python
			
	# dump Civ python module directory
	if PythonHelp:		
		import CvPythonExtensions
		helpFile=file("CvPythonExtensions.hlp.txt", "w")
		sys.stdout=helpFile
		import pydoc                  
		pydoc.help(CvPythonExtensions)
		helpFile.close()
	
	sys.stderr=CvUtil.RedirectError()
	sys.excepthook = CvUtil.myExceptHook
	sys.stdout=CvUtil.RedirectDebug()

def onSave():
	'Here is your chance to save data.  This function should return a string'
	import CvWBDesc
	try:
		import cPickle as pickle
	except:
		import pickle
	import CvEventInterface
	# if the tutorial is active, it will save out the Shown Messages list
	saveDataStr = pickle.dumps( CvEventInterface.onEvent( ('OnSave',0,0,0,0,0 ) ) )
	return saveDataStr
	
def onLoad(argsList):
	'Called when a file is loaded'
	try:
		import cPickle as pickle
	except:
		import pickle
	import CvEventInterface
	loadDataStr=argsList[0]	
	if len(loadDataStr):
		CvEventInterface.onEvent( ('OnLoad',pickle.loads(loadDataStr),0,0,0,0,0 ) )	

def preGameStart():
	import CvScreensInterface
	
# BUG - core - start
	import CvEventInterface
	CvEventInterface.getEventManager().fireEvent("PreGameStart")
# BUG - core - end
	
	if not CyGame().isPitbossHost():
		NiTextOut("Initializing font icons")
		# Load dynamic font icons into the icon map
		CvUtil.initDynamicFontIcons()

	if not CyGame().isPitbossHost():
		# Preload the tech chooser..., only do this release builds, in debug build we may not be raising the tech chooser
		if (not gc.isDebugBuild()):
			NiTextOut("Preloading tech chooser")
			CvScreensInterface.showTechChooser()
			CvScreensInterface.techChooser.hideScreen()
		
	NiTextOut("Loading main interface...")
	CvScreensInterface.showMainInterface()	

def onPbemSend(argsList):
	import sys, smtplib, MimeWriter, base64, StringIO
			
	szToAddr = argsList[0]	
	szFromAddr = argsList[1]	
	szSubject = argsList[2]	
	szPath = argsList[3]
	szFilename = argsList[4]
	szHost = argsList[5]
	szUser = argsList[6]
	szPassword = argsList[7]
	
	print 'sending e-mail'
	print 'To:', szToAddr
	print 'From:', szFromAddr
	print 'Subject:', szSubject
	print 'Path:', szPath
	print 'File:', szFilename
	print 'Server:', szHost
	print 'User:', szUser
	
	if len(szFromAddr) == 0 or len(szHost) == 0:
		print 'host or address empty'
		return 1

	message = StringIO.StringIO()
	writer = MimeWriter.MimeWriter(message)

	writer.addheader('To', szToAddr)
	writer.addheader('From', szFromAddr)
	writer.addheader('Subject', szSubject)
	writer.addheader('MIME-Version', '1.0')
	writer.startmultipartbody('mixed')

	part = writer.nextpart()
	body = part.startbody('text/plain')
	body.write('CIV4 PBEM save attached')

	part = writer.nextpart()
	part.addheader('Content-Transfer-Encoding', 'base64')
	szStartBody = "application/CivBeyondSwordSave; name=%s" % szFilename
	body = part.startbody(szStartBody)
	base64.encode(open(szPath+szFilename, 'rb'), body)

	# finish off
	writer.lastpart()

	# send the mail
	try:
		smtp = smtplib.SMTP(szHost)
		# trying to get TLS to work...
		#smtp.set_debuglevel(1)
		#smtp.ehlo()
		#smtp.starttls()
		#smtp.ehlo()
		if len(szUser) > 0:
			smtp.login(szUser, szPassword)
		smtp.sendmail(szFromAddr, szToAddr, message.getvalue())
		smtp.quit()
	except smtplib.SMTPAuthenticationError, e:
		CyInterface().addImmediateMessage("Authentication Error: The server didn't accept the username/password combination provided.", "")	
		CyInterface().addImmediateMessage("Error %d: %s" % (e.smtp_code, e.smtp_error), "")	
		return 1
	except smtplib.SMTPHeloError, e:
		CyInterface().addImmediateMessage("The server refused our HELO reply.", "")	
		CyInterface().addImmediateMessage("Error %d: %s" % (e.smtp_code, e.smtp_error), "")	
		return 1
	except smtplib.SMTPConnectError, e:
		CyInterface().addImmediateMessage("Error establishing connection.", "")	
		CyInterface().addImmediateMessage("Error %d: %s" % (e.smtp_code, e.smtp_error), "")	
		return 1
	except smtplib.SMTPDataError, e:
		CyInterface().addImmediateMessage("The SMTP server didn't accept the data.", "")	
		CyInterface().addImmediateMessage("Error %d: %s" % (e.smtp_code, e.smtp_error), "")	
		return 1
	except smtplib.SMTPRecipientsRefused, e:
		CyInterface().addImmediateMessage("All recipient addresses refused.", "")	
		return 1
	except smtplib.SMTPSenderRefused, e:
		CyInterface().addImmediateMessage("Sender address refused.", "")	
		CyInterface().addImmediateMessage("Error %d: %s" % (e.smtp_code, e.smtp_error), "")	
		return 1
	except smtplib.SMTPResponseException, e:
		CyInterface().addImmediateMessage("Error %d: %s" % (e.smtp_code, e.smtp_error), "")	
		return 1
	except smtplib.SMTPServerDisconnected:
		CyInterface().addImmediateMessage("Not connected to any SMTP server", "")	
		return 1
	except:
		return 1
	return 0

#####################################33
## INTERNAL USE ONLY
#####################################33
def normalizePath(argsList):
	CvUtil.pyPrint("PathName in = %s" %(argsList[0],))
	pathOut=os.path.normpath(argsList[0])
	CvUtil.pyPrint("PathName out = %s" %(pathOut,))
	return pathOut

def getConsoleMacro(argsList):
	'return a string macro that is used by the in-game python console, fxnKey goes from 1 to 10'
	fxnKey = argsList[0]
	if (fxnKey==1): return "player = gc.getPlayer(0)"
	if (fxnKey==2): return "import CvCameraControls"
	if (fxnKey==3): return "CvCameraControls.g_CameraControls.resetCameraControls()"
	if (fxnKey==4): return "CvCameraControls.g_CameraControls.doRotateCamera(360, 45.0)"
	if (fxnKey==5): return "CvCameraControls.g_CameraControls.doZoomCamera(0.2, 0.5)"
	if (fxnKey==6): return "CvCameraControls.g_CameraControls.doZoomCamera(0.5, 0.15)"
	if (fxnKey==7): return "CvCameraControls.g_CameraControls.doPitchCamera(0.5, 0.5)"
	return ""

# BUG - DLL - start
def isBug():
	return True

g_options = None
def getOption(id):
	global g_options
	if g_options is None:
		import BugOptions
		g_options = BugOptions.g_options
		if g_options is None:
			import BugUtil
			BugUtil.warn("Cannot access BUG options")
			return None
	return g_options.getOption(id)

def getOptionBOOL(argsList):
	#import BugUtil
	id, default = argsList
	#BugUtil.alert("checking option %s with default %s", id, bool(default))
	try:
		option = getOption(id)
		return bool(option.getValue())
		#val = bool(option.getValue())
		#BugUtil.alert("value = %s", val)
		#return val
	except:
		#BugUtil.alert("returning default %s", bool(default))
		return default

def getOptionINT(argsList):
	#import BugUtil
	id, default = argsList
	#BugUtil.alert("checking option %s with default %d", id, int(default))
	try:
		option = getOption(id)
		return int(option.getValue())
		#val = int(option.getValue())
		#BugUtil.alert("value = %s", val)
		#return val
	except:
		#BugUtil.alert("returning default %d", int(default))
		return default

def getOptionFLOAT(argsList):
	#import BugUtil
	id, default = argsList
	#BugUtil.alert("checking option %s with default %f", id, float(default))
	try:
		option = getOption(id)
		return float(option.getValue())
		#val = int(option.getValue())
		#BugUtil.alert("value = %s", val)
		#return val
	except:
		#BugUtil.alert("returning default %f", float(default))
		return default

def getOptionSTRING(argsList):
	#import BugUtil
	id, default = argsList
	#BugUtil.alert("checking option %s with default %s", id, str(default))
	try:
		option = getOption(id)
		return str(option.getValue())
		#val = int(option.getValue())
		#BugUtil.alert("value = %s", val)
		#return val
	except:
		#BugUtil.alert("returning default %s", str(default))
		return default

g_nameAndVersion = None
def getModNameAndVersion():
	global g_nameAndVersion
	if g_nameAndVersion is None:
		import CvModName
		g_nameAndVersion = CvModName.getNameAndVersion()
	return g_nameAndVersion
# BUG - DLL - end

# BUG - AutoSave - start
def gameStartSave():
	# called when the map is generated
	import AutoSave
	AutoSave.saveGameStart()

def gameEndSave():
	# called when the game ends
	import AutoSave
	AutoSave.saveGameEnd()

def gameExitSave():
	# called when the game ends
	import AutoSave
	AutoSave.saveGameExit()
# BUG - AutoSave - end
