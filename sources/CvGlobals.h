#pragma once

// cvGlobals.h

#ifndef CIV4_GLOBALS_H
#define CIV4_GLOBALS_H

//#include "CvStructs.h"
//
// 'global' vars for Civ IV.  singleton class.
// All globals and global types should be contained in this class
//

class FProfiler;
class CvDLLUtilityIFaceBase;
class CvRandom;
class CvGameAI;
class CMessageControl;
class CvDropMgr;
class CMessageQueue;
class CvSetupData;
class CvInitCore;
class CvMessageCodeTranslator;
class CvPortal;
class CvStatsReporter;
class CvDLLInterfaceIFaceBase;
class CvPlayerAI;
class CvDiplomacyScreen;
class CvCivicsScreen;
class CvWBUnitEditScreen;
class CvWBCityEditScreen;
class CMPDiplomacyScreen;
class FMPIManager;
class FAStar;
class CvInterface;
class CMainMenu;
class CvEngine;
class CvArtFileMgr;
class FVariableSystem;
class CvMap;
class CvPlayerAI;
class CvTeamAI;
class CvInterfaceModeInfo;
class CvWorldInfo;
class CvClimateInfo;
class CvSeaLevelInfo;
class CvColorInfo;
class CvPlayerColorInfo;
class CvAdvisorInfo;
class CvRouteModelInfo;
class CvRiverInfo;
class CvRiverModelInfo;
class CvWaterPlaneInfo;
class CvTerrainPlaneInfo;
class CvCameraOverlayInfo;
class CvAnimationPathInfo;
class CvAnimationCategoryInfo;
class CvEntityEventInfo;
class CvEffectInfo;
class CvAttachableInfo;
class CvCameraInfo;
class CvUnitFormationInfo;
class CvGameText;
class CvLandscapeInfo;
class CvTerrainInfo;
class CvBonusClassInfo;
class CvBonusInfo;
class CvFeatureInfo;
class CvCivilizationInfo;
class CvLeaderHeadInfo;
class CvTraitInfo;
class CvCursorInfo;
class CvThroneRoomCamera;
class CvThroneRoomInfo;
class CvThroneRoomStyleInfo;
class CvSlideShowInfo;
class CvSlideShowRandomInfo;
class CvWorldPickerInfo;
class CvSpaceShipInfo;
class CvUnitInfo;
class CvSpawnInfo;
class CvSpecialUnitInfo;
class CvInfoBase;
class CvYieldInfo;
class CvCommerceInfo;
class CvRouteInfo;
class CvImprovementInfo;
class CvGoodyInfo;
class CvBuildInfo;
class CvHandicapInfo;
class CvGameSpeedInfo;
class CvTurnTimerInfo;
class CvProcessInfo;
class CvVoteInfo;
class CvProjectInfo;
class CvBuildingClassInfo;
class CvBuildingInfo;
class CvSpecialBuildingInfo;
class CvUnitClassInfo;
class CvActionInfo;
class CvMissionInfo;
class CvControlInfo;
class CvCommandInfo;
class CvAutomateInfo;
class CvPromotionInfo;
class CvTechInfo;
class CvReligionInfo;
class CvCorporationInfo;
class CvSpecialistInfo;
class CvCivicOptionInfo;
class CvCivicInfo;
class CvDiplomacyInfo;
class CvEraInfo;
class CvHurryInfo;
class CvEmphasizeInfo;
class CvUpkeepInfo;
class CvCultureLevelInfo;
class CvVictoryInfo;
class CvQuestInfo;
class CvGameOptionInfo;
class CvMPOptionInfo;
class CvForceControlInfo;
class CvPlayerOptionInfo;
class CvGraphicOptionInfo;
class CvTutorialInfo;
class CvEventTriggerInfo;
class CvEventInfo;
class CvEspionageMissionInfo;
class CvUnitArtStyleTypeInfo;
class CvVoteSourceInfo;
class CvMainMenuInfo;
class CvCulturalAgeInfo;
class CvPropertyInfo;
class CvOutcomeInfo;
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 10/24/07                                MRGENIE      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
// Python Modular Loading
class CvPythonModulesInfo;
// MLF loading
class CvModLoadControlInfo;
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/

//	KOSHLING - granular control over callback enabling
#define GRANULAR_CALLBACK_CONTROL
#ifdef GRANULAR_CALLBACK_CONTROL
typedef enum
{
	CALLBACK_TYPE_CAN_TRAIN = 1,
	CALLBACK_TYPE_CANNOT_TRAIN = 2,
	CALLBACK_TYPE_CAN_BUILD = 3
} PythonCallbackTypes;

class GranularCallbackController
{
public:
	GranularCallbackController()
	{
		m_rawInputProcessed = false;
	}

	//	Unit list for a named (unit based) callback which must be enabled
	//	Logically OR'd into the current set
	void RegisterUnitCallback(PythonCallbackTypes eCallbackType, const char* unitList);
	//	Unit list for a named (improvement based) callback which must be enabled
	//	Logically OR'd into the current set
	void RegisterBuildCallback(PythonCallbackTypes eCallbackType, const char* buildList);
	
	bool IsUnitCallbackEnabled(PythonCallbackTypes eCallbackType, UnitTypes eUnit) const;
	bool IsBuildCallbackEnabled(PythonCallbackTypes eCallbackType, BuildTypes eBuild) const;

private:
	void ProcessRawInput(void) const;

	std::map<PythonCallbackTypes,std::vector<CvString> > m_rawUnitCallbacks;			//	Raw strings aggregated from each registartion call
	std::map<PythonCallbackTypes,std::vector<CvString> > m_rawBuildCallbacks;		//	Raw strings aggregated from each registartion call
	mutable std::map<PythonCallbackTypes,std::map<UnitTypes,bool> > m_unitCallbacks;			//	Processed list indexed by unit types
	mutable std::map<PythonCallbackTypes,std::map<BuildTypes,bool> > m_buildCallbacks;	//	Processed list indexed by improvement types
	mutable bool m_rawInputProcessed;
};
#endif

extern CvDLLUtilityIFaceBase* g_DLL;

class cvInternalGlobals
{
//	friend class CvDLLUtilityIFace;
	friend class CvXMLLoadUtility;
public:

	// singleton accessor
	inline static cvInternalGlobals& getInstance();

	cvInternalGlobals();
	virtual ~cvInternalGlobals();

	void init();
	void uninit();
	void clearTypesMap();

	CvDiplomacyScreen* getDiplomacyScreen();
	CMPDiplomacyScreen* getMPDiplomacyScreen();

	FMPIManager*& getFMPMgrPtr();
	CvPortal& getPortal();
	CvSetupData& getSetupData();
	CvInitCore& getInitCore();
	CvInitCore& getLoadedInitCore();
	CvInitCore& getIniInitCore();
	CvMessageCodeTranslator& getMessageCodes();
	CvStatsReporter& getStatsReporter();
	CvStatsReporter* getStatsReporterPtr();
	CvInterface& getInterface();
	CvInterface* getInterfacePtr();
	int getMaxCivPlayers() const;
#ifdef _USRDLL
	CvMap& getMapINLINE() { return *m_map; }				// inlined for perf reasons, do not use outside of dll
	CvGameAI& getGameINLINE() { return *m_game; }			// inlined for perf reasons, do not use outside of dll
#endif
	CvMap& getMap();
	CvGameAI& getGame();
	CvGameAI *getGamePointer();
	CvRandom& getASyncRand();
	CMessageQueue& getMessageQueue();
	CMessageQueue& getHotMessageQueue();
	CMessageControl& getMessageControl();
	CvDropMgr& getDropMgr();
	FAStar& getPathFinder();
	FAStar& getInterfacePathFinder();
	FAStar& getStepFinder();
	FAStar& getRouteFinder();
	FAStar& getBorderFinder();
	FAStar& getAreaFinder();
	FAStar& getPlotGroupFinder();
	NiPoint3& getPt3Origin();

	std::vector<CvInterfaceModeInfo*>& getInterfaceModeInfo();
	CvInterfaceModeInfo& getInterfaceModeInfo(InterfaceModeTypes e);

	DllExport NiPoint3& getPt3CameraDir();

	bool& getLogging();
	bool& getRandLogging();
	bool& getSynchLogging();
	bool& overwriteLogs();

	int* getPlotDirectionX();
	int* getPlotDirectionY();
	int* getPlotCardinalDirectionX();
	int* getPlotCardinalDirectionY();
	int* getCityPlotX();
	int* getCityPlotY();
	int* getCityPlotPriority();
	int getXYCityPlot(int i, int j);
	DirectionTypes* getTurnLeftDirection();
	DirectionTypes getTurnLeftDirection(int i);
	DirectionTypes* getTurnRightDirection();
	DirectionTypes getTurnRightDirection(int i);
	DirectionTypes getXYDirection(int i, int j);

/************************************************************************************************/
/* SORT_ALPHABET                           11/19/07                                MRGENIE      */
/*                                                                                              */
/* Method for alphabetically order tags                                                         */
/************************************************************************************************/
void initUnitEventMissions();
	//
	// Global Infos
	// All info type strings are upper case and are kept in this hash map for fast lookup
	//
	int getInfoTypeForString(const char* szType, bool hideAssert = false) const;			// returns the infos index, use this when searching for an info type string
/*
	DllExport void setInfoTypeFromString(const char* szType, int idx);
*/
	void setInfoTypeFromString(const char* szType, int idx);
	void logInfoTypeMap(const char* tagMsg = "");
/************************************************************************************************/
/* SORT_ALPHABET                           END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 11/30/07                                MRGENIE      */
/*                                                                                              */
/* Savegame compatibility                                                                       */
/************************************************************************************************/
	void infoTypeFromStringReset();
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
	void addToInfosVectors(void *infoVector);
	void infosReset();

	int getNumWorldInfos();
	std::vector<CvWorldInfo*>& getWorldInfo();
	CvWorldInfo& getWorldInfo(WorldSizeTypes e);

	int getNumClimateInfos();
	std::vector<CvClimateInfo*>& getClimateInfo();
	CvClimateInfo& getClimateInfo(ClimateTypes e);

	int getNumSeaLevelInfos();
	std::vector<CvSeaLevelInfo*>& getSeaLevelInfo();
	CvSeaLevelInfo& getSeaLevelInfo(SeaLevelTypes e);

	int getNumColorInfos();
	std::vector<CvColorInfo*>& getColorInfo();
	CvColorInfo& getColorInfo(ColorTypes e);

	int getNumPlayerColorInfos();
	std::vector<CvPlayerColorInfo*>& getPlayerColorInfo();
	CvPlayerColorInfo& getPlayerColorInfo(PlayerColorTypes e);

	int getNumAdvisorInfos();
	std::vector<CvAdvisorInfo*>& getAdvisorInfo();
	CvAdvisorInfo& getAdvisorInfo(AdvisorTypes e);

	int getNumHints();
	std::vector<CvInfoBase*>& getHints();
	CvInfoBase& getHints(int i);

	int getNumMainMenus();
	std::vector<CvMainMenuInfo*>& getMainMenus();
	CvMainMenuInfo& getMainMenus(int i);
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 10/30/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	// Python Modular Loading
	int getNumPythonModulesInfos();
	std::vector<CvPythonModulesInfo*>& getPythonModulesInfo();
	CvPythonModulesInfo& getPythonModulesInfo(int i);
	// MLF loading
	void resetModLoadControlVector();
	int getModLoadControlVectorSize();
	void setModLoadControlVector(const char* szModule);
	CvString getModLoadControlVector(int i);

	int getTotalNumModules();
	void setTotalNumModules();
	int getNumModLoadControlInfos();
	std::vector<CvModLoadControlInfo*>& getModLoadControlInfos();
	CvModLoadControlInfo& getModLoadControlInfos(int i);
	// Modular loading Dependencies
	void resetDependencies();
	bool isAnyDependency();
	void setAnyDependency(bool bAnyDependency);
	bool& getTypeDependency();
	void setTypeDependency(bool newValue);
	const int getAndNumDependencyTypes() const;
	void setAndDependencyTypes(const char* szDependencyTypes);
	const CvString getAndDependencyTypes(int i) const;
	const int getOrNumDependencyTypes() const;
	void setOrDependencyTypes(const char* szDependencyTypes);
	const CvString getOrDependencyTypes(int i) const;
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 10/26/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	void setModDir(const char* szModDir);
	std::string getModDir();
	std::string m_cszModDir;
/************************************************************************************************/
/* XML_MODULAR_ART_LOADING                 END                                                  */
/************************************************************************************************/	

	int getNumRouteModelInfos();
	std::vector<CvRouteModelInfo*>& getRouteModelInfo();
	CvRouteModelInfo& getRouteModelInfo(int i);

	int getNumRiverInfos();
	std::vector<CvRiverInfo*>& getRiverInfo();
	CvRiverInfo& getRiverInfo(RiverTypes e);

	int getNumRiverModelInfos();
	std::vector<CvRiverModelInfo*>& getRiverModelInfo();
	CvRiverModelInfo& getRiverModelInfo(int i);

	int getNumWaterPlaneInfos();
	std::vector<CvWaterPlaneInfo*>& getWaterPlaneInfo();
	CvWaterPlaneInfo& getWaterPlaneInfo(int i);

	int getNumTerrainPlaneInfos();
	std::vector<CvTerrainPlaneInfo*>& getTerrainPlaneInfo();
	CvTerrainPlaneInfo& getTerrainPlaneInfo(int i);

	int getNumCameraOverlayInfos();
	std::vector<CvCameraOverlayInfo*>& getCameraOverlayInfo();
	CvCameraOverlayInfo& getCameraOverlayInfo(int i);

	int getNumAnimationPathInfos();
	std::vector<CvAnimationPathInfo*>& getAnimationPathInfo();
	CvAnimationPathInfo& getAnimationPathInfo(AnimationPathTypes e);

	int getNumAnimationCategoryInfos();
	std::vector<CvAnimationCategoryInfo*>& getAnimationCategoryInfo();
	CvAnimationCategoryInfo& getAnimationCategoryInfo(AnimationCategoryTypes e);

	int getNumEntityEventInfos();
	std::vector<CvEntityEventInfo*>& getEntityEventInfo();
	CvEntityEventInfo& getEntityEventInfo(EntityEventTypes e);

	int getNumEffectInfos();
	std::vector<CvEffectInfo*>& getEffectInfo();
	CvEffectInfo& getEffectInfo(int i);

	int getNumAttachableInfos();
	std::vector<CvAttachableInfo*>& getAttachableInfo();
	CvAttachableInfo& getAttachableInfo(int i);

	int getNumCameraInfos();
	std::vector<CvCameraInfo*>& getCameraInfo();
	CvCameraInfo& getCameraInfo(CameraAnimationTypes eCameraAnimationNum);

	int getNumUnitFormationInfos();
	std::vector<CvUnitFormationInfo*>& getUnitFormationInfo();
	CvUnitFormationInfo& getUnitFormationInfo(int i);

	int getNumGameTextXML();
	std::vector<CvGameText*>& getGameTextXML();

	int getNumLandscapeInfos();
	std::vector<CvLandscapeInfo*>& getLandscapeInfo();
	CvLandscapeInfo& getLandscapeInfo(int iIndex);
	int getActiveLandscapeID();
	void setActiveLandscapeID(int iLandscapeID);

	int getNumTerrainInfos();
	std::vector<CvTerrainInfo*>& getTerrainInfo();
	CvTerrainInfo& getTerrainInfo(TerrainTypes eTerrainNum);

	int getNumBonusClassInfos();
	std::vector<CvBonusClassInfo*>& getBonusClassInfo();
	CvBonusClassInfo& getBonusClassInfo(BonusClassTypes eBonusNum);

	int getNumBonusInfos();
	std::vector<CvBonusInfo*>& getBonusInfo();
	CvBonusInfo& getBonusInfo(BonusTypes eBonusNum);

	int getNumFeatureInfos();
	std::vector<CvFeatureInfo*>& getFeatureInfo();
	CvFeatureInfo& getFeatureInfo(FeatureTypes eFeatureNum);

	int& getNumPlayableCivilizationInfos();
	int& getNumAIPlayableCivilizationInfos();
	int getNumCivilizationInfos();
	std::vector<CvCivilizationInfo*>& getCivilizationInfo();
	CvCivilizationInfo& getCivilizationInfo(CivilizationTypes eCivilizationNum);

	int getNumLeaderHeadInfos();
	std::vector<CvLeaderHeadInfo*>& getLeaderHeadInfo();
	CvLeaderHeadInfo& getLeaderHeadInfo(LeaderHeadTypes eLeaderHeadNum);

	int getNumTraitInfos();
	std::vector<CvTraitInfo*>& getTraitInfo();
	CvTraitInfo& getTraitInfo(TraitTypes eTraitNum);

	int getNumCursorInfos();
	std::vector<CvCursorInfo*>& getCursorInfo();
	CvCursorInfo& getCursorInfo(CursorTypes eCursorNum);

	int getNumThroneRoomCameras();
	std::vector<CvThroneRoomCamera*>& getThroneRoomCamera();
	CvThroneRoomCamera& getThroneRoomCamera(int iIndex);

	int getNumThroneRoomInfos();
	std::vector<CvThroneRoomInfo*>& getThroneRoomInfo();
	CvThroneRoomInfo& getThroneRoomInfo(int iIndex);

	int getNumThroneRoomStyleInfos();
	std::vector<CvThroneRoomStyleInfo*>& getThroneRoomStyleInfo();
	CvThroneRoomStyleInfo& getThroneRoomStyleInfo(int iIndex);

