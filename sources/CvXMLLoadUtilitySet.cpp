//
// XML Set functions
//

#include "CvGameCoreDLL.h"
#include "CvDLLXMLIFaceBase.h"
#include "CvXMLLoadUtility.h"
#include "CvGlobals.h"
#include "CvArtFileMgr.h"
#include "CvGameTextMgr.h"
#include <algorithm>
#include "CvInfoWater.h"
#include "FProfiler.h"
#include "FVariableSystem.h"
#include "CvGameCoreUtils.h"
#include "CvInitCore.h"
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 11/15/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#include "CvXMLLoadUtilitySetMod.h"
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/

// Macro for Setting Global Art Defines
#define INIT_XML_GLOBAL_LOAD(xmlInfoPath, infoArray, numInfos)  SetGlobalClassInfo(infoArray, xmlInfoPath, numInfos);

bool CvXMLLoadUtility::ReadGlobalDefines(const TCHAR* szXMLFileName, CvCacheObject* cache)
{
	bool bLoaded = false;	// used to make sure that the xml file was loaded correctly

	if (!gDLL->cacheRead(cache, szXMLFileName))			// src data file name
	{
		// load normally
		if (!CreateFXml())
		{
			return false;
		}

		// load the new FXml variable with the szXMLFileName file
		bLoaded = LoadCivXml(m_pFXml, szXMLFileName);
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "LoadXML call failed for %s \n Current XML file is: %s", szXMLFileName, GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}

		// if the load succeeded we will continue
		if (bLoaded)
		{
			// locate the first define tag in the xml
			if (gDLL->getXMLIFace()->LocateNode(m_pFXml,"Civ4Defines/Define"))
			{
				int i;	// loop counter
				// get the number of other Define tags in the xml file
				int iNumDefines = gDLL->getXMLIFace()->GetNumSiblings(m_pFXml);
				// add one to the total in order to include the current Define tag
				iNumDefines++;

				// loop through all the Define tags
				for (i=0;i<iNumDefines;i++)
				{
					char szNodeType[256];	// holds the type of the current node
					char szName[256];

					// Skip any comments and stop at the next value we might want
					if (SkipToNextVal())
					{
						// call the function that sets the FXml pointer to the first non-comment child of
						// the current tag and gets the value of that new node
						if (GetChildXmlVal(szName))
						{
							// set the FXml pointer to the next sibling of the current tag``
							if (gDLL->getXMLIFace()->NextSibling(GetXML()))
							{
								// Skip any comments and stop at the next value we might want
								if (SkipToNextVal())
								{
									// if we successfuly get the node type for the current tag
									if (gDLL->getXMLIFace()->GetLastLocatedNodeType(GetXML(),szNodeType))
									{
										// if the node type of the current tag isn't null
										if (strcmp(szNodeType,"")!=0)
										{
											// if the node type of the current tag is a float then
											if (strcmp(szNodeType,"float")==0)
											{
												// get the float value for the define
												float fVal;
												GetXmlVal(&fVal);
												GC.getDefinesVarSystem()->SetValue(szName, fVal);
											}
											// else if the node type of the current tag is an int then
											else if (strcmp(szNodeType,"int")==0)
											{
												// get the int value for the define
												int iVal;
												GetXmlVal(&iVal);
												GC.getDefinesVarSystem()->SetValue(szName, iVal);
											}
											// else if the node type of the current tag is a boolean then
											else if (strcmp(szNodeType,"boolean")==0)
											{
												// get the boolean value for the define
												bool bVal;
												GetXmlVal(&bVal);
												GC.getDefinesVarSystem()->SetValue(szName, bVal);
											}
											// otherwise we will assume it is a string/text value
											else
											{
												char szVal[2048];
												// get the string/text value for the define
												GetXmlVal(szVal);
												GC.getDefinesVarSystem()->SetValue(szName, szVal);
											}
										}
										// otherwise we will default to getting the string/text value for the define
										else
										{
											char szVal[2048];
											// get the string/text value for the define
											GetXmlVal(szVal);
											GC.getDefinesVarSystem()->SetValue(szName, szVal);
										}
									}
								}
							}

							// since we are looking at the children of a Define tag we will need to go up
							// one level so that we can go to the next Define tag.
							// Set the FXml pointer to the parent of the current tag
							gDLL->getXMLIFace()->SetToParent(GetXML());
						}
					}

					// now we set the FXml pointer to the sibling of the current tag, which should be the next
					// Define tag
					if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
					{
						break;
					}
				}

				// write global defines info to cache
				bool bOk = gDLL->cacheWrite(cache);
				if (!bOk)
				{
					char	szMessage[1024];
					sprintf( szMessage, "Failed writing to global defines cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Caching Error");
				}
				else
				{
					logMsg("Wrote GlobalDefines to cache");
				}
			}
		}

		// delete the pointer to the FXml variable
		gDLL->getXMLIFace()->DestroyFXml(m_pFXml);
	}
	else
	{
		logMsg("Read GobalDefines from cache");
	}

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalDefines()
//
//  PURPOSE :   Initialize the variables located in globaldefines.cpp/h with the values in
//				GlobalDefines.xml
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::SetGlobalDefines()
{
	UpdateProgressCB("GlobalDefines");

	/////////////////////////////////
	//
	// use disk cache if possible.
	// if no cache or cache is older than xml file, use xml file like normal, else read from cache
	//

	CvCacheObject* cache = gDLL->createGlobalDefinesCacheObject("GlobalDefines.dat");	// cache file name
/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   03/14/08                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#ifdef _DEBUG
	logXmlCheckDoubleTypes("\nEntering: GlobalDefines\n");
#endif
/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   END                                                  */
/************************************************************************************************/

	if (!ReadGlobalDefines("xml\\GlobalDefines.xml", cache))
	{
		return false;
	}

	if (!ReadGlobalDefines("xml\\GlobalDefinesAlt.xml", cache))
	{
		return false;
	}

	if (!ReadGlobalDefines("xml\\PythonCallbackDefines.xml", cache))
	{
		return false;
	}

	//Affores
	if (!ReadGlobalDefines("xml\\A_New_Dawn_GlobalDefines.xml", cache))
	{
		FAssertMsg(false,"A_New_Dawn_GlobalDefines Failed to load!");
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* XML Options                                                                                  */
/************************************************************************************************/
	if (!ReadGlobalDefines("xml\\BBAI_Game_Options_GlobalDefines.xml", cache))
	{
		FAssertMsg(false,"BBAI_Game_Options_GlobalDefines Failed to load!");
	}

	if (!ReadGlobalDefines("xml\\BBAI_AI_Variables_GlobalDefines.xml", cache))
	{
		FAssertMsg(false,"BBAI_AI_Variables_GlobalDefines Failed to load!");
	}

	if (!ReadGlobalDefines("xml\\TechDiffusion_GlobalDefines.xml", cache))
	{
		FAssertMsg(false,"TechDiffusion_GlobalDefines Failed to load!");
	}

	// if (!ReadGlobalDefines("xml\\LeadFromBehind_GlobalDefines.xml", cache))
	// {
		// FAssertMsg(false,"A_New_Dawn_GlobalDefines Failed to load!");
	// }
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* TGA_INDEXATION                          02/19/08                                MRGENIE      */
/*                                                                                              */
/* reading the Defines to know the modded TGA icons                                             */
/************************************************************************************************/
	if (!ReadGlobalDefines("res\\Fonts\\GameFont_GlobalDefines.xml", cache))
	{
		FAssertMsg(false,"The \"GameFont_GlobalDefines.xml\" must reside in the \"Mods\\World of Civilization\\Assets\\res\\Fonts\" directory next to the 2 GameFont.tga files");
	}
/************************************************************************************************/
/* TGA_INDEXATION                          END                                                  */
/************************************************************************************************/

	if (gDLL->isModularXMLLoading())
	{
		std::vector<CvString> aszFiles;
		gDLL->enumerateFiles(aszFiles, "modules\\*_GlobalDefines.xml");

		for (std::vector<CvString>::iterator it = aszFiles.begin(); it != aszFiles.end(); ++it)
		{
			if (!ReadGlobalDefines(*it, cache))
			{
				return false;
			}
		}

		std::vector<CvString> aszModularFiles;
		gDLL->enumerateFiles(aszModularFiles, "modules\\*_PythonCallbackDefines.xml");

		for (std::vector<CvString>::iterator it = aszModularFiles.begin(); it != aszModularFiles.end(); ++it)
		{
			if (!ReadGlobalDefines(*it, cache))
			{
				return false;
			}
		}
	}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 11/15/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	else
	{
		std::vector<CvString> aszFiles;		
		CvXMLLoadUtilitySetMod* pModEnumVector = new CvXMLLoadUtilitySetMod;
		aszFiles.reserve(10000);
		pModEnumVector->loadModControlArray(aszFiles, "globaldefines");

		for (std::vector<CvString>::iterator it = aszFiles.begin(); it != aszFiles.end(); ++it)
		{
			if (!ReadGlobalDefines(*it, cache))
			{
				return false;
			}
		}

		std::vector<CvString> aszModularFiles;
		aszModularFiles.reserve(10000);
		pModEnumVector->loadModControlArray(aszModularFiles, "pythoncallbackdefines");		

		for (std::vector<CvString>::iterator it = aszModularFiles.begin(); it != aszModularFiles.end(); ++it)
		{
			if (!ReadGlobalDefines(*it, cache))
			{
				return false;
			}

#ifdef GRANULAR_CALLBACK_CONTROL
			//	KOSHLING - granular control over Python callbacks - so far implements
			//		CanTrain
			//		CannotTrain
			const char* entityList;
			//	CanTrain
			if ( GC.getDefinesVarSystem()->GetValue("USE_CAN_TRAIN_CALLBACK_GRANULAR", entityList) )
			{
				cvInternalGlobals::getInstance().m_pythonCallbackController.RegisterUnitCallback(CALLBACK_TYPE_CAN_TRAIN, entityList);

				GC.getDefinesVarSystem()->RemValue("USE_CAN_TRAIN_CALLBACK_GRANULAR");
			}
			//	CannotTrain
			if ( GC.getDefinesVarSystem()->GetValue("USE_CANNOT_TRAIN_CALLBACK_GRANULAR", entityList) )
			{
				cvInternalGlobals::getInstance().m_pythonCallbackController.RegisterUnitCallback(CALLBACK_TYPE_CANNOT_TRAIN, entityList);

				GC.getDefinesVarSystem()->RemValue("USE_CANNOT_TRAIN_CALLBACK_GRANULAR");
			}
			//	CanBuild
			if ( GC.getDefinesVarSystem()->GetValue("USE_CAN_BUILD_CALLBACK_GRANULAR", entityList) )
			{
				cvInternalGlobals::getInstance().m_pythonCallbackController.RegisterBuildCallback(CALLBACK_TYPE_CAN_BUILD, entityList);

				GC.getDefinesVarSystem()->RemValue("USE_CAN_BUILD_CALLBACK_GRANULAR");
			}
#endif
		}
		aszFiles.clear();
		aszModularFiles.clear();
		SAFE_DELETE(pModEnumVector);
	}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/


	gDLL->destroyCache(cache);
	////////////////////////////////////////////////////////////////////////

	GC.cacheGlobals();

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetPostGlobalsGlobalDefines()
//
//  PURPOSE :   This function assumes that the SetGlobalDefines function has already been called
//							it then loads the few global defines that needed to reference a global variable that
//							hadn't been loaded in prior to the SetGlobalDefines call
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::SetPostGlobalsGlobalDefines()
{
	const char* szVal=NULL;		// holds the string value from the define queue
	int idx;

	if (GC.getDefinesVarSystem()->GetSize() > 0)
	{
		SetGlobalDefine("LAND_TERRAIN", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("LAND_TERRAIN", idx);

		SetGlobalDefine("DEEP_WATER_TERRAIN", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("DEEP_WATER_TERRAIN", idx);

		SetGlobalDefine("SHALLOW_WATER_TERRAIN", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("SHALLOW_WATER_TERRAIN", idx);
//GWMod Start M.A.
		SetGlobalDefine("FROZEN_TERRAIN", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("FROZEN_TERRAIN", idx);
		
		SetGlobalDefine("COLD_TERRAIN", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("COLD_TERRAIN", idx);
		
		SetGlobalDefine("TEMPERATE_TERRAIN", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("TEMPERATE_TERRAIN", idx);
		
		SetGlobalDefine("DRY_TERRAIN", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("DRY_TERRAIN", idx);
		
		SetGlobalDefine("BARREN_TERRAIN", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("BARREN_TERRAIN", idx);

		SetGlobalDefine("COLD_FEATURE", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("COLD_FEATURE", idx);

		SetGlobalDefine("TEMPERATE_FEATURE", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("TEMPERATE_FEATURE", idx);

		SetGlobalDefine("WARM_FEATURE", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("WARM_FEATURE", idx);
		
		SetGlobalDefine("MARSH_TERRAIN", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("MARSH_TERRAIN", idx);

//GWMod end M.A.
		SetGlobalDefine("LAND_IMPROVEMENT", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("LAND_IMPROVEMENT", idx);

		SetGlobalDefine("WATER_IMPROVEMENT", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("WATER_IMPROVEMENT", idx);

		SetGlobalDefine("RUINS_IMPROVEMENT", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("RUINS_IMPROVEMENT", idx);

		SetGlobalDefine("NUKE_FEATURE", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("NUKE_FEATURE", idx);

		SetGlobalDefine("GLOBAL_WARMING_TERRAIN", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("GLOBAL_WARMING_TERRAIN", idx);

		SetGlobalDefine("CAPITAL_BUILDINGCLASS", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("CAPITAL_BUILDINGCLASS", idx);

		SetGlobalDefine("DEFAULT_SPECIALIST", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("DEFAULT_SPECIALIST", idx);

		SetGlobalDefine("INITIAL_CITY_ROUTE_TYPE", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("INITIAL_CITY_ROUTE_TYPE", idx);

		SetGlobalDefine("STANDARD_HANDICAP", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("STANDARD_HANDICAP", idx);

		SetGlobalDefine("STANDARD_HANDICAP_QUICK", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("STANDARD_HANDICAP_QUICK", idx);

		SetGlobalDefine("STANDARD_GAMESPEED", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("STANDARD_GAMESPEED", idx);

		SetGlobalDefine("STANDARD_TURNTIMER", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("STANDARD_TURNTIMER", idx);

		SetGlobalDefine("STANDARD_CLIMATE", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("STANDARD_CLIMATE", idx);

		SetGlobalDefine("STANDARD_SEALEVEL", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("STANDARD_SEALEVEL", idx);

		SetGlobalDefine("STANDARD_ERA", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("STANDARD_ERA", idx);

		SetGlobalDefine("STANDARD_CALENDAR", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("STANDARD_CALENDAR", idx);

		SetGlobalDefine("AI_HANDICAP", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("AI_HANDICAP", idx);

		SetGlobalDefine("BARBARIAN_HANDICAP", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("BARBARIAN_HANDICAP", idx);

		SetGlobalDefine("BARBARIAN_CIVILIZATION", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("BARBARIAN_CIVILIZATION", idx);

		SetGlobalDefine("BARBARIAN_LEADER", szVal);
		idx = FindInInfoClass(szVal);
		GC.getDefinesVarSystem()->SetValue("BARBARIAN_LEADER", idx);
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	gDLL->ChangeINIKeyValue("CONFIG", "HideMinSpecWarning ", "1");
	gDLL->ChangeINIKeyValue("GAME", "ModularLoading  ", "0");
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		return true;
	}

	char	szMessage[1024];
	sprintf( szMessage, "Size of Global Defines is not greater than 0. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
	gDLL->MessageBox(szMessage, "XML Load Error");

	return false;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalTypes()
//
//  PURPOSE :   Initialize the variables located in globaltypes.cpp/h with the values in
//				GlobalTypes.xml
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::SetGlobalTypes()
{
	UpdateProgressCB("GlobalTypes");

	bool bLoaded = false;	// used to make sure that the xml file was loaded correctly
/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   03/14/08                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#ifdef _DEBUG	
	logXmlCheckDoubleTypes("\nEntering: GlobalTypes\n");
#endif
/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   END                                                  */
/************************************************************************************************/
	if (!CreateFXml())
	{
		return false;
	}

	// load the new FXml variable with the GlobalTypes.xml file
	bLoaded = LoadCivXml(m_pFXml, "xml/GlobalTypes.xml");
	if (!bLoaded)
	{
		char	szMessage[1024];
		sprintf( szMessage, "LoadXML call failed for GlobalTypes.xml. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Load Error");
	}

/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   03/14/08                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	// Debugging dependencies problems - BEGIN
#ifdef _DEBUG
	GC.logInfoTypeMap("SetGlobalTypes PRE");
#endif
	// Debugging dependencies problems - END
/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   END                                                  */
/************************************************************************************************/

	// if the load succeeded we will continue
	if (bLoaded)
	{
		SetGlobalStringArray(&GC.getAnimationOperatorTypes(), "Civ4Types/AnimationOperatorTypes/AnimationOperatorType", &GC.getNumAnimationOperatorTypes());
		int iEnumVal = NUM_FUNC_TYPES;
		SetGlobalStringArray(&GC.getFunctionTypes(), "Civ4Types/FunctionTypes/FunctionType", &iEnumVal, true);
		SetGlobalStringArray(&GC.getFlavorTypes(), "Civ4Types/FlavorTypes/FlavorType", &GC.getNumFlavorTypes());
		SetGlobalStringArray(&GC.getArtStyleTypes(), "Civ4Types/ArtStyleTypes/ArtStyleType", &GC.getNumArtStyleTypes());
		SetGlobalStringArray(&GC.getCitySizeTypes(), "Civ4Types/CitySizeTypes/CitySizeType", &GC.getNumCitySizeTypes());
		iEnumVal = NUM_CONTACT_TYPES;
		SetGlobalStringArray(&GC.getContactTypes(), "Civ4Types/ContactTypes/ContactType", &iEnumVal, true);
		iEnumVal = NUM_DIPLOMACYPOWER_TYPES;
		SetGlobalStringArray(&GC.getDiplomacyPowerTypes(), "Civ4Types/DiplomacyPowerTypes/DiplomacyPowerType", &iEnumVal, true);
		iEnumVal = NUM_AUTOMATE_TYPES;
		SetGlobalStringArray(&GC.getAutomateTypes(), "Civ4Types/AutomateTypes/AutomateType", &iEnumVal, true);
		iEnumVal = NUM_DIRECTION_TYPES;
		SetGlobalStringArray(&GC.getDirectionTypes(), "Civ4Types/DirectionTypes/DirectionType", &iEnumVal, true);
		SetGlobalStringArray(&GC.getFootstepAudioTypes(), "Civ4Types/FootstepAudioTypes/FootstepAudioType", &GC.getNumFootstepAudioTypes());

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		SetVariableListTagPair(&GC.getFootstepAudioTags(), "FootstepAudioTags", GC.getFootstepAudioTypes(), GC.getNumFootstepAudioTypes(), "");
	}

/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   03/14/08                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	// Debugging dependencies problems - BEGIN
#ifdef _DEBUG
	GC.logInfoTypeMap("SetGlobalTypes POST");
#endif
	// Debugging dependencies problems - END
/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   END                                                  */
/************************************************************************************************/
	// delete the pointer to the FXml variable
	DestroyFXml();

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetDiplomacyCommentTypes()
//
//  PURPOSE :   Creates a full list of Diplomacy Comments
//
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetDiplomacyCommentTypes(CvString** ppszString, int* iNumVals)
{
	FAssertMsg(false, "should never get here");
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetupGlobalLandscapeInfos()
//
//  PURPOSE :   Initialize the appropriate variables located in globals.cpp/h with the values in
//				Terrain\Civ4TerrainSettings.xml
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::SetupGlobalLandscapeInfo()
{
	if (!CreateFXml())
	{
		return false;
	}

	LoadGlobalClassInfo(GC.getLandscapeInfo(), "CIV4TerrainSettings", "Terrain", "Civ4TerrainSettings/LandscapeInfos/LandscapeInfo", false);

	// delete the pointer to the FXml variable
	DestroyFXml();

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalArtDefines()
//
//  PURPOSE :   Initialize the appropriate variables located in globals.cpp/h with the values in
//				Civ4ArtDefines.xml
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::SetGlobalArtDefines()
{
	if (!CreateFXml())
	{
		return false;
	}

	LoadGlobalClassInfo(ARTFILEMGR.getInterfaceArtInfo(), "CIV4ArtDefines_Interface", "Art", "Civ4ArtDefines/InterfaceArtInfos/InterfaceArtInfo", false);
	LoadGlobalClassInfo(ARTFILEMGR.getMovieArtInfo(), "CIV4ArtDefines_Movie", "Art", "Civ4ArtDefines/MovieArtInfos/MovieArtInfo", false);
	LoadGlobalClassInfo(ARTFILEMGR.getMiscArtInfo(), "CIV4ArtDefines_Misc", "Art", "Civ4ArtDefines/MiscArtInfos/MiscArtInfo", false);
	LoadGlobalClassInfo(ARTFILEMGR.getUnitArtInfo(), "CIV4ArtDefines_Unit", "Art", "Civ4ArtDefines/UnitArtInfos/UnitArtInfo", false);
	LoadGlobalClassInfo(ARTFILEMGR.getBuildingArtInfo(), "CIV4ArtDefines_Building", "Art", "Civ4ArtDefines/BuildingArtInfos/BuildingArtInfo", false);
	LoadGlobalClassInfo(ARTFILEMGR.getCivilizationArtInfo(), "CIV4ArtDefines_Civilization", "Art", "Civ4ArtDefines/CivilizationArtInfos/CivilizationArtInfo", false);
	LoadGlobalClassInfo(ARTFILEMGR.getLeaderheadArtInfo(), "CIV4ArtDefines_Leaderhead", "Art", "Civ4ArtDefines/LeaderheadArtInfos/LeaderheadArtInfo", false);
	LoadGlobalClassInfo(ARTFILEMGR.getBonusArtInfo(), "CIV4ArtDefines_Bonus", "Art", "Civ4ArtDefines/BonusArtInfos/BonusArtInfo", false);
	LoadGlobalClassInfo(ARTFILEMGR.getImprovementArtInfo(), "CIV4ArtDefines_Improvement", "Art", "Civ4ArtDefines/ImprovementArtInfos/ImprovementArtInfo", false);
	LoadGlobalClassInfo(ARTFILEMGR.getTerrainArtInfo(), "CIV4ArtDefines_Terrain", "Art", "Civ4ArtDefines/TerrainArtInfos/TerrainArtInfo", false);
	LoadGlobalClassInfo(ARTFILEMGR.getFeatureArtInfo(), "CIV4ArtDefines_Feature", "Art", "Civ4ArtDefines/FeatureArtInfos/FeatureArtInfo", false);

	DestroyFXml();

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalText()
//
//  PURPOSE :   Handles all Global Text Infos
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::LoadGlobalText()
{
	CvCacheObject* cache = gDLL->createGlobalTextCacheObject("GlobalText.dat");	// cache file name
	if (!gDLL->cacheRead(cache))
	{
		bool bLoaded = false;
/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   03/14/08                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#ifdef _DEBUG	
	logXmlCheckDoubleTypes("\nEntering: GameText\n");
#endif
/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   END                                                  */
/************************************************************************************************/

		if (!CreateFXml())
		{
			return false;
		}

		//
		// load all files in the xml text directory
		//
		std::vector<CvString> aszFiles;
		std::vector<CvString> aszModfiles;

		gDLL->enumerateFiles(aszFiles, "xml\\text\\*.xml");

/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 05/23/08                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/* original firaxis
		if (gDLL->isModularXMLLoading())
		{
			gDLL->enumerateFiles(aszModfiles, "modules\\*_CIV4GameText.xml");
			aszFiles.insert(aszFiles.end(), aszModfiles.begin(), aszModfiles.end());
		}
*/
		gDLL->enumerateFiles(aszModfiles, "modules\\*_CIV4GameText*.xml");
		aszFiles.insert(aszFiles.end(), aszModfiles.begin(), aszModfiles.end());
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/

		for(std::vector<CvString>::iterator it = aszFiles.begin(); it != aszFiles.end(); ++it)
		{
			bLoaded = LoadCivXml(m_pFXml, *it); // Load the XML
			if (!bLoaded)
			{
				char	szMessage[1024];
				sprintf( szMessage, "LoadXML call failed for %s. \n Current XML file is: %s", (*it).c_str(), GC.getCurrentXMLFile().GetCString());
				gDLL->MessageBox(szMessage, "XML Load Error");
			}
			if (bLoaded)
			{
				// if the xml is successfully validated
				SetGameText("Civ4GameText", "Civ4GameText/TEXT");
			}
		}

		DestroyFXml();

		// write global text info to cache
		bool bOk = gDLL->cacheWrite(cache);
		if (!bLoaded)
		{
			char	szMessage[1024];
			sprintf( szMessage, "Failed writing to Global Text cache. \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Caching Error");
		}
		if (bOk)
		{
			logMsg("Wrote GlobalText to cache");
		}
	}	// didn't read from cache
	else
	{
		logMsg("Read GlobalText from cache");
	}

	gDLL->destroyCache(cache);

	return true;
}

bool CvXMLLoadUtility::LoadBasicInfos()
{
	if (!CreateFXml())
	{
		return false;
	}

	LoadGlobalClassInfo(GC.getConceptInfo(), "CIV4BasicInfos", "BasicInfos", "Civ4BasicInfos/ConceptInfos/ConceptInfo", false);
	LoadGlobalClassInfo(GC.getNewConceptInfo(), "CIV4NewConceptInfos", "BasicInfos", "Civ4NewConceptInfos/NewConceptInfos/NewConceptInfo", false);
	// Dale - DCM: Pedia Concepts START
	LoadGlobalClassInfo(GC.getDCMConceptInfo(), "CIV4DCMConceptInfos", "BasicInfos", "Civ4DCMConceptInfos/DCMConceptInfos/DCMConceptInfo", false);
	// Dale - DCM: Pedia Concepts END
	// Afforess - A New Dawn Concepts
	LoadGlobalClassInfo(GC.getANDConceptInfo(), "CIV4ANDConceptInfos", "BasicInfos", "Civ4ANDConceptInfos/ANDConceptInfos/ANDConceptInfo", false);
	// Afforess
	LoadGlobalClassInfo(GC.getCityTabInfo(), "CIV4CityTabInfos", "BasicInfos", "Civ4CityTabInfos/CityTabInfos/CityTabInfo", false);
	LoadGlobalClassInfo(GC.getCalendarInfo(), "CIV4CalendarInfos", "BasicInfos", "Civ4CalendarInfos/CalendarInfos/CalendarInfo", false);
	LoadGlobalClassInfo(GC.getSeasonInfo(), "CIV4SeasonInfos", "BasicInfos", "Civ4SeasonInfos/SeasonInfos/SeasonInfo", false);
	LoadGlobalClassInfo(GC.getMonthInfo(), "CIV4MonthInfos", "BasicInfos", "Civ4MonthInfos/MonthInfos/MonthInfo", false);
	LoadGlobalClassInfo(GC.getDenialInfo(), "CIV4DenialInfos", "BasicInfos", "Civ4DenialInfos/DenialInfos/DenialInfo", false);
	LoadGlobalClassInfo(GC.getInvisibleInfo(), "CIV4InvisibleInfos", "BasicInfos", "Civ4InvisibleInfos/InvisibleInfos/InvisibleInfo", false);
	LoadGlobalClassInfo(GC.getUnitCombatInfo(), "CIV4UnitCombatInfos", "BasicInfos", "Civ4UnitCombatInfos/UnitCombatInfos/UnitCombatInfo", false);
	LoadGlobalClassInfo(GC.getDomainInfo(), "CIV4DomainInfos", "BasicInfos", "Civ4DomainInfos/DomainInfos/DomainInfo", false);
	//	Koshling - replaced XML-based registration of UNITAI types with internal registration.  Since they are a DLL-defined enum
	//	anyway this allows new UNITAIs to be defined freely without ordering issues (in the XML or DLL), which in turn makes it
	//	easier to merge mods with different UNITAI changes.  The XML is retained purely for documentary purposes
	GC.registerUnitAIs();
	GC.registerAIScales();
	GC.registerGameObjects();
	//LoadGlobalClassInfo(GC.getUnitAIInfo(), "CIV4UnitAIInfos", "BasicInfos", "Civ4UnitAIInfos/UnitAIInfos/UnitAIInfo", false);
	LoadGlobalClassInfo(GC.getAttitudeInfo(), "CIV4AttitudeInfos", "BasicInfos", "Civ4AttitudeInfos/AttitudeInfos/AttitudeInfo", false);
	LoadGlobalClassInfo(GC.getMemoryInfo(), "CIV4MemoryInfos", "BasicInfos", "Civ4MemoryInfos/MemoryInfos/MemoryInfo", false);

	DestroyFXml();
	return true;
}

//
// Globals which must be loaded before the main menus.
// Don't put anything in here unless it has to be loaded before the main menus,
// instead try to load things in LoadPostMenuGlobals()
//
bool CvXMLLoadUtility::LoadPreMenuGlobals()
{
	if (!CreateFXml())
	{
		return false;
	}

	LoadGlobalClassInfo(GC.getGameSpeedInfo(), "CIV4GameSpeedInfo", "GameInfo", "Civ4GameSpeedInfo/GameSpeedInfos/GameSpeedInfo", false);
/************************************************************************************************/
/* REVDCM                                 02/16/10                                phungus420    */
/*                                                                                              */
/* CanTrain                                                                                     */
/************************************************************************************************/
	LoadGlobalClassInfo(GC.getGameOptionInfo(), "CIV4GameOptionInfos", "GameInfo", "Civ4GameOptionInfos/GameOptionInfos/GameOptionInfo", false);
/************************************************************************************************/
/* REVDCM                                  END                                                  */
/************************************************************************************************/
	LoadGlobalClassInfo(GC.getTurnTimerInfo(), "CIV4TurnTimerInfo", "GameInfo", "Civ4TurnTimerInfo/TurnTimerInfos/TurnTimerInfo", false);
	LoadGlobalClassInfo(GC.getWorldInfo(), "CIV4WorldInfo", "GameInfo", "Civ4WorldInfo/WorldInfos/WorldInfo", false);
	LoadGlobalClassInfo(GC.getClimateInfo(), "CIV4ClimateInfo", "GameInfo", "Civ4ClimateInfo/ClimateInfos/ClimateInfo", false);
	LoadGlobalClassInfo(GC.getSeaLevelInfo(), "CIV4SeaLevelInfo", "GameInfo", "Civ4SeaLevelInfo/SeaLevelInfos/SeaLevelInfo", false);
	LoadGlobalClassInfo(GC.getAdvisorInfo(), "CIV4AdvisorInfos", "Interface", "Civ4AdvisorInfos/AdvisorInfos/AdvisorInfo", false);
	LoadGlobalClassInfo(GC.getTerrainInfo(), "CIV4TerrainInfos", "Terrain", "Civ4TerrainInfos/TerrainInfos/TerrainInfo", false);
	LoadGlobalClassInfo(GC.getEraInfo(), "CIV4EraInfos", "GameInfo", "Civ4EraInfos/EraInfos/EraInfo", false);
	LoadGlobalClassInfo(GC.getCulturalAgeInfo(), "CIV4CulturalAgeInfos", "GameInfo", "Civ4CulturalAgeInfos/CulturalAgeInfos/CulturalAgeInfo", true);
	LoadGlobalClassInfo(GC.getPropertyInfo(), "CIV4PropertyInfos", "GameInfo", "Civ4PropertyInfos/PropertyInfos/PropertyInfo", false);
	LoadGlobalClassInfo(GC.getUnitClassInfo(), "CIV4UnitClassInfos", "Units", "Civ4UnitClassInfos/UnitClassInfos/UnitClassInfo", false);
	LoadGlobalClassInfo(GC.getSpecialistInfo(), "CIV4SpecialistInfos", "GameInfo", "Civ4SpecialistInfos/SpecialistInfos/SpecialistInfo", false);
	LoadGlobalClassInfo(GC.getVoteSourceInfo(), "CIV4VoteSourceInfos", "GameInfo", "Civ4VoteSourceInfos/VoteSourceInfos/VoteSourceInfo", false);
	LoadGlobalClassInfo(GC.getTechInfo(), "CIV4TechInfos", "Technologies", "Civ4TechInfos/TechInfos/TechInfo", true, &CvDLLUtilityIFaceBase::createTechInfoCacheObject);
	LoadGlobalClassInfo(GC.getFeatureInfo(), "Civ4FeatureInfos", "Terrain", "Civ4FeatureInfos/FeatureInfos/FeatureInfo", false);
	LoadGlobalClassInfo(GC.getReligionInfo(), "CIV4ReligionInfo", "GameInfo", "Civ4ReligionInfo/ReligionInfos/ReligionInfo", false);
/************************************************************************************************/
/* TGA_INDEXATION                          03/25/08                                MRGENIE      */
/*                                                                                              */
/* reorganizing the Religion vector by checking the TGAIndex of the xml and placing everything  */
/* properly                                                                                     */
/************************************************************************************************/
	ArrangeTGA(GC.getReligionInfo(), "CIV4ReligionInfo");

/************************************************************************************************/
/* TGA_INDEXATION                          END                                                  */
/************************************************************************************************/
	LoadGlobalClassInfo(GC.getAnimationCategoryInfo(), "CIV4AnimationInfos", "Units", "Civ4AnimationInfos/AnimationCategories/AnimationCategory", false);
	LoadGlobalClassInfo(GC.getAnimationPathInfo(), "CIV4AnimationPathInfos", "Units", "Civ4AnimationPathInfos/AnimationPaths/AnimationPath", false);
	LoadGlobalClassInfo(GC.getPromotionInfo(), "CIV4PromotionInfos", "Units", "Civ4PromotionInfos/PromotionInfos/PromotionInfo", true, &CvDLLUtilityIFaceBase::createPromotionInfoCacheObject);
	LoadGlobalClassInfo(GC.getTraitInfo(), "CIV4TraitInfos", "Civilizations", "Civ4TraitInfos/TraitInfos/TraitInfo", false);
	LoadGlobalClassInfo(GC.getGoodyInfo(), "CIV4GoodyInfo", "GameInfo", "Civ4GoodyInfo/GoodyInfos/GoodyInfo", false);
	LoadGlobalClassInfo(GC.getHandicapInfo(), "CIV4HandicapInfo", "GameInfo", "Civ4HandicapInfo/HandicapInfos/HandicapInfo", false, &CvDLLUtilityIFaceBase::createHandicapInfoCacheObject);
	LoadGlobalClassInfo(GC.getCursorInfo(), "CIV4CursorInfo", "GameInfo", "Civ4CursorInfo/CursorInfos/CursorInfo", false);
	LoadGlobalClassInfo(GC.getCivicOptionInfo(), "CIV4CivicOptionInfos", "GameInfo", "Civ4CivicOptionInfos/CivicOptionInfos/CivicOptionInfo", false);
	LoadGlobalClassInfo(GC.getUpkeepInfo(), "CIV4UpkeepInfo", "GameInfo", "Civ4UpkeepInfo/UpkeepInfos/UpkeepInfo", false);
	LoadGlobalClassInfo(GC.getHurryInfo(), "CIV4HurryInfo", "GameInfo", "Civ4HurryInfo/HurryInfos/HurryInfo", false);
	LoadGlobalClassInfo(GC.getSpecialBuildingInfo(), "CIV4SpecialBuildingInfos", "Buildings", "Civ4SpecialBuildingInfos/SpecialBuildingInfos/SpecialBuildingInfo", false);
	LoadGlobalClassInfo(GC.getCultureLevelInfo(), "CIV4CultureLevelInfo", "GameInfo", "Civ4CultureLevelInfo/CultureLevelInfos/CultureLevelInfo", false);
	LoadGlobalClassInfo(GC.getBonusClassInfo(), "CIV4BonusClassInfos", "Terrain", "Civ4BonusClassInfos/BonusClassInfos/BonusClassInfo", false);
	LoadGlobalClassInfo(GC.getVictoryInfo(), "CIV4VictoryInfo", "GameInfo", "Civ4VictoryInfo/VictoryInfos/VictoryInfo", false);
	LoadGlobalClassInfo(GC.getBonusInfo(), "CIV4BonusInfos", "Terrain", "Civ4BonusInfos/BonusInfos/BonusInfo", false, &CvDLLUtilityIFaceBase::createBonusInfoCacheObject);
	LoadGlobalClassInfo(GC.getCorporationInfo(), "CIV4CorporationInfo", "GameInfo", "Civ4CorporationInfo/CorporationInfos/CorporationInfo", false);
/************************************************************************************************/
/* TGA_INDEXATION                          03/25/08                                MRGENIE      */
/*                                                                                              */
/* reorganizing the Corporations vector by checking the TGAIndex of the xml and placing         */
/* everything properly                                                                          */
/************************************************************************************************/
	ArrangeTGA(GC.getCorporationInfo(), "CIV4CorporationInfo");
/************************************************************************************************/
/* TGA_INDEXATION                          END                                                  */
/************************************************************************************************/
	LoadGlobalClassInfo(GC.getRouteInfo(), "Civ4RouteInfos", "Misc", "Civ4RouteInfos/RouteInfos/RouteInfo", false);
	LoadGlobalClassInfo(GC.getImprovementInfo(), "CIV4ImprovementInfos", "Terrain", "Civ4ImprovementInfos/ImprovementInfos/ImprovementInfo", true, &CvDLLUtilityIFaceBase::createImprovementInfoCacheObject);
	LoadGlobalClassInfo(GC.getBuildingClassInfo(), "CIV4BuildingClassInfos", "Buildings", "Civ4BuildingClassInfos/BuildingClassInfos/BuildingClassInfo", false);
	LoadGlobalClassInfo(GC.getBuildingInfo(), "CIV4BuildingInfos", "Buildings", "Civ4BuildingInfos/BuildingInfos/BuildingInfo", false, &CvDLLUtilityIFaceBase::createBuildingInfoCacheObject);
	for (int i=0; i < GC.getNumBuildingClassInfos(); ++i)
	{
		GC.getBuildingClassInfo((BuildingClassTypes)i).readPass3();
	}
	LoadGlobalClassInfo(GC.getOutcomeInfo(), "CIV4OutcomeInfos", "GameInfo", "Civ4OutcomeInfos/OutcomeInfos/OutcomeInfo", true);
	LoadGlobalClassInfo(GC.getSpecialUnitInfo(), "CIV4SpecialUnitInfos", "Units", "Civ4SpecialUnitInfos/SpecialUnitInfos/SpecialUnitInfo", false);
/************************************************************************************************/
/* Afforess	                  Start		 07/30/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	LoadGlobalClassInfo(GC.getProjectInfo(), "CIV4ProjectInfo", "GameInfo", "Civ4ProjectInfo/ProjectInfos/ProjectInfo", false);
/************************************************************************************************/
/*                                         END                                                  */
/************************************************************************************************/
	LoadGlobalClassInfo(GC.getCivicInfo(), "CIV4CivicInfos", "GameInfo", "Civ4CivicInfos/CivicInfos/CivicInfo", false, &CvDLLUtilityIFaceBase::createCivicInfoCacheObject);
	for (int i=0; i < GC.getNumVoteSourceInfos(); ++i)
	{
		GC.getVoteSourceInfo((VoteSourceTypes)i).readPass3();
	}
	LoadGlobalClassInfo(GC.getLeaderHeadInfo(), "CIV4LeaderHeadInfos", "Civilizations", "Civ4LeaderHeadInfos/LeaderHeadInfos/LeaderHeadInfo", false, &CvDLLUtilityIFaceBase::createLeaderHeadInfoCacheObject);
/************************************************************************************************/
/* SORT_ALPHABET                           11/17/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	SortAlphabetGlobalClassInfo(GC.getLeaderHeadInfo());
/************************************************************************************************/
/* SORT_ALPHABET                           END                                                  */
/************************************************************************************************/
	LoadGlobalClassInfo(GC.getColorInfo(), "CIV4ColorVals", "Interface", "Civ4ColorVals/ColorVals/ColorVal", false);
	LoadGlobalClassInfo(GC.getPlayerColorInfo(), "CIV4PlayerColorInfos", "Interface", "Civ4PlayerColorInfos/PlayerColorInfos/PlayerColorInfo", false);
	LoadGlobalClassInfo(GC.getEffectInfo(), "CIV4EffectInfos", "Misc", "Civ4EffectInfos/EffectInfos/EffectInfo", false);
	LoadGlobalClassInfo(GC.getEntityEventInfo(), "CIV4EntityEventInfos", "Units", "Civ4EntityEventInfos/EntityEventInfos/EntityEventInfo", false);
	LoadGlobalClassInfo(GC.getBuildInfo(), "CIV4BuildInfos", "Units", "Civ4BuildInfos/BuildInfos/BuildInfo", false);
/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 07/07/09                                                        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	LoadGlobalClassInfo(GC.getMissionInfo(), "CIV4MissionInfos", "Units", "Civ4MissionInfos/MissionInfos/MissionInfo", false);
	LoadGlobalClassInfo(GC.getUnitInfo(), "CIV4UnitInfos", "Units", "Civ4UnitInfos/UnitInfos/UnitInfo", true, &CvDLLUtilityIFaceBase::createUnitInfoCacheObject);
	LoadGlobalClassInfo(GC.getSpawnInfo(), "CIV4SpawnInfos", "Units", "Civ4SpawnInfos/SpawnInfos/SpawnInfo", false);
//	LoadGlobalClassInfo(GC.getUnitInfo(), "CIV4UnitInfos", "Units", "Civ4UnitInfos/UnitInfos/UnitInfo", true, &CvDLLUtilityIFaceBase::createUnitInfoCacheObject);
/************************************************************************************************/
/* JOOYO_ADDON                          END                                                     */
/************************************************************************************************/
	for (int i=0; i < GC.getNumUnitClassInfos(); ++i)
	{
		GC.getUnitClassInfo((UnitClassTypes)i).readPass3();
	}
	LoadGlobalClassInfo(GC.getUnitArtStyleTypeInfo(), "CIV4UnitArtStyleTypeInfos", "Civilizations", "Civ4UnitArtStyleTypeInfos/UnitArtStyleTypeInfos/UnitArtStyleTypeInfo", false);
	LoadGlobalClassInfo(GC.getCivilizationInfo(), "CIV4CivilizationInfos", "Civilizations", "Civ4CivilizationInfos/CivilizationInfos/CivilizationInfo", true, &CvDLLUtilityIFaceBase::createCivilizationInfoCacheObject);
/************************************************************************************************/
/* SORT_ALPHABET                           11/17/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	SortAlphabetGlobalClassInfo(GC.getCivilizationInfo());
/************************************************************************************************/
/* SORT_ALPHABET                           END                                                  */
/************************************************************************************************/
	LoadGlobalClassInfo(GC.getHints(), "CIV4Hints", "GameInfo", "Civ4Hints/HintInfos/HintInfo", false);
	LoadGlobalClassInfo(GC.getMainMenus(), "CIV4MainMenus", "Art", "Civ4MainMenus/MainMenus/MainMenu", false);
	LoadGlobalClassInfo(GC.getSlideShowInfo(), "CIV4SlideShowInfos", "Interface", "Civ4SlideShowInfos/SlideShowInfos/SlideShowInfo", false);
	LoadGlobalClassInfo(GC.getSlideShowRandomInfo(), "CIV4SlideShowRandomInfos", "Interface", "Civ4SlideShowRandomInfos/SlideShowRandomInfos/SlideShowRandomInfo", false);
	LoadGlobalClassInfo(GC.getWorldPickerInfo(), "CIV4WorldPickerInfos", "Interface", "Civ4WorldPickerInfos/WorldPickerInfos/WorldPickerInfo", false);
	LoadGlobalClassInfo(GC.getSpaceShipInfo(), "Civ4SpaceShipInfos", "Interface", "Civ4SpaceShipInfos/SpaceShipInfos/SpaceShipInfo", false);


	LoadGlobalClassInfo(GC.getYieldInfo(), "CIV4YieldInfos", "Terrain", "Civ4YieldInfos/YieldInfos/YieldInfo", false);
	LoadGlobalClassInfo(GC.getCommerceInfo(), "CIV4CommerceInfo", "GameInfo", "Civ4CommerceInfo/CommerceInfos/CommerceInfo", false);
	LoadGlobalClassInfo(GC.getMPOptionInfo(), "CIV4MPOptionInfos", "GameInfo", "Civ4MPOptionInfos/MPOptionInfos/MPOptionInfo", false);
	LoadGlobalClassInfo(GC.getForceControlInfo(), "CIV4ForceControlInfos", "GameInfo", "Civ4ForceControlInfos/ForceControlInfos/ForceControlInfo", false);
/************************************************************************************************/
/* Afforess                                03/18/10                                             */
/*                            ReadPass3                                                         */
/*                                                                                              */
/************************************************************************************************/
	LoadGlobalClassInfo(GC.getEventInfo(), "CIV4EventInfos", "Events", "Civ4EventInfos/EventInfos/EventInfo", false, &CvDLLUtilityIFaceBase::createEventInfoCacheObject);
	LoadGlobalClassInfo(GC.getEventTriggerInfo(), "CIV4EventTriggerInfos", "Events", "Civ4EventTriggerInfos/EventTriggerInfos/EventTriggerInfo", false, &CvDLLUtilityIFaceBase::createEventTriggerInfoCacheObject);

	for (int i=0; i < GC.getNumBuildingInfos(); ++i)
	{
		GC.getBuildingInfo((BuildingTypes)i).readPass3();
	}
	for (int i=0; i < GC.getNumCivicInfos(); ++i)
	{
		GC.getCivicInfo((CivicTypes)i).readPass3();
	}
	for (int i=0; i < GC.getNumCorporationInfos(); ++i)
	{
		GC.getCorporationInfo((CorporationTypes)i).readPass3();
	}
	for (int i=0; i < GC.getNumEventInfos(); ++i)
	{
		GC.getEventInfo((EventTypes)i).readPass3();
	}
	for (int i=0; i < GC.getNumProjectInfos(); ++i)
	{
		GC.getProjectInfo((ProjectTypes)i).readPass3();
	}
	for (int i=0; i < GC.getNumTechInfos(); ++i)
	{
		GC.getTechInfo((TechTypes)i).readPass3();
	}
	for (int i=0; i < GC.getNumUnitInfos(); ++i)
	{
		GC.getUnitInfo((UnitTypes)i).readPass3();
	}
	for (int i=0; i < GC.getNumTraitInfos(); ++i)
	{
		GC.getTraitInfo((TraitTypes)i).readPass3();
	}
/************************************************************************************************/
/* Afforess                                END                                                  */
/************************************************************************************************/


	// add types to global var system
	for (int i = 0; i < GC.getNumCursorInfos(); ++i)
	{
		int iVal;
		CvString szType = GC.getCursorInfo((CursorTypes)i).getType();
		if (GC.getDefinesVarSystem()->GetValue(szType, iVal))
		{
			char szMessage[1024];
			sprintf(szMessage, "cursor type already set? \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
			gDLL->MessageBox(szMessage, "XML Error");
		}
		GC.getDefinesVarSystem()->SetValue(szType, i);
	}

	// Check Playables
	for (int i=0; i < GC.getNumCivilizationInfos(); ++i)
	{
		// if the civilization is playable we will increment the playable var
		if (GC.getCivilizationInfo((CivilizationTypes) i).isPlayable())
		{
			GC.getNumPlayableCivilizationInfos() += 1;
		}

		// if the civilization is playable by AI increments num playable
		if (GC.getCivilizationInfo((CivilizationTypes) i).isAIPlayable())
		{
			GC.getNumAIPlayableCivilizationInfos() += 1;
		}
	}

	GC.getInitCore().checkInitialCivics();

/************************************************************************************************/
/* TGA_INDEXATION                          03/25/08                                MRGENIE      */
/*                                                                                              */
/* Filling up empty spots(i.e. because module not loaded)                                       */
/************************************************************************************************/
	AddTGABogus(GC.getCorporationInfo(), "CIV4CorporationInfo");
	AddTGABogus(GC.getReligionInfo(), "CIV4ReligionInfo");
/************************************************************************************************/
/* TGA_INDEXATION                          END                                                  */
/************************************************************************************************/

	UpdateProgressCB("GlobalOther");

	DestroyFXml();

	return true;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   LoadPostMenuGlobals()
//
//  PURPOSE :   loads global xml data which isn't needed for the main menus
//		this data is loaded as a secodn stage, when the game is launched
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::LoadPostMenuGlobals()
{
	PROFILE_FUNC();
	if (!CreateFXml())
	{
		return false;
	}

	//throne room disabled
	UpdateProgressCB("Global Throne Room");

	LoadGlobalClassInfo(GC.getThroneRoomCamera(), "CIV4ThroneRoomCameraInfos", "Interface", "Civ4ThroneRoomCameraInfos/ThroneRoomCameraInfos/ThroneRoomCamera", false);
	LoadGlobalClassInfo(GC.getThroneRoomInfo(), "CIV4ThroneRoomInfos", "Interface", "Civ4ThroneRoomInfos/ThroneRoomInfos/ThroneRoomInfo", false);
	LoadGlobalClassInfo(GC.getThroneRoomStyleInfo(), "CIV4ThroneRoomStyleInfos", "Interface", "Civ4ThroneRoomStyleInfos/ThroneRoomStyleInfos/ThroneRoomStyleInfo", false);

	UpdateProgressCB("Global Events");
	UpdateProgressCB("Global Routes");

	LoadGlobalClassInfo(GC.getRouteModelInfo(), "Civ4RouteModelInfos", "Art", "Civ4RouteModelInfos/RouteModelInfos/RouteModelInfo", false);

	UpdateProgressCB("Global Rivers");

	LoadGlobalClassInfo(GC.getRiverInfo(), "CIV4RiverInfos", "Misc", "Civ4RiverInfos/RiverInfos/RiverInfo", false);
	LoadGlobalClassInfo(GC.getRiverModelInfo(), "CIV4RiverModelInfos", "Art", "Civ4RiverModelInfos/RiverModelInfos/RiverModelInfo", false);

	UpdateProgressCB("Global Other");

	LoadGlobalClassInfo(GC.getWaterPlaneInfo(), "CIV4WaterPlaneInfos", "Misc", "Civ4WaterPlaneInfos/WaterPlaneInfos/WaterPlaneInfo", false);
	LoadGlobalClassInfo(GC.getTerrainPlaneInfo(), "CIV4TerrainPlaneInfos", "Misc", "Civ4TerrainPlaneInfos/TerrainPlaneInfos/TerrainPlaneInfo", false);
	LoadGlobalClassInfo(GC.getCameraOverlayInfo(), "CIV4CameraOverlayInfos", "Misc", "Civ4CameraOverlayInfos/CameraOverlayInfos/CameraOverlayInfo", false);


	UpdateProgressCB("Global Process");

	LoadGlobalClassInfo(GC.getProcessInfo(), "CIV4ProcessInfo", "GameInfo", "Civ4ProcessInfo/ProcessInfos/ProcessInfo", false);

	UpdateProgressCB("Global Emphasize");

	LoadGlobalClassInfo(GC.getEmphasizeInfo(), "CIV4EmphasizeInfo", "GameInfo", "Civ4EmphasizeInfo/EmphasizeInfos/EmphasizeInfo", false);

	UpdateProgressCB("Global Other");

	LoadGlobalClassInfo(GC.getControlInfo(), "CIV4ControlInfos", "Units", "Civ4ControlInfos/ControlInfos/ControlInfo", false);
	LoadGlobalClassInfo(GC.getCommandInfo(), "CIV4CommandInfos", "Units", "Civ4CommandInfos/CommandInfos/CommandInfo", false);
	LoadGlobalClassInfo(GC.getAutomateInfo(), "CIV4AutomateInfos", "Units", "Civ4AutomateInfos/AutomateInfos/AutomateInfo", false);

	UpdateProgressCB("Global Vote");

	LoadGlobalClassInfo(GC.getVoteInfo(), "CIV4VoteInfo", "GameInfo", "Civ4VoteInfo/VoteInfos/VoteInfo", false);

	UpdateProgressCB("Global Interface");

	LoadGlobalClassInfo(GC.getCameraInfo(), "CIV4CameraInfos", "Interface", "Civ4CameraInfos/CameraInfos/CameraInfo", false);
	LoadGlobalClassInfo(GC.getInterfaceModeInfo(), "CIV4InterfaceModeInfos", "Interface", "Civ4InterfaceModeInfos/InterfaceModeInfos/InterfaceModeInfo", false);

	SetGlobalActionInfo();


	// Load the formation info
	LoadGlobalClassInfo(GC.getUnitFormationInfo(), "CIV4FormationInfos", "Units", "UnitFormations/UnitFormation", false);

	// Load the attachable infos
	LoadGlobalClassInfo(GC.getAttachableInfo(), "CIV4AttachableInfos", "Misc", "Civ4AttachableInfos/AttachableInfos/AttachableInfo", false);

	// Specail Case Diplomacy Info due to double vectored nature and appending of Responses
	LoadDiplomacyInfo(GC.getDiplomacyInfo(), "CIV4DiplomacyInfos", "GameInfo", "Civ4DiplomacyInfos/DiplomacyInfos/DiplomacyInfo", &CvDLLUtilityIFaceBase::createDiplomacyInfoCacheObject);

	LoadGlobalClassInfo(GC.getQuestInfo(), "Civ4QuestInfos", "Misc", "Civ4QuestInfos/QuestInfo", false);
	LoadGlobalClassInfo(GC.getTutorialInfo(), "Civ4TutorialInfos", "Misc", "Civ4TutorialInfos/TutorialInfo", false);

	LoadGlobalClassInfo(GC.getEspionageMissionInfo(), "CIV4EspionageMissionInfo", "GameInfo", "Civ4EspionageMissionInfo/EspionageMissionInfos/EspionageMissionInfo", false);

	DestroyFXml();

	GC.getInitCore().calculateAssetCheckSum();

	return true;
}


//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalStringArray(TCHAR (**ppszString)[256], char* szTagName, int* iNumVals)
//
//  PURPOSE :   takes the szTagName parameter and if it finds it in the m_pFXml member variable
//				then it loads the ppszString parameter with the string values under it and the
//				iNumVals with the total number of tags with the szTagName in the xml file
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalStringArray(CvString **ppszString, char* szTagName, int* iNumVals, bool bUseEnum)
{
	PROFILE_FUNC();
	logMsg("SetGlobalStringArray %s\n", szTagName);

	int i=0;					//loop counter
	CvString *pszString;	// hold the local pointer to the newly allocated string memory
	pszString = NULL;			// null out the local string pointer so that it can be checked at the
	// end of the function in an FAssert

	// if we locate the szTagName, the current node is set to the first instance of the tag name in the xml file
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		if (!bUseEnum)
		{
			// get the total number of times this tag appears in the xml
			*iNumVals = gDLL->getXMLIFace()->NumOfElementsByTagName(m_pFXml,szTagName);
		}
		// initialize the memory based on the total number of tags in the xml and the 256 character length we selected
		*ppszString = new CvString[*iNumVals];
		// set the local pointer to the memory just allocated
		pszString = *ppszString;

		// loop through each of the tags
		for (i=0;i<*iNumVals;i++)
		{
			// get the string value at the current node
			GetXmlVal(pszString[i]);
			GC.setTypesEnum(pszString[i], i);
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 05/29/08                                MRGENIE      */
/*                                                                                              */
/* adding to hash map - used for the dependencies                                               */
/************************************************************************************************/
#ifdef _DEBUG
			logMsg(" Adding info type %s with ID %i", pszString[i].c_str(), i);
#endif
			GC.setInfoTypeFromString(pszString[i], i);
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/

			// if can't set the current node to a sibling node we will break out of the for loop
			// otherwise we will keep looping
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}
	}

	// if the local string pointer is null then we weren't able to find the szTagName in the xml
	// so we will FAssert to let whoever know it
	if (!pszString)
	{
		char	szMessage[1024];
		sprintf( szMessage, "Error locating tag node in SetGlobalStringArray function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}




//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalActionInfo(CvActionInfo** ppActionInfo, int* iNumVals)
//
//  PURPOSE :   Takes the szTagName parameter and if it exists in the xml it loads the ppActionInfo
//				with the value under it and sets the value of the iNumVals parameter to the total number
//				of occurances of the szTagName tag in the xml.
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalActionInfo()
{
	PROFILE_FUNC();
	logMsg("SetGlobalActionInfo\n");
	int i=0;					//loop counter

	if(!(NUM_INTERFACEMODE_TYPES > 0))
	{
		char	szMessage[1024];
		sprintf( szMessage, "NUM_INTERFACE_TYPES is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(GC.getNumBuildInfos() > 0))
	{
		char	szMessage[1024];
		sprintf( szMessage, "GC.getNumBuildInfos() is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(GC.getNumPromotionInfos() > 0))
	{
		char	szMessage[1024];
		sprintf( szMessage, "GC.getNumPromotionInfos() is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(GC.getNumUnitClassInfos() > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "GC.getNumUnitClassInfos() is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(GC.getNumSpecialistInfos() > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "GC.getNumSpecialistInfos() is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(GC.getNumBuildingInfos() > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "GC.getNumBuildingInfos() is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(NUM_CONTROL_TYPES > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "NUM_CONTROL_TYPES is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(GC.getNumAutomateInfos() > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "GC.getNumAutomateInfos() is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(NUM_COMMAND_TYPES > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "NUM_COMMAND_TYPES is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if(!(NUM_MISSION_TYPES > 0) )
	{
		char	szMessage[1024];
		sprintf( szMessage, "NUM_MISSION_TYPES is not greater than zero in CvXMLLoadUtility::SetGlobalActionInfo \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}

	int* piOrderedIndex=NULL;

	int iNumOrigVals = GC.getNumActionInfos();

	int iNumActionInfos = iNumOrigVals +
		NUM_INTERFACEMODE_TYPES +
		GC.getNumBuildInfos() +
		GC.getNumPromotionInfos() +
		GC.getNumReligionInfos() +
		GC.getNumCorporationInfos() +
		GC.getNumUnitInfos() +
		GC.getNumSpecialistInfos() +
		GC.getNumBuildingInfos() +
		NUM_CONTROL_TYPES +
		NUM_COMMAND_TYPES +
		GC.getNumAutomateInfos() +
		NUM_MISSION_TYPES;

	int* piIndexList = new int[iNumActionInfos];
	int* piPriorityList = new int[iNumActionInfos];
	int* piActionInfoTypeList = new int[iNumActionInfos];

	int iTotalActionInfoCount = 0;

	// loop through control info
	for (i=0;i<NUM_COMMAND_TYPES;i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getCommandInfo((CommandTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_COMMAND;
		iTotalActionInfoCount++;
	}

	for (i=0;i<NUM_INTERFACEMODE_TYPES;i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getInterfaceModeInfo((InterfaceModeTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_INTERFACEMODE;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumBuildInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getBuildInfo((BuildTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_BUILD;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumPromotionInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getPromotionInfo((PromotionTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_PROMOTION;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumUnitInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getUnitInfo((UnitTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_UNIT;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumReligionInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getReligionInfo((ReligionTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_RELIGION;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumCorporationInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getCorporationInfo((CorporationTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_CORPORATION;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumSpecialistInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getSpecialistInfo((SpecialistTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_SPECIALIST;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumBuildingInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getBuildingInfo((BuildingTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_BUILDING;
		iTotalActionInfoCount++;
	}

	for (i=0;i<NUM_CONTROL_TYPES;i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getControlInfo((ControlTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_CONTROL;
		iTotalActionInfoCount++;
	}

	for (i=0;i<GC.getNumAutomateInfos();i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getAutomateInfo(i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_AUTOMATE;
		iTotalActionInfoCount++;
	}

	for (i=0;i<NUM_MISSION_TYPES;i++)
	{
		piIndexList[iTotalActionInfoCount] = i;
		piPriorityList[iTotalActionInfoCount] = GC.getMissionInfo((MissionTypes)i).getOrderPriority();
		piActionInfoTypeList[iTotalActionInfoCount] = ACTIONSUBTYPE_MISSION;
		iTotalActionInfoCount++;
	}

	SAFE_DELETE_ARRAY(piOrderedIndex);
	piOrderedIndex = new int[iNumActionInfos];

	orderHotkeyInfo(&piOrderedIndex, piPriorityList, iNumActionInfos);
	for (i=0;i<iNumActionInfos;i++)
	{
		CvActionInfo* pActionInfo = new CvActionInfo;
		pActionInfo->setOriginalIndex(piIndexList[piOrderedIndex[i]]);
		pActionInfo->setSubType((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]]);
		if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_COMMAND)
		{
			GC.getCommandInfo((CommandTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_INTERFACEMODE)
		{
			GC.getInterfaceModeInfo((InterfaceModeTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_BUILD)
		{
			GC.getBuildInfo((BuildTypes)piIndexList[piOrderedIndex[i]]).setMissionType(FindInInfoClass("MISSION_BUILD"));
			GC.getBuildInfo((BuildTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_PROMOTION)
		{
			GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).setCommandType(FindInInfoClass("COMMAND_PROMOTION"));
			GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
			GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).setHotKeyDescription(GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).getTextKeyWide(), GC.getCommandInfo((CommandTypes)(GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).getCommandType())).getTextKeyWide(), CreateHotKeyFromDescription(GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).getHotKey(), GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).isShiftDown(), GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).isAltDown(), GC.getPromotionInfo((PromotionTypes)piIndexList[piOrderedIndex[i]]).isCtrlDown()));
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_UNIT)
		{
			GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).setCommandType(FindInInfoClass("COMMAND_UPGRADE"));
			GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
			GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).setHotKeyDescription(GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).getTextKeyWide(), GC.getCommandInfo((CommandTypes)(GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).getCommandType())).getTextKeyWide(), CreateHotKeyFromDescription(GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).getHotKey(), GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).isShiftDown(), GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).isAltDown(), GC.getUnitInfo((UnitTypes)piIndexList[piOrderedIndex[i]]).isCtrlDown()));
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_RELIGION)
		{
			GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).setMissionType(FindInInfoClass("MISSION_SPREAD"));
			GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
			GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).setHotKeyDescription(GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).getTextKeyWide(), GC.getMissionInfo((MissionTypes)(GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).getMissionType())).getTextKeyWide(), CreateHotKeyFromDescription(GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).getHotKey(), GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).isShiftDown(), GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).isAltDown(), GC.getReligionInfo((ReligionTypes)piIndexList[piOrderedIndex[i]]).isCtrlDown()));
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_CORPORATION)
		{
			GC.getCorporationInfo((CorporationTypes)piIndexList[piOrderedIndex[i]]).setMissionType(FindInInfoClass("MISSION_SPREAD_CORPORATION"));
			GC.getCorporationInfo((CorporationTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
			GC.getCorporationInfo((CorporationTypes)piIndexList[piOrderedIndex[i]]).setHotKeyDescription(GC.getCorporationInfo((CorporationTypes)piIndexList[piOrderedIndex[i]]).getTextKeyWide(), GC.getMissionInfo((MissionTypes)(GC.getCorporationInfo((CorporationTypes)piIndexList[piOrderedIndex[i]]).getMissionType())).getTextKeyWide(), CreateHotKeyFromDescription(GC.getCorporationInfo((CorporationTypes)piIndexList[piOrderedIndex[i]]).getHotKey(), GC.getCorporationInfo((CorporationTypes)piIndexList[piOrderedIndex[i]]).isShiftDown(), GC.getCorporationInfo((CorporationTypes)piIndexList[piOrderedIndex[i]]).isAltDown(), GC.getCorporationInfo((CorporationTypes)piIndexList[piOrderedIndex[i]]).isCtrlDown()));
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_SPECIALIST)
		{
			GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).setMissionType(FindInInfoClass("MISSION_JOIN"));
			GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
			GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).setHotKeyDescription(GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).getTextKeyWide(), GC.getMissionInfo((MissionTypes)(GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).getMissionType())).getTextKeyWide(), CreateHotKeyFromDescription(GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).getHotKey(), GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).isShiftDown(), GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).isAltDown(), GC.getSpecialistInfo((SpecialistTypes)piIndexList[piOrderedIndex[i]]).isCtrlDown()));
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_BUILDING)
		{
			GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).setMissionType(FindInInfoClass("MISSION_CONSTRUCT"));
			GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
			GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).setHotKeyDescription(GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).getTextKeyWide(), GC.getMissionInfo((MissionTypes)(GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).getMissionType())).getTextKeyWide(), CreateHotKeyFromDescription(GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).getHotKey(), GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).isShiftDown(), GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).isAltDown(), GC.getBuildingInfo((BuildingTypes)piIndexList[piOrderedIndex[i]]).isCtrlDown()));
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_CONTROL)
		{
			GC.getControlInfo((ControlTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_AUTOMATE)
		{
			GC.getAutomateInfo(piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i);
		}
		else if ((ActionSubTypes)piActionInfoTypeList[piOrderedIndex[i]] == ACTIONSUBTYPE_MISSION)
		{
			GC.getMissionInfo((MissionTypes)piIndexList[piOrderedIndex[i]]).setActionInfoIndex(i + iNumOrigVals);
		}

		GC.getActionInfo().push_back(pActionInfo);
	}

	SAFE_DELETE_ARRAY(piOrderedIndex);
	SAFE_DELETE_ARRAY(piIndexList);
	SAFE_DELETE_ARRAY(piPriorityList);
	SAFE_DELETE_ARRAY(piActionInfoTypeList);
}


//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalAnimationPathInfo(CvAnimationPathInfo** ppAnimationPathInfo, char* szTagName, int* iNumVals)
//
//  PURPOSE :   Takes the szTagName parameter and if it exists in the xml it loads the ppAnimationPathInfo
//				with the value under it and sets the value of the iNumVals parameter to the total number
//				of occurances of the szTagName tag in the xml.
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalAnimationPathInfo(CvAnimationPathInfo** ppAnimationPathInfo, char* szTagName, int* iNumVals)
{
	PROFILE_FUNC();
	logMsg( "SetGlobalAnimationPathInfo %s\n", szTagName );

	int		i;						// Loop counters
	CvAnimationPathInfo * pAnimPathInfo = NULL;	// local pointer to the domain info memory

	if ( gDLL->getXMLIFace()->LocateNode(m_pFXml, szTagName ))
	{
		// get the number of times the szTagName tag appears in the xml file
		*iNumVals = gDLL->getXMLIFace()->NumOfElementsByTagName(m_pFXml,szTagName);

		// allocate memory for the domain info based on the number above
		*ppAnimationPathInfo = new CvAnimationPathInfo[*iNumVals];
		pAnimPathInfo = *ppAnimationPathInfo;

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		gDLL->getXMLIFace()->SetToChild(m_pFXml);
		gDLL->getXMLIFace()->SetToChild(m_pFXml);


		// Loop through each tag.
		for (i=0;i<*iNumVals;i++)
		{
			SkipToNextVal();	// skip to the next non-comment node

			if (!pAnimPathInfo[i].read(this))
				break;
			GC.setInfoTypeFromString(pAnimPathInfo[i].getType(), i);	// add type to global info type hash map
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
			{
				break;
			}
		}
	}

	// if we didn't find the tag name in the xml then we never set the local pointer to the
	// newly allocated memory and there for we will FAssert to let people know this most
	// interesting fact
	if(!pAnimPathInfo )
	{
		char	szMessage[1024];
		sprintf( szMessage, "Error finding tag node in SetGlobalAnimationPathInfo function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalUnitScales(float* pfLargeScale, float* pfSmallScale, char* szTagName)
//
//  PURPOSE :   Takes the szTagName parameter and if it exists in the xml it loads the ppPromotionInfo
//				with the value under it and sets the value of the iNumVals parameter to the total number
//				of occurances of the szTagName tag in the xml.
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGlobalUnitScales(float* pfLargeScale, float* pfSmallScale, char* szTagName)
{
	PROFILE_FUNC();
	logMsg("SetGlobalUnitScales %s\n", szTagName);
	// if we successfully locate the szTagName node
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml,szTagName))
	{
		// call the function that sets the FXml pointer to the first non-comment child of
		// the current tag and gets the value of that new node
		if (GetChildXmlVal(pfLargeScale))
		{
			// set the current xml node to it's next sibling and then
			// get the sibling's TCHAR value
			GetNextXmlVal(pfSmallScale);

			// set the current xml node to it's parent node
			gDLL->getXMLIFace()->SetToParent(m_pFXml);
		}
	}
	else
	{
		// if we didn't find the tag name in the xml then we never set the local pointer to the
		// newly allocated memory and there for we will FAssert to let people know this most
		// interesting fact
		char	szMessage[1024];
		sprintf( szMessage, "Error finding tag node in SetGlobalUnitScales function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
}


//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGameText()
//
//  PURPOSE :   Reads game text info from XML and adds it to the translation manager
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetGameText(const char* szTextGroup, const char* szTagName)
{
	PROFILE_FUNC();
	logMsg("SetGameText %s\n", szTagName);
	int i=0;		//loop counter - Index into pTextInfo

	if (gDLL->getXMLIFace()->LocateNode(m_pFXml, szTextGroup)) // Get the Text Group 1st
	{
		int iNumVals = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);	// Get the number of Children that the Text Group has
		gDLL->getXMLIFace()->LocateNode(m_pFXml, szTagName); // Now switch to the TEXT Tag
		gDLL->getXMLIFace()->SetToParent(m_pFXml);
		gDLL->getXMLIFace()->SetToChild(m_pFXml);

		// loop through each tag
		for (i=0; i < iNumVals; i++)
		{
			CvGameText textInfo;
			textInfo.read(this);

			gDLL->addText(textInfo.getType() /*id*/, textInfo.getText(), textInfo.getGender(), textInfo.getPlural());
			if (!gDLL->getXMLIFace()->NextSibling(m_pFXml) && i!=iNumVals-1)
			{
				char	szMessage[1024];
				sprintf( szMessage, "failed to find sibling \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
				gDLL->MessageBox(szMessage, "XML Error");
				break;
			}
		}
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetGlobalClassInfo - This is a template function that is USED FOR ALMOST ALL INFO CLASSES.
//		Each info class should have a read(CvXMLLoadUtility*) function that is responsible for initializing
//		the class from xml data.
//
//  PURPOSE :   takes the szTagName parameter and loads the ppszString with the text values
//				under the tags.  This will be the hints displayed during game initialization and load
//
//------------------------------------------------------------------------------------------------------
template <class T>
void CvXMLLoadUtility::SetGlobalClassInfo(std::vector<T*>& aInfos, const char* szTagName, bool bTwoPass)
{
	char szLog[256];
	sprintf(szLog, "SetGlobalClassInfo (%s)", szTagName);
	PROFILE(szLog);
	logMsg(szLog);

	// if we successfully locate the tag name in the xml file
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml, szTagName))
	{
		// loop through each tag
		do
		{
				if (!SkipToNextVal())	// skip to the next non-comment node
					break;              // AIAndy: need to break the loop when the last sibling is a comment node

				T* pClassInfo = new T;

				FAssert(NULL != pClassInfo);
				if (NULL == pClassInfo)
				{
					break;
				}

				bool bSuccess = pClassInfo->read(this);
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 02/20/08                                MRGENIE      */
/*                                                                                              */
/* If a Type is dependent on it's existence(i.e. a modder adds something to an existing         */
/* Type but doesn't want to actual initialize it in case the Type didn't exist previously)      */
/* this check here makes sure that the file will be skipped then and not used to create the     */
/* object                                                                                       */
/************************************************************************************************/
/*
				FAssert(bSuccess);
				if (!bSuccess)
				{
					delete pClassInfo;
					break;
				}
*/
				if ( !GC.isAnyDependency() )
				{
					CvString szAssertBuffer;
					szAssertBuffer.Format("OWN TYPE - dependency not found: %s, in file: \"%s\"", pClassInfo->getType(), GC.getModDir().c_str());
					//FAssertMsg(bSuccess, szAssertBuffer.c_str());
					if (!bSuccess)
					{
				    //#ifdef _DEBUG
						logXmlDependencyTypes("\n\nOWN TYPE - dependency not found: %s, in file: \"%s\"", pClassInfo->getType(), GC.getModDir().c_str());
						logXmlDependencyTypes("My new check!");
					//#endif
						delete pClassInfo;
						break;
					}
				}
				else
				{
					int iTypeIndex = -1;
					if (NULL != pClassInfo->getType())
					{
						iTypeIndex = GC.getInfoTypeForString(pClassInfo->getType(), true);
					}

					// TYPE dependency? (Condition 1)
					if ( GC.getTypeDependency() && (-1 == iTypeIndex))
					{
						//#ifdef _DEBUG
							logXmlDependencyTypes("\n\nOWN TYPE - dependency not found: %s, in file: \"%s\"", pClassInfo->getType(), GC.getModDir().c_str());
							logXmlDependencyTypes("Possible reasons: missing module, wrong MLF order, bLoad set to 0?");
						//#endif
						delete pClassInfo;
						GC.resetDependencies();		// make sure we always reset once anydependency was true!
						continue;
					}

					// OR Dependencies (Condition 2)
					GC.setTypeDependency(false);
					if ( GC.getOrNumDependencyTypes() > 0 )
					{
						// if we have Or dependencies, set to dependend by default(this will prevent loading)
						// the moment ANY condition is met, we can safely load the ClassInfo and set the
						// dependency to false
						GC.setTypeDependency(true);
						//#ifdef _DEBUG
							logXmlDependencyTypes("\n\nOR - dependencies will be checked here for, TYPE: %s, in file: \"%s\"", pClassInfo->getType(), GC.getModDir().c_str());
						//#endif
					}
					for ( int iI = 0; iI < GC.getOrNumDependencyTypes(); iI++ )
					{
						iTypeIndex = GC.getInfoTypeForString( GC.getOrDependencyTypes(iI), true );
						if ( iTypeIndex == -1 )
						{
							//#ifdef _DEBUG
								logXmlDependencyTypes("OR - Dependency not found, TYPE: %s", GC.getOrDependencyTypes(iI).c_str());
							//#endif
						}
						else
						{
							// we found a OR dependent Type, so we can load safely!
							// dependency will be set disabled(false)
#ifdef _DEBUG
	logXmlDependencyTypes("OR - Dependency found, TYPE: %s", GC.getOrDependencyTypes(iI).c_str());
#endif
							GC.setTypeDependency(false);
						}
					}

					// AND Dependencies (Condition 3)
					if (!(GC.getAndNumDependencyTypes() > 0 ))
					{
#ifdef _DEBUG
	logXmlDependencyTypes("\n\n");
#endif
					}
#ifdef _DEBUG
	logXmlDependencyTypes("AND - dependencies will be checked here for, TYPE: %s, in file: \"%s\"", pClassInfo->getType(), GC.getModDir().c_str());
#endif

					if (GC.getAndNumDependencyTypes() > 0)
					{
						bool bAllAndDepsFound = true;
						for ( int iI = 0; iI < GC.getAndNumDependencyTypes(); iI++ )
						{
							iTypeIndex = GC.getInfoTypeForString( GC.getAndDependencyTypes(iI), true );
							if ( iTypeIndex == -1 )
							{
								// if any AND condition is not met, we disable the loading of the Class Info!
								//#ifdef _DEBUG
									logXmlDependencyTypes("AND - Dependency not found, TYPE: %s", GC.getAndDependencyTypes(iI).c_str());
								//#endif
								GC.setTypeDependency(true);
								bAllAndDepsFound = false;
							}
							else
							{
	#ifdef _DEBUG
		logXmlDependencyTypes("AND - Dependency found, TYPE: %s", GC.getAndDependencyTypes(iI).c_str());
	#endif
							}
						}

						if (bAllAndDepsFound)
						{
							//#ifdef _DEBUG
								logXmlDependencyTypes("AND - dependencies were ALL met for TYPE: %s, in file: \"%s\"", pClassInfo->getType(), GC.getModDir().c_str());
							//#endif
						}
					}

					//This covers both the bTypeDependency and the And/Or-DependencyTypes tags!
					if ( GC.getTypeDependency() )
					{
						// We found that any of the 3! conditions NOT to load this class info has been met!
						//#ifdef _DEBUG
							logXmlDependencyTypes("TYPE: %s, NOT LOADED", pClassInfo->getType());
						//#endif
						delete pClassInfo;
						GC.resetDependencies();		// make sure we always reset once anydependency was true!
						continue;
					}
					else
					{
						//#ifdef _DEBUG
							logXmlDependencyTypes("OK!");
						//#endif
						bool bSuccess = pClassInfo->read(this);
						GC.resetDependencies();		// make sure we always reset once anydependency was true!
						FAssert(bSuccess);
						if (!bSuccess)
						{
							delete pClassInfo;
							break;
						}
					}
				}
/************************************************************************************************/
/* Afforess	                  Start		 03/17/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
				//Afforess Force Delete
				if (NULL != pClassInfo->getType())
				{
					if (GC.getNumGameSpeedInfos() > 0)
					{
						if(GC.getForceDelete() ||
						(GC.getDefineINT(pClassInfo->getType())))
						{
							delete pClassInfo;
							GC.resetOverwrites();	
							continue;
						}
					}
					if (GC.getModDir() != "NONE")
					{
						logXML("%s Loaded From %s", pClassInfo->getType(), GC.getModDir().c_str());
					}
					else 
					{
						logXML("%s Loaded From Base XML Files", pClassInfo->getType());
					}
				}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/

			int iIndex = -1;
			if (NULL != pClassInfo->getType())
			{
				iIndex = GC.getInfoTypeForString(pClassInfo->getType(), true);
			}

			if (-1 == iIndex)
			{
/************************************************************************************************/
/* Afforess	                  Start		 03/17/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
					//Valid Location
				/*	if (GC.getForceInsertLocation() >= 0 &&  GC.getForceInsertLocation() < (int)aInfos.size())
					{
						aInfos.insert(aInfos.begin() + GC.getForceInsertLocation(), pClassInfo); 
						logXmlDependencyTypes("%s moved to position %d" , pClassInfo->getType(), GC.getForceInsertLocation());
						//Move the old entries up one
						for (int iK = GC.getForceInsertLocation(); iK < (int)aInfos.size(); iK++)
						{
							GC.setInfoTypeFromString(aInfos[iK]->getType(), iK + 1);
						}
						if (NULL != pClassInfo->getType())
							GC.setInfoTypeFromString(pClassInfo->getType(), GC.getForceInsertLocation());
					}
					else*/
					{
						aInfos.push_back(pClassInfo);
						if (NULL != pClassInfo->getType())
						{
							GC.setInfoTypeFromString(pClassInfo->getType(), (int)aInfos.size() - 1);	// add type to global info type hash map
						}
					}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			}
			else	//Here we become modular!  (Warning, this also means having a double entry gets a LITTLE MORE CONFUSING as now you get data from both of them
			{
/*************************************************************************************************/
/**	TrueModular								05/26/09	Written: Mr. Genie	Imported: Xienwolf	**/
/**																								**/
/**	Special Method to disable overwriting of defaults like 0, NO_TECH, NULL, false, etc			**/
/**	over non default values. Added myself, has to be handled strange for pass 2 unfortunately	**/
/**																								**/
/**																								**/
/**	If a Variable has been initialized by any previous Module, we don't want to reset it by a	**/
/**			default value just because the 2nd Module doesn't know about it's existance			**/
/**			(assuming in the previous XML there was a reason for a non-default value)			**/
/** Arrays forming a list like Uniqueunitnames will be extended and appended so we don't loose  **/
/** functionality of the Modules using the same Type(iIndex) of the same classinfo				**/
/**																								**/
/**	Properly links Modular modifications to previous elements, and allows partial overwriting	**/
/*************************************************************************************************/
				//Copy information out of the previous entry where this new entry is boring...
				if(!GC.getForceOverwrite())
					pClassInfo->copyNonDefaults(aInfos[iIndex], this);
				else
				{
					logXML("Warning! bForceOverwrite used on %s", pClassInfo->getType());
				}
				//New information (just loaded) is running this function to pull information out of the old stuff
/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/
				//Delete the old entry data, but keep the spot reserved
				SAFE_DELETE(aInfos[iIndex]);
				//Now put this new data (which holds some of the old data) into that spot instead
				aInfos[iIndex] = pClassInfo;
				//So looks like all of this is done during the first readpass, since we haven't tried to call readpass2 CopyNonDefaults yet.  That means we need to ensure that anything bound for readpass3 is appended, and then it should sort itself out quite nicely for us.  Also means that unless I append information, anything from the original XML load doesn't exist anymore, as we deleted the memory container it was in
			}


		} while (gDLL->getXMLIFace()->NextSibling(m_pFXml));
//This ends readpass1, above loop keeps going till you reach the end of the XML file (or more correctly, go up one parent element from those which contain Types).
		if (bTwoPass)
		{
/************************************************************************************************/
/* XMLCOPY                                 10/13/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
			// if we successfully locate the szTagName node
			if (gDLL->getXMLIFace()->LocateNode(m_pFXml, szTagName))
			{
				gDLL->getXMLIFace()->SetToParent(m_pFXml);
				gDLL->getXMLIFace()->SetToChild(m_pFXml);

				// loop through each tag
				for (std::vector<T*>::iterator it = aInfos.begin(); it != aInfos.end(); ++it)
				{
					SkipToNextVal();	// skip to the next non-comment node

					(*it)->readPass2(this);

					if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
					{
						break;
					}
				}
			}
*/
			if (gDLL->getXMLIFace()->LocateNode(m_pFXml, szTagName))
			{
				// loop through each tag
				do
				{
					if (!SkipToNextVal())	// skip to the next non-comment node
						break;              // AIAndy: need to break the loop if the last sibling is a comment

					T* pClassInfo = new T;

					FAssert(NULL != pClassInfo);

					bool bSuccess = pClassInfo->readPass2(this);
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 02/20/08                                MRGENIE      */
/*                                                                                              */
/* If a Type is dependent on it's existence(i.e. a modder adds something to an existing         */
/* Type but doesn't want to actual initialize it in case the Type didn't exist previously)      */
/* this check here makes sure that the file will be skipped then and not used to create the     */
/* object                                                                                       */
/************************************************************************************************/
/*
					FAssert(bSuccess);
*/
					if ( !GC.isAnyDependency() )
					{
						FAssert(bSuccess);
					}
					else
					{
						int iTypeIndex = -1;
						if (NULL != pClassInfo->getType())
						{
							iTypeIndex = GC.getInfoTypeForString(pClassInfo->getType(), true);
						}

						// TYPE dependency? (Condition 1)
						if ( GC.getTypeDependency() && -1 == iTypeIndex)
						{
							delete pClassInfo;
							GC.resetDependencies();		// make sure we always reset once anydependency was true!
							continue;
						}

						// OR Dependencies (Condition 2)
						GC.setTypeDependency(false);
						if ( GC.getOrNumDependencyTypes() > 0 )
						{
							// if we have Or dependencies, set to dependend by default(this will prevent loading)
							// the moment ANY condition is met, we can safely load the ClassInfo and set the
							// dependency to false
							GC.setTypeDependency(true);
						}
						for ( int iI = 0; iI < GC.getOrNumDependencyTypes(); iI++ )
						{
							iTypeIndex = GC.getInfoTypeForString( GC.getOrDependencyTypes(iI), true );
							if ( !(iTypeIndex == -1) )
							{
								// we found a OR dependent Type, so we can load safely!
								// dependency will be set disabled(false)
								GC.setTypeDependency(false);
							}
						}

						for ( int iI = 0; iI < GC.getAndNumDependencyTypes(); iI++ )
						{
							iTypeIndex = GC.getInfoTypeForString( GC.getAndDependencyTypes(iI), true );
							if ( iTypeIndex == -1 )
							{
								// if any AND condition is not met, we disable the loading of the Class Info!
								GC.setTypeDependency(true);
							}
						}

						//This covers both the bTypeDependency and the And/Or-DependencyTypes tags!
						if ( GC.getTypeDependency() )
						{
							// We found that any of the 3! conditions NOT to load this class info has been met!
							delete pClassInfo;
							GC.resetDependencies();		// make sure we always reset once anydependency was true!
							continue;
						}
						else
						{
							bool bSuccess = pClassInfo->read(this);
							GC.resetDependencies();		// make sure we always reset once anydependency was true!
							FAssert(bSuccess);
							if (!bSuccess)
							{
								delete pClassInfo;
								break;
							}
						}
					}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* Afforess	                  Start		 03/17/10                                               */
/*                                                                                              */
/* Need to catch ReadPass2's.                                                                   */
/************************************************************************************************/
					//Afforess Force Delete
					//bForceDelete is not saved in the Infos, so it's safe to read it directly...
					GC.resetOverwrites();
					GetChildXmlValByName(&GC.getForceDelete(), "bForceDelete");
					if(GC.getForceDelete())
					{
						delete pClassInfo;
						GC.resetOverwrites();	
						continue;
					}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
					int iIndex = -1;
					if (NULL != pClassInfo->getType())
					{
						iIndex = GC.getInfoTypeForString(pClassInfo->getType(), true);
					}

					aInfos[iIndex]->copyNonDefaultsReadPass2(pClassInfo, this);
					SAFE_DELETE(pClassInfo);

				} while (gDLL->getXMLIFace()->NextSibling(m_pFXml));
			}
/************************************************************************************************/
/* XMLCOPY                                 END                                                  */
/************************************************************************************************/
		}
	}
}

/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 05/17/08                                MRGENIE      */
/*                                                                                              */
/* This method is a replacement for the bTwoPass, if stuff that is depending on each other in   */
/* a loop, the bTwoPass would fail since it doesn't look first in the other Modules!            */
/************************************************************************************************/
template <class T>
void CvXMLLoadUtility::SetGlobalClassInfoTwoPassReplacement(std::vector<T*>& aInfos, const char* szTagName)
{
	char szLog[256];
	sprintf(szLog, "SetGlobalClassInfo (%s)", szTagName);
	PROFILE(szLog);
	logMsg(szLog);

	if (gDLL->getXMLIFace()->LocateNode(m_pFXml, szTagName))
	{
		// loop through each tag
		do
		{
			if (!SkipToNextVal())	// skip to the next non-comment node
				break;              // AIAndy: if the last sibling node is a comment, need to break the loop

			T* pClassInfo = new T;

			FAssert(NULL != pClassInfo);

			bool bSuccess = pClassInfo->readPass2(this);

			if ( !GC.isAnyDependency() )
			{
				FAssert(bSuccess);
			}
			else
			{
				int iTypeIndex = -1;
				if (NULL != pClassInfo->getType())
				{
					iTypeIndex = GC.getInfoTypeForString(pClassInfo->getType(), true);
				}

				// TYPE dependency? (Condition 1)
				if ( GC.getTypeDependency() && -1 == iTypeIndex)
				{
					delete pClassInfo;
					GC.resetDependencies();		// make sure we always reset once anydependency was true!
					continue;
				}

				// OR Dependencies (Condition 2)
				GC.setTypeDependency(false);
				if ( GC.getOrNumDependencyTypes() > 0 )
				{
					// if we have Or dependencies, set to dependend by default(this will prevent loading)
					// the moment ANY condition is met, we can safely load the ClassInfo and set the
					// dependency to false
					GC.setTypeDependency(true);
				}
				for ( int iI = 0; iI < GC.getOrNumDependencyTypes(); iI++ )
				{
					iTypeIndex = GC.getInfoTypeForString( GC.getOrDependencyTypes(iI), true );
					if ( !(iTypeIndex == -1) )
					{
						// we found a OR dependent Type, so we can load safely!
						// dependency will be set disabled(false)
						GC.setTypeDependency(false);
					}
				}

				for ( int iI = 0; iI < GC.getAndNumDependencyTypes(); iI++ )
				{
					iTypeIndex = GC.getInfoTypeForString( GC.getAndDependencyTypes(iI), true );
					if ( iTypeIndex == -1 )
					{
						// if any AND condition is not met, we disable the loading of the Class Info!
						GC.setTypeDependency(true);
					}
				}

				//This covers both the bTypeDependency and the And/Or-DependencyTypes tags!
				if ( GC.getTypeDependency() )
				{
					// We found that any of the 3! conditions NOT to load this class info has been met!
					delete pClassInfo;
					GC.resetDependencies();		// make sure we always reset once anydependency was true!
					continue;
				}
				else
				{
					bool bSuccess = pClassInfo->read(this);
					GC.resetDependencies();		// make sure we always reset once anydependency was true!
					FAssert(bSuccess);
					if (!bSuccess)
					{
						delete pClassInfo;
						break;
					}
				}
			}
			
/************************************************************************************************/
/* Afforess	                  Start		 03/17/10                                               */
/*                                                                                              */
/* Need to catch ReadPass2's.                                                                   */
/************************************************************************************************/
					//Afforess Force Delete
					//bForceDelete is not saved in the Infos, so it's safe to read it directly...
					GC.resetOverwrites();
					GetChildXmlValByName(&GC.getForceDelete(), "bForceDelete");
					if (NULL != pClassInfo->getType())
					{
						if (GC.getNumGameSpeedInfos() > 0)
						{
							if(GC.getForceDelete() ||
							(GC.getDefineINT(pClassInfo->getType())))
							{
								delete pClassInfo;
								GC.resetOverwrites();	
								continue;
							}
						}
					}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

			int iIndex = -1;
			if (NULL != pClassInfo->getType())
			{
				iIndex = GC.getInfoTypeForString(pClassInfo->getType(), true);
			}

			aInfos[iIndex]->copyNonDefaultsReadPass2(pClassInfo, this);
			SAFE_DELETE(pClassInfo);

		} while (gDLL->getXMLIFace()->NextSibling(m_pFXml));
	}
}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
void CvXMLLoadUtility::SetDiplomacyInfo(std::vector<CvDiplomacyInfo*>& DiploInfos, const char* szTagName)
{
	char szLog[256];
	sprintf(szLog, "SetDiplomacyInfo (%s)", szTagName);
	PROFILE(szLog);
	logMsg(szLog);

	// if we successfully locate the tag name in the xml file
	if (gDLL->getXMLIFace()->LocateNode(m_pFXml, szTagName))
	{
		// loop through each tag
		do
		{
			if (!SkipToNextVal())	// skip to the next non-comment node
				break;              // AIAndy: need to break the loop if the last sibling is a comment

			CvString szType;
			GetChildXmlValByName(szType, "Type");
			int iIndex = GC.getInfoTypeForString(szType, true);

			if (-1 == iIndex)
			{
				CvDiplomacyInfo* pClassInfo = new CvDiplomacyInfo;

				if (NULL == pClassInfo)
				{
					FAssert(false);
					break;
				}

				pClassInfo->read(this);
				if (NULL != pClassInfo->getType())
				{
					GC.setInfoTypeFromString(pClassInfo->getType(), (int)DiploInfos.size());	// add type to global info type hash map
				}
				DiploInfos.push_back(pClassInfo);
			}
			else
			{
/*************************************************************************************************/
/**	TrueModular								05/26/09	Written: Mr. Genie	Imported: Xienwolf	**/
/**																								**/
/**	Properly links Modular modifications to previous elements, and allows partial overwriting	**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
				DiploInfos[iIndex]->read(this);
/**								----  End Original Code  ----									**/
				if(!GC.getForceOverwrite())
					DiploInfos[iIndex]->copyNonDefaults(this);
/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/
			}

		} while (gDLL->getXMLIFace()->NextSibling(m_pFXml));
	}
}

template <class T>
void CvXMLLoadUtility::LoadGlobalClassInfo(std::vector<T*>& aInfos, const char* szFileRoot, const char* szFileDirectory, const char* szXmlPath, bool bTwoPass, CvCacheObject* (CvDLLUtilityIFaceBase::*pArgFunction) (const TCHAR*))
{
	bool bLoaded = false;
	bool bWriteCache = true;
	CvCacheObject* pCache = NULL;
	GC.addToInfosVectors(&aInfos);

/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 05/17/08                                MRGENIE      */
/*                                                                                              */
/* This method is a replacement for the bTwoPass, if stuff that is depending on each other in   */
/* a loop, the bTwoPass would fail since it doesn't look first in the other Modules!            */
/************************************************************************************************/
	bool bTwoPassReplacement = true; // Use this if you wanna use the regular Firaxis bTwoPass
	if ( bTwoPassReplacement )
	{
		if (!bTwoPass )
		{
			bTwoPassReplacement = false;
		}
		bTwoPass = false;
	}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   10/10/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#ifdef _DEBUG
	logXmlCheckDoubleTypes("\nEntering: %s\n", szFileRoot);
#endif
	CvXMLLoadUtilitySetMod* pModEnumVector = new CvXMLLoadUtilitySetMod;
	CvXMLLoadUtilityModTools* p_szDirName = new CvXMLLoadUtilityModTools;
/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   END                                                  */
/************************************************************************************************/
	if (NULL != pArgFunction)
	{
		pCache = (gDLL->*pArgFunction)(CvString::format("%s.dat", szFileRoot));	// cache file name

		if (gDLL->cacheRead(pCache, CvString::format("xml\\\\%s\\\\%s.xml", szFileDirectory, szFileRoot)))
		{
			logMsg("Read %s from cache", szFileDirectory);
			bLoaded = true;
			bWriteCache = false;
		}
	}

	if (!bLoaded)
	{
		bLoaded = LoadCivXml(m_pFXml, CvString::format("xml\\%s/%s.xml", szFileDirectory, szFileRoot));

		if (!bLoaded)
		{
			char szMessage[1024];
			sprintf(szMessage, "LoadXML call failed for %s.", CvString::format("%s/%s.xml", szFileDirectory, szFileRoot).GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}
		else
		{
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 10/26/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			GC.setModDir("NONE");
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 END                                                  */
/************************************************************************************************/
			SetGlobalClassInfo(aInfos, szXmlPath, bTwoPass);
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 05/17/08                                MRGENIE      */
/*                                                                                              */
/* This method is a replacement for the bTwoPass, if stuff that is depending on each other in   */
/* a loop, the bTwoPass would fail since it doesn't look first in the other Modules!            */
/************************************************************************************************/
			// AIAndy: This early call to the second pass for the base XML does not allow for the base XML directory to be based directly on the module stuff
			//if ( bTwoPassReplacement )
			//{
			//	SetGlobalClassInfoTwoPassReplacement(aInfos, szXmlPath);
			//}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/

			if (gDLL->isModularXMLLoading())
			{
				std::vector<CvString> aszFiles;
				gDLL->enumerateFiles(aszFiles, CvString::format("modules\\*_%s.xml", szFileRoot));  // search for the modular files

				for (std::vector<CvString>::iterator it = aszFiles.begin(); it != aszFiles.end(); ++it)
				{
					bLoaded = LoadCivXml(m_pFXml, *it);

					if (!bLoaded)
					{
						char szMessage[1024];
						sprintf(szMessage, "LoadXML call failed for %s.", (*it).GetCString());
						gDLL->MessageBox(szMessage, "XML Load Error");
					}
					else
					{
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 10/26/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
						CvString szDirName = (*it).GetCString();	
						szDirName = p_szDirName->deleteFileName(szDirName, '\\');
						GC.setModDir(szDirName);
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 END                                                  */
/************************************************************************************************/
						SetGlobalClassInfo(aInfos, szXmlPath, bTwoPass);
					}
				}

				//AIAndy: Moved to this place so module stuff first pass is loaded before the second pass to the base XML
				if ( bTwoPassReplacement )
				{
					bLoaded = LoadCivXml(m_pFXml, CvString::format("xml\\%s/%s.xml", szFileDirectory, szFileRoot));

					if (!bLoaded)
					{
						char szMessage[1024];
						sprintf(szMessage, "LoadXML call failed for %s.", CvString::format("%s/%s.xml", szFileDirectory, szFileRoot).GetCString());
						gDLL->MessageBox(szMessage, "XML Load Error");
					}
					else
					{
						
						GC.setModDir("NONE");
						SetGlobalClassInfoTwoPassReplacement(aInfos, szXmlPath);
					}
				}

/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 05/17/08                                MRGENIE      */
/*                                                                                              */
/* This method is a replacement for the bTwoPass, if stuff that is depending on each other in   */
/* a loop, the bTwoPass would fail since it doesn't look first in the other Modules!            */
/************************************************************************************************/
				if ( bTwoPassReplacement )	// reloop through the modules!
				{
					for (std::vector<CvString>::iterator it = aszFiles.begin(); it != aszFiles.end(); ++it)
					{
						bLoaded = LoadCivXml(m_pFXml, *it);

						if (!bLoaded)
						{
							char szMessage[1024];
							sprintf(szMessage, "LoadXML call failed for %s.", (*it).GetCString());
							gDLL->MessageBox(szMessage, "XML Load Error");
						}
						else
						{

							CvString szDirName = (*it).GetCString();	
							szDirName = p_szDirName->deleteFileName(szDirName, '\\');
							GC.setModDir(szDirName);
							SetGlobalClassInfoTwoPassReplacement(aInfos, szXmlPath);

						}
					}
				}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
			}

/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 11/15/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			else
			{
				std::vector<CvString> aszFiles;
				aszFiles.reserve(10000);
				pModEnumVector->loadModControlArray(aszFiles, szFileRoot);

				for (std::vector<CvString>::iterator it = aszFiles.begin(); it != aszFiles.end(); ++it)
				{

					bLoaded = LoadCivXml(m_pFXml, *it);

					if (!bLoaded)
					{
						char szMessage[1024];
						sprintf(szMessage, "LoadXML call failed for %s.", (*it).GetCString());
						gDLL->MessageBox(szMessage, "XML Load Error");
					}
					else
					{
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 10/26/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
						CvString szDirName = (*it).GetCString();
						szDirName = p_szDirName->deleteFileName(szDirName, '\\');
						GC.setModDir(szDirName);
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 END                                                  */
/************************************************************************************************/
						SetGlobalClassInfo(aInfos, szXmlPath, bTwoPass);
					}
				}

				//AIAndy: Moved to this place so module stuff first pass is loaded before the second pass to the base XML
				if ( bTwoPassReplacement )
				{
					bLoaded = LoadCivXml(m_pFXml, CvString::format("xml\\%s/%s.xml", szFileDirectory, szFileRoot));

					if (!bLoaded)
					{
						char szMessage[1024];
						sprintf(szMessage, "LoadXML call failed for %s.", CvString::format("%s/%s.xml", szFileDirectory, szFileRoot).GetCString());
						gDLL->MessageBox(szMessage, "XML Load Error");
					}
					else
					{
						
						GC.setModDir("NONE");
						SetGlobalClassInfoTwoPassReplacement(aInfos, szXmlPath);
					}
				}

/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 05/17/08                                MRGENIE      */
/*                                                                                              */
/* This method is a replacement for the bTwoPass, if stuff that is depending on each other in   */
/* a loop, the bTwoPass would fail since it doesn't look first in the other Modules!            */
/************************************************************************************************/
				if ( bTwoPassReplacement )	// reloop through the modules!
				{
					for (std::vector<CvString>::iterator it = aszFiles.begin(); it != aszFiles.end(); ++it)
					{
						bLoaded = LoadCivXml(m_pFXml, *it);

						if (!bLoaded)
						{
							char szMessage[1024];
							sprintf(szMessage, "LoadXML call failed for %s.", (*it).GetCString());
							gDLL->MessageBox(szMessage, "XML Load Error");
						}
						else
						{

							CvString szDirName = (*it).GetCString();	
							szDirName = p_szDirName->deleteFileName(szDirName, '\\');
							GC.setModDir(szDirName);
							SetGlobalClassInfoTwoPassReplacement(aInfos, szXmlPath);

						}
					}
				}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
			}

			if (NULL != pArgFunction && bWriteCache)
			{
				// write info to cache
				bool bOk = gDLL->cacheWrite(pCache);
				if (!bOk)
				{
					char szMessage[1024];
					sprintf(szMessage, "Failed writing to %s cache. \n Current XML file is: %s", szFileDirectory, GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Caching Error");
				}
				if (bOk)
				{
					logMsg("Wrote %s to cache", szFileDirectory);
				}
			}
		}
	}

	if (NULL != pArgFunction)
	{
		gDLL->destroyCache(pCache);
	}
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 10/26/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	SAFE_DELETE(pModEnumVector);
	SAFE_DELETE(p_szDirName);
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
}

/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 05/13/08                                MRGENIE      */
/* we use this for xml's that should only be loaded as a module                                 */
/*                                                                                              */
/************************************************************************************************/
template <class T>
void CvXMLLoadUtility::LoadGlobalClassInfoModular(std::vector<T*>& aInfos, const char* szFileRoot, const char* szFileDirectory, const char* szXmlPath, bool bTwoPass)
{
	// we don't wanna use cache for modular loading, could do that, but safer just to load modules from scratch..
	bool bLoaded = false;
	GC.addToInfosVectors(&aInfos);

/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   10/10/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#ifdef _DEBUG	
	logXmlCheckDoubleTypes("\nEntering: %s\n", szFileRoot);
#endif
	CvXMLLoadUtilitySetMod* pModEnumVector = new CvXMLLoadUtilitySetMod;
	CvXMLLoadUtilityModTools* p_szDirName = new CvXMLLoadUtilityModTools;
/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   END                                                  */
/************************************************************************************************/

	std::vector<CvString> aszFiles;
	aszFiles.reserve(10000);
	pModEnumVector->loadModControlArray(aszFiles, szFileRoot);

	for (std::vector<CvString>::iterator it = aszFiles.begin(); it != aszFiles.end(); ++it)
	{

		bLoaded = LoadCivXml(m_pFXml, *it);

		if (!bLoaded)
		{
			char szMessage[1024];
			sprintf(szMessage, "LoadXML call failed for %s.", (*it).GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}
		else
		{
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 10/26/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			CvString szDirName = (*it).GetCString();	
			szDirName = p_szDirName->deleteFileName(szDirName, '\\');
			GC.setModDir(szDirName);
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 END                                                  */
/************************************************************************************************/
			SetGlobalClassInfo(aInfos, szXmlPath, bTwoPass);
		}
	}
	
	SAFE_DELETE(pModEnumVector);
	SAFE_DELETE(p_szDirName);
}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/

void CvXMLLoadUtility::LoadDiplomacyInfo(std::vector<CvDiplomacyInfo*>& DiploInfos, const char* szFileRoot, const char* szFileDirectory, const char* szXmlPath, CvCacheObject* (CvDLLUtilityIFaceBase::*pArgFunction) (const TCHAR*))
{
	bool bLoaded = false;
	bool bWriteCache = true;
	CvCacheObject* pCache = NULL;

	if (NULL != pArgFunction)
	{
		pCache = (gDLL->*pArgFunction)(CvString::format("%s.dat", szFileRoot));	// cache file name

		if (gDLL->cacheRead(pCache, CvString::format("xml\\\\%s\\\\%s.xml", szFileDirectory, szFileRoot)))
		{
			logMsg("Read %s from cache", szFileDirectory);
			bLoaded = true;
			bWriteCache = false;
		}
	}

	if (!bLoaded)
	{
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 10/26/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		CvXMLLoadUtilityModTools* p_szDirName = new CvXMLLoadUtilityModTools;
		CvXMLLoadUtilitySetMod* pModEnumVector = new CvXMLLoadUtilitySetMod;
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 END                                                  */
/************************************************************************************************/
		bLoaded = LoadCivXml(m_pFXml, CvString::format("xml\\%s/%s.xml", szFileDirectory, szFileRoot));

		if (!bLoaded)
		{
			char szMessage[1024];
			sprintf(szMessage, "LoadXML call failed for %s.", CvString::format("%s/%s.xml", szFileDirectory, szFileRoot).GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}
		else
		{
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 10/26/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			GC.setModDir("NONE");
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 END                                                  */
/************************************************************************************************/
			SetDiplomacyInfo(DiploInfos, szXmlPath);

			if (gDLL->isModularXMLLoading())
			{
				std::vector<CvString> aszFiles;
				gDLL->enumerateFiles(aszFiles, CvString::format("modules\\*_%s.xml", szFileRoot));  // search for the modular files

				for (std::vector<CvString>::iterator it = aszFiles.begin(); it != aszFiles.end(); ++it)
				{
					bLoaded = LoadCivXml(m_pFXml, *it);

					if (!bLoaded)
					{
						char szMessage[1024];
						sprintf(szMessage, "LoadXML call failed for %s.", (*it).GetCString());
						gDLL->MessageBox(szMessage, "XML Load Error");
					}
					else
					{
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 10/26/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
						CvString szDirName = (*it).GetCString();
						szDirName = p_szDirName->deleteFileName(szDirName, '\\');
						GC.setModDir(szDirName);
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 END                                                  */
/************************************************************************************************/
						SetDiplomacyInfo(DiploInfos, szXmlPath);
					}
				}
			}

/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 11/15/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			else
			{
				std::vector<CvString> aszFiles;
				aszFiles.reserve(10000);
				pModEnumVector->loadModControlArray(aszFiles, szFileRoot);

				for (std::vector<CvString>::iterator it = aszFiles.begin(); it != aszFiles.end(); ++it)
				{
					bLoaded = LoadCivXml(m_pFXml, *it);

					if (!bLoaded)
					{
						char szMessage[1024];
						sprintf(szMessage, "LoadXML call failed for %s.", (*it).GetCString());
						gDLL->MessageBox(szMessage, "XML Load Error");
					}
					else
					{
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 10/26/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
						CvString szDirName = (*it).GetCString();	
						szDirName = p_szDirName->deleteFileName(szDirName, '\\');
						GC.setModDir(szDirName);
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 END                                                  */
/************************************************************************************************/
						SetDiplomacyInfo(DiploInfos, szXmlPath);
					}
				}
			}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
			if (NULL != pArgFunction && bWriteCache)
			{
				// write info to cache
				bool bOk = gDLL->cacheWrite(pCache);
				if (!bOk)
				{
					char szMessage[1024];
					sprintf(szMessage, "Failed writing to %s cache. \n Current XML file is: %s", szFileDirectory, GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Caching Error");
				}
				if (bOk)
				{
					logMsg("Wrote %s to cache", szFileDirectory);
				}
			}
		}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 11/15/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		SAFE_DELETE(pModEnumVector);
		SAFE_DELETE(p_szDirName);
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
	}

	if (NULL != pArgFunction)
	{
		gDLL->destroyCache(pCache);
	}
}

//
// helper sort predicate
//

struct OrderIndex {int m_iPriority; int m_iIndex;};
bool sortHotkeyPriority(const OrderIndex orderIndex1, const OrderIndex orderIndex2)
{
	return (orderIndex1.m_iPriority > orderIndex2.m_iPriority);
}

void CvXMLLoadUtility::orderHotkeyInfo(int** ppiSortedIndex, int* pHotkeyIndex, int iLength)
{
	int iI;
	int* piSortedIndex;
	std::vector<OrderIndex> viOrderPriority;

	viOrderPriority.resize(iLength);
	piSortedIndex = *ppiSortedIndex;

	// set up vector
	for(iI=0;iI<iLength;iI++)
	{
		viOrderPriority[iI].m_iPriority = pHotkeyIndex[iI];
		viOrderPriority[iI].m_iIndex = iI;
	}

	// sort the array
	std::sort(viOrderPriority.begin(), viOrderPriority.end(), sortHotkeyPriority);

	// insert new order into the array to return
	for (iI=0;iI<iLength;iI++)
	{
		piSortedIndex[iI] = viOrderPriority[iI].m_iIndex;
	}
}

//
// helper sort predicate
//
/*
bool sortHotkeyPriorityOld(const CvHotkeyInfo* hotkey1, const CvHotkeyInfo* hotkey2)
{
return (hotkey1->getOrderPriority() < hotkey2->getOrderPriority());
}
*/

//------------------------------------------------------------------------------------------------
// FUNCTION:    void CvXMLLoadUtility::orderHotkeyInfoOld(T **ppHotkeyInfos, int iLength)
//! \brief      order a hotkey info derived class
//! \param      ppHotkeyInfos is a hotkey info derived class
//!							iLength is the length of the hotkey info derived class array
//! \retval
//------------------------------------------------------------------------------------------------
/*
template <class T>
void CvXMLLoadUtility::orderHotkeyInfoOld(T **ppHotkeyInfos, int iLength)
{
int iI;
std::vector<T*> vHotkeyInfo;
T* pHotkeyInfo;	// local pointer to the hotkey info memory

for (iI=0;iI<iLength;iI++)
{
pHotkeyInfo = new T;
*pHotkeyInfo = (*ppHotkeyInfos)[iI];
vHotkeyInfo.push_back(pHotkeyInfo);
}

std::sort(vHotkeyInfo.begin(), vHotkeyInfo.end(), sortHotkeyPriority);

for (iI=0;iI<iLength;iI++)
{
(*ppHotkeyInfos)[iI] = *vHotkeyInfo[iI];
}

for (iI=0;iI<(int)vHotkeyInfo.size();iI++)
{
vHotkeyInfo[iI]->reset();
SAFE_DELETE(vHotkeyInfo[iI]);
}
vHotkeyInfo.clear();
}
*/

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetYields(int** ppiYield)
//
//  PURPOSE :   Allocate memory for the yield parameter and set it to the values
//				in the xml file.  The current/last located node must be the first child of the
//				yield changes node
//
//------------------------------------------------------------------------------------------------------
int CvXMLLoadUtility::SetYields(int** ppiYield)
{
	int i=0;			//loop counter
	int iNumSibs=0;		// the number of siblings the current xml node has
	int *piYield;	// local pointer for the yield memory

	// Skip any comments and stop at the next value we might want
	if (SkipToNextVal())
	{
		// get the total number of children the current xml node has
		iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);

		InitList(ppiYield, NUM_YIELD_TYPES);

		// set the local pointer to the memory we just allocated
		piYield = *ppiYield;

		if (0 < iNumSibs)
		{
			// if the call to the function that sets the current xml node to it's first non-comment
			// child and sets the parameter with the new node's value succeeds
			if (GetChildXmlVal(&piYield[0]))
			{
				if(!(iNumSibs <= NUM_YIELD_TYPES))
				{
					char	szMessage[1024];
					sprintf( szMessage, "For loop iterator is greater than array size \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				// loop through all the siblings, we start at 1 since we already have the first value
				for (i=1;i<iNumSibs;i++)
				{
					// if the call to the function that sets the current xml node to it's first non-comment
					// sibling and sets the parameter with the new node's value does not succeed
					// we will break out of this for loop
					if (!GetNextXmlVal(&piYield[i]))
					{
						break;
					}
				}
				// set the current xml node to it's parent node
				gDLL->getXMLIFace()->SetToParent(m_pFXml);
			}
		}
	}

	return iNumSibs;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetFeatureStruct(int** ppiFeatureTech, int** ppiFeatureTime, int** ppiFeatureProduction, bool** ppbFeatureRemove, bool** ppbNoTechCanRemoveWithNoProductionGain)
//
//  PURPOSE :   allocate and set the feature struct variables for the CvBuildInfo class
//	Last Modified: Afforess, 5/25/10
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetFeatureStruct(int** ppiFeatureTech, int** ppiFeatureTime, int** ppiFeatureProduction, bool** ppbFeatureRemove, bool** ppbNoTechCanRemoveWithNoProductionGain)
{
	int i=0;				//loop counter
	int iNumSibs;					// the number of siblings the current xml node has
	int iFeatureIndex;
	TCHAR szTextVal[256];	// temporarily hold the text value of the current xml node
	int* paiFeatureTech = NULL;
	int* paiFeatureTime = NULL;
	int* paiFeatureProduction = NULL;
	bool* pabFeatureRemove = NULL;
	bool* pabNoTechCanRemoveWithNoProductionGain = NULL;

	if(GC.getNumFeatureInfos() < 1)
	{
		char	szMessage[1024];
		sprintf( szMessage, "no feature infos set yet! \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	InitList(ppiFeatureTech, GC.getNumFeatureInfos(), -1);
	InitList(ppiFeatureTime, GC.getNumFeatureInfos());
	InitList(ppiFeatureProduction, GC.getNumFeatureInfos());
	InitList(ppbFeatureRemove, GC.getNumFeatureInfos());
	InitList(ppbNoTechCanRemoveWithNoProductionGain, GC.getNumFeatureInfos());

	paiFeatureTech = *ppiFeatureTech;
	paiFeatureTime = *ppiFeatureTime;
	paiFeatureProduction = *ppiFeatureProduction;
	pabFeatureRemove = *ppbFeatureRemove;
	pabNoTechCanRemoveWithNoProductionGain = *ppbNoTechCanRemoveWithNoProductionGain;

	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,"FeatureStructs"))
	{
		iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);

		if (0 < iNumSibs)
		{
			if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,"FeatureStruct"))
			{
				if(!(iNumSibs <= GC.getNumFeatureInfos()))
				{
					char	szMessage[1024];
					sprintf( szMessage, "iNumSibs is greater than GC.getNumFeatureInfos in SetFeatureStruct function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				for (i=0;i<iNumSibs;i++)
				{
					GetChildXmlValByName(szTextVal, "FeatureType");
					iFeatureIndex = FindInInfoClass(szTextVal);
					if(!(iFeatureIndex != -1))
					{
						char	szMessage[1024];
						sprintf( szMessage, "iFeatureIndex is -1 inside SetFeatureStruct function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
						gDLL->MessageBox(szMessage, "XML Error");
					}
					GetChildXmlValByName(szTextVal, "PrereqTech");
					paiFeatureTech[iFeatureIndex] = FindInInfoClass(szTextVal);
					GetChildXmlValByName(&paiFeatureTime[iFeatureIndex], "iTime");
					GetChildXmlValByName(&paiFeatureProduction[iFeatureIndex], "iProduction");
					GetChildXmlValByName(&pabFeatureRemove[iFeatureIndex], "bRemove");
					GetChildXmlValByName(&pabNoTechCanRemoveWithNoProductionGain[iFeatureIndex], "bCanRemoveWithNoProductionGain", false);

					if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
					{
						break;
					}
				}

				gDLL->getXMLIFace()->SetToParent(m_pFXml);
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetImprovementBonuses(CvImprovementBonusInfo** ppImprovementBonus)
//
//  PURPOSE :   Allocate memory for the improvement bonus pointer and fill it based on the
//				values in the xml.
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetImprovementBonuses(CvImprovementBonusInfo** ppImprovementBonus)
{
	int i=0;				//loop counter
	int iNumSibs;			// the number of siblings the current xml node has
	TCHAR szNodeVal[256];	// temporarily holds the string value of the current xml node
	CvImprovementBonusInfo* paImprovementBonus;	// local pointer to the bonus type struct in memory

	// Skip any comments and stop at the next value we might want
	if (SkipToNextVal())
	{
		// initialize the boolean list to the correct size and all the booleans to false
		InitImprovementBonusList(ppImprovementBonus, GC.getNumBonusInfos());
		// set the local pointer to the memory we just allocated
		paImprovementBonus = *ppImprovementBonus;

		// get the total number of children the current xml node has
		iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
		// if we can set the current xml node to the child of the one it is at now
		if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
		{
			if(!(iNumSibs <= GC.getNumBonusInfos()))
			{
				char	szMessage[1024];
				sprintf( szMessage, "For loop iterator is greater than array size \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
				gDLL->MessageBox(szMessage, "XML Error");
			}
			// loop through all the siblings
			for (i=0;i<iNumSibs;i++)
			{
				// skip to the next non-comment node
				if (SkipToNextVal())
				{
					// call the function that sets the FXml pointer to the first non-comment child of
					// the current tag and gets the value of that new node
					if (GetChildXmlVal(szNodeVal))
					{
						int iBonusIndex;	// index of the match in the bonus types list
						// call the find in list function to return either -1 if no value is found
						// or the index in the list the match is found at
						iBonusIndex = FindInInfoClass(szNodeVal);
						// if we found a match we will get the next sibling's boolean value at that match's index
						if (iBonusIndex >= 0)
						{
							GetNextXmlVal(&paImprovementBonus[iBonusIndex].m_bBonusMakesValid);
							GetNextXmlVal(&paImprovementBonus[iBonusIndex].m_bBonusTrade);
							GetNextXmlVal(&paImprovementBonus[iBonusIndex].m_iDiscoverRand);
/************************************************************************************************/
/* Afforess	                  Start		 07/27/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
							GetNextXmlVal(&paImprovementBonus[iBonusIndex].m_iDepletionRand);
							//Impossible to tell if we failed to find the DepletionRand, other than nonsense values
							if (paImprovementBonus[iBonusIndex].m_iDepletionRand > 50000)
								paImprovementBonus[iBonusIndex].m_iDepletionRand = 0;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
							gDLL->getXMLIFace()->SetToParent(m_pFXml);

							SAFE_DELETE_ARRAY(paImprovementBonus[iBonusIndex].m_piYieldChange);	// free memory - MT, since we are about to reallocate
							if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,"YieldChanges"))
							{
								SetYields(&paImprovementBonus[iBonusIndex].m_piYieldChange);
								gDLL->getXMLIFace()->SetToParent(m_pFXml);
							}
							else
							{
								InitList(&paImprovementBonus[iBonusIndex].m_piYieldChange, NUM_YIELD_TYPES);
							}
						}
						else
						{
							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						// set the current xml node to it's parent node
					}

					// if we cannot set the current xml node to it's next sibling then we will break out of the for loop
					// otherwise we will continue looping
					if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
					{
						break;
					}
				}
			}
			// set the current xml node to it's parent node
			gDLL->getXMLIFace()->SetToParent(m_pFXml);
		}
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetAndLoadVar(int** ppiVar, int iDefault)
//
//  PURPOSE :   set the variable to a default and load it from the xml if there are any children
//
//------------------------------------------------------------------------------------------------------
bool CvXMLLoadUtility::SetAndLoadVar(int** ppiVar, int iDefault)
{
	int iNumSibs;
	int* piVar;
	bool bReturn = false;
	int i; // loop counter

	// Skip any comments and stop at the next value we might want
	if (SkipToNextVal())
	{
		bReturn = true;

		// get the total number of children the current xml node has
		iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);

		// allocate memory
		InitList(ppiVar, iNumSibs, iDefault);

		// set the a local pointer to the newly allocated memory
		piVar = *ppiVar;

		// if the call to the function that sets the current xml node to it's first non-comment
		// child and sets the parameter with the new node's value succeeds
		if (GetChildXmlVal(&piVar[0]))
		{
			// loop through all the siblings, we start at 1 since we already got the first sibling
			for (i=1;i<iNumSibs;i++)
			{
				// if the call to the function that sets the current xml node to it's next non-comment
				// sibling and sets the parameter with the new node's value does not succeed
				// we will break out of this for loop
				if (!GetNextXmlVal(&piVar[i]))
				{
					break;
				}
			}

			// set the current xml node to it's parent node
			gDLL->getXMLIFace()->SetToParent(m_pFXml);
		}
	}

	return bReturn;
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(	int **ppiList, const TCHAR* szRootTagName,
//										int iInfoBaseSize, int iInfoBaseLength, int iDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(int **ppiList, const TCHAR* szRootTagName,
											  int iInfoBaseSize, int iInfoBaseLength, int iDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	int* piList;

	if (0 > iInfoBaseLength)
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			if (0 < iNumSibs)
			{
				InitList(ppiList, iInfoBaseLength, iDefaultListVal);
				piList = *ppiList;
				if(!(iNumSibs <= iInfoBaseLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal = FindInInfoClass(szTextVal);

							if (iIndexVal != -1)
							{
								GetNextXmlVal(&piList[iIndexVal]);
							}

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

/************************************************************************************************/
/* RevDCM  XMLloading                             05/05/10             phungus420               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
//allows loading of a variable pair into an array by tag name
void CvXMLLoadUtility::SetVariableListTagPair(int **ppiList, const TCHAR* szRootTagName,
		int iInfoBaseSize, int iInfoBaseLength,
		const TCHAR* szValueTagName, int iValueInfoBaseLength, int iDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	int iValue;
	TCHAR szTextPosition[256];
	TCHAR szTextVal[256];
	int* piList = NULL;

	if (0 > iInfoBaseLength)
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}

	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			if (0 < iNumSibs)
			{
				InitList(ppiList, iInfoBaseLength, iDefaultListVal);
				piList = *ppiList;
				if(!(iNumSibs <= iInfoBaseLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextPosition))
						{
							iIndexVal = FindInInfoClass(szTextPosition);

							if (iIndexVal != -1)
							{
								gDLL->getXMLIFace()->SetToParent(m_pFXml);
								GetChildXmlValByName(szTextVal, szValueTagName);
								iValue = FindInInfoClass(szTextVal);
								if( (iValue < -1) || (iValue >= iValueInfoBaseLength) )
								{
									char	szMessage[1024];
									sprintf( szMessage, "A defined value for an array is outside of the accepted size of the infoclass!\n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
									gDLL->MessageBox(szMessage, "XML Error");
								} else
								{
									piList[iIndexVal] = iValue;
								}
							} else
							{
								gDLL->getXMLIFace()->SetToParent(m_pFXml);
							}
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}
/************************************************************************************************/
/* RevDCM	                                 END                                                */
/************************************************************************************************/

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(	bool **ppbList, const TCHAR* szRootTagName,
//										int iInfoBaseSize, int iInfoBaseLength, bool bDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(bool **ppbList, const TCHAR* szRootTagName,
											  int iInfoBaseSize, int iInfoBaseLength, bool bDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	bool* pbList;

	if(!(0 < iInfoBaseLength))
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			if (0 < iNumSibs)
			{
				InitList(ppbList, iInfoBaseLength, bDefaultListVal);
				pbList = *ppbList;
				if(!(iNumSibs <= iInfoBaseLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal = FindInInfoClass(szTextVal);
							if (iIndexVal != -1)
							{
								GetNextXmlVal(&pbList[iIndexVal]);
							}

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(	float **ppfList, const TCHAR* szRootTagName,
//										int iInfoBaseSize, int iInfoBaseLength, float fDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(float **ppfList, const TCHAR* szRootTagName,
											  int iInfoBaseSize, int iInfoBaseLength, float fDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	float* pfList;

	if(!(0 < iInfoBaseLength))
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			if (0 < iNumSibs)
			{
				InitList(ppfList, iInfoBaseLength, fDefaultListVal);
				pfList = *ppfList;
				if(!(iNumSibs <= iInfoBaseLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal = FindInInfoClass(szTextVal);
							if (iIndexVal != -1)
							{
								GetNextXmlVal(&pfList[iIndexVal]);
							}

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(	CvString **ppfList, const TCHAR* szRootTagName,
//										int iInfoBaseSize, int iInfoBaseLength, CvString szDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(CvString **ppszList, const TCHAR* szRootTagName,
											  int iInfoBaseSize, int iInfoBaseLength, CvString szDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	CvString* pszList;

	if(!(0 < iInfoBaseLength))
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			if (0 < iNumSibs)
			{
				InitStringList(ppszList, iInfoBaseLength, szDefaultListVal);
				pszList = *ppszList;
				if(!(iNumSibs <= iInfoBaseLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal = FindInInfoClass(szTextVal);
							if (iIndexVal != -1)
							{
								GetNextXmlVal(pszList[iIndexVal]);
							}

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(int **ppiList, const TCHAR* szRootTagName,
//										CvString* m_paszTagList, int iTagListLength, int iDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(int **ppiList, const TCHAR* szRootTagName,
											  CvString* m_paszTagList, int iTagListLength, int iDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	int* piList;

	if(!(0 < iTagListLength))
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			if (0 < iNumSibs)
			{
				InitList(ppiList, iTagListLength, iDefaultListVal);
				piList = *ppiList;
				if(!(iNumSibs <= iTagListLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal = GC.getTypesEnum(szTextVal);
							if (iIndexVal != -1)
							{
								GetNextXmlVal(&piList[iIndexVal]);
							}

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(int **ppiList, const TCHAR* szRootTagName,
//										CvString* m_paszTagList, int iTagListLength, int iDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml for audio scripts
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPairForAudioScripts(int **ppiList, const TCHAR* szRootTagName,
															 CvString* m_paszTagList, int iTagListLength, int iDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	int* piList;
	CvString szTemp;

	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			if(!(0 < iTagListLength))
			{
				char	szMessage[1024];
				sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPairForAudio \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
				gDLL->MessageBox(szMessage, "XML Error");
			}
			if (0 < iNumSibs)
			{
				InitList(ppiList, iTagListLength, iDefaultListVal);
				piList = *ppiList;
				if(!(iNumSibs <= iTagListLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPairForAudio \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal =	GC.getTypesEnum(szTextVal);
							if (iIndexVal != -1)
							{
								GetNextXmlVal(szTemp);
								if ( szTemp.GetLength() > 0 )
									piList[iIndexVal] = gDLL->getAudioTagIndex(szTemp);
								else
									piList[iIndexVal] = -1;
							}

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
/************************************************************************************************/
/* XMLCOPY                                 10/15/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	else
	{
		InitList(ppiList, iTagListLength, iDefaultListVal);
	}
/************************************************************************************************/
/* XMLCOPY                                 END                                                  */
/************************************************************************************************/
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPairForAudioScripts(int **ppiList, const TCHAR* szRootTagName,
//										int iInfoBaseLength, int iDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml for audio scripts
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPairForAudioScripts(int **ppiList, const TCHAR* szRootTagName, int iInfoBaseLength, int iDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	int* piList;
	CvString szTemp;

	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			if(!(0 < iInfoBaseLength))
			{
				char	szMessage[1024];
				sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
				gDLL->MessageBox(szMessage, "XML Error");
			}
			if (0 < iNumSibs)
			{
				InitList(ppiList, iInfoBaseLength, iDefaultListVal);
				piList = *ppiList;
				if(!(iNumSibs <= iInfoBaseLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal = FindInInfoClass(szTextVal);
							if (iIndexVal != -1)
							{
								GetNextXmlVal(szTemp);
								if ( szTemp.GetLength() > 0 )
									piList[iIndexVal] = gDLL->getAudioTagIndex(szTemp);
								else
									piList[iIndexVal] = -1;
							}

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
/************************************************************************************************/
/* XMLCOPY                                 10/12/07                                MRGENIE      */
/*                                                                                              */
/* Always create this array, for XML copy comparison                                            */
/************************************************************************************************/
	else
	{
		InitList(ppiList, iInfoBaseLength, iDefaultListVal);
	}
/************************************************************************************************/
/* XMLCOPY                                 END                                                  */
/************************************************************************************************/
}

//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetVariableListTagPair(bool **ppbList, const TCHAR* szRootTagName,
//										CvString* m_paszTagList, int iTagListLength, int iDefaultListVal)
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(bool **ppbList, const TCHAR* szRootTagName,
											  CvString* m_paszTagList, int iTagListLength, bool bDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	bool* pbList;

	if(!(0 < iTagListLength))
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			if (0 < iNumSibs)
			{
				InitList(ppbList, iTagListLength, bDefaultListVal);
				pbList = *ppbList;
				if(!(iNumSibs <= iTagListLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal =	GC.getTypesEnum(szTextVal);
							if (iIndexVal != -1)
							{
								GetNextXmlVal(&pbList[iIndexVal]);
							}

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

//------------------------------------------------------------------------------------------------------
//
//	FUNCTION:	SetVariableListTagPair(CvString **ppszList, const TCHAR* szRootTagName,
//							CvString* m_paszTagList, int iTagListLength, CvString szDefaultListVal = "")
//
//  PURPOSE :   allocate and initialize a list from a tag pair in the xml
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetVariableListTagPair(CvString **ppszList, const TCHAR* szRootTagName,
											  CvString* m_paszTagList, int iTagListLength, CvString szDefaultListVal)
{
	int i;
	int iIndexVal;
	int iNumSibs;
	TCHAR szTextVal[256];
	CvString* pszList;

	if(!(0 < iTagListLength))
	{
		char	szMessage[1024];
		sprintf( szMessage, "Allocating zero or less memory in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
		gDLL->MessageBox(szMessage, "XML Error");
	}
	if (gDLL->getXMLIFace()->SetToChildByTagName(m_pFXml,szRootTagName))
	{
		if (SkipToNextVal())
		{
			iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
			if (0 < iNumSibs)
			{
				InitStringList(ppszList, iTagListLength, szDefaultListVal);
				pszList = *ppszList;
				if(!(iNumSibs <= iTagListLength))
				{
					char	szMessage[1024];
					sprintf( szMessage, "There are more siblings than memory allocated for them in CvXMLLoadUtility::SetVariableListTagPair \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
					gDLL->MessageBox(szMessage, "XML Error");
				}
				if (gDLL->getXMLIFace()->SetToChild(m_pFXml))
				{
					for (i=0;i<iNumSibs;i++)
					{
						if (GetChildXmlVal(szTextVal))
						{
							iIndexVal =	GC.getTypesEnum(szTextVal);
							if (iIndexVal != -1)
							{
								GetNextXmlVal(pszList[iIndexVal]);
							}

							gDLL->getXMLIFace()->SetToParent(m_pFXml);
						}

						if (!gDLL->getXMLIFace()->NextSibling(m_pFXml))
						{
							break;
						}
					}

					gDLL->getXMLIFace()->SetToParent(m_pFXml);
				}
			}
		}

		gDLL->getXMLIFace()->SetToParent(m_pFXml);
	}
}

DllExport bool CvXMLLoadUtility::LoadPlayerOptions()
{
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 10/30/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	// Python Modular Loading
	if (!CreateFXml())
		return false;
	LoadPythonModulesInfo(GC.getPythonModulesInfo(), "CIV4PythonModulesInfos", "Civ4PythonModulesInfos/PythonModulesInfos/PythonModulesInfo", false);
	DestroyFXml();

	// MLF loading
	if (!gDLL->isModularXMLLoading())
	{
		if (!CreateFXml())
			return false;

		ModularLoadingControlXML();

		DestroyFXml();
	}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
	if (!CreateFXml())
		return false;

	LoadGlobalClassInfo(GC.getPlayerOptionInfo(), "CIV4PlayerOptionInfos", "GameInfo", "Civ4PlayerOptionInfos/PlayerOptionInfos/PlayerOptionInfo", false);
	FAssert(GC.getNumPlayerOptionInfos() == NUM_PLAYEROPTION_TYPES);

	DestroyFXml();
	return true;
}

DllExport bool CvXMLLoadUtility::LoadGraphicOptions()
{
	if (!CreateFXml())
		return false;

	LoadGlobalClassInfo(GC.getGraphicOptionInfo(), "CIV4GraphicOptionInfos", "GameInfo", "Civ4GraphicOptionInfos/GraphicOptionInfos/GraphicOptionInfo", false);
	FAssert(GC.getNumGraphicOptions() == NUM_GRAPHICOPTION_TYPES);

	DestroyFXml();
	return true;
}


/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 10/24/07                            MRGENIE          */
/*                                                                                              */
/* Initialize the list of Module Directories to be loaded                                       */
/************************************************************************************************/
// Python Modular Loading
template <class T>
void CvXMLLoadUtility::LoadPythonModulesInfo(std::vector<T*>& aInfos, const char* szFileRoot, const char* szXmlPath, bool bTwoPass)
{
	bool bLoaded = false;
	GC.addToInfosVectors(&aInfos);

//	XML_CHECK_DOUBLE_TYPE						10/10/07							MRGENIE
#ifdef _DEBUG	
	logXmlCheckDoubleTypes("Entering: %s\n", szFileRoot);
#endif
//	XML_CHECK_DOUBLE_TYPE						END   
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 03/28/08                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	GC.setModDir("NONE");
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 END                                                  */
/************************************************************************************************/
	
	std::vector<CvString> aszFiles;
	gDLL->enumerateFiles(aszFiles, CvString::format("Python Config\\*_%s.xml", szFileRoot));  // search for the modular files

	for (std::vector<CvString>::iterator it = aszFiles.begin(); it != aszFiles.end(); ++it)
	{
		bLoaded = LoadCivXml(m_pFXml, *it);

		if (!bLoaded)
		{
			char szMessage[1024];
			sprintf(szMessage, "LoadXML call failed for %s.", (*it).GetCString());
			gDLL->MessageBox(szMessage, "XML Load Error");
		}
		else
		{
			SetGlobalClassInfo(aInfos, szXmlPath, bTwoPass);
		}
	}
}
// Main control of the MLF feature
void CvXMLLoadUtility::ModularLoadingControlXML()
{
	CvXMLLoadUtilitySetMod* pSetMod = new CvXMLLoadUtilitySetMod;
	pSetMod->setModLoadControlDirArray(LoadModLoadControlInfo(GC.getModLoadControlInfos(), "CIV4ModularLoadingControls", "Type"));
	SAFE_DELETE(pSetMod);
}


// In the next 2 methods we load the MLF classes
template <class T>
bool CvXMLLoadUtility::LoadModLoadControlInfo(std::vector<T*>& aInfos, const char* szFileRoot, const char* szXmlPath)
{
	GC.addToInfosVectors(&aInfos);
	logMLF("Entering MLF");		//logging
/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   03/14/08                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#ifdef _DEBUG	
	logXmlCheckDoubleTypes("\nEntering: MLF_CIV4ModularLoadingControls\n");
#endif
/************************************************************************************************/
/* XML_CHECK_DOUBLE_TYPE                   END                                                  */
/************************************************************************************************/

	bool bLoaded = false;
	bool bContinue = true;
	int m_iDirDepth = 0;

	CvXMLLoadUtilityModTools* pProgramDir = new CvXMLLoadUtilityModTools;

	std::string szDirDepth = "modules\\";
	std::string szModDirectory = "modules";
	std::string szConfigString;

	bLoaded = LoadCivXml(m_pFXml, CvString::format("%s\\MLF_%s.xml", szModDirectory.c_str(), szFileRoot));

	if (!bLoaded)
	{
		logMLF("MLF not found, you will now load the modules without Modular Loading Control");  //logging
		return false;
	}	
	else
	{
		if ( gDLL->getXMLIFace()->LocateNode(m_pFXml,"Civ4ModularLoadControls/DefaultConfiguration"))
		{
			// call the function that sets the FXml pointer to the first non-comment child of
			// the current tag and gets the value of that new node
			GetChildXmlVal(szConfigString, "NONE");
			if (szConfigString == "NONE")
			{
				logMLF("The default configuration in \"%s\\MLF_%s.xml\" was set to \"NONE\", you will continue loading the regular Firaxian method", szModDirectory.c_str(), szFileRoot);
				return false;   // abort without enumerating anything
			}
		}
		else 
		{
			logMLF("The default configuration in \"%s\\MLF_%s.xml\" couldn't be found, you will continue loading using the regular Firaxian method", szModDirectory.c_str(), szFileRoot);
			return false;
		}

		if (!SetModLoadControlInfo(aInfos, szXmlPath, szConfigString, szDirDepth, m_iDirDepth))
		{
			logMLF("The default configuration in \"%s\\MLF_%s.xml\" set by you could not be found, please check your XML settings!", szModDirectory.c_str(), szFileRoot);
			return false;
		}

		//	We want a top to bottom control mechanism. If in any new level there wasn't found a
		//	new MLF we don't want to loop further downwards into the directory hyrarchy 
		while (bContinue)
		{
			m_iDirDepth++;
			bContinue = false;	//we want to stop the while loop, unless a new MLF will be found
			//loop through all MLF's so far loaded
			for ( int iInfos = 0; iInfos < GC.getNumModLoadControlInfos(); iInfos++)
			{	
				//only loop through files in the actual depth
				if (GC.getModLoadControlInfos(iInfos).getDirDepth() + 1 == m_iDirDepth)
				{
					//loop through the modules of each MLF
					for ( int i = 0; i < GC.getModLoadControlInfos(iInfos).getNumModules(); i++ )
					{
						if ( GC.getModLoadControlInfos(iInfos).isLoad(i) )
						{

							//each new loop we load the previous dir, and check if a MLF file exist on a lower level
							szModDirectory = GC.getModLoadControlInfos(iInfos).getModuleFolder(i);					
							
							//Check if this Modulefolder is parent to a child MLF							
							if ( pProgramDir->isModularArt(CvString::format("%s\\MLF_%s.xml", szModDirectory.c_str(), szFileRoot)))
							{
								bLoaded = LoadCivXml(m_pFXml, CvString::format("%s\\MLF_%s.xml", szModDirectory.c_str(), szFileRoot));
							}
							else
							{
								bLoaded = false;
							}

							if (!bLoaded)
							{
								logMLF( "Found module: \"%s\\\"", szModDirectory.c_str() );
							}
							else
							{
								if ( gDLL->getXMLIFace()->LocateNode(m_pFXml,"Civ4ModularLoadControls/DefaultConfiguration"))
								{
									// call the function that sets the FXml pointer to the first non-comment child of
									// the current tag and gets the value of that new node
									GetChildXmlVal(szConfigString, "NONE");
									if (szConfigString == "NONE") 
									{
										logMLF("The default configuration in \"%s\\MLF_%s.xml\" was set to \"NONE\", settings in this file will be disregarded", szModDirectory.c_str(), szFileRoot);
									}
									else
									{
										szDirDepth = CvString::format("%s\\", szModDirectory.c_str());
										SetModLoadControlInfo(aInfos, szXmlPath, szConfigString.c_str(), szDirDepth.c_str(), m_iDirDepth);
										bContinue = true; //found a new MLF in a subdir, continue the loop
									}
								}
							}
						}
					}
				}
			}
		}		
	}
	SAFE_DELETE(pProgramDir);
	return true;
}

template <class T>
bool CvXMLLoadUtility::SetModLoadControlInfo(std::vector<T*>& aInfos, const char* szTagName, CvString szConfigString, CvString szDirDepth, int iDirDepth)
{
	std::string szCandidateConfig;

	if ( gDLL->getXMLIFace()->LocateNode(m_pFXml,"Civ4ModularLoadControls/ConfigurationInfos/ConfigurationInfo"))
	{		
		// loop through each tag
		do
		{
			SkipToNextVal();	// skip to the next non-comment node

			szCandidateConfig = "NONE";
			GetChildXmlValByName(szCandidateConfig, szTagName);
			if (szCandidateConfig == szConfigString)
			{
				T* pClassInfo = new T;

				FAssert(NULL != pClassInfo);
				if (NULL == pClassInfo)
				{
					break;
				}

				bool bSuccess = pClassInfo->read(this, szDirDepth, iDirDepth);
				FAssert(bSuccess);
				if (!bSuccess)
				{
					delete pClassInfo;
					break;
				}

				int iIndex = -1;
				if (NULL != pClassInfo->getType())
				{
					iIndex = GC.getInfoTypeForString(pClassInfo->getType(), true);
					if ( iIndex != -1 )
					{
						logMLF("This type \"%s\" is double", pClassInfo->getType());
						//Catch dupes here, we don't want the overwrite or copy method for the MLF
						CvString szFAssertMsg = CvString::format("The <type>%s</type> of the \"MLF_CIV4ModularLoadingControls.xml\" in directory: \"%s\" is already in use, please use an alternative <type> -name",pClassInfo->getType(), szDirDepth.c_str());
						FAssertMsg(iIndex == -1, szFAssertMsg);
						return false;
					}
				}

				aInfos.push_back(pClassInfo);
				if (NULL != pClassInfo->getType())
				{
					GC.setInfoTypeFromString(pClassInfo->getType(), (int)aInfos.size() - 1);	// add type to global info type hash map
					return true;
				}				
			}				
		} while (gDLL->getXMLIFace()->NextSibling(m_pFXml));
	}
	return false;
}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* SORT_ALPHABET                           11/17/07                                MRGENIE      */
/*                                                                                              */
/* Method for alphabetically order tags                                                         */
/************************************************************************************************/
template <class T>
void CvXMLLoadUtility::SortAlphabetGlobalClassInfo(std::vector<T*>& aInfos)
{
	int iLenghtInfos = aInfos.size();	//Number of Elements
	T* pTempClassInfo = new T;	// Temp to store while swapping positions

	for ( int iI = 0; iI < iLenghtInfos; iI++ )
	{
		int minIndex = iI; //stores index of the min array value		
	
		for ( int iJ = iI + 1; iJ < iLenghtInfos; iJ++ )
		{
			if ( CvWString::format(aInfos[iJ]->getTextKeyWide()) < CvWString::format(aInfos[minIndex]->getTextKeyWide()) )
			{
				minIndex = iJ;
			}
		}
	    
		//swap the Info classes at positions iI and minIndex		
		pTempClassInfo = aInfos[minIndex];
		aInfos[minIndex] = aInfos[iI];
		aInfos[iI] = pTempClassInfo;		// This is the actual new order

		// Set the InfoTypeFromString map properly according to the new alphabetical order
		if (NULL != aInfos[iI]->getType())
		{
			// overwrite existing info maps with the proper id's
			GC.setInfoTypeFromString(aInfos[iI]->getType(), iI);
		}
	}
}
/************************************************************************************************/
/* SORT_ALPHABET                           END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 11/30/07                                MRGENIE      */
/*                                                                                              */
/* Savegame compatibility                                                                       */
/************************************************************************************************/
bool CvXMLLoadUtility::doResetGlobalInfoClasses()
{
	// PlayerOptions reloaded seperatly because of the MLF Array initialization
	if (!CreateFXml())
		return false;

	LoadGlobalClassInfo(GC.getPlayerOptionInfo(), "CIV4PlayerOptionInfos", "GameInfo", "Civ4PlayerOptionInfos/PlayerOptionInfos/PlayerOptionInfo", false);
	FAssert(GC.getNumPlayerOptionInfos() == NUM_PLAYEROPTION_TYPES);

	DestroyFXml();

	LoadGraphicOptions();
	SetGlobalDefines();
//	LoadGlobalText();   //Thus far everything loaded anyway, it's just text
	SetGlobalTypes();

	return true;
}
bool CvXMLLoadUtility::doResetInfoClasses()
{
	LoadBasicInfos();
	LoadPreMenuGlobals();
	SetPostGlobalsGlobalDefines();
	SetupGlobalLandscapeInfo();
	LoadPostMenuGlobals();

	return true;
}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* TGA_INDEXATION                          01/21/08                                MRGENIE      */
/*                                                                                              */
/* reorganizing the Corporations and Religions vectors                                          */
/************************************************************************************************/
template <class T>
void CvXMLLoadUtility::ArrangeTGA(std::vector<T*>& aInfos, const char* szInfo)
{
	int iMinIndex;
	CvString szDebugBuffer;

	T* pTempClassInfo = new T;	// Temp to store while swapping positions

	FAssert(NULL != pTempClassInfo);
	if (NULL == pTempClassInfo)
	{
		return;
	}

	CvString verify = CvString::format("%s", szInfo).GetCString();  //string comparison

	if ( verify == "CIV4ReligionInfo" )
	{
#ifdef DEBUG_TGA_LOADING
		GC.logInfoTypeMap("Before religion game font rearranging");
		// loop through the religions see what we've got
		szDebugBuffer.Format("=== BEFORE %s TGA indices. ===", szInfo);
		gDLL->logMsg("CvXMLLoadUtility_ArrangeTGA.log", szDebugBuffer.c_str());
		for (std::vector<T*>::const_iterator it = aInfos.begin(); it != aInfos.end(); it++)
		{
			szDebugBuffer.Format("* Religion %s: TGA index %i; type ID %i", (*it)->getType(), (*it)->getTGAIndex(), GC.getInfoTypeForString((*it)->getType()));
			gDLL->logMsg("CvXMLLoadUtility_ArrangeTGA.log", szDebugBuffer.c_str());

		}
#endif
		for ( int iReligion = 0; iReligion < (int)aInfos.size(); iReligion++ )
		{
			iMinIndex = iReligion;

			// if we found the Religion with the proper GTAIndex, make sure it's on the right spot in the aInfos vector
			for ( int iReligionCheck = iReligion + 1; iReligionCheck < (int)aInfos.size(); iReligionCheck++ )
			{
				if ( GC.getReligionInfo((ReligionTypes)iReligionCheck).getTGAIndex() < GC.getReligionInfo((ReligionTypes)iMinIndex).getTGAIndex() )
				{
					iMinIndex = iReligionCheck;
				}
			}

			if ( iMinIndex != iReligion )
			{
#ifdef DEBUG_TGA_LOADING
				szDebugBuffer.Format(" * Found candidates to swap: %i <-> %i", iMinIndex, iReligion);
				gDLL->logMsg("CvXMLLoadUtility_ArrangeTGA.log", szDebugBuffer.c_str());
#endif

				int iOldType1 = GC.getInfoTypeForString(aInfos[iReligion]->getType());
				int iOldType2 = GC.getInfoTypeForString(aInfos[iMinIndex]->getType());

#ifdef DEBUG_TGA_LOADING
				szDebugBuffer.Format(" -- Old info types: %s: %i <-> %s: %i", aInfos[iReligion]->getType(), iOldType1, aInfos[iMinIndex]->getType(), iOldType2);
				gDLL->logMsg("CvXMLLoadUtility_ArrangeTGA.log", szDebugBuffer.c_str());
#endif

				GC.setInfoTypeFromString(aInfos[iReligion]->getType(), iOldType2);
				GC.setInfoTypeFromString(aInfos[iMinIndex]->getType(), iOldType1);

				pTempClassInfo = aInfos[iReligion];			//store in temp value
				aInfos[iReligion] = aInfos[iMinIndex];		//move iReligion to proper place
				aInfos[iMinIndex] = pTempClassInfo;			//finish the swap

				// Set the InfoTypeFromString map properly according to the new alphabetical order
				/*if (NULL != aInfos[iReligion]->getType())
				{*/
					// overwrite existing info maps with the proper id's
					//GC.setInfoTypeFromString(aInfos[iReligion]->getType(), iMinIndex, true);
				/*}
				if (NULL != aInfos[iMinIndex]->getType())
				{*/
					// overwrite existing info maps with the proper id's
					//GC.setInfoTypeFromString(aInfos[iMinIndex]->getType(), iReligion, true);
				//}

#ifdef DEBUG_TGA_LOADING
				szDebugBuffer.Format(" -- NEW info types: %s: %i <-> %s: %i", aInfos[iReligion]->getType(), GC.getInfoTypeForString(aInfos[iReligion]->getType()), aInfos[iMinIndex]->getType(), GC.getInfoTypeForString(aInfos[iMinIndex]->getType()));
				gDLL->logMsg("CvXMLLoadUtility_ArrangeTGA.log", szDebugBuffer.c_str());
#endif
			}
			}
#ifdef DEBUG_TGA_LOADING
		szDebugBuffer.Format("=== AFTER %s TGA indices. ===", szInfo);
		gDLL->logMsg("CvXMLLoadUtility_ArrangeTGA.log", szDebugBuffer.c_str());
		for (std::vector<T*>::const_iterator it = aInfos.begin(); it != aInfos.end(); it++)
		{
			szDebugBuffer.Format("* Religion %s: TGA index %i; type ID %i", (*it)->getType(), (*it)->getTGAIndex(), GC.getInfoTypeForString((*it)->getType()));
			gDLL->logMsg("CvXMLLoadUtility_ArrangeTGA.log", szDebugBuffer.c_str());

		}

		GC.logInfoTypeMap("After religion game font rearranging");
#endif
	}


	if ( verify == "CIV4CorporationInfo" )
	{
		// loop through the religions see what we've got
		for ( int iCorporation = 0; iCorporation < (int)aInfos.size(); iCorporation++ )
		{
			iMinIndex = iCorporation;

			// if we found the Religion with the proper GTAIndex, make sure it's on the right spot in the aInfos vector
			for ( int iCorporationCheck = iCorporation + 1; iCorporationCheck < (int)aInfos.size(); iCorporationCheck++ )
			{
				if ( GC.getCorporationInfo((CorporationTypes)iCorporationCheck).getTGAIndex() < GC.getCorporationInfo((CorporationTypes)iMinIndex).getTGAIndex() )
				{
					iMinIndex = iCorporationCheck;
				}
			}

			if ( iMinIndex != iCorporation )
			{
				pTempClassInfo = aInfos[iCorporation];			//store in temp value
				aInfos[iCorporation] = aInfos[iMinIndex];		//move iReligion to proper place
				aInfos[iMinIndex] = pTempClassInfo;			//finish the swap
				// Set the InfoTypeFromString map properly according to the new alphabetical order
				if (NULL != aInfos[iCorporation]->getType())
				{
					// overwrite existing info maps with the proper id's
					GC.setInfoTypeFromString(aInfos[iCorporation]->getType(), iMinIndex);
				}
				if (NULL != aInfos[iMinIndex]->getType())
				{
					// overwrite existing info maps with the proper id's
					GC.setInfoTypeFromString(aInfos[iMinIndex]->getType(), iCorporation);
				}
			}
		}
	}
}

template <class T>
void CvXMLLoadUtility::AddTGABogus(std::vector<T*>& aInfos, const char* szInfo)
{
	bool bTGAInfoTypeValid = false;

	T* pTempClassInfo = new T;	// Temp to store while swapping positions
	T* pClassInfo = new T;		// Bogus InfoType

	FAssert(NULL != pTempClassInfo);
	FAssert(NULL != pClassInfo);
	if (NULL == pClassInfo || NULL == pTempClassInfo)
	{
		return;
	}

	CvString verify = CvString::format("%s", szInfo).GetCString();  //string comparison

	if ( verify == "CIV4ReligionInfo" )
	{
		for ( int iI = 0; iI < TGA_RELIGIONS; iI++ )
		{
			bTGAInfoTypeValid = false;
			// loop through the religions see what we've got
			for ( int iReligion = 0; iReligion < (int)aInfos.size(); iReligion++ )
			{
				// if we found the Religion with the proper GTAIndex, make sure it's on the right spot in the aInfos vector
				if ( GC.getReligionInfo((ReligionTypes)iReligion).getTGAIndex() == iI )
				{
					bTGAInfoTypeValid = true;
				}
			}
			if ( !bTGAInfoTypeValid)
			{
				aInfos.insert(aInfos.begin() + iI, pClassInfo);
			}
		}
	}

	if ( verify == "CIV4CorporationInfo" )
	{
		for ( int iI = 0; iI < TGA_CORPORATIONS; iI++ )
		{
			bTGAInfoTypeValid = false;
			// loop through the religions see what we've got
			for ( int iCorporation = 0; iCorporation < (int)aInfos.size(); iCorporation++ )
			{
				// if we found the Religion with the proper GTAIndex, make sure it's on the right spot in the aInfos vector
				if ( GC.getCorporationInfo((CorporationTypes)iCorporation).getTGAIndex() == iI )
				{
					bTGAInfoTypeValid = true;
				}
			}
			if ( !bTGAInfoTypeValid)
			{
				aInfos.insert(aInfos.begin() + iI, pClassInfo);
			}
		}
	}
}

// Made an own little function for this one in case we have to clean up any further info vectors in future
// principly any one can be added here in future, if everything is ok, nothing will be done anyway
void CvXMLLoadUtility::cleanTGA()
{
	RemoveTGABogusReligion(GC.getReligionInfo());
	RemoveTGABogusCorporation(GC.getCorporationInfo());
}

template <class T>
void CvXMLLoadUtility::RemoveTGABogusReligion(std::vector<T*>& aInfos)
{
	std::vector <int> viDeletionVector;

	for ( int iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if ( NULL == GC.getReligionInfo((ReligionTypes)iI).getType() )
		{
			viDeletionVector.push_back(iI);
		}
	}

	for ( int iI = (int)viDeletionVector.size() - 1; iI >= 0; iI--)	// gotta do this backwards
	{
		aInfos.erase( aInfos.begin()+viDeletionVector.at(iI));
	}
}

template <class T>
void CvXMLLoadUtility::RemoveTGABogusCorporation(std::vector<T*>& aInfos)
{
	std::vector <int> viDeletionVector;

	for ( int iI = 0; iI < GC.getNumCorporationInfos(); iI++)
	{
		if ( NULL == GC.getCorporationInfo((CorporationTypes)iI).getType() )
		{
			viDeletionVector.push_back(iI);
		}
	}

	for ( int iI = (int)viDeletionVector.size() - 1; iI >= 0; iI--)	// gotta do this backwards
	{
		aInfos.erase( aInfos.begin()+viDeletionVector.at(iI));
	}
}
/************************************************************************************************/
/* TGA_INDEXATION                          END                                                  */
/************************************************************************************************/

/*************************************************************************************************/
/**	New Tag Defs	(XMLInfos)				08/09/08								Xienwolf	**/
/**																								**/
/**									Loads Information from XML									**/
/*************************************************************************************************/
//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetChildList(int** ppiYield)
//
//  PURPOSE :   Generates a Vector String containing all data from single entry children fields
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetStringWithChildList(int* iNumEntries, std::vector<CvString>* aszXMLLoad)
{
    std::vector<CvString> paszXMLLoad;
	CvString szTextVal;

    if (SkipToNextVal())
    {
        int iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
        if (0 < iNumSibs)
        {
            if (GetChildXmlVal(szTextVal))
            {
                for (int iI = 0; iI < iNumSibs; iI++)
                {
					bool bLoad = true;
					int iSize = paszXMLLoad.size();
					for (int iJ = 0; iJ < iSize; ++iJ)
					{
						if(szTextVal == paszXMLLoad[iJ])
						{
							bLoad = false;
						}
					}
					if (bLoad)
					{
	                    paszXMLLoad.push_back(szTextVal);
					}
                    if (!GetNextXmlVal(szTextVal))
                    {
                        break;
                    }
                }

                gDLL->getXMLIFace()->SetToParent(m_pFXml);
            }
        }
    }
    gDLL->getXMLIFace()->SetToParent(m_pFXml);

	*iNumEntries = paszXMLLoad.size();
	*aszXMLLoad = paszXMLLoad;
	paszXMLLoad.clear();
}
//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetIntWithChildList(int* iNumEntries, int** piXMLLoad)
//
//  PURPOSE :   Generates an Int Array containing all data from single entry children fields
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetIntWithChildList(int* iNumEntries, int** piXMLLoad)
{
	int* ppiXMLLoad;
	CvString szTextVal;
	std::vector<int> szTemp;

    if (SkipToNextVal())
    {
        int iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
        if (iNumSibs > 0)
        {
            if (GetChildXmlVal(szTextVal))
            {
                for (int iI = 0; iI < iNumSibs; iI++)
                {
                    int iNew = FindInInfoClass(szTextVal);
					if(iNew == -1)
					{
						char szMessage[1024];
						sprintf( szMessage, "Index is -1 inside function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
						gDLL->MessageBox(szMessage, "XML Error");
					}
					else
					{
						bool bLoad = true;
						int iSize = szTemp.size();
						for (int iJ = 0; iJ < iSize; ++iJ)
						{
							if(iNew == szTemp[iJ])
							{
								bLoad = false;
							}
						}
						if (bLoad)
						{
							szTemp.push_back(iNew);
						}
					}
                    if (!GetNextXmlVal(szTextVal))
                    {
                        break;
                    }
                }

                gDLL->getXMLIFace()->SetToParent(m_pFXml);
            }
        }
    }
	gDLL->getXMLIFace()->SetToParent(m_pFXml);

	int iSize = szTemp.size();
	*iNumEntries = iSize;
	//Redo function to use and return a list, then build the array from that list
	ppiXMLLoad = new int[iSize];
	*piXMLLoad = new int[iSize];
	ppiXMLLoad = *piXMLLoad;
	for (int i = 0; i < iSize; ++i)
	{
		ppiXMLLoad[i] = szTemp[i];
	}
	szTemp.clear();
}
//------------------------------------------------------------------------------------------------------
//
//  FUNCTION:   SetBoolFromChildList(int iNumEntries, bool** pbXMLLoad)
//
//  PURPOSE :   Generates an Bool Array containing True in any listed field
//
//------------------------------------------------------------------------------------------------------
void CvXMLLoadUtility::SetBoolFromChildList(int iNumEntries, bool** pbXMLLoad)
{
    bool* ppbXMLLoad = NULL;
	ppbXMLLoad = *pbXMLLoad;
	CvString szTextVal;
    if (SkipToNextVal())
    {
        int iNumSibs = gDLL->getXMLIFace()->GetNumChildren(m_pFXml);
        if (iNumEntries < iNumSibs)
		{
			char	szMessage[1024];
            sprintf( szMessage, "Too many Children values \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
            gDLL->MessageBox(szMessage, "XML Error");
		}
        if (iNumSibs > 0)
        {
			if (GetChildXmlVal(szTextVal))
            {
                for (int iI = 0; iI < iNumSibs; iI++)
                {
                    int eLoad = FindInInfoClass(szTextVal);
					if(eLoad == -1)
					{
						char	szMessage[1024];
						sprintf( szMessage, "Index is -1 inside function \n Current XML file is: %s", GC.getCurrentXMLFile().GetCString());
						gDLL->MessageBox(szMessage, "XML Error");
					}
                    ppbXMLLoad[eLoad] = true;
                    if (!GetNextXmlVal(szTextVal))
                    {
                        break;
                    }
                }

                gDLL->getXMLIFace()->SetToParent(m_pFXml);
            }
        }
    }

    gDLL->getXMLIFace()->SetToParent(m_pFXml);
}
/*************************************************************************************************/
/**	New Tag Defs							END													**/
/*************************************************************************************************/
