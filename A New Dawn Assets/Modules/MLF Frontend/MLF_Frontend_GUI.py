#Copyright (C) 2010  Devon McAvoy


#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.

from tkinter import *
from tkinter.constants import *
from tkinter.filedialog import *
from time import *
import os
version = '2.b'
configuration='''<?xml version="1.0"?>
<!-- Sid Meier's Civilization 4 Beyond the Sword -->
<!-- Modified by the World of Civilization Team -->
<!-- Master Load File -->
<!-- -->
<Civ4ModularLoadControls xmlns="x-schema:CIV4ModularLoadingControlsSchema.xml">
	<DefaultConfiguration>'''
typee='''</DefaultConfiguration>
	<ConfigurationInfos>
		<ConfigurationInfo>
			<Type>'''
descript='''</Type>
			<Description>Initialize ALL Modules by using the MLF file structure</Description>
			<Modules>
'''
end='''			</Modules>
		</ConfigurationInfo>
	</ConfigurationInfos>
</Civ4ModularLoadControls>'''
startmod='''				<Module>
					<Directory>'''
startload='''</Directory>
					<bLoad>'''
endload='''</bLoad>
				</Module>
'''

class StatusBar(Frame):
    '''Class that creates a status bar for windows'''
    def __init__(self, master):
        Frame.__init__(self, master)
        self.label = Label(self, bd=1, relief=SUNKEN, anchor=W)
        self.label.pack(fill=X)

    def set(self, format, *args):
        self.label.config(text=format % args)
        self.label.update_idletasks()

    def clear(self):
        self.label.config(text="")
        self.label.update_idletasks()


class MLF:
    def __init__(self, master):
        '''Creates main window'''
        def goadvanced():
            '''Switches between advanced and simple veiws on the main window'''
            nonlocal master, btsmenu, helpmenu, filemenu, aboutframe, modifyframe, licenseframe, loadrframe, adddframe, addrframe, addaframe, bar, modders, bar1
            if self.advanced == 0:
                self.advanced = 1
                master.geometry("%dx%d+%d+%d" %(500, 540, 100, 50))
                mlfmenu = Menu(menu)
                menu.insert_cascade(2, label='MLF', menu=mlfmenu)
                filemenu.insert_command(3, label='Load Restrict File', command=self.loadmlf)
                mlfmenu.add_command(label='Add Dir to MLF', command=self.adddirectory)
                mlfmenu.add_command(label='Add Restricted Dir', command=self.addrestricted)
                mlfmenu.add_command(label='Add Alias', command=self.addalias)
                mlfmenu.add_command(label='Add Image', command=self.addimagedir)
                mlfmenu.add_command(label='Add Description', command=self.adddescription)
                filemenu.add_separator()
                filemenu.entryconfig(5,label='Go Simple')
                aboutframe.pack_forget()
                modifyframe.pack_forget()
                licenseframe.pack_forget()  
                bar.pack()
                modders.pack()
                loadrframe.pack(fill=X)
                adddframe.pack(fill=X)
                addrframe.pack(fill=X)
                addaframe.pack(fill=X)
                addiframe.pack(fill=X)
                adddrframe.pack(fill=X)
                bar1.pack()
                aboutframe.pack(fill=X)
                modifyframe.pack(fill=X)
                licenseframe.pack(fill=X)
            else:
                self.advanced = 0
                master.geometry("%dx%d+%d+%d" %(500, 285, 100, 50))
                filemenu.delete(3)
                filemenu.entryconfig(4, label='Go Advanced')
                menu.delete(2)
                bar.pack_forget()
                bar1.pack_forget()
                modders.pack_forget()
                loadrframe.pack_forget()
                adddframe.pack_forget()
                addrframe.pack_forget()
                addaframe.pack_forget()
                addiframe.pack_forget()
                adddrframe.pack_forget()
        master.wm_iconbitmap('Civ4BtS.ico')
        self.abspath = os.path.normpath(os.path.abspath(r'..'))
        self.next = 0
        self.exit = 0
        self.norestrfile = 1
        self.first = 0
        self.fail = 1
        self.nofile = 1
        self.advanced = 0
        self.noimage = 1
        first = 1
        menu = Menu(master)
        master.config(menu=menu)
        self.status = StatusBar(master)
        self.status.pack(side=BOTTOM, fill=X)
        self.status.set('')
        loadrframe = Frame(master)
        adddframe = Frame(master)
        addrframe = Frame(master)
        addaframe = Frame(master)
        addiframe = Frame(master)
        adddrframe = Frame(master)
        adddrlabel = Label(adddrframe, text='Add Description', anchor=W, justify=LEFT)
        adddr = Button(adddrframe, text='   Add Description   ', command=self.adddescription)
        addilabel = Label(addiframe, text='Add Image', anchor=W, justify=LEFT)
        addi = Button(addiframe, text='       Add Image       ', command=self.addimagedir)
        bar = Label(master, text='============================================================', anchor=W, justify=LEFT)
        modders = Label(master, text='For Modders Only:', anchor=W, justify=LEFT)
        addr = Button(addrframe, text='    Add Restricted    ', command=self.addrestricted, anchor=W)
        addrlabel = Label(addrframe, text='Add to the Restrictions File', anchor=W, justify=LEFT)
        addd = Button(adddframe, text='     Add Directory    ', command=self.adddirectory, anchor=W)
        adddlabel = Label(adddframe, text='Add to the MLF File', anchor=W, justify=LEFT)
        self.loadr = Button(loadrframe, text='  Load Restrict File  ', command=self.loadrestrict, anchor=W, activebackground='black')
        loadrlabel = Label(loadrframe, text='Load the Restrictons File', anchor=W, justify=LEFT)
        adda = Button(addaframe, text='        Add Alias         ', command=self.addalias, anchor=W)
        addalabel = Label(addaframe, text='Add an Alias to the Restrictions File', anchor=W, justify=LEFT)
        bar1 = Label(master, text='============================================================', anchor=W, justify=LEFT)
        quit = Button(master, text=' Quit ', command=master.destroy)
        quit.pack(side=BOTTOM)
        self.loadr.pack(side=LEFT, padx=10, pady=5)
        loadrlabel.pack(side=LEFT)
        addd.pack(side=LEFT, padx=10, pady=5)
        adddlabel.pack(side=LEFT)
        addr.pack(side=LEFT, padx=10, pady=5)
        addrlabel.pack(side=LEFT)
        adda.pack(side=LEFT, padx=10, pady=5)
        addalabel.pack(side=LEFT)
        addi.pack(side=LEFT, padx=10, pady=5)
        addilabel.pack(side=LEFT)
        adddr.pack(side=LEFT, padx=10, pady=5)
        adddrlabel.pack(side=LEFT)
        startframe = Frame(master)
        startframe.pack(fill=X)
        start = Button(startframe, text='  Launch BTS ROM ', command=self.execbts, anchor=W)
        start.pack(side=LEFT, padx=10, pady=5)
        startlabel = Label(startframe, text='Execute BTS ROM', anchor=W, justify=LEFT)
        startlabel.pack(side=LEFT)
        loadframe = Frame(master)
        loadframe.pack(fill=X)
        self.load = Button(loadframe, text='        Load MLF        ', command=self.loadmlf, anchor=W, activebackground='black')
        self.load.pack(side=LEFT, padx=10, pady=5)
        loadlabel = Label(loadframe, text='Load the Civ4 Modular Loading Control File', anchor=W, justify=LEFT)
        loadlabel.pack(side=LEFT)
        loadnframe = Frame(master)
        loadnframe.pack(fill=X)
        loadn = Button(loadnframe, text='    Load Next MLF   ', command=self.loadnextmlf, anchor=W)
        loadn.pack(side=LEFT, padx=10, pady=5)
        loadnlabel = Label(loadnframe, text='Load the MLFs in subdirectories', anchor=W, justify=LEFT)
        loadnlabel.pack(side=LEFT)
        filemenu = Menu(menu)
        menu.add_cascade(label='File', menu=filemenu)
        filemenu.add_command(label='Load MLF', command=self.loadmlf)
        filemenu.add_command(label='Load next MLF', command=self.loadnextmlf)
        if self.advanced == 0:
            master.geometry("%dx%d+%d+%d" %(500, 285, 100, 50))
            btsmenu = Menu(menu)
            menu.add_cascade(label='BTS', menu=btsmenu)
            helpmenu = Menu(menu)
            menu.add_cascade(label='Help', menu=helpmenu)
            filemenu.add_separator()
            filemenu.add_command(label='Go Advanced', command=goadvanced)
            filemenu.add_command(label='Exit', command=master.destroy)
            helpmenu.add_command(label='About', command=self.about)
            helpmenu.add_command(label='Modify Source', command=self.modify)
            helpmenu.add_command(label='License', command=self.gpllicense)
            btsmenu.add_command(label='Launch BTS ROM', command=self.execbts)
        aboutframe = Frame(master)
        aboutframe.pack(fill=X)
        about = Button(aboutframe, text='           About            ', command=self.about, anchor=W)
        about.pack(side=LEFT, padx=10, pady=5)
        aboutlabel = Label(aboutframe, text='About MLF Frontend', anchor=W, justify=LEFT)
        aboutlabel.pack(side=LEFT)
        modifyframe = Frame(master)
        modifyframe.pack(fill=X)
        modify = Button(modifyframe, text='    Modify Source     ', command=self.modify, anchor=W)
        modify.pack(side=LEFT, padx=10, pady=5)
        modifylabel = Label(modifyframe, text='How to Modify the Source code', anchor=W, justify=LEFT)
        modifylabel.pack(side=LEFT)
        licenseframe = Frame(master)
        licenseframe.pack(fill=X)
        license = Button(licenseframe, text='           License           ', command=self.gpllicense, anchor=W)
        license.pack(side=LEFT, padx=10, pady=5)
        licenselabel = Label(licenseframe, text='The GNU General Public License', anchor=W, justify=LEFT)
        licenselabel.pack(side=LEFT)
        copyright = Toplevel()
        copyright.title('Warning!')
        copyright.wm_iconbitmap('Civ4BtS.ico')
        copyright.geometry('%dx%d+%d+%d' %(500, 100, 100, 50))
        txt=str('''MLF Frontend Command Line  Copyright (C) 2010  Devon McAvoy
This program comes with ABSOLUTELY NO WARRANTY; for details veiw the
options under the Help menu.This is free software, and you are welcome
to redistribute it under certain conditions; Also see the options under Help''')
        warning = Label(copyright, text=txt, anchor=W, justify=LEFT)
        warning.pack()
        qbutton = Button(copyright, text=' OK ', anchor=W, command=copyright.destroy)
        qbutton.pack(side=BOTTOM)
        copyright.wait_window()

    def loadrestrict(self):
        '''Calls the restrict function'''
        self.restrict()

    def addalias(self):
        '''Calls the Add Alias function if the load MLF function
or Load Restrict funtion has been called befor and the Restrict file exists'''
        if self.norestrfile != 1:
            self.addali()
        else:
            self.status.set('Load the Restrict File first!')
            self.load.flash()
            self.loadr.flash()
            sleep(1)
            self.load.flash()
            self.loadr.flash()
            self.status.clear()

    def loadnextmlf(self):
        '''Calls the Load Next MLF function if the Load MLF function has been
called before this and the MLF file exists'''
        if self.fail != 1:
            self.loadnext()
        else:
            self.status.set('Load the MLF File first!')
            self.load.flash()
            sleep(1)
            self.load.flash()
            self.status.clear()

    def addrestricted(self):
        '''Calls the Add Restrict function if the load MLF function
or Load Restrict funtion has been called befor and the Restrict file exists'''
        if self.norestrfile != 1:
            self.addrestrict()
        else:
            self.status.set('Load the Restrict File first!')
            self.load.flash()
            self.loadr.flash()
            sleep(1)
            self.load.flash()
            self.loadr.flash()
            self.status.clear()

    def adddirectory(self):
        '''Calls the Add Directory funtion if the Load MLF function has been
called before this and the MLF file exists'''
        if self.fail != 1:
            self.readfile()
            self.sort()
            self.adddir()
        else:
            self.status.set('Load the MLF File first!')
            self.load.flash()
            sleep(1)
            self.load.flash()
            self.status.clear()

    def addimagedir(self):
        '''Calls the Add Image function if the load MLF function
or Load Restrict funtion has been called befor and the Restrict file exists'''
        if self.norestrfile != 1:
            self.addimage()
        else:
            self.status.set('Load the Restrict File first!')
            self.load.flash()
            self.loadr.flash()
            sleep(1)
            self.load.flash()
            self.loadr.flash()
            self.status.clear()
            
    def adddescription(self):
        '''Calls the Add Description function if the load MLF function
or Load Restrict funtion has been called befor and the Restrict file exists'''
        if self.norestrfile != 1:
            self.adddis()
        else:
            self.status.set('Load the Restrict File first!')
            self.load.flash()
            self.loadr.flash()
            sleep(1)
            self.load.flash()
            self.loadr.flash()
            self.status.clear()
            
    def loadmlf(self):
        '''Calls all the function to read, edit, and load the MLF File'''
        self.loadfile()
        self.restrict()
        if self.fail != 1:
            self.readfile()
            self.sort()
            self.loadlist()
            if self.t != 1:
                self.writefile()

    def about(self):
        '''Window that displays a little about the program and its maker'''
        self.status.set('Opening About Window')
        sleep(1)
        self.status.clear()
        about = Toplevel()
        about.wm_iconbitmap('Civ4BtS.ico')
        about.title('About MLF Frontend %s' % version)
        about.geometry("%dx%d+%d+%d" %(500, 280, 100, 50))
        message = Label(about, text='''Developer: Devon McAvoy
Contact: gen.mcavoyii@gmail.com
About: MLF Frontend created to help the user edit and add
    the load order for ROM Modmods.

Warning: Adding a new directory to the MLF file adds it
    to the end of the file. If you need to load directories
    in a specific order I recommend that you edit the MLF file
    manually.

Special Thanks to:
    Afforess
    The Developers of Python 3.1
    The helpful members on the Daniweb Python Forum''', anchor=W, justify=LEFT)
        message.pack(fill=X)
        copyright = Label(about, text='Copyright (C) 2010 Devon McAvoy', anchor=S, justify=CENTER)
        copyright.pack(side=BOTTOM)
        quit = Button(about, text=' Close Window ', command=about.destroy)
        quit.pack(side=BOTTOM)

    def modify(self):
        '''Window that displays a little of what you will need to modify the program'''
        self.status.set('Opening Modify Source Window')
        sleep(1)
        self.status.clear()
        modify = Toplevel()
        modify.wm_iconbitmap('Civ4BtS.ico')
        modify.title('How to modify MLF Frontend %s source' % version)
        modify.geometry("%dx%d+%d+%d" %(500, 360, 100, 50))
        message = Label(modify, text='''
To modify this program you must have a copy of Python 3.1, which
    can be found at http://www.python.org and to recompile
    it you will need cxFreeze, which may be assertained at
    http://cx-freeze.sourceforge.net/cx_Freeze.html, and the
    setup.py file that is included as a install feature.
You may freely modify this program as long as you meet the criteria
    laid down by the GNU General Public License and you comment out
    any code you do not use, in any derivative work as to allow
    others to better see the changes you have enacted upon this
    program.
The installer used to install this program was created with
    installjammer which can be found at
    http://www.installjammer.com
    
I here by grant you any and all rights as specified by the GNU
    General Public License to copy, modify, or redistribute this
    work.
    
Thanks for using my program...''', anchor=W, justify=LEFT)
        message.pack(fill=X)
        copyright = Label(modify, text='Copyright (C) 2010 Devon McAvoy', anchor=S, justify=CENTER)
        copyright.pack(side=BOTTOM)
        quit = Button(modify, text=' Close Window ', command=modify.destroy)
        quit.pack(side=BOTTOM)

    def gpllicense(self):
        '''Window that displays the GNU General Public License'''
        self.status.set('Opening License Window')
        sleep(1)
        self.status.clear()
        licensew = Toplevel()
        licensew.wm_iconbitmap('Civ4BtS.ico')
        licensew.title('License: GNU General Public License')
        licensew.geometry("%dx%d+%d+%d" %(650, 600, 100, 50))
        file = 'License.txt'
        licensefile = open(file, 'r')
        quit = Button(licensew, text=' Close Window ', command=licensew.destroy)
        quit.pack(side=BOTTOM)
        licensef = licensefile.readlines()
        scrollbary = Scrollbar(licensew)
        scrollbary.pack(side=RIGHT, fill=Y)
        message = Text(licensew, yscrollcommand=scrollbary.set, width=650, height=500)
        for l in licensef:
            message.insert(END, l)
        message.pack(side=TOP, fill=BOTH)
        message.config(state=DISABLED)
        scrollbary.config(command=message.yview)
        

    def execbts(self):
        '''Calls the functions required to start Civ4 BTS ROM'''
        self.search()
        if self.nofile != 1:
            self.execute()

    def search(self):
        '''Function that searchs for the Civ4 BTS .exe'''
        self.filepath = ''
        fname = "Civ4BeyondSword.exe"
        self.status.set('Searching for %s...' % fname)
        def limited_walk(folder, limit, n = 0):
            """generator similar to os.walk(), but with limited subdirectory depth"""
            if n > limit:
                return
            for file in os.listdir(folder):
                file_path = os.path.join(folder, file)
                if os.path.isdir(file_path):
                    for item in limited_walk(file_path, limit, n + 1):
                        yield item
                else:
                    yield file_path
        n = 0
        while n < 5:
            if n == 0:
                folder = r'..\..\..\..'
            if n == 1:
                folder = r"C:\Program Files (x86)"
            if n == 2:
                folder = r"C:\Program Files"
            if n == 3:
                folder = r"C:\Valve"
            if n == 4:
                folder = r"C:\Steam"
            try:
                for filename in limited_walk(folder, 5):
                    if os.path.basename(filename).lower() == fname.lower():
                        result = filename
                        self.filepath = filename
                        n = 10
                        break
                    else:
                        result = None
            except WindowsError:
                result = None
        if n > 5:
            result = None
        if result:
            self.status.set('%s Located' % fname)
            sleep(1)
            self.status.clear()
            self.nofile = 0
        else:
            self.status.set('Could not locate %s' % fname)
            self.filepath = askopenfilename(title='Search for Civ4BeyondSword.exe', filetypes=[('exe', '*.exe')])
            #print(self.filepath)
            if self.filepath == '':
                self.nofile = 1
                self.filepath = ''
            else:
                self.status.clear()
                self.nofile = 2
            

    def execute(self):
        '''Function that starts Civ4 BTS ROM'''
        self.status.set('Starting BTS ROM')
        path = self.filepath
        path = '"'+path+'"'
        #print(path)
        input1 = 'mod="mods\Rise of Mankind"'
        #pyexe = sys.executable
        os.system('start "BTS" %s %s' %(path,input1))
        #subprocess.call([path, input1])
        sleep(1)
        self.status.clear()
        
    def loadfile(self):
        '''Function that loads the MLF originally'''
        self.next = 0
        self.first = 1
        self.filedir = self.abspath+'\MLF_CIV4ModularLoadingControls.xml'
        try:
            f = open(self.filedir, 'r')
            self.fail = 0
            self.status.set('Loading MLF File')
            sleep(1)
            self.status.clear()
        except IOError:
            warning=Toplevel()
            warning.title('ERROR: NO MLF FILE')
            warning.geometry("%dx%d+%d+%d" %(250, 75, 100, 50))
            error = Label(warning, text='''This application is not in the correct directory.
Please move it, then run again.''')
            error.pack(side=TOP)
            quit = Button(warning, text=' Close Window ', command=warning.destroy)
            quit.pack(side=BOTTOM)
            self.fail = 1
        #print(self.filedir, ' Opened!')

    def readfile(self):
        '''Reads the MLF File'''
        self.line = ''
        input_file = open(self.filedir, 'r')
        self.line = input_file.readlines()
        input_file.close()
        self.config = []
        self.directory = []
        self.load = []
        #print('Read', self.filedir)

    def restrict(self):
        '''Opens, Reads, Sorts the Restrict Text file'''
        restrfile = 'restricted.txt'
        try:
            f = open(restrfile, 'r')
            self.norestrfile = 0
            self.status.set('Loading Restrict File')
            sleep(1)
            self.status.clear()
        except IOError:
            warning=Toplevel()
            warning.title('ERROR: NO RESTRICT FILE')
            warning.geometry("%dx%d+%d+%d" %(350, 75, 100, 50))
            error = Label(warning, text='''This option is disabled due to the lack of the resricted.txt.
Please create a new one or move the file into the proper directory.''')
            error.pack(side=TOP)
            quit = Button(warning, text=' Close Window ', command=warning.destroy)
            quit.pack(side=BOTTOM)
            self.norestrfile = 1
        if self.norestrfile != 1:
            start = 0
            lines = ''
            restfile = []
            self.restricted = []
            self.modules = []
            self.alias = []
            self.imagedir = []
            self.imagepath = []
            self.dismod = []
            self.discript = []
            input_file = open(restrfile, 'r')
            lines = input_file.readlines()
            input_file.close()
            for l in lines:
                restfile.append(l.strip('\t\n'))
            for l in restfile:
                if l.startswith('!'):
                    self.restricted.append(l)
                if l.startswith('#'):
                    self.modules.append(l)
                if l.startswith('@'):
                    self.alias.append(l)
                if l.startswith('%'):
                    self.imagedir.append(l)
                if l.startswith('*'):
                    self.imagepath.append(l)
                if l.startswith('^'):
                    self.dismod.append(l)
                if l.startswith('&'):
                    self.discript.append(l)
            n = 0
            for l in self.restricted:
                f = self.restricted[n]
                if f.startswith('!'):
                    f = f.split('!')
                    x = 0
                    f = f[1]
                if x == 0:
                    f = f.split('$')
                f = f[0]
                self.restricted[n] = f
                n += 1
            n = 0
            for l in self.modules:
                f = self.modules[n]
                if f.startswith('#'):
                    f = f.split('#')
                    x = 0
                    f = f[1]
                if x == 0:
                    f = f.split('$')
                f = f[0]
                self.modules[n] = f
                n += 1
            n = 0
            for l in self.alias:
                f = self.alias[n]
                if f.startswith('@'):
                    f = f.split('@')
                    x = 0
                    f = f[1]
                if x == 0:
                    f = f.split('$')
                f = f[0]
                self.alias[n] = f
                n += 1
            n = 0
            for l in self.imagedir:
                f = self.imagedir[n]
                if f.startswith('%'):
                    f = f.split('%')
                    x = 0
                    f = f[1]
                if x == 0:
                    f = f.split('$')
                f = f[0]
                self.imagedir[n] = f
                n += 1
            n = 0
            for l in self.imagepath:
                f = self.imagepath[n]
                if f.startswith('*'):
                    f = f.split('*')
                    x = 0
                    f = f[1]
                if x == 0:
                    f = f.split('$')
                f = f[0]
                self.imagepath[n] = f
                n += 1
            if self.imagepath == []:
                self.noimage = 1
            else:
                self.noimage = 0
            n = 0
            for l in self.dismod:
                f = self.dismod[n]
                if f.startswith('^'):
                    f = f.split('^')
                    x = 0
                    f = f[1]
                if x == 0:
                    f = f.split('$')
                f = f[0]
                self.dismod[n] = f
                n += 1
            n = 0
            for l in self.discript:
                f = self.discript[n]
                if f.startswith('&'):
                    f = f.split('&')
                    x = 0
                    f = f[1]
                if x == 0:
                    f = f.split('$')
                f = f[0]
                self.discript[n] = f
                n += 1
            #print(self.imagedir)
            #print(self.imagepath)
            
                

    def sort(self):
        '''Sorts MLF File'''
        file = []
        for l in self.line:
            file.append(l.strip('\t\n'))
        for l in file:
            if l.startswith('<DefaultConfiguration>'):
                self.config.append(l)
            if l.startswith('<Type>'):
                self.config.append(l)
            if l.startswith('<Directory>'):
                self.directory.append(l)
            if l.startswith('<bLoad>'):
                self.load.append(l)
        n = 0
        for l in self.config:
            f = self.config[n]
            if f.startswith('<DefaultConfiguration>'):
                f = f.split('<DefaultConfiguration>')
                x = 0
                f = f[1]
            if f.startswith('<Type>'):
                f = f.split('<Type>')
                x = 1
                f = f[1]
            if x == 0:
                f = f.split('</DefaultConfiguration>')
            if x == 1:
                f = f.split('</Type>')
            f = f[0]
            self.config[n] = f
            n += 1
        n = 0
        for l in self.directory:
            f = self.directory[n]
            if f.startswith('<Directory>'):
                f = f.split('<Directory>')
                x = 0
                f = f[1]
            if x == 0:
                f = f.split('</Directory>')
            f = f[0]
            self.directory[n] = f
            n += 1
        n = 0
        for l in self.load:
            f = self.load[n]
            if f.startswith('<bLoad>'):
                f = f.split('<bLoad>')
                x = 0
                f = f[1]
            if x == 0:
                f = f.split('</bLoad>')
            f = f[0]
            self.load[n] = f
            n += 1
        self.dirtotal = 0
        for l in self.directory:
            self.dirtotal += 1
        #print('Sorted ', self.filedir)

    def writefile(self):
        '''Writes MLF File'''
        global configuration, typee, descript, end, startmod, startload, endload
        self.status.set('Writing MLF File')
        sleep(1)
        self.status.clear()
        n = 0
        input_file = open(self.filedir, 'w')
        input_file.write(configuration)
        for l in self.config:
            input_file.write(self.config[n])
            if n == 0:
                input_file.write(typee)
            n += 1
        input_file.write(descript)
        n = 0
        for l in self.directory:
            input_file.write(startmod)
            input_file.write(self.directory[n])
            input_file.write(startload)
            input_file.write(self.load[n])
            input_file.write(endload)
            n += 1
        input_file.write(end)
        input_file.close()
        #print('Written ', self.filedir)

    def adddir(self):
        '''Window that allows modders to add new directories to the MLF File'''
        self.status.set('Loading MLF File')
        sleep(1)
        self.status.clear()
        def adddirect():
            direct = dir.get()
            if direct != '':
                lod = ld.get()
                if lod == '':
                    lod = '1'
                self.directory.append(direct)
                self.load.append(lod)
                #print(self.directory)
                #print(self.load)
                self.writefile()
            add.destroy()
        add = Toplevel()
        add.wm_iconbitmap('Civ4BtS.ico')
        add.title('Add Directory to MLF File')
        add.geometry("%dx%d+%d+%d" %(300, 75, 100, 50))
        ld = StringVar()
        adddir = Frame(add)
        adddir.pack()
        newdir = Label(adddir, text='New directory name:', anchor=W, justify=LEFT)
        newdir.pack(side=LEFT)
        dir = Entry(adddir, justify=LEFT)
        dir.pack(side=LEFT)
        addload = Frame(add)
        addload.pack()
        newload = Label(addload, text='Check to Load:', anchor=W, justify=LEFT)
        newload.pack(side=LEFT)
        load = Checkbutton(addload, anchor=W, variable=ld, onvalue='1', offvalue='0', state=ACTIVE)
        load.pack(side=LEFT)
        buttons = Frame(add)
        buttons.pack(side=BOTTOM)
        quit = Button(buttons, text=' Close Window ', command=add.destroy)
        quit.pack(side=LEFT)
        submit = Button(buttons, text=' Add Directory ', command=adddirect)
        submit.pack(side=RIGHT)

    def writerestrict(self):
        '''Writes the Restrict Text File located in the same folder as program'''
        self.status.set('Writing Restricted File')
        sleep(1)
        self.status.clear()
        n = 0
        file = 'restricted.txt'
        input_file = open(file, 'w')
        input_file.write('''Please do not edit this file by hand...
To create a restricted directory: 
	start a line with a ! and ends with a $.
To create an alias for a directory: 
	start a line with a # type the directory name and end with a $.
	start a line with a @ type the name you wish to appear in the MLF Frontend and end with a $.

''')
        for l in self.restricted:
            input_file.write('!')
            input_file.write(self.restricted[n])
            input_file.write('''$
''')
            n += 1
        input_file.write('''
''')
        n = 0
        for l in self.modules:
            input_file.write('#')
            input_file.write(self.modules[n])
            input_file.write('''$
@''')
            input_file.write(self.alias[n])
            input_file.write('''$
''')
            n += 1
        input_file.write('''
''')
        n = 0
        for l in self.imagedir:
            input_file.write('%')
            input_file.write(self.imagedir[n])
            input_file.write('''$
*''')
            input_file.write(self.imagepath[n])
            input_file.write('''$
''')
            n += 1
        input_file.write('''
''')
        n = 0
        for l in self.dismod:
            input_file.write('^')
            input_file.write(self.dismod[n])
            input_file.write('''$
&''')
            input_file.write(self.discript[n])
            input_file.write('''$
''')
            n += 1
        input_file.write('''

Copyright (C) 2010 Devon McAvoy
Licensed under the GNU General Public License''')
        input_file.close()

    def addrestrict(self):
        '''Window that allows modders to restrict directories in the MLF File'''
        self.status.set('Loading Restrict File')
        sleep(1)
        self.status.clear()
        def addres():
            direct = dir.get()
            if direct != '':
                self.restricted.append(direct)
                #print(self.restricted)
                self.writerestrict()
            add.destroy()
        add = Toplevel()
        add.wm_iconbitmap('Civ4BtS.ico')
        add.title('Add Directory to Restrict')
        add.geometry("%dx%d+%d+%d" %(300, 150, 100, 50))
        ld = StringVar()
        warning = Label(add, text='''Make sure you have changed the Directory
to the Load option you want.
The Directory will be locked here after,
along with any MLFs in the Directory.
You may remove the restriction by deleting
the line in the restricted.txt.''', anchor=W, justify=LEFT)
        warning.pack()
        adddir = Frame(add)
        adddir.pack()
        newdir = Label(adddir, text='Directory to Restrict:', anchor=W, justify=LEFT)
        newdir.pack(side=LEFT)
        dir = Entry(adddir, justify=LEFT)
        dir.pack(side=LEFT)
        buttons = Frame(add)
        buttons.pack(side=BOTTOM)
        quit = Button(buttons, text=' Close Window ', command=add.destroy)
        quit.pack(side=LEFT)
        submit = Button(buttons, text=' Restrict ', command=addres)
        submit.pack(side=RIGHT)

    def addali(self):
        '''Window that allows modders to rename directories in the MLF File'''
        self.status.set('Loading Restrict File')
        sleep(1)
        self.status.clear()
        def addal():
            direct = dir.get()
            direct1 = a.get()
            if direct != '' and direct1 != '':
                self.modules.append(direct)
                self.alias.append(direct1)
                #print(self.modules)
                #print(self.alias)
                self.writerestrict()
            add.destroy()
        add = Toplevel()
        add.wm_iconbitmap('Civ4BtS.ico')
        add.title('Add Alias to Restrict')
        add.geometry("%dx%d+%d+%d" %(300, 90, 100, 50))
        ld = StringVar()
        adddir = Frame(add)
        adddir.pack()
        newdir = Label(adddir, text='Directory to Rename:', anchor=W, justify=LEFT)
        newdir.pack(side=LEFT)
        dir = Entry(adddir, justify=LEFT)
        dir.pack(side=LEFT)
        pad = Frame(add, height=10)
        pad.pack()
        adda = Frame(add)
        adda.pack()
        newa = Label(adda, text='Alias to Create:', anchor=W, justify=LEFT)
        newa.pack(side=LEFT)
        a = Entry(adda, justify=LEFT)
        a.pack(side=LEFT)
        buttons = Frame(add)
        buttons.pack(side=BOTTOM)
        quit = Button(buttons, text=' Close Window ', command=add.destroy)
        quit.pack(side=LEFT)
        submit = Button(buttons, text=' Rename ', command=addal)
        submit.pack(side=RIGHT)

    def addimage(self):
        '''Window that allows modders to add images for the directories in the MLF File'''
        self.status.set('Loading Restrict File')
        sleep(1)
        self.status.clear()
        def addim():
            direct = dir.get()
            direct1 = a.get()
            direct2 = b.get()
            direct3 = c.get()
            direct4 = d.get()
            direct5 = e.get()
            if direct != '' and direct1 != '':
                self.imagedir.append(direct)
                if direct2 != '':
                    f = '%s\%s' %(direct1, direct2)
                elif direct3 != '':
                    f = '%s\%s\%s' %(direct1, direct2, direct3)
                elif direct4 != '':
                    f = '%s\%s\%s\%s' %(direct1, direct2, direct3, direct4)
                elif direct5 != '':
                    f = '%s\%s\%s\%s\%s' %(direct1, direct2, direct3, direct4, direct5)
                else:
                    f = direct1
                self.imagepath.append(f)
                #print(self.imagedir)
                #print(self.imagepath)
                self.writerestrict()
            add.destroy()
        add = Toplevel()
        add.wm_iconbitmap('Civ4BtS.ico')
        add.title('Add Image to Restrict')
        add.geometry("%dx%d+%d+%d" %(740, 130, 100, 50))
        ld = StringVar()
        adddir = Frame(add)
        adddir.pack()
        newdir = Label(adddir, text='Directory to add Image to:', anchor=W, justify=LEFT)
        newdir.pack(side=LEFT)
        dir = Entry(adddir, justify=LEFT)
        dir.pack(side=LEFT)
        pad = Frame(add, height=10)
        pad.pack()
        warn = Frame(add)
        warn.pack()
        warning = Label(warn, text='''You do not need to fill all of the path boxes, those left blank will be ignored.''', anchor=W, justify=LEFT)
        warning.pack()
        pad1 = Frame(add, height=10)
        pad1.pack()
        addi = Frame(add)
        addi.pack()
        newi = Label(addi, text='Image Path:', anchor=W, justify=LEFT)
        newi.pack(side=LEFT)
        addp = Frame(add)
        addp.pack()
        newp = Label(addp, text='.../Modules/', anchor=W, justify=LEFT)
        newp.pack(side=LEFT)
        a = Entry(addp, justify=LEFT)
        a.pack(side=LEFT)
        slash = Label(addp, text='/', anchor=W, justify=LEFT)
        slash.pack(side=LEFT)
        b = Entry(addp, justify=LEFT)
        b.pack(side=LEFT)
        slash1 = Label(addp, text='/', anchor=W, justify=LEFT)
        slash1.pack(side=LEFT)
        c = Entry(addp, justify=LEFT)
        c.pack(side=LEFT)
        slash2 = Label(addp, text='/', anchor=W, justify=LEFT)
        slash2.pack(side=LEFT)
        d = Entry(addp, justify=LEFT)
        d.pack(side=LEFT)
        slash3 = Label(addp, text='/', anchor=W, justify=LEFT)
        slash3.pack(side=LEFT)
        e = Entry(addp, justify=LEFT)
        e.pack(side=LEFT)
        buttons = Frame(add)
        buttons.pack(side=BOTTOM)
        quit = Button(buttons, text=' Close Window ', command=add.destroy)
        quit.pack(side=LEFT)
        submit = Button(buttons, text=' Add Image ', command=addim)
        submit.pack(side=RIGHT)

    def adddis(self):
        '''Window that allows modders to add descriptions for directories in the MLF File'''
        self.status.set('Loading Restrict File')
        sleep(1)
        self.status.clear()
        def adddi():
            nonlocal a, dir
            s = 0
            direct = dir.get()
            direct1 = a.get(1.0, END)
            if len(direct1) > 200:
                self.disstatus.set('Discription Length Exceeds Limit of 200 Char.')
                s = 1
            #print(direct1)
            if direct != '' and direct1 != '' and s == 0:
                direct1 = direct1.split('\n')
                a = 0
                b = len(direct1) - 1
                if b == 0:
                    direct1 = direct1[b]
                if b == 1:
                    direct1 = direct1[a] + ' ' + direct1[b]
                if b == 2:
                    direct1 = direct1[a] + ' ' + direct1[b-1] + ' ' + direct1[b]
                if b == 3:
                    direct1 = direct1[a] + ' ' + direct1[b-2] + ' ' + direct1[b-1] + ' ' + direct1[b]
                if b == 4:
                    direct1 = direct1[a] + ' ' + direct1[b-3] + ' ' + direct1[b-2] + ' ' + direct1[b-1] + ' ' + direct1[b]
                self.disstatus.clear()
                self.dismod.append(direct)
                self.discript.append(direct1)
                self.writerestrict()
                add.destroy()
        add = Toplevel()
        add.wm_iconbitmap('Civ4BtS.ico')
        add.title('Add a Discription to Restrict')
        add.geometry("%dx%d+%d+%d" %(400, 165, 100, 50))
        self.disstatus = StatusBar(add)
        self.disstatus.pack(side=BOTTOM, fill=X)
        self.disstatus.set('')
        ld = StringVar()
        adddir = Frame(add)
        adddir.pack()
        newdir = Label(adddir, text='Directory to add Discription to:', anchor=W, justify=LEFT)
        newdir.pack(side=LEFT)
        dir = Entry(adddir, justify=LEFT)
        dir.pack(side=LEFT)
        pad = Frame(add, height=10)
        pad.pack()
        adda = Frame(add)
        adda.pack()
        newa = Label(adda, text='Discription:', anchor=W, justify=LEFT)
        newa.pack(side=LEFT)
        a = Text(adda, height=5, width=40)
        a.pack(fill=X, side=LEFT)
        buttons = Frame(add)
        buttons.pack(side=BOTTOM)
        quit = Button(buttons, text=' Close Window ', command=add.destroy)
        quit.pack(side=LEFT)
        submit = Button(buttons, text=' Add Discription ', command=adddi)
        submit.pack(side=RIGHT)

        

    def loadlist(self):
        '''Displays the load list for the MLF file'''
        def close():
            self.t = 1
            #print(self.t)
            lod.destroy()
        w = 0
        n = 0
        x = 0
        e = 0
        y = 0
        r = 0
        p = 0
        v = 1
        c = 0
        z = 0
        k = 0
        u = 0
        self.u = 0
        self.t = 0
        #print(self.alias)
        #print(self.modules)
        #print(self.restricted)
        #print(self.directory)
        #print(self.norestrfile)
        #print(self.load)
        for l in self.directory:
            w += 1
        r = w
        if r == 0:
            r += 1
        for l in self.directory:
            try:
                if self.next == 0:
                    path = r"%s\%s" % (self.abspath, self.directory[p])
                if self.next == 1:
                    path = r"%s\%s\%s" % (self.abspath, self.nextfi[self.n], self.directory[p])
                if os.access(path, 0) is False:
                    r -= 1
                else:
                    try:
                        for l in self.restricted:
                            if l == self.directory[p]:
                                z += 1
                    except AttributeError:
                        z = 0
                p += 1
            except IndexError:
                pass
        conf = str('Default Config: '+ self.config[0])
        type = str('Type: '+ self.config[1])
        #print('T ', t)
        f = StringVar()
        f1 = StringVar()
        f2 = StringVar()
        f3 = StringVar()
        f4 = StringVar()
        while n < w and e == 0 and self.t == 0:
            lod = Toplevel()
            lod.wm_iconbitmap('Civ4BtS.ico')
            lod.title('MLF Load List Page %s, # Listed Dir: %s, Actual Dir: %s, Restricted Dir: %s' % (v, w, r, z))
            lod.geometry('%dx%d+%d+%d' %(500, 430, 100, 50))
            ldim = Frame(lod)
            ldim.pack(fill=Y, anchor=W, side=LEFT)
            ld1 = Frame(lod)
            ld1.pack(fill=X, anchor=W)
            ld2 = Frame(lod)
            ld2.pack(fill=X, anchor=W)
            ld3 = Frame(lod)
            ld3.pack(fill=X, anchor=W)
            ld4 = Frame(lod)
            ld4.pack(fill=X, anchor=W)
            ld5 = Frame(lod)
            ld5.pack(fill=X, anchor=W)
            ld6 = Frame(lod)
            ld6.pack(fill=X, anchor=W)
            pad = Label(ldim, text='', height=3)
            pad.pack()
            ld1im = Label(ldim, height=64)
            ld1im.pack()
            ld2im = Label(ldim, height=64)
            ld2im.pack()
            ld3im = Label(ldim, height=64)
            ld3im.pack()
            ld4im = Label(ldim, height=64)
            ld4im.pack()
            ld5im = Label(ldim, height=64)
            ld5im.pack()
            ld1lbl = Label(ld1, text=conf, anchor=W, justify=LEFT)
            ld1lbl.pack()
            ld1lbl2 = Label(ld1, text=type, anchor=W, justify=LEFT)
            ld1lbl2.pack()
            ld2lbl = Label(ld2, text='', anchor=W, justify=LEFT, height=5, wraplength=400)
            ld2lbl.pack(side=LEFT)
            ld2ld = Checkbutton(ld2, anchor=W, variable=f, onvalue='1', offvalue='0')
            ld3lbl = Label(ld3, text='', anchor=W, justify=LEFT, height=3, wraplength=400)
            ld3lbl.pack(side=LEFT)
            ld3ld = Checkbutton(ld3, anchor=W, variable=f1, onvalue='1', offvalue='0')
            ld4lbl = Label(ld4, text='', anchor=W, justify=LEFT, height=5, wraplength=400)
            ld4lbl.pack(side=LEFT)
            ld4ld = Checkbutton(ld4, anchor=W, variable=f2, onvalue='1', offvalue='0')
            ld5lbl = Label(ld5, text='', anchor=W, justify=LEFT, height=4, wraplength=400)
            ld5lbl.pack(side=LEFT)
            ld5ld = Checkbutton(ld5, anchor=W, variable=f3, onvalue='1', offvalue='0')
            ld6lbl = Label(ld6, text='', anchor=W, justify=LEFT, height=4, wraplength=400)
            ld6lbl.pack(side=LEFT)
            ld6ld = Checkbutton(ld6, anchor=W, variable=f4, onvalue='1', offvalue='0')
            buttons = Frame(lod)
            buttons.pack(side=BOTTOM)
            quit = Button(buttons, anchor=W, text=' Close Window ', command=close)
            quit.pack(side=LEFT)
            cont = Button(buttons, anchor=W, text=' Next Page ', command=lod.destroy)
            cont.pack(side=RIGHT)
            savegame= Label(lod, anchor=W, text='Warning: Changing the Load order will break saved Games', justify=LEFT)
            savegame.pack(side=BOTTOM)
            c = 0
            #print('Marker: Created window')
            while c < 5 and e == 0 and self.t == 0:
                y = 0
                direct = ' '
                image_file = ''
                descript = ''
                #print('N ', n)
                if self.norestrfile == 0:
                    y = 0
                    while y < w and e != 1:
                        #print('Marker: Checkr')
                        for i in self.restricted:
                            try:
                                l = self.directory[n]
                            except IndexError:
                                e = 1
                            if l == i:
                                n += 1
                                try:
                                    l = self.directory[n]
                                except IndexError:
                                    e = 1
                        #print('N ', n)
                        #print('Marker: Checkp')
                        try:
                            if self.next == 0:
                                path = r"%s\%s" % (self.abspath, self.directory[n])
                            if self.next == 1:
                                path = r"%s\%s\%s" % (self.abspath, self.nextfi[self.n], self.directory[n])
                            if os.access(path, 0) is False:
                                n += 1
                        except IndexError:
                            e = 1
                        y += 1
                    #print('N ', n)
                    #print('Marker: Checka')
                    x = 0
                    if e == 0:
                        try:
                            l = self.directory[n]
                        except IndexError:
                            e = 1
                        for i in self.modules:
                            if l == i:
                                direct = self.alias[x]
                                a = x
                            x += 1
                    k = 0
                    if e == 0:
                        try:
                            l = self.directory[n]
                        except IndexError:
                            pass
                        for i in self.imagedir:
                            if l == i:
                                image_file = '%s%s' % (self.abspath, self.imagepath[k])
                                hg = k
                            k += 1
                    k = 0
                    if e == 0:
                        try:
                            l = self.directory[n]
                        except IndexError:
                            pass
                        for i in self.dismod:
                            if l == i:
                                descript = self.discript[k]
                                am = k
                            k += 1
                else:
                    if e == 0:
                        for l in self.directory:
                            #print('N ', n)
                            #print('Marker: Checkp')
                            try:
                                if self.next == 0:
                                    path = r"%s\%s" % (self.abspath, self.directory[n])
                                if self.next == 1:
                                    path = r"%s\%s\%s" % (self.abspath, self.nextfi[self.n], self.directory[n])
                                if os.access(path, 0) is False:
                                    n += 1
                            except IndexError:
                                e = 1
                        try:
                            direct = self.directory[n]
                        except IndexError:
                            e = 1
                if direct == ' ' and e == 0:
                    try:
                        direct = self.directory[n]
                    except IndexError:
                            e = 1
                #print('Marker: 1')
                #print(direct)
                if e == 0:
                    if descript == '':
                        dir = str('Directory: '+ direct)
                    else:
                        dir = str('Directory: '+ direct +'''
'''+ descript)
                    if c == 0:
                        if self.norestrfile == 0 and self.noimage == 0 and image_file != '':
                            photo1 = PhotoImage(file=image_file)
                            ld1im.configure(image=photo1)
                        else:
                            image_file = 'Civ4BtS.gif'
                            photo1 = PhotoImage(file=image_file)
                            ld1im.configure(image=photo1)
                        ld2lbl.config(text=dir)
                        ld2ld.pack(side=RIGHT)
                        if self.load[n] == '1':
                            ld2ld.configure(foreground='red')
                        q = n
                    if c == 1:
                        if self.norestrfile == 0 and self.noimage == 0 and image_file != '':
                            photo2 = PhotoImage(file=image_file)
                            ld2im.configure(image=photo2)
                        else:
                            image_file = 'Civ4BtS.gif'
                            photo2 = PhotoImage(file=image_file)
                            ld2im.configure(image=photo2)
                        ld3lbl.config(text=dir)
                        ld3ld.pack(side=RIGHT)
                        if self.load[n] == '1':
                            ld3ld.configure(foreground='red')
                        q1 = n
                    if c == 2:
                        if self.norestrfile == 0 and self.noimage == 0 and image_file != '':
                            photo3 = PhotoImage(file=image_file)
                            ld3im.configure(image=photo3)
                        else:
                            image_file = 'Civ4BtS.gif'
                            photo3 = PhotoImage(file=image_file)
                            ld3im.configure(image=photo3)
                        ld4lbl.config(text=dir)
                        ld4ld.pack(side=RIGHT)
                        if self.load[n] == '1':
                            ld4ld.configure(foreground='red')
                        q2 = n
                    if c == 3:
                        if self.norestrfile == 0 and self.noimage == 0 and image_file != '':
                            photo4 = PhotoImage(file=image_file)
                            ld4im.configure(image=photo4)
                        else:
                            image_file = 'Civ4BtS.gif'
                            photo4 = PhotoImage(file=image_file)
                            ld4im.configure(image=photo4)
                        ld5lbl.config(text=dir)
                        ld5ld.pack(side=RIGHT)
                        if self.load[n] == '1':
                            ld5ld.configure(foreground='red')
                        q3 = n
                    if c == 4:
                        if self.norestrfile == 0 and self.noimage == 0 and image_file != '':
                            photo5 = PhotoImage(file=image_file)
                            ld5im.configure(image=photo5)
                        else:
                            image_file = 'Civ4BtS.gif'
                            photo5 = PhotoImage(file=image_file)
                            ld5im.configure(image=photo5)
                        ld6lbl.config(text=dir)
                        ld6ld.pack(side=RIGHT)
                        if self.load[n] == '1':
                            ld6ld.configure(foreground='red')
                        q4 = n
                    #print('N ', n)
                    #input('')
                    if direct == self.directory[n] or direct == self.alias[a]:
                        n += 1
                    c += 1
            #print('Marker: 2')
            lod.wait_window()
            if self.t != 1:
                if c == 0:
                    pass
                if c == 1:
                    t = f.get()
                    if t != '':
                        self.load[q] = t
                if c == 2:
                    t = f.get()
                    t1 = f1.get()
                    if t != '':
                        self.load[q] = t
                    if t1 != '':
                        self.load[q1] = t1
                if c == 3:
                    t = f.get()
                    t1 = f1.get()
                    t2 = f2.get()
                    if t != '':
                        self.load[q] = t
                    if t1 != '':
                        self.load[q1] = t1
                    if t2 != '':
                        self.load[q2] = t2
                if c == 4:
                    t = f.get()
                    t1 = f1.get()
                    t2 = f2.get()
                    t3 = f3.get()
                    if t != '':
                        self.load[q] = t
                    if t1 != '':
                        self.load[q1] = t1
                    if t2 != '':
                        self.load[q2] = t2
                    if t3 != '':
                        self.load[q3] = t3
                if c == 5:
                    t = f.get()
                    t1 = f1.get()
                    t2 = f2.get()
                    t3 = f3.get()
                    t4 = f4.get()
                    if t != '':
                        self.load[q] = t
                    if t1 != '':
                        self.load[q1] = t1
                    if t2 != '':
                        self.load[q2] = t2
                    if t3 != '':
                        self.load[q3] = t3
                    if t4 != '':
                        self.load[q4] = t4
        #if f == '1' or f == '0':
        #        self.load[n] = f
        #else:
        #    pass
        #print(self.directory)
        #print(self.load)

    def loadnext(self):
        '''Loads MLF's in subdirectories in the modules folder'''
        self.next = 1
        ans = ''
        self.n = 0
        ask = 0
        self.nextfi = self.directory
        self.preload = self.load
        for l in self.nextfi:
            direct = self.nextfi[self.n]
            direct = '%s\%s\MLF_CIV4ModularLoadingControls.xml' % (self.abspath, direct)
            self.filedir = direct
            try:
                f = open(self.filedir, 'r')
                #print('Opening ', self.filedir)
                self.readfile()
                self.sort()
                #print(self.config, '\n', self.directory, '\n', self.load)
                self.difer()
                for l in self.restricted:
                    if self.nextfi[self.n] == l:
                        self.x = 3
                if self.x == 2:
                    #print(self.nextfi[n])
                    #print(self.preload[n])
                    if self.preload[self.n] == '0':
                        ask = 1
                    if self.preload[self.n] == '1':
                        ask = 0
                        ans = ' '
                    #print(ask)
                    if ask == 1:
                        def yes():
                            nonlocal ans
                            ans = 'Yes'
                            top.destroy()
                        def no():
                            nonlocal ans
                            ans = 'No'
                            top.destroy()
                        top = Toplevel()
                        top.title('Load?')
                        top.geometry('%dx%d+%d+%d' % (300, 100, 100, 50))
                        q = str(self.nextfi[self.n] + ''' will not be loaded.
Do you wish to still edit the
MLF File in the Directory?''')
                        question = Label(top, text=q, anchor=W, justify=LEFT)
                        question.pack()
                        buttons = Frame(top)
                        buttons.pack(side=BOTTOM)
                        ansno = Button(buttons, text= 'No ', command=no)
                        ansno.pack(side=LEFT)
                        ansyes = Button(buttons, text=' Yes ', command=yes)
                        ansyes.pack(side=RIGHT)
                        top.wait_window()
                    if ans.startswith('Y') or ans.startswith('y') or ask == 0:
                        self.loadlist()
                        if self.t != 1:
                            self.writefile()
                        #print('')
                    else:
                        pass
                if self.x == 3:
                    rest = str('Access to the MLF File in '+ self.nextfi[self.n]+ ' is restricted!')
                    self.status.set(rest)
                    sleep(1)
                    self.status.clear()
            except IOError:
                nomlf = str('No MLF File in '+ self.nextfi[self.n])
                self.status.set(nomlf)
                sleep(1)
                self.status.clear()
            self.filedir = self.abspath+'\MLF_CIV4ModularLoadingControls.xml'
            self.readfile()
            self.sort()
            self.n += 1
        self.next = 0

    def difer(self):
        '''Makes sure MLF in subdirectory is different than the main MLF folder'''
        self.x = 0
        if self.nextfi == self.directory:
            self.x = 1
            #print(self.filedir, ' does not differ from previous file!')
        else:
            self.x = 2
            #print(self.filedir, ' is different than previous file!')

if __name__ == '__main__':
    root = Tk()
    root.title('MLF Frontend %s' % version)
    app = MLF(root)
    root.mainloop()

#Copyright (C) 2010 Devon McAvoy