	int getNumSlideShowInfos();
	std::vector<CvSlideShowInfo*>& getSlideShowInfo();
	CvSlideShowInfo& getSlideShowInfo(int iIndex);

	int getNumSlideShowRandomInfos();
	std::vector<CvSlideShowRandomInfo*>& getSlideShowRandomInfo();
	CvSlideShowRandomInfo& getSlideShowRandomInfo(int iIndex);

	int getNumWorldPickerInfos();
	std::vector<CvWorldPickerInfo*>& getWorldPickerInfo();
	CvWorldPickerInfo& getWorldPickerInfo(int iIndex);

	int getNumSpaceShipInfos();
	std::vector<CvSpaceShipInfo*>& getSpaceShipInfo();
	CvSpaceShipInfo& getSpaceShipInfo(int iIndex);

	int getNumUnitInfos();
	std::vector<CvUnitInfo*>& getUnitInfo();
	CvUnitInfo& getUnitInfo(UnitTypes eUnitNum);

	int getNumSpawnInfos();
	std::vector<CvSpawnInfo*>& getSpawnInfo();
	CvSpawnInfo& getSpawnInfo(SpawnTypes eSpawnNum);

	int getNumSpecialUnitInfos();
	std::vector<CvSpecialUnitInfo*>& getSpecialUnitInfo();
	CvSpecialUnitInfo& getSpecialUnitInfo(SpecialUnitTypes eSpecialUnitNum);

	int getNumConceptInfos();
	std::vector<CvInfoBase*>& getConceptInfo();
	CvInfoBase& getConceptInfo(ConceptTypes e);

	int getNumNewConceptInfos();
	std::vector<CvInfoBase*>& getNewConceptInfo();
	CvInfoBase& getNewConceptInfo(NewConceptTypes e);

	int getNumCulturalAgeInfos();
	std::vector<CvCulturalAgeInfo*>& getCulturalAgeInfo();
	CvCulturalAgeInfo& getCulturalAgeInfo(CulturalAgeTypes eCulturalAgeNum);

	int getNumPropertyInfos();
	std::vector<CvPropertyInfo*>& getPropertyInfo();
	CvPropertyInfo& getPropertyInfo(PropertyTypes ePropertyNum);

	int getNumOutcomeInfos();
	std::vector<CvOutcomeInfo*>& getOutcomeInfo();
	CvOutcomeInfo& getOutcomeInfo(OutcomeTypes eOutcomeNum);

/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - DCM: Pedia Concepts START
	int getNumDCMConceptInfos();
	std::vector<CvInfoBase*>& getDCMConceptInfo();
	CvInfoBase& getDCMConceptInfo(DCMConceptTypes e);
	// Dale - DCM: Pedia Concepts END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/*Afforess                                     12/21/09                                         */
/************************************************************************************************/
	int getNumANDConceptInfos();
	std::vector<CvInfoBase*>& getANDConceptInfo();
	CvInfoBase& getANDConceptInfo(ANDConceptTypes e);
	
	int getPEAK_EXTRA_DEFENSE();
	int getPEAK_EXTRA_MOVEMENT();
	bool isBetterRoM() const;
	bool isFormationsMod() const;
	
	int iStuckUnitID;
	int iStuckUnitCount;
	
	bool isLoadedPlayerOptions() const;
	void setLoadedPlayerOptions(bool bNewVal);
	
	bool isXMLLogging();
	void setXMLLogging(bool bNewVal);
	
	bool& getForceOverwrite();
	void setForceOverwrite(bool newValue);
	
	bool& getForceDelete();
	void setForceDelete(bool newValue);
	
	int& getForceInsertLocation();
	void setForceInsertLocation(int newValue);
	
	void resetOverwrites();
	void insertInfoTypeFromString(const char* szType, int idx);
	
	const wchar* parseDenialHover(DenialTypes eDenial);
	
	int getSCORE_FREE_PERCENT();
	int getSCORE_POPULATION_FACTOR();
	int getSCORE_LAND_FACTOR();
	int getSCORE_TECH_FACTOR();
	int getSCORE_WONDER_FACTOR();
	
	int getUSE_CAN_CREATE_PROJECT_CALLBACK();
	int getUSE_CANNOT_CREATE_PROJECT_CALLBACK();
	int getUSE_CAN_DO_MELTDOWN_CALLBACK();
	int getUSE_CAN_MAINTAIN_PROCESS_CALLBACK();
	int getUSE_CANNOT_MAINTAIN_PROCESS_CALLBACK();
	int getUSE_CAN_DO_GROWTH_CALLBACK();
	int getUSE_CAN_DO_CULTURE_CALLBACK();
	int getUSE_CAN_DO_PLOT_CULTURE_CALLBACK();
	int getUSE_CAN_DO_PRODUCTION_CALLBACK();
	int getUSE_CAN_DO_RELIGION_CALLBACK();
	int getUSE_CAN_DO_GREATPEOPLE_CALLBACK();
	int getUSE_CAN_RAZE_CITY_CALLBACK();
	int getUSE_CAN_DO_GOLD_CALLBACK();
	int getUSE_CAN_DO_RESEARCH_CALLBACK();
	int getUSE_UPGRADE_UNIT_PRICE_CALLBACK();
	int getUSE_IS_VICTORY_CALLBACK();
	int getUSE_AI_UPDATE_UNIT_CALLBACK();
	int getUSE_AI_CHOOSE_PRODUCTION_CALLBACK();
	int getUSE_EXTRA_PLAYER_COSTS_CALLBACK();
	int getUSE_AI_DO_DIPLO_CALLBACK();
	int getUSE_AI_BESTTECH_CALLBACK();
	int getUSE_CAN_DO_COMBAT_CALLBACK();
	int getUSE_AI_CAN_DO_WARPLANS_CALLBACK();
/************************************************************************************************/
/* Afforess                                END                                                  */
/************************************************************************************************/
	int getNumCityTabInfos();
	std::vector<CvInfoBase*>& getCityTabInfo();
	CvInfoBase& getCityTabInfo(CityTabTypes e);

	int getNumCalendarInfos();
	std::vector<CvInfoBase*>& getCalendarInfo();
	CvInfoBase& getCalendarInfo(CalendarTypes e);

	int getNumSeasonInfos();
	std::vector<CvInfoBase*>& getSeasonInfo();
	CvInfoBase& getSeasonInfo(SeasonTypes e);

	int getNumMonthInfos();
	std::vector<CvInfoBase*>& getMonthInfo();
	CvInfoBase& getMonthInfo(MonthTypes e);

	int getNumDenialInfos();
	std::vector<CvInfoBase*>& getDenialInfo();
	CvInfoBase& getDenialInfo(DenialTypes e);

	int getNumInvisibleInfos();
	std::vector<CvInfoBase*>& getInvisibleInfo();
	CvInfoBase& getInvisibleInfo(InvisibleTypes e);

	int getNumVoteSourceInfos();
	std::vector<CvVoteSourceInfo*>& getVoteSourceInfo();
	CvVoteSourceInfo& getVoteSourceInfo(VoteSourceTypes e);

	int getNumUnitCombatInfos();
	std::vector<CvInfoBase*>& getUnitCombatInfo();
	CvInfoBase& getUnitCombatInfo(UnitCombatTypes e);

	std::vector<CvInfoBase*>& getDomainInfo();
	CvInfoBase& getDomainInfo(DomainTypes e);

	std::vector<CvInfoBase*>& getUnitAIInfo();
	CvInfoBase& getUnitAIInfo(UnitAITypes eUnitAINum);

	//	Koshling - added internal registration odf supported UnitAI types, not reliant
	//	on external definition in XML
private:
	void registerUnitAI(const char* szType, int enumVal);
public:
	void registerUnitAIs(void);
	void registerAIScales(void);
	void registerGameObjects(void);

	std::vector<CvInfoBase*>& getAttitudeInfo();
	CvInfoBase& getAttitudeInfo(AttitudeTypes eAttitudeNum);

	std::vector<CvInfoBase*>& getMemoryInfo();
	CvInfoBase& getMemoryInfo(MemoryTypes eMemoryNum);

	int getNumGameOptionInfos();
	std::vector<CvGameOptionInfo*>& getGameOptionInfo();
	CvGameOptionInfo& getGameOptionInfo(GameOptionTypes eGameOptionNum);

	int getNumMPOptionInfos();
	std::vector<CvMPOptionInfo*>& getMPOptionInfo();
	CvMPOptionInfo& getMPOptionInfo(MultiplayerOptionTypes eMPOptionNum);

	int getNumForceControlInfos();
	std::vector<CvForceControlInfo*>& getForceControlInfo();
	CvForceControlInfo& getForceControlInfo(ForceControlTypes eForceControlNum);

	std::vector<CvPlayerOptionInfo*>& getPlayerOptionInfo();
	CvPlayerOptionInfo& getPlayerOptionInfo(PlayerOptionTypes ePlayerOptionNum);

	std::vector<CvGraphicOptionInfo*>& getGraphicOptionInfo();
	CvGraphicOptionInfo& getGraphicOptionInfo(GraphicOptionTypes eGraphicOptionNum);

	std::vector<CvYieldInfo*>& getYieldInfo();
	CvYieldInfo& getYieldInfo(YieldTypes eYieldNum);

	std::vector<CvCommerceInfo*>& getCommerceInfo();
	CvCommerceInfo& getCommerceInfo(CommerceTypes eCommerceNum);

	int getNumRouteInfos();
	std::vector<CvRouteInfo*>& getRouteInfo();
	CvRouteInfo& getRouteInfo(RouteTypes eRouteNum);

	int getNumImprovementInfos();
	std::vector<CvImprovementInfo*>& getImprovementInfo();
	CvImprovementInfo& getImprovementInfo(ImprovementTypes eImprovementNum);

	int getNumGoodyInfos();
	std::vector<CvGoodyInfo*>& getGoodyInfo();
	CvGoodyInfo& getGoodyInfo(GoodyTypes eGoodyNum);

	int getNumBuildInfos();
	std::vector<CvBuildInfo*>& getBuildInfo();
	CvBuildInfo& getBuildInfo(BuildTypes eBuildNum);

	int getNumHandicapInfos();
	std::vector<CvHandicapInfo*>& getHandicapInfo();
	CvHandicapInfo& getHandicapInfo(HandicapTypes eHandicapNum);

	int getNumGameSpeedInfos();
	std::vector<CvGameSpeedInfo*>& getGameSpeedInfo();
	CvGameSpeedInfo& getGameSpeedInfo(GameSpeedTypes eGameSpeedNum);

	int getNumTurnTimerInfos();
	std::vector<CvTurnTimerInfo*>& getTurnTimerInfo();
	CvTurnTimerInfo& getTurnTimerInfo(TurnTimerTypes eTurnTimerNum);

	int getNumProcessInfos();
	std::vector<CvProcessInfo*>& getProcessInfo();
	CvProcessInfo& getProcessInfo(ProcessTypes e);

	int getNumVoteInfos();
	std::vector<CvVoteInfo*>& getVoteInfo();
	CvVoteInfo& getVoteInfo(VoteTypes e);

	int getNumProjectInfos();
	std::vector<CvProjectInfo*>& getProjectInfo();
	CvProjectInfo& getProjectInfo(ProjectTypes e);

	int getNumBuildingClassInfos();
	std::vector<CvBuildingClassInfo*>& getBuildingClassInfo();
	CvBuildingClassInfo& getBuildingClassInfo(BuildingClassTypes eBuildingClassNum);

	int getNumBuildingInfos();
	std::vector<CvBuildingInfo*>& getBuildingInfo();
	CvBuildingInfo& getBuildingInfo(BuildingTypes eBuildingNum);

	int getNumSpecialBuildingInfos();
	std::vector<CvSpecialBuildingInfo*>& getSpecialBuildingInfo();
	CvSpecialBuildingInfo& getSpecialBuildingInfo(SpecialBuildingTypes eSpecialBuildingNum);

	int getNumUnitClassInfos();
	std::vector<CvUnitClassInfo*>& getUnitClassInfo();
	CvUnitClassInfo& getUnitClassInfo(UnitClassTypes eUnitClassNum);

	int getNumActionInfos();
	std::vector<CvActionInfo*>& getActionInfo();
	CvActionInfo& getActionInfo(int i);

	std::vector<CvMissionInfo*>& getMissionInfo();
	CvMissionInfo& getMissionInfo(MissionTypes eMissionNum);

	std::vector<CvControlInfo*>& getControlInfo();
	CvControlInfo& getControlInfo(ControlTypes eControlNum);

	std::vector<CvCommandInfo*>& getCommandInfo();
	CvCommandInfo& getCommandInfo(CommandTypes eCommandNum);

	int getNumAutomateInfos();
	std::vector<CvAutomateInfo*>& getAutomateInfo();
	CvAutomateInfo& getAutomateInfo(int iAutomateNum);

	int getNumPromotionInfos();
	std::vector<CvPromotionInfo*>& getPromotionInfo();
	CvPromotionInfo& getPromotionInfo(PromotionTypes ePromotionNum);

	int getNumTechInfos();
	std::vector<CvTechInfo*>& getTechInfo();
	CvTechInfo& getTechInfo(TechTypes eTechNum);

	int getNumReligionInfos();
	std::vector<CvReligionInfo*>& getReligionInfo();
	CvReligionInfo& getReligionInfo(ReligionTypes eReligionNum);

	int getNumCorporationInfos();
	std::vector<CvCorporationInfo*>& getCorporationInfo();
	CvCorporationInfo& getCorporationInfo(CorporationTypes eCorporationNum);

	int getNumSpecialistInfos();
	std::vector<CvSpecialistInfo*>& getSpecialistInfo();
	CvSpecialistInfo& getSpecialistInfo(SpecialistTypes eSpecialistNum);

	int getNumCivicOptionInfos();
	std::vector<CvCivicOptionInfo*>& getCivicOptionInfo();
	CvCivicOptionInfo& getCivicOptionInfo(CivicOptionTypes eCivicOptionNum);

	int getNumCivicInfos();
	std::vector<CvCivicInfo*>& getCivicInfo();
	CvCivicInfo& getCivicInfo(CivicTypes eCivicNum);

	int getNumDiplomacyInfos();
	std::vector<CvDiplomacyInfo*>& getDiplomacyInfo();
	CvDiplomacyInfo& getDiplomacyInfo(int iDiplomacyNum);

	int getNumEraInfos();
	std::vector<CvEraInfo*>& getEraInfo();
	CvEraInfo& getEraInfo(EraTypes eEraNum);

	int getNumHurryInfos();
	std::vector<CvHurryInfo*>& getHurryInfo();
	CvHurryInfo& getHurryInfo(HurryTypes eHurryNum);

	int getNumEmphasizeInfos();
	std::vector<CvEmphasizeInfo*>& getEmphasizeInfo();
	CvEmphasizeInfo& getEmphasizeInfo(EmphasizeTypes eEmphasizeNum);

	int getNumUpkeepInfos();
	std::vector<CvUpkeepInfo*>& getUpkeepInfo();
	CvUpkeepInfo& getUpkeepInfo(UpkeepTypes eUpkeepNum);

	int getNumCultureLevelInfos();
	std::vector<CvCultureLevelInfo*>& getCultureLevelInfo();
	CvCultureLevelInfo& getCultureLevelInfo(CultureLevelTypes eCultureLevelNum);

	int getNumVictoryInfos();
	std::vector<CvVictoryInfo*>& getVictoryInfo();
	CvVictoryInfo& getVictoryInfo(VictoryTypes eVictoryNum);

	int getNumQuestInfos();
	std::vector<CvQuestInfo*>& getQuestInfo();
	CvQuestInfo& getQuestInfo(int iIndex);

	int getNumTutorialInfos();
	std::vector<CvTutorialInfo*>& getTutorialInfo();
	CvTutorialInfo& getTutorialInfo(int i);

	int getNumEventTriggerInfos();
	std::vector<CvEventTriggerInfo*>& getEventTriggerInfo();
	CvEventTriggerInfo& getEventTriggerInfo(EventTriggerTypes eEventTrigger);

	int getNumEventInfos();
	std::vector<CvEventInfo*>& getEventInfo();
	CvEventInfo& getEventInfo(EventTypes eEvent);

	int getNumEspionageMissionInfos();
	std::vector<CvEspionageMissionInfo*>& getEspionageMissionInfo();
	CvEspionageMissionInfo& getEspionageMissionInfo(EspionageMissionTypes eEspionageMissionNum);

	int getNumUnitArtStyleTypeInfos();
	std::vector<CvUnitArtStyleTypeInfo*>& getUnitArtStyleTypeInfo();
	CvUnitArtStyleTypeInfo& getUnitArtStyleTypeInfo(UnitArtStyleTypes eUnitArtStyleTypeNum);

	//
	// Global Types
	// All type strings are upper case and are kept in this hash map for fast lookup
	// The other functions are kept for convenience when enumerating, but most are not used
	//
	int getTypesEnum(const char* szType) const;				// use this when searching for a type
	void setTypesEnum(const char* szType, int iEnum);

	int getNUM_ENGINE_DIRTY_BITS() const;
	int getNUM_INTERFACE_DIRTY_BITS() const;
	int getNUM_YIELD_TYPES() const;
	int getNUM_COMMERCE_TYPES() const;
	int getNUM_FORCECONTROL_TYPES() const;
	int getNUM_INFOBAR_TYPES() const;
	int getNUM_HEALTHBAR_TYPES() const;
	int getNUM_CONTROL_TYPES() const;
	int getNUM_LEADERANIM_TYPES() const;

	int& getNumEntityEventTypes();
	CvString*& getEntityEventTypes();
	CvString& getEntityEventTypes(EntityEventTypes e);

