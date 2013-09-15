//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:	CvOutcome.cpp
//
//  PURPOSE: A single outcome from an outcome list
//
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "CvOutcome.h"
#include "CvXMLLoadUtility.h"
#include "CvDLLXMLIFaceBase.h"
#include "CvDLLInterfaceIFaceBase.h"

CvOutcome::CvOutcome(): m_eUnitType(NO_UNIT),
						m_iChance(0),
						m_eType(NO_OUTCOME),
						m_ePromotionType(NO_PROMOTION),
						m_iGPP(0),
						m_eGPUnitType(NO_UNIT),
						m_eBonusType(NO_BONUS),
						m_bUnitToCity(false),
						m_eEventTrigger(NO_EVENTTRIGGER)
{
	for (int i=0; i<NUM_YIELD_TYPES; i++)
	{
		m_aiYield[i] = 0;
	}

	for (int i=0; i<NUM_COMMERCE_TYPES; i++)
	{
		m_aiCommerce[i] = 0;
	}
}

CvOutcome::~CvOutcome()
{
	GC.removeDelayedResolution((int*)&m_eEventTrigger);
}

int CvOutcome::getYield(YieldTypes eYield) const
{
	FAssert(0 <= eYield);
	FAssert(eYield < NUM_YIELD_TYPES);
	return m_aiYield[eYield];
}

int CvOutcome::getCommerce(CommerceTypes eCommerce) const
{
	FAssert(0 <= eCommerce);
	FAssert(eCommerce < NUM_COMMERCE_TYPES);
	return m_aiCommerce[eCommerce];
}

OutcomeTypes CvOutcome::getType() const
{
	return m_eType;
}

UnitTypes CvOutcome::getUnitType() const
{
	return m_eUnitType;
}

bool CvOutcome::getUnitToCity() const
{
	return m_bUnitToCity;
}

PromotionTypes CvOutcome::getPromotionType() const
{
	return m_ePromotionType;
}

BonusTypes CvOutcome::getBonusType() const
{
	return m_eBonusType;
}

UnitTypes CvOutcome::getGPUnitType() const
{
	return m_eGPUnitType;
}

int CvOutcome::getGPP() const
{
	return m_iGPP;
}

CvProperties* CvOutcome::getProperties()
{
	return &m_Properties;
}

int CvOutcome::getHappinessTimer()
{
	return m_iHappinessTimer;
}

EventTriggerTypes CvOutcome::getEventTrigger()
{
	return m_eEventTrigger;
}

int CvOutcome::getChancePerPop() const
{
	return m_iChancePerPop;
}

int CvOutcome::getChance(const CvUnit &kUnit) const
{
	int iChance = m_iChance;
	CvOutcomeInfo& kInfo = GC.getOutcomeInfo(m_eType);

	CvCity* pCity = kUnit.plot()->getPlotCity();

	if (pCity)
	{
		iChance += getChancePerPop() * pCity->getPopulation();
	}
	
	for (int i=0; i<kInfo.getNumExtraChancePromotions(); i++)
	{
		if (kUnit.isHasPromotion(kInfo.getExtraChancePromotion(i)))
		{
			iChance += kInfo.getExtraChancePromotionChance(i);
		}
	}
	return iChance > 0 ? iChance : 0;
}

bool CvOutcome::isPossible(const CvUnit& kUnit) const
{
	CvTeam& kTeam = GET_TEAM(kUnit.getTeam());
	CvOutcomeInfo& kInfo = GC.getOutcomeInfo(m_eType);

	if (!kTeam.isHasTech(kInfo.getPrereqTech()))
	{
		return false;
	}

	if (kInfo.getObsoleteTech() != NO_TECH)
	{
		if (kTeam.isHasTech(kInfo.getObsoleteTech()))
		{
			return false;
		}
	}

	if (kInfo.getPrereqCivic() != NO_CIVIC)
	{
		if (!GET_PLAYER(kUnit.getOwnerINLINE()).isCivic(kInfo.getPrereqCivic()))
		{
			return false;
		}
	}

	if (kInfo.getCity())
	{
		if (!kUnit.plot()->isCity())
		{
			return false;
		}
	}

	if (kInfo.getNotCity())
	{
		if (kUnit.plot()->isCity())
		{
			return false;
		}
	}

	TeamTypes eOwnerTeam = GET_PLAYER(kUnit.getOwnerINLINE()).getTeam();
	CvTeam& kOwnerTeam = GET_TEAM(eOwnerTeam);
	PlayerTypes ePlotOwner = kUnit.plot()->getOwnerINLINE();
	if (ePlotOwner == NO_PLAYER)
	{
		if (!kInfo.getNeutralTerritory())
		{
			return false;
		}
	}
	else
	{
		if (GET_PLAYER(ePlotOwner).isBarbarian())
		{
			if (!kInfo.getBarbarianTerritory())
			{
				return false;
			}
		}
		else
		{
			TeamTypes ePlotOwnerTeam = GET_PLAYER(ePlotOwner).getTeam();
			CvTeam& kPlotOwnerTeam = GET_TEAM(ePlotOwnerTeam);
			if (kOwnerTeam.isAtWar(ePlotOwnerTeam))
			{
				if (!kInfo.getHostileTerritory())
				{
					return false;
				}
			}
			else if ((eOwnerTeam == ePlotOwnerTeam) || (kPlotOwnerTeam.isVassal(eOwnerTeam)))
			{
				if (!kInfo.getFriendlyTerritory())
				{
					return false;
				}
			}
			else
			{
				if (!kInfo.getNeutralTerritory())
				{
					return false;
				}
			}
		}
	}

	int iPrereqBuildings = kInfo.getNumPrereqBuildings();
	if (iPrereqBuildings > 0)
	{
		CvCity* pCity = kUnit.plot()->getPlotCity();
		if (!pCity)
		{
			return false;
		}

		for (int i=0; i<iPrereqBuildings; i++)
		{
			if (pCity->getNumBuilding(kInfo.getPrereqBuilding(i)) <= 0)
			{
				return false;
			}
		}
	}

	if (m_ePromotionType != NO_PROMOTION)
	{
		CvPromotionInfo& kPromotion = GC.getPromotionInfo(m_ePromotionType);
		if (!kTeam.isHasTech((TechTypes)kPromotion.getTechPrereq()))
		{
			return false;
		}
		if ((TechTypes)kPromotion.getObsoleteTech() != NO_TECH)
		{
			if (kTeam.isHasTech((TechTypes)kPromotion.getObsoleteTech()))
			{
				return false;
			}
		}
	}

	if (m_eBonusType != NO_BONUS)
	{
		CvBonusInfo& kBonus = GC.getBonusInfo(m_eBonusType);
		if (!kTeam.isHasTech((TechTypes)kBonus.getTechReveal()))
		{
			return false;
		}
		if ((TechTypes)kBonus.getTechObsolete() != NO_TECH)
		{
			if (kTeam.isHasTech((TechTypes)kBonus.getTechObsolete()))
			{
				return false;
			}
		}
		if (kUnit.plot()->getBonusType() != NO_BONUS)
		{
			return false;
		}
#ifdef MULTI_FEATURE_MOD
		if (kUnit.plot()->getNumFeatures() == 0)
#else
		if (kUnit.plot()->getFeatureType() == NO_FEATURE)
#endif
		{
			if (!kBonus.isTerrain(kUnit.plot()->getTerrainType()))
			{
				return false;
			}
		}
		else
		{
#ifdef MULTI_FEATURE_MOD
			bool bValid = false;
			for (int i=0; i<kUnit.plot()->getNumFeatures(); i++)
			{
				if (kBonus.isFeature(kUnit.plot()->getFeatureByIndex(i)))
				{
					bValid = true;
					break;
				}
			}
			if (!bValid)
			{
				return false;
			}
#else
			if (!kBonus.isFeature(kUnit.plot()->getFeatureType()))
			{
				return false;
			}
#endif
			if (!kBonus.isFeatureTerrain(kUnit.plot()->getTerrainType()))
			{
				return false;
			}
		}

		int iCount = 0;
		for (int iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
		{
			CvPlot* pAdjacentPlot = plotDirection(kUnit.plot()->getX_INLINE(), kUnit.plot()->getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot != NULL)
			{
				if (pAdjacentPlot->getBonusType() == m_eBonusType)
				{
					iCount++;
				}
			}
		}
		if (!(iCount == 0 || (iCount == 1 && kUnit.plot()->isWater())))
		{
			return false;
		}
	}

	if (m_eEventTrigger != NO_EVENTTRIGGER)
	{
		CvPlayer& kOwner = GET_PLAYER(kUnit.getOwnerINLINE());
		CvEventTriggerInfo& kTriggerInfo = GC.getEventTriggerInfo(m_eEventTrigger);
		if (!kOwner.isEventTriggerPossible(m_eEventTrigger))
		{
			return false;
		}

		if (kTriggerInfo.isPickCity())
		{
			CvCity* pCity = kUnit.plot()->getPlotCity();
			if (pCity)
			{
				if (!pCity->isEventTriggerPossible(m_eEventTrigger))
				{
					return false;
				}
			}
		}

		if (isPlotEventTrigger(m_eEventTrigger))
		{
			if (!kUnit.plot()->canTrigger(m_eEventTrigger, kUnit.getOwnerINLINE()))
			{
				return false;
			}
		}
	}

	return getChance(kUnit) > 0;
}

// Can return a false positive if an outcome requires a building combination
bool CvOutcome::isPossibleSomewhere(const CvUnit& kUnit) const
{
	CvTeam& kTeam = GET_TEAM(kUnit.getTeam());
	CvOutcomeInfo& kInfo = GC.getOutcomeInfo(m_eType);

	if (!kTeam.isHasTech(kInfo.getPrereqTech()))
	{
		return false;
	}

	if (kInfo.getObsoleteTech() != NO_TECH)
	{
		if (kTeam.isHasTech(kInfo.getObsoleteTech()))
		{
			return false;
		}
	}

	if (kInfo.getPrereqCivic() != NO_CIVIC)
	{
		if (!GET_PLAYER(kUnit.getOwnerINLINE()).isCivic(kInfo.getPrereqCivic()))
		{
			return false;
		}
	}

	TeamTypes eOwnerTeam = GET_PLAYER(kUnit.getOwnerINLINE()).getTeam();
	CvTeam& kOwnerTeam = GET_TEAM(eOwnerTeam);

	int iPrereqBuildings = kInfo.getNumPrereqBuildings();
	if (iPrereqBuildings > 0)
	{
		for (int i=0; i<iPrereqBuildings; i++)
		{
			if (GET_PLAYER(kUnit.getOwnerINLINE()).getBuildingClassCount((BuildingClassTypes)GC.getBuildingInfo(kInfo.getPrereqBuilding(i)).getBuildingClassType()) <= 0)
			{
				return false;
			}
		}
	}

	if (m_ePromotionType != NO_PROMOTION)
	{
		CvPromotionInfo& kPromotion = GC.getPromotionInfo(m_ePromotionType);
		if (!kTeam.isHasTech((TechTypes)kPromotion.getTechPrereq()))
		{
			return false;
		}
		if ((TechTypes)kPromotion.getObsoleteTech() != NO_TECH)
		{
			if (kTeam.isHasTech((TechTypes)kPromotion.getObsoleteTech()))
			{
				return false;
			}
		}
	}

	if (m_eBonusType != NO_BONUS)
	{
		CvBonusInfo& kBonus = GC.getBonusInfo(m_eBonusType);
		if (!kTeam.isHasTech((TechTypes)kBonus.getTechReveal()))
		{
			return false;
		}
		if ((TechTypes)kBonus.getTechObsolete() != NO_TECH)
		{
			if (kTeam.isHasTech((TechTypes)kBonus.getTechObsolete()))
			{
				return false;
			}
		}
	}

	if (m_eEventTrigger != NO_EVENTTRIGGER)
	{
		CvPlayer& kOwner = GET_PLAYER(kUnit.getOwnerINLINE());
		if (!kOwner.isEventTriggerPossible(m_eEventTrigger))
		{
			return false;
		}
	}

	return getChance(kUnit) > 0;
}

bool CvOutcome::isPossibleInPlot(const CvUnit& kUnit, const CvPlot& kPlot, bool bForTrade) const
{
	CvTeam& kTeam = GET_TEAM(kUnit.getTeam());
	CvOutcomeInfo& kInfo = GC.getOutcomeInfo(m_eType);

	if (!kTeam.isHasTech(kInfo.getPrereqTech()))
	{
		return false;
	}

	if (kInfo.getObsoleteTech() != NO_TECH)
	{
		if (kTeam.isHasTech(kInfo.getObsoleteTech()))
		{
			return false;
		}
	}

	if (kInfo.getPrereqCivic() != NO_CIVIC)
	{
		if (!GET_PLAYER(kUnit.getOwnerINLINE()).isCivic(kInfo.getPrereqCivic()))
		{
			return false;
		}
	}

	if (kInfo.getCity())
	{
		if (!kPlot.isCity())
		{
			return false;
		}
	}

	if (kInfo.getNotCity())
	{
		if (kPlot.isCity())
		{
			return false;
		}
	}

	TeamTypes eOwnerTeam = GET_PLAYER(kUnit.getOwnerINLINE()).getTeam();
	CvTeam& kOwnerTeam = GET_TEAM(eOwnerTeam);
	PlayerTypes ePlotOwner = kPlot.getOwnerINLINE();
	if (ePlotOwner == NO_PLAYER)
	{
		if (!kInfo.getNeutralTerritory())
		{
			return false;
		}
	}
	else
	{
		if (GET_PLAYER(ePlotOwner).isBarbarian())
		{
			if (!kInfo.getBarbarianTerritory())
			{
				return false;
			}
		}
		else
		{
			TeamTypes ePlotOwnerTeam = GET_PLAYER(ePlotOwner).getTeam();
			CvTeam& kPlotOwnerTeam = GET_TEAM(ePlotOwnerTeam);
			if (kOwnerTeam.isAtWar(ePlotOwnerTeam))
			{
				if (!kInfo.getHostileTerritory())
				{
					return false;
				}
			}
			else if ((eOwnerTeam == ePlotOwnerTeam) || (kPlotOwnerTeam.isVassal(eOwnerTeam)))
			{
				if (!kInfo.getFriendlyTerritory())
				{
					return false;
				}
			}
			else
			{
				if (!kInfo.getNeutralTerritory())
				{
					return false;
				}
			}
		}
	}

	int iPrereqBuildings = kInfo.getNumPrereqBuildings();
	if (iPrereqBuildings > 0)
	{
		CvCity* pCity = kPlot.getPlotCity();
		if (!pCity)
		{
			return false;
		}

		for (int i=0; i<iPrereqBuildings; i++)
		{
			if (pCity->getNumBuilding(kInfo.getPrereqBuilding(i)) <= 0)
			{
				return false;
			}
		}
	}

	if (m_ePromotionType != NO_PROMOTION)
	{
		CvPromotionInfo& kPromotion = GC.getPromotionInfo(m_ePromotionType);
		if (!kTeam.isHasTech((TechTypes)kPromotion.getTechPrereq()))
		{
			return false;
		}
		if ((TechTypes)kPromotion.getObsoleteTech() != NO_TECH)
		{
			if (kTeam.isHasTech((TechTypes)kPromotion.getObsoleteTech()))
			{
				return false;
			}
		}
	}

	if (m_eBonusType != NO_BONUS)
	{
		CvBonusInfo& kBonus = GC.getBonusInfo(m_eBonusType);
		if (!kTeam.isHasTech((TechTypes)kBonus.getTechReveal()))
		{
			return false;
		}
		if ((TechTypes)kBonus.getTechObsolete() != NO_TECH)
		{
			if (kTeam.isHasTech((TechTypes)kBonus.getTechObsolete()))
			{
				return false;
			}
		}
		if (kPlot.getBonusType() != NO_BONUS)
		{
			return false;
		}
#ifdef MULTI_FEATURE_MOD
		if (kPlot.getNumFeatures() == 0)
#else
		if (kPlot.getFeatureType() == NO_FEATURE)
#endif
		{
			if (!kBonus.isTerrain(kPlot.getTerrainType()))
			{
				return false;
			}
		}
		else
		{
#ifdef MULTI_FEATURE_MOD
			bool bValid = false;
			for (int i=0; i<kPlot.getNumFeatures(); i++)
			{
				if (kBonus.isFeature(kPlot.getFeatureByIndex(i)))
				{
					bValid = true;
					break;
				}
			}
			if (!bValid)
			{
				return false;
			}
#else
			if (!kBonus.isFeature(kPlot.getFeatureType()))
			{
				return false;
			}
#endif
			if (!kBonus.isFeatureTerrain(kPlot.getTerrainType()))
			{
				return false;
			}
		}

		int iCount = 0;
		for (int iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
		{
			CvPlot* pAdjacentPlot = plotDirection(kPlot.getX_INLINE(), kPlot.getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot != NULL)
			{
				if (pAdjacentPlot->getBonusType() == m_eBonusType)
				{
					iCount++;
				}
			}
		}
		if (!(iCount == 0 || (iCount == 1 && kPlot.isWater())))
		{
			return false;
		}
	}

	if (m_eEventTrigger != NO_EVENTTRIGGER)
	{
		CvPlayer& kOwner = GET_PLAYER(kUnit.getOwnerINLINE());
		CvEventTriggerInfo& kTriggerInfo = GC.getEventTriggerInfo(m_eEventTrigger);
		if (!kOwner.isEventTriggerPossible(m_eEventTrigger))
		{
			return false;
		}

		if (kTriggerInfo.isPickCity())
		{
			CvCity* pCity = kPlot.getPlotCity();
			if (pCity)
			{
				if (!pCity->isEventTriggerPossible(m_eEventTrigger))
				{
					return false;
				}
			}
		}

		if (isPlotEventTrigger(m_eEventTrigger))
		{
			if (!kPlot.canTrigger(m_eEventTrigger, kUnit.getOwnerINLINE()))
			{
				return false;
			}
		}
	}

	return getChance(kUnit) > 0;
}

bool CvOutcome::isPossible(const CvPlayerAI& kPlayer) const
{
	CvTeam& kTeam = GET_TEAM(kPlayer.getTeam());
	CvOutcomeInfo& kInfo = GC.getOutcomeInfo(m_eType);

	if (!kTeam.isHasTech(kInfo.getPrereqTech()))
	{
		return false;
	}

	if (kInfo.getObsoleteTech() != NO_TECH)
	{
		if (kTeam.isHasTech(kInfo.getObsoleteTech()))
		{
			return false;
		}
	}

	if (kInfo.getPrereqCivic() != NO_CIVIC)
	{
		if (!kPlayer.isCivic(kInfo.getPrereqCivic()))
		{
			return false;
		}
	}

	if (m_ePromotionType != NO_PROMOTION)
	{
		CvPromotionInfo& kPromotion = GC.getPromotionInfo(m_ePromotionType);
		if (!kTeam.isHasTech((TechTypes)kPromotion.getTechPrereq()))
		{
			return false;
		}
		if ((TechTypes)kPromotion.getObsoleteTech() != NO_TECH)
		{
			if (kTeam.isHasTech((TechTypes)kPromotion.getObsoleteTech()))
			{
				return false;
			}
		}
	}

	if (m_eBonusType != NO_BONUS)
	{
		CvBonusInfo& kBonus = GC.getBonusInfo(m_eBonusType);
		if (!kTeam.isHasTech((TechTypes)kBonus.getTechReveal()))
		{
			return false;
		}
		if ((TechTypes)kBonus.getTechObsolete() != NO_TECH)
		{
			if (kTeam.isHasTech((TechTypes)kBonus.getTechObsolete()))
			{
				return false;
			}
		}
	}

	if (m_eEventTrigger != NO_EVENTTRIGGER)
	{
		if (!kPlayer.isEventTriggerPossible(m_eEventTrigger))
		{
			return false;
		}
	}

	return m_iChance > 0;
}

bool CvOutcome::execute(CvUnit &kUnit, PlayerTypes eDefeatedUnitPlayer, UnitTypes eDefeatedUnitType) const
{
	if (!isPossible(kUnit))
	{
		return false;
	}

	CvWStringBuffer szBuffer;

	CvPlayer& kPlayer = GET_PLAYER(kUnit.getOwnerINLINE());
	bool bToCoastalCity = GC.getOutcomeInfo(getType()).getToCoastalCity();
	CvUnitInfo* pUnitInfo;
	if (eDefeatedUnitType > NO_UNIT)
		pUnitInfo = &GC.getUnitInfo(eDefeatedUnitType);
	else
		pUnitInfo = &kUnit.getUnitInfo();

	CvWString& szMessage = GC.getOutcomeInfo(getType()).getMessageText();
	bool bNothing = true;
	if (!szMessage.empty())
	{
		szBuffer.append(gDLL->getText(szMessage, kUnit.getNameKey(), pUnitInfo->getDescription()));
		szBuffer.append(L" ( ");
		bNothing = false;
	}

	bool bFirst = true;

	if (m_ePromotionType > NO_PROMOTION)
	{
		kUnit.setHasPromotion(m_ePromotionType, true);
		if (!bFirst)
		{
			szBuffer.append(L", ");
		}
		else
		{
			bFirst = false;
		}
		szBuffer.append(GC.getPromotionInfo(m_ePromotionType).getDescription());
	}

	if (m_eUnitType > NO_UNIT && !m_bUnitToCity)
	{
		CvUnit* pUnit = kPlayer.initUnit(m_eUnitType, kUnit.plot()->getX_INLINE(), kUnit.plot()->getY_INLINE(), (UnitAITypes)GC.getUnitInfo(m_eUnitType).getDefaultUnitAIType());
		FAssertMsg(pUnit != NULL, "pUnit is expected to be assigned a valid unit object");
		pUnit->setDamage(75, NO_PLAYER, false);
		pUnit->finishMoves();

		if (!bFirst)
		{
			szBuffer.append(L", ");
		}
		else
		{
			bFirst = false;
		}
		szBuffer.append(GC.getUnitInfo(m_eUnitType).getDescription());
	}

	if (m_aiYield[YIELD_PRODUCTION] || m_aiYield[YIELD_FOOD] || m_aiYield[YIELD_COMMERCE] || m_aiCommerce[COMMERCE_CULTURE] || m_iGPP || (m_bUnitToCity && m_eUnitType > NO_UNIT))
	{
		CvCity* pCity = GC.getMapINLINE().findCity(kUnit.plot()->getX_INLINE(), kUnit.plot()->getY_INLINE(), kUnit.getOwnerINLINE(), NO_TEAM, true, bToCoastalCity);
		if (!pCity)
			pCity = GC.getMapINLINE().findCity(kUnit.plot()->getX_INLINE(), kUnit.plot()->getY_INLINE(), kUnit.getOwnerINLINE(), NO_TEAM, false, bToCoastalCity);

		if (pCity)
		{
			if (!bFirst)
			{
				szBuffer.append(L", ");
			}
			else
			{
				bFirst = false;
			}
			if (m_aiYield[YIELD_PRODUCTION])
			{
				pCity->changeProduction(m_aiYield[YIELD_PRODUCTION]);
				CvWString szTemp;
				szTemp.Format(L" %d%c", m_aiYield[YIELD_PRODUCTION], GC.getYieldInfo(YIELD_PRODUCTION).getChar());
				szBuffer.append(szTemp);
			}
			if (m_aiYield[YIELD_FOOD])
			{
				pCity->changeFood(m_aiYield[YIELD_FOOD]);
				CvWString szTemp;
				szTemp.Format(L" %d%c", m_aiYield[YIELD_FOOD], GC.getYieldInfo(YIELD_FOOD).getChar());
				szBuffer.append(szTemp);
			}

			if (m_aiCommerce[COMMERCE_CULTURE])
			{
				pCity->changeCulture(kUnit.getOwnerINLINE(), m_aiCommerce[COMMERCE_CULTURE], true, true);
				CvWString szTemp;
				szTemp.Format(L" %d%c", m_aiCommerce[COMMERCE_CULTURE], GC.getCommerceInfo(COMMERCE_CULTURE).getChar());
				szBuffer.append(szTemp);
			}

			if (m_iGPP)
			{
				pCity->changeGreatPeopleProgress(m_iGPP);
				if (m_eGPUnitType > NO_UNIT)
				{
					pCity->changeGreatPeopleUnitProgress(m_eGPUnitType, m_iGPP);
				}
				CvWString szTemp;
				szTemp.Format(L" %d%c", m_iGPP, gDLL->getSymbolID(GREAT_PEOPLE_CHAR));
				szBuffer.append(szTemp);
			}

			if (!m_Properties.isEmpty())
			{
				pCity->getProperties()->addProperties(&m_Properties);
				m_Properties.buildCompactChangesString(szBuffer);
			}

			if (m_iHappinessTimer)
			{
				pCity->changeHappinessTimer(m_iHappinessTimer);
				int iHappy = GC.getDefineINT("TEMP_HAPPY");
				szBuffer.append(L" ");
				szBuffer.append(gDLL->getText("TXT_KEY_OUTCOME_TEMP_HAPPY", iHappy, m_iHappinessTimer));
			}

			if (m_bUnitToCity && m_eUnitType > NO_UNIT)
			{
				CvUnit* pUnit = kPlayer.initUnit(m_eUnitType, pCity->getX_INLINE(), pCity->getY_INLINE(), (UnitAITypes)GC.getUnitInfo(m_eUnitType).getDefaultUnitAIType());
				FAssertMsg(pUnit != NULL, "pUnit is expected to be assigned a valid unit object");
				pUnit->setDamage(75, NO_PLAYER, false);
				pUnit->finishMoves();

				szBuffer.append(L" ");
				szBuffer.append(GC.getUnitInfo(m_eUnitType).getDescription());
			}

			szBuffer.append(L" to ");
			szBuffer.append(pCity->getName());
		}
		
		if (m_aiYield[YIELD_COMMERCE])
		{
			if (kPlayer.getCommercePercent(COMMERCE_CULTURE))
			{
				if (pCity)
				{
					pCity->changeCultureTimes100(kUnit.getOwnerINLINE(), m_aiYield[YIELD_COMMERCE] * kPlayer.getCommercePercent(COMMERCE_CULTURE), true, true);
				}
			}
		}
	}

	int iGoldTimes100 = 0;
	int iResearchTimes100 = 0;
	int iEspionageTimes100 = 0;

	if (m_aiYield[YIELD_COMMERCE])
	{
		iGoldTimes100 = m_aiYield[YIELD_COMMERCE] * kPlayer.getCommercePercent(COMMERCE_GOLD);
		iResearchTimes100 = m_aiYield[YIELD_COMMERCE] * kPlayer.getCommercePercent(COMMERCE_RESEARCH);
		iEspionageTimes100 = m_aiYield[YIELD_COMMERCE] * kPlayer.getCommercePercent(COMMERCE_ESPIONAGE);
	}

	iGoldTimes100 += m_aiCommerce[COMMERCE_GOLD] * 100;
	iResearchTimes100 += m_aiCommerce[COMMERCE_RESEARCH] * 100;
	iEspionageTimes100 += m_aiCommerce[COMMERCE_ESPIONAGE] * 100;
	
	if (iGoldTimes100)
	{
		kPlayer.changeGold(iGoldTimes100 / 100);
		if (!bFirst)
		{
			szBuffer.append(L", ");
		}
		else
		{
			bFirst = false;
		}
		CvWString szTemp;
		szTemp.Format(L" %d%c", iGoldTimes100 / 100, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
		szBuffer.append(szTemp);
	}
	CvTeam& kTeam = GET_TEAM(kUnit.getTeam());
	if (iResearchTimes100)
	{
		kTeam.changeResearchProgress(kPlayer.getCurrentResearch(), iResearchTimes100 / 100, kUnit.getOwnerINLINE());
		if (!bFirst)
		{
			szBuffer.append(L", ");
		}
		else
		{
			bFirst = false;
		}
		CvWString szTemp;
		szTemp.Format(L" %d%c", iResearchTimes100 / 100, GC.getCommerceInfo(COMMERCE_RESEARCH).getChar());
		szBuffer.append(szTemp);
	}
	if (iEspionageTimes100 && (eDefeatedUnitPlayer != NO_PLAYER))
	{
		kTeam.changeEspionagePointsEver(iEspionageTimes100 / 100);
		kTeam.changeEspionagePointsAgainstTeam(GET_PLAYER(eDefeatedUnitPlayer).getTeam(), iEspionageTimes100 / 100);
		if (!bFirst)
		{
			szBuffer.append(L", ");
		}
		else
		{
			bFirst = false;
		}
		CvWString szTemp;
		szTemp.Format(L" %d%c", iEspionageTimes100 / 100, GC.getCommerceInfo(COMMERCE_ESPIONAGE).getChar());
		szBuffer.append(szTemp);
	}

	if (m_eBonusType != NO_BONUS)
	{
		kUnit.plot()->setBonusType(m_eBonusType);
		if (!bFirst)
		{
			szBuffer.append(L", ");
		}
		else
		{
			bFirst = false;
		}
		szBuffer.append(GC.getBonusInfo(m_eBonusType).getDescription());
	}

	szBuffer.append(L" )");

	if (!bNothing)
	{
		MEMORY_TRACK_EXEMPT();

		gDLL->getInterfaceIFace()->addMessage(kUnit.getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer.getCString(), NULL, MESSAGE_TYPE_INFO, pUnitInfo->getButton(), NO_COLOR, kUnit.plot()->getX_INLINE(), kUnit.plot()->getY_INLINE(), true, true);
	}

	if (m_eEventTrigger != NO_EVENTTRIGGER)
	{
		CvEventTriggerInfo& kTriggerInfo = GC.getEventTriggerInfo(m_eEventTrigger);
		if (kTriggerInfo.isPickCity() && (kUnit.plot()->getPlotCity() != NULL))
		{
			kPlayer.initTriggeredData(m_eEventTrigger, true, kUnit.plot()->getPlotCity()->getID());
		}
		else
		{
			kPlayer.initTriggeredData(m_eEventTrigger, true, -1, kUnit.plot()->getX_INLINE(), kUnit.plot()->getY_INLINE());
		}
	}

	return true;
}

int CvOutcome::AI_getValueInPlot(const CvUnit &kUnit, const CvPlot &kPlot, bool bForTrade) const
{
	if (!isPossibleInPlot(kUnit, kPlot, bForTrade))
	{
		return false;
	}

	int iValue = 0;

	CvPlayerAI& kPlayer = GET_PLAYER(kUnit.getOwnerINLINE());
	bool bToCoastalCity = GC.getOutcomeInfo(getType()).getToCoastalCity();
	CvUnitInfo* pUnitInfo = &kUnit.getUnitInfo();

	if (m_ePromotionType > NO_PROMOTION)
	{
			iValue += kPlayer.AI_promotionValue(m_ePromotionType, kUnit.getUnitType(), &kUnit, kUnit.AI_getUnitAIType());
	}

	if (m_eUnitType > NO_UNIT)
	{
		iValue += kPlayer.AI_unitValue(m_eUnitType, (UnitAITypes)GC.getUnitInfo(m_eUnitType).getDefaultUnitAIType(), kPlot.area());
	}

	// We go the easy way and use AI_yieldValue for all the yields and commerces despite that city multipliers do not apply for some
	if (m_aiYield[YIELD_PRODUCTION] || m_aiYield[YIELD_FOOD] || m_aiYield[YIELD_COMMERCE] || m_aiCommerce[COMMERCE_GOLD] || m_aiCommerce[COMMERCE_RESEARCH] || m_aiCommerce[COMMERCE_CULTURE] || m_aiCommerce[COMMERCE_ESPIONAGE] || m_iGPP)
	{
		// short circuit plot city as this method will be called for city plots most of the time
		CvCityAI* pCity = (CvCityAI*) kPlot.getPlotCity();
		if (!pCity || (bToCoastalCity && (!pCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))))
			pCity = (CvCityAI*) GC.getMapINLINE().findCity(kPlot.getX_INLINE(), kPlot.getY_INLINE(), kUnit.getOwnerINLINE(), NO_TEAM, true, bToCoastalCity);
		if (!pCity)
			pCity = (CvCityAI*) GC.getMapINLINE().findCity(kPlot.getX_INLINE(), kPlot.getY_INLINE(), kUnit.getOwnerINLINE(), NO_TEAM, false, bToCoastalCity);

		if (pCity)
		{
			if (m_aiYield[YIELD_PRODUCTION] || m_aiYield[YIELD_FOOD] || m_aiYield[YIELD_COMMERCE] || m_aiCommerce[COMMERCE_GOLD] || m_aiCommerce[COMMERCE_RESEARCH] || m_aiCommerce[COMMERCE_CULTURE] || m_aiCommerce[COMMERCE_ESPIONAGE])
			{
				// We need shorts and not ints
				short aiYields[NUM_YIELD_TYPES];
				short aiCommerce[NUM_COMMERCE_TYPES];
				for (int i=0; i<NUM_YIELD_TYPES; i++)
					aiYields[i] = (short)m_aiYield[i];
				for (int i=0; i<NUM_COMMERCE_TYPES; i++)
					aiCommerce[i] = (short)m_aiCommerce[i];
				iValue += pCity->AI_yieldValue(aiYields, aiCommerce, false, false);
			}

			if (m_iGPP)
			{
				// Currently there is no use of that feature so go the easy way. If it is used more, code similar to AI_specialistValue should be used
				iValue += m_iGPP * 4;
			}

			if (m_iHappinessTimer)
			{
				if (pCity->happyLevel() - pCity->unhappyLevel(1) < 0)
				{
					iValue += m_iHappinessTimer * 10;
				}
			}
		}

	}


	return iValue;
}

void CvOutcome::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eType);
	pStream->Read(&m_iChance);
	pStream->Read(&m_iChancePerPop);
	pStream->Read((int*)&m_eUnitType);
	pStream->Read(&m_bUnitToCity);
	pStream->Read((int*)&m_ePromotionType);
	pStream->Read((int*)&m_eBonusType);
	pStream->Read(&m_iGPP);
	pStream->Read((int*)&m_eGPUnitType);
	pStream->Read(NUM_YIELD_TYPES, m_aiYield);
	pStream->Read(NUM_COMMERCE_TYPES, m_aiCommerce);
	pStream->Read(&m_iHappinessTimer);
	pStream->Read((int*)&m_eEventTrigger);
	m_Properties.read(pStream);
}

void CvOutcome::write(FDataStreamBase *pStream)
{
	pStream->Write((int)m_eType);
	pStream->Write(m_iChance);
	pStream->Write(m_iChancePerPop);
	pStream->Write((int)m_eUnitType);
	pStream->Write(m_bUnitToCity);
	pStream->Write((int)m_ePromotionType);
	pStream->Write((int)m_eBonusType);
	pStream->Write(m_iGPP);
	pStream->Write((int)m_eGPUnitType);
	pStream->Write(NUM_YIELD_TYPES, m_aiYield);
	pStream->Write(NUM_COMMERCE_TYPES, m_aiCommerce);
	pStream->Write(m_iHappinessTimer);
	pStream->Write((int)m_eEventTrigger);
	m_Properties.write(pStream);
}

bool CvOutcome::read(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	pXML->GetChildXmlValByName(szTextVal, "OutcomeType");
	m_eType = (OutcomeTypes) pXML->FindInInfoClass(szTextVal);
	pXML->GetChildXmlValByName(&m_iChance, "iChance");
	pXML->GetChildXmlValByName(&m_iChancePerPop, "iChancePerPop");
	pXML->GetChildXmlValByName(szTextVal, "UnitType");
	m_eUnitType = (UnitTypes) pXML->FindInInfoClass(szTextVal);
	pXML->GetChildXmlValByName(&m_bUnitToCity, "bUnitToCity", false);
	pXML->GetChildXmlValByName(szTextVal, "PromotionType");
	m_ePromotionType = (PromotionTypes) pXML->FindInInfoClass(szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "BonusType");
	m_eBonusType = (BonusTypes) pXML->FindInInfoClass(szTextVal);
	pXML->GetChildXmlValByName(&m_iGPP, "iGPP");
	pXML->GetChildXmlValByName(szTextVal, "GPUnitType");
	m_eGPUnitType = (UnitTypes) pXML->FindInInfoClass(szTextVal);
	pXML->GetChildXmlValByName(&m_iHappinessTimer, "iHappinessTimer");
	pXML->GetChildXmlValByName(szTextVal, "EventTrigger");
	GC.addDelayedResolution((int*)&m_eEventTrigger, szTextVal);
	
	if(gDLL->getXMLIFace()->SetToChildByTagName( pXML->GetXML(), "Yields"))
	{
		if(gDLL->getXMLIFace()->SetToChild( pXML->GetXML() ))
		{

			if (gDLL->getXMLIFace()->LocateFirstSiblingNodeByTagName(pXML->GetXML(), "iYield"))
			{
				int iI = 0;
				do
				{
					pXML->GetXmlVal(&m_aiYield[iI]);
					iI++;
				} while(gDLL->getXMLIFace()->NextSibling(pXML->GetXML()));
			}
			gDLL->getXMLIFace()->SetToParent( pXML->GetXML() );
		}
		gDLL->getXMLIFace()->SetToParent( pXML->GetXML() );
	}

	if(gDLL->getXMLIFace()->SetToChildByTagName( pXML->GetXML(), "Commerces"))
	{
		if(gDLL->getXMLIFace()->SetToChild( pXML->GetXML() ))
		{

			if (gDLL->getXMLIFace()->LocateFirstSiblingNodeByTagName(pXML->GetXML(), "iCommerce"))
			{
				int iI = 0;
				do
				{
					pXML->GetXmlVal(&m_aiCommerce[iI]);
					iI++;
				} while(gDLL->getXMLIFace()->NextSibling(pXML->GetXML()));
			}
			gDLL->getXMLIFace()->SetToParent( pXML->GetXML() );
		}
		gDLL->getXMLIFace()->SetToParent( pXML->GetXML() );
	}

	m_Properties.read(pXML);

	return true;
}

void CvOutcome::copyNonDefaults(CvOutcome* pOutcome, CvXMLLoadUtility* pXML )
{
	if (m_eType == NO_OUTCOME)
	{
		m_eType = pOutcome->getType();
	}
	if (m_iChance == 0)
	{
		m_iChance = pOutcome->m_iChance;
	}
	if (m_eUnitType == NO_UNIT)
	{
		m_eUnitType = pOutcome->getUnitType();
	}
	if (!m_bUnitToCity)
	{
		m_bUnitToCity = pOutcome->getUnitToCity();
	}
	if (m_ePromotionType == NO_PROMOTION)
	{
		m_ePromotionType = pOutcome->getPromotionType();
	}
	if (m_eBonusType == NO_BONUS)
	{
		m_eBonusType = pOutcome->getBonusType();
	}
	if (m_iGPP == 0)
	{
		m_iGPP = pOutcome->getGPP();
		m_eGPUnitType = pOutcome->getGPUnitType();
	}
	if (m_iHappinessTimer == 0)
	{
		m_iHappinessTimer = pOutcome->getHappinessTimer();
	}

	GC.copyNonDefaultDelayedResolution((int*)&m_eEventTrigger,(int*)&(pOutcome->m_eEventTrigger));

	bool bDefault = true;
	for (int i=0; i<NUM_YIELD_TYPES; i++)
	{
		bDefault = bDefault && (m_aiYield[i] == 0);
	}
	if (bDefault)
	{
		for (int i=0; i<NUM_YIELD_TYPES; i++)
		{
			m_aiYield[i] = pOutcome->m_aiYield[i];
		}
	}
	bDefault = true;
	for (int i=0; i<NUM_COMMERCE_TYPES; i++)
	{
		bDefault = bDefault && (m_aiCommerce[i] == 0);
	}
	if (bDefault)
	{
		for (int i=0; i<NUM_COMMERCE_TYPES; i++)
		{
			m_aiCommerce[i] = pOutcome->m_aiCommerce[i];
		}
	}

	m_Properties.copyNonDefaults(pOutcome->getProperties(), pXML);
}

void CvOutcome::buildDisplayString(CvWStringBuffer &szBuffer, const CvUnit& kUnit) const
{

	CvPlayer& kPlayer = GET_PLAYER(kUnit.getOwnerINLINE());
	bool bToCoastalCity = GC.getOutcomeInfo(getType()).getToCoastalCity();
	CvUnitInfo* pUnitInfo = &kUnit.getUnitInfo();

	szBuffer.append(GC.getOutcomeInfo(getType()).getText());
	szBuffer.append(L" ( ");

	bool bFirst = true;

	if (m_ePromotionType > NO_PROMOTION)
	{
		if (!bFirst)
		{
			szBuffer.append(L", ");
		}
		else
		{
			bFirst = false;
		}
		szBuffer.append(GC.getPromotionInfo(m_ePromotionType).getDescription());
	}

	if (m_eUnitType > NO_UNIT && !m_bUnitToCity)
	{
		if (!bFirst)
		{
			szBuffer.append(L", ");
		}
		else
		{
			bFirst = false;
		}
		szBuffer.append(GC.getUnitInfo(m_eUnitType).getDescription());
	}

	if (m_aiYield[YIELD_PRODUCTION] || m_aiYield[YIELD_FOOD] || m_aiCommerce[COMMERCE_CULTURE] || m_iGPP || (m_bUnitToCity && m_eUnitType > NO_UNIT))
	{
		if (!bFirst)
		{
			szBuffer.append(L", ");
		}
		else
		{
			bFirst = false;
		}
		if (m_aiYield[YIELD_PRODUCTION])
		{
			CvWString szTemp;
			szTemp.Format(L" %d%c", m_aiYield[YIELD_PRODUCTION], GC.getYieldInfo(YIELD_PRODUCTION).getChar());
			szBuffer.append(szTemp);
		}
		if (m_aiYield[YIELD_FOOD])
		{
			CvWString szTemp;
			szTemp.Format(L" %d%c", m_aiYield[YIELD_FOOD], GC.getYieldInfo(YIELD_FOOD).getChar());
			szBuffer.append(szTemp);
		}

		if (m_aiCommerce[COMMERCE_CULTURE])
		{
			CvWString szTemp;
			szTemp.Format(L" %d%c", m_aiCommerce[COMMERCE_CULTURE], GC.getCommerceInfo(COMMERCE_CULTURE).getChar());
			szBuffer.append(szTemp);
		}

		if (m_iGPP)
		{
			CvWString szTemp;
			szTemp.Format(L" %d%c", m_iGPP, gDLL->getSymbolID(GREAT_PEOPLE_CHAR));
			szBuffer.append(szTemp);
		}

		if (!m_Properties.isEmpty())
		{
			m_Properties.buildCompactChangesString(szBuffer);
		}

		if (m_iHappinessTimer)
		{
			int iHappy = GC.getDefineINT("TEMP_HAPPY");
			szBuffer.append(L" ");
			szBuffer.append(gDLL->getText("TXT_KEY_OUTCOME_TEMP_HAPPY", iHappy, m_iHappinessTimer));
		}

		if (m_bUnitToCity && m_eUnitType > NO_UNIT)
		{
			szBuffer.append(L" ");
			szBuffer.append(GC.getUnitInfo(m_eUnitType).getDescription());
		}

		if (bToCoastalCity)
			szBuffer.append(L" to nearest coastal city");
		else
			szBuffer.append(L" to nearest city");
	}

	int iGoldTimes100 = 0;
	int iResearchTimes100 = 0;

	if (m_aiYield[YIELD_COMMERCE])
	{
		iGoldTimes100 = m_aiYield[YIELD_COMMERCE] * kPlayer.getCommercePercent(COMMERCE_GOLD);
		iResearchTimes100 = m_aiYield[YIELD_COMMERCE] * kPlayer.getCommercePercent(COMMERCE_RESEARCH);
	}

	iGoldTimes100 += m_aiCommerce[COMMERCE_GOLD] * 100;
	iResearchTimes100 += m_aiCommerce[COMMERCE_RESEARCH] * 100;
	
	if (iGoldTimes100)
	{
		if (!bFirst)
		{
			szBuffer.append(L", ");
		}
		else
		{
			bFirst = false;
		}
		CvWString szTemp;
		szTemp.Format(L" %d%c", iGoldTimes100 / 100, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
		szBuffer.append(szTemp);
	}
	CvTeam& kTeam = GET_TEAM(kUnit.getTeam());
	if (iResearchTimes100)
	{
		if (!bFirst)
		{
			szBuffer.append(L", ");
		}
		else
		{
			bFirst = false;
		}
		CvWString szTemp;
		szTemp.Format(L" %d%c", iResearchTimes100 / 100, GC.getCommerceInfo(COMMERCE_RESEARCH).getChar());
		szBuffer.append(szTemp);
	}

	if (m_eBonusType != NO_BONUS)
	{
		if (!bFirst)
		{
			szBuffer.append(L", ");
		}
		else
		{
			bFirst = false;
		}
		szBuffer.append(GC.getBonusInfo(m_eBonusType).getDescription());
	}

	if (m_eEventTrigger != NO_EVENTTRIGGER)
	{
		if (!bFirst)
		{
			szBuffer.append(L", ");
		}
		else
		{
			bFirst = false;
		}
		szBuffer.append(GC.getEventTriggerInfo(m_eEventTrigger).getDescription());
	}

	szBuffer.append(L" )");
}