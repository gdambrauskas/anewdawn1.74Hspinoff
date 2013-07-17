#### Civilization 4 SDK Makefile 1.0 ####
####  Copyright 2010 Danny Daemonic  ####
#########################################

#### Paths ####
TOOLKIT=W:\games-info\_game_files\civilization4\modding-development\Microsoft Visual C++ Toolkit 2003
PSDK=W:\games-info\_game_files\civilization4\modding-development\WindowsSDK-v6
## Uncomment to have newly compiled dlls copied to your mod's Assets directory
#YOURMOD=C:\Users\Danny\Documents\My Games\beyond the sword\MODS\MyMod
YOURMOD=W:\games\civ4\Beyond the Sword\Mods\gand

#### Tools ####
CC="$(TOOLKIT)\bin\cl.exe"
CPP="$(TOOLKIT)\bin\cl.exe"
LD="$(TOOLKIT)\bin\link.exe"
RC="$(PSDK)\bin\rc.exe"
## Uncomment to build dependencies using fastdep
FD="$(MAKEDIR)\bin\fastdep.exe"

#### BLACKLIST ####
## Uncomment to block CvTextScreen (accidentally included by Firaxis)
BLACKLIST=CvTextScreens

#### You shouldn't need to modify anything beyond this point ####
#################################################################

Debug_GLOBAL_CFLAGS= /D_MOD_SENTRY /D_MOD_FRACTRADE /D_MOD_GOVWORKERS /DQC_MASTERY_VICTORY /DLOG_AI /D_MOD_GWARM /D_MOD_SHAM_SPOILER
Final_Release_GLOBAL_CFLAGS= /D_MOD_SENTRY /D_MOD_FRACTRADE /D_MOD_GOVWORKERS /DQC_MASTERY_VICTORY /D_MOD_GWARM /D_MOD_SHAM_SPOILER
Release_GLOBAL_CFLAGS= /D_MOD_SENTRY /D_MOD_FRACTRADE /D_MOD_GOVWORKERS /DQC_MASTERY_VICTORY /DLOG_AI /DFP_PROFILE_ENABLE /D_MOD_GWARM /D_MOD_SHAM_SPOILER

#### Target Files ####
Debug_BIN=Debug\CvGameCoreDLL.dll
Release_BIN=Release\CvGameCoreDLL.dll
Final_Release_BIN=Final_Release\CvGameCoreDLL.dll

!IF [IF NOT EXIST CvGameCoreDLL.rc EXIT 1] == 0
Debug_RESOURCE=Debug\CvGameCoreDLL.res
Release_RESOURCE=Release\CvGameCoreDLL.res
Final_Release_RESOURCE=Final_Release\CvGameCoreDLL.res
!ENDIF

Debug_STATICLIB=Debug\CvGameCoreDLL.lib
Release_STATICLIB=Release\CvGameCoreDLL.lib
Final_Release_STATICLIB=Final_Release\CvGameCoreDLL.lib

Debug_LIBDEF=Debug\CvGameCoreDLL.def
Release_LIBDEF=Release\CvGameCoreDLL.def
Final_Release_LIBDEF=Final_Release\CvGameCoreDLL.def

Debug_PCH=Debug\CvGameCoreDLL.pch
Release_PCH=Release\CvGameCoreDLL.pch
Final_Release_PCH=Final_Release\CvGameCoreDLL.pch

Debug_PDB=Debug\CvGameCoreDLL.pdb
Release_PDB=Release\CvGameCoreDLL.pdb
Final_Release_PDB=Final_Release\CvGameCoreDLL.pdb

Debug_OTHER=Debug\CvGameCoreDLL.exp Debug\CvGameCoreDLL.ilk
Release_OTHER=Release\CvGameCoreDLL.exp
Final_Release_OTHER=Final_Release\CvGameCoreDLL.exp

#### CFLAGS ####
GLOBAL_CFLAGS=/GR /Gy /W3 /EHsc /Gd /Gm- /DWIN32 /D_WINDOWS /D_USRDLL /DCVGAMECOREDLL_EXPORTS /Yu"CvGameCoreDLL.h"
Debug_CFLAGS=/MD /Zi /Od /D_DEBUG /RTC1 /Fp"$(Debug_PCH)" $(GLOBAL_CFLAGS) $(Debug_GLOBAL_CFLAGS)
Release_CFLAGS=/MD /O2 /Oy /Oi /G7 /DNDEBUG /DFINAL_RELEASE /Fp"$(Release_PCH)" $(GLOBAL_CFLAGS) $(Release_GLOBAL_CFLAGS)
Final_Release_CFLAGS=/MD /O2 /Oy /Oi /G7 /DNDEBUG /DFINAL_RELEASE /Fp"$(Final_Release_PCH)" $(GLOBAL_CFLAGS) $(Final_Release_GLOBAL_CFLAGS)

#### LDFLAGS ####
GLOBAL_LDFLAGS=/DLL /NOLOGO /SUBSYSTEM:WINDOWS /LARGEADDRESSAWARE /TLBID:1
Debug_LDFLAGS=/INCREMENTAL /DEBUG /PDB:"$(Debug_PDB)" /IMPLIB:"$(Debug_STATICLIB)" $(GLOBAL_LDFLAGS)
Release_LDFLAGS=/INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF /IMPLIB:"$(Release_STATICLIB)" /PDB:"$(Release_PDB)" $(GLOBAL_LDFLAGS)
Final_Release_LDFLAGS=/INCREMENTAL:NO /OPT:REF /OPT:ICF /IMPLIB:"$(Final_Release_LIBDEF)" /PDB:"$(Final_Release_PDB)" $(GLOBAL_LDFLAGS)

#### INCLUDES ####
GLOBAL_INCS=/I"$(TOOLKIT)/include" /I"$(PSDK)/Include" /I"$(PSDK)/Include/mfc"
PROJECT_INCS=/IBoost-1.32.0/include /IPython24/include
Debug_INCS=$(PROJECT_INCS) $(GLOBAL_INCS)
Release_INCS=$(PROJECT_INCS) $(GLOBAL_INCS)
Final_Release_INCS=$(PROJECT_INCS) $(GLOBAL_INCS)

#### LIBS ####
GLOBAL_LIBS=/LIBPATH:"$(TOOLKIT)/lib" /LIBPATH:"$(PSDK)/Lib" winmm.lib user32.lib
PROJECT_LIBS=/LIBPATH:Python24/libs /LIBPATH:boost-1.32.0/libs/ boost_python-vc71-mt-1_32.lib
Debug_LIBS=$(PROJECT_LIBS) $(GLOBAL_LIBS) msvcprt.lib #/NODEFAULTLIB:msvcprtd.lib
Release_LIBS=$(PROJECT_LIBS) $(GLOBAL_LIBS)
Final_Release_LIBS=$(PROJECT_LIBS) $(GLOBAL_LIBS)

#### Objects ####
Debug_LINKOBJS=$(Debug_OBJS)
Release_LINKOBJS=$(Release_OBJS)
Final_Release_LINKOBJS=$(Final_Release_OBJS)

#### Auto SOURCES/OBJS ####
!IF [ECHO SOURCES= \> sources.mk] == 0 && \
    [FOR %i IN (*.cpp) DO @ECHO. "%i" \>> sources.mk] == 0 && \
    [ECHO.>> sources.mk] == 0 && \
    [ECHO Debug_OBJS= \>> sources.mk] == 0 && \
    [FOR /F "delims=." %i IN ('dir /b *.cpp') DO @ECHO. Debug\%i.obj \>> sources.mk] == 0 && \
    [ECHO.>> sources.mk] == 0 && \
    [ECHO Release_OBJS= \>> sources.mk] == 0 && \
    [FOR /F "delims=." %i IN ('dir /b *.cpp') DO @ECHO. Release\%i.obj \>> sources.mk] == 0 && \
    [ECHO.>> sources.mk] == 0  && \
    [ECHO Final_Release_OBJS= \>> sources.mk] == 0 && \
    [FOR /F "delims=." %i IN ('dir /b *.cpp') DO @ECHO. Final_Release\%i.obj \>> sources.mk] == 0 && \
    [ECHO.>> sources.mk] == 0
!INCLUDE sources.mk
!IF [DEL sources.mk]
!ENDIF
!ENDIF

#### Targets ####
#################

.PHONY: all clean Debug_clean Release_clean Final_Release_clean Debug Release Final_Release

all: Debug Final_Release Release

clean: Debug_clean Release_clean Final_Release_clean