	int& getNumAnimationOperatorTypes();
	CvString*& getAnimationOperatorTypes();
	CvString& getAnimationOperatorTypes(AnimationOperatorTypes e);

	CvString*& getFunctionTypes();
	CvString& getFunctionTypes(FunctionTypes e);

	int& getNumFlavorTypes();
	CvString*& getFlavorTypes();
	CvString& getFlavorTypes(FlavorTypes e);

	int& getNumArtStyleTypes();
	CvString*& getArtStyleTypes();
	DllExport CvString& getArtStyleTypes(ArtStyleTypes e);

	int& getNumCitySizeTypes();
	CvString*& getCitySizeTypes();
	CvString& getCitySizeTypes(int i);

	CvString*& getContactTypes();
	CvString& getContactTypes(ContactTypes e);

	CvString*& getDiplomacyPowerTypes();
	CvString& getDiplomacyPowerTypes(DiplomacyPowerTypes e);

	CvString*& getAutomateTypes();
	CvString& getAutomateTypes(AutomateTypes e);

	CvString*& getDirectionTypes();
	CvString& getDirectionTypes(AutomateTypes e);

	int& getNumFootstepAudioTypes();
	CvString*& getFootstepAudioTypes();
	CvString& getFootstepAudioTypes(int i);
	int getFootstepAudioTypeByTag(CvString strTag);

	CvString*& getFootstepAudioTags();
	CvString& getFootstepAudioTags(int i);

	CvString& getCurrentXMLFile();
	void setCurrentXMLFile(const TCHAR* szFileName);

	//
	///////////////// BEGIN global defines
	// THESE ARE READ-ONLY
	//

	FVariableSystem* getDefinesVarSystem();
	void cacheGlobals();

	// ***** EXPOSED TO PYTHON *****
/************************************************************************************************/
/* MOD_COMPONENT_CONTROL                   08/02/07                            MRGENIE          */
/*                                                                                              */
/* Return true/false from                                                                       */
/************************************************************************************************/
	bool getDefineBOOL( const char * szName ) const;
/************************************************************************************************/
/* MOD_COMPONENT_CONTROL                   END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* Mod Globals    Start                          09/13/10                           phungus420  */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	bool isDCM_BATTLE_EFFECTS();
	int getBATTLE_EFFECT_LESS_FOOD();
	int getBATTLE_EFFECT_LESS_PRODUCTION();
	int getBATTLE_EFFECT_LESS_COMMERCE();
	int getBATTLE_EFFECTS_MINIMUM_TURN_INCREMENTS();
	int getMAX_BATTLE_TURNS();

	bool isDCM_AIR_BOMBING();
	bool isDCM_RANGE_BOMBARD();
	int getDCM_RB_CITY_INACCURACY();
	int getDCM_RB_CITYBOMBARD_CHANCE();
	bool isDCM_ATTACK_SUPPORT();
	bool isDCM_STACK_ATTACK();
	bool isDCM_OPP_FIRE();
	bool isDCM_ACTIVE_DEFENSE();
	bool isDCM_ARCHER_BOMBARD();
	bool isDCM_FIGHTER_ENGAGE();

	bool isDYNAMIC_CIV_NAMES();

	bool isLIMITED_RELIGIONS_EXCEPTIONS();
	bool isOC_RESPAWN_HOLY_CITIES();

	bool isIDW_ENABLED();
	float getIDW_BASE_COMBAT_INFLUENCE();
	float getIDW_NO_CITY_DEFENDER_MULTIPLIER();
	float getIDW_FORT_CAPTURE_MULTIPLIER();
	float getIDW_EXPERIENCE_FACTOR();
	float getIDW_WARLORD_MULTIPLIER();
	int getIDW_INFLUENCE_RADIUS();
	float getIDW_PLOT_DISTANCE_FACTOR();
	float getIDW_WINNER_PLOT_MULTIPLIER();
	float getIDW_LOSER_PLOT_MULTIPLIER();
	bool isIDW_EMERGENCY_DRAFT_ENABLED();
	int getIDW_EMERGENCY_DRAFT_MIN_POPULATION();
	float getIDW_EMERGENCY_DRAFT_STRENGTH();
	float getIDW_EMERGENCY_DRAFT_ANGER_MULTIPLIER();
	bool isIDW_NO_BARBARIAN_INFLUENCE();
	bool isIDW_NO_NAVAL_INFLUENCE();
	bool isIDW_PILLAGE_INFLUENCE_ENABLED();
	float getIDW_BASE_PILLAGE_INFLUENCE();
	float getIDW_CITY_TILE_MULTIPLIER();

	bool isSS_ENABLED();
	bool isSS_BRIBE();
	bool isSS_ASSASSINATE();
/************************************************************************************************/
/* Mod Globals                        END                                           phungus420  */
/************************************************************************************************/
	int getDefineINT( const char * szName ) const;
	float getDefineFLOAT( const char * szName ) const;
	const char * getDefineSTRING( const char * szName ) const;
/************************************************************************************************/
/* Afforess	                  Start		 08/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	void setDefineINT( const char * szName, int iValue, bool bUpdate = true);
	void setDefineFLOAT( const char * szName, float fValue, bool bUpdate = true );
	void setDefineSTRING( const char * szName, const char * szValue, bool bUpdate = true );
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


	int getMOVE_DENOMINATOR();
	int getNUM_UNIT_PREREQ_OR_BONUSES();
	int getNUM_BUILDING_PREREQ_OR_BONUSES();
	int getFOOD_CONSUMPTION_PER_POPULATION();
	int getMAX_HIT_POINTS();
	int getPATH_DAMAGE_WEIGHT();
	int getHILLS_EXTRA_DEFENSE();
	int getRIVER_ATTACK_MODIFIER();
	int getAMPHIB_ATTACK_MODIFIER();
	int getHILLS_EXTRA_MOVEMENT();
	int getMAX_PLOT_LIST_ROWS();
	int getUNIT_MULTISELECT_MAX();
	int getPERCENT_ANGER_DIVISOR();
	int getEVENT_MESSAGE_TIME();
	int getROUTE_FEATURE_GROWTH_MODIFIER();
	int getFEATURE_GROWTH_MODIFIER();
	int getMIN_CITY_RANGE();
	int getCITY_MAX_NUM_BUILDINGS();
	int getNUM_UNIT_AND_TECH_PREREQS();
	int getNUM_AND_TECH_PREREQS();
	int getNUM_OR_TECH_PREREQS();
	int getLAKE_MAX_AREA_SIZE();
	int getNUM_ROUTE_PREREQ_OR_BONUSES();
	int getNUM_BUILDING_AND_TECH_PREREQS();
	int getMIN_WATER_SIZE_FOR_OCEAN();
	int getFORTIFY_MODIFIER_PER_TURN();
	int getMAX_CITY_DEFENSE_DAMAGE();
	int getNUM_CORPORATION_PREREQ_BONUSES();
	int getPEAK_SEE_THROUGH_CHANGE();
	int getHILLS_SEE_THROUGH_CHANGE();
	int getSEAWATER_SEE_FROM_CHANGE();
	int getPEAK_SEE_FROM_CHANGE();
	int getHILLS_SEE_FROM_CHANGE();
	int getUSE_SPIES_NO_ENTER_BORDERS();

	float getCAMERA_MIN_YAW();
	float getCAMERA_MAX_YAW();
	float getCAMERA_FAR_CLIP_Z_HEIGHT();
	float getCAMERA_MAX_TRAVEL_DISTANCE();
	float getCAMERA_START_DISTANCE();
	float getAIR_BOMB_HEIGHT();
	float getPLOT_SIZE();
	float getCAMERA_SPECIAL_PITCH();
	float getCAMERA_MAX_TURN_OFFSET();
	float getCAMERA_MIN_DISTANCE();
	float getCAMERA_UPPER_PITCH();
	float getCAMERA_LOWER_PITCH();
	float getFIELD_OF_VIEW();
	float getSHADOW_SCALE();
	 float getUNIT_MULTISELECT_DISTANCE();

	int getUSE_CANNOT_FOUND_CITY_CALLBACK();
	int getUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK();
	int getUSE_IS_PLAYER_RESEARCH_CALLBACK();
	int getUSE_CAN_RESEARCH_CALLBACK();
	int getUSE_CANNOT_DO_CIVIC_CALLBACK();
	int getUSE_CAN_DO_CIVIC_CALLBACK();
	int getUSE_CANNOT_CONSTRUCT_CALLBACK();
	int getUSE_CAN_CONSTRUCT_CALLBACK();
	int getUSE_CAN_DECLARE_WAR_CALLBACK();
	int getUSE_CANNOT_RESEARCH_CALLBACK();
	int getUSE_GET_UNIT_COST_MOD_CALLBACK();
	int getUSE_GET_BUILDING_COST_MOD_CALLBACK();
	int getUSE_GET_CITY_FOUND_VALUE_CALLBACK();
	int getUSE_CANNOT_HANDLE_ACTION_CALLBACK();
	int getUSE_CAN_TRAIN_CALLBACK();
	int getUSE_CANNOT_TRAIN_CALLBACK();
	int getUSE_CAN_BUILD_CALLBACK();
	int getUSE_CAN_TRAIN_CALLBACK(UnitTypes eUnit);
	int getUSE_CANNOT_TRAIN_CALLBACK(UnitTypes eUnit);
	int getUSE_CAN_BUILD_CALLBACK(BuildTypes eBuild);
	int getUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK();
	int getUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK();
	int getUSE_FINISH_TEXT_CALLBACK();
	int getUSE_ON_UNIT_SET_XY_CALLBACK();
	int getUSE_ON_UNIT_SELECTED_CALLBACK();
	int getUSE_ON_UPDATE_CALLBACK();
	int getUSE_ON_UNIT_CREATED_CALLBACK();
	int getUSE_ON_UNIT_LOST_CALLBACK();
/************************************************************************************************/
/* MODULES                                 11/13/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	int getTGA_RELIGIONS();								// GAMEFONT
	int getTGA_CORPORATIONS();
/************************************************************************************************/
/* MODULES                                 END                                                  */
/************************************************************************************************/
	int getMAX_CIV_PLAYERS();
	int getMAX_PLAYERS();
	int getMAX_CIV_TEAMS();
	int getMAX_TEAMS();
	int getBARBARIAN_PLAYER();
	int getBARBARIAN_TEAM();
	int getINVALID_PLOT_COORD();
	int getNUM_CITY_PLOTS();
	int getCITY_HOME_PLOT();

	// ***** END EXPOSED TO PYTHON *****

	////////////// END DEFINES //////////////////

#ifdef _USRDLL
	CvDLLUtilityIFaceBase* getDLLIFace() { return g_DLL; }		// inlined for perf reasons, do not use outside of dll
#endif
	CvDLLUtilityIFaceBase* getDLLIFaceNonInl();
	void setDLLProfiler(FProfiler* prof);
	FProfiler* getDLLProfiler();
	void enableDLLProfiler(bool bEnable);
	bool isDLLProfilerEnabled() const;

	bool IsGraphicsInitialized() const;
	void SetGraphicsInitialized(bool bVal);

	// for caching
	bool readBuildingInfoArray(FDataStreamBase* pStream);
	void writeBuildingInfoArray(FDataStreamBase* pStream);

	bool readTechInfoArray(FDataStreamBase* pStream);
	void writeTechInfoArray(FDataStreamBase* pStream);

	bool readUnitInfoArray(FDataStreamBase* pStream);
	void writeUnitInfoArray(FDataStreamBase* pStream);

	bool readLeaderHeadInfoArray(FDataStreamBase* pStream);
	void writeLeaderHeadInfoArray(FDataStreamBase* pStream);

	bool readCivilizationInfoArray(FDataStreamBase* pStream);
	void writeCivilizationInfoArray(FDataStreamBase* pStream);

	bool readPromotionInfoArray(FDataStreamBase* pStream);
	void writePromotionInfoArray(FDataStreamBase* pStream);

	bool readDiplomacyInfoArray(FDataStreamBase* pStream);
	void writeDiplomacyInfoArray(FDataStreamBase* pStream);

	bool readCivicInfoArray(FDataStreamBase* pStream);
	void writeCivicInfoArray(FDataStreamBase* pStream);

	bool readHandicapInfoArray(FDataStreamBase* pStream);
	void writeHandicapInfoArray(FDataStreamBase* pStream);

	bool readBonusInfoArray(FDataStreamBase* pStream);
	void writeBonusInfoArray(FDataStreamBase* pStream);

	bool readImprovementInfoArray(FDataStreamBase* pStream);
	void writeImprovementInfoArray(FDataStreamBase* pStream);

	bool readEventInfoArray(FDataStreamBase* pStream);
	void writeEventInfoArray(FDataStreamBase* pStream);

	bool readEventTriggerInfoArray(FDataStreamBase* pStream);
	void writeEventTriggerInfoArray(FDataStreamBase* pStream);

	//
	// additional accessors for initting globals
	//

	void setInterface(CvInterface* pVal);
	 void setDiplomacyScreen(CvDiplomacyScreen* pVal);
	 void setMPDiplomacyScreen(CMPDiplomacyScreen* pVal);
	 void setMessageQueue(CMessageQueue* pVal);
	 void setHotJoinMessageQueue(CMessageQueue* pVal);
	 void setMessageControl(CMessageControl* pVal);
	 void setSetupData(CvSetupData* pVal);
	 void setMessageCodeTranslator(CvMessageCodeTranslator* pVal);
	 void setDropMgr(CvDropMgr* pVal);
	 void setPortal(CvPortal* pVal);
	 void setStatsReport(CvStatsReporter* pVal);
	 void setPathFinder(FAStar* pVal);
	 void setInterfacePathFinder(FAStar* pVal);
	 void setStepFinder(FAStar* pVal);
	 void setRouteFinder(FAStar* pVal);
	 void setBorderFinder(FAStar* pVal);
	 void setAreaFinder(FAStar* pVal);
	 void setPlotGroupFinder(FAStar* pVal);

	// So that CvEnums are moddable in the DLL
	 int getNumDirections() const;
	 int getNumGameOptions() const;
	 int getNumMPOptions() const;
	 int getNumSpecialOptions() const;
	 int getNumGraphicOptions() const;
	 int getNumTradeableItems() const;
	 int getNumBasicItems() const;
	 int getNumTradeableHeadings() const;
	 int getNumCommandInfos() const;
	 int getNumControlInfos() const;
	 int getNumMissionInfos() const;
	 int getNumPlayerOptionInfos() const;
	 int getMaxNumSymbols() const;
	 int getNumGraphicLevels() const;
	 int getNumGlobeLayers() const;

// BUG - DLL Info - start
	bool isBull() const;
	int getBullApiVersion() const;

	const wchar* getBullName() const;
	const wchar* getBullVersion() const;
// BUG - DLL Info - end

// BUG - BUG Info - start
	void setIsBug(bool bIsBug);
// BUG - BUG Info - end

// BUFFY - DLL Info - start
#ifdef _BUFFY
	bool isBuffy() const;
	int getBuffyApiVersion() const;

	const wchar* getBuffyName() const;
	const wchar* getBuffyVersion() const;
#endif
// BUFFY - DLL Info - end

	unsigned int getAssetCheckSum();

	void deleteInfoArrays();
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 11/30/07                                MRGENIE      */
/*                                                                                              */
/* Savegame compatibility                                                                       */
/************************************************************************************************/
	void doResetInfoClasses(int iNumSaveGameVector, std::vector<CvString> m_aszSaveGameVector);
	void StoreExeSettings();
	void LoadExeSettings();
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/

	/**** Dexy - Dark Ages START ****/
	const wchar* getRankingTextKeyWide(RankingTypes eRanking) const;
	/**** Dexy - Dark Ages  END  ****/

protected:

	bool m_bGraphicsInitialized;
	bool m_bDLLProfiler;
	bool m_bLogging;
	bool m_bRandLogging;
	bool m_bSynchLogging;
	bool m_bOverwriteLogs;
	NiPoint3  m_pt3CameraDir;
	int m_iNewPlayers;

	CMainMenu* m_pkMainMenu;

	bool m_bZoomOut;
	bool m_bZoomIn;
	bool m_bLoadGameFromFile;

	FMPIManager * m_pFMPMgr;

	CvRandom* m_asyncRand;

	CvGameAI* m_game;

	CMessageQueue* m_messageQueue;
	CMessageQueue* m_hotJoinMsgQueue;
	CMessageControl* m_messageControl;
	CvSetupData* m_setupData;
	CvInitCore* m_iniInitCore;
	CvInitCore* m_loadedInitCore;
	CvInitCore* m_initCore;
	CvMessageCodeTranslator * m_messageCodes;
	CvDropMgr* m_dropMgr;
	CvPortal* m_portal;
	CvStatsReporter * m_statsReporter;
	CvInterface* m_interface;

	CvArtFileMgr* m_pArtFileMgr;

	CvMap* m_map;

	CvDiplomacyScreen* m_diplomacyScreen;
	CMPDiplomacyScreen* m_mpDiplomacyScreen;

	FAStar* m_pathFinder;
	FAStar* m_interfacePathFinder;
	FAStar* m_stepFinder;
	FAStar* m_routeFinder;
	FAStar* m_borderFinder;
	FAStar* m_areaFinder;
	FAStar* m_plotGroupFinder;

	NiPoint3 m_pt3Origin;

	int* m_aiPlotDirectionX;	// [NUM_DIRECTION_TYPES];
	int* m_aiPlotDirectionY;	// [NUM_DIRECTION_TYPES];
	int* m_aiPlotCardinalDirectionX;	// [NUM_CARDINALDIRECTION_TYPES];
	int* m_aiPlotCardinalDirectionY;	// [NUM_CARDINALDIRECTION_TYPES];
	int* m_aiCityPlotX;	// [NUM_CITY_PLOTS];
	int* m_aiCityPlotY;	// [NUM_CITY_PLOTS];
	int* m_aiCityPlotPriority;	// [NUM_CITY_PLOTS];
	int m_aaiXYCityPlot[CITY_PLOTS_DIAMETER][CITY_PLOTS_DIAMETER];

	DirectionTypes* m_aeTurnLeftDirection;	// [NUM_DIRECTION_TYPES];
	DirectionTypes* m_aeTurnRightDirection;	// [NUM_DIRECTION_TYPES];
	DirectionTypes m_aaeXYDirection[DIRECTION_DIAMETER][DIRECTION_DIAMETER];

	//InterfaceModeInfo m_aInterfaceModeInfo[NUM_INTERFACEMODE_TYPES] =
	std::vector<CvInterfaceModeInfo*> m_paInterfaceModeInfo;

	/***********************************************************************************************************************
	Globals loaded from XML
	************************************************************************************************************************/

	// all type strings are upper case and are kept in this hash map for fast lookup, Moose
	typedef stdext::hash_map<std::string /* type string */, int /* info index */> InfosMap;
	InfosMap m_infosMap;
	std::vector<std::vector<CvInfoBase *> *> m_aInfoVectors;

	std::vector<CvColorInfo*> m_paColorInfo;
	std::vector<CvPlayerColorInfo*> m_paPlayerColorInfo;
	std::vector<CvAdvisorInfo*> m_paAdvisorInfo;
	std::vector<CvInfoBase*> m_paHints;
	std::vector<CvMainMenuInfo*> m_paMainMenus;
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 10/30/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	// Python Modular Loading
	std::vector<CvPythonModulesInfo*> m_paPythonModulesInfo;
	// MLF loading
	std::vector<CvString> m_paModLoadControlVector;
	std::vector<CvModLoadControlInfo*> m_paModLoadControls;
	int m_iTotalNumModules;
	// Modular loading Dependencies
	bool m_bAnyDependency;
	bool m_bTypeDependency;
	std::vector<CvString> m_paszAndDependencyTypes;
	std::vector<CvString> m_paszOrDependencyTypes;
	
	bool m_bForceOverwrite;
	bool m_bForceDelete;
	int m_iForceInsertLocation;
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
	std::vector<CvTerrainInfo*> m_paTerrainInfo;
	std::vector<CvLandscapeInfo*> m_paLandscapeInfo;
	int m_iActiveLandscapeID;
	std::vector<CvWorldInfo*> m_paWorldInfo;
	std::vector<CvClimateInfo*> m_paClimateInfo;
	std::vector<CvSeaLevelInfo*> m_paSeaLevelInfo;
	std::vector<CvYieldInfo*> m_paYieldInfo;
	std::vector<CvCommerceInfo*> m_paCommerceInfo;
	std::vector<CvRouteInfo*> m_paRouteInfo;
	std::vector<CvFeatureInfo*> m_paFeatureInfo;
	std::vector<CvBonusClassInfo*> m_paBonusClassInfo;
	std::vector<CvBonusInfo*> m_paBonusInfo;
	std::vector<CvImprovementInfo*> m_paImprovementInfo;
	std::vector<CvGoodyInfo*> m_paGoodyInfo;
	std::vector<CvBuildInfo*> m_paBuildInfo;
	std::vector<CvHandicapInfo*> m_paHandicapInfo;
	std::vector<CvGameSpeedInfo*> m_paGameSpeedInfo;
	std::vector<CvTurnTimerInfo*> m_paTurnTimerInfo;
	std::vector<CvCivilizationInfo*> m_paCivilizationInfo;
	int m_iNumPlayableCivilizationInfos;
	int m_iNumAIPlayableCivilizationInfos;
	std::vector<CvLeaderHeadInfo*> m_paLeaderHeadInfo;
	std::vector<CvTraitInfo*> m_paTraitInfo;
	std::vector<CvCursorInfo*> m_paCursorInfo;
	std::vector<CvThroneRoomCamera*> m_paThroneRoomCamera;
	std::vector<CvThroneRoomInfo*> m_paThroneRoomInfo;
	std::vector<CvThroneRoomStyleInfo*> m_paThroneRoomStyleInfo;
	std::vector<CvSlideShowInfo*> m_paSlideShowInfo;
	std::vector<CvSlideShowRandomInfo*> m_paSlideShowRandomInfo;
	std::vector<CvWorldPickerInfo*> m_paWorldPickerInfo;
	std::vector<CvSpaceShipInfo*> m_paSpaceShipInfo;
	std::vector<CvProcessInfo*> m_paProcessInfo;
	std::vector<CvVoteInfo*> m_paVoteInfo;
	std::vector<CvProjectInfo*> m_paProjectInfo;
	std::vector<CvBuildingClassInfo*> m_paBuildingClassInfo;
	std::vector<CvBuildingInfo*> m_paBuildingInfo;
	std::vector<CvSpecialBuildingInfo*> m_paSpecialBuildingInfo;
	std::vector<CvUnitClassInfo*> m_paUnitClassInfo;
	std::vector<CvUnitInfo*> m_paUnitInfo;
	std::vector<CvSpawnInfo*> m_paSpawnInfo;
	std::vector<CvSpecialUnitInfo*> m_paSpecialUnitInfo;
	std::vector<CvInfoBase*> m_paConceptInfo;
	std::vector<CvInfoBase*> m_paNewConceptInfo;
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - DCM: Pedia Concepts START
	std::vector<CvInfoBase*> m_paDCMConceptInfo;
	// Dale - DCM: Pedia Concepts END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/*Afforess                                     11/13/09                                         */
/************************************************************************************************/
	std::vector<CvInfoBase*> m_paANDConceptInfo;
/************************************************************************************************/
/* Afforess                                END                                                  */
/************************************************************************************************/
	std::vector<CvInfoBase*> m_paCityTabInfo;
	std::vector<CvInfoBase*> m_paCalendarInfo;
	std::vector<CvInfoBase*> m_paSeasonInfo;
	std::vector<CvInfoBase*> m_paMonthInfo;
	std::vector<CvInfoBase*> m_paDenialInfo;
	std::vector<CvInfoBase*> m_paInvisibleInfo;
	std::vector<CvVoteSourceInfo*> m_paVoteSourceInfo;
	std::vector<CvInfoBase*> m_paUnitCombatInfo;
	std::vector<CvInfoBase*> m_paDomainInfo;
	std::vector<CvInfoBase*> m_paUnitAIInfos;
	std::vector<CvInfoBase*> m_paAttitudeInfos;
	std::vector<CvInfoBase*> m_paMemoryInfos;
	std::vector<CvInfoBase*> m_paFeatInfos;
	std::vector<CvGameOptionInfo*> m_paGameOptionInfos;
	std::vector<CvMPOptionInfo*> m_paMPOptionInfos;
	std::vector<CvForceControlInfo*> m_paForceControlInfos;
	std::vector<CvPlayerOptionInfo*> m_paPlayerOptionInfos;
	std::vector<CvGraphicOptionInfo*> m_paGraphicOptionInfos;
	std::vector<CvSpecialistInfo*> m_paSpecialistInfo;
	std::vector<CvEmphasizeInfo*> m_paEmphasizeInfo;
	std::vector<CvUpkeepInfo*> m_paUpkeepInfo;
	std::vector<CvCultureLevelInfo*> m_paCultureLevelInfo;
	std::vector<CvReligionInfo*> m_paReligionInfo;
	std::vector<CvCorporationInfo*> m_paCorporationInfo;
	std::vector<CvActionInfo*> m_paActionInfo;
	std::vector<CvMissionInfo*> m_paMissionInfo;
	std::vector<CvControlInfo*> m_paControlInfo;
	std::vector<CvCommandInfo*> m_paCommandInfo;
	std::vector<CvAutomateInfo*> m_paAutomateInfo;
	std::vector<CvPromotionInfo*> m_paPromotionInfo;
	std::vector<CvTechInfo*> m_paTechInfo;
	std::vector<CvCivicOptionInfo*> m_paCivicOptionInfo;
	std::vector<CvCivicInfo*> m_paCivicInfo;
	std::vector<CvDiplomacyInfo*> m_paDiplomacyInfo;
	std::vector<CvEraInfo*> m_aEraInfo;	// [NUM_ERA_TYPES];
	std::vector<CvHurryInfo*> m_paHurryInfo;
	std::vector<CvVictoryInfo*> m_paVictoryInfo;
	std::vector<CvRouteModelInfo*> m_paRouteModelInfo;
	std::vector<CvRiverInfo*> m_paRiverInfo;
	std::vector<CvRiverModelInfo*> m_paRiverModelInfo;
	std::vector<CvWaterPlaneInfo*> m_paWaterPlaneInfo;
	std::vector<CvTerrainPlaneInfo*> m_paTerrainPlaneInfo;
	std::vector<CvCameraOverlayInfo*> m_paCameraOverlayInfo;
	std::vector<CvAnimationPathInfo*> m_paAnimationPathInfo;
	std::vector<CvAnimationCategoryInfo*> m_paAnimationCategoryInfo;
	std::vector<CvEntityEventInfo*> m_paEntityEventInfo;
	std::vector<CvUnitFormationInfo*> m_paUnitFormationInfo;
	std::vector<CvEffectInfo*> m_paEffectInfo;
	std::vector<CvAttachableInfo*> m_paAttachableInfo;
	std::vector<CvCameraInfo*> m_paCameraInfo;
	std::vector<CvQuestInfo*> m_paQuestInfo;
	std::vector<CvTutorialInfo*> m_paTutorialInfo;
	std::vector<CvEventTriggerInfo*> m_paEventTriggerInfo;
	std::vector<CvEventInfo*> m_paEventInfo;
	std::vector<CvEspionageMissionInfo*> m_paEspionageMissionInfo;
    std::vector<CvUnitArtStyleTypeInfo*> m_paUnitArtStyleTypeInfo;
	std::vector<CvCulturalAgeInfo*> m_paCulturalAgeInfo;
	std::vector<CvPropertyInfo*> m_paPropertyInfo;
	std::vector<CvOutcomeInfo*> m_paOutcomeInfo;

	// Game Text
	std::vector<CvGameText*> m_paGameTextXML;

	//////////////////////////////////////////////////////////////////////////
	// GLOBAL TYPES
	//////////////////////////////////////////////////////////////////////////

	// all type strings are upper case and are kept in this hash map for fast lookup, Moose
	typedef stdext::hash_map<std::string /* type string */, int /*enum value */> TypesMap;
	TypesMap m_typesMap;

	// XXX These are duplicates and are kept for enumeration convenience - most could be removed, Moose
	CvString *m_paszEntityEventTypes2;
	CvString *m_paszEntityEventTypes;
	int m_iNumEntityEventTypes;

	CvString *m_paszAnimationOperatorTypes;
	int m_iNumAnimationOperatorTypes;

	CvString* m_paszFunctionTypes;

	CvString* m_paszFlavorTypes;
	int m_iNumFlavorTypes;

	CvString *m_paszArtStyleTypes;
	int m_iNumArtStyleTypes;

	CvString *m_paszCitySizeTypes;
	int m_iNumCitySizeTypes;

	CvString *m_paszContactTypes;

	CvString *m_paszDiplomacyPowerTypes;

	CvString *m_paszAutomateTypes;

	CvString *m_paszDirectionTypes;

	CvString *m_paszFootstepAudioTypes;
	int m_iNumFootstepAudioTypes;

	CvString *m_paszFootstepAudioTags;
	int m_iNumFootstepAudioTags;

	CvString m_szCurrentXMLFile;
	//////////////////////////////////////////////////////////////////////////
	// Formerly Global Defines
	//////////////////////////////////////////////////////////////////////////

	FVariableSystem* m_VarSystem;

/************************************************************************************************/
/* Mod Globals    Start                          09/13/10                           phungus420  */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	bool m_bDCM_BATTLE_EFFECTS;
	int m_iBATTLE_EFFECT_LESS_FOOD;
	int m_iBATTLE_EFFECT_LESS_PRODUCTION;
	int m_iBATTLE_EFFECT_LESS_COMMERCE;
	int m_iBATTLE_EFFECTS_MINIMUM_TURN_INCREMENTS;
	int m_iMAX_BATTLE_TURNS;

	bool m_bDCM_AIR_BOMBING;
	bool m_bDCM_RANGE_BOMBARD;
	int m_iDCM_RB_CITY_INACCURACY;
	int m_iDCM_RB_CITYBOMBARD_CHANCE;
	bool m_bDCM_ATTACK_SUPPORT;
	bool m_bDCM_STACK_ATTACK;
	bool m_bDCM_OPP_FIRE;
	bool m_bDCM_ACTIVE_DEFENSE;
	bool m_bDCM_ARCHER_BOMBARD;
	bool m_bDCM_FIGHTER_ENGAGE;

	bool m_bDYNAMIC_CIV_NAMES;

	bool m_bLIMITED_RELIGIONS_EXCEPTIONS;
	bool m_bOC_RESPAWN_HOLY_CITIES;

	bool m_bIDW_ENABLED;
	float m_fIDW_BASE_COMBAT_INFLUENCE;
	float m_fIDW_NO_CITY_DEFENDER_MULTIPLIER;
	float m_fIDW_FORT_CAPTURE_MULTIPLIER;
	float m_fIDW_EXPERIENCE_FACTOR;
	float m_fIDW_WARLORD_MULTIPLIER;
	int m_iIDW_INFLUENCE_RADIUS;
	float m_fIDW_PLOT_DISTANCE_FACTOR;
	float m_fIDW_WINNER_PLOT_MULTIPLIER;
	float m_fIDW_LOSER_PLOT_MULTIPLIER;
	bool m_bIDW_EMERGENCY_DRAFT_ENABLED;
	int m_iIDW_EMERGENCY_DRAFT_MIN_POPULATION;
	float m_fIDW_EMERGENCY_DRAFT_STRENGTH;
	float m_fIDW_EMERGENCY_DRAFT_ANGER_MULTIPLIER;
	bool m_bIDW_NO_BARBARIAN_INFLUENCE;
	bool m_bIDW_NO_NAVAL_INFLUENCE;
	bool m_bIDW_PILLAGE_INFLUENCE_ENABLED;
	float m_fIDW_BASE_PILLAGE_INFLUENCE;
	float m_fIDW_CITY_TILE_MULTIPLIER;

	bool m_bSS_ENABLED;
	bool m_bSS_BRIBE;
	bool m_bSS_ASSASSINATE;
/************************************************************************************************/
/* Mod Globals                        END                                           phungus420  */
/************************************************************************************************/
	int m_iMOVE_DENOMINATOR;
	int m_iNUM_UNIT_PREREQ_OR_BONUSES;
	int m_iNUM_BUILDING_PREREQ_OR_BONUSES;
	int m_iFOOD_CONSUMPTION_PER_POPULATION;
	int m_iMAX_HIT_POINTS;
	int m_iPATH_DAMAGE_WEIGHT;
	int m_iHILLS_EXTRA_DEFENSE;
/************************************************************************************************/
/* Afforess	                		 12/21/09                                                   */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	int m_iPEAK_EXTRA_DEFENSE;
	int m_iPEAK_EXTRA_MOVEMENT;
	bool m_bFormationsMod;
	bool m_bXMLLogging;
	bool m_bLoadedPlayerOptions;
	
	int m_iSCORE_FREE_PERCENT;
	int m_iSCORE_POPULATION_FACTOR;
	int m_iSCORE_LAND_FACTOR;
	int m_iSCORE_TECH_FACTOR;
	int m_iSCORE_WONDER_FACTOR;
	
	int m_iUSE_CAN_CREATE_PROJECT_CALLBACK;
	int m_iUSE_CANNOT_CREATE_PROJECT_CALLBACK;
	int m_iUSE_CAN_DO_MELTDOWN_CALLBACK;
	int m_iUSE_CAN_MAINTAIN_PROCESS_CALLBACK;
	int m_iUSE_CANNOT_MAINTAIN_PROCESS_CALLBACK;
	int m_iUSE_CAN_DO_GROWTH_CALLBACK;
	int m_iUSE_CAN_DO_CULTURE_CALLBACK;
	int m_iUSE_CAN_DO_PLOT_CULTURE_CALLBACK;
	int m_iUSE_CAN_DO_PRODUCTION_CALLBACK;
	int m_iUSE_CAN_DO_RELIGION_CALLBACK;
	int m_iUSE_CAN_DO_GREATPEOPLE_CALLBACK;
	int m_iUSE_CAN_RAZE_CITY_CALLBACK;
	int m_iUSE_CAN_DO_GOLD_CALLBACK;
	int m_iUSE_CAN_DO_RESEARCH_CALLBACK;
	int m_iUSE_UPGRADE_UNIT_PRICE_CALLBACK;
	int m_iUSE_IS_VICTORY_CALLBACK;
	int m_iUSE_AI_UPDATE_UNIT_CALLBACK;
	int m_iUSE_AI_CHOOSE_PRODUCTION_CALLBACK;
	int m_iUSE_EXTRA_PLAYER_COSTS_CALLBACK;
	int m_iUSE_AI_DO_DIPLO_CALLBACK;
	int m_iUSE_AI_BESTTECH_CALLBACK;
	int m_iUSE_CAN_DO_COMBAT_CALLBACK;
	int m_iUSE_AI_CAN_DO_WARPLANS_CALLBACK;
/************************************************************************************************/
/* Afforess	Mountains End       END        		                                             */
/************************************************************************************************/
	int m_iRIVER_ATTACK_MODIFIER;
	int m_iAMPHIB_ATTACK_MODIFIER;
	int m_iHILLS_EXTRA_MOVEMENT;
	int m_iMAX_PLOT_LIST_ROWS;
	int m_iUNIT_MULTISELECT_MAX;
	int m_iPERCENT_ANGER_DIVISOR;
	int m_iEVENT_MESSAGE_TIME;
	int m_iROUTE_FEATURE_GROWTH_MODIFIER;
	int m_iFEATURE_GROWTH_MODIFIER;
	int m_iMIN_CITY_RANGE;
	int m_iCITY_MAX_NUM_BUILDINGS;
	int m_iNUM_UNIT_AND_TECH_PREREQS;
	int m_iNUM_AND_TECH_PREREQS;
	int m_iNUM_OR_TECH_PREREQS;
	int m_iLAKE_MAX_AREA_SIZE;
	int m_iNUM_ROUTE_PREREQ_OR_BONUSES;
	int m_iNUM_BUILDING_AND_TECH_PREREQS;
	int m_iMIN_WATER_SIZE_FOR_OCEAN;
	int m_iFORTIFY_MODIFIER_PER_TURN;
	int m_iMAX_CITY_DEFENSE_DAMAGE;
	int m_iNUM_CORPORATION_PREREQ_BONUSES;
	int m_iPEAK_SEE_THROUGH_CHANGE;
	int m_iHILLS_SEE_THROUGH_CHANGE;
	int m_iSEAWATER_SEE_FROM_CHANGE;
	int m_iPEAK_SEE_FROM_CHANGE;
	int m_iHILLS_SEE_FROM_CHANGE;
	int m_iUSE_SPIES_NO_ENTER_BORDERS;

