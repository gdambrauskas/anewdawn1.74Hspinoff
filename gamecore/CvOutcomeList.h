#pragma once

//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvOutcomeList.h
//
//  PURPOSE: A list of possible outcomes with a relative chance
//
//------------------------------------------------------------------------------------------------
#ifndef CV_OUTCOME_LIST_H
#define CV_OUTCOME_LIST_H

#include "CvXMLLoadUtilityModTools.h"
#include "CvOutcome.h"

class CvUnitAI;

class CvOutcomeList
{
public:
	CvOutcome* getOutcome(int index);
	int getNumOutcomes() const;

	bool isPossible(const CvUnit& kUnit) const;
	bool isPossibleSomewhere(const CvUnit& kUnit) const;
	bool isPossibleInPlot(const CvUnit& kUnit, const CvPlot& kPlot, bool bForTrade = false) const;
	bool execute(CvUnit& kUnit, PlayerTypes eDefeatedUnitPlayer = NO_PLAYER, UnitTypes eDefeatedUnitType = NO_UNIT);

	int AI_getValueInPlot(const CvUnit& kUnit, const CvPlot& kPlot, bool bForTrade = false);
	
	bool isEmpty() const;
	void clear();

	void buildDisplayString(CvWStringBuffer& szBuffer, const CvUnit& kUnit);
	
	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);
	bool read(CvXMLLoadUtility* pXML, const TCHAR* szTagName = "Outcomes");
	void copyNonDefaults(CvOutcomeList* pOutcomeList, CvXMLLoadUtility* pXML );

	void getCheckSum(unsigned int& iSum);
protected:
	std::vector<CvOutcome> m_aOutcome;
};

#endif