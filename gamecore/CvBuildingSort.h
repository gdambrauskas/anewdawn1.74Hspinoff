#pragma once

//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvBuildingSort.h
//
//  PURPOSE: Sorting classes for buildings
//
//------------------------------------------------------------------------------------------------

#ifndef CV_BUILDING_SORT_H
#define CV_BUILDING_SORT_H

#include "CvEnums.h"

enum BuildingSortTypes
{
	NO_BUILDING_SORT = -1,

	BUILDING_SORT_NAME,
	BUILDING_SORT_COST,
	BUILDING_SORT_SCIENCE,
	BUILDING_SORT_CULTURE,
	BUILDING_SORT_ESPIONAGE,
	BUILDING_SORT_GOLD,
	BUILDING_SORT_FOOD,
	BUILDING_SORT_PRODUCTION,
	BUILDING_SORT_HAPPINESS,
	BUILDING_SORT_HEALTH,
	BUILDING_SORT_CRIME,
	BUILDING_SORT_FLAMMABILITY,

	NUM_BUILDING_SORT
};

void CyEnumsBuildingSortPythonInterface();

class BuildingSortBase
{
public:
	BuildingSortBase(bool bInvert = false);
	// Returns the value of eBuilding in the sorting category
	virtual int getBuildingValue(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding) = 0;
	// Returns if eBuilding1 is before eBuilding2, default for the value based sorts is greater first
	virtual bool isLesserBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding1, BuildingTypes eBuilding2);
	bool isInverse();
	bool setInverse(bool bInvert);
	void deleteCache();
protected:
	bool m_bInvert;
	stdext::hash_map<BuildingTypes, int> m_mapValueCache;
};

class BuildingSortCommerce : public BuildingSortBase
{
public:
	BuildingSortCommerce(CommerceTypes eCommerce, bool bInvert = false);
	int getBuildingValue(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding);

protected:
	CommerceTypes m_eCommerce;
};

class BuildingSortYield : public BuildingSortBase
{
public:
	BuildingSortYield(YieldTypes eYield, bool bInvert = false);
	int getBuildingValue(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding);

protected:
	YieldTypes m_eYield;
};

class BuildingSortHappiness : public BuildingSortBase
{
public:
	BuildingSortHappiness(bool bInvert = false) : BuildingSortBase(bInvert) {};
	int getBuildingValue(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding);
};

class BuildingSortHealth : public BuildingSortBase
{
public:
	BuildingSortHealth(bool bInvert = false) : BuildingSortBase(bInvert) {};
	int getBuildingValue(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding);
};

class BuildingSortCost : public BuildingSortBase
{
public:
	BuildingSortCost(bool bInvert = false) : BuildingSortBase(bInvert) {};
	int getBuildingValue(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding);
};

class BuildingSortName : public BuildingSortBase
{
public:
	BuildingSortName(bool bInvert = false) : BuildingSortBase(bInvert) {};
	bool isLesserBuilding(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding1, BuildingTypes eBuilding2);
	int getBuildingValue(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding);
};

class BuildingSortProperty : public BuildingSortBase
{
public:
	BuildingSortProperty(PropertyTypes eProperty, bool bInvert = false) : BuildingSortBase(bInvert), m_eProperty(eProperty) {};
	int getBuildingValue(CvPlayer *pPlayer, CvCity *pCity, BuildingTypes eBuilding);

protected:
	PropertyTypes m_eProperty;
};

class BuildingSortList
{
public:
	BuildingSortList(CvPlayer *pPlayer = NULL, CvCity *pCity = NULL);
	~BuildingSortList();
	void init();
	BuildingSortTypes getActiveSort();
	bool setActiveSort(BuildingSortTypes eActiveSort);
	int getNumSort();
	void setPlayer(CvPlayer* pPlayer);
	void setCity(CvCity* pCity);
	bool operator()(BuildingTypes eBuilding1, BuildingTypes eBuilding2);
	void deleteCache();

protected:
	BuildingSortBase* m_apBuildingSort[NUM_BUILDING_SORT];
	CvCity* m_pCity;
	CvPlayer* m_pPlayer;
	BuildingSortTypes m_eActiveSort;
};

class BuildingSortListWrapper
{
public:
	BuildingSortListWrapper(BuildingSortList* pList) : m_pList(pList) {}
	bool operator()(BuildingTypes eBuilding1, BuildingTypes eBuilding2) {return m_pList->operator()(eBuilding1, eBuilding2);}
	void deleteCache() {m_pList->deleteCache();}
protected:
	BuildingSortList* m_pList;
};

#endif