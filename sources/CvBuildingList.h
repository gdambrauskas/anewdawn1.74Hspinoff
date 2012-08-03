#pragma once

//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvBuildingList.h
//
//  PURPOSE: Filter, group, sort and cache a building list for a city or player
//
//------------------------------------------------------------------------------------------------

#ifndef CV_BUILDING_LIST_H
#define CV_BUILDING_LIST_H
#include "CvBuildingFilters.h"
#include "CvBuildingGrouping.h"
#include "CvBuildingSort.h"

void CyEnumsBuildingListPythonInterface();

class CvBuildingList
{
public:
	CvBuildingList(CvPlayer* pPlayer = NULL, CvCity* pCity = NULL);
	void setPlayerToOwner();
	void setInvalid();
	bool getFilterActive(BuildingFilterTypes eFilter);
	void setFilterActive(BuildingFilterTypes eFilter, bool bActive);
	BuildingGroupingTypes getGroupingActive();
	void setGroupingActive(BuildingGroupingTypes eGrouping);
	BuildingSortTypes getSortingActive();
	void setSortingActive(BuildingSortTypes eSorting);
	int getGroupNum();
	int getNumInGroup(int iGroup);
	BuildingTypes getBuildingType(int iGroup, int iPos);
	int getBuildingSelectionRow();
	int getWonderSelectionRow();
	void setSelectedBuilding(BuildingTypes eSelectedBuilding);
	void setSelectedWonder(BuildingTypes eSelectedWonder);
	BuildingTypes getSelectedBuilding();
	BuildingTypes getSelectedWonder();
protected:
	void doFilter();
	void doGroup();
	void doSort();
	BuildingFilterList m_BuildingFilters;
	BuildingGroupingList m_BuildingGrouping;
	BuildingSortList m_BuildingSort;
	std::vector<BuildingTypes> m_aiBuildingList;
	std::vector<std::vector<BuildingTypes>*> m_aaiGroupedBuildingList;
	bool m_bFilteringValid;
	bool m_bGroupingValid;
	bool m_bSortingValid;
	CvCity* m_pCity;
	CvPlayer* m_pPlayer;
	BuildingTypes m_eSelectedBuilding;
	BuildingTypes m_eSelectedWonder;
};


#endif
