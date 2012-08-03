#pragma once

//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvProperties.h
//
//  PURPOSE: Generic properties for Civ4 classes
//
//------------------------------------------------------------------------------------------------
#ifndef CV_PROPERTIES_H
#define CV_PROPERTIES_H

#include "CvXMLLoadUtilityModTools.h"

class CvGame;
class CvTeam;
class CvPlayer;
class CvCity;
class CvUnit;
class CvPlot;

class CvProperties
{
public:
	CvProperties();
	CvProperties(CvGame* pGame);
	CvProperties(CvTeam* pTeam);
	CvProperties(CvPlayer* pPlayer);
	CvProperties(CvCity* pCity);
	CvProperties(CvUnit* pUnit);
	CvProperties(CvPlot* pPlot);
	int getProperty(int index) const;
	int getValue(int index) const;
	int getNumProperties() const;
	int getPositionByProperty(int eProp) const;
	int getValueByProperty(int eProp) const;
	void setValue(int index, int iVal);
	void setValueByProperty(int eProp, int iVal);
	void changeValue(int index, int iChange);
	void changeValueByProperty(int eProp, int iChange);
	void propagateChange(int eProp, int iChange);
	
	void addProperties(CvProperties* pProp);
	void subtractProperties(CvProperties* pProp);

	bool isEmpty() const;
	void clear();
	void clearForRecalculate();

	//Note: The comparison operators are NOT symmetric. Only properties defined in the second operand are considered.
	// That means any property object is smaller than the empty property and bigger as well.
	bool operator<(const CvProperties& prop) const;
	bool operator<=(const CvProperties& prop) const;
	bool operator>(const CvProperties& prop) const;
	bool operator>=(const CvProperties& prop) const;
	bool operator==(const CvProperties& prop) const;
	bool operator!=(const CvProperties& prop) const;

	void buildRequiresMinString(CvWStringBuffer& szBuffer, const CvProperties& prop) const;
	void buildRequiresMaxString(CvWStringBuffer& szBuffer, const CvProperties& prop) const;
	void buildChangesString(CvWStringBuffer& szBuffer, CvWString* szCity = NULL) const;
	void buildChangesAllCitiesString(CvWStringBuffer& szBuffer) const;
	void buildDisplayString(CvWStringBuffer& szBuffer) const;

	// For Python
	std::wstring getPropertyDisplay(int index) const;
	
	void read(FDataStreamBase* pStream);
	void readWrapper(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);
	void writeWrapper(FDataStreamBase* pStream);
	bool read(CvXMLLoadUtility* pXML, const TCHAR* szTagName = "Properties");
	void copyNonDefaults(CvProperties* pProp, CvXMLLoadUtility* pXML );

	void getCheckSum(unsigned int& iSum);
protected:
	std::vector<std::pair<int,int> > m_aiProperty;
	
	// These are filled on creation of the properties object
	GameObjectTypes m_eParentType;
	union
	{
		CvGame* m_pGame;
		CvTeam* m_pTeam;
		CvPlayer* m_pPlayer;
		CvCity* m_pCity;
		CvUnit* m_pUnit;
		CvPlot* m_pPlot;
	};
};

#endif