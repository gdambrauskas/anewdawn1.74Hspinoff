//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvXMLLoadUtilitySetMod.h
//
//  AUTHOR:  Vincent Veldman  --  11/2007
//
//  PURPOSE: Group of functions for Vectorized Enumeration and MLF for Civilization 4 BtS
//
//------------------------------------------------------------------------------------------------
//  Copyright (c) 2003 Firaxis Games, Inc. All rights reserved.
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "CvDLLXMLIFaceBase.h"
#include "CvXMLLoadUtility.h"
#include "CvXMLLoadUtilitySetMod.h"
/************************************************************************************************/
/* Afforess	                  Start		 06/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#include "CvInitCore.h"
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

// In the following method we set the order of loading properly
void CvXMLLoadUtilitySetMod::setModLoadControlDirArray(bool bSetModControl)
{
	// basic variables
	GC.resetModLoadControlVector();
	if (!bSetModControl)
	{
		GC.setModLoadControlVector("Modules");
		return;
	}
	bool bContinue = true;
	int iDirDepthTemp = 0;		//we don't want to change the global value	

	// To check when bLoad = 1 if the module is valid
	CvXMLLoadUtilityModTools* p_szDirName = new CvXMLLoadUtilityModTools;
/************************************************************************************************/
/* Afforess	                  Start		 06/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	CvString szModDirectory;
	szModDirectory = GC.getInitCore().getDLLPath() + "\\";
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	bool bModuleExist = false;	// Valid Module?

	// logging to file
	CvXMLLoadUtility* pXMLLoadUtility = new CvXMLLoadUtility;
	pXMLLoadUtility->logMLF("\n\nThe game will now load the modules into the load vector in the order set by the MLF files:");

	// Initialization of a temp LoadArray 
	bool** aabLoaded = NULL;	// pointer to a pointer	
	aabLoaded = new bool*[GC.getNumModLoadControlInfos()];	//allocate the rows	
   	for ( int iInfosSet = 0; iInfosSet < GC.getNumModLoadControlInfos(); iInfosSet++)
	{
		aabLoaded[iInfosSet] = new bool[GC.getModLoadControlInfos(iInfosSet).getNumModules()];	// allocate the colums
		//loop through the modules of each MLF
		for ( int iSet = 0; iSet < GC.getModLoadControlInfos(iInfosSet).getNumModules(); iSet++ )
		{
			// set the values according to the InfoClass( CvInfos.cpp )
			if ( GC.getModLoadControlInfos(iInfosSet).isLoad(iSet))
			{
				aabLoaded[iInfosSet][iSet] = true;
			}
			else
			{
				aabLoaded[iInfosSet][iSet] = false;
			}
		}
	}

	// more basic variables
	int iInfosLoad;
	int iLoad;
	bool bParentDisable;

	while(bContinue)	//we start with iDirDepth at the highest value(set during the MLF loading)
	{		
		iDirDepthTemp = 0;
		iInfosLoad = 0;
		iLoad = 0;

		//Set the top level MLF to have a start to loop through
		for ( iLoad = 0; iLoad < GC.getModLoadControlInfos(0).getNumModules(); iLoad++ )
		{
			if (aabLoaded[0][iLoad])
			{
				break;  //we found next iLoad
			}
		}

		while(bContinue)
		{
			bContinue = false;
			bool bHitDeepestDepthLoop = false;
			//loop through all the MLF's
			for ( int iiInfos = 0; iiInfos < GC.getNumModLoadControlInfos(); iiInfos++)
			{
				// only loop through files that are actually 1 dir deeper
				if (GC.getModLoadControlInfos(iiInfos).getDirDepth() == iDirDepthTemp + 1)
				{
					// Check if the Loop Module is a Child of the actual Module which we will load if no Children will be found.
					if ( CvString::format("%s\\", GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str()) == GC.getModLoadControlInfos(iiInfos).getParentFolder().c_str())
					{
						//loop through the modules of each MLF
						for ( int ii = 0; ii < GC.getModLoadControlInfos(iiInfos).getNumModules(); ii++ )
						{
							//Loaded already? Don't need an endless loop
							if (aabLoaded[iiInfos][ii])
							{
								//set the loading module to the current module
								iInfosLoad = iiInfos;
								iLoad = ii;										

								//The first valid we find on this level will be the first to load, 
								//so we abord further search inside this class, and Continue the loop to the next level
								//To see if this new Module itself has another MLF the next level
								bHitDeepestDepthLoop = true;
								iDirDepthTemp++;

								bContinue = true;                                      
							}
							if (bHitDeepestDepthLoop) break;
						}								
					}							
				}
				if (bHitDeepestDepthLoop) break;
			}
		}

		// means we are loading something which is not the deepest in directory structure that has been found valid to load...
		if ( GC.getModLoadControlInfos(iInfosLoad).getDirDepth() != iDirDepthTemp )
		{
			FAssertMsg(false, "Something ain't right with the parental MLF disabling function, put a bug report on http://www.worldofcivilization.net/bugtracker/bugtracker.htm, please supply your MLF configuration. All of them!");
			break;
		}

		//load the deepest file
		if (aabLoaded[iInfosLoad][iLoad])
		{
			// check for valid xml files			
			if ( isValidModule(bModuleExist, (szModDirectory + GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str()).c_str(), GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str(), CvString::format(".xml").c_str())/*(int)aszValidFilesVerification.size() > 0*/ )	// does ANY valid xml file exist?
			{
				// if valid, module XML file(s) exist
				// note: if dir isn't valid, of course xml's for that dir aren't valid either
				pXMLLoadUtility->logMLF("Load Priority: %d, \"%s\"", GC.getModLoadControlVectorSize(), GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str());
				GC.setModLoadControlVector(GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str());
			}
			else
			{
				// if not valid, module XML file(s) doesn't exist
				pXMLLoadUtility->logMLF("No valid module: \"%s\"", GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str());
			}
			aabLoaded[iInfosLoad][iLoad] = false;
		}
		else
		{
			pXMLLoadUtility->logMLF("ERROR Vector element: %d, \"%s\", GC.getModLoadControlVectorSize(), You shouldn't have come here!", GC.getModLoadControlInfos(iInfosLoad).getModuleFolder(iLoad).c_str());
			FAssertMsg(aabLoaded[iInfosLoad][iLoad], "Something is wrong with the MLF Array");
		}

		// iInfosLoad is A child of the iInfosParentLoop, we have to check if all the children are loaded now
		// If there are no children to load anymore, we must disable the parent, so it's not looped through on the next run
		int iBreakEndless = 0;	// this is not needed for the code, just a security fix

		// If iDirDepth is 0 we only have 1 MLF anyway..no need to loop and spoil CPU power
		while( iDirDepthTemp > 0 )
		{
			// this code is just a security thing, not important for the actual code..
			iBreakEndless++;
			if ( iBreakEndless >= 20 )	//I assume noone will go 20 depths in dir structure anyway
			{
				FAssertMsg(false, "Something ain't right with the parental MLF disabling function, put a bug report on http://www.worldofcivilization.net/bugtracker/bugtracker.htm, please supply your MLF configuration. All of them!"); 
				break;
			}

			// actual code below
			iDirDepthTemp--;
			// loop through ALL MLF infos...
			for ( int IloopInfosDirDepth = 0; IloopInfosDirDepth < GC.getNumModLoadControlInfos(); IloopInfosDirDepth++)
			{
				// verify all MLF for their dir depth, only 1 higher(dir Depth lower) then the last loaded MLF needs to be checked
				if ( GC.getModLoadControlInfos(IloopInfosDirDepth).getDirDepth() == iDirDepthTemp) 
				{
					bParentDisable = true;  // by default we assume any parent we find must be disabled

					// loop throuhg the modules of every MLF to see if we can find the parent inside this one..
					for ( int IloopInfosModDirectory = 0; IloopInfosModDirectory < GC.getModLoadControlInfos(IloopInfosDirDepth).getNumModules(); IloopInfosModDirectory++)
					{
						// parent module of our latest loaded module?
						if ( CvString::format("%s\\", GC.getModLoadControlInfos(IloopInfosDirDepth).getModuleFolder(IloopInfosModDirectory).c_str()) == GC.getModLoadControlInfos(iInfosLoad).getParentFolder().c_str())
						{							
							//only disable if this is the actual parent

							// Loop through the loaded MLF if everything is now set to false?
							for ( iLoad = 0; iLoad < GC.getModLoadControlInfos(iInfosLoad).getNumModules(); iLoad++)
							{
								// if just ANY dir is still active, don't disable the parent
								if (aabLoaded[iInfosLoad][iLoad])
								{
									bParentDisable = false;
								}
							}
							
							// both conditions are met, we have a Parent, and it has no children left needed to be loaded
							if ( bParentDisable  )	
							{
								aabLoaded[IloopInfosDirDepth][IloopInfosModDirectory] = false;

								// we updated 1 entry, so we have to continue the loop with the new IInfosLoad and iLoad
								iInfosLoad = IloopInfosDirDepth;
								iLoad = IloopInfosModDirectory;
							}
						}
					}
				}
			}

			//break when we just finished checking the top level
			if ( iDirDepthTemp == 0 )
			{
				FAssert(iDirDepthTemp >= 0); //should never reach negative values!!
				break;
			}
		}

		// Check if we must continue
		bContinue = false;		
		//loop through all the MLF's
		for ( int iInfos = 0; iInfos < GC.getNumModLoadControlInfos(); iInfos++)
		{
			//loop through the modules of each MLF
			for ( int iIJ = 0; iIJ < GC.getModLoadControlInfos(iInfos).getNumModules(); iIJ++ )
			{
				// As long as Modules need to be loaded, we continue
				if ( aabLoaded[iInfos][iIJ])
				{
					bContinue = true;
					break;
				}				
			}
			if ( bContinue ) break;
		}
	}
	pXMLLoadUtility->logMLF("Finished the MLF, you will now continue loading regular XML files");		//logging

	SAFE_DELETE_ARRAY(aabLoaded);
	SAFE_DELETE(p_szDirName);
	SAFE_DELETE(pXMLLoadUtility);
}

void CvXMLLoadUtilitySetMod::MLFEnumerateFiles(
					std::vector<CvString>&			aszFiles,
                    const CvString&					refcstrRootDirectory,
					const CvString&					refcstrModularDirectory,
                    const CvString&					refcstrExtension,					
                    bool							bSearchSubdirectories)
{
	CvString		strFilePath;		// Filepath
	CvString		strModPath;			// Modules path
	CvString		strPattern;			// Pattern
	CvString		strExtension;		// Extension
	CvString		compareCStrExtension; //Compare with refcstrExtension
	HANDLE          hFile;				// Handle to file
	WIN32_FIND_DATA FileInformation;	// File information

	CvString szDebugBuffer;

	strPattern = refcstrRootDirectory + "\\*.*";

	
	if (GC.isXMLLogging())
	{
		szDebugBuffer.Format("=== MLFEnumerateFiles begin in root directory %s ===", refcstrRootDirectory.c_str());
		gDLL->logMsg("CvXMLLoadUtilitySetMod_MLFEnumerateFiles.log", szDebugBuffer.c_str());
	}

	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;
				strModPath = refcstrModularDirectory + "\\" + FileInformation.cFileName;
				
				if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if(bSearchSubdirectories)
					{
						if (GC.isXMLLogging())
						{
							szDebugBuffer.Format(" * Search subdirectory %s", strFilePath.c_str());
							gDLL->logMsg("CvXMLLoadUtilitySetMod_MLFEnumerateFiles.log", szDebugBuffer.c_str());
						}

						// Search subdirectory
						MLFEnumerateFiles(aszFiles, strFilePath, strModPath, refcstrExtension);
					}
				}
				else
				{
					// Check extension
					strExtension = FileInformation.cFileName;
					//force lowercase for comparison
					int length = strExtension.size();
					for (int i = 0; i < length; ++i)
					{
						strExtension[i] = tolower(strExtension[i]);
					}
					if ( strExtension.rfind("_civ4") != std::string::npos )
					{
						strExtension = strExtension.substr(strExtension.rfind("_civ4") + 1);
					}
/*	 - old buggy rules, just left here for history reminder
					if ( strExtension.rfind("civ4") != std::string::npos )
					{
						strExtension = strExtension.substr(strExtension.rfind("civ4") + 4);
					}
*/
					else if ( strExtension.rfind("globaldefines") != std::string::npos )
					{
						strExtension = strExtension.substr(strExtension.rfind("globaldefines"));
					}
					else if ( strExtension.rfind("pythoncallbackdefines") != std::string::npos )
					{
						strExtension = strExtension.substr(strExtension.rfind("pythoncallbackdefines"));
					}

					//force lowercase for comparison
					compareCStrExtension = refcstrExtension;
					length = compareCStrExtension.size();
					for (int i = 0; i < length; ++i)
					{
						compareCStrExtension[i] = tolower(compareCStrExtension[i]);
					}
					if ( compareCStrExtension.rfind("_civ4") != std::string::npos )
					{
						compareCStrExtension = compareCStrExtension.substr(compareCStrExtension.rfind("_civ4") + 1);
					}
/*	 - old buggy rules, just left here for history reminder
					if ( compareCStrExtension.rfind("civ4") != std::string::npos )
					{
						compareCStrExtension = compareCStrExtension.substr(compareCStrExtension.rfind("civ4") + 4);
					}
*/
					
					if (!strcmp(strExtension.c_str(),compareCStrExtension.c_str()))
					{
						aszFiles.push_back(strModPath.c_str());						
					}
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if(dwError != ERROR_NO_MORE_FILES)
		{
			FAssertMsg(false, "something wrong");
			return;
		}
	}
	return;
}

bool CvXMLLoadUtilitySetMod::isValidModule(
					bool&							bValid,
                    const CvString&					refcstrRootDirectory,
					const CvString&					refcstrModularDirectory,
                    const CvString&					refcstrExtension,					
                    bool							bSearchSubdirectories)
{
	CvString		strFilePath;		// Filepath
	CvString		strModPath;			// Modules path
	CvString		strPattern;			// Pattern
	CvString		strExtension;		// Extension
	CvString		compareCStrExtension; //Compare with refcstrExtension
	HANDLE          hFile;				// Handle to file
	WIN32_FIND_DATA FileInformation;	// File information

	strPattern = refcstrRootDirectory + "\\*.*";

	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;
				strModPath = refcstrModularDirectory + "\\" + FileInformation.cFileName;
				
				if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if(bSearchSubdirectories)
					{
						// Search subdirectory
						bValid = isValidModule(bValid, strFilePath, strModPath, refcstrExtension);
						if (bValid) break;
					}
				}
				else
				{
					// Check extension
					strExtension = FileInformation.cFileName;
					//force lowercase for comparison
					int length = strExtension.size();
					for (int i = 0; i < length; ++i)
					{
						strExtension[i] = tolower(strExtension[i]);
					}
					//is xml file?
					if ( strExtension.rfind(".xml") != std::string::npos )
					{
						bValid = true;
						break;
					}					
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);		

		// Close handle
		::FindClose(hFile);

		DWORD dwError = ::GetLastError();
		if(!bValid && dwError != ERROR_NO_MORE_FILES)
		{
			FAssertMsg(false, "something wrong");
			return false;
		}

		if ( bValid )
		{
			return true;
		}
	}
	return false;
}

void CvXMLLoadUtilitySetMod::loadModControlArray(std::vector<CvString>& aszFiles, const char* szFileRoot, int i)
{	
	CvString szModDirectory;


/************************************************************************************************/
/* Afforess	                  Start		 06/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	/*
	CvXMLLoadUtilityModTools* p_szDirName = new CvXMLLoadUtilityModTools;
	szModDirectory = p_szDirName->GetProgramDir();		// Dir where the Civ4BeyondSword.exe is started from
	SAFE_DELETE(p_szDirName);
	szModDirectory += gDLL->getModName();		// "Mods\Modname\"
	szModDirectory += "Assets\\";		//Assets in the Moddirectory
	 */

	szModDirectory = GC.getInitCore().getDLLPath() + "\\";
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	


	for (int iI = 0; iI < GC.getModLoadControlVectorSize(); iI++)
	{
		MLFEnumerateFiles(aszFiles, (szModDirectory + GC.getModLoadControlVector(iI).c_str()).c_str(), GC.getModLoadControlVector(iI).c_str(), CvString::format("%s.xml", szFileRoot).c_str());
	}
}