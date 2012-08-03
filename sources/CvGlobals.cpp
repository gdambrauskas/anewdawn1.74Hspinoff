//
// globals.cpp
//
#include "CvGameCoreDLL.h"
#include "cvGlobals.h"
#include "CvRandom.h"
#include "CvGameAI.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvMap.h"
#include "CvPlayerAI.h"
#include "CvTeamAI.h"
#include "CvInfos.h"
#include "CvDLLUtilityIFaceBase.h"
#include "CvArtFileMgr.h"
#include "CvDLLXMLIFaceBase.h"
#include "CvPlayerAI.h"
#include "CvInfoWater.h"
#include "CvGameTextMgr.h"
#include "FProfiler.h"
#include "FVariableSystem.h"
#include "CvInitCore.h"
/************************************************************************************************/
/* Afforess	                  Start		 08/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
#include "CvMessageControl.h"
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


// BUG - start
// RevolutionDCM - BugMod included in cvGlobals.h
//#include "BugMod.h"
// BUG - end

// BUG - BUG Info - start
#include "CvBugOptions.h"
// BUG - BUG Info - end

// BUFFY - DLL Info - start
#ifdef _BUFFY
#include "Buffy.h"
#endif
// BUFFY - DLL Info - end


/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 11/30/07                                MRGENIE      */
/*                                                                                              */
/* Savegame compatibility                                                                       */
/************************************************************************************************/
#include "CvXMLLoadUtility.h"
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/

#define COPY(dst, src, typeName) \
	{ \
		int iNum = sizeof(src)/sizeof(typeName); \
		dst = new typeName[iNum]; \
		for (int i =0;i<iNum;i++) \
			dst[i] = src[i]; \
	}

template <class T>
void deleteInfoArray(std::vector<T*>& array)
{
	for (std::vector<T*>::iterator it = array.begin(); it != array.end(); ++it)
	{
		SAFE_DELETE(*it);
	}

	array.clear();
}

template <class T>
bool readInfoArray(FDataStreamBase* pStream, std::vector<T*>& array, const char* szClassName)
{
	GC.addToInfosVectors(&array);

	int iSize;
	pStream->Read(&iSize);
	FAssertMsg(iSize==sizeof(T), CvString::format("class size doesn't match cache size - check info read/write functions:%s", szClassName).c_str());
	if (iSize!=sizeof(T))
		return false;
	pStream->Read(&iSize);

	deleteInfoArray(array);

	for (int i = 0; i < iSize; ++i)
	{
		array.push_back(new T);
	}

	int iIndex = 0;
	for (std::vector<T*>::iterator it = array.begin(); it != array.end(); ++it)
	{
		(*it)->read(pStream);
		GC.setInfoTypeFromString((*it)->getType(), iIndex);
		++iIndex;
	}

	return true;
}

template <class T>
bool writeInfoArray(FDataStreamBase* pStream,  std::vector<T*>& array)
{
	int iSize = sizeof(T);
	pStream->Write(iSize);
	pStream->Write(array.size());
	for (std::vector<T*>::iterator it = array.begin(); it != array.end(); ++it)
	{
		(*it)->write(pStream);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

CvGlobals gGlobalsProxy;	// for debugging
cvInternalGlobals* gGlobals = NULL;
CvDLLUtilityIFaceBase* g_DLL = NULL;

int giProfilerDisabled = 0;  // set to 1 or more in threaded areas as the profiler is not thread safe

#ifdef _DEBUG
int inDLL = 0;
const char* fnName = NULL;

//	Wrapper for debugging so as to be able to always tell last method entered
ProxyTracker::ProxyTracker(const CvGlobals* proxy, const char* name)
{
	inDLL++;
	fnName = name;

	proxy->CheckProxy(name);
}

ProxyTracker::~ProxyTracker()
{
	inDLL--;
	fnName = NULL;
}
#endif

void CvGlobals::CheckProxy(const char* fnName) const
{
	//OutputDebugString(fnName);
	//OutputDebugString("\n");

	if ( gGlobals == NULL )
	{
		OutputDebugString("Method called prior to global instantiation\n");

		::MessageBoxA(NULL,"Method called prior to global instantiation",":CvGameCore",MB_OK);
		//throw new <exception>;
	}
}

//
// CONSTRUCTOR
//
cvInternalGlobals::cvInternalGlobals() :
/************************************************************************************************/
/* Mod Globals    Start                          09/13/10                           phungus420  */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
m_paszEntityEventTypes2(NULL),
m_paszEntityEventTypes(NULL),
m_paszAnimationOperatorTypes(NULL),
m_paszFunctionTypes(NULL),
m_paszFlavorTypes(NULL),
m_paszArtStyleTypes(NULL),
m_paszCitySizeTypes(NULL),
m_paszContactTypes(NULL),
m_paszDiplomacyPowerTypes(NULL),
m_paszAutomateTypes(NULL),
m_paszDirectionTypes(NULL),
m_paszFootstepAudioTypes(NULL),
m_paszFootstepAudioTags(NULL),
m_bDCM_BATTLE_EFFECTS(false),
m_iBATTLE_EFFECT_LESS_FOOD(0),
m_iBATTLE_EFFECT_LESS_PRODUCTION(0),
m_iBATTLE_EFFECT_LESS_COMMERCE(0),
m_iBATTLE_EFFECTS_MINIMUM_TURN_INCREMENTS(0),
m_iMAX_BATTLE_TURNS(0),

m_bDCM_AIR_BOMBING(false),
m_bDCM_RANGE_BOMBARD(false),
m_iDCM_RB_CITY_INACCURACY(0),
m_iDCM_RB_CITYBOMBARD_CHANCE(0),
m_bDCM_ATTACK_SUPPORT(false),
m_bDCM_STACK_ATTACK(false),
m_bDCM_OPP_FIRE(false),
m_bDCM_ACTIVE_DEFENSE(false),
m_bDCM_ARCHER_BOMBARD(false),
m_bDCM_FIGHTER_ENGAGE(false),

m_bDYNAMIC_CIV_NAMES(false),

m_bLIMITED_RELIGIONS_EXCEPTIONS(false),
m_bOC_RESPAWN_HOLY_CITIES(false),

m_bIDW_ENABLED(false),
m_fIDW_BASE_COMBAT_INFLUENCE(0),
m_fIDW_NO_CITY_DEFENDER_MULTIPLIER(1.0f),
m_fIDW_FORT_CAPTURE_MULTIPLIER(1.0f),
m_fIDW_EXPERIENCE_FACTOR(0),
m_fIDW_WARLORD_MULTIPLIER(1.0f),
m_iIDW_INFLUENCE_RADIUS(0),
m_fIDW_PLOT_DISTANCE_FACTOR(0),
m_fIDW_WINNER_PLOT_MULTIPLIER(1.0f),
m_fIDW_LOSER_PLOT_MULTIPLIER(1.0f),
m_bIDW_EMERGENCY_DRAFT_ENABLED(false),
m_iIDW_EMERGENCY_DRAFT_MIN_POPULATION(2),
m_fIDW_EMERGENCY_DRAFT_STRENGTH(1.0f),
m_fIDW_EMERGENCY_DRAFT_ANGER_MULTIPLIER(0),
m_bIDW_NO_BARBARIAN_INFLUENCE(false),
m_bIDW_NO_NAVAL_INFLUENCE(false),
m_bIDW_PILLAGE_INFLUENCE_ENABLED(false),
m_fIDW_BASE_PILLAGE_INFLUENCE(0),
m_fIDW_CITY_TILE_MULTIPLIER(0),

m_bSS_ENABLED(false),
m_bSS_BRIBE(false),
m_bSS_ASSASSINATE(false),
/************************************************************************************************/
/* Mod Globals                        END                                           phungus420  */
/************************************************************************************************/
m_bGraphicsInitialized(false),
m_bLogging(false),
m_bRandLogging(false),
m_bOverwriteLogs(false),
m_bSynchLogging(false),
m_bDLLProfiler(false),
m_pkMainMenu(NULL),
m_iNewPlayers(0),
m_bZoomOut(false),
m_bZoomIn(false),
m_bLoadGameFromFile(false),
m_pFMPMgr(NULL),
m_asyncRand(NULL),
m_interface(NULL),
m_game(NULL),
m_messageQueue(NULL),
m_hotJoinMsgQueue(NULL),
m_messageControl(NULL),
m_messageCodes(NULL),
m_dropMgr(NULL),
m_portal(NULL),
m_setupData(NULL),
m_initCore(NULL),
m_statsReporter(NULL),
m_map(NULL),
m_diplomacyScreen(NULL),
m_mpDiplomacyScreen(NULL),
m_pathFinder(NULL),
m_interfacePathFinder(NULL),
m_stepFinder(NULL),
m_routeFinder(NULL),
m_borderFinder(NULL),
m_areaFinder(NULL),
m_plotGroupFinder(NULL),
m_aiPlotDirectionX(NULL),
m_aiPlotDirectionY(NULL),
m_aiPlotCardinalDirectionX(NULL),
m_aiPlotCardinalDirectionY(NULL),
m_aiCityPlotX(NULL),
m_aiCityPlotY(NULL),
m_aiCityPlotPriority(NULL),
m_aeTurnLeftDirection(NULL),
m_aeTurnRightDirection(NULL),
//m_aGameOptionsInfo(NULL),
//m_aPlayerOptionsInfo(NULL),
m_Profiler(NULL),
m_VarSystem(NULL),
m_iMOVE_DENOMINATOR(0),
m_iNUM_UNIT_PREREQ_OR_BONUSES(0),
m_iNUM_BUILDING_PREREQ_OR_BONUSES(0),
m_iFOOD_CONSUMPTION_PER_POPULATION(0),
m_iMAX_HIT_POINTS(0),
m_iPATH_DAMAGE_WEIGHT(0),
m_iHILLS_EXTRA_DEFENSE(0),
m_iRIVER_ATTACK_MODIFIER(0),
m_iAMPHIB_ATTACK_MODIFIER(0),
m_iHILLS_EXTRA_MOVEMENT(0),
m_iMAX_PLOT_LIST_ROWS(0),
m_iUNIT_MULTISELECT_MAX(0),
m_iPERCENT_ANGER_DIVISOR(0),
m_iEVENT_MESSAGE_TIME(0),
m_iROUTE_FEATURE_GROWTH_MODIFIER(0),
m_iFEATURE_GROWTH_MODIFIER(0),
m_iMIN_CITY_RANGE(0),
m_iCITY_MAX_NUM_BUILDINGS(0),
m_iNUM_UNIT_AND_TECH_PREREQS(0),
m_iNUM_AND_TECH_PREREQS(0),
m_iNUM_OR_TECH_PREREQS(0),
m_iLAKE_MAX_AREA_SIZE(0),
m_iNUM_ROUTE_PREREQ_OR_BONUSES(0),
m_iNUM_BUILDING_AND_TECH_PREREQS(0),
m_iMIN_WATER_SIZE_FOR_OCEAN(0),
m_iFORTIFY_MODIFIER_PER_TURN(0),
m_iMAX_CITY_DEFENSE_DAMAGE(0),
m_iNUM_CORPORATION_PREREQ_BONUSES(0),
m_iPEAK_SEE_THROUGH_CHANGE(0),
m_iHILLS_SEE_THROUGH_CHANGE(0),
m_iSEAWATER_SEE_FROM_CHANGE(0),
m_iPEAK_SEE_FROM_CHANGE(0),
m_iHILLS_SEE_FROM_CHANGE(0),
m_iUSE_SPIES_NO_ENTER_BORDERS(0),
m_fCAMERA_MIN_YAW(0),
m_fCAMERA_MAX_YAW(0),
m_fCAMERA_FAR_CLIP_Z_HEIGHT(0),
m_fCAMERA_MAX_TRAVEL_DISTANCE(0),
m_fCAMERA_START_DISTANCE(0),
m_fAIR_BOMB_HEIGHT(0),
m_fPLOT_SIZE(0),
m_fCAMERA_SPECIAL_PITCH(0),
m_fCAMERA_MAX_TURN_OFFSET(0),
m_fCAMERA_MIN_DISTANCE(0),
m_fCAMERA_UPPER_PITCH(0),
m_fCAMERA_LOWER_PITCH(0),
m_fFIELD_OF_VIEW(0),
m_fSHADOW_SCALE(0),
m_fUNIT_MULTISELECT_DISTANCE(0),
m_iUSE_CANNOT_FOUND_CITY_CALLBACK(0),
m_iUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK(0),
m_iUSE_IS_PLAYER_RESEARCH_CALLBACK(0),
m_iUSE_CAN_RESEARCH_CALLBACK(0),
m_iUSE_CANNOT_DO_CIVIC_CALLBACK(0),
m_iUSE_CAN_DO_CIVIC_CALLBACK(0),
m_iUSE_CANNOT_CONSTRUCT_CALLBACK(0),
m_iUSE_CAN_CONSTRUCT_CALLBACK(0),
m_iUSE_CAN_DECLARE_WAR_CALLBACK(0),
m_iUSE_CANNOT_RESEARCH_CALLBACK(0),
m_iUSE_GET_UNIT_COST_MOD_CALLBACK(0),
m_iUSE_GET_CITY_FOUND_VALUE_CALLBACK(0),
m_iUSE_CANNOT_HANDLE_ACTION_CALLBACK(0),
m_iUSE_CAN_BUILD_CALLBACK(0),
m_iUSE_CANNOT_TRAIN_CALLBACK(0),
m_iUSE_CAN_TRAIN_CALLBACK(0),
m_iUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK(0),
m_iUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK(0),
m_iUSE_FINISH_TEXT_CALLBACK(0),
m_iUSE_ON_UNIT_SET_XY_CALLBACK(0),
m_iUSE_ON_UNIT_SELECTED_CALLBACK(0),
m_iUSE_ON_UPDATE_CALLBACK(0),
m_iUSE_ON_UNIT_CREATED_CALLBACK(0),
m_iUSE_ON_UNIT_LOST_CALLBACK(0),
/************************************************************************************************/
/* MODULES                                 11/13/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
m_iTGA_RELIGIONS(0),                            // GAMEFONT_TGA_RELIGIONS
m_iTGA_CORPORATIONS(0),                         // GAMEFONT_TGA_CORPORATIONS
/************************************************************************************************/
/* MODULES                                 END                                                  */
/************************************************************************************************/
m_paHints(NULL),
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 10/30/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
// MLF loading
m_paModLoadControlVector(NULL),
m_paModLoadControls(NULL),
// Modular loading Dependencies
m_bAnyDependency(false),
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 03/28/08                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
m_paMainMenus(NULL)
*/
m_paMainMenus(NULL),
m_cszModDir("NONE")
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* Afforess	                  Start		 12/8/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
,m_iPEAK_EXTRA_MOVEMENT(0)
,m_iPEAK_EXTRA_DEFENSE(0)
,m_bFormationsMod(false)
,m_bLoadedPlayerOptions(false)
,m_bXMLLogging(false)
,m_iSCORE_FREE_PERCENT(0)
,m_iSCORE_POPULATION_FACTOR(0)
,m_iSCORE_LAND_FACTOR(0)
,m_iSCORE_TECH_FACTOR(0)
,m_iSCORE_WONDER_FACTOR(0)

//New Python Callbacks
,m_iUSE_CAN_CREATE_PROJECT_CALLBACK(0)
,m_iUSE_CANNOT_CREATE_PROJECT_CALLBACK(0)
,m_iUSE_CAN_DO_MELTDOWN_CALLBACK(0)
,m_iUSE_CAN_MAINTAIN_PROCESS_CALLBACK(0)
,m_iUSE_CANNOT_MAINTAIN_PROCESS_CALLBACK(0)
,m_iUSE_CAN_DO_GROWTH_CALLBACK(0)
,m_iUSE_CAN_DO_CULTURE_CALLBACK(0)
,m_iUSE_CAN_DO_PLOT_CULTURE_CALLBACK(0)
,m_iUSE_CAN_DO_PRODUCTION_CALLBACK(0)
,m_iUSE_CAN_DO_RELIGION_CALLBACK(0)
,m_iUSE_CAN_DO_GREATPEOPLE_CALLBACK(0)
,m_iUSE_CAN_RAZE_CITY_CALLBACK(0)
,m_iUSE_CAN_DO_GOLD_CALLBACK(0)
,m_iUSE_CAN_DO_RESEARCH_CALLBACK(0)
,m_iUSE_UPGRADE_UNIT_PRICE_CALLBACK(0)
,m_iUSE_IS_VICTORY_CALLBACK(0)
,m_iUSE_AI_UPDATE_UNIT_CALLBACK(0)
,m_iUSE_AI_CHOOSE_PRODUCTION_CALLBACK(0)
,m_iUSE_EXTRA_PLAYER_COSTS_CALLBACK(0)
,m_iUSE_AI_DO_DIPLO_CALLBACK(0)
,m_iUSE_AI_BESTTECH_CALLBACK(0)
,m_iUSE_CAN_DO_COMBAT_CALLBACK(0)
,m_iUSE_AI_CAN_DO_WARPLANS_CALLBACK(0)

/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Efficiency, Options                                                                          */
/************************************************************************************************/
// BBAI Options
,m_bBBAI_AIR_COMBAT(false)
,m_bBBAI_HUMAN_VASSAL_WAR_BUILD(false)
,m_iBBAI_DEFENSIVE_PACT_BEHAVIOR(0)
,m_bBBAI_HUMAN_AS_VASSAL_OPTION(false)

// BBAI AI Variables
,m_iWAR_SUCCESS_CITY_CAPTURING(25)
,m_iBBAI_ATTACK_CITY_STACK_RATIO(110)
,m_iBBAI_SKIP_BOMBARD_BEST_ATTACK_ODDS(12)
,m_iBBAI_SKIP_BOMBARD_BASE_STACK_RATIO(300)
,m_iBBAI_SKIP_BOMBARD_MIN_STACK_RATIO(140)

// Tech Diffusion
,m_bTECH_DIFFUSION_ENABLE(false)
,m_iTECH_DIFFUSION_KNOWN_TEAM_MODIFIER(30)
,m_iTECH_DIFFUSION_WELFARE_THRESHOLD(88)
,m_iTECH_DIFFUSION_WELFARE_MODIFIER(30)
,m_iTECH_COST_FIRST_KNOWN_PREREQ_MODIFIER(20)
,m_iTECH_COST_KNOWN_PREREQ_MODIFIER(20)
,m_iTECH_COST_MODIFIER(0)

// From Lead From Behind by UncutDragon
// Lead from Behind flags
,m_bLFBEnable(false)
,m_iLFBBasedOnGeneral(1)
,m_iLFBBasedOnExperience(1)
,m_iLFBBasedOnLimited(1)
,m_iLFBBasedOnHealer(1)
,m_iLFBBasedOnAverage(1)
,m_bLFBUseSlidingScale(true)
,m_iLFBAdjustNumerator(1)
,m_iLFBAdjustDenominator(3)
,m_bLFBUseCombatOdds(true)
,m_iCOMBAT_DIE_SIDES(-1)
,m_iCOMBAT_DAMAGE(-1)
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
{
}

cvInternalGlobals::~cvInternalGlobals()
{
}


/************************************************************************************************/
/* MINIDUMP_MOD                           04/10/11                                terkhen       */
/*                                                                                              */
/* See http://www.debuginfo.com/articles/effminidumps.html                                      */
/************************************************************************************************/
#define MINIDUMP
#ifdef MINIDUMP

#include <dbghelp.h>
#pragma comment (lib, "dbghelp.lib")

void CreateMiniDump(EXCEPTION_POINTERS *pep)
{
	/* Open a file to store the minidump. */
	HANDLE hFile = CreateFile(_T("MiniDump.dmp"),
	                          GENERIC_READ | GENERIC_WRITE,
	                          0,
	                          NULL,
	                          CREATE_ALWAYS,
	                          FILE_ATTRIBUTE_NORMAL,
	                          NULL);

	if((hFile == NULL) || (hFile == INVALID_HANDLE_VALUE)) {
		_tprintf(_T("CreateFile failed. Error: %u \n"), GetLastError());
		return;
	}
	/* Create the minidump. */
	MINIDUMP_EXCEPTION_INFORMATION mdei;

	mdei.ThreadId           = GetCurrentThreadId();
	mdei.ExceptionPointers  = pep;
	mdei.ClientPointers     = FALSE;

	MINIDUMP_TYPE mdt       = MiniDumpNormal;

	BOOL result = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
	                                hFile,
	                                mdt,
	                                (pep != NULL) ? &mdei : NULL,
	                                NULL,
	                                NULL);

	/* Close the file. */
	CloseHandle(hFile);
}

LONG WINAPI CustomFilter(EXCEPTION_POINTERS *ExceptionInfo)
{
	CreateMiniDump(ExceptionInfo);
	return EXCEPTION_EXECUTE_HANDLER;
}

#endif
/************************************************************************************************/
/* MINIDUMP_MOD                                END                                              */
/************************************************************************************************/

//
// allocate
//
void cvInternalGlobals::init()
{
/************************************************************************************************/
/* MINIDUMP_MOD                           04/10/11                                terkhen       */
/************************************************************************************************/

#ifdef MINIDUMP
	/* Enable our custom exception that will write the minidump for us. */
	SetUnhandledExceptionFilter(CustomFilter);
#endif

/************************************************************************************************/
/* MINIDUMP_MOD                                END                                              */
/************************************************************************************************/

	//
	// These vars are used to initialize the globals.
	//

	int aiPlotDirectionX[NUM_DIRECTION_TYPES] =
	{
		0,	// DIRECTION_NORTH
		1,	// DIRECTION_NORTHEAST
		1,	// DIRECTION_EAST
		1,	// DIRECTION_SOUTHEAST
		0,	// DIRECTION_SOUTH
		-1,	// DIRECTION_SOUTHWEST
		-1,	// DIRECTION_WEST
		-1,	// DIRECTION_NORTHWEST
	};

	int aiPlotDirectionY[NUM_DIRECTION_TYPES] =
	{
		1,	// DIRECTION_NORTH
		1,	// DIRECTION_NORTHEAST
		0,	// DIRECTION_EAST
		-1,	// DIRECTION_SOUTHEAST
		-1,	// DIRECTION_SOUTH
		-1,	// DIRECTION_SOUTHWEST
		0,	// DIRECTION_WEST
		1,	// DIRECTION_NORTHWEST
	};

	int aiPlotCardinalDirectionX[NUM_CARDINALDIRECTION_TYPES] =
	{
		0,	// CARDINALDIRECTION_NORTH
		1,	// CARDINALDIRECTION_EAST
		0,	// CARDINALDIRECTION_SOUTH
		-1,	// CARDINALDIRECTION_WEST
	};

	int aiPlotCardinalDirectionY[NUM_CARDINALDIRECTION_TYPES] =
	{
		1,	// CARDINALDIRECTION_NORTH
		0,	// CARDINALDIRECTION_EAST
		-1,	// CARDINALDIRECTION_SOUTH
		0,	// CARDINALDIRECTION_WEST
	};

	int aiCityPlotX[NUM_CITY_PLOTS] =
	{
		0,
		0, 1, 1, 1, 0,-1,-1,-1,
		0, 1, 2, 2, 2, 1, 0,-1,-2,-2,-2,-1,
/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09                                                        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		0, 1, 2, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1,
/************************************************************************************************/
/* JOOYO_ADDON                          END                                                     */
/************************************************************************************************/
	};

	int aiCityPlotY[NUM_CITY_PLOTS] =
	{
		0,
		1, 1, 0,-1,-1,-1, 0, 1,
		2, 2, 1, 0,-1,-2,-2,-2,-1, 0, 1, 2,
/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09                                                        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1, 0, 1, 2, 3,
/************************************************************************************************/
/* JOOYO_ADDON                          END                                                     */
/************************************************************************************************/
	};

	int aiCityPlotPriority[NUM_CITY_PLOTS] =
	{
		0,
		1, 2, 1, 2, 1, 2, 1, 2,
		3, 4, 4, 3, 4, 4, 3, 4, 4, 3, 4, 4,
/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09                                                        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		5, 6, 7, 6, 5, 6, 7, 6, 5, 6, 7, 6, 5, 6, 7, 6,
/************************************************************************************************/
/* JOOYO_ADDON                          END                                                     */
/************************************************************************************************/
	};

	int aaiXYCityPlot[CITY_PLOTS_DIAMETER][CITY_PLOTS_DIAMETER] =
	{
/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/17/09                                                        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	    {-1, -1, 32, 33, 34, -1, -1},
	    {-1, 31, 17, 18, 19, 35, -1},
	    {30, 16, 6,   7,  8, 20, 36},
	    {29, 15, 5,   0,  1,  9, 21},
	    {28, 14, 4,   3,  2, 10, 22},
	    {-1, 27, 13, 12, 11, 23, -1},
	    {-1, -1, 26, 25, 24, -1, -1},
/************************************************************************************************/
/* JOOYO_ADDON                          END                                                     */
/************************************************************************************************/
	};

	DirectionTypes aeTurnRightDirection[NUM_DIRECTION_TYPES] =
	{
		DIRECTION_NORTHEAST,	// DIRECTION_NORTH
		DIRECTION_EAST,				// DIRECTION_NORTHEAST
		DIRECTION_SOUTHEAST,	// DIRECTION_EAST
		DIRECTION_SOUTH,			// DIRECTION_SOUTHEAST
		DIRECTION_SOUTHWEST,	// DIRECTION_SOUTH
		DIRECTION_WEST,				// DIRECTION_SOUTHWEST
		DIRECTION_NORTHWEST,	// DIRECTION_WEST
		DIRECTION_NORTH,			// DIRECTION_NORTHWEST
	};

	DirectionTypes aeTurnLeftDirection[NUM_DIRECTION_TYPES] =
	{
		DIRECTION_NORTHWEST,	// DIRECTION_NORTH
		DIRECTION_NORTH,			// DIRECTION_NORTHEAST
		DIRECTION_NORTHEAST,	// DIRECTION_EAST
		DIRECTION_EAST,				// DIRECTION_SOUTHEAST
		DIRECTION_SOUTHEAST,	// DIRECTION_SOUTH
		DIRECTION_SOUTH,			// DIRECTION_SOUTHWEST
		DIRECTION_SOUTHWEST,	// DIRECTION_WEST
		DIRECTION_WEST,				// DIRECTION_NORTHWEST
	};

	DirectionTypes aaeXYDirection[DIRECTION_DIAMETER][DIRECTION_DIAMETER] =
	{
		DIRECTION_SOUTHWEST, DIRECTION_WEST,	DIRECTION_NORTHWEST,
		DIRECTION_SOUTH,     NO_DIRECTION,    DIRECTION_NORTH,
		DIRECTION_SOUTHEAST, DIRECTION_EAST,	DIRECTION_NORTHEAST,
	};

	FAssertMsg(gDLL != NULL, "Civ app needs to set gDLL");

	m_VarSystem = new FVariableSystem;
	m_asyncRand = new CvRandom;
	m_initCore = new CvInitCore;
	m_loadedInitCore = new CvInitCore;
	m_iniInitCore = new CvInitCore;

	gDLL->initGlobals();	// some globals need to be allocated outside the dll

	m_game = new CvGameAI;
	m_map = new CvMap;

	CvPlayerAI::initStatics();
	CvTeamAI::initStatics();

	m_pt3Origin = NiPoint3(0.0f, 0.0f, 0.0f);

	COPY(m_aiPlotDirectionX, aiPlotDirectionX, int);
	COPY(m_aiPlotDirectionY, aiPlotDirectionY, int);
	COPY(m_aiPlotCardinalDirectionX, aiPlotCardinalDirectionX, int);
	COPY(m_aiPlotCardinalDirectionY, aiPlotCardinalDirectionY, int);
	COPY(m_aiCityPlotX, aiCityPlotX, int);
	COPY(m_aiCityPlotY, aiCityPlotY, int);
	COPY(m_aiCityPlotPriority, aiCityPlotPriority, int);
	COPY(m_aeTurnLeftDirection, aeTurnLeftDirection, DirectionTypes);
	COPY(m_aeTurnRightDirection, aeTurnRightDirection, DirectionTypes);
	memcpy(m_aaiXYCityPlot, aaiXYCityPlot, sizeof(m_aaiXYCityPlot));
	memcpy(m_aaeXYDirection, aaeXYDirection,sizeof(m_aaeXYDirection));
}

//
// free
//
void cvInternalGlobals::uninit()
{
	//
	// See also CvXMLLoadUtilityInit.cpp::CleanUpGlobalVariables()
	//
	SAFE_DELETE_ARRAY(m_aiPlotDirectionX);
	SAFE_DELETE_ARRAY(m_aiPlotDirectionY);
	SAFE_DELETE_ARRAY(m_aiPlotCardinalDirectionX);
	SAFE_DELETE_ARRAY(m_aiPlotCardinalDirectionY);
	SAFE_DELETE_ARRAY(m_aiCityPlotX);
	SAFE_DELETE_ARRAY(m_aiCityPlotY);
	SAFE_DELETE_ARRAY(m_aiCityPlotPriority);
	SAFE_DELETE_ARRAY(m_aeTurnLeftDirection);
	SAFE_DELETE_ARRAY(m_aeTurnRightDirection);

	SAFE_DELETE(m_game);
	SAFE_DELETE(m_map);

	CvPlayerAI::freeStatics();
	CvTeamAI::freeStatics();

	SAFE_DELETE(m_asyncRand);
	SAFE_DELETE(m_initCore);
	SAFE_DELETE(m_loadedInitCore);
	SAFE_DELETE(m_iniInitCore);
	gDLL->uninitGlobals();	// free globals allocated outside the dll
	SAFE_DELETE(m_VarSystem);

	// already deleted outside of the dll, set to null for safety
	m_messageQueue=NULL;
	m_hotJoinMsgQueue=NULL;
	m_messageControl=NULL;
	m_setupData=NULL;
	m_messageCodes=NULL;
	m_dropMgr=NULL;
	m_portal=NULL;
	m_statsReporter=NULL;
	m_interface=NULL;
	m_diplomacyScreen=NULL;
	m_mpDiplomacyScreen=NULL;
	m_pathFinder=NULL;
	m_interfacePathFinder=NULL;
	m_stepFinder=NULL;
	m_routeFinder=NULL;
	m_borderFinder=NULL;
	m_areaFinder=NULL;
	m_plotGroupFinder=NULL;

	m_typesMap.clear();
	m_aInfoVectors.clear();
}

void cvInternalGlobals::clearTypesMap()
{
	m_typesMap.clear();
	if (m_VarSystem)
	{
		m_VarSystem->UnInit();
	}
}


CvDiplomacyScreen* cvInternalGlobals::getDiplomacyScreen()
{
	return m_diplomacyScreen;
}

CMPDiplomacyScreen* cvInternalGlobals::getMPDiplomacyScreen()
{
	return m_mpDiplomacyScreen;
}

CvMessageCodeTranslator& cvInternalGlobals::getMessageCodes()
{
	return *m_messageCodes;
}

FMPIManager*& cvInternalGlobals::getFMPMgrPtr()
{
	return m_pFMPMgr;
}

CvPortal& cvInternalGlobals::getPortal()
{
	return *m_portal;
}

CvSetupData& cvInternalGlobals::getSetupData()
{
	return *m_setupData;
}

CvInitCore& cvInternalGlobals::getInitCore()
{
	return *m_initCore;
}

CvInitCore& cvInternalGlobals::getLoadedInitCore()
{
	return *m_loadedInitCore;
}

CvInitCore& cvInternalGlobals::getIniInitCore()
{
	return *m_iniInitCore;
}

CvStatsReporter& cvInternalGlobals::getStatsReporter()
{
	return *m_statsReporter;
}

CvStatsReporter* cvInternalGlobals::getStatsReporterPtr()
{
	return m_statsReporter;
}

CvInterface& cvInternalGlobals::getInterface()
{
	return *m_interface;
}

CvInterface* cvInternalGlobals::getInterfacePtr()
{
	return m_interface;
}

CvRandom& cvInternalGlobals::getASyncRand()
{
	return *m_asyncRand;
}

CMessageQueue& cvInternalGlobals::getMessageQueue()
{
	return *m_messageQueue;
}

CMessageQueue& cvInternalGlobals::getHotMessageQueue()
{
	return *m_hotJoinMsgQueue;
}

CMessageControl& cvInternalGlobals::getMessageControl()
{
	return *m_messageControl;
}

CvDropMgr& cvInternalGlobals::getDropMgr()
{
	return *m_dropMgr;
}

FAStar& cvInternalGlobals::getPathFinder()
{
	return *m_pathFinder;
}

FAStar& cvInternalGlobals::getInterfacePathFinder()
{
	return *m_interfacePathFinder;
}

FAStar& cvInternalGlobals::getStepFinder()
{
	return *m_stepFinder;
}

FAStar& cvInternalGlobals::getRouteFinder()
{
	return *m_routeFinder;
}

FAStar& cvInternalGlobals::getBorderFinder()
{
	return *m_borderFinder;
}

FAStar& cvInternalGlobals::getAreaFinder()
{
	return *m_areaFinder;
}

FAStar& cvInternalGlobals::getPlotGroupFinder()
{
	return *m_plotGroupFinder;
}

NiPoint3& cvInternalGlobals::getPt3Origin()
{
	return m_pt3Origin;
}

std::vector<CvInterfaceModeInfo*>& cvInternalGlobals::getInterfaceModeInfo()		// For Moose - XML Load Util and CvInfos
{
	return m_paInterfaceModeInfo;
}

CvInterfaceModeInfo& cvInternalGlobals::getInterfaceModeInfo(InterfaceModeTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_INTERFACEMODE_TYPES);
	return *(m_paInterfaceModeInfo[e]);
}

