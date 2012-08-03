#pragma once

//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvOutcome.h
//
//  PURPOSE: A single outcome from an outcome list
//
//------------------------------------------------------------------------------------------------
#ifndef CV_OUTCOME_H
#define CV_OUTCOME_H

#include "CvXMLLoadUtilityModTools.h"

class CvOutcome
{
public:
	CvOutcome();
	int getYield(YieldTypes eYield) const;
	int getCommerce(CommerceTypes eCommerce) const;
	int getChance(const CvUnit& kUnit) const;
	OutcomeTypes getType() const;
	UnitTypes getUnitType() const;
	bool getUnitToCity() const;
	PromotionTypes getPromotionType() const;
	int getGPP() const;
	UnitTypes getGPUnitType() const;
	BonusTypes getBonusType() const;
	
	bool isPossible(const CvUnit& kUnit) const;
	bool isPossible(const CvPlayerAI& kPlayer) const;
	bool execute(CvUnit& kUnit, PlayerTypes eDefeatedUnitPlayer = NO_PLAYER, UnitTypes eDefeatedUnitType = NO_UNIT) const;

	void buildDisplayString(CvWStringBuffer& szBuffer, const CvUnit& kUnit) const;
	
	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);
	bool read(CvXMLLoadUtility* pXML);
	void copyNonDefaults(CvOutcome* pOutcome, CvXMLLoadUtility* pXML );

	void getCheckSum(unsigned int& iSum);
protected:
	OutcomeTypes m_eType;
	int m_iChance;
	int m_aiYield[NUM_YIELD_TYPES];
	int m_aiCommerce[NUM_COMMERCE_TYPES];
	UnitTypes m_eUnitType;
	bool m_bUnitToCity;
	PromotionTypes m_ePromotionType;
	BonusTypes m_eBonusType;
	int m_iGPP;
	UnitTypes m_eGPUnitType;
};

#endif