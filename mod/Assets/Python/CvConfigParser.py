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

import os.path
import ConfigParser
import BugPath
#import BugConfigTracker

class CvConfigParser(ConfigParser.SafeConfigParser, object):
    
    """Extends ConfigParser.SafeConfigParser adding two important features.
    
    First, all of the get functions take an additional argument that serves
    as a default value.  If a default value is given but the option is not
    found in the configuration file, the default value is returned instead
    of throwing an exception.  If no default is given, or if the default is
    None, then an exception is thrown as in the super class function.
    
    Second, the constructor accepts a filename argument.  If this argument
    is given, the Civilization 4 directories are searched for files
    with that name, and options are automatically read from a file if one
    is found.  If multiple files are found with conflicting options,
    files found earlier on the search path override options in files found 
    later.
    
    The search path is made up of the parent directory of all Assets 
    directories on the game's load path.  For example, if the assets path
    contains <userDir>\CustomAssets and <installDir>\Assets, the parser will
    look for .ini files in <userDir> and <installDir>.
    
    The example below constructs a parser that searches for "Foo.ini" files.
    The variable n is then initialized to the value found in the .ini files
    or a default of 5.
    
        foo = CvConfigParser.CvConfigParser("Foo.ini")
        i = foo.getint("Foo", "i", 5)
    
    """

    def __init__(self, filename = None, *args, **kwargs):
        """Initializes the parser by reading options from the named file."""
        super(CvConfigParser, self).__init__(*args, **kwargs)
# Rise of Mankind 2.8 - init paths before searching config files		
        BugPath.init()
#        BugPath.initSearchPaths()
        if (filename != None):
            filenames = [os.path.join(os.path.dirname(dir), filename) 
                for dir in BugPath._assetFileSearchPaths]
#			filenames += [os.path.join(os.path.dirname(dir), filename) 
#				for dir in BugPath.iniFileSearchPaths]
# Rise of Mankind 2.6 end		
			
			# print "filenames\n", filenames
            self.read(filenames)
			

    def get(self, section, option, default = None, *args, **kwargs):
        """Looks up the specified section/option pair.
        
        This extends the base functionality of the Python ConfigParser 
        class by adding support for a default value in the event that the 
        option is not given in the configuration file.

        """
        return self._wrappedGet(super(CvConfigParser, self).get, 
                                section, option, default, *args, **kwargs)

    def getint(self, section, option, default = None, *args, **kwargs):
        """Like get(), but converts the value to an integer."""
        return self._wrappedGet(super(CvConfigParser, self).getint, 
                                section, option, default, *args, **kwargs)
        
    def getfloat(self, section, option, default = None, *args, **kwargs):
        """Like get(), but converts the value to a float."""
        return self._wrappedGet(super(CvConfigParser, self).getfloat, 
                                section, option, default, *args, **kwargs)

    def getboolean(self, section, option, default = None, *args, **kwargs):
        """Like get(), but converts the value to a boolean."""
        return self._wrappedGet(super(CvConfigParser, self).getboolean, 
                                section, option, default, *args, **kwargs)

    def _wrappedGet(self, getter, section, option, default, *args, **kwargs):
        """Wraps the specified getter function with an exception handler
        and returns a default value if NoSectionError or NoOptionError
        is raised.

        """
        try:
            return getter(section, option, *args, **kwargs)
        except (ConfigParser.NoSectionError, ConfigParser.NoOptionError):
            if (default != None):
                return default
            else:
                raise