NiPoint3& cvInternalGlobals::getPt3CameraDir()
{
	return m_pt3CameraDir;
}

bool& cvInternalGlobals::getLogging()
{
	return m_bLogging;
}

bool& cvInternalGlobals::getRandLogging()
{
	return m_bRandLogging;
}

bool& cvInternalGlobals::getSynchLogging()
{
	return m_bSynchLogging;
}

bool& cvInternalGlobals::overwriteLogs()
{
	return m_bOverwriteLogs;
}

int* cvInternalGlobals::getPlotDirectionX()
{
	return m_aiPlotDirectionX;
}

int* cvInternalGlobals::getPlotDirectionY()
{
	return m_aiPlotDirectionY;
}

int* cvInternalGlobals::getPlotCardinalDirectionX()
{
	return m_aiPlotCardinalDirectionX;
}

int* cvInternalGlobals::getPlotCardinalDirectionY()
{
	return m_aiPlotCardinalDirectionY;
}

int* cvInternalGlobals::getCityPlotX()
{
	return m_aiCityPlotX;
}

int* cvInternalGlobals::getCityPlotY()
{
	return m_aiCityPlotY;
}

int* cvInternalGlobals::getCityPlotPriority()
{
	return m_aiCityPlotPriority;
}

int cvInternalGlobals::getXYCityPlot(int i, int j)
{
	FAssertMsg(i < CITY_PLOTS_DIAMETER, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < CITY_PLOTS_DIAMETER, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_aaiXYCityPlot[i][j];
}

DirectionTypes* cvInternalGlobals::getTurnLeftDirection()
{
	return m_aeTurnLeftDirection;
}

DirectionTypes cvInternalGlobals::getTurnLeftDirection(int i)
{
	FAssertMsg(i < NUM_DIRECTION_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_aeTurnLeftDirection[i];
}

DirectionTypes* cvInternalGlobals::getTurnRightDirection()
{
	return m_aeTurnRightDirection;
}

DirectionTypes cvInternalGlobals::getTurnRightDirection(int i)
{
	FAssertMsg(i < NUM_DIRECTION_TYPES, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_aeTurnRightDirection[i];
}

DirectionTypes cvInternalGlobals::getXYDirection(int i, int j)
{
	FAssertMsg(i < DIRECTION_DIAMETER, "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	FAssertMsg(j < DIRECTION_DIAMETER, "Index out of bounds");
	FAssertMsg(j > -1, "Index out of bounds");
	return m_aaeXYDirection[i][j];
}

int cvInternalGlobals::getNumWorldInfos()
{
	return (int)m_paWorldInfo.size();
}

std::vector<CvWorldInfo*>& cvInternalGlobals::getWorldInfo()
{
	return m_paWorldInfo;
}

CvWorldInfo& cvInternalGlobals::getWorldInfo(WorldSizeTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumWorldInfos());
	return *(m_paWorldInfo[e]);
}

/////////////////////////////////////////////
// CLIMATE
/////////////////////////////////////////////

int cvInternalGlobals::getNumClimateInfos()
{
	return (int)m_paClimateInfo.size();
}

std::vector<CvClimateInfo*>& cvInternalGlobals::getClimateInfo()
{
	return m_paClimateInfo;
}

CvClimateInfo& cvInternalGlobals::getClimateInfo(ClimateTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumClimateInfos());
	return *(m_paClimateInfo[e]);
}

/////////////////////////////////////////////
// SEALEVEL
/////////////////////////////////////////////

int cvInternalGlobals::getNumSeaLevelInfos()
{
	return (int)m_paSeaLevelInfo.size();
}

std::vector<CvSeaLevelInfo*>& cvInternalGlobals::getSeaLevelInfo()
{
	return m_paSeaLevelInfo;
}

CvSeaLevelInfo& cvInternalGlobals::getSeaLevelInfo(SeaLevelTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumSeaLevelInfos());
	return *(m_paSeaLevelInfo[e]);
}

int cvInternalGlobals::getNumHints()
{
	return (int)m_paHints.size();
}

std::vector<CvInfoBase*>& cvInternalGlobals::getHints()
{
	return m_paHints;
}

CvInfoBase& cvInternalGlobals::getHints(int i)
{
	return *(m_paHints[i]);
}

int cvInternalGlobals::getNumMainMenus()
{
	return (int)m_paMainMenus.size();
}

std::vector<CvMainMenuInfo*>& cvInternalGlobals::getMainMenus()
{
	return m_paMainMenus;
}

CvMainMenuInfo& cvInternalGlobals::getMainMenus(int i)
{
	if (i >= getNumMainMenus())
	{
		return *(m_paMainMenus[0]);
	}

	return *(m_paMainMenus[i]);
}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 10/30/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
// Python Modular Loading
int cvInternalGlobals::getNumPythonModulesInfos()
{
	return (int)m_paPythonModulesInfo.size();
}

std::vector<CvPythonModulesInfo*>& cvInternalGlobals::getPythonModulesInfo()
{
	return m_paPythonModulesInfo;
}

CvPythonModulesInfo& cvInternalGlobals::getPythonModulesInfo(int iIndex)
{
	FAssertMsg(iIndex < GC.getNumPythonModulesInfos(), "Index out of bounds");
	FAssertMsg(iIndex > -1, "Index out of bounds");
	return *(m_paPythonModulesInfo[iIndex]);
}

// MLF loading
void cvInternalGlobals::resetModLoadControlVector()
{
	m_paModLoadControlVector.clear();
}

int cvInternalGlobals::getModLoadControlVectorSize()
{
	return (int)m_paModLoadControlVector.size();
}

void cvInternalGlobals::setModLoadControlVector(const char* szModule)
{
	m_paModLoadControlVector.push_back(szModule);
}

CvString cvInternalGlobals::getModLoadControlVector(int i)
{
	return (CvString)m_paModLoadControlVector.at(i);
}

int cvInternalGlobals::getTotalNumModules()
{
	return m_iTotalNumModules;
}

void cvInternalGlobals::setTotalNumModules()
{
	m_iTotalNumModules++;
}

int cvInternalGlobals::getNumModLoadControlInfos()
{
	return (int)m_paModLoadControls.size();
}

std::vector<CvModLoadControlInfo*>& cvInternalGlobals::getModLoadControlInfos()
{
	return m_paModLoadControls;
}

CvModLoadControlInfo& cvInternalGlobals::getModLoadControlInfos(int iIndex)
{
	FAssertMsg(iIndex < getNumModLoadControlInfos(), "Index out of bounds");
	FAssertMsg(iIndex > -1, "Index out of bounds");
	return *(m_paModLoadControls[iIndex]);
}
// Modular loading Dependencies
void cvInternalGlobals::resetDependencies()
{
	m_bAnyDependency = false;
	m_bTypeDependency = false;
	m_paszAndDependencyTypes.clear();
	m_paszOrDependencyTypes.clear();
}

bool cvInternalGlobals::isAnyDependency()
{
	return m_bAnyDependency;
}

void cvInternalGlobals::setAnyDependency(bool bAnyDependency)
{
	m_bAnyDependency = bAnyDependency;
}

bool& cvInternalGlobals::getTypeDependency()
{
	return m_bTypeDependency;
}

void cvInternalGlobals::setTypeDependency(bool newValue)
{
	m_bTypeDependency = newValue;
}

bool& cvInternalGlobals::getForceOverwrite()
{
	return m_bForceOverwrite;
}

void cvInternalGlobals::setForceOverwrite(bool newValue)
{
	m_bForceOverwrite = newValue;
}

bool& cvInternalGlobals::getForceDelete()
{
	return m_bForceDelete;
}

void cvInternalGlobals::setForceDelete(bool newValue)
{
	m_bForceDelete = newValue;
}

int& cvInternalGlobals::getForceInsertLocation()
{
	return m_iForceInsertLocation;
}

void cvInternalGlobals::setForceInsertLocation(int newValue)
{
	m_iForceInsertLocation = newValue;
}

void cvInternalGlobals::resetOverwrites()
{
	m_iForceInsertLocation = -1;
	m_bForceDelete = false;
	m_bForceOverwrite = false;
}

const int cvInternalGlobals::getAndNumDependencyTypes() const
{
	return m_paszAndDependencyTypes.size();
}

void cvInternalGlobals::setAndDependencyTypes(const char* szDependencyTypes)
{
	m_paszAndDependencyTypes.push_back(szDependencyTypes);
}

const CvString cvInternalGlobals::getAndDependencyTypes(int i) const
{
	FAssertMsg(i < GC.getAndNumDependencyTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszAndDependencyTypes.at(i);
}

const int cvInternalGlobals::getOrNumDependencyTypes() const
{
	return m_paszOrDependencyTypes.size();
}

void cvInternalGlobals::setOrDependencyTypes(const char* szDependencyTypes)
{
	m_paszOrDependencyTypes.push_back(szDependencyTypes);
}

const CvString cvInternalGlobals::getOrDependencyTypes(int i) const
{
	FAssertMsg(i < GC.getOrNumDependencyTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszOrDependencyTypes.at(i);
}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 10/26/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
void cvInternalGlobals::setModDir(const char* szModDir)
{
	m_cszModDir = szModDir;
}

std::string cvInternalGlobals::getModDir()
{
	return m_cszModDir;
}
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 END                                                  */
/************************************************************************************************/

int cvInternalGlobals::getNumColorInfos()
{
	return (int)m_paColorInfo.size();
}

std::vector<CvColorInfo*>& cvInternalGlobals::getColorInfo()
{
	return m_paColorInfo;
}

CvColorInfo& cvInternalGlobals::getColorInfo(ColorTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumColorInfos());
	return *(m_paColorInfo[e]);
}


int cvInternalGlobals::getNumPlayerColorInfos()
{
	return (int)m_paPlayerColorInfo.size();
}

std::vector<CvPlayerColorInfo*>& cvInternalGlobals::getPlayerColorInfo()
{
	return m_paPlayerColorInfo;
}

CvPlayerColorInfo& cvInternalGlobals::getPlayerColorInfo(PlayerColorTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumPlayerColorInfos());
	return *(m_paPlayerColorInfo[e]);
}

int cvInternalGlobals::getNumAdvisorInfos()
{
	return (int)m_paAdvisorInfo.size();
}

std::vector<CvAdvisorInfo*>& cvInternalGlobals::getAdvisorInfo()
{
	return m_paAdvisorInfo;
}

CvAdvisorInfo& cvInternalGlobals::getAdvisorInfo(AdvisorTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumAdvisorInfos());
	return *(m_paAdvisorInfo[e]);
}

int cvInternalGlobals::getNumRouteModelInfos()
{
	return (int)m_paRouteModelInfo.size();
}

std::vector<CvRouteModelInfo*>& cvInternalGlobals::getRouteModelInfo()
{
	return m_paRouteModelInfo;
}

CvRouteModelInfo& cvInternalGlobals::getRouteModelInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumRouteModelInfos());
	return *(m_paRouteModelInfo[i]);
}

int cvInternalGlobals::getNumRiverInfos()
{
	return (int)m_paRiverInfo.size();
}

std::vector<CvRiverInfo*>& cvInternalGlobals::getRiverInfo()
{
	return m_paRiverInfo;
}

CvRiverInfo& cvInternalGlobals::getRiverInfo(RiverTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumRiverInfos());
	return *(m_paRiverInfo[e]);
}

int cvInternalGlobals::getNumRiverModelInfos()
{
	return (int)m_paRiverModelInfo.size();
}

std::vector<CvRiverModelInfo*>& cvInternalGlobals::getRiverModelInfo()
{
	return m_paRiverModelInfo;
}

CvRiverModelInfo& cvInternalGlobals::getRiverModelInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumRiverModelInfos());
	return *(m_paRiverModelInfo[i]);
}

int cvInternalGlobals::getNumWaterPlaneInfos()
{
	return (int)m_paWaterPlaneInfo.size();
}

std::vector<CvWaterPlaneInfo*>& cvInternalGlobals::getWaterPlaneInfo()		// For Moose - CvDecal and CvWater
{
	return m_paWaterPlaneInfo;
}

CvWaterPlaneInfo& cvInternalGlobals::getWaterPlaneInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumWaterPlaneInfos());
	return *(m_paWaterPlaneInfo[i]);
}

int cvInternalGlobals::getNumTerrainPlaneInfos()
{
	return (int)m_paTerrainPlaneInfo.size();
}

std::vector<CvTerrainPlaneInfo*>& cvInternalGlobals::getTerrainPlaneInfo()
{
	return m_paTerrainPlaneInfo;
}

CvTerrainPlaneInfo& cvInternalGlobals::getTerrainPlaneInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumTerrainPlaneInfos());
	return *(m_paTerrainPlaneInfo[i]);
}

int cvInternalGlobals::getNumCameraOverlayInfos()
{
	return (int)m_paCameraOverlayInfo.size();
}

std::vector<CvCameraOverlayInfo*>& cvInternalGlobals::getCameraOverlayInfo()
{
	return m_paCameraOverlayInfo;
}

CvCameraOverlayInfo& cvInternalGlobals::getCameraOverlayInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumCameraOverlayInfos());
	return *(m_paCameraOverlayInfo[i]);
}

int cvInternalGlobals::getNumAnimationPathInfos()
{
	return (int)m_paAnimationPathInfo.size();
}

std::vector<CvAnimationPathInfo*>& cvInternalGlobals::getAnimationPathInfo()
{
	return m_paAnimationPathInfo;
}

CvAnimationPathInfo& cvInternalGlobals::getAnimationPathInfo(AnimationPathTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumAnimationPathInfos());
	return *(m_paAnimationPathInfo[e]);
}

int cvInternalGlobals::getNumAnimationCategoryInfos()
{
	return (int)m_paAnimationCategoryInfo.size();
}

std::vector<CvAnimationCategoryInfo*>& cvInternalGlobals::getAnimationCategoryInfo()
{
	return m_paAnimationCategoryInfo;
}

CvAnimationCategoryInfo& cvInternalGlobals::getAnimationCategoryInfo(AnimationCategoryTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumAnimationCategoryInfos());
	return *(m_paAnimationCategoryInfo[e]);
}

int cvInternalGlobals::getNumEntityEventInfos()
{
	return (int)m_paEntityEventInfo.size();
}

std::vector<CvEntityEventInfo*>& cvInternalGlobals::getEntityEventInfo()
{
	return m_paEntityEventInfo;
}

CvEntityEventInfo& cvInternalGlobals::getEntityEventInfo(EntityEventTypes e)
{
	FAssert( e > -1 );
	FAssert( e < GC.getNumEntityEventInfos() );
	return *(m_paEntityEventInfo[e]);
}

int cvInternalGlobals::getNumEffectInfos()
{
	return (int)m_paEffectInfo.size();
}

std::vector<CvEffectInfo*>& cvInternalGlobals::getEffectInfo()
{
	return m_paEffectInfo;
}

CvEffectInfo& cvInternalGlobals::getEffectInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumEffectInfos());
	return *(m_paEffectInfo[i]);
}


int cvInternalGlobals::getNumAttachableInfos()
{
	return (int)m_paAttachableInfo.size();
}

std::vector<CvAttachableInfo*>& cvInternalGlobals::getAttachableInfo()
{
	return m_paAttachableInfo;
}

CvAttachableInfo& cvInternalGlobals::getAttachableInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumAttachableInfos());
	return *(m_paAttachableInfo[i]);
}

int cvInternalGlobals::getNumCameraInfos()
{
	return (int)m_paCameraInfo.size();
}

std::vector<CvCameraInfo*>& cvInternalGlobals::getCameraInfo()
{
	return m_paCameraInfo;
}

CvCameraInfo& cvInternalGlobals::getCameraInfo(CameraAnimationTypes eCameraAnimationNum)
{
	return *(m_paCameraInfo[eCameraAnimationNum]);
}

int cvInternalGlobals::getNumUnitFormationInfos()
{
	return (int)m_paUnitFormationInfo.size();
}

std::vector<CvUnitFormationInfo*>& cvInternalGlobals::getUnitFormationInfo()		// For Moose - CvUnitEntity
{
	return m_paUnitFormationInfo;
}

CvUnitFormationInfo& cvInternalGlobals::getUnitFormationInfo(int i)
{
	FAssert(i > -1);
	FAssert(i < GC.getNumUnitFormationInfos());
	return *(m_paUnitFormationInfo[i]);
}

// TEXT
int cvInternalGlobals::getNumGameTextXML()
{
	return (int)m_paGameTextXML.size();
}

std::vector<CvGameText*>& cvInternalGlobals::getGameTextXML()
{
	return m_paGameTextXML;
}

// Landscape INFOS
int cvInternalGlobals::getNumLandscapeInfos()
{
	return (int)m_paLandscapeInfo.size();
}

std::vector<CvLandscapeInfo*>& cvInternalGlobals::getLandscapeInfo()
{
	return m_paLandscapeInfo;
}

CvLandscapeInfo& cvInternalGlobals::getLandscapeInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumLandscapeInfos());
	return *(m_paLandscapeInfo[iIndex]);
}

int cvInternalGlobals::getActiveLandscapeID()
{
	return m_iActiveLandscapeID;
}

void cvInternalGlobals::setActiveLandscapeID(int iLandscapeID)
{
	m_iActiveLandscapeID = iLandscapeID;
}


int cvInternalGlobals::getNumTerrainInfos()
{
	return (int)m_paTerrainInfo.size();
}

std::vector<CvTerrainInfo*>& cvInternalGlobals::getTerrainInfo()		// For Moose - XML Load Util, CvInfos, CvTerrainTypeWBPalette
{
	return m_paTerrainInfo;
}

CvTerrainInfo& cvInternalGlobals::getTerrainInfo(TerrainTypes eTerrainNum)
{
	FAssert(eTerrainNum > -1);
	FAssert(eTerrainNum < GC.getNumTerrainInfos());
	return *(m_paTerrainInfo[eTerrainNum]);
}

int cvInternalGlobals::getNumBonusClassInfos()
{
	return (int)m_paBonusClassInfo.size();
}

std::vector<CvBonusClassInfo*>& cvInternalGlobals::getBonusClassInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paBonusClassInfo;
}

CvBonusClassInfo& cvInternalGlobals::getBonusClassInfo(BonusClassTypes eBonusNum)
{
	FAssert(eBonusNum > -1);
	FAssert(eBonusNum < GC.getNumBonusClassInfos());
	return *(m_paBonusClassInfo[eBonusNum]);
}


int cvInternalGlobals::getNumBonusInfos()
{
	return (int)m_paBonusInfo.size();
}

std::vector<CvBonusInfo*>& cvInternalGlobals::getBonusInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paBonusInfo;
}

CvBonusInfo& cvInternalGlobals::getBonusInfo(BonusTypes eBonusNum)
{
	FAssert(eBonusNum > -1);
	FAssert(eBonusNum < GC.getNumBonusInfos());
	return *(m_paBonusInfo[eBonusNum]);
}

int cvInternalGlobals::getNumFeatureInfos()
{
	return (int)m_paFeatureInfo.size();
}

std::vector<CvFeatureInfo*>& cvInternalGlobals::getFeatureInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paFeatureInfo;
}

CvFeatureInfo& cvInternalGlobals::getFeatureInfo(FeatureTypes eFeatureNum)
{
	FAssert(eFeatureNum > -1);
	FAssert(eFeatureNum < GC.getNumFeatureInfos());
	return *(m_paFeatureInfo[eFeatureNum]);
}

int& cvInternalGlobals::getNumPlayableCivilizationInfos()
{
	return m_iNumPlayableCivilizationInfos;
}

int& cvInternalGlobals::getNumAIPlayableCivilizationInfos()
{
	return m_iNumAIPlayableCivilizationInfos;
}

int cvInternalGlobals::getNumCivilizationInfos()
{
	return (int)m_paCivilizationInfo.size();
}

std::vector<CvCivilizationInfo*>& cvInternalGlobals::getCivilizationInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCivilizationInfo;
}

CvCivilizationInfo& cvInternalGlobals::getCivilizationInfo(CivilizationTypes eCivilizationNum)
{
	FAssert(eCivilizationNum > -1);
	FAssert(eCivilizationNum < GC.getNumCivilizationInfos());
	return *(m_paCivilizationInfo[eCivilizationNum]);
}


int cvInternalGlobals::getNumLeaderHeadInfos()
{
	return (int)m_paLeaderHeadInfo.size();
}

std::vector<CvLeaderHeadInfo*>& cvInternalGlobals::getLeaderHeadInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paLeaderHeadInfo;
}

CvLeaderHeadInfo& cvInternalGlobals::getLeaderHeadInfo(LeaderHeadTypes eLeaderHeadNum)
{
	FAssert(eLeaderHeadNum > -1);
	FAssert(eLeaderHeadNum < GC.getNumLeaderHeadInfos());
	return *(m_paLeaderHeadInfo[eLeaderHeadNum]);
}


int cvInternalGlobals::getNumTraitInfos()
{
	return (int)m_paTraitInfo.size();
}

std::vector<CvTraitInfo*>& cvInternalGlobals::getTraitInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paTraitInfo;
}

CvTraitInfo& cvInternalGlobals::getTraitInfo(TraitTypes eTraitNum)
{
	FAssert(eTraitNum > -1);
	FAssert(eTraitNum < GC.getNumTraitInfos());
	return *(m_paTraitInfo[eTraitNum]);
}


int cvInternalGlobals::getNumCursorInfos()
{
	return (int)m_paCursorInfo.size();
}

std::vector<CvCursorInfo*>& cvInternalGlobals::getCursorInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCursorInfo;
}

CvCursorInfo& cvInternalGlobals::getCursorInfo(CursorTypes eCursorNum)
{
	FAssert(eCursorNum > -1);
	FAssert(eCursorNum < GC.getNumCursorInfos());
	return *(m_paCursorInfo[eCursorNum]);
}

int cvInternalGlobals::getNumThroneRoomCameras()
{
	return (int)m_paThroneRoomCamera.size();
}

std::vector<CvThroneRoomCamera*>& cvInternalGlobals::getThroneRoomCamera()	// For Moose - XML Load Util, CvInfos
{
	return m_paThroneRoomCamera;
}

CvThroneRoomCamera& cvInternalGlobals::getThroneRoomCamera(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumThroneRoomCameras());
	return *(m_paThroneRoomCamera[iIndex]);
}

int cvInternalGlobals::getNumThroneRoomInfos()
{
	return (int)m_paThroneRoomInfo.size();
}

std::vector<CvThroneRoomInfo*>& cvInternalGlobals::getThroneRoomInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paThroneRoomInfo;
}

CvThroneRoomInfo& cvInternalGlobals::getThroneRoomInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumThroneRoomInfos());
	return *(m_paThroneRoomInfo[iIndex]);
}

int cvInternalGlobals::getNumThroneRoomStyleInfos()
{
	return (int)m_paThroneRoomStyleInfo.size();
}

std::vector<CvThroneRoomStyleInfo*>& cvInternalGlobals::getThroneRoomStyleInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paThroneRoomStyleInfo;
}

CvThroneRoomStyleInfo& cvInternalGlobals::getThroneRoomStyleInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumThroneRoomStyleInfos());
	return *(m_paThroneRoomStyleInfo[iIndex]);
}

int cvInternalGlobals::getNumSlideShowInfos()
{
	return (int)m_paSlideShowInfo.size();
}

std::vector<CvSlideShowInfo*>& cvInternalGlobals::getSlideShowInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSlideShowInfo;
}

CvSlideShowInfo& cvInternalGlobals::getSlideShowInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumSlideShowInfos());
	return *(m_paSlideShowInfo[iIndex]);
}

int cvInternalGlobals::getNumSlideShowRandomInfos()
{
	return (int)m_paSlideShowRandomInfo.size();
}

std::vector<CvSlideShowRandomInfo*>& cvInternalGlobals::getSlideShowRandomInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSlideShowRandomInfo;
}

CvSlideShowRandomInfo& cvInternalGlobals::getSlideShowRandomInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumSlideShowRandomInfos());
	return *(m_paSlideShowRandomInfo[iIndex]);
}

int cvInternalGlobals::getNumWorldPickerInfos()
{
	return (int)m_paWorldPickerInfo.size();
}

std::vector<CvWorldPickerInfo*>& cvInternalGlobals::getWorldPickerInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paWorldPickerInfo;
}

CvWorldPickerInfo& cvInternalGlobals::getWorldPickerInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumWorldPickerInfos());
	return *(m_paWorldPickerInfo[iIndex]);
}

int cvInternalGlobals::getNumSpaceShipInfos()
{
	return (int)m_paSpaceShipInfo.size();
}

std::vector<CvSpaceShipInfo*>& cvInternalGlobals::getSpaceShipInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSpaceShipInfo;
}

CvSpaceShipInfo& cvInternalGlobals::getSpaceShipInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumSpaceShipInfos());
	return *(m_paSpaceShipInfo[iIndex]);
}

int cvInternalGlobals::getNumUnitInfos()
{
	return (int)m_paUnitInfo.size();
}

std::vector<CvUnitInfo*>& cvInternalGlobals::getUnitInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paUnitInfo;
}

CvUnitInfo& cvInternalGlobals::getUnitInfo(UnitTypes eUnitNum)
{
	FAssert(eUnitNum > -1);
	FAssert(eUnitNum < GC.getNumUnitInfos());
	return *(m_paUnitInfo[eUnitNum]);
}

int cvInternalGlobals::getNumSpawnInfos()
{
	return (int)m_paSpawnInfo.size();
}

std::vector<CvSpawnInfo*>& cvInternalGlobals::getSpawnInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSpawnInfo;
}

CvSpawnInfo& cvInternalGlobals::getSpawnInfo(SpawnTypes eSpawnNum)
{
	FAssert(eSpawnNum > -1);
	FAssert(eSpawnNum < GC.getNumSpawnInfos());
	return *(m_paSpawnInfo[eSpawnNum]);
}

int cvInternalGlobals::getNumSpecialUnitInfos()
{
	return (int)m_paSpecialUnitInfo.size();
}

std::vector<CvSpecialUnitInfo*>& cvInternalGlobals::getSpecialUnitInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSpecialUnitInfo;
}

CvSpecialUnitInfo& cvInternalGlobals::getSpecialUnitInfo(SpecialUnitTypes eSpecialUnitNum)
{
	FAssert(eSpecialUnitNum > -1);
	FAssert(eSpecialUnitNum < GC.getNumSpecialUnitInfos());
	return *(m_paSpecialUnitInfo[eSpecialUnitNum]);
}


int cvInternalGlobals::getNumConceptInfos()
{
	return (int)m_paConceptInfo.size();
}

std::vector<CvInfoBase*>& cvInternalGlobals::getConceptInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paConceptInfo;
}

CvInfoBase& cvInternalGlobals::getConceptInfo(ConceptTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumConceptInfos());
	return *(m_paConceptInfo[e]);
}


int cvInternalGlobals::getNumNewConceptInfos()
{
	return (int)m_paNewConceptInfo.size();
}

std::vector<CvInfoBase*>& cvInternalGlobals::getNewConceptInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paNewConceptInfo;
}

CvInfoBase& cvInternalGlobals::getNewConceptInfo(NewConceptTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumNewConceptInfos());
	return *(m_paNewConceptInfo[e]);
}


int cvInternalGlobals::getNumCityTabInfos()
{
	return (int)m_paCityTabInfo.size();
}

std::vector<CvInfoBase*>& cvInternalGlobals::getCityTabInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCityTabInfo;
}

CvInfoBase& cvInternalGlobals::getCityTabInfo(CityTabTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumCityTabInfos());
	return *(m_paCityTabInfo[e]);
}


int cvInternalGlobals::getNumCalendarInfos()
{
	return (int)m_paCalendarInfo.size();
}

std::vector<CvInfoBase*>& cvInternalGlobals::getCalendarInfo()
{
	return m_paCalendarInfo;
}

CvInfoBase& cvInternalGlobals::getCalendarInfo(CalendarTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumCalendarInfos());
	return *(m_paCalendarInfo[e]);
}


int cvInternalGlobals::getNumSeasonInfos()
{
	return (int)m_paSeasonInfo.size();
}

std::vector<CvInfoBase*>& cvInternalGlobals::getSeasonInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSeasonInfo;
}

CvInfoBase& cvInternalGlobals::getSeasonInfo(SeasonTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumSeasonInfos());
	return *(m_paSeasonInfo[e]);
}


int cvInternalGlobals::getNumMonthInfos()
{
	return (int)m_paMonthInfo.size();
}

std::vector<CvInfoBase*>& cvInternalGlobals::getMonthInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paMonthInfo;
}

CvInfoBase& cvInternalGlobals::getMonthInfo(MonthTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumMonthInfos());
	return *(m_paMonthInfo[e]);
}


int cvInternalGlobals::getNumDenialInfos()
{
	return (int)m_paDenialInfo.size();
}

std::vector<CvInfoBase*>& cvInternalGlobals::getDenialInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paDenialInfo;
}

CvInfoBase& cvInternalGlobals::getDenialInfo(DenialTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumDenialInfos());
	return *(m_paDenialInfo[e]);
}


int cvInternalGlobals::getNumInvisibleInfos()
{
	return (int)m_paInvisibleInfo.size();
}

std::vector<CvInfoBase*>& cvInternalGlobals::getInvisibleInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paInvisibleInfo;
}

CvInfoBase& cvInternalGlobals::getInvisibleInfo(InvisibleTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumInvisibleInfos());
	return *(m_paInvisibleInfo[e]);
}


int cvInternalGlobals::getNumVoteSourceInfos()
{
	return (int)m_paVoteSourceInfo.size();
}

std::vector<CvVoteSourceInfo*>& cvInternalGlobals::getVoteSourceInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paVoteSourceInfo;
}

CvVoteSourceInfo& cvInternalGlobals::getVoteSourceInfo(VoteSourceTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumVoteSourceInfos());
	return *(m_paVoteSourceInfo[e]);
}


int cvInternalGlobals::getNumUnitCombatInfos()
{
	return (int)m_paUnitCombatInfo.size();
}

std::vector<CvInfoBase*>& cvInternalGlobals::getUnitCombatInfo()
{
	return m_paUnitCombatInfo;
}

CvInfoBase& cvInternalGlobals::getUnitCombatInfo(UnitCombatTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumUnitCombatInfos());
	return *(m_paUnitCombatInfo[e]);
}


std::vector<CvInfoBase*>& cvInternalGlobals::getDomainInfo()
{
	return m_paDomainInfo;
}

CvInfoBase& cvInternalGlobals::getDomainInfo(DomainTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_DOMAIN_TYPES);
	return *(m_paDomainInfo[e]);
}


std::vector<CvInfoBase*>& cvInternalGlobals::getUnitAIInfo()
{
	return m_paUnitAIInfos;
}

CvInfoBase& cvInternalGlobals::getUnitAIInfo(UnitAITypes eUnitAINum)
{
	FAssert(eUnitAINum >= 0);
	FAssert(eUnitAINum < NUM_UNITAI_TYPES);
	return *(m_paUnitAIInfos[eUnitAINum]);
}

//	Koshling - added internal registration of supported UnitAI types, not reliant
//	on external definition in XML
void cvInternalGlobals::registerUnitAI(const char* szType, int enumVal)
{
	FAssert(m_paUnitAIInfos.size() == enumVal);

	CvInfoBase* entry = new	CvInfoBase(szType);

	m_paUnitAIInfos.push_back(entry);
	setInfoTypeFromString(szType, enumVal);
}

#define	REGISTER_UNITAI(x)	registerUnitAI(#x,x)

void cvInternalGlobals::registerUnitAIs(void)
{
	//	Sadly C++ doesn't have any reflection capability so need to do this explicitly
	REGISTER_UNITAI(UNITAI_UNKNOWN);
	REGISTER_UNITAI(UNITAI_ANIMAL);
	REGISTER_UNITAI(UNITAI_SETTLE);
	REGISTER_UNITAI(UNITAI_WORKER);
	REGISTER_UNITAI(UNITAI_ATTACK);
	REGISTER_UNITAI(UNITAI_ATTACK_CITY);
	REGISTER_UNITAI(UNITAI_COLLATERAL);
	REGISTER_UNITAI(UNITAI_PILLAGE);
	REGISTER_UNITAI(UNITAI_RESERVE);
	REGISTER_UNITAI(UNITAI_COUNTER);
	REGISTER_UNITAI(UNITAI_CITY_DEFENSE);
	REGISTER_UNITAI(UNITAI_CITY_COUNTER);
	REGISTER_UNITAI(UNITAI_CITY_SPECIAL);
	REGISTER_UNITAI(UNITAI_EXPLORE);
	REGISTER_UNITAI(UNITAI_MISSIONARY);
	REGISTER_UNITAI(UNITAI_PROPHET);
	REGISTER_UNITAI(UNITAI_ARTIST);
	REGISTER_UNITAI(UNITAI_SCIENTIST);
	REGISTER_UNITAI(UNITAI_GENERAL);
	REGISTER_UNITAI(UNITAI_MERCHANT);
	REGISTER_UNITAI(UNITAI_ENGINEER);
	REGISTER_UNITAI(UNITAI_SPY);
	REGISTER_UNITAI(UNITAI_ICBM);
	REGISTER_UNITAI(UNITAI_WORKER_SEA);
	REGISTER_UNITAI(UNITAI_ATTACK_SEA);
	REGISTER_UNITAI(UNITAI_RESERVE_SEA);
	REGISTER_UNITAI(UNITAI_ESCORT_SEA);
	REGISTER_UNITAI(UNITAI_EXPLORE_SEA);
	REGISTER_UNITAI(UNITAI_ASSAULT_SEA);
	REGISTER_UNITAI(UNITAI_SETTLER_SEA);
	REGISTER_UNITAI(UNITAI_MISSIONARY_SEA);
	REGISTER_UNITAI(UNITAI_SPY_SEA);
	REGISTER_UNITAI(UNITAI_CARRIER_SEA);
	REGISTER_UNITAI(UNITAI_MISSILE_CARRIER_SEA);
	REGISTER_UNITAI(UNITAI_PIRATE_SEA);
	REGISTER_UNITAI(UNITAI_ATTACK_AIR);
	REGISTER_UNITAI(UNITAI_DEFENSE_AIR);
	REGISTER_UNITAI(UNITAI_CARRIER_AIR);
	REGISTER_UNITAI(UNITAI_MISSILE_AIR);
	REGISTER_UNITAI(UNITAI_PARADROP);
	REGISTER_UNITAI(UNITAI_ATTACK_CITY_LEMMING);
	REGISTER_UNITAI(UNITAI_PILLAGE_COUNTER);
	REGISTER_UNITAI(UNITAI_SUBDUED_ANIMAL);
	REGISTER_UNITAI(UNITAI_HUNTER);
}

//	AIAndy - added internal registration of supported AIScale types similar to UnitAIs but without info class
#define	REGISTER_AISCALE(x)	setInfoTypeFromString(#x,x)

void cvInternalGlobals::registerAIScales(void)
{
	//	Sadly C++ doesn't have any reflection capability so need to do this explicitly
	REGISTER_AISCALE(AISCALE_NONE);
	REGISTER_AISCALE(AISCALE_CITY);
	REGISTER_AISCALE(AISCALE_AREA);
	REGISTER_AISCALE(AISCALE_PLAYER);
	REGISTER_AISCALE(AISCALE_TEAM);
}

//	AIAndy: Register game object types
#define	REGISTER_GAMEOBJECT(x)	setInfoTypeFromString(#x,x)

void cvInternalGlobals::registerGameObjects()
{
	//	Sadly C++ doesn't have any reflection capability so need to do this explicitly
	REGISTER_GAMEOBJECT(NO_GAMEOBJECT);
	REGISTER_GAMEOBJECT(GAMEOBJECT_GAME);
	REGISTER_GAMEOBJECT(GAMEOBJECT_TEAM);
	REGISTER_GAMEOBJECT(GAMEOBJECT_PLAYER);
	REGISTER_GAMEOBJECT(GAMEOBJECT_CITY);
	REGISTER_GAMEOBJECT(GAMEOBJECT_UNIT);
	REGISTER_GAMEOBJECT(GAMEOBJECT_PLOT);
}

std::vector<CvInfoBase*>& cvInternalGlobals::getAttitudeInfo()
{
	return m_paAttitudeInfos;
}

CvInfoBase& cvInternalGlobals::getAttitudeInfo(AttitudeTypes eAttitudeNum)
{
	FAssert(eAttitudeNum >= 0);
	FAssert(eAttitudeNum < NUM_ATTITUDE_TYPES);
	return *(m_paAttitudeInfos[eAttitudeNum]);
}


std::vector<CvInfoBase*>& cvInternalGlobals::getMemoryInfo()
{
	return m_paMemoryInfos;
}

CvInfoBase& cvInternalGlobals::getMemoryInfo(MemoryTypes eMemoryNum)
{
	FAssert(eMemoryNum >= 0);
	FAssert(eMemoryNum < NUM_MEMORY_TYPES);
	return *(m_paMemoryInfos[eMemoryNum]);
}


int cvInternalGlobals::getNumGameOptionInfos()
{
	return (int)m_paGameOptionInfos.size();
}

std::vector<CvGameOptionInfo*>& cvInternalGlobals::getGameOptionInfo()
{
	return m_paGameOptionInfos;
}

CvGameOptionInfo& cvInternalGlobals::getGameOptionInfo(GameOptionTypes eGameOptionNum)
{
	FAssert(eGameOptionNum >= 0);
	FAssert(eGameOptionNum < GC.getNumGameOptionInfos());
	return *(m_paGameOptionInfos[eGameOptionNum]);
}

int cvInternalGlobals::getNumMPOptionInfos()
{
	return (int)m_paMPOptionInfos.size();
}

std::vector<CvMPOptionInfo*>& cvInternalGlobals::getMPOptionInfo()
{
	 return m_paMPOptionInfos;
}

CvMPOptionInfo& cvInternalGlobals::getMPOptionInfo(MultiplayerOptionTypes eMPOptionNum)
{
	FAssert(eMPOptionNum >= 0);
	FAssert(eMPOptionNum < GC.getNumMPOptionInfos());
	return *(m_paMPOptionInfos[eMPOptionNum]);
}

int cvInternalGlobals::getNumForceControlInfos()
{
	return (int)m_paForceControlInfos.size();
}

std::vector<CvForceControlInfo*>& cvInternalGlobals::getForceControlInfo()
{
	return m_paForceControlInfos;
}

CvForceControlInfo& cvInternalGlobals::getForceControlInfo(ForceControlTypes eForceControlNum)
{
	FAssert(eForceControlNum >= 0);
	FAssert(eForceControlNum < GC.getNumForceControlInfos());
	return *(m_paForceControlInfos[eForceControlNum]);
}

std::vector<CvPlayerOptionInfo*>& cvInternalGlobals::getPlayerOptionInfo()
{
	return m_paPlayerOptionInfos;
}

CvPlayerOptionInfo& cvInternalGlobals::getPlayerOptionInfo(PlayerOptionTypes ePlayerOptionNum)
{
	FAssert(ePlayerOptionNum >= 0);
	FAssert(ePlayerOptionNum < NUM_PLAYEROPTION_TYPES);
	return *(m_paPlayerOptionInfos[ePlayerOptionNum]);
}

std::vector<CvGraphicOptionInfo*>& cvInternalGlobals::getGraphicOptionInfo()
{
	return m_paGraphicOptionInfos;
}

CvGraphicOptionInfo& cvInternalGlobals::getGraphicOptionInfo(GraphicOptionTypes eGraphicOptionNum)
{
	FAssert(eGraphicOptionNum >= 0);
	FAssert(eGraphicOptionNum < NUM_GRAPHICOPTION_TYPES);
	return *(m_paGraphicOptionInfos[eGraphicOptionNum]);
}


std::vector<CvYieldInfo*>& cvInternalGlobals::getYieldInfo()	// For Moose - XML Load Util
{
	return m_paYieldInfo;
}

CvYieldInfo& cvInternalGlobals::getYieldInfo(YieldTypes eYieldNum)
{
	FAssert(eYieldNum > -1);
	FAssert(eYieldNum < NUM_YIELD_TYPES);
	return *(m_paYieldInfo[eYieldNum]);
}


std::vector<CvCommerceInfo*>& cvInternalGlobals::getCommerceInfo()	// For Moose - XML Load Util
{
	return m_paCommerceInfo;
}

CvCommerceInfo& cvInternalGlobals::getCommerceInfo(CommerceTypes eCommerceNum)
{
	FAssert(eCommerceNum > -1);
	FAssert(eCommerceNum < NUM_COMMERCE_TYPES);
	return *(m_paCommerceInfo[eCommerceNum]);
}

int cvInternalGlobals::getNumRouteInfos()
{
	return (int)m_paRouteInfo.size();
}

std::vector<CvRouteInfo*>& cvInternalGlobals::getRouteInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paRouteInfo;
}

CvRouteInfo& cvInternalGlobals::getRouteInfo(RouteTypes eRouteNum)
{
	FAssert(eRouteNum > -1);
	FAssert(eRouteNum < GC.getNumRouteInfos());
	return *(m_paRouteInfo[eRouteNum]);
}

int cvInternalGlobals::getNumImprovementInfos()
{
	return (int)m_paImprovementInfo.size();
}

std::vector<CvImprovementInfo*>& cvInternalGlobals::getImprovementInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paImprovementInfo;
}

CvImprovementInfo& cvInternalGlobals::getImprovementInfo(ImprovementTypes eImprovementNum)
{
	FAssert(eImprovementNum > -1);
	FAssert(eImprovementNum < GC.getNumImprovementInfos());
	return *(m_paImprovementInfo[eImprovementNum]);
}

int cvInternalGlobals::getNumGoodyInfos()
{
	return (int)m_paGoodyInfo.size();
}

std::vector<CvGoodyInfo*>& cvInternalGlobals::getGoodyInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paGoodyInfo;
}

CvGoodyInfo& cvInternalGlobals::getGoodyInfo(GoodyTypes eGoodyNum)
{
	FAssert(eGoodyNum > -1);
	FAssert(eGoodyNum < GC.getNumGoodyInfos());
	return *(m_paGoodyInfo[eGoodyNum]);
}

int cvInternalGlobals::getNumBuildInfos()
{
	return (int)m_paBuildInfo.size();
}

std::vector<CvBuildInfo*>& cvInternalGlobals::getBuildInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paBuildInfo;
}

CvBuildInfo& cvInternalGlobals::getBuildInfo(BuildTypes eBuildNum)
{
	FAssert(eBuildNum > -1);
	FAssert(eBuildNum < GC.getNumBuildInfos());
	return *(m_paBuildInfo[eBuildNum]);
}

int cvInternalGlobals::getNumHandicapInfos()
{
	return (int)m_paHandicapInfo.size();
}

std::vector<CvHandicapInfo*>& cvInternalGlobals::getHandicapInfo()	// Do NOT export outside of the DLL	// For Moose - XML Load Util
{
	return m_paHandicapInfo;
}

CvHandicapInfo& cvInternalGlobals::getHandicapInfo(HandicapTypes eHandicapNum)
{
	FAssert(eHandicapNum > -1);
	FAssert(eHandicapNum < GC.getNumHandicapInfos());
	return *(m_paHandicapInfo[eHandicapNum]);
}

int cvInternalGlobals::getNumGameSpeedInfos()
{
	return (int)m_paGameSpeedInfo.size();
}

std::vector<CvGameSpeedInfo*>& cvInternalGlobals::getGameSpeedInfo()	// Do NOT export outside of the DLL	// For Moose - XML Load Util
{
	return m_paGameSpeedInfo;
}

CvGameSpeedInfo& cvInternalGlobals::getGameSpeedInfo(GameSpeedTypes eGameSpeedNum)
{
	FAssert(eGameSpeedNum > -1);
	FAssert(eGameSpeedNum < GC.getNumGameSpeedInfos());
	return *(m_paGameSpeedInfo[eGameSpeedNum]);
}

int cvInternalGlobals::getNumTurnTimerInfos()
{
	return (int)m_paTurnTimerInfo.size();
}

std::vector<CvTurnTimerInfo*>& cvInternalGlobals::getTurnTimerInfo()	// Do NOT export outside of the DLL	// For Moose - XML Load Util
{
	return m_paTurnTimerInfo;
}

CvTurnTimerInfo& cvInternalGlobals::getTurnTimerInfo(TurnTimerTypes eTurnTimerNum)
{
	FAssert(eTurnTimerNum > -1);
	FAssert(eTurnTimerNum < GC.getNumTurnTimerInfos());
	return *(m_paTurnTimerInfo[eTurnTimerNum]);
}

int cvInternalGlobals::getNumProcessInfos()
{
	return (int)m_paProcessInfo.size();
}

std::vector<CvProcessInfo*>& cvInternalGlobals::getProcessInfo()
{
	return m_paProcessInfo;
}

CvProcessInfo& cvInternalGlobals::getProcessInfo(ProcessTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumProcessInfos());
	return *(m_paProcessInfo[e]);
}

int cvInternalGlobals::getNumVoteInfos()
{
	return (int)m_paVoteInfo.size();
}

std::vector<CvVoteInfo*>& cvInternalGlobals::getVoteInfo()
{
	return m_paVoteInfo;
}

CvVoteInfo& cvInternalGlobals::getVoteInfo(VoteTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumVoteInfos());
	return *(m_paVoteInfo[e]);
}

int cvInternalGlobals::getNumProjectInfos()
{
	return (int)m_paProjectInfo.size();
}

std::vector<CvProjectInfo*>& cvInternalGlobals::getProjectInfo()
{
	return m_paProjectInfo;
}

CvProjectInfo& cvInternalGlobals::getProjectInfo(ProjectTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumProjectInfos());
	return *(m_paProjectInfo[e]);
}

int cvInternalGlobals::getNumBuildingClassInfos()
{
	return (int)m_paBuildingClassInfo.size();
}

std::vector<CvBuildingClassInfo*>& cvInternalGlobals::getBuildingClassInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paBuildingClassInfo;
}

CvBuildingClassInfo& cvInternalGlobals::getBuildingClassInfo(BuildingClassTypes eBuildingClassNum)
{
	FAssert(eBuildingClassNum > -1);
	FAssert(eBuildingClassNum < GC.getNumBuildingClassInfos());
	return *(m_paBuildingClassInfo[eBuildingClassNum]);
}

int cvInternalGlobals::getNumBuildingInfos()
{
	return (int)m_paBuildingInfo.size();
}

std::vector<CvBuildingInfo*>& cvInternalGlobals::getBuildingInfo()	// For Moose - XML Load Util, CvInfos, CvCacheObject
{
	return m_paBuildingInfo;
}

CvBuildingInfo& cvInternalGlobals::getBuildingInfo(BuildingTypes eBuildingNum)
{
	FAssert(eBuildingNum > -1);
	FAssert(eBuildingNum < GC.getNumBuildingInfos());
	return *(m_paBuildingInfo[eBuildingNum]);
}

int cvInternalGlobals::getNumSpecialBuildingInfos()
{
	return (int)m_paSpecialBuildingInfo.size();
}

std::vector<CvSpecialBuildingInfo*>& cvInternalGlobals::getSpecialBuildingInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSpecialBuildingInfo;
}

CvSpecialBuildingInfo& cvInternalGlobals::getSpecialBuildingInfo(SpecialBuildingTypes eSpecialBuildingNum)
{
	FAssert(eSpecialBuildingNum > -1);
	FAssert(eSpecialBuildingNum < GC.getNumSpecialBuildingInfos());
	return *(m_paSpecialBuildingInfo[eSpecialBuildingNum]);
}

int cvInternalGlobals::getNumUnitClassInfos()
{
	return (int)m_paUnitClassInfo.size();
}

std::vector<CvUnitClassInfo*>& cvInternalGlobals::getUnitClassInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paUnitClassInfo;
}

CvUnitClassInfo& cvInternalGlobals::getUnitClassInfo(UnitClassTypes eUnitClassNum)
{
	FAssert(eUnitClassNum > -1);
	FAssert(eUnitClassNum < GC.getNumUnitClassInfos());
	return *(m_paUnitClassInfo[eUnitClassNum]);
}

int cvInternalGlobals::getNumActionInfos()
{
	return (int)m_paActionInfo.size();
}

std::vector<CvActionInfo*>& cvInternalGlobals::getActionInfo()	// For Moose - XML Load Util
{
	return m_paActionInfo;
}

CvActionInfo& cvInternalGlobals::getActionInfo(int i)
{
	FAssertMsg(i < getNumActionInfos(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return *(m_paActionInfo[i]);
}

std::vector<CvMissionInfo*>& cvInternalGlobals::getMissionInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paMissionInfo;
}

CvMissionInfo& cvInternalGlobals::getMissionInfo(MissionTypes eMissionNum)
{
	FAssert(eMissionNum > -1);
	FAssert(eMissionNum < NUM_MISSION_TYPES);
	return *(m_paMissionInfo[eMissionNum]);
}

std::vector<CvControlInfo*>& cvInternalGlobals::getControlInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paControlInfo;
}

CvControlInfo& cvInternalGlobals::getControlInfo(ControlTypes eControlNum)
{
	FAssert(eControlNum > -1);
	FAssert(eControlNum < NUM_CONTROL_TYPES);
	FAssert(m_paControlInfo.size() > 0);
	return *(m_paControlInfo[eControlNum]);
}

std::vector<CvCommandInfo*>& cvInternalGlobals::getCommandInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCommandInfo;
}

CvCommandInfo& cvInternalGlobals::getCommandInfo(CommandTypes eCommandNum)
{
	FAssert(eCommandNum > -1);
	FAssert(eCommandNum < NUM_COMMAND_TYPES);
	return *(m_paCommandInfo[eCommandNum]);
}

int cvInternalGlobals::getNumAutomateInfos()
{
	return (int)m_paAutomateInfo.size();
}

std::vector<CvAutomateInfo*>& cvInternalGlobals::getAutomateInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paAutomateInfo;
}

CvAutomateInfo& cvInternalGlobals::getAutomateInfo(int iAutomateNum)
{
	FAssertMsg(iAutomateNum < getNumAutomateInfos(), "Index out of bounds");
	FAssertMsg(iAutomateNum > -1, "Index out of bounds");
	return *(m_paAutomateInfo[iAutomateNum]);
}

int cvInternalGlobals::getNumPromotionInfos()
{
	return (int)m_paPromotionInfo.size();
}

std::vector<CvPromotionInfo*>& cvInternalGlobals::getPromotionInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paPromotionInfo;
}

CvPromotionInfo& cvInternalGlobals::getPromotionInfo(PromotionTypes ePromotionNum)
{
	FAssert(ePromotionNum > -1);
	FAssert(ePromotionNum < GC.getNumPromotionInfos());
	return *(m_paPromotionInfo[ePromotionNum]);
}

int cvInternalGlobals::getNumTechInfos()
{
	return (int)m_paTechInfo.size();
}

std::vector<CvTechInfo*>& cvInternalGlobals::getTechInfo()	// For Moose - XML Load Util, CvInfos, CvCacheObject
{
	return m_paTechInfo;
}

CvTechInfo& cvInternalGlobals::getTechInfo(TechTypes eTechNum)
{
	FAssert(eTechNum > -1);
	FAssert(eTechNum < GC.getNumTechInfos());
	return *(m_paTechInfo[eTechNum]);
}

int cvInternalGlobals::getNumReligionInfos()
{
	return (int)m_paReligionInfo.size();
}

std::vector<CvReligionInfo*>& cvInternalGlobals::getReligionInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paReligionInfo;
}

CvReligionInfo& cvInternalGlobals::getReligionInfo(ReligionTypes eReligionNum)
{
	FAssert(eReligionNum > -1);
	FAssert(eReligionNum < GC.getNumReligionInfos());
	return *(m_paReligionInfo[eReligionNum]);
}

int cvInternalGlobals::getNumCorporationInfos()
{
	return (int)m_paCorporationInfo.size();
}

std::vector<CvCorporationInfo*>& cvInternalGlobals::getCorporationInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCorporationInfo;
}

CvCorporationInfo& cvInternalGlobals::getCorporationInfo(CorporationTypes eCorporationNum)
{
	FAssert(eCorporationNum > -1);
	FAssert(eCorporationNum < GC.getNumCorporationInfos());
	return *(m_paCorporationInfo[eCorporationNum]);
}

int cvInternalGlobals::getNumSpecialistInfos()
{
	return (int)m_paSpecialistInfo.size();
}

std::vector<CvSpecialistInfo*>& cvInternalGlobals::getSpecialistInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paSpecialistInfo;
}

CvSpecialistInfo& cvInternalGlobals::getSpecialistInfo(SpecialistTypes eSpecialistNum)
{
	FAssert(eSpecialistNum > -1);
	FAssert(eSpecialistNum < GC.getNumSpecialistInfos());
	return *(m_paSpecialistInfo[eSpecialistNum]);
}

int cvInternalGlobals::getNumCivicOptionInfos()
{
	return (int)m_paCivicOptionInfo.size();
}

std::vector<CvCivicOptionInfo*>& cvInternalGlobals::getCivicOptionInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCivicOptionInfo;
}

CvCivicOptionInfo& cvInternalGlobals::getCivicOptionInfo(CivicOptionTypes eCivicOptionNum)
{
	FAssert(eCivicOptionNum > -1);
	FAssert(eCivicOptionNum < GC.getNumCivicOptionInfos());
	return *(m_paCivicOptionInfo[eCivicOptionNum]);
}

int cvInternalGlobals::getNumCivicInfos()
{
	return (int)m_paCivicInfo.size();
}

std::vector<CvCivicInfo*>& cvInternalGlobals::getCivicInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCivicInfo;
}

CvCivicInfo& cvInternalGlobals::getCivicInfo(CivicTypes eCivicNum)
{
	FAssert(eCivicNum > -1);
	FAssert(eCivicNum < GC.getNumCivicInfos());
	return *(m_paCivicInfo[eCivicNum]);
}

int cvInternalGlobals::getNumDiplomacyInfos()
{
	return (int)m_paDiplomacyInfo.size();
}

std::vector<CvDiplomacyInfo*>& cvInternalGlobals::getDiplomacyInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paDiplomacyInfo;
}

CvDiplomacyInfo& cvInternalGlobals::getDiplomacyInfo(int iDiplomacyNum)
{
	FAssertMsg(iDiplomacyNum < getNumDiplomacyInfos(), "Index out of bounds");
	FAssertMsg(iDiplomacyNum > -1, "Index out of bounds");
	return *(m_paDiplomacyInfo[iDiplomacyNum]);
}

int cvInternalGlobals::getNumEraInfos()
{
	return (int)m_aEraInfo.size();
}

std::vector<CvEraInfo*>& cvInternalGlobals::getEraInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_aEraInfo;
}

CvEraInfo& cvInternalGlobals::getEraInfo(EraTypes eEraNum)
{
	FAssert(eEraNum > -1);
	FAssert(eEraNum < GC.getNumEraInfos());
	return *(m_aEraInfo[eEraNum]);
}

int cvInternalGlobals::getNumHurryInfos()
{
	return (int)m_paHurryInfo.size();
}

std::vector<CvHurryInfo*>& cvInternalGlobals::getHurryInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paHurryInfo;
}

CvHurryInfo& cvInternalGlobals::getHurryInfo(HurryTypes eHurryNum)
{
	FAssert(eHurryNum > -1);
	FAssert(eHurryNum < GC.getNumHurryInfos());
	return *(m_paHurryInfo[eHurryNum]);
}

int cvInternalGlobals::getNumEmphasizeInfos()
{
	return (int)m_paEmphasizeInfo.size();
}

std::vector<CvEmphasizeInfo*>& cvInternalGlobals::getEmphasizeInfo()	// For Moose - XML Load Util
{
	return m_paEmphasizeInfo;
}

CvEmphasizeInfo& cvInternalGlobals::getEmphasizeInfo(EmphasizeTypes eEmphasizeNum)
{
	FAssert(eEmphasizeNum > -1);
	FAssert(eEmphasizeNum < GC.getNumEmphasizeInfos());
	return *(m_paEmphasizeInfo[eEmphasizeNum]);
}

int cvInternalGlobals::getNumUpkeepInfos()
{
	return (int)m_paUpkeepInfo.size();
}

std::vector<CvUpkeepInfo*>& cvInternalGlobals::getUpkeepInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paUpkeepInfo;
}

CvUpkeepInfo& cvInternalGlobals::getUpkeepInfo(UpkeepTypes eUpkeepNum)
{
	FAssert(eUpkeepNum > -1);
	FAssert(eUpkeepNum < GC.getNumUpkeepInfos());
	return *(m_paUpkeepInfo[eUpkeepNum]);
}

int cvInternalGlobals::getNumCultureLevelInfos()
{
	return (int)m_paCultureLevelInfo.size();
}

std::vector<CvCultureLevelInfo*>& cvInternalGlobals::getCultureLevelInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paCultureLevelInfo;
}

CvCultureLevelInfo& cvInternalGlobals::getCultureLevelInfo(CultureLevelTypes eCultureLevelNum)
{
	FAssert(eCultureLevelNum > -1);
	FAssert(eCultureLevelNum < GC.getNumCultureLevelInfos());
	return *(m_paCultureLevelInfo[eCultureLevelNum]);
}

int cvInternalGlobals::getNumVictoryInfos()
{
	return (int)m_paVictoryInfo.size();
}

std::vector<CvVictoryInfo*>& cvInternalGlobals::getVictoryInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paVictoryInfo;
}

CvVictoryInfo& cvInternalGlobals::getVictoryInfo(VictoryTypes eVictoryNum)
{
	FAssert(eVictoryNum > -1);
	FAssert(eVictoryNum < GC.getNumVictoryInfos());
	return *(m_paVictoryInfo[eVictoryNum]);
}

int cvInternalGlobals::getNumQuestInfos()
{
	return (int)m_paQuestInfo.size();
}

std::vector<CvQuestInfo*>& cvInternalGlobals::getQuestInfo()
{
	return m_paQuestInfo;
}

CvQuestInfo& cvInternalGlobals::getQuestInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumQuestInfos());
	return *(m_paQuestInfo[iIndex]);
}

int cvInternalGlobals::getNumTutorialInfos()
{
	return (int)m_paTutorialInfo.size();
}

std::vector<CvTutorialInfo*>& cvInternalGlobals::getTutorialInfo()
{
	return m_paTutorialInfo;
}

CvTutorialInfo& cvInternalGlobals::getTutorialInfo(int iIndex)
{
	FAssert(iIndex > -1);
	FAssert(iIndex < GC.getNumTutorialInfos());
	return *(m_paTutorialInfo[iIndex]);
}

int cvInternalGlobals::getNumEventTriggerInfos()
{
	return (int)m_paEventTriggerInfo.size();
}

std::vector<CvEventTriggerInfo*>& cvInternalGlobals::getEventTriggerInfo()
{
	return m_paEventTriggerInfo;
}

CvEventTriggerInfo& cvInternalGlobals::getEventTriggerInfo(EventTriggerTypes eEventTrigger)
{
	FAssert(eEventTrigger > -1);
	FAssert(eEventTrigger < GC.getNumEventTriggerInfos());
	return *(m_paEventTriggerInfo[eEventTrigger]);
}

int cvInternalGlobals::getNumEventInfos()
{
	return (int)m_paEventInfo.size();
}

std::vector<CvEventInfo*>& cvInternalGlobals::getEventInfo()
{
	return m_paEventInfo;
}

CvEventInfo& cvInternalGlobals::getEventInfo(EventTypes eEvent)
{
	FAssert(eEvent > -1);
	FAssert(eEvent < GC.getNumEventInfos());
	return *(m_paEventInfo[eEvent]);
}

int cvInternalGlobals::getNumEspionageMissionInfos()
{
	return (int)m_paEspionageMissionInfo.size();
}

std::vector<CvEspionageMissionInfo*>& cvInternalGlobals::getEspionageMissionInfo()
{
	return m_paEspionageMissionInfo;
}

CvEspionageMissionInfo& cvInternalGlobals::getEspionageMissionInfo(EspionageMissionTypes eEspionageMissionNum)
{
	FAssert(eEspionageMissionNum > -1);
	FAssert(eEspionageMissionNum < GC.getNumEspionageMissionInfos());
	return *(m_paEspionageMissionInfo[eEspionageMissionNum]);
}

int& cvInternalGlobals::getNumEntityEventTypes()
{
	return m_iNumEntityEventTypes;
}

CvString*& cvInternalGlobals::getEntityEventTypes()
{
	return m_paszEntityEventTypes;
}

CvString& cvInternalGlobals::getEntityEventTypes(EntityEventTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumEntityEventTypes());
	return m_paszEntityEventTypes[e];
}

int& cvInternalGlobals::getNumAnimationOperatorTypes()
{
	return m_iNumAnimationOperatorTypes;
}

CvString*& cvInternalGlobals::getAnimationOperatorTypes()
{
	return m_paszAnimationOperatorTypes;
}

CvString& cvInternalGlobals::getAnimationOperatorTypes(AnimationOperatorTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumAnimationOperatorTypes());
	return m_paszAnimationOperatorTypes[e];
}

CvString*& cvInternalGlobals::getFunctionTypes()
{
	return m_paszFunctionTypes;
}

CvString& cvInternalGlobals::getFunctionTypes(FunctionTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_FUNC_TYPES);
	return m_paszFunctionTypes[e];
}

int& cvInternalGlobals::getNumFlavorTypes()
{
	return m_iNumFlavorTypes;
}

CvString*& cvInternalGlobals::getFlavorTypes()
{
	return m_paszFlavorTypes;
}

CvString& cvInternalGlobals::getFlavorTypes(FlavorTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumFlavorTypes());
	return m_paszFlavorTypes[e];
}

int& cvInternalGlobals::getNumArtStyleTypes()
{
	return m_iNumArtStyleTypes;
}

CvString*& cvInternalGlobals::getArtStyleTypes()
{
	return m_paszArtStyleTypes;
}

CvString& cvInternalGlobals::getArtStyleTypes(ArtStyleTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumArtStyleTypes());
	return m_paszArtStyleTypes[e];
}

int cvInternalGlobals::getNumUnitArtStyleTypeInfos()
{
    return (int)m_paUnitArtStyleTypeInfo.size();
}

std::vector<CvUnitArtStyleTypeInfo*>& cvInternalGlobals::getUnitArtStyleTypeInfo()
{
	return m_paUnitArtStyleTypeInfo;
}

CvUnitArtStyleTypeInfo& cvInternalGlobals::getUnitArtStyleTypeInfo(UnitArtStyleTypes eUnitArtStyleTypeNum)
{
	FAssert(eUnitArtStyleTypeNum > -1);
	FAssert(eUnitArtStyleTypeNum < GC.getNumUnitArtStyleTypeInfos());
	return *(m_paUnitArtStyleTypeInfo[eUnitArtStyleTypeNum]);
}

int& cvInternalGlobals::getNumCitySizeTypes()
{
	return m_iNumCitySizeTypes;
}

CvString*& cvInternalGlobals::getCitySizeTypes()
{
	return m_paszCitySizeTypes;
}

CvString& cvInternalGlobals::getCitySizeTypes(int i)
{
	FAssertMsg(i < getNumCitySizeTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszCitySizeTypes[i];
}

CvString*& cvInternalGlobals::getContactTypes()
{
	return m_paszContactTypes;
}

CvString& cvInternalGlobals::getContactTypes(ContactTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_CONTACT_TYPES);
	return m_paszContactTypes[e];
}

CvString*& cvInternalGlobals::getDiplomacyPowerTypes()
{
	return m_paszDiplomacyPowerTypes;
}

CvString& cvInternalGlobals::getDiplomacyPowerTypes(DiplomacyPowerTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_DIPLOMACYPOWER_TYPES);
	return m_paszDiplomacyPowerTypes[e];
}

CvString*& cvInternalGlobals::getAutomateTypes()
{
	return m_paszAutomateTypes;
}

CvString& cvInternalGlobals::getAutomateTypes(AutomateTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_AUTOMATE_TYPES);
	return m_paszAutomateTypes[e];
}

CvString*& cvInternalGlobals::getDirectionTypes()
{
	return m_paszDirectionTypes;
}

CvString& cvInternalGlobals::getDirectionTypes(AutomateTypes e)
{
	FAssert(e > -1);
	FAssert(e < NUM_DIRECTION_TYPES);
	return m_paszDirectionTypes[e];
}

int cvInternalGlobals::getNumCulturalAgeInfos()
{
	return (int)m_paCulturalAgeInfo.size();
}

std::vector<CvCulturalAgeInfo*>& cvInternalGlobals::getCulturalAgeInfo()
{
	return m_paCulturalAgeInfo;
}

CvCulturalAgeInfo& cvInternalGlobals::getCulturalAgeInfo(CulturalAgeTypes eCulturalAgeNum)
{
	FAssert(eCulturalAgeNum > -1);
	FAssert(eCulturalAgeNum < GC.getNumCulturalAgeInfos());
	return *(m_paCulturalAgeInfo[eCulturalAgeNum]);
}

int cvInternalGlobals::getNumPropertyInfos()
{
	return (int)m_paPropertyInfo.size();
}

std::vector<CvPropertyInfo*>& cvInternalGlobals::getPropertyInfo()
{
	return m_paPropertyInfo;
}

CvPropertyInfo& cvInternalGlobals::getPropertyInfo(PropertyTypes ePropertyNum)
{
	FAssert(ePropertyNum > -1);
	FAssert(ePropertyNum < GC.getNumPropertyInfos());
	return *(m_paPropertyInfo[ePropertyNum]);
}

int cvInternalGlobals::getNumOutcomeInfos()
{
	return (int)m_paOutcomeInfo.size();
}

std::vector<CvOutcomeInfo*>& cvInternalGlobals::getOutcomeInfo()
{
	return m_paOutcomeInfo;
}

CvOutcomeInfo& cvInternalGlobals::getOutcomeInfo(OutcomeTypes eOutcomeNum)
{
	FAssert(eOutcomeNum > -1);
	FAssert(eOutcomeNum < GC.getNumOutcomeInfos());
	return *(m_paOutcomeInfo[eOutcomeNum]);
}

int& cvInternalGlobals::getNumFootstepAudioTypes()
{
	return m_iNumFootstepAudioTypes;
}

CvString*& cvInternalGlobals::getFootstepAudioTypes()
{
	return m_paszFootstepAudioTypes;
}

CvString& cvInternalGlobals::getFootstepAudioTypes(int i)
{
	FAssertMsg(i < getNumFootstepAudioTypes(), "Index out of bounds");
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszFootstepAudioTypes[i];
}

int cvInternalGlobals::getFootstepAudioTypeByTag(CvString strTag)
{
	int iIndex = -1;

	if ( strTag.GetLength() <= 0 )
	{
		return iIndex;
	}

	for ( int i = 0; i < m_iNumFootstepAudioTypes; i++ )
	{
		if ( strTag.CompareNoCase(m_paszFootstepAudioTypes[i]) == 0 )
		{
			iIndex = i;
			break;
		}
	}

	return iIndex;
}

CvString*& cvInternalGlobals::getFootstepAudioTags()
{
	return m_paszFootstepAudioTags;
}

CvString& cvInternalGlobals::getFootstepAudioTags(int i)
{
	static CvString*	emptyString = NULL;

	if ( emptyString == NULL )
	{
		emptyString = new CvString("");
	}
	FAssertMsg(i < GC.getNumFootstepAudioTypes(), "Index out of bounds")
	FAssertMsg(i > -1, "Index out of bounds");
	return m_paszFootstepAudioTags ? m_paszFootstepAudioTags[i] : *emptyString;
}

void cvInternalGlobals::setCurrentXMLFile(const TCHAR* szFileName)
{
	m_szCurrentXMLFile = szFileName;
}

CvString& cvInternalGlobals::getCurrentXMLFile()
{
	return m_szCurrentXMLFile;
}

FVariableSystem* cvInternalGlobals::getDefinesVarSystem()
{
	return m_VarSystem;
}

void cvInternalGlobals::cacheGlobals()
{
/************************************************************************************************/
/* Mod Globals    Start                          09/13/10                           phungus420  */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	m_bDCM_BATTLE_EFFECTS = (getDefineINT("DCM_BATTLE_EFFECTS") > 0) ? true : false;
	m_iBATTLE_EFFECT_LESS_FOOD = getDefineINT("BATTLE_EFFECT_LESS_FOOD");
	m_iBATTLE_EFFECT_LESS_PRODUCTION = getDefineINT("BATTLE_EFFECT_LESS_PRODUCTION");
	m_iBATTLE_EFFECT_LESS_COMMERCE = getDefineINT("BATTLE_EFFECT_LESS_COMMERCE");
	m_iBATTLE_EFFECTS_MINIMUM_TURN_INCREMENTS = getDefineINT("BATTLE_EFFECTS_MINIMUM_TURN_INCREMENTS");
	m_iMAX_BATTLE_TURNS = getDefineINT("MAX_BATTLE_TURNS");

	m_bDCM_AIR_BOMBING = (getDefineINT("DCM_AIR_BOMBING") > 0) ? true : false;
	m_bDCM_RANGE_BOMBARD = (getDefineINT("DCM_RANGE_BOMBARD") > 0) ? true : false;
	m_iDCM_RB_CITY_INACCURACY = getDefineINT("DCM_RB_CITY_INACCURACY");
	m_iDCM_RB_CITYBOMBARD_CHANCE = getDefineINT("DCM_RB_CITYBOMBARD_CHANCE");
	m_bDCM_ATTACK_SUPPORT = (getDefineINT("DCM_ATTACK_SUPPORT") > 0) ? true : false;
	m_bDCM_STACK_ATTACK = (getDefineINT("DCM_STACK_ATTACK") > 0) ? true : false;
	m_bDCM_OPP_FIRE = (getDefineINT("DCM_OPP_FIRE") > 0) ? true : false;
	m_bDCM_ACTIVE_DEFENSE = (getDefineINT("DCM_ACTIVE_DEFENSE") > 0) ? true : false;
	m_bDCM_ARCHER_BOMBARD = (getDefineINT("DCM_ARCHER_BOMBARD") > 0) ? true : false;
	m_bDCM_FIGHTER_ENGAGE = (getDefineINT("DCM_FIGHTER_ENGAGE") > 0) ? true : false;

	m_bDYNAMIC_CIV_NAMES = (getDefineINT("DYNAMIC_CIV_NAMES") > 0) ? true : false;

	m_bLIMITED_RELIGIONS_EXCEPTIONS = (getDefineINT("LIMITED_RELIGIONS_EXCEPTIONS") > 0) ? true : false;
	m_bOC_RESPAWN_HOLY_CITIES = (getDefineINT("OC_RESPAWN_HOLY_CITIES") > 0) ? true : false;

	m_bIDW_ENABLED = (getDefineINT("IDW_ENABLED") > 0) ? true : false;
	m_fIDW_BASE_COMBAT_INFLUENCE = getDefineFLOAT("IDW_BASE_COMBAT_INFLUENCE");
	m_fIDW_NO_CITY_DEFENDER_MULTIPLIER = getDefineFLOAT("IDW_NO_CITY_DEFENDER_MULTIPLIER");
	m_fIDW_FORT_CAPTURE_MULTIPLIER = getDefineFLOAT("IDW_FORT_CAPTURE_MULTIPLIER");
	m_fIDW_EXPERIENCE_FACTOR = getDefineFLOAT("IDW_EXPERIENCE_FACTOR");
	m_fIDW_WARLORD_MULTIPLIER = getDefineFLOAT("IDW_WARLORD_MULTIPLIER");
	m_iIDW_INFLUENCE_RADIUS = getDefineINT("IDW_INFLUENCE_RADIUS");
	m_fIDW_PLOT_DISTANCE_FACTOR = getDefineFLOAT("IDW_PLOT_DISTANCE_FACTOR");
	m_fIDW_WINNER_PLOT_MULTIPLIER = getDefineFLOAT("IDW_WINNER_PLOT_MULTIPLIER");
	m_fIDW_LOSER_PLOT_MULTIPLIER = getDefineFLOAT("IDW_LOSER_PLOT_MULTIPLIER");
	m_bIDW_EMERGENCY_DRAFT_ENABLED = (getDefineINT("IDW_EMERGENCY_DRAFT_ENABLED") > 0) ? true : false;
	m_iIDW_EMERGENCY_DRAFT_MIN_POPULATION = (getDefineINT("IDW_EMERGENCY_DRAFT_MIN_POPULATION") > 1) ? getDefineINT("IDW_EMERGENCY_DRAFT_ENABLED") : 2;
	m_fIDW_EMERGENCY_DRAFT_STRENGTH = getDefineFLOAT("IDW_EMERGENCY_DRAFT_STRENGTH");
	m_fIDW_EMERGENCY_DRAFT_ANGER_MULTIPLIER = getDefineFLOAT("IDW_EMERGENCY_DRAFT_ANGER_MULTIPLIER");
	m_bIDW_NO_BARBARIAN_INFLUENCE = (getDefineINT("IDW_NO_BARBARIAN_INFLUENCE") > 0) ? true : false;
	m_bIDW_NO_NAVAL_INFLUENCE = (getDefineINT("IDW_NO_NAVAL_INFLUENCE") > 0) ? true : false;
	m_bIDW_PILLAGE_INFLUENCE_ENABLED = (getDefineINT("IDW_PILLAGE_INFLUENCE_ENABLED") > 0) ? true : false;
	m_fIDW_BASE_PILLAGE_INFLUENCE = getDefineFLOAT("IDW_BASE_PILLAGE_INFLUENCE");
	m_fIDW_CITY_TILE_MULTIPLIER = getDefineFLOAT("IDW_CITY_TILE_MULTIPLIER");

	m_bSS_ENABLED = (getDefineINT("SS_ENABLED") > 0) ? true : false;
	m_bSS_BRIBE = (getDefineINT("SS_BRIBE") > 0) ? true : false;
	m_bSS_ASSASSINATE = (getDefineINT("SS_ASSASSINATE") > 0) ? true : false;
/************************************************************************************************/
/* Mod Globals                        END                                           phungus420  */
/************************************************************************************************/
	m_iMOVE_DENOMINATOR = getDefineINT("MOVE_DENOMINATOR");
	m_iNUM_UNIT_PREREQ_OR_BONUSES = getDefineINT("NUM_UNIT_PREREQ_OR_BONUSES");
	m_iNUM_BUILDING_PREREQ_OR_BONUSES = getDefineINT("NUM_BUILDING_PREREQ_OR_BONUSES");
	m_iFOOD_CONSUMPTION_PER_POPULATION = getDefineINT("FOOD_CONSUMPTION_PER_POPULATION");
	m_iMAX_HIT_POINTS = getDefineINT("MAX_HIT_POINTS");
	m_iPATH_DAMAGE_WEIGHT = getDefineINT("PATH_DAMAGE_WEIGHT");
	m_iHILLS_EXTRA_DEFENSE = getDefineINT("HILLS_EXTRA_DEFENSE");
	m_iRIVER_ATTACK_MODIFIER = getDefineINT("RIVER_ATTACK_MODIFIER");
	m_iAMPHIB_ATTACK_MODIFIER = getDefineINT("AMPHIB_ATTACK_MODIFIER");
	m_iHILLS_EXTRA_MOVEMENT = getDefineINT("HILLS_EXTRA_MOVEMENT");
	m_iMAX_PLOT_LIST_ROWS = getDefineINT("MAX_PLOT_LIST_ROWS");
	m_iUNIT_MULTISELECT_MAX = getDefineINT("UNIT_MULTISELECT_MAX");
	m_iPERCENT_ANGER_DIVISOR = getDefineINT("PERCENT_ANGER_DIVISOR");
	m_iEVENT_MESSAGE_TIME = getDefineINT("EVENT_MESSAGE_TIME");
	m_iROUTE_FEATURE_GROWTH_MODIFIER = getDefineINT("ROUTE_FEATURE_GROWTH_MODIFIER");
	m_iFEATURE_GROWTH_MODIFIER = getDefineINT("FEATURE_GROWTH_MODIFIER");
	m_iMIN_CITY_RANGE = getDefineINT("MIN_CITY_RANGE");
	m_iCITY_MAX_NUM_BUILDINGS = getDefineINT("CITY_MAX_NUM_BUILDINGS");
	m_iNUM_UNIT_AND_TECH_PREREQS = getDefineINT("NUM_UNIT_AND_TECH_PREREQS");
	m_iNUM_AND_TECH_PREREQS = getDefineINT("NUM_AND_TECH_PREREQS");
	m_iNUM_OR_TECH_PREREQS = getDefineINT("NUM_OR_TECH_PREREQS");
	m_iLAKE_MAX_AREA_SIZE = getDefineINT("LAKE_MAX_AREA_SIZE");
	m_iNUM_ROUTE_PREREQ_OR_BONUSES = getDefineINT("NUM_ROUTE_PREREQ_OR_BONUSES");
	m_iNUM_BUILDING_AND_TECH_PREREQS = getDefineINT("NUM_BUILDING_AND_TECH_PREREQS");
	m_iMIN_WATER_SIZE_FOR_OCEAN = getDefineINT("MIN_WATER_SIZE_FOR_OCEAN");
	m_iFORTIFY_MODIFIER_PER_TURN = getDefineINT("FORTIFY_MODIFIER_PER_TURN");
	m_iMAX_CITY_DEFENSE_DAMAGE = getDefineINT("MAX_CITY_DEFENSE_DAMAGE");
	m_iNUM_CORPORATION_PREREQ_BONUSES = getDefineINT("NUM_CORPORATION_PREREQ_BONUSES");
	m_iPEAK_SEE_THROUGH_CHANGE = getDefineINT("PEAK_SEE_THROUGH_CHANGE");
	m_iHILLS_SEE_THROUGH_CHANGE = getDefineINT("HILLS_SEE_THROUGH_CHANGE");
	m_iSEAWATER_SEE_FROM_CHANGE = getDefineINT("SEAWATER_SEE_FROM_CHANGE");
	m_iPEAK_SEE_FROM_CHANGE = getDefineINT("PEAK_SEE_FROM_CHANGE");
	m_iHILLS_SEE_FROM_CHANGE = getDefineINT("HILLS_SEE_FROM_CHANGE");
	m_iUSE_SPIES_NO_ENTER_BORDERS = getDefineINT("USE_SPIES_NO_ENTER_BORDERS");
	
	m_fCAMERA_MIN_YAW = getDefineFLOAT("CAMERA_MIN_YAW");
	m_fCAMERA_MAX_YAW = getDefineFLOAT("CAMERA_MAX_YAW");
	m_fCAMERA_FAR_CLIP_Z_HEIGHT = getDefineFLOAT("CAMERA_FAR_CLIP_Z_HEIGHT");
	m_fCAMERA_MAX_TRAVEL_DISTANCE = getDefineFLOAT("CAMERA_MAX_TRAVEL_DISTANCE");
	m_fCAMERA_START_DISTANCE = getDefineFLOAT("CAMERA_START_DISTANCE");
	m_fAIR_BOMB_HEIGHT = getDefineFLOAT("AIR_BOMB_HEIGHT");
	m_fPLOT_SIZE = getDefineFLOAT("PLOT_SIZE");
	m_fCAMERA_SPECIAL_PITCH = getDefineFLOAT("CAMERA_SPECIAL_PITCH");
	m_fCAMERA_MAX_TURN_OFFSET = getDefineFLOAT("CAMERA_MAX_TURN_OFFSET");
	m_fCAMERA_MIN_DISTANCE = getDefineFLOAT("CAMERA_MIN_DISTANCE");
	m_fCAMERA_UPPER_PITCH = getDefineFLOAT("CAMERA_UPPER_PITCH");
	m_fCAMERA_LOWER_PITCH = getDefineFLOAT("CAMERA_LOWER_PITCH");
	m_fFIELD_OF_VIEW = getDefineFLOAT("FIELD_OF_VIEW");
	m_fSHADOW_SCALE = getDefineFLOAT("SHADOW_SCALE");
	m_fUNIT_MULTISELECT_DISTANCE = getDefineFLOAT("UNIT_MULTISELECT_DISTANCE");

	m_iUSE_CANNOT_FOUND_CITY_CALLBACK = getDefineINT("USE_CANNOT_FOUND_CITY_CALLBACK");
	m_iUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK = getDefineINT("USE_CAN_FOUND_CITIES_ON_WATER_CALLBACK");
	m_iUSE_IS_PLAYER_RESEARCH_CALLBACK = getDefineINT("USE_IS_PLAYER_RESEARCH_CALLBACK");
	m_iUSE_CAN_RESEARCH_CALLBACK = getDefineINT("USE_CAN_RESEARCH_CALLBACK");
	m_iUSE_CANNOT_DO_CIVIC_CALLBACK = getDefineINT("USE_CANNOT_DO_CIVIC_CALLBACK");
	m_iUSE_CAN_DO_CIVIC_CALLBACK = getDefineINT("USE_CAN_DO_CIVIC_CALLBACK");
	m_iUSE_CANNOT_CONSTRUCT_CALLBACK = getDefineINT("USE_CANNOT_CONSTRUCT_CALLBACK");
	m_iUSE_CAN_CONSTRUCT_CALLBACK = getDefineINT("USE_CAN_CONSTRUCT_CALLBACK");
	m_iUSE_CAN_DECLARE_WAR_CALLBACK = getDefineINT("USE_CAN_DECLARE_WAR_CALLBACK");
	m_iUSE_CANNOT_RESEARCH_CALLBACK = getDefineINT("USE_CANNOT_RESEARCH_CALLBACK");
	m_iUSE_GET_UNIT_COST_MOD_CALLBACK = getDefineINT("USE_GET_UNIT_COST_MOD_CALLBACK");
	m_iUSE_GET_BUILDING_COST_MOD_CALLBACK = getDefineINT("USE_GET_BUILDING_COST_MOD_CALLBACK");
	m_iUSE_GET_CITY_FOUND_VALUE_CALLBACK = getDefineINT("USE_GET_CITY_FOUND_VALUE_CALLBACK");
	m_iUSE_CANNOT_HANDLE_ACTION_CALLBACK = getDefineINT("USE_CANNOT_HANDLE_ACTION_CALLBACK");
	m_iUSE_CAN_BUILD_CALLBACK = getDefineINT("USE_CAN_BUILD_CALLBACK");
	m_iUSE_CANNOT_TRAIN_CALLBACK = getDefineINT("USE_CANNOT_TRAIN_CALLBACK");
	m_iUSE_CAN_TRAIN_CALLBACK = getDefineINT("USE_CAN_TRAIN_CALLBACK");
	m_iUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK = getDefineINT("USE_UNIT_CANNOT_MOVE_INTO_CALLBACK");
	m_iUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK = getDefineINT("USE_USE_CANNOT_SPREAD_RELIGION_CALLBACK");
	m_iUSE_FINISH_TEXT_CALLBACK = getDefineINT("USE_FINISH_TEXT_CALLBACK");
	m_iUSE_ON_UNIT_SET_XY_CALLBACK = getDefineINT("USE_ON_UNIT_SET_XY_CALLBACK");
	m_iUSE_ON_UNIT_SELECTED_CALLBACK = getDefineINT("USE_ON_UNIT_SELECTED_CALLBACK");
	m_iUSE_ON_UPDATE_CALLBACK = getDefineINT("USE_ON_UPDATE_CALLBACK");
	m_iUSE_ON_UNIT_CREATED_CALLBACK = getDefineINT("USE_ON_UNIT_CREATED_CALLBACK");
	m_iUSE_ON_UNIT_LOST_CALLBACK = getDefineINT("USE_ON_UNIT_LOST_CALLBACK");

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Efficiency, Options                                                                          */
/************************************************************************************************/
// BBAI Options
	m_bBBAI_AIR_COMBAT = !(getDefineINT("BBAI_AIR_COMBAT") == 0);
	m_bBBAI_HUMAN_VASSAL_WAR_BUILD = !(getDefineINT("BBAI_HUMAN_VASSAL_WAR_BUILD") == 0);
	m_iBBAI_DEFENSIVE_PACT_BEHAVIOR = getDefineINT("BBAI_DEFENSIVE_PACT_BEHAVIOR");
	m_bBBAI_HUMAN_AS_VASSAL_OPTION = !(getDefineINT("BBAI_HUMAN_AS_VASSAL_OPTION") == 0);

// BBAI AI Variables
	m_iWAR_SUCCESS_CITY_CAPTURING = getDefineINT("WAR_SUCCESS_CITY_CAPTURING", m_iWAR_SUCCESS_CITY_CAPTURING);
	m_iBBAI_ATTACK_CITY_STACK_RATIO = getDefineINT("BBAI_ATTACK_CITY_STACK_RATIO", m_iBBAI_ATTACK_CITY_STACK_RATIO);
	m_iBBAI_SKIP_BOMBARD_BEST_ATTACK_ODDS = getDefineINT("BBAI_SKIP_BOMBARD_BEST_ATTACK_ODDS", m_iBBAI_SKIP_BOMBARD_BEST_ATTACK_ODDS);
	m_iBBAI_SKIP_BOMBARD_BASE_STACK_RATIO = getDefineINT("BBAI_SKIP_BOMBARD_BASE_STACK_RATIO", m_iBBAI_SKIP_BOMBARD_BASE_STACK_RATIO);
	m_iBBAI_SKIP_BOMBARD_MIN_STACK_RATIO = getDefineINT("BBAI_SKIP_BOMBARD_MIN_STACK_RATIO", m_iBBAI_SKIP_BOMBARD_MIN_STACK_RATIO);

// Tech Diffusion
	m_bTECH_DIFFUSION_ENABLE = !(getDefineINT("TECH_DIFFUSION_ENABLE") == 0);
	m_iTECH_DIFFUSION_KNOWN_TEAM_MODIFIER = getDefineINT("TECH_DIFFUSION_KNOWN_TEAM_MODIFIER", m_iTECH_DIFFUSION_KNOWN_TEAM_MODIFIER);
	m_iTECH_DIFFUSION_WELFARE_THRESHOLD = getDefineINT("TECH_DIFFUSION_WELFARE_THRESHOLD", m_iTECH_DIFFUSION_WELFARE_THRESHOLD);
	m_iTECH_DIFFUSION_WELFARE_MODIFIER = getDefineINT("TECH_DIFFUSION_WELFARE_MODIFIER", m_iTECH_DIFFUSION_WELFARE_MODIFIER);
	m_iTECH_COST_FIRST_KNOWN_PREREQ_MODIFIER = getDefineINT("TECH_COST_FIRST_KNOWN_PREREQ_MODIFIER", m_iTECH_COST_FIRST_KNOWN_PREREQ_MODIFIER);
	m_iTECH_COST_KNOWN_PREREQ_MODIFIER = getDefineINT("TECH_COST_KNOWN_PREREQ_MODIFIER", m_iTECH_COST_KNOWN_PREREQ_MODIFIER);
	m_iTECH_COST_MODIFIER = getDefineINT("TECH_COST_MODIFIER", m_iTECH_COST_MODIFIER);
	
// From Lead From Behind by UncutDragon
// Lead from Behind flags
	m_bLFBEnable = !(getDefineINT("LFB_ENABLE") == 0);
	m_iLFBBasedOnGeneral = getDefineINT("LFB_BASEDONGENERAL");
	m_iLFBBasedOnExperience = getDefineINT("LFB_BASEDONEXPERIENCE");
	m_iLFBBasedOnLimited = getDefineINT("LFB_BASEDONLIMITED");
	m_iLFBBasedOnHealer = getDefineINT("LFB_BASEDONHEALER");
	m_iLFBBasedOnAverage = std::max(1, (m_iLFBBasedOnGeneral+m_iLFBBasedOnLimited+m_iLFBBasedOnHealer+(5*m_iLFBBasedOnExperience)+3)/4);
	m_bLFBUseSlidingScale = !(getDefineINT("LFB_USESLIDINGSCALE") == 0);
	m_iLFBAdjustNumerator = getDefineINT("LFB_ADJUSTNUMERATOR");
	m_iLFBAdjustDenominator = getDefineINT("LFB_ADJUSTDENOMINATOR");
	m_bLFBUseCombatOdds = !(getDefineINT("LFB_USECOMBATODDS") == 0);
	m_iCOMBAT_DIE_SIDES = getDefineINT("COMBAT_DIE_SIDES");
	m_iCOMBAT_DAMAGE = getDefineINT("COMBAT_DAMAGE");
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* Afforess	                  Start		 12/8/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	m_iPEAK_EXTRA_MOVEMENT = getDefineINT("PEAK_EXTRA_MOVEMENT");
	m_iPEAK_EXTRA_DEFENSE = getDefineINT("PEAK_EXTRA_DEFENSE");
	//m_bBetterRoMEnabled = getDefineINT("BETTER_ROM");
	m_bFormationsMod = getDefineINT("FORMATIONS");
	m_bXMLLogging = getDefineINT("XML_LOGGING_ENABLED");
	m_iSCORE_FREE_PERCENT = getDefineINT("SCORE_FREE_PERCENT");
	m_iSCORE_POPULATION_FACTOR = getDefineINT("SCORE_POPULATION_FACTOR");
	m_iSCORE_LAND_FACTOR = getDefineINT("SCORE_LAND_FACTOR");
	m_iSCORE_TECH_FACTOR = getDefineINT("SCORE_TECH_FACTOR");
	m_iSCORE_WONDER_FACTOR = getDefineINT("SCORE_WONDER_FACTOR");
	
	//New Python Callbacks
	m_iUSE_CAN_CREATE_PROJECT_CALLBACK = getDefineINT("USE_CAN_CREATE_PROJECT_CALLBACK");
	m_iUSE_CANNOT_CREATE_PROJECT_CALLBACK = getDefineINT("USE_CANNOT_CREATE_PROJECT_CALLBACK");
	m_iUSE_CAN_DO_MELTDOWN_CALLBACK = getDefineINT("USE_CAN_DO_MELTDOWN_CALLBACK");
	m_iUSE_CAN_MAINTAIN_PROCESS_CALLBACK = getDefineINT("USE_CAN_MAINTAIN_PROCESS_CALLBACK");
	m_iUSE_CANNOT_MAINTAIN_PROCESS_CALLBACK = getDefineINT("USE_CANNOT_MAINTAIN_PROCESS_CALLBACK");
	m_iUSE_CAN_DO_GROWTH_CALLBACK = getDefineINT("USE_CAN_DO_GROWTH_CALLBACK");
	m_iUSE_CAN_DO_CULTURE_CALLBACK = getDefineINT("USE_CAN_DO_CULTURE_CALLBACK");
	m_iUSE_CAN_DO_PLOT_CULTURE_CALLBACK = getDefineINT("USE_CAN_DO_PLOT_CULTURE_CALLBACK");
	m_iUSE_CAN_DO_PRODUCTION_CALLBACK = getDefineINT("USE_CAN_DO_PRODUCTION_CALLBACK");
	m_iUSE_CAN_DO_RELIGION_CALLBACK = getDefineINT("USE_CAN_DO_RELIGION_CALLBACK");
	m_iUSE_CAN_DO_GREATPEOPLE_CALLBACK = getDefineINT("USE_CAN_DO_GREATPEOPLE_CALLBACK");
	m_iUSE_CAN_RAZE_CITY_CALLBACK = getDefineINT("USE_CAN_RAZE_CITY_CALLBACK");
	m_iUSE_CAN_DO_GOLD_CALLBACK = getDefineINT("USE_CAN_DO_GOLD_CALLBACK");
	m_iUSE_CAN_DO_RESEARCH_CALLBACK = getDefineINT("USE_CAN_DO_RESEARCH_CALLBACK");
	m_iUSE_UPGRADE_UNIT_PRICE_CALLBACK = getDefineINT("USE_UPGRADE_UNIT_PRICE_CALLBACK");
	m_iUSE_IS_VICTORY_CALLBACK = getDefineINT("USE_IS_VICTORY_CALLBACK");
	m_iUSE_AI_UPDATE_UNIT_CALLBACK = getDefineINT("USE_AI_UPDATE_UNIT_CALLBACK");
	m_iUSE_AI_CHOOSE_PRODUCTION_CALLBACK = getDefineINT("USE_AI_CHOOSE_PRODUCTION_CALLBACK");
	m_iUSE_EXTRA_PLAYER_COSTS_CALLBACK = getDefineINT("USE_EXTRA_PLAYER_COSTS_CALLBACK");
	m_iUSE_AI_DO_DIPLO_CALLBACK = getDefineINT("USE_AI_DO_DIPLO_CALLBACK");
	m_iUSE_AI_BESTTECH_CALLBACK = getDefineINT("USE_AI_BESTTECH_CALLBACK");
	m_iUSE_CAN_DO_COMBAT_CALLBACK = getDefineINT("USE_CAN_DO_COMBAT_CALLBACK");
	m_iUSE_AI_CAN_DO_WARPLANS_CALLBACK = getDefineINT("USE_AI_CAN_DO_WARPLANS_CALLBACK");
	
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
/************************************************************************************************/
/* MODULES                                 11/13/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	m_iTGA_RELIGIONS = getDefineINT("GAMEFONT_TGA_RELIGIONS");													// GAMEFONT_TGA_RELIGIONS
	m_iTGA_CORPORATIONS = getDefineINT("GAMEFONT_TGA_CORPORATIONS");											// GAMEFONT_TGA_CORPORATIONS
/************************************************************************************************/
/* MODULES                                 END                                                  */
/************************************************************************************************/
}
/************************************************************************************************/
/* MOD COMPONENT CONTROL                   08/02/07                            MRGENIE          */
/*                                                                                              */
/* Return true/false from                                                                       */
/************************************************************************************************/
bool cvInternalGlobals::getDefineBOOL( const char * szName ) const
{
	bool bReturn = false;
	GC.getDefinesVarSystem()->GetValue( szName, bReturn );
	return bReturn;
}
/************************************************************************************************/
/* MOD COMPONENT CONTROL                   END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
int cvInternalGlobals::getDefineINT( const char * szName, const int iDefault ) const
{
	int iReturn = 0;

	if( GC.getDefinesVarSystem()->GetValue( szName, iReturn ) )
	{
		return iReturn;
	}

	return iDefault;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/


int cvInternalGlobals::getDefineINT( const char * szName ) const
{
	int iReturn = 0;
	GC.getDefinesVarSystem()->GetValue( szName, iReturn );
	return iReturn;
}

float cvInternalGlobals::getDefineFLOAT( const char * szName ) const
{
	float fReturn = 0;
	GC.getDefinesVarSystem()->GetValue( szName, fReturn );
	return fReturn;
}

const char * cvInternalGlobals::getDefineSTRING( const char * szName ) const
{
	const char * szReturn = NULL;
	GC.getDefinesVarSystem()->GetValue( szName, szReturn );
	return szReturn;
}
/************************************************************************************************/
/* Afforess	                  Start		 08/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
void cvInternalGlobals::setDefineINT( const char * szName, int iValue, bool bUpdate )
{
	if (getDefineINT(szName) != iValue)
	{
		if (bUpdate)
			CvMessageControl::getInstance().sendGlobalDefineUpdate(szName, iValue, -1.0f, "");
		else
			GC.getDefinesVarSystem()->SetValue( szName, iValue );
		cacheGlobals();
			
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
}
/************************************************************************************************/
/* Afforess	                  Start		 08/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
void cvInternalGlobals::setDefineFLOAT( const char * szName, float fValue, bool bUpdate )
{

	if (getDefineFLOAT(szName) != fValue)
	{
		if (bUpdate)
			CvMessageControl::getInstance().sendGlobalDefineUpdate(szName, -1, fValue, "");
		else
			GC.getDefinesVarSystem()->SetValue( szName, fValue );
		cacheGlobals();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
}
/************************************************************************************************/
/* Afforess	                  Start		 08/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
void cvInternalGlobals::setDefineSTRING( const char * szName, const char * szValue, bool bUpdate )
{
	if (getDefineSTRING(szName) != szValue)
	{
		if (bUpdate)
			CvMessageControl::getInstance().sendGlobalDefineUpdate(szName, -1, -1.0f, szValue);
		else
			GC.getDefinesVarSystem()->SetValue( szName, szValue );
		cacheGlobals();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
}

int cvInternalGlobals::getMOVE_DENOMINATOR()
{
	return m_iMOVE_DENOMINATOR;
}

int cvInternalGlobals::getNUM_UNIT_PREREQ_OR_BONUSES()
{
	return m_iNUM_UNIT_PREREQ_OR_BONUSES;
}

int cvInternalGlobals::getNUM_BUILDING_PREREQ_OR_BONUSES()
{
	return m_iNUM_BUILDING_PREREQ_OR_BONUSES;
}

int cvInternalGlobals::getFOOD_CONSUMPTION_PER_POPULATION()
{
	return m_iFOOD_CONSUMPTION_PER_POPULATION;
}

int cvInternalGlobals::getMAX_HIT_POINTS()
{
	return m_iMAX_HIT_POINTS;
}

int cvInternalGlobals::getPATH_DAMAGE_WEIGHT()
{
	return m_iPATH_DAMAGE_WEIGHT;
}

int cvInternalGlobals::getHILLS_EXTRA_DEFENSE()
{
	return m_iHILLS_EXTRA_DEFENSE;
}

int cvInternalGlobals::getRIVER_ATTACK_MODIFIER()
{
	return m_iRIVER_ATTACK_MODIFIER;
}

int cvInternalGlobals::getAMPHIB_ATTACK_MODIFIER()
{
	return m_iAMPHIB_ATTACK_MODIFIER;
}

int cvInternalGlobals::getHILLS_EXTRA_MOVEMENT()
{
	return m_iHILLS_EXTRA_MOVEMENT;
}

int cvInternalGlobals::getMAX_PLOT_LIST_ROWS()
{
	return m_iMAX_PLOT_LIST_ROWS;
}

int cvInternalGlobals::getUNIT_MULTISELECT_MAX()
{
	return m_iUNIT_MULTISELECT_MAX;
}

int cvInternalGlobals::getPERCENT_ANGER_DIVISOR()
{
	return m_iPERCENT_ANGER_DIVISOR;
}

int cvInternalGlobals::getEVENT_MESSAGE_TIME()
{
	return m_iEVENT_MESSAGE_TIME;
}

int cvInternalGlobals::getROUTE_FEATURE_GROWTH_MODIFIER()
{
	return m_iROUTE_FEATURE_GROWTH_MODIFIER;
}

int cvInternalGlobals::getFEATURE_GROWTH_MODIFIER()
{
	return m_iFEATURE_GROWTH_MODIFIER;
}

int cvInternalGlobals::getMIN_CITY_RANGE()
{
	return m_iMIN_CITY_RANGE;
}

int cvInternalGlobals::getCITY_MAX_NUM_BUILDINGS()
{
	return m_iCITY_MAX_NUM_BUILDINGS;
}

int cvInternalGlobals::getNUM_UNIT_AND_TECH_PREREQS()
{
	return m_iNUM_UNIT_AND_TECH_PREREQS;
}

int cvInternalGlobals::getNUM_AND_TECH_PREREQS()
{
	return m_iNUM_AND_TECH_PREREQS;
}

int cvInternalGlobals::getNUM_OR_TECH_PREREQS()
{
	return m_iNUM_OR_TECH_PREREQS;
}

int cvInternalGlobals::getLAKE_MAX_AREA_SIZE()
{
	return m_iLAKE_MAX_AREA_SIZE;
}

int cvInternalGlobals::getNUM_ROUTE_PREREQ_OR_BONUSES()
{
	return m_iNUM_ROUTE_PREREQ_OR_BONUSES;
}

int cvInternalGlobals::getNUM_BUILDING_AND_TECH_PREREQS()
{
	return m_iNUM_BUILDING_AND_TECH_PREREQS;
}

int cvInternalGlobals::getMIN_WATER_SIZE_FOR_OCEAN()
{
	return m_iMIN_WATER_SIZE_FOR_OCEAN;
}

int cvInternalGlobals::getFORTIFY_MODIFIER_PER_TURN()
{
	return m_iFORTIFY_MODIFIER_PER_TURN;
}

int cvInternalGlobals::getMAX_CITY_DEFENSE_DAMAGE()
{
	return m_iMAX_CITY_DEFENSE_DAMAGE;
}

int cvInternalGlobals::getPEAK_SEE_THROUGH_CHANGE()
{
	return m_iPEAK_SEE_THROUGH_CHANGE;
}

int cvInternalGlobals::getHILLS_SEE_THROUGH_CHANGE()
{
	return m_iHILLS_SEE_THROUGH_CHANGE;
}

int cvInternalGlobals::getSEAWATER_SEE_FROM_CHANGE()
{
	return m_iSEAWATER_SEE_FROM_CHANGE;
}

int cvInternalGlobals::getPEAK_SEE_FROM_CHANGE()
{
	return m_iPEAK_SEE_FROM_CHANGE;
}

int cvInternalGlobals::getHILLS_SEE_FROM_CHANGE()
{
	return m_iHILLS_SEE_FROM_CHANGE;
}

int cvInternalGlobals::getUSE_SPIES_NO_ENTER_BORDERS()
{
	return m_iUSE_SPIES_NO_ENTER_BORDERS;
}

int cvInternalGlobals::getNUM_CORPORATION_PREREQ_BONUSES()
{
	return m_iNUM_CORPORATION_PREREQ_BONUSES;
}

float cvInternalGlobals::getCAMERA_MIN_YAW()
{
	return m_fCAMERA_MIN_YAW;
}

float cvInternalGlobals::getCAMERA_MAX_YAW()
{
	return m_fCAMERA_MAX_YAW;
}

float cvInternalGlobals::getCAMERA_FAR_CLIP_Z_HEIGHT()
{
	return m_fCAMERA_FAR_CLIP_Z_HEIGHT;
}

float cvInternalGlobals::getCAMERA_MAX_TRAVEL_DISTANCE()
{
	return m_fCAMERA_MAX_TRAVEL_DISTANCE;
}

float cvInternalGlobals::getCAMERA_START_DISTANCE()
{
	return m_fCAMERA_START_DISTANCE;
}

float cvInternalGlobals::getAIR_BOMB_HEIGHT()
{
	return m_fAIR_BOMB_HEIGHT;
}

float cvInternalGlobals::getPLOT_SIZE()
{
	return m_fPLOT_SIZE;
}

float cvInternalGlobals::getCAMERA_SPECIAL_PITCH()
{
	return m_fCAMERA_SPECIAL_PITCH;
}

float cvInternalGlobals::getCAMERA_MAX_TURN_OFFSET()
{
	return m_fCAMERA_MAX_TURN_OFFSET;
}

float cvInternalGlobals::getCAMERA_MIN_DISTANCE()
{
	return m_fCAMERA_MIN_DISTANCE;
}

float cvInternalGlobals::getCAMERA_UPPER_PITCH()
{
	return m_fCAMERA_UPPER_PITCH;
}

float cvInternalGlobals::getCAMERA_LOWER_PITCH()
{
	return m_fCAMERA_LOWER_PITCH;
}

float cvInternalGlobals::getFIELD_OF_VIEW()
{
	return m_fFIELD_OF_VIEW;
}

float cvInternalGlobals::getSHADOW_SCALE()
{
	return m_fSHADOW_SCALE;
}

float cvInternalGlobals::getUNIT_MULTISELECT_DISTANCE()
{
	return m_fUNIT_MULTISELECT_DISTANCE;
}

int cvInternalGlobals::getUSE_CANNOT_FOUND_CITY_CALLBACK()
{
	return m_iUSE_CANNOT_FOUND_CITY_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK()
{
	return m_iUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK;
}

int cvInternalGlobals::getUSE_IS_PLAYER_RESEARCH_CALLBACK()
{
	return m_iUSE_IS_PLAYER_RESEARCH_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_RESEARCH_CALLBACK()
{
	return m_iUSE_CAN_RESEARCH_CALLBACK;
}

int cvInternalGlobals::getUSE_CANNOT_DO_CIVIC_CALLBACK()
{
	return m_iUSE_CANNOT_DO_CIVIC_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_DO_CIVIC_CALLBACK()
{
	return m_iUSE_CAN_DO_CIVIC_CALLBACK;
}

int cvInternalGlobals::getUSE_CANNOT_CONSTRUCT_CALLBACK()
{
	return m_iUSE_CANNOT_CONSTRUCT_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_CONSTRUCT_CALLBACK()
{
	return m_iUSE_CAN_CONSTRUCT_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_DECLARE_WAR_CALLBACK()
{
	return m_iUSE_CAN_DECLARE_WAR_CALLBACK;
}

int cvInternalGlobals::getUSE_CANNOT_RESEARCH_CALLBACK()
{
	return m_iUSE_CANNOT_RESEARCH_CALLBACK;
}

int cvInternalGlobals::getUSE_GET_UNIT_COST_MOD_CALLBACK()
{
	return m_iUSE_GET_UNIT_COST_MOD_CALLBACK;
}

int cvInternalGlobals::getUSE_GET_BUILDING_COST_MOD_CALLBACK()
{
	return m_iUSE_GET_BUILDING_COST_MOD_CALLBACK;
}

int cvInternalGlobals::getUSE_GET_CITY_FOUND_VALUE_CALLBACK()
{
	return m_iUSE_GET_CITY_FOUND_VALUE_CALLBACK;
}

int cvInternalGlobals::getUSE_CANNOT_HANDLE_ACTION_CALLBACK()
{
	return m_iUSE_CANNOT_HANDLE_ACTION_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_BUILD_CALLBACK()
{
	return m_iUSE_CAN_BUILD_CALLBACK;
}

int cvInternalGlobals::getUSE_CANNOT_TRAIN_CALLBACK()
{
	return m_iUSE_CANNOT_TRAIN_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_TRAIN_CALLBACK()
{
	return m_iUSE_CAN_TRAIN_CALLBACK;
}

#ifdef GRANULAR_CALLBACK_CONTROL
int cvInternalGlobals::getUSE_CAN_BUILD_CALLBACK(BuildTypes eBuild)
{
	return (m_iUSE_CAN_BUILD_CALLBACK || m_pythonCallbackController.IsBuildCallbackEnabled(CALLBACK_TYPE_CAN_BUILD, eBuild));
}

int cvInternalGlobals::getUSE_CANNOT_TRAIN_CALLBACK(UnitTypes eUnit)
{
	return (m_iUSE_CANNOT_TRAIN_CALLBACK || m_pythonCallbackController.IsUnitCallbackEnabled(CALLBACK_TYPE_CANNOT_TRAIN, eUnit));
}

int cvInternalGlobals::getUSE_CAN_TRAIN_CALLBACK(UnitTypes eUnit)
{
	return (m_iUSE_CAN_TRAIN_CALLBACK || m_pythonCallbackController.IsUnitCallbackEnabled(CALLBACK_TYPE_CAN_TRAIN, eUnit));
}
#else
int cvInternalGlobals::getUSE_CAN_BUILD_CALLBACK(BuildTypes eBuild)
{
	return m_iUSE_CAN_BUILD_CALLBACK;
}

int cvInternalGlobals::getUSE_CANNOT_TRAIN_CALLBACK(UnitTypes eUnit)
{
	return m_iUSE_CANNOT_TRAIN_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_TRAIN_CALLBACK(UnitTypes eUnit)
{
	return m_iUSE_CAN_TRAIN_CALLBACK;
}
#endif

int cvInternalGlobals::getUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK()
{
	return m_iUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK;
}

int cvInternalGlobals::getUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK()
{
	return m_iUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK;
}

int cvInternalGlobals::getUSE_FINISH_TEXT_CALLBACK()
{
	return m_iUSE_FINISH_TEXT_CALLBACK;
}

int cvInternalGlobals::getUSE_ON_UNIT_SET_XY_CALLBACK()
{
	return m_iUSE_ON_UNIT_SET_XY_CALLBACK;
}

int cvInternalGlobals::getUSE_ON_UNIT_SELECTED_CALLBACK()
{
	return m_iUSE_ON_UNIT_SELECTED_CALLBACK;
}

int cvInternalGlobals::getUSE_ON_UPDATE_CALLBACK()
{
	return m_iUSE_ON_UPDATE_CALLBACK;
}

int cvInternalGlobals::getUSE_ON_UNIT_CREATED_CALLBACK()
{
	return m_iUSE_ON_UNIT_CREATED_CALLBACK;
}

int cvInternalGlobals::getUSE_ON_UNIT_LOST_CALLBACK()
{
	return m_iUSE_ON_UNIT_LOST_CALLBACK;
}

/************************************************************************************************/
/* MODULES                                 11/13/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
int cvInternalGlobals::getTGA_RELIGIONS()								// GAMEFONT_TGA_RELIGIONS
{
	return m_iTGA_RELIGIONS;
}

int cvInternalGlobals::getTGA_CORPORATIONS()							// GAMEFONT_TGA_CORPORATIONS
{
	return m_iTGA_CORPORATIONS;
}

/************************************************************************************************/
/* MODULES                                 END                                                  */
/************************************************************************************************/
int cvInternalGlobals::getMAX_CIV_PLAYERS()
{
	return MAX_CIV_PLAYERS;
}

int cvInternalGlobals::getMAX_PLAYERS()
{
	return MAX_PLAYERS;
}

int cvInternalGlobals::getMAX_CIV_TEAMS()
{
	return MAX_CIV_TEAMS;
}

int cvInternalGlobals::getMAX_TEAMS()
{
	return MAX_TEAMS;
}

int cvInternalGlobals::getBARBARIAN_PLAYER()
{
	return BARBARIAN_PLAYER;
}

int cvInternalGlobals::getBARBARIAN_TEAM()
{
	return BARBARIAN_TEAM;
}

int cvInternalGlobals::getINVALID_PLOT_COORD()
{
	return INVALID_PLOT_COORD;
}

int cvInternalGlobals::getNUM_CITY_PLOTS()
{
	return NUM_CITY_PLOTS;
}

int cvInternalGlobals::getCITY_HOME_PLOT()
{
	return CITY_HOME_PLOT;
}

void cvInternalGlobals::setDLLProfiler(FProfiler* prof)
{
	m_Profiler=prof;
}

FProfiler* cvInternalGlobals::getDLLProfiler()
{
	return m_Profiler;
}

void cvInternalGlobals::enableDLLProfiler(bool bEnable)
{
	m_bDLLProfiler = bEnable;
}

bool cvInternalGlobals::isDLLProfilerEnabled() const
{
	return m_bDLLProfiler;
}

bool cvInternalGlobals::readBuildingInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paBuildingInfo, "CvBuildingInfo");
}

void cvInternalGlobals::writeBuildingInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paBuildingInfo);
}

bool cvInternalGlobals::readTechInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paTechInfo, "CvTechInfo");
}

void cvInternalGlobals::writeTechInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paTechInfo);
}

bool cvInternalGlobals::readUnitInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paUnitInfo, "CvUnitInfo");
}

void cvInternalGlobals::writeUnitInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paUnitInfo);
}

bool cvInternalGlobals::readLeaderHeadInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paLeaderHeadInfo, "CvLeaderHeadInfo");
}

void cvInternalGlobals::writeLeaderHeadInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paLeaderHeadInfo);
}

bool cvInternalGlobals::readCivilizationInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paCivilizationInfo, "CvCivilizationInfo");
}

void cvInternalGlobals::writeCivilizationInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paCivilizationInfo);
}

bool cvInternalGlobals::readPromotionInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paPromotionInfo, "CvPromotionInfo");
}

void cvInternalGlobals::writePromotionInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paPromotionInfo);
}

bool cvInternalGlobals::readDiplomacyInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paDiplomacyInfo, "CvDiplomacyInfo");
}

void cvInternalGlobals::writeDiplomacyInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paDiplomacyInfo);
}

bool cvInternalGlobals::readCivicInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paCivicInfo, "CvCivicInfo");
}

void cvInternalGlobals::writeCivicInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paCivicInfo);
}

bool cvInternalGlobals::readHandicapInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paHandicapInfo, "CvHandicapInfo");
}

void cvInternalGlobals::writeHandicapInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paHandicapInfo);
}

bool cvInternalGlobals::readBonusInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paBonusInfo, "CvBonusInfo");
}

void cvInternalGlobals::writeBonusInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paBonusInfo);
}

bool cvInternalGlobals::readImprovementInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paImprovementInfo, "CvImprovementInfo");
}

void cvInternalGlobals::writeImprovementInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paImprovementInfo);
}

bool cvInternalGlobals::readEventInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paEventInfo, "CvEventInfo");
}

void cvInternalGlobals::writeEventInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paEventInfo);
}

bool cvInternalGlobals::readEventTriggerInfoArray(FDataStreamBase* pStream)
{
	return readInfoArray(pStream, m_paEventTriggerInfo, "CvEventTriggerInfo");
}

void cvInternalGlobals::writeEventTriggerInfoArray(FDataStreamBase* pStream)
{
	writeInfoArray(pStream, m_paEventTriggerInfo);
}


//
// Global Types Hash Map
//

int cvInternalGlobals::getTypesEnum(const char* szType) const
{
	FAssertMsg(szType, "null type string");
	TypesMap::const_iterator it = m_typesMap.find(szType);
	if (it!=m_typesMap.end())
	{
		return it->second;
	}

	FAssertMsg(strcmp(szType, "NONE")==0 || strcmp(szType, "")==0, CvString::format("type %s not found", szType).c_str());
	return -1;
}

void cvInternalGlobals::setTypesEnum(const char* szType, int iEnum)
{
	FAssertMsg(szType, "null type string");
	FAssertMsg(m_typesMap.find(szType)==m_typesMap.end(), "types entry already exists");
	m_typesMap[szType] = iEnum;
}


int cvInternalGlobals::getNUM_ENGINE_DIRTY_BITS() const
{
	return NUM_ENGINE_DIRTY_BITS;
}

int cvInternalGlobals::getNUM_INTERFACE_DIRTY_BITS() const
{
	return NUM_INTERFACE_DIRTY_BITS;
}

int cvInternalGlobals::getNUM_YIELD_TYPES() const
{
	return NUM_YIELD_TYPES;
}

int cvInternalGlobals::getNUM_COMMERCE_TYPES() const
{
	return NUM_COMMERCE_TYPES;
}

int cvInternalGlobals::getNUM_FORCECONTROL_TYPES() const
{
	return NUM_FORCECONTROL_TYPES;
}

int cvInternalGlobals::getNUM_INFOBAR_TYPES() const
{
	return NUM_INFOBAR_TYPES;
}

int cvInternalGlobals::getNUM_HEALTHBAR_TYPES() const
{
	return NUM_HEALTHBAR_TYPES;
}

int cvInternalGlobals::getNUM_CONTROL_TYPES() const
{
	return NUM_CONTROL_TYPES;
}

int cvInternalGlobals::getNUM_LEADERANIM_TYPES() const
{
	return NUM_LEADERANIM_TYPES;
}


void cvInternalGlobals::deleteInfoArrays()
{
	deleteInfoArray(m_paBuildingClassInfo);
	deleteInfoArray(m_paBuildingInfo);
	deleteInfoArray(m_paSpecialBuildingInfo);

	deleteInfoArray(m_paLeaderHeadInfo);
	deleteInfoArray(m_paTraitInfo);
	deleteInfoArray(m_paCivilizationInfo);
	deleteInfoArray(m_paUnitArtStyleTypeInfo);

	deleteInfoArray(m_paVoteSourceInfo);
	deleteInfoArray(m_paHints);
	deleteInfoArray(m_paMainMenus);
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 11/01/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	// Python Modular Loading
	deleteInfoArray(m_paPythonModulesInfo);
	// MLF loading
	m_paModLoadControlVector.clear();
	deleteInfoArray(m_paModLoadControls);
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
	deleteInfoArray(m_paGoodyInfo);
	deleteInfoArray(m_paHandicapInfo);
	deleteInfoArray(m_paGameSpeedInfo);
	deleteInfoArray(m_paTurnTimerInfo);
	deleteInfoArray(m_paVictoryInfo);
	deleteInfoArray(m_paHurryInfo);
	deleteInfoArray(m_paWorldInfo);
	deleteInfoArray(m_paSeaLevelInfo);
	deleteInfoArray(m_paClimateInfo);
	deleteInfoArray(m_paProcessInfo);
	deleteInfoArray(m_paVoteInfo);
	deleteInfoArray(m_paProjectInfo);
	deleteInfoArray(m_paReligionInfo);
	deleteInfoArray(m_paCorporationInfo);
	deleteInfoArray(m_paCommerceInfo);
	deleteInfoArray(m_paEmphasizeInfo);
	deleteInfoArray(m_paUpkeepInfo);
	deleteInfoArray(m_paCultureLevelInfo);

	deleteInfoArray(m_paColorInfo);
	deleteInfoArray(m_paPlayerColorInfo);
	deleteInfoArray(m_paInterfaceModeInfo);
	deleteInfoArray(m_paCameraInfo);
	deleteInfoArray(m_paAdvisorInfo);
	deleteInfoArray(m_paThroneRoomCamera);
	deleteInfoArray(m_paThroneRoomInfo);
	deleteInfoArray(m_paThroneRoomStyleInfo);
	deleteInfoArray(m_paSlideShowInfo);
	deleteInfoArray(m_paSlideShowRandomInfo);
	deleteInfoArray(m_paWorldPickerInfo);
	deleteInfoArray(m_paSpaceShipInfo);

	deleteInfoArray(m_paCivicInfo);
	deleteInfoArray(m_paImprovementInfo);

	deleteInfoArray(m_paRouteInfo);
	deleteInfoArray(m_paRouteModelInfo);
	deleteInfoArray(m_paRiverInfo);
	deleteInfoArray(m_paRiverModelInfo);

	deleteInfoArray(m_paWaterPlaneInfo);
	deleteInfoArray(m_paTerrainPlaneInfo);
	deleteInfoArray(m_paCameraOverlayInfo);

	deleteInfoArray(m_aEraInfo);
	deleteInfoArray(m_paEffectInfo);
	deleteInfoArray(m_paAttachableInfo);

	deleteInfoArray(m_paTechInfo);
	deleteInfoArray(m_paDiplomacyInfo);

	deleteInfoArray(m_paBuildInfo);
	deleteInfoArray(m_paUnitClassInfo);
	deleteInfoArray(m_paUnitInfo);
	deleteInfoArray(m_paSpawnInfo);
	deleteInfoArray(m_paSpecialUnitInfo);
	deleteInfoArray(m_paSpecialistInfo);
	deleteInfoArray(m_paActionInfo);
	deleteInfoArray(m_paMissionInfo);
	deleteInfoArray(m_paControlInfo);
	deleteInfoArray(m_paCommandInfo);
	deleteInfoArray(m_paAutomateInfo);
	deleteInfoArray(m_paPromotionInfo);

	deleteInfoArray(m_paConceptInfo);
	deleteInfoArray(m_paNewConceptInfo);
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - DCM: Pedia Concepts START
	deleteInfoArray(m_paDCMConceptInfo);
	// Dale - DCM: Pedia Concepts END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/*Afforess                                     11/13/09                                         */
/************************************************************************************************/
	deleteInfoArray(m_paANDConceptInfo);
/************************************************************************************************/
/* Afforess                                END                                                  */
/************************************************************************************************/
	deleteInfoArray(m_paCityTabInfo);
	deleteInfoArray(m_paCalendarInfo);
	deleteInfoArray(m_paSeasonInfo);
	deleteInfoArray(m_paMonthInfo);
	deleteInfoArray(m_paDenialInfo);
	deleteInfoArray(m_paInvisibleInfo);
	deleteInfoArray(m_paUnitCombatInfo);
	deleteInfoArray(m_paDomainInfo);
	deleteInfoArray(m_paUnitAIInfos);
	deleteInfoArray(m_paAttitudeInfos);
	deleteInfoArray(m_paMemoryInfos);
	deleteInfoArray(m_paGameOptionInfos);
	deleteInfoArray(m_paMPOptionInfos);
	deleteInfoArray(m_paForceControlInfos);
	deleteInfoArray(m_paPlayerOptionInfos);
	deleteInfoArray(m_paGraphicOptionInfos);

	deleteInfoArray(m_paYieldInfo);
	deleteInfoArray(m_paTerrainInfo);
	deleteInfoArray(m_paFeatureInfo);
	deleteInfoArray(m_paBonusClassInfo);
	deleteInfoArray(m_paBonusInfo);
	deleteInfoArray(m_paLandscapeInfo);

	deleteInfoArray(m_paUnitFormationInfo);
	deleteInfoArray(m_paCivicOptionInfo);
	deleteInfoArray(m_paCursorInfo);

	SAFE_DELETE_ARRAY(GC.getEntityEventTypes());
	SAFE_DELETE_ARRAY(GC.getAnimationOperatorTypes());
	SAFE_DELETE_ARRAY(GC.getFunctionTypes());
	SAFE_DELETE_ARRAY(GC.getFlavorTypes());
	SAFE_DELETE_ARRAY(GC.getArtStyleTypes());
	SAFE_DELETE_ARRAY(GC.getCitySizeTypes());
	SAFE_DELETE_ARRAY(GC.getContactTypes());
	SAFE_DELETE_ARRAY(GC.getDiplomacyPowerTypes());
	SAFE_DELETE_ARRAY(GC.getAutomateTypes());
	SAFE_DELETE_ARRAY(GC.getDirectionTypes());
	SAFE_DELETE_ARRAY(GC.getFootstepAudioTypes());
	SAFE_DELETE_ARRAY(GC.getFootstepAudioTags());
	deleteInfoArray(m_paQuestInfo);
	deleteInfoArray(m_paTutorialInfo);

	deleteInfoArray(m_paEventInfo);
	deleteInfoArray(m_paEventTriggerInfo);
	deleteInfoArray(m_paEspionageMissionInfo);

	deleteInfoArray(m_paEntityEventInfo);
	deleteInfoArray(m_paAnimationCategoryInfo);
	deleteInfoArray(m_paAnimationPathInfo);
	deleteInfoArray(m_paCulturalAgeInfo);
	deleteInfoArray(m_paPropertyInfo);

	clearTypesMap();
	m_aInfoVectors.clear();
}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 11/30/07                                MRGENIE      */
/*                                                                                              */
/* Savegame compatibility                                                                       */
/************************************************************************************************/
void cvInternalGlobals::doResetInfoClasses(int iNumSaveGameVector, std::vector<CvString> m_aszSaveGameVector)
{
	// Store stuff being set by the exe in temp arrays
	StoreExeSettings();

	//delete obsolete InfoClasses
	deleteInfoArrays();

	// reset the ModLoadControlVector
	m_paModLoadControlVector.erase(m_paModLoadControlVector.begin(), m_paModLoadControlVector.end());

	//Load the Savegame vector to the ModLoadControlVector(being used by the enum)
	for ( int i = 0; i < iNumSaveGameVector; i++ )
	{
		m_paModLoadControlVector.push_back(m_aszSaveGameVector[i]);
	}

	//Delete InfoMap Keys
	infoTypeFromStringReset();

	//reload the new infoclasses
	CvXMLLoadUtility XMLLoadUtility;
	XMLLoadUtility.doResetGlobalInfoClasses();

	//Reload Arts with the Current MLF
	CvArtFileMgr ArtFileMgr = ArtFileMgr.GetInstance();
	ArtFileMgr.Reset();

	XMLLoadUtility.doResetInfoClasses();		// Reloads/allocs Art Defines

	// Load stuff being set by the exe from temp arrays
	LoadExeSettings();
}
void cvInternalGlobals::StoreExeSettings()
{
	// Chars from TGA files, CommerceInfo
	m_iStoreExeSettingsCommerceInfo =  new int[NUM_COMMERCE_TYPES];
	for ( int i = 0; i < NUM_COMMERCE_TYPES; i++ )
	{
		m_iStoreExeSettingsCommerceInfo[i] = getCommerceInfo((CommerceTypes)i).getChar();
	}
	// Chars from TGA files, YieldInfo
	m_iStoreExeSettingsYieldInfo =  new int[NUM_YIELD_TYPES];
	for ( int i = 0; i < NUM_YIELD_TYPES; i++ )
	{
		m_iStoreExeSettingsYieldInfo[i] = getYieldInfo((YieldTypes)i).getChar();
	}	
	// Chars from TGA files, ReligionInfo
	m_iStoreExeSettingsReligionInfo =  new int[getNumReligionInfos()];
	for ( int i = 0; i < getNumReligionInfos(); i++ )
	{
		m_iStoreExeSettingsReligionInfo[i] = getReligionInfo((ReligionTypes)i).getChar();
	}
	// Chars from TGA files, CorporationInfo
	m_iStoreExeSettingsCorporationInfo =  new int[getNumCorporationInfos()];
	for ( int i = 0; i < getNumCorporationInfos(); i++ )
	{
		m_iStoreExeSettingsCorporationInfo[i] = getCorporationInfo((CorporationTypes)i).getChar();
	}
	// Chars from TGA files, BonusInfo
	m_iStoreExeSettingsBonusInfo =  new int[getNumBonusInfos()];
	for ( int i = 0; i < getNumBonusInfos(); i++ )
	{
		m_iStoreExeSettingsBonusInfo[i] = getBonusInfo((BonusTypes)i).getChar();
	}
}
void cvInternalGlobals::LoadExeSettings()
{
	// Chars from TGA files, CommerceInfo
	for ( int i = 0; i < NUM_COMMERCE_TYPES; i++ )
	{
		getCommerceInfo((CommerceTypes)i).setChar(m_iStoreExeSettingsCommerceInfo[i]);
	}
	SAFE_DELETE_ARRAY(m_iStoreExeSettingsCommerceInfo);
	// Chars from TGA files, YieldInfo
	for ( int i = 0; i < NUM_YIELD_TYPES; i++ )
	{
		getYieldInfo((YieldTypes)i).setChar(m_iStoreExeSettingsYieldInfo[i]);
	}
	SAFE_DELETE_ARRAY(m_iStoreExeSettingsYieldInfo);
	// Chars from TGA files, ReligionInfo
	for ( int i = 0; i < getNumReligionInfos(); i++ )
	{
		getReligionInfo((ReligionTypes)i).setChar(m_iStoreExeSettingsReligionInfo[i]);
	}
	SAFE_DELETE_ARRAY(m_iStoreExeSettingsReligionInfo);
	// Chars from TGA files, CorporationInfo
	for ( int i = 0; i < getNumCorporationInfos(); i++ )
	{
		getCorporationInfo((CorporationTypes)i).setChar(m_iStoreExeSettingsCorporationInfo[i]);
	}
	SAFE_DELETE_ARRAY(m_iStoreExeSettingsCorporationInfo);
	// Chars from TGA files, BonusInfo
	for ( int i = 0; i < getNumBonusInfos(); i++ )
	{
		getBonusInfo((BonusTypes)i).setChar(m_iStoreExeSettingsBonusInfo[i]);
	}
	SAFE_DELETE_ARRAY(m_iStoreExeSettingsBonusInfo);
}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/


//
// Global Infos Hash Map
//

int cvInternalGlobals::getInfoTypeForString(const char* szType, bool hideAssert) const
	{
	FAssertMsg(szType, "null info type string");
	InfosMap::const_iterator it = m_infosMap.find(szType);
	if (it!=m_infosMap.end())
	{
		return it->second;
	}

	if(!hideAssert && !getDefineINT(szType))
	{
		
		CvString szError;
		szError.Format("info type %s not found, Current XML file is: %s", szType, GC.getCurrentXMLFile().GetCString());
		FAssertMsg(strcmp(szType, "NONE")==0 || strcmp(szType, "")==0, szError.c_str());
		if (GC.isXMLLogging())
		{
			gDLL->logMsg("xml.log", szError);
		}
	}

	return -1;
}

/************************************************************************************************/
/* SORT_ALPHABET                           11/19/07                                MRGENIE      */
/*                                                                                              */
/* Rearranging the infos map                                                                    */
/************************************************************************************************/
/*
void cvInternalGlobals::setInfoTypeFromString(const char* szType, int idx)
{
	FAssertMsg(szType, "null info type string");
#ifdef _DEBUG
	InfosMap::const_iterator it = m_infosMap.find(szType);
	int iExisting = (it!=m_infosMap.end()) ? it->second : -1;
	FAssertMsg(iExisting==-1 || iExisting==idx || strcmp(szType, "ERROR")==0, CvString::format("xml info type entry %s already exists", szType).c_str());
#endif
	m_infosMap[szType] = idx;
}
*/
void cvInternalGlobals::setInfoTypeFromString(const char* szType, int idx)
{
	FAssertMsg(szType, "null info type string");
#ifdef _DEBUG
	//InfosMap::const_iterator it = m_infosMap.find(szType);
	//int iExisting = (it!=m_infosMap.end()) ? it->second : -1;
	//CvString szError;
	//szError.Format("info type %s already exists, Current XML file is: %s", szType, GC.getCurrentXMLFile().GetCString());
	//FAssertMsg(iExisting==-1 || iExisting==idx || strcmp(szType, "ERROR")==0, szError.c_str());

	if ( stricmp("LEADER_MENZIES",szType)==0)
	{
		OutputDebugString("!\n");
	}
	OutputDebugString(CvString::format("%s -> %d\n", szType, idx).c_str());
#endif
	m_infosMap[szType] = idx;
}

void cvInternalGlobals::logInfoTypeMap(const char* tagMsg)
{
	if (GC.isXMLLogging())
	{
		CvString szDebugBuffer;
		szDebugBuffer.Format(" === Info Type Map Dump BEGIN: %s ===", tagMsg);
		gDLL->logMsg("cvInternalGlobals_logInfoTypeMap.log", szDebugBuffer.c_str());

		int iCnt = 0;
		std::vector<std::string> vInfoMapKeys;
		for (InfosMap::const_iterator it = m_infosMap.begin(); it != m_infosMap.end(); it++)
		{
			std::string sKey = it->first;
			vInfoMapKeys.push_back(sKey);
		}

		std::sort(vInfoMapKeys.begin(), vInfoMapKeys.end());

		for (std::vector<std::string>::const_iterator it = vInfoMapKeys.begin(); it != vInfoMapKeys.end(); it++)
		{
			std::string sKey = *it;
			int iVal = m_infosMap[sKey];
			szDebugBuffer.Format(" * %i --  %s: %i", iCnt, sKey.c_str(), iVal);
			gDLL->logMsg("cvInternalGlobals_logInfoTypeMap.log", szDebugBuffer.c_str());
			iCnt++;
		}

		szDebugBuffer.Format("Entries in total: %i", iCnt);
		gDLL->logMsg("cvInternalGlobals_logInfoTypeMap.log", szDebugBuffer.c_str());
		szDebugBuffer.Format(" === Info Type Map Dump END: %s ===", tagMsg);
		gDLL->logMsg("cvInternalGlobals_logInfoTypeMap.log", szDebugBuffer.c_str());
	}
}
/************************************************************************************************/
/* SORT_ALPHABET                           END                                                  */
/************************************************************************************************/

void cvInternalGlobals::infoTypeFromStringReset()
{
	m_infosMap.clear();
}

void cvInternalGlobals::addToInfosVectors(void *infoVector)
{
	std::vector<CvInfoBase *> *infoBaseVector = (std::vector<CvInfoBase *> *) infoVector;
	m_aInfoVectors.push_back(infoBaseVector);
}

void cvInternalGlobals::infosReset()
{
	for(int i=0;i<(int)m_aInfoVectors.size();i++)
	{
		std::vector<CvInfoBase *> *infoBaseVector = m_aInfoVectors[i];
		for(int j=0;j<(int)infoBaseVector->size();j++)
			infoBaseVector->at(j)->reset();
	}
}

int cvInternalGlobals::getNumDirections() const { return NUM_DIRECTION_TYPES; }
int cvInternalGlobals::getNumGameOptions() const { return NUM_GAMEOPTION_TYPES; }
int cvInternalGlobals::getNumMPOptions() const { return NUM_MPOPTION_TYPES; }
int cvInternalGlobals::getNumSpecialOptions() const { return NUM_SPECIALOPTION_TYPES; }
int cvInternalGlobals::getNumGraphicOptions() const { return NUM_GRAPHICOPTION_TYPES; }
int cvInternalGlobals::getNumTradeableItems() const { return NUM_TRADEABLE_ITEMS; }
int cvInternalGlobals::getNumBasicItems() const { return NUM_BASIC_ITEMS; }
int cvInternalGlobals::getNumTradeableHeadings() const { return NUM_TRADEABLE_HEADINGS; }
int cvInternalGlobals::getNumCommandInfos() const { return NUM_COMMAND_TYPES; }
int cvInternalGlobals::getNumControlInfos() const { return NUM_CONTROL_TYPES; }
int cvInternalGlobals::getNumPlayerOptionInfos() const { return NUM_PLAYEROPTION_TYPES; }
int cvInternalGlobals::getMaxNumSymbols() const { return MAX_NUM_SYMBOLS; }
int cvInternalGlobals::getNumGraphicLevels() const { return NUM_GRAPHICLEVELS; }
int cvInternalGlobals::getNumGlobeLayers() const { return NUM_GLOBE_LAYER_TYPES; }

int cvInternalGlobals::getNumMissionInfos() const
{ 
#ifdef FIXED_MISSION_NUMBER
	return NUM_MISSION_TYPES;
#else
	return (int) m_paMissionInfo.size();
#endif
}


//
// non-inline versions
//
CvMap& cvInternalGlobals::getMap() { return *m_map; }
CvGameAI& cvInternalGlobals::getGame() { return *m_game; }
CvGameAI *cvInternalGlobals::getGamePointer(){ return m_game; }

int cvInternalGlobals::getMaxCivPlayers() const
{
	return MAX_CIV_PLAYERS;
}

bool cvInternalGlobals::IsGraphicsInitialized() const { return m_bGraphicsInitialized;}
void cvInternalGlobals::SetGraphicsInitialized(bool bVal) { m_bGraphicsInitialized = bVal;}
void cvInternalGlobals::setInterface(CvInterface* pVal) { m_interface = pVal; }
void cvInternalGlobals::setDiplomacyScreen(CvDiplomacyScreen* pVal) { m_diplomacyScreen = pVal; }
void cvInternalGlobals::setMPDiplomacyScreen(CMPDiplomacyScreen* pVal) { m_mpDiplomacyScreen = pVal; }
void cvInternalGlobals::setMessageQueue(CMessageQueue* pVal) { m_messageQueue = pVal; }
void cvInternalGlobals::setHotJoinMessageQueue(CMessageQueue* pVal) { m_hotJoinMsgQueue = pVal; }
void cvInternalGlobals::setMessageControl(CMessageControl* pVal) { m_messageControl = pVal; }
void cvInternalGlobals::setSetupData(CvSetupData* pVal) { m_setupData = pVal; }
void cvInternalGlobals::setMessageCodeTranslator(CvMessageCodeTranslator* pVal) { m_messageCodes = pVal; }
void cvInternalGlobals::setDropMgr(CvDropMgr* pVal) { m_dropMgr = pVal; }
void cvInternalGlobals::setPortal(CvPortal* pVal) { m_portal = pVal; }
void cvInternalGlobals::setStatsReport(CvStatsReporter* pVal) { m_statsReporter = pVal; }
void cvInternalGlobals::setPathFinder(FAStar* pVal) { m_pathFinder = pVal; }
void cvInternalGlobals::setInterfacePathFinder(FAStar* pVal) { m_interfacePathFinder = pVal; }
void cvInternalGlobals::setStepFinder(FAStar* pVal) { m_stepFinder = pVal; }
void cvInternalGlobals::setRouteFinder(FAStar* pVal) { m_routeFinder = pVal; }
void cvInternalGlobals::setBorderFinder(FAStar* pVal) { m_borderFinder = pVal; }
void cvInternalGlobals::setAreaFinder(FAStar* pVal) { m_areaFinder = pVal; }
void cvInternalGlobals::setPlotGroupFinder(FAStar* pVal) { m_plotGroupFinder = pVal; }
CvDLLUtilityIFaceBase* cvInternalGlobals::getDLLIFaceNonInl() { return g_DLL; }
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
// Dale - DCM: Pedia Concepts START
int cvInternalGlobals::getNumDCMConceptInfos()
{
	return (int)m_paDCMConceptInfo.size();
}

// BUG - DLL Info - start
bool cvInternalGlobals::isBull() const { return true; }
int cvInternalGlobals::getBullApiVersion() const { return BUG_DLL_API_VERSION; }
const wchar* cvInternalGlobals::getBullName() const { return BUG_DLL_NAME; }
const wchar* cvInternalGlobals::getBullVersion() const { return BUG_DLL_VERSION; }
// BUG - DLL Info - end

// BUG - BUG Info - start
void cvInternalGlobals::setIsBug(bool bIsBug) { ::setIsBug(bIsBug); }
// BUG - BUG Info - end

// BUFFY - DLL Info - start
#ifdef _BUFFY
bool cvInternalGlobals::isBuffy() const { return true; }
int cvInternalGlobals::getBuffyApiVersion() const { return BUFFY_DLL_API_VERSION; }
const wchar* cvInternalGlobals::getBuffyName() const { return BUFFY_DLL_NAME; }
const wchar* cvInternalGlobals::getBuffyVersion() const { return BUFFY_DLL_VERSION; }
#endif
// BUFFY - DLL Info - end

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Efficiency, Options                                                                          */
/************************************************************************************************/
// BBAI Options
bool cvInternalGlobals::getBBAI_AIR_COMBAT()
{
	return m_bBBAI_AIR_COMBAT;
}

bool cvInternalGlobals::getBBAI_HUMAN_VASSAL_WAR_BUILD()
{
	return m_bBBAI_HUMAN_VASSAL_WAR_BUILD;
}

int cvInternalGlobals::getBBAI_DEFENSIVE_PACT_BEHAVIOR()
{
	return m_iBBAI_DEFENSIVE_PACT_BEHAVIOR;
}

bool cvInternalGlobals::getBBAI_HUMAN_AS_VASSAL_OPTION()
{
	return m_bBBAI_HUMAN_AS_VASSAL_OPTION;
}

	
// BBAI AI Variables
int cvInternalGlobals::getWAR_SUCCESS_CITY_CAPTURING()
{
	return m_iWAR_SUCCESS_CITY_CAPTURING;
}

int cvInternalGlobals::getBBAI_ATTACK_CITY_STACK_RATIO()
{
	return m_iBBAI_ATTACK_CITY_STACK_RATIO;
}

int cvInternalGlobals::getBBAI_SKIP_BOMBARD_BEST_ATTACK_ODDS()
{
	return m_iBBAI_SKIP_BOMBARD_BEST_ATTACK_ODDS;
}

int cvInternalGlobals::getBBAI_SKIP_BOMBARD_BASE_STACK_RATIO()
{
	return m_iBBAI_SKIP_BOMBARD_BASE_STACK_RATIO;
}

int cvInternalGlobals::getBBAI_SKIP_BOMBARD_MIN_STACK_RATIO()
{
	return m_iBBAI_SKIP_BOMBARD_MIN_STACK_RATIO;
}

// Tech Diffusion
bool cvInternalGlobals::getTECH_DIFFUSION_ENABLE()
{
	return m_bTECH_DIFFUSION_ENABLE;
}

int cvInternalGlobals::getTECH_DIFFUSION_KNOWN_TEAM_MODIFIER()
{
	return m_iTECH_DIFFUSION_KNOWN_TEAM_MODIFIER;
}

int cvInternalGlobals::getTECH_DIFFUSION_WELFARE_THRESHOLD()
{
	return m_iTECH_DIFFUSION_WELFARE_THRESHOLD;
}

int cvInternalGlobals::getTECH_DIFFUSION_WELFARE_MODIFIER()
{
	return m_iTECH_DIFFUSION_WELFARE_MODIFIER;
}

int cvInternalGlobals::getTECH_COST_FIRST_KNOWN_PREREQ_MODIFIER()
{
	return m_iTECH_COST_FIRST_KNOWN_PREREQ_MODIFIER;
}

int cvInternalGlobals::getTECH_COST_KNOWN_PREREQ_MODIFIER()
{
	return m_iTECH_COST_KNOWN_PREREQ_MODIFIER;
}

int cvInternalGlobals::getTECH_COST_MODIFIER()
{
	return m_iTECH_COST_MODIFIER;
}


// From Lead From Behind by UncutDragon
// Lead from Behind flags
bool cvInternalGlobals::getLFBEnable()
{
	return m_bLFBEnable;
}

int cvInternalGlobals::getLFBBasedOnGeneral()
{
	return m_iLFBBasedOnGeneral;
}

int cvInternalGlobals::getLFBBasedOnExperience()
{
	return m_iLFBBasedOnExperience;
}

int cvInternalGlobals::getLFBBasedOnLimited()
{
	return m_iLFBBasedOnLimited;
}

int cvInternalGlobals::getLFBBasedOnHealer()
{
	return m_iLFBBasedOnHealer;
}

int cvInternalGlobals::getLFBBasedOnAverage()
{
	return m_iLFBBasedOnAverage;
}

bool cvInternalGlobals::getLFBUseSlidingScale()
{
	return m_bLFBUseSlidingScale;
}

int cvInternalGlobals::getLFBAdjustNumerator()
{
	return m_iLFBAdjustNumerator;
}

int cvInternalGlobals::getLFBAdjustDenominator()
{
	return m_iLFBAdjustDenominator;
}

bool cvInternalGlobals::getLFBUseCombatOdds()
{
	return m_bLFBUseCombatOdds;
}

int cvInternalGlobals::getCOMBAT_DIE_SIDES()
{
	return m_iCOMBAT_DIE_SIDES;
}

int cvInternalGlobals::getCOMBAT_DAMAGE()
{
	return m_iCOMBAT_DAMAGE;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* Afforess	                  Start		 12/8/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
int cvInternalGlobals::getNumANDConceptInfos()
{
	return (int)m_paANDConceptInfo.size();
}
std::vector<CvInfoBase*>& cvInternalGlobals::getANDConceptInfo()
{
	return m_paANDConceptInfo;
}

CvInfoBase& cvInternalGlobals::getANDConceptInfo(ANDConceptTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumANDConceptInfos());
	return *(m_paANDConceptInfo[e]);
}

int cvInternalGlobals::getPEAK_EXTRA_MOVEMENT()
{
	return m_iPEAK_EXTRA_MOVEMENT;
}

int cvInternalGlobals::getPEAK_EXTRA_DEFENSE()
{
	return m_iPEAK_EXTRA_DEFENSE;
}

bool cvInternalGlobals::isFormationsMod() const
{
	return m_bFormationsMod;
}

bool cvInternalGlobals::isLoadedPlayerOptions() const
{
	return m_bLoadedPlayerOptions;
}

void cvInternalGlobals::setLoadedPlayerOptions(bool bNewVal)
{
	m_bLoadedPlayerOptions = bNewVal;
}

void cvInternalGlobals::setXMLLogging(bool bNewVal)
{
	m_bXMLLogging = bNewVal;
}

bool cvInternalGlobals::isXMLLogging()
{
	return m_bXMLLogging;
}

int cvInternalGlobals::getSCORE_FREE_PERCENT()
{
	return m_iSCORE_FREE_PERCENT;
}

int cvInternalGlobals::getSCORE_POPULATION_FACTOR()
{
	return m_iSCORE_POPULATION_FACTOR;
}

int cvInternalGlobals::getSCORE_LAND_FACTOR()
{
	return m_iSCORE_LAND_FACTOR;
}

int cvInternalGlobals::getSCORE_TECH_FACTOR()
{
	return m_iSCORE_TECH_FACTOR;
}

int cvInternalGlobals::getSCORE_WONDER_FACTOR()
{
	return m_iSCORE_WONDER_FACTOR;
}

//New Python Callbacks
int cvInternalGlobals::getUSE_CAN_CREATE_PROJECT_CALLBACK()
{
	return m_iUSE_CAN_CREATE_PROJECT_CALLBACK;
}

int cvInternalGlobals::getUSE_CANNOT_CREATE_PROJECT_CALLBACK()
{
	return m_iUSE_CANNOT_CREATE_PROJECT_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_DO_MELTDOWN_CALLBACK()
{
	return m_iUSE_CAN_DO_MELTDOWN_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_MAINTAIN_PROCESS_CALLBACK()
{
	return m_iUSE_CAN_MAINTAIN_PROCESS_CALLBACK;
}

int cvInternalGlobals::getUSE_CANNOT_MAINTAIN_PROCESS_CALLBACK()
{
	return m_iUSE_CANNOT_MAINTAIN_PROCESS_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_DO_GROWTH_CALLBACK()
{
	return m_iUSE_CAN_DO_GROWTH_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_DO_CULTURE_CALLBACK()
{
	return m_iUSE_CAN_DO_CULTURE_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_DO_PLOT_CULTURE_CALLBACK()
{
	return m_iUSE_CAN_DO_PLOT_CULTURE_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_DO_PRODUCTION_CALLBACK()
{
	return m_iUSE_CAN_DO_PRODUCTION_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_DO_RELIGION_CALLBACK()
{
	return m_iUSE_CAN_DO_RELIGION_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_DO_GREATPEOPLE_CALLBACK()
{
	return m_iUSE_CAN_DO_GREATPEOPLE_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_RAZE_CITY_CALLBACK()
{
	return m_iUSE_CAN_RAZE_CITY_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_DO_GOLD_CALLBACK()
{
	return m_iUSE_CAN_DO_GOLD_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_DO_RESEARCH_CALLBACK()
{
	return m_iUSE_CAN_DO_RESEARCH_CALLBACK;
}

int cvInternalGlobals::getUSE_UPGRADE_UNIT_PRICE_CALLBACK()
{
	return m_iUSE_UPGRADE_UNIT_PRICE_CALLBACK;
}

int cvInternalGlobals::getUSE_IS_VICTORY_CALLBACK()
{
	return m_iUSE_IS_VICTORY_CALLBACK;
}

int cvInternalGlobals::getUSE_AI_UPDATE_UNIT_CALLBACK()
{
	return m_iUSE_AI_UPDATE_UNIT_CALLBACK;
}

int cvInternalGlobals::getUSE_AI_CHOOSE_PRODUCTION_CALLBACK()
{
	return m_iUSE_AI_CHOOSE_PRODUCTION_CALLBACK;
}

int cvInternalGlobals::getUSE_EXTRA_PLAYER_COSTS_CALLBACK()
{
	return m_iUSE_EXTRA_PLAYER_COSTS_CALLBACK;
}

int cvInternalGlobals::getUSE_AI_DO_DIPLO_CALLBACK()
{
	return m_iUSE_AI_DO_DIPLO_CALLBACK;
}

int cvInternalGlobals::getUSE_AI_BESTTECH_CALLBACK()
{
	return m_iUSE_AI_BESTTECH_CALLBACK;
}

int cvInternalGlobals::getUSE_CAN_DO_COMBAT_CALLBACK()
{
	return m_iUSE_CAN_DO_COMBAT_CALLBACK;
}

int cvInternalGlobals::getUSE_AI_CAN_DO_WARPLANS_CALLBACK()
{
	return m_iUSE_AI_CAN_DO_WARPLANS_CALLBACK;
}

/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
std::vector<CvInfoBase*>& cvInternalGlobals::getDCMConceptInfo()	// For Moose - XML Load Util, CvInfos
{
	return m_paDCMConceptInfo;
}

CvInfoBase& cvInternalGlobals::getDCMConceptInfo(DCMConceptTypes e)
{
	FAssert(e > -1);
	FAssert(e < GC.getNumDCMConceptInfos());
	return *(m_paDCMConceptInfo[e]);
}
// Dale - DCM: Pedia Concepts END

/************************************************************************************************/
/* Mod Globals    Start                          09/13/10                           phungus420  */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
bool cvInternalGlobals::isDCM_BATTLE_EFFECTS()
{
	return m_bDCM_BATTLE_EFFECTS;
}

int cvInternalGlobals::getBATTLE_EFFECT_LESS_FOOD()
{
	return m_iBATTLE_EFFECT_LESS_FOOD;
}

int cvInternalGlobals::getBATTLE_EFFECT_LESS_PRODUCTION()
{
	return m_iBATTLE_EFFECT_LESS_PRODUCTION;
}

int cvInternalGlobals::getBATTLE_EFFECT_LESS_COMMERCE()
{
	return m_iBATTLE_EFFECT_LESS_COMMERCE;
}

int cvInternalGlobals::getBATTLE_EFFECTS_MINIMUM_TURN_INCREMENTS()
{
	return m_iBATTLE_EFFECTS_MINIMUM_TURN_INCREMENTS;
}

int cvInternalGlobals::getMAX_BATTLE_TURNS()
{
	return m_iMAX_BATTLE_TURNS;
}

bool cvInternalGlobals::isDCM_AIR_BOMBING()
{
	return m_bDCM_AIR_BOMBING;
}

bool cvInternalGlobals::isDCM_RANGE_BOMBARD()
{
	return m_bDCM_RANGE_BOMBARD;
}

int cvInternalGlobals::getDCM_RB_CITY_INACCURACY()
{
	return m_iDCM_RB_CITY_INACCURACY;
}

int cvInternalGlobals::getDCM_RB_CITYBOMBARD_CHANCE()
{
	return m_iDCM_RB_CITYBOMBARD_CHANCE;
}

bool cvInternalGlobals::isDCM_ATTACK_SUPPORT()
{
	return m_bDCM_ATTACK_SUPPORT;
}

bool cvInternalGlobals::isDCM_STACK_ATTACK()
{
	return m_bDCM_STACK_ATTACK;
}

bool cvInternalGlobals::isDCM_OPP_FIRE()
{
	return m_bDCM_OPP_FIRE;
}

bool cvInternalGlobals::isDCM_ACTIVE_DEFENSE()
{
	return m_bDCM_ACTIVE_DEFENSE;
}

bool cvInternalGlobals::isDCM_ARCHER_BOMBARD()
{
	return m_bDCM_ARCHER_BOMBARD;
}

bool cvInternalGlobals::isDCM_FIGHTER_ENGAGE()
{
	return m_bDCM_FIGHTER_ENGAGE;
}

bool cvInternalGlobals::isDYNAMIC_CIV_NAMES()
{
	return m_bDYNAMIC_CIV_NAMES;
}

bool cvInternalGlobals::isLIMITED_RELIGIONS_EXCEPTIONS()
{
	return m_bLIMITED_RELIGIONS_EXCEPTIONS;
}

bool cvInternalGlobals::isOC_RESPAWN_HOLY_CITIES()
{
	return m_bOC_RESPAWN_HOLY_CITIES;
}

bool cvInternalGlobals::isIDW_ENABLED()
{
	return m_bIDW_ENABLED;
}

float cvInternalGlobals::getIDW_BASE_COMBAT_INFLUENCE()
{
	return m_fIDW_BASE_COMBAT_INFLUENCE;
}

float cvInternalGlobals::getIDW_NO_CITY_DEFENDER_MULTIPLIER()
{
	return m_fIDW_NO_CITY_DEFENDER_MULTIPLIER;
}

float cvInternalGlobals::getIDW_FORT_CAPTURE_MULTIPLIER()
{
	return m_fIDW_FORT_CAPTURE_MULTIPLIER;
}

float cvInternalGlobals::getIDW_EXPERIENCE_FACTOR()
{
	return m_fIDW_EXPERIENCE_FACTOR;
}

float cvInternalGlobals::getIDW_WARLORD_MULTIPLIER()
{
	return m_fIDW_WARLORD_MULTIPLIER;
}

int cvInternalGlobals::getIDW_INFLUENCE_RADIUS()
{
	return m_iIDW_INFLUENCE_RADIUS;
}

float cvInternalGlobals::getIDW_PLOT_DISTANCE_FACTOR()
{
	return m_fIDW_PLOT_DISTANCE_FACTOR;
}

float cvInternalGlobals::getIDW_WINNER_PLOT_MULTIPLIER()
{
	return m_fIDW_WINNER_PLOT_MULTIPLIER;
}

float cvInternalGlobals::getIDW_LOSER_PLOT_MULTIPLIER()
{
	return m_fIDW_LOSER_PLOT_MULTIPLIER;
}

bool cvInternalGlobals::isIDW_EMERGENCY_DRAFT_ENABLED()
{
	return m_bIDW_EMERGENCY_DRAFT_ENABLED;
}

int cvInternalGlobals::getIDW_EMERGENCY_DRAFT_MIN_POPULATION()
{
	return m_iIDW_EMERGENCY_DRAFT_MIN_POPULATION;
}

float cvInternalGlobals::getIDW_EMERGENCY_DRAFT_STRENGTH()
{
	return m_fIDW_EMERGENCY_DRAFT_STRENGTH;
}

float cvInternalGlobals::getIDW_EMERGENCY_DRAFT_ANGER_MULTIPLIER()
{
	return m_fIDW_EMERGENCY_DRAFT_ANGER_MULTIPLIER;
}

bool cvInternalGlobals::isIDW_NO_BARBARIAN_INFLUENCE()
{
	return m_bIDW_NO_BARBARIAN_INFLUENCE;
}

bool cvInternalGlobals::isIDW_NO_NAVAL_INFLUENCE()
{
	return m_bIDW_NO_NAVAL_INFLUENCE;
}

bool cvInternalGlobals::isIDW_PILLAGE_INFLUENCE_ENABLED()
{
	return m_bIDW_PILLAGE_INFLUENCE_ENABLED;
}

float cvInternalGlobals::getIDW_BASE_PILLAGE_INFLUENCE()
{
	return m_fIDW_BASE_PILLAGE_INFLUENCE;
}

float cvInternalGlobals::getIDW_CITY_TILE_MULTIPLIER()
{
	return m_fIDW_CITY_TILE_MULTIPLIER;
}

bool cvInternalGlobals::isSS_ENABLED()
{
	return m_bSS_ENABLED;
}

bool cvInternalGlobals::isSS_BRIBE()
{
	return m_bSS_BRIBE;
}

bool cvInternalGlobals::isSS_ASSASSINATE()
{
	return m_bSS_ASSASSINATE;
}
/************************************************************************************************/
/* Mod Globals                        END                                           phungus420  */
/************************************************************************************************/
// BUFFY - DLL Info - start
#ifdef _BUFFY
bool cvInternalGlobals::isBuffy() const { return true; }
int cvInternalGlobals::getBuffyApiVersion() const { return BUFFY_DLL_API_VERSION; }
const wchar* cvInternalGlobals::getBuffyName() const { return BUFFY_DLL_NAME; }
const wchar* cvInternalGlobals::getBuffyVersion() const { return BUFFY_DLL_VERSION; }
#endif
// BUFFY - DLL Info - end


/**** Dexy - Dark Ages START ****/
const wchar* cvInternalGlobals::getRankingTextKeyWide(RankingTypes eRanking) const
{
	/* TODO read these from XML */
	switch (eRanking)
	{
	case RANKING_POWER:
		return L"TXT_KEY_RANKING_POWER";
	case RANKING_POPULATION:
		return L"TXT_KEY_RANKING_POPULATION";
	case RANKING_LAND:
		return L"TXT_KEY_RANKING_LAND";
	case RANKING_CULTURE:
		return L"TXT_KEY_RANKING_CULTURE";
	case RANKING_ESPIONAGE:
		return L"TXT_KEY_RANKING_ESPIONAGE";
	case RANKING_WONDERS:
		return L"TXT_KEY_RANKING_WONDERS";
	case RANKING_TECH:
		return L"TXT_KEY_RANKING_TECH";
	default:
		FAssertMsg(false, "Ranking type unknown");
		return L"Ranking Type Unknown";
	}			
}
/**** Dexy - Dark Ages  END  ****/

const wchar* cvInternalGlobals::parseDenialHover(DenialTypes eDenial)
{
	int iCount = getDefineINT(getDenialInfo(eDenial).getType());
	int iRand;
	if (iCount > 0)
	{
		iRand = getASyncRand().get(iCount);
		if (iRand == 0)
		{
			return GC.getDenialInfo(eDenial).getDescription();
		}
		else
		{
			CvWString szType = getDenialInfo(eDenial).getType();
			szType.append(CvWString::format(L"_%d", iRand));
			return gDLL->getText(szType);
		}
	}
	return GC.getDenialInfo(eDenial).getDescription();
}

// calculate asset checksum
unsigned int cvInternalGlobals::getAssetCheckSum()
{
	CvString szLog;
	unsigned int iSum = 0;
	for (std::vector<std::vector<CvInfoBase*> *>::iterator itOuter = m_aInfoVectors.begin(); itOuter != m_aInfoVectors.end(); itOuter++)
	{
		for (std::vector<CvInfoBase*>::iterator itInner = (*itOuter)->begin(); itInner != (*itOuter)->end(); itInner++)
		{
			(*itInner)->getCheckSum(iSum);
			szLog.Format("%s : %u", (*itInner)->getType(), iSum );
			gDLL->logMsg("Checksum.log", szLog.c_str());
		}
	}
	return iSum;
}


//	KOSHLING -  granular callback control
#ifdef GRANULAR_CALLBACK_CONTROL

//	Unit list for a named (unit based) callback which must be enabled
//	Logically OR'd into the current set
void GranularCallbackController::RegisterUnitCallback(PythonCallbackTypes eCallbackType, const char* unitList)
{
	//	UnitList is a comma-separated list
	char		unitBuffer[100];
	const char*	startPtr = unitList;
	const char*	endPtr;

	do
	{
		endPtr = strchr(startPtr, ',');
		if ( endPtr == NULL )
		{
			endPtr = startPtr + strlen(startPtr);
		}

		memcpy(unitBuffer, startPtr, (endPtr - startPtr));
		unitBuffer[(endPtr-startPtr)] = '\0';

		m_rawUnitCallbacks[eCallbackType].push_back(CvString(unitBuffer));

		startPtr = endPtr + 1;
	} while(*endPtr != '\0');
}

//	Unit list for a named (improvement based) callback which must be enabled
//	Logically OR'd into the current set
void GranularCallbackController::RegisterBuildCallback(PythonCallbackTypes eCallbackType, const char* buildList)
{
	//	buildList is a command-separated list
	char		buildBuffer[100];
	const char*	startPtr = buildList;
	const char*	endPtr;

	do
	{
		endPtr = strchr(startPtr, ',');
		if ( endPtr == NULL )
		{
			endPtr = startPtr + strlen(startPtr);
		}

		memcpy(buildBuffer, startPtr, (endPtr - startPtr));
		buildBuffer[(endPtr-startPtr)] = '\0';

		m_rawBuildCallbacks[eCallbackType].push_back(CvString(buildBuffer));

		startPtr = endPtr + 1;
	} while(*endPtr != '\0');
}

bool GranularCallbackController::IsUnitCallbackEnabled(PythonCallbackTypes eCallbackType, UnitTypes eUnit) const
{
	if ( !m_rawInputProcessed )
	{
		ProcessRawInput();
	}

	std::map<PythonCallbackTypes,std::map<UnitTypes,bool> >::const_iterator itr1 = m_unitCallbacks.find(eCallbackType);
	if ( itr1 != m_unitCallbacks.end())
	{
		std::map<UnitTypes,bool>::const_iterator itr2 = itr1->second.find(eUnit);

		return (itr2 != itr1->second.end());
	}

	return false;
}

bool GranularCallbackController::IsBuildCallbackEnabled(PythonCallbackTypes eCallbackType, BuildTypes eBuild) const
{
	if ( !m_rawInputProcessed )
	{
		ProcessRawInput();
	}

	std::map<PythonCallbackTypes,std::map<BuildTypes,bool> >::const_iterator itr1 = m_buildCallbacks.find(eCallbackType);
	if ( itr1 != m_buildCallbacks.end())
	{
		std::map<BuildTypes,bool>::const_iterator itr2 = itr1->second.find(eBuild);

		return (itr2 != itr1->second.end());
	}

	return false;
}

void GranularCallbackController::ProcessRawInput(void) const
{
	std::map<PythonCallbackTypes,std::vector<CvString> >::const_iterator itr;

	for(itr = m_rawUnitCallbacks.begin(); itr != m_rawUnitCallbacks.end(); itr++)
	{
		std::vector<CvString>::const_iterator unitNameItr;

		//	Loop over the units that have been specified
		for(unitNameItr = itr->second.begin(); unitNameItr != itr->second.end(); unitNameItr++)
		{
			OutputDebugString(CvString::format("Searching for unit type %s\n",unitNameItr->c_str()).c_str());

			UnitTypes eUnit = (UnitTypes)GC.getInfoTypeForString(unitNameItr->c_str());
			if ( eUnit != NO_UNIT )
			{
				m_unitCallbacks[itr->first][eUnit] = true;
			}
		}
	}

	for(itr = m_rawBuildCallbacks.begin(); itr != m_rawBuildCallbacks.end(); itr++)
	{
		std::vector<CvString>::const_iterator buildNameItr;

		//	Loop over the units that have been specified
		for(buildNameItr = itr->second.begin(); buildNameItr != itr->second.end(); buildNameItr++)
		{
			BuildTypes eBuild = (BuildTypes)GC.getInfoTypeForString(buildNameItr->c_str());
			if ( eBuild != NO_IMPROVEMENT )
			{
				m_buildCallbacks[itr->first][eBuild] = true;
			}
		}
	}

	m_rawInputProcessed = true;
}
#endif