	float m_fCAMERA_MIN_YAW;
	float m_fCAMERA_MAX_YAW;
	float m_fCAMERA_FAR_CLIP_Z_HEIGHT;
	float m_fCAMERA_MAX_TRAVEL_DISTANCE;
	float m_fCAMERA_START_DISTANCE;
	float m_fAIR_BOMB_HEIGHT;
	float m_fPLOT_SIZE;
	float m_fCAMERA_SPECIAL_PITCH;
	float m_fCAMERA_MAX_TURN_OFFSET;
	float m_fCAMERA_MIN_DISTANCE;
	float m_fCAMERA_UPPER_PITCH;
	float m_fCAMERA_LOWER_PITCH;
	float m_fFIELD_OF_VIEW;
	float m_fSHADOW_SCALE;
	float m_fUNIT_MULTISELECT_DISTANCE;

	int m_iUSE_CANNOT_FOUND_CITY_CALLBACK;
	int m_iUSE_CAN_FOUND_CITIES_ON_WATER_CALLBACK;
	int m_iUSE_IS_PLAYER_RESEARCH_CALLBACK;
	int m_iUSE_CAN_RESEARCH_CALLBACK;
	int m_iUSE_CANNOT_DO_CIVIC_CALLBACK;
	int m_iUSE_CAN_DO_CIVIC_CALLBACK;
	int m_iUSE_CANNOT_CONSTRUCT_CALLBACK;
	int m_iUSE_CAN_CONSTRUCT_CALLBACK;
	int m_iUSE_CAN_DECLARE_WAR_CALLBACK;
	int m_iUSE_CANNOT_RESEARCH_CALLBACK;
	int m_iUSE_GET_UNIT_COST_MOD_CALLBACK;
	int m_iUSE_GET_BUILDING_COST_MOD_CALLBACK;
	int m_iUSE_GET_CITY_FOUND_VALUE_CALLBACK;
	int m_iUSE_CANNOT_HANDLE_ACTION_CALLBACK;
	int m_iUSE_CAN_BUILD_CALLBACK;
	int m_iUSE_CANNOT_TRAIN_CALLBACK;
	int m_iUSE_CAN_TRAIN_CALLBACK;
	int m_iUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK;
	int m_iUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK;
	int m_iUSE_FINISH_TEXT_CALLBACK;
	int m_iUSE_ON_UNIT_SET_XY_CALLBACK;
	int m_iUSE_ON_UNIT_SELECTED_CALLBACK;
	int m_iUSE_ON_UPDATE_CALLBACK;
	int m_iUSE_ON_UNIT_CREATED_CALLBACK;
	int m_iUSE_ON_UNIT_LOST_CALLBACK;
/************************************************************************************************/
/* MODULES                                 11/13/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	int m_iTGA_RELIGIONS;
	int m_iTGA_CORPORATIONS;

/************************************************************************************************/
/* MODULES                                 END                                                  */
/************************************************************************************************/
	FProfiler* m_Profiler;		// profiler
	CvString m_szDllProfileText;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Efficiency, Options                                                                          */
/************************************************************************************************/
	//	Koshling - granular callback control
#ifdef GRANULAR_CALLBACK_CONTROL
public:
	mutable GranularCallbackController	m_pythonCallbackController;
#endif

public:
	int getDefineINT( const char * szName, const int iDefault ) const;
	
// BBAI Options
public:
	bool getBBAI_AIR_COMBAT();
	bool getBBAI_HUMAN_VASSAL_WAR_BUILD();
	int getBBAI_DEFENSIVE_PACT_BEHAVIOR();
	bool getBBAI_HUMAN_AS_VASSAL_OPTION();

protected:
	bool m_bBBAI_AIR_COMBAT;
	bool m_bBBAI_HUMAN_VASSAL_WAR_BUILD;
	int m_iBBAI_DEFENSIVE_PACT_BEHAVIOR;
	bool m_bBBAI_HUMAN_AS_VASSAL_OPTION;

// BBAI AI Variables
public:
	int getWAR_SUCCESS_CITY_CAPTURING();
	int getBBAI_ATTACK_CITY_STACK_RATIO();
	int getBBAI_SKIP_BOMBARD_BEST_ATTACK_ODDS();
	int getBBAI_SKIP_BOMBARD_BASE_STACK_RATIO();
	int getBBAI_SKIP_BOMBARD_MIN_STACK_RATIO();

protected:
	int m_iWAR_SUCCESS_CITY_CAPTURING;
	int m_iBBAI_ATTACK_CITY_STACK_RATIO;
	int m_iBBAI_SKIP_BOMBARD_BEST_ATTACK_ODDS;
	int m_iBBAI_SKIP_BOMBARD_BASE_STACK_RATIO;
	int m_iBBAI_SKIP_BOMBARD_MIN_STACK_RATIO;

// Tech Diffusion
public:
	bool getTECH_DIFFUSION_ENABLE();
	int getTECH_DIFFUSION_KNOWN_TEAM_MODIFIER();
	int getTECH_DIFFUSION_WELFARE_THRESHOLD();
	int getTECH_DIFFUSION_WELFARE_MODIFIER();
	int getTECH_COST_FIRST_KNOWN_PREREQ_MODIFIER();
	int getTECH_COST_KNOWN_PREREQ_MODIFIER();
	int getTECH_COST_MODIFIER();

protected:
	bool m_bTECH_DIFFUSION_ENABLE;
	int m_iTECH_DIFFUSION_KNOWN_TEAM_MODIFIER;
	int m_iTECH_DIFFUSION_WELFARE_THRESHOLD;
	int m_iTECH_DIFFUSION_WELFARE_MODIFIER;
	int m_iTECH_COST_FIRST_KNOWN_PREREQ_MODIFIER;
	int m_iTECH_COST_KNOWN_PREREQ_MODIFIER;
	int m_iTECH_COST_MODIFIER;
	
// From Lead From Behind by UncutDragon
public:
	bool getLFBEnable();
	int getLFBBasedOnGeneral();
	int getLFBBasedOnExperience();
	int getLFBBasedOnLimited();
	int getLFBBasedOnHealer();
	int getLFBBasedOnAverage();
	bool getLFBUseSlidingScale();
	int getLFBAdjustNumerator();
	int getLFBAdjustDenominator();
	bool getLFBUseCombatOdds();
	int getCOMBAT_DIE_SIDES();
	int getCOMBAT_DAMAGE();

protected:
	bool m_bLFBEnable;
	int m_iLFBBasedOnGeneral;
	int m_iLFBBasedOnExperience;
	int m_iLFBBasedOnLimited;
	int m_iLFBBasedOnHealer;
	int m_iLFBBasedOnAverage;
	bool m_bLFBUseSlidingScale;
	int	m_iLFBAdjustNumerator;
	int	m_iLFBAdjustDenominator;
	bool m_bLFBUseCombatOdds;
	int m_iCOMBAT_DIE_SIDES;
	int m_iCOMBAT_DAMAGE;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 11/30/07                                MRGENIE      */
/*                                                                                              */
/* Savegame compatibility                                                                       */
/************************************************************************************************/
	int* m_iStoreExeSettingsCommerceInfo;
	int* m_iStoreExeSettingsYieldInfo;
	int* m_iStoreExeSettingsReligionInfo;
	int* m_iStoreExeSettingsCorporationInfo;
	int* m_iStoreExeSettingsBonusInfo;
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
};

extern cvInternalGlobals* gGlobals;	// for debugging

class CvGlobals;

#ifdef _DEBUG
extern int inDLL;
extern const char* fnName;

class ProxyTracker
{
public:
	ProxyTracker(const CvGlobals* proxy, const char* name);
	~ProxyTracker();
};

#define PROXY_TRACK(x)	ProxyTracker tracker(this,x);

#else
#define	PROXY_TRACK(x)	;
#endif

//	cvGlobals is a proxy class with the same virtual interface as cvInternalGlobals
//	which just passes all requests onto the global internal instance.  This allows it to
//	be statically constructed so that IT's GetInstance() (as called by the Civ core engine
//	to retrieve a pointer to the DLLs virtual function table) can have a value to return prior
//	to any allocations takign pace (whicgh must be deferred until after SetDLLIFace() has been called
//	by the core engine to establish memorty allocators)
class CvGlobals
{
//	friend class CvDLLUtilityIFace;
	friend class CvXMLLoadUtility;
	friend class ProxyTracker;
protected:
	void CheckProxy(const char* fnName) const;

public:

	// singleton accessor
	DllExport inline static CvGlobals& getInstance();

	DllExport CvGlobals()
	{
	}
	DllExport virtual ~CvGlobals()
	{
	}

	DllExport void init()
	{
		PROXY_TRACK("init");	
		gGlobals->init();	
	}
	DllExport void uninit()
	{
		PROXY_TRACK("uninit");	
		gGlobals->uninit();	
	}

	DllExport void clearTypesMap()
	{
		PROXY_TRACK("clearTypesMap");	
		gGlobals->clearTypesMap();	
	}

	DllExport CvDiplomacyScreen* getDiplomacyScreen()
	{
		PROXY_TRACK("getDiplomacyScreen");	
		return gGlobals->getDiplomacyScreen();	
	}

	DllExport CMPDiplomacyScreen* getMPDiplomacyScreen()
	{
		PROXY_TRACK("getMPDiplomacyScreen");	
		return gGlobals->getMPDiplomacyScreen();	
	}

	DllExport FMPIManager*& getFMPMgrPtr()
	{
		PROXY_TRACK("getFMPMgrPtr");	
		return gGlobals->getFMPMgrPtr();	
	}
	DllExport CvPortal& getPortal()
	{
		PROXY_TRACK("getPortal");	
		return gGlobals->getPortal();	
	}
	DllExport CvSetupData& getSetupData()
	{
		PROXY_TRACK("getSetupData");	
		return gGlobals->getSetupData();	
	}
	DllExport CvInitCore& getInitCore()
	{
		PROXY_TRACK("getInitCore");	
		return gGlobals->getInitCore();	
	}
	DllExport CvInitCore& getLoadedInitCore()
	{
		PROXY_TRACK("getLoadedInitCore");	
		return gGlobals->getLoadedInitCore();	
	}
	DllExport CvInitCore& getIniInitCore()
	{
		PROXY_TRACK("getIniInitCore");	
		return gGlobals->getIniInitCore();	
	}
	DllExport CvMessageCodeTranslator& getMessageCodes()
	{
		PROXY_TRACK("getMessageCodes");	
		return gGlobals->getMessageCodes();	
	}
	DllExport CvStatsReporter& getStatsReporter()
	{
		PROXY_TRACK("getStatsReporter");	
		return gGlobals->getStatsReporter();	
	}
	DllExport CvStatsReporter* getStatsReporterPtr()
	{
		PROXY_TRACK("getStatsReporterPtr");	
		return gGlobals->getStatsReporterPtr();	
	}
	DllExport CvInterface& getInterface()
	{
		PROXY_TRACK("getInterface");	
		return gGlobals->getInterface();	
	}
	DllExport CvInterface* getInterfacePtr()
	{
		PROXY_TRACK("getInterfacePtr");	
		return gGlobals->getInterfacePtr();	
	}
	DllExport int getMaxCivPlayers() const
	{
		PROXY_TRACK("getMaxCivPlayers");	
		return gGlobals->getMaxCivPlayers();	
	}
	DllExport CvMap& getMap()
	{
		PROXY_TRACK("getMap");	
		return gGlobals->getMap();	
	}
	DllExport CvGameAI& getGame()
	{
		PROXY_TRACK("getGame");	
		return gGlobals->getGame();	
	}
	DllExport CvGameAI *getGamePointer()
	{
		PROXY_TRACK("getGamePointer");	
		return gGlobals->getGamePointer();	
	}
	DllExport CvRandom& getASyncRand()
	{
		PROXY_TRACK("getASyncRand");	
		return gGlobals->getASyncRand();	
	}
	DllExport CMessageQueue& getMessageQueue()
	{
		PROXY_TRACK("getMessageQueue");	
		return gGlobals->getMessageQueue();	
	}
	DllExport CMessageQueue& getHotMessageQueue()
	{
		PROXY_TRACK("getHotMessageQueue");	
		return gGlobals->getHotMessageQueue();	
	}
	DllExport CMessageControl& getMessageControl()
	{
		PROXY_TRACK("getMessageControl");	
		return gGlobals->getMessageControl();	
	}
	DllExport CvDropMgr& getDropMgr()
	{
		PROXY_TRACK("getDropMgr");	
		return gGlobals->getDropMgr();	
	}
	DllExport FAStar& getPathFinder()
	{
		PROXY_TRACK("getPathFinder");	
		return gGlobals->getPathFinder();	
	}
	DllExport FAStar& getInterfacePathFinder()
	{
		PROXY_TRACK("getInterfacePathFinder");	
		return gGlobals->getInterfacePathFinder();	
	}
	DllExport FAStar& getStepFinder()
	{
		PROXY_TRACK("getStepFinder");	
		return gGlobals->getStepFinder();	
	}
	DllExport FAStar& getRouteFinder()
	{
		PROXY_TRACK("getRouteFinder");	
		return gGlobals->getRouteFinder();	
	}
	DllExport FAStar& getBorderFinder()
	{
		PROXY_TRACK("getBorderFinder");	
		return gGlobals->getBorderFinder();	
	}
	DllExport FAStar& getAreaFinder()
	{
		PROXY_TRACK("getAreaFinder");	
		return gGlobals->getAreaFinder();	
	}
	DllExport FAStar& getPlotGroupFinder()
	{
		PROXY_TRACK("getPlotGroupFinder");	
		return gGlobals->getPlotGroupFinder();	
	}
	DllExport NiPoint3& getPt3Origin()
	{
		PROXY_TRACK("getPt3Origin");	
		return gGlobals->getPt3Origin();	
	}

	DllExport std::vector<CvInterfaceModeInfo*>& getInterfaceModeInfo()
	{
		PROXY_TRACK("getInterfaceModeInfo");	
		return gGlobals->getInterfaceModeInfo();	
	}
	DllExport CvInterfaceModeInfo& getInterfaceModeInfo(InterfaceModeTypes e)
	{
		PROXY_TRACK("getInterfaceModeInfo");	
		return gGlobals->getInterfaceModeInfo(e);	
	}

	DllExport NiPoint3& getPt3CameraDir()
	{
		PROXY_TRACK("getPt3CameraDir");	
		return gGlobals->getPt3CameraDir();	
	}

	DllExport bool& getLogging()
	{
		PROXY_TRACK("getLogging");	
		return gGlobals->getLogging();	
	}
	DllExport bool& getRandLogging()
	{
		PROXY_TRACK("getRandLogging");	
		return gGlobals->getRandLogging();	
	}
	DllExport bool& getSynchLogging()
	{
		PROXY_TRACK("getSynchLogging");	
		return gGlobals->getSynchLogging();	
	}
	DllExport bool& overwriteLogs()
	{
		PROXY_TRACK("overwriteLogs");	
		return gGlobals->overwriteLogs();	
	}

	DllExport int* getPlotDirectionX()
	{
		PROXY_TRACK("getPlotDirectionX");	
		return gGlobals->getPlotDirectionX();	
	}
	DllExport int* getPlotDirectionY()
	{
		PROXY_TRACK("getPlotDirectionY");	
		return gGlobals->getPlotDirectionY();	
	}
	DllExport int* getPlotCardinalDirectionX()
	{
		PROXY_TRACK("getPlotCardinalDirectionX");	
		return gGlobals->getPlotCardinalDirectionX();	
	}
	DllExport int* getPlotCardinalDirectionY()
	{
		PROXY_TRACK("getPlotCardinalDirectionY");	
		return gGlobals->getPlotCardinalDirectionY();	
	}
	DllExport int* getCityPlotX()
	{
		PROXY_TRACK("getCityPlotX");	
		return gGlobals->getCityPlotX();	
	}
	DllExport int* getCityPlotY()
	{
		PROXY_TRACK("getCityPlotY");	
		return gGlobals->getCityPlotY();	
	}
	DllExport int* getCityPlotPriority()
	{
		PROXY_TRACK("getCityPlotPriority");	
		return gGlobals->getCityPlotPriority();	
	}
	DllExport int getXYCityPlot(int i, int j)
	{
		PROXY_TRACK("getXYCityPlot");	
		return gGlobals->getXYCityPlot(i,j);	
	}
	DirectionTypes* getTurnLeftDirection()
	{
		PROXY_TRACK("getTurnLeftDirection");	
		return gGlobals->getTurnLeftDirection();	
	}
	DirectionTypes getTurnLeftDirection(int i)
	{
		PROXY_TRACK("getTurnLeftDirection(i)");	
		return gGlobals->getTurnLeftDirection(i);	
	}
	DirectionTypes* getTurnRightDirection()
	{
		PROXY_TRACK("getTurnRightDirection");	
		return gGlobals->getTurnRightDirection();	
	}
	DirectionTypes getTurnRightDirection(int i)
	{
		PROXY_TRACK("getTurnRightDirection(i)");	
		return gGlobals->getTurnRightDirection(i);	
	}
	DllExport DirectionTypes getXYDirection(int i, int j)
	{
		PROXY_TRACK("getXYDirection");	
		return gGlobals->getXYDirection(i,j);	
	}
	//
	// Global Infos
	// All info type strings are upper case and are kept in this hash map for fast lookup
	//
	DllExport int getInfoTypeForString(const char* szType, bool hideAssert = false) const			// returns the infos index, use this when searching for an info type string
	{
		PROXY_TRACK("getInfoTypeForString");	
		return gGlobals->getInfoTypeForString(szType, hideAssert);	
	}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 11/30/07                                MRGENIE      */
/*                                                                                              */
/* Savegame compatibility                                                                       */
/************************************************************************************************/
	DllExport void infoTypeFromStringReset()
	{
		PROXY_TRACK("infoTypeFromStringReset");	
		gGlobals->infoTypeFromStringReset();	
	}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/
	DllExport void addToInfosVectors(void *infoVector)
	{
		PROXY_TRACK("addToInfosVectors");	
		gGlobals->addToInfosVectors(infoVector);	
	}
	DllExport void infosReset()
	{
		PROXY_TRACK("infosReset");	
		gGlobals->infosReset();	
	}

	DllExport int getNumWorldInfos()
	{
		PROXY_TRACK("getNumWorldInfos");	
		return gGlobals->getNumWorldInfos();	
	}
	DllExport CvWorldInfo& getWorldInfo(WorldSizeTypes e)
	{
		PROXY_TRACK("getWorldInfo");	
		return gGlobals->getWorldInfo(e);	
	}

	DllExport int getNumClimateInfos()
	{
		PROXY_TRACK("getNumClimateInfos");	
		return gGlobals->getNumClimateInfos();	
	}
	DllExport CvClimateInfo& getClimateInfo(ClimateTypes e)
	{
		PROXY_TRACK("getClimateInfo");	
		return gGlobals->getClimateInfo(e);	
	}

	DllExport int getNumSeaLevelInfos()
	{
		PROXY_TRACK("getNumSeaLevelInfos");	
		return gGlobals->getNumSeaLevelInfos();	
	}
	DllExport CvSeaLevelInfo& getSeaLevelInfo(SeaLevelTypes e)
	{
		PROXY_TRACK("getSeaLevelInfo");	
		return gGlobals->getSeaLevelInfo(e);	
	}

	DllExport int getNumColorInfos()
	{
		PROXY_TRACK("getNumColorInfos");	
		return gGlobals->getNumColorInfos();	
	}
	DllExport CvColorInfo& getColorInfo(ColorTypes e)
	{
		PROXY_TRACK("getColorInfo");	
		return gGlobals->getColorInfo(e);	
	}

	DllExport int getNumPlayerColorInfos()
	{
		PROXY_TRACK("getNumPlayerColorInfos");	
		return gGlobals->getNumPlayerColorInfos();	
	}
	DllExport CvPlayerColorInfo& getPlayerColorInfo(PlayerColorTypes e)
	{
		PROXY_TRACK("getPlayerColorInfo");	
		return gGlobals->getPlayerColorInfo(e);	
	}

	DllExport  int getNumHints()
	{
		PROXY_TRACK("getNumHints");	
		return gGlobals->getNumHints();	
	}
	DllExport CvInfoBase& getHints(int i)
	{
		PROXY_TRACK("getHints");	
		return gGlobals->getHints(i);	
	}

	DllExport int getNumMainMenus()
	{
		PROXY_TRACK("getNumMainMenus");	
		return gGlobals->getNumMainMenus();	
	}
	DllExport CvMainMenuInfo& getMainMenus(int i)
	{
		PROXY_TRACK("getMainMenus");	
		return gGlobals->getMainMenus(i);	
	}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 10/30/07                            MRGENIE          */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	// Python Modular Loading
	DllExport int getNumPythonModulesInfos()
	{
		PROXY_TRACK("getNumPythonModulesInfos");	
		return gGlobals->getNumPythonModulesInfos();	
	}
	DllExport CvPythonModulesInfo& getPythonModulesInfo(int i)
	{
		PROXY_TRACK("getPythonModulesInfo");	
		return gGlobals->getPythonModulesInfo(i);	
	}
	DllExport CvModLoadControlInfo& getModLoadControlInfos(int i)
	{
		PROXY_TRACK("getModLoadControlInfos");	
		return gGlobals->getModLoadControlInfos(i);	
	}
/************************************************************************************************/
/* MODULAR_LOADING_CONTROL                 END                                                  */
/************************************************************************************************/

	DllExport int getNumRouteModelInfos()
	{
		PROXY_TRACK("getNumRouteModelInfos");	
		return gGlobals->getNumRouteModelInfos();	
	}
	DllExport CvRouteModelInfo& getRouteModelInfo(int i)
	{
		PROXY_TRACK("getRouteModelInfo");	
		return gGlobals->getRouteModelInfo(i);	
	}

	DllExport int getNumRiverInfos()
	{
		PROXY_TRACK("getNumRiverInfos");	
		return gGlobals->getNumRiverInfos();	
	}
	DllExport CvRiverInfo& getRiverInfo(RiverTypes e)
	{
		PROXY_TRACK("getRiverInfo");	
		return gGlobals->getRiverInfo(e);	
	}

	DllExport int getNumRiverModelInfos()
	{
		PROXY_TRACK("getNumRiverModelInfos");	
		return gGlobals->getNumRiverModelInfos();	
	}
	DllExport CvRiverModelInfo& getRiverModelInfo(int i)
	{
		PROXY_TRACK("getRiverModelInfo");	
		return gGlobals->getRiverModelInfo(i);	
	}

	DllExport int getNumWaterPlaneInfos()
	{
		PROXY_TRACK("getNumWaterPlaneInfos");	
		return gGlobals->getNumWaterPlaneInfos();	
	}
	DllExport CvWaterPlaneInfo& getWaterPlaneInfo(int i)
	{
		PROXY_TRACK("getWaterPlaneInfo");	
		return gGlobals->getWaterPlaneInfo(i);	
	}

	DllExport int getNumTerrainPlaneInfos()
	{
		PROXY_TRACK("getNumTerrainPlaneInfos");	
		return gGlobals->getNumTerrainPlaneInfos();	
	}
	DllExport CvTerrainPlaneInfo& getTerrainPlaneInfo(int i)
	{
		PROXY_TRACK("getTerrainPlaneInfo");	
		return gGlobals->getTerrainPlaneInfo(i);	
	}

	DllExport int getNumCameraOverlayInfos()
	{
		PROXY_TRACK("getNumCameraOverlayInfos");	
		return gGlobals->getNumCameraOverlayInfos();	
	}
	DllExport CvCameraOverlayInfo& getCameraOverlayInfo(int i)
	{
		PROXY_TRACK("getCameraOverlayInfo");	
		return gGlobals->getCameraOverlayInfo(i);	
	}

	DllExport int getNumAnimationPathInfos()
	{
		PROXY_TRACK("getNumAnimationPathInfos");	
		return gGlobals->getNumAnimationPathInfos();	
	}
	DllExport CvAnimationPathInfo& getAnimationPathInfo(AnimationPathTypes e)
	{
		PROXY_TRACK("getAnimationPathInfo");	
		return gGlobals->getAnimationPathInfo(e);	
	}

	DllExport int getNumAnimationCategoryInfos()
	{
		PROXY_TRACK("getNumAnimationCategoryInfos");	
		return gGlobals->getNumAnimationCategoryInfos();	
	}
	DllExport CvAnimationCategoryInfo& getAnimationCategoryInfo(AnimationCategoryTypes e)
	{
		PROXY_TRACK("getAnimationCategoryInfo");	
		return gGlobals->getAnimationCategoryInfo(e);	
	}

	DllExport int getNumEntityEventInfos()
	{
		PROXY_TRACK("getNumEntityEventInfos");	
		return gGlobals->getNumEntityEventInfos();	
	}
	DllExport CvEntityEventInfo& getEntityEventInfo(EntityEventTypes e)
	{
		PROXY_TRACK("getEntityEventInfo");	
		return gGlobals->getEntityEventInfo(e);	
	}

	DllExport int getNumEffectInfos()
	{
		PROXY_TRACK("getNumEffectInfos");	
		return gGlobals->getNumEffectInfos();	
	}
	DllExport CvEffectInfo& getEffectInfo(int i)
	{
		PROXY_TRACK("getEffectInfo");	
		return gGlobals->getEffectInfo(i);	
	}

	DllExport int getNumAttachableInfos()
	{
		PROXY_TRACK("getNumAttachableInfos");	
		return gGlobals->getNumAttachableInfos();	
	}
	DllExport CvAttachableInfo& getAttachableInfo(int i)
	{
		PROXY_TRACK("getAttachableInfo");	
		return gGlobals->getAttachableInfo(i);	
	}

	DllExport int getNumCameraInfos()
	{
		PROXY_TRACK("getNumCameraInfos");	
		return gGlobals->getNumCameraInfos();	
	}
	DllExport	CvCameraInfo& getCameraInfo(CameraAnimationTypes eCameraAnimationNum)
	{
		PROXY_TRACK("getCameraInfo");	
		return gGlobals->getCameraInfo(eCameraAnimationNum);	
	}

	DllExport int getNumUnitFormationInfos()
	{
		PROXY_TRACK("getNumUnitFormationInfos");	
		return gGlobals->getNumUnitFormationInfos();	
	}
	DllExport CvUnitFormationInfo& getUnitFormationInfo(int i)
	{
		PROXY_TRACK("getUnitFormationInfo");	
		return gGlobals->getUnitFormationInfo(i);	
	}

	DllExport int getNumLandscapeInfos()
	{
		PROXY_TRACK("getNumLandscapeInfos");	
		return gGlobals->getNumLandscapeInfos();	
	}
	DllExport CvLandscapeInfo& getLandscapeInfo(int iIndex)
	{
		PROXY_TRACK("getLandscapeInfo");	
		return gGlobals->getLandscapeInfo(iIndex);	
	}
	DllExport int getActiveLandscapeID()
	{
		PROXY_TRACK("getActiveLandscapeID");	
		return gGlobals->getActiveLandscapeID();	
	}
	DllExport void setActiveLandscapeID(int iLandscapeID)
	{
		PROXY_TRACK("setActiveLandscapeID");	
		return gGlobals->setActiveLandscapeID(iLandscapeID);	
	}

	DllExport int getNumTerrainInfos()
	{
		PROXY_TRACK("getNumTerrainInfos");	
		return gGlobals->getNumTerrainInfos();	
	}
	DllExport CvTerrainInfo& getTerrainInfo(TerrainTypes eTerrainNum)
	{
		PROXY_TRACK("getTerrainInfo");	
		return gGlobals->getTerrainInfo(eTerrainNum);	
	}

	DllExport int getNumBonusInfos()
	{
		PROXY_TRACK("getNumBonusInfos");	
		return gGlobals->getNumBonusInfos();	
	}
	DllExport CvBonusInfo& getBonusInfo(BonusTypes eBonusNum)
	{
		PROXY_TRACK("getBonusInfo");	
		return gGlobals->getBonusInfo(eBonusNum);	
	}

	DllExport int getNumFeatureInfos()
	{
		PROXY_TRACK("getNumFeatureInfos");	
		return gGlobals->getNumFeatureInfos();	
	}
	DllExport CvFeatureInfo& getFeatureInfo(FeatureTypes eFeatureNum)
	{
		PROXY_TRACK("getFeatureInfo");	
		return gGlobals->getFeatureInfo(eFeatureNum);	
	}

	DllExport int& getNumPlayableCivilizationInfos()
	{
		PROXY_TRACK("getNumPlayableCivilizationInfos");	
		return gGlobals->getNumPlayableCivilizationInfos();	
	}
	DllExport int& getNumAIPlayableCivilizationInfos()
	{
		PROXY_TRACK("getNumAIPlayableCivilizationInfos");	
		return gGlobals->getNumAIPlayableCivilizationInfos();	
	}
	DllExport int getNumCivilizationInfos()
	{
		PROXY_TRACK("getNumCivilizationInfos");	
		return gGlobals->getNumCivilizationInfos();	
	}
	DllExport CvCivilizationInfo& getCivilizationInfo(CivilizationTypes eCivilizationNum)
	{
		PROXY_TRACK("getCivilizationInfo");	
		return gGlobals->getCivilizationInfo(eCivilizationNum);	
	}

	DllExport int getNumLeaderHeadInfos()
	{
		PROXY_TRACK("getNumLeaderHeadInfos");	
		return gGlobals->getNumLeaderHeadInfos();	
	}
	DllExport CvLeaderHeadInfo& getLeaderHeadInfo(LeaderHeadTypes eLeaderHeadNum)
	{
		PROXY_TRACK("getLeaderHeadInfo");	
		return gGlobals->getLeaderHeadInfo(eLeaderHeadNum);	
	}

	DllExport int getNumCursorInfos()
	{
		PROXY_TRACK("getNumCursorInfos");	
		return gGlobals->getNumCursorInfos();	
	}
	DllExport	CvCursorInfo& getCursorInfo(CursorTypes eCursorNum)
	{
		PROXY_TRACK("getCursorInfo");	
		return gGlobals->getCursorInfo(eCursorNum);	
	}

	DllExport int getNumThroneRoomCameras()
	{
		PROXY_TRACK("getNumThroneRoomCameras");	
		return gGlobals->getNumThroneRoomCameras();	
	}
	DllExport	CvThroneRoomCamera& getThroneRoomCamera(int iIndex)
	{
		PROXY_TRACK("getThroneRoomCamera");	
		return gGlobals->getThroneRoomCamera(iIndex);	
	}

	DllExport int getNumThroneRoomInfos()
	{
		PROXY_TRACK("getNumThroneRoomInfos");	
		return gGlobals->getNumThroneRoomInfos();	
	}
	DllExport	CvThroneRoomInfo& getThroneRoomInfo(int iIndex)
	{
		PROXY_TRACK("getThroneRoomInfo");	
		return gGlobals->getThroneRoomInfo(iIndex);	
	}

	DllExport int getNumThroneRoomStyleInfos()
	{
		PROXY_TRACK("getNumThroneRoomStyleInfos");	
		return gGlobals->getNumThroneRoomStyleInfos();	
	}
	std::vector<CvThroneRoomStyleInfo*>& getThroneRoomStyleInfo()
	{
		PROXY_TRACK("getThroneRoomStyleInfo");	
		return gGlobals->getThroneRoomStyleInfo();	
	}
	DllExport	CvThroneRoomStyleInfo& getThroneRoomStyleInfo(int iIndex)
	{
		PROXY_TRACK("getThroneRoomStyleInfo");	
		return gGlobals->getThroneRoomStyleInfo(iIndex);	
	}

	DllExport int getNumSlideShowInfos()
	{
		PROXY_TRACK("getNumSlideShowInfos");	
		return gGlobals->getNumSlideShowInfos();	
	}
	DllExport	CvSlideShowInfo& getSlideShowInfo(int iIndex)
	{
		PROXY_TRACK("getSlideShowInfo");	
		return gGlobals->getSlideShowInfo(iIndex);	
	}

	DllExport int getNumSlideShowRandomInfos()
	{
		PROXY_TRACK("getNumSlideShowRandomInfos");	
		return gGlobals->getNumSlideShowRandomInfos();	
	}
	DllExport	CvSlideShowRandomInfo& getSlideShowRandomInfo(int iIndex)
	{
		PROXY_TRACK("getSlideShowRandomInfo");	
		return gGlobals->getSlideShowRandomInfo(iIndex);	
	}

	DllExport int getNumWorldPickerInfos()
	{
		PROXY_TRACK("getNumWorldPickerInfos");	
		return gGlobals->getNumWorldPickerInfos();	
	}
	DllExport	CvWorldPickerInfo& getWorldPickerInfo(int iIndex)
	{
		PROXY_TRACK("getWorldPickerInfo");	
		return gGlobals->getWorldPickerInfo(iIndex);	
	}

