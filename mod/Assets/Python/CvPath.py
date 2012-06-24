## Copyright (c) 2006, Gillmer J. Derge.

## This file is part of Civilization IV Alerts mod.
##
## Civilization IV Alerts mod is free software; you can redistribute
## it and/or modify it under the terms of the GNU General Public
## License as published by the Free Software Foundation; either
## version 2 of the License, or (at your option) any later version.
##
## Civilization IV Alerts mod is distributed in the hope that it will
## be useful, but WITHOUT ANY WARRANTY; without even the implied
## warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Civilization IV Alerts mod; if not, write to the Free
## Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
## 02110-1301 USA

__version__ = "$Revision$"
# $Source$

"""Implements a collection of utility methods and variables for determining
the location of Civilization 4 components.

The following variables are exposed.

* activeModName: the name of the currently active mod or None if no mod has
  been loaded.
  
  NOTE: activeModName does not currently work in a completely automated 
  fashion.  There does not appear to be a way to determine the active mod 
  programmatically from Python code.  A mod that wishes to export its name 
  to this module must create a Python module called CvModName that contains 
  a string variable named modName set to the name of the mod.  A sample 
  CvModName is shown below.

  # CvModName.py
  modName = "American Revolution"

  Of course, a CvModName Python module should only be used if the mod is 
  indeed installed in the Mods directory, not when it is installed in 
  CustomAssets.  Furthermore, if the value of the modName variable does not
  correctly match the mod directory name, the path variables will not be
  set properly.

* userDir: the user's Civilization 4 directory, typically
  C:\Documents and Settings\User\My Documents\My Games\Sid Meier's Civilization 4
  
* userAssetsDir: <userDir>\CustomAssets

* userModsDir: <userDir>\Mods

* userActiveModDir: <userDir>\Mods\<activeModName>

* userActiveModAssetsDir: <userDir>\Mods\<activeModName>\Assets

* installDir: the Civilization 4 installation directory, typically
  C:\Program Files\Firaxis Games\Sid Meier's Civilization 4

* installAssetsDir: <installDir>\Assets

* installModsDir: <installDir>\Mods

* installActiveModDir: <installDir>\Mods\<activeModName>

* installActiveModAssetsDir: <installDir>\Mods\<activeModName>\Assets

* assetsPath: a list containing all Assets directories that appear on the
  game's load paths.  Typically [userAssetsDir, installAssetsDir] or
  [userActiveModAssetsDir, installActiveModAssetsDir, userAssetsDir, installAssetsDir]

* pythonPath: a list containing all directories that appear on the
  game's Python load path.  The game's Python module loader does not support
  Python packages, so this list includes not only the Python subdirectory
  of each element of the assetsPath but also all non-empty subdirectories.

"""


import os
import os.path
import sys

if (sys.platform == 'darwin'):
	""" Mac OS X """
	def _getUserDir():
		return os.path.join(os.environ['HOME'], "Documents", "Civilization IV")

	def _getInstallDir():
		import commands
		civ4name = "Civilization IV.app/Contents/MacOS/Civilization IV"
		str = commands.getoutput("ps -xo 'command' | grep " + "'" + civ4name + "'")
		m = str.find(civ4name)
		if (m >= 0):
			installDir = str[0:m]
		return installDir
else:
	import _winreg
	""" Windows """
	def __getRegValue(root, subkey, name):
		key = _winreg.OpenKey(root, subkey)
		try:
			value = _winreg.QueryValueEx(key, name)
			return value[0]
		finally:
			key.Close()
	
	def _getUserDir():
		myDocuments = __getRegValue(_winreg.HKEY_CURRENT_USER, 
				r"Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders",
				"Personal")
		civ4Dir = os.path.basename(_getInstallDir())
		return os.path.join(myDocuments, "My Games", civ4Dir)
	
	def _getInstallDir():
		#subkey = r"Software\Firaxis Games\Sid Meier's Civilization 4 - Beyond the Sword"
		#dir = __getRegValue(_winreg.HKEY_LOCAL_MACHINE, subkey, "INSTALLDIR")
		#if (not dir):
		#	subkey = r"Software\Firaxis Games\Sid Meier's Civilization 4 Complete"
		#	dir = __getRegValue(_winreg.HKEY_LOCAL_MACHINE, subkey, "INSTALLDIR")
		#return dir
		installDir = os.getcwd()
		print "os.getcwd():  ", installDir
		return installDir


activeModName = None
try:
    import CvModName
    activeModName = CvModName.modName
except:
    pass

userDir = _getUserDir()

userAssetsDir = os.path.join(userDir, "CustomAssets")

userModsDir = os.path.join(userDir, "Mods")

userActiveModDir = None

userActiveModAssetsDir = None

installDir = _getInstallDir()

installAssetsDir = os.path.join(installDir, "Assets")

installModsDir = os.path.join(installDir, "Mods")

installActiveModDir = None

installActiveModAssetsDir = None

assetsPath = [userAssetsDir, installAssetsDir]

if (activeModName != None):
    userActiveModDir = os.path.join(userModsDir, activeModName)
    userActiveModAssetsDir = os.path.join(userActiveModDir, "Assets")
    installActiveModDir = os.path.join(installModsDir, activeModName)
    installActiveModAssetsDir = os.path.join(installActiveModDir, "Assets")
    assetsPath.insert(0, userActiveModAssetsDir)
# YAUGM changes begin
#   assetsPath.insert(1, installActiveModAssetsDir)
    assetsPath.insert(2, installActiveModAssetsDir) # now goes after CustomAssets!
# YAUGM changes end

pythonPath = []
for dir in [os.path.join(d, "Python") for d in assetsPath]:
    for root, subdirs, files in os.walk(dir):
        if (len(files) > 0):
            pythonPath.append(root)

# YAUGM changes begin

# Designed for GreatPersonMod, returns absolute path for
# a relative path to Assets dir if and only if fileName is
# found in the location specified.
# see _test() for example.
def getPath(relPath, fileName):
    for filePath in [os.path.join(d, relPath) for d in assetsPath]:
        if os.path.isfile(os.path.join(filePath, fileName)):
            return filePath
    return ""

# Designed for Ruff's Cobbled SG Modpack 1.2b, returns list
# of potential locations for the ini file. This is called by
# CvConfigParser.py. File locations are listed in reverse
# assetsPath order so that userDir\Mods overrides CustomAssets
# overrides installDir\MODS.
# Use like this: files = CvPath.getINIPathForCvConfigParser("RuffMod.ini")
# Note: CvConfigParser.py is no longer used in Ruff's Cobbled SG Modpack 2.0+,
# but this is still useful for many of TheLopez's mod components.
def getINIPathForCvConfigParser(fileName):
    if (fileName != None):
        filenames = [os.path.join(os.path.dirname(dir), fileName)
                     for dir in assetsPath]
        filenames.reverse()
        return filenames
    return None

# Designed for Ruff's Cobbled SG Modpack 2.0.x(w), return the path
# to an ini file which should be located just outside one of the
# directories in the assets path. This is called from RuffModControl.py
def get_INI_File(szINIFileName):
    filepaths = [os.path.join(os.path.dirname(dir), szINIFileName) 
        for dir in assetsPath]
    for filepath in filepaths:
        if os.path.isfile(filepath):
            return filepath
    return ""

# YAUGM changes end

def _test():
    print "activeModName = " + str(activeModName)
    print "userDir = " + userDir
    print "userAssetsDir = " + userAssetsDir
    print "userModsDir = " + userModsDir
    print "userActiveModDir = " + str(userActiveModDir)
    print "userActiveModAssetsDir = " + str(userActiveModAssetsDir)
    print "installDir = " + installDir
    print "installAssetsDir = " + installAssetsDir
    print "installModsDir = " + installModsDir
    print "installActiveModDir = " + str(installActiveModDir)
    print "installActiveModAssetsDir = " + str(installActiveModAssetsDir)
    print "assetsPath = " 
    for dir in assetsPath:
        print "  " + dir
    print "pythonPath = "
    for dir in pythonPath:
        print "  " + dir

# YAUGM changes begin
    print "greatPeopleArtPath = " + getPath("art\\GreatPeople", "Great Person.dds")
    print "RuffMod.ini location: " + get_INI_File("RuffMod_2w.ini")
    configFilePath = getINIPathForCvConfigParser("")
    configFilePath.reverse()
    print "configFilePath = "
    for filePath in configFilePath:
    	print "  " + filePath
# YAUGM changes end

if __name__ == "__main__": 
    _test()