Debug_clean:
	@FOR %i IN ($(Debug_BIN) $(Debug_STATICLIB) $(Debug_LIBDEF) \
		Debug\*.obj Debug\*.@ $(Debug_RESOURCE) \
		$(Debug_PCH) $(Debug_PDB) $(Debug_OTHER)) DO @IF EXIST "%i" DEL "%i"

Release_clean:
	@FOR %i IN ($(Release_BIN) $(Release_STATICLIB) $(Release_LIBDEF) \
		Release\*.obj Release\*.@ $(Release_RESOURCE) \
		$(Release_PCH) $(Release_PDB) $(Release_OTHER)) DO @IF EXIST "%i" DEL "%i"
		
Final_Release_clean:
	@FOR %i IN ($(Final_Release_BIN) $(Final_Release_STATICLIB) $(Final_Release_LIBDEF) \
		Final_Release\*.obj Final_Release\*.@ $(Final_Release_RESOURCE) \
		$(Final_Release_PCH) $(Final_Release_PDB) $(Final_Release_OTHER)) DO @IF EXIST "%i" DEL "%i"

Debug: Debug_DIR Debug_unfinished $(Debug_PCH) $(Debug_BIN)
!IFDEF YOURMOD
	-COPY "$(Debug_BIN)" "$(YOURMOD)\Assets\."
!ENDIF

Release: Release_DIR Release_unfinished $(Release_PCH) $(Release_BIN)
!IFDEF YOURMOD
	-COPY "$(Release_BIN)" "$(YOURMOD)\Assets\."
!ENDIF

Final_Release: Final_Release_DIR Final_Release_unfinished $(Final_Release_PCH) $(Final_Release_BIN)
!IFDEF YOURMOD
	-COPY "$(Final_Release_BIN)" "$(YOURMOD)\Assets\."
!ENDIF

Debug_DIR:
	-@IF NOT EXIST "Debug\." MKDIR "Debug"

Release_DIR:
	-@IF NOT EXIST "Release\." MKDIR "Release"
	
Final_Release_DIR:
	-@IF NOT EXIST "Final_Release\." MKDIR "Final_Release"

Debug_unfinished:
	@ECHO.>Debug\unfinished.@
	@FOR /F "delims=@" %i IN ('dir /b Debug\*.@') DO \
		@IF EXIST "Debug\%i" DEL "Debug\%i"
	@FOR /F %i IN ('dir /b Debug\*.@') DO \
		@IF EXIST "Debug\%i" DEL "Debug\%i"

Release_unfinished:
	@ECHO.>Release\unfinished.@
	@FOR /F "delims=@" %i IN ('dir /b Release\*.@') DO \
		@IF EXIST "Release\%i" DEL "Release\%i"
	@FOR /F %i IN ('dir /b Release\*.@') DO \
		@IF EXIST "Release\%i" DEL "Release\%i"
		
Final_Release_unfinished:
	@ECHO.>Final_Release\unfinished.@
	@FOR /F "delims=@" %i IN ('dir /b Final_Release\*.@') DO \
		@IF EXIST "Final_Release\%i" DEL "Final_Release\%i"
	@FOR /F %i IN ('dir /b Final_Release\*.@') DO \
		@IF EXIST "Final_Release\%i" DEL "Final_Release\%i"

$(Debug_BIN): $(Debug_LINKOBJS) $(Debug_RESOURCE)
	$(LD) /out:$(Debug_BIN) $(Debug_LDFLAGS) $(Debug_LIBS) $(Debug_LINKOBJS) $(Debug_RESOURCE)

$(Release_BIN): $(Release_LINKOBJS) $(Release_RESOURCE)
	$(LD) /out:$(Release_BIN) $(Release_LDFLAGS) $(Release_LIBS) $(Release_LINKOBJS) $(Release_RESOURCE)
	
$(Final_Release_BIN): $(Final_Release_LINKOBJS) $(Final_Release_RESOURCE)
	$(LD) /out:$(Final_Release_BIN) $(Final_Release_LDFLAGS) $(Final_Release_LIBS) $(Final_Release_LINKOBJS) $(Final_Release_RESOURCE)

.cpp{Debug}.obj:
	@ECHO.>"$*.obj.@"
    $(CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /Fo$*.obj /c $<
	@DEL "$*.obj.@"

.cpp{Release}.obj:
	@ECHO.>"$*.obj.@"
    $(CPP) /nologo $(Release_CFLAGS) $(Release_INCS) /Fo$*.obj /c $<
	@DEL "$*.obj.@"
	
.cpp{Final_Release}.obj:
	@ECHO.>"$*.obj.@"
    $(CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /Fo$*.obj /c $<
	@DEL "$*.obj.@"

$(Debug_PCH) Debug\_precompile.obj:
	@ECHO.>"$(Debug_PCH).@"
	@ECHO.>"Debug\_precompile.obj.@"
	$(CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /YcCvGameCoreDLL.h /Fo"Debug\_precompile.obj" /c _precompile.cpp
	@DEL "$(Debug_PCH).@"
	@DEL "Debug\_precompile.obj.@"

$(Release_PCH) Release\_precompile.obj:
	@ECHO.>"$(Release_PCH).@"
	@ECHO.>"Release\_precompile.obj.@"
    $(CPP) /nologo $(Release_CFLAGS) $(Release_INCS) /YcCvGameCoreDLL.h /Fo"Release\_precompile.obj" /c _precompile.cpp
	@DEL "$(Release_PCH).@"
	@DEL "Release\_precompile.obj.@"
	
$(Final_Release_PCH) Final_Release\_precompile.obj:
	@ECHO.>"$(Final_Release_PCH).@"
	@ECHO.>"Final_Release\_precompile.obj.@"
    $(CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /YcCvGameCoreDLL.h /Fo"Final_Release\_precompile.obj" /c _precompile.cpp
	@DEL "$(Final_Release_PCH).@"
	@DEL "Final_Release\_precompile.obj.@"

.rc{Debug}.res:
	@ECHO.>"$*.res.@"
	$(RC) /Fo$@ $(Debug_INCS) $<
	@DEL "$*.res.@"

.rc{Release}.res:
	@ECHO.>"$*.res.@"
	$(RC) /Fo$@ $(Release_INCS) $<
	@DEL "$*.res.@"
	
.rc{Final_Release}.res:
	@ECHO.>"$*.res.@"
	$(RC) /Fo$@ $(Final_Release_INCS) $<
	@DEL "$*.res.@"

!IFDEF BLACKLIST

Debug\$(BLACKLIST).obj: $(BLACKLIST).cpp
	@ECHO.>"$*.obj.@"
	@ECHO.>"$*-dummy.cpp"
	$(CPP) /nologo $(Debug_CFLAGS) $(Debug_INCS) /Y- /Fo$@ /c "$*-dummy.cpp"
	@DEL "$*-dummy.cpp"
	@DEL "$*.obj.@"

Release\$(BLACKLIST).obj: $(BLACKLIST).cpp
	@ECHO.>"$*.obj.@"
	@ECHO.>"$*-dummy.cpp"
	$(CPP) /nologo $(Release_CFLAGS) $(Release_INCS) /Y- /Fo$@ /c "$*-dummy.cpp"
	@DEL "$*-dummy.cpp"
	@DEL "$*.obj.@"
	
Final_Release\$(BLACKLIST).obj: $(BLACKLIST).cpp
	@ECHO.>"$*.obj.@"
	@ECHO.>"$*-dummy.cpp"
	$(CPP) /nologo $(Final_Release_CFLAGS) $(Final_Release_INCS) /Y- /Fo$@ /c "$*-dummy.cpp"
	@DEL "$*-dummy.cpp"
	@DEL "$*.obj.@"

!ENDIF


!IFDEF FD

!IF [IF NOT EXIST $(FD) EXIT 1] == 0
!IF [$(FD) --objectextension=pch -q -O Debug CvGameCoreDLL.cpp > depends] != 0 || \
    [$(FD) --objectextension=obj -q -O Debug $(SOURCES) >> depends] != 0 || \
    [$(FD) --objectextension=pch -q -O Release CvGameCoreDLL.cpp >> depends] != 0 || \
    [$(FD) --objectextension=obj -q -O Release $(SOURCES) >> depends] != 0 || \
    [$(FD) --objectextension=pch -q -O Final_Release CvGameCoreDLL.cpp >> depends] != 0 || \
    [$(FD) --objectextension=obj -q -O Final_Release $(SOURCES) >> depends] != 0
!MESSAGE Error running fastdep.
!ENDIF
!ELSE
!IF [ECHO "fastdep.exe" NOT FOUND! && \
     ECHO Please edit Makefile to reflect the correct path of fastdep. && \
     ECHO. ]
!ENDIF
!ENDIF

!ENDIF

!IF EXIST(depends)
!INCLUDE depends
!ENDIF