	DllExport int getNumSpaceShipInfos()
	{
		PROXY_TRACK("getNumSpaceShipInfos");	
		return gGlobals->getNumSpaceShipInfos();	
	}
	DllExport	CvSpaceShipInfo& getSpaceShipInfo(int iIndex)
	{
		PROXY_TRACK("getSpaceShipInfo");	
		return gGlobals->getSpaceShipInfo(iIndex);	
	}

/************************************************************************************************/
/*Afforess                                     12/21/09                                         */
/************************************************************************************************/
	DllExport int getNumANDConceptInfos()
	{
		PROXY_TRACK("getNumANDConceptInfos");	
		return gGlobals->getNumANDConceptInfos();	
	}
	DllExport CvInfoBase& getANDConceptInfo(ANDConceptTypes e)
	{
		PROXY_TRACK("getANDConceptInfo");	
		return gGlobals->getANDConceptInfo(e);	
	}
/************************************************************************************************/
/* Afforess                                END                                                  */
/************************************************************************************************/

	DllExport int getNumGameOptionInfos()
	{
		PROXY_TRACK("getNumGameOptionInfos");	
		return gGlobals->getNumGameOptionInfos();	
	}
	DllExport	CvGameOptionInfo& getGameOptionInfo(GameOptionTypes eGameOptionNum)
	{
		PROXY_TRACK("getGameOptionInfo");	
		return gGlobals->getGameOptionInfo(eGameOptionNum);	
	}

	DllExport int getNumMPOptionInfos()
	{
		PROXY_TRACK("getNumMPOptionInfos");	
		return gGlobals->getNumMPOptionInfos();	
	}
	DllExport	CvMPOptionInfo& getMPOptionInfo(MultiplayerOptionTypes eMPOptionNum)
	{
		PROXY_TRACK("getMPOptionInfo");	
		return gGlobals->getMPOptionInfo(eMPOptionNum);	
	}

	DllExport int getNumForceControlInfos()
	{
		PROXY_TRACK("getNumForceControlInfos");	
		return gGlobals->getNumForceControlInfos();	
	}
	DllExport	CvForceControlInfo& getForceControlInfo(ForceControlTypes eForceControlNum)
	{
		PROXY_TRACK("getForceControlInfo");	
		return gGlobals->getForceControlInfo(eForceControlNum);	
	}

	DllExport	CvPlayerOptionInfo& getPlayerOptionInfo(PlayerOptionTypes ePlayerOptionNum)
	{
		PROXY_TRACK("getPlayerOptionInfo");	
		return gGlobals->getPlayerOptionInfo(ePlayerOptionNum);	
	}

	DllExport	CvGraphicOptionInfo& getGraphicOptionInfo(GraphicOptionTypes eGraphicOptionNum)
	{
		PROXY_TRACK("getGraphicOptionInfo");	
		return gGlobals->getGraphicOptionInfo(eGraphicOptionNum);	
	}

	DllExport int getNumRouteInfos()
	{
		PROXY_TRACK("getNumRouteInfos");	
		return gGlobals->getNumRouteInfos();	
	}
	DllExport	CvRouteInfo& getRouteInfo(RouteTypes eRouteNum)
	{
		PROXY_TRACK("getRouteInfo");	
		return gGlobals->getRouteInfo(eRouteNum);	
	}

	DllExport int getNumImprovementInfos()
	{
		PROXY_TRACK("getNumImprovementInfos");	
		return gGlobals->getNumImprovementInfos();	
	}
	DllExport CvImprovementInfo& getImprovementInfo(ImprovementTypes eImprovementNum)
	{
		PROXY_TRACK("getImprovementInfo");	
		return gGlobals->getImprovementInfo(eImprovementNum);	
	}

	DllExport int getNumGoodyInfos()
	{
		PROXY_TRACK("getNumGoodyInfos");	
		return gGlobals->getNumGoodyInfos();	
	}
	DllExport CvGoodyInfo& getGoodyInfo(GoodyTypes eGoodyNum)
	{
		PROXY_TRACK("getGoodyInfo");	
		return gGlobals->getGoodyInfo(eGoodyNum);	
	}

	DllExport int getNumBuildInfos()
	{
		PROXY_TRACK("getNumBuildInfos");	
		return gGlobals->getNumBuildInfos();	
	}
	DllExport CvBuildInfo& getBuildInfo(BuildTypes eBuildNum)
	{
		PROXY_TRACK("getBuildInfo");	
		return gGlobals->getBuildInfo(eBuildNum);	
	}

	DllExport int getNumHandicapInfos()
	{
		PROXY_TRACK("getNumHandicapInfos");	
		return gGlobals->getNumHandicapInfos();	
	}
	DllExport CvHandicapInfo& getHandicapInfo(HandicapTypes eHandicapNum)
	{
		PROXY_TRACK("getHandicapInfo");	
		return gGlobals->getHandicapInfo(eHandicapNum);	
	}

	DllExport int getNumGameSpeedInfos()
	{
		PROXY_TRACK("getNumGameSpeedInfos");	
		return gGlobals->getNumGameSpeedInfos();	
	}
	DllExport CvGameSpeedInfo& getGameSpeedInfo(GameSpeedTypes eGameSpeedNum)
	{
		PROXY_TRACK("getGameSpeedInfo");	
		return gGlobals->getGameSpeedInfo(eGameSpeedNum);	
	}

	DllExport int getNumTurnTimerInfos()
	{
		PROXY_TRACK("getNumTurnTimerInfos");	
		return gGlobals->getNumTurnTimerInfos();	
	}
	DllExport CvTurnTimerInfo& getTurnTimerInfo(TurnTimerTypes eTurnTimerNum)
	{
		PROXY_TRACK("getTurnTimerInfo");	
		return gGlobals->getTurnTimerInfo(eTurnTimerNum);	
	}

	DllExport int getNumActionInfos()
	{
		PROXY_TRACK("getNumActionInfos");	
		return gGlobals->getNumActionInfos();	
	}
	DllExport CvActionInfo& getActionInfo(int i)
	{
		PROXY_TRACK("getActionInfo");	
		return gGlobals->getActionInfo(i);	
	}

	DllExport CvMissionInfo& getMissionInfo(MissionTypes eMissionNum)
	{
		PROXY_TRACK("getMissionInfo");	
		return gGlobals->getMissionInfo(eMissionNum);	
	}

	DllExport CvControlInfo& getControlInfo(ControlTypes eControlNum)
	{
		PROXY_TRACK("getControlInfo");	
		return gGlobals->getControlInfo(eControlNum);	
	}

	DllExport CvCommandInfo& getCommandInfo(CommandTypes eCommandNum)
	{
		PROXY_TRACK("getCommandInfo");	
		return gGlobals->getCommandInfo(eCommandNum);	
	}

	DllExport int getNumAutomateInfos()
	{
		PROXY_TRACK("getNumAutomateInfos");	
		return gGlobals->getNumAutomateInfos();	
	}
	DllExport CvAutomateInfo& getAutomateInfo(int iAutomateNum)
	{
		PROXY_TRACK("getAutomateInfo");	
		return gGlobals->getAutomateInfo(iAutomateNum);	
	}

	DllExport int getNumEraInfos()
	{
		PROXY_TRACK("getNumEraInfos");	
		return gGlobals->getNumEraInfos();	
	}
	DllExport CvEraInfo& getEraInfo(EraTypes eEraNum)
	{
		PROXY_TRACK("getEraInfo");	
		return gGlobals->getEraInfo(eEraNum);	
	}

	DllExport int getNumVictoryInfos()
	{
		PROXY_TRACK("getNumVictoryInfos");	
		return gGlobals->getNumVictoryInfos();	
	}
	DllExport CvVictoryInfo& getVictoryInfo(VictoryTypes eVictoryNum)
	{
		PROXY_TRACK("getVictoryInfo");	
		return gGlobals->getVictoryInfo(eVictoryNum);	
	}

	//
	// Global Types
	// All type strings are upper case and are kept in this hash map for fast lookup
	// The other functions are kept for convenience when enumerating, but most are not used
	//
	DllExport int getTypesEnum(const char* szType) const				// use this when searching for a type
	{
		PROXY_TRACK("getTypesEnum");	
		return gGlobals->getTypesEnum(szType);	
	}
	DllExport void setTypesEnum(const char* szType, int iEnum)
	{
		PROXY_TRACK("setTypesEnum");	
		gGlobals->setTypesEnum(szType, iEnum);	
	}

	DllExport int getNUM_ENGINE_DIRTY_BITS() const
	{
		PROXY_TRACK("getNUM_ENGINE_DIRTY_BITS");	
		return gGlobals->getNUM_ENGINE_DIRTY_BITS();	
	}
	DllExport int getNUM_INTERFACE_DIRTY_BITS() const
	{
		PROXY_TRACK("getNUM_INTERFACE_DIRTY_BITS");	
		return gGlobals->getNUM_INTERFACE_DIRTY_BITS();	
	}
	DllExport int getNUM_YIELD_TYPES() const
	{
		PROXY_TRACK("getNUM_YIELD_TYPES");	
		return gGlobals->getNUM_YIELD_TYPES();	
	}
	DllExport int getNUM_COMMERCE_TYPES() const
	{
		PROXY_TRACK("getNUM_COMMERCE_TYPES");	
		return gGlobals->getNUM_COMMERCE_TYPES();	
	}
	DllExport int getNUM_FORCECONTROL_TYPES() const
	{
		PROXY_TRACK("getNUM_FORCECONTROL_TYPES");	
		return gGlobals->getNUM_FORCECONTROL_TYPES();	
	}
	DllExport int getNUM_INFOBAR_TYPES() const
	{
		PROXY_TRACK("getNUM_INFOBAR_TYPES");	
		return gGlobals->getNUM_INFOBAR_TYPES();	
	}
	DllExport int getNUM_HEALTHBAR_TYPES() const
	{
		PROXY_TRACK("getNUM_HEALTHBAR_TYPES");	
		return gGlobals->getNUM_HEALTHBAR_TYPES();	
	}
	DllExport int getNUM_CONTROL_TYPES() const
	{
		PROXY_TRACK("getNUM_CONTROL_TYPES");	
		return gGlobals->getNUM_CONTROL_TYPES();	
	}
	DllExport int getNUM_LEADERANIM_TYPES() const
	{
		PROXY_TRACK("getNUM_LEADERANIM_TYPES");	
		return gGlobals->getNUM_LEADERANIM_TYPES();	
	}

	DllExport int& getNumEntityEventTypes()
	{
		PROXY_TRACK("getNumEntityEventTypes");	
		return gGlobals->getNumEntityEventTypes();	
	}
	DllExport CvString& getEntityEventTypes(EntityEventTypes e)
	{
		PROXY_TRACK("getEntityEventTypes");	
		return gGlobals->getEntityEventTypes(e);	
	}

	DllExport int& getNumAnimationOperatorTypes()
	{
		PROXY_TRACK("getNumAnimationOperatorTypes");	
		return gGlobals->getNumAnimationOperatorTypes();	
	}
	DllExport CvString& getAnimationOperatorTypes(AnimationOperatorTypes e)
	{
		PROXY_TRACK("getAnimationOperatorTypes");	
		return gGlobals->getAnimationOperatorTypes(e);	
	}

	DllExport CvString& getFunctionTypes(FunctionTypes e)
	{
		PROXY_TRACK("getFunctionTypes");	
		return gGlobals->getFunctionTypes(e);	
	}

	DllExport int& getNumArtStyleTypes()
	{
		PROXY_TRACK("getNumArtStyleTypes");	
		return gGlobals->getNumArtStyleTypes();	
	}
	DllExport CvString& getArtStyleTypes(ArtStyleTypes e)
	{
		PROXY_TRACK("getArtStyleTypes");	
		return gGlobals->getArtStyleTypes(e);	
	}

	DllExport CvString& getDirectionTypes(AutomateTypes e)
	{
		PROXY_TRACK("getDirectionTypes");	
		return gGlobals->getDirectionTypes(e);	
	}

	DllExport int& getNumFootstepAudioTypes()
	{
		PROXY_TRACK("getNumFootstepAudioTypes");	
		return gGlobals->getNumFootstepAudioTypes();	
	}
	DllExport CvString& getFootstepAudioTypes(int i)
	{
		PROXY_TRACK("getFootstepAudioTypes");	
		return gGlobals->getFootstepAudioTypes(i);	
	}
	DllExport int getFootstepAudioTypeByTag(CvString strTag)
	{
		PROXY_TRACK("getFootstepAudioTypeByTag");	
		return gGlobals->getFootstepAudioTypeByTag(strTag);	
	}

	DllExport CvString& getFootstepAudioTags(int i)
	{
		PROXY_TRACK("getFootstepAudioTags");	
		return gGlobals->getFootstepAudioTags(i);	
	}

	//
	///////////////// BEGIN global defines
	// THESE ARE READ-ONLY
	//

	DllExport FVariableSystem* getDefinesVarSystem()
	{
		PROXY_TRACK("getDefinesVarSystem");	
		return gGlobals->getDefinesVarSystem();	
	}
	DllExport void cacheGlobals()
	{
		PROXY_TRACK("cacheGlobals");	
		gGlobals->cacheGlobals();	
	}

	// ***** EXPOSED TO PYTHON *****
/************************************************************************************************/
/* MOD_COMPONENT_CONTROL                   08/02/07                            MRGENIE          */
/*                                                                                              */
/* Return true/false from                                                                       */
/************************************************************************************************/
	DllExport bool getDefineBOOL( const char * szName ) const
	{
		PROXY_TRACK("getDefineBOOL");	
		return gGlobals->getDefineBOOL(szName);	
	}
/************************************************************************************************/
/* MOD_COMPONENT_CONTROL                   END                                                  */
/************************************************************************************************/

	DllExport int getDefineINT( const char * szName ) const
	{
		PROXY_TRACK("getDefineINT");	
		return gGlobals->getDefineINT(szName);	
	}
	DllExport float getDefineFLOAT( const char * szName ) const
	{
		PROXY_TRACK("getDefineFLOAT");	
		return gGlobals->getDefineFLOAT(szName);	
	}
	DllExport const char * getDefineSTRING( const char * szName ) const
	{
		PROXY_TRACK("getDefineSTRING");	
		return gGlobals->getDefineSTRING(szName);	
	}
/************************************************************************************************/
/* Afforess	                  Start		 08/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	DllExport void setDefineINT( const char * szName, int iValue, bool bUpdate = true)
	{
		PROXY_TRACK("setDefineINT");	
		gGlobals->setDefineINT(szName, iValue, bUpdate);	
	}
	DllExport void setDefineFLOAT( const char * szName, float fValue, bool bUpdate = true )
	{
		PROXY_TRACK("setDefineFLOAT");	
		gGlobals->setDefineFLOAT(szName, fValue, bUpdate);	
	}
	DllExport void setDefineSTRING( const char * szName, const char * szValue, bool bUpdate = true )
	{
		PROXY_TRACK("setDefineSTRING");	
		gGlobals->setDefineSTRING(szName, szValue, bUpdate);	
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	DllExport int getMAX_PLOT_LIST_ROWS()
	{
		PROXY_TRACK("getMAX_PLOT_LIST_ROWS");	
		return gGlobals->getMAX_PLOT_LIST_ROWS();	
	}
	DllExport int getUNIT_MULTISELECT_MAX()
	{
		PROXY_TRACK("getUNIT_MULTISELECT_MAX");	
		return gGlobals->getUNIT_MULTISELECT_MAX();	
	}
	DllExport int getEVENT_MESSAGE_TIME()
	{
		PROXY_TRACK("getEVENT_MESSAGE_TIME");	
		return gGlobals->getEVENT_MESSAGE_TIME();	
	}

	DllExport float getCAMERA_MIN_YAW()
	{
		PROXY_TRACK("getCAMERA_MIN_YAW");	
		return gGlobals->getCAMERA_MIN_YAW();	
	}
	DllExport float getCAMERA_MAX_YAW()
	{
		PROXY_TRACK("getCAMERA_MAX_YAW");	
		return gGlobals->getCAMERA_MAX_YAW();	
	}
	DllExport float getCAMERA_FAR_CLIP_Z_HEIGHT()
	{
		PROXY_TRACK("getCAMERA_FAR_CLIP_Z_HEIGHT");	
		return gGlobals->getCAMERA_FAR_CLIP_Z_HEIGHT();	
	}
	DllExport float getCAMERA_MAX_TRAVEL_DISTANCE()
	{
		PROXY_TRACK("getCAMERA_MAX_TRAVEL_DISTANCE");	
		return gGlobals->getCAMERA_MAX_TRAVEL_DISTANCE();	
	}
	DllExport float getCAMERA_START_DISTANCE()
	{
		PROXY_TRACK("getCAMERA_START_DISTANCE");	
		return gGlobals->getCAMERA_START_DISTANCE();	
	}
	DllExport float getAIR_BOMB_HEIGHT()
	{
		PROXY_TRACK("getAIR_BOMB_HEIGHT");	
		return gGlobals->getAIR_BOMB_HEIGHT();	
	}
	DllExport float getPLOT_SIZE()
	{
		PROXY_TRACK("getPLOT_SIZE");	
		return gGlobals->getPLOT_SIZE();	
	}
	DllExport float getCAMERA_SPECIAL_PITCH()
	{
		PROXY_TRACK("getCAMERA_SPECIAL_PITCH");	
		return gGlobals->getCAMERA_SPECIAL_PITCH();	
	}
	DllExport float getCAMERA_MAX_TURN_OFFSET()
	{
		PROXY_TRACK("getCAMERA_MAX_TURN_OFFSET");	
		return gGlobals->getCAMERA_MAX_TURN_OFFSET();	
	}
	DllExport float getCAMERA_MIN_DISTANCE()
	{
		PROXY_TRACK("getCAMERA_MIN_DISTANCE");	
		return gGlobals->getCAMERA_MIN_DISTANCE();	
	}
	DllExport float getCAMERA_UPPER_PITCH()
	{
		PROXY_TRACK("getCAMERA_UPPER_PITCH");	
		return gGlobals->getCAMERA_UPPER_PITCH();	
	}
	DllExport float getCAMERA_LOWER_PITCH()
	{
		PROXY_TRACK("getCAMERA_LOWER_PITCH");	
		return gGlobals->getCAMERA_LOWER_PITCH();	
	}
	DllExport float getFIELD_OF_VIEW()
	{
		PROXY_TRACK("getFIELD_OF_VIEW");	
		return gGlobals->getFIELD_OF_VIEW();	
	}
	DllExport float getSHADOW_SCALE()
	{
		PROXY_TRACK("getSHADOW_SCALE");	
		return gGlobals->getSHADOW_SCALE();	
	}
	DllExport float getUNIT_MULTISELECT_DISTANCE()
	{
		PROXY_TRACK("getUNIT_MULTISELECT_DISTANCE");	
		return gGlobals->getUNIT_MULTISELECT_DISTANCE();	
	}

	DllExport int getUSE_FINISH_TEXT_CALLBACK()
	{
		PROXY_TRACK("getUSE_FINISH_TEXT_CALLBACK");	
		return gGlobals->getUSE_FINISH_TEXT_CALLBACK();	
	}

	DllExport int getMAX_CIV_PLAYERS()
	{
		PROXY_TRACK("getMAX_CIV_PLAYERS");	
		return gGlobals->getMAX_CIV_PLAYERS();	
	}
	DllExport int getMAX_PLAYERS()
	{
		PROXY_TRACK("getMAX_PLAYERS");	
		return gGlobals->getMAX_PLAYERS();	
	}
	DllExport int getMAX_CIV_TEAMS()
	{
		PROXY_TRACK("getMAX_CIV_TEAMS");	
		return gGlobals->getMAX_CIV_TEAMS();	
	}
	DllExport int getMAX_TEAMS()
	{
		PROXY_TRACK("getMAX_TEAMS");	
		return gGlobals->getMAX_TEAMS();	
	}
	DllExport int getBARBARIAN_PLAYER()
	{
		PROXY_TRACK("getBARBARIAN_PLAYER");	
		return gGlobals->getBARBARIAN_PLAYER();	
	}
	DllExport int getBARBARIAN_TEAM()
	{
		PROXY_TRACK("getBARBARIAN_TEAM");	
		return gGlobals->getBARBARIAN_TEAM();	
	}
	DllExport int getINVALID_PLOT_COORD()
	{
		PROXY_TRACK("getINVALID_PLOT_COORD");	
		return gGlobals->getINVALID_PLOT_COORD();	
	}
	DllExport int getNUM_CITY_PLOTS()
	{
		PROXY_TRACK("getNUM_CITY_PLOTS");	
		return gGlobals->getNUM_CITY_PLOTS();	
	}
	DllExport int getCITY_HOME_PLOT()
	{
		PROXY_TRACK("getCITY_HOME_PLOT");	
		return gGlobals->getCITY_HOME_PLOT();	
	}

	// ***** END EXPOSED TO PYTHON *****

	////////////// END DEFINES //////////////////

	DllExport void setDLLIFace(CvDLLUtilityIFaceBase* pDll)
	{
		if ( pDll != NULL )
		{
			g_DLL = pDll;

			OutputDebugString("setDLLIFace()\n");
			if (gGlobals == NULL)
			{
				OutputDebugString("Constructing internal globals\n");
				gGlobals = new cvInternalGlobals();
			}
		}
		else
		{
			delete gGlobals;

			g_DLL = NULL;
		}
	}

	DllExport CvDLLUtilityIFaceBase* getDLLIFaceNonInl()
	{
		//PROXY_TRACK("getDLLIFaceNonInl");	
		return gGlobals->getDLLIFaceNonInl();	
	}
	DllExport void setDLLProfiler(FProfiler* prof)
	{
		PROXY_TRACK("setDLLProfiler");	
		gGlobals->setDLLProfiler(prof);	
	}
	DllExport void enableDLLProfiler(bool bEnable)
	{
		PROXY_TRACK("enableDLLProfiler");	
		gGlobals->enableDLLProfiler(bEnable);	
	}

	DllExport bool IsGraphicsInitialized() const
	{
		PROXY_TRACK("IsGraphicsInitialized");	
		return gGlobals->IsGraphicsInitialized();	
	}
	DllExport void SetGraphicsInitialized(bool bVal)
	{
		PROXY_TRACK("SetGraphicsInitialized");	
		gGlobals->SetGraphicsInitialized(bVal);	
	}

	// for caching
	DllExport bool readBuildingInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("readBuildingInfoArray");	
		return gGlobals->readBuildingInfoArray(pStream);	
	}
	DllExport void writeBuildingInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("writeBuildingInfoArray");	
		gGlobals->writeBuildingInfoArray(pStream);	
	}

	DllExport bool readTechInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("readTechInfoArray");	
		return gGlobals->readTechInfoArray(pStream);	
	}
	DllExport void writeTechInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("writeTechInfoArray");	
		gGlobals->writeTechInfoArray(pStream);	
	}

	DllExport bool readUnitInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("readUnitInfoArray");	
		return gGlobals->readUnitInfoArray(pStream);	
	}
	DllExport void writeUnitInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("writeUnitInfoArray");	
		gGlobals->writeUnitInfoArray(pStream);	
	}

	DllExport bool readLeaderHeadInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("readLeaderHeadInfoArray");	
		return gGlobals->readLeaderHeadInfoArray(pStream);	
	}
	DllExport void writeLeaderHeadInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("writeLeaderHeadInfoArray");	
		gGlobals->writeLeaderHeadInfoArray(pStream);	
	}

	DllExport bool readCivilizationInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("readCivilizationInfoArray");	
		return gGlobals->readCivilizationInfoArray(pStream);	
	}
	DllExport void writeCivilizationInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("writeCivilizationInfoArray");	
		gGlobals->writeCivilizationInfoArray(pStream);	
	}

	DllExport bool readPromotionInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("readPromotionInfoArray");	
		return gGlobals->readPromotionInfoArray(pStream);	
	}
	DllExport void writePromotionInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("writePromotionInfoArray");	
		gGlobals->writePromotionInfoArray(pStream);	
	}

	DllExport bool readDiplomacyInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("readDiplomacyInfoArray");	
		return gGlobals->readDiplomacyInfoArray(pStream);	
	}
	DllExport void writeDiplomacyInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("writeDiplomacyInfoArray");	
		gGlobals->writeDiplomacyInfoArray(pStream);	
	}

	DllExport bool readCivicInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("readCivicInfoArray");	
		return gGlobals->readCivicInfoArray(pStream);	
	}
	DllExport void writeCivicInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("writeCivicInfoArray");	
		gGlobals->writeCivicInfoArray(pStream);	
	}

	DllExport bool readHandicapInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("readHandicapInfoArray");	
		return gGlobals->readHandicapInfoArray(pStream);	
	}
	DllExport void writeHandicapInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("writeHandicapInfoArray");	
		gGlobals->writeHandicapInfoArray(pStream);	
	}

	DllExport bool readBonusInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("readBonusInfoArray");	
		return gGlobals->readBonusInfoArray(pStream);	
	}
	DllExport void writeBonusInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("writeBonusInfoArray");	
		gGlobals->writeBonusInfoArray(pStream);	
	}

	DllExport bool readImprovementInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("readImprovementInfoArray");	
		return gGlobals->readImprovementInfoArray(pStream);	
	}
	DllExport void writeImprovementInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("writeImprovementInfoArray");	
		gGlobals->writeImprovementInfoArray(pStream);	
	}

	DllExport bool readEventInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("readEventInfoArray");	
		return gGlobals->readEventInfoArray(pStream);	
	}
	DllExport void writeEventInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("writeEventInfoArray");	
		gGlobals->writeEventInfoArray(pStream);	
	}

	DllExport bool readEventTriggerInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("readEventTriggerInfoArray");	
		return gGlobals->readEventTriggerInfoArray(pStream);	
	}
	DllExport void writeEventTriggerInfoArray(FDataStreamBase* pStream)
	{
		PROXY_TRACK("writeEventTriggerInfoArray");	
		gGlobals->writeEventTriggerInfoArray(pStream);	
	}

	//
	// additional accessors for initting globals
	//

	DllExport void setInterface(CvInterface* pVal)
	{
		PROXY_TRACK("setInterface");	
		gGlobals->setInterface(pVal);	
	}
	DllExport void setDiplomacyScreen(CvDiplomacyScreen* pVal)
	{
		PROXY_TRACK("setDiplomacyScreen");	
		gGlobals->setDiplomacyScreen(pVal);	
	}
	DllExport void setMPDiplomacyScreen(CMPDiplomacyScreen* pVal)
	{
		PROXY_TRACK("setMPDiplomacyScreen");	
		gGlobals->setMPDiplomacyScreen(pVal);	
	}
	DllExport void setMessageQueue(CMessageQueue* pVal)
	{
		PROXY_TRACK("setMessageQueue");	
		gGlobals->setMessageQueue(pVal);	
	}
	DllExport void setHotJoinMessageQueue(CMessageQueue* pVal)
	{
		PROXY_TRACK("setHotJoinMessageQueue");	
		gGlobals->setHotJoinMessageQueue(pVal);	
	}
	DllExport void setMessageControl(CMessageControl* pVal)
	{
		PROXY_TRACK("setMessageControl");	
		gGlobals->setMessageControl(pVal);	
	}
	DllExport void setSetupData(CvSetupData* pVal)
	{
		PROXY_TRACK("setSetupData");	
		gGlobals->setSetupData(pVal);	
	}
	DllExport void setMessageCodeTranslator(CvMessageCodeTranslator* pVal)
	{
		PROXY_TRACK("setMessageCodeTranslator");	
		gGlobals->setMessageCodeTranslator(pVal);	
	}
	DllExport void setDropMgr(CvDropMgr* pVal)
	{
		PROXY_TRACK("setDropMgr");	
		gGlobals->setDropMgr(pVal);	
	}
	DllExport void setPortal(CvPortal* pVal)
	{
		PROXY_TRACK("setPortal");	
		gGlobals->setPortal(pVal);	
	}
	DllExport void setStatsReport(CvStatsReporter* pVal)
	{
		PROXY_TRACK("setStatsReport");	
		gGlobals->setStatsReport(pVal);	
	}
	DllExport void setPathFinder(FAStar* pVal)
	{
		PROXY_TRACK("setPathFinder");	
		gGlobals->setPathFinder(pVal);	
	}
	DllExport void setInterfacePathFinder(FAStar* pVal)
	{
		PROXY_TRACK("setInterfacePathFinder");	
		gGlobals->setInterfacePathFinder(pVal);	
	}
	DllExport void setStepFinder(FAStar* pVal)
	{
		PROXY_TRACK("setStepFinder");	
		gGlobals->setStepFinder(pVal);	
	}
	DllExport void setRouteFinder(FAStar* pVal)
	{
		PROXY_TRACK("setRouteFinder");	
		gGlobals->setRouteFinder(pVal);	
	}
	DllExport void setBorderFinder(FAStar* pVal)
	{
		PROXY_TRACK("setBorderFinder");	
		gGlobals->setBorderFinder(pVal);	
	}
	DllExport void setAreaFinder(FAStar* pVal)
	{
		PROXY_TRACK("setAreaFinder");	
		gGlobals->setAreaFinder(pVal);	
	}
	DllExport void setPlotGroupFinder(FAStar* pVal)
	{
		PROXY_TRACK("setPlotGroupFinder");	
		gGlobals->setPlotGroupFinder(pVal);	
	}

	// So that CvEnums are moddable in the DLL
	DllExport int getNumDirections() const
	{
		PROXY_TRACK("getNumDirections");	
		return gGlobals->getNumDirections();	
	}
	DllExport int getNumGameOptions() const
	{
		PROXY_TRACK("getNumGameOptions");	
		return gGlobals->getNumGameOptions();	
	}
	DllExport int getNumMPOptions() const
	{
		PROXY_TRACK("getNumMPOptions");	
		return gGlobals->getNumMPOptions();	
	}
	DllExport int getNumSpecialOptions() const
	{
		PROXY_TRACK("getNumSpecialOptions");	
		return gGlobals->getNumSpecialOptions();	
	}
	DllExport int getNumGraphicOptions() const
	{
		PROXY_TRACK("getNumGraphicOptions");	
		return gGlobals->getNumGraphicOptions();	
	}
	DllExport int getNumTradeableItems() const
	{
		PROXY_TRACK("getNumTradeableItems");	
		return gGlobals->getNumTradeableItems();	
	}
	DllExport int getNumBasicItems() const
	{
		PROXY_TRACK("getNumBasicItems");	
		return gGlobals->getNumBasicItems();	
	}
	DllExport int getNumTradeableHeadings() const
	{
		PROXY_TRACK("getNumTradeableHeadings");	
		return gGlobals->getNumTradeableHeadings();	
	}
	DllExport int getNumCommandInfos() const
	{
		PROXY_TRACK("getNumCommandInfos");	
		return gGlobals->getNumCommandInfos();	
	}
	DllExport int getNumControlInfos() const
	{
		PROXY_TRACK("getNumControlInfos");	
		return gGlobals->getNumControlInfos();	
	}
	DllExport int getNumMissionInfos() const
	{
		PROXY_TRACK("getNumMissionInfos");	
		return gGlobals->getNumMissionInfos();	
	}
	DllExport int getNumPlayerOptionInfos() const
	{
		PROXY_TRACK("getNumPlayerOptionInfos");	
		return gGlobals->getNumPlayerOptionInfos();	
	}
	DllExport int getMaxNumSymbols() const
	{
		PROXY_TRACK("getMaxNumSymbols");	
		return gGlobals->getMaxNumSymbols();	
	}
	DllExport int getNumGraphicLevels() const
	{
		PROXY_TRACK("getNumGraphicLevels");	
		return gGlobals->getNumGraphicLevels();	
	}
	DllExport int getNumGlobeLayers() const
	{
		PROXY_TRACK("getNumGlobeLayers");	
		return gGlobals->getNumGlobeLayers();	
	}
};

extern CvGlobals gGlobalsProxy;	// for debugging

extern int giProfilerDisabled;  // set to 1 or more in threaded areas as the profiler is not thread safe

//
// inlines
//
inline cvInternalGlobals& cvInternalGlobals::getInstance()
{
	if ( gGlobals == NULL )
	{
		::MessageBoxA(NULL, "cvInternalGlobals::getInstance() called prior to instantiation\n","CvGameCore",MB_OK);
	}
	return *gGlobals;
}

inline CvGlobals& CvGlobals::getInstance()
{
	return gGlobalsProxy;
}

//
// helpers
//
#define GC cvInternalGlobals::getInstance()
#define gDLL g_DLL

#ifndef _USRDLL
#define NUM_DIRECTION_TYPES (GC.getNumDirections())
#define NUM_GAMEOPTION_TYPES (GC.getNumGameOptions())
#define NUM_MPOPTION_TYPES (GC.getNumMPOptions())
#define NUM_SPECIALOPTION_TYPES (GC.getNumSpecialOptions())
#define NUM_GRAPHICOPTION_TYPES (GC.getNumGraphicOptions())
#define NUM_TRADEABLE_ITEMS (GC.getNumTradeableItems())
#define NUM_BASIC_ITEMS (GC.getNumBasicItems())
#define NUM_TRADEABLE_HEADINGS (GC.getNumTradeableHeadings())
#define NUM_COMMAND_TYPES (GC.getNumCommandInfos())
#define NUM_CONTROL_TYPES (GC.getNumControlInfos())
#define NUM_PLAYEROPTION_TYPES (GC.getNumPlayerOptionInfos())
#define MAX_NUM_SYMBOLS (GC.getMaxNumSymbols())
#define NUM_GRAPHICLEVELS (GC.getNumGraphicLevels())
#define NUM_GLOBE_LAYER_TYPES (GC.getNumGlobeLayers())
#endif

#ifndef FIXED_MISSION_NUMBER
#define NUM_MISSION_TYPES (GC.getNumMissionInfos())
#endif



/************************************************************************************************/
/* ADVANCED COMBAT ODDS                      17/02/09                          PieceOfMind      */
/*                                                                                              */
/************************************************************************************************/

#ifndef ADVANCED_COMBAT_ODDS_H
#define ADVANCED_COMBAT_ODDS_H

#define ACO_DETAIL_LOW          0
#define ACO_DETAIL_MEDIUM       1
#define ACO_DETAIL_HIGH         2
#define ACO_DETAIL_EVERYTHING   3

#endif

#endif

/**********************************************************************

File:		BugMod.h
Author:		EmperorFool
Created:	2009-01-22

Defines common constants and functions for use throughout the BUG Mod.

		Copyright (c) 2009 The BUG Mod. All rights reserved.

**********************************************************************/

#pragma once

#ifndef BUG_MOD_H
#define BUG_MOD_H

// name of the Python module where all the BUG functions that the DLL calls must live
// MUST BE A BUILT-IN MODULE IN THE ENTRYPOINTS FOLDER
// currently CvAppInterface
#define PYBugModule				PYCivModule

// Increment this by 1 each time you commit new/changed functions/constants in the Python API.
#define BUG_DLL_API_VERSION		6

// Used to signal the BULL saved game format is used
#define BUG_DLL_SAVE_FORMAT		64

// These are display-only values, and the version should be changed for each release.
#define BUG_DLL_NAME			L"BULL"
#define BUG_DLL_VERSION			L"1.3"
#define BUG_DLL_BUILD			L"219"

#endif