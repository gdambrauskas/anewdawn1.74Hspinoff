-------------------------------------
WoC INSTALLATION AND CONVERTER NOTES
-------------------------------------

The application woc_installer.jar adds extra functionality to standard WoC.
The installer does not have to be run because basic WoC functionality has
not been inhibited. To run the application, double click the jar file. 
If the application interface does not appear, a Java SE Runtime Environment(JRE)
will be required. A Java runtime can be downloaded from http://java.sun.com/.


--------
Credits:
--------

Big thanks to the WoC team for supplying the underlying structure for this 
application and their "WoC Converter" application which provides a highly
flexible, recursive, portable and object oriented xml tool.


-------
Author:
-------

Glider1, WoC Team


------------------------
Instructions for player:
------------------------

1) Add your module to the modules folder as per normal.

2) If you want "extra" module functionality to be installed (like audio),
   double click the woc_installer.jar file and close the application after 
   it says it is "done". Otherwise ignore the woc_installer.

3) The woc_installer can be run as many times as you like, but it needs to
   be executed only when you install a new module that you want extra
   functionality for.

4) To uninstall the module, delete the module folder or move the module
   to the unloaded modules folder as per normal WoC practice.
   The woc_installer is not required for un-installing modules.
   After uninstalling, the mod will contain extra xml, but this 
   does not matter. To clean the mod, just reinstall the mod.


---------------------------------
Instructions for module creators:
---------------------------------

1) If you want your module to run audio, you need to include an audio folder
   under your module's directory. It does not matter where the audio folder goes
   so long as it is installed under your module folder, but a most practical
   suggestion is /modules/Your Module/Your Module's xml folder/audio.

2) The name of the xml audio data file is flexible as per the WoC standard.
   The format of the xml audio data file is as per the WoC standard.
   You can shortcut if you want so long as your audio data is enclosed
   by it's immediate tag. It does not need to be enclosed by any deeper levels
   of tags, but it is nice to stick to WoC practice.

3) For examples of the a module, download the RevolutionDCM
   sample modules from CivFanatics.com.


----------------------------
Instructions for developers:
----------------------------

1) The source files are included in the jar. A standard java project can be
   reconstructed from these sources.

2) The standard BTS xml is included in the jar, for the sake of the future
   where the application will read the jar's contents and install standard
   xml if needed.

3) The application could easily be extended to merge any or all a modules xml
   by adding extra implementations of the CvDefines interface and including
   the implementation in the info definitions. However this functionality
   is already covered by the WoC SDK.

4) The application can be run from the command line:
   java -jar "woc_installer.jar"


---------------------------
What woc_installer DOES do:
---------------------------

1) Searches the entire modules folder for /audio directories and merges the
   xml content of those folders into the mod/assets/xml/audio contents.
   This gives WoC modules complete independence from the mod itself.


-------------------------------
What woc_installer DOES NOT do:
-------------------------------

1) If the module contains audio xml files that are not present in the mod,
   the merge will be unsuccessful.

2) The merge engine does not order the audio xml according to the MLF yet.
   If you want a specific order for merge information in your mod, install
   each woc module, one at a time.

3) If module xml is seperated out by extra tabs or spaces, it can make the
   merged mods xml look a little "ugly". This can be tidied up by hand if desired.


---------------------------
What woc_installer WILL do:
---------------------------

1) It will be made to install a default xml file if it is not present in the mod.

2) It will be made to install python functionality to a woc module.

3) It will be made to install an appropriate interface tab for the module.
   allowing the module to be configured in game.

4) It will enable a python mechanism for laying dormant to a module's
   python code and interface, if the module is removed.


--------------------------
WoC Converter notes
--------------------------

When you build an Java application project that has a main class, the IDE
automatically copies all of the JAR
files on the projects classpath to your projects dist/lib folder. The IDE
also adds each of the JAR files to the Class-Path element in the application
JAR files manifest file (MANIFEST.MF).

To run the project from the command line, go to the dist folder and
type the following:

java -jar "woc_converter.jar" 

To distribute this project, zip up the dist folder (including the lib folder)
and distribute the ZIP file.

Notes:

* If two JAR files on the project classpath have the same name, only the first
JAR file is copied to the lib folder.
* Only JAR files are copied to the lib folder.
If the classpath contains other types of files or folders, none of the
classpath elements are copied to the lib folder. In such a case,
you need to copy the classpath elements to the lib folder manually after the build.
* If a library on the projects classpath also has a Class-Path element
specified in the manifest,the content of the Class-Path element has to be on
the projects runtime path.
* To set a main class in a standard Java project, right-click the project node
in the Projects window and choose Properties. Then click Run and enter the
class name in the Main Class field. Alternatively, you can manually type the
class name in the manifest Main-Class element.

-------------
Build Issues:
-------------

1) So long as woc_installer and woc_converter is used as explained here, none have been found yet.


------------
Build Notes:
------------

v0.1
	- Basic audio merging engine in woc_installer.jar
v0.2 
	- v0.64 WocConv.jar renamed to woc_converter.jar

----Glider1 and WoC team-----