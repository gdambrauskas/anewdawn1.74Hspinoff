// playerAI.cpp

#include "CvGameCoreDLL.h"
#include "CvPlayerAI.h"
#include "CvRandom.h"
#include "CvGlobals.h"
#include "CvGameCoreUtils.h"
#include "CvMap.h"
#include "CvArea.h"
#include "CvPlot.h"
#include "CvGameAI.h"
#include "CvTeamAI.h"
#include "CvGameCoreUtils.h"
#include "CvDiploParameters.h"
#include "CvInitCore.h"
#include "CyArgsList.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvDLLEngineIFaceBase.h"
#include "CvInfos.h"
#include "CvPopupInfo.h"
#include "FProfiler.h"
#include "CvDLLFAStarIFaceBase.h"
#include "FAStarNode.h"
#include "CvEventReporter.h"

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/02/09                                jdog5000      */
/*                                                                                              */
/* AI logging                                                                                   */
/************************************************************************************************/
#include "BetterBTSAI.h"
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/21/09                                jdog5000      */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
// Plot danger cache
//#define DANGER_RANGE						(4)
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

#define GREATER_FOUND_RANGE			(10)
#define CIVIC_CHANGE_DELAY			(25)
#define RELIGION_CHANGE_DELAY		(15)

// statics

CvPlayerAI* CvPlayerAI::m_aPlayers = NULL;

void CvPlayerAI::initStatics()
{
	m_aPlayers = new CvPlayerAI[MAX_PLAYERS];
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_aPlayers[iI].m_eID = ((PlayerTypes)iI);
	}
}

void CvPlayerAI::freeStatics()
{
	SAFE_DELETE_ARRAY(m_aPlayers);
}

bool CvPlayerAI::areStaticsInitialized()
{
	if(m_aPlayers == NULL)
	{
		return false;
	}

	return true;
}

DllExport CvPlayerAI& CvPlayerAI::getPlayerNonInl(PlayerTypes ePlayer)
{
	return getPlayer(ePlayer);
}

// Public Functions...

CvPlayerAI::CvPlayerAI()
{
	m_aiNumTrainAIUnits = new int[NUM_UNITAI_TYPES];
	m_aiNumAIUnits = new int[NUM_UNITAI_TYPES];
	m_aiSameReligionCounter = new int[MAX_PLAYERS];
	m_aiDifferentReligionCounter = new int[MAX_PLAYERS];
	m_aiFavoriteCivicCounter = new int[MAX_PLAYERS];
	m_aiBonusTradeCounter = new int[MAX_PLAYERS];
	m_aiPeacetimeTradeValue = new int[MAX_PLAYERS];
	m_aiPeacetimeGrantValue = new int[MAX_PLAYERS];
	m_aiGoldTradedTo = new int[MAX_PLAYERS];
	m_aiAttitudeExtra = new int[MAX_PLAYERS];

	m_abFirstContact = new bool[MAX_PLAYERS];

	m_aaiContactTimer = new int*[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		m_aaiContactTimer[i] = new int[NUM_CONTACT_TYPES];
	}

	m_aaiMemoryCount = new int*[MAX_PLAYERS];
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		m_aaiMemoryCount[i] = new int[NUM_MEMORY_TYPES];
	}

	m_aiAverageYieldMultiplier = new int[NUM_YIELD_TYPES];
	m_aiAverageCommerceMultiplier = new int[NUM_COMMERCE_TYPES];
	m_aiAverageCommerceExchange = new int[NUM_COMMERCE_TYPES];
	
	m_aiBonusValue = NULL;
	m_aiUnitClassWeights = NULL;
	m_aiUnitCombatWeights = NULL;
	m_aiCloseBordersAttitudeCache = new int[MAX_PLAYERS];

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/03/09                       poyuzhe & jdog5000     */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
	// From Sanguo Mod Performance, ie the CAR Mod
	// Attitude cache
	m_aiAttitudeCache = new int[MAX_PLAYERS];
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	AI_reset(true);
}


CvPlayerAI::~CvPlayerAI()
{
	AI_uninit();

	SAFE_DELETE_ARRAY(m_aiNumTrainAIUnits);
	SAFE_DELETE_ARRAY(m_aiNumAIUnits);
	SAFE_DELETE_ARRAY(m_aiSameReligionCounter);
	SAFE_DELETE_ARRAY(m_aiDifferentReligionCounter);
	SAFE_DELETE_ARRAY(m_aiFavoriteCivicCounter);
	SAFE_DELETE_ARRAY(m_aiBonusTradeCounter);
	SAFE_DELETE_ARRAY(m_aiPeacetimeTradeValue);
	SAFE_DELETE_ARRAY(m_aiPeacetimeGrantValue);
	SAFE_DELETE_ARRAY(m_aiGoldTradedTo);
	SAFE_DELETE_ARRAY(m_aiAttitudeExtra);
	SAFE_DELETE_ARRAY(m_abFirstContact);
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		SAFE_DELETE_ARRAY(m_aaiContactTimer[i]);
	}
	SAFE_DELETE_ARRAY(m_aaiContactTimer);

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		SAFE_DELETE_ARRAY(m_aaiMemoryCount[i]);
	}
	SAFE_DELETE_ARRAY(m_aaiMemoryCount);
	
	SAFE_DELETE_ARRAY(m_aiAverageYieldMultiplier);
	SAFE_DELETE_ARRAY(m_aiAverageCommerceMultiplier);
	SAFE_DELETE_ARRAY(m_aiAverageCommerceExchange);
	SAFE_DELETE_ARRAY(m_aiCloseBordersAttitudeCache);

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/03/09                       poyuzhe & jdog5000     */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
	// From Sanguo Mod Performance, ie the CAR Mod
	// Attitude cache
	SAFE_DELETE_ARRAY(m_aiAttitudeCache);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

}


void CvPlayerAI::AI_init()
{
	AI_reset(false);

	//--------------------------------
	// Init other game data
	if ((GC.getInitCore().getSlotStatus(getID()) == SS_TAKEN) || (GC.getInitCore().getSlotStatus(getID()) == SS_COMPUTER))
	{
		FAssert(getPersonalityType() != NO_LEADER);
		AI_setPeaceWeight(GC.getLeaderHeadInfo(getPersonalityType()).getBasePeaceWeight() + GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getPeaceWeightRand(), "AI Peace Weight"));
		AI_setEspionageWeight(GC.getLeaderHeadInfo(getPersonalityType()).getEspionageWeight());
		//AI_setCivicTimer(((getMaxAnarchyTurns() == 0) ? (GC.getDefineINT("MIN_REVOLUTION_TURNS") * 2) : CIVIC_CHANGE_DELAY) / 2);
		AI_setReligionTimer(1);
		AI_setCivicTimer((getMaxAnarchyTurns() == 0) ? 1 : 2);
	}
}


void CvPlayerAI::AI_uninit()
{
	SAFE_DELETE_ARRAY(m_aiBonusValue);
	SAFE_DELETE_ARRAY(m_aiUnitClassWeights);
	SAFE_DELETE_ARRAY(m_aiUnitCombatWeights);
}


void CvPlayerAI::AI_reset(bool bConstructor)
{
	int iI;

	AI_uninit();

	m_iPeaceWeight = 0;
	m_iEspionageWeight = 0;
	m_iAttackOddsChange = 0;
	m_iCivicTimer = 0;
	m_iReligionTimer = 0;
	m_iExtraGoldTarget = 0;

/************************************************************************************************/
/* CHANGE_PLAYER                         06/08/09                                 jdog5000      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if( bConstructor || getNumUnits() == 0 )
	{
		for (iI = 0; iI < NUM_UNITAI_TYPES; iI++)
		{
			m_aiNumTrainAIUnits[iI] = 0;
			m_aiNumAIUnits[iI] = 0;
		}
	}
/************************************************************************************************/
/* CHANGE_PLAYER                           END                                                  */
/************************************************************************************************/

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_aiSameReligionCounter[iI] = 0;
		m_aiDifferentReligionCounter[iI] = 0;
		m_aiFavoriteCivicCounter[iI] = 0;
		m_aiBonusTradeCounter[iI] = 0;
		m_aiPeacetimeTradeValue[iI] = 0;
		m_aiPeacetimeGrantValue[iI] = 0;
		m_aiGoldTradedTo[iI] = 0;
		m_aiAttitudeExtra[iI] = 0;
		m_abFirstContact[iI] = false;
		for (int iJ = 0; iJ < NUM_CONTACT_TYPES; iJ++)
		{
			m_aaiContactTimer[iI][iJ] = 0;
		}
		for (int iJ = 0; iJ < NUM_MEMORY_TYPES; iJ++)
		{
			m_aaiMemoryCount[iI][iJ] = 0;
		}

		if (!bConstructor && getID() != NO_PLAYER)
		{
			PlayerTypes eLoopPlayer = (PlayerTypes) iI;
			CvPlayerAI& kLoopPlayer = GET_PLAYER(eLoopPlayer);
			kLoopPlayer.m_aiSameReligionCounter[getID()] = 0;
			kLoopPlayer.m_aiDifferentReligionCounter[getID()] = 0;
			kLoopPlayer.m_aiFavoriteCivicCounter[getID()] = 0;
			kLoopPlayer.m_aiBonusTradeCounter[getID()] = 0;
			kLoopPlayer.m_aiPeacetimeTradeValue[getID()] = 0;
			kLoopPlayer.m_aiPeacetimeGrantValue[getID()] = 0;
			kLoopPlayer.m_aiGoldTradedTo[getID()] = 0;
			kLoopPlayer.m_aiAttitudeExtra[getID()] = 0;
			kLoopPlayer.m_abFirstContact[getID()] = false;
			for (int iJ = 0; iJ < NUM_CONTACT_TYPES; iJ++)
			{
				kLoopPlayer.m_aaiContactTimer[getID()][iJ] = 0;
			}
			for (int iJ = 0; iJ < NUM_MEMORY_TYPES; iJ++)
			{
				kLoopPlayer.m_aaiMemoryCount[getID()][iJ] = 0;
			}
		}
	}
	
	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiAverageYieldMultiplier[iI] = 0;
	}
	for (iI = 0; iI< NUM_COMMERCE_TYPES; iI++)
	{
		m_aiAverageCommerceMultiplier[iI] = 0;
		m_aiAverageCommerceExchange[iI] = 0;
	}
	m_iAverageGreatPeopleMultiplier = 0;
	m_iAveragesCacheTurn = -1;
	
	m_iStrategyHash = 0;
	m_iStrategyHashCacheTurn = -1;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/18/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
	m_iStrategyRand = 0;
	m_iVictoryStrategyHash = 0;
	m_iVictoryStrategyHashCacheTurn = -1;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/		
/************************************************************************************************/
/* RevDCM	                  Start		 12/9/09                                                */
/*                                                                                              */
/* Inquisitions                                                                                 */
/************************************************************************************************/
	m_bPushReligiousVictory = false;
	m_bConsiderReligiousVictory = false;
	m_bHasInquisitionTarget = false;
/************************************************************************************************/
/* RevDCM	                     END                                                            */
/************************************************************************************************/
	m_bWasFinancialTrouble = false;
	m_iTurnLastProductionDirty = -1;

	m_iUpgradeUnitsCacheTurn = -1;
	m_iUpgradeUnitsCachedExpThreshold = 0;
	m_iUpgradeUnitsCachedGold = 0;

	m_aiAICitySites.clear();
	
	FAssert(m_aiBonusValue == NULL);
	m_aiBonusValue = new int[GC.getNumBonusInfos()];
	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		m_aiBonusValue[iI] = -1;		
	}
	
	FAssert(m_aiUnitClassWeights == NULL);
	m_aiUnitClassWeights = new int[GC.getNumUnitClassInfos()];
	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		m_aiUnitClassWeights[iI] = 0;		
	}

	FAssert(m_aiUnitCombatWeights == NULL);
	m_aiUnitCombatWeights = new int[GC.getNumUnitCombatInfos()];
	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		m_aiUnitCombatWeights[iI] = 0;		
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		m_aiCloseBordersAttitudeCache[iI] = 0;

		if (!bConstructor && getID() != NO_PLAYER)
		{
			GET_PLAYER((PlayerTypes) iI).m_aiCloseBordersAttitudeCache[getID()] = 0;
		}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/03/09                       poyuzhe & jdog5000     */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
		// From Sanguo Mod Performance, ie the CAR Mod
		// Attitude cache
		m_aiAttitudeCache[iI] = MAX_INT;

		if (!bConstructor && getID() != NO_PLAYER)
		{
			GET_PLAYER((PlayerTypes) iI).m_aiAttitudeCache[getID()] = MAX_INT;
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	}
}


int CvPlayerAI::AI_getFlavorValue(FlavorTypes eFlavor) const
{
	FAssertMsg((getPersonalityType() >= 0), "getPersonalityType() is less than zero");
	FAssertMsg((eFlavor >= 0), "eFlavor is less than zero");
	return GC.getLeaderHeadInfo(getPersonalityType()).getFlavorValue(eFlavor);
}


void CvPlayerAI::AI_doTurnPre()
{
	PROFILE_FUNC();

	FAssertMsg(getPersonalityType() != NO_LEADER, "getPersonalityType() is not expected to be equal with NO_LEADER");
	FAssertMsg(getLeaderType() != NO_LEADER, "getLeaderType() is not expected to be equal with NO_LEADER");
	FAssertMsg(getCivilizationType() != NO_CIVILIZATION, "getCivilizationType() is not expected to be equal with NO_CIVILIZATION");

	AI_invalidateCloseBordersAttitudeCache();

	AI_doCounter();

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/03/09                       poyuzhe & jdog5000     */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
	// From Sanguo Mod Performance, ie the CAR Mod
	// Attitude cache
	AI_invalidateAttitudeCache();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	AI_updateBonusValue();
	
	AI_doEnemyUnitData();
	
	if (isHuman())
	{
		return;
	}

	AI_doResearch();

	AI_doCommerce();

	AI_doMilitary();

	AI_doCivics();

	AI_doReligion();

/************************************************************************************************/
/* RevDCM	                  Start		 12/9/09                                                */
/*                                                                                              */
/* Inquisitions                                                                                 */
/************************************************************************************************/
	AI_setPushReligiousVictory();
	AI_setConsiderReligiousVictory();
	AI_setHasInquisitionTarget();
/************************************************************************************************/
/* RevDCM	                     END                                                            */
/************************************************************************************************/

	AI_doCheckFinancialTrouble();

	if (isBarbarian())
	{
		return;
	}

	if (isMinorCiv())
	{
		return;
	}
}


void CvPlayerAI::AI_doTurnPost()
{
	PROFILE_FUNC();

	if (isHuman())
	{
		return;
	}

	if (isBarbarian())
	{
		return;
	}

	if (isMinorCiv())
	{
		return;
	}

	AI_doDiplo();
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       06/16/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
	// Moved per alexman's suggestion
	//AI_doSplit();	
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

	for (int i = 0; i < GC.getNumVictoryInfos(); ++i)
	{
		AI_launch((VictoryTypes)i);
	}
}


void CvPlayerAI::AI_doTurnUnitsPre()
{
	PROFILE_FUNC();

	AI_updateFoundValues();

	if (GC.getGameINLINE().getSorenRandNum(8, "AI Update Area Targets") == 0) // XXX personality???
	{
		AI_updateAreaTargets();
	}

	if (isHuman())
	{
		return;
	}

	if (isBarbarian())
	{
		return;
	}
	
//	if (AI_isDoStrategy(AI_STRATEGY_CRUSH))
//	{
//		AI_convertUnitAITypesForCrush();		
//	}
}


void CvPlayerAI::AI_doTurnUnitsPost()
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	CvPlot* pUnitPlot;
	bool bValid;
	int iPass;
	int iLoop;

	if (!isHuman() || isOption(PLAYEROPTION_AUTO_PROMOTION))
	{
		for(pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
		{
/************************************************************************************************/
/* Afforess	                  Start		 06/24/10                                               */
/*                                                                                              */
/* Afforess Speed Tweak                                                                         */
/************************************************************************************************/
			if (pLoopUnit->isPromotionReady())
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			pLoopUnit->AI_promote();
		}
	}

	if (isHuman())
	{
		return;
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/24/10                                jdog5000      */
/*                                                                                              */
/* Gold AI                                                                                      */
/************************************************************************************************/
	bool bAnyWar = (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0);
	int iStartingGold = getGold();
	int iTargetGold = AI_goldTarget();
	int iUpgradeBudget = (AI_goldToUpgradeAllUnits() / (bAnyWar ? 1 : 2));

	iUpgradeBudget = std::min(iUpgradeBudget, iStartingGold - ((iTargetGold > iUpgradeBudget) ? (iTargetGold - iUpgradeBudget) : iStartingGold/2));

	if( AI_isFinancialTrouble() )
	{
		iUpgradeBudget /= 3;
	}

	// Always willing to upgrade 1 unit if we have the money
	iUpgradeBudget = std::max(iUpgradeBudget,1);

	bool bUnderBudget = true;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	CvPlot* pLastUpgradePlot = NULL;
	for (iPass = 0; iPass < 4; iPass++)
	{
		for(pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
		{
			bool bNoDisband = false;
			bValid = false;

			switch (iPass)
			{
			case 0:
				// BBAI note:  Effectively only for galleys, triremes, and ironclads ... unit types which are limited in
				// what terrain they can operate in
				if (AI_unitImpassableCount(pLoopUnit->getUnitType()) > 0)
				{
					bValid = true;
				}
				break;
			case 1:
				pUnitPlot = pLoopUnit->plot();
				if (pUnitPlot->isCity())
				{
					if (pUnitPlot->getBestDefender(getID()) == pLoopUnit)
					{
						bNoDisband = true;
						bValid = true;
						pLastUpgradePlot = pUnitPlot;
					}

					// try to upgrade units which are in danger... but don't get obsessed
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/20/09                                jdog5000      */
/*                                                                                              */
/* Unit AI, Efficiency                                                                          */
/************************************************************************************************/
					//if (!bValid && (pLastUpgradePlot != pUnitPlot) && ((AI_getPlotDanger(pUnitPlot, 1, false)) > 0))
					if (!bValid && (pLastUpgradePlot != pUnitPlot) && ((AI_getAnyPlotDanger(pUnitPlot, 1, false))))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/	
					{
						bNoDisband = true;
						bValid = true;
						pLastUpgradePlot = pUnitPlot;
					}
				}
				break;
			case 2:
				/********************************************************************************/
				/* 	BETTER_BTS_AI_MOD						9/15/08			jdog5000		*/
				/* 																			*/
				/* 	Gold AI																	*/
				/********************************************************************************/
				/* original BTS code
				if (pLoopUnit->cargoSpace() > 0)
				{
					bValid = true;
				}
				*/
				bUnderBudget = (iStartingGold - getGold()) < iUpgradeBudget;

				// Only normal transports
				if ( (pLoopUnit->cargoSpace() > 0) && (pLoopUnit->specialCargo() == NO_SPECIALUNIT) )
				{
					bValid = (bAnyWar || bUnderBudget);
				}
				// Also upgrade escort ships
				if ( pLoopUnit->AI_getUnitAIType() == UNITAI_ESCORT_SEA )
				{
					bValid = (bAnyWar || bUnderBudget);
				}
				/********************************************************************************/
				/* 	BETTER_BTS_AI_MOD						END								*/
				/********************************************************************************/
				
				break;
			case 3:
				/********************************************************************************/
				/* 	BETTER_BTS_AI_MOD						9/15/08			jdog5000		*/
				/* 																			*/
				/* 	Gold AI																	*/
				/********************************************************************************/
				/* original BTS code
				bValid = true;
				*/
				bUnderBudget = (iStartingGold - getGold()) < iUpgradeBudget;

				bValid = (bAnyWar || bUnderBudget);
				/********************************************************************************/
				/* 	BETTER_BTS_AI_MOD						END								*/
				/********************************************************************************/
				
				break;
			default:
				FAssert(false);
				break;
			}

			if (bValid)
			{
				bool bKilled = false;
				if (!bNoDisband)
				{
					// RevolutionDCM start
					if (pLoopUnit->canFight() && !(pLoopUnit->isAnimal()))
					// RevolutionDCM end
					{
						int iExp = pLoopUnit->getExperience();
						CvCity* pPlotCity = pLoopUnit->plot()->getPlotCity();
						if (pPlotCity != NULL && pPlotCity->getOwnerINLINE() == getID())
						{
							int iCityExp = 0;
							iCityExp += pPlotCity->getFreeExperience();
							iCityExp += pPlotCity->getDomainFreeExperience(pLoopUnit->getDomainType());
							iCityExp += pPlotCity->getUnitCombatFreeExperience(pLoopUnit->getUnitCombatType());
							if (iCityExp > 0)
							{
								if ((iExp == 0) || (iExp < (iCityExp + 1) / 2))
								{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/20/09                                jdog5000      */
/*                                                                                              */
/* Unit AI, Efficiency                                                                          */
/************************************************************************************************/
									if ((pLoopUnit->getDomainType() != DOMAIN_LAND) || pLoopUnit->plot()->plotCount(PUF_isMilitaryHappiness, -1, -1, getID()) > 1)
									{
										//if ((calculateUnitCost() > 0) && (AI_getPlotDanger( pLoopUnit->plot(), 2, false) == 0))
										if ((calculateUnitCost() > 0) && (AI_getAnyPlotDanger( pLoopUnit->plot(), 2, false)))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/	
										{
										
											pLoopUnit->kill(false);
											bKilled = true;
											pLastUpgradePlot = NULL;
										}
									}
								}
							}
						}
					}
				}
				if (!bKilled)
				{
					pLoopUnit->AI_upgrade(); // CAN DELETE UNIT!!!
				}
			}
		}
	}

	if (isBarbarian())
	{
		return;
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/24/10                                jdog5000      */
/*                                                                                              */
/* AI Logging                                                                                   */
/************************************************************************************************/
	if( gPlayerLogLevel > 2 )
	{
		if( iStartingGold - getGold() > 0 )
		{
			logBBAI("    %S spends %d on unit upgrades out of budget of %d, %d gold remaining", getCivilizationDescription(0), iStartingGold - getGold(), iUpgradeBudget, getGold());
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       06/16/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
	// Moved here per alexman's suggestion
	AI_doSplit();	
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

}


void CvPlayerAI::AI_doPeace()
{
	PROFILE_FUNC();

	CvDiploParameters* pDiplo;
	CvCity* pBestReceiveCity;
	CvCity* pBestGiveCity;
	CvCity* pLoopCity;
	CLinkList<TradeData> ourList;
	CLinkList<TradeData> theirList;
	bool abContacted[MAX_TEAMS];
	TradeData item;
	TechTypes eBestReceiveTech;
	TechTypes eBestGiveTech;
	int iReceiveGold;
	int iGiveGold;
	int iGold;
	int iValue;
	int iBestValue;
	int iOurValue;
	int iTheirValue;
	int iLoop;
	int iI, iJ;

	FAssert(!isHuman());
	FAssert(!isMinorCiv());
	FAssert(!isBarbarian());

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		abContacted[iI] = false;
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (iI != getID())
			{
				if (canContact((PlayerTypes)iI) && AI_isWillingToTalk((PlayerTypes)iI))
				{
					if (!(GET_TEAM(getTeam()).isHuman()) && (GET_PLAYER((PlayerTypes)iI).isHuman() || !(GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHuman())))
					{
						if (GET_TEAM(getTeam()).isAtWar(GET_PLAYER((PlayerTypes)iI).getTeam()))
						{
							if (!(GET_PLAYER((PlayerTypes)iI).isHuman()) || (GET_TEAM(getTeam()).getLeaderID() == getID()))
							{
								FAssertMsg(!(GET_PLAYER((PlayerTypes)iI).isBarbarian()), "(GET_PLAYER((PlayerTypes)iI).isBarbarian()) did not return false as expected");
								FAssertMsg(iI != getID(), "iI is not expected to be equal with getID()");
								FAssert(GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam());

/************************************************************************************************/
/* Afforess	                  Start		 04/06/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
								bool bConsiderPeace;
								if (GC.getGameINLINE().isOption(GAMEOPTION_ADVANCED_DIPLOMACY))
								{
									bConsiderPeace = ((GET_TEAM(getTeam()).AI_getAtWarCounter(GET_PLAYER((PlayerTypes)iI).getTeam()) > 10) || (GET_TEAM(getTeam()).getAtWarCount(false, true) > 1) ||
														(GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_getWarSuccess(getTeam()) > (GET_TEAM(getTeam()).AI_getWarSuccess(GET_PLAYER((PlayerTypes)iI).getTeam()) * 2)));
								}
								else
								{
									bConsiderPeace = (GET_TEAM(getTeam()).AI_getAtWarCounter(GET_PLAYER((PlayerTypes)iI).getTeam()) > 10);
								}
								if (bConsiderPeace)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
								{
									if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_PEACE_TREATY) == 0)
									{
										bool bOffered = false;

										setTradeItem(&item, TRADE_SURRENDER);

										if (canTradeItem((PlayerTypes)iI, item, true))
										{
											ourList.clear();
											theirList.clear();

											ourList.insertAtEnd(item);

											bOffered = true;

											if (GET_PLAYER((PlayerTypes)iI).isHuman())
											{
												if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
												{
													AI_changeContactTimer(((PlayerTypes)iI), CONTACT_PEACE_TREATY, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_PEACE_TREATY));
													pDiplo = new CvDiploParameters(getID());
													FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
													pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_PEACE"));
													pDiplo->setAIContact(true);
													pDiplo->setOurOfferList(theirList);
													pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
													// RevolutionDCM start - new diplomacy option
													AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
													// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
													// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/
													abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
												}
											}
											else
											{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      01/22/09                                jdog5000      */
/*                                                                                              */
/* War Strategy AI                                                                              */
/************************************************************************************************/
												if( GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_acceptSurrender(getTeam()) )
												{
													GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
												}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
											}
										}

										if (!bOffered)
										{
											if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_PEACE_TREATY), "AI Diplo Peace Treaty") == 0)
											{
												setTradeItem(&item, TRADE_PEACE_TREATY);

												if (canTradeItem(((PlayerTypes)iI), item, true) && GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
												{
													iOurValue = GET_TEAM(getTeam()).AI_endWarVal(GET_PLAYER((PlayerTypes)iI).getTeam());
													iTheirValue = GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_endWarVal(getTeam());

													eBestReceiveTech = NO_TECH;
													eBestGiveTech = NO_TECH;

													iReceiveGold = 0;
													iGiveGold = 0;

													pBestReceiveCity = NULL;
													pBestGiveCity = NULL;

													if (iTheirValue > iOurValue)
													{
														if (iTheirValue > iOurValue)
														{
															iBestValue = 0;

															for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
															{
																setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

																if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
																{
																	iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Peace Trading (Tech #1)"));

																	if (iValue > iBestValue)
																	{
																		iBestValue = iValue;
																		eBestReceiveTech = ((TechTypes)iJ);
																	}
																}
															}

															if (eBestReceiveTech != NO_TECH)
															{
																iOurValue += GET_TEAM(getTeam()).AI_techTradeVal(eBestReceiveTech, GET_PLAYER((PlayerTypes)iI).getTeam());
															}
														}

														iGold = std::min((iTheirValue - iOurValue), GET_PLAYER((PlayerTypes)iI).AI_maxGoldTrade(getID()));

														if (iGold > 0)
														{
															setTradeItem(&item, TRADE_GOLD, iGold);

															if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
															{
																iReceiveGold = iGold;
																iOurValue += iGold;
															}
														}

														if (iTheirValue > iOurValue)
														{
															iBestValue = 0;

															for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
															{
																setTradeItem(&item, TRADE_CITIES, pLoopCity->getID());

																if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
																{
																	iValue = pLoopCity->plot()->calculateCulturePercent(getID());

																	if (iValue > iBestValue)
																	{
																		iBestValue = iValue;
																		pBestReceiveCity = pLoopCity;
																	}
																}
															}

															if (pBestReceiveCity != NULL)
															{
																iOurValue += AI_cityTradeVal(pBestReceiveCity);
															}
														}
													}
													else if (iOurValue > iTheirValue)
													{
														iBestValue = 0;

														for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
														{
															setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

															if (canTradeItem(((PlayerTypes)iI), item, true))
															{
																if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_techTradeVal((TechTypes)iJ, getTeam()) <= (iOurValue - iTheirValue))
																{
																	iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Peace Trading (Tech #2)"));

																	if (iValue > iBestValue)
																	{
																		iBestValue = iValue;
																		eBestGiveTech = ((TechTypes)iJ);
																	}
																}
															}
														}

														if (eBestGiveTech != NO_TECH)
														{
															iTheirValue += GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_techTradeVal(eBestGiveTech, getTeam());
														}

														iGold = std::min((iOurValue - iTheirValue), AI_maxGoldTrade((PlayerTypes)iI));

														if (iGold > 0)
														{
															setTradeItem(&item, TRADE_GOLD, iGold);

															if (canTradeItem(((PlayerTypes)iI), item, true))
															{
																iGiveGold = iGold;
																iTheirValue += iGold;
															}
														}

														iBestValue = 0;

														for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
														{
															setTradeItem(&item, TRADE_CITIES, pLoopCity->getID());

															if (canTradeItem(((PlayerTypes)iI), item, true))
															{
																if (GET_PLAYER((PlayerTypes)iI).AI_cityTradeVal(pLoopCity) <= (iOurValue - iTheirValue))
																{
																	iValue = pLoopCity->plot()->calculateCulturePercent((PlayerTypes)iI);

																	if (iValue > iBestValue)
																	{
																		iBestValue = iValue;
																		pBestGiveCity = pLoopCity;
																	}
																}
															}
														}

														if (pBestGiveCity != NULL)
														{
															iTheirValue += GET_PLAYER((PlayerTypes)iI).AI_cityTradeVal(pBestGiveCity);
														}
													}

													if ((GET_PLAYER((PlayerTypes)iI).isHuman()) ? (iOurValue >= iTheirValue) : ((iOurValue > ((iTheirValue * 3) / 5)) && (iTheirValue > ((iOurValue * 3) / 5))))
													{
														ourList.clear();
														theirList.clear();

														setTradeItem(&item, TRADE_PEACE_TREATY);

														ourList.insertAtEnd(item);
														theirList.insertAtEnd(item);

														if (eBestGiveTech != NO_TECH)
														{
															setTradeItem(&item, TRADE_TECHNOLOGIES, eBestGiveTech);
															ourList.insertAtEnd(item);
														}

														if (eBestReceiveTech != NO_TECH)
														{
															setTradeItem(&item, TRADE_TECHNOLOGIES, eBestReceiveTech);
															theirList.insertAtEnd(item);
														}

														if (iGiveGold != 0)
														{
															setTradeItem(&item, TRADE_GOLD, iGiveGold);
															ourList.insertAtEnd(item);
														}

														if (iReceiveGold != 0)
														{
															setTradeItem(&item, TRADE_GOLD, iReceiveGold);
															theirList.insertAtEnd(item);
														}

														if (pBestGiveCity != NULL)
														{
															setTradeItem(&item, TRADE_CITIES, pBestGiveCity->getID());
															ourList.insertAtEnd(item);
														}

														if (pBestReceiveCity != NULL)
														{
															setTradeItem(&item, TRADE_CITIES, pBestReceiveCity->getID());
															theirList.insertAtEnd(item);
														}

														if (GET_PLAYER((PlayerTypes)iI).isHuman())
														{
															if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
															{
																AI_changeContactTimer(((PlayerTypes)iI), CONTACT_PEACE_TREATY, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_PEACE_TREATY));
																pDiplo = new CvDiploParameters(getID());
																FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_PEACE"));
																pDiplo->setAIContact(true);
																pDiplo->setOurOfferList(theirList);
																pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
																// RevolutionDCM start - new diplomacy option
																AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/
																abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
															}
														}
														else
														{
															GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


void CvPlayerAI::AI_updateFoundValues(bool bStartingLoc) const
{
	PROFILE_FUNC();

	CvArea* pLoopArea;
	CvPlot* pLoopPlot;
	int iValue;
	int iLoop;
	int iI;
	bool bCitySiteCalculations = (GC.getGame().getGameTurn() > GC.getGame().getStartTurn());
	

	for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
	{
		pLoopArea->setBestFoundValue(getID(), 0);
	}

	if (bStartingLoc)
	{
		for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
		{
			GC.getMapINLINE().plotByIndexINLINE(iI)->setFoundValue(getID(), -1);
		}
	}
	else
	{
		if (!isBarbarian())
		{
			AI_invalidateCitySites(AI_getMinFoundValue());
		}
		for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
		{
			pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

			if (pLoopPlot->isRevealed(getTeam(), false))// || AI_isPrimaryArea(pLoopPlot->area()))
			{
				long lResult=-1;
				if(GC.getUSE_GET_CITY_FOUND_VALUE_CALLBACK())
				{
					CyArgsList argsList;
					argsList.add((int)getID());
					argsList.add(pLoopPlot->getX());
					argsList.add(pLoopPlot->getY());
					gDLL->getPythonIFace()->callFunction(PYGameModule, "getCityFoundValue", argsList.makeFunctionArgs(), &lResult);
				}

				if (lResult == -1)
				{
					iValue = AI_foundValue(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
				}
				else
				{
					iValue = lResult;
				}

				pLoopPlot->setFoundValue(getID(), iValue);

				if (iValue > pLoopPlot->area()->getBestFoundValue(getID()))
				{
					pLoopPlot->area()->setBestFoundValue(getID(), iValue);
				}
			}
		}
		if (!isBarbarian())
		{
			int iMaxCityCount = 4;
			AI_updateCitySites(AI_getMinFoundValue(), iMaxCityCount);
		}
	}
}


void CvPlayerAI::AI_updateAreaTargets()
{
	CvArea* pLoopArea;
	int iLoop;

	for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
	{
		if (!(pLoopArea->isWater()))
		{
			if (GC.getGameINLINE().getSorenRandNum(3, "AI Target City") == 0)
			{
				pLoopArea->setTargetCity(getID(), NULL);
			}
			else
			{
				pLoopArea->setTargetCity(getID(), AI_findTargetCity(pLoopArea));
			}
		}
	}
}


// Returns priority for unit movement (lower values move first...)
int CvPlayerAI::AI_movementPriority(CvSelectionGroup* pGroup) const
{
	CvUnit* pHeadUnit;
	int iCurrCombat;
	int iBestCombat;

	pHeadUnit = pGroup->getHeadUnit();

	if (pHeadUnit != NULL)
	{
/********************************************************************************/
/* 	BETTER_BTS_AI_MOD							10/08/09		jdog5000		*/
/* 																				*/
/* 	Air AI, Espionage AI														*/
/********************************************************************************/
/* original BTS code
		if (pHeadUnit->hasCargo())
		{
			if (pHeadUnit->specialCargo() == NO_SPECIALUNIT)
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
	
		if (pHeadUnit->getDomainType() == DOMAIN_AIR)
		{
			return 2;
		}

		if ((pHeadUnit->AI_getUnitAIType() == UNITAI_WORKER) || (pHeadUnit->AI_getUnitAIType() == UNITAI_WORKER_SEA))
		{
			return 3;
		}

		if ((pHeadUnit->AI_getUnitAIType() == UNITAI_EXPLORE) || (pHeadUnit->AI_getUnitAIType() == UNITAI_EXPLORE_SEA))
		{
			return 4;
		}

		if (pHeadUnit->bombardRate() > 0)
		{
			return 5;
		}

		if (pHeadUnit->collateralDamage() > 0)
		{
			return 6;
		}

		if (pHeadUnit->canFight())
		{
			if (pHeadUnit->withdrawalProbability() > 20)
			{
				return 7;
			}

			if (pHeadUnit->withdrawalProbability() > 0)
			{
				return 8;
			}

			iCurrCombat = pHeadUnit->currCombatStr(NULL, NULL);
			iBestCombat = (GC.getGameINLINE().getBestLandUnitCombat() * 100);

			if (pHeadUnit->noDefensiveBonus())
			{
				iCurrCombat *= 3;
				iCurrCombat /= 2;
			}

			if (pHeadUnit->AI_isCityAIType())
			{
				iCurrCombat /= 2;
			}

			if (iCurrCombat > iBestCombat)
			{
				return 9;
			}
			else if (iCurrCombat > ((iBestCombat * 4) / 5))
			{
				return 10;
			}
			else if (iCurrCombat > ((iBestCombat * 3) / 5))
			{
				return 11;
			}
			else if (iCurrCombat > ((iBestCombat * 2) / 5))
			{
				return 12;
			}
			else if (iCurrCombat > ((iBestCombat * 1) / 5))
			{
				return 13;
			}
			else
			{
				return 14;
			}
		}

		return 15;
	}

	return 16;
*/
		if( pHeadUnit->isSpy() )
		{
			return 0;
		}

		if (pHeadUnit->hasCargo())
		{
			if (pHeadUnit->specialCargo() == NO_SPECIALUNIT)
			{
				return 1;
			}
			else
			{
				return 2;
			}
		}

		// Make fighters move before bombers, they are better at clearing out air defenses
		if (pHeadUnit->getDomainType() == DOMAIN_AIR)
		{
			if( pHeadUnit->canAirDefend() )
			{
				return 3;
			}
			else
			{
				return 4;
			}
		}

		if ((pHeadUnit->AI_getUnitAIType() == UNITAI_WORKER) || (pHeadUnit->AI_getUnitAIType() == UNITAI_WORKER_SEA))
		{
			return 5;
		}

		if ((pHeadUnit->AI_getUnitAIType() == UNITAI_EXPLORE) || (pHeadUnit->AI_getUnitAIType() == UNITAI_EXPLORE_SEA))
		{
			return 6;
		}

		if (pHeadUnit->bombardRate() > 0)
		{
			return 7;
		}

		if (pHeadUnit->collateralDamage() > 0)
		{
			return 8;
		}

		if (pHeadUnit->canFight())
		{
			if (pHeadUnit->withdrawalProbability() > 20)
			{
				return 9;
			}

			if (pHeadUnit->withdrawalProbability() > 0)
			{
				return 10;
			}

			iCurrCombat = pHeadUnit->currCombatStr(NULL, NULL);
			iBestCombat = (GC.getGameINLINE().getBestLandUnitCombat() * 100);

			if (pHeadUnit->noDefensiveBonus())
			{
				iCurrCombat *= 3;
				iCurrCombat /= 2;
			}

			if (pHeadUnit->AI_isCityAIType())
			{
				iCurrCombat /= 2;
			}

			if (iCurrCombat > iBestCombat)
			{
				return 11;
			}
			else if (iCurrCombat > ((iBestCombat * 4) / 5))
			{
				return 12;
			}
			else if (iCurrCombat > ((iBestCombat * 3) / 5))
			{
				return 13;
			}
			else if (iCurrCombat > ((iBestCombat * 2) / 5))
			{
				return 14;
			}
			else if (iCurrCombat > ((iBestCombat * 1) / 5))
			{
				return 15;
			}
			else
			{
				return 16;
			}
		}

		return 17;
	}

	return 18;
/********************************************************************************/
/* BETTER_BTS_AI_MOD                           END                              */
/********************************************************************************/
}


void CvPlayerAI::AI_unitUpdate()
{
	PROFILE_FUNC();

	CLLNode<int>* pCurrUnitNode;
	CvSelectionGroup* pLoopSelectionGroup;
	CLinkList<int> tempGroupCycle;
	CLinkList<int> finalGroupCycle;
	int iValue;

	if (!hasBusyUnit())
	{
		pCurrUnitNode = headGroupCycleNode();

		while (pCurrUnitNode != NULL)
		{
			pLoopSelectionGroup = getSelectionGroup(pCurrUnitNode->m_data);
			pCurrUnitNode = nextGroupCycleNode(pCurrUnitNode);

			if (pLoopSelectionGroup->AI_isForceSeparate())
			{
				// do not split groups that are in the midst of attacking
				if (pLoopSelectionGroup->isForceUpdate() || !pLoopSelectionGroup->AI_isGroupAttack())
				{
					pLoopSelectionGroup->AI_separate();	// pointers could become invalid...
				}
			}
		}

		if (isHuman())
		{
			pCurrUnitNode = headGroupCycleNode();

			while (pCurrUnitNode != NULL)
			{
				pLoopSelectionGroup = getSelectionGroup(pCurrUnitNode->m_data);
				pCurrUnitNode = nextGroupCycleNode(pCurrUnitNode);

				if (pLoopSelectionGroup->AI_update())
				{
					break; // pointers could become invalid...
				}
			}
		}
		else
		{
			tempGroupCycle.clear();
			finalGroupCycle.clear();

			pCurrUnitNode = headGroupCycleNode();

			while (pCurrUnitNode != NULL)
			{
				tempGroupCycle.insertAtEnd(pCurrUnitNode->m_data);
				pCurrUnitNode = nextGroupCycleNode(pCurrUnitNode);
			}

			iValue = 0;

			while (tempGroupCycle.getLength() > 0)
			{
				pCurrUnitNode = tempGroupCycle.head();

				while (pCurrUnitNode != NULL)
				{
					pLoopSelectionGroup = getSelectionGroup(pCurrUnitNode->m_data);
					FAssertMsg(pLoopSelectionGroup != NULL, "selection group node with NULL selection group");

					if (AI_movementPriority(pLoopSelectionGroup) <= iValue)
					{
						finalGroupCycle.insertAtEnd(pCurrUnitNode->m_data);
						pCurrUnitNode = tempGroupCycle.deleteNode(pCurrUnitNode);
					}
					else
					{
						pCurrUnitNode = tempGroupCycle.next(pCurrUnitNode);
					}
				}

				iValue++;
			}

			pCurrUnitNode = finalGroupCycle.head();

			while (pCurrUnitNode != NULL)
			{
				pLoopSelectionGroup = getSelectionGroup(pCurrUnitNode->m_data);

				if (NULL != pLoopSelectionGroup)  // group might have been killed by a previous group update
				{
					if (pLoopSelectionGroup->AI_update())
					{
						break; // pointers could become invalid...
					}
				}

				pCurrUnitNode = finalGroupCycle.next(pCurrUnitNode);
			}
		}
	}
}


void CvPlayerAI::AI_makeAssignWorkDirty()
{
	CvCity* pLoopCity;
	int iLoop;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->AI_setAssignWorkDirty(true);
	}
}


void CvPlayerAI::AI_assignWorkingPlots()
{
	CvCity* pLoopCity;
	int iLoop;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->AI_assignWorkingPlots();
	}
}


void CvPlayerAI::AI_updateAssignWork()
{
	CvCity* pLoopCity;
	int iLoop;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->AI_updateAssignWork();
	}
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/08/09                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
void CvPlayerAI::AI_doCentralizedProduction()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	int iLoop;
	int iI;

	if( isHuman() )
	{
		return;
	}

	if( isBarbarian() )
	{
		return;
	}

	// BBAI TODO: Temp testing
	//if( getID() % 2 == 1 )
	//{
		return;
	//}
	
	// Determine number of cities player can use building wonders currently
	int iMaxNumWonderCities = 1 + getNumCities()/5;
	bool bIndustrious = (getMaxPlayerBuildingProductionModifier() > 0);
	bool bAtWar = (GET_TEAM(getTeam()).getAtWarCount(true) > 0);

	if( bIndustrious )
	{
		iMaxNumWonderCities += 1;
	}

	// Dagger?
	// Power?
	// Research?

	if( bAtWar )
	{
		int iWarCapRatio = GET_TEAM(getTeam()).AI_getWarSuccessCapitulationRatio();
		if( iWarCapRatio < -90 )
		{
			iMaxNumWonderCities = 0;
		}
		else 
		{
			if( iWarCapRatio < 30 )
			{
				iMaxNumWonderCities -= 1;
			}
			if( iWarCapRatio < -50 )
			{
				iMaxNumWonderCities /= 2;
			}
		}
	}

	if( isMinorCiv() && (GET_TEAM(getTeam()).getHasMetCivCount(false) > 1) )
	{
		iMaxNumWonderCities /= 2;
	}

	iMaxNumWonderCities = std::min(iMaxNumWonderCities, getNumCities());

	// Gather city statistics
	// Could rank cities based on gold, production here, could be O(n) instead of O(n^2)
	int iWorldWonderCities = 0;
	int iLimitedWonderCities = 0;
	int iNumDangerCities = 0;
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if( pLoopCity->isProductionBuilding() )
		{
			if( isLimitedWonderClass((BuildingClassTypes)(GC.getBuildingInfo(pLoopCity->getProductionBuilding()).getBuildingClassType())))
			{
				iLimitedWonderCities++;

				if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(pLoopCity->getProductionBuilding()).getBuildingClassType())))
				{
					iWorldWonderCities++;
				}
			}
		}

		if( pLoopCity->isProductionProject() )
		{
			if( isLimitedProject(pLoopCity->getProductionProject()))
			{
				iLimitedWonderCities++;
				if( isWorldProject(pLoopCity->getProductionProject()))
				{
					iWorldWonderCities++;
				}
			}
		}
	}

	// Check for any global wonders to build
	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		if (isWorldWonderClass((BuildingClassTypes)iI))
		{

			//canConstruct(
		}
	}

	// Check for any projects to build
	for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
	{
		
	}

	// Check for any national/team wonders to build
	
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/


void CvPlayerAI::AI_makeProductionDirty()
{
	CvCity* pLoopCity;
	int iLoop;

	FAssertMsg(!isHuman(), "isHuman did not return false as expected");

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		pLoopCity->AI_setChooseProductionDirty(true);
	}
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/16/10                              jdog5000        */
/*                                                                                              */
/* War tactics AI                                                                               */
/************************************************************************************************/
void CvPlayerAI::AI_conquerCity(CvCity* pCity)
{
	bool bRaze = false;
	int iRazeValue;
	int iI;

	if (canRaze(pCity))
	{
	    iRazeValue = 0;
		int iCloseness = pCity->AI_playerCloseness(getID());

		if ( (pCity->isHolyCity()) || (pCity->hasActiveWorldWonder()))
		{
			if( gPlayerLogLevel >= 1 )
			{
				logBBAI("    Player %d (%S) decides not to raze holy/wonder city %S", getID(), getCivilizationDescription(0), pCity->getName().GetCString() );
			}
		}
		else if( (getNumCities() <= 1) || (getNumCities() < 5 && iCloseness > 0) )
		{
			if( gPlayerLogLevel >= 1 )
			{
				logBBAI("    Player %d (%S) decides not to raze %S because they have few cities", getID(), getCivilizationDescription(0), pCity->getName().GetCString() );
			}
		}
		else if( AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION3) && GET_TEAM(getTeam()).AI_isPrimaryArea(pCity->area()) )
		{
			// Do not raze, going for domination
			if( gPlayerLogLevel >= 1 )
			{
				logBBAI("    Player %d (%S) decides not to raze %S because they're going for domination", getID(), getCivilizationDescription(0), pCity->getName().GetCString() );
			}
		}
		else if( isBarbarian() )
		{
			if( (pCity->getPreviousOwner() != BARBARIAN_PLAYER) && (pCity->getOriginalOwner() != BARBARIAN_PLAYER) )
			{
				iRazeValue += GC.getLeaderHeadInfo(getPersonalityType()).getRazeCityProb();
				iRazeValue -= iCloseness;
			}
		}
		else
		{
			bool bFinancialTrouble = AI_isFinancialTrouble();
			bool bBarbCity = (pCity->getPreviousOwner() == BARBARIAN_PLAYER) && (pCity->getOriginalOwner() == BARBARIAN_PLAYER);
			bool bPrevOwnerBarb = (pCity->getPreviousOwner() == BARBARIAN_PLAYER);
			
			if (GET_TEAM(getTeam()).countNumCitiesByArea(pCity->area()) == 0)
			{
				// Conquered city in new continent/island
				int iBestValue;

				if( pCity->area()->getNumCities() == 1 && AI_getNumAreaCitySites(pCity->area()->getID(), iBestValue) == 0 )
				{
					// Probably small island
					if( iCloseness == 0 )
					{
						// Safe to raze these now that AI can do pick up ...
						iRazeValue += GC.getLeaderHeadInfo(getPersonalityType()).getRazeCityProb();
					}
				}
				else
				{
					// At least medium sized island
					if( iCloseness < 10 )
					{
						if( bFinancialTrouble )
						{
							// Raze if we might start incuring colony maintenance
							iRazeValue = 100;
						}
						else
						{
							if (pCity->getPreviousOwner() != NO_PLAYER && !bPrevOwnerBarb)
							{
								if (GET_TEAM(GET_PLAYER(pCity->getPreviousOwner()).getTeam()).countNumCitiesByArea(pCity->area()) > 3)
								{
									iRazeValue += GC.getLeaderHeadInfo(getPersonalityType()).getRazeCityProb();

									// More inclined to raze if we're unlikely to hold it
									if( GET_TEAM(getTeam()).getPower(false)*10 < GET_TEAM(GET_PLAYER(pCity->getPreviousOwner()).getTeam()).getPower(true)*9 )
									{
										iRazeValue += ((100 * GET_TEAM(GET_PLAYER(pCity->getPreviousOwner()).getTeam()).getPower(true))/(GET_TEAM(getTeam()).getPower(false))) - 100;
									}
								}
							}
						}
					}
				}
			}
			else
			{
				// Distance related aspects
				if (iCloseness > 0)
				{
					iRazeValue -= iCloseness;
				}
				else
				{
					iRazeValue += 40;

					CvCity* pNearestTeamAreaCity = GC.getMapINLINE().findCity(pCity->getX_INLINE(), pCity->getY_INLINE(), NO_PLAYER, getTeam(), true, false, NO_TEAM, NO_DIRECTION, pCity);

					if( pNearestTeamAreaCity == NULL )
					{
						// Shouldn't happen
						iRazeValue += 30;
					}
					else
					{
						int iDistance = plotDistance(pCity->getX_INLINE(), pCity->getY_INLINE(), pNearestTeamAreaCity->getX_INLINE(), pNearestTeamAreaCity->getY_INLINE());
						iDistance -= DEFAULT_PLAYER_CLOSENESS + 2;
						if ( iDistance > 0 )
						{
							iRazeValue += iDistance * (bBarbCity ? 8 : 5);
						}
					}
				}

				if (bFinancialTrouble)
				{
					iRazeValue += std::max(0, (70 - 15 * pCity->getPopulation()));
				}

				// Scale down distance/maintenance effects for organized
				if( iRazeValue > 0 )
				{
					for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
					{
						if (hasTrait((TraitTypes)iI))
						{
							iRazeValue *= (100 - (GC.getTraitInfo((TraitTypes)iI).getUpkeepModifier()));
							iRazeValue /= 100;

							if( (GC.getTraitInfo((TraitTypes)iI).getUpkeepModifier() > 0) && gPlayerLogLevel >= 1 )
							{
								logBBAI("      Reduction for upkeep modifier %d", (GC.getTraitInfo((TraitTypes)iI).getUpkeepModifier()) );
							}
						}
					}
				}
				// Non-distance related aspects
				iRazeValue += GC.getLeaderHeadInfo(getPersonalityType()).getRazeCityProb();
	                
				if (getStateReligion() != NO_RELIGION)
				{
					if (pCity->isHasReligion(getStateReligion()))
					{
						if (GET_TEAM(getTeam()).hasShrine(getStateReligion()))
						{
							iRazeValue -= 50;

							if( gPlayerLogLevel >= 1 )
							{
								logBBAI("      Reduction for state religion with shrine" );
							}
						}
						else
						{
							iRazeValue -= 10;

							if( gPlayerLogLevel >= 1 )
							{
								logBBAI("      Reduction for state religion" );
							}
						}
					}
				}
			}

			// BBAI TODO: Highly valuable resource??

			if( gPlayerLogLevel >= 1 )
			{
				if( bBarbCity ) logBBAI("      %S is a barb city", pCity->getName().GetCString() );
				if( bPrevOwnerBarb ) logBBAI("      %S was last owned by barbs", pCity->getName().GetCString() );
				logBBAI("      %S has area cities %d, closeness %d, bFinTrouble %d", pCity->getName().GetCString(), GET_TEAM(getTeam()).countNumCitiesByArea(pCity->area()), iCloseness, bFinancialTrouble );
			}
		}

		if( gPlayerLogLevel >= 1 )
		{
			logBBAI("    Player %d (%S) has odds %d to raze city %S", getID(), getCivilizationDescription(0), iRazeValue, pCity->getName().GetCString() );
		}
					
		if (iRazeValue > 0)
		{
            if (GC.getGameINLINE().getSorenRandNum(100, "AI Raze City") < iRazeValue)
			{
				bRaze = true;
				logBBAI("    Player %d (%S) decides to to raze city %S!!!", getID(), getCivilizationDescription(0), pCity->getName().GetCString() );
				pCity->doTask(TASK_RAZE);
			}
		}		
	}

	if (!bRaze)
	{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       06/14/09                       Maniac & jdog5000      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/* original bts code
		CvEventReporter::getInstance().cityAcquiredAndKept(GC.getGameINLINE().getActivePlayer(), pCity);
*/
		CvEventReporter::getInstance().cityAcquiredAndKept(getID(), pCity);
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
	}
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/


bool CvPlayerAI::AI_acceptUnit(CvUnit* pUnit) const
{
	if (isHuman())
	{
		return true;
	}

	if (AI_isFinancialTrouble())
	{
		if (pUnit->AI_getUnitAIType() == UNITAI_WORKER)
		{
			if (AI_neededWorkers(pUnit->area()) > 0)
			{
				return true;
			}
		}
		
		if (pUnit->AI_getUnitAIType() == UNITAI_WORKER_SEA)
		{
			return true;
		}
		
		if (pUnit->AI_getUnitAIType() == UNITAI_MISSIONARY)
		{
			return true; //XXX
		}		
		return false;
	}
	
	return true;
}


bool CvPlayerAI::AI_captureUnit(UnitTypes eUnit, CvPlot* pPlot) const
{
	CvCity* pNearestCity;

	FAssert(!isHuman());
/************************************************************************************************/
/* Afforess	                  Start		 07/15/10                                               */
/*                                                                                              */
/* AI Always capture too                                                                        */
/************************************************************************************************/
	return true;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	if (pPlot->getTeam() == getTeam())
	{
		return true;
	}

	pNearestCity = GC.getMapINLINE().findCity(pPlot->getX_INLINE(), pPlot->getY_INLINE(), NO_PLAYER, getTeam());

	if (pNearestCity != NULL)
	{
		if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE()) <= 4)
		{
			return true;
		}
	}

	return false;
}


DomainTypes CvPlayerAI::AI_unitAIDomainType(UnitAITypes eUnitAI) const
{
	switch (eUnitAI)
	{
	case UNITAI_UNKNOWN:
		return NO_DOMAIN;
		break;

	case UNITAI_ANIMAL:
	case UNITAI_SETTLE:
	case UNITAI_WORKER:
	case UNITAI_ATTACK:
	case UNITAI_ATTACK_CITY:
	case UNITAI_COLLATERAL:
	case UNITAI_PILLAGE:
	case UNITAI_RESERVE:
	case UNITAI_COUNTER:
	case UNITAI_PARADROP:
	case UNITAI_CITY_DEFENSE:
	case UNITAI_CITY_COUNTER:
	case UNITAI_CITY_SPECIAL:
	case UNITAI_EXPLORE:
	case UNITAI_MISSIONARY:
	case UNITAI_PROPHET:
	case UNITAI_ARTIST:
	case UNITAI_SCIENTIST:
	case UNITAI_GENERAL:
	case UNITAI_MERCHANT:
	case UNITAI_ENGINEER:
	case UNITAI_SPY:
	case UNITAI_ATTACK_CITY_LEMMING:
		return DOMAIN_LAND;
		break;

	case UNITAI_ICBM:
		return DOMAIN_IMMOBILE;
		break;

	case UNITAI_WORKER_SEA:
	case UNITAI_ATTACK_SEA:
	case UNITAI_RESERVE_SEA:
	case UNITAI_ESCORT_SEA:
	case UNITAI_EXPLORE_SEA:
	case UNITAI_ASSAULT_SEA:
	case UNITAI_SETTLER_SEA:
	case UNITAI_MISSIONARY_SEA:
	case UNITAI_SPY_SEA:
	case UNITAI_CARRIER_SEA:
	case UNITAI_MISSILE_CARRIER_SEA:
	case UNITAI_PIRATE_SEA:
		return DOMAIN_SEA;
		break;

	case UNITAI_ATTACK_AIR:
	case UNITAI_DEFENSE_AIR:
	case UNITAI_CARRIER_AIR:
	case UNITAI_MISSILE_AIR:
		return DOMAIN_AIR;
		break;

	default:
		FAssert(false);
		break;
	}

	return NO_DOMAIN;
}


int CvPlayerAI::AI_yieldWeight(YieldTypes eYield) const
{
	if (eYield == YIELD_PRODUCTION)
	{
		int iProductionModifier = 100 + (30 * std::max(0, GC.getGame().getCurrentEra() - 1) / std::max(1, (GC.getNumEraInfos() - 2)));
		return (GC.getYieldInfo(eYield).getAIWeightPercent() * iProductionModifier) / 100;
	}
	return GC.getYieldInfo(eYield).getAIWeightPercent();
}


int CvPlayerAI::AI_commerceWeight(CommerceTypes eCommerce, CvCity* pCity) const
{
	int iWeight;

	iWeight = GC.getCommerceInfo(eCommerce).getAIWeightPercent();

	//sorry but the merchant descrimination must stop.
	iWeight = std::min(110, iWeight);
	
	//XXX Add something for 100%/0% type situations
	switch (eCommerce)
	{
	case COMMERCE_RESEARCH:
		if (AI_avoidScience())
		{
			if (isNoResearchAvailable())
			{
				iWeight = 0;
			}
			else
			{
				iWeight /= 8;
			}
		}
		break;
	case COMMERCE_GOLD:
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      06/12/09                                jdog5000      */
/*                                                                                              */
/* Gold AI                                                                        */
/************************************************************************************************/
		if (getCommercePercent(COMMERCE_GOLD) > 70)
		{
			//avoid strikes
			if (getGoldPerTurn() < -getGold()/100)
			{
				iWeight += 15;
			}
		}
		else if (getCommercePercent(COMMERCE_GOLD) < 25)
		{
			//put more money towards other commerce types
			if (getGoldPerTurn() > -getGold()/40)
			{
				iWeight -= 25 - getCommercePercent(COMMERCE_GOLD);
			}
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		break;
	case COMMERCE_CULTURE:
		// COMMERCE_CULTURE AIWeightPercent is 25% in default xml
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Bugfix, Cultural Victory AI                                                                  */
/************************************************************************************************/
		// Adjustments for human player going for cultural victory (who won't have AI strategy set) 
		// so that governors do smart things
		if (pCity != NULL)
		{
			if (pCity->getCultureTimes100(getID()) >= 100 * GC.getGameINLINE().getCultureThreshold((CultureLevelTypes)(GC.getNumCultureLevelInfos() - 1)))
			{
				iWeight /= 50;
			}
			// Slider check works for detection of whether human player is going for cultural victory
			else if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3) || getCommercePercent(COMMERCE_CULTURE) > 80 )
			{
				int iCultureRateRank = pCity->findCommerceRateRank(COMMERCE_CULTURE);
				int iCulturalVictoryNumCultureCities = GC.getGameINLINE().culturalVictoryNumCultureCities();
				
				// if one of the currently best cities, then focus hard, *4 or more
				if (iCultureRateRank <= iCulturalVictoryNumCultureCities)
				{
					iWeight *= (3 + iCultureRateRank);
				}
				// if one of the 3 close to the top, then still emphasize culture some, *2
				else if (iCultureRateRank <= iCulturalVictoryNumCultureCities + 3)
				{
					iWeight *= 2;
				}
				else if (isHuman())
				{
					iWeight *= 2;
				}

			}
			else if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2) || getCommercePercent(COMMERCE_CULTURE) > 60)
			{
				iWeight *= 3;
			}
			else if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE1) || getCommercePercent(COMMERCE_CULTURE) > 40)
			{
				iWeight *= 2;
			}
			
			iWeight += (100 - pCity->plot()->calculateCulturePercent(getID()));
			
			if (pCity->getCultureLevel() <= (CultureLevelTypes) 1)
			{
				iWeight = std::max(iWeight, 800);				
			}
		}
		// pCity == NULL
		else
		{
			if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3) || getCommercePercent(COMMERCE_CULTURE) > 80 )
			{
				iWeight *= 3;
				iWeight /= 4;
			}
			else if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2) || getCommercePercent(COMMERCE_CULTURE) > 60 )
			{
				iWeight *= 2;
				iWeight /= 3;
			}
			else if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE1) || getCommercePercent(COMMERCE_CULTURE) > 40 )
			{
				iWeight /= 2;
			}
			else 
			{
				iWeight /= 3;
			}
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		break;
	case COMMERCE_ESPIONAGE:
		{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/29/09                                jdog5000      */
/*                                                                                              */
/* Espionage AI, Bugfix                                                                         */
/************************************************************************************************/
			// Fixed bug where espionage weight set to 0 if winning all esp point races
			// Smoothed out emphasis
			int iEspBehindWeight = 0;
			for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; ++iTeam)
			{
				CvTeam& kLoopTeam = GET_TEAM((TeamTypes)iTeam);
				if (kLoopTeam.isAlive() && iTeam != getTeam() && !kLoopTeam.isVassal(getTeam()) && !GET_TEAM(getTeam()).isVassal((TeamTypes)iTeam))
				{
					int iPointDiff = kLoopTeam.getEspionagePointsAgainstTeam(getTeam()) - GET_TEAM(getTeam()).getEspionagePointsAgainstTeam((TeamTypes)iTeam);
					if (iPointDiff > 0)
					{
						iEspBehindWeight += 1;
						if( GET_TEAM(getTeam()).AI_getAttitude((TeamTypes)iTeam) < ATTITUDE_CAUTIOUS )
						{
							iEspBehindWeight += 1;
						}
					}
				}
			}
			
			iWeight *= 2*iEspBehindWeight + (3*GET_TEAM(getTeam()).getHasMetCivCount(true))/4 + 1;
			iWeight *= AI_getEspionageWeight();
			iWeight /= GET_TEAM(getTeam()).getHasMetCivCount(true) + 1;
			iWeight /= 100;

			if( getCommercePercent(COMMERCE_ESPIONAGE) == 0 )
			{
				iWeight *= 2;
				iWeight /= 3;
			}
			else if( isHuman() )
			{
				// UNOFFICIAL_PATCH todo:  should this tweak come over in some form?
				// There's still an issue with upping espionage slider for human player.
				if( getCommercePercent(COMMERCE_ESPIONAGE) > 50 )
				{
					iWeight *= getCommercePercent(COMMERCE_ESPIONAGE);
					iWeight /= 50;
				}
			}
			else
			{
				// AI Espionage slider use maxed out at 20 percent
				if( getCommercePercent(COMMERCE_ESPIONAGE) >= 20 )
				{
					iWeight *= 3;
					iWeight /= 2;
				}
			}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		}
		break;
		
	default:
		break;
	}

	return iWeight;
}

// Improved as per Blake - thanks!
int CvPlayerAI::AI_foundValue(int iX, int iY, int iMinRivalRange, bool bStartingLoc) const
{
	CvCity* pNearestCity;
	CvArea* pArea;
	CvPlot* pPlot;
	CvPlot* pLoopPlot;
	FeatureTypes eFeature;
	BonusTypes eBonus;
	ImprovementTypes eBonusImprovement;
	bool bHasGoodBonus;
	int iOwnedTiles;
	int iBadTile;
	int iTakenTiles;
	int iTeammateTakenTiles;
	int iDifferentAreaTile;
	int iTeamAreaCities;
	int iHealth;
	int iValue;
	int iTempValue;
	int iRange;
	int iDX, iDY;
	int iI;
	bool bIsCoastal;
	int iResourceValue = 0;
	int iSpecialFood = 0;
	int iSpecialFoodPlus = 0;
	int iSpecialFoodMinus = 0;
	int iSpecialProduction = 0;
	int iSpecialCommerce = 0;
	
	bool bNeutralTerritory = true;

	int iGreed;
	int iNumAreaCities;

	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	
	if (!canFound(iX, iY))
	{
		return 0;
	}
	
	bIsCoastal = pPlot->isCoastalLand(GC.getMIN_WATER_SIZE_FOR_OCEAN());
	pArea = pPlot->area();
	iNumAreaCities = pArea->getCitiesPerPlayer(getID());
	
	bool bAdvancedStart = (getAdvancedStartPoints() >= 0);

	if (!bStartingLoc && !bAdvancedStart)
	{
		if (!bIsCoastal && iNumAreaCities == 0)
		{
			return 0;
		}
	}
	
	if (bAdvancedStart)
	{
		//FAssert(!bStartingLoc);
		FAssert(GC.getGameINLINE().isOption(GAMEOPTION_ADVANCED_START));
		if (bStartingLoc)
		{
			bAdvancedStart = false;
		}
	}
	
	//Explaination of city site adjustment:
	//Any plot which is otherwise within the radius of a city site
	//is basically treated as if it's within an existing city radius
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
	std::vector<bool> abCitySiteRadius(NUM_CITY_PLOTS, false);
*/
	std::vector<bool> abCitySiteRadius(NUM_CITY_PLOTS_2, false);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

		
	if (!bStartingLoc)
	{
		if (!AI_isPlotCitySite(pPlot))
		{
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
			for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
*/
			for (iI = 0; iI < NUM_CITY_PLOTS_2; iI++)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			{
				pLoopPlot = plotCity(iX, iY, iI);
				if (pLoopPlot != NULL)
				{
					for (int iJ = 0; iJ < AI_getNumCitySites(); iJ++)
					{
						CvPlot* pCitySitePlot = AI_getCitySite(iJ);
						if (pCitySitePlot != pPlot)
						{
							if (plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pCitySitePlot->getX_INLINE(), pCitySitePlot->getY_INLINE()) <= CITY_PLOTS_RADIUS)
							{
								//Plot is inside the radius of a city site
								abCitySiteRadius[iI] = true;
								break;
							}
						}
					}
				}
			}
		}
	}
	
	std::vector<int> paiBonusCount;

    for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
    {
        paiBonusCount.push_back(0);
    }

	if (iMinRivalRange != -1)
	{
		for (iDX = -(iMinRivalRange); iDX <= iMinRivalRange; iDX++)
		{
			for (iDY = -(iMinRivalRange); iDY <= iMinRivalRange; iDY++)
			{
				pLoopPlot	= plotXY(iX, iY, iDX, iDY);

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->plotCheck(PUF_isOtherTeam, getID()) != NULL)
					{
						return 0;
					}
				}
			}
		}
	}

	if (bStartingLoc)
	{
		if (pPlot->isGoody())
		{
			return 0;
		}
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
*/
		for (iI = 0; iI < NUM_CITY_PLOTS_2; iI++)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

		{
			pLoopPlot = plotCity(iX, iY, iI);

			if (pLoopPlot == NULL)
			{
				return 0;
			}
		}
	}

	iOwnedTiles = 0;
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
*/
	for (iI = 0; iI < NUM_CITY_PLOTS_2; iI++)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	{
		pLoopPlot = plotCity(iX, iY, iI);

		if (pLoopPlot == NULL)
		{
			iOwnedTiles++;
		}
		else if (pLoopPlot->isOwned())
        {
            if (pLoopPlot->getTeam() != getTeam())
            {
                iOwnedTiles++;
            }
        }
	}
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
	if (iOwnedTiles > (NUM_CITY_PLOTS / 3))
*/
	if (iOwnedTiles > (NUM_CITY_PLOTS_2 / 3))
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	{
		return 0;
	}

	iBadTile = 0;
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
*/
	for (iI = 0; iI < NUM_CITY_PLOTS_2; iI++)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	{
		pLoopPlot = plotCity(iX, iY, iI);

		if (iI != CITY_HOME_PLOT)
		{
		/************************************************************************************************/
		/* Afforess	Mountains Start		 09/18/09                                           		 */
		/*                                                                                              */
		/*                                                                                              */
		/************************************************************************************************/
			if ((pLoopPlot == NULL) || pLoopPlot->isImpassable(getTeam())) //added getTeam()
		/************************************************************************************************/
		/* Afforess	Mountains End       END        		                                             */
		/************************************************************************************************/

			{
				iBadTile += 2;
			}
			else if (!(pLoopPlot->isFreshWater()) && !(pLoopPlot->isHills()))
			{
				if ((pLoopPlot->calculateBestNatureYield(YIELD_FOOD, getTeam()) == 0) || (pLoopPlot->calculateTotalBestNatureYield(getTeam()) <= 1))
				{
					iBadTile += 2;
				}
				else if (pLoopPlot->isWater() && !bIsCoastal && (pLoopPlot->calculateBestNatureYield(YIELD_FOOD, getTeam()) <= 1))
				{
					iBadTile++;
				}
			}
            else if (pLoopPlot->isOwned())
            {
                if (pLoopPlot->getTeam() == getTeam())
                {
                    if (pLoopPlot->isCityRadius() || abCitySiteRadius[iI])
                    {
                        iBadTile += bAdvancedStart ? 2 : 1;
                    }
                }
            }
		}
	}

	iBadTile /= 2;

	if (!bStartingLoc)
	{
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
		if ((iBadTile > (NUM_CITY_PLOTS / 2)) || (pArea->getNumTiles() <= 2))
*/
		if ((iBadTile > (NUM_CITY_PLOTS_2 / 2)) || (pArea->getNumTiles() <= 2))
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		{
			bHasGoodBonus = false;
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
			for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
*/
			for (iI = 0; iI < NUM_CITY_PLOTS_2; iI++)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			{
				pLoopPlot = plotCity(iX, iY, iI);

				if (pLoopPlot != NULL)
				{
					if (!(pLoopPlot->isOwned()))
					{
						if (pLoopPlot->isWater() || (pLoopPlot->area() == pArea) || (pLoopPlot->area()->getCitiesPerPlayer(getID()) > 0))
						{
							eBonus = pLoopPlot->getBonusType(getTeam());

							if (eBonus != NO_BONUS)
							{
								if ((getNumTradeableBonuses(eBonus) == 0) || (AI_bonusVal(eBonus) > 10)
									|| (GC.getBonusInfo(eBonus).getYieldChange(YIELD_FOOD) > 0))
								{
									bHasGoodBonus = true;
									break;
								}
							}
						}
					}
				}
			}

			if (!bHasGoodBonus)
			{
				return 0;
			}
		}
	}

	iTakenTiles = 0;
	iTeammateTakenTiles = 0;
	iHealth = 0;
	iValue = 1000;

	iGreed = 100;

	if (bAdvancedStart)
	{
		iGreed = 150;		
	}
	else if (!bStartingLoc)
    {
        for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
        {
            if (hasTrait((TraitTypes)iI))
            {
                //Greedy founding means getting the best possible sites - fitting maximum
                //resources into the fat cross.
                iGreed += (GC.getTraitInfo((TraitTypes)iI).getUpkeepModifier() / 2);
                iGreed += 20 * (GC.getTraitInfo((TraitTypes)iI).getCommerceChange(COMMERCE_CULTURE));
            }
        }
    }
    //iClaimThreshold is the culture required to pop the 2nd borders.
    int iClaimThreshold = GC.getGameINLINE().getCultureThreshold((CultureLevelTypes)(std::min(2, (GC.getNumCultureLevelInfos() - 1))));
    iClaimThreshold = std::max(1, iClaimThreshold);
    iClaimThreshold *= (std::max(100, iGreed));
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       04/25/10                          denev & jdog5000    */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
	// Was missing this
/* Fuyu: we can do this when we are actually using this variable
	iClaimThreshold /= 100;
*/
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
	int iYieldLostHere = 0;
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
*/
	for (iI = 0; iI < NUM_CITY_PLOTS_2; iI++)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	{
		pLoopPlot = plotCity(iX, iY, iI);

		if (pLoopPlot == NULL)
		{
			iTakenTiles++;
		}
		else if (pLoopPlot->isCityRadius() || abCitySiteRadius[iI])
		{
			iTakenTiles++;

			if (abCitySiteRadius[iI])
			{
				iTeammateTakenTiles++;
			}
		}
		else
		{
			iTempValue = 0;

			eFeature = pLoopPlot->getFeatureType();
			eBonus = pLoopPlot->getBonusType((bStartingLoc) ? NO_TEAM : getTeam());
			eBonusImprovement = NO_IMPROVEMENT;

			int iCultureMultiplier;
            if (!pLoopPlot->isOwned() || (pLoopPlot->getOwnerINLINE() == getID()))
            {	
                iCultureMultiplier = 100;    
            }
            else
            {
            	bNeutralTerritory = false;
                int iOurCulture = pLoopPlot->getCulture(getID());
                int iOtherCulture = std::max(1, pLoopPlot->getCulture(pLoopPlot->getOwnerINLINE()));
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       04/25/10                          denev & Fuyu	    */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/*original code
                iCultureMultiplier = 100 * (iOurCulture + iClaimThreshold);
                iCultureMultiplier /= (iOtherCulture + iClaimThreshold);

*/
                iCultureMultiplier = (100 * iOurCulture) + iClaimThreshold;
                iCultureMultiplier /= (((100 * iOtherCulture) + iClaimThreshold) / 100);
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
                iCultureMultiplier = std::min(100, iCultureMultiplier);
                //The multiplier is basically normalized...
                //100% means we own (or rightfully own) the tile.
                //50% means the hostile culture is fairly firmly entrenched.
            }
            
            if (iCultureMultiplier < ((iNumAreaCities > 0) ? 25 : 50))
            {
                //discourage hopeless cases, especially on other continents.
                iTakenTiles += (iNumAreaCities > 0) ? 1 : 2;
            }

			if (eBonus != NO_BONUS)
			{
				for (int iImprovement = 0; iImprovement < GC.getNumImprovementInfos(); ++iImprovement)
				{
					CvImprovementInfo& kImprovement = GC.getImprovementInfo((ImprovementTypes)iImprovement);

					if (kImprovement.isImprovementBonusMakesValid(eBonus))
					{
						eBonusImprovement = (ImprovementTypes)iImprovement;
						break;
					}
				}
			}

			int aiYield[NUM_YIELD_TYPES];

			for (int iYieldType = 0; iYieldType < NUM_YIELD_TYPES; ++iYieldType)
			{
				YieldTypes eYield = (YieldTypes)iYieldType;
				aiYield[eYield] = pLoopPlot->getYield(eYield);

				if (iI == CITY_HOME_PLOT)
				{
					int iBasePlotYield = aiYield[eYield];
					aiYield[eYield] += GC.getYieldInfo(eYield).getCityChange();

					if (eFeature != NO_FEATURE)
					{
						aiYield[eYield] -= GC.getFeatureInfo(eFeature).getYieldChange(eYield);
						iBasePlotYield = std::max(iBasePlotYield, aiYield[eYield]);
					}

					if (eBonus == NO_BONUS)
					{
						aiYield[eYield] = std::max(aiYield[eYield], GC.getYieldInfo(eYield).getMinCity());
					}
					else
					{
						int iBonusYieldChange = GC.getBonusInfo(eBonus).getYieldChange(eYield);
						aiYield[eYield] += iBonusYieldChange;
						iBasePlotYield += iBonusYieldChange;
						
						aiYield[eYield] = std::max(aiYield[eYield], GC.getYieldInfo(eYield).getMinCity());
					}
				
					if (eBonusImprovement != NO_IMPROVEMENT)
					{
						iBasePlotYield += GC.getImprovementInfo(eBonusImprovement).getImprovementBonusYield(eBonus, eYield);
						
						if (iBasePlotYield > aiYield[eYield])
						{
							aiYield[eYield] -= 2 * (iBasePlotYield - aiYield[eYield]);
						}
						else
						{
							aiYield[eYield] += aiYield[eYield] - iBasePlotYield;
						}
					}
				}
			}

			if (iI == CITY_HOME_PLOT)
			{
				iTempValue += aiYield[YIELD_FOOD] * 60;
				iTempValue += aiYield[YIELD_PRODUCTION] * 60;
				iTempValue += aiYield[YIELD_COMMERCE] * 40;
			}
			else if (aiYield[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION())
			{
				iTempValue += aiYield[YIELD_FOOD] * 40;
				iTempValue += aiYield[YIELD_PRODUCTION] * 40;
				iTempValue += aiYield[YIELD_COMMERCE] * 30;

				if (bStartingLoc)
				{
					iTempValue *= 2;
				}
			}
			else if (aiYield[YIELD_FOOD] == GC.getFOOD_CONSUMPTION_PER_POPULATION() - 1)
			{
				iTempValue += aiYield[YIELD_FOOD] * 25;
				iTempValue += aiYield[YIELD_PRODUCTION] * 25;
				iTempValue += aiYield[YIELD_COMMERCE] * 20;
			}
			else
			{
				iTempValue += aiYield[YIELD_FOOD] * 15;
				iTempValue += aiYield[YIELD_PRODUCTION] * 15;
				iTempValue += aiYield[YIELD_COMMERCE] * 10;
			}

			if (pLoopPlot->isWater())
			{
				if (aiYield[YIELD_COMMERCE] > 1)
				{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/05/09                                jdog5000      */
/*                                                                                              */
/* Settler AI                                                                                   */
/************************************************************************************************/
/* orginal bts code
					iTempValue += bIsCoastal ? 30 : -20;
*/
					// Upside is much higher based on multipliers above, with lighthouse a standard coast
					// plot moves up into the higher multiplier category.  
					iTempValue += bIsCoastal ? 40 + 10*aiYield[YIELD_COMMERCE] : -10*aiYield[YIELD_COMMERCE];
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
					if (bIsCoastal && (aiYield[YIELD_FOOD] >= GC.getFOOD_CONSUMPTION_PER_POPULATION()))
					{
						iSpecialFoodPlus += 1;                    	
					}
					if (bStartingLoc && !pPlot->isStartingPlot())
					{
						// I'm pretty much forbidding starting 1 tile inland non-coastal.
						// with more than a few coast tiles.
						iTempValue += bIsCoastal ? 0 : -400;
					}
				}
			}

			if (pLoopPlot->isRiver())
			{
/************************************************************************************************/
/* Afforess	                  Start		 07/21/10                                               */
/*                                                                                              */
/* Favor extra river tiles for eventual building yields, if we are on a river                   */
/************************************************************************************************/
				if (pPlot->isRiver())
				{
					iTempValue += 10 * GC.getGameINLINE().getRiverBuildings();
				}
				else
				{
					iTempValue += 10;
				}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			}
			
/************************************************************************************************/
/* Afforess	                  Start		 03/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			if (pLoopPlot->isPeak())
			{
				if (GC.getGameINLINE().isOption(GAMEOPTION_MOUNTAINS))
				{
					iTempValue += 10;
				}
				else
				{
					iTempValue -= 10;
				}
			}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

			if (iI == CITY_HOME_PLOT)
			{
				iTempValue *= 2;
			}
			else if ((pLoopPlot->getOwnerINLINE() == getID()) || (stepDistance(iX, iY, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) == 1))
			{
				// BBAI Notes:  Extra weight on tiles which will be available immediately
				iTempValue *= 3;
				iTempValue /= 2;
			}
			else
			{
				iTempValue *= iGreed;
				iTempValue /= 100;
			}
			
			iTempValue *= iCultureMultiplier;
			iTempValue /= 100;

			iValue += iTempValue;

			if (iCultureMultiplier > 33) //ignore hopelessly entrenched tiles.
			{
				if (eFeature != NO_FEATURE)
				{
					if (iI != CITY_HOME_PLOT)
					{
						iHealth += GC.getFeatureInfo(eFeature).getHealthPercent();

						iSpecialFoodPlus += std::max(0, aiYield[YIELD_FOOD] - GC.getFOOD_CONSUMPTION_PER_POPULATION());
					}
				}

				if ((eBonus != NO_BONUS) && ((pLoopPlot->area() == pPlot->area()) || 
					(pLoopPlot->area()->getCitiesPerPlayer(getID()) > 0)))
				{
                    paiBonusCount[eBonus]++;
                    FAssert(paiBonusCount[eBonus] > 0);

                    iTempValue = (AI_bonusVal(eBonus) * ((!bStartingLoc && (getNumTradeableBonuses(eBonus) == 0) && (paiBonusCount[eBonus] == 1)) ? 80 : 20));
                    iTempValue *= ((bStartingLoc) ? 100 : iGreed);
                    iTempValue /= 100;

                    if (iI != CITY_HOME_PLOT && !bStartingLoc)
                    {
                        if ((pLoopPlot->getOwnerINLINE() != getID()) && stepDistance(pPlot->getX_INLINE(),pPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) > 1)
						{
                            iTempValue *= 2;
                            iTempValue /= 3;

                            iTempValue *= std::min(150, iGreed);
                            iTempValue /= 100;
						}
					}

                    iValue += (iTempValue + 10);

					if (iI != CITY_HOME_PLOT)
					{
						if (eBonusImprovement != NO_IMPROVEMENT)
						{
							int iSpecialFoodTemp;
							iSpecialFoodTemp = pLoopPlot->calculateBestNatureYield(YIELD_FOOD, getTeam()) + GC.getImprovementInfo(eBonusImprovement).getImprovementBonusYield(eBonus, YIELD_FOOD);

							iSpecialFood += iSpecialFoodTemp;

							iSpecialFoodTemp -= GC.getFOOD_CONSUMPTION_PER_POPULATION();

							iSpecialFoodPlus += std::max(0,iSpecialFoodTemp);
							iSpecialFoodMinus -= std::min(0,iSpecialFoodTemp);
							iSpecialProduction += pLoopPlot->calculateBestNatureYield(YIELD_PRODUCTION, getTeam()) + GC.getImprovementInfo(eBonusImprovement).getImprovementBonusYield(eBonus, YIELD_PRODUCTION);
							iSpecialCommerce += pLoopPlot->calculateBestNatureYield(YIELD_COMMERCE, getTeam()) + GC.getImprovementInfo(eBonusImprovement).getImprovementBonusYield(eBonus, YIELD_COMMERCE);
						}

						if (eFeature != NO_FEATURE)
						{
							if (GC.getFeatureInfo(eFeature).getYieldChange(YIELD_FOOD) < 0)
							{
								iResourceValue -= 30;
							}
						}

                        if (pLoopPlot->isWater())
                        {
                            iValue += (bIsCoastal ? 100 : -800);
                        }
                    }
				}
			}
		}
	}

	iResourceValue += iSpecialFood * 50;
	iResourceValue += iSpecialProduction * 50;
	iResourceValue += iSpecialCommerce * 50;
    if (bStartingLoc)
    {
        iResourceValue /= 2;
    }

	iValue += std::max(0, iResourceValue);
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
	if (iTakenTiles > (NUM_CITY_PLOTS / 3) && iResourceValue < 250)
*/
	if (iTakenTiles > (NUM_CITY_PLOTS_2 / 3) && iResourceValue < 250)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	{
		return 0;
	}

	if (iTeammateTakenTiles > 1)
	{
		return 0;
	}

	iValue += (iHealth / 5);

	if (bIsCoastal)
	{
		if (!bStartingLoc)
		{
			if (pArea->getCitiesPerPlayer(getID()) == 0)
			{
				if (bNeutralTerritory)
				{
					iValue += (iResourceValue > 0) ? 800 : 100;
				}
			}
			else
			{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/03/09                                jdog5000      */
/*                                                                                              */
/* Settler AI                                                                                   */
/************************************************************************************************/
				iValue += 200;

				// Push players to get more coastal cities so they can build navies
				CvArea* pWaterArea = pPlot->waterArea(true);
				if( pWaterArea != NULL )
				{
					iValue += 200;

					if( GET_TEAM(getTeam()).AI_isWaterAreaRelevant(pWaterArea) )
					{
						iValue += 200;

						if( (countNumCoastalCities() < (getNumCities()/4)) || (countNumCoastalCitiesByArea(pPlot->area()) == 0) )
						{
							iValue += 200;
						}
					}
				}				
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
			}
		}
		else
		{
		    //let other penalties bring this down.
		    iValue += 600;
		    if (!pPlot->isStartingPlot())
		    {
                if (pArea->getNumStartingPlots() == 0)
                {
                    iValue += 1000;                    
                }
			}
		}
	}

	if (pPlot->isHills())
	{
		iValue += 200;
	}

	if (pPlot->isRiver())
	{
/************************************************************************************************/
/* Afforess	                  Start		 07/21/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		iValue += 100 * GC.getGameINLINE().getRiverBuildings();
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	}
/************************************************************************************************/
/* Afforess	                  Start		 04/01/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (pPlot->isFreshWater(true))
	{
		iValue += 40;
		iValue += (GC.getDefineINT("FRESH_WATER_HEALTH_CHANGE") * 30);
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


	if (bStartingLoc)
	{
		iRange = GREATER_FOUND_RANGE;
		int iGreaterBadTile = 0;

		for (iDX = -(iRange); iDX <= iRange; iDX++)
		{
			for (iDY = -(iRange); iDY <= iRange; iDY++)
			{
				pLoopPlot = plotXY(iX, iY, iDX, iDY);

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->isWater() || (pLoopPlot->area() == pArea))
					{
						if (plotDistance(iX, iY, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) <= iRange)
						{
						    iTempValue = 0;
							iTempValue += (pLoopPlot->getYield(YIELD_FOOD) * 15);
							iTempValue += (pLoopPlot->getYield(YIELD_PRODUCTION) * 11);
							iTempValue += (pLoopPlot->getYield(YIELD_COMMERCE) * 5);
							iValue += iTempValue;
							if (iTempValue < 21)
							{

								iGreaterBadTile += 2;
								if (pLoopPlot->getFeatureType() != NO_FEATURE)
								{
							    	if (pLoopPlot->calculateBestNatureYield(YIELD_FOOD,getTeam()) > 1)
							    	{
										iGreaterBadTile--;
							    	}
								}
							}
						}
					}
				}
			}
		}

		if (!pPlot->isStartingPlot())
		{
			iGreaterBadTile /= 2;
			if (iGreaterBadTile > 12)
			{
				iValue *= 11;
				iValue /= iGreaterBadTile;
			}
		}
		
		int iWaterCount = 0;
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
*/
		for (iI = 0; iI < NUM_CITY_PLOTS_2; iI++)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/		
		{
		    pLoopPlot = plotCity(iX, iY, iI);
            
            if (pLoopPlot != NULL)
		    {
		        if (pLoopPlot->isWater())
		        {
		            iWaterCount ++;
		            if (pLoopPlot->getYield(YIELD_FOOD) <= 1)
		            {
		                iWaterCount++;
					}
				}
			}
		}
		iWaterCount /= 2;
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
		int iLandCount = (NUM_CITY_PLOTS - iWaterCount);
*/
		int iLandCount = (NUM_CITY_PLOTS_2 - iWaterCount);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/		
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
		if (iLandCount < (NUM_CITY_PLOTS / 2))
*/
		if (iLandCount < (NUM_CITY_PLOTS_2 / 2))
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/		
		{
		    //discourage very water-heavy starts.
		    iValue *= 1 + iLandCount;
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
		    iValue /= (1 + (NUM_CITY_PLOTS / 2));
*/
		    iValue /= (1 + (NUM_CITY_PLOTS_2 / 2));
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		}
	}

	if (bStartingLoc)
	{
		if (pPlot->getMinOriginalStartDist() == -1)
		{
			iValue += (GC.getMapINLINE().maxStepDistance() * 100);
		}
		else
		{
			iValue *= (1 + 4 * pPlot->getMinOriginalStartDist());
			iValue /= (1 + 2 * GC.getMapINLINE().maxStepDistance());
		}
		
		//nice hacky way to avoid this messing with normalizer, use elsewhere?
		if (!pPlot->isStartingPlot())
		{
			int iMinDistanceFactor = MAX_INT;
			int iMinRange = startingPlotRange();
			
			iValue *= 100;
			for (int iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
			{
				if (GET_PLAYER((PlayerTypes)iJ).isAlive())
				{
					if (iJ != getID())
					{
						int iClosenessFactor = GET_PLAYER((PlayerTypes)iJ).startingPlotDistanceFactor(pPlot, getID(), iMinRange);
						iMinDistanceFactor = std::min(iClosenessFactor, iMinDistanceFactor);
						
						if (iClosenessFactor < 1000)
						{
							iValue *= 2000 + iClosenessFactor;
							iValue /= 3000;
						}
					}
				}
			}
			
			if (iMinDistanceFactor > 1000)
			{
				//give a maximum boost of 25% for somewhat distant locations, don't go overboard.
				iMinDistanceFactor = std::min(1500, iMinDistanceFactor);
				iValue *= (1000 + iMinDistanceFactor);				
				iValue /= 2000;
			}
			else if (iMinDistanceFactor < 1000)
			{
				//this is too close so penalize again.
				iValue *= iMinDistanceFactor;
				iValue /= 1000;
				iValue *= iMinDistanceFactor;
				iValue /= 1000;
			}
			
			iValue /= 10;
			
            if (pPlot->getBonusType() != NO_BONUS)
            {
                iValue /= 2;
            }
		}
	}
	
	if (bAdvancedStart)
	{
		if (pPlot->getBonusType() != NO_BONUS)
		{
			iValue *= 70;
			iValue /= 100;
		}
	}

	pNearestCity = GC.getMapINLINE().findCity(iX, iY, ((isBarbarian()) ? NO_PLAYER : getID()));

	if (pNearestCity != NULL)
	{
		if (isBarbarian())
		{
			iValue -= (std::max(0, (8 - plotDistance(iX, iY, pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE()))) * 200);
		}
		else
		{
		    int iDistance = plotDistance(iX, iY, pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE());
		    int iNumCities = getNumCities();
		    if (iDistance > 5)
		    {
		    	iValue -= (iDistance - 5) * 500;		    	
		    }
		    else if (iDistance < 4)
		    {
		    	iValue -= (4 - iDistance) * 2000;
		    }
			iValue *= (8 + iNumCities * 4);
			iValue /= (2 + (iNumCities * 4) + iDistance);
			if (pNearestCity->isCapital())
			{
				iValue *= 150;
				iValue /= 100;
			}
			else if (getCapitalCity() != NULL)
			{
				//Provide up to a 50% boost to value (80% for adv.start)
				//for city sites which are relatively close to the core
				//compared with the most distance city from the core
				//(having a boost rather than distance penalty avoids some distortion)
				
				//This is not primarly about maitenance but more about empire 
				//shape as such forbidden palace/state property are not big deal.
				CvCity* pLoopCity;
				int iLoop;
				int iMaxDistanceFromCapital = 0;
				
				int iCapitalX = getCapitalCity()->getX();
				int iCapitalY = getCapitalCity()->getY();

				for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
				{
					iMaxDistanceFromCapital = std::max(iMaxDistanceFromCapital, plotDistance(iCapitalX, iCapitalY, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE()));					
				}
				
				int iDistanceToCapital = plotDistance(iCapitalX, iCapitalY, iX, iY);
				
				FAssert(iMaxDistanceFromCapital > 0);
				iValue *= 100 + (((bAdvancedStart ? 80 : 50) * std::max(0, (iMaxDistanceFromCapital - iDistance))) / iMaxDistanceFromCapital);
				iValue /= 100;
			}
		}
	}
	else
	{
		pNearestCity = GC.getMapINLINE().findCity(iX, iY, ((isBarbarian()) ? NO_PLAYER : getID()), ((isBarbarian()) ? NO_TEAM : getTeam()), false);
		if (pNearestCity != NULL)
		{
			int iDistance = plotDistance(iX, iY, pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE());
			iValue -= std::min(500 * iDistance, (8000 * iDistance) / GC.getMapINLINE().maxPlotDistance());
		}
	}
	
	if (iValue <= 0)
	{
		return 1;
	}

	if (pArea->getNumCities() == 0)
	{
		iValue *= 2;
	}
	else
	{
		iTeamAreaCities = GET_TEAM(getTeam()).countNumCitiesByArea(pArea);

		if (pArea->getNumCities() == iTeamAreaCities)
		{
			iValue *= 3;
			iValue /= 2;
		}
		else if (pArea->getNumCities() == (iTeamAreaCities + GET_TEAM(BARBARIAN_TEAM).countNumCitiesByArea(pArea)))
		{
			iValue *= 4;
			iValue /= 3;
		}
		else if (iTeamAreaCities > 0)
		{
			iValue *= 5;
			iValue /= 4;
		}
	}

	if (!bStartingLoc)
	{
		int iFoodSurplus = std::max(0, iSpecialFoodPlus - iSpecialFoodMinus);
		int iFoodDeficit = std::max(0, iSpecialFoodMinus - iSpecialFoodPlus);

		iValue *= 100 + 20 * std::max(0, std::min(iFoodSurplus, 2 * GC.getFOOD_CONSUMPTION_PER_POPULATION()));
		iValue /= 100 + 20 * std::max(0, iFoodDeficit);
	}
	
	if ((!bStartingLoc) && (getNumCities() > 0))
	{
	    int iBonusCount = 0;
	    int iUniqueBonusCount = 0;
	    for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	    {
	        iBonusCount += paiBonusCount[iI];
	        iUniqueBonusCount += (paiBonusCount[iI] > 0) ? 1 : 0;
	    }
	    if (iBonusCount > 4)
	    {
	        iValue *= 5;
	        iValue /= (1 + iBonusCount);
	    }
	    else if (iUniqueBonusCount > 2)
	    {
	        iValue *= 5;
	        iValue /= (3 + iUniqueBonusCount);	        
	    }
	}
	
	if (!bStartingLoc)
	{
		int iDeadLockCount = AI_countDeadlockedBonuses(pPlot);
		if (bAdvancedStart && (iDeadLockCount > 0))
		{
			iDeadLockCount += 2;
		}
		iValue /= (1 + iDeadLockCount);
	}
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
	iValue /= (std::max(0, (iBadTile - (NUM_CITY_PLOTS / 4))) + 3);
*/
	iValue /= (std::max(0, (iBadTile - (NUM_CITY_PLOTS_2 / 4))) + 3);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


	if (bStartingLoc)
	{
		iDifferentAreaTile = 0;
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
*/
		for (iI = 0; iI < NUM_CITY_PLOTS_2; iI++)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/	
		{
			pLoopPlot = plotCity(iX, iY, iI);

			if ((pLoopPlot == NULL) || !(pLoopPlot->isWater() || pLoopPlot->area() == pArea))
			{
				iDifferentAreaTile++;
			}
		}
/************************************************************************************************/
/* Afforess	                  Start		 06/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
		iValue /= (std::max(0, (iDifferentAreaTile - ((NUM_CITY_PLOTS * 2) / 3))) + 2);
*/
		iValue /= (std::max(0, (iDifferentAreaTile - ((NUM_CITY_PLOTS_2 * 2) / 3))) + 2);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	}

	return std::max(1, iValue);
}


bool CvPlayerAI::AI_isAreaAlone(CvArea* pArea) const
{
	return ((pArea->getNumCities() - GET_TEAM(BARBARIAN_TEAM).countNumCitiesByArea(pArea)) == GET_TEAM(getTeam()).countNumCitiesByArea(pArea));
}


bool CvPlayerAI::AI_isCapitalAreaAlone() const
{
	CvCity* pCapitalCity;

	pCapitalCity = getCapitalCity();

	if (pCapitalCity != NULL)
	{
		return AI_isAreaAlone(pCapitalCity->area());
	}

	return false;
}


bool CvPlayerAI::AI_isPrimaryArea(CvArea* pArea) const
{
	CvCity* pCapitalCity;

	if (pArea->isWater())
	{
		return false;
	}

	if (pArea->getCitiesPerPlayer(getID()) > 2)
	{
		return true;
	}

	pCapitalCity = getCapitalCity();

	if (pCapitalCity != NULL)
	{
		if (pCapitalCity->area() == pArea)
		{
			return true;
		}
	}

	return false;
}


int CvPlayerAI::AI_militaryWeight(CvArea* pArea) const
{
	return (pArea->getPopulationPerPlayer(getID()) + pArea->getCitiesPerPlayer(getID()) + 1);
}


int CvPlayerAI::AI_targetCityValue(CvCity* pCity, bool bRandomize, bool bIgnoreAttackers) const
{
	PROFILE_FUNC();

	CvCity* pNearestCity;
	CvPlot* pLoopPlot;
	int iValue;
	int iI;

	FAssertMsg(pCity != NULL, "City is not assigned a valid value");

	iValue = 1;

	iValue += ((pCity->getPopulation() * (50 + pCity->calculateCulturePercent(getID()))) / 100);

	if (pCity->getDefenseDamage() > 0)
	{
		iValue += ((pCity->getDefenseDamage() / 30) + 1);
	}

	if (pCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
	{
		iValue++;
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/16/10                     Mongoose & jdog5000      */
/*                                                                                              */
/* War strategy AI                                                                              */
/************************************************************************************************/
	iValue += 2*pCity->getNumActiveWorldWonders();

	for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (pCity->isHolyCity((ReligionTypes)iI))
		{
			iValue += 3;

			if( getStateReligion() == iI )
			{
				iValue += 4;
			}
		}
	}

	if (pCity->isEverOwned(getID()))
	{
		iValue += 3;
	}
	if (!bIgnoreAttackers)
	{
		iValue += std::min( 8, (AI_adjacentPotentialAttackers(pCity->plot()) + 2)/3 );
	}

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = plotCity(pCity->getX_INLINE(), pCity->getY_INLINE(), iI);

		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
			{
				iValue += std::min(4,std::max(1, AI_bonusVal(pLoopPlot->getBonusType(getTeam()))/10));
			}

			if (pLoopPlot->getOwnerINLINE() == getID())
			{
				iValue++;
			}

			if (pLoopPlot->isAdjacentPlayer(getID(), true))
			{
				iValue++;
			}
		}
	}

	if( GET_PLAYER(pCity->getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3) )
	{
		if( pCity->getCultureLevel() >= (GC.getGameINLINE().culturalVictoryCultureLevel() - 1) )
		{
			iValue += 10;
			
			if( GET_PLAYER(pCity->getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_CULTURE4) )
			{
				iValue += 10;

				if( pCity->getCultureLevel() >= (GC.getGameINLINE().culturalVictoryCultureLevel()) )
				{
					iValue += 10;
				}
			}
		}
	}

	if( GET_PLAYER(pCity->getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_SPACE3) )
	{
		if( pCity->isCapital() )
		{
			iValue += 10;

			if( GET_PLAYER(pCity->getOwnerINLINE()).AI_isDoVictoryStrategy(AI_VICTORY_SPACE4) )
			{
				iValue += 10;

				if( GET_TEAM(pCity->getTeam()).getVictoryCountdown(GC.getGameINLINE().getSpaceVictory()) >= 0 )
				{
					iValue += 30;
				}
			}
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	pNearestCity = GC.getMapINLINE().findCity(pCity->getX_INLINE(), pCity->getY_INLINE(), getID());

	if (pNearestCity != NULL)
	{
		iValue += std::max(1, ((GC.getMapINLINE().maxStepDistance() * 2) - GC.getMapINLINE().calculatePathDistance(pNearestCity->plot(), pCity->plot())));
	}

	if (bRandomize)
	{
		iValue += GC.getGameINLINE().getSorenRandNum(((pCity->getPopulation() / 2) + 1), "AI Target City Value");
	}

	return iValue;
}


CvCity* CvPlayerAI::AI_findTargetCity(CvArea* pArea) const
{
	CvCity* pLoopCity;
	CvCity* pBestCity;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	iBestValue = 0;
	pBestCity = NULL;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (isPotentialEnemy(getTeam(), GET_PLAYER((PlayerTypes)iI).getTeam()))
			{
				for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
				{
					if (pLoopCity->area() == pArea)
					{
						iValue = AI_targetCityValue(pLoopCity, true);

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestCity = pLoopCity;
						}
					}
				}
			}
		}
	}

	return pBestCity;
}


bool CvPlayerAI::AI_isCommercePlot(CvPlot* pPlot) const
{
	return (pPlot->getYield(YIELD_FOOD) >= GC.getFOOD_CONSUMPTION_PER_POPULATION());
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/20/09                                jdog5000      */
/*                                                                                              */
/* General AI, Efficiency                                                                       */
/************************************************************************************************/
// Plot danger cache

// The vast majority of checks for plot danger are boolean checks during path planning for non-combat
// units like workers, settlers, and GP.  Since these are simple checks for any danger they can be 
// cutoff early if danger is found.  To this end, the two caches tracked are for whether a given plot
// is either known to be safe for the player who is currently moving, or for whether the plot is
// known to be a plot bordering an enemy of this team and therefore unsafe.
//
// The safe plot cache is only for the active moving player and is only set if this is not a
// multiplayer game with simultaneous turns.  The safety cache for all plots is reset when the active
// player changes or a new game is loaded.
// 
// The border cache is done by team and works for all game types.  The border cache is reset for all
// plots when war or peace are declared, and reset over a limited range whenever a ownership over a plot
// changes.
bool CvPlayerAI::AI_getAnyPlotDanger(CvPlot* pPlot, int iRange, bool bTestMoves) const
{
	PROFILE_FUNC();

	if (iRange == -1)
	{
		iRange = DANGER_RANGE;
	}

	if( bTestMoves && isTurnActive() )
	{
		if( (iRange <= DANGER_RANGE) && pPlot->isActivePlayerNoDangerCache() )
		{
			return false;
		}
	}

	TeamTypes eTeam = getTeam();
	bool bCheckBorder = (!isHuman() && !pPlot->isCity());
	
	if( bCheckBorder )
	{
		if( (iRange >= DANGER_RANGE) && pPlot->isTeamBorderCache(eTeam) )
		{
			return true;
		}
	}

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iDistance;
	int iDX, iDY;
	CvArea *pPlotArea = pPlot->area();
	int iDangerRange;

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot	= plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pPlotArea)
				{
				    iDistance = stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
				    if( bCheckBorder )
					{
						if (atWar(pLoopPlot->getTeam(), eTeam))
						{
							// Border cache is reversible, set for both team and enemy
							if (iDistance == 1)
							{
								pPlot->setIsTeamBorderCache(eTeam, true);
								pPlot->setIsTeamBorderCache(pLoopPlot->getTeam(), true);
								pLoopPlot->setIsTeamBorderCache(eTeam, true);
								pLoopPlot->setIsTeamBorderCache(pLoopPlot->getTeam(), true);
								return true;
							}
							else if ((iDistance == 2) && (pLoopPlot->isRoute()))
							{
								pPlot->setIsTeamBorderCache(eTeam, true);
								pPlot->setIsTeamBorderCache(pLoopPlot->getTeam(), true);
								pLoopPlot->setIsTeamBorderCache(eTeam, true);
								pLoopPlot->setIsTeamBorderCache(pLoopPlot->getTeam(), true);
								return true;
							}
						}
					}

					pUnitNode = pLoopPlot->headUnitNode();

					while (pUnitNode != NULL)
					{
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

						// No need to loop over tiles full of our own units
						if( pLoopUnit->getTeam() == eTeam )
						{
							if( !(pLoopUnit->alwaysInvisible()) && (pLoopUnit->getInvisibleType() == NO_INVISIBLE) )
							{
								break;
							}
						}

						if (pLoopUnit->isEnemy(eTeam))
						{
							if (pLoopUnit->canAttack())
							{
								if (!(pLoopUnit->isInvisible(eTeam, false)))
								{
								    if (pLoopUnit->canMoveOrAttackInto(pPlot))
								    {
                                        if (!bTestMoves)
                                        {
                                            return true;
                                        }
                                        else
                                        {
                                            iDangerRange = pLoopUnit->baseMoves();
                                            iDangerRange += ((pLoopPlot->isValidRoute(pLoopUnit)) ? 1 : 0);
                                            if (iDangerRange >= iDistance)
											{
												return true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}


	// The test moves case is a strict subset of the more general case,
	// either is appropriate for setting the cache.  However, since the test moves
	// case is called far more frequently, it is more important and the cache 
	// value being true is only assumed to mean that the plot is safe in the
	// test moves case.
	//if( bTestMoves )
	{
		if( isTurnActive() )
		{
			if( !(GC.getGameINLINE().isMPOption(MPOPTION_SIMULTANEOUS_TURNS)) && (GC.getGameINLINE().getNumGameTurnActive() == 1) )
			{
				pPlot->setIsActivePlayerNoDangerCache(true);
			}
		}
	}

	return false;
}


int CvPlayerAI::AI_getPlotDanger(CvPlot* pPlot, int iRange, bool bTestMoves) const
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iCount;
	int iDistance;
	int iBorderDanger;
	int iDX, iDY;
	CvArea *pPlotArea = pPlot->area();
	int iDangerRange;
	TeamTypes eTeam = getTeam();

	iCount = 0;
	iBorderDanger = 0;

	if (iRange == -1)
	{
		iRange = DANGER_RANGE;
	}

	if( bTestMoves && isTurnActive() )
	{
		if( (iRange <= DANGER_RANGE) && pPlot->isActivePlayerNoDangerCache() )
		{
			return 0;
		}
	}

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot	= plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pPlotArea)
				{
				    iDistance = stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
				    if (atWar(pLoopPlot->getTeam(), eTeam))
				    {
				        if (iDistance == 1)
				        {
				            iBorderDanger++;
				        }
				        else if ((iDistance == 2) && (pLoopPlot->isRoute()))
				        {
				            iBorderDanger++;
				        }
				    }


					pUnitNode = pLoopPlot->headUnitNode();

					while (pUnitNode != NULL)
					{
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

						// No need to loop over tiles full of our own units
						if( pLoopUnit->getTeam() == eTeam )
						{
							if( !(pLoopUnit->alwaysInvisible()) && (pLoopUnit->getInvisibleType() == NO_INVISIBLE) )
							{
								break;
							}
						}

						if (pLoopUnit->isEnemy(eTeam))
						{
							if (pLoopUnit->canAttack())
							{
								if (!(pLoopUnit->isInvisible(eTeam, false)))
								{
								    if (pLoopUnit->canMoveOrAttackInto(pPlot))
								    {
                                        if (!bTestMoves)
                                        {
                                            iCount++;
                                        }
                                        else
                                        {
                                            iDangerRange = pLoopUnit->baseMoves();
                                            iDangerRange += ((pLoopPlot->isValidRoute(pLoopUnit)) ? 1 : 0);
                                            if (iDangerRange >= iDistance)
											{
												iCount++;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (iBorderDanger > 0)
	{
	    if (!isHuman() && !pPlot->isCity())
	    {
            iCount += iBorderDanger;
	    }
	}

	return iCount;
}

// Never used ...
/*
int CvPlayerAI::AI_getUnitDanger(CvUnit* pUnit, int iRange, bool bTestMoves, bool bAnyDanger) const
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iCount;
	int iDistance;
	int iBorderDanger;
	int iDX, iDY;

    CvPlot* pPlot = pUnit->plot();
	iCount = 0;
	iBorderDanger = 0;

	if (iRange == -1)
	{
		iRange = DANGER_RANGE;
	}
	
	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pPlot->area())
				{
				    iDistance = stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
				    if (atWar(pLoopPlot->getTeam(), getTeam()))
				    {
				        if (iDistance == 1)
				        {
				            iBorderDanger++;
				        }
				        else if ((iDistance == 2) && (pLoopPlot->isRoute()))
				        {
				            iBorderDanger++;
				        }
				    }


					pUnitNode = pLoopPlot->headUnitNode();

					while (pUnitNode != NULL)
					{
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

						if (atWar(pLoopUnit->getTeam(), getTeam()))
						{
							if (pLoopUnit->canAttack())
							{
								if (!(pLoopUnit->isInvisible(getTeam(), false)))
								{
								    if (pLoopUnit->canMoveOrAttackInto(pPlot))
								    {
                                        if (!bTestMoves)
                                        {
                                            iCount++;
                                        }
                                        else
                                        {
                                            int iDangerRange = pLoopUnit->baseMoves();
                                            iDangerRange += ((pLoopPlot->isValidRoute(pLoopUnit)) ? 1 : 0);
                                            if (iDangerRange >= iDistance)
                                            {
                                                iCount++;
                                            }
                                        }
								    }
								}
							}
						}
					}
				}
			}
		}
	}

	if (iBorderDanger > 0)
	{
	    if (!isHuman() || pUnit->isAutomated())
	    {
            iCount += iBorderDanger;
	    }
	}

	return iCount;
}
*/
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/


int CvPlayerAI::AI_getWaterDanger(CvPlot* pPlot, int iRange, bool bTestMoves) const
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iCount;
	int iDX, iDY;

	iCount = 0;

	if (iRange == -1)
	{
		iRange = DANGER_RANGE;
	}
	
	CvArea* pWaterArea = pPlot->waterArea();

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->isWater())
				{
					if (pPlot->isAdjacentToArea(pLoopPlot->getArea()))
					{
						pUnitNode = pLoopPlot->headUnitNode();

						while (pUnitNode != NULL)
						{
							pLoopUnit = ::getUnit(pUnitNode->m_data);
							pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

							if (pLoopUnit->isEnemy(getTeam()))
							{
								if (pLoopUnit->canAttack())
								{
									if (!(pLoopUnit->isInvisible(getTeam(), false)))
									{
										iCount++;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	return iCount;
}


bool CvPlayerAI::AI_avoidScience() const
{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
	if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE4))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
    {
        return true;
    }
	if (isCurrentResearchRepeat())
	{
		return true;
	}

	if (isNoResearchAvailable())
	{
		return true;
	}

	return false;
}


// XXX
bool CvPlayerAI::AI_isFinancialTrouble() const
{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      06/12/09                                jdog5000      */
/*                                                                                              */
/* Barbarian AI                                                                                 */
/************************************************************************************************/
	if( isBarbarian() )
	{
		return false;
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	//if (getCommercePercent(COMMERCE_GOLD) > 50)
	{
		int iNetCommerce = 1 + getCommerceRate(COMMERCE_GOLD) + getCommerceRate(COMMERCE_RESEARCH) + std::max(0, getGoldPerTurn());
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       06/11/09                       jdog5000 & DanF5771    */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original BTS code
		int iNetExpenses = calculateInflatedCosts() + std::min(0, getGoldPerTurn());
*/
		int iNetExpenses = calculateInflatedCosts() + std::max(0, -getGoldPerTurn());
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/		
		
		int iFundedPercent = (100 * (iNetCommerce - iNetExpenses)) / std::max(1, iNetCommerce);
/************************************************************************************************/
/* Afforess	                  Start		 04/24/10                                              */
/*                                                                                              */
/*   Increased from 40% to 50%                                                                  */
/************************************************************************************************/
		int iSafePercent = 50;
		
		if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_REVOLUTION))
		{
			iSafePercent += 15;
		}
		int iWarSuccessRatio = GET_TEAM(getTeam()).AI_getWarSuccessCapitulationRatio();
		if (iWarSuccessRatio < -30)
		{
			iSafePercent -= std::max(20, iWarSuccessRatio / 3);
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		if (AI_avoidScience())
		{
			iSafePercent -= 8;
		}
		
		if (GET_TEAM(getTeam()).getAnyWarPlanCount(true))
		{
			iSafePercent -= 12;
		}
		
		if (isCurrentResearchRepeat())
		{
			iSafePercent -= 10;
		}
		
		if (iFundedPercent < iSafePercent)
		{
			return true;
		}
	}

	return false;
}

/************************************************************************************************/
/* Afforess	                  Start		 02/01/10                                               */
/*                                                                                              */
/*  This function has been re-written                                                           */
/************************************************************************************************/
int CvPlayerAI::AI_goldTarget() const
{
	int iGold = 0;

	int iMultiplier = 0;
	iMultiplier += GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent();
	iMultiplier += GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();
	iMultiplier += GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
	iMultiplier /= 3;

	iGold += ((getNumCities() * 3) + (getTotalPopulation() / 3));

	iGold += (GC.getGameINLINE().getElapsedGameTurns() / 8);
	
	iGold *= (100 + calculateInflationRate());
	iGold /= 100;

	iGold *= iMultiplier;
	iGold /= 100;
	
	bool bAnyWar = GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0;
	if (bAnyWar)
	{
		iGold *= 3;
		iGold /= 2;
	}

// Don't bother saving gold if we can't trade it for anything
	if (!GET_TEAM(getTeam()).isGoldTrading() || !(GET_TEAM(getTeam()).isTechTrading()) || (GC.getGameINLINE().isOption(GAMEOPTION_NO_TECH_TRADING)))
	{
		iGold /= 3;
	}

	if (AI_avoidScience())
	{
		iGold *= 10;
	}

	iGold += (AI_goldToUpgradeAllUnits() / (bAnyWar ? 1 : 2));

	CorporationTypes eActiveCorporation = NO_CORPORATION;
	for (int iI = 0; iI < GC.getNumCorporationInfos(); iI++)
	{
		if (getHasCorporationCount((CorporationTypes)iI) > 0)
		{
			eActiveCorporation = (CorporationTypes)iI;
			break;
		}
	}
	if (eActiveCorporation != NO_CORPORATION)
	{
		int iSpreadCost = std::max(0, GC.getCorporationInfo(eActiveCorporation).getSpreadCost() * (100 + calculateInflationRate()));
		iSpreadCost /= 50;
		iGold += iSpreadCost;
	}

	return iGold + AI_getExtraGoldTarget();
}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/28/09                                jdog5000      */
/*                                                                                              */
/* Tech AI                                                                                      */
/************************************************************************************************/
TechTypes CvPlayerAI::AI_bestTech(int iMaxPathLength, bool bIgnoreCost, bool bAsync, TechTypes eIgnoreTech, AdvisorTypes eIgnoreAdvisor) const
{
	PROFILE("CvPlayerAI::AI_bestTech");

	int iValue;
	int iBestValue = 0;
	TechTypes eBestTech = NO_TECH;
	int iPathLength;
	CvTeam& kTeam = GET_TEAM(getTeam());
/************************************************************************************************/
/* Afforess	                  Start		 08/09/10                                               */
/*                                                                                              */
/*  Forces AI to Beeline for Religious Techs if they have no religions                          */
/************************************************************************************************/
	bool bValid = GC.getGameINLINE().isOption(GAMEOPTION_RUTHLESS_AI);
	if (!bValid)
	{
		for (int iI = 0; iI < GC.getNumTraitInfos(); iI++)
		{
			if (hasTrait((TraitTypes)iI))
			{
				if (GC.getTraitInfo((TraitTypes)iI).getMaxAnarchy() >= 0)
				{
					bValid = true;
					break;
				}
			}
		}
	}
	if (bValid)
	{
		if (getCommercePercent(COMMERCE_RESEARCH) < 90)
		{
			bValid = false;
		}
		if (countHolyCities() > 0 && (!GC.getGameINLINE().isOption(GAMEOPTION_NO_REVOLUTION) || GC.getGameINLINE().isOption(GAMEOPTION_LIMITED_RELIGIONS)))
		{
			bValid = false;
		}
		if (GET_TEAM(getTeam()).getAnyWarPlanCount(true) == 0)
		{
			bValid = false;
		}
		if (getNumCities() == 1)
		{
			bValid = false;
		}
	}
	if (bValid)
	{
		eBestTech = AI_bestReligiousTech(iMaxPathLength * 3, eIgnoreTech, eIgnoreAdvisor);
		if (eBestTech != NO_TECH)
		{
			return eBestTech;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	int* paiBonusClassRevealed;
	int* paiBonusClassUnrevealed;
	int* paiBonusClassHave;
	
	paiBonusClassRevealed = new int[GC.getNumBonusClassInfos()];
	paiBonusClassUnrevealed = new int[GC.getNumBonusClassInfos()];
	paiBonusClassHave = new int[GC.getNumBonusClassInfos()];
	
	for (int iI = 0; iI < GC.getNumBonusClassInfos(); iI++)
	{
        paiBonusClassRevealed[iI] = 0;
        paiBonusClassUnrevealed[iI] = 0;
        paiBonusClassHave[iI] = 0;	    
	}
	
	for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
	    TechTypes eRevealTech = (TechTypes)GC.getBonusInfo((BonusTypes)iI).getTechReveal();
	    BonusClassTypes eBonusClass = (BonusClassTypes)GC.getBonusInfo((BonusTypes)iI).getBonusClassType();
	    if (eRevealTech != NO_TECH)
	    {
	        if ((kTeam.isHasTech(eRevealTech)))
	        {
	            paiBonusClassRevealed[eBonusClass]++;
	        }
	        else
	        {
	            paiBonusClassUnrevealed[eBonusClass]++;
	        }

            if (getNumAvailableBonuses((BonusTypes)iI) > 0)
            {
                paiBonusClassHave[eBonusClass]++;                
            }
            else if (countOwnedBonuses((BonusTypes)iI) > 0)
            {
                paiBonusClassHave[eBonusClass]++;
            }
	    }
	}

#ifdef DEBUG_TECH_CHOICES
	CvWString szPlayerName = getName();
	DEBUGLOG("AI_bestTech:%S\n", szPlayerName.GetCString());
#endif

	for (int iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		if ((eIgnoreTech == NO_TECH) || (iI != eIgnoreTech))
		{
			if ((eIgnoreAdvisor == NO_ADVISOR) || (GC.getTechInfo((TechTypes)iI).getAdvisorType() != eIgnoreAdvisor))
			{
				if (canEverResearch((TechTypes)iI))
				{
					if (!(kTeam.isHasTech((TechTypes)iI)))
					{
						if (GC.getTechInfo((TechTypes)iI).getEra() <= (getCurrentEra() + 1))
						{
							iPathLength = findPathLength(((TechTypes)iI), false);

							if (iPathLength <= iMaxPathLength)
							{
								iValue = AI_techValue( (TechTypes)iI, iPathLength, bIgnoreCost, bAsync, paiBonusClassRevealed, paiBonusClassUnrevealed, paiBonusClassHave );

								/*if( gPlayerLogLevel >= 3 )
								{
									logBBAI("      Player %d (%S) consider tech %S with value %d", getID(), getCivilizationDescription(0), GC.getTechInfo((TechTypes)iI).getDescription(), iValue );
								}*/

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestTech = ((TechTypes)iI);
								}
							}
						}
					}
				}
			}
		}
	}

	if( gPlayerLogLevel >= 1 && eBestTech != NO_TECH )
	{
		logBBAI("  Player %d (%S) selects tech %S with value %d", getID(), getCivilizationDescription(0), GC.getTechInfo(eBestTech).getDescription(), iBestValue );
	}

    SAFE_DELETE_ARRAY(paiBonusClassRevealed);
    SAFE_DELETE_ARRAY(paiBonusClassUnrevealed);
    SAFE_DELETE_ARRAY(paiBonusClassHave);	

	return eBestTech;
}

int CvPlayerAI::AI_techValue( TechTypes eTech, int iPathLength, bool bIgnoreCost, bool bAsync, int* paiBonusClassRevealed, int* paiBonusClassUnrevealed, int* paiBonusClassHave ) const
{
	PROFILE_FUNC();

	CvCity* pCapitalCity;
	ImprovementTypes eImprovement;
	RouteTypes eRoute;
	
	int iNumBonuses;
	int iValue;
	int iTempValue;
	int iBuildValue;
	int iBonusValue;
	//int iI, iJ, iK, iL;

	pCapitalCity = getCapitalCity();

	CvTeam& kTeam = GET_TEAM(getTeam());

	//bool bWarPlan = (kTeam.getAnyWarPlanCount(true) > 0);
	bool bCapitalAlone = (GC.getGameINLINE().getElapsedGameTurns() > 0) ? AI_isCapitalAreaAlone() : false;
	bool bFinancialTrouble = AI_isFinancialTrouble();
	bool bAdvancedStart = getAdvancedStartPoints() >= 0;

	int iHasMetCount = kTeam.getHasMetCivCount(true);
	int iCoastalCities = countNumCoastalCities();
	int iConnectedForeignCities = countPotentialForeignTradeCitiesConnected();

	int iCityCount = getNumCities();
	//int iTeamCityCount = kTeam.getNumCities();
	
	iValue = 1;

	int iRandomFactor = ((bAsync) ? GC.getASyncRand().get(2000, "AI Research ASYNC") : GC.getGameINLINE().getSorenRandNum(2000, "AI Research"));
	int iRandomMax = 2000;
	iValue += iRandomFactor;

	iValue += kTeam.getResearchProgress(eTech);

	// Map stuff
	if (GC.getTechInfo(eTech).isExtraWaterSeeFrom())
	{
		if (iCoastalCities > 0)
		{
			iValue += 100;

			if (bCapitalAlone)
			{
				iValue += 400;
			}
		}
	}

	if (GC.getTechInfo(eTech).isMapCentering())
	{
		iValue += 100;
	}

	if (GC.getTechInfo(eTech).isMapVisible())
	{
		iValue += 100;

		if (bCapitalAlone)
		{
			iValue += 400;
		}
	}

	// Expand trading options
	if (GC.getTechInfo(eTech).isMapTrading())
	{
		iValue += 100;

		if (bCapitalAlone)
		{
			iValue += 400;
		}
	}

	if (GC.getTechInfo(eTech).isTechTrading() && !GC.getGameINLINE().isOption(GAMEOPTION_NO_TECH_TRADING))
	{
		iValue += 500;

		iValue += 500 * iHasMetCount;
	}

	if (GC.getTechInfo(eTech).isGoldTrading())
	{
		iValue += 200;

		if (iHasMetCount > 0)
		{
			iValue += 400;
		}
	}

	if (GC.getTechInfo(eTech).isOpenBordersTrading())
	{
		if (iHasMetCount > 0)
		{
			iValue += 500;

			if (iCoastalCities > 0)
			{
				iValue += 400;
			}
/************************************************************************************************/
/* REVOLUTION_MOD                         05/30/08                                jdog5000      */
/*                                                                                              */
/* Revolution AI                                                                                */
/************************************************************************************************/
			if( isMinorCiv() && GC.getGame().isOption(GAMEOPTION_START_AS_MINORS) )
			{
				iValue += 250 + 120*iHasMetCount;
			}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
		}	
	}

	if (GC.getTechInfo(eTech).isDefensivePactTrading())
	{
		iValue += 400;
	}

	if (GC.getTechInfo(eTech).isPermanentAllianceTrading() && (GC.getGameINLINE().isOption(GAMEOPTION_PERMANENT_ALLIANCES)))
	{
		iValue += 200;
	}

	if (GC.getTechInfo(eTech).isVassalStateTrading() && !(GC.getGameINLINE().isOption(GAMEOPTION_NO_VASSAL_STATES)))
	{
		iValue += 200;
	}

	// Tile improvement abilities
	if (GC.getTechInfo(eTech).isBridgeBuilding())
	{
		iValue += 200;
	}

	if (GC.getTechInfo(eTech).isIrrigation())
	{
		iValue += 400;
	}

	if (GC.getTechInfo(eTech).isIgnoreIrrigation())
	{
		iValue += 500;
	}

	if (GC.getTechInfo(eTech).isWaterWork())
	{
		iValue += (600 * iCoastalCities);
	}
/************************************************************************************************/
/* Afforess	                  Start		 03/7/10                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	CvTechInfo& kTech = GC.getTechInfo(eTech);
	int iLoop;
	iTempValue = 0;
	if (kTech.isCanPassPeaks())
	{
		if (!GC.getGameINLINE().isOption(GAMEOPTION_MOUNTAINS))
		{
			CvPlot* pPlot;
			for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
			{
				pPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
				if (pPlot->isPeak())
				{
					if (pPlot->getOwnerINLINE() != NO_PLAYER)
					{
						if (GET_PLAYER(pPlot->getOwnerINLINE()).getID() == getID())
						{
							iTempValue += 35;
						}
					}
				}
			}
		}
		iValue += iTempValue;
		iTempValue = 0;
	}
	
	if (kTech.isMoveFastPeaks())
	{
		iValue += 150;
	}
	
	if (kTech.isCanFoundOnPeaks())
	{
		iValue += 100;
	}
	
	iTempValue = 0;
	for (int iI = 0; iI < GC.getNumCorporationInfos(); iI++)
	{
		if (GC.getCorporationInfo((CorporationTypes)iI).getObsoleteTech() != NO_TECH)
		{
			if (GC.getCorporationInfo((CorporationTypes)iI).getObsoleteTech() == eTech)
			{
				for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
				{
					if (pLoopCity->isHasCorporation((CorporationTypes)iI))
					{
						iTempValue -= AI_corporationValue((CorporationTypes)iI, pLoopCity);
					}
				}
			}
		}
	}
	iValue += iTempValue / 1000;
	iTempValue = 0;
	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (GC.getPromotionInfo((PromotionTypes)iI).getObsoleteTech() != NO_TECH)
		{
			if (GC.getPromotionInfo((PromotionTypes)iI).getObsoleteTech() == eTech)
			{
				for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
				{
					if (pLoopUnit->isHasPromotion((PromotionTypes)iI))
					{
						iTempValue -= AI_promotionValue((PromotionTypes)iI, pLoopUnit->getUnitType(), pLoopUnit);
					}
				}
			}
		}
	}
	iValue += iTempValue / 100;
	iTempValue = 0;
	
	if (kTech.isRebaseAnywhere() && GC.getDefineINT("MAX_AIRLIFT_RANGE") > 0)
	{
		iValue += 300;
	}
	
	for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		if (kTech.getFreeSpecialistCount(iI) != 0)
		{
			iValue += 50 * getNumCities() * kTech.getFreeSpecialistCount(iI);
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	iValue += (GC.getTechInfo(eTech).getFeatureProductionModifier() * 2);
	iValue += (GC.getTechInfo(eTech).getWorkerSpeedModifier() * 4);
	iValue += (GC.getTechInfo(eTech).getTradeRoutes() * (std::max((getNumCities() + 2), iConnectedForeignCities) + 1) * ((bFinancialTrouble) ? 200 : 100));
	
	if ( AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION4) )
	{
		iValue += (GC.getTechInfo(eTech).getHealth() * 350);
	}
	else
	{
		iValue += (GC.getTechInfo(eTech).getHealth() * 200);
	}

	for (int iJ = 0; iJ < GC.getNumRouteInfos(); iJ++)
	{
		iValue += -(GC.getRouteInfo((RouteTypes)iJ).getTechMovementChange(eTech) * 100);
	}

	for (int iJ = 0; iJ < NUM_DOMAIN_TYPES; iJ++)
	{
		iValue += (GC.getTechInfo(eTech).getDomainExtraMoves(iJ) * 200);
	}

	for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
	{
		if (GC.getTechInfo(eTech).isCommerceFlexible(iJ))
		{
			iValue += 100;
			if ((iJ == COMMERCE_CULTURE) && (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2)))
			{
				iValue += 1000;
			}
		}
	}

	for (int iJ = 0; iJ < GC.getNumTerrainInfos(); iJ++)
	{
		if (GC.getTechInfo(eTech).isTerrainTrade(iJ))
		{
			if (GC.getTerrainInfo((TerrainTypes)iJ).isWater())
			{
				if (pCapitalCity != NULL)
				{
					iValue += (countPotentialForeignTradeCities(pCapitalCity->area()) * 100);
				}

				if (iCoastalCities > 0)
				{
					iValue += ((bCapitalAlone) ? 950 : 350);
				}

				iValue += 50;
			}
			else
			{
				iValue += 1000;
			}
		}
	}

	if (GC.getTechInfo(eTech).isRiverTrade())
	{
		iValue += 1000;
	}

	/* ------------------ Tile Improvement Value  ------------------ */
	for (int iJ = 0; iJ < GC.getNumImprovementInfos(); iJ++)
	{
		for (int iK = 0; iK < NUM_YIELD_TYPES; iK++)
		{
			iTempValue = 0;

			iTempValue += (GC.getImprovementInfo((ImprovementTypes)iJ).getTechYieldChanges(eTech, iK) * getImprovementCount((ImprovementTypes)iJ) * 50);

			iTempValue *= AI_yieldWeight((YieldTypes)iK);
			iTempValue /= 100;

			iValue += iTempValue;
		}
	}

	iBuildValue = 0;
	for (int iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
	{
		if (GC.getBuildInfo((BuildTypes)iJ).getTechPrereq() == eTech)
		{
			eImprovement = (ImprovementTypes)(GC.getBuildInfo((BuildTypes)iJ).getImprovement());
			if (eImprovement != NO_IMPROVEMENT)
			{
				eImprovement = finalImprovementUpgrade(eImprovement);
			}
			else
			{
				// only increment build value if it is not an improvement, otherwise handle it there
				iBuildValue += 200;
			}

			if (eImprovement != NO_IMPROVEMENT)
			{
				CvImprovementInfo& kImprovement = GC.getImprovementInfo(eImprovement);
				
				int iImprovementValue = 300;

				iImprovementValue += ((kImprovement.isActsAsCity()) ? 100 : 0);
				iImprovementValue += ((kImprovement.isHillsMakesValid()) ? 100 : 0);
				iImprovementValue += ((kImprovement.isFreshWaterMakesValid()) ? 200 : 0);
				iImprovementValue += ((kImprovement.isRiverSideMakesValid()) ? 100 : 0);
				iImprovementValue += ((kImprovement.isCarriesIrrigation()) ? 300 : 0);

				for (int iK = 0; iK < GC.getNumTerrainInfos(); iK++)
				{
					iImprovementValue += (kImprovement.getTerrainMakesValid(iK) ? 50 : 0);
/************************************************************************************************/
/* Afforess	                  Start		 05/22/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
					//Desert has negative defense
					if (GC.getTerrainInfo((TerrainTypes)iK).getDefenseModifier() < 0)
					{
						if (GET_TEAM(getTeam()).isCanFarmDesert())
						{
							iImprovementValue += 50;
						}
					}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				}

				for (int iK = 0; iK < GC.getNumFeatureInfos(); iK++)
				{
					iImprovementValue += (kImprovement.getFeatureMakesValid(iK) ? 50 : 0);
				}

				for (int iK = 0; iK < NUM_YIELD_TYPES; iK++)
				{
					iTempValue = 0;

					iTempValue += (kImprovement.getYieldChange(iK) * 200);
					iTempValue += (kImprovement.getRiverSideYieldChange(iK) * 100);
					iTempValue += (kImprovement.getHillsYieldChange(iK) * 100);
					iTempValue += (kImprovement.getIrrigatedYieldChange(iK) * 150);

					// land food yield is more valueble
					if (iK == YIELD_FOOD && !kImprovement.isWater())
					{
						iTempValue *= 3;
						iTempValue /= 2;
					}
					
					if (bFinancialTrouble && iK == YIELD_COMMERCE)
					{
						iTempValue *= 2;
					}

					iTempValue *= AI_yieldWeight((YieldTypes)iK);
					iTempValue /= 100;

					iImprovementValue += iTempValue;
				}

				for (int iK = 0; iK < GC.getNumBonusInfos(); iK++)
				{
					iBonusValue = 0;

					iBonusValue += ((kImprovement.isImprovementBonusMakesValid(iK)) ? 450 : 0);
					iBonusValue += ((kImprovement.isImprovementBonusTrade(iK)) ? (45 * AI_bonusVal((BonusTypes) iK)) : 0);

					if (iBonusValue > 0)
					{
						for (int iL = 0; iL < NUM_YIELD_TYPES; iL++)
						{
							iTempValue = 0;

							iTempValue += (kImprovement.getImprovementBonusYield(iK, iL) * 300);
							iTempValue += (kImprovement.getIrrigatedYieldChange(iL) * 200);

							// food bonuses are more valueble
							if (iL == YIELD_FOOD)
							{
								iTempValue *= 2;
							}
							// otherwise, devalue the bonus slightly
							else if (iL == YIELD_COMMERCE && bFinancialTrouble)
							{
								iTempValue *= 4;
								iTempValue /= 3;
							}
							else
							{
								iTempValue *= 3;
								iTempValue /= 4;
							}
							
							if (bAdvancedStart && getCurrentEra() < 2)
							{
								iValue *= (iL == YIELD_FOOD) ? 3 : 2;
							}

							iTempValue *= AI_yieldWeight((YieldTypes)iL);
							iTempValue /= 100;

							iBonusValue += iTempValue;
						}

						iNumBonuses = countOwnedBonuses((BonusTypes)iK);

						if (iNumBonuses > 0)
						{
							iBonusValue *= (iNumBonuses + 2);
							iBonusValue /= kImprovement.isWater() ? 4 : 3;	// water resources are worth less

							iImprovementValue += iBonusValue;
						}
					}
				}
				
				// if water improvement, weight by coastal cities (weight the whole build)
				if (kImprovement.isWater())
				{
					iImprovementValue *= iCoastalCities;
					iImprovementValue /= std::max(1, iCityCount/2);
				}
				
				iBuildValue += iImprovementValue;
			}

			eRoute = ((RouteTypes)(GC.getBuildInfo((BuildTypes)iJ).getRoute()));

			if (eRoute != NO_ROUTE)
			{
				iBuildValue += ((getBestRoute() == NO_ROUTE) ? 700 : 200) * (getNumCities() + (bAdvancedStart ? 4 : 0));

				for (int iK = 0; iK < NUM_YIELD_TYPES; iK++)
				{
					iTempValue = 0;

					iTempValue += (GC.getRouteInfo(eRoute).getYieldChange(iK) * 100);

					for (int iL = 0; iL < GC.getNumImprovementInfos(); iL++)
					{
						iTempValue += (GC.getImprovementInfo((ImprovementTypes)iL).getRouteYieldChanges(eRoute, iK) * 50);
					}

					iTempValue *= AI_yieldWeight((YieldTypes)iK);
					iTempValue /= 100;

					iBuildValue += iTempValue;
				}
			}
		}
	}

    //the way feature-remove is done in XML is pretty weird
    //I believe this code needs to be outside the general BuildTypes loop
    //to ensure the feature-remove is only counted once rather than once per build
	//which could be a lot since nearly every build clears jungle...

	for (int iJ = 0; iJ < GC.getNumFeatureInfos(); iJ++)
    {
        bool bIsFeatureRemove = false;
        for (int iK = 0; iK < GC.getNumBuildInfos(); iK++)
        {
            if (GC.getBuildInfo((BuildTypes)iK).getFeatureTech(iJ) == eTech)
            {
                bIsFeatureRemove = true;
                break;
            }
        }

        if (bIsFeatureRemove)
        {
            iBuildValue += 100;

            if ((GC.getFeatureInfo(FeatureTypes(iJ)).getHealthPercent() < 0) ||
                ((GC.getFeatureInfo(FeatureTypes(iJ)).getYieldChange(YIELD_FOOD) + GC.getFeatureInfo(FeatureTypes(iJ)).getYieldChange(YIELD_PRODUCTION) + GC.getFeatureInfo(FeatureTypes(iJ)).getYieldChange(YIELD_COMMERCE)) < 0))
            {
                iBuildValue += 25 * countCityFeatures((FeatureTypes)iJ);
            }
        }
    }

	/*if (AI_totalUnitAIs(UNITAI_WORKER) == 0)
	{
		iBuildValue /= 2;
	}*/

	iValue += iBuildValue;

	// does tech reveal bonus resources
	for (int iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
	{
		if (GC.getBonusInfo((BonusTypes)iJ).getTechReveal() == eTech)
		{
			int iRevealValue = 150;
			iRevealValue += (AI_bonusVal((BonusTypes)iJ) * 50);
			
			BonusClassTypes eBonusClass = (BonusClassTypes)GC.getBonusInfo((BonusTypes)iJ).getBonusClassType();
			int iBonusClassTotal = (paiBonusClassRevealed[eBonusClass] + paiBonusClassUnrevealed[eBonusClass]);
			
			//iMultiplier is basically a desperation value
			//it gets larger as the AI runs out of options
			//Copper after failing to get horses is +66%
			//Iron after failing to get horse or copper is +200%
			//but with either copper or horse, Iron is only +25%
			int iMultiplier = 0;
			if (iBonusClassTotal > 0)
			{
                iMultiplier = (paiBonusClassRevealed[eBonusClass] - paiBonusClassHave[eBonusClass]);
                iMultiplier *= 100;
                iMultiplier /= iBonusClassTotal;
                
                iMultiplier *= (paiBonusClassRevealed[eBonusClass] + 1);
                iMultiplier /= ((paiBonusClassHave[eBonusClass] * iBonusClassTotal) + 1);
			}
			
			iMultiplier *= std::min(3, getNumCities());
			iMultiplier /= 3;
			
			iRevealValue *= 100 + iMultiplier;
			iRevealValue /= 100;
        

			iValue += iRevealValue;
			
		}
	}


	/* ------------------ Unit Value  ------------------ */
	bool bEnablesUnitWonder;
	iValue += AI_techUnitValue( eTech, iPathLength, bEnablesUnitWonder );
	
	if (bEnablesUnitWonder)
	{
		int iWonderRandom = ((bAsync) ? GC.getASyncRand().get(400, "AI Research Wonder Unit ASYNC") : GC.getGameINLINE().getSorenRandNum(400, "AI Research Wonder Unit"));
		iValue += iWonderRandom + (bCapitalAlone ? 200 : 0);

		iRandomMax += 400;
		iRandomFactor += iWonderRandom;
	}


	/* ------------------ Building Value  ------------------ */
	bool bEnablesWonder;
	iValue += AI_techBuildingValue( eTech, iPathLength, bEnablesWonder );

	// if it gives at least one wonder
	if (bEnablesWonder)
	{
		int iWonderRandom = ((bAsync) ? GC.getASyncRand().get(800, "AI Research Wonder Building ASYNC") : GC.getGameINLINE().getSorenRandNum(800, "AI Research Wonder Building"));
		iValue += (500 + iWonderRandom) / (bAdvancedStart ? 5 : 1);

		iRandomMax += 800;
		iRandomFactor += iWonderRandom;
	}

	/* ------------------ Project Value  ------------------ */
	bool bEnablesProjectWonder = false;
	for (int iJ = 0; iJ < GC.getNumProjectInfos(); iJ++)
	{
		if (GC.getProjectInfo((ProjectTypes)iJ).getTechPrereq() == eTech)
		{
			iValue += 1000;

			if( (VictoryTypes)GC.getProjectInfo((ProjectTypes)iJ).getVictoryPrereq() != NO_VICTORY )
			{
				if( !(GC.getProjectInfo((ProjectTypes)iJ).isSpaceship()) )
				{
					// Apollo
					iValue += (AI_isDoVictoryStrategy(AI_VICTORY_SPACE2) ? 2000 : 100);
				}
				else
				{
					// Space ship parts
					if( AI_isDoVictoryStrategy(AI_VICTORY_SPACE3) )
					{
						iValue += 1000;
					}
				}
			}

			if (iPathLength <= 1)
			{
				if (getTotalPopulation() > 5)
				{
					if (isWorldProject((ProjectTypes)iJ))
					{
						if (!(GC.getGameINLINE().isProjectMaxedOut((ProjectTypes)iJ)))
						{
							bEnablesProjectWonder = true;


							if (bCapitalAlone)
							{
								iValue += 100;
							}
						}
					}
				}
			}
		}
	}
	if (bEnablesProjectWonder)
	{
		int iWonderRandom = ((bAsync) ? GC.getASyncRand().get(200, "AI Research Wonder Project ASYNC") : GC.getGameINLINE().getSorenRandNum(200, "AI Research Wonder Project"));
		iValue += iWonderRandom;

		iRandomMax += 200;
		iRandomFactor += iWonderRandom;
	}


	/* ------------------ Process Value  ------------------ */
	bool bIsGoodProcess = false;
	for (int iJ = 0; iJ < GC.getNumProcessInfos(); iJ++)
	{
		if (GC.getProcessInfo((ProcessTypes)iJ).getTechPrereq() == eTech)
		{
			iValue += 100;

			for (int iK = 0; iK < NUM_COMMERCE_TYPES; iK++)
			{
				iTempValue = (GC.getProcessInfo((ProcessTypes)iJ).getProductionToCommerceModifier(iK) * 4);

				iTempValue *= AI_commerceWeight((CommerceTypes)iK);
				iTempValue /= 100;

				if (iK == COMMERCE_GOLD || iK == COMMERCE_RESEARCH)
				{
					bIsGoodProcess = true;
				}
				else if ((iK == COMMERCE_CULTURE) && AI_isDoVictoryStrategy(AI_VICTORY_CULTURE1))
				{
					iTempValue *= 3;
				}

				iValue += iTempValue;
			}
		}
	}

	if (bIsGoodProcess && bFinancialTrouble)
	{
		bool bHaveGoodProcess = false;
		for (int iJ = 0; iJ < GC.getNumProcessInfos(); iJ++)
		{
			if (kTeam.isHasTech((TechTypes)GC.getProcessInfo((ProcessTypes)iJ).getTechPrereq()))
			{
				bHaveGoodProcess = (GC.getProcessInfo((ProcessTypes)iJ).getProductionToCommerceModifier(COMMERCE_GOLD) + GC.getProcessInfo((ProcessTypes)iJ).getProductionToCommerceModifier(COMMERCE_RESEARCH)) > 0;
				if (bHaveGoodProcess)
				{
					break;
				}
			}
		}
		if (!bHaveGoodProcess)
		{
			iValue += 1500;										
		}
	}

	/* ------------------ Civic Value  ------------------ */
	for (int iJ = 0; iJ < GC.getNumCivicInfos(); iJ++)
	{
		if (GC.getCivicInfo((CivicTypes)iJ).getTechPrereq() == eTech)
		{
			iValue += 200;

			CivicTypes eCivic = getCivics((CivicOptionTypes)(GC.getCivicInfo((CivicTypes)iJ).getCivicOptionType()));
			if (NO_CIVIC != eCivic)
			{
				int iCurrentCivicValue = AI_civicValue(eCivic);
				int iNewCivicValue = AI_civicValue((CivicTypes)iJ);

				if (iNewCivicValue > iCurrentCivicValue)
				{
					iValue += std::min(2400, (2400 * (iNewCivicValue - iCurrentCivicValue)) / std::max(1, iCurrentCivicValue));
				}
				
				if (eCivic == GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic())
				{
					iValue += 600;
				}
			}
		}
	}

	if (iPathLength <= 2)
	{
		if (GC.getGameINLINE().countKnownTechNumTeams(eTech) == 0)
		{
			int iReligionValue = 0;
			int iPotentialReligions = 0;
			for (int iJ = 0; iJ < GC.getNumReligionInfos(); iJ++)
			{
				TechTypes eReligionTech = (TechTypes)GC.getReligionInfo((ReligionTypes)iJ).getTechPrereq();
				if (kTeam.isHasTech(eReligionTech))
				{
					if (!(GC.getGameINLINE().isReligionSlotTaken((ReligionTypes)iJ)))
					{
						iPotentialReligions++;
					}
				}
				if (eReligionTech == eTech)
				{
					if (!(GC.getGameINLINE().isReligionSlotTaken((ReligionTypes)iJ)))
					{
						int iRoll = 2400;
						if (!GC.getGame().isOption(GAMEOPTION_PICK_RELIGION))
						{
							ReligionTypes eFavorite = (ReligionTypes)GC.getLeaderHeadInfo(getLeaderType()).getFavoriteReligion();
							if (eFavorite != NO_RELIGION)
							{
								if (iJ == eFavorite)
								{
									iReligionValue += 1 + ((bAsync) ? GC.getASyncRand().get(1200, "AI Research Religion (Favorite) ASYNC") : GC.getGameINLINE().getSorenRandNum(1200, "AI Research Religion (Favorite)"));
									iRandomMax += 1200;
								}
								else
								{
									iRoll *= 2;
									iRoll /= 3;
								}
							}
						}
						iReligionValue += 1 + ((bAsync) ? GC.getASyncRand().get(iRoll, "AI Research Religion ASYNC") : GC.getGameINLINE().getSorenRandNum(iRoll, "AI Research Religion"));
						iRandomMax += iRoll;
						iRandomFactor += iReligionValue;

						if (iPathLength < 2)
						{
							iReligionValue *= 3;
							iReligionValue /= 2;
						}
					}
				}
			}
			
			if (iReligionValue > 0)
			{
				if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE1))
				{
					iReligionValue += 500;

					if (countHolyCities() < 1)
					{
						iReligionValue += 1000;
					}
				}
				else
				{
					iReligionValue /= (1 + countHolyCities() + ((iPotentialReligions > 0) ? 1 : 0));
				}

				if ((countTotalHasReligion() == 0) && (iPotentialReligions == 0))
				{
					iReligionValue *= 2;
					iReligionValue += 500;
				}

				if (AI_isDoStrategy(AI_STRATEGY_DAGGER))
				{
					iReligionValue /= 2;
				}
				iValue += iReligionValue;
			}

			for (int iJ = 0; iJ < GC.getNumCorporationInfos(); iJ++)
			{
				if (GC.getCorporationInfo((CorporationTypes)iJ).getTechPrereq() == eTech)
				{
					if (!(GC.getGameINLINE().isCorporationFounded((CorporationTypes)iJ)))
					{
						iValue += 100 + ((bAsync) ? GC.getASyncRand().get(2400, "AI Research Corporation ASYNC") : GC.getGameINLINE().getSorenRandNum(2400, "AI Research Corporation"));
					}
				}
			}

			if (getTechFreeUnit(eTech) != NO_UNIT)
			{
				int iGreatPeopleRandom = ((bAsync) ? GC.getASyncRand().get(3200, "AI Research Great People ASYNC") : GC.getGameINLINE().getSorenRandNum(3200, "AI Research Great People"));
				iValue += iGreatPeopleRandom;
				
				iRandomMax += 3200;
				iRandomFactor += iGreatPeopleRandom;

				if (bCapitalAlone)
				{
					iValue += 400;
				}

				iValue += 200;
			}

			iValue += (GC.getTechInfo(eTech).getFirstFreeTechs() * (200 + ((bCapitalAlone) ? 400 : 0) + ((bAsync) ? GC.getASyncRand().get(3200, "AI Research Free Tech ASYNC") : GC.getGameINLINE().getSorenRandNum(3200, "AI Research Free Tech"))));
		}
	}

	iValue += GC.getTechInfo(eTech).getAIWeight();

	if (!isHuman())
	{
		for (int iJ = 0; iJ < GC.getNumFlavorTypes(); iJ++)
		{
			iValue += (AI_getFlavorValue((FlavorTypes)iJ) * GC.getTechInfo(eTech).getFlavorValue(iJ) * 20);
		}
	}

	if (GC.getTechInfo(eTech).isRepeat())
	{
		iValue /= 10;
	}

	if (bIgnoreCost)
	{
		iValue *= (1 + (getResearchTurnsLeft((eTech), false)));
		iValue /= 10;
	}
	else
	{
		if (iValue > 0)
		{
			//this stops quick speed messing up.... might want to adjust by other things too...
			int iAdjustment = GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent();
	
			// Shouldn't run this during anarchy
			int iTurnsLeft = getResearchTurnsLeftTimes100((eTech), false);
			bool bCheapBooster = ((iTurnsLeft < (2 * iAdjustment)) && (0 == ((bAsync) ? GC.getASyncRand().get(5, "AI Choose Cheap Tech") : GC.getGameINLINE().getSorenRandNum(5, "AI Choose Cheap Tech"))));
			
			
			iValue *= 100000;
			
            iValue /= (iTurnsLeft + (bCheapBooster ? 1 : (5 * iAdjustment)));
		}
	}
	
	//Tech Whore								
	if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_TECH_TRADING))
	{
		if (GC.getTechInfo(eTech).isTechTrading() || kTeam.isTechTrading())
		{
			if (((bAsync) ? GC.getASyncRand().get(100, "AI Tech Whore ASYNC") : GC.getGameINLINE().getSorenRandNum(100, "AI Tech Whore")) < (GC.getGameINLINE().isOption(GAMEOPTION_NO_TECH_BROKERING) ? 20 : 10))
			{
				int iKnownCount = 0;
				int iPossibleKnownCount = 0;

				for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++)
				{
					if (GET_TEAM((TeamTypes)iTeam).isAlive())
					{
						if (GET_TEAM(getTeam()).isHasMet((TeamTypes)iTeam))
						{
							if (GET_TEAM((TeamTypes)iTeam).isHasTech(eTech))
							{
								iKnownCount++;
							}
						}

						iPossibleKnownCount++;
					}
				}
				
				if (iKnownCount == 0)
				{
					if (iPossibleKnownCount > 2)
					{
						int iTradeModifier = std::min(150, 25 * (iPossibleKnownCount - 2));
						iValue *= 100 + iTradeModifier;
						iValue /= 100;
					}
				}
			}
		}
	}

	if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3))
	{
		int iCVValue = AI_cultureVictoryTechValue(eTech);
		iValue *= (iCVValue + 10);
		iValue /= ((iCVValue < 100) ? 400 : 100);
	}

	iValue = std::max(1, iValue);

	return iValue;
}


int CvPlayerAI::AI_techBuildingValue( TechTypes eTech, int iPathLength, bool &bEnablesWonder ) const
{
	bool bCapitalAlone = (GC.getGameINLINE().getElapsedGameTurns() > 0) ? AI_isCapitalAreaAlone() : false;
	bool bFinancialTrouble = AI_isFinancialTrouble();
	int iTeamCityCount = GET_TEAM(getTeam()).getNumCities();
	int iCoastalCities = countNumCoastalCities();
	int iCityCount = getNumCities();

	BuildingTypes eLoopBuilding;
	int iTempValue = 0;
	int iValue = 0;

	int iBestLandBuildingValue = 0;
	bEnablesWonder = false;
	int iExistingCultureBuildingCount = 0;
	bool bIsCultureBuilding = false;
	for (int iJ = 0; iJ < GC.getNumBuildingClassInfos(); iJ++)
	{
		eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iJ)));
/************************************************************************************************/
/* Afforess	                  Start		 07/27/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
		if (eLoopBuilding != NO_BUILDING)
*/
		if (GC.getGameINLINE().canEverConstruct(eLoopBuilding))
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		{
			CvBuildingInfo& kLoopBuilding = GC.getBuildingInfo(eLoopBuilding);
			if (isTechRequiredForBuilding((eTech), eLoopBuilding))
			{
				int iBuildingValue = 0;
				
				if (kLoopBuilding.getSpecialBuildingType() != NO_BUILDING)
				{
					iBuildingValue += ((bCapitalAlone) ? 100 : 25);
				}
				else
				{
					iBuildingValue += ((bCapitalAlone) ? 200 : 50);
				}

				//the granary effect is SO powerful it deserves special code
				if (kLoopBuilding.getFoodKept() > 0)
				{
					iBuildingValue += (15 * kLoopBuilding.getFoodKept());
				}

				if (kLoopBuilding.getMaintenanceModifier() < 0)
				{
					int iLoop;
					int iCount = 0;
					CvCity* pLoopCity;
					iTempValue = 0;
					for (pLoopCity = firstCity(&iLoop, true); pLoopCity != NULL; pLoopCity = nextCity(&iLoop, true))
					{
						iTempValue += pLoopCity->getMaintenanceTimes100();
						iCount++;
						if (iCount > 4)
						{
							break;
						}
					}
					iTempValue /= std::max(1, iCount);
					iTempValue *= -kLoopBuilding.getMaintenanceModifier();
					iTempValue /= 10 * 100;

					iValue += iTempValue;
				}

				iBuildingValue += 100;

                if ((GC.getBuildingClassInfo((BuildingClassTypes)iJ).getDefaultBuildingIndex()) != (GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iJ)))
                {
                    //UB
                    iBuildingValue += 600;
                }
                
				if( !isLimitedWonderClass((BuildingClassTypes)iJ) )
				{
					if (kLoopBuilding.getCommerceChange(COMMERCE_CULTURE) > 0 || kLoopBuilding.getObsoleteSafeCommerceChange(COMMERCE_CULTURE) > 0)
					{
						bIsCultureBuilding = true;
					}
				}
                
                if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2))
                {
                    int iMultiplier = (isLimitedWonderClass((BuildingClassTypes)iJ) ? 1 : 3);
                    iBuildingValue += (150 * (kLoopBuilding.getCommerceChange(COMMERCE_CULTURE) + kLoopBuilding.getObsoleteSafeCommerceChange(COMMERCE_CULTURE))) * iMultiplier;
                    iBuildingValue += kLoopBuilding.getCommerceModifier(COMMERCE_CULTURE) * 4 * iMultiplier ;
                }
				
				if (bFinancialTrouble)
				{
					iBuildingValue += (-kLoopBuilding.getMaintenanceModifier()) * 15;
					iBuildingValue += kLoopBuilding.getYieldModifier(YIELD_COMMERCE) * 8;
					iBuildingValue += kLoopBuilding.getCommerceModifier(COMMERCE_GOLD) * 15;
				}

				// if this is a religious building, its not as useful
				ReligionTypes eReligion = (ReligionTypes) kLoopBuilding.getReligionType();
				if (eReligion != NO_RELIGION)
				{
					
					// reduce by a factor based on how many cities we have with that relgion
					if (iTeamCityCount > 0)
					{
						int iCitiesWithReligion = GET_TEAM(getTeam()).getHasReligionCount(eReligion);
						
						iBuildingValue *= (4 + iCitiesWithReligion);
						iBuildingValue /= (4 + iTeamCityCount);
					}

					// if this building requires a religion, then only count it as 1/7th as much
					// or in other words, only count things like temples once, not 7 times
					// doing it this way in case some mods give buildings to only one religion
					iBuildingValue /= std::max(1, GC.getNumReligionInfos());
				}

				// if we're close to pop domination, we love medicine!
				// don't adjust for negative modifiers to prevent ignoring assembly line, etc.
				if ( AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION3) && kLoopBuilding.getHealth() > 0)
				{
					iBuildingValue += kLoopBuilding.getHealth() * 150;
				}

				if (kLoopBuilding.getPrereqAndTech() == eTech)
				{
					if (iPathLength <= 1)
					{
						if (getTotalPopulation() > 5)
						{
							if (isWorldWonderClass((BuildingClassTypes)iJ))
							{
								if (!(GC.getGameINLINE().isBuildingClassMaxedOut((BuildingClassTypes)iJ)))
								{
									bEnablesWonder = true;

									if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE1))
									{
										if (kLoopBuilding.getCommerceChange(COMMERCE_CULTURE) >= 3 || 
											kLoopBuilding.getObsoleteSafeCommerceChange(COMMERCE_CULTURE) >= 3 ||
											kLoopBuilding.getCommerceModifier(COMMERCE_CULTURE) >= 10)
										{
											iValue += 400;
										}
									}

									if (bCapitalAlone)
									{
										iBuildingValue += 400;
									}
								}
							}
						}
					}
				}

				if( AI_isDoVictoryStrategy(AI_VICTORY_DIPLOMACY1) )
				{
					if( kLoopBuilding.getVoteSourceType() >= 0 )
					{
						iValue += 400;
					}
				}

				if (iBuildingValue > iBestLandBuildingValue)
				{
					iBestLandBuildingValue = iBuildingValue;
				}

				// if water building, weight by coastal cities
				if (kLoopBuilding.isWater())
				{
					iBuildingValue *= iCoastalCities;
					iBuildingValue /= std::max(1, iCityCount/2);
				}
				// if land building, is it the best?
				else if (iBuildingValue > iBestLandBuildingValue)
				{
					iBestLandBuildingValue = iBuildingValue;
				}

				iValue += iBuildingValue;
			}
			else
			{
				if (canConstruct(eLoopBuilding))
				{
					if (!isLimitedWonderClass((BuildingClassTypes)iJ))
					{
						if (kLoopBuilding.getCommerceChange(COMMERCE_CULTURE) > 0 || kLoopBuilding.getObsoleteSafeCommerceChange(COMMERCE_CULTURE) > 0)
						{
							iExistingCultureBuildingCount++;
						}
					}												
				}
			}
		}
	}
	
	if (bIsCultureBuilding && iExistingCultureBuildingCount < 2)
	{
		if (getFreeCityCommerce(COMMERCE_CULTURE) == 0)
		{
			if (getNumCities() > 1)
			{
				iValue += 150 * std::max(1, (3 - 2 * iExistingCultureBuildingCount)) * (getNumCities() - 1);
			}
		}
	}
	// if tech gives at least one building (so we can count each individual building less)
	if (iBestLandBuildingValue > 0)
	{
		iValue += iBestLandBuildingValue;
	}

	return iValue;
}


int CvPlayerAI::AI_techUnitValue( TechTypes eTech, int iPathLength, bool &bEnablesUnitWonder ) const
{
	bool bWarPlan = (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0);
	if( !bWarPlan )
	{
		// Aggressive players will stick with war civics
		if( GET_TEAM(getTeam()).AI_getTotalWarOddsTimes100() > 400 )
		{
			bWarPlan = true;
		}
	}

	bool bCapitalAlone = (GC.getGameINLINE().getElapsedGameTurns() > 0) ? AI_isCapitalAreaAlone() : false;
	int iHasMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);
	int iCoastalCities = countNumCoastalCities();
	CvCity* pCapitalCity = getCapitalCity();

	UnitTypes eLoopUnit;
	int iMilitaryValue = 0;
	int iValue = 0;

	bEnablesUnitWonder = false;
	for (int iJ = 0; iJ < GC.getNumUnitClassInfos(); iJ++)
	{
		eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iJ)));

/************************************************************************************************/
/* Afforess	                  Start		 07/27/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
		if (eLoopUnit != NO_UNIT)
*/
		if (GC.getGameINLINE().canEverTrain(eLoopUnit))
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		{
			if (isTechRequiredForUnit((eTech), eLoopUnit))
			{
				CvUnitInfo& kLoopUnit = GC.getUnitInfo(eLoopUnit);
				iValue += 200;
				int iUnitValue = 0;
				int iNavalValue = 0;
				
				if ((GC.getUnitClassInfo((UnitClassTypes)iJ).getDefaultUnitIndex()) != (GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iJ)))
                {
                    //UU
                    iUnitValue += 600;
                }

				if (kLoopUnit.getPrereqAndTech() == eTech)
				{
					iMilitaryValue = 0;

					// BBAI TODO: Change this to evaluating all unitai types defined in XML for unit?
					// Without this change many unit types are hard to evaluate, like offensive value of rifles
					// or defensive value of collateral seige
					switch (kLoopUnit.getDefaultUnitAIType())
					{
					case UNITAI_UNKNOWN:
					case UNITAI_ANIMAL:
						break;

					case UNITAI_SETTLE:
						iUnitValue += 1200;
						break;

					case UNITAI_WORKER:
						iUnitValue += 800;
						break;

					case UNITAI_ATTACK:
						iMilitaryValue += ((bWarPlan) ? 600 : 300);
						iMilitaryValue += (AI_isDoStrategy(AI_STRATEGY_DAGGER ) ? 800 : 0);
						iUnitValue += 100;
						break;

					case UNITAI_ATTACK_CITY:
						iMilitaryValue += ((bWarPlan) ? 800 : 400);
						iMilitaryValue += (AI_isDoStrategy(AI_STRATEGY_DAGGER ) ? 800 : 0);
						if (kLoopUnit.getBombardRate() > 0)
						{
							iMilitaryValue += 200;
							
							if (AI_calculateTotalBombard(DOMAIN_LAND) == 0)
							{
								iMilitaryValue += 800;
								if (AI_isDoStrategy(AI_STRATEGY_DAGGER))
								{
									iMilitaryValue += 1000;
								}															
							}
						}					
						iUnitValue += 100;
						break;

					case UNITAI_COLLATERAL:
						iMilitaryValue += ((bWarPlan) ? 600 : 300);
						break;

					case UNITAI_PILLAGE:
						iMilitaryValue += ((bWarPlan) ? 200 : 100);
						break;

					case UNITAI_RESERVE:
						iMilitaryValue += ((bWarPlan) ? 200 : 100);
						break;

					case UNITAI_COUNTER:
						iMilitaryValue += ((bWarPlan) ? 600 : 300);
						iMilitaryValue += (AI_isDoStrategy(AI_STRATEGY_DAGGER ) ? 600 : 0);
						break;

					case UNITAI_PARADROP:
						iMilitaryValue += ((bWarPlan) ? 600 : 300);
						break;

					case UNITAI_CITY_DEFENSE:
						iMilitaryValue += ((bWarPlan) ? 800 : 400);
						iMilitaryValue += ((!bCapitalAlone) ? 400 : 200);
						iUnitValue += ((iHasMetCount > 0) ? 800 : 200);
						break;

					case UNITAI_CITY_COUNTER:
						iMilitaryValue += ((bWarPlan) ? 800 : 400);
						break;

					case UNITAI_CITY_SPECIAL:
						iMilitaryValue += ((bWarPlan) ? 800 : 400);
						break;

					case UNITAI_EXPLORE:
						iUnitValue += ((bCapitalAlone) ? 100 : 200);
						break;

					case UNITAI_MISSIONARY:
						iUnitValue += ((getStateReligion() != NO_RELIGION) ? 600 : 300);
						break;

					case UNITAI_PROPHET:
					case UNITAI_ARTIST:
					case UNITAI_SCIENTIST:
					case UNITAI_GENERAL:
					case UNITAI_MERCHANT:
					case UNITAI_ENGINEER:
						break;

					case UNITAI_SPY:
						iMilitaryValue += ((bWarPlan) ? 100 : 50);
						break;

					case UNITAI_ICBM:
						iMilitaryValue += ((bWarPlan) ? 200 : 100);
						break;

					case UNITAI_WORKER_SEA:
						if (iCoastalCities > 0)
						{
							// note, workboat improvements are already counted in the improvement section
						}
						break;

					case UNITAI_ATTACK_SEA:
						// BBAI TODO: Boost value for maps where Barb ships are pestering us
						if (iCoastalCities > 0)
						{
							iMilitaryValue += ((bWarPlan) ? 200 : 100);
						}
						iNavalValue += 100;
						break;

					case UNITAI_RESERVE_SEA:
						if (iCoastalCities > 0)
						{
							iMilitaryValue += ((bWarPlan) ? 100 : 50);
						}
						iNavalValue += 100;
						break;

					case UNITAI_ESCORT_SEA:
						if (iCoastalCities > 0)
						{
							iMilitaryValue += ((bWarPlan) ? 100 : 50);
						}
						iNavalValue += 100;
						break;

					case UNITAI_EXPLORE_SEA:
						if (iCoastalCities > 0)
						{
							iUnitValue += ((bCapitalAlone) ? 1800 : 600);
						}
						break;

					case UNITAI_ASSAULT_SEA:
						if (iCoastalCities > 0)
						{
							iMilitaryValue += ((bWarPlan || bCapitalAlone) ? 400 : 200);
						}
						iNavalValue += 200;
						break;

					case UNITAI_SETTLER_SEA:
						if (iCoastalCities > 0)
						{
							iUnitValue += ((bWarPlan || bCapitalAlone) ? 100 : 200);
						}
						iNavalValue += 200;
						break;

					case UNITAI_MISSIONARY_SEA:
						if (iCoastalCities > 0)
						{
							iUnitValue += 100;
						}
						break;

					case UNITAI_SPY_SEA:
						if (iCoastalCities > 0)
						{
							iMilitaryValue += 100;
						}
						break;

					case UNITAI_CARRIER_SEA:
						if (iCoastalCities > 0)
						{
							iMilitaryValue += ((bWarPlan) ? 100 : 50);
						}
						break;

					case UNITAI_MISSILE_CARRIER_SEA:
						if (iCoastalCities > 0)
						{
							iMilitaryValue += ((bWarPlan) ? 100 : 50);
						}
						break;

					case UNITAI_PIRATE_SEA:
						if (iCoastalCities > 0)
						{
							iMilitaryValue += 100;
						}
						iNavalValue += 100;
						break;

					case UNITAI_ATTACK_AIR:
						iMilitaryValue += ((bWarPlan) ? 1200 : 800);
						break;

					case UNITAI_DEFENSE_AIR:
						iMilitaryValue += ((bWarPlan) ? 1200 : 800);
						break;

					case UNITAI_CARRIER_AIR:
						if (iCoastalCities > 0)
						{
							iMilitaryValue += ((bWarPlan) ? 200 : 100);
						}
						iNavalValue += 400;
						break;

					case UNITAI_MISSILE_AIR:
						iMilitaryValue += ((bWarPlan) ? 200 : 100);
						break;

					default:
						FAssert(false);
						break;
					}
					
					if( AI_isDoStrategy(AI_STRATEGY_ALERT1) )
					{
						if( kLoopUnit.getUnitAIType(UNITAI_COLLATERAL) )
						{
							iUnitValue += 500;
						}

						if( kLoopUnit.getUnitAIType(UNITAI_CITY_DEFENSE) )
						{
							iUnitValue += (1000 * GC.getGameINLINE().AI_combatValue(eLoopUnit))/100;
						}
					}
					
					if( AI_isDoStrategy(AI_STRATEGY_TURTLE) && iPathLength <= 1)
					{
						if( kLoopUnit.getUnitAIType(UNITAI_COLLATERAL) )
						{
							iUnitValue += 1000;
						}

						if( kLoopUnit.getUnitAIType(UNITAI_CITY_DEFENSE) )
						{
							iUnitValue += (2000 * GC.getGameINLINE().AI_combatValue(eLoopUnit))/100;
						}
					}

					if( AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST3) )
					{
						if( kLoopUnit.getUnitAIType(UNITAI_ATTACK_CITY) )
						{
							iUnitValue += (1500 * GC.getGameINLINE().AI_combatValue(eLoopUnit))/100;
						}
					}
					else if( AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST1) )
					{
						if( kLoopUnit.getUnitAIType(UNITAI_ATTACK_CITY) )
						{
							iUnitValue += (500 * GC.getGameINLINE().AI_combatValue(eLoopUnit))/100;
						}
					}
					
					if (kLoopUnit.getUnitAIType(UNITAI_ASSAULT_SEA) && iCoastalCities > 0)
					{
						int iAssaultValue = 0;
						UnitTypes eExistingUnit = NO_UNIT;
						if (AI_bestAreaUnitAIValue(UNITAI_ASSAULT_SEA, NULL, &eExistingUnit) == 0)
						{
							iAssaultValue += 250;
						}
						else if( eExistingUnit != NO_UNIT )
						{
							iAssaultValue += 1000 * std::max(0, AI_unitImpassableCount(eLoopUnit) - AI_unitImpassableCount(eExistingUnit));

							int iNewCapacity = kLoopUnit.getMoves() * kLoopUnit.getCargoSpace();
							int iOldCapacity = GC.getUnitInfo(eExistingUnit).getMoves() * GC.getUnitInfo(eExistingUnit).getCargoSpace();

							iAssaultValue += (800 * (iNewCapacity - iOldCapacity)) / std::max(1, iOldCapacity);
						}
						
						if (iAssaultValue > 0)
						{
							int iLoop;
							CvArea* pLoopArea;
							bool bIsAnyAssault = false;
							for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
							{
								if (AI_isPrimaryArea(pLoopArea))
								{
									if (pLoopArea->getAreaAIType(getTeam()) == AREAAI_ASSAULT)
									{
										bIsAnyAssault = true;
										break;
									}
								}
							}
							if (bIsAnyAssault)
							{
								iUnitValue += iAssaultValue * 4;
							}
							else
							{
								iUnitValue += iAssaultValue;
							}
						}
					}
					
					if (iNavalValue > 0)
					{
						if (getCapitalCity() != NULL)
						{
							// BBAI TODO: A little odd ... naval value is 0 if have no colonies.
							iNavalValue *= 2 * (getNumCities() - getCapitalCity()->area()->getCitiesPerPlayer(getID()));
							iNavalValue /= getNumCities();
							
							iUnitValue += iNavalValue;
						}
					}

					if (AI_totalUnitAIs((UnitAITypes)(kLoopUnit.getDefaultUnitAIType())) == 0)
					{
						// do not give bonus to seagoing units if they are worthless
						if (iUnitValue > 0)
						{
							iUnitValue *= 2;
						}

						if (kLoopUnit.getDefaultUnitAIType() == UNITAI_EXPLORE)
						{
							if (pCapitalCity != NULL)
							{
								iUnitValue += (AI_neededExplorers(pCapitalCity->area()) * 400);
							}
						}

						if (kLoopUnit.getDefaultUnitAIType() == UNITAI_EXPLORE_SEA)
						{
							iUnitValue += 400;
							iUnitValue += ((GC.getGameINLINE().countCivTeamsAlive() - iHasMetCount) * 200);
						}
					}

					if (kLoopUnit.getUnitAIType(UNITAI_SETTLER_SEA))
					{
						if (getCapitalCity() != NULL)
						{
							UnitTypes eExistingUnit = NO_UNIT;
							int iBestAreaValue = 0;
							AI_getNumAreaCitySites(getCapitalCity()->getArea(), iBestAreaValue);

							//Early Expansion by sea
							if (AI_bestAreaUnitAIValue(UNITAI_SETTLER_SEA, NULL, &eExistingUnit) == 0)
							{
								CvArea* pWaterArea = getCapitalCity()->waterArea();
								if (pWaterArea != NULL)
								{
									int iBestOtherValue = 0;
									AI_getNumAdjacentAreaCitySites(pWaterArea->getID(), getCapitalCity()->getArea(), iBestOtherValue);
									
									if (iBestAreaValue == 0)
									{
										iUnitValue += 2000;
									}
									else if (iBestAreaValue < iBestOtherValue)
									{
										iUnitValue += 1000;
									}
									else if (iBestOtherValue > 0)
									{
										iUnitValue += 500;
									}
								}
							}
							// Landlocked expansion over ocean
							else if( eExistingUnit != NO_UNIT )
							{
								if( AI_unitImpassableCount(eLoopUnit) < AI_unitImpassableCount(eExistingUnit) )
								{
									if( iBestAreaValue < AI_getMinFoundValue() )
									{
										iUnitValue += (AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION2) ? 2000 : 500);
									}
								}
							}
						}
					}
					
					if( iMilitaryValue > 0 )
					{
						if (iHasMetCount == 0)
						{
							iMilitaryValue /= 2;
						}

						if (bCapitalAlone)
						{
							iMilitaryValue *= 2;
							iMilitaryValue /= 3;
						}

						iUnitValue += iMilitaryValue;
					}
					
					if (iPathLength <= 1)
					{
						if (getTotalPopulation() > 5)
						{
							if (isWorldUnitClass((UnitClassTypes)iJ))
							{
								if (!(GC.getGameINLINE().isUnitClassMaxedOut((UnitClassTypes)iJ)))
								{
									bEnablesUnitWonder = true;
								}
							}
						}
					}

					iValue += iUnitValue;
				}
			}
		}
	}

	return iValue;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

void CvPlayerAI::AI_chooseFreeTech()
{
	TechTypes eBestTech;

	clearResearchQueue();

/************************************************************************************************/
/* Afforess	                  Start		 04/29/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (GC.getUSE_AI_BESTTECH_CALLBACK())
	{
		CyArgsList argsList;
		long lResult;
		argsList.add(getID());
		argsList.add(true);
		lResult = -1;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_chooseTech", argsList.makeFunctionArgs(), &lResult);
		eBestTech = ((TechTypes)lResult);
	}
	else
	{
		eBestTech = NO_TECH;
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	if (eBestTech == NO_TECH)
	{
		eBestTech = AI_bestTech(1, true);
	}

	if (eBestTech != NO_TECH)
	{
		GET_TEAM(getTeam()).setHasTech(eBestTech, true, getID(), true, true);
	}
}


void CvPlayerAI::AI_chooseResearch()
{
	TechTypes eBestTech;
	int iI;

	clearResearchQueue();

	if (getCurrentResearch() == NO_TECH)
	{
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if ((iI != getID()) && (GET_PLAYER((PlayerTypes)iI).getTeam() == getTeam()))
				{
					if (GET_PLAYER((PlayerTypes)iI).getCurrentResearch() != NO_TECH)
					{
						if (canResearch(GET_PLAYER((PlayerTypes)iI).getCurrentResearch()))
						{
							pushResearch(GET_PLAYER((PlayerTypes)iI).getCurrentResearch());
						}
					}
				}
			}
		}
	}

	if (getCurrentResearch() == NO_TECH)
	{
/************************************************************************************************/
/* Afforess	                  Start		 04/29/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		if (GC.getUSE_AI_BESTTECH_CALLBACK())
		{
			CyArgsList argsList;
			long lResult;
			argsList.add(getID());
			argsList.add(false);
			lResult = -1;
			gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_chooseTech", argsList.makeFunctionArgs(), &lResult);
			eBestTech = ((TechTypes)lResult);
		}
		else
		{
			eBestTech = NO_TECH;
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

		if (eBestTech == NO_TECH)
		{
			int iAIResearchDepth;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
			iAIResearchDepth = AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3) ? 3 : 6;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

			
			eBestTech = AI_bestTech((isHuman()) ? 1 : iAIResearchDepth);
		}

		if (eBestTech != NO_TECH)
		{
			pushResearch(eBestTech);
		}
	}
}


DiploCommentTypes CvPlayerAI::AI_getGreeting(PlayerTypes ePlayer) const
{
	TeamTypes eWorstEnemy;

	if (GET_PLAYER(ePlayer).getTeam() != getTeam())
	{
		eWorstEnemy = GET_TEAM(getTeam()).AI_getWorstEnemy();

		if ((eWorstEnemy != NO_TEAM) && (eWorstEnemy != GET_PLAYER(ePlayer).getTeam()) && GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasMet(eWorstEnemy) && (GC.getASyncRand().get(4) == 0))
		{
			if (GET_PLAYER(ePlayer).AI_hasTradedWithTeam(eWorstEnemy) && !atWar(GET_PLAYER(ePlayer).getTeam(), eWorstEnemy))
			{
				return (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_WORST_ENEMY_TRADING");
			}
			else
			{
				return (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_WORST_ENEMY");
			}
		}
		else if ((getNumNukeUnits() > 0) && (GC.getASyncRand().get(4) == 0))
		{
			return (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_NUKES");
		}
		else if ((GET_PLAYER(ePlayer).getPower() < getPower()) && AI_getAttitude(ePlayer) < ATTITUDE_PLEASED && (GC.getASyncRand().get(4) == 0))
		{
			return (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_UNIT_BRAG");
		}
	}

	return (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_GREETINGS");
}


bool CvPlayerAI::AI_isWillingToTalk(PlayerTypes ePlayer) const
{
	FAssertMsg(getPersonalityType() != NO_LEADER, "getPersonalityType() is not expected to be equal with NO_LEADER");
	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	if (GET_PLAYER(ePlayer).getTeam() == getTeam()
		|| GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam())
		|| GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()))
	{
		return true;
	}

	if (GET_TEAM(getTeam()).isHuman())
	{
		return false;
	}
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	bool bRuthlessAI = GC.getGameINLINE().isOption(GAMEOPTION_RUTHLESS_AI);
	if (bRuthlessAI)
	{
		if (AI_getMemoryCount(ePlayer, MEMORY_BACKSTAB_FRIEND) > 0 || AI_getMemoryCount(ePlayer, MEMORY_BACKSTAB) > 0)
		{
			if (AI_getAttitudeVal(ePlayer) < 3)
			{
				return false;
			}
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		int iRefuseDuration = (GC.getLeaderHeadInfo(getPersonalityType()).getRefuseToTalkWarThreshold() * ((GET_TEAM(getTeam()).AI_isChosenWar(GET_PLAYER(ePlayer).getTeam())) ? 2 : 1));
		
		int iOurSuccess = 1 + GET_TEAM(getTeam()).AI_getWarSuccess(GET_PLAYER(ePlayer).getTeam());
		int iTheirSuccess = 1 + GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_getWarSuccess(getTeam());
		if (iTheirSuccess > iOurSuccess * 2)
		{
			iRefuseDuration *= 20 + ((80 * iOurSuccess * 2) / iTheirSuccess);
			iRefuseDuration /= 100;
		}

		if (GET_TEAM(getTeam()).AI_getAtWarCounter(GET_PLAYER(ePlayer).getTeam()) < iRefuseDuration)
		{
			return false;
		}

		if (GET_TEAM(getTeam()).isAVassal())
		{
			return false;
		}
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		if (GET_PLAYER(ePlayer).getNumCities() == 0)
		{
			return false;
		}
		
		if (bRuthlessAI)
		{
			if (!AI_isFinancialTrouble())
			{
				if (iOurSuccess * 2 > iTheirSuccess * 3)
				{
					return false;
				}
			}
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	}
	else
	{
		if (AI_getMemoryCount(ePlayer, MEMORY_STOPPED_TRADING_RECENT) > 0)
		{
			return false;
		}
	}

	return true;
}


// XXX what if already at war???
// Returns true if the AI wants to sneak attack...
bool CvPlayerAI::AI_demandRebukedSneak(PlayerTypes ePlayer) const
{
	FAssertMsg(!isHuman(), "isHuman did not return false as expected");
	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	FAssert(!(GET_TEAM(getTeam()).isAVassal()));
	FAssert(!(GET_TEAM(getTeam()).isHuman()));

	if (GC.getGameINLINE().getSorenRandNum(100, "AI Demand Rebuked") < GC.getLeaderHeadInfo(getPersonalityType()).getDemandRebukedSneakProb())
	{
		if (GET_TEAM(getTeam()).getPower(true) > GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getDefensivePower())
		{
			return true;
		}
	}

	return false;
}


// XXX what if already at war???
// Returns true if the AI wants to declare war...
bool CvPlayerAI::AI_demandRebukedWar(PlayerTypes ePlayer) const
{
	FAssertMsg(!isHuman(), "isHuman did not return false as expected");
	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	FAssert(!(GET_TEAM(getTeam()).isAVassal()));
	FAssert(!(GET_TEAM(getTeam()).isHuman()));

	// needs to be async because it only happens on the computer of the player who is in diplomacy...
	if (GC.getASyncRand().get(100, "AI Demand Rebuked ASYNC") < GC.getLeaderHeadInfo(getPersonalityType()).getDemandRebukedWarProb())
	{
		if (GET_TEAM(getTeam()).getPower(true) > GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getDefensivePower())
		{
			if (GET_TEAM(getTeam()).AI_isAllyLandTarget(GET_PLAYER(ePlayer).getTeam()))
			{
				return true;
			}
		}
	}

	return false;
}


// XXX maybe make this a little looser (by time...)
bool CvPlayerAI::AI_hasTradedWithTeam(TeamTypes eTeam) const
{
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam)
			{
				if ((AI_getPeacetimeGrantValue((PlayerTypes)iI) + AI_getPeacetimeTradeValue((PlayerTypes)iI)) > 0)
				{
					return true;
				}
			}
		}
	}

	return false;
}

// static
AttitudeTypes CvPlayerAI::AI_getAttitudeFromValue(int iAttitudeVal)
{
	if (iAttitudeVal >= 10)
	{
		return ATTITUDE_FRIENDLY;
	}
	else if (iAttitudeVal >= 3)
	{
		return ATTITUDE_PLEASED;
	}
	else if (iAttitudeVal <= -10)
	{
		return ATTITUDE_FURIOUS;
	}
	else if (iAttitudeVal <= -3)
	{
		return ATTITUDE_ANNOYED;
	}
	else
	{
		return ATTITUDE_CAUTIOUS;
	}
}

AttitudeTypes CvPlayerAI::AI_getAttitude(PlayerTypes ePlayer, bool bForced) const
{
	PROFILE_FUNC();

//	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");
//AI Autoplay calls this
	return (AI_getAttitudeFromValue(AI_getAttitudeVal(ePlayer, bForced)));
}


int CvPlayerAI::AI_getAttitudeVal(PlayerTypes ePlayer, bool bForced) const
{
	PROFILE_FUNC();

	int iRankDifference;
	int iAttitude;
	int iI;

//	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");
//AI Autoplay calls this
	if (bForced)
	{
		if (getTeam() == GET_PLAYER(ePlayer).getTeam() || (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()) && !GET_TEAM(getTeam()).isCapitulated()))
		{
			return 100;
		}

		if (isBarbarian() || GET_PLAYER(ePlayer).isBarbarian())
		{
			return -100;
		}
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/03/09                       poyuzhe & jdog5000     */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
	// From Sanguo Mod Performance, ie the CAR Mod
	// Attitude cache
	if (m_aiAttitudeCache[ePlayer] != MAX_INT)
	{
		return m_aiAttitudeCache[ePlayer];
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	iAttitude = GC.getLeaderHeadInfo(getPersonalityType()).getBaseAttitude();

	iAttitude += GC.getHandicapInfo(GET_PLAYER(ePlayer).getHandicapType()).getAttitudeChange();

//	if (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
//	{
//		if (GET_PLAYER(ePlayer).isHuman())
//		{
//			iAttitude -= 2;
//		}
//	}

	if (!(GET_PLAYER(ePlayer).isHuman()))
	{
		iAttitude += (4 - abs(AI_getPeaceWeight() - GET_PLAYER(ePlayer).AI_getPeaceWeight()));
		iAttitude += std::min(GC.getLeaderHeadInfo(getPersonalityType()).getWarmongerRespect(), GC.getLeaderHeadInfo(GET_PLAYER(ePlayer).getPersonalityType()).getWarmongerRespect());
	}

	iAttitude -= std::max(0, (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getNumMembers() - GET_TEAM(getTeam()).getNumMembers()));

	iRankDifference = (GC.getGameINLINE().getPlayerRank(getID()) - GC.getGameINLINE().getPlayerRank(ePlayer));

	if (iRankDifference > 0)
	{
		iAttitude += ((GC.getLeaderHeadInfo(getPersonalityType()).getWorseRankDifferenceAttitudeChange() * iRankDifference) / (GC.getGameINLINE().countCivPlayersEverAlive() + 1));
	}
	else
	{
		iAttitude += ((GC.getLeaderHeadInfo(getPersonalityType()).getBetterRankDifferenceAttitudeChange() * -(iRankDifference)) / (GC.getGameINLINE().countCivPlayersEverAlive() + 1));
	}

	if ((GC.getGameINLINE().getPlayerRank(getID()) >= (GC.getGameINLINE().countCivPlayersEverAlive() / 2)) &&
		  (GC.getGameINLINE().getPlayerRank(ePlayer) >= (GC.getGameINLINE().countCivPlayersEverAlive() / 2)))
	{
		iAttitude++;
	}

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_getWarSuccess(getTeam()) > GET_TEAM(getTeam()).AI_getWarSuccess(GET_PLAYER(ePlayer).getTeam()))
	{
		iAttitude += GC.getLeaderHeadInfo(getPersonalityType()).getLostWarAttitudeChange();
	}

	iAttitude += AI_getCloseBordersAttitude(ePlayer);
	iAttitude += AI_getWarAttitude(ePlayer);
	iAttitude += AI_getPeaceAttitude(ePlayer);
	iAttitude += AI_getSameReligionAttitude(ePlayer);
	iAttitude += AI_getDifferentReligionAttitude(ePlayer);
	iAttitude += AI_getBonusTradeAttitude(ePlayer);
	iAttitude += AI_getOpenBordersAttitude(ePlayer);
	iAttitude += AI_getDefensivePactAttitude(ePlayer);
	iAttitude += AI_getRivalDefensivePactAttitude(ePlayer);
	iAttitude += AI_getRivalVassalAttitude(ePlayer);
	iAttitude += AI_getShareWarAttitude(ePlayer);
	iAttitude += AI_getFavoriteCivicAttitude(ePlayer);
	iAttitude += AI_getTradeAttitude(ePlayer);
	iAttitude += AI_getRivalTradeAttitude(ePlayer);
/************************************************************************************************/
/* Afforess	                  Start		 12/14/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	iAttitude += AI_getCivicShareAttitude(ePlayer);
	iAttitude += AI_getEmbassyAttitude(ePlayer);
	iAttitude += AI_getCivicAttitudeChange(ePlayer);
/************************************************************************************************/
/* Afforess                             END                                                     */
/************************************************************************************************/

	for (iI = 0; iI < NUM_MEMORY_TYPES; iI++)
	{
		iAttitude += AI_getMemoryAttitude(ePlayer, ((MemoryTypes)iI));
	}

	iAttitude += AI_getColonyAttitude(ePlayer);
	iAttitude += AI_getAttitudeExtra(ePlayer);

/************************************************************************************************/
/* REVOLUTION_MOD                         05/18/08                                jdog5000      */
/*                                                                                              */
/* Revolution AI                                                                                */
/************************************************************************************************/
	if( GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isRebelAgainst(getTeam()) )
	{
		iAttitude -= 5;
	}
	else if( GET_TEAM(getTeam()).isRebelAgainst(GET_PLAYER(ePlayer).getTeam()) )
	{
		iAttitude -= 3;
	}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* Afforess	                  Start		 06/01/10                                               */
/*                                                                                              */
/* Ruthless AI: The Enemy of Our Enemy is our Friend!                                           */
/************************************************************************************************/
	if (GC.getGameINLINE().isOption(GAMEOPTION_RUTHLESS_AI))
	{
		if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_getWorstEnemy() == GET_TEAM(getTeam()).AI_getWorstEnemy())
		{
			iAttitude += 2;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/03/09                       poyuzhe & jdog5000     */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
	// From Sanguo Mod Performance, ie the CAR Mod
	// Attitude cache
	m_aiAttitudeCache[ePlayer] = range(iAttitude, -100, 100);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	return range(iAttitude, -100, 100);
}


// BEGIN: Show Hidden Attitude Mod 01/22/2009
bool isShowPersonalityModifiers()
{
#ifdef _MOD_SHAM_SPOILER
	return true;
#else
	return !GC.getGameINLINE().isOption(GAMEOPTION_RANDOM_PERSONALITIES) || GC.getGameINLINE().isDebugMode();
#endif
}

bool isShowSpoilerModifiers()
{
#ifdef _MOD_SHAM_SPOILER
	return true;
#else
	return GC.getGameINLINE().isDebugMode();
#endif
}

int CvPlayerAI::AI_getFirstImpressionAttitude(PlayerTypes ePlayer) const
{
	bool bShowPersonalityAttitude = isShowPersonalityModifiers();
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
    int iAttitude = GC.getHandicapInfo(kPlayer.getHandicapType()).getAttitudeChange();

	if (bShowPersonalityAttitude)
	{
		iAttitude += GC.getLeaderHeadInfo(getPersonalityType()).getBaseAttitude();
		if (!kPlayer.isHuman())
		{
			if (isShowSpoilerModifiers())
			{
				// iBasePeaceWeight + iPeaceWeightRand
				iAttitude += (4 - abs(AI_getPeaceWeight() - kPlayer.AI_getPeaceWeight()));
			}
			else
			{
				// iBasePeaceWeight
				iAttitude += (4 - abs(GC.getLeaderHeadInfo(getPersonalityType()).getBasePeaceWeight() - GC.getLeaderHeadInfo(kPlayer.getPersonalityType()).getBasePeaceWeight()));
			}
			iAttitude += std::min(GC.getLeaderHeadInfo(getPersonalityType()).getWarmongerRespect(), GC.getLeaderHeadInfo(kPlayer.getPersonalityType()).getWarmongerRespect());
		}
	}

    return iAttitude;
}


int CvPlayerAI::AI_getTeamSizeAttitude(PlayerTypes ePlayer) const
{
	return -std::max(0, (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getNumMembers() - GET_TEAM(getTeam()).getNumMembers()));
}


// Count only players visible on the active player's scoreboard
int CvPlayerAI::AI_getKnownPlayerRank(PlayerTypes ePlayer) const
{
    PlayerTypes eActivePlayer = GC.getGameINLINE().getActivePlayer();
    if (NO_PLAYER == eActivePlayer || GC.getGameINLINE().isDebugMode()) {
        // Use the full scoreboard
        return GC.getGameINLINE().getPlayerRank(ePlayer);
    }

	TeamTypes eActiveTeam = GC.getGameINLINE().getActiveTeam();
    int iRank = 0;
    for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
    {
        PlayerTypes eRankPlayer = GC.getGameINLINE().getRankPlayer(iI);
		if (eRankPlayer != NO_PLAYER)
		{
			CvTeam& kRankTeam = GET_TEAM(GET_PLAYER(eRankPlayer).getTeam());
			if (kRankTeam.isAlive() && (kRankTeam.isHasMet(eActiveTeam) || kRankTeam.isHuman()))
			{
				if (eRankPlayer == ePlayer) {
					return iRank;
				}
				iRank++;
			}
        }
    }

    // Should only get here if we tried to find the rank of an unknown player
    return iRank + 1;
}


int CvPlayerAI::AI_getBetterRankDifferenceAttitude(PlayerTypes ePlayer) const
{
	if (!isShowPersonalityModifiers())
	{
		return 0;
	}

	int iRankDifference;
	if (isShowSpoilerModifiers())
	{
	    iRankDifference = GC.getGameINLINE().getPlayerRank(ePlayer) - GC.getGameINLINE().getPlayerRank(getID());
	}
	else
	{
	    iRankDifference = AI_getKnownPlayerRank(ePlayer) - AI_getKnownPlayerRank(getID());
	}

	if (iRankDifference > 0)
	{
		return GC.getLeaderHeadInfo(getPersonalityType()).getBetterRankDifferenceAttitudeChange() * iRankDifference / (GC.getGameINLINE().countCivPlayersEverAlive() + 1);
	}

    return 0;
}


int CvPlayerAI::AI_getWorseRankDifferenceAttitude(PlayerTypes ePlayer) const
{
	if (!isShowPersonalityModifiers())
	{
		return 0;
	}

	int iRankDifference;
	if (isShowSpoilerModifiers())
	{
	    iRankDifference = GC.getGameINLINE().getPlayerRank(getID()) - GC.getGameINLINE().getPlayerRank(ePlayer);
	}
	else
	{
	    iRankDifference = AI_getKnownPlayerRank(getID()) - AI_getKnownPlayerRank(ePlayer);
	}

	if (iRankDifference > 0)
	{
		return GC.getLeaderHeadInfo(getPersonalityType()).getWorseRankDifferenceAttitudeChange() * iRankDifference / (GC.getGameINLINE().countCivPlayersEverAlive() + 1);
	}

    return 0;
}


int CvPlayerAI::AI_getLowRankAttitude(PlayerTypes ePlayer) const
{
	int iThisPlayerRank;
	int iPlayerRank;
	if (isShowSpoilerModifiers())
	{
		iThisPlayerRank = GC.getGameINLINE().getPlayerRank(getID());
		iPlayerRank = GC.getGameINLINE().getPlayerRank(ePlayer);
	}
	else
	{
		iThisPlayerRank = AI_getKnownPlayerRank(getID());
		iPlayerRank = AI_getKnownPlayerRank(ePlayer);
	}

	int iMedianRank = GC.getGameINLINE().countCivPlayersEverAlive() / 2;
	return (iThisPlayerRank >= iMedianRank && iPlayerRank >= iMedianRank) ? 1 : 0;
}


int CvPlayerAI::AI_getLostWarAttitude(PlayerTypes ePlayer) const
{
	if (!isShowPersonalityModifiers())
	{
		return 0;
	}

	TeamTypes eTeam = GET_PLAYER(ePlayer).getTeam();
    if (!isShowSpoilerModifiers() && NO_PLAYER != GC.getGameINLINE().getActivePlayer())
    {
        // Hide war success for wars you are not involved in
        if (GC.getGameINLINE().getActiveTeam() != getTeam() && GC.getGameINLINE().getActiveTeam() != eTeam)
        {
            return 0;
        }
    }

	if (GET_TEAM(eTeam).AI_getWarSuccess(getTeam()) > GET_TEAM(getTeam()).AI_getWarSuccess(eTeam))
	{
		return GC.getLeaderHeadInfo(getPersonalityType()).getLostWarAttitudeChange();
	}

    return 0;
}
// END: Show Hidden Attitude Mod


int CvPlayerAI::AI_calculateStolenCityRadiusPlots(PlayerTypes ePlayer) const
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	int iCount;
	int iI;

	FAssert(ePlayer != getID());

	iCount = 0;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->getOwnerINLINE() == ePlayer)
		{
			if (pLoopPlot->isPlayerCityRadius(getID()))
			{
				iCount++;
			}
		}
	}

	return iCount;
}


int CvPlayerAI::AI_getCloseBordersAttitude(PlayerTypes ePlayer) const
{
	if (m_aiCloseBordersAttitudeCache[ePlayer] == MAX_INT)
	{
		PROFILE_FUNC();
		int iPercent;

		if (getTeam() == GET_PLAYER(ePlayer).getTeam() || GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()) || GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()))
		{
			return 0;
		}

		iPercent = std::min(60, (AI_calculateStolenCityRadiusPlots(ePlayer) * 3));

		if (GET_TEAM(getTeam()).AI_isLandTarget(GET_PLAYER(ePlayer).getTeam()))
		{
			iPercent += 40;
		}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/24/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
		if( AI_isDoStrategy(AI_VICTORY_CONQUEST3) )
		{
			iPercent = std::min( 120, (3 * iPercent)/2 );
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

		m_aiCloseBordersAttitudeCache[ePlayer] = ((GC.getLeaderHeadInfo(getPersonalityType()).getCloseBordersAttitudeChange() * iPercent) / 100);
	}

	return m_aiCloseBordersAttitudeCache[ePlayer];
}


int CvPlayerAI::AI_getWarAttitude(PlayerTypes ePlayer) const
{
	int iAttitudeChange;
	int iAttitude;

	iAttitude = 0;

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		iAttitude -= 3;
	}

	if (GC.getLeaderHeadInfo(getPersonalityType()).getAtWarAttitudeDivisor() != 0)
	{
		iAttitudeChange = (GET_TEAM(getTeam()).AI_getAtWarCounter(GET_PLAYER(ePlayer).getTeam()) / GC.getLeaderHeadInfo(getPersonalityType()).getAtWarAttitudeDivisor());
		iAttitude += range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getAtWarAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getAtWarAttitudeChangeLimit()));
	}

	return iAttitude;
}


int CvPlayerAI::AI_getPeaceAttitude(PlayerTypes ePlayer) const
{
	int iAttitudeChange;

	if (GC.getLeaderHeadInfo(getPersonalityType()).getAtPeaceAttitudeDivisor() != 0)
	{
		iAttitudeChange = (GET_TEAM(getTeam()).AI_getAtPeaceCounter(GET_PLAYER(ePlayer).getTeam()) / GC.getLeaderHeadInfo(getPersonalityType()).getAtPeaceAttitudeDivisor());
		return range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getAtPeaceAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getAtPeaceAttitudeChangeLimit()));
	}

	return 0;
}


int CvPlayerAI::AI_getSameReligionAttitude(PlayerTypes ePlayer) const
{
	int iAttitudeChange;
	int iAttitude;

	iAttitude = 0;

	if ((getStateReligion() != NO_RELIGION) && (getStateReligion() == GET_PLAYER(ePlayer).getStateReligion()))
	{
		iAttitude += GC.getLeaderHeadInfo(getPersonalityType()).getSameReligionAttitudeChange();

		if (hasHolyCity(getStateReligion()))
		{
			iAttitude++;
		}

		if (GC.getLeaderHeadInfo(getPersonalityType()).getSameReligionAttitudeDivisor() != 0)
		{
			iAttitudeChange = (AI_getSameReligionCounter(ePlayer) / GC.getLeaderHeadInfo(getPersonalityType()).getSameReligionAttitudeDivisor());
			iAttitude += range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getSameReligionAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getSameReligionAttitudeChangeLimit()));
		}
	}

	return iAttitude;
}


int CvPlayerAI::AI_getDifferentReligionAttitude(PlayerTypes ePlayer) const
{
	int iAttitudeChange;
	int iAttitude;

	iAttitude = 0;

	if ((getStateReligion() != NO_RELIGION) && (GET_PLAYER(ePlayer).getStateReligion() != NO_RELIGION) && (getStateReligion() != GET_PLAYER(ePlayer).getStateReligion()))
	{
		iAttitude += GC.getLeaderHeadInfo(getPersonalityType()).getDifferentReligionAttitudeChange();

		if (hasHolyCity(getStateReligion()))
		{
			iAttitude--;
		}

		if (GC.getLeaderHeadInfo(getPersonalityType()).getDifferentReligionAttitudeDivisor() != 0)
		{
			iAttitudeChange = (AI_getDifferentReligionCounter(ePlayer) / GC.getLeaderHeadInfo(getPersonalityType()).getDifferentReligionAttitudeDivisor());
			iAttitude += range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getDifferentReligionAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getDifferentReligionAttitudeChangeLimit()));
		}
	}

	return iAttitude;
}


int CvPlayerAI::AI_getBonusTradeAttitude(PlayerTypes ePlayer) const
{
	int iAttitudeChange;

	if (!atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		if (GC.getLeaderHeadInfo(getPersonalityType()).getBonusTradeAttitudeDivisor() != 0)
		{
			iAttitudeChange = (AI_getBonusTradeCounter(ePlayer) / GC.getLeaderHeadInfo(getPersonalityType()).getBonusTradeAttitudeDivisor());
			return range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getBonusTradeAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getBonusTradeAttitudeChangeLimit()));
		}
	}

	return 0;
}


int CvPlayerAI::AI_getOpenBordersAttitude(PlayerTypes ePlayer) const
{
	int iAttitudeChange;

	if (!atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		if (GC.getLeaderHeadInfo(getPersonalityType()).getOpenBordersAttitudeDivisor() != 0)
		{
			iAttitudeChange = (GET_TEAM(getTeam()).AI_getOpenBordersCounter(GET_PLAYER(ePlayer).getTeam()) / GC.getLeaderHeadInfo(getPersonalityType()).getOpenBordersAttitudeDivisor());
			return range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getOpenBordersAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getOpenBordersAttitudeChangeLimit()));
		}
	}

	return 0;
}


int CvPlayerAI::AI_getDefensivePactAttitude(PlayerTypes ePlayer) const
{
	int iAttitudeChange;

	if (getTeam() != GET_PLAYER(ePlayer).getTeam() && (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()) || GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam())))
	{
		return GC.getLeaderHeadInfo(getPersonalityType()).getDefensivePactAttitudeChangeLimit();
	}

	if (!atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		if (GC.getLeaderHeadInfo(getPersonalityType()).getDefensivePactAttitudeDivisor() != 0)
		{
			iAttitudeChange = (GET_TEAM(getTeam()).AI_getDefensivePactCounter(GET_PLAYER(ePlayer).getTeam()) / GC.getLeaderHeadInfo(getPersonalityType()).getDefensivePactAttitudeDivisor());
			return range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getDefensivePactAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getDefensivePactAttitudeChangeLimit()));
		}
	}

	return 0;
}


int CvPlayerAI::AI_getRivalDefensivePactAttitude(PlayerTypes ePlayer) const
{
	int iAttitude = 0;

	if (getTeam() == GET_PLAYER(ePlayer).getTeam() || GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()) || GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()))
	{
		return iAttitude;
	}

	if (!(GET_TEAM(getTeam()).isDefensivePact(GET_PLAYER(ePlayer).getTeam())))
	{
		iAttitude -= ((4 * GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getDefensivePactCount(GET_PLAYER(ePlayer).getTeam())) / std::max(1, (GC.getGameINLINE().countCivTeamsAlive() - 2)));
	}

	return iAttitude;
}


int CvPlayerAI::AI_getRivalVassalAttitude(PlayerTypes ePlayer) const
{
	int iAttitude = 0;

	if (getTeam() == GET_PLAYER(ePlayer).getTeam() || GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()) || GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()))
	{
		return iAttitude;
	}

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getVassalCount(getTeam()) > 0)
	{
		iAttitude -= (6 * GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getPower(true)) / std::max(1, GC.getGameINLINE().countTotalCivPower());
	}

	return iAttitude;
}


int CvPlayerAI::AI_getShareWarAttitude(PlayerTypes ePlayer) const
{
	int iAttitudeChange;
	int iAttitude;

	iAttitude = 0;

	if (!atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		if (GET_TEAM(getTeam()).AI_shareWar(GET_PLAYER(ePlayer).getTeam()))
		{
			iAttitude += GC.getLeaderHeadInfo(getPersonalityType()).getShareWarAttitudeChange();
		}

		if (GC.getLeaderHeadInfo(getPersonalityType()).getShareWarAttitudeDivisor() != 0)
		{
			iAttitudeChange = (GET_TEAM(getTeam()).AI_getShareWarCounter(GET_PLAYER(ePlayer).getTeam()) / GC.getLeaderHeadInfo(getPersonalityType()).getShareWarAttitudeDivisor());
			iAttitude += range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getShareWarAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getShareWarAttitudeChangeLimit()));
		}
	}

	return iAttitude;
}


int CvPlayerAI::AI_getFavoriteCivicAttitude(PlayerTypes ePlayer) const
{
	int iAttitudeChange;

	int iAttitude;

	iAttitude = 0;

	if (GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic() != NO_CIVIC)
	{
		if (isCivic((CivicTypes)(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic())) && GET_PLAYER(ePlayer).isCivic((CivicTypes)(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic())))
		{
			iAttitude += GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivicAttitudeChange();

			if (GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivicAttitudeDivisor() != 0)
			{
				iAttitudeChange = (AI_getFavoriteCivicCounter(ePlayer) / GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivicAttitudeDivisor());
				iAttitude += range(iAttitudeChange, -(abs(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivicAttitudeChangeLimit())), abs(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivicAttitudeChangeLimit()));
			}
		}
	}

	return iAttitude;
}


int CvPlayerAI::AI_getTradeAttitude(PlayerTypes ePlayer) const
{
	// XXX human only?
	return range(((AI_getPeacetimeGrantValue(ePlayer) + std::max(0, (AI_getPeacetimeTradeValue(ePlayer) - GET_PLAYER(ePlayer).AI_getPeacetimeTradeValue(getID())))) / ((GET_TEAM(getTeam()).AI_getHasMetCounter(GET_PLAYER(ePlayer).getTeam()) + 1) * 5)), 0, 4);
}


int CvPlayerAI::AI_getRivalTradeAttitude(PlayerTypes ePlayer) const
{
	// XXX human only?

	return -(range(((GET_TEAM(getTeam()).AI_getEnemyPeacetimeGrantValue(GET_PLAYER(ePlayer).getTeam()) + (GET_TEAM(getTeam()).AI_getEnemyPeacetimeTradeValue(GET_PLAYER(ePlayer).getTeam()) / 3)) / ((GET_TEAM(getTeam()).AI_getHasMetCounter(GET_PLAYER(ePlayer).getTeam()) + 1) * 10)), 0, 4));
}


int CvPlayerAI::AI_getMemoryAttitude(PlayerTypes ePlayer, MemoryTypes eMemory) const
{
	return ((AI_getMemoryCount(ePlayer, eMemory) * GC.getLeaderHeadInfo(getPersonalityType()).getMemoryAttitudePercent(eMemory)) / 100);
}

int CvPlayerAI::AI_getColonyAttitude(PlayerTypes ePlayer) const
{
	int iAttitude = 0;

	if (getParent() == ePlayer)
	{
		iAttitude += GC.getLeaderHeadInfo(getPersonalityType()).getFreedomAppreciation();
	}

	return iAttitude;
}



PlayerVoteTypes CvPlayerAI::AI_diploVote(const VoteSelectionSubData& kVoteData, VoteSourceTypes eVoteSource, bool bPropose)
{
	PROFILE_FUNC();

	CivicTypes eBestCivic;
	int iOpenCount;
	int iClosedCount;
	int iValue;
	int iBestValue;
	int iI;

	VoteTypes eVote = kVoteData.eVote;

	if (GC.getGameINLINE().isTeamVote(eVote))
	{
		if (GC.getGameINLINE().isTeamVoteEligible(getTeam(), eVoteSource))
		{
			return (PlayerVoteTypes)getTeam();
		}

		if (GC.getVoteInfo(eVote).isVictory())
		{
			iBestValue = 7;
		}
		else
		{
			iBestValue = 0;
		}

		PlayerVoteTypes eBestTeam = PLAYER_VOTE_ABSTAIN;

		for (iI = 0; iI < MAX_TEAMS; iI++)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (GC.getGameINLINE().isTeamVoteEligible((TeamTypes)iI, eVoteSource))
				{
					if (GET_TEAM(getTeam()).isVassal((TeamTypes)iI))
					{
						return (PlayerVoteTypes)iI;
					}

					iValue = GET_TEAM(getTeam()).AI_getAttitudeVal((TeamTypes)iI);

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						eBestTeam = (PlayerVoteTypes)iI;
					}
				}
			}
		}

		return eBestTeam;
	}
	else
	{
		TeamTypes eSecretaryGeneral = GC.getGameINLINE().getSecretaryGeneral(eVoteSource);

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/30/08                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
/* original BTS code
		if (!bPropose)
		{
			if (eSecretaryGeneral != NO_TEAM)
			{
				if (eSecretaryGeneral == getTeam() ||(GET_TEAM(getTeam()).AI_getAttitude(eSecretaryGeneral) == ATTITUDE_FRIENDLY))
				{
					return PLAYER_VOTE_YES;
				}
			}
		}						
*/
		// Remove blanket auto approval for friendly secretary
		bool bFriendlyToSecretary = false;
		if (!bPropose)
		{
			if (eSecretaryGeneral != NO_TEAM)
			{
				if (eSecretaryGeneral == getTeam())
				{
					return PLAYER_VOTE_YES;
				}
				else
				{
					bFriendlyToSecretary = (GET_TEAM(getTeam()).AI_getAttitude(eSecretaryGeneral) == ATTITUDE_FRIENDLY);
				}
			}
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		
		bool bDefy = false;

		bool bValid = true;

		if (bValid)
		{
			for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
			{
				if (GC.getVoteInfo(eVote).isForceCivic(iI))
				{
					if (!isCivic((CivicTypes)iI))
					{
						eBestCivic = AI_bestCivic((CivicOptionTypes)(GC.getCivicInfo((CivicTypes)iI).getCivicOptionType()));

						if (eBestCivic != NO_CIVIC)
						{
							if (eBestCivic != ((CivicTypes)iI))
							{
								int iBestCivicValue = AI_civicValue(eBestCivic);
								int iNewCivicValue = AI_civicValue((CivicTypes)iI);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/30/08                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
/* original BTS code
								if (iBestCivicValue > ((iNewCivicValue * 120) / 100))
								{
									bValid = false;
									if (iBestCivicValue > ((iNewCivicValue * (140 + (GC.getGame().getSorenRandNum(120, "AI Erratic Defiance (Force Civic)"))) / 100)))
*/
								// Increase threshold of voting for friend's proposal
								if( bFriendlyToSecretary )
								{
									iNewCivicValue *= 6;
									iNewCivicValue /= 5;
								}

								if (iBestCivicValue > ((iNewCivicValue * 120) / 100))
								{
									bValid = false;

									// Increase odds of defiance, particularly on AggressiveAI
									if (iBestCivicValue > ((iNewCivicValue * (140 + (GC.getGame().getSorenRandNum((GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? 60 : 80), "AI Erratic Defiance (Force Civic)"))) / 100)))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
									{
										bDefy = true;										
									}
									break;
								}
							}
						}
					}
				}
			}
		}

		if (bValid)
		{
			if (GC.getVoteInfo(eVote).getTradeRoutes() > 0)
			{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/30/08                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
				if( bFriendlyToSecretary )
				{
					return PLAYER_VOTE_YES;
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
				if (getNumCities() > ((GC.getGameINLINE().getNumCities() * 2) / (GC.getGameINLINE().countCivPlayersAlive() + 1)))
				{
					bValid = false;
				}
			}
		}

		if (bValid)
		{
			if (GC.getVoteInfo(eVote).isNoNukes())
			{
				int iVoteBanThreshold = 0;
				iVoteBanThreshold += GET_TEAM(getTeam()).getNukeInterception() / 3;
				iVoteBanThreshold += GC.getLeaderHeadInfo(getPersonalityType()).getBuildUnitProb();
				iVoteBanThreshold *= std::max(1, GC.getLeaderHeadInfo(getPersonalityType()).getWarmongerRespect());
				if (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
				{
					iVoteBanThreshold *= 2;
				}

				bool bAnyHasSdi = false;
				for (iI = 0; iI < MAX_TEAMS; iI++)
				{
					if (GET_TEAM((TeamTypes)iI).isAlive() && iI != getTeam())
					{
						if (GET_TEAM((TeamTypes)iI).getNukeInterception() > 0)
						{
							bAnyHasSdi = true;
							break;
						}
					}
				}

				if (!bAnyHasSdi && GET_TEAM(getTeam()).getNukeInterception() > 0 && GET_TEAM(getTeam()).getNumNukeUnits() > 0)
				{
					iVoteBanThreshold *= 2;
				}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/30/08                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
				if( bFriendlyToSecretary )
				{
					iVoteBanThreshold *= 2;
					iVoteBanThreshold /= 3;
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

				bValid = (GC.getGameINLINE().getSorenRandNum(100, "AI nuke ban vote") > iVoteBanThreshold);
				
				if (AI_isDoStrategy(AI_STRATEGY_OWABWNW))
				{
					bValid = false;
				}
				else if ((GET_TEAM(getTeam()).getNumNukeUnits() / std::max(1, GET_TEAM(getTeam()).getNumMembers())) < (GC.getGameINLINE().countTotalNukeUnits() / std::max(1, GC.getGameINLINE().countCivPlayersAlive())))
				{
					bValid = false;
				}
				if (!bValid && AI_getNumTrainAIUnits(UNITAI_ICBM) > 0)
				{
					if (GC.getGame().getSorenRandNum(AI_isDoStrategy(AI_STRATEGY_OWABWNW) ? 2 : 3, "AI Erratic Defiance (No Nukes)") == 0)
					{
						bDefy = true;
					}
				}
			}
		}

		if (bValid)
		{
			if (GC.getVoteInfo(eVote).isFreeTrade())
			{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/30/08                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
				if( bFriendlyToSecretary )
				{
					return PLAYER_VOTE_YES;
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

				iOpenCount = 0;
				iClosedCount = 0;

				for (iI = 0; iI < MAX_TEAMS; iI++)
				{
					if (GET_TEAM((TeamTypes)iI).isAlive())
					{
						if (iI != getTeam())
						{
							if (GET_TEAM(getTeam()).isOpenBorders((TeamTypes)iI))
							{
								iOpenCount += GET_TEAM((TeamTypes)iI).getNumCities();
							}
							else
							{
								iClosedCount += GET_TEAM((TeamTypes)iI).getNumCities();
							}
						}
					}
				}

				if (iOpenCount >= (getNumCities() * getTradeRoutes()))
				{
					bValid = false;
				}

				if (iClosedCount == 0)
				{
					bValid = false;
				}
			}
		}

		if (bValid)
		{
			if (GC.getVoteInfo(eVote).isOpenBorders())
			{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/30/08                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
				if( bFriendlyToSecretary )
				{
					return PLAYER_VOTE_YES;
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

				bValid = true;

				for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
				{
					if (iI != getTeam())
					{
						if (GET_TEAM((TeamTypes)iI).isVotingMember(eVoteSource))
						{
							if (NO_DENIAL != GET_TEAM(getTeam()).AI_openBordersTrade((TeamTypes)iI))
							{
								bValid = false;
								break;
							}
						}
					}
				}
			}
			else if (GC.getVoteInfo(eVote).isDefensivePact())
			{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/30/08                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
				if( bFriendlyToSecretary )
				{
					return PLAYER_VOTE_YES;
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

				bValid = true;

				for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
				{
					if (iI != getTeam())
					{
						if (GET_TEAM((TeamTypes)iI).isVotingMember(eVoteSource))
						{
							if (NO_DENIAL != GET_TEAM(getTeam()).AI_defensivePactTrade((TeamTypes)iI))
							{
								bValid = false;
								break;
							}
						}
					}
				}
			}
			else if (GC.getVoteInfo(eVote).isForcePeace())
			{
				FAssert(kVoteData.ePlayer != NO_PLAYER);
				TeamTypes ePeaceTeam = GET_PLAYER(kVoteData.ePlayer).getTeam();
				
				int iWarsWinning = 0;
				int iWarsLosing = 0;
				int iChosenWar = 0;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      01/03/09                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
				bool bLosingBig = false;
				bool bWinningBig = false;
				bool bThisPlayerWinning = false;

				int iWinDeltaThreshold = 3*GC.getDefineINT("WAR_SUCCESS_ATTACKING");
				int iLossAbsThreshold = std::max(3, getNumMilitaryUnits()/40)*GC.getDefineINT("WAR_SUCCESS_ATTACKING");

				bool bAggressiveAI = GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI);
				if( bAggressiveAI )
				{
					iWinDeltaThreshold *= 2;
					iWinDeltaThreshold /= 3;

					iLossAbsThreshold *= 4;
					iLossAbsThreshold /= 3;
				}

				// Is ePeaceTeam winning wars?
				for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
				{
					if (GET_TEAM((TeamTypes)iI).isAlive())
					{
						if (iI != ePeaceTeam)
						{
							if (GET_TEAM((TeamTypes)iI).isAtWar(ePeaceTeam))
							{
								int iPeaceTeamSuccess = GET_TEAM(ePeaceTeam).AI_getWarSuccess((TeamTypes)iI);
								int iOtherTeamSuccess = GET_TEAM((TeamTypes)iI).AI_getWarSuccess(ePeaceTeam);
								
								if ( (iPeaceTeamSuccess - iOtherTeamSuccess) > iWinDeltaThreshold )
								{
									// Have to be ahead by at least a few victories to count as win
									++iWarsWinning;

									if ( (iPeaceTeamSuccess - iOtherTeamSuccess) > (3*iWinDeltaThreshold) )
									{
										bWinningBig = true;
									}
								}
								else if( (iOtherTeamSuccess >= iPeaceTeamSuccess) )
								{
									if( iI == getTeam() )
									{
										if( (iOtherTeamSuccess - iPeaceTeamSuccess) > iWinDeltaThreshold )
										{
											bThisPlayerWinning = true;
										}
									}

									if( (iOtherTeamSuccess > iLossAbsThreshold) )
									{
										// Have to have non-trivial loses
										++iWarsLosing;

										if( (iOtherTeamSuccess - iPeaceTeamSuccess) > (3*iLossAbsThreshold) )
										{
											bLosingBig = true;
										}						
									}
									else if( GET_TEAM(ePeaceTeam).AI_getAtWarCounter((TeamTypes)iI) < 10 )
									{
										// Not winning, just recently attacked, and in multiple wars, be pessimistic
										// Counts ties from no actual battles
										if( (GET_TEAM(ePeaceTeam).getAtWarCount(true) > 1) && !(GET_TEAM(ePeaceTeam).AI_isChosenWar((TeamTypes)iI)) )
										{
											++iWarsLosing;
										}
									}
								}

								if (GET_TEAM(ePeaceTeam).AI_isChosenWar((TeamTypes)iI))
								{
									++iChosenWar;									
								}
							}
						}
					}
				}
				
				if (ePeaceTeam == getTeam())
				{
					int iPeaceRand = GC.getLeaderHeadInfo(getPersonalityType()).getBasePeaceWeight();
					iPeaceRand /= (bAggressiveAI ? 2 : 1);
					
					// Always true for real war-mongers, rarely true for less aggressive types
					bool bWarmongerRoll = (GC.getGame().getSorenRandNum(iPeaceRand, "AI Erratic Defiance (Force Peace)") == 0);

					if( bLosingBig && (!bWarmongerRoll || bPropose) )
					{
						// Non-warmongers want peace to escape loss
						bValid = true;
					}
					else if ( !bLosingBig && (iChosenWar > iWarsLosing) )
					{
						// If chosen to be in most wars, keep it going
						bValid = false;
					}
					else
					{
						// If losing most wars, vote for peace
						bValid = (iWarsLosing > iWarsWinning);
					}

					if (!bValid && !bLosingBig && bWinningBig)
					{
						// Can we continue this war with defiance penalties?
						if( !AI_isFinancialTrouble() )
						{
							if (bWarmongerRoll)
							{
								bDefy = true;
							}
						}
					}
				}
				else if (eSecretaryGeneral == getTeam() && !bPropose)
				{
					bValid = true;
				}
				else if (GET_TEAM(ePeaceTeam).isAtWar(getTeam()))
				{
					bool bWantsToEndWar = (GET_TEAM(getTeam()).AI_endWarVal(ePeaceTeam) > (3*GET_TEAM(ePeaceTeam).AI_endWarVal(getTeam()))/2);
					bValid = bWantsToEndWar;

					if( bValid )
					{
						bValid = bWinningBig || (iWarsWinning > iWarsLosing) || (GET_TEAM(getTeam()).getAtWarCount(true, true) > 1);
					}

					if (!bValid && bThisPlayerWinning && (iWarsLosing >= iWarsWinning) && !bPropose )
					{
						if( !GET_TEAM(getTeam()).isAVassal() )
						{
							if( (GET_TEAM(getTeam()).getAtWarCount(true) == 1) || bLosingBig )
							{
								// Can we continue this war with defiance penalties?
								if( !AI_isFinancialTrouble() )
								{
									int iDefyRand = GC.getLeaderHeadInfo(getPersonalityType()).getBasePeaceWeight();
									iDefyRand /= (bAggressiveAI ? 2 : 1);

									if (GC.getGame().getSorenRandNum(iDefyRand, "AI Erratic Defiance (Force Peace)") == 0)
									{
										bDefy = true;
									}
								}
							}
						}
					}

					if( !bValid && !bDefy && !bPropose )
					{
						if((GET_TEAM(getTeam()).AI_getAttitude(eSecretaryGeneral) > GC.getLeaderHeadInfo(getPersonalityType()).getVassalRefuseAttitudeThreshold()) )
						{
							// Influence by secretary
							if( NO_DENIAL == GET_TEAM(getTeam()).AI_makePeaceTrade(ePeaceTeam, eSecretaryGeneral) )
							{
								bValid = true;
							}
							else if( eSecretaryGeneral != NO_TEAM && GET_TEAM(getTeam()).isVassal(eSecretaryGeneral) )
							{
								bValid = true;
							}
						}
					}
				}
				else
				{
					if( GET_TEAM(getTeam()).AI_getWarPlan(ePeaceTeam) != NO_WARPLAN )
					{
						// Keep planned enemy occupied
						bValid = false;
					}
					else if( GET_TEAM(getTeam()).AI_shareWar(ePeaceTeam)  && !(GET_TEAM(getTeam()).isVassal(ePeaceTeam)) )
					{
						// Keep ePeaceTeam at war with our common enemies
						bValid = false;
					}
					else if(iWarsLosing > iWarsWinning)
					{
						// Feel pity for team that is losing (if like them enough to not declare war on them)
						bValid = (GET_TEAM(getTeam()).AI_getAttitude(ePeaceTeam) >= GC.getLeaderHeadInfo(getPersonalityType()).getDeclareWarThemRefuseAttitudeThreshold());
					}
					else 
					{
						// Stop a team that is winning (if don't like them enough to join them in war)
						bValid = (GET_TEAM(getTeam()).AI_getAttitude(ePeaceTeam) < GC.getLeaderHeadInfo(getPersonalityType()).getDeclareWarRefuseAttitudeThreshold());
					}

					if( !bValid )
					{
						if( bFriendlyToSecretary && !GET_TEAM(getTeam()).isVassal(ePeaceTeam) )
						{
							// Influence by secretary
							bValid = true;
						}
					}
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

			}
			else if (GC.getVoteInfo(eVote).isForceNoTrade())
			{
				FAssert(kVoteData.ePlayer != NO_PLAYER);
				TeamTypes eEmbargoTeam = GET_PLAYER(kVoteData.ePlayer).getTeam();

				if (eSecretaryGeneral == getTeam() && !bPropose)
				{
					bValid = true;
				}
				else if (eEmbargoTeam == getTeam())
				{
					bValid = false;
					if (!isNoForeignTrade())
					{
						bDefy = true;
					}
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/30/08                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
				else
				{
					if( bFriendlyToSecretary )
					{
						return PLAYER_VOTE_YES;
					}
					else if( canStopTradingWithTeam(eEmbargoTeam) )
					{
						bValid = (NO_DENIAL == AI_stopTradingTrade(eEmbargoTeam, kVoteData.ePlayer));
						if (bValid)
						{
							bValid = (GET_TEAM(getTeam()).AI_getAttitude(eEmbargoTeam) <= ATTITUDE_CAUTIOUS);
						}
					}
					else
					{
						bValid = (GET_TEAM(getTeam()).AI_getAttitude(eEmbargoTeam) < ATTITUDE_CAUTIOUS);
					}
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
			}
			else if (GC.getVoteInfo(eVote).isForceWar())
			{
				FAssert(kVoteData.ePlayer != NO_PLAYER);
				TeamTypes eWarTeam = GET_PLAYER(kVoteData.ePlayer).getTeam();

				if (eSecretaryGeneral == getTeam() && !bPropose)
				{
					bValid = true;
				}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/30/08                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
/* original BTS code
				else if (eWarTeam == getTeam())
				{
					bValid = false;
				}
				else if (GET_TEAM(eWarTeam).isAtWar(getTeam()))
*/
				else if (eWarTeam == getTeam() || GET_TEAM(getTeam()).isVassal(eWarTeam))
				{
					// Explicit rejection by all who will definitely be attacked
					bValid = false;
				}
				else if ( GET_TEAM(getTeam()).AI_getWarPlan(eWarTeam) != NO_WARPLAN )
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
				{
					bValid = true;
				}
				else
				{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      07/20/09                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
/* original BTS code
					bValid = (bPropose || NO_DENIAL == GET_TEAM(getTeam()).AI_declareWarTrade(eWarTeam, eSecretaryGeneral));
					if (bValid)
					{
						bValid = (GET_TEAM(getTeam()).AI_getAttitude(eWarTeam) < ATTITUDE_CAUTIOUS);
					}
*/
					if( !bPropose && GET_TEAM(getTeam()).isAVassal() )
					{
						// Vassals always deny war trade requests and thus previously always voted no
						bValid = false;

						if( GET_TEAM(getTeam()).getAnyWarPlanCount(true) == 0 )
						{
							if( eSecretaryGeneral == NO_TEAM || (GET_TEAM(getTeam()).AI_getAttitude(eSecretaryGeneral) > GC.getLeaderHeadInfo(getPersonalityType()).getDeclareWarRefuseAttitudeThreshold()) )
							{
								if( eSecretaryGeneral != NO_TEAM && GET_TEAM(getTeam()).isVassal(eSecretaryGeneral) )
								{
									bValid = true;
								}
								else if( (GET_TEAM(getTeam()).isAVassal() ? GET_TEAM(getTeam()).getCurrentMasterPower(true) : GET_TEAM(getTeam()).getPower(true)) > GET_TEAM(eWarTeam).getDefensivePower() )
								{
									bValid = true;
								}
							}
						}
					}
					else
					{
						bValid = (bPropose || NO_DENIAL == GET_TEAM(getTeam()).AI_declareWarTrade(eWarTeam, eSecretaryGeneral));
					}

					if (bValid)
					{
						int iNoWarOdds = GC.getLeaderHeadInfo(getPersonalityType()).getNoWarAttitudeProb((GET_TEAM(getTeam()).AI_getAttitude(eWarTeam)));
						bValid = ((iNoWarOdds < 30) || (GC.getGame().getSorenRandNum(100, "AI War Vote Attitude Check (Force War)") > iNoWarOdds));
					}
					/*
					else
					{
						// Consider defying resolution
						if( !GET_TEAM(getTeam()).isAVassal() )
						{
							if( eSecretaryGeneral == NO_TEAM || GET_TEAM(getTeam()).AI_getAttitude(eWarTeam) > GET_TEAM(getTeam()).AI_getAttitude(eSecretaryGeneral) )
							{
								if( GET_TEAM(getTeam()).AI_getAttitude(eWarTeam) > GC.getLeaderHeadInfo(getPersonalityType()).getDefensivePactRefuseAttitudeThreshold() )
								{
									int iDefyRand = GC.getLeaderHeadInfo(getPersonalityType()).getBasePeaceWeight();
									iDefyRand /= (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? 2 : 1);

									if (GC.getGame().getSorenRandNum(iDefyRand, "AI Erratic Defiance (Force War)") > 0)
									{
										bDefy = true;
									}
								}
							}
						}
					}
					*/
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
				}
			}
			else if (GC.getVoteInfo(eVote).isAssignCity())
			{
				bValid = false;

				FAssert(kVoteData.ePlayer != NO_PLAYER);
				CvPlayer& kPlayer = GET_PLAYER(kVoteData.ePlayer);
				CvCity* pCity = kPlayer.getCity(kVoteData.iCityId);
				if (NULL != pCity)
				{
					if (NO_PLAYER != kVoteData.eOtherPlayer && kVoteData.eOtherPlayer != pCity->getOwnerINLINE())
					{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/03/09                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
						if ((!bPropose && eSecretaryGeneral == getTeam()) || GET_PLAYER(kVoteData.eOtherPlayer).getTeam() == getTeam())
						{
							bValid = true;
						}
						else if (kPlayer.getTeam() == getTeam())
						{
							bValid = false;
							// BBAI TODO: Wonders, holy city, aggressive AI?
							if (GC.getGame().getSorenRandNum(3, "AI Erratic Defiance (Assign City)") == 0)
							{
								bDefy = true;
							}
						}
						else
						{
							bValid = (AI_getAttitude(kVoteData.ePlayer) < AI_getAttitude(kVoteData.eOtherPlayer));
						}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
					}
				}
			}
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/30/08                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
/* original BTS code
		if (bDefy && canDefyResolution(eVoteSource, kVoteData))
*/
		// Don't defy resolutions from friends
		if( bDefy && !bFriendlyToSecretary && canDefyResolution(eVoteSource, kVoteData))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		{
			return PLAYER_VOTE_NEVER;
		}

		return (bValid ? PLAYER_VOTE_YES : PLAYER_VOTE_NO);
	}

}

int CvPlayerAI::AI_dealVal(PlayerTypes ePlayer, const CLinkList<TradeData>* pList, bool bIgnoreAnnual, int iChange) const
{
	CLLNode<TradeData>* pNode;
	CvCity* pCity;
	int iValue;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	iValue = 0;

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		iValue += GET_TEAM(getTeam()).AI_endWarVal(GET_PLAYER(ePlayer).getTeam());
	}

	for (pNode = pList->head(); pNode; pNode = pList->next(pNode))
	{
		FAssertMsg(!(pNode->m_data.m_bHidden), "(pNode->m_data.m_bHidden) did not return false as expected");

		switch (pNode->m_data.m_eItemType)
		{
		case TRADE_TECHNOLOGIES:
			iValue += GET_TEAM(getTeam()).AI_techTradeVal((TechTypes)(pNode->m_data.m_iData), GET_PLAYER(ePlayer).getTeam());
			break;
		case TRADE_RESOURCES:
			if (!bIgnoreAnnual)
			{
				iValue += AI_bonusTradeVal(((BonusTypes)(pNode->m_data.m_iData)), ePlayer, iChange);
			}
			break;
		case TRADE_CITIES:
			pCity = GET_PLAYER(ePlayer).getCity(pNode->m_data.m_iData);
			if (pCity != NULL)
			{
				iValue += AI_cityTradeVal(pCity);
			}
			break;
		case TRADE_GOLD:
			iValue += (pNode->m_data.m_iData * AI_goldTradeValuePercent()) / 100;
			break;
		case TRADE_GOLD_PER_TURN:
			if (!bIgnoreAnnual)
			{
				iValue += AI_goldPerTurnTradeVal(pNode->m_data.m_iData);
			}
			break;
		case TRADE_MAPS:
			iValue += GET_TEAM(getTeam()).AI_mapTradeVal(GET_PLAYER(ePlayer).getTeam());
			break;
		case TRADE_SURRENDER:
			if (!bIgnoreAnnual)
			{
				iValue += GET_TEAM(getTeam()).AI_surrenderTradeVal(GET_PLAYER(ePlayer).getTeam());
			}
			break;
		case TRADE_VASSAL:
			if (!bIgnoreAnnual)
			{
				iValue += GET_TEAM(getTeam()).AI_vassalTradeVal(GET_PLAYER(ePlayer).getTeam());
			}
			break;
		case TRADE_OPEN_BORDERS:
			iValue += GET_TEAM(getTeam()).AI_openBordersTradeVal(GET_PLAYER(ePlayer).getTeam());
			break;
		case TRADE_DEFENSIVE_PACT:
			iValue += GET_TEAM(getTeam()).AI_defensivePactTradeVal(GET_PLAYER(ePlayer).getTeam());
			break;
		case TRADE_PEACE:
			iValue += GET_TEAM(getTeam()).AI_makePeaceTradeVal(((TeamTypes)(pNode->m_data.m_iData)), GET_PLAYER(ePlayer).getTeam());
			break;
		case TRADE_WAR:
			iValue += GET_TEAM(getTeam()).AI_declareWarTradeVal(((TeamTypes)(pNode->m_data.m_iData)), GET_PLAYER(ePlayer).getTeam());
			break;
		case TRADE_EMBARGO:
			iValue += AI_stopTradingTradeVal(((TeamTypes)(pNode->m_data.m_iData)), ePlayer);
			break;
		case TRADE_CIVIC:
			iValue += AI_civicTradeVal(((CivicTypes)(pNode->m_data.m_iData)), ePlayer);
			break;
		case TRADE_RELIGION:
			iValue += AI_religionTradeVal(((ReligionTypes)(pNode->m_data.m_iData)), ePlayer);
			break;
/************************************************************************************************/
/* Afforess	                  Start		 06/16/10                                               */
/*                                                                                              */
/* Advanced Diplomacy                                                                           */
/************************************************************************************************/
        case TRADE_EMBASSY:
            iValue += GET_TEAM(getTeam()).AI_embassyTradeVal(GET_PLAYER(ePlayer).getTeam());
            break;
        case TRADE_CONTACT:
            iValue += GET_TEAM(getTeam()).AI_contactTradeVal((TeamTypes)(pNode->m_data.m_iData), GET_PLAYER(ePlayer).getTeam());
            break;
        case TRADE_CORPORATION:
            iValue += AI_corporationTradeVal((CorporationTypes)(pNode->m_data.m_iData), ePlayer);
            break;
        case TRADE_PLEDGE_VOTE:
            iValue += AI_pledgeVoteTradeVal(GC.getGameINLINE().getVoteTriggered(GC.getGameINLINE().getCurrentVoteID()), ((PlayerVoteTypes)(pNode->m_data.m_iData)), ePlayer);
            break;
		case TRADE_SECRETARY_GENERAL_VOTE:
			iValue += AI_secretaryGeneralTradeVal((VoteSourceTypes)(pNode->m_data.m_iData), ePlayer);
			break;
		case TRADE_RITE_OF_PASSAGE:
			iValue += GET_TEAM(getTeam()).AI_LimitedBordersTradeVal(GET_PLAYER(ePlayer).getTeam());
			break;
		case TRADE_FREE_TRADE_ZONE:
			iValue += GET_TEAM(getTeam()).AI_FreeTradeAgreementVal(GET_PLAYER(ePlayer).getTeam());
			break;
        case TRADE_WORKER:
			{
				CvUnit* pUnit = GET_PLAYER(ePlayer).getUnit(pNode->m_data.m_iData);
				if (pUnit != NULL)
				{
					iValue += AI_workerTradeVal(pUnit);
				}
			}
            break;
        case TRADE_MILITARY_UNIT:
			{
				CvUnit* pUnit = GET_PLAYER(ePlayer).getUnit(pNode->m_data.m_iData);
				if (pUnit != NULL)
				{
					iValue += AI_militaryUnitTradeVal(pUnit);
				}
			}
            break;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		}
	}

	return iValue;
}


bool CvPlayerAI::AI_goldDeal(const CLinkList<TradeData>* pList) const
{
	CLLNode<TradeData>* pNode;

	for (pNode = pList->head(); pNode; pNode = pList->next(pNode))
	{
		FAssert(!(pNode->m_data.m_bHidden));

		switch (pNode->m_data.m_eItemType)
		{
		case TRADE_GOLD:
		case TRADE_GOLD_PER_TURN:
			return true;
			break;
		}
	}

	return false;
}


/// \brief AI decision making on a proposal it is given
///
/// In this function the AI considers whether or not to accept another player's proposal.  This is used when
/// considering proposals from the human player made in the diplomacy window as well as a couple other places.
bool CvPlayerAI::AI_considerOffer(PlayerTypes ePlayer, const CLinkList<TradeData>* pTheirList, const CLinkList<TradeData>* pOurList, int iChange) const
{
	CLLNode<TradeData>* pNode;
	int iThreshold;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	if (AI_goldDeal(pTheirList) && AI_goldDeal(pOurList))
	{
		return false;
	}

	if (iChange > -1)
	{
		for (pNode = pOurList->head(); pNode; pNode = pOurList->next(pNode))
		{
			if (getTradeDenial(ePlayer, pNode->m_data) != NO_DENIAL)
			{
				return false;
			}
		}
	}

	if (GET_PLAYER(ePlayer).getTeam() == getTeam())
	{
		return true;
	}
/************************************************************************************************/
/* Afforess	                  Start		 04/02/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	for (pNode = pOurList->head(); pNode; pNode = pOurList->next(pNode))
	{
		if( pNode->m_data.m_eItemType == TRADE_CORPORATION )
		{
			if (pTheirList->getLength() == 0)
			{
				return false;
			}
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/23/09                                jdog5000      */
/*                                                                                              */
/* Diplomacy AI                                                                                 */
/************************************************************************************************/
	// Don't always accept giving deals, TRADE_VASSAL and TRADE_SURRENDER come with strings attached
	bool bVassalTrade = false;
	for (pNode = pTheirList->head(); pNode; pNode = pTheirList->next(pNode))
	{
		if( pNode->m_data.m_eItemType == TRADE_VASSAL )
		{
			bVassalTrade = true;

			for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++)
			{
				if (GET_TEAM((TeamTypes)iTeam).isAlive())
				{
					if (iTeam != getTeam() && iTeam != GET_PLAYER(ePlayer).getTeam() && atWar(GET_PLAYER(ePlayer).getTeam(), (TeamTypes)iTeam) && !atWar(getTeam(), (TeamTypes)iTeam))
					{
						if (GET_TEAM(getTeam()).AI_declareWarTrade((TeamTypes)iTeam, GET_PLAYER(ePlayer).getTeam(), false) != NO_DENIAL)
						{
							return false;
						}
					}
				}
			}
		}
		else if( pNode->m_data.m_eItemType == TRADE_SURRENDER )
		{
			bVassalTrade = true;

			if( !(GET_TEAM(getTeam()).AI_acceptSurrender(GET_PLAYER(ePlayer).getTeam())) )
			{
				return false;
			}
		}
	}
	
	if( !bVassalTrade )
	{
		if ((pOurList->getLength() == 0) && (pTheirList->getLength() > 0))
		{
			return true;
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	int iOurValue = GET_PLAYER(ePlayer).AI_dealVal(getID(), pOurList, false, iChange);
	int iTheirValue = AI_dealVal(ePlayer, pTheirList, false, iChange);

	if (iOurValue > 0 && 0 == pTheirList->getLength() && 0 == iTheirValue)
	{
		if (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()) && CvDeal::isVassalTributeDeal(pOurList))
		{
			if (AI_getAttitude(ePlayer, false) <= GC.getLeaderHeadInfo(getPersonalityType()).getVassalRefuseAttitudeThreshold()
				&& GET_TEAM(getTeam()).getAtWarCount(true) == 0
				&& GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getDefensivePactCount() == 0)
			{
				iOurValue *= (GET_TEAM(getTeam()).getPower(false) + 10);
				iOurValue /= (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getPower(false) + 10);
			}
			else
			{
				return true;
			}
		}
		else
		{
			if (AI_getAttitude(ePlayer) < ATTITUDE_PLEASED)
			{
				if (GET_TEAM(getTeam()).getPower(false) > ((GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getPower(false) * 4) / 3))
				{
					return false;
				}
			}

			if (AI_getMemoryCount(ePlayer, MEMORY_MADE_DEMAND_RECENT) > 0)
			{
				return false;
			}
		}

		iThreshold = (GET_TEAM(getTeam()).AI_getHasMetCounter(GET_PLAYER(ePlayer).getTeam()) + 50);

		iThreshold *= 2;

		if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_isLandTarget(getTeam()))
		{
			iThreshold *= 3;
		}

		iThreshold *= (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getPower(false) + 100);
		iThreshold /= (GET_TEAM(getTeam()).getPower(false) + 100);

		iThreshold -= GET_PLAYER(ePlayer).AI_getPeacetimeGrantValue(getID());

		return (iOurValue < iThreshold);
	}

	if (iChange < 0)
	{
		return (iTheirValue * 110 >= iOurValue * 100);
	}

	return (iTheirValue >= iOurValue);
}


bool CvPlayerAI::AI_counterPropose(PlayerTypes ePlayer, const CLinkList<TradeData>* pTheirList, const CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirInventory, CLinkList<TradeData>* pOurInventory, CLinkList<TradeData>* pTheirCounter, CLinkList<TradeData>* pOurCounter) const
{
	CLLNode<TradeData>* pNode;
	CLLNode<TradeData>* pBestNode;
	CLLNode<TradeData>* pGoldPerTurnNode;
	CLLNode<TradeData>* pGoldNode;
	bool* pabBonusDeal;
	CvCity* pCity;
	bool bTheirGoldDeal;
	bool bOurGoldDeal;
	int iHumanDealWeight;
	int iAIDealWeight;
	int iGoldData;
	int iGoldWeight;
	int iWeight;
	int iBestWeight;
	int iValue;
	int iBestValue;
	int iI;

	bTheirGoldDeal = AI_goldDeal(pTheirList);
	bOurGoldDeal = AI_goldDeal(pOurList);

	if (bOurGoldDeal && bTheirGoldDeal)
	{
		return false;
	}

	pabBonusDeal = new bool[GC.getNumBonusInfos()];

	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		pabBonusDeal[iI] = false;
	}

	pGoldPerTurnNode = NULL;
	pGoldNode = NULL;

	iHumanDealWeight = AI_dealVal(ePlayer, pTheirList);
	iAIDealWeight = GET_PLAYER(ePlayer).AI_dealVal(getID(), pOurList);

	int iGoldValuePercent = AI_goldTradeValuePercent();

	pTheirCounter->clear();
	pOurCounter->clear();

	if (iAIDealWeight > iHumanDealWeight)
	{
		if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
		{
			iBestValue = 0;
			iBestWeight = 0;
			pBestNode = NULL;

			for (pNode = pTheirInventory->head(); pNode && iAIDealWeight > iHumanDealWeight; pNode = pTheirInventory->next(pNode))
			{
				if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
				{
					if (pNode->m_data.m_eItemType == TRADE_CITIES)
					{
						FAssert(GET_PLAYER(ePlayer).canTradeItem(getID(), pNode->m_data));

						if (GET_PLAYER(ePlayer).getTradeDenial(getID(), pNode->m_data) == NO_DENIAL)
						{
							pCity = GET_PLAYER(ePlayer).getCity(pNode->m_data.m_iData);

							if (pCity != NULL)
							{
								iWeight = AI_cityTradeVal(pCity);

								if (iWeight > 0)
								{
									iValue = AI_targetCityValue(pCity, false);

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										iBestWeight = iWeight;
										pBestNode = pNode;
									}
								}
							}
						}
					}
				}
			}

			if (pBestNode != NULL)
			{
				iHumanDealWeight += iBestWeight;
				pTheirCounter->insertAtEnd(pBestNode->m_data);
			}
		}

		for (pNode = pTheirInventory->head(); pNode && iAIDealWeight > iHumanDealWeight; pNode = pTheirInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				FAssert(GET_PLAYER(ePlayer).canTradeItem(getID(), pNode->m_data));

				if (GET_PLAYER(ePlayer).getTradeDenial(getID(), pNode->m_data) == NO_DENIAL)
				{
					switch (pNode->m_data.m_eItemType)
					{
					case TRADE_GOLD:
						if (!bOurGoldDeal)
						{
							pGoldNode = pNode;
						}
						break;
					case TRADE_GOLD_PER_TURN:
						if (!bOurGoldDeal)
						{
							pGoldPerTurnNode = pNode;
						}
						break;
					}
				}
			}
		}

		int iGoldWeight = iAIDealWeight - iHumanDealWeight;

		if (iGoldWeight > 0)
		{
			if (pGoldNode)
			{
				iGoldData = iGoldWeight * 100;
				iGoldData /= iGoldValuePercent;
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       09/17/09                     dilandau & jdog5000      */
/*                                                                                              */
/* Bugfix				                                                                        */
/************************************************************************************************/
/* original bts code
				if ((iGoldData * iGoldValuePercent) < iGoldWeight)
*/
				if ((iGoldData * iGoldValuePercent) < iGoldWeight * 100)
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
				{
					iGoldData++;
				}
				if (GET_PLAYER(ePlayer).AI_maxGoldTrade(getID()) >= iGoldData)
				{
					pGoldNode->m_data.m_iData = iGoldData;
					iHumanDealWeight += (iGoldData * iGoldValuePercent) / 100;
					pTheirCounter->insertAtEnd(pGoldNode->m_data);
					pGoldNode = NULL;
				}
			}
		}

		for (pNode = pOurList->head(); pNode; pNode = pOurList->next(pNode))
		{
			FAssert(!(pNode->m_data.m_bHidden));

			switch (pNode->m_data.m_eItemType)
			{
			case TRADE_RESOURCES:
				pabBonusDeal[pNode->m_data.m_iData] = true;
				break;
			}
		}
		
		for (pNode = pTheirInventory->head(); pNode && iAIDealWeight > iHumanDealWeight; pNode = pTheirInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				FAssert(GET_PLAYER(ePlayer).canTradeItem(getID(), pNode->m_data));

				if (GET_PLAYER(ePlayer).getTradeDenial(getID(), pNode->m_data) == NO_DENIAL)
				{
					iWeight = 0;

					switch (pNode->m_data.m_eItemType)
					{
					case TRADE_TECHNOLOGIES:
						iWeight += GET_TEAM(getTeam()).AI_techTradeVal((TechTypes)(pNode->m_data.m_iData), GET_PLAYER(ePlayer).getTeam());
						break;
					case TRADE_RESOURCES:
						if (!pabBonusDeal[pNode->m_data.m_iData])
						{
							if (GET_PLAYER(ePlayer).getNumTradeableBonuses((BonusTypes)(pNode->m_data.m_iData)) > 1)
							{
								if (GET_PLAYER(ePlayer).AI_corporationBonusVal((BonusTypes)(pNode->m_data.m_iData)) == 0)
								{
									iWeight += AI_bonusTradeVal(((BonusTypes)(pNode->m_data.m_iData)), ePlayer, 1);
									pabBonusDeal[pNode->m_data.m_iData] = true;
								}
							}
						}
						break;
					}

					if (iWeight > 0)
					{
						iHumanDealWeight += iWeight;
						pTheirCounter->insertAtEnd(pNode->m_data);
					}
				}
			}
		}

		for (pNode = pTheirInventory->head(); pNode && iAIDealWeight > iHumanDealWeight; pNode = pTheirInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				if (pNode->m_data.m_eItemType == TRADE_MAPS)
				{
					FAssert(GET_PLAYER(ePlayer).canTradeItem(getID(), pNode->m_data));

					if (GET_PLAYER(ePlayer).getTradeDenial(getID(), pNode->m_data) == NO_DENIAL)
					{
						iWeight = GET_TEAM(getTeam()).AI_mapTradeVal(GET_PLAYER(ePlayer).getTeam());

						if (iWeight > 0)
						{
							iHumanDealWeight += iWeight;
							pTheirCounter->insertAtEnd(pNode->m_data);
						}
					}
				}
			}
		}

		iGoldWeight = iAIDealWeight - iHumanDealWeight;

		if (iGoldWeight > 0)
		{
			if (pGoldNode)
			{
				iGoldData = iGoldWeight * 100;
				iGoldData /= iGoldValuePercent;
				
				if ((iGoldWeight * 100) > (iGoldData * iGoldValuePercent))
				{
					iGoldData++;
				}

				iGoldData = std::min(iGoldData, GET_PLAYER(ePlayer).AI_maxGoldTrade(getID()));

				if (iGoldData > 0)
				{
					pGoldNode->m_data.m_iData = iGoldData;
					iHumanDealWeight += (iGoldData * iGoldValuePercent) / 100;
					pTheirCounter->insertAtEnd(pGoldNode->m_data);
					pGoldNode = NULL;
				}
			}
		}

		if (iAIDealWeight > iHumanDealWeight)
		{
			if (pGoldPerTurnNode)
			{
				iGoldData = 0;

				while (AI_goldPerTurnTradeVal(iGoldData) < (iAIDealWeight - iHumanDealWeight))
				{
					iGoldData++;
				}

				iGoldData = std::min(iGoldData, GET_PLAYER(ePlayer).AI_maxGoldPerTurnTrade(getID()));

				if (iGoldData > 0)
				{
					pGoldPerTurnNode->m_data.m_iData = iGoldData;
					iHumanDealWeight += AI_goldPerTurnTradeVal(pGoldPerTurnNode->m_data.m_iData);
					pTheirCounter->insertAtEnd(pGoldPerTurnNode->m_data);
					pGoldPerTurnNode = NULL;
				}
			}
		}

		for (pNode = pTheirInventory->head(); pNode && iAIDealWeight > iHumanDealWeight; pNode = pTheirInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				if (pNode->m_data.m_eItemType == TRADE_RESOURCES)
				{
					FAssert(GET_PLAYER(ePlayer).canTradeItem(getID(), pNode->m_data));

					if (GET_PLAYER(ePlayer).getTradeDenial(getID(), pNode->m_data) == NO_DENIAL)
					{
						iWeight = 0;

						if (!pabBonusDeal[pNode->m_data.m_iData])
						{
							if (GET_PLAYER(ePlayer).getNumTradeableBonuses((BonusTypes)(pNode->m_data.m_iData)) > 0)
							{
								iWeight += AI_bonusTradeVal(((BonusTypes)(pNode->m_data.m_iData)), ePlayer, 1);
								pabBonusDeal[pNode->m_data.m_iData] = true;
							}
						}

						if (iWeight > 0)
						{
							iHumanDealWeight += iWeight;
							pTheirCounter->insertAtEnd(pNode->m_data);
						}
					}
				}
			}
		}
	}
	else if (iHumanDealWeight > iAIDealWeight)
	{
		if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
		{
			bool bSurrender = false;
			for (pNode = pOurInventory->head(); pNode; pNode = pOurInventory->next(pNode))
			{
				if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
				{
					if (pNode->m_data.m_eItemType == TRADE_SURRENDER)
					{
						if (getTradeDenial(ePlayer, pNode->m_data) == NO_DENIAL)
						{
							iAIDealWeight += GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_surrenderTradeVal(getTeam());
							pOurCounter->insertAtEnd(pNode->m_data);
							bSurrender = true;
						}
						break;
					}
				}
			}

			if (!bSurrender)
			{
				for (pNode = pOurInventory->head(); pNode; pNode = pOurInventory->next(pNode))
				{
					if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
					{
						if (pNode->m_data.m_eItemType == TRADE_PEACE_TREATY)
						{
							pOurCounter->insertAtEnd(pNode->m_data);
							break;
						}
					}
				}
			}

			iBestValue = 0;
			iBestWeight = 0;
			pBestNode = NULL;

			for (pNode = pOurInventory->head(); pNode && iHumanDealWeight > iAIDealWeight; pNode = pOurInventory->next(pNode))
			{
				if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
				{
					if (pNode->m_data.m_eItemType == TRADE_CITIES)
					{
						FAssert(canTradeItem(ePlayer, pNode->m_data));

						if (getTradeDenial(ePlayer, pNode->m_data) == NO_DENIAL)
						{
							pCity = getCity(pNode->m_data.m_iData);

							if (pCity != NULL)
							{
								iWeight = GET_PLAYER(ePlayer).AI_cityTradeVal(pCity);

								if (iWeight > 0)
								{
									iValue = GET_PLAYER(ePlayer).AI_targetCityValue(pCity, false);

									if (iValue > iBestValue)
									{
										if (iHumanDealWeight >= (iAIDealWeight + iWeight))
										{
											iBestValue = iValue;
											iBestWeight = iWeight;
											pBestNode = pNode;
										}
									}
								}
							}
						}
					}
				}
			}

			if (pBestNode != NULL)
			{
				iAIDealWeight += iBestWeight;
				pOurCounter->insertAtEnd(pBestNode->m_data);
			}
		}

		for (pNode = pOurInventory->head(); pNode && iHumanDealWeight > iAIDealWeight; pNode = pOurInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				FAssert(canTradeItem(ePlayer, pNode->m_data));

				if (getTradeDenial(ePlayer, pNode->m_data) == NO_DENIAL)
				{
					switch (pNode->m_data.m_eItemType)
					{
					case TRADE_GOLD:
						if (!bTheirGoldDeal)
						{
							pGoldNode = pNode;
						}
						break;
					case TRADE_GOLD_PER_TURN:
						if (!bTheirGoldDeal)
						{
							pGoldPerTurnNode = pNode;
						}
						break;
					}
				}
			}
		}

		iGoldWeight = iHumanDealWeight - iAIDealWeight;

		if (iGoldWeight > 0)
		{
			if (pGoldNode)
			{
				int iGoldData = iGoldWeight * 100;
				iGoldData /= iGoldValuePercent;

				if (AI_maxGoldTrade(ePlayer) >= iGoldData)
				{
					pGoldNode->m_data.m_iData = iGoldData;
					iAIDealWeight += ((iGoldData * iGoldValuePercent) / 100);
					pOurCounter->insertAtEnd(pGoldNode->m_data);
					pGoldNode = NULL;
				}
			}
		}

		for (pNode = pTheirList->head(); pNode; pNode = pTheirList->next(pNode))
		{
			FAssert(!(pNode->m_data.m_bHidden));

			switch (pNode->m_data.m_eItemType)
			{
			case TRADE_RESOURCES:
				pabBonusDeal[pNode->m_data.m_iData] = true;
				break;
			}
		}

		for (pNode = pOurInventory->head(); pNode && iHumanDealWeight > iAIDealWeight; pNode = pOurInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				FAssert(canTradeItem(ePlayer, pNode->m_data));

				if (getTradeDenial(ePlayer, pNode->m_data) == NO_DENIAL)
				{
					iWeight = 0;

					switch (pNode->m_data.m_eItemType)
					{
					case TRADE_TECHNOLOGIES:
						iWeight += GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_techTradeVal((TechTypes)(pNode->m_data.m_iData), getTeam());
						break;
					case TRADE_RESOURCES:
						if (!pabBonusDeal[pNode->m_data.m_iData])
						{
							if (getNumTradeableBonuses((BonusTypes)(pNode->m_data.m_iData)) > 1)
							{
								iWeight += GET_PLAYER(ePlayer).AI_bonusTradeVal(((BonusTypes)(pNode->m_data.m_iData)), getID(), 1);
								pabBonusDeal[pNode->m_data.m_iData] = true;
							}
						}
						break;
					}

					if (iWeight > 0)
					{
						if (iHumanDealWeight >= (iAIDealWeight + iWeight))
						{
							iAIDealWeight += iWeight;
							pOurCounter->insertAtEnd(pNode->m_data);
						}
					}
				}
			}
		}

		for (pNode = pOurInventory->head(); pNode && iHumanDealWeight > iAIDealWeight; pNode = pOurInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				if (pNode->m_data.m_eItemType == TRADE_MAPS)
				{
					FAssert(canTradeItem(ePlayer, pNode->m_data));

					if (getTradeDenial(ePlayer, pNode->m_data) == NO_DENIAL)
					{
						iWeight = GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_mapTradeVal(getTeam());

						if (iWeight > 0)
						{
							if (iHumanDealWeight >= (iAIDealWeight + iWeight))
							{
								iAIDealWeight += iWeight;
								pOurCounter->insertAtEnd(pNode->m_data);
							}
						}
					}
				}
			}
		}
/************************************************************************************************/
/* Afforess	                  Start		 04/06/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		for (pNode = pOurInventory->head(); pNode && iHumanDealWeight > iAIDealWeight; pNode = pOurInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				if (pNode->m_data.m_eItemType == TRADE_CONTACT)
				{
					FAssert(canTradeItem(ePlayer, pNode->m_data));

					if (getTradeDenial(ePlayer, pNode->m_data) == NO_DENIAL)
					{
						if ((TeamTypes)pNode->m_data.m_iData != NO_TEAM)
						{
							iWeight = GET_TEAM(getTeam()).AI_contactTradeVal((TeamTypes)pNode->m_data.m_iData, GET_PLAYER(ePlayer).getTeam());
							if (iWeight > 0)
							{
								if (iHumanDealWeight >= (iAIDealWeight + iWeight))
								{
									iAIDealWeight += iWeight;
									pOurCounter->insertAtEnd(pNode->m_data);
								}
							}
						}
					}
				}
			}
		}
		for (pNode = pOurInventory->head(); pNode && iHumanDealWeight > iAIDealWeight; pNode = pOurInventory->next(pNode))
		{
			if (!pNode->m_data.m_bOffering && !pNode->m_data.m_bHidden)
			{
				if (pNode->m_data.m_eItemType == TRADE_MILITARY_UNIT)
				{
					FAssert(canTradeItem(ePlayer, pNode->m_data));

					if (getTradeDenial(ePlayer, pNode->m_data) == NO_DENIAL)
					{
						CvUnit* pUnit = getUnit(pNode->m_data.m_iData);
						if (pUnit != NULL)
						{
							iWeight = GET_PLAYER(ePlayer).AI_militaryUnitTradeVal(pUnit);
							if (iWeight > 0)
							{
								if (iHumanDealWeight >= (iAIDealWeight + iWeight))
								{
									iAIDealWeight += iWeight;
									pOurCounter->insertAtEnd(pNode->m_data);
								}
							}
						}
					}
				}
			}
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		iGoldWeight = iHumanDealWeight - iAIDealWeight;
		if (iGoldWeight > 0)
		{
			if (pGoldNode)
			{
				iGoldData = iGoldWeight * 100;
				iGoldData /= AI_goldTradeValuePercent();

				iGoldData = std::min(iGoldData, AI_maxGoldTrade(ePlayer));

				if (iGoldData > 0)
				{
					pGoldNode->m_data.m_iData = iGoldData;
					iAIDealWeight += (iGoldData * AI_goldTradeValuePercent()) / 100;
					pOurCounter->insertAtEnd(pGoldNode->m_data);
					pGoldNode = NULL;
				}
			}
		}

		if (iHumanDealWeight > iAIDealWeight)
		{
			if (pGoldPerTurnNode)
			{
				iGoldData = 0;

				while (GET_PLAYER(ePlayer).AI_goldPerTurnTradeVal(iGoldData + 1) <= (iHumanDealWeight - iAIDealWeight))
				{
					iGoldData++;
				}

				iGoldData = std::min(iGoldData, AI_maxGoldPerTurnTrade(ePlayer));

				if (iGoldData > 0)
				{
					pGoldPerTurnNode->m_data.m_iData = iGoldData;
					iAIDealWeight += GET_PLAYER(ePlayer).AI_goldPerTurnTradeVal(pGoldPerTurnNode->m_data.m_iData);
					pOurCounter->insertAtEnd(pGoldPerTurnNode->m_data);
					pGoldPerTurnNode = NULL;
				}
			}
		}
	}

	SAFE_DELETE_ARRAY(pabBonusDeal);

	return ((iAIDealWeight <= iHumanDealWeight) && ((pOurList->getLength() > 0) || (pOurCounter->getLength() > 0) || (pTheirCounter->getLength() > 0)));
}


int CvPlayerAI::AI_maxGoldTrade(PlayerTypes ePlayer) const
{
	int iMaxGold;
	int iResearchBuffer;

	FAssert(ePlayer != getID());

	if (isHuman() || (GET_PLAYER(ePlayer).getTeam() == getTeam()))
	{
		iMaxGold = getGold();
	}
	else
	{
		iMaxGold = getTotalPopulation();

		iMaxGold *= (GET_TEAM(getTeam()).AI_getHasMetCounter(GET_PLAYER(ePlayer).getTeam()) + 10);

		iMaxGold *= GC.getLeaderHeadInfo(getPersonalityType()).getMaxGoldTradePercent();
		iMaxGold /= 100;

		iMaxGold -= AI_getGoldTradedTo(ePlayer);

        iResearchBuffer = -calculateGoldRate() * 12;
        iResearchBuffer *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent();
        iResearchBuffer /= 100;
/************************************************************************************************/
/* Afforess	                  Start		 04/06/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		iMaxGold *= (100 + calculateInflationRate());
		iMaxGold /= 100;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		iMaxGold = std::min(iMaxGold, getGold() - iResearchBuffer);

		iMaxGold = std::min(iMaxGold, getGold());

		iMaxGold -= (iMaxGold % GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));
		
		
	}

	return std::max(0, iMaxGold);
}


int CvPlayerAI::AI_maxGoldPerTurnTrade(PlayerTypes ePlayer) const
{
	int iMaxGoldPerTurn;

	FAssert(ePlayer != getID());

	if (isHuman() || (GET_PLAYER(ePlayer).getTeam() == getTeam()))
	{
		iMaxGoldPerTurn = (calculateGoldRate() + (getGold() / GC.getDefineINT("PEACE_TREATY_LENGTH")));
	}
	else
	{
		iMaxGoldPerTurn = getTotalPopulation();

		iMaxGoldPerTurn *= GC.getLeaderHeadInfo(getPersonalityType()).getMaxGoldPerTurnTradePercent();
		iMaxGoldPerTurn /= 100;

		iMaxGoldPerTurn += std::min(0, getGoldPerTurnByPlayer(ePlayer));
	}

	return std::max(0, std::min(iMaxGoldPerTurn, calculateGoldRate()));
}


int CvPlayerAI::AI_goldPerTurnTradeVal(int iGoldPerTurn) const
{
	int iValue = iGoldPerTurn * GC.getDefineINT("PEACE_TREATY_LENGTH");
	iValue *= AI_goldTradeValuePercent();
	iValue /= 100;
	
	return iValue;
}

int CvPlayerAI::AI_bonusVal(BonusTypes eBonus, int iChange) const
{
	PROFILE_FUNC();
	
	int iValue = 0;
	int iBonusCount = getNumAvailableBonuses(eBonus);
	if ((iChange == 0) || ((iChange == 1) && (iBonusCount == 0)) || ((iChange == -1) && (iBonusCount == 1)))
	{
		//This is assuming the none-to-one or one-to-none case.
		iValue += AI_baseBonusVal(eBonus);
		iValue += AI_corporationBonusVal(eBonus);
	}
	else
	{
		//This is basically the marginal value of an additional instance of a bonus.
		iValue += AI_baseBonusVal(eBonus) / 5;
		iValue += AI_corporationBonusVal(eBonus);		
	}
	return iValue;
}

//Value sans corporation
int CvPlayerAI::AI_baseBonusVal(BonusTypes eBonus) const
{
	PROFILE_FUNC();

	//recalculate if not defined
	if(m_aiBonusValue[eBonus] == -1)
	{
		PROFILE("CvPlayerAI::AI_baseBonusVal::recalculate");

		UnitTypes eLoopUnit;
		BuildingTypes eLoopBuilding;
		int iDiff;
		int iValue = 0;
		int iTempValue;
		int iI, iJ;

		if (!GET_TEAM(getTeam()).isBonusObsolete(eBonus))
		{
			iValue += (GC.getBonusInfo(eBonus).getHappiness() * 100);
			iValue += (GC.getBonusInfo(eBonus).getHealth() * 100);

			CvTeam& kTeam = GET_TEAM(getTeam());

			CvCity* pCapital = getCapitalCity();
			int iCityCount = getNumCities();
			int iCoastalCityCount = countNumCoastalCities();
			
			// find the first coastal city
			CvCity* pCoastalCity = NULL;
			CvCity* pUnconnectedCoastalCity = NULL;
			if (iCoastalCityCount > 0)
			{
				int iLoop;
				for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
				{
					if (pLoopCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
					{
						if (pLoopCity->isConnectedToCapital(getID()))
						{
							pCoastalCity = pLoopCity;
							break;
						}
						else if (pUnconnectedCoastalCity == NULL)
						{
							pUnconnectedCoastalCity = pLoopCity;
						}
					}
				}
			}
			if (pCoastalCity == NULL && pUnconnectedCoastalCity != NULL)
			{
				pCoastalCity = pUnconnectedCoastalCity;
			}


			for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
			{
				eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

				if (eLoopUnit != NO_UNIT)
				{
					CvUnitInfo& kLoopUnit = GC.getUnitInfo(eLoopUnit);

					iTempValue = 0;

					if (kLoopUnit.getPrereqAndBonus() == eBonus)
					{
						iTempValue += 50;
					}

					for (iJ = 0; iJ < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); iJ++)
					{
						if (kLoopUnit.getPrereqOrBonuses(iJ) == eBonus)
						{
							iTempValue += 40;
						}
					}

					iTempValue += kLoopUnit.getBonusProductionModifier(eBonus) / 10;

					if (iTempValue > 0)
					{
						bool bIsWater = (kLoopUnit.getDomainType() == DOMAIN_SEA);
						
						// if non-limited water unit, weight by coastal cities
						if (bIsWater && !isLimitedUnitClass((UnitClassTypes)(kLoopUnit.getUnitClassType())))
						{
							iTempValue *= std::min(iCoastalCityCount * 2, iCityCount);	// double coastal cities, cap at total cities
							iTempValue /= std::max(1, iCityCount);
						}

						if (canTrain(eLoopUnit))
						{
							// is it a water unit and no coastal cities or our coastal city cannot build because its obsolete
							if ((bIsWater && (pCoastalCity == NULL || (pCoastalCity->allUpgradesAvailable(eLoopUnit) != NO_UNIT))) ||
								// or our capital cannot build because its obsolete (we can already build all its upgrades)
								(pCapital != NULL && pCapital->allUpgradesAvailable(eLoopUnit) != NO_UNIT))
							{
								// its worthless
								iTempValue = 2;
							}
							// otherwise, value units we could build if we had this bonus double
							else
							{
								iTempValue *= 2;
							}
						}

						if (kLoopUnit.getPrereqAndTech() != NO_TECH)
						{
							iDiff = abs(GC.getTechInfo((TechTypes)(kLoopUnit.getPrereqAndTech())).getEra() - getCurrentEra());

							if (iDiff == 0)
							{
								iTempValue *= 3;
								iTempValue /= 2;
							}
							else
							{
								iTempValue /= iDiff;
							}
						}

						iValue += iTempValue;
					}
				}
			}

			for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
			{
				eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));

				if (eLoopBuilding != NO_BUILDING)
				{
					CvBuildingInfo& kLoopBuilding = GC.getBuildingInfo(eLoopBuilding);
					
					iTempValue = 0;

					if (kLoopBuilding.getPrereqAndBonus() == eBonus)
					{
						iTempValue += 30;
					}

					for (iJ = 0; iJ < GC.getNUM_BUILDING_PREREQ_OR_BONUSES(); iJ++)
					{
						if (kLoopBuilding.getPrereqOrBonuses(iJ) == eBonus)
						{
							iTempValue += 20;
						}
					}

					iTempValue += kLoopBuilding.getBonusProductionModifier(eBonus) / 10;

					if (kLoopBuilding.getPowerBonus() == eBonus)
					{
						iTempValue += 60;
					}
					
					for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
					{
						iTempValue += kLoopBuilding.getBonusYieldModifier(eBonus, iJ) / 2;
						if (kLoopBuilding.getPowerBonus() == eBonus)
						{
							iTempValue += kLoopBuilding.getPowerYieldModifier(iJ);
						}
/************************************************************************************************/
/* Afforess	                  Start		 03/2/10                                               */
/*                                                                                              */
/*  Better AI Resources Evalution                                                               */
/************************************************************************************************/
						//Remember, these are all divided by 10 at the end...
						iValue += kLoopBuilding.getBonusYieldChanges(eBonus, iJ) * std::max(1, getBuildingClassCount((BuildingClassTypes)iI)) * 60;
						iValue += kLoopBuilding.getBonusYieldModifier(eBonus, iJ)  * std::max(1, getBuildingClassCount((BuildingClassTypes)iI)) * 10;
						iValue += kLoopBuilding.getBonusCommerceModifier(eBonus, iJ)  * std::max(1, getBuildingClassCount((BuildingClassTypes)iI)) * 10;
					}
					iValue += std::max(1, getBuildingClassCount((BuildingClassTypes)iI)) * kLoopBuilding.getBonusHappinessChanges(eBonus) * 120;
					iValue += std::max(1, getBuildingClassCount((BuildingClassTypes)iI)) * kLoopBuilding.getBonusHealthChanges(eBonus) * 80;
					iValue += std::max(1, getBuildingClassCount((BuildingClassTypes)iI)) * kLoopBuilding.getBonusDefenseChanges(eBonus) * 10;
					//Special Wonder Considerations...
					if (isLimitedWonderClass((BuildingClassTypes)iI) && canConstruct(eLoopBuilding, false, false, false))
					{
						iValue += kLoopBuilding.getBonusProductionModifier(eBonus) * 25;
					}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
					
					{
						// determine whether we have the tech for this building
						bool bHasTechForBuilding = true;
						if (!(kTeam.isHasTech((TechTypes)(kLoopBuilding.getPrereqAndTech()))))
						{
							bHasTechForBuilding = false;
						}
						for (int iPrereqIndex = 0; bHasTechForBuilding && iPrereqIndex < GC.getNUM_BUILDING_AND_TECH_PREREQS(); iPrereqIndex++)
						{
							if (kLoopBuilding.getPrereqAndTechs(iPrereqIndex) != NO_TECH)
							{
								if (!(kTeam.isHasTech((TechTypes)(kLoopBuilding.getPrereqAndTechs(iPrereqIndex)))))
								{
									bHasTechForBuilding = false;
								}
							}
						}
						
						bool bIsStateReligion = (((ReligionTypes) kLoopBuilding.getStateReligion()) != NO_RELIGION);

						//check if function call is cached
						bool bCanConstruct = canConstruct(eLoopBuilding, false, /*bTestVisible*/ true, /*bIgnoreCost*/ true);
						
						// bCanNeverBuild when true is accurate, it may be false in some cases where we will never be able to build 
						bool bCanNeverBuild = (bHasTechForBuilding && !bCanConstruct && !bIsStateReligion);

						// if we can never build this, it is worthless
						if (bCanNeverBuild)
						{
							iTempValue = 0;
						}
						// double value if we can build it right now
						else if (bCanConstruct)
						{
							iTempValue *= 2;
						}

						// if non-limited water building, weight by coastal cities
						if (kLoopBuilding.isWater() && !isLimitedWonderClass((BuildingClassTypes)(kLoopBuilding.getBuildingClassType())))
						{
							iTempValue *= iCoastalCityCount;
							iTempValue /= std::max(1, iCityCount/2);
						}

						if (kLoopBuilding.getPrereqAndTech() != NO_TECH)
						{
							iDiff = abs(GC.getTechInfo((TechTypes)(kLoopBuilding.getPrereqAndTech())).getEra() - getCurrentEra());

							if (iDiff == 0)
							{
								iTempValue *= 3;
								iTempValue /= 2;
							}
							else
							{
								iTempValue /= iDiff;
							}
						}

						iValue += iTempValue;
					}
				}
			}

			for (iI = 0; iI < GC.getNumProjectInfos(); iI++)
			{
				ProjectTypes eProject = (ProjectTypes) iI;
				CvProjectInfo& kLoopProject = GC.getProjectInfo(eProject);
				iTempValue = 0;

				iTempValue += kLoopProject.getBonusProductionModifier(eBonus) / 10;

				if (iTempValue > 0)
				{
					bool bMaxedOut = (GC.getGameINLINE().isProjectMaxedOut(eProject) || kTeam.isProjectMaxedOut(eProject));

					if (bMaxedOut)
					{
						// project worthless
						iTempValue = 0;
					}
					else if (canCreate(eProject))
					{
						iTempValue *= 2;
					}

					if (kLoopProject.getTechPrereq() != NO_TECH)
					{
						iDiff = abs(GC.getTechInfo((TechTypes)(kLoopProject.getTechPrereq())).getEra() - getCurrentEra());

						if (iDiff == 0)
						{
							iTempValue *= 3;
							iTempValue /= 2;
						}
						else
						{
							iTempValue /= iDiff;
						}
					}

					iValue += iTempValue;
				}
			}

			RouteTypes eBestRoute = getBestRoute();
			for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
			{
				RouteTypes eRoute = (RouteTypes)(GC.getBuildInfo((BuildTypes)iI).getRoute());

				if (eRoute != NO_ROUTE)
				{
					iTempValue = 0;
					if (GC.getRouteInfo(eRoute).getPrereqBonus() == eBonus)
					{
						iTempValue += 80;
					}
					for (iJ = 0; iJ < GC.getNUM_ROUTE_PREREQ_OR_BONUSES(); iJ++)
					{
						if (GC.getRouteInfo(eRoute).getPrereqOrBonus(iJ) == eBonus)
						{
							iTempValue += 40;
						}
					}
					if ((eBestRoute != NO_ROUTE) && (GC.getRouteInfo(getBestRoute()).getValue() <= GC.getRouteInfo(eRoute).getValue()))
					{
						iValue += iTempValue;
					}
					else
					{
						iValue += iTempValue / 2;
					}
				}
			}

/************************************************************************************************/
/* Afforess	                  Start		 01/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			//Resource scarcity. If there are only limited quantities of this resource, treasure it.
			int iTotalBonusCount = 0;
			for (int iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					if (GET_TEAM(getTeam()).isHasMet((GET_PLAYER((PlayerTypes)iI).getTeam())))
					{
						iTotalBonusCount += GET_PLAYER((PlayerTypes)iI).getNumAvailableBonuses(eBonus);
					}
				}
			}
			iTempValue = getNumAvailableBonuses(eBonus) * 300;
			iTempValue /= std::max(1, iTotalBonusCount);
			iValue += iTempValue;
			
			iValue += GC.getBonusInfo(eBonus).getAIObjective() * 10;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			//	int iCorporationValue = AI_corporationBonusVal(eBonus);
			//	iValue += iCorporationValue;
			//
			//	if (iCorporationValue <= 0 && getNumAvailableBonuses(eBonus) > 0)
			//	{
			//		iValue /= 3;
			//	}

			iValue /= 10;
		}


		//clamp value non-negative
		m_aiBonusValue[eBonus] = std::max(0, iValue);
	}

	return m_aiBonusValue[eBonus];
}

int CvPlayerAI::AI_corporationBonusVal(BonusTypes eBonus) const
{
	int iValue = 0;
	int iCityCount = getNumCities();
	iCityCount += iCityCount / 6 + 1;

	for (int iCorporation = 0; iCorporation < GC.getNumCorporationInfos(); ++iCorporation)
	{
		int iCorpCount = getHasCorporationCount((CorporationTypes)iCorporation);
		if (iCorpCount > 0)
		{
			int iNumCorpBonuses = 0;
			iCorpCount += getNumCities() / 6 + 1;
			CvCorporationInfo& kCorp = GC.getCorporationInfo((CorporationTypes)iCorporation);
			for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
			{
				if (eBonus == kCorp.getPrereqBonus(i))
				{
					iValue += (50 * kCorp.getYieldProduced(YIELD_FOOD) * iCorpCount) / iCityCount;
					iValue += (50 * kCorp.getYieldProduced(YIELD_PRODUCTION) * iCorpCount) / iCityCount;
					iValue += (30 * kCorp.getYieldProduced(YIELD_COMMERCE) * iCorpCount) / iCityCount;

					iValue += (30 * kCorp.getCommerceProduced(COMMERCE_GOLD) * iCorpCount) / iCityCount;
					iValue += (30 * kCorp.getCommerceProduced(COMMERCE_RESEARCH) * iCorpCount) / iCityCount;
					iValue += (12 * kCorp.getCommerceProduced(COMMERCE_CULTURE) * iCorpCount) / iCityCount;
					iValue += (20 * kCorp.getCommerceProduced(COMMERCE_ESPIONAGE) * iCorpCount) / iCityCount;
					
					//Disabled since you can't found/spread a corp unless there is already a bonus,
					//and that bonus will provide the entirity of the bonusProduced benefit.

					/*if (NO_BONUS != kCorp.getBonusProduced())
					{
						if (getNumAvailableBonuses((BonusTypes)kCorp.getBonusProduced()) == 0)
						{
							iBonusValue += (1000 * iCorpCount * AI_baseBonusVal((BonusTypes)kCorp.getBonusProduced())) / (10 * iCityCount);
					}
					}*/
				}
			}
		}
	}

	iValue /= 100;	//percent
	iValue /= 10;	//match AI_baseBonusVal

	return iValue;
}


int CvPlayerAI::AI_bonusTradeVal(BonusTypes eBonus, PlayerTypes ePlayer, int iChange) const
{
	int iValue;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	iValue = AI_bonusVal(eBonus, iChange);

	iValue *= ((std::min(getNumCities(), GET_PLAYER(ePlayer).getNumCities()) + 3) * 30);
	iValue /= 100;

	iValue *= std::max(0, (GC.getBonusInfo(eBonus).getAITradeModifier() + 100));
	iValue /= 100;

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()) && !GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isCapitulated())
	{
		iValue /= 2;
	}

	return (iValue * GC.getDefineINT("PEACE_TREATY_LENGTH"));
}


DenialTypes CvPlayerAI::AI_bonusTrade(BonusTypes eBonus, PlayerTypes ePlayer) const
{
	PROFILE_FUNC();

	AttitudeTypes eAttitude;
	bool bStrategic;
	int iI, iJ;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");

	if (isHuman() && GET_PLAYER(ePlayer).isHuman())
	{
		return NO_DENIAL;
	}

	if (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (GET_PLAYER(ePlayer).getTeam() == getTeam())
	{
		return NO_DENIAL;
	}

	if (GET_PLAYER(ePlayer).getNumAvailableBonuses(eBonus) > 0 && GET_PLAYER(ePlayer).AI_corporationBonusVal(eBonus) <= 0)
	{
		return (GET_PLAYER(ePlayer).isHuman() ? DENIAL_JOKING : DENIAL_NO_GAIN);
	}

	if (isHuman())
	{
		return NO_DENIAL;
	}

	if (GET_TEAM(getTeam()).AI_getWorstEnemy() == GET_PLAYER(ePlayer).getTeam())
	{
		return DENIAL_WORST_ENEMY;
	}

	if (AI_corporationBonusVal(eBonus) > 0)
	{
		return DENIAL_JOKING;
	}

	bStrategic = false;

	for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
	{
/************************************************************************************************/
/* Afforess	                  Start		 03/19/10                                               */
/*                                                                                              */
/*  Better AI: Strategic For Current Era                                                        */
/************************************************************************************************/
		if (GC.getUnitInfo((UnitTypes)iI).getPrereqAndTech() != NO_TECH)
		{
			if (GC.getTechInfo((TechTypes)GC.getUnitInfo((UnitTypes)iI).getPrereqAndTech()).getEra() < getCurrentEra())
			{
				continue;
			}
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		if (GC.getUnitInfo((UnitTypes) iI).getPrereqAndBonus() == eBonus)
		{
			bStrategic = true;
		}

		for (iJ = 0; iJ < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); iJ++)
		{
			if (GC.getUnitInfo((UnitTypes) iI).getPrereqOrBonuses(iJ) == eBonus)
			{
				bStrategic = true;
			}
		}
	}

	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
/************************************************************************************************/
/* Afforess	                  Start		 03/19/10                                               */
/*                                                                                              */
/*  Better AI: Strategic For Current Era                                                        */
/************************************************************************************************/
		if (GC.getBuildingInfo((BuildingTypes)iI).getPrereqAndTech() != NO_TECH)
		{
			if (GC.getTechInfo((TechTypes)GC.getBuildingInfo((BuildingTypes)iI).getPrereqAndTech()).getEra() < getCurrentEra())
			{
				continue;
			}
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		if (GC.getBuildingInfo((BuildingTypes) iI).getPrereqAndBonus() == eBonus)
		{
			bStrategic = true;
		}

		for (iJ = 0; iJ < GC.getNUM_BUILDING_PREREQ_OR_BONUSES(); iJ++)
		{
			if (GC.getBuildingInfo((BuildingTypes) iI).getPrereqOrBonuses(iJ) == eBonus)
			{
				bStrategic = true;
			}
		}
	}

	// XXX marble and stone???

	eAttitude = AI_getAttitude(ePlayer);

	if (bStrategic)
	{
/************************************************************************************************/
/* Afforess	                  Start		 03/19/10                                               */
/*                                                                                              */
/* Ruthless AI                                                                                  */
/************************************************************************************************/
//If we are planning war, don't sell our resources!
		if (GC.getGameINLINE().isOption(GAMEOPTION_RUTHLESS_AI))
		{
			if (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0)
			{
				return DENIAL_MYSTERY;
			}
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		if (eAttitude <= GC.getLeaderHeadInfo(getPersonalityType()).getStrategicBonusRefuseAttitudeThreshold())
		{
			return DENIAL_ATTITUDE;
		}
	}

	if (GC.getBonusInfo(eBonus).getHappiness() > 0)
	{
		if (eAttitude <= GC.getLeaderHeadInfo(getPersonalityType()).getHappinessBonusRefuseAttitudeThreshold())
		{
			return DENIAL_ATTITUDE;
		}
	}

	if (GC.getBonusInfo(eBonus).getHealth() > 0)
	{
		if (eAttitude <= GC.getLeaderHeadInfo(getPersonalityType()).getHealthBonusRefuseAttitudeThreshold())
		{
			return DENIAL_ATTITUDE;
		}
	}

	return NO_DENIAL;
}


int CvPlayerAI::AI_cityTradeVal(CvCity* pCity) const
{
	CvPlot* pLoopPlot;
	int iValue;
	int iI;

	FAssert(pCity->getOwnerINLINE() != getID());

	iValue = 300;

	iValue += (pCity->getPopulation() * 50);

	iValue += (pCity->getCultureLevel() * 200);

	iValue += (((((pCity->getPopulation() * 50) + GC.getGameINLINE().getElapsedGameTurns() + 100) * 4) * pCity->plot()->calculateCulturePercent(pCity->getOwnerINLINE())) / 100);

	for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
	{
		pLoopPlot = plotCity(pCity->getX_INLINE(), pCity->getY_INLINE(), iI);

		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
			{
				iValue += (AI_bonusVal(pLoopPlot->getBonusType(getTeam())) * 10);
			}
		}
	}

	if (!(pCity->isEverOwned(getID())))
	{
		iValue *= 3;
		iValue /= 2;
	}

	iValue -= (iValue % GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));

	if (isHuman())
	{
		return std::max(iValue, GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));
	}
	else
	{
		return iValue;
	}
}


DenialTypes CvPlayerAI::AI_cityTrade(CvCity* pCity, PlayerTypes ePlayer) const
{
	FAssert(pCity->getOwnerINLINE() == getID());

	if (pCity->getLiberationPlayer(false) == ePlayer)
	{
		return NO_DENIAL;
	}
	
	// gda start: allow to gift city to ai without culture/finance checks
	/*
	CvCity* pNearestCity;
	if (!(GET_PLAYER(ePlayer).isHuman()))
	{
		if (GET_PLAYER(ePlayer).getTeam() != getTeam())
		{
			if ((pCity->plot()->calculateCulturePercent(ePlayer) == 0) && !(pCity->isEverOwned(ePlayer)) && (GET_PLAYER(ePlayer).getNumCities() > 3))
			{
				if (GET_PLAYER(ePlayer).AI_isFinancialTrouble())
				{
					return DENIAL_UNKNOWN;
				}

				pNearestCity = GC.getMapINLINE().findCity(pCity->getX_INLINE(), pCity->getY_INLINE(), ePlayer, NO_TEAM, true, false, NO_TEAM, NO_DIRECTION, pCity);

				if ((pNearestCity == NULL) || (plotDistance(pCity->getX_INLINE(), pCity->getY_INLINE(), pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE()) > 9))
				{
					return DENIAL_UNKNOWN;
				}
			}
		}
	}
	*/
	// gda end

	if (isHuman())
	{
		return NO_DENIAL;
	}

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}
/************************************************************************************************/
/* Afforess	                  Start		 03/19/10                                               */
/*                                                                                              */
/* Revolution AI                                                                                */
/************************************************************************************************/
	if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_REVOLUTION))
	{
		if (pCity->getRevolutionIndex() > 3000)
		{
			return NO_DENIAL;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	if (GET_PLAYER(ePlayer).getTeam() != getTeam())
	{
		return DENIAL_NEVER;
	}
	
	// gda start: allow to gift city to ai without culture/finance checks
	/*
	if (pCity->calculateCulturePercent(getID()) > 50)
	{
		return DENIAL_TOO_MUCH;
	}
	*/
	//gda end
	return NO_DENIAL;
}


int CvPlayerAI::AI_stopTradingTradeVal(TeamTypes eTradeTeam, PlayerTypes ePlayer) const
{
	CvDeal* pLoopDeal;
	int iModifier;
	int iValue;
	int iLoop;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");
	FAssertMsg(GET_PLAYER(ePlayer).getTeam() != getTeam(), "shouldn't call this function on ourselves");
	FAssertMsg(eTradeTeam != getTeam(), "shouldn't call this function on ourselves");
	FAssertMsg(GET_TEAM(eTradeTeam).isAlive(), "GET_TEAM(eWarTeam).isAlive is expected to be true");
	FAssertMsg(!atWar(eTradeTeam, GET_PLAYER(ePlayer).getTeam()), "eTeam should be at peace with eWarTeam");

	iValue = (50 + (GC.getGameINLINE().getGameTurn() / 2));
	iValue += (GET_TEAM(eTradeTeam).getNumCities() * 5);

	iModifier = 0;

	switch (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_getAttitude(eTradeTeam))
	{
	case ATTITUDE_FURIOUS:
		break;

	case ATTITUDE_ANNOYED:
		iModifier += 25;
		break;

	case ATTITUDE_CAUTIOUS:
		iModifier += 50;
		break;

	case ATTITUDE_PLEASED:
		iModifier += 100;
		break;

	case ATTITUDE_FRIENDLY:
		iModifier += 200;
		break;

	default:
		FAssert(false);
		break;
	}

	iValue *= std::max(0, (iModifier + 100));
	iValue /= 100;

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isOpenBorders(eTradeTeam))
	{
		iValue *= 2;
	}

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isDefensivePact(eTradeTeam))
	{
		iValue *= 3;
	}

	for(pLoopDeal = GC.getGameINLINE().firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = GC.getGameINLINE().nextDeal(&iLoop))
	{
		if (pLoopDeal->isCancelable(getID()) && !(pLoopDeal->isPeaceDeal()))
		{
			if (GET_PLAYER(pLoopDeal->getFirstPlayer()).getTeam() == GET_PLAYER(ePlayer).getTeam())
			{
				if (pLoopDeal->getLengthSecondTrades() > 0)
				{
					iValue += (GET_PLAYER(pLoopDeal->getFirstPlayer()).AI_dealVal(pLoopDeal->getSecondPlayer(), pLoopDeal->getSecondTrades()) * ((pLoopDeal->getLengthFirstTrades() == 0) ? 2 : 1));
				}
			}

			if (GET_PLAYER(pLoopDeal->getSecondPlayer()).getTeam() == GET_PLAYER(ePlayer).getTeam())
			{
				if (pLoopDeal->getLengthFirstTrades() > 0)
				{
					iValue += (GET_PLAYER(pLoopDeal->getSecondPlayer()).AI_dealVal(pLoopDeal->getFirstPlayer(), pLoopDeal->getFirstTrades()) * ((pLoopDeal->getLengthSecondTrades() == 0) ? 2 : 1));
				}
			}
		}
	}

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()))
	{
		iValue /= 2;
	}

	iValue -= (iValue % GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));

	if (isHuman())
	{
		return std::max(iValue, GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));
	}
	else
	{
		return iValue;
	}
}


DenialTypes CvPlayerAI::AI_stopTradingTrade(TeamTypes eTradeTeam, PlayerTypes ePlayer) const
{
	AttitudeTypes eAttitude;
	AttitudeTypes eAttitudeThem;
	int iI;

	FAssertMsg(ePlayer != getID(), "shouldn't call this function on ourselves");
	FAssertMsg(GET_PLAYER(ePlayer).getTeam() != getTeam(), "shouldn't call this function on ourselves");
	FAssertMsg(eTradeTeam != getTeam(), "shouldn't call this function on ourselves");
	FAssertMsg(GET_TEAM(eTradeTeam).isAlive(), "GET_TEAM(eTradeTeam).isAlive is expected to be true");
	FAssertMsg(!atWar(getTeam(), eTradeTeam), "should be at peace with eTradeTeam");

	if (isHuman())
	{
		return NO_DENIAL;
	}

	if (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (GET_TEAM(getTeam()).isVassal(eTradeTeam))
	{
		return DENIAL_POWER_THEM;
	}

	eAttitude = GET_TEAM(getTeam()).AI_getAttitude(GET_PLAYER(ePlayer).getTeam());

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getTeam())
			{
				if (eAttitude <= GC.getLeaderHeadInfo(GET_PLAYER((PlayerTypes)iI).getPersonalityType()).getStopTradingRefuseAttitudeThreshold())
				{
					return DENIAL_ATTITUDE;
				}
			}
		}
	}

	eAttitudeThem = GET_TEAM(getTeam()).AI_getAttitude(eTradeTeam);

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == getTeam())
			{
				if (eAttitudeThem > GC.getLeaderHeadInfo(GET_PLAYER((PlayerTypes)iI).getPersonalityType()).getStopTradingThemRefuseAttitudeThreshold())
				{
					return DENIAL_ATTITUDE_THEM;
				}
			}
		}
	}
/************************************************************************************************/
/* Afforess	                  Start		 03/19/10                                               */
/*                                                                                              */
/* Ruthless AI: Don't cancel open borders, we may need those in war                             */
/************************************************************************************************/
	if (GC.getGameINLINE().isOption(GAMEOPTION_RUTHLESS_AI))
	{
		if (GET_TEAM(getTeam()).isOpenBorders(eTradeTeam))
		{
			if (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0)
			{
				return DENIAL_MYSTERY;
			}
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return NO_DENIAL;
}


int CvPlayerAI::AI_civicTradeVal(CivicTypes eCivic, PlayerTypes ePlayer) const
{
	CivicTypes eBestCivic;
	int iValue;

	iValue = (2 * (getTotalPopulation() + GET_PLAYER(ePlayer).getTotalPopulation())); // XXX

	eBestCivic = GET_PLAYER(ePlayer).AI_bestCivic((CivicOptionTypes)(GC.getCivicInfo(eCivic).getCivicOptionType()));

	if (eBestCivic != NO_CIVIC)
	{
		if (eBestCivic != eCivic)
		{
			iValue += std::max(0, ((GET_PLAYER(ePlayer).AI_civicValue(eBestCivic) - GET_PLAYER(ePlayer).AI_civicValue(eCivic)) * 2));
		}
	}

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()))
	{
		iValue /= 2;
	}

	iValue -= (iValue % GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));

	if (isHuman())
	{
		return std::max(iValue, GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));
	}
	else
	{
		return iValue;
	}
}


DenialTypes CvPlayerAI::AI_civicTrade(CivicTypes eCivic, PlayerTypes ePlayer) const
{
/************************************************************************************************/
/* Afforess	                  Start		 04/05/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (GC.getGameINLINE().isOption(GAMEOPTION_ADVANCED_DIPLOMACY))
	{
		if (GET_TEAM(getTeam()).isAtWar(GET_PLAYER(ePlayer).getTeam()))
		{
			return NO_DENIAL;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	if (isHuman())
	{
		return NO_DENIAL;
	}

	if (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (GET_PLAYER(ePlayer).getTeam() == getTeam())
	{
		return NO_DENIAL;
	}

	if (getCivicPercentAnger(getCivics((CivicOptionTypes)(GC.getCivicInfo(eCivic).getCivicOptionType())),true) > getCivicPercentAnger(eCivic))
	{
		return DENIAL_ANGER_CIVIC;
	}

	CivicTypes eFavoriteCivic = (CivicTypes)GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic();
	if (eFavoriteCivic != NO_CIVIC)
	{
		if (isCivic(eFavoriteCivic) && (GC.getCivicInfo(eCivic).getCivicOptionType() == GC.getCivicInfo(eFavoriteCivic).getCivicOptionType()))
		{
			return DENIAL_FAVORITE_CIVIC;
		}
	}

	if (GC.getCivilizationInfo(getCivilizationType()).getCivilizationInitialCivics(GC.getCivicInfo(eCivic).getCivicOptionType()) == eCivic)
	{
		return DENIAL_JOKING;
	}

	if (AI_getAttitude(ePlayer) <= GC.getLeaderHeadInfo(getPersonalityType()).getAdoptCivicRefuseAttitudeThreshold())
	{
		return DENIAL_ATTITUDE;
	}
/************************************************************************************************/
/* Afforess	                  Start		 03/19/10                                               */
/*                                                                                              */
/* Ruthless AI: Don't change civics when planning war                                           */
/************************************************************************************************/
	if (GC.getGameINLINE().isOption(GAMEOPTION_RUTHLESS_AI))
	{
		if (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0)
		{
			return DENIAL_JOKING;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return NO_DENIAL;
}


int CvPlayerAI::AI_religionTradeVal(ReligionTypes eReligion, PlayerTypes ePlayer) const
{
	ReligionTypes eBestReligion;
	int iValue;

	iValue = (3 * (getTotalPopulation() + GET_PLAYER(ePlayer).getTotalPopulation())); // XXX

	eBestReligion = GET_PLAYER(ePlayer).AI_bestReligion();

	if (eBestReligion != NO_RELIGION)
	{
		if (eBestReligion != eReligion)
		{
			iValue += std::max(0, (GET_PLAYER(ePlayer).AI_religionValue(eBestReligion) - GET_PLAYER(ePlayer).AI_religionValue(eReligion)));
		}
	}

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isVassal(getTeam()))
	{
		iValue /= 2;
	}

	iValue -= (iValue % GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));

	if (isHuman())
	{
		return std::max(iValue, GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));
	}
	else
	{
		return iValue;
	}
}


DenialTypes CvPlayerAI::AI_religionTrade(ReligionTypes eReligion, PlayerTypes ePlayer) const
{
	if (isHuman())
	{
		return NO_DENIAL;
	}

	if (GET_TEAM(getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (atWar(getTeam(), GET_PLAYER(ePlayer).getTeam()))
	{
		return NO_DENIAL;
	}

	if (GET_PLAYER(ePlayer).getTeam() == getTeam())
	{
		return NO_DENIAL;
	}

	if (getStateReligion() != NO_RELIGION)
	{
		if (getHasReligionCount(eReligion) < std::min((getHasReligionCount(getStateReligion()) - 1), (getNumCities() / 2)))
		{
			return DENIAL_MINORITY_RELIGION;
		}
	}

	if (AI_getAttitude(ePlayer) <= GC.getLeaderHeadInfo(getPersonalityType()).getConvertReligionRefuseAttitudeThreshold())
	{
		return DENIAL_ATTITUDE;
	}
/************************************************************************************************/
/* Afforess	                  Start		 03/19/10                                               */
/*                                                                                              */
/* Ruthless AI: Don't Change Religions When we are planning war (Anarchy is bad)                */
/************************************************************************************************/
	if (GC.getGameINLINE().isOption(GAMEOPTION_RUTHLESS_AI))
	{
		if (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0)
		{
			return DENIAL_NO_GAIN;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return NO_DENIAL;
}


int CvPlayerAI::AI_unitImpassableCount(UnitTypes eUnit) const
{
	int iCount = 0;
	for (int iI = 0; iI < GC.getNumTerrainInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getTerrainImpassable(iI))
		{
			TechTypes eTech = (TechTypes)GC.getUnitInfo(eUnit).getTerrainPassableTech(iI);
			if (NO_TECH == eTech || !GET_TEAM(getTeam()).isHasTech(eTech))
			{
				iCount++;
			}
		}
	}

	for (int iI = 0; iI < GC.getNumFeatureInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getFeatureImpassable(iI))
		{
			TechTypes eTech = (TechTypes)GC.getUnitInfo(eUnit).getFeaturePassableTech(iI);
			if (NO_TECH == eTech || !GET_TEAM(getTeam()).isHasTech(eTech))
			{
				iCount++;
			}
		}
	}

	return iCount;
}


int CvPlayerAI::AI_unitValue(UnitTypes eUnit, UnitAITypes eUnitAI, CvArea* pArea) const
{
	PROFILE_FUNC();

	bool bValid;
	int iNeededMissionaries;
	int iCombatValue;
	int iValue;
	int iTempValue;
	int iI;

	FAssertMsg(eUnit != NO_UNIT, "Unit is not assigned a valid value");
	FAssertMsg(eUnitAI != NO_UNITAI, "UnitAI is not assigned a valid value");

	if (GC.getUnitInfo(eUnit).getDomainType() != AI_unitAIDomainType(eUnitAI))
	{
		if (eUnitAI != UNITAI_ICBM)//XXX
		{
			return 0;
		}
	}

	if (GC.getUnitInfo(eUnit).getNotUnitAIType(eUnitAI))
	{
		return 0;
	}

	bValid = GC.getUnitInfo(eUnit).getUnitAIType(eUnitAI);

	if (!bValid)
	{
		switch (eUnitAI)
		{
		case UNITAI_UNKNOWN:
			break;

		case UNITAI_ANIMAL:
			if (GC.getUnitInfo(eUnit).isAnimal())
			{
				bValid = true;
			}
			break;

		case UNITAI_SETTLE:
			if (GC.getUnitInfo(eUnit).isFound())
			{
				bValid = true;
			}
			break;

		case UNITAI_WORKER:
			for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
			{
				if (GC.getUnitInfo(eUnit).getBuilds(iI))
				{
					bValid = true;
					break;
				}
			}
			break;

		case UNITAI_ATTACK:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isOnlyDefensive()))
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_ATTACK_CITY:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isOnlyDefensive()))
				{
					if (!(GC.getUnitInfo(eUnit).isNoCapture()))
					{
						bValid = true;
					}
				}
			}
			break;

		case UNITAI_COLLATERAL:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isOnlyDefensive()))
				{
					if (GC.getUnitInfo(eUnit).getCollateralDamage() > 0)
					{
						bValid = true;
					}
				}
			}
			break;

		case UNITAI_PILLAGE:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isOnlyDefensive()))
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_RESERVE:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isOnlyDefensive()))
				{
						bValid = true;
					}
				}
			break;

		case UNITAI_COUNTER:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isOnlyDefensive()))
				{
					if (GC.getUnitInfo(eUnit).getInterceptionProbability() > 0)
					{
						bValid = true;
					}

					if (!bValid)
					{
						for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
						{
							if (GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI) > 0)
							{
								bValid = true;
								break;
							}

							if (GC.getUnitInfo(eUnit).getTargetUnitClass(iI))
							{
								bValid = true;
								break;
							}
						}
					}

					if (!bValid)
					{
						for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
						{
							if (GC.getUnitInfo(eUnit).getUnitCombatModifier(iI) > 0)
							{
								bValid = true;
								break;
							}

							if (GC.getUnitInfo(eUnit).getTargetUnitCombat(iI))
							{
								bValid = true;
								break;
							}
						}
					}

					if (!bValid)
					{

						for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
						{
							int iUnitClass = GC.getUnitInfo(eUnit).getUnitClassType();
							if (NO_UNITCLASS != iUnitClass && GC.getUnitInfo((UnitTypes)iI).getDefenderUnitClass(iUnitClass))
							{
								bValid = true;
								break;
							}

							int iUnitCombat = GC.getUnitInfo(eUnit).getUnitCombatType();
							if (NO_UNITCOMBAT !=  iUnitCombat && GC.getUnitInfo((UnitTypes)iI).getDefenderUnitCombat(iUnitCombat))
							{
								bValid = true;
								break;
							}
						}
					}
				}
			}
			break;

		case UNITAI_CITY_DEFENSE:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isNoDefensiveBonus()))
				{
					if (GC.getUnitInfo(eUnit).getCityDefenseModifier() > 0)
					{
						bValid = true;
					}
				}
			}
			break;

		case UNITAI_CITY_COUNTER:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (!(GC.getUnitInfo(eUnit).isNoDefensiveBonus()))
				{
					if (GC.getUnitInfo(eUnit).getInterceptionProbability() > 0)
					{
						bValid = true;
					}

					if (!bValid)
					{
						for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
						{
							if (GC.getUnitInfo(eUnit).getUnitClassDefenseModifier(iI) > 0)
							{
								bValid = true;
								break;
							}
						}
					}

					if (!bValid)
					{
						for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
						{
							if (GC.getUnitInfo(eUnit).getUnitCombatModifier(iI) > 0)
							{
								bValid = true;
								break;
							}
						}
					}
				}
			}
			break;

		case UNITAI_CITY_SPECIAL:
			break;

		case UNITAI_PARADROP:
			if (GC.getUnitInfo(eUnit).getDropRange() > 0)
			{
				bValid = true;
			}
			break;

		case UNITAI_EXPLORE:
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/09/09                                jdog5000      */
/*                                                                                              */
/* Unit AI                                                                                      */
/************************************************************************************************/
			if (GC.getUnitInfo(eUnit).getCombat() > 0 && !(GC.getUnitInfo(eUnit).isNoRevealMap()))
			{
				if (0 == AI_unitImpassableCount(eUnit))
				{
					bValid = true;
				}
			}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
			break;

		case UNITAI_MISSIONARY:
			if (pArea != NULL)
			{
				for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
				{
					if (GC.getUnitInfo(eUnit).getReligionSpreads((ReligionTypes)iI) > 0)
					{
						iNeededMissionaries = AI_neededMissionaries(pArea, ((ReligionTypes)iI));

						if (iNeededMissionaries > 0)
						{
							if (iNeededMissionaries > countReligionSpreadUnits(pArea, ((ReligionTypes)iI)))
							{
								bValid = true;
								break;
							}
						}
					}
				}

				for (iI = 0; iI < GC.getNumCorporationInfos(); iI++)
				{
					if (GC.getUnitInfo(eUnit).getCorporationSpreads((CorporationTypes)iI) > 0)
					{
						iNeededMissionaries = AI_neededExecutives(pArea, ((CorporationTypes)iI));

						if (iNeededMissionaries > 0)
						{
							if (iNeededMissionaries > countCorporationSpreadUnits(pArea, ((CorporationTypes)iI)))
							{
								bValid = true;
								break;
							}
						}
					}
				}
			}
			break;

		case UNITAI_PROPHET:
		case UNITAI_ARTIST:
		case UNITAI_SCIENTIST:
		case UNITAI_GENERAL:
		case UNITAI_MERCHANT:
		case UNITAI_ENGINEER:
		case UNITAI_SPY:
			break;

		case UNITAI_ICBM:
			if (GC.getUnitInfo(eUnit).getNukeRange() != -1)
			{
				bValid = true;
			}
			break;

		case UNITAI_WORKER_SEA:
			for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
			{
				if (GC.getUnitInfo(eUnit).getBuilds(iI))
				{
					bValid = true;
					break;
				}
			}
			break;

		case UNITAI_ATTACK_SEA:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (GC.getUnitInfo(eUnit).getCargoSpace() == 0)
				{
					if (!(GC.getUnitInfo(eUnit).isInvisible()) && (GC.getUnitInfo(eUnit).getInvisibleType() == NO_INVISIBLE))
					{
						bValid = true;
					}
				}
			}
			break;

		case UNITAI_RESERVE_SEA:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (GC.getUnitInfo(eUnit).getCargoSpace() == 0)
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_ESCORT_SEA:
			if (GC.getUnitInfo(eUnit).getCombat() > 0)
			{
				if (GC.getUnitInfo(eUnit).getCargoSpace() == 0)
				{
					if (0 == AI_unitImpassableCount(eUnit))
					{
						bValid = true;
					}
				}
			}
			break;

		case UNITAI_EXPLORE_SEA:
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/09/09                                jdog5000      */
/*                                                                                              */
/* Unit AI                                                                                      */
/************************************************************************************************/
			if (GC.getUnitInfo(eUnit).getCargoSpace() <= 1 && !(GC.getUnitInfo(eUnit).isNoRevealMap()))
			{
				bValid = true;
			}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
			break;

		case UNITAI_ASSAULT_SEA:
		case UNITAI_SETTLER_SEA:
			if (GC.getUnitInfo(eUnit).getCargoSpace() > 0)
			{
				if (GC.getUnitInfo(eUnit).getSpecialCargo() == NO_SPECIALUNIT)
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_MISSIONARY_SEA:
		case UNITAI_SPY_SEA:
		case UNITAI_CARRIER_SEA:
		case UNITAI_MISSILE_CARRIER_SEA:
			if (GC.getUnitInfo(eUnit).getCargoSpace() > 0)
			{
				if (GC.getUnitInfo(eUnit).getSpecialCargo() != NO_SPECIALUNIT)
				{
					for (int i = 0; i < NUM_UNITAI_TYPES; ++i)
					{
						if (GC.getSpecialUnitInfo((SpecialUnitTypes)GC.getUnitInfo(eUnit).getSpecialCargo()).isCarrierUnitAIType(eUnitAI))
						{
							bValid = true;
							break;
						}
					}
				}
			}
			break;

		case UNITAI_PIRATE_SEA:
			if (GC.getUnitInfo(eUnit).isAlwaysHostile() && GC.getUnitInfo(eUnit).isHiddenNationality())
			{
				bValid = true;
			}
			break;

		case UNITAI_ATTACK_AIR:
			if (GC.getUnitInfo(eUnit).getAirCombat() > 0)
			{
				if (!GC.getUnitInfo(eUnit).isSuicide())
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_DEFENSE_AIR:
			if (GC.getUnitInfo(eUnit).getInterceptionProbability() > 0)
			{
				bValid = true;
			}
			break;

		case UNITAI_CARRIER_AIR:
			if (GC.getUnitInfo(eUnit).getAirCombat() > 0)
			{
				if (GC.getUnitInfo(eUnit).getInterceptionProbability() > 0)
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_MISSILE_AIR:
			if (GC.getUnitInfo(eUnit).getAirCombat() > 0)
			{
				if (GC.getUnitInfo(eUnit).isSuicide())
				{
					bValid = true;
				}
			}
			break;

		case UNITAI_ATTACK_CITY_LEMMING:
			bValid = false;
			break;

		default:
			FAssert(false);
			break;
		}
	}

	if (!bValid)
	{
		return 0;
	}

	iCombatValue = GC.getGameINLINE().AI_combatValue(eUnit);

	iValue = 1;

	iValue += GC.getUnitInfo(eUnit).getAIWeight();
	
	int iFastMoverMultiplier;

	switch (eUnitAI)
	{
	case UNITAI_UNKNOWN:
	case UNITAI_ANIMAL:
		break;

	case UNITAI_SETTLE:
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 100);
		break;

	case UNITAI_WORKER:
		for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
		{
			if (GC.getUnitInfo(eUnit).getBuilds(iI))
			{
				iValue += 50;
			}
		}
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 100);
		break;

	case UNITAI_ATTACK:
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      06/12/09                                jdog5000      */
/*                                                                                              */
/* Unit AI                                                                                      */
/************************************************************************************************/
		iFastMoverMultiplier = AI_isDoStrategy(AI_STRATEGY_FASTMOVERS) ? 3 : 1;
		
		iValue += iCombatValue;
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getMoves() * iFastMoverMultiplier) / 3);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getWithdrawalProbability()) / 100);
		if (GC.getUnitInfo(eUnit).getCombatLimit() < 100)
		{
			iValue -= (iCombatValue * (125 - GC.getUnitInfo(eUnit).getCombatLimit())) / 100;
		}
		
		break;

	case UNITAI_ATTACK_CITY:
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/24/10                                jdog5000      */
/*                                                                                              */
/* War strategy AI                                                                              */
/************************************************************************************************/
		// Effect army composition to have more collateral/bombard units
		iFastMoverMultiplier = AI_isDoStrategy(AI_STRATEGY_FASTMOVERS) ? 4 : 1;
		
		iTempValue = ((iCombatValue * iCombatValue) / 75) + (iCombatValue / 2);
		iValue += iTempValue;
		if (GC.getUnitInfo(eUnit).isNoDefensiveBonus())
		{
			iValue -= iTempValue / 2;
		}
		if (GC.getUnitInfo(eUnit).getDropRange() > 0)
		{
			iValue -= iTempValue / 2;
		}
		if (GC.getUnitInfo(eUnit).isFirstStrikeImmune())
		{
			iValue += (iTempValue * 8) / 100;
		}		
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getCityAttackModifier()) / 75);
/* Collateral Damage valuation moved to bombard part
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getCollateralDamage()) / 400);
*/
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getMoves() * iFastMoverMultiplier) / 4);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getWithdrawalProbability()) / 100);
/*
		if (!AI_isDoStrategy(AI_STRATEGY_AIR_BLITZ))
		{
*/
			if (GC.getUnitInfo(eUnit).getBombardRate() > 0 || (GC.getUnitInfo(eUnit).getCollateralDamageMaxUnits() > 0 && GC.getUnitInfo(eUnit).getCollateralDamage() > 0))
			{
				// Army composition needs to scale with army size, bombard unit potency

				//modified AI_calculateTotalBombard(DOMAIN_LAND) code
				int iII;
				int iTotalBombard = 0;
				int iThisBombard = GC.getUnitInfo(eUnit).getBombardRate();
				int iSiegeUnits = 0;
				int iSiegeImmune = 0;
				int iTotalSiegeMaxUnits = 0;
				bool bNoBombardValue = false;
				
				for (iII = 0; iII < GC.getNumUnitClassInfos(); iII++)
				{
					UnitTypes eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iII)));
					if (eLoopUnit != NO_UNIT)
					{
						if (GC.getUnitInfo(eLoopUnit).getDomainType() == DOMAIN_LAND)
						{
							int iClassCount = getUnitClassCount((UnitClassTypes)iII);
							int iBombardRate = GC.getUnitInfo(eLoopUnit).getBombardRate();
							
							if (iBombardRate > 0)
							{
								iTotalBombard += ((iBombardRate * iClassCount * ((GC.getUnitInfo(eLoopUnit).isIgnoreBuildingDefense()) ? 3 : 2)) / 2);
							}
							
							int iBombRate = GC.getUnitInfo(eLoopUnit).getBombRate();
							if (iBombRate > 0)
							{
								iTotalBombard += iBombRate * iClassCount;
							}
							
							int iCollateralDamageMaxUnits = GC.getUnitInfo(eLoopUnit).getCollateralDamageMaxUnits();
							if (iCollateralDamageMaxUnits > 0 && GC.getUnitInfo(eLoopUnit).getCollateralDamage() > 0)
							{
								iTotalSiegeMaxUnits += iCollateralDamageMaxUnits * iClassCount;
								iSiegeUnits += iClassCount;
							}
							else if (GC.getUnitInfo(eLoopUnit).getUnitCombatCollateralImmune((UnitCombatTypes)GC.getUnitInfo(eUnit).getUnitCombatType()))
							{
								iSiegeImmune+= iClassCount;
							}
						}
					}
				}

				if (iThisBombard == 0)
				{
					bNoBombardValue = true;
				}
				else if ((100*iTotalBombard)/(std::max(1, (iThisBombard*AI_totalUnitAIs(UNITAI_ATTACK_CITY)))) >= GC.getDefineINT("BBAI_BOMBARD_ATTACK_CITY_MAX_STACK_FRACTION"))
				{
					//too many bombard units already
					bNoBombardValue = true;
				}

				int iNumOffensiveUnits = AI_totalUnitAIs(UNITAI_ATTACK_CITY) + AI_totalUnitAIs(UNITAI_ATTACK) + AI_totalUnitAIs(UNITAI_COUNTER)/2;
				int iNumDefensiveUnits = AI_totalUnitAIs(UNITAI_CITY_DEFENSE) + AI_totalUnitAIs(UNITAI_RESERVE) + AI_totalUnitAIs(UNITAI_CITY_COUNTER)/2 + AI_totalUnitAIs(UNITAI_COLLATERAL)/2;
				iSiegeUnits += (iSiegeImmune*iNumOffensiveUnits)/std::max(1, (iNumOffensiveUnits+iNumDefensiveUnits));

				int iMAX_HIT_POINTS = GC.getDefineINT("MAX_HIT_POINTS");

				int iCollateralDamageMaxUnitsWeight = (100 * (iNumOffensiveUnits - iSiegeUnits)) / std::max(1,iTotalSiegeMaxUnits);
				iCollateralDamageMaxUnitsWeight = std::min(100, iCollateralDamageMaxUnitsWeight);
				//to decrease value further for units with low damage limits:
				int iCollateralDamageLimitWeight = 100*iMAX_HIT_POINTS - std::max(0, ((iMAX_HIT_POINTS - GC.getUnitInfo(eUnit).getCollateralDamageLimit()) * (100 -  iCollateralDamageMaxUnitsWeight)));
				iCollateralDamageLimitWeight /= iMAX_HIT_POINTS;

				int iCollateralValue = iCombatValue * GC.getUnitInfo(eUnit).getCollateralDamage() * GC.getDefineINT("COLLATERAL_COMBAT_DAMAGE");
				iCollateralValue /= 100;
				iCollateralValue *= std::max(100, (GC.getUnitInfo(eUnit).getCollateralDamageMaxUnits() * iCollateralDamageMaxUnitsWeight));
				iCollateralValue /= 100;
				iCollateralValue *= iCollateralDamageLimitWeight;
				iCollateralValue /= 100;
				iCollateralValue /= iMAX_HIT_POINTS;
				iValue += iCollateralValue;
				
				if (!bNoBombardValue && !AI_isDoStrategy(AI_STRATEGY_AIR_BLITZ))
				{
					/* original code
					int iBombardValue = GC.getUnitInfo(eUnit).getBombardRate() * 4;
					*/
					int iBombardValue = GC.getUnitInfo(eUnit).getBombardRate() * ((GC.getUnitInfo(eUnit).isIgnoreBuildingDefense()) ? 3 : 2);
					//int iTotalBombardValue = 4 * iTotalBombard;
					//int iNumBombardUnits = 2 * iTotalBombard / iBombardValue;
					int iAIDesiredBombardFraction = std::max( 5, GC.getDefineINT("BBAI_BOMBARD_ATTACK_STACK_FRACTION")); /*default: 15*/
					int iActualBombardFraction = (100 * 2 * iTotalBombard)/(iBombardValue * std::max(1, iNumOffensiveUnits));
					iActualBombardFraction = std::min(100, iActualBombardFraction);

					int iGoalTotalBombard = 200;
					int iTempBombardValue = 0;
					if (iTotalBombard < iGoalTotalBombard) //still less than 200 bombard points
					{
						iTempBombardValue = iBombardValue * (iGoalTotalBombard + 4 * (iGoalTotalBombard - iTotalBombard));
						iTempBombardValue /= iGoalTotalBombard;
						//iTempBombardValue is at most (5 * iBombardValue)
					}
					else
					{
						iTempBombardValue *= iGoalTotalBombard;
						iTempBombardValue /= std::min(2*iGoalTotalBombard, 2*iTotalBombard - iGoalTotalBombard);
					}

					if (iActualBombardFraction < iAIDesiredBombardFraction)
					{
						iBombardValue *= (iAIDesiredBombardFraction + 4 * (iAIDesiredBombardFraction - iActualBombardFraction));
						iBombardValue /= iAIDesiredBombardFraction;
						//new iBombardValue is at most (5 * old iBombardValue)
					}
					else
					{
						iBombardValue *= iAIDesiredBombardFraction;
						iBombardValue /= std::max(1, iActualBombardFraction);
					}

					if (iTempBombardValue > iBombardValue)
					{
						iBombardValue = iTempBombardValue;
					}

					iValue += iBombardValue;
				}
			}
/*
		}
*/
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		break;

	case UNITAI_COLLATERAL:
		iValue += iCombatValue;
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getCollateralDamage()) / 50);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getMoves()) / 4);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getWithdrawalProbability()) / 25);
		iValue -= ((iCombatValue * GC.getUnitInfo(eUnit).getCityAttackModifier()) / 100);
		break;

	case UNITAI_PILLAGE:
		iValue += iCombatValue;
		iValue += (iCombatValue * GC.getUnitInfo(eUnit).getMoves());
		break;

	case UNITAI_RESERVE:
		iValue += iCombatValue;
		iValue -= ((iCombatValue * GC.getUnitInfo(eUnit).getCollateralDamage()) / 200);
		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
//			int iCombatModifier = GC.getUnitInfo(eUnit).getUnitCombatModifier(iI);
//			iCombatModifier = (iCombatModifier < 40) ? iCombatModifier : (40 + (iCombatModifier - 40) / 2);
//			iValue += ((iCombatValue * iCombatModifier) / 100);
			iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getUnitCombatModifier(iI) * AI_getUnitCombatWeight((UnitCombatTypes)iI)) / 12000);
		}
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getMoves()) / 2);
		break;

	case UNITAI_COUNTER:
		iValue += (iCombatValue / 2);
		for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI) * AI_getUnitClassWeight((UnitClassTypes)iI)) / 7500);
			iValue += ((iCombatValue * (GC.getUnitInfo(eUnit).getTargetUnitClass(iI) ? 50 : 0)) / 100);
		}
		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
//			int iCombatModifier = GC.getUnitInfo(eUnit).getUnitCombatModifier(iI);
//			iCombatModifier = (iCombatModifier < 40) ? iCombatModifier : (40 + (iCombatModifier - 40) / 2);
//			iValue += ((iCombatValue * iCombatModifier) / 100);
			iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getUnitCombatModifier(iI) * AI_getUnitCombatWeight((UnitCombatTypes)iI)) / 10000);
			iValue += ((iCombatValue * (GC.getUnitInfo(eUnit).getTargetUnitCombat(iI) ? 50 : 0)) / 100);
		}
		for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
		{
			int eUnitClass = GC.getUnitInfo(eUnit).getUnitClassType();
			if (NO_UNITCLASS != eUnitClass && GC.getUnitInfo((UnitTypes)iI).getDefenderUnitClass(eUnitClass))
			{
				iValue += (50 * iCombatValue) / 100;
			}

			int eUnitCombat = GC.getUnitInfo(eUnit).getUnitCombatType();
			if (NO_UNITCOMBAT != eUnitCombat && GC.getUnitInfo((UnitTypes)iI).getDefenderUnitCombat(eUnitCombat))
			{
				iValue += (50 * iCombatValue) / 100;
			}
		}

		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getMoves()) / 2);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getWithdrawalProbability()) / 100);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/20/10                                jdog5000      */
/*                                                                                              */
/* War strategy AI                                                                              */
/************************************************************************************************/
		//iValue += (GC.getUnitInfo(eUnit).getInterceptionProbability() * 2);
		if( GC.getUnitInfo(eUnit).getInterceptionProbability() > 0 )
		{
			int iTempValue = GC.getUnitInfo(eUnit).getInterceptionProbability();

			iTempValue *= (25 + std::min(175, GET_TEAM(getTeam()).AI_getRivalAirPower()));
			iTempValue /= 100;

			iValue += iTempValue;
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/	
		break;

	case UNITAI_CITY_DEFENSE:
		iValue += ((iCombatValue * 2) / 3);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getCityDefenseModifier()) / 75);
		break;

	case UNITAI_CITY_COUNTER:
	case UNITAI_CITY_SPECIAL:
	case UNITAI_PARADROP:
		iValue += (iCombatValue / 2);
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getCityDefenseModifier()) / 100);
		for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI) * AI_getUnitClassWeight((UnitClassTypes)iI)) / 10000);
			iValue += ((iCombatValue * (GC.getUnitInfo(eUnit).getDefenderUnitClass(iI) ? 50 : 0)) / 100);
		}
		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getUnitCombatModifier(iI) * AI_getUnitCombatWeight((UnitCombatTypes)iI)) / 10000);
			iValue += ((iCombatValue * (GC.getUnitInfo(eUnit).getDefenderUnitCombat(iI) ? 50 : 0)) / 100);
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/20/10                                jdog5000      */
/*                                                                                              */
/* War strategy AI                                                                              */
/************************************************************************************************/
		//iValue += (GC.getUnitInfo(eUnit).getInterceptionProbability() * 3);
		if( GC.getUnitInfo(eUnit).getInterceptionProbability() > 0 )
		{
			int iTempValue = GC.getUnitInfo(eUnit).getInterceptionProbability();

			iTempValue *= (25 + std::min(125, GET_TEAM(getTeam()).AI_getRivalAirPower()));
			iTempValue /= 50;

			iValue += iTempValue;
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/	
		break;

	case UNITAI_EXPLORE:
		iValue += (iCombatValue / 2);
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 200);
		if (GC.getUnitInfo(eUnit).isNoBadGoodies())
		{
			iValue += 100;
		}
		break;

	case UNITAI_MISSIONARY:
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 100);
		if (getStateReligion() != NO_RELIGION)
		{
			if (GC.getUnitInfo(eUnit).getReligionSpreads(getStateReligion()) > 0)
			{
				iValue += (5 * GC.getUnitInfo(eUnit).getReligionSpreads(getStateReligion())) / 2;
			}
		}
		for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
		{
			if (GC.getUnitInfo(eUnit).getReligionSpreads((ReligionTypes)iI) && hasHolyCity((ReligionTypes)iI))
			{
				iValue += 80;
				break;
			}
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
		if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		{
		    int iTempValue = 0;
		    for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
		    {
                if (GC.getUnitInfo(eUnit).getReligionSpreads((ReligionTypes)iI))
                {
                    iTempValue += (50 * getNumCities()) / (1 + getHasReligionCount((ReligionTypes)iI));
                }
		    }
		    iValue += iTempValue;		    
		}
		for (iI = 0; iI < GC.getNumCorporationInfos(); ++iI)
		{
			if (hasHeadquarters((CorporationTypes)iI))
			{
				if (GC.getUnitInfo(eUnit).getCorporationSpreads(iI) > 0)
				{
					iValue += (5 * GC.getUnitInfo(eUnit).getCorporationSpreads(iI)) / 2;
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       06/03/09                                jdog5000      */
/*                                                                                              */
/* Bugfix				                                                                         */
/************************************************************************************************/
					// Fix potential crash, probably would only happen in mods
					if( pArea != NULL )
					{
						iValue += 300 / std::max(1, pArea->countHasCorporation((CorporationTypes)iI, getID()));
					}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
				}
			}
		}
		break;

	case UNITAI_PROPHET:
	case UNITAI_ARTIST:
	case UNITAI_SCIENTIST:
	case UNITAI_GENERAL:
	case UNITAI_MERCHANT:
	case UNITAI_ENGINEER:
		break;

	case UNITAI_SPY:
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 100);
		if (GC.getUnitInfo(eUnit).isSabotage())
		{
			iValue += 50;
		}
		if (GC.getUnitInfo(eUnit).isDestroy())
		{
			iValue += 50;
		}
		if (GC.getUnitInfo(eUnit).isCounterSpy())
		{
			iValue += 100;
		}
		break;

	case UNITAI_ICBM:
		if (GC.getUnitInfo(eUnit).getNukeRange() != -1)
		{
			iTempValue = 40 + (GC.getUnitInfo(eUnit).getNukeRange() * 40);
			if (GC.getUnitInfo(eUnit).getAirRange() == 0)
			{
				iValue += iTempValue;
			}
			else
			{
				iValue += (iTempValue * std::min(10, GC.getUnitInfo(eUnit).getAirRange())) / 10;
			}
			iValue += (iTempValue * (60 + GC.getUnitInfo(eUnit).getEvasionProbability())) / 100;
		}
		break;

	case UNITAI_WORKER_SEA:
		for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
		{
			if (GC.getUnitInfo(eUnit).getBuilds(iI))
			{
				iValue += 50;
			}
		}
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 100);
		break;

	case UNITAI_ATTACK_SEA:
		iValue += iCombatValue;
		iValue += ((iCombatValue * GC.getUnitInfo(eUnit).getMoves()) / 2);
		iValue += (GC.getUnitInfo(eUnit).getBombardRate() * 4);
		break;

	case UNITAI_RESERVE_SEA:
		iValue += iCombatValue;
		iValue += (iCombatValue * GC.getUnitInfo(eUnit).getMoves());
		break;

	case UNITAI_ESCORT_SEA:
		iValue += iCombatValue;
		iValue += (iCombatValue * GC.getUnitInfo(eUnit).getMoves());
		iValue += (GC.getUnitInfo(eUnit).getInterceptionProbability() * 3);
		if (GC.getUnitInfo(eUnit).getNumSeeInvisibleTypes() > 0)
		{
			iValue += 200;
		}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       06/03/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
		// Boats which can't be seen don't play defense, don't make good escorts
		if (GC.getUnitInfo(eUnit).getInvisibleType() != NO_INVISIBLE)
		{
			iValue /= 2;
		}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
		break;

	case UNITAI_EXPLORE_SEA:
		{
			int iExploreValue = 100;
			if (pArea != NULL)
			{
				if (pArea->isWater())
				{
					if (pArea->getUnitsPerPlayer(BARBARIAN_PLAYER) > 0)
					{
						iExploreValue += (2 * iCombatValue);
					}
				}
			}
			iValue += (GC.getUnitInfo(eUnit).getMoves() * iExploreValue);
			if (GC.getUnitInfo(eUnit).isAlwaysHostile())
			{
				iValue /= 2;
			}
			iValue /= (1 + AI_unitImpassableCount(eUnit));
		}
		break;

	case UNITAI_ASSAULT_SEA:
	case UNITAI_SETTLER_SEA:
	case UNITAI_MISSIONARY_SEA:
	case UNITAI_SPY_SEA:
		iValue += (iCombatValue / 2);
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 200);
		iValue += (GC.getUnitInfo(eUnit).getCargoSpace() * 300);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/18/09                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
		// Never build galley transports when ocean faring ones exist (issue mainly for Carracks)
		iValue /= (1 + AI_unitImpassableCount(eUnit));
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		break;

	case UNITAI_CARRIER_SEA:
		iValue += iCombatValue;
		iValue += (GC.getUnitInfo(eUnit).getMoves() * 50);
		iValue += (GC.getUnitInfo(eUnit).getCargoSpace() * 400);
		break;

	case UNITAI_MISSILE_CARRIER_SEA:
		iValue += iCombatValue * GC.getUnitInfo(eUnit).getMoves();
		iValue += (25 + iCombatValue) * (3 + (GC.getUnitInfo(eUnit).getCargoSpace()));
		break;

	case UNITAI_PIRATE_SEA:
		iValue += iCombatValue;
		iValue += (iCombatValue * GC.getUnitInfo(eUnit).getMoves());
		break;

	case UNITAI_ATTACK_AIR:
		iValue += iCombatValue;
		iValue += (GC.getUnitInfo(eUnit).getCollateralDamage() * iCombatValue) / 100;
		iValue += 4 * GC.getUnitInfo(eUnit).getBombRate();
		iValue += (iCombatValue * (100 + 2 * GC.getUnitInfo(eUnit).getCollateralDamage()) * GC.getUnitInfo(eUnit).getAirRange()) / 100;
		break;

	case UNITAI_DEFENSE_AIR:
		iValue += iCombatValue;
		iValue += (GC.getUnitInfo(eUnit).getInterceptionProbability() * 3);
		iValue += (GC.getUnitInfo(eUnit).getAirRange() * iCombatValue);
		break;

	case UNITAI_CARRIER_AIR:
		iValue += (iCombatValue);
		iValue += (GC.getUnitInfo(eUnit).getInterceptionProbability() * 2);
		iValue += (GC.getUnitInfo(eUnit).getAirRange() * iCombatValue);
		break;

	case UNITAI_MISSILE_AIR:
		iValue += iCombatValue;
		iValue += 4 * GC.getUnitInfo(eUnit).getBombRate();
		iValue += GC.getUnitInfo(eUnit).getAirRange() * iCombatValue;
		break;

	case UNITAI_ATTACK_CITY_LEMMING:
		iValue += iCombatValue;
		break;

	default:
		FAssert(false);
		break;
	}
	
	if ((iCombatValue > 0) && ((eUnitAI == UNITAI_ATTACK) || (eUnitAI == UNITAI_ATTACK_CITY)))
	{
		if (pArea != NULL)
		{
			AreaAITypes eAreaAI = pArea->getAreaAIType(getTeam());
			if (eAreaAI == AREAAI_ASSAULT || eAreaAI == AREAAI_ASSAULT_MASSING)
			{
				for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
				{
					if (GC.getUnitInfo(eUnit).getFreePromotions(iI))
					{
						if (GC.getPromotionInfo((PromotionTypes)iI).isAmphib())
						{
							iValue *= 133;
							iValue /= 100;
							break;
						}
					}
				}				
			}
		}
	}

	return std::max(0, iValue);
}


int CvPlayerAI::AI_totalUnitAIs(UnitAITypes eUnitAI) const
{
	return (AI_getNumTrainAIUnits(eUnitAI) + AI_getNumAIUnits(eUnitAI));
}


int CvPlayerAI::AI_totalAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI) const
{
	return (pArea->getNumTrainAIUnits(getID(), eUnitAI) + pArea->getNumAIUnits(getID(), eUnitAI));
}


int CvPlayerAI::AI_totalWaterAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI) const
{
	CvCity* pLoopCity;
	int iCount;
	int iLoop;
	int iI;

	iCount = AI_totalAreaUnitAIs(pArea, eUnitAI);

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{
				if (pLoopCity->waterArea() == pArea)
				{
					iCount += pLoopCity->plot()->plotCount(PUF_isUnitAIType, eUnitAI, -1, getID());

					if (pLoopCity->getOwnerINLINE() == getID())
					{
						iCount += pLoopCity->getNumTrainUnitAI(eUnitAI);
					}
				}
			}
		}
	}


	return iCount;
}


int CvPlayerAI::AI_countCargoSpace(UnitAITypes eUnitAI) const
{
	CvUnit* pLoopUnit;
	int iCount;
	int iLoop;

	iCount = 0;

	for(pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
		{
			iCount += pLoopUnit->cargoSpace();
		}
	}

	return iCount;
}


int CvPlayerAI::AI_neededExplorers(CvArea* pArea) const
{
	FAssert(pArea != NULL);
	int iNeeded = 0;

	if (pArea->isWater())
	{
		iNeeded = std::min(iNeeded + (pArea->getNumUnrevealedTiles(getTeam()) / 400), std::min(2, ((getNumCities() / 2) + 1)));
	}
	else
	{
		iNeeded = std::min(iNeeded + (pArea->getNumUnrevealedTiles(getTeam()) / 150), std::min(3, ((getNumCities() / 3) + 2)));
	}

	if (0 == iNeeded)
	{
		if ((GC.getGameINLINE().countCivTeamsAlive() - 1) > GET_TEAM(getTeam()).getHasMetCivCount(true))
		{
			if (pArea->isWater())
			{
				if (GC.getMap().findBiggestArea(true) == pArea)
				{
					iNeeded++;
				}
			}
			else
			{
			    if (getCapitalCity() != NULL && pArea->getID() == getCapitalCity()->getArea())
			    {
                    for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; iPlayer++)
                    {
                        CvPlayerAI& kPlayer = GET_PLAYER((PlayerTypes)iPlayer);
                        if (kPlayer.isAlive() && kPlayer.getTeam() != getTeam())
                        {
                            if (!GET_TEAM(getTeam()).isHasMet(kPlayer.getTeam()))
                            {
                                if (pArea->getCitiesPerPlayer(kPlayer.getID()) > 0)
                                {
                                    iNeeded++;
                                    break;
                                }
                            }
                        }
                    }
			    }
			}
		}
	}
	return iNeeded;

}


int CvPlayerAI::AI_neededWorkers(CvArea* pArea) const
{
	CvCity* pLoopCity;
	int iCount;
	int iLoop;

	iCount = countUnimprovedBonuses(pArea) * 2;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (pLoopCity->getArea() == pArea->getID())
		{
			iCount += pLoopCity->AI_getWorkersNeeded() * 3;
		}
	}
	
	if (iCount == 0)
	{
		return 0;
	}

	if (getBestRoute() != NO_ROUTE)
	{
		iCount += pArea->getCitiesPerPlayer(getID()) / 2;
	}


	iCount += 1;
	iCount /= 3;
	iCount = std::min(iCount, 3 * pArea->getCitiesPerPlayer(getID()));
	iCount = std::min(iCount, (1 + getTotalPopulation()) / 2);

	return std::max(1, iCount);

}


int CvPlayerAI::AI_neededMissionaries(CvArea* pArea, ReligionTypes eReligion) const
{
    PROFILE_FUNC();
	int iCount;
	bool bHoly, bState, bHolyState;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
	bool bCultureVictory = AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	bHoly = hasHolyCity(eReligion);
	bState = (getStateReligion() == eReligion);
	bHolyState = ((getStateReligion() != NO_RELIGION) && hasHolyCity(getStateReligion()));

	iCount = 0;
/************************************************************************************************/
/* Afforess	                  Start		 08/19/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	//AI can't waste time on missionaries if they won't earn money from it.
	if (AI_isFinancialTrouble())
	{
		if (bHoly)
		{
			CvCity* pHolyCity = GC.getGameINLINE().getHolyCity(eReligion);
			FAssertMsg(pHolyCity != NULL, "Holy City Should Not Be Null!");
			
			bool bHasHolyBuilding = false;
			for (int iI = 0; iI < GC.getNumBuildingInfos(); iI++)
			{
				if (GC.getBuildingInfo((BuildingTypes)iI).getGlobalReligionCommerce() == eReligion)
				{
					if (pHolyCity->getNumActiveBuilding((BuildingTypes)iI) > 0)
					{
						bHasHolyBuilding = true;
						break;
					}
				}
			}
			if (!bHasHolyBuilding)
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
    //internal spread.
    if (bCultureVictory || bState || bHoly)
    {
        iCount = std::max(iCount, (pArea->getCitiesPerPlayer(getID()) - pArea->countHasReligion(eReligion, getID())));
        if (iCount > 0)
        {
            if (!bCultureVictory)
			{
                iCount = std::max(1, iCount / (bHoly ? 2 : 4));
            }
            return iCount;
        }
	}

    //external spread.
    if ((bHoly && bState) || (bHoly && !bHolyState && (getStateReligion() != NO_RELIGION)))
    {
/************************************************************************************************/
/* Afforess	                  Start		 08/19/10                                               */
/*                                                                                              */
/*  What about Civics that block religion spread and open borders?                               */
/************************************************************************************************/
/*
        iCount += ((pArea->getNumCities() * 2) - (pArea->countHasReligion(eReligion) * 3));
        iCount /= 8;

        iCount = std::max(0, iCount);

		if (AI_isPrimaryArea(pArea))
		{
			iCount++;
		}
*/
		int iLoop;
		for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
		{
			CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
			if (kLoopPlayer.isAlive() && kLoopPlayer.getID() != getID())
			{
				if (GET_TEAM(getTeam()).isHasMet(kLoopPlayer.getTeam()))
				{
					if (GET_TEAM(getTeam()).isOpenBorders(kLoopPlayer.getTeam()) || GET_TEAM(getTeam()).isLimitedBorders(kLoopPlayer.getTeam()))
					{
						if (!kLoopPlayer.isNoNonStateReligionSpread())
						{
							for (CvCity* pLoopCity = kLoopPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kLoopPlayer.nextCity(&iLoop))
							{
								if (pLoopCity->area() == pArea)
								{
									if (!pLoopCity->isHasReligion(eReligion))
									{
										if (pLoopCity->plot()->plotCount(PUF_isMissionary, eReligion, -1) == 0)
										{
											iCount++;
										}
									}
								}
							}
						}
					}
				}
			}
		}
		if (isPushReligiousVictory())
		{
			iCount /= 2;
		} 
		else if (isConsiderReligiousVictory())
		{
			iCount /= 4;
		}
		else
		{
			iCount /= 4;
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
    }


	return iCount;
}


int CvPlayerAI::AI_neededExecutives(CvArea* pArea, CorporationTypes eCorporation) const
{
	if (!hasHeadquarters(eCorporation))
	{
		return 0;
	}

	int iCount = ((pArea->getCitiesPerPlayer(getID()) - pArea->countHasCorporation(eCorporation, getID())) * 2);
	iCount += (pArea->getNumCities() - pArea->countHasCorporation(eCorporation));

	iCount /= 3;

	if (AI_isPrimaryArea(pArea))
	{
		++iCount;
	}

	return iCount;
}


int CvPlayerAI::AI_adjacentPotentialAttackers(CvPlot* pPlot, bool bTestCanMove) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->area() == pPlot->area())
			{
				pUnitNode = pLoopPlot->headUnitNode();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

					if (pLoopUnit->getOwnerINLINE() == getID())
					{
						if (pLoopUnit->getDomainType() == ((pPlot->isWater()) ? DOMAIN_SEA : DOMAIN_LAND))
						{
							if (pLoopUnit->canAttack())
							{
								if (!bTestCanMove || pLoopUnit->canMove())
								{
									if (!(pLoopUnit->AI_isCityAIType()))
									{
										iCount++;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return iCount;
}


int CvPlayerAI::AI_totalMissionAIs(MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup) const
{
	PROFILE_FUNC();

	CvSelectionGroup* pLoopSelectionGroup;
	int iCount;
	int iLoop;

	iCount = 0;

	for(pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup != pSkipSelectionGroup)
		{
			if (pLoopSelectionGroup->AI_getMissionAIType() == eMissionAI)
			{
				iCount += pLoopSelectionGroup->getNumUnits();
			}
		}
	}

	return iCount;
}

int CvPlayerAI::AI_missionaryValue(CvArea* pArea, ReligionTypes eReligion, PlayerTypes* peBestPlayer) const
{
	CvTeam& kTeam = GET_TEAM(getTeam());
	CvGame& kGame = GC.getGame();
	
	int iSpreadInternalValue = 100;
	int iSpreadExternalValue = 0;

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
	// Obvious copy & paste bug
	if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE1))
	{
		iSpreadInternalValue += 500;
		if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2))
		{
			iSpreadInternalValue += 1500;
			if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3))
			{
				iSpreadInternalValue += 3000;
			}
		}
	}			
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/03/09                                jdog5000      */
/*                                                                                              */
/* Missionary AI                                                                                */
/************************************************************************************************/
	// In free religion, treat all religions like state religions
	bool bStateReligion = (getStateReligion() == eReligion);
	
	if (!isStateReligion())
	{
		// Free religion
		iSpreadInternalValue += 500;
		bStateReligion = true;
	}
	else if(bStateReligion)
	{
		iSpreadInternalValue += 1000;
	}
	else
	{
		iSpreadInternalValue += (500 * getHasReligionCount(eReligion)) / std::max(1, getNumCities());
	}
	
	int iGoldValue = 0;
	if (kTeam.hasHolyCity(eReligion))
	{
		iSpreadInternalValue += bStateReligion ? 1000 : 300;
		iSpreadExternalValue += bStateReligion ? 1000 : 150;
		if (kTeam.hasShrine(eReligion))
		{
			iSpreadInternalValue += bStateReligion ? 500 : 300;
			iSpreadExternalValue += bStateReligion ? 300 : 200;
			int iGoldMultiplier = kGame.getHolyCity(eReligion)->getTotalCommerceRateModifier(COMMERCE_GOLD);
			iGoldValue = 6 * iGoldMultiplier;
		}
	}
	
	int iOurCitiesHave = 0;
	int iOurCitiesCount = 0;
	
	if (NULL == pArea)
	{
		iOurCitiesHave = kTeam.getHasReligionCount(eReligion);
		iOurCitiesCount = kTeam.getNumCities();
	}
	else
	{
		iOurCitiesHave = pArea->countHasReligion(eReligion, getID()) + countReligionSpreadUnits(pArea, eReligion,true);
		iOurCitiesCount = pArea->getCitiesPerPlayer(getID());
	}
	
	if (iOurCitiesHave < iOurCitiesCount)
	{
		iSpreadInternalValue *= 30 + ((100 * (iOurCitiesCount - iOurCitiesHave))/ iOurCitiesCount);
		iSpreadInternalValue /= 100;
		iSpreadInternalValue += iGoldValue;
	}
	else
	{
		iSpreadInternalValue = 0;
	}
	
	if (iSpreadExternalValue > 0)
	{
		int iBestPlayer = NO_PLAYER;
		int iBestValue = 0;
		for (int iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
		{
			if (iPlayer != getID())
			{
				CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
				if (kLoopPlayer.isAlive() && kLoopPlayer.getTeam() != getTeam() && kLoopPlayer.getNumCities() > 0)
				{
/************************************************************************************************/
/* Afforess	                  Start		 12/9/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
					if (GET_TEAM(kLoopPlayer.getTeam()).isOpenBorders(getTeam())|| GET_TEAM(kLoopPlayer.getTeam()).isLimitedBorders(getTeam()))
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
					{
						int iCitiesCount = 0;
						int iCitiesHave = 0;
						int iMultiplier = AI_isDoStrategy(AI_STRATEGY_MISSIONARY) ? 60 : 25;
						if (!kLoopPlayer.isNoNonStateReligionSpread() || (kLoopPlayer.getStateReligion() == eReligion))
						{
							if (NULL == pArea)
							{
								iCitiesCount += 1 + (kLoopPlayer.getNumCities() * 75) / 100;
								iCitiesHave += std::min(iCitiesCount, kLoopPlayer.getHasReligionCount(eReligion));
							}
							else
							{
								int iPlayerSpreadPercent = (100 * kLoopPlayer.getHasReligionCount(eReligion)) / kLoopPlayer.getNumCities();
								iCitiesCount += pArea->getCitiesPerPlayer((PlayerTypes)iPlayer);
								iCitiesHave += std::min(iCitiesCount, (iCitiesCount * iPlayerSpreadPercent) / 75);
							}
						}
						
						if (kLoopPlayer.getStateReligion() == NO_RELIGION)
						{
							// Paganism counts as a state religion civic, that's what's caught below
							if (kLoopPlayer.getStateReligionCount() > 0)
							{
								int iTotalReligions = kLoopPlayer.countTotalHasReligion();
								iMultiplier += 100 * std::max(0, kLoopPlayer.getNumCities() - iTotalReligions);
								iMultiplier += (iTotalReligions == 0) ? 100 : 0;
							}
						}
						
						int iValue = (iMultiplier * iSpreadExternalValue * (iCitiesCount - iCitiesHave)) / std::max(1, iCitiesCount);
						iValue /= 100;
						iValue += iGoldValue;

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							iBestPlayer = iPlayer;
						}
					}
				}
			}
		}

		if (iBestValue > iSpreadInternalValue)
		{
			if (NULL != peBestPlayer)
			{
				*peBestPlayer = (PlayerTypes)iBestPlayer;
			}
			return iBestValue;
		}

	}
	
	if (NULL != peBestPlayer)
	{
		*peBestPlayer = getID();
	}
	return iSpreadInternalValue;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
}

int CvPlayerAI::AI_executiveValue(CvArea* pArea, CorporationTypes eCorporation, PlayerTypes* peBestPlayer) const
{
	CvTeam& kTeam = GET_TEAM(getTeam());
	CvGame& kGame = GC.getGame();
	CvCorporationInfo& kCorp = GC.getCorporationInfo(eCorporation);

	int iSpreadInternalValue = 100;
	int iSpreadExternalValue = 0;

	if (kTeam.hasHeadquarters(eCorporation))
	{
		int iGoldMultiplier = kGame.getHeadquarters(eCorporation)->getTotalCommerceRateModifier(COMMERCE_GOLD);
		iSpreadInternalValue += 10 * std::max(0, (iGoldMultiplier - 100));
		iSpreadExternalValue += 15 * std::max(0, (iGoldMultiplier - 150));
	}
	
	int iOurCitiesHave = 0;
	int iOurCitiesCount = 0;
	
	if (NULL == pArea)
	{
		iOurCitiesHave = kTeam.getHasCorporationCount(eCorporation);
		iOurCitiesCount = kTeam.getNumCities();
	}
	else
	{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      11/14/09                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
		iOurCitiesHave = pArea->countHasCorporation(eCorporation, getID()) + countCorporationSpreadUnits(pArea,eCorporation,true);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		iOurCitiesCount = pArea->getCitiesPerPlayer(getID());
	}
	
	for (int iCorp = 0; iCorp < GC.getNumCorporationInfos(); iCorp++)
	{
		if (kGame.isCompetingCorporation(eCorporation, (CorporationTypes)iCorp))
		{
			if (NULL == pArea)
			{
				iOurCitiesHave += kTeam.getHasCorporationCount(eCorporation);
			}
			else
			{
				iOurCitiesHave += pArea->countHasCorporation(eCorporation, getID());
			}
		}
	}
	
	if (iOurCitiesHave >= iOurCitiesCount)
	{
		iSpreadInternalValue = 0;
		if (iSpreadExternalValue = 0)
		{
			return 0;
		}
	}
	
	int iBonusValue = 0;
	CvCity* pCity = getCapitalCity();
	if (pCity != NULL)
	{
		iBonusValue = AI_corporationValue(eCorporation, pCity);
		iBonusValue /= 100;
	}
	
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (kLoopPlayer.isAlive() && (kLoopPlayer.getNumCities() > 0))
		{
			if ((kLoopPlayer.getTeam() == getTeam()) || GET_TEAM(kLoopPlayer.getTeam()).isVassal(getTeam()))
			{
				if (kLoopPlayer.getHasCorporationCount(eCorporation) == 0)
				{
					iBonusValue += 1000;
				}
			}
		}
	}
	
	if (iBonusValue == 0)
	{
		return 0;
	}
	
	iSpreadInternalValue += iBonusValue;
	
	if (iSpreadExternalValue > 0)
	{
		int iBestPlayer = NO_PLAYER;
		int iBestValue = 0;
		for (int iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
		{
			if (iPlayer != getID())
			{
				CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
				if (kLoopPlayer.isAlive() && (kLoopPlayer.getTeam() != getTeam()) && (kLoopPlayer.getNumCities() > 0))
				{
/************************************************************************************************/
/* Afforess	                  Start		 12/9/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
					if (GET_TEAM(kLoopPlayer.getTeam()).isOpenBorders(getTeam()) || GET_TEAM(kLoopPlayer.getTeam()).isLimitedBorders(getTeam()))
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
					{
						if (!kLoopPlayer.isNoCorporations() && !kLoopPlayer.isNoForeignCorporations())
						{
							int iCitiesCount = 0;
							int iCitiesHave = 0;
							int iMultiplier = AI_getAttitudeWeight((PlayerTypes)iPlayer);
							if (NULL == pArea)
							{
								iCitiesCount += 1 + (kLoopPlayer.getNumCities() * 50) / 100;
								iCitiesHave += std::min(iCitiesCount, kLoopPlayer.getHasCorporationCount(eCorporation));
							}
							else
							{
								int iPlayerSpreadPercent = (100 * kLoopPlayer.getHasCorporationCount(eCorporation)) / kLoopPlayer.getNumCities();
								iCitiesCount += pArea->getCitiesPerPlayer((PlayerTypes)iPlayer);
								iCitiesHave += std::min(iCitiesCount, (iCitiesCount * iPlayerSpreadPercent) / 50);
							}
						
							if (iCitiesHave < iCitiesCount)
							{
								int iValue = (iMultiplier * iSpreadExternalValue);
								iValue += ((iMultiplier - 55) * iBonusValue) / 4;
								iValue /= 100;
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									iBestPlayer = iPlayer;
								}
							}
						}
					}
				}
			}
		}

		if (iBestValue > iSpreadInternalValue)
		{
			if (NULL != peBestPlayer)
			{
				*peBestPlayer = (PlayerTypes)iBestPlayer;
			}
			return iBestValue;
		}

	}
	
	if (NULL != peBestPlayer)
	{
		*peBestPlayer = getID();
	}
	return iSpreadInternalValue;
}

//Returns approximately 100 x gpt value of the corporation.
int CvPlayerAI::AI_corporationValue(CorporationTypes eCorporation, CvCity* pCity) const
{
	if (pCity == NULL)
	{
		if (getCapitalCity() != NULL)
		{
			pCity = getCapitalCity();
		}
	}
	if (NULL == pCity)
	{
		return 0;
	}
	CvCorporationInfo& kCorp = GC.getCorporationInfo(eCorporation);
	int iBonusValue = 0;
	
	for (int iBonus = 0; iBonus < GC.getNumBonusInfos(); iBonus++)
	{
		BonusTypes eBonus = (BonusTypes)iBonus;
		int iBonusCount = pCity->getNumBonuses(eBonus);
		if (iBonusCount > 0)
		{
			for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
			{
				if (eBonus == kCorp.getPrereqBonus(i))
				{
					iBonusValue += (100 * kCorp.getYieldProduced(YIELD_FOOD) * iBonusCount);
					iBonusValue += (100 * kCorp.getYieldProduced(YIELD_PRODUCTION) * iBonusCount);
					iBonusValue += (60 * kCorp.getYieldProduced(YIELD_COMMERCE) * iBonusCount);

					iBonusValue += (60 * kCorp.getCommerceProduced(COMMERCE_GOLD) * iBonusCount);
					iBonusValue += (60 * kCorp.getCommerceProduced(COMMERCE_RESEARCH) * iBonusCount);
					iBonusValue += (25 * kCorp.getCommerceProduced(COMMERCE_CULTURE) * iBonusCount);
					iBonusValue += (40 * kCorp.getCommerceProduced(COMMERCE_ESPIONAGE) * iBonusCount);

					if (NO_BONUS != kCorp.getBonusProduced())
					{
						int iBonuses = getNumAvailableBonuses((BonusTypes)kCorp.getBonusProduced());
						iBonusValue += (AI_baseBonusVal((BonusTypes)kCorp.getBonusProduced()) * 1000) / (1 + 3 * iBonuses * iBonuses);						
					}
				}
			}
		}
	}
	iBonusValue *= 3;
	
/************************************************************************************************/
/* Afforess	                  Start		 02/09/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
//TODO: Move this to CityAI?
	iBonusValue += kCorp.getHealth() * 15000;
	iBonusValue += kCorp.getHappiness() * 25000;
	iBonusValue += kCorp.getMilitaryProductionModifier() * 3500;
	iBonusValue += kCorp.getFreeXP() * 15000;
	
	//these are whole numbers, not like the percents above. 
	iBonusValue += (10000 * kCorp.getYieldChange(YIELD_FOOD));
	iBonusValue += (10000 * kCorp.getYieldChange(YIELD_PRODUCTION));
	iBonusValue += (6000 * kCorp.getYieldChange(YIELD_COMMERCE));

	iBonusValue += (6000 * kCorp.getCommerceChange(COMMERCE_GOLD));
	iBonusValue += (6000 * kCorp.getCommerceChange(COMMERCE_RESEARCH));
	iBonusValue += (2500 * kCorp.getCommerceChange(COMMERCE_CULTURE));
	iBonusValue += (4000 * kCorp.getCommerceChange(COMMERCE_ESPIONAGE));
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	
	return iBonusValue;
}

int CvPlayerAI::AI_areaMissionAIs(CvArea* pArea, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup) const
{
	PROFILE_FUNC();

	CvSelectionGroup* pLoopSelectionGroup;
	CvPlot* pMissionPlot;
	int iCount;
	int iLoop;

	iCount = 0;

	for(pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup != pSkipSelectionGroup)
		{
			if (pLoopSelectionGroup->AI_getMissionAIType() == eMissionAI)
			{
				pMissionPlot = pLoopSelectionGroup->AI_getMissionAIPlot();

				if (pMissionPlot != NULL)
				{
					if (pMissionPlot->area() == pArea)
					{
						iCount += pLoopSelectionGroup->getNumUnits();
					}
				}
			}
		}
	}

	return iCount;
}


int CvPlayerAI::AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup, int iRange) const
{
	int iClosestTargetRange;
	return AI_plotTargetMissionAIs(pPlot, &eMissionAI, 1, iClosestTargetRange, pSkipSelectionGroup, iRange);
}

int CvPlayerAI::AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, int& iClosestTargetRange, CvSelectionGroup* pSkipSelectionGroup, int iRange) const
{
	return AI_plotTargetMissionAIs(pPlot, &eMissionAI, 1, iClosestTargetRange, pSkipSelectionGroup, iRange);
}

int CvPlayerAI::AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes* aeMissionAI, int iMissionAICount, int& iClosestTargetRange, CvSelectionGroup* pSkipSelectionGroup, int iRange) const
{
	PROFILE_FUNC();

	int iCount = 0;
	iClosestTargetRange = MAX_INT;

	int iLoop;
	for(CvSelectionGroup* pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup != pSkipSelectionGroup)
		{
			CvPlot* pMissionPlot = pLoopSelectionGroup->AI_getMissionAIPlot();

			if (pMissionPlot != NULL)
			{
				MissionAITypes eGroupMissionAI = pLoopSelectionGroup->AI_getMissionAIType();
				int iDistance = stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE());

				if (iDistance <= iRange)
				{
					for (int iMissionAIIndex = 0; iMissionAIIndex < iMissionAICount; iMissionAIIndex++)
					{
						if (eGroupMissionAI == aeMissionAI[iMissionAIIndex] || aeMissionAI[iMissionAIIndex] == NO_MISSIONAI)
						{
							iCount += pLoopSelectionGroup->getNumUnits();

							if (iDistance < iClosestTargetRange)
							{
								iClosestTargetRange = iDistance;
							}
						}
					}
				}
			}
		}
	}

	return iCount;
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/03/10                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
int CvPlayerAI::AI_cityTargetUnitsByPath(CvCity* pCity, CvSelectionGroup* pSkipSelectionGroup, int iMaxPathTurns) const
{
	PROFILE_FUNC();

	int iCount = 0;

	int iLoop;
	int iPathTurns;
	for(CvSelectionGroup* pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup != pSkipSelectionGroup && pLoopSelectionGroup->plot() != NULL && pLoopSelectionGroup->getNumUnits() > 0)
		{
			CvPlot* pMissionPlot = pLoopSelectionGroup->AI_getMissionAIPlot();

			if (pMissionPlot != NULL )
			{
				int iDistance = stepDistance(pCity->getX_INLINE(), pCity->getY_INLINE(), pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE());

				if (iDistance <= 1)
				{
					if( pLoopSelectionGroup->generatePath(pLoopSelectionGroup->plot(), pMissionPlot, 0, true, &iPathTurns) )
					{
						if( !(pLoopSelectionGroup->canAllMove()) )
						{
							iPathTurns++;
						}

						if( iPathTurns <= iMaxPathTurns )
						{
							iCount += pLoopSelectionGroup->getNumUnits();
						}
					}
				}
			}
		}
	}

	return iCount;
}

int CvPlayerAI::AI_unitTargetMissionAIs(CvUnit* pUnit, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup) const
{
	return AI_unitTargetMissionAIs(pUnit, &eMissionAI, 1, pSkipSelectionGroup, -1);
}

int CvPlayerAI::AI_unitTargetMissionAIs(CvUnit* pUnit, MissionAITypes* aeMissionAI, int iMissionAICount, CvSelectionGroup* pSkipSelectionGroup) const
{
	return AI_unitTargetMissionAIs(pUnit, aeMissionAI, iMissionAICount, pSkipSelectionGroup, -1);
}

int CvPlayerAI::AI_unitTargetMissionAIs(CvUnit* pUnit, MissionAITypes* aeMissionAI, int iMissionAICount, CvSelectionGroup* pSkipSelectionGroup, int iMaxPathTurns) const
{
	PROFILE_FUNC();

	CvSelectionGroup* pLoopSelectionGroup;
	int iCount;
	int iLoop;

	iCount = 0;

	for(pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup != pSkipSelectionGroup)
		{
			if (pLoopSelectionGroup->AI_getMissionAIUnit() == pUnit)
			{
				MissionAITypes eGroupMissionAI = pLoopSelectionGroup->AI_getMissionAIType();
				int iPathTurns = MAX_INT;

				if( iMaxPathTurns >= 0 && (pUnit->plot() != NULL) && (pLoopSelectionGroup->plot() != NULL))
				{
					pLoopSelectionGroup->generatePath(pLoopSelectionGroup->plot(), pUnit->plot(), 0, false, &iPathTurns);
					if( !(pLoopSelectionGroup->canAllMove()) )
					{
						iPathTurns++;
					}
				}

				if ((iMaxPathTurns == -1) || (iPathTurns <= iMaxPathTurns))
				{
					for (int iMissionAIIndex = 0; iMissionAIIndex < iMissionAICount; iMissionAIIndex++)
					{
						if (eGroupMissionAI == aeMissionAI[iMissionAIIndex] || NO_MISSIONAI == aeMissionAI[iMissionAIIndex])
						{
							iCount += pLoopSelectionGroup->getNumUnits();
						}
					}
				}
			}
		}
	}

	return iCount;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/		

int CvPlayerAI::AI_enemyTargetMissionAIs(MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup) const
{
	return AI_enemyTargetMissionAIs(&eMissionAI, 1, pSkipSelectionGroup);
}

int CvPlayerAI::AI_enemyTargetMissionAIs(MissionAITypes* aeMissionAI, int iMissionAICount, CvSelectionGroup* pSkipSelectionGroup) const
{
	PROFILE_FUNC();

	int iCount = 0;

	int iLoop;
	for(CvSelectionGroup* pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup != pSkipSelectionGroup)
		{
			CvPlot* pMissionPlot = pLoopSelectionGroup->AI_getMissionAIPlot();

			if (NULL != pMissionPlot && pMissionPlot->isOwned())
			{
				MissionAITypes eGroupMissionAI = pLoopSelectionGroup->AI_getMissionAIType();
				for (int iMissionAIIndex = 0; iMissionAIIndex < iMissionAICount; iMissionAIIndex++)
				{
					if (eGroupMissionAI == aeMissionAI[iMissionAIIndex] || NO_MISSIONAI == aeMissionAI[iMissionAIIndex])
					{
						if (GET_TEAM(getTeam()).AI_isChosenWar(pMissionPlot->getTeam()))
						{
							iCount += pLoopSelectionGroup->getNumUnits();
							iCount += pLoopSelectionGroup->getCargo();
						}
					}
				}
			}
		}
	}

	return iCount;
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/19/10                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
int CvPlayerAI::AI_enemyTargetMissions(TeamTypes eTargetTeam, CvSelectionGroup* pSkipSelectionGroup) const
{
	PROFILE_FUNC();

	int iCount = 0;

	int iLoop;
	for(CvSelectionGroup* pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup != pSkipSelectionGroup)
		{
			CvPlot* pMissionPlot = pLoopSelectionGroup->AI_getMissionAIPlot();

			if( pMissionPlot == NULL )
			{
				pMissionPlot = pLoopSelectionGroup->plot();
			}

			if (NULL != pMissionPlot )
			{
				if( pMissionPlot->isOwned() && pMissionPlot->getTeam() == eTargetTeam )
				{
					if (atWar(getTeam(),pMissionPlot->getTeam()) || pLoopSelectionGroup->AI_isDeclareWar(pMissionPlot))
					{
						iCount += pLoopSelectionGroup->getNumUnits();
						iCount += pLoopSelectionGroup->getCargo();
					}
				}
			}
		}
	}

	return iCount;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

int CvPlayerAI::AI_wakePlotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup) const
{
	PROFILE_FUNC();

	FAssert(pPlot != NULL);
	
	int iCount = 0;

	int iLoop;
	for(CvSelectionGroup* pLoopSelectionGroup = firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = nextSelectionGroup(&iLoop))
	{
		if (pLoopSelectionGroup != pSkipSelectionGroup)
		{
			MissionAITypes eGroupMissionAI = pLoopSelectionGroup->AI_getMissionAIType();
			if (eMissionAI == NO_MISSIONAI || eMissionAI == eGroupMissionAI)
			{
				CvPlot* pMissionPlot = pLoopSelectionGroup->AI_getMissionAIPlot();
				if (pMissionPlot != NULL && pMissionPlot == pPlot)
				{
					iCount += pLoopSelectionGroup->getNumUnits();
					pLoopSelectionGroup->setActivityType(ACTIVITY_AWAKE);
				}
			}
		}
	}

	return iCount;
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      07/19/09                                jdog5000      */
/*                                                                                              */
/* Civic AI                                                                                     */
/************************************************************************************************/
CivicTypes CvPlayerAI::AI_bestCivic(CivicOptionTypes eCivicOption) const
{
	int iBestValue;
	return AI_bestCivic( eCivicOption, &iBestValue );
}

CivicTypes CvPlayerAI::AI_bestCivic(CivicOptionTypes eCivicOption, int* iBestValue) const
{
	CivicTypes eBestCivic;
	int iValue;
	int iI;

	(*iBestValue) = MIN_INT;
	eBestCivic = NO_CIVIC;

	for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
	{
		if (GC.getCivicInfo((CivicTypes)iI).getCivicOptionType() == eCivicOption)
		{
			if (canDoCivics((CivicTypes)iI))
			{
				iValue = AI_civicValue((CivicTypes)iI);

				if (iValue > (*iBestValue))
				{
					(*iBestValue) = iValue;
					eBestCivic = ((CivicTypes)iI);
				}
			}
		}
	}

	return eBestCivic;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/		



/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/15/10                                jdog5000      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/

void CvPlayerAI::AI_findBestCivics(CivicTypes* paeBestCivics, CivicTypes* paeBestFixedBordersCivics, int* paiBestValues, int* paiBestFixedBordersValues) const
{
	int iValue, iI, iJ;
	if (GC.getGameINLINE().isOption(GAMEOPTION_FIXED_BORDERS))
	{
		for (iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
		{
			paiBestValues[iI] = MIN_INT;
			paiBestFixedBordersValues[iI] = MIN_INT;
			
			paeBestCivics[iI] = NO_CIVIC;
			paeBestFixedBordersCivics[iI] = NO_CIVIC;
				
			for (iJ = 0; iJ < GC.getNumCivicInfos(); iJ++)
			{			
				if (GC.getCivicInfo((CivicTypes)iJ).getCivicOptionType() == iI)
				{
					if (canDoCivics((CivicTypes)iJ))
					{
						iValue = AI_civicValue((CivicTypes)iJ);
	
						if (iValue > paiBestValues[iI])
						{
							paiBestValues[iI] = iValue;
							paeBestCivics[iI] = ((CivicTypes)iJ);
						}
						
						if (iValue > paiBestFixedBordersValues[iI])
						{
							if (GC.getCivicInfo((CivicTypes)iJ).IsFixedBorders())
							{
								paiBestFixedBordersValues[iI] = iValue;
								paeBestFixedBordersCivics[iI] = ((CivicTypes)iJ);
							}
						}
					}
				}
			}
		}
	}
}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
int CvPlayerAI::AI_civicValue(CivicTypes eCivic) const
{
	PROFILE_FUNC();

	bool bWarPlan;
	int iConnectedForeignCities;
	int iTotalReligonCount;
	int iHighestReligionCount;
	int iWarmongerPercent;
	int iHappiness;
	int iValue;
	int iTempValue;
	int iI, iJ;
	bool bCultureVictory3 = AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3);
	bool bCultureVictory2 = AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2);

	FAssertMsg(eCivic < GC.getNumCivicInfos(), "eCivic is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eCivic >= 0, "eCivic is expected to be non-negative (invalid Index)");

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/05/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
	// Circumvents crash bug in simultaneous turns MP games
	if( eCivic == NO_CIVIC )
	{
		return 1;
	}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

	if( isBarbarian() )
	{
		return 1;
	}

	CvCivicInfo& kCivic = GC.getCivicInfo(eCivic);

	bWarPlan = (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0);
	if( bWarPlan )
	{
		bWarPlan = false;
		int iEnemyWarSuccess = 0;

		for( int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++ )
		{
			if( GET_TEAM((TeamTypes)iTeam).isAlive() && !GET_TEAM((TeamTypes)iTeam).isMinorCiv() )
			{
				if( GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iTeam) != NO_WARPLAN )
				{
					if( GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iTeam) == WARPLAN_TOTAL || GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iTeam) == WARPLAN_PREPARING_TOTAL )
					{
						bWarPlan = true;
						break;
					}

					if( GET_TEAM(getTeam()).AI_isLandTarget((TeamTypes)iTeam) )
					{
						bWarPlan = true;
						break;
					}

					iEnemyWarSuccess += GET_TEAM((TeamTypes)iTeam).AI_getWarSuccess(getTeam());
				}
			}
		}

		if( !bWarPlan )
		{
			if( iEnemyWarSuccess > std::min(getNumCities(), 4) * GC.getWAR_SUCCESS_CITY_CAPTURING() )
			{
				// Lots of fighting, so war is real
				bWarPlan = true;
			}
			else if( iEnemyWarSuccess > std::min(getNumCities(), 2) * GC.getWAR_SUCCESS_CITY_CAPTURING() )
			{
				if( GET_TEAM(getTeam()).AI_getEnemyPowerPercent() > 120 )
				{
					bWarPlan = true;
				}
			}
		}
	}

	if( !bWarPlan )
	{
		// Aggressive players will stick with war civics
		if( GET_TEAM(getTeam()).AI_getTotalWarOddsTimes100() > 200 )
		{
			bWarPlan = true;
		}
	}

	iConnectedForeignCities = countPotentialForeignTradeCitiesConnected();
	iTotalReligonCount = countTotalHasReligion();
	ReligionTypes eBestReligion = AI_bestReligion();
	if (eBestReligion == NO_RELIGION)
	{
		eBestReligion = getStateReligion();
	}
	iHighestReligionCount = ((eBestReligion == NO_RELIGION) ? 0 : getHasReligionCount(eBestReligion));
	iWarmongerPercent = 25000 / std::max(100, (100 + GC.getLeaderHeadInfo(getPersonalityType()).getMaxWarRand())); 

	iValue = (getNumCities() * 6);

	iValue += (GC.getCivicInfo(eCivic).getAIWeight() * getNumCities());

	iValue += (getCivicPercentAnger(eCivic) / 10);

	iValue += -(GC.getCivicInfo(eCivic).getAnarchyLength() * getNumCities());

	iValue += -(getSingleCivicUpkeep(eCivic, true)*80)/100;

	int iTemp = 0;
	CvCity* pCapital = getCapitalCity();
	iValue += ((kCivic.getGreatPeopleRateModifier() * getNumCities()) / 10);
	iValue += ((kCivic.getGreatGeneralRateModifier() * getNumMilitaryUnits()) / 50);
	iValue += ((kCivic.getDomesticGreatGeneralRateModifier() * getNumMilitaryUnits()) / 100);
	iValue += -((kCivic.getDistanceMaintenanceModifier() * std::max(0, (getNumCities() - 3))) / 8);
	iValue += -((kCivic.getNumCitiesMaintenanceModifier() * std::max(0, (getNumCities() - 3))) / 8);
	iTemp = kCivic.getFreeExperience();
	if( iTemp > 0 )
	{
		// Free experience increases value of hammers spent on units, population is an okay measure of base hammer production
		iTempValue = (iTemp * getTotalPopulation() * (bWarPlan ? 30 : 12))/100;
		iTempValue *= AI_averageYieldMultiplier(YIELD_PRODUCTION);
		iTempValue /= 100;
		iTempValue *= iWarmongerPercent;
		iTempValue /= 100;
		iValue += iTempValue;
	}

	iValue += ((kCivic.getWorkerSpeedModifier() * AI_getNumAIUnits(UNITAI_WORKER)) / 15);
	iValue += ((kCivic.getImprovementUpgradeRateModifier() * getNumCities()) / 50);
	iValue += (kCivic.getMilitaryProductionModifier() * getNumCities() * iWarmongerPercent) / (bWarPlan ? 300 : 500 ); 
	iValue += (kCivic.getBaseFreeUnits() / 2);
	iValue += (kCivic.getBaseFreeMilitaryUnits() / 2);
	iValue += ((kCivic.getFreeUnitsPopulationPercent() * getTotalPopulation()) / 200);
	iValue += ((kCivic.getFreeMilitaryUnitsPopulationPercent() * getTotalPopulation()) / 300);
	iValue += -(kCivic.getGoldPerUnit() * getNumUnits());
	iValue += -(kCivic.getGoldPerMilitaryUnit() * getNumMilitaryUnits() * iWarmongerPercent) / 200;

/************************************************************************************************/
/* Afforess	                  Start		 01/16/10                                               */
/*                                                                                              */
/* Better AI Civic Calculation                                                                  */
/************************************************************************************************/
	iTempValue = 0;
	int iLoop;
	if (kCivic.isMilitaryFoodProduction())
	{
		for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			iTempValue += pLoopCity->foodDifference(false);
		}
		//If not at war Food is generally more valuable then hammers
		if (!bWarPlan)
		{
			iTempValue /= -4;
		}
		//If we are at war hammers are more valuable
		else
		{
			iTempValue *= 3;
		}
		iValue += iTempValue;
	}
	
	//Upgrade Anywhere 
	iTempValue = 0;
	if (kCivic.isUpgradeAnywhere())
	{
		iTempValue += getNumMilitaryUnits() * iWarmongerPercent / 100;
		bool bRich = false;
		//the current gold we have plus the gold we will have in 10 turns is a decent
		//estimate of whether we are rich (if we can afford to upgrade units, anyway)
		if (getGold() + (calculateBaseNetGold() * 10) > (50 + 100 * getCurrentEra()))
			bRich = true;
		if (bWarPlan)
		{
			iTempValue *= 2;
			if (bRich)
			{
				iTempValue *= 2;
			}
		}
		else
		{
			iTempValue /= 2;
		}
		iValue += iTempValue;
	}
	bool bValid = true;
	iTempValue = 0;
	int iNonstateReligionCount = 0;
	//Inquisition Civic Values
	if (kCivic.isAllowInquisitions())
	{
		if (getStateReligion() != NO_RELIGION)
		{
			//check that we don't have a civic that already blocks this inquisitions...
			for (iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
			{
				//we are considering changing this civic, so ignore it
				if (GC.getCivicInfo(eCivic).getCivicOptionType() != (CivicOptionTypes)iI)
				{
					if (GC.getCivicInfo(getCivics((CivicOptionTypes)iI)).isDisallowInquisitions())
					{
						bValid = false;
					}
				}
			}
			if (bValid)
			{
				if(hasInquisitionTarget())
				{
					for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
					{
						if ((ReligionTypes)iI != getStateReligion())
						{
							for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
							{
								if (pLoopCity->isHasReligion((ReligionTypes)iI))
								{
									iNonstateReligionCount++;
								}
							}
						}
					}
				}
			}
		}
		if(isPushReligiousVictory())
		{
			iValue += iNonstateReligionCount * 20;
		} else if(isConsiderReligiousVictory())
		{
			iValue /= 5;
		}
		iValue += countCityReligionRevolts() * 5;
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	//iValue += ((kCivic.isMilitaryFoodProduction()) ? 0 : 0);
	iTemp = getWorldSizeMaxConscript(eCivic);
	if( iTemp > 0 && (pCapital != NULL) )
	{
		UnitTypes eConscript = pCapital->getConscriptUnit();
		if( eConscript != NO_UNIT )
		{
			// Nationhood
			int iCombatValue = GC.getGameINLINE().AI_combatValue(eConscript);
			if( iCombatValue > 33 )
			{
				iTempValue = getNumCities() + ((bWarPlan) ? 30 : 10);

				iTempValue *= range(GET_TEAM(getTeam()).AI_getEnemyPowerPercent(), 50, 300);
				iTempValue /= 100;

				iTempValue *= iCombatValue;
				iTempValue /= 75;

				int iWarSuccessRatio = GET_TEAM(getTeam()).AI_getWarSuccessCapitulationRatio();
				if( iWarSuccessRatio < -25 )
				{
					iTempValue *= 75 + range(-iWarSuccessRatio, 25, 100);
					iTempValue /= 100;
				}

				iValue += iTempValue;
			}
		}
	}
	iValue += ((kCivic.isNoUnhealthyPopulation()) ? (getTotalPopulation() / 3) : 0);
	if (bWarPlan)
	{
		iValue += ((kCivic.getExpInBorderModifier() * getNumMilitaryUnits()) / 200);
	}
	iValue += ((kCivic.isBuildingOnlyHealthy()) ? (getNumCities() * 3) : 0);
	iValue += -((kCivic.getWarWearinessModifier() * getNumCities()) / ((bWarPlan) ? 10 : 50));
	iValue += (kCivic.getFreeSpecialist() * getNumCities() * 12);
	iValue += (kCivic.getTradeRoutes() * (std::max(0, iConnectedForeignCities - getNumCities() * 3) * 6 + (getNumCities() * 2))); 
	iValue += -((kCivic.isNoForeignTrade()) ? (iConnectedForeignCities * 3) : 0);
	if (kCivic.isNoCorporations())
	{
		iValue -= countHeadquarters() * (40 + 3 * getNumCities());
	}
/************************************************************************************************/
/* Afforess	                  Start		 12/9/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		iValue += (kCivic.getUnitCombatProductionModifier(iI) * 2) / 3;
	}
	
	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		iValue += (kCivic.getBuildingClassProductionModifier(iI) * 2) / 5;
	}	
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	if (kCivic.isNoForeignCorporations())
	{
		for (int iCorp = 0; iCorp < GC.getNumCorporationInfos(); ++iCorp)
		{
			if (!GET_TEAM(getTeam()).hasHeadquarters((CorporationTypes)iCorp))
			{
				iValue += countCorporations((CorporationTypes)iCorp) * 3;
			}
		}
	}
/************************************************************************************************/
/* Afforess	                  Start		 06/17/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	int iCorpMaintenanceMod;
	if (GC.getGameINLINE().isOption(GAMEOPTION_REALISTIC_CORPORATIONS))
	{
		iCorpMaintenanceMod = kCivic.getRealCorporationMaintenanceModifier() + 100;
	}
	else
	{
		iCorpMaintenanceMod = kCivic.getCorporationMaintenanceModifier();
	}
	if (iCorpMaintenanceMod != 0)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	{
		int iCorpCount = 0;
		int iHQCount = 0;
		for (int iCorp = 0; iCorp < GC.getNumCorporationInfos(); ++iCorp)
		{
			if (GET_TEAM(getTeam()).hasHeadquarters((CorporationTypes)iCorp))
			{
				iHQCount++;
			}
			iCorpCount += countCorporations((CorporationTypes)iCorp);
		}
/************************************************************************************************/
/* Afforess	                  Start		 06/17/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	iValue += (-iCorpMaintenanceMod * (iHQCount * (25 + getNumCities() * 2) + iCorpCount * 7)) / 25;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	}

	if (kCivic.getCivicPercentAnger() != 0)
	{
		int iNumOtherCities = GC.getGameINLINE().getNumCities() - getNumCities();
		iValue += (30 * getNumCities() * getCivicPercentAnger(eCivic, true)) / kCivic.getCivicPercentAnger();
		
		int iTargetGameTurn = 2 * getNumCities() * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent();
		iTargetGameTurn /= GC.getGame().countCivPlayersEverAlive();
		iTargetGameTurn += GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent() * 30;
		
		iTargetGameTurn /= 100;
		iTargetGameTurn = std::max(10, iTargetGameTurn);
		
		int iElapsedTurns = GC.getGame().getElapsedGameTurns();

		if (iElapsedTurns > iTargetGameTurn)
		{
			iValue += (std::min(iTargetGameTurn, iElapsedTurns - iTargetGameTurn) * (iNumOtherCities * kCivic.getCivicPercentAnger())) / (15 * iTargetGameTurn);
		}
	}

	//
	// Mongoose AICivicFix BEGIN
	//
	iTempValue = kCivic.getExtraHealth();
	if (iTempValue != 0)
	{
	//
	// Mongoose AICivicFix END
	//
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/21/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* orginal bts code
		iValue += (getNumCities() * 6 * AI_getHealthWeight(isCivic(eCivic) ? -kCivic.getExtraHealth() : kCivic.getExtraHealth(), 1)) / 100;
*/
		//
		// Mongoose AICivicFix BEGIN
		//
		// -- thanks to Fuyu for this code, which is an imperfect kludge/hack but still an improvement
		//
		iValue += (getNumCities() * 6 * (isCivic(eCivic)? -AI_getHealthWeight(-iTempValue, 1) : AI_getHealthWeight(iTempValue, 1))) / 100;
		//
		// Mongoose AICivicFix END
		//
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
	}
			
	//
	// Mongoose AICivicFix BEGIN
	//
	// -- thanks to Afforess for mentioning I could get a UNITAI_CITY_DEFENSE count directly
	//
	iTempValue = (kCivic.getHappyPerMilitaryUnit() * AI_getNumAIUnits(UNITAI_CITY_DEFENSE)) / std::max(1, getNumCities());
	//
	// Mongoose AICivicFix END
	//
	if (iTempValue != 0)
	{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/21/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* orginal bts code
		iValue += (getNumCities() * 9 * AI_getHappinessWeight(isCivic(eCivic) ? -iTempValue : iTempValue, 1)) / 100;
*/
		//
		// Mongoose AICivicFix BEGIN
		//
		// -- thanks to Fuyu for this code, which is an imperfect kludge/hack but still an improvement
		//
		iValue += (getNumCities() * 9 * (isCivic(eCivic)? -AI_getHappinessWeight(-iTempValue, 1) : AI_getHappinessWeight(iTempValue, 1))) / 100;
		//
		// Mongoose AICivicFix END
		//
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
	}
		
	iTempValue = kCivic.getLargestCityHappiness();
	if (iTempValue != 0)
	{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/21/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* orginal bts code
		iValue += (12 * std::min(getNumCities(), GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getTargetNumCities()) * AI_getHappinessWeight(isCivic(eCivic) ? -iTempValue : iTempValue, 1)) / 100;
*/
		//
		// Mongoose AICivicFix BEGIN
		//
		// -- thanks to Fuyu for this code, which is an imperfect kludge/hack but still an improvement
		//
		iValue += (12 * std::min(getNumCities(), GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getTargetNumCities()) * (isCivic(eCivic)? -AI_getHappinessWeight(-iTempValue, 1) : AI_getHappinessWeight(iTempValue, 1))) / 100;
		//
		// Mongoose AICivicFix END
		//
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
	}
	
	if (kCivic.getWarWearinessModifier() != 0)
	{
		int iAngerPercent = getWarWearinessPercentAnger();
		int iPopulation = 3 + (getTotalPopulation() / std::max(1, getNumCities()));

		int iTempValue = (-kCivic.getWarWearinessModifier() * iAngerPercent * iPopulation) / (GC.getPERCENT_ANGER_DIVISOR() * 100);
		if (iTempValue != 0)
		{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/21/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* orginal bts code
			iValue += (11 * getNumCities() * AI_getHappinessWeight(isCivic(eCivic) ? -iTempValue : iTempValue, 1)) / 100;
*/
			//
			// Mongoose AICivicFix BEGIN
			//
			// -- thanks to Fuyu for this code, which is an imperfect kludge/hack but still an improvement
			//
			iValue += (11 * getNumCities() * (isCivic(eCivic)? -AI_getHappinessWeight(-iTempValue, 1) : AI_getHappinessWeight(iTempValue, 1))) / 100;
			//
			// Mongoose AICivicFix END
			//
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
		}
	}
	
	iValue += (kCivic.getNonStateReligionHappiness() * (iTotalReligonCount - iHighestReligionCount) * 5);

	if (kCivic.isStateReligion())
	{
		if (iHighestReligionCount > 0)
		{
			iValue += iHighestReligionCount;

			iValue += ((kCivic.isNoNonStateReligionSpread()) ? ((getNumCities() - iHighestReligionCount) * 2) : 0);
			iValue += (kCivic.getStateReligionHappiness() * iHighestReligionCount * 4);
			iValue += ((kCivic.getStateReligionGreatPeopleRateModifier() * iHighestReligionCount) / 20);
			iValue += (kCivic.getStateReligionGreatPeopleRateModifier() / 4);
			iValue += ((kCivic.getStateReligionUnitProductionModifier() * iHighestReligionCount) / 4);
			iValue += ((kCivic.getStateReligionBuildingProductionModifier() * iHighestReligionCount) / 3);
			iValue += (kCivic.getStateReligionFreeExperience() * iHighestReligionCount * ((bWarPlan) ? 6 : 2));

			// Value civic based on current gains from having a state religion
			for (int iI = 0; iI < GC.getNumVoteSourceInfos(); ++iI)
			{
				if (GC.getGameINLINE().isDiploVote((VoteSourceTypes)iI))
				{
					ReligionTypes eReligion = GC.getGameINLINE().getVoteSourceReligion((VoteSourceTypes)iI);

					if( NO_RELIGION != eReligion && eReligion == eBestReligion )
					{
						// Are we leader of AP?
						if( getTeam() == GC.getGameINLINE().getSecretaryGeneral((VoteSourceTypes)iI) )
						{
							iValue += 100;
						}

						// Any benefits we get from AP tied to state religion?
						/*
						for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
						{
							iTempValue = iHighestReligionCount*GC.getVoteSourceInfo((VoteSourceTypes)iI).getReligionYield(iYield);

							iTempValue *= AI_yieldWeight((YieldTypes)iYield);
							iTempValue /= 100;

							iValue += iTempValue;
						}

						for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; ++iCommerce)
						{
							iTempValue = (iHighestReligionCount*GC.getVoteSourceInfo((VoteSourceTypes)iI).getReligionCommerce(iCommerce))/2;

							iTempValue *= AI_commerceWeight((CommerceTypes)iCommerce);
							iTempValue = 100;

							iValue += iTempValue;
						}
						*/
					}
				}
			}

			// Value civic based on wonders granting state religion boosts
			for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; ++iCommerce)
			{
				iTempValue = (iHighestReligionCount * getStateReligionBuildingCommerce((CommerceTypes)iCommerce))/2;

				iTempValue *= AI_commerceWeight((CommerceTypes)iCommerce);
				iTempValue /= 100;

				iValue += iTempValue;
			}
		}
	}

/************************************************************************************************/
/* Afforess  Civic AI Logic            Start          08/16/09                   		        */
/*                                                                                              */
/*                        AI Values                                                             */
/************************************************************************************************/
	iTempValue = (kCivic.getCivicHappiness());
	if (iTempValue != 0)
	{
		iValue += (12 * getNumCities() * AI_getHappinessWeight(isCivic(eCivic) ? -iTempValue : iTempValue, 1)) / 100;
	}
	
	if (kCivic.getPopulationgrowthratepercentage() != 0)
	{
		int iLoop;
		int iTempValue;
		int iCityHappy;

		iTempValue = 0;
		iCityHappy = 0;

		for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			iCityHappy += pLoopCity->happyLevel() - pLoopCity->unhappyLevel();
		}

		if (iCityHappy > 0)

		iTempValue += -(kCivic.getPopulationgrowthratepercentage() * iCityHappy / 10);

		else if (iCityHappy < 0)

		iTempValue += -(kCivic.getPopulationgrowthratepercentage() * iCityHappy / 8);

		else if (iCityHappy = 0)

		iTempValue += (kCivic.getPopulationgrowthratepercentage() / 5);

		iValue += iTempValue;

	}

//This logic is pretty poor, but poor logic is better than no logic at all
	if (kCivic.getAttitudeShareMod() != 0)
	{
		int iTempValue = 0;
		
		// The AI will disfavor bad attitude modifiers more than good ones
		if (kCivic.getAttitudeShareMod() < 0)
		{
			iTempValue += (kCivic.getAttitudeShareMod() * 4);
		}
		else if (kCivic.getAttitudeShareMod() > 0)
		{
			iTempValue += (kCivic.getAttitudeShareMod() * 3);
		}
		iValue += iTempValue;
	}

	if (bWarPlan)
	{
		int iTempValue = 0;
		
		//negative values are good, positive ones, bad
		if (kCivic.getDistantUnitSupportCostModifier() < 0)
		{
			iTempValue = (-kCivic.getDistantUnitSupportCostModifier() * 2);
		}
		//if we are going to war soon, we can't afford high costs
		else if (kCivic.getDistantUnitSupportCostModifier() > 0)
		{
			iTempValue = (-kCivic.getDistantUnitSupportCostModifier() * 3);
		}
		//City defense is good, especially during wars
		if (kCivic.getExtraCityDefense() > 0)
		{
			iTempValue += (kCivic.getExtraCityDefense() * 2);
		}
		//Negative city defense would be really bad in a war, avoid at all costs
		else if (kCivic.getExtraCityDefense() < 0)
		{
			iTempValue -= (kCivic.getExtraCityDefense() * 4);
		}
		iValue += iTempValue;
	}
	else
	{
		iValue += (-kCivic.getDistantUnitSupportCostModifier() / 2);
		iValue += kCivic.getExtraCityDefense();
	}
	
	if (kCivic.isNoCapitalUnhappiness() && pCapital != NULL)
	{
		iValue += std::max(10, (std::min(0,(pCapital->unhappyLevel() - pCapital->happyLevel())) * 3));
	}
	
	if (kCivic.getTaxRateUnhappiness() != 0)
	{
		int iNewAnger = (kCivic.getTaxRateUnhappiness() * (100 - getCommercePercent(COMMERCE_GOLD)) / 100);
		int iLoop;
		int iCityHappy;
		iTempValue = 0;
		iCityHappy = 0;

		for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			iCityHappy = pLoopCity->happyLevel() - pLoopCity->unhappyLevel();
			if (iCityHappy > iNewAnger)
				;//do nothing
			else
				iTempValue -= iNewAnger * 12;
		}
		iNewAnger += iTempValue;
		//iValue += (12 * getNumCities() * AI_getHappinessWeight(-iNewAnger, 1)) / 50;
	}
	
	bool bFinancialTrouble = AI_isFinancialTrouble();
	iTempValue = 0;
	if (kCivic.isTaxationAnger())
	{
		int iNetIncome = 1 + getCommerceRate(COMMERCE_GOLD) + std::max(0, getGoldPerTurn());
		int iNetExpenses = 1 + calculateInflatedCosts() + std::min(0, getGoldPerTurn());
		int iNetProfit = iNetIncome - iNetExpenses;
	
		iTempValue += iNetProfit;
		iTempValue -= getNumCities() * 5;
		if (bFinancialTrouble || (!GC.getGameINLINE().isOption(GAMEOPTION_NO_REVOLUTION)))
		{
			iTempValue *= 2;
		}
		iValue += iTempValue;
	}
	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		if (kCivic.getFreeSpecialistCount(iI) > 0)
		{
			iValue += getNumCities() * kCivic.getFreeSpecialistCount(iI) * 12;
		}
	}
	
	if (pCapital != NULL)
	{
		for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			BuildingTypes eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));
			bool bValidCivics = true;
			if (eLoopBuilding != NO_BUILDING)
			{
				if (GC.getBuildingInfo(eLoopBuilding).getPrereqAndTech() == NO_TECH || GC.getTechInfo((TechTypes)GC.getBuildingInfo(eLoopBuilding).getPrereqAndTech()).getEra() == getCurrentEra())
				{
					if (GC.getBuildingInfo(eLoopBuilding).isPrereqAndCivics(eCivic) || GC.getBuildingInfo(eLoopBuilding).isPrereqOrCivics(eCivic))
					{
						bool bValid = false;
						bool bCanConstruct = canConstruct(eLoopBuilding);
						for (int iJ = 0; iJ < GC.getNumCivicInfos(); iJ++)
						{
							if (eCivic != iJ)
							{
								if (GC.getBuildingInfo(eLoopBuilding).isPrereqAndCivics(iJ))
								{
									if (!isCivic((CivicTypes)iJ))
									{
										bValidCivics = false;
									}
								}
							}
						}
						//Make sure we have the correct prereq Or Civics. We need just one
						if (bValidCivics)
						{
							for (int iJ = 0; iJ < GC.getNumCivicInfos(); iJ++)
							{
								if (eCivic != iJ)
								{
									if (GC.getBuildingInfo(eLoopBuilding).isPrereqOrCivics(iJ))
									{
										if (isCivic((CivicTypes)iJ))
										{
											bValid = true;
											break;
										}
									}
								}
							}
							if (!bValid)
							{
								bValidCivics = false;
							}
						}
						
						if (bValidCivics && !bCanConstruct)
						{
							//Estimate value from capital city
							iValue += pCapital->AI_buildingValue(eLoopBuilding, 0) * getNumCities() / 6;
						}
						else if (!bValidCivics && bCanConstruct)
						{
							//Loses us the ability to construct the building
							iTempValue = -pCapital->AI_buildingValue(eLoopBuilding, 0) * (std::max(getNumCities(), getBuildingClassCount((BuildingClassTypes)iI) + getNumCities() / 4));
							iTempValue /= 6;
							iValue += iTempValue;
						}
					}
				}
			}
		}
	}
	
	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		BuildingTypes eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));
		if (eLoopBuilding != NO_BUILDING)
		{
			for (iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
			{
				if (kCivic.getBuildingCommerceModifier(eLoopBuilding, iJ) != 0)
				{
					int iMulitplier = (iJ == COMMERCE_GOLD && bFinancialTrouble) ? 6 : 4;
					iValue += getBuildingClassCount((BuildingClassTypes)iI) * kCivic.getBuildingCommerceModifier(eLoopBuilding, iJ) * iMulitplier;
				}
			}
		}
	}
	
	for (iJ = 0; iJ < GC.getNumImprovementInfos(); iJ++)
	{
		iValue += (8 * (kCivic.getImprovementHappinessChanges(iJ) * (getImprovementCount((ImprovementTypes)iJ) + getNumCities())));
		iValue += ((8 * (kCivic.getImprovementHealthPercentChanges(iJ) * (getImprovementCount((ImprovementTypes)iJ) + getNumCities()))) / 100);
	}
	
	iTempValue = 0;
	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
	{
		for (iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
		{
			iTempValue += ((kCivic.getSpecialistCommercePercentChanges(iI, iJ) * getTotalPopulation()) / 500);
		}
		for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
		{
			iTempValue += ((kCivic.getSpecialistYieldPercentChanges(iI, iJ) * getTotalPopulation()) / 500);
		}
	}
	iValue += iTempValue;
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		iTempValue = 0;

		iTempValue += ((kCivic.getYieldModifier(iI) * getNumCities()) / 2);
		
		if (pCapital) 
		{
			// Bureaucracy
			// Benefit of having a supercity is higher than just increases in yield since will also win more
			// wonder races, build things that much faster
			//iTempValue += ((kCivic.getCapitalYieldModifier(iI)) / 2);

			iTemp = (kCivic.getCapitalYieldModifier(iI) * pCapital->getBaseYieldRate((YieldTypes)iI));
			iTemp /= 80;
			//iTemp *= pCapital->AI_yieldMultiplier((YieldTypes)iI);
			//iTemp /= 100;
			iTempValue += iTemp;
		}
		iTempValue += ((kCivic.getTradeYieldModifier(iI) * getNumCities()) / 11);

		for (iJ = 0; iJ < GC.getNumImprovementInfos(); iJ++)
		{
			// Free Speech
			iTempValue += (AI_averageYieldMultiplier((YieldTypes)iI) * (kCivic.getImprovementYieldChanges(iJ, iI) * (getImprovementCount((ImprovementTypes)iJ) + getNumCities()))) / 100;
		}
		

		for (iJ = 0; iJ < GC.getNumTerrainInfos(); iJ++)
		{
			iTempValue += (AI_averageYieldMultiplier((YieldTypes)iI) * (kCivic.getTerrainYieldChanges(iJ, iI) * (NUM_CITY_PLOTS + getNumCities()/2))) / 100;
		}
		
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		if (iI == YIELD_FOOD) 
		{ 
			iTempValue *= 3; 
		} 
		else if (iI == YIELD_PRODUCTION) 
		{ 
			iTempValue *= ((AI_avoidScience()) ? 6 : 2); 
		} 
		else if (iI == YIELD_COMMERCE) 
		{ 
			iTempValue *= ((AI_avoidScience()) ? 2 : 4);
			iTempValue /= 3;
		} 

		iValue += iTempValue;
	}

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		iTempValue = 0;

		// Nationhood
		iTempValue += ((kCivic.getCommerceModifier(iI) * getNumCities()) / 3);
		iTempValue += (kCivic.getCapitalCommerceModifier(iI) / 2);
		if (iI == COMMERCE_ESPIONAGE)
		{
			iTempValue *= AI_getEspionageWeight();
			iTempValue /= 500;
		}

		// Representation
		iTempValue += ((kCivic.getSpecialistExtraCommerce(iI) * getTotalPopulation()) / 15);

		iTempValue *= AI_commerceWeight((CommerceTypes)iI);

		if ((iI == COMMERCE_CULTURE) && bCultureVictory2)
		{
		    iTempValue *= 2;
		    if (bCultureVictory3)
		    {
		        iTempValue *= 2;		        
		    }
		}
		iTempValue /= 100;

		iValue += iTempValue;
	}

	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		iTempValue = kCivic.getBuildingHappinessChanges(iI);
/************************************************************************************************/
/* Afforess	                  Start		 01/17/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		iTempValue += kCivic.getBuildingHealthChanges(iI);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		if (iTempValue != 0)
		{
			// Nationalism
			if( !isNationalWonderClass((BuildingClassTypes)iI) )
			{
				iValue += (iTempValue * getNumCities())/2;
			}
			iValue += (iTempValue * getBuildingClassCountPlusMaking((BuildingClassTypes)iI) * 2);
		}
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
	{
		// Environmentalism
		iHappiness = kCivic.getFeatureHappinessChanges(iI);

		if (iHappiness != 0)
		{
			iValue += (iHappiness * countCityFeatures((FeatureTypes)iI) * 5);
		}
	}

	for (iI = 0; iI < GC.getNumHurryInfos(); iI++)
	{
		if (kCivic.isHurry(iI))
		{
			iTempValue = 0;

			if (GC.getHurryInfo((HurryTypes)iI).getGoldPerProduction() > 0)
			{
				iTempValue += ((((AI_avoidScience()) ? 50 : 25) * getNumCities()) / GC.getHurryInfo((HurryTypes)iI).getGoldPerProduction());
			}
			iTempValue += (GC.getHurryInfo((HurryTypes)iI).getProductionPerPopulation() * getNumCities() * (bWarPlan ? 2 : 1)) / 5;
			iValue += iTempValue;
		}
	}

	for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); iI++)
	{
		if (kCivic.isSpecialBuildingNotRequired(iI))
		{
			iValue += ((getNumCities() / 2) + 1); // XXX
		}
	}

	for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++) 
	{ 
		iTempValue = 0; 
		if (kCivic.isSpecialistValid(iI)) 
		{ 
			iTempValue += ((getNumCities() *  (bCultureVictory3 ? 10 : 1)) + 6);
		} 
		iValue += (iTempValue / 2); 
	} 

	if (GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic() == eCivic)
	{
		if (!kCivic.isStateReligion() || iHighestReligionCount > 0)
		{
			iValue *= 5; 
			iValue /= 4; 
			iValue += 6 * getNumCities();
			iValue += 20; 
		}
	}

	if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2) && (GC.getCivicInfo(eCivic).isNoNonStateReligionSpread()))
	{
	    iValue /= 10;	    
	}
/************************************************************************************************/
/* Afforess	                  Start		 8/18/10                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/

	for (iI = 0; iI < GC.getNumFlavorTypes(); iI++)
	{
		iValue+=AI_getFlavorValue((FlavorTypes)iI)*GC.getCivicInfo(eCivic).getFlavorValue((FlavorTypes)iI);
	}
	iTempValue = 0;
	int iPlayerValue = 0;
	CivicTypes eTargetCivic;
	CivicTypes eCurrentCivic = getCivics((CivicOptionTypes)GC.getCivicInfo(eCivic).getCivicOptionType());
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		int iOurPower = std::max(1, GET_TEAM(getTeam()).getPower(true));
		int iTheirPower = std::max(1, GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).getDefensivePower());
		
		if (!GET_PLAYER((PlayerTypes)iI).isHuman() && GET_PLAYER((PlayerTypes)iI).isAlive() && (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam()) && (!GET_PLAYER((PlayerTypes)iI).isBarbarian()))
		{
			for (int iJ = 0; iJ < GC.getNumCivicOptionInfos(); iJ++)
			{
				eTargetCivic = GET_PLAYER((PlayerTypes)iI).getCivics((CivicOptionTypes)iJ);
				int iAttitudeChange = GC.getCivicInfo(eCivic).getCivicAttitudeChange(eTargetCivic) - (eCurrentCivic != NO_CIVIC ? GC.getCivicInfo(eCurrentCivic).getCivicAttitudeChange(eTargetCivic) : 0);
				//New Civic Attitude minus old civic attitude
				int iCurrentAttitude = AI_getAttitudeVal((PlayerTypes)iI);
				//We are close friends
				if (iCurrentAttitude > 5)
				{//Positive Changes are welcome, negative ones, not so much
					iPlayerValue += iAttitudeChange * 3;
				}
				//we aren't friends
				else
				{//if we aren't gearing up for a war yet...
					if (GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iI) != NO_WARPLAN)
					{//Then we would welcome some diplomatic improvements
						iPlayerValue += iAttitudeChange * 3;
						iPlayerValue /= 2;
					}
					else
					{
						//We are going to war, screw diplomacy
					}
				}
			}
			if (GET_TEAM(getTeam()).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam()))
			{//Who cares about vassals?
				iPlayerValue /= 5;
			}
			float fPowerRatio = (float)((iTheirPower)/iOurPower);
			iPlayerValue = (int)((float)iPlayerValue * fPowerRatio);
		}
		iTempValue += iPlayerValue;
		iPlayerValue = 0;
	}
	iValue += iTempValue;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

/************************************************************************************************/
/* REVOLUTION_MOD                         05/22/08                                jdog5000      */
/*                                                                                              */
/* Revolution AI                                                                                */
/************************************************************************************************/
	iValue += ( AI_RevCalcCivicRelEffect(eCivic) );
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/		

	return iValue;
}

/************************************************************************************************/
/* REVOLUTION_MOD                         05/30/08                                jdog5000      */
/*                                                                                              */
/* Revolution AI                                                                                */
/************************************************************************************************/
int CvPlayerAI::AI_RevCalcCivicRelEffect(CivicTypes eCivic) const
{
	if (isBarbarian())
		return 0;
	if(!isAlive())
		return 0;
	if(getNumCities() == 0)
		return 0;

	int iTotalScore = 0;

	if  ( GC.getCivicInfo(eCivic).isStateReligion() )
	{
		int iRelScore = 0;

		float fRelGoodMod = GC.getCivicInfo(eCivic).getRevIdxGoodReligionMod();
		float fRelBadMod = GC.getCivicInfo(eCivic).getRevIdxBadReligionMod();
		int iHolyCityGood = GC.getCivicInfo(eCivic).getRevIdxHolyCityGood();
		int iHolyCityBad = GC.getCivicInfo(eCivic).getRevIdxHolyCityBad();

		ReligionTypes eStateReligion = getStateReligion();

		if( eStateReligion == NO_RELIGION )
		{
			eStateReligion = getLastStateReligion();
		}
		if( eStateReligion == NO_RELIGION )
		{
			eStateReligion = GET_PLAYER(getID()).AI_findHighestHasReligion();
		}
		if( eStateReligion == NO_RELIGION )
		{
			return 0;
		}
		
		CvCity * pHolyCity = GC.getGame().getHolyCity(eStateReligion);

		int iLoop;
		for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			float fCityStateReligion = 0;
			float fCityNonStateReligion = 0;
			if (pLoopCity == NULL)
			{
				//logMsg("error pLoopCity is NULL");
			}
			if (pLoopCity->isHasReligion(eStateReligion))
			{
				fCityStateReligion += 4;
			}
			for ( int iI = 0; iI < GC.getNumReligionInfos(); iI++ )
			{
				if ((pLoopCity->isHasReligion((ReligionTypes)iI )) && !(eStateReligion == iI) )
				{
					if (fCityNonStateReligion <= 4 )
					{
						fCityNonStateReligion += 2.5;
					}
					else
					{
						fCityNonStateReligion += 1;
					}
				}
			}
			if (pLoopCity->isHolyCity())
			{
				if (pLoopCity->isHolyCity(eStateReligion))
				{
					fCityStateReligion += 5;
				}
				else
				{
					fCityNonStateReligion += 4;
				}
			}
			int iLiberalism = GC.getInfoTypeForString("TECH_LIBERALISM");
			int iSciMethod = GC.getInfoTypeForString("TECH_SCIENTIFIC_METHOD");
			bool bHeathens = false;
			if (!(GET_TEAM(getTeam()).isHasTech((TechTypes)iLiberalism)) && (pLoopCity->isHasReligion(eStateReligion)))
			{
				if (pHolyCity != NULL)
				{
					PlayerTypes eHolyCityOwnerID = pHolyCity->getOwner();
					if (getID() == eHolyCityOwnerID)
					{
						fCityStateReligion += iHolyCityGood;
					}
					else
					{
						if (GET_PLAYER(eHolyCityOwnerID).getStateReligion() != eStateReligion)//heathens!
						{
							bHeathens = true;
						}
					}
				}
			}

			int iRelBadEffect = (int)floor((fCityNonStateReligion * (1+fRelBadMod)) + .5);
			int iRelGoodEffect = (int)floor((fCityStateReligion * (1+fRelGoodMod)) + .5);

			if (GET_TEAM(getTeam()).getAtWarCount(true) > 0 )
			{
				iRelGoodEffect = (int)floor((iRelGoodEffect * 1.5) + .5);
			}

			int iNetCivicRelEffect = iRelBadEffect - iRelGoodEffect;
			if (bHeathens)
			{
				iNetCivicRelEffect += iHolyCityBad;
			}
			
			if (GET_TEAM(getTeam()).isHasTech((TechTypes)iSciMethod))
			{
				iNetCivicRelEffect /= 3;
			}
			else if (GET_TEAM(getTeam()).isHasTech((TechTypes)iLiberalism))
			{
				iNetCivicRelEffect /= 2;
			}
			int iRevIdx = pLoopCity->getRevolutionIndex();
			iRevIdx = std::max(iRevIdx-300,100);
			float fCityReligionScore = iNetCivicRelEffect*(((float)iRevIdx)/ 600);
			iRelScore += (int)(floor(fCityReligionScore));
		}//end of each city loop
		
		iRelScore *= 3;
		iTotalScore -= iRelScore;
	}//end of if eCivic isStateRel

	if( GC.getCivicInfo(eCivic).getNonStateReligionHappiness() > 0 )
	{
		int iCivicScore = 0;

		int iLoop;
		for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			int iCityScore = GC.getCivicInfo(eCivic).getNonStateReligionHappiness()*pLoopCity->getReligionCount();

			int iRevIdx = pLoopCity->getRevolutionIndex();
			iRevIdx = std::max(iRevIdx-300,100);
			
			iCityScore *= iRevIdx;
			iCityScore /= (pLoopCity->angryPopulation() > 0) ? 500 : 700;

			iCivicScore += iCityScore;
		}

		iTotalScore += iCivicScore;
	}
	
	return iTotalScore;
}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

ReligionTypes CvPlayerAI::AI_bestReligion() const
{
	ReligionTypes eBestReligion;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	eBestReligion = NO_RELIGION;

	ReligionTypes eFavorite = (ReligionTypes)GC.getLeaderHeadInfo(getLeaderType()).getFavoriteReligion();

	for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (canDoReligion((ReligionTypes)iI))
		{
			iValue = AI_religionValue((ReligionTypes)iI);

			if (getStateReligion() == ((ReligionTypes)iI))
			{
				iValue *= 4;
				iValue /= 3;
			}

			if (eFavorite == iI)
			{
				iValue *= 5;
				iValue /= 4;
			}

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestReligion = ((ReligionTypes)iI);
			}
		}
	}

	if ((NO_RELIGION == eBestReligion) || AI_isDoStrategy(AI_STRATEGY_MISSIONARY))
	{
		return eBestReligion;
	}
	
	int iBestCount = getHasReligionCount(eBestReligion);
	int iSpreadPercent = (iBestCount * 100) / std::max(1, getNumCities());
	int iPurityPercent = (iBestCount * 100) / std::max(1, countTotalHasReligion());
	if (iPurityPercent < 49)
	{
		if (iSpreadPercent > ((eBestReligion == eFavorite) ? 65 : 75))
		{
			if (iPurityPercent > ((eBestReligion == eFavorite) ? 25 : 32))
			{
				return eBestReligion;
			}
		}
		return NO_RELIGION;
	}
	
	return eBestReligion;
}


int CvPlayerAI::AI_religionValue(ReligionTypes eReligion) const
{
	if (getHasReligionCount(eReligion) == 0)
	{
		return 0;
	}

	int iValue = GC.getGameINLINE().countReligionLevels(eReligion);

	int iLoop;
	CvCity* pLoopCity;
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (pLoopCity->isHasReligion(eReligion))
		{
			iValue += pLoopCity->getPopulation();
		}
	}

	CvCity* pHolyCity = GC.getGameINLINE().getHolyCity(eReligion);
	if (pHolyCity != NULL)
	{
		bool bOurHolyCity = pHolyCity->getOwnerINLINE() == getID();
		bool bOurTeamHolyCity = pHolyCity->getTeam() == getTeam();

		if (bOurHolyCity || bOurTeamHolyCity)
		{
			int iCommerceCount = 0;

			for (int iI = 0; iI < GC.getNumBuildingInfos(); iI++)
			{
				if (pHolyCity->getNumActiveBuilding((BuildingTypes)iI) > 0)
				{
					for (int iJ = 0; iJ < NUM_COMMERCE_TYPES; iJ++)
					{
						if (GC.getBuildingInfo((BuildingTypes)iI).getGlobalReligionCommerce() == eReligion)
						{
							iCommerceCount += GC.getReligionInfo(eReligion).getGlobalReligionCommerce((CommerceTypes)iJ) * pHolyCity->getNumActiveBuilding((BuildingTypes)iI);
						}
					}
				}
			}

			if (bOurHolyCity)
			{
				iValue *= (3 + iCommerceCount);
				iValue /= 2;
			}
			else if (bOurTeamHolyCity)
			{
				iValue *= (4 + iCommerceCount);
				iValue /= 3;
			}
		}
	}

	return iValue;
}

/************************************************************************************************/
/* REVOLUTION_MOD                         05/22/08                                jdog5000      */
/*                                                                                              */
/* Revolution AI                                                                                */
/************************************************************************************************/
ReligionTypes CvPlayerAI::AI_findHighestHasReligion()
{
	int iValue;
	int iBestValue;
	int iI;
	ReligionTypes eMostReligion = NO_RELIGION;

	iBestValue = 0;

	for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		iValue = getHasReligionCount((ReligionTypes)iI);

		if (iValue > iBestValue)
		{
			iBestValue = iValue;
			eMostReligion = (ReligionTypes)iI;
		}
	}
	return eMostReligion;
}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      01/07/10                                jdog5000      */
/*                                                                                              */
/* Espionage AI                                                                                 */
/************************************************************************************************/
EspionageMissionTypes CvPlayerAI::AI_bestPlotEspionage(CvPlot* pSpyPlot, PlayerTypes& eTargetPlayer, CvPlot*& pPlot, int& iData) const
{
	//ooookay what missions are possible
	
	FAssert(pSpyPlot != NULL);
	
	pPlot = NULL;
	iData = -1;

	EspionageMissionTypes eBestMission = NO_ESPIONAGEMISSION;
	//int iBestValue = 0;
	int iBestValue = 20;
/************************************************************************************************/
/* Afforess	                  Start		 06/29/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (pSpyPlot->isBarbarian())
	{
		return eBestMission;
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	if (pSpyPlot->isOwned())
	{
		if (pSpyPlot->getTeam() != getTeam())
		{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       09/05/08                                jdog5000      */
/*                                                                                              */
/* Bugfix				                                                                         */
/************************************************************************************************/
/* original BTS code
			if (!AI_isDoStrategy(AI_STRATEGY_BIG_ESPIONAGE) && (GET_TEAM(getTeam()).AI_getWarPlan(pSpyPlot->getTeam()) != NO_WARPLAN || AI_getAttitudeWeight(pSpyPlot->getOwner()) < (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? 50 : 1)))
*/
			// Attitude weight < 50 is equivalent to < 1, < 51 is clearly what was intended
			if (!AI_isDoStrategy(AI_STRATEGY_BIG_ESPIONAGE) && (GET_TEAM(getTeam()).AI_getWarPlan(pSpyPlot->getTeam()) != NO_WARPLAN || AI_getAttitudeWeight(pSpyPlot->getOwner()) < (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? 51 : 1)))
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
			{
				//Destroy Improvement.
				if (pSpyPlot->getImprovementType() != NO_IMPROVEMENT)
				{
					for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
					{
						CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);

						if (kMissionInfo.isDestroyImprovement())
						{
							int iValue = AI_espionageVal(pSpyPlot->getOwnerINLINE(), (EspionageMissionTypes)iMission, pSpyPlot, -1);
							
							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								eBestMission = (EspionageMissionTypes)iMission;
								eTargetPlayer = pSpyPlot->getOwnerINLINE();
								pPlot = pSpyPlot;
								iData = -1;
							}
						}
					}
				}
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
				//RevolutionDCM start
				//Bribe
				if (pSpyPlot->plotCount(PUF_isOtherTeam, getID(), -1, NO_PLAYER, NO_TEAM, PUF_isVisible, getID()) >= 1)
				{
					if (pSpyPlot->plotCount(PUF_isUnitAIType, UNITAI_WORKER, -1) >= 1)
					{
						for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
						{
							CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);

							if (kMissionInfo.getBuyUnitCostFactor() > 0 && GC.getDefineINT("SS_BRIBE"))
							{
								int iValue = AI_espionageVal(pSpyPlot->getOwnerINLINE(), (EspionageMissionTypes)iMission, pSpyPlot, -1);
								
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestMission = (EspionageMissionTypes)iMission;
									eTargetPlayer = pSpyPlot->getOwnerINLINE();
									pPlot = pSpyPlot;
									iData = -1;
								}
							}
						}
					}
				}
			}
			// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/

/************************************************************************************************/
/* Afforess	                  Start		 06/29/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			CvCity* pCity = pSpyPlot->getPlotCity();
			if (pCity != NULL)
			{
				for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
				{
					CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);
					if (kMissionInfo.isRevolt() || kMissionInfo.isDisablePower() || kMissionInfo.getWarWearinessCounter() > 0 || kMissionInfo.getBuyCityCostFactor() > 0)
					{
						int iValue = AI_espionageVal(pSpyPlot->getOwnerINLINE(), (EspionageMissionTypes)iMission, pSpyPlot, -1);
								
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							eBestMission = (EspionageMissionTypes)iMission;
							eTargetPlayer = pSpyPlot->getOwnerINLINE();
							pPlot = pSpyPlot;
							iData = -1;
						}
					}
				}
			}
			if (pCity != NULL)
			{
				if (GET_TEAM(getTeam()).AI_getWarPlan(pCity->getTeam()) != NO_WARPLAN)
				{
					for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
					{
						CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);
						if (kMissionInfo.isNuke())
						{
							int iValue = AI_espionageVal(pSpyPlot->getOwnerINLINE(), (EspionageMissionTypes)iMission, pSpyPlot, -1);
									
							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								eBestMission = (EspionageMissionTypes)iMission;
								eTargetPlayer = pSpyPlot->getOwnerINLINE();
								pPlot = pSpyPlot;
								iData = -1;
							}
						}
					}
				}
			}
			if (pCity != NULL)
			{
				if ((pCity->plot()->countTotalCulture() / std::max(1, pCity->plot()->getCulture(getID()))) > 25)
				{
					for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
					{
						CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);
						if (kMissionInfo.getCityInsertCultureAmountFactor() > 0)
						{
							int iValue = AI_espionageVal(pSpyPlot->getOwnerINLINE(), (EspionageMissionTypes)iMission, pSpyPlot, -1);
									
							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								eBestMission = (EspionageMissionTypes)iMission;
								eTargetPlayer = pSpyPlot->getOwnerINLINE();
								pPlot = pSpyPlot;
								iData = -1;
							}
						}
					}
				}
			}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			if (pCity != NULL)
			{
				//Something malicious
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       09/05/08                                jdog5000      */
/*                                                                                              */
/* Bugfix				                                                                         */
/************************************************************************************************/
/* original BTS code
				if (AI_getAttitudeWeight(pSpyPlot->getOwner()) < (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? 50 : 1))
*/
				// Attitude weight < 50 is equivalent to < 1, < 51 is clearly what was intended
				if (AI_getAttitudeWeight(pSpyPlot->getOwner()) < (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? 51 : 1))
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
				{
					//Destroy Building.
					if (!AI_isDoStrategy(AI_STRATEGY_BIG_ESPIONAGE))
					{
						for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
						{
							CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);
							if (kMissionInfo.getDestroyBuildingCostFactor() > 0)
							{
								for (int iBuilding = 0; iBuilding < GC.getNumBuildingInfos(); iBuilding++)
								{
									BuildingTypes eBuilding = (BuildingTypes)iBuilding;
									
									if (pCity->getNumBuilding(eBuilding) > 0)
									{
										int iValue = AI_espionageVal(pSpyPlot->getOwnerINLINE(), (EspionageMissionTypes)iMission, pSpyPlot, iBuilding);
										
										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											eBestMission = (EspionageMissionTypes)iMission;
											eTargetPlayer = pSpyPlot->getOwnerINLINE();
											pPlot = pSpyPlot;
											iData = iBuilding;
										}
									}
								}
							}
						}
					}
					
					//Destroy Project
					for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
					{
						CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);
						if (kMissionInfo.getDestroyProjectCostFactor() > 0)
						{
							for (int iProject = 0; iProject < GC.getNumProjectInfos(); iProject++)
							{
								ProjectTypes eProject = (ProjectTypes)iProject;
								
								int iValue = AI_espionageVal(pSpyPlot->getOwnerINLINE(), (EspionageMissionTypes)iMission, pSpyPlot, iProject);
								
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestMission = (EspionageMissionTypes)iMission;
									eTargetPlayer = pSpyPlot->getOwnerINLINE();
									pPlot = pSpyPlot;
									iData = iProject;
								}
							}
						}
					}
					
					//General dataless city mission.
					if (!AI_isDoStrategy(AI_STRATEGY_BIG_ESPIONAGE))
					{
						for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
						{
							CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);
							{
								if ((kMissionInfo.getCityPoisonWaterCounter() > 0) || (kMissionInfo.getDestroyProductionCostFactor() > 0)
									|| (kMissionInfo.getStealTreasuryTypes() > 0))
								{
									int iValue = AI_espionageVal(pSpyPlot->getOwnerINLINE(), (EspionageMissionTypes)iMission, pSpyPlot, -1);
									
									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										eBestMission = (EspionageMissionTypes)iMission;
										eTargetPlayer = pSpyPlot->getOwnerINLINE();
										pPlot = pSpyPlot;
										iData = -1;
									}
								}
							}
						}
					}
				
					//Disruption suitable for war.
					if (GET_TEAM(getTeam()).isAtWar(pSpyPlot->getTeam()))
					{
						for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
						{
							CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);
							if ((kMissionInfo.getCityRevoltCounter() > 0) || (kMissionInfo.getPlayerAnarchyCounter() > 0))
							{
								int iValue = AI_espionageVal(pSpyPlot->getOwnerINLINE(), (EspionageMissionTypes)iMission, pSpyPlot, -1);
								
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestMission = (EspionageMissionTypes)iMission;
									eTargetPlayer = pSpyPlot->getOwnerINLINE();
									pPlot = pSpyPlot;
									iData = -1;
								}
							}
						}
					}
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
					//RevolutionDCM start
					//Assassinate 
					if (true)
					{
						for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
						{
							CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);

							if (kMissionInfo.getDestroyUnitCostFactor() > 0 && GC.getDefineINT("SS_ASSASSINATE"))
							{
								SpecialistTypes theGreatSpecialistTarget = (SpecialistTypes)0;
								
								CvCity* pCity = pSpyPlot->getPlotCity();
								if (NULL != pCity)
								{
									//loop through all great specialist types
									for (int iSpecialist = 7; iSpecialist < GC.getNumSpecialistInfos(); iSpecialist++)
									{
										SpecialistTypes tempSpecialist = (SpecialistTypes)0;
										//does this city contain this great specialist type?
										if (pCity->getFreeSpecialistCount((SpecialistTypes)iSpecialist) > 0)
										{
											//sort who is the most significant great specialist in the city
											//prefer any custom specialist	(SpecialistTypes)>13
											//then great spies				(SpecialistTypes)13
											//then great generals			(SpecialistTypes)12
											//then great engineers			(SpecialistTypes)11
											//then great merchants			(SpecialistTypes)10
											//then great scientists			(SpecialistTypes)9
											//then great artists			(SpecialistTypes)8
											//then great priests			(SpecialistTypes)7
											tempSpecialist = (SpecialistTypes)iSpecialist;
											if (tempSpecialist > theGreatSpecialistTarget)
											{
												theGreatSpecialistTarget = tempSpecialist;
											}
										}
									}
								}	
							
								int iValue = AI_espionageVal(pSpyPlot->getOwnerINLINE(), (EspionageMissionTypes)iMission, pSpyPlot, -1);
								
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestMission = (EspionageMissionTypes)iMission;
									eTargetPlayer = pSpyPlot->getOwnerINLINE();
									pPlot = pSpyPlot;
									iData = theGreatSpecialistTarget;
								}
							}
						}
					}
				}
				// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/				

				//TSHEEP - Counter Espionage (Why the heck don't AIs use this in vanilla?) - 
				//Requires either annoyance or memory of past Spy transgression
				if ((AI_getAttitudeWeight(pSpyPlot->getOwner()) < (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? 50 : 0) ||
					AI_getMemoryCount(pSpyPlot->getOwner(), MEMORY_SPY_CAUGHT) > 0) &&
					GET_TEAM(getTeam()).getCounterespionageTurnsLeftAgainstTeam(GET_PLAYER(pSpyPlot->getOwner()).getTeam()) <= 0)
				{
					for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
					{
						CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);
						if (kMissionInfo.getCounterespionageNumTurns() > 0)
						{
							int iValue = AI_espionageVal(pSpyPlot->getOwnerINLINE(), (EspionageMissionTypes)iMission, pSpyPlot, -1);
							
							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								eBestMission = (EspionageMissionTypes)iMission;
								eTargetPlayer = pSpyPlot->getOwnerINLINE();
								pPlot = pSpyPlot;
								iData = -1;
							}
						}
					}
				}
				//TSHEEP End of Counter Espionage
				
				//Steal Technology
				for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
				{
					CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);
					if (kMissionInfo.getBuyTechCostFactor() > 0)
					{
						for (int iTech = 0; iTech < GC.getNumTechInfos(); iTech++)
						{
							TechTypes eTech = (TechTypes)iTech;
							int iValue = AI_espionageVal(pSpyPlot->getOwnerINLINE(), (EspionageMissionTypes)iMission, pSpyPlot, eTech);
							
							iValue *= 2;		//Increase AI weight of techvalues TSHEEP

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								eBestMission = (EspionageMissionTypes)iMission;
								eTargetPlayer = pSpyPlot->getOwnerINLINE();
								pPlot = pSpyPlot;
								iData = eTech;
							}
						}
					}
				}
			}
		}
	}
	
	return eBestMission;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/23/09                                jdog5000      */
/*                                                                                              */
/* Espionage AI                                                                                 */
/************************************************************************************************/					
/// \brief Value of espionage mission at this plot.
///
/// Assigns value to espionage mission against ePlayer at pPlot, where iData can provide additional information about mission.
int CvPlayerAI::AI_espionageVal(PlayerTypes eTargetPlayer, EspionageMissionTypes eMission, CvPlot* pPlot, int iData) const
{
	TeamTypes eTargetTeam = GET_PLAYER(eTargetPlayer).getTeam();

	if (eTargetPlayer == NO_PLAYER)
	{
		return 0;
	}

	int iCost = getEspionageMissionCost(eMission, eTargetPlayer, pPlot, iData);

	if (!canDoEspionageMission(eMission, eTargetPlayer, pPlot, iData, NULL))
	{
		return 0;
	}

	bool bMalicious = (AI_getAttitudeWeight(pPlot->getOwner()) < (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? 51 : 1) || GET_TEAM(getTeam()).AI_getWarPlan(eTargetTeam) != NO_WARPLAN);

	int iValue = 0;
	if (bMalicious && GC.getEspionageMissionInfo(eMission).isDestroyImprovement())
	{
		if (NULL != pPlot)
		{
			if (pPlot->getOwnerINLINE() == eTargetPlayer)
			{
				ImprovementTypes eImprovement = pPlot->getImprovementType();
				if (eImprovement != NO_IMPROVEMENT)
				{
					BonusTypes eBonus = pPlot->getNonObsoleteBonusType(GET_PLAYER(eTargetPlayer).getTeam());
					if (NO_BONUS != eBonus)
					{
						iValue += GET_PLAYER(eTargetPlayer).AI_bonusVal(eBonus, -1);
						
						int iTempValue = 0;
						if (NULL != pPlot->getWorkingCity())
						{
							iTempValue += (pPlot->calculateImprovementYieldChange(eImprovement, YIELD_FOOD, pPlot->getOwnerINLINE()) * 2);
							iTempValue += (pPlot->calculateImprovementYieldChange(eImprovement, YIELD_PRODUCTION, pPlot->getOwnerINLINE()) * 1);
							iTempValue += (pPlot->calculateImprovementYieldChange(eImprovement, YIELD_COMMERCE, pPlot->getOwnerINLINE()) * 2);
							iTempValue += GC.getImprovementInfo(eImprovement).getUpgradeTime() / 2;
							iValue += iTempValue;
						}
					}
				}
			}
		}
	}

	if (bMalicious && GC.getEspionageMissionInfo(eMission).getDestroyBuildingCostFactor() > 0)
	{
		if (canSpyDestroyBuilding(eTargetPlayer, (BuildingTypes)iData))
		{
			if (NULL != pPlot)
			{
				CvCity* pCity = pPlot->getPlotCity();

				if (NULL != pCity)
				{
					if (pCity->getNumRealBuilding((BuildingTypes)iData) > 0)
					{
						CvBuildingInfo& kBuilding = GC.getBuildingInfo((BuildingTypes)iData);
						if ((kBuilding.getProductionCost() > 1) && !isWorldWonderClass((BuildingClassTypes)kBuilding.getBuildingClassType()))
						{
							// BBAI TODO: Should this be based on production cost of building?  Others are
							/*int iEspionageFlags = 0;
							iEspionageFlags |= BUILDINGFOCUS_FOOD;
							iEspionageFlags |= BUILDINGFOCUS_PRODUCTION;
							iEspionageFlags |= BUILDINGFOCUS_DEFENSE;
							iEspionageFlags |= BUILDINGFOCUS_HAPPY;
							iEspionageFlags |= BUILDINGFOCUS_HEALTHY;
							iEspionageFlags |= BUILDINGFOCUS_GOLD;
							iEspionageFlags |= BUILDINGFOCUS_RESEARCH;*/
							iValue += pCity->AI_buildingValue((BuildingTypes)iData);
						}
					}
				}
			}
		}
	}

	if (bMalicious && GC.getEspionageMissionInfo(eMission).getDestroyProjectCostFactor() > 0)
	{
		if (canSpyDestroyProject(eTargetPlayer, (ProjectTypes)iData))
		{
			CvProjectInfo& kProject = GC.getProjectInfo((ProjectTypes)iData);
			
			iValue += getProductionNeeded((ProjectTypes)iData) * ((kProject.getMaxTeamInstances() == 1) ? 3 : 2);
		}
	}

	if (bMalicious && GC.getEspionageMissionInfo(eMission).getDestroyProductionCostFactor() > 0)
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();
			FAssert(pCity != NULL);
			if (pCity != NULL)
			{
				int iTempValue = pCity->getProduction();
				if (iTempValue > 0)
				{
					if (pCity->getProductionProject() != NO_PROJECT)
					{
						CvProjectInfo& kProject = GC.getProjectInfo(pCity->getProductionProject());
						iValue += iTempValue * ((kProject.getMaxTeamInstances() == 1) ? 4 : 2);	
					}
					else if (pCity->getProductionBuilding() != NO_BUILDING)
					{
						CvBuildingInfo& kBuilding = GC.getBuildingInfo(pCity->getProductionBuilding());
						if (isWorldWonderClass((BuildingClassTypes)kBuilding.getBuildingClassType()))
						{
							iValue += 3 * iTempValue;
						}
						iValue += iTempValue;
					}
					else
					{
						iValue += iTempValue;
					}
				}
			}
		}
	}
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	// RevolutionDCM start
	if (bMalicious && GC.getEspionageMissionInfo(eMission).getDestroyUnitCostFactor() > 0)
	{
		/*
		Assassination iValues competes with:
		poisoning (64-768)
		destroy building (2-4439)
		destroy production (8-137)
		revolt (45-150?)
		counter espionage (104-112)
		steal tech (180-17080)
		*/
		SpecialistTypes theGreatSpecialistTarget = (SpecialistTypes)0;

		CvCity* pCity = pPlot->getPlotCity();
		if (NULL != pCity)
		{
			for (int iSpecialist = 7; iSpecialist < GC.getNumSpecialistInfos(); iSpecialist++)
			{
				SpecialistTypes tempSpecialist = (SpecialistTypes)0;
				if (pCity->getFreeSpecialistCount((SpecialistTypes)iSpecialist) > 0)
				{
					tempSpecialist = (SpecialistTypes)iSpecialist;
					if (tempSpecialist > theGreatSpecialistTarget)
					{
						theGreatSpecialistTarget = tempSpecialist;
					}
				}
			}
		}
		if (theGreatSpecialistTarget >= 7) 
		{
			iValue += 1000;
		}
	}
	
	if (bMalicious && GC.getEspionageMissionInfo(eMission).getBuyUnitCostFactor() > 0)
	{
		/*
		Bribe iValues compete with:
		destroy improvement (1-60)
		*/
		if (pPlot->plotCount(PUF_isOtherTeam, getID(), -1, NO_PLAYER, NO_TEAM, PUF_isVisible, getID()) >= 1)
		{
			if (pPlot->plotCount(PUF_isUnitAIType, UNITAI_WORKER, -1) >= 1)
			{
				iValue += 100;
			}
		}
	}
	//RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/


	if (bMalicious && GC.getEspionageMissionInfo(eMission).getStealTreasuryTypes() > 0)
	{
		if( pPlot != NULL && pPlot->getPlotCity() != NULL )
		{
			int iGoldStolen = (GET_PLAYER(eTargetPlayer).getGold() * GC.getEspionageMissionInfo(eMission).getStealTreasuryTypes()) / 100;
			iGoldStolen *= pPlot->getPlotCity()->getPopulation();
			iGoldStolen /= std::max(1, GET_PLAYER(eTargetPlayer).getTotalPopulation());
			iValue += ((GET_PLAYER(eTargetPlayer).AI_isFinancialTrouble() || AI_isFinancialTrouble()) ? 4 : 2) * (2 * std::max(0, iGoldStolen - iCost));
		}
	}

	if (GC.getEspionageMissionInfo(eMission).getCounterespionageNumTurns() > 0)
	{
		//iValue += 100 * GET_TEAM(getTeam()).AI_getAttitudeVal(GET_PLAYER(eTargetPlayer).getTeam());
		//TSHEEP - Make Counterespionage matter
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				iValue += std::max((100 - GET_TEAM(getTeam()).AI_getAttitudeVal(GET_PLAYER(eTargetPlayer).getTeam())) * (1 + std::max(AI_getMemoryCount(eTargetPlayer, MEMORY_SPY_CAUGHT), 0)),0);
			}
		}
		//TSHEEP End
	}

	if (bMalicious && GC.getEspionageMissionInfo(eMission).getBuyCityCostFactor() > 0)
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				iValue += AI_cityTradeVal(pCity);
/************************************************************************************************/
/* Afforess	                  Start		 06/29/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
				if (GET_PLAYER(pCity->getOwnerINLINE()).getNumCities() == 1)
				{
					iValue *= 3;
				}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			}
		}
	}

	if (bMalicious && GC.getEspionageMissionInfo(eMission).getCityInsertCultureAmountFactor() > 0)
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();
			if (NULL != pCity)
			{
				if (pCity->getOwner() != getID())
				{
					int iCultureAmount = GC.getEspionageMissionInfo(eMission).getCityInsertCultureAmountFactor() * pPlot->getCulture(getID());
					iCultureAmount /= 100;
					if (pCity->calculateCulturePercent(getID()) > 40)
					{
						iValue += iCultureAmount * 3;					
					}
				}
			}
		}
	}

	if (bMalicious && GC.getEspionageMissionInfo(eMission).getCityPoisonWaterCounter() > 0)
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				int iCityHealth = pCity->goodHealth() - pCity->badHealth(false, 0);
				int iBaseUnhealth = GC.getEspionageMissionInfo(eMission).getCityPoisonWaterCounter();

				int iAvgFoodShortage = std::max(0, iBaseUnhealth - iCityHealth) - pCity->foodDifference();
				iAvgFoodShortage += std::max(0, iBaseUnhealth/2 - iCityHealth) - pCity->foodDifference();
				
				iAvgFoodShortage /= 2;
				
				if( iAvgFoodShortage > 0 )
				{
					iValue += 8 * iAvgFoodShortage * iAvgFoodShortage;
				}
			}
		}
	}

	if (bMalicious && GC.getEspionageMissionInfo(eMission).getCityUnhappinessCounter() > 0)
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				int iCityCurAngerLevel = pCity->happyLevel() - pCity->unhappyLevel(0);
				int iBaseAnger = GC.getEspionageMissionInfo(eMission).getCityUnhappinessCounter();
				int iAvgUnhappy = iCityCurAngerLevel - iBaseAnger/2;
				
				if (iAvgUnhappy < 0)
				{
					iValue += 14 * abs(iAvgUnhappy) * iBaseAnger;
				}
			}
		}
	}

	if (bMalicious && GC.getEspionageMissionInfo(eMission).getCityRevoltCounter() > 0)
	{
		// Handled else where
	}

	if (GC.getEspionageMissionInfo(eMission).getBuyTechCostFactor() > 0)
	{
		if (iCost < GET_TEAM(getTeam()).getResearchLeft((TechTypes)iData) * 4 / 3)
		{
			int iTempValue = GET_TEAM(getTeam()).AI_techTradeVal((TechTypes)iData, GET_PLAYER(eTargetPlayer).getTeam());

			if( GET_TEAM(getTeam()).getBestKnownTechScorePercent() < 85 )
			{
				iTempValue *= 2;
			}

			iValue += iTempValue;
		}
	}

	if (bMalicious && GC.getEspionageMissionInfo(eMission).getSwitchCivicCostFactor() > 0)
	{
		iValue += AI_civicTradeVal((CivicTypes)iData, eTargetPlayer);
	}

	if (bMalicious && GC.getEspionageMissionInfo(eMission).getSwitchReligionCostFactor() > 0)
	{
		iValue += AI_religionTradeVal((ReligionTypes)iData, eTargetPlayer);
	}

	if (bMalicious && GC.getEspionageMissionInfo(eMission).getPlayerAnarchyCounter() > 0)
	{
/************************************************************************************************/
/* Afforess	                  Start		 06/29/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		iValue += GC.getEspionageMissionInfo(eMission).getPlayerAnarchyCounter() * 40;
	}
	
	if (bMalicious && GC.getEspionageMissionInfo(eMission).isRevolt())
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				int iCurRevStatus = pCity->getRevolutionIndex();
				iValue += std::max(300, 300+ (iCurRevStatus / 5));
			}
		}
	}
	if (bMalicious && GC.getEspionageMissionInfo(eMission).isNuke())
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				int iTempValue = 1;
				int iRange = 1;

				iTempValue += GC.getGameINLINE().getSorenRandNum((pCity->getPopulation() + 1), "AI Nuke City Value");
				iTempValue += std::max(0, pCity->getPopulation() - 10);

				iTempValue += ((pCity->getPopulation() * (100 + pCity->calculateCulturePercent(pCity->getOwnerINLINE()))) / 100);

				iTempValue += AI_getAttitudeVal(pCity->getOwnerINLINE()) / 3;

				for (int iDX = -(iRange); iDX <= iRange; iDX++)
				{
					for (int iDY = -(iRange); iDY <= iRange; iDY++)
					{
						CvPlot* pLoopPlot = plotXY(pCity->getX_INLINE(), pCity->getY_INLINE(), iDX, iDY);

						if (pLoopPlot != NULL)
						{
							if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
							{
								iTempValue++;
							}
							if (pLoopPlot->getNonObsoleteBonusType(getTeam()) != NO_BONUS)
							{
								iTempValue++;
							}
						}
					}
				}
				if (!(pCity->isEverOwned(getID())))
				{
					iTempValue *= 3;
					iTempValue /= 2;
				}
				if (!GET_TEAM(pCity->getTeam()).isAVassal())
				{
					iTempValue *= 2;
				}
				if (pCity->plot()->isVisible(getTeam(), false))
				{
					iTempValue += 2 * pCity->plot()->getNumVisibleUnits(getID());
				}
				else
				{
					iTempValue += 6;
				}
				
				iValue += iTempValue * 10;
			}
		}
	}
	if (bMalicious && GC.getEspionageMissionInfo(eMission).isDisablePower())
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				int iTempValue = 0;
				for (int iI = 0; iI < GC.getNumBuildingInfos(); iI++)
				{
					if (pCity->getNumRealBuilding((BuildingTypes)iI) > 0)
					{
						if (GC.getBuildingInfo((BuildingTypes)iI).isPrereqPower())
						{
							iTempValue += 20;
						}
						for (int iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
						{
							iTempValue += GC.getBuildingInfo((BuildingTypes)iI).getPowerYieldModifier(iJ);
						}
					}
				}
				iValue += iTempValue;
			}
		}
	}
	if (bMalicious && GC.getEspionageMissionInfo(eMission).getWarWearinessCounter() > 0)
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				int iCityCurAngerLevel = pCity->happyLevel() - pCity->unhappyLevel(0);
				int iBaseAnger = pCity->getWarWearinessPercentAnger();
				iBaseAnger *= (100 + GC.getEspionageMissionInfo(eMission).getWarWearinessCounter());
				iBaseAnger /= 100;
				iBaseAnger -= pCity->getWarWearinessPercentAnger();
				int iAvgUnhappy = iCityCurAngerLevel - iBaseAnger/2;
				
				if (iAvgUnhappy < 0)
				{
					iValue += 14 * abs(iAvgUnhappy) * iBaseAnger;
				}
			}
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


	return iValue;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/


int CvPlayerAI::AI_getPeaceWeight() const
{
	return m_iPeaceWeight;
}


void CvPlayerAI::AI_setPeaceWeight(int iNewValue)
{
	m_iPeaceWeight = iNewValue;
}

int CvPlayerAI::AI_getEspionageWeight() const
{
	if (GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
	{
		return 0;
	}
	return m_iEspionageWeight;
}

void CvPlayerAI::AI_setEspionageWeight(int iNewValue)
{
	m_iEspionageWeight = iNewValue;
}


int CvPlayerAI::AI_getAttackOddsChange() const
{
	return m_iAttackOddsChange;
}


void CvPlayerAI::AI_setAttackOddsChange(int iNewValue)
{
	m_iAttackOddsChange = iNewValue;
}


int CvPlayerAI::AI_getCivicTimer() const
{
	return m_iCivicTimer;
}


void CvPlayerAI::AI_setCivicTimer(int iNewValue)
{
	m_iCivicTimer = iNewValue;
	FAssert(AI_getCivicTimer() >= 0);
}


void CvPlayerAI::AI_changeCivicTimer(int iChange)
{
	AI_setCivicTimer(AI_getCivicTimer() + iChange);
}


int CvPlayerAI::AI_getReligionTimer() const
{
	return m_iReligionTimer;
}


void CvPlayerAI::AI_setReligionTimer(int iNewValue)
{
	m_iReligionTimer = iNewValue;
	FAssert(AI_getReligionTimer() >= 0);
}


void CvPlayerAI::AI_changeReligionTimer(int iChange)
{
	AI_setReligionTimer(AI_getReligionTimer() + iChange);
}

int CvPlayerAI::AI_getExtraGoldTarget() const
{
	return m_iExtraGoldTarget;
}

void CvPlayerAI::AI_setExtraGoldTarget(int iNewValue)
{
	m_iExtraGoldTarget = iNewValue;
}

int CvPlayerAI::AI_getNumTrainAIUnits(UnitAITypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_UNITAI_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiNumTrainAIUnits[eIndex];
}


void CvPlayerAI::AI_changeNumTrainAIUnits(UnitAITypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_UNITAI_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiNumTrainAIUnits[eIndex] = (m_aiNumTrainAIUnits[eIndex] + iChange);
	FAssert(AI_getNumTrainAIUnits(eIndex) >= 0);
}


int CvPlayerAI::AI_getNumAIUnits(UnitAITypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_UNITAI_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiNumAIUnits[eIndex];
}


void CvPlayerAI::AI_changeNumAIUnits(UnitAITypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_UNITAI_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiNumAIUnits[eIndex] = (m_aiNumAIUnits[eIndex] + iChange);
	FAssert(AI_getNumAIUnits(eIndex) >= 0);
}


int CvPlayerAI::AI_getSameReligionCounter(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiSameReligionCounter[eIndex];
}


void CvPlayerAI::AI_changeSameReligionCounter(PlayerTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiSameReligionCounter[eIndex] = (m_aiSameReligionCounter[eIndex] + iChange);
	FAssert(AI_getSameReligionCounter(eIndex) >= 0);
}


int CvPlayerAI::AI_getDifferentReligionCounter(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiDifferentReligionCounter[eIndex];
}


void CvPlayerAI::AI_changeDifferentReligionCounter(PlayerTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiDifferentReligionCounter[eIndex] = (m_aiDifferentReligionCounter[eIndex] + iChange);
	FAssert(AI_getDifferentReligionCounter(eIndex) >= 0);
}


int CvPlayerAI::AI_getFavoriteCivicCounter(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiFavoriteCivicCounter[eIndex];
}


void CvPlayerAI::AI_changeFavoriteCivicCounter(PlayerTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiFavoriteCivicCounter[eIndex] = (m_aiFavoriteCivicCounter[eIndex] + iChange);
	FAssert(AI_getFavoriteCivicCounter(eIndex) >= 0);
}


int CvPlayerAI::AI_getBonusTradeCounter(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiBonusTradeCounter[eIndex];
}


void CvPlayerAI::AI_changeBonusTradeCounter(PlayerTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiBonusTradeCounter[eIndex] = (m_aiBonusTradeCounter[eIndex] + iChange);
	FAssert(AI_getBonusTradeCounter(eIndex) >= 0);
}


int CvPlayerAI::AI_getPeacetimeTradeValue(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiPeacetimeTradeValue[eIndex];
}


void CvPlayerAI::AI_changePeacetimeTradeValue(PlayerTypes eIndex, int iChange)
{
	PROFILE_FUNC();

	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/03/09                       poyuzhe & jdog5000     */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
		// From Sanguo Mod Performance, ie the CAR Mod
		// Attitude cache
		AI_invalidateAttitudeCache(eIndex);
		GET_PLAYER(eIndex).AI_invalidateAttitudeCache(getID());
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

		m_aiPeacetimeTradeValue[eIndex] = (m_aiPeacetimeTradeValue[eIndex] + iChange);
		FAssert(AI_getPeacetimeTradeValue(eIndex) >= 0);

		FAssert(iChange > 0);

		if (iChange > 0)
		{
			if (GET_PLAYER(eIndex).getTeam() != getTeam())
			{
				for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
				{
					if (GET_TEAM((TeamTypes)iI).isAlive())
					{
						if (GET_TEAM((TeamTypes)iI).AI_getWorstEnemy() == getTeam())
						{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       03/02/10                                Sephi         */
/*                                                                                              */
/* Bug fix                                                                                      */
/************************************************************************************************/
/* orig bts code
							GET_TEAM((TeamTypes)iI).AI_changeEnemyPeacetimeTradeValue(GET_PLAYER(eIndex).getTeam(), iChange);
*/
							//make sure that if A trades with B and A is C's worst enemy, C is only mad at B if C has met B before
							//A = this
							//B = eIndex
                            //C = (TeamTypes)iI
							if (GET_TEAM((TeamTypes)iI).isHasMet(GET_PLAYER(eIndex).getTeam()))
							{
								GET_TEAM((TeamTypes)iI).AI_changeEnemyPeacetimeTradeValue(GET_PLAYER(eIndex).getTeam(), iChange);
							}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
						}
					}
				}
			}
		}
	}
}


int CvPlayerAI::AI_getPeacetimeGrantValue(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiPeacetimeGrantValue[eIndex];
}


void CvPlayerAI::AI_changePeacetimeGrantValue(PlayerTypes eIndex, int iChange)
{
	PROFILE_FUNC();

	int iI;

	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_aiPeacetimeGrantValue[eIndex] = (m_aiPeacetimeGrantValue[eIndex] + iChange);
		FAssert(AI_getPeacetimeGrantValue(eIndex) >= 0);

		FAssert(iChange > 0);

		if (iChange > 0)
		{
			if (GET_PLAYER(eIndex).getTeam() != getTeam())
			{
				for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
				{
					if (GET_TEAM((TeamTypes)iI).isAlive())
					{
						if (GET_TEAM((TeamTypes)iI).AI_getWorstEnemy() == getTeam())
						{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       03/02/10                                Sephi         */
/*                                                                                              */
/* Bug fix                                                                                      */
/************************************************************************************************/
/* orig bts code
							GET_TEAM((TeamTypes)iI).AI_changeEnemyPeacetimeGrantValue(GET_PLAYER(eIndex).getTeam(), iChange);
*/
							//make sure that if A trades with B and A is C's worst enemy, C is only mad at B if C has met B before
							//A = this
							//B = eIndex
							//C = (TeamTypes)iI
							if (GET_TEAM((TeamTypes)iI).isHasMet(GET_PLAYER(eIndex).getTeam()))
							{
								GET_TEAM((TeamTypes)iI).AI_changeEnemyPeacetimeGrantValue(GET_PLAYER(eIndex).getTeam(), iChange);
							}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
						}
					}
				}
			}
		}
	}
}


int CvPlayerAI::AI_getGoldTradedTo(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiGoldTradedTo[eIndex];
}


void CvPlayerAI::AI_changeGoldTradedTo(PlayerTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiGoldTradedTo[eIndex] = (m_aiGoldTradedTo[eIndex] + iChange);
	FAssert(AI_getGoldTradedTo(eIndex) >= 0);
}


int CvPlayerAI::AI_getAttitudeExtra(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiAttitudeExtra[eIndex];
}


void CvPlayerAI::AI_setAttitudeExtra(PlayerTypes eIndex, int iNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/03/09                       poyuzhe & jdog5000     */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
	// From Sanguo Mod Performance, ie the CAR Mod
	// Attitude cache
	if (m_aiAttitudeExtra[eIndex] != iNewValue)
	{
		GET_PLAYER(getID()).AI_invalidateAttitudeCache(eIndex);
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	m_aiAttitudeExtra[eIndex] = iNewValue;
}


void CvPlayerAI::AI_changeAttitudeExtra(PlayerTypes eIndex, int iChange)
{
	AI_setAttitudeExtra(eIndex, (AI_getAttitudeExtra(eIndex) + iChange));
}


bool CvPlayerAI::AI_isFirstContact(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_abFirstContact[eIndex];
}


void CvPlayerAI::AI_setFirstContact(PlayerTypes eIndex, bool bNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_abFirstContact[eIndex] = bNewValue;
}


int CvPlayerAI::AI_getContactTimer(PlayerTypes eIndex1, ContactTypes eIndex2) const
{
	FAssertMsg(eIndex1 >= 0, "eIndex1 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex1 < MAX_PLAYERS, "eIndex1 is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eIndex2 >= 0, "eIndex2 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex2 < NUM_CONTACT_TYPES, "eIndex2 is expected to be within maximum bounds (invalid Index)");
	return m_aaiContactTimer[eIndex1][eIndex2];
}


void CvPlayerAI::AI_changeContactTimer(PlayerTypes eIndex1, ContactTypes eIndex2, int iChange)
{
	FAssertMsg(eIndex1 >= 0, "eIndex1 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex1 < MAX_PLAYERS, "eIndex1 is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eIndex2 >= 0, "eIndex2 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex2 < NUM_CONTACT_TYPES, "eIndex2 is expected to be within maximum bounds (invalid Index)");
	m_aaiContactTimer[eIndex1][eIndex2] = (AI_getContactTimer(eIndex1, eIndex2) + iChange);
	FAssert(AI_getContactTimer(eIndex1, eIndex2) >= 0);
}


int CvPlayerAI::AI_getMemoryCount(PlayerTypes eIndex1, MemoryTypes eIndex2) const
{
	FAssertMsg(eIndex1 >= 0, "eIndex1 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex1 < MAX_PLAYERS, "eIndex1 is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eIndex2 >= 0, "eIndex2 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex2 < NUM_MEMORY_TYPES, "eIndex2 is expected to be within maximum bounds (invalid Index)");
	return m_aaiMemoryCount[eIndex1][eIndex2];
}


void CvPlayerAI::AI_changeMemoryCount(PlayerTypes eIndex1, MemoryTypes eIndex2, int iChange)
{
	FAssertMsg(eIndex1 >= 0, "eIndex1 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex1 < MAX_PLAYERS, "eIndex1 is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eIndex2 >= 0, "eIndex2 is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex2 < NUM_MEMORY_TYPES, "eIndex2 is expected to be within maximum bounds (invalid Index)");
	m_aaiMemoryCount[eIndex1][eIndex2] += iChange;
// BUG - Update Attitude Icons - start
	if (eIndex1 == GC.getGameINLINE().getActivePlayer())
	{
		gDLL->getInterfaceIFace()->setDirty(Score_DIRTY_BIT, true);
	}
// BUG - Update Attitude Icons - end
	FAssert(AI_getMemoryCount(eIndex1, eIndex2) >= 0);
}

int CvPlayerAI::AI_calculateGoldenAgeValue() const
{
    int iValue;
    int iTempValue;
    int iI;

    iValue = 0;
    for (iI = 0; iI <  NUM_YIELD_TYPES; ++iI)
    {
        iTempValue = (GC.getYieldInfo((YieldTypes)iI).getGoldenAgeYield() * AI_yieldWeight((YieldTypes)iI));
        iTempValue /= std::max(1, (1 + GC.getYieldInfo((YieldTypes)iI).getGoldenAgeYieldThreshold()));
        iValue += iTempValue;
    }

    iValue *= getTotalPopulation();
    iValue *= GC.getGameINLINE().goldenAgeLength();
    iValue /= 100;
	
/************************************************************************************************/
/* Afforess	                  Start		 02/033/10                                              */
/*                                                                                              */
/* Do Golden Ages during a Dark Age to end it early                                             */
/************************************************************************************************/
	if (isDarkAge())
		iValue *= 2;
		
//Golden Ages Reduce Revolutions
	if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_REVOLUTION))
	{
		int iLoop;
		CvCity* pLoopCity;
		int iNationalRevIndex = 0;
		for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			iNationalRevIndex += pLoopCity->getRevolutionIndex();
		}
		iNationalRevIndex /= std::max(1, getNumCities());
		
		iValue *= std::max(1, iNationalRevIndex / 500);
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


    return iValue;
}

// Protected Functions...

void CvPlayerAI::AI_doCounter()
{
	int iBonusImports;
	int iI, iJ;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
			{
				if (GET_TEAM(getTeam()).isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))
				{
					if ((getStateReligion() != NO_RELIGION) &&
						  (getStateReligion() == GET_PLAYER((PlayerTypes)iI).getStateReligion()))
					{
						AI_changeSameReligionCounter(((PlayerTypes)iI), 1);
					}
					else
					{
						if (AI_getSameReligionCounter((PlayerTypes)iI) > 0)
						{
							AI_changeSameReligionCounter(((PlayerTypes)iI), -1);
						}
					}

					if ((getStateReligion() != NO_RELIGION) &&
						  (GET_PLAYER((PlayerTypes)iI).getStateReligion() != NO_RELIGION) &&
						  (getStateReligion() != GET_PLAYER((PlayerTypes)iI).getStateReligion()))
					{
						AI_changeDifferentReligionCounter(((PlayerTypes)iI), 1);
					}
					else
					{
						if (AI_getDifferentReligionCounter((PlayerTypes)iI) > 0)
						{
							AI_changeDifferentReligionCounter(((PlayerTypes)iI), -1);
						}
					}

					if (GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic() != NO_CIVIC)
					{
						if (isCivic((CivicTypes)(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic())) &&
							  GET_PLAYER((PlayerTypes)iI).isCivic((CivicTypes)(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic())))
						{
							AI_changeFavoriteCivicCounter(((PlayerTypes)iI), 1);
						}
						else
						{
							if (AI_getFavoriteCivicCounter((PlayerTypes)iI) > 0)
							{
								AI_changeFavoriteCivicCounter(((PlayerTypes)iI), -1);
							}
						}
					}

					iBonusImports = getNumTradeBonusImports((PlayerTypes)iI);

					if (iBonusImports > 0)
					{
						AI_changeBonusTradeCounter(((PlayerTypes)iI), iBonusImports);
					}
					else
					{
						AI_changeBonusTradeCounter(((PlayerTypes)iI), -(std::min(AI_getBonusTradeCounter((PlayerTypes)iI), ((GET_PLAYER((PlayerTypes)iI).getNumCities() / 4) + 1))));
					}
				}
			}
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			for (iJ = 0; iJ < NUM_CONTACT_TYPES; iJ++)
			{
				if (AI_getContactTimer(((PlayerTypes)iI), ((ContactTypes)iJ)) > 0)
				{
					AI_changeContactTimer(((PlayerTypes)iI), ((ContactTypes)iJ), -1);
				}
			}
		}
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			for (iJ = 0; iJ < NUM_MEMORY_TYPES; iJ++)
			{
				if (AI_getMemoryCount(((PlayerTypes)iI), ((MemoryTypes)iJ)) > 0)
				{
					if (GC.getLeaderHeadInfo(getPersonalityType()).getMemoryDecayRand(iJ) > 0)
					{
						/************************************************************************************************/
						/* Afforess	                  Start		 02/19/10                                               */
						/*                                                                                              */
						/* Ruthless AI: Harder for the AI to forget past wrongs.                                        */
						/************************************************************************************************/
						int iRand = GC.getLeaderHeadInfo(getPersonalityType()).getMemoryDecayRand(iJ);
						if (GC.getGameINLINE().isModderGameOption(MODDERGAMEOPTION_REALISTIC_DIPLOMACY))
						{
							iRand /= 3;
							
							iRand /= AI_getMemoryCount(((PlayerTypes)iI), ((MemoryTypes)iJ));
						}
						if (GC.getGameINLINE().isOption(GAMEOPTION_RUTHLESS_AI))
						{
							iRand *= 2;
						}
						if (GC.getGameINLINE().getSorenRandNum(iRand, "Memory Decay") == 0)
						{
						/************************************************************************************************/
						/* Afforess	                     END                                                            */
						/************************************************************************************************/
							AI_changeMemoryCount(((PlayerTypes)iI), ((MemoryTypes)iJ), -1);
						}
					}
				}
			}
		}
	}
}


void CvPlayerAI::AI_doMilitary()
{
	if (GET_TEAM(getTeam()).getAnyWarPlanCount(true) == 0)
	{
		while (AI_isFinancialTrouble() && (calculateUnitCost() > 0))
		{
			if (!AI_disbandUnit(1, false))
			{
				break;
			}
		}
	}
	
	

	AI_setAttackOddsChange(GC.getLeaderHeadInfo(getPersonalityType()).getBaseAttackOddsChange() +
		GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getAttackOddsChangeRand(), "AI Attack Odds Change #1") +
		GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getAttackOddsChangeRand(), "AI Attack Odds Change #2"));
}


void CvPlayerAI::AI_doResearch()
{
	FAssertMsg(!isHuman(), "isHuman did not return false as expected");

	if (getCurrentResearch() == NO_TECH)
	{
		AI_chooseResearch();
		AI_forceUpdateStrategies(); //to account for current research.
	}
}


void CvPlayerAI::AI_doCommerce()
{
	CvCity* pLoopCity;
	int iIdealPercent;
	int iGoldTarget;
	int iLoop;

	FAssertMsg(!isHuman(), "isHuman did not return false as expected");

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      07/20/09                                jdog5000      */
/*                                                                                              */
/* Barbarian AI, efficiency                                                                     */
/************************************************************************************************/
	if( isBarbarian() || getAnarchyTurns() > 0)
	{
		return;
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	iGoldTarget = AI_goldTarget();
	int iTargetTurns = 4 * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent();
	iTargetTurns /= 100;
	iTargetTurns = std::max(3, iTargetTurns);

    if (isCommerceFlexible(COMMERCE_RESEARCH) && !AI_avoidScience())
	{
        // set research rate to 100%
		setCommercePercent(COMMERCE_RESEARCH, 100);
		
		// if the gold rate is under 0 at 90% research
		int iGoldRate = calculateGoldRate();
		if (iGoldRate < 0)
		{
			TechTypes eCurrentResearch = getCurrentResearch();
			if (eCurrentResearch != NO_TECH)
			{
				int iResearchTurnsLeft = getResearchTurnsLeft(eCurrentResearch, true);
				
				// if we can finish the current research without running out of gold, let us spend 2/3rds of our gold 
				if (getGold() >= iResearchTurnsLeft * iGoldRate)
				{
					iGoldTarget /= 3;
				}
			}
		}
	}

	bool bReset = false;

	if (isCommerceFlexible(COMMERCE_CULTURE))
	{
		if (getCommercePercent(COMMERCE_CULTURE) > 0)
		{
			setCommercePercent(COMMERCE_CULTURE, 0);

			bReset = true;
		}
	}

	if (isCommerceFlexible(COMMERCE_ESPIONAGE))
	{
/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						9/6/08				jdog5000	    */
/* 																			    */
/* 	Espionage AI															    */
/********************************************************************************/
		/* original BTS code
		if (getCommercePercent(COMMERCE_ESPIONAGE) > 0)
		{
			setCommercePercent(COMMERCE_ESPIONAGE, 0);

			for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; ++iTeam)
			{
				setEspionageSpendingWeightAgainstTeam((TeamTypes)iTeam, 0);
			}

			bReset = true;
		}
		*/
		
		// Reset espionage spending always
		for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; ++iTeam)
		{
			setEspionageSpendingWeightAgainstTeam((TeamTypes)iTeam, 0);
		}

		if (getCommercePercent(COMMERCE_ESPIONAGE) > 0)
		{
			setCommercePercent(COMMERCE_ESPIONAGE, 0);

			bReset = true;
		}
/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						END								    */
/********************************************************************************/
	}

	if (bReset)
	{
		AI_assignWorkingPlots();
	}

	bool bFirstTech = AI_isFirstTech(getCurrentResearch());
	if (isCommerceFlexible(COMMERCE_CULTURE))
	{
		if (getNumCities() > 0)
		{
			iIdealPercent = 0;

			for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
			{
				if (pLoopCity->getCommerceHappinessPer(COMMERCE_CULTURE) > 0)
				{
					iIdealPercent += ((pLoopCity->angryPopulation() * 100) / pLoopCity->getCommerceHappinessPer(COMMERCE_CULTURE));
				}
			}

			iIdealPercent /= getNumCities();

			iIdealPercent -= (iIdealPercent % GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));

			iIdealPercent = std::min(iIdealPercent, 20);

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
			if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE4))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
			{
			    iIdealPercent = 100;
			}

			setCommercePercent(COMMERCE_CULTURE, iIdealPercent);
		}
	}

	if (isCommerceFlexible(COMMERCE_RESEARCH))
	{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
		if ((isNoResearchAvailable() || AI_isDoVictoryStrategy(AI_VICTORY_CULTURE4)) && !bFirstTech)
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		{
			setCommercePercent(COMMERCE_RESEARCH, 0);
		}
		else
		{
			while (calculateGoldRate() > 0)
			{
				changeCommercePercent(COMMERCE_RESEARCH, GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));

				if (getCommercePercent(COMMERCE_RESEARCH) == 100)
				{
					break;
				}
			}

			if (getGold() + iTargetTurns * calculateGoldRate() < iGoldTarget)
			{
				while (getGold() + iTargetTurns * calculateGoldRate() <= iGoldTarget)
				{
					changeCommercePercent(COMMERCE_RESEARCH, -(GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS")));

					if ((getCommercePercent(COMMERCE_RESEARCH) == 0))
					{
						break;
					}
				}
			}
			else
			{
				if (AI_avoidScience())
				{
					changeCommercePercent(COMMERCE_RESEARCH, -(GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS")));
				}
			}

			if ((GET_TEAM(getTeam()).getChosenWarCount(true) > 0) || (GET_TEAM(getTeam()).getWarPlanCount(WARPLAN_ATTACKED_RECENT, true) > 0))
			{
				changeCommercePercent(COMMERCE_RESEARCH, -(GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS")));
			}

			if ((getCommercePercent(COMMERCE_RESEARCH) == 0) && (calculateGoldRate() > 0))
			{
				setCommercePercent(COMMERCE_RESEARCH, GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));
			}
		}
	}

	if (isCommerceFlexible(COMMERCE_ESPIONAGE) && !bFirstTech)
	{
/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						9/7/08				jdog5000	    */
/* 																			    */
/* 	Espionage AI															    */
/********************************************************************************/
		// original BTS code
		/*
		int iEspionageTargetRate = 0;
		//TSHEEP Additional variables for targetting tech leader
		int iTechTeam = MAX_CIV_TEAMS;
		int iTechScore = 0;
		int iTechHighScore = 0;
		//TSHEEP End

		for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; ++iTeam)
		{
			CvTeam& kLoopTeam = GET_TEAM((TeamTypes)iTeam);
			if (kLoopTeam.isAlive() && iTeam != getTeam() && !kLoopTeam.isVassal(getTeam()) && !GET_TEAM(getTeam()).isVassal((TeamTypes)iTeam))
			{
				int iTarget = (kLoopTeam.getEspionagePointsAgainstTeam(getTeam()) - GET_TEAM(getTeam()).getEspionagePointsAgainstTeam((TeamTypes)iTeam)) / 8;

				iTarget -= GET_TEAM(getTeam()).AI_getAttitudeVal((TeamTypes)iTeam);

				if (iTarget > 0)
				{
					iEspionageTargetRate += iTarget;
					changeEspionageSpendingWeightAgainstTeam((TeamTypes)iTeam, iTarget);
				}
			}
		}
		*/
		
		int iEspionageTargetRate = 0;
		int* piTarget = new int[MAX_CIV_TEAMS];
		int* piWeight = new int[MAX_CIV_TEAMS];

		for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; ++iTeam)
		{
			piTarget[iTeam] = 0;
			piWeight[iTeam] = 0;

			CvTeam& kLoopTeam = GET_TEAM((TeamTypes)iTeam);
			if (kLoopTeam.isAlive() && iTeam != getTeam() && !kLoopTeam.isVassal(getTeam()) && !GET_TEAM(getTeam()).isVassal((TeamTypes)iTeam))
			{
				if( GET_TEAM(getTeam()).isHasMet((TeamTypes)iTeam) )
				{	
					int iTheirEspPoints = kLoopTeam.getEspionagePointsAgainstTeam(getTeam());
					int iOurEspPoints = GET_TEAM(getTeam()).getEspionagePointsAgainstTeam((TeamTypes)iTeam);
					int iDesiredMissionPoints = 0;
					int iDesiredEspPoints = 0;
					
					piWeight[iTeam] = 10;
					int iRateDivisor = 12;

					if( GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iTeam) != NO_WARPLAN )
					{
						iTheirEspPoints *= 3;
						iTheirEspPoints /= 2;

						for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
						{
							CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);
							
							if( kMissionInfo.isPassive() )
							{
								if( kMissionInfo.isSeeDemographics() || kMissionInfo.isSeeResearch() )
								{
									int iMissionCost = (11*getEspionageMissionCost((EspionageMissionTypes)iMission, GET_TEAM((TeamTypes)iTeam).getLeaderID(), NULL, -1, NULL))/10;
									if( iDesiredMissionPoints < iMissionCost )
									{
										iDesiredMissionPoints = iMissionCost;
									}
								}
							}
						}

						iRateDivisor = 10;
						piWeight[iTeam] = 20;

						if( GET_TEAM(getTeam()).AI_hasCitiesInPrimaryArea((TeamTypes)iTeam) )
						{
							piWeight[iTeam] = 30;
							iRateDivisor = 8;
						}
					}
					else
					{
						int iAttitude = range(GET_TEAM(getTeam()).AI_getAttitudeVal((TeamTypes)iTeam), -12, 12);

						iTheirEspPoints -= (iTheirEspPoints*iAttitude)/(2*12);

						if( iAttitude <= -3 )
						{
							for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
							{
								CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);
								
								if( kMissionInfo.isPassive() )
								{
									if( kMissionInfo.isSeeDemographics() || kMissionInfo.isSeeResearch() )
									{
										int iMissionCost = (11*getEspionageMissionCost((EspionageMissionTypes)iMission, GET_TEAM((TeamTypes)iTeam).getLeaderID(), NULL, -1, NULL))/10;
										if( iDesiredMissionPoints < iMissionCost )
										{
											iDesiredMissionPoints = iMissionCost;
										}
									}
								}
							}
						}
						else if( iAttitude < 3 )
						{
							for (int iMission = 0; iMission < GC.getNumEspionageMissionInfos(); ++iMission)
							{
								CvEspionageMissionInfo& kMissionInfo = GC.getEspionageMissionInfo((EspionageMissionTypes)iMission);
								
								if( kMissionInfo.isPassive() )
								{
									if( kMissionInfo.isSeeDemographics() )
									{
										int iMissionCost = (11*getEspionageMissionCost((EspionageMissionTypes)iMission, GET_TEAM((TeamTypes)iTeam).getLeaderID(), NULL, -1, NULL))/10;
										if( iDesiredMissionPoints < iMissionCost )
										{
											iDesiredMissionPoints = iMissionCost;
										}
									}
								}
							}
						}

						iRateDivisor += (iAttitude/5);
						piWeight[iTeam] -= (iAttitude/2);
					}

					iDesiredEspPoints = std::max(iTheirEspPoints,iDesiredMissionPoints);

					piTarget[iTeam] = (iDesiredEspPoints - iOurEspPoints)/std::max(6,iRateDivisor);

					if( piTarget[iTeam] > 0 )
					{
						iEspionageTargetRate += piTarget[iTeam];
					}
				}
			}
		}

		for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; ++iTeam)
		{
			if( piTarget[iTeam] > 0 )
			{
				piWeight[iTeam] += (150*piTarget[iTeam])/std::max(4,iEspionageTargetRate);
			}
			else if( piTarget[iTeam] < 0 )
			{
				piWeight[iTeam] += 2*piTarget[iTeam];
			}	
			setEspionageSpendingWeightAgainstTeam((TeamTypes)iTeam, std::max(0,piWeight[iTeam]));
		}
		SAFE_DELETE(piTarget);
		SAFE_DELETE(piWeight);
/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						END								    */
/********************************************************************************/

		//if economy is weak, neglect espionage spending.
		//instead invest hammers into espionage via spies/builds
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/08/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
		if (AI_isFinancialTrouble() || AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		{
			//can still get trickle espionage income
			iEspionageTargetRate = 0;
		}
		else
		{
			iEspionageTargetRate *= (110 - getCommercePercent(COMMERCE_GOLD) * 2);
			iEspionageTargetRate /= 110;

			iEspionageTargetRate *= GC.getLeaderHeadInfo(getLeaderType()).getEspionageWeight();
			iEspionageTargetRate /= 100;

			int iInitialResearchPercent = getCommercePercent(COMMERCE_RESEARCH);
/************************************************************************************************/
/* Afforess	                  Start		 06/28/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
			while (getCommerceRate(COMMERCE_ESPIONAGE) < iEspionageTargetRate && getCommercePercent(COMMERCE_ESPIONAGE) < 20)
*/
			int iMaxEspionage = 5;
			int iRank = GC.getGameINLINE().getPlayerRank(getID());
			if (iRank < 2)
			{
				iMaxEspionage = 15;
			}
			else if (iRank < 4)
			{
				iMaxEspionage = 10;
			}
			if ((GET_TEAM(getTeam()).getChosenWarCount(true) > 0) || (GET_TEAM(getTeam()).getWarPlanCount(WARPLAN_ATTACKED_RECENT, true) > 0))
			{
				iMaxEspionage = std::min(iMaxEspionage, 5);
			}
			if (AI_isFinancialTrouble())
			{
				iMaxEspionage = 0;
			}

			while (getCommerceRate(COMMERCE_ESPIONAGE) < iEspionageTargetRate && getCommercePercent(COMMERCE_ESPIONAGE) < iMaxEspionage)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			{
				changeCommercePercent(COMMERCE_RESEARCH, -GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));			
				changeCommercePercent(COMMERCE_ESPIONAGE, GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));

				if (getGold() + iTargetTurns * calculateGoldRate() < iGoldTarget)
				{
					break;
				}

				if (!AI_avoidScience() && !isNoResearchAvailable())
				{
	//				if (2 * getCommercePercent(COMMERCE_RESEARCH) < iInitialResearchPercent)
	//				{
	//					break;
	//				}
					if (getCommercePercent(COMMERCE_RESEARCH) * 2 <= (getCommercePercent(COMMERCE_ESPIONAGE) + GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS")) * 3)
					{
						break;
					}
				}
			}
		}
	}
	
	if (!bFirstTech && (getGold() < iGoldTarget) && (getCommercePercent(COMMERCE_RESEARCH) > 40))
	{
		bool bHurryGold = false;
		for (int iHurry = 0; iHurry < GC.getNumHurryInfos(); iHurry++)
		{
			if ((GC.getHurryInfo((HurryTypes)iHurry).getGoldPerProduction() > 0) && canHurry((HurryTypes)iHurry))
			{
				bHurryGold = true;
				break;				
			}
		}
		if (bHurryGold)
		{
			if (getCommercePercent(COMMERCE_ESPIONAGE) > 0)
			{
				changeCommercePercent(COMMERCE_ESPIONAGE, -GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));			
			}
			else
			{
				changeCommercePercent(COMMERCE_RESEARCH, -GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));			
			}
			//changeCommercePercent(COMMERCE_GOLD, GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));
		}
	}
	
/************************************************************************************************/
/* Afforess	                  Start		 01/14/10                                               */
/*                                                                                              */
/*    Taxation Anger Check                                                                      */
/************************************************************************************************/
	int iMaxTaxRate = 100;
	if (AI_avoidIncreasingTaxes())
	{
		iMaxTaxRate = getLastTurnTaxRate() + 10;
		//if we round down instead, abuse our extra knowledge...
		if (GC.getDefineINT("TAXATION_ANGER_ROUND_DOWN"))
			iMaxTaxRate += 10;
	}
	//This allows the AI still to adjust their taxes up by at least 10% each turn, so they have some leeway.
	
	while ((iMaxTaxRate <= getCommercePercent(COMMERCE_GOLD)) && iMaxTaxRate != 100)
	{
		if (isCommerceFlexible(COMMERCE_ESPIONAGE) && (getCommercePercent(COMMERCE_ESPIONAGE) > GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS")))
		{
			changeCommercePercent(COMMERCE_ESPIONAGE, -GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));
		}
		//Check to see if we meet the target now
		if (iMaxTaxRate < getCommercePercent(COMMERCE_GOLD))
			break;
					
		if (isCommerceFlexible(COMMERCE_RESEARCH) && (getCommercePercent(COMMERCE_RESEARCH) > GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS")))
		{	
			changeCommercePercent(COMMERCE_RESEARCH, -GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));
		}
		//Check to see if we meet the target now
		if (iMaxTaxRate < getCommercePercent(COMMERCE_GOLD))
			break;
					
		if (isCommerceFlexible(COMMERCE_CULTURE) && (getCommercePercent(COMMERCE_CULTURE) > GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS")))
		{
			changeCommercePercent(COMMERCE_CULTURE, -GC.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"));
		}				
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/	
	
	// this is called on doTurn, so make sure our gold is high enough keep us above zero gold.
	verifyGoldCommercePercent();
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      07/19/09                                jdog5000      */
/*                                                                                              */
/* Civic AI                                                                                     */
/************************************************************************************************/
void CvPlayerAI::AI_doCivics()
{
	CivicTypes* paeBestCivic;
	//int iCurCivicsValue = 0;
	//int iBestCivicsValue = 0;
	int iI;

	FAssertMsg(!isHuman(), "isHuman did not return false as expected");

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      07/20/09                                jdog5000      */
/*                                                                                              */
/* Barbarian AI, efficiency                                                                     */
/************************************************************************************************/
	if( isBarbarian() )
	{
		return;
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	if (AI_getCivicTimer() > 0)
	{
		AI_changeCivicTimer(-1);
		return;
	}

	if (!canRevolution(NULL))
	{
		return;
	}

	FAssertMsg(AI_getCivicTimer() == 0, "AI Civic timer is expected to be 0");

	paeBestCivic = new CivicTypes[GC.getNumCivicOptionInfos()];

	// Threshold to make AI hold off on civics changes, threshold is percentage to add to
	// value of current civics

	//
	// Mongoose AICivicFix BEGIN
	//
	int iThreshold;
	if ((getAnarchyTurns() == 0) || isGoldenAge())
	{
		iThreshold = 105;
	}
	else
	{
		iThreshold = 120;
	}
	//
	// Mongoose AICivicFix END
	//
	// BBAI TODO:  Other factors?  Roll back???

/************************************************************************************************/
/* Afforess	                  Start		 12/9/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (!GC.getGameINLINE().isOption(GAMEOPTION_FIXED_BORDERS))
	{
		int iCurValue;
		int iBestValue;
		for (iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
		{
			paeBestCivic[iI] = AI_bestCivic((CivicOptionTypes)iI, &iBestValue);
			iCurValue = AI_civicValue( getCivics((CivicOptionTypes)iI) );
			
			//
			// Mongoose AICivicFix BEGIN
			//
			iCurValue = (iCurValue * iThreshold) / 100;
			if (paeBestCivic[iI] == NO_CIVIC || iBestValue < iCurValue)
			{
				paeBestCivic[iI] = getCivics((CivicOptionTypes)iI);
			}
			//
			// Mongoose AICivicFix END
			//
	
			//iCurCivicsValue += iCurValue;
			//iBestCivicsValue += iBestValue;
		}
	}
	else
	{
		CivicTypes* paeBestFixedBordersCivic = new CivicTypes[GC.getNumCivicOptionInfos()];
	
		int* paiBestValues = new int[GC.getNumCivicOptionInfos()];
		int* paiBestFixedBordersValues = new int[GC.getNumCivicOptionInfos()];
	
		AI_findBestCivics(paeBestCivic, paeBestFixedBordersCivic, paiBestValues, paiBestFixedBordersValues);
		
		int iCurValue;
		for (iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
		{
			iCurValue = AI_civicValue( getCivics((CivicOptionTypes)iI) );
			
			iCurValue = (iCurValue * (100 + iThreshold)) / 100;
	
			if (paeBestCivic[iI] == NO_CIVIC || paiBestValues[iI] < iCurValue)
			{
				paeBestCivic[iI] = getCivics((CivicOptionTypes)iI);
				paiBestValues[iI] = iCurValue;
			}
		}
	
		if (!containFixedBorders(paeBestCivic))
		{
			/* find a fixed borders civic with the min difference from the best one (by category) and switch that category to it */
			int iBestDifference = MAXINT, iCivicOptionInfoToChange = -1;
			for (iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
			{
				if (paeBestFixedBordersCivic[iI] != NO_CIVIC)
				{
					if (paiBestValues[iI] - paiBestFixedBordersValues[iI] < iBestDifference)
					{
						iBestDifference = paiBestValues[iI] - paiBestFixedBordersValues[iI];
						iCivicOptionInfoToChange = iI;
					}
				}
			}
			
			if (iCivicOptionInfoToChange != -1)
			/* if a fixed borders civic is found, iCivicOptionInfoToChange is the category */
			{
				paeBestCivic[iCivicOptionInfoToChange] = paeBestFixedBordersCivic[iCivicOptionInfoToChange];
			}
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


	// XXX AI skips revolution???
	if (canRevolution(paeBestCivic))
	{
		revolution(paeBestCivic);
		AI_setCivicTimer((getMaxAnarchyTurns() == 0) ? (GC.getDefineINT("MIN_REVOLUTION_TURNS") * 2) : CIVIC_CHANGE_DELAY);
	}

	SAFE_DELETE_ARRAY(paeBestCivic);
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/


void CvPlayerAI::AI_doReligion()
{
	ReligionTypes eBestReligion;

	FAssertMsg(!isHuman(), "isHuman did not return false as expected");
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      07/20/09                                jdog5000      */
/*                                                                                              */
/* Barbarian AI, efficiency                                                                     */
/************************************************************************************************/
	if( isBarbarian() )
	{
		return;
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	if (AI_getReligionTimer() > 0)
	{
		AI_changeReligionTimer(-1);
		return;
	}

	if (!canChangeReligion())
	{
		return;
	}

	FAssertMsg(AI_getReligionTimer() == 0, "AI Religion timer is expected to be 0");

	eBestReligion = AI_bestReligion();

	if (eBestReligion == NO_RELIGION)
	{
		eBestReligion = getStateReligion();
	}

	if (canConvert(eBestReligion))
	{
		convert(eBestReligion);
		AI_setReligionTimer((getMaxAnarchyTurns() == 0) ? (GC.getDefineINT("MIN_CONVERSION_TURNS") * 2) : RELIGION_CHANGE_DELAY);
	}
}

/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
// RevolutionDCM start - new diplomacy option
void CvPlayerAI::AI_beginDiplomacy(CvDiploParameters* pDiploParams, PlayerTypes ePlayer)
{
	if (isDoNotBotherStatus(ePlayer))
	{
		// Divert AI diplomacy away from the diplomacy screen and induce the appropriate reaction
		// in the AI equivalent to a human rejecting the AI's requests in the interface. There are
		// a number of AI requests that do not need handling and that simply time out. There are
		// also AI requests that occur in CvTeam that induce the diplomacy screen in any case.
		// This diplomacy modification does not alter the AI's characteristics at all and is actually 
		// just an interface modification for a player to shut down talks with an AI automatically.
		int ai_request;
		ai_request = (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_RELIGION_PRESSURE");
		if (ai_request == pDiploParams->getDiploComment())
		{
			this->handleDiploEvent(DIPLOEVENT_NO_CONVERT, ePlayer, -1, -1);
		}

		ai_request = (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_CIVIC_PRESSURE");
		if (ai_request == pDiploParams->getDiploComment())
		{
			this->handleDiploEvent(DIPLOEVENT_NO_REVOLUTION, ePlayer, -1, -1);
		}

		ai_request = (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_JOIN_WAR");
		if (ai_request == pDiploParams->getDiploComment())
		{
			this->handleDiploEvent(DIPLOEVENT_NO_JOIN_WAR, ePlayer, -1, -1);
		}

		ai_request = (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_STOP_TRADING");
		if (ai_request == pDiploParams->getDiploComment())
		{
			this->handleDiploEvent(DIPLOEVENT_NO_STOP_TRADING, ePlayer, -1, -1);
		}

		ai_request = (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_ASK_FOR_HELP");
		if (ai_request == pDiploParams->getDiploComment())
		{
			this->handleDiploEvent(DIPLOEVENT_REFUSED_HELP, ePlayer, -1, -1);
		}

		ai_request = (DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_DEMAND_TRIBUTE");
		if (ai_request == pDiploParams->getDiploComment())
		{
			this->handleDiploEvent(DIPLOEVENT_REJECTED_DEMAND, ePlayer, -1, -1);
			if (AI_demandRebukedWar(ePlayer))
			{
				this->handleDiploEvent(DIPLOEVENT_DEMAND_WAR, ePlayer, -1, -1);
			}
		}
	}
	else
	{
		gDLL->beginDiplomacy(pDiploParams, (PlayerTypes)ePlayer);
	}
}
// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/

void CvPlayerAI::AI_doDiplo()
{
	PROFILE_FUNC();

	CLLNode<TradeData>* pNode;
	CvDiploParameters* pDiplo;
	CvDeal* pLoopDeal;
	CvCity* pLoopCity;
	CvPlot* pLoopPlot;
	CLinkList<TradeData> ourList;
	CLinkList<TradeData> theirList;
	bool abContacted[MAX_TEAMS];
	TradeData item;
	CivicTypes eFavoriteCivic;
	BonusTypes eBestReceiveBonus;
	BonusTypes eBestGiveBonus;
	TechTypes eBestReceiveTech;
	TechTypes eBestGiveTech;
	TeamTypes eBestTeam;
	bool bCancelDeal;
	int iReceiveGold;
	int iGiveGold;
	int iGold;
	int iGoldData;
	int iGoldWeight;
	int iGoldValuePercent;
	int iCount;
	int iPossibleCount;
	int iValue;
	int iBestValue;
	int iOurValue;
	int iTheirValue;
	int iPass;
	int iLoop;
	int iI, iJ;

	FAssert(!isHuman());
	FAssert(!isMinorCiv());
	FAssert(!isBarbarian());

/************************************************************************************************/
/* Afforess	                  Start		 04/29/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (GC.getUSE_AI_DO_DIPLO_CALLBACK())
	{
		CyArgsList argsList;
		argsList.add(getID());
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_doDiplo", argsList.makeFunctionArgs(), &lResult);
		if (lResult == 1)
		{
			return;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	iGoldValuePercent = AI_goldTradeValuePercent();

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		abContacted[iI] = false;
	}

	for (iPass = 0; iPass < 2; iPass++)
	{
		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)iI).isHuman() == (iPass == 1))
				{
					if (iI != getID())
					{
						if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
						{
							for(pLoopDeal = GC.getGameINLINE().firstDeal(&iLoop); pLoopDeal != NULL; pLoopDeal = GC.getGameINLINE().nextDeal(&iLoop))
							{
								if (pLoopDeal->isCancelable(getID()))
								{
									if ((GC.getGameINLINE().getGameTurn() - pLoopDeal->getInitialGameTurn()) >= (GC.getDefineINT("PEACE_TREATY_LENGTH") * 2))
									{
										bCancelDeal = false;

										if ((pLoopDeal->getFirstPlayer() == getID()) && (pLoopDeal->getSecondPlayer() == ((PlayerTypes)iI)))
										{
											if (GET_PLAYER((PlayerTypes)iI).isHuman())
											{
												if (!AI_considerOffer(((PlayerTypes)iI), pLoopDeal->getSecondTrades(), pLoopDeal->getFirstTrades(), -1))
												{
													bCancelDeal = true;
												}
											}
											else
											{
												for (pNode = pLoopDeal->getFirstTrades()->head(); pNode; pNode = pLoopDeal->getFirstTrades()->next(pNode))
												{
													if (getTradeDenial(((PlayerTypes)iI), pNode->m_data) != NO_DENIAL)
													{
														bCancelDeal = true;
														break;
													}
												}
											}
										}
										else if ((pLoopDeal->getFirstPlayer() == ((PlayerTypes)iI)) && (pLoopDeal->getSecondPlayer() == getID()))
										{
											if (GET_PLAYER((PlayerTypes)iI).isHuman())
											{
												if (!AI_considerOffer(((PlayerTypes)iI), pLoopDeal->getFirstTrades(), pLoopDeal->getSecondTrades(), -1))
												{
													bCancelDeal = true;
												}
											}
											else
											{
												for (pNode = pLoopDeal->getSecondTrades()->head(); pNode; pNode = pLoopDeal->getSecondTrades()->next(pNode))
												{
													if (getTradeDenial(((PlayerTypes)iI), pNode->m_data) != NO_DENIAL)
													{
														bCancelDeal = true;
														break;
													}
												}
											}
										}

										if (bCancelDeal)
										{
											if (canContact((PlayerTypes)iI) && AI_isWillingToTalk((PlayerTypes)iI))
											{
												if (GET_PLAYER((PlayerTypes)iI).isHuman())
												{
													ourList.clear();
													theirList.clear();

													for (pNode = pLoopDeal->headFirstTradesNode(); (pNode != NULL); pNode = pLoopDeal->nextFirstTradesNode(pNode))
													{
														if (pLoopDeal->getFirstPlayer() == getID())
														{
															ourList.insertAtEnd(pNode->m_data);
														}
														else
														{
															theirList.insertAtEnd(pNode->m_data);
														}
													}

													for (pNode = pLoopDeal->headSecondTradesNode(); (pNode != NULL); pNode = pLoopDeal->nextSecondTradesNode(pNode))
													{
														if (pLoopDeal->getSecondPlayer() == getID())
														{
															ourList.insertAtEnd(pNode->m_data);
														}
														else
														{
															theirList.insertAtEnd(pNode->m_data);
														}
													}

													pDiplo = new CvDiploParameters(getID());
													FAssertMsg(pDiplo != NULL, "pDiplo must be valid");

													if (pLoopDeal->isVassalDeal())
													{
														pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_NO_VASSAL"));
														pDiplo->setAIContact(true);
/***************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
														// RevolutionDCM start - new diplomacy option
														AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
														// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
														// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/
													}
													else
													{
														pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_CANCEL_DEAL"));
														pDiplo->setAIContact(true);
														pDiplo->setOurOfferList(theirList);
														pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
														// RevolutionDCM start - new diplomacy option
														AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
														// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
														// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/													}
													abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
												}
											}
/************************************************************************************************/
/* Afforess	                  Start		 07/29/10                                               */
/*                                                                                              */
/* Advanced Diplomacy                                                                           */
/************************************************************************************************/
											bool bEmbassyCanceled = pLoopDeal->isEmbassy();
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
											pLoopDeal->kill(); // XXX test this for AI...
/************************************************************************************************/
/* Afforess	                  Start		 07/29/10                                               */
/*                                                                                              */
/* Advanced Diplomacy                                                                           */
/************************************************************************************************/
											if (bEmbassyCanceled)
											{
												for (int iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
												{
													if (GET_PLAYER((PlayerTypes)iPlayer).isAlive())
													{
														if (getTeam() == GET_PLAYER((PlayerTypes)iPlayer).getTeam())
														{
															GET_PLAYER((PlayerTypes)iPlayer).AI_changeMemoryCount(((PlayerTypes)iI), MEMORY_RECALLED_AMBASSADOR, -AI_getMemoryCount(((PlayerTypes)iI), MEMORY_RECALLED_AMBASSADOR));
														}
													}
												}
											}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
										}
									}
								}
							}
						}

						if (canContact((PlayerTypes)iI) && AI_isWillingToTalk((PlayerTypes)iI))
						{
							if (GET_PLAYER((PlayerTypes)iI).getTeam() == getTeam() || GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isVassal(getTeam()))
							{
								// XXX will it cancel this deal if it loses it's first resource???

								iBestValue = 0;
								eBestGiveBonus = NO_BONUS;

								for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
								{
									if (getNumTradeableBonuses((BonusTypes)iJ) > 1)
									{
										if ((GET_PLAYER((PlayerTypes)iI).AI_bonusTradeVal(((BonusTypes)iJ), getID(), 1) > 0)
											&& (GET_PLAYER((PlayerTypes)iI).AI_bonusVal((BonusTypes)iJ, 1) > AI_bonusVal((BonusTypes)iJ, -1)))
										{
											setTradeItem(&item, TRADE_RESOURCES, iJ);

											if (canTradeItem(((PlayerTypes)iI), item, true))
											{
												iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Bonus Trading #1"));

												if (iValue > iBestValue)
												{
													iBestValue = iValue;
													eBestGiveBonus = ((BonusTypes)iJ);
												}
											}
										}
									}
								}

								if (eBestGiveBonus != NO_BONUS)
								{
									ourList.clear();
									theirList.clear();

									setTradeItem(&item, TRADE_RESOURCES, eBestGiveBonus);
									ourList.insertAtEnd(item);

									if (GET_PLAYER((PlayerTypes)iI).isHuman())
									{
										if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
										{
											pDiplo = new CvDiploParameters(getID());
											FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
											pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_GIVE_HELP"));
											pDiplo->setAIContact(true);
											pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
											// RevolutionDCM start - new diplomacy option
											AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
											// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
											// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/											abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
										}
									}
									else
									{
										GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
									}
								}
							}

							if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam() && GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isVassal(getTeam()))
							{
								iBestValue = 0;
								eBestGiveTech = NO_TECH;

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/06/09                                jdog5000      */
/*                                                                                              */
/* Diplomacy                                                                                    */
/************************************************************************************************/
								// Don't give techs for tree to advanced vassals ...
								if( GET_PLAYER((PlayerTypes)iI).getTechScore()*10 < getTechScore()*9 )
								{
									for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
									{
										if (GET_TEAM(getTeam()).AI_techTrade((TechTypes)iJ, GET_PLAYER((PlayerTypes)iI).getTeam()) == NO_DENIAL)
										{
											setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

											if (canTradeItem(((PlayerTypes)iI), item, true))
											{
												iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Vassal Tech gift"));

												if (iValue > iBestValue)
												{
													iBestValue = iValue;
													eBestGiveTech = ((TechTypes)iJ);
												}
											}
										}
									}
								}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

								if (eBestGiveTech != NO_TECH)
								{
									ourList.clear();
									theirList.clear();

									setTradeItem(&item, TRADE_TECHNOLOGIES, eBestGiveTech);
									ourList.insertAtEnd(item);

									if (GET_PLAYER((PlayerTypes)iI).isHuman())
									{
										if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
										{
											pDiplo = new CvDiploParameters(getID());
											FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
											pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_GIVE_HELP"));
											pDiplo->setAIContact(true);
											pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
											// RevolutionDCM start - new diplomacy option
											AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
											// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
											// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/											abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
										}
									}
									else
									{
										GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
									}
								}
							}

							if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam() && !(GET_TEAM(getTeam()).isHuman()) && (GET_PLAYER((PlayerTypes)iI).isHuman() || !(GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHuman())))
							{
								FAssertMsg(!(GET_PLAYER((PlayerTypes)iI).isBarbarian()), "(GET_PLAYER((PlayerTypes)iI).isBarbarian()) did not return false as expected");
								FAssertMsg(iI != getID(), "iI is not expected to be equal with getID()");

								if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isVassal(getTeam()))
								{
									iBestValue = 0;
									eBestGiveBonus = NO_BONUS;

									for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
									{
										if (GET_PLAYER((PlayerTypes)iI).getNumTradeableBonuses((BonusTypes)iJ) > 0 && getNumAvailableBonuses((BonusTypes)iJ) == 0)
										{
											iValue = AI_bonusTradeVal((BonusTypes)iJ, (PlayerTypes)iI, 1);

											if (iValue > iBestValue)
											{
												iBestValue = iValue;
												eBestGiveBonus = ((BonusTypes)iJ);
											}
										}
									}

									if (eBestGiveBonus != NO_BONUS)
									{
										theirList.clear();
										ourList.clear();

										setTradeItem(&item, TRADE_RESOURCES, eBestGiveBonus);
										theirList.insertAtEnd(item);

										if (GET_PLAYER((PlayerTypes)iI).isHuman())
										{
											if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
											{
												CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_VASSAL_GRANT_TRIBUTE, getID(), eBestGiveBonus);
												if (pInfo)
												{
													gDLL->getInterfaceIFace()->addPopup(pInfo, (PlayerTypes)iI);
													abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
												}
											}
										}
										else
										{
											GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
										}
									}
								}

								if (!(GET_TEAM(getTeam()).isAtWar(GET_PLAYER((PlayerTypes)iI).getTeam())))
								{
									if (AI_getAttitude((PlayerTypes)iI) >= ATTITUDE_CAUTIOUS)
									{
										for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
										{
											if (pLoopCity->getPreviousOwner() != ((PlayerTypes)iI))
											{
												if (((pLoopCity->getGameTurnAcquired() + 4) % 20) == (GC.getGameINLINE().getGameTurn() % 20))
												{
													iCount = 0;
													iPossibleCount = 0;

													for (iJ = 0; iJ < NUM_CITY_PLOTS; iJ++)
													{
														pLoopPlot = plotCity(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), iJ);

														if (pLoopPlot != NULL)
														{
															if (pLoopPlot->getOwnerINLINE() == iI)
															{
																iCount++;
															}

															iPossibleCount++;
														}
													}

													if (iCount >= (iPossibleCount / 2))
													{
														setTradeItem(&item, TRADE_CITIES, pLoopCity->getID());

														if (canTradeItem(((PlayerTypes)iI), item, true))
														{
															ourList.clear();

															ourList.insertAtEnd(item);

															if (GET_PLAYER((PlayerTypes)iI).isHuman())
															{
																//if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_CITY"));
																	pDiplo->setAIContact(true);
																	pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
																	// RevolutionDCM start - new diplomacy option
																	AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
															else
															{
																GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, NULL);
															}
														}
													}
												}
											}
										}
									}

									if (GET_TEAM(getTeam()).getLeaderID() == getID())
									{
										if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_PERMANENT_ALLIANCE) == 0)
										{
											bool bOffered = false;
											if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_PERMANENT_ALLIANCE), "AI Diplo Alliance") == 0)
											{
												setTradeItem(&item, TRADE_PERMANENT_ALLIANCE);

												if (canTradeItem(((PlayerTypes)iI), item, true) && GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
												{
													ourList.clear();
													theirList.clear();

													ourList.insertAtEnd(item);
													theirList.insertAtEnd(item);

													bOffered = true;

													if (GET_PLAYER((PlayerTypes)iI).isHuman())
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_PERMANENT_ALLIANCE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_PERMANENT_ALLIANCE));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
															pDiplo->setAIContact(true);
															pDiplo->setOurOfferList(theirList);
															pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
															// RevolutionDCM start - new diplomacy option
															AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
													else
													{
														GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
														break; // move on to next player since we are on the same team now
													}
												}
											}

											if (!bOffered)
											{
												setTradeItem(&item, TRADE_VASSAL);

												if (canTradeItem((PlayerTypes)iI, item, true))
												{
													ourList.clear();
													theirList.clear();

													ourList.insertAtEnd(item);

													if (GET_PLAYER((PlayerTypes)iI).isHuman())
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_PERMANENT_ALLIANCE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_PERMANENT_ALLIANCE));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_VASSAL"));
															pDiplo->setAIContact(true);
															pDiplo->setOurOfferList(theirList);
															pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
															// RevolutionDCM start - new diplomacy option
															AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
													else
													{
														bool bAccepted = true;
														TeamTypes eMasterTeam = GET_PLAYER((PlayerTypes)iI).getTeam();
														for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++)
														{
															if (GET_TEAM((TeamTypes)iTeam).isAlive())
															{
															if (iTeam != getTeam() && iTeam != eMasterTeam && atWar(getTeam(), (TeamTypes)iTeam) && !atWar(eMasterTeam, (TeamTypes)iTeam))
															{
																if (GET_TEAM(eMasterTeam).AI_declareWarTrade((TeamTypes)iTeam, getTeam(), false) != NO_DENIAL)
																{
																	bAccepted = false;
																	break;
																}
															}
														}
														}

														if (bAccepted)
														{
															GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()) && !GET_TEAM(getTeam()).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam()))
									{
										if (getStateReligion() != NO_RELIGION)
										{
											if (GET_PLAYER((PlayerTypes)iI).canConvert(getStateReligion()))
											{
												if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_RELIGION_PRESSURE) == 0)
												{
													if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_RELIGION_PRESSURE), "AI Diplo Religion Pressure") == 0)
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_RELIGION_PRESSURE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_RELIGION_PRESSURE));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_RELIGION_PRESSURE"));
															pDiplo->setAIContact(true);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
															// RevolutionDCM start - new diplomacy option
															AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()) && !GET_TEAM(getTeam()).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam()))
									{
										eFavoriteCivic = ((CivicTypes)(GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic()));

										if (eFavoriteCivic != NO_CIVIC)
										{
											if (isCivic(eFavoriteCivic))
											{
												if (GET_PLAYER((PlayerTypes)iI).canDoCivics(eFavoriteCivic) && !(GET_PLAYER((PlayerTypes)iI).isCivic(eFavoriteCivic)))
												{
													if (GET_PLAYER((PlayerTypes)iI).canRevolution(NULL))
													{
														if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_CIVIC_PRESSURE) == 0)
														{
															if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_CIVIC_PRESSURE), "AI Diplo Civic Pressure") == 0)
															{
																if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	AI_changeContactTimer(((PlayerTypes)iI), CONTACT_CIVIC_PRESSURE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_CIVIC_PRESSURE));
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_CIVIC_PRESSURE"), GC.getCivicInfo(eFavoriteCivic).getTextKeyWide());
																	pDiplo->setAIContact(true);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
																	// RevolutionDCM start - new diplomacy option
																	AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
														}
													}
												}
											}
										}
									}
/************************************************************************************************/
/* Afforess	                  Start		 05/16/10                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()) && GC.getDefineINT("CAN_TRADE_WAR") > 0)

/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
									{
										if ((AI_getMemoryCount(((PlayerTypes)iI), MEMORY_DECLARED_WAR) == 0) && (AI_getMemoryCount(((PlayerTypes)iI), MEMORY_HIRED_WAR_ALLY) == 0))
										{
											if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_JOIN_WAR) == 0)
											{
												if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_JOIN_WAR), "AI Diplo Join War") == 0)
												{
													iBestValue = 0;
													eBestTeam = NO_TEAM;

													for (iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
													{
														if (GET_TEAM((TeamTypes)iJ).isAlive())
														{
															if (atWar(((TeamTypes)iJ), getTeam()) && !atWar(((TeamTypes)iJ), GET_PLAYER((PlayerTypes)iI).getTeam()))
															{
																if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasMet((TeamTypes)iJ))
																{
																	if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).canDeclareWar((TeamTypes)iJ))
																	{
																		iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Joining War"));

																		if (iValue > iBestValue)
																		{
																			iBestValue = iValue;
																			eBestTeam = ((TeamTypes)iJ);
																		}
																	}
																}
															}
														}
													}

													if (eBestTeam != NO_TEAM)
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
/************************************************************************************************/
/* Afforess	                  Start		 05/23/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
															m_eDemandWarAgainstTeam = eBestTeam;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_JOIN_WAR, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_JOIN_WAR));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_JOIN_WAR"), GET_PLAYER(GET_TEAM(eBestTeam).getLeaderID()).getCivilizationAdjectiveKey());
															pDiplo->setAIContact(true);
															pDiplo->setData(eBestTeam);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
															// RevolutionDCM start - new diplomacy option
															AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()) && !GET_TEAM(getTeam()).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam()))
									{
										if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_STOP_TRADING) == 0)
										{
											if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_STOP_TRADING), "AI Diplo Stop Trading") == 0)
											{
/************************************************************************************************/
/* Afforess	                  Start		 07/29/10                                               */
/*                                                                                              */
/* Advanced Diplomacy                                                                           */
/************************************************************************************************/
												if (GC.getGameINLINE().isOption(GAMEOPTION_ADVANCED_DIPLOMACY))
												{
													eBestTeam = AI_bestStopTradeTeam((PlayerTypes)iI);
												}
												else
												{
													eBestTeam = GET_TEAM(getTeam()).AI_getWorstEnemy();
												}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
												if ((eBestTeam != NO_TEAM) && GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isHasMet(eBestTeam) && !GET_TEAM(eBestTeam).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam()))
												{
													if (GET_PLAYER((PlayerTypes)iI).canStopTradingWithTeam(eBestTeam))
													{
														FAssert(!atWar(GET_PLAYER((PlayerTypes)iI).getTeam(), eBestTeam));
														FAssert(GET_PLAYER((PlayerTypes)iI).getTeam() != eBestTeam);

														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_STOP_TRADING, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_STOP_TRADING));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_STOP_TRADING"), GET_PLAYER(GET_TEAM(eBestTeam).getLeaderID()).getCivilizationAdjectiveKey());
															pDiplo->setAIContact(true);
															pDiplo->setData(eBestTeam);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
															// RevolutionDCM start - new diplomacy option
															AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()))
									{
										if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).getAssets() < (GET_TEAM(getTeam()).getAssets() / 2))
										{
											if (AI_getAttitude((PlayerTypes)iI) > GC.getLeaderHeadInfo(GET_PLAYER((PlayerTypes)iI).getPersonalityType()).getNoGiveHelpAttitudeThreshold())
											{
												if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_GIVE_HELP) == 0)
												{
													if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_GIVE_HELP), "AI Diplo Give Help") == 0)
													{
														// XXX maybe do gold instead???

														iBestValue = 0;
														eBestGiveTech = NO_TECH;

														for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
														{
															setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

															if (canTradeItem(((PlayerTypes)iI), item, true))
															{
																iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Giving Help"));

																if (iValue > iBestValue)
																{
																	iBestValue = iValue;
																	eBestGiveTech = ((TechTypes)iJ);
																}
															}
														}

														if (eBestGiveTech != NO_TECH)
														{
															ourList.clear();

															setTradeItem(&item, TRADE_TECHNOLOGIES, eBestGiveTech);
															ourList.insertAtEnd(item);

															if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
															{
																AI_changeContactTimer(((PlayerTypes)iI), CONTACT_GIVE_HELP, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_GIVE_HELP));
																pDiplo = new CvDiploParameters(getID());
																FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_GIVE_HELP"));
																pDiplo->setAIContact(true);
																pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
																// RevolutionDCM start - new diplomacy option
																AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/																abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
															}
														}
													}
												}
											}
										}
									}

									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()))
									{
										if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).getAssets() > (GET_TEAM(getTeam()).getAssets() / 2))
										{
											if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_ASK_FOR_HELP) == 0)
											{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/12/10                                jdog5000      */
/*                                                                                              */
/* Diplomacy                                                                                    */
/************************************************************************************************/
												int iRand = GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_ASK_FOR_HELP);
												int iTechPerc = GET_TEAM(getTeam()).getBestKnownTechScorePercent();
												if( iTechPerc < 90 )
												{
													iRand *= std::max(1, iTechPerc - 60);
													iRand /= 30;
												}

												if (GC.getGameINLINE().getSorenRandNum(iRand, "AI Diplo Ask For Help") == 0)
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
												{
													iBestValue = 0;
													eBestReceiveTech = NO_TECH;

													for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
													{
														setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

														if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
														{
															iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Asking For Help"));

															if (iValue > iBestValue)
															{
																iBestValue = iValue;
																eBestReceiveTech = ((TechTypes)iJ);
															}
														}
													}

													if (eBestReceiveTech != NO_TECH)
													{
														theirList.clear();

														setTradeItem(&item, TRADE_TECHNOLOGIES, eBestReceiveTech);
														theirList.insertAtEnd(item);

														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_ASK_FOR_HELP, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_ASK_FOR_HELP));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_ASK_FOR_HELP"));
															pDiplo->setAIContact(true);
															pDiplo->setOurOfferList(theirList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
															// RevolutionDCM start - new diplomacy option
															AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
												}
											}
										}
									}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      05/06/09                                jdog5000      */
/*                                                                                              */
/* Bugfix, Diplomacy AI                                                                         */
/************************************************************************************************/
									// Reduced duplication so easier to maintain
									if (GET_PLAYER((PlayerTypes)iI).isHuman() && (GET_TEAM(getTeam()).getLeaderID() == getID()))
									{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       05/06/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
										if (GET_TEAM(getTeam()).canDeclareWar(GET_PLAYER((PlayerTypes)iI).getTeam()) && !GET_TEAM(getTeam()).AI_isSneakAttackPreparing(GET_PLAYER((PlayerTypes)iI).getTeam()))
*/
										// Bug fix: when team was sneak attack ready but hadn't declared, could demand tribute
										// If other team accepted, it blocked war declaration for 10 turns but AI didn't react.
										if (GET_TEAM(getTeam()).canDeclareWar(GET_PLAYER((PlayerTypes)iI).getTeam()) && !GET_TEAM(getTeam()).AI_isChosenWar(GET_PLAYER((PlayerTypes)iI).getTeam()))
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
										{
											if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).getDefensivePower() < GET_TEAM(getTeam()).getPower(true))
											{
												if (AI_getAttitude((PlayerTypes)iI) <= GC.getLeaderHeadInfo(GET_PLAYER((PlayerTypes)iI).getPersonalityType()).getDemandTributeAttitudeThreshold())
												{
													if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_DEMAND_TRIBUTE) == 0)
													{
														if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_DEMAND_TRIBUTE), "AI Diplo Demand Tribute") == 0)
														{
															iReceiveGold = std::min(std::max(0, (GET_PLAYER((PlayerTypes)iI).getGold() - 50)), GET_PLAYER((PlayerTypes)iI).AI_goldTarget());

															iReceiveGold -= (iReceiveGold % GC.getDefineINT("DIPLOMACY_VALUE_REMAINDER"));

															if (iReceiveGold > 50)
															{
																theirList.clear();

																setTradeItem(&item, TRADE_GOLD, iReceiveGold);
																theirList.insertAtEnd(item);

																if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	AI_changeContactTimer(((PlayerTypes)iI), CONTACT_DEMAND_TRIBUTE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_DEMAND_TRIBUTE));
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_DEMAND_TRIBUTE"));
																	pDiplo->setAIContact(true);
																	pDiplo->setOurOfferList(theirList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
																	// RevolutionDCM start - new diplomacy option
																	AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
														}

														if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_DEMAND_TRIBUTE), "AI Diplo Demand Tribute") == 0)
														{
															if (GET_TEAM(getTeam()).AI_mapTradeVal(GET_PLAYER((PlayerTypes)iI).getTeam()) > 100)
															{
																theirList.clear();

																setTradeItem(&item, TRADE_MAPS);
																theirList.insertAtEnd(item);

																if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	AI_changeContactTimer(((PlayerTypes)iI), CONTACT_DEMAND_TRIBUTE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_DEMAND_TRIBUTE));
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_DEMAND_TRIBUTE"));
																	pDiplo->setAIContact(true);
																	pDiplo->setOurOfferList(theirList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
																	// RevolutionDCM start - new diplomacy option
																	AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
														}

														if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_DEMAND_TRIBUTE), "AI Diplo Demand Tribute") == 0)
														{
															iBestValue = 0;
															eBestReceiveTech = NO_TECH;

															for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
															{
																setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

																if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
																{
																	if (GC.getGameINLINE().countKnownTechNumTeams((TechTypes)iJ) > 1)
																	{
																		iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Demanding Tribute (Tech)"));

																		if (iValue > iBestValue)
																		{
																			iBestValue = iValue;
																			eBestReceiveTech = ((TechTypes)iJ);
																		}
																	}
																}
															}

															if (eBestReceiveTech != NO_TECH)
															{
																theirList.clear();

																setTradeItem(&item, TRADE_TECHNOLOGIES, eBestReceiveTech);
																theirList.insertAtEnd(item);

																if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	AI_changeContactTimer(((PlayerTypes)iI), CONTACT_DEMAND_TRIBUTE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_DEMAND_TRIBUTE));
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_DEMAND_TRIBUTE"));
																	pDiplo->setAIContact(true);
																	pDiplo->setOurOfferList(theirList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
																	// RevolutionDCM start - new diplomacy option
																	AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
														}

														if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_DEMAND_TRIBUTE), "AI Diplo Demand Tribute") == 0)
														{
															iBestValue = 0;
															eBestReceiveBonus = NO_BONUS;

															for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
															{
																if (GET_PLAYER((PlayerTypes)iI).getNumTradeableBonuses((BonusTypes)iJ) > 1)
																{
																	if (AI_bonusTradeVal(((BonusTypes)iJ), ((PlayerTypes)iI), 1) > 0)
																	{
																		setTradeItem(&item, TRADE_RESOURCES, iJ);

																		if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
																		{
																			iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Demanding Tribute (Bonus)"));

																			if (iValue > iBestValue)
																			{
																				iBestValue = iValue;
																				eBestReceiveBonus = ((BonusTypes)iJ);
																			}
																		}
																	}
																}
															}

															if (eBestReceiveBonus != NO_BONUS)
															{
																theirList.clear();

																setTradeItem(&item, TRADE_RESOURCES, eBestReceiveBonus);
																theirList.insertAtEnd(item);

																if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	AI_changeContactTimer(((PlayerTypes)iI), CONTACT_DEMAND_TRIBUTE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_DEMAND_TRIBUTE));
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_DEMAND_TRIBUTE"));
																	pDiplo->setAIContact(true);
																	pDiplo->setOurOfferList(theirList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
																	// RevolutionDCM start - new diplomacy option
																	AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
														}
													}
												}
											}
										}
									}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

									if (GET_TEAM(getTeam()).getLeaderID() == getID())
									{
										if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_OPEN_BORDERS) == 0)
										{
											if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_OPEN_BORDERS), "AI Diplo Open Borders") == 0)
											{
												setTradeItem(&item, TRADE_OPEN_BORDERS);

												if (canTradeItem(((PlayerTypes)iI), item, true) && GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
												{
													ourList.clear();
													theirList.clear();

													ourList.insertAtEnd(item);
													theirList.insertAtEnd(item);

													if (GET_PLAYER((PlayerTypes)iI).isHuman())
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_OPEN_BORDERS, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_OPEN_BORDERS));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
															pDiplo->setAIContact(true);
															pDiplo->setOurOfferList(theirList);
															pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
															// RevolutionDCM start - new diplomacy option
															AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
													else
													{
														GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
													}
												}
											}
										}
									}

									if (GET_TEAM(getTeam()).getLeaderID() == getID())
									{
										if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_DEFENSIVE_PACT) == 0)
										{
											if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_DEFENSIVE_PACT), "AI Diplo Defensive Pact") == 0)
											{
												setTradeItem(&item, TRADE_DEFENSIVE_PACT);

												if (canTradeItem(((PlayerTypes)iI), item, true) && GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
												{
													ourList.clear();
													theirList.clear();

													ourList.insertAtEnd(item);
													theirList.insertAtEnd(item);

													if (GET_PLAYER((PlayerTypes)iI).isHuman())
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_DEFENSIVE_PACT, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_DEFENSIVE_PACT));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
															pDiplo->setAIContact(true);
															pDiplo->setOurOfferList(theirList);
															pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
															// RevolutionDCM start - new diplomacy option
															AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
													else
													{
														GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
													}
												}
											}
										}
									}

									if (!(GET_PLAYER((PlayerTypes)iI).isHuman()) || (GET_TEAM(getTeam()).getLeaderID() == getID()))
									{
										if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_TRADE_TECH) == 0)
										{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/24/10                                jdog5000      */
/*                                                                                              */
/* Diplomacy                                                                                    */
/************************************************************************************************/
											int iRand = GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_TRADE_TECH);
											int iTechPerc = GET_TEAM(getTeam()).getBestKnownTechScorePercent();
											if( iTechPerc < 90 )
											{
												iRand *= std::max(1, iTechPerc - 60);
												iRand /= 30;
											}

											if( AI_isDoVictoryStrategy(AI_VICTORY_SPACE1) )
											{
												iRand /= 2;
											}
						
											iRand = std::max(1, iRand);
											if (GC.getGameINLINE().getSorenRandNum(iRand, "AI Diplo Trade Tech") == 0)
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
											{
												iBestValue = 0;
												eBestReceiveTech = NO_TECH;

												for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
												{
													setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

													if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
													{
														iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Tech Trading #1"));

														if (iValue > iBestValue)
														{
															iBestValue = iValue;
															eBestReceiveTech = ((TechTypes)iJ);
														}
													}
												}

												if (eBestReceiveTech != NO_TECH)
												{
													iBestValue = 0;
													eBestGiveTech = NO_TECH;

													for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
													{
														setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

														if (canTradeItem(((PlayerTypes)iI), item, true))
														{
															iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Tech Trading #2"));

															if (iValue > iBestValue)
															{
																iBestValue = iValue;
																eBestGiveTech = ((TechTypes)iJ);
															}
														}
													}

													iOurValue = GET_TEAM(getTeam()).AI_techTradeVal(eBestReceiveTech, GET_PLAYER((PlayerTypes)iI).getTeam());
													if (eBestGiveTech != NO_TECH)
													{
														iTheirValue = GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_techTradeVal(eBestGiveTech, getTeam());
													}
													else
													{
														iTheirValue = 0;
													}

													iReceiveGold = 0;
													iGiveGold = 0;

													if (iTheirValue > iOurValue)
													{
														iGoldWeight = iTheirValue - iOurValue;
														iGoldData = (iGoldWeight * 100) / iGoldValuePercent;
														if (iGoldWeight * 100 > iGoldData * iGoldValuePercent)
														{
															iGoldData++;															
														}
														
														iGold = std::min(iGoldData, GET_PLAYER((PlayerTypes)iI).AI_maxGoldTrade(getID()));

														if (iGold > 0)
														{
															setTradeItem(&item, TRADE_GOLD, iGold);

															if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
															{
																iReceiveGold = iGold;
																iOurValue += (iGold * iGoldValuePercent) / 100;
															}
														}
													}
													else if (iOurValue > iTheirValue)
													{
														
														iGoldData = ((iOurValue - iTheirValue) * 100) / iGoldValuePercent;
														iGold = std::min(iGoldData, AI_maxGoldTrade((PlayerTypes)iI));

														if (iGold > 0)
														{
															setTradeItem(&item, TRADE_GOLD, iGold);

															if (canTradeItem(((PlayerTypes)iI), item, true))
															{
																iGiveGold = iGold;
																iTheirValue += (iGold * iGoldValuePercent) / 100;
															}
														}
													}

													if (!(GET_PLAYER((PlayerTypes)iI).isHuman()) || (iOurValue >= iTheirValue))
													{
														if ((iOurValue > ((iTheirValue * 2) / 3)) && (iTheirValue > ((iOurValue * 2) / 3)))
														{
															ourList.clear();
															theirList.clear();

															if (eBestGiveTech != NO_TECH)
															{
																setTradeItem(&item, TRADE_TECHNOLOGIES, eBestGiveTech);
																ourList.insertAtEnd(item);
															}

															setTradeItem(&item, TRADE_TECHNOLOGIES, eBestReceiveTech);
															theirList.insertAtEnd(item);

															if (iGiveGold != 0)
															{
																setTradeItem(&item, TRADE_GOLD, iGiveGold);
																ourList.insertAtEnd(item);
															}

															if (iReceiveGold != 0)
															{
																setTradeItem(&item, TRADE_GOLD, iReceiveGold);
																theirList.insertAtEnd(item);
															}

															if (GET_PLAYER((PlayerTypes)iI).isHuman())
															{
																if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	AI_changeContactTimer(((PlayerTypes)iI), CONTACT_TRADE_TECH, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_TRADE_TECH));
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
																	pDiplo->setAIContact(true);
																	pDiplo->setOurOfferList(theirList);
																	pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
																	// RevolutionDCM start - new diplomacy option
																	AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
															else
															{
																GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
															}
														}
													}
												}
											}
										}
									}


/************************************************************************************************/
/* Afforess	                  Start		 06/16/10                                               */
/*                                                                                              */
/* Advanced Diplomacy                                                                           */
/************************************************************************************************/
									if (GC.getGameINLINE().isOption(GAMEOPTION_ADVANCED_DIPLOMACY) || GC.getGameINLINE().isOption(GAMEOPTION_RUTHLESS_AI))
									{
										//Purchase War Ally
										if (GET_PLAYER((PlayerTypes)iI).isHuman() || ((GET_TEAM(getTeam()).getLeaderID() == getID()) && !GET_TEAM(getTeam()).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam())))
										{
											if ((AI_getMemoryCount(((PlayerTypes)iI), MEMORY_DECLARED_WAR) == 0) && (AI_getMemoryCount(((PlayerTypes)iI), MEMORY_HIRED_WAR_ALLY) == 0))
											{
												if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_TRADE_JOIN_WAR) == 0)
												{
													if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_TRADE_JOIN_WAR), "AI Diplo Trade War") == 0)
													{
														TeamTypes eBestWarTeam = AI_bestJoinWarTeam((PlayerTypes)iI);

														if (eBestWarTeam != NO_TEAM)
														{
															if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_declareWarTrade(eBestWarTeam, getTeam(), true) == NO_DENIAL)
															{
																//if (GET_TEAM(eBestWarTeam).getPower(true) > (GET_TEAM(getTeam()).getPower(true) * 0.75))
																{

																	iBestValue = 0;
																	eBestGiveTech = NO_TECH;

																	for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
																	{
																		setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

																		if (canTradeItem(((PlayerTypes)iI), item, true))
																		{
																			iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Tech Trading #2"));

																			if (iValue > iBestValue)
																			{
																				iBestValue = iValue;
																					eBestGiveTech = ((TechTypes)iJ);
																			}
																		}
																	}

																	iOurValue = GET_TEAM(getTeam()).AI_declareWarTradeVal(eBestWarTeam, GET_PLAYER((PlayerTypes)iI).getTeam());
																	if (eBestGiveTech != NO_TECH)
																	{
																		iTheirValue = GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_techTradeVal(eBestGiveTech, getTeam());
																	}
																	else
																	{
																		iTheirValue = 0;
																	}

																	iReceiveGold = 0;
																	iGiveGold = 0;

																	if (iTheirValue > iOurValue)
																	{
																		iGoldWeight = iTheirValue - iOurValue;
																		iGoldData = (iGoldWeight * 100) / iGoldValuePercent;
																		if (iGoldWeight * 100 > iGoldData * iGoldValuePercent)
																		{
																			iGoldData++;															
																		}

																		iGold = std::min(iGoldData, GET_PLAYER((PlayerTypes)iI).AI_maxGoldTrade(getID()));

																		if (iGold > 0)
																		{
																			setTradeItem(&item, TRADE_GOLD, iGold);

																			if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
																			{
																				iReceiveGold = iGold;
																				iOurValue += (iGold * iGoldValuePercent) / 100;
																			}
																		}
																	}
																	else if (iOurValue > iTheirValue)
																	{
																		
																		iGoldData = ((iOurValue - iTheirValue) * 100) / iGoldValuePercent;
																		iGold = std::min(iGoldData, AI_maxGoldTrade((PlayerTypes)iI));

																		if (iGold > 0)
																		{
																			setTradeItem(&item, TRADE_GOLD, iGold);

																			if (canTradeItem(((PlayerTypes)iI), item, true))
																			{
																				iGiveGold = iGold;
																				iTheirValue += (iGold * iGoldValuePercent) / 100;
																			}
																		}
																	}

																	if (!(GET_PLAYER((PlayerTypes)iI).isHuman()) || (iOurValue >= iTheirValue))
																	{
																		if ((iOurValue > ((iTheirValue * 2) / 3)) && (iTheirValue > ((iOurValue * 2) / 3)))
																		{
																			ourList.clear();
																			theirList.clear();

																			if (eBestGiveTech != NO_TECH)
																			{
																				setTradeItem(&item, TRADE_TECHNOLOGIES, eBestGiveTech);
																				ourList.insertAtEnd(item);
																			}

																			setTradeItem(&item, TRADE_WAR, eBestWarTeam);
																			theirList.insertAtEnd(item);

																			if (iGiveGold != 0)
																			{
																				setTradeItem(&item, TRADE_GOLD, iGiveGold);
																				ourList.insertAtEnd(item);
																			}

																			if (iReceiveGold != 0)
																			{
																				setTradeItem(&item, TRADE_GOLD, iReceiveGold);
																				theirList.insertAtEnd(item);
																			}

																			if (GET_PLAYER((PlayerTypes)iI).isHuman())
																			{
																				if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																				{
																					m_eDemandWarAgainstTeam = eBestWarTeam;
																					AI_changeContactTimer(((PlayerTypes)iI), CONTACT_TRADE_JOIN_WAR, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_TRADE_JOIN_WAR));
																					pDiplo = new CvDiploParameters(getID());
																					FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																					pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_JOIN_WAR"), GET_PLAYER(GET_TEAM(eBestWarTeam).getLeaderID()).getCivilizationAdjectiveKey());
																					pDiplo->setAIContact(true);
																					pDiplo->setOurOfferList(theirList);
																					pDiplo->setTheirOfferList(ourList);
																					AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																					abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																				}
																			}
																			else
																			{
																				if (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_declareWarTrade(eBestWarTeam, getTeam(), true) == NO_DENIAL)
																				{
																					GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
																					CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_HIRED_WAR_ALLY", getCivilizationAdjectiveKey(), GET_PLAYER((PlayerTypes)iI).getCivilizationAdjectiveKey());
																					for (int i = 0; i < MAX_CIV_PLAYERS; i++)
																					{
																						if (GET_PLAYER((PlayerTypes)i).getTeam() == eBestWarTeam)
																						{
																							gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)i), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BUILD_BARRACKS", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"));
																						}
																					}
																				}
																				else
																				{
																					m_eDemandWarAgainstTeam = eBestWarTeam;
																				}
																			}
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
										//Broker Peace
										if (GET_TEAM(getTeam()).getLeaderID() == getID() && !GC.getGame().isPreviousRequest((PlayerTypes)iI))
										{
											if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_PEACE_PRESSURE) == 0)
											{
												if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_PEACE_PRESSURE), "AI Diplo End War") == 0)
												{
													eBestTeam = NO_TEAM;
													
													eBestTeam = AI_bestMakePeaceTeam((PlayerTypes)iI);

													if (eBestTeam != NO_TEAM)
													{
														if (GET_PLAYER((PlayerTypes)iI).isHuman())
														{
															if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
															{
																GC.getGameINLINE().setPreviousRequest((PlayerTypes)iI, true);
																AI_changeContactTimer(((PlayerTypes)iI), CONTACT_PEACE_PRESSURE, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_PEACE_PRESSURE));
																pDiplo = new CvDiploParameters(getID());
																FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_MAKE_PEACE_WITH"), GET_PLAYER(GET_TEAM(eBestTeam).getLeaderID()).getCivilizationAdjectiveKey());
																pDiplo->setAIContact(true);
																pDiplo->setData(eBestTeam);
																AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
															}
														}
														else
														{
															GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
														}
													}
												}
											}
										}
										//Purchase Trade Embargo
										if (GET_PLAYER((PlayerTypes)iI).isHuman() || ((GET_TEAM(getTeam()).getLeaderID() == getID()) && !GET_TEAM(getTeam()).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam())))
										{
											if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_TRADE_STOP_TRADING) == 0)
											{
												if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_TRADE_JOIN_WAR), "AI Diplo Trade War") == 0)
												{
													TeamTypes eBestStopTradeTeam = AI_bestStopTradeTeam((PlayerTypes)iI);

													if (eBestStopTradeTeam != NO_TEAM)
													{

														iBestValue = 0;
														eBestGiveTech = NO_TECH;

														for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
														{
															setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

															if (canTradeItem(((PlayerTypes)iI), item, true))
															{
																iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Tech Trading #2"));

																if (iValue > iBestValue)
																{
																	iBestValue = iValue;
																	eBestGiveTech = ((TechTypes)iJ);
																}
															}
														}

														iOurValue = AI_stopTradingTradeVal(eBestStopTradeTeam, ((PlayerTypes)iI));
														if (eBestGiveTech != NO_TECH)
														{
															iTheirValue = GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_techTradeVal(eBestGiveTech, getTeam());
														}
														else
														{
															iTheirValue = 0;
														}

														iReceiveGold = 0;
														iGiveGold = 0;

														if (iTheirValue > iOurValue)
														{
															iGoldWeight = iTheirValue - iOurValue;
															iGoldData = (iGoldWeight * 100) / iGoldValuePercent;
															if (iGoldWeight * 100 > iGoldData * iGoldValuePercent)
															{
																iGoldData++;															
															}
																	
															iGold = std::min(iGoldData, GET_PLAYER((PlayerTypes)iI).AI_maxGoldTrade(getID()));

															if (iGold > 0)
															{
																setTradeItem(&item, TRADE_GOLD, iGold);

																if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
																{
																	iReceiveGold = iGold;
																	iOurValue += (iGold * iGoldValuePercent) / 100;
																}
															}
														}
														else if (iOurValue > iTheirValue)
														{
															iGoldData = ((iOurValue - iTheirValue) * 100) / iGoldValuePercent;
															iGold = std::min(iGoldData, AI_maxGoldTrade((PlayerTypes)iI));

															if (iGold > 0)
															{
																setTradeItem(&item, TRADE_GOLD, iGold);

																if (canTradeItem(((PlayerTypes)iI), item, true))
																{
																	iGiveGold = iGold;
																	iTheirValue += (iGold * iGoldValuePercent) / 100;
																}
															}
														}

														if (!(GET_PLAYER((PlayerTypes)iI).isHuman()) || (iOurValue >= iTheirValue))
														{
															if ((iOurValue > ((iTheirValue * 2) / 3)) && (iTheirValue > ((iOurValue * 2) / 3)))
															{
																ourList.clear();
																theirList.clear();

																if (eBestGiveTech != NO_TECH)
																{
																	setTradeItem(&item, TRADE_TECHNOLOGIES, eBestGiveTech);
																	ourList.insertAtEnd(item);
																}

																setTradeItem(&item, TRADE_EMBARGO, eBestStopTradeTeam);
																theirList.insertAtEnd(item);

																if (iGiveGold != 0)
																{
																	setTradeItem(&item, TRADE_GOLD, iGiveGold);
																	ourList.insertAtEnd(item);
																}

																if (iReceiveGold != 0)
																{
																	setTradeItem(&item, TRADE_GOLD, iReceiveGold);
																	theirList.insertAtEnd(item);
																}

																if (GET_PLAYER((PlayerTypes)iI).isHuman())
																{
																	if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																	{
																		AI_changeContactTimer(((PlayerTypes)iI), CONTACT_TRADE_STOP_TRADING, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_TRADE_STOP_TRADING));
																		pDiplo = new CvDiploParameters(getID());
																		FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																		pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
																		pDiplo->setAIContact(true);
																		pDiplo->setOurOfferList(theirList);
																		pDiplo->setTheirOfferList(ourList);
																		AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																		abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																	}
																}
																else
																{
																	GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
																}
															}
														}
													}
												}
											}
										}
									}
									if (GC.getGameINLINE().isOption(GAMEOPTION_ADVANCED_DIPLOMACY))
									{
										//Establish Embassy
										if (GET_TEAM(getTeam()).getLeaderID() == getID())
										{
											if (!GET_TEAM(getTeam()).isHasEmbassy(GET_PLAYER((PlayerTypes)iI).getTeam()))
											{
												if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_EMBASSY) == 0)
												{
													if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_EMBASSY), "AI Diplo Open Borders") == 0)
													{
														setTradeItem(&item, TRADE_EMBASSY);

														if (canTradeItem(((PlayerTypes)iI), item, true) && GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
														{
															ourList.clear();
															theirList.clear();

															ourList.insertAtEnd(item);
															theirList.insertAtEnd(item);

															if (GET_PLAYER((PlayerTypes)iI).isHuman())
															{
																if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																{
																	AI_changeContactTimer(((PlayerTypes)iI), CONTACT_EMBASSY, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_EMBASSY));
																	pDiplo = new CvDiploParameters(getID());
																	FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																	pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
																	pDiplo->setAIContact(true);
																	pDiplo->setOurOfferList(theirList);
																	pDiplo->setTheirOfferList(ourList);
																	AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																	abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																}
															}
															else
															{
																GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
															}
														}
													}
												}
											}
										}
										//Open Free Trade
										if (GET_TEAM(getTeam()).getLeaderID() == getID())
										{
											if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_OPEN_BORDERS) == 0)
											{
												if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_OPEN_BORDERS), "AI Diplo Limited Borders") == 0)
												{
													setTradeItem(&item, TRADE_FREE_TRADE_ZONE);

													if (canTradeItem(((PlayerTypes)iI), item, true) && GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
													{
														ourList.clear();
														theirList.clear();

														ourList.insertAtEnd(item);
														theirList.insertAtEnd(item);

														if (GET_PLAYER((PlayerTypes)iI).isHuman())
														{
															if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
															{
																AI_changeContactTimer(((PlayerTypes)iI), CONTACT_OPEN_BORDERS, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_OPEN_BORDERS));
																pDiplo = new CvDiploParameters(getID());
																FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
																pDiplo->setAIContact(true);
																pDiplo->setOurOfferList(theirList);
																pDiplo->setTheirOfferList(ourList);
																// RevolutionDCM start - new diplomacy option
																AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																// RevolutionDCM end
																abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
															}
														}
														else
														{
															GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
														}
													}
												}
											}
										}
										//Open Limited Borders
										if (GET_TEAM(getTeam()).getLeaderID() == getID())
										{
											if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_OPEN_BORDERS) == 0)
											{
												if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_OPEN_BORDERS), "AI Diplo Limited Borders") == 0)
												{
													setTradeItem(&item, TRADE_RITE_OF_PASSAGE);

													if (canTradeItem(((PlayerTypes)iI), item, true) && GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
													{
														ourList.clear();
														theirList.clear();

														ourList.insertAtEnd(item);
														theirList.insertAtEnd(item);

														if (GET_PLAYER((PlayerTypes)iI).isHuman())
														{
															if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
															{
																AI_changeContactTimer(((PlayerTypes)iI), CONTACT_OPEN_BORDERS, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_OPEN_BORDERS));
																pDiplo = new CvDiploParameters(getID());
																FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
																pDiplo->setAIContact(true);
																pDiplo->setOurOfferList(theirList);
																pDiplo->setTheirOfferList(ourList);
																// RevolutionDCM start - new diplomacy option
																AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																// RevolutionDCM end
																abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
															}
														}
														else
														{
															GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
														}
													}
												}
											}
										}
										//Sell contacts to other teams
										if (GET_TEAM(getTeam()).getLeaderID() == getID())
										{
											if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_TRADE_CONTACTS) == 0)
											{
												if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_TRADE_CONTACTS) / 2, "AI Diplo Trade Contacts") == 0)
												{
													TeamTypes eBestTeam = NO_TEAM;
													for (iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
													{
														CvTeam& kTeam = GET_TEAM((TeamTypes) iJ);
														if (kTeam.isAlive() && !(kTeam.isMinorCiv()))
														{
															setTradeItem(&item, TRADE_CONTACT, (TeamTypes)iJ);

															if (canTradeItem((PlayerTypes)iI, item, true))
															{
																//Yes, this is the first team we can sell contact with... Not like it really matters
																eBestTeam = (TeamTypes)iJ;
																break;
															}
														}
													}
													if (eBestTeam != NO_TEAM)
													{
														iOurValue = GET_TEAM(getTeam()).AI_contactTradeVal(eBestTeam, GET_PLAYER((PlayerTypes)iI).getTeam());

														iGold = std::max(0, GET_PLAYER((PlayerTypes)iI).AI_maxGoldTrade(getID()));
														if (iOurValue <= iGold && iOurValue > 0)
														{
															setTradeItem(&item, TRADE_GOLD, iOurValue);

															if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
															{
																ourList.clear();
																theirList.clear();

																setTradeItem(&item, TRADE_CONTACT, eBestTeam);
																ourList.insertAtEnd(item);

																setTradeItem(&item, TRADE_GOLD, iOurValue);
																theirList.insertAtEnd(item);

																if (GET_PLAYER((PlayerTypes)iI).isHuman())
																{
																	if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																	{
																		AI_changeContactTimer(((PlayerTypes)iI), CONTACT_TRADE_CONTACTS, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_TRADE_CONTACTS));
																		pDiplo = new CvDiploParameters(getID());
																		FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																		pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
																		pDiplo->setAIContact(true);
																		pDiplo->setOurOfferList(theirList);
																		pDiplo->setTheirOfferList(ourList);
																		AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																		abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																	}
																}
																else
																{
																	GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
																	GC.getGameINLINE().logMsg("Player %d has traded contact of %d to %d for %d gold", getID(), GET_TEAM(eBestTeam).getLeaderID(), iI, iOurValue);
																	CvWString szBuffer;
																	switch (AI_getAttitude(GET_TEAM(eBestTeam).getLeaderID()))
																	{
																	case ATTITUDE_FURIOUS:
																		szBuffer = gDLL->getText("TXT_KEY_MISC_TRADED_CONTACT_FURIOUS", getCivilizationDescription(), GET_PLAYER((PlayerTypes)iI).getCivilizationDescription());
																		break;
																	case ATTITUDE_ANNOYED:
																		szBuffer = gDLL->getText("TXT_KEY_MISC_TRADED_CONTACT_ANNOYED", getCivilizationDescription(), GET_PLAYER((PlayerTypes)iI).getCivilizationDescription());
																		break;
																	case ATTITUDE_CAUTIOUS:
																		szBuffer = gDLL->getText("TXT_KEY_MISC_TRADED_CONTACT_CAUTIOUS", getCivilizationDescription(), GET_PLAYER((PlayerTypes)iI).getCivilizationDescription());
																		break;
																	case ATTITUDE_PLEASED:
																		szBuffer = gDLL->getText("TXT_KEY_MISC_TRADED_CONTACT_PLEASED", getCivilizationDescription(), GET_PLAYER((PlayerTypes)iI).getCivilizationDescription());
																		break;
																	case ATTITUDE_FRIENDLY:
																		szBuffer = gDLL->getText("TXT_KEY_MISC_TRADED_CONTACT_FRIENDLY", getCivilizationDescription(), GET_PLAYER((PlayerTypes)iI).getCivilizationDescription());
																		break;
																	default:
																		FAssertMsg(false, "No Valid Attitude");
																		szBuffer = gDLL->getText("TXT_KEY_MISC_TRADED_CONTACT_CAUTIOUS", getCivilizationDescription(), GET_PLAYER((PlayerTypes)iI).getCivilizationDescription());
																		break;
																	}
																	
																	for (int i = 0; i < MAX_PLAYERS; i++)
																	{
																		if (GET_PLAYER((PlayerTypes)i).getTeam() == eBestTeam)
																		{
																			gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)i), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_FEAT_ACCOMPLISHED", MESSAGE_TYPE_MAJOR_EVENT, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"));
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
										//Purchase Workers
										//Why Does this matter?
										//if (GET_TEAM(getTeam()).getLeaderID() == getID())
										{
											if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_TRADE_WORKERS) == 0)
											{
												if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_TRADE_WORKERS), "AI Diplo Trade Workers") == 0)
												{
													if (GET_TEAM(getTeam()).isHasEmbassy(GET_PLAYER((PlayerTypes)iI).getTeam()))
													{
															CvUnit* pWorker = NULL;
															int iNeededWorkers = 0;
															
														//figure out if we need workers or not
														for (CvArea* pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
														{
															if (pLoopArea->getCitiesPerPlayer(getID()) > 0)
															{
																iNeededWorkers += AI_neededWorkers(pLoopArea);
															}
														}
														//if we need workers
														if (iNeededWorkers > 0)
														{
															for (CvUnit* pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
															{
																if (pLoopUnit->canTradeUnit(getID()))
																{
																	setTradeItem(&item, TRADE_WORKER, pLoopUnit->getID());
																	//if they can trade the worker to us
																	if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
																	{
																		pWorker = pLoopUnit;
																		break;
																	}
																}
															}
														}
														if (pWorker != NULL)
														{
															iTheirValue = GET_PLAYER((PlayerTypes)iI).AI_workerTradeVal(pWorker);
															iGold = AI_maxGoldTrade((PlayerTypes)iI);
															if (iGold >= iTheirValue && iTheirValue > 0)
															{
																setTradeItem(&item, TRADE_GOLD, iTheirValue);
																//if we can trade the gold to them
																if (canTradeItem((PlayerTypes)iI, item, true))
																{
																	ourList.clear();
																	theirList.clear();

																	setTradeItem(&item, TRADE_GOLD, iTheirValue);
																	ourList.insertAtEnd(item);
																		
																	setTradeItem(&item, TRADE_WORKER, pWorker->getID());
																	theirList.insertAtEnd(item);

																	if (GET_PLAYER((PlayerTypes)iI).isHuman())
																	{
																		if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																		{
																			AI_changeContactTimer(((PlayerTypes)iI), CONTACT_TRADE_WORKERS, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_TRADE_WORKERS));
																			pDiplo = new CvDiploParameters(getID());
																			FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																			pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
																			pDiplo->setAIContact(true);
																			pDiplo->setOurOfferList(theirList);
																			pDiplo->setTheirOfferList(ourList);
																			AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																			abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																		}
																	}
																	else
																	{
																		GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
																		//GC.getGameINLINE().logMsg("Player %d has traded a worker to %d for %d gold", getID(), iI, iTheirValue);
																	}
																}
															}
														}
													}
												}
											}
										}
									
										if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_TRADE_MILITARY_UNITS) == 0)
										{
											if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_TRADE_MILITARY_UNITS) / std::max(1, GET_TEAM(getTeam()).getAnyWarPlanCount(true)), "AI Diplo Trade Military Units") == 0)
											{
												if (GET_TEAM(getTeam()).isHasEmbassy(GET_PLAYER((PlayerTypes)iI).getTeam()) )
												{
													if (!AI_isFinancialTrouble())
													{
														int* paiMilitaryUnits;
														CvUnit* pLoopUnit;
														paiMilitaryUnits = new int[GET_PLAYER((PlayerTypes)iI).getNumUnits()];
														for (iJ = 0; iJ < GET_PLAYER((PlayerTypes)iI).getNumUnits(); iJ++)
														{
															paiMilitaryUnits[iJ] = -1;
														}
														CvUnit* pBestUnit = NULL;
														int iNumTradableUnits = 0;
														for (iJ = 0, pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; iJ++, pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
														{
															if (pLoopUnit->canTradeUnit(getID()))
															{
																setTradeItem(&item, TRADE_MILITARY_UNIT, pLoopUnit->getID());
																if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
																{
																	paiMilitaryUnits[iJ] = pLoopUnit->getID();
																	iNumTradableUnits++;
																}
															}
														}
														TechTypes eBestTech = NO_TECH;
														int iBestValue = 0;
														if (iNumTradableUnits > 0)
														{
															for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
															{
																setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

																if (canTradeItem(((PlayerTypes)iI), item, true))
																{
																	iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Tech For Military"));

																	iValue /= std::max(1, GC.getTechInfo((TechTypes)iJ).getFlavorValue(GC.getInfoTypeForString("FLAVOR_MILITARY")));
																	if (iValue > iBestValue)
																	{
																		iBestValue = iValue;
																		eBestTech = (TechTypes)iJ;
																	}
																}
															}
														}
														if (eBestTech != NO_TECH)
														{
															int iUnitValue = 0;
															int iTechValue = GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_techTradeVal(eBestTech, getTeam());
															for (iJ = 0; iJ < GET_PLAYER((PlayerTypes)iI).getNumUnits(); iJ++)
															{
																if (paiMilitaryUnits[iJ] > 0)
																{
																	if (iUnitValue > iTechValue)
																	{
																		paiMilitaryUnits[iJ] = -1;
																	}
																	else
																	{
																		iUnitValue += AI_militaryUnitTradeVal(GET_PLAYER((PlayerTypes)iI).getUnit(paiMilitaryUnits[iJ]));
																	}
																}
															}
														
															ourList.clear();
															theirList.clear();
															
															int iNeededGold = iUnitValue - iTechValue;
															//Units are worth more than the tech
															if (iNeededGold > 0)
															{
																iGold = AI_maxGoldTrade((PlayerTypes)iI);
																
																setTradeItem(&item, TRADE_GOLD, std::min(iNeededGold, iGold));
																if (canTradeItem((PlayerTypes)iI, item, true))
																{
																	setTradeItem(&item, TRADE_GOLD, std::min(iNeededGold, iGold));
																	ourList.insertAtEnd(item);
																}
															}
															//The tech is worth more than the units
															else if (iNeededGold < 0)
															{
																iGold = GET_PLAYER((PlayerTypes)iI).AI_maxGoldTrade(getID());
																
																setTradeItem(&item, TRADE_GOLD, std::min(-iNeededGold, iGold));
																if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
																{
																	setTradeItem(&item, TRADE_GOLD, std::min(-iNeededGold, iGold));
																	theirList.insertAtEnd(item);
																}
															}
															//The difference is too large, the trade isn't worth it
															if ( std::max(iNeededGold, -iNeededGold) - iGold < 300)
															{
																for (iJ = 0; iJ < GET_PLAYER((PlayerTypes)iI).getNumUnits(); iJ++)
																{
																	if (paiMilitaryUnits[iJ] > 0)
																	{
																		setTradeItem(&item, TRADE_MILITARY_UNIT, paiMilitaryUnits[iJ]);
																		theirList.insertAtEnd(item);
																	}
																}
																
																setTradeItem(&item, TRADE_TECHNOLOGIES, eBestTech);
																ourList.insertAtEnd(item);

																if (GET_PLAYER((PlayerTypes)iI).isHuman())
																{
																	if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
																	{
																		AI_changeContactTimer(((PlayerTypes)iI), CONTACT_TRADE_MILITARY_UNITS, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_TRADE_MILITARY_UNITS));
																		pDiplo = new CvDiploParameters(getID());
																		FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																		pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
																		pDiplo->setAIContact(true);
																		pDiplo->setOurOfferList(theirList);
																		pDiplo->setTheirOfferList(ourList);
																		AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																		abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
																	}
																}
																else
																{
																	GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
																	//GC.getGameINLINE().logMsg("Player %d has traded military units to %d.", getID(), iI);
																}
															}
														}
														SAFE_DELETE_ARRAY(paiMilitaryUnits);
													}
												}
											}
										}
									}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
									if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_TRADE_BONUS) == 0)
									{
										if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_TRADE_BONUS), "AI Diplo Trade Bonus") == 0)
										{
											iBestValue = 0;
											eBestReceiveBonus = NO_BONUS;

											for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
											{
												if (GET_PLAYER((PlayerTypes)iI).getNumTradeableBonuses((BonusTypes)iJ) > 1)
												{
													if (GET_PLAYER((PlayerTypes)iI).AI_corporationBonusVal((BonusTypes)iJ) == 0)
													{
														if (AI_bonusTradeVal(((BonusTypes)iJ), ((PlayerTypes)iI), 1) > 0)
														{
															setTradeItem(&item, TRADE_RESOURCES, iJ);

															if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
															{
/************************************************************************************************/
/* Afforess	                  Start		 06/16/10                                               */
/*                                                                                              */
/* Advanced Diplomacy                                                                           */
/************************************************************************************************/
										/*Original Code:
																iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Bonus Trading #1"));
										*/
																iValue = (AI_bonusTradeVal(((BonusTypes)iJ), ((PlayerTypes)iI), 1) + GC.getGameINLINE().getSorenRandNum(200, "AI Bonus Trading #1"));
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
																if (iValue > iBestValue)
																{
																	iBestValue = iValue;
																	eBestReceiveBonus = ((BonusTypes)iJ);
																}
															}
														}
													}
												}
											}

											if (eBestReceiveBonus != NO_BONUS)
											{
												iBestValue = 0;
												eBestGiveBonus = NO_BONUS;

												for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
												{
													if (iJ != eBestReceiveBonus)
													{
														if (getNumTradeableBonuses((BonusTypes)iJ) > 1)
														{
															if (GET_PLAYER((PlayerTypes)iI).AI_bonusTradeVal(((BonusTypes)iJ), getID(), 1) > 0)
															{
																setTradeItem(&item, TRADE_RESOURCES, iJ);

																if (canTradeItem(((PlayerTypes)iI), item, true))
																{
																	iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Bonus Trading #2"));

																	if (iValue > iBestValue)
																	{
																		iBestValue = iValue;
																		eBestGiveBonus = ((BonusTypes)iJ);
																	}
																}
															}
														}
													}
												}

												if (eBestGiveBonus != NO_BONUS)
												{
													if (!(GET_PLAYER((PlayerTypes)iI).isHuman()) || (AI_bonusTradeVal(eBestReceiveBonus, ((PlayerTypes)iI), -1) >= GET_PLAYER((PlayerTypes)iI).AI_bonusTradeVal(eBestGiveBonus, getID(), 1)))
													{
														ourList.clear();
														theirList.clear();

														setTradeItem(&item, TRADE_RESOURCES, eBestGiveBonus);
														ourList.insertAtEnd(item);

														setTradeItem(&item, TRADE_RESOURCES, eBestReceiveBonus);
														theirList.insertAtEnd(item);

														if (GET_PLAYER((PlayerTypes)iI).isHuman())
														{
															if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
															{
																AI_changeContactTimer(((PlayerTypes)iI), CONTACT_TRADE_BONUS, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_TRADE_BONUS));
																pDiplo = new CvDiploParameters(getID());
																FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
																pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
																pDiplo->setAIContact(true);
																pDiplo->setOurOfferList(theirList);
																pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
																// RevolutionDCM start - new diplomacy option
																AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
																// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
																// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/																abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
															}
														}
														else
														{
															GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
														}
													}
												}
											}
										}
									}

									if (AI_getContactTimer(((PlayerTypes)iI), CONTACT_TRADE_MAP) == 0)
									{
										if (GC.getGameINLINE().getSorenRandNum(GC.getLeaderHeadInfo(getPersonalityType()).getContactRand(CONTACT_TRADE_MAP), "AI Diplo Trade Map") == 0)
										{
											setTradeItem(&item, TRADE_MAPS);

											if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true) && canTradeItem(((PlayerTypes)iI), item, true))
											{
												if (!(GET_PLAYER((PlayerTypes)iI).isHuman()) || (GET_TEAM(getTeam()).AI_mapTradeVal(GET_PLAYER((PlayerTypes)iI).getTeam()) >= GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_mapTradeVal(getTeam())))
												{
													ourList.clear();
													theirList.clear();

													setTradeItem(&item, TRADE_MAPS);
													ourList.insertAtEnd(item);

													setTradeItem(&item, TRADE_MAPS);
													theirList.insertAtEnd(item);

													if (GET_PLAYER((PlayerTypes)iI).isHuman())
													{
														if (!(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
														{
															AI_changeContactTimer(((PlayerTypes)iI), CONTACT_TRADE_MAP, GC.getLeaderHeadInfo(getPersonalityType()).getContactDelay(CONTACT_TRADE_MAP));
															pDiplo = new CvDiploParameters(getID());
															FAssertMsg(pDiplo != NULL, "pDiplo must be valid");
															pDiplo->setDiploComment((DiploCommentTypes)GC.getInfoTypeForString("AI_DIPLOCOMMENT_OFFER_DEAL"));
															pDiplo->setAIContact(true);
															pDiplo->setOurOfferList(theirList);
															pDiplo->setTheirOfferList(ourList);
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         02/04/08                            Glider1        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
															// RevolutionDCM start - new diplomacy option
															AI_beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// gDLL->beginDiplomacy(pDiplo, (PlayerTypes)iI);
															// RevolutionDCM end
/************************************************************************************************/
/* REVOLUTIONDCM_MOD                         END                                 Glider1        */
/************************************************************************************************/															abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()] = true;
														}
													}
													else
													{
														GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
													}
												}
											}
										}
									}

									if (!(GET_PLAYER((PlayerTypes)iI).isHuman()) || !(abContacted[GET_PLAYER((PlayerTypes)iI).getTeam()]))
									{
										int iDeclareWarTradeRand = GC.getLeaderHeadInfo(getPersonalityType()).getDeclareWarTradeRand();
										int iMinAtWarCounter = MAX_INT;
										for (iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
										{
											if (GET_TEAM((TeamTypes)iJ).isAlive())
											{
												if (atWar(((TeamTypes)iJ), getTeam()))
												{
													int iAtWarCounter = GET_TEAM(getTeam()).AI_getAtWarCounter((TeamTypes)iJ);
													if (GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iJ) == WARPLAN_DOGPILE)
													{
														iAtWarCounter *= 2;
														iAtWarCounter += 5;
													}
													iMinAtWarCounter = std::min(iAtWarCounter, iMinAtWarCounter);
												}
											}
										}
										
										if (iMinAtWarCounter < 10)
										{
											iDeclareWarTradeRand *= iMinAtWarCounter;
											iDeclareWarTradeRand /= 10;
											iDeclareWarTradeRand ++;
										}
										
										if (iMinAtWarCounter < 4)
										{
											iDeclareWarTradeRand /= 4;
											iDeclareWarTradeRand ++;
										}

										if (GC.getGameINLINE().getSorenRandNum(iDeclareWarTradeRand, "AI Diplo Declare War Trade") == 0)
										{
											iBestValue = 0;
											eBestTeam = NO_TEAM;

											for (iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
											{
												if (GET_TEAM((TeamTypes)iJ).isAlive())
												{
													if (atWar(((TeamTypes)iJ), getTeam()) && !atWar(((TeamTypes)iJ), GET_PLAYER((PlayerTypes)iI).getTeam()))
													{
														if (GET_TEAM((TeamTypes)iJ).getAtWarCount(true) < std::max(2, (GC.getGameINLINE().countCivTeamsAlive() / 2)))
														{
															setTradeItem(&item, TRADE_WAR, iJ);

															if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
															{
																iValue = (1 + GC.getGameINLINE().getSorenRandNum(1000, "AI Declare War Trading"));
																
																iValue *= (101 + GET_TEAM((TeamTypes)iJ).AI_getAttitudeWeight(getTeam()));
																iValue /= 100;

																if (iValue > iBestValue)
																{
																	iBestValue = iValue;
																	eBestTeam = ((TeamTypes)iJ);
																}
															}
														}
													}
												}
											}

											if (eBestTeam != NO_TEAM)
											{
												iBestValue = 0;
												eBestGiveTech = NO_TECH;

												for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
												{
													setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

													if (canTradeItem(((PlayerTypes)iI), item, true))
													{
														iValue = (1 + GC.getGameINLINE().getSorenRandNum(100, "AI Tech Trading #2"));
														
														iValue *= GET_TEAM(eBestTeam).getResearchLeft((TechTypes)iJ);

														if (iValue > iBestValue)
														{
															iBestValue = iValue;
															eBestGiveTech = ((TechTypes)iJ);
														}
													}
												}

												iOurValue = GET_TEAM(getTeam()).AI_declareWarTradeVal(eBestTeam, GET_PLAYER((PlayerTypes)iI).getTeam());
												if (eBestGiveTech != NO_TECH)
												{
													iTheirValue = GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_techTradeVal(eBestGiveTech, getTeam());
												}
												else
												{
													iTheirValue = 0;
												}

												int iBestValue2 = 0;
												TechTypes eBestGiveTech2 = NO_TECH;
												
												if ((iTheirValue < iOurValue) && (eBestGiveTech != NO_TECH))
												{
													for (iJ = 0; iJ < GC.getNumTechInfos(); iJ++)
													{
														if (iJ != eBestGiveTech)
														{
															setTradeItem(&item, TRADE_TECHNOLOGIES, iJ);

															if (canTradeItem(((PlayerTypes)iI), item, true))
															{
																iValue = (1 + GC.getGameINLINE().getSorenRandNum(100, "AI Tech Trading #2"));
																
																iValue *= GET_TEAM(eBestTeam).getResearchLeft((TechTypes)iJ);

																if (iValue > iBestValue)
																{
																	iBestValue2 = iValue;
																	eBestGiveTech2 = ((TechTypes)iJ);
																}
															}
														}
													}
													
													if (eBestGiveTech2 != NO_TECH)
													{
														iTheirValue += GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_techTradeVal(eBestGiveTech2, getTeam());
													}
												}

												iReceiveGold = 0;
												iGiveGold = 0;

												if (iTheirValue > iOurValue)
												{
													iGold = std::min(((iTheirValue - iOurValue) * 100) / iGoldValuePercent, GET_PLAYER((PlayerTypes)iI).AI_maxGoldTrade(getID()));

													if (iGold > 0)
													{
														setTradeItem(&item, TRADE_GOLD, iGold);

														if (GET_PLAYER((PlayerTypes)iI).canTradeItem(getID(), item, true))
														{
															iReceiveGold = iGold;
															iOurValue += (iGold * iGoldValuePercent) / 100;
														}
													}
												}
												else if (iOurValue > iTheirValue)
												{
													iGold = std::min(((iOurValue - iTheirValue) * 100) / iGoldValuePercent, AI_maxGoldTrade((PlayerTypes)iI));

													if (iGold > 0)
													{
														setTradeItem(&item, TRADE_GOLD, iGold);

														if (canTradeItem(((PlayerTypes)iI), item, true))
														{
															iGiveGold = iGold;
															iTheirValue += (iGold * iGoldValuePercent) / 100;
														}
													}
												}

												if (iTheirValue > (iOurValue * 3 / 4))
												{
													ourList.clear();
													theirList.clear();

													if (eBestGiveTech != NO_TECH)
													{
														setTradeItem(&item, TRADE_TECHNOLOGIES, eBestGiveTech);
														ourList.insertAtEnd(item);
													}
													
													if (eBestGiveTech2 != NO_TECH)
													{
														setTradeItem(&item, TRADE_TECHNOLOGIES, eBestGiveTech2);
														ourList.insertAtEnd(item);
													}

													setTradeItem(&item, TRADE_WAR, eBestTeam);
													theirList.insertAtEnd(item);

													if (iGiveGold != 0)
													{
														setTradeItem(&item, TRADE_GOLD, iGiveGold);
														ourList.insertAtEnd(item);
													}

													if (iReceiveGold != 0)
													{
														setTradeItem(&item, TRADE_GOLD, iReceiveGold);
														theirList.insertAtEnd(item);
													}

													GC.getGameINLINE().implementDeal(getID(), ((PlayerTypes)iI), &ourList, &theirList);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


//
// read object from a stream
// used during load
//
void CvPlayerAI::read(FDataStreamBase* pStream)
{
	CvPlayer::read(pStream);	// read base class data first

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iPeaceWeight);
	pStream->Read(&m_iEspionageWeight);
	pStream->Read(&m_iAttackOddsChange);
	pStream->Read(&m_iCivicTimer);
	pStream->Read(&m_iReligionTimer);
	pStream->Read(&m_iExtraGoldTarget);

	pStream->Read(&m_iStrategyHash);
	pStream->Read(&m_iStrategyHashCacheTurn);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/18/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI, War strategy AI                                                         */
/************************************************************************************************/
	if( uiFlag < 3 )
	{
		m_iStrategyHash = 0;
		m_iStrategyHashCacheTurn = -1;
	}

	if( uiFlag > 2 )
	{
		pStream->Read(&m_iStrategyRand);
	}
	else
	{
		m_iStrategyRand = 0;
	}

	if( uiFlag > 0 )
	{
		pStream->Read(&m_iVictoryStrategyHash);
		pStream->Read(&m_iVictoryStrategyHashCacheTurn);
	}
	else
	{
		m_iVictoryStrategyHash = 0;
		m_iVictoryStrategyHashCacheTurn = -1;
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/		
/************************************************************************************************/
/* RevDCM	                  Start		 12/9/09                                                */
/*                                                                                              */
/* Inquisitions                                                                                 */
/************************************************************************************************/
	if( uiFlag > 1 )
	{
		pStream->Read(&m_bPushReligiousVictory);
		pStream->Read(&m_bConsiderReligiousVictory);
		pStream->Read(&m_bHasInquisitionTarget);
	}
	else
	{
		m_bPushReligiousVictory = false;
		m_bConsiderReligiousVictory = false;
		m_bHasInquisitionTarget = false;
	}
/************************************************************************************************/
/* RevDCM	                     END                                                            */
/************************************************************************************************/
	pStream->Read(&m_iAveragesCacheTurn);
	pStream->Read(&m_iAverageGreatPeopleMultiplier);

	pStream->Read(NUM_YIELD_TYPES, m_aiAverageYieldMultiplier);
	pStream->Read(NUM_COMMERCE_TYPES, m_aiAverageCommerceMultiplier);
	pStream->Read(NUM_COMMERCE_TYPES, m_aiAverageCommerceExchange);

	pStream->Read(&m_iUpgradeUnitsCacheTurn);
	pStream->Read(&m_iUpgradeUnitsCachedExpThreshold);
	pStream->Read(&m_iUpgradeUnitsCachedGold);

	pStream->Read(NUM_UNITAI_TYPES, m_aiNumTrainAIUnits);
	pStream->Read(NUM_UNITAI_TYPES, m_aiNumAIUnits);
	pStream->Read(MAX_PLAYERS, m_aiSameReligionCounter);
	pStream->Read(MAX_PLAYERS, m_aiDifferentReligionCounter);
	pStream->Read(MAX_PLAYERS, m_aiFavoriteCivicCounter);
	pStream->Read(MAX_PLAYERS, m_aiBonusTradeCounter);
	pStream->Read(MAX_PLAYERS, m_aiPeacetimeTradeValue);
	pStream->Read(MAX_PLAYERS, m_aiPeacetimeGrantValue);
	pStream->Read(MAX_PLAYERS, m_aiGoldTradedTo);
	pStream->Read(MAX_PLAYERS, m_aiAttitudeExtra);

	pStream->Read(MAX_PLAYERS, m_abFirstContact);

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		pStream->Read(NUM_CONTACT_TYPES, m_aaiContactTimer[i]);
	}
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		pStream->Read(NUM_MEMORY_TYPES, m_aaiMemoryCount[i]);
	}
	
	

	pStream->Read(&m_bWasFinancialTrouble);
	pStream->Read(&m_iTurnLastProductionDirty);
	
	{
		m_aiAICitySites.clear();
		uint iSize;
		pStream->Read(&iSize);
		for (uint i = 0; i < iSize; i++)
		{
			int iCitySite;
			pStream->Read(&iCitySite);
			m_aiAICitySites.push_back(iCitySite);
		}
	}

	pStream->Read(GC.getNumBonusInfos(), m_aiBonusValue);
	pStream->Read(GC.getNumUnitClassInfos(), m_aiUnitClassWeights);
	pStream->Read(GC.getNumUnitCombatInfos(), m_aiUnitCombatWeights);
	pStream->Read(MAX_PLAYERS, m_aiCloseBordersAttitudeCache);

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/03/09                       poyuzhe & jdog5000     */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
	// From Sanguo Mod Performance, ie the CAR Mod
	// Attitude cache
	AI_invalidateAttitudeCache();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

}


//
// save object to a stream
// used during save
//
void CvPlayerAI::write(FDataStreamBase* pStream)
{
	CvPlayer::write(pStream);	// write base class data first

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/18/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
/*
	uint uiFlag=0;
*/
	// Flag for type of save
	uint uiFlag=3;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/		
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iPeaceWeight);
	pStream->Write(m_iEspionageWeight);
	pStream->Write(m_iAttackOddsChange);
	pStream->Write(m_iCivicTimer);
	pStream->Write(m_iReligionTimer);
	pStream->Write(m_iExtraGoldTarget);

	pStream->Write(m_iStrategyHash);
	pStream->Write(m_iStrategyHashCacheTurn);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/18/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
	pStream->Write(m_iStrategyRand);
	pStream->Write(m_iVictoryStrategyHash);
	pStream->Write(m_iVictoryStrategyHashCacheTurn);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/		
/************************************************************************************************/
/* RevDCM	                  Start		 12/9/09                                                */
/*                                                                                              */
/* Inquisitions                                                                                 */
/************************************************************************************************/
	pStream->Write(m_bPushReligiousVictory);
	pStream->Write(m_bConsiderReligiousVictory);
	pStream->Write(m_bHasInquisitionTarget);
/************************************************************************************************/
/* RevDCM	                     END                                                            */
/************************************************************************************************/
	pStream->Write(m_iAveragesCacheTurn);
	pStream->Write(m_iAverageGreatPeopleMultiplier);

	pStream->Write(NUM_YIELD_TYPES, m_aiAverageYieldMultiplier);
	pStream->Write(NUM_COMMERCE_TYPES, m_aiAverageCommerceMultiplier);
	pStream->Write(NUM_COMMERCE_TYPES, m_aiAverageCommerceExchange);

	pStream->Write(m_iUpgradeUnitsCacheTurn);
	pStream->Write(m_iUpgradeUnitsCachedExpThreshold);
	pStream->Write(m_iUpgradeUnitsCachedGold);

	pStream->Write(NUM_UNITAI_TYPES, m_aiNumTrainAIUnits);
	pStream->Write(NUM_UNITAI_TYPES, m_aiNumAIUnits);
	pStream->Write(MAX_PLAYERS, m_aiSameReligionCounter);
	pStream->Write(MAX_PLAYERS, m_aiDifferentReligionCounter);
	pStream->Write(MAX_PLAYERS, m_aiFavoriteCivicCounter);
	pStream->Write(MAX_PLAYERS, m_aiBonusTradeCounter);
	pStream->Write(MAX_PLAYERS, m_aiPeacetimeTradeValue);
	pStream->Write(MAX_PLAYERS, m_aiPeacetimeGrantValue);
	pStream->Write(MAX_PLAYERS, m_aiGoldTradedTo);
	pStream->Write(MAX_PLAYERS, m_aiAttitudeExtra);

	pStream->Write(MAX_PLAYERS, m_abFirstContact);

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		pStream->Write(NUM_CONTACT_TYPES, m_aaiContactTimer[i]);
	}
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		pStream->Write(NUM_MEMORY_TYPES, m_aaiMemoryCount[i]);
	}

	pStream->Write(m_bWasFinancialTrouble);
	pStream->Write(m_iTurnLastProductionDirty);
	
	{
		uint iSize = m_aiAICitySites.size();
		pStream->Write(iSize);
		std::vector<int>::iterator it;
		for (it = m_aiAICitySites.begin(); it != m_aiAICitySites.end(); ++it)
		{
			pStream->Write((*it));
		}
	}

	pStream->Write(GC.getNumBonusInfos(), m_aiBonusValue);
	pStream->Write(GC.getNumUnitClassInfos(), m_aiUnitClassWeights);
	pStream->Write(GC.getNumUnitCombatInfos(), m_aiUnitCombatWeights);
	pStream->Write(MAX_PLAYERS, m_aiCloseBordersAttitudeCache);
}


int CvPlayerAI::AI_eventValue(EventTypes eEvent, const EventTriggeredData& kTriggeredData) const
{
	CvEventTriggerInfo& kTrigger = GC.getEventTriggerInfo(kTriggeredData.m_eTrigger);
	CvEventInfo& kEvent = GC.getEventInfo(eEvent);

	int iNumCities = getNumCities();
	CvCity* pCity = getCity(kTriggeredData.m_iCityId);
	CvPlot* pPlot = GC.getMapINLINE().plot(kTriggeredData.m_iPlotX, kTriggeredData.m_iPlotY);
	CvUnit* pUnit = getUnit(kTriggeredData.m_iUnitId);

	int iHappy = 0;
	int iHealth = 0;
	int aiYields[NUM_YIELD_TYPES];
	int aiCommerceYields[NUM_COMMERCE_TYPES];
	
	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aiYields[iI] = 0;
	}
	for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aiCommerceYields[iI] = 0;
	}

	if (NO_PLAYER != kTriggeredData.m_eOtherPlayer)
	{
		if (kEvent.isDeclareWar())
		{
			switch (AI_getAttitude(kTriggeredData.m_eOtherPlayer))
			{
			case ATTITUDE_FURIOUS:
			case ATTITUDE_ANNOYED:
			case ATTITUDE_CAUTIOUS:
				if (GET_TEAM(getTeam()).getDefensivePower() < GET_TEAM(GET_PLAYER(kTriggeredData.m_eOtherPlayer).getTeam()).getPower(true))
				{
					return -MAX_INT + 1;
				}
				break;
			case ATTITUDE_PLEASED:
			case ATTITUDE_FRIENDLY:
				return -MAX_INT + 1;
				break;
			}
		}
	}

	//Proportional to #turns in the game...
	//(AI evaluation will generally assume proper game speed scaling!)
	int iGameSpeedPercent = GC.getGameSpeedInfo(GC.getGame().getGameSpeedType()).getVictoryDelayPercent();

	int iValue = GC.getGameINLINE().getSorenRandNum(kEvent.getAIValue(), "AI Event choice");
	iValue += (getEventCost(eEvent, kTriggeredData.m_eOtherPlayer, false) + getEventCost(eEvent, kTriggeredData.m_eOtherPlayer, true)) / 2;

	iValue += kEvent.getEspionagePoints();

	if (kEvent.getTech() != NO_TECH)
	{
		iValue += (GET_TEAM(getTeam()).getResearchCost((TechTypes)kEvent.getTech()) * kEvent.getTechPercent()) / 100;
	}

	if (kEvent.getUnitClass() != NO_UNITCLASS)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(kEvent.getUnitClass());
		if (eUnit != NO_UNIT)
		{
			//Altough AI_unitValue compares well within units, the value is somewhat independent of cost
			int iUnitValue = GC.getUnitInfo(eUnit).getProductionCost();
			if (iUnitValue > 0)
			{
				iUnitValue *= 2;
			}
			else if (iUnitValue == -1)
			{
				iUnitValue = 200; //Great Person?
			}

			iUnitValue *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();
			iValue += kEvent.getNumUnits() * iUnitValue;
		}
	}
	
	if (kEvent.isDisbandUnit())
	{
		CvUnit* pUnit = getUnit(kTriggeredData.m_iUnitId);
		if (NULL != pUnit)
		{
			int iUnitValue = pUnit->getUnitInfo().getProductionCost();
			if (iUnitValue > 0)
			{
				iUnitValue *= 2;
			}
			else if (iUnitValue == -1)
			{
				iUnitValue = 200; //Great Person?
			}

			iUnitValue *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();
			iValue -= iUnitValue;
		}
	}

	if (kEvent.getBuildingClass() != NO_BUILDINGCLASS)
	{
		BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(kEvent.getBuildingClass());
		if (eBuilding != NO_BUILDING)
		{
			if (pCity)
			{
				//iValue += kEvent.getBuildingChange() * pCity->AI_buildingValue(eBuilding);
				int iBuildingValue = GC.getBuildingInfo(eBuilding).getProductionCost();
				if (iBuildingValue > 0)
				{
					iBuildingValue *= 2;
				}
				else if (iBuildingValue == -1)
				{
					iBuildingValue = 300; //Shrine?
				}

				iBuildingValue *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
				iValue += kEvent.getBuildingChange() * iBuildingValue;
			}
		}
	}

	TechTypes eBestTech = NO_TECH;
	int iBestValue = 0;
	for (int iTech = 0; iTech < GC.getNumTechInfos(); ++iTech)
	{
		if (canResearch((TechTypes)iTech))
		{
			if (NO_PLAYER == kTriggeredData.m_eOtherPlayer || GET_TEAM(GET_PLAYER(kTriggeredData.m_eOtherPlayer).getTeam()).isHasTech((TechTypes)iTech))
			{
				int iValue = 0;
				for (int i = 0; i < GC.getNumFlavorTypes(); ++i)
				{
					iValue += kEvent.getTechFlavorValue(i) * GC.getTechInfo((TechTypes)iTech).getFlavorValue(i);
				}

				if (iValue > iBestValue)
				{
					eBestTech = (TechTypes)iTech;
					iBestValue = iValue;
				}
			}
		}
	}

	if (eBestTech != NO_TECH)
	{
		iValue += (GET_TEAM(getTeam()).getResearchCost(eBestTech) * kEvent.getTechPercent()) / 100;
	}

	if (kEvent.isGoldenAge())
	{
		iValue += AI_calculateGoldenAgeValue();
	}

	{	//Yield and other changes
		if (kEvent.getNumBuildingYieldChanges() > 0)
		{
			for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
			{
				for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
				{
					aiYields[iYield] += kEvent.getBuildingYieldChange(iBuildingClass, iYield);
				}
			}
		}

		if (kEvent.getNumBuildingCommerceChanges() > 0)
		{
			for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
			{
				for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; ++iCommerce)
				{
					aiCommerceYields[iCommerce] += kEvent.getBuildingCommerceChange(iBuildingClass, iCommerce);
				}
			}
		}

		if (kEvent.getNumBuildingHappyChanges() > 0)
		{
			for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
			{
				iHappy += kEvent.getBuildingHappyChange(iBuildingClass);
			}
		}

		if (kEvent.getNumBuildingHealthChanges() > 0)
		{
			for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
			{
				iHealth += kEvent.getBuildingHealthChange(iBuildingClass);
			}
		}
	}
	
	if (kEvent.isCityEffect())
	{
		int iCityPopulation = -1;
		int iCityTurnValue = 0;
		if (NULL != pCity)
		{
			iCityPopulation = pCity->getPopulation();
			for (int iSpecialist = 0; iSpecialist < GC.getNumSpecialistInfos(); ++iSpecialist)
			{
				if (kEvent.getFreeSpecialistCount(iSpecialist) > 0)
				{
					iCityTurnValue += (pCity->AI_specialistValue((SpecialistTypes)iSpecialist, false, false) / 300);
				}
			}
		}

		if (-1 == iCityPopulation)
		{
			//What is going on here?
			iCityPopulation = 5;
		}

		iCityTurnValue += ((iHappy + kEvent.getHappy()) * 8);
		iCityTurnValue += ((iHealth + kEvent.getHealth()) * 6);
		
		iCityTurnValue += aiYields[YIELD_FOOD] * 5;
		iCityTurnValue += aiYields[YIELD_PRODUCTION] * 5;
		iCityTurnValue += aiYields[YIELD_COMMERCE] * 3;
		
		iCityTurnValue += aiCommerceYields[COMMERCE_RESEARCH] * 3;
		iCityTurnValue += aiCommerceYields[COMMERCE_GOLD] * 3;
		iCityTurnValue += aiCommerceYields[COMMERCE_CULTURE] * 1;
		iCityTurnValue += aiCommerceYields[COMMERCE_ESPIONAGE] * 2;

		iValue += (iCityTurnValue * 20 * iGameSpeedPercent) / 100;

		iValue += kEvent.getFood();
		iValue += kEvent.getFoodPercent() / 4;
		iValue += kEvent.getPopulationChange() * 30;
		iValue -= kEvent.getRevoltTurns() * (12 + iCityPopulation * 16);
		iValue -= (kEvent.getHurryAnger() * 6 * GC.getDefineINT("HURRY_ANGER_DIVISOR") * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getHurryConscriptAngerPercent()) / 100;
		iValue += kEvent.getHappyTurns() * 10;
		iValue += kEvent.getCulture() / 2;
	}
	else if (!kEvent.isOtherPlayerCityEffect())
	{
		int iPerTurnValue = 0;
		iPerTurnValue += iNumCities * ((iHappy * 4) + (kEvent.getHappy() * 8));
		iPerTurnValue += iNumCities * ((iHealth * 3) + (kEvent.getHealth() * 6));
		
		iValue += (iPerTurnValue * 20 * iGameSpeedPercent) / 100;
		
		iValue += (kEvent.getFood() * iNumCities);
		iValue += (kEvent.getFoodPercent() * iNumCities) / 4;
		iValue += (kEvent.getPopulationChange() * iNumCities * 40);
		iValue -= (iNumCities * kEvent.getHurryAnger() * 6 * GC.getDefineINT("HURRY_ANGER_DIVISOR") * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getHurryConscriptAngerPercent()) / 100;
		iValue += iNumCities * kEvent.getHappyTurns() * 10;
		iValue += iNumCities * kEvent.getCulture() / 2;
	}
	
	int iBonusValue = 0;
	if (NO_BONUS != kEvent.getBonus())
	{
		iBonusValue = AI_bonusVal((BonusTypes)kEvent.getBonus());
	}

	if (NULL != pPlot)
	{
		if (kEvent.getFeatureChange() != 0)
		{
			int iOldFeatureValue = 0;
			int iNewFeatureValue = 0;
			if (pPlot->getFeatureType() != NO_FEATURE)
			{
				//*grumble* who tied feature production to builds rather than the feature...
				iOldFeatureValue = GC.getFeatureInfo(pPlot->getFeatureType()).getHealthPercent();

				if (kEvent.getFeatureChange() > 0)
				{
					FeatureTypes eFeature = (FeatureTypes)kEvent.getFeature();
					FAssert(eFeature != NO_FEATURE);
					if (eFeature != NO_FEATURE)
					{
						iNewFeatureValue = GC.getFeatureInfo(eFeature).getHealthPercent();
					}
				}

				iValue += ((iNewFeatureValue - iOldFeatureValue) * iGameSpeedPercent) / 100;
			}
		}

		if (kEvent.getImprovementChange() > 0)
		{
			iValue += (30 * iGameSpeedPercent) / 100;
		}
		else if (kEvent.getImprovementChange() < 0)
		{
			iValue -= (30 * iGameSpeedPercent) / 100;
		}

		if (kEvent.getRouteChange() > 0)
		{
			iValue += (10 * iGameSpeedPercent) / 100;
		}
		else if (kEvent.getRouteChange() < 0)
		{
			iValue -= (10 * iGameSpeedPercent) / 100;
		}

		if (kEvent.getBonusChange() > 0)
		{
			iValue += (iBonusValue * 15 * iGameSpeedPercent) / 100;
		}
		else if (kEvent.getBonusChange() < 0)
		{
			iValue -= (iBonusValue * 15 * iGameSpeedPercent) / 100;
		}
		
		for (int i = 0; i < NUM_YIELD_TYPES; ++i)
		{
			if (0 != kEvent.getPlotExtraYield(i))
			{
				if (pPlot->getWorkingCity() != NULL)
				{
					FAssertMsg(pPlot->getWorkingCity()->getOwner() == getID(), "Event creates a boni for another player?");
					aiYields[i] += kEvent.getPlotExtraYield(i);
				}
				else
				{
					iValue += (20 * 8 * kEvent.getPlotExtraYield(i) * iGameSpeedPercent) / 100;
				}
			}
		}
	}

	if (NO_BONUS != kEvent.getBonusRevealed())
	{
		iValue += (iBonusValue * 10 * iGameSpeedPercent) / 100;
	}

	if (NULL != pUnit)
	{
		iValue += (2 * pUnit->baseCombatStr() * kEvent.getUnitExperience() * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent()) / 100;

		iValue -= 10 * kEvent.getUnitImmobileTurns();
	}

	{
		int iPromotionValue = 0;
		
		for (int i = 0; i < GC.getNumUnitCombatInfos(); ++i)
		{
			if (NO_PROMOTION != kEvent.getUnitCombatPromotion(i))
			{
				int iLoop;
				for (CvUnit* pLoopUnit = firstUnit(&iLoop); NULL != pLoopUnit; pLoopUnit = nextUnit(&iLoop))
				{
					if (pLoopUnit->getUnitCombatType() == i)
					{
						if (!pLoopUnit->isHasPromotion((PromotionTypes)kEvent.getUnitCombatPromotion(i)))
						{
							iPromotionValue += 5 * pLoopUnit->baseCombatStr();
						}
					}
				}

				iPromotionValue += iNumCities * 50;
			}
		}
		
		iValue += (iPromotionValue * iGameSpeedPercent) / 100;
	}

	if (kEvent.getFreeUnitSupport() != 0)
	{
		iValue += (20 * kEvent.getFreeUnitSupport() * iGameSpeedPercent) / 100;
	}
	
	if (kEvent.getInflationModifier() != 0)
	{
		iValue -= (20 * kEvent.getInflationModifier() * calculatePreInflatedCosts() * iGameSpeedPercent) / 100;		
	}

	if (kEvent.getSpaceProductionModifier() != 0)
	{
		iValue += ((20 + iNumCities) * getSpaceProductionModifier() * iGameSpeedPercent) / 100;
	}

	int iOtherPlayerAttitudeWeight = 0;
	if (kTriggeredData.m_eOtherPlayer != NO_PLAYER)
	{
		iOtherPlayerAttitudeWeight = AI_getAttitudeWeight(kTriggeredData.m_eOtherPlayer);
		iOtherPlayerAttitudeWeight += 10 - GC.getGame().getSorenRandNum(20, "AI event value attitude");
	}

	//Religion
	if (kTriggeredData.m_eReligion != NO_RELIGION)
	{
		ReligionTypes eReligion = kTriggeredData.m_eReligion;
		
		int iReligionValue = 15;
		
		if (getStateReligion() == eReligion)
		{
			iReligionValue += 15;
		}
		if (hasHolyCity(eReligion))
		{
			iReligionValue += 15;
		}
	
		iValue += (kEvent.getConvertOwnCities() * iReligionValue * iGameSpeedPercent) / 100;

		if (kEvent.getConvertOtherCities() > 0)
		{
			//Don't like them much = fairly indifferent, hate them = negative.
			iValue += (kEvent.getConvertOtherCities() * (iOtherPlayerAttitudeWeight + 50) * iReligionValue * iGameSpeedPercent) / 15000;
		}
	}

	if (NO_PLAYER != kTriggeredData.m_eOtherPlayer)
	{
		CvPlayerAI& kOtherPlayer = GET_PLAYER(kTriggeredData.m_eOtherPlayer);

		int iDiploValue = 0;
		//if we like this player then value positive attitude, if however we really hate them then
		//actually value negative attitude.
		iDiploValue += ((iOtherPlayerAttitudeWeight + 50) * kEvent.getAttitudeModifier() * GET_PLAYER(kTriggeredData.m_eOtherPlayer).getPower()) / std::max(1, getPower());
		
		if (kEvent.getTheirEnemyAttitudeModifier() != 0)
		{
			//Oh wow this sure is mildly complicated.
			TeamTypes eWorstEnemy = GET_TEAM(GET_PLAYER(kTriggeredData.m_eOtherPlayer).getTeam()).AI_getWorstEnemy();
			
			if (NO_TEAM != eWorstEnemy && eWorstEnemy != getTeam())
			{
			int iThirdPartyAttitudeWeight = GET_TEAM(getTeam()).AI_getAttitudeWeight(eWorstEnemy);
			
			//If we like both teams, we want them to get along.
			//If we like otherPlayer but not enemy (or vice-verca), we don't want them to get along.
			//If we don't like either, we don't want them to get along.
			//Also just value stirring up trouble in general.
			
			int iThirdPartyDiploValue = 50 * kEvent.getTheirEnemyAttitudeModifier();
			iThirdPartyDiploValue *= (iThirdPartyAttitudeWeight - 10);
			iThirdPartyDiploValue *= (iOtherPlayerAttitudeWeight - 10);
			iThirdPartyDiploValue /= 10000;
			
			if ((iOtherPlayerAttitudeWeight) < 0 && (iThirdPartyAttitudeWeight < 0))
			{
				iThirdPartyDiploValue *= -1;
			}
			
			iThirdPartyDiploValue /= 2;
			
			iDiploValue += iThirdPartyDiploValue;
		}
		}
		
		iDiploValue *= iGameSpeedPercent;
		iDiploValue /= 100;
		
		if (NO_BONUS != kEvent.getBonusGift())
		{
			int iBonusValue = -AI_bonusVal((BonusTypes)kEvent.getBonusGift(), -1);
			iBonusValue += (iOtherPlayerAttitudeWeight - 40) * kOtherPlayer.AI_bonusVal((BonusTypes)kEvent.getBonusGift(), +1);
			//Positive for friends, negative for enemies.
			iDiploValue += (iBonusValue * GC.getDefineINT("PEACE_TREATY_LENGTH")) / 60;
		}
		
		if (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
		{
			//What is this "relationships" thing?
			iDiploValue /= 2;
		}
		
		if (kEvent.isGoldToPlayer())
		{
			//If the gold goes to another player instead of the void, then this is a positive
			//thing if we like the player, otherwise it's a negative thing.
			int iGiftValue = (getEventCost(eEvent, kTriggeredData.m_eOtherPlayer, false) + getEventCost(eEvent, kTriggeredData.m_eOtherPlayer, true)) / 2;
			iGiftValue *= -iOtherPlayerAttitudeWeight;
			iGiftValue /= 110;
			
			iValue += iGiftValue;
		}

		if (kEvent.isDeclareWar())
		{
			int iWarValue = (GET_TEAM(getTeam()).getDefensivePower() - GET_TEAM(GET_PLAYER(kTriggeredData.m_eOtherPlayer).getTeam()).getPower(true));// / std::max(1, GET_TEAM(getTeam()).getDefensivePower());
			iWarValue -= 30 * AI_getAttitudeVal(kTriggeredData.m_eOtherPlayer);
		}
			
		if (kEvent.getMaxPillage() > 0)
		{
			int iPillageValue = (40 * (kEvent.getMinPillage() + kEvent.getMaxPillage())) / 2;
			//If we hate them, this is good to do.
			iPillageValue *= 25 - iOtherPlayerAttitudeWeight;
			iPillageValue *= iGameSpeedPercent;
			iPillageValue /= 12500;
		}

		iValue += (iDiploValue * iGameSpeedPercent) / 100;
	}

	int iThisEventValue = iValue;
	//XXX THIS IS VULNERABLE TO NON-TRIVIAL RECURSIONS!
	//Event A effects Event B, Event B effects Event A
	for (int iEvent = 0; iEvent < GC.getNumEventInfos(); ++iEvent)
	{
		if (kEvent.getAdditionalEventChance(iEvent) > 0)
		{
			if (iEvent == eEvent)
			{
				//Infinite recursion is not our friend.
				//Fortunately we have the event value for this event - sans values of other events
				//disabled or cleared. Hopefully no events will be that complicated...
				//Double the value since it's recursive.
				iValue += (kEvent.getAdditionalEventChance(iEvent) * iThisEventValue) / 50;
			}
			else
			{
				iValue += (kEvent.getAdditionalEventChance(iEvent) * AI_eventValue((EventTypes)iEvent, kTriggeredData)) / 100;
			}
		}
	
		if (kEvent.getClearEventChance(iEvent) > 0)
		{
			if (iEvent == eEvent)
			{
				iValue -= (kEvent.getClearEventChance(iEvent) * iThisEventValue) / 50;
			}
			else
			{
				iValue -= (kEvent.getClearEventChance(iEvent) * AI_eventValue((EventTypes)iEvent, kTriggeredData)) / 100;
			}
		}
	}

	iValue *= 100 + GC.getGameINLINE().getSorenRandNum(20, "AI Event choice");
	iValue /= 100;

	return iValue;
}

EventTypes CvPlayerAI::AI_chooseEvent(int iTriggeredId) const
{
	EventTriggeredData* pTriggeredData = getEventTriggered(iTriggeredId);
	if (NULL == pTriggeredData)
	{
		return NO_EVENT;
	}

	CvEventTriggerInfo& kTrigger = GC.getEventTriggerInfo(pTriggeredData->m_eTrigger);

	int iBestValue = -MAX_INT;
	EventTypes eBestEvent = NO_EVENT;

	for (int i = 0; i < kTrigger.getNumEvents(); i++)
	{
		int iValue = -MAX_INT;
		if (kTrigger.getEvent(i) != NO_EVENT)
		{
			CvEventInfo& kEvent = GC.getEventInfo((EventTypes)kTrigger.getEvent(i));
			if (canDoEvent((EventTypes)kTrigger.getEvent(i), *pTriggeredData))
			{
				iValue = AI_eventValue((EventTypes)kTrigger.getEvent(i), *pTriggeredData);
			}
		}

		if (iValue > iBestValue)
		{
			iBestValue = iValue;
			eBestEvent = (EventTypes)kTrigger.getEvent(i);
		}
	}

	return eBestEvent;
}

void CvPlayerAI::AI_doSplit()
{
	PROFILE_FUNC();

	if (!canSplitEmpire())
	{
		return;
	}

	int iLoop;
	std::map<int, int> mapAreaValues;

	for (CvArea* pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
	{
		mapAreaValues[pLoopArea->getID()] = 0;
	}

	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		mapAreaValues[pLoopCity->area()->getID()] += pLoopCity->AI_cityValue();
	}

	std::map<int, int>::iterator it;
	for (it = mapAreaValues.begin(); it != mapAreaValues.end(); ++it)
	{
		if (it->second < 0)
		{
			int iAreaId = it->first;

			if (canSplitArea(iAreaId))
			{
				splitEmpire(iAreaId);

				for (CvUnit* pUnit = firstUnit(&iLoop); pUnit != NULL; pUnit = nextUnit(&iLoop))
				{
					if (pUnit->area()->getID() == iAreaId)
					{
						TeamTypes ePlotTeam = pUnit->plot()->getTeam();

						if (NO_TEAM != ePlotTeam)
						{
							CvTeam& kPlotTeam = GET_TEAM(ePlotTeam);
							if (kPlotTeam.isVassal(getTeam()) && GET_TEAM(getTeam()).isParent(ePlotTeam))
							{
								pUnit->gift();
							}
						}
					}
				}
				break;
			}
		}
	}
}

void CvPlayerAI::AI_launch(VictoryTypes eVictory)
{
	if (GET_TEAM(getTeam()).isHuman())
	{
		return;
	}

	if (!GET_TEAM(getTeam()).canLaunch(eVictory))
	{
		return;
	}

	bool bLaunch = true;

	int iBestArrival = MAX_INT;
	TeamTypes eBestTeam = NO_TEAM;

	for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; ++iTeam)
	{
		if (iTeam != getTeam())
		{
			CvTeam& kTeam = GET_TEAM((TeamTypes)iTeam);
			if (kTeam.isAlive())
			{
				int iCountdown = kTeam.getVictoryCountdown(eVictory);
				if (iCountdown > 0)
				{
					if (iCountdown < iBestArrival)
					{
						iBestArrival = iCountdown;
						eBestTeam = (TeamTypes)iTeam;
					}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/07/09                       r_rolo1 & jdog5000     */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/* original BTS code
					if (iCountdown < GET_TEAM(getTeam()).getVictoryDelay(eVictory) && kTeam.getLaunchSuccessRate(eVictory) == 100)
					{
						bLaunch = false;
						break;
					}
*/
					// Other civs capital could still be captured, might as well send spaceship
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
				}
			}
		}
	}

	if (bLaunch)
	{
		if (NO_TEAM == eBestTeam || iBestArrival > GET_TEAM(getTeam()).getVictoryDelay(eVictory))
		{
			if (GET_TEAM(getTeam()).getLaunchSuccessRate(eVictory) < 100)
			{
				bLaunch = false;
			}
		}
	}

	if (bLaunch)
	{
		launch(eVictory);
	}
}

void CvPlayerAI::AI_doCheckFinancialTrouble()
{
	// if we just ran into financial trouble this turn
	bool bFinancialTrouble = AI_isFinancialTrouble();
	if (bFinancialTrouble != m_bWasFinancialTrouble)
	{
		if (bFinancialTrouble)
		{
			int iGameTurn = GC.getGameINLINE().getGameTurn();
			
			// only reset at most every 10 turns
			if (iGameTurn > m_iTurnLastProductionDirty + 10)
			{
				// redeterimine the best things to build in each city
				AI_makeProductionDirty();
			
				m_iTurnLastProductionDirty = iGameTurn;
			}
		}
		
		m_bWasFinancialTrouble = bFinancialTrouble;
	}
}

bool CvPlayerAI::AI_disbandUnit(int iExpThreshold, bool bObsolete)
{
	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = MAX_INT;
	pBestUnit = NULL;

	for(pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		if (!(pLoopUnit->hasCargo()))
		{
			if (!(pLoopUnit->isGoldenAge()))
			{
				if (pLoopUnit->getUnitInfo().getProductionCost() > 0)
				{
					if ((iExpThreshold == -1) || (pLoopUnit->canFight() && pLoopUnit->getExperience() <= iExpThreshold))
					{
						if (!(pLoopUnit->isMilitaryHappiness()) || !(pLoopUnit->plot()->isCity()) || (pLoopUnit->plot()->plotCount(PUF_isMilitaryHappiness, -1, -1, getID()) > 2))
						{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      01/12/10                                jdog5000      */
/*                                                                                              */
/* Gold AI                                                                                      */
/************************************************************************************************/
							iValue = (10000 + GC.getGameINLINE().getSorenRandNum(1000, "Disband Unit"));

							iValue *= 100 + (pLoopUnit->getUnitInfo().getProductionCost() * 3);
							iValue /= 100;

							iValue *= 100 + (pLoopUnit->getExperience() * 10);
							iValue /= 100;
							
							iValue *= 100 + (pLoopUnit->getLevel() * 25);
							iValue /= 100;

							if (pLoopUnit->plot()->getTeam() == pLoopUnit->getTeam())
							{
								iValue *= 3;

								if (pLoopUnit->canDefend() && pLoopUnit->plot()->isCity())
								{
									iValue *= 2;
								}
							}

							// Multiplying by higher number means unit has higher priority, less likely to be disbanded
							switch (pLoopUnit->AI_getUnitAIType())
							{
							case UNITAI_UNKNOWN:
							case UNITAI_ANIMAL:
								break;

							case UNITAI_SETTLE:
								iValue *= 20;
								break;

							case UNITAI_WORKER:
								if ((GC.getGame().getGameTurn() - pLoopUnit->getGameTurnCreated()) > 10)
								{
									if (pLoopUnit->plot()->isCity())
									{
										if (pLoopUnit->plot()->getPlotCity()->AI_getWorkersNeeded() == 0)
										{
											iValue *= 10;
										}
									}
								}
								break;

							case UNITAI_ATTACK:
							case UNITAI_ATTACK_CITY:
							case UNITAI_COLLATERAL:
							case UNITAI_PILLAGE:
							case UNITAI_RESERVE:
							case UNITAI_COUNTER:
								iValue *= 2;
								break;

							case UNITAI_CITY_DEFENSE:
							case UNITAI_CITY_COUNTER:
							case UNITAI_CITY_SPECIAL:
							case UNITAI_PARADROP:
								iValue *= 6;
								break;

							case UNITAI_EXPLORE:
								if ((GC.getGame().getGameTurn() - pLoopUnit->getGameTurnCreated()) < 10 
									|| pLoopUnit->plot()->getTeam() != getTeam())
								{
									iValue *= 15;
								}
								else
								{
									iValue *= 2;
								}
								break;

							case UNITAI_MISSIONARY:
								if ((GC.getGame().getGameTurn() - pLoopUnit->getGameTurnCreated()) < 10 
									|| pLoopUnit->plot()->getTeam() != getTeam())
								{
									iValue *= 8;
								}
								break;

							case UNITAI_PROPHET:
							case UNITAI_ARTIST:
							case UNITAI_SCIENTIST:
							case UNITAI_GENERAL:
							case UNITAI_MERCHANT:
							case UNITAI_ENGINEER:
								iValue *= 20;
								break;

							case UNITAI_SPY:
								iValue *= 12;
								break;

							case UNITAI_ICBM:
								iValue *= 4;
								break;

							case UNITAI_WORKER_SEA:
								iValue *= 18;
								break;

							case UNITAI_ATTACK_SEA:
							case UNITAI_RESERVE_SEA:
							case UNITAI_ESCORT_SEA:
								break;

							case UNITAI_EXPLORE_SEA:
								if ((GC.getGame().getGameTurn() - pLoopUnit->getGameTurnCreated()) < 10 
									|| pLoopUnit->plot()->getTeam() != getTeam())
								{
									iValue *= 12;
								}
								break;

							case UNITAI_SETTLER_SEA:
								iValue *= 6;

							case UNITAI_MISSIONARY_SEA:
							case UNITAI_SPY_SEA:
								iValue *= 4;

							case UNITAI_ASSAULT_SEA:
							case UNITAI_CARRIER_SEA:
							case UNITAI_MISSILE_CARRIER_SEA:
								if( GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0 )
								{
									iValue *= 5;
								}
								else
								{
									iValue *= 2;
								}
								break;

							case UNITAI_PIRATE_SEA:
							case UNITAI_ATTACK_AIR:
								break;

							case UNITAI_DEFENSE_AIR:
							case UNITAI_CARRIER_AIR:
							case UNITAI_MISSILE_AIR:
								if( GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0 )
								{
									iValue *= 5;
								}
								else
								{
									iValue *= 3;
								}
								break;

							default:
								FAssert(false);
								break;
							}

							if (pLoopUnit->getUnitInfo().getExtraCost() > 0)
							{
								iValue /= (pLoopUnit->getUnitInfo().getExtraCost() + 1);
							}

							if (iValue < iBestValue)
							{
								iBestValue = iValue;
								pBestUnit = pLoopUnit;
							}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
						}
					}
				}
			}
		}
	}

	if (pBestUnit != NULL)
	{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      01/12/10                                jdog5000      */
/*                                                                                              */
/* AI logging                                                                                   */
/************************************************************************************************/
		if( gPlayerLogLevel >= 2 )
		{
			CvWString szString;
			getUnitAIString(szString, pBestUnit->AI_getUnitAIType());

			logBBAI("    Player %d (%S) disbanding %S with UNITAI %S to save cash", getID(), getCivilizationDescription(0), pBestUnit->getName().GetCString(), szString.GetCString());
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		pBestUnit->kill(false);
		return true;
	}
	return false;
}

int CvPlayerAI::AI_cultureVictoryTechValue(TechTypes eTech) const
{
	int iI;
	
	if (eTech == NO_TECH)
	{
		return 0;
	}
	
	int iValue = 0;
	
	if (GC.getTechInfo(eTech).isDefensivePactTrading())
	{
		iValue += 50;
	}
	
	if (GC.getTechInfo(eTech).isCommerceFlexible(COMMERCE_CULTURE))
	{
		iValue += 100;
	}
	
	//units
	bool bAnyWarplan = (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0);
	int iBestUnitValue = 0;
	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		UnitTypes eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

		if (eLoopUnit != NO_UNIT)
		{
			if (isTechRequiredForUnit((eTech), eLoopUnit))
			{
				int iTempValue = (GC.getUnitInfo(eLoopUnit).getCombat() * 100) / std::max(1, (GC.getGame().getBestLandUnitCombat()));
				iTempValue *= bAnyWarplan ? 2 : 1;
				
				iValue += iTempValue / 3;
				iBestUnitValue = std::max(iBestUnitValue, iTempValue);
			}
		}
	}
	iValue += std::max(0, iBestUnitValue - 15);
	
	//cultural things
	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		BuildingTypes eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)));

		if (eLoopBuilding != NO_BUILDING)
		{
			if (isTechRequiredForBuilding((eTech), eLoopBuilding))
			{
				CvBuildingInfo& kLoopBuilding = GC.getBuildingInfo(eLoopBuilding);

				if ((GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex()) != (GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI)))
				{
					//UB
					iValue += 100;
				}

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       05/25/10                          Fuyu & jdog5000     */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
						iValue += (150 * kLoopBuilding.getCommerceChange(COMMERCE_CULTURE)) * 20;
*/
						iValue += (150 * (kLoopBuilding.getCommerceChange(COMMERCE_CULTURE) + kLoopBuilding.getObsoleteSafeCommerceChange(COMMERCE_CULTURE))) / 20;
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
				iValue += kLoopBuilding.getCommerceModifier(COMMERCE_CULTURE) * 2;
			}
		}
	}
	
	//important civics
	for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
	{
		if (GC.getCivicInfo((CivicTypes)iI).getTechPrereq() == eTech)
		{
			iValue += GC.getCivicInfo((CivicTypes)iI).getCommerceModifier(COMMERCE_CULTURE) * 2;
		}
	}
	
	return iValue;
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/25/10                                jdog5000      */
/*                                                                                              */
/* Victory Strategy AI                                                                          */
/************************************************************************************************/
int CvPlayerAI::AI_getCultureVictoryStage() const
{
    int iValue;

	if( GC.getDefineINT("BBAI_VICTORY_STRATEGY_CULTURE") <= 0 )
	{
		return 0;
	}
	
    if (!GC.getGameINLINE().culturalVictoryValid())
    {
        return 0;
    }

	// Necessary as capital city pointer is used later
    if (getCapitalCity() == NULL)
    {
        return 0;
    }

	int iHighCultureCount = 0;
	int iCloseToLegendaryCount = 0;
	int iLegendaryCount = 0;
		
	int iLoop;
	CvCity* pLoopCity;
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (pLoopCity->getCultureLevel() >= (GC.getGameINLINE().culturalVictoryCultureLevel() - 1))
		{
			if (pLoopCity->getBaseCommerceRate(COMMERCE_CULTURE) > 100)
			{
				iHighCultureCount++;
			}

			// is over 1/2 of the way there?
			if( 2*pLoopCity->getCulture(getID()) > pLoopCity->getCultureThreshold(GC.getGameINLINE().culturalVictoryCultureLevel()) )
			{
				iCloseToLegendaryCount++;
			}

			// is already there?
			if( pLoopCity->getCulture(getID()) > pLoopCity->getCultureThreshold(GC.getGameINLINE().culturalVictoryCultureLevel()) )
			{
				iLegendaryCount++;
			}
		}
	}

	if( iLegendaryCount >= GC.getGameINLINE().culturalVictoryNumCultureCities() )
	{
		// Already won, keep playing culture heavy but do some tech to keep pace if human wants to keep playing
		return 3;
	}

	if( iCloseToLegendaryCount >= GC.getGameINLINE().culturalVictoryNumCultureCities() )
	{
		return 4;
	}
	else if( isHuman() )
	{
		if( getCommercePercent(COMMERCE_CULTURE) > 50 )
		{
			return 3;
		}
	}

	if( isHuman() && !(GC.getGameINLINE().isDebugMode()) )
	{
		return 0;
	}

	if (GC.getGame().getStartEra() > 1)
    {
    	return 0;
    }

	if (getCapitalCity()->getGameTurnFounded() > (10 + GC.getGameINLINE().getStartTurn()))
    {
		if( iHighCultureCount < GC.getGameINLINE().culturalVictoryNumCultureCities() )
		{
			//the loss of the capital indicates it might be a good idea to abort any culture victory
			return 0;
		}
    }

    iValue = GC.getLeaderHeadInfo(getPersonalityType()).getCultureVictoryWeight();

	if (GC.getGameINLINE().isOption(GAMEOPTION_ALWAYS_PEACE))
	{
    	iValue += 30;
	}
	
	iValue += (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? -20 : 0);
	
	if( iValue > 20 && getNumCities() >= GC.getGameINLINE().culturalVictoryNumCultureCities() )
	{
		iValue += 10*countHolyCities();
	}
	/*
	if ((GET_TEAM(getTeam()).isAVassal()) && (getNumCities() > 5))
	{
		int iReligionCount = countTotalHasReligion();
		if (((iReligionCount * 100) / getNumCities()) > 250)
		{
			iValue += 1;
			iValue += ((2 * iReligionCount) + 1) / getNumCities();
		}
	}
	*/

	int iNonsense = AI_getStrategyRand() + 10;
	iValue += (iNonsense % 100);

	if (iValue < 100)
	{
		return 0;
	}
    
    if (getCurrentEra() >= (GC.getNumEraInfos() - (2 + iNonsense % 2)))
    {
		bool bAt3 = false;
        
		// if we have enough high culture cities, go to stage 3
		if (iHighCultureCount >= GC.getGameINLINE().culturalVictoryNumCultureCities())
		{
			bAt3 = true;
		}

		// if we have a lot of religion, may be able to catch up quickly
		if (countTotalHasReligion() >= getNumCities() * 3)
        {
			if( getNumCities() >= GC.getGameINLINE().culturalVictoryNumCultureCities() )
			{
				bAt3 = true;
			}
        }

		if( bAt3 )
		{
			if (AI_cultureVictoryTechValue(getCurrentResearch()) < 100)
			{
				return 4;
			}

			return 3;
		}
    }

	if (getCurrentEra() >= ((GC.getNumEraInfos() / 3) + iNonsense % 2))
	{
	    return 2;
	}
        
	return 1;
}

int CvPlayerAI::AI_getSpaceVictoryStage() const
{
    int iValue;

	if( GC.getDefineINT("BBAI_VICTORY_STRATEGY_SPACE") <= 0 )
	{
		return 0;
	}

	if (getCapitalCity() == NULL)
    {
        return 0;
    }
	
	// Better way to determine if spaceship victory is valid?
	VictoryTypes eSpace = NO_VICTORY;
	for (int iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		if (GC.getGameINLINE().isVictoryValid((VictoryTypes) iI))
		{
			CvVictoryInfo& kVictoryInfo = GC.getVictoryInfo((VictoryTypes) iI);
			if( kVictoryInfo.getVictoryDelayTurns() > 0 )
			{
				eSpace = (VictoryTypes)iI;
				break;
			}
		}
	}

	if( eSpace == NO_VICTORY )
	{
		return 0;
	}

	// If have built Apollo, then the race is on!
	bool bHasApollo = false;
	bool bNearAllTechs = true;
	for( int iI = 0; iI < GC.getNumProjectInfos(); iI++ )
	{
		if( GC.getProjectInfo((ProjectTypes)iI).getVictoryPrereq() == eSpace )
		{
			if( GET_TEAM(getTeam()).getProjectCount((ProjectTypes)iI) > 0 )
			{
				bHasApollo = true;
			}
			else
			{
				if( !GET_TEAM(getTeam()).isHasTech((TechTypes)GC.getProjectInfo((ProjectTypes)iI).getTechPrereq()) )
				{
					if( !isResearchingTech((TechTypes)GC.getProjectInfo((ProjectTypes)iI).getTechPrereq()) )
					{
						bNearAllTechs = false;
					}
				}
			}
		}
	}

	if( bHasApollo )
	{
		if( bNearAllTechs )
		{
			bool bOtherLaunched = false;
			if( GET_TEAM(getTeam()).getVictoryCountdown(eSpace) >= 0 )
			{
				for( int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++ )
				{
					if( iTeam != getTeam() )
					{
						if( GET_TEAM((TeamTypes)iTeam).getVictoryCountdown(eSpace) >= 0 )
						{
							if( GET_TEAM((TeamTypes)iTeam).getVictoryCountdown(eSpace) < GET_TEAM(getTeam()).getVictoryCountdown(eSpace) )
							{
								bOtherLaunched = true;
								break;
							}

							if( GET_TEAM((TeamTypes)iTeam).getVictoryCountdown(eSpace) == GET_TEAM(getTeam()).getVictoryCountdown(eSpace) && (iTeam < getTeam()) )
							{
								bOtherLaunched = true;
								break;
							}
						}
					}
				}
			}
			else
			{
				for( int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++ )
				{
					if( GET_TEAM((TeamTypes)iTeam).getVictoryCountdown(eSpace) >= 0 )
					{
						bOtherLaunched = true;
						break;
					}
				}
			}

			if( !bOtherLaunched )
			{
				return 4;
			}

			return 3;
		}

		if( GET_TEAM(getTeam()).getBestKnownTechScorePercent() > (m_iVictoryStrategyHash & AI_VICTORY_SPACE3 ? 80 : 85) )
		{
			return 3;
		}
	}

	if( isHuman() && !(GC.getGameINLINE().isDebugMode()) )
	{
		return 0;
	}

	// If can't build Apollo yet, then consider making player push for this victory type
	{
		iValue = GC.getLeaderHeadInfo(getPersonalityType()).getSpaceVictoryWeight();

		if (GC.getGameINLINE().isOption(GAMEOPTION_ALWAYS_PEACE))
		{
    		iValue += 30;
		}
		
		iValue += (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? -20 : 0);

		int iNonsense = AI_getStrategyRand() + 50;
		iValue += (iNonsense % 100);

		if (iValue >= 100)
		{
			if( getCurrentEra() >= GC.getNumEraInfos() - 3 )
			{
				return 2;
			}

			return 1;
		}
	}

	return 0;
}

int CvPlayerAI::AI_getConquestVictoryStage() const
{
	int iValue;

	if (GC.getGameINLINE().isOption(GAMEOPTION_ALWAYS_PEACE))
	{
    	return 0;
	}

	if(GET_TEAM(getTeam()).isAVassal())
	{
		return 0;
	}

	if( GC.getDefineINT("BBAI_VICTORY_STRATEGY_CONQUEST") <= 0 )
	{
		return 0;
	}

	VictoryTypes eConquest = NO_VICTORY;
	for (int iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		if (GC.getGameINLINE().isVictoryValid((VictoryTypes) iI))
		{
			CvVictoryInfo& kVictoryInfo = GC.getVictoryInfo((VictoryTypes) iI);
			if( kVictoryInfo.isConquest() )
			{
				eConquest = (VictoryTypes)iI;
				break;
			}
		}
	}

	if( eConquest == NO_VICTORY )
	{
		return 0;
	}

	// Check for whether we are very powerful, looking good for conquest
	int iOurPower = GET_TEAM(getTeam()).getPower(true);
	int iOurPowerRank = 1;
	int iTotalPower = 0;
	int iNumNonVassals = 0;
	for (int iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		if( iI != getTeam() )
		{
			CvTeam& kTeam = GET_TEAM((TeamTypes) iI);
			if (kTeam.isAlive() && !(kTeam.isMinorCiv()))
			{
				if( !kTeam.isAVassal() )
				{
					iTotalPower += kTeam.getPower(true);
					iNumNonVassals++;

					if( GET_TEAM(getTeam()).isHasMet((TeamTypes) iI) )
					{
						if( 95*kTeam.getPower(false) > 100*iOurPower )
						{
							iOurPowerRank++;
						}
					}
				}
			}
		}
	}
	int iAverageOtherPower = iTotalPower / std::max(1, iNumNonVassals);

	if( 3*iOurPower > 4*iAverageOtherPower )
	{
		// BBAI TODO: Have we declared total war on anyone?  Need some aggressive action taken, maybe past war success
		int iOthersWarMemoryOfUs = 0;
		for( int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; iPlayer++ )
		{
			if( GET_PLAYER((PlayerTypes)iPlayer).getTeam() != getTeam() && GET_PLAYER((PlayerTypes)iPlayer).isEverAlive() )
			{
				iOthersWarMemoryOfUs += GET_PLAYER((PlayerTypes)iPlayer).AI_getMemoryCount(getID(), MEMORY_DECLARED_WAR);
			}
		}

		if( GET_TEAM(getTeam()).getHasMetCivCount(false) > GC.getGameINLINE().countCivPlayersAlive()/4 )
		{
			if( iOurPowerRank <= 1 + (GET_TEAM(getTeam()).getHasMetCivCount(true)/10) )
			{
				if( (iOurPower > 2*iAverageOtherPower) && (iOurPower - iAverageOtherPower > 100) )
				{
					if( iOthersWarMemoryOfUs > 0  )
					{
						return 4;
					}
				}
			}
		}

		if (getCurrentEra() >= ((GC.getNumEraInfos() / 3)))
		{
			if( iOurPowerRank <= 1 + (GET_TEAM(getTeam()).getHasMetCivCount(true)/7) )
			{
				if( iOthersWarMemoryOfUs > 2  )
				{
					return 3;
				}
			}
		}
	}

	if( isHuman() && !(GC.getGameINLINE().isDebugMode()) )
	{
		return 0;
	}

	// Check for whether we are inclined to pursue a conquest strategy
	{
		iValue = GC.getLeaderHeadInfo(getPersonalityType()).getConquestVictoryWeight();
		
		iValue += (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? 20 : 0);

		int iNonsense = AI_getStrategyRand() + 30;
		iValue += (iNonsense % 100);

		if (iValue >= 100)
		{
			if( m_iStrategyHash & AI_STRATEGY_GET_BETTER_UNITS )
			{
				if( (getNumCities() > 3) && (4*iOurPower > 5*iAverageOtherPower) )
				{
					return 2;
				}
			}

			return 1;
		}
	}

	return 0;
}

int CvPlayerAI::AI_getDominationVictoryStage() const
{
	int iValue = 0;

	if (GC.getGameINLINE().isOption(GAMEOPTION_ALWAYS_PEACE))
	{
    	return 0;
	}

	if(GET_TEAM(getTeam()).isAVassal())
	{
		return 0;
	}

	if( GC.getDefineINT("BBAI_VICTORY_STRATEGY_DOMINATION") <= 0 )
	{
		return 0;
	}

	VictoryTypes eDomination = NO_VICTORY;
	for (int iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		if (GC.getGameINLINE().isVictoryValid((VictoryTypes) iI))
		{
			CvVictoryInfo& kVictoryInfo = GC.getVictoryInfo((VictoryTypes) iI);
			if( kVictoryInfo.getLandPercent() > 0 && kVictoryInfo.getPopulationPercentLead() )
			{
				eDomination = (VictoryTypes)iI;
				break;
			}
		}
	}

	if( eDomination == NO_VICTORY )
	{
		return 0;
	}

	int iPercentOfDomination = 0;
	int iOurPopPercent = (100 * GET_TEAM(getTeam()).getTotalPopulation()) / std::max(1, GC.getGameINLINE().getTotalPopulation());
	int iOurLandPercent = (100 * GET_TEAM(getTeam()).getTotalLand()) / std::max(1, GC.getMapINLINE().getLandPlots());
	
	iPercentOfDomination = (100 * iOurPopPercent) / std::max(1, GC.getGameINLINE().getAdjustedPopulationPercent(eDomination));
	iPercentOfDomination = std::min( iPercentOfDomination, (100 * iOurLandPercent) / std::max(1, GC.getGameINLINE().getAdjustedLandPercent(eDomination)) );


	if( iPercentOfDomination > 80 )
	{
		return 4;
	}

	if( iPercentOfDomination > 50 )
	{
		return 3;
	}

	if( isHuman() && !(GC.getGameINLINE().isDebugMode()) )
	{
		return 0;
	}

	// Check for whether we are inclined to pursue a domination strategy
	{
		iValue = GC.getLeaderHeadInfo(getPersonalityType()).getDominationVictoryWeight();
		
		iValue += (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? 20 : 0);

		int iNonsense = AI_getStrategyRand() + 70;
		iValue += (iNonsense % 100);

		if (iValue >= 100)
		{
			if( getNumCities() > 3 && (GC.getGameINLINE().getPlayerRank(getID()) < (GC.getGameINLINE().countCivPlayersAlive() + 1)/2) )
			{
				return 2;
			}

			return 1;
		}
	}

	return 0;
}

int CvPlayerAI::AI_getDiplomacyVictoryStage() const
{
	int iValue = 0;

	if( GC.getDefineINT("BBAI_VICTORY_STRATEGY_DIPLOMACY") <= 0 )
	{
		return 0;
	}

	std::vector<VictoryTypes> veDiplomacy;
	for (int iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		if (GC.getGameINLINE().isVictoryValid((VictoryTypes) iI))
		{
			CvVictoryInfo& kVictoryInfo = GC.getVictoryInfo((VictoryTypes) iI);
			if( kVictoryInfo.isDiploVote() )
			{
				veDiplomacy.push_back((VictoryTypes)iI);
			}
		}
	}

	if( veDiplomacy.size() == 0 )
	{
		return 0;
	}

	// Check for whether we are elligible for election
	bool bVoteEligible = false;
	for( int iVoteSource = 0; iVoteSource < GC.getNumVoteSourceInfos(); iVoteSource++ )
	{
		if( GC.getGameINLINE().isDiploVote((VoteSourceTypes)iVoteSource) )
		{
			if( GC.getGameINLINE().isTeamVoteEligible(getTeam(),(VoteSourceTypes)iVoteSource) )
			{
				bVoteEligible = true;
				break;
			}
		}
	}

	bool bDiploInclined = false;

	// Check for whether we are inclined to pursue a diplomacy strategy
	{
		iValue = GC.getLeaderHeadInfo(getPersonalityType()).getDiplomacyVictoryWeight();

		if (GC.getGameINLINE().isOption(GAMEOPTION_ALWAYS_PEACE))
		{
    		iValue += 30;
		}
		
		iValue += (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? -20 : 0);

		int iNonsense = AI_getStrategyRand() + 90;
		iValue += (iNonsense % 100);

		// BBAI TODO: Level 2?

		if (iValue >= 100)
		{
			bDiploInclined = true;
		}
	}

	if( bVoteEligible && (bDiploInclined || isHuman()) )
	{
		// BBAI TODO: Level 4 - close to enough to win a vote?

		return 3;
	}

	if( isHuman() && !(GC.getGameINLINE().isDebugMode()) )
	{
		return 0;
	}

	if( bDiploInclined )
	{
		return 1;
	}

	return 0;
}

/// \brief Returns whether player is pursuing a particular victory strategy.
///
/// Victory strategies are computed on demand once per turn and stored for the rest
/// of the turn.  Each victory strategy type has 4 levels, the first two are
/// determined largely from AI tendencies and randomn dice rolls.  The second
/// two are based on measurables and past actions, so the AI can use them to
/// determine what other players (including the human player) are doing.
bool CvPlayerAI::AI_isDoVictoryStrategy(int iVictoryStrategy) const
{
	if( isBarbarian() || isMinorCiv() || !isAlive() )
	{
		return false;
	}
	
    return (iVictoryStrategy & AI_getVictoryStrategyHash());
}

bool CvPlayerAI::AI_isDoVictoryStrategyLevel4() const
{
	if( AI_isDoVictoryStrategy(AI_VICTORY_SPACE4) )
	{
		return true;
	}

	if( AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST4) )
	{
		return true;
	}

	if( AI_isDoVictoryStrategy(AI_VICTORY_CULTURE4) )
	{
		return true;
	}

	if( AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION4) )
	{
		return true;
	}

	if( AI_isDoVictoryStrategy(AI_VICTORY_DIPLOMACY4) )
	{
		return true;
	}

	return false;
}

bool CvPlayerAI::AI_isDoVictoryStrategyLevel3() const
{
	if( AI_isDoVictoryStrategy(AI_VICTORY_SPACE3) )
	{
		return true;
	}

	if( AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST3) )
	{
		return true;
	}

	if( AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3) )
	{
		return true;
	}

	if( AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION3) )
	{
		return true;
	}

	if( AI_isDoVictoryStrategy(AI_VICTORY_DIPLOMACY3) )
	{
		return true;
	}

	return false;
}

void CvPlayerAI::AI_forceUpdateVictoryStrategies()
{
	//this forces a recache.
	m_iVictoryStrategyHashCacheTurn = -1;
}

int CvPlayerAI::AI_getVictoryStrategyHash() const
{
	PROFILE_FUNC();

	if( isBarbarian() || isMinorCiv() || !isAlive() )
	{
		return 0;
	}

    if ((m_iVictoryStrategyHash != 0) && (m_iVictoryStrategyHashCacheTurn == GC.getGameINLINE().getGameTurn()))
    {
        return m_iVictoryStrategyHash;        
    }
 
	m_iVictoryStrategyHash = AI_DEFAULT_VICTORY_STRATEGY;
    m_iVictoryStrategyHashCacheTurn = GC.getGameINLINE().getGameTurn();
	
	if (getCapitalCity() == NULL)
    {
        return m_iVictoryStrategyHash;
    }

	bool bStartedOtherLevel3 = false;
	bool bStartedOtherLevel4 = false;

	// Space victory
	int iVictoryStage = AI_getSpaceVictoryStage();

	if( iVictoryStage >= 1 )
	{
		m_iVictoryStrategyHash |= AI_VICTORY_SPACE1;
	    if (iVictoryStage > 1)
	    {
	        m_iVictoryStrategyHash |= AI_VICTORY_SPACE2;
            if (iVictoryStage > 2)
            {
				bStartedOtherLevel3 = true;
                m_iVictoryStrategyHash |= AI_VICTORY_SPACE3;

				if (iVictoryStage > 3 && !bStartedOtherLevel4)
				{
					bStartedOtherLevel4 = true;
                	m_iVictoryStrategyHash |= AI_VICTORY_SPACE4;
                }
            }
	    }
	}

	// Conquest victory
	iVictoryStage = AI_getConquestVictoryStage();

	if( iVictoryStage >= 1 )
	{
		m_iVictoryStrategyHash |= AI_VICTORY_CONQUEST1;
	    if (iVictoryStage > 1)
	    {
	        m_iVictoryStrategyHash |= AI_VICTORY_CONQUEST2;
            if (iVictoryStage > 2)
            {
				bStartedOtherLevel3 = true;
                m_iVictoryStrategyHash |= AI_VICTORY_CONQUEST3;

				if (iVictoryStage > 3 && !bStartedOtherLevel4)
				{
					bStartedOtherLevel4 = true;
                	m_iVictoryStrategyHash |= AI_VICTORY_CONQUEST4;
                }
            }
	    }
	}

	// Domination victory
	iVictoryStage = AI_getDominationVictoryStage();

	if( iVictoryStage >= 1 )
	{
		m_iVictoryStrategyHash |= AI_VICTORY_DOMINATION1;
	    if (iVictoryStage > 1)
	    {
	        m_iVictoryStrategyHash |= AI_VICTORY_DOMINATION2;
            if (iVictoryStage > 2)
            {
				bStartedOtherLevel3 = true;
                m_iVictoryStrategyHash |= AI_VICTORY_DOMINATION3;

				if (iVictoryStage > 3 && !bStartedOtherLevel4)
				{
					bStartedOtherLevel4 = true;
                	m_iVictoryStrategyHash |= AI_VICTORY_DOMINATION4;
                }
            }
	    }
	}

	// Cultural victory
	iVictoryStage = AI_getCultureVictoryStage();

	if( iVictoryStage >= 1 )
	{
		m_iVictoryStrategyHash |= AI_VICTORY_CULTURE1;
	    if (iVictoryStage > 1)
	    {
	        m_iVictoryStrategyHash |= AI_VICTORY_CULTURE2;
            if (iVictoryStage > 2)
            {
				bStartedOtherLevel3 = true;
                m_iVictoryStrategyHash |= AI_VICTORY_CULTURE3;

				if (iVictoryStage > 3 && !bStartedOtherLevel4)
				{
					bStartedOtherLevel4 = true;
                	m_iVictoryStrategyHash |= AI_VICTORY_CULTURE4;
                }
            }
	    }
	}

	// Diplomacy victory
	iVictoryStage = AI_getDiplomacyVictoryStage();

	if( iVictoryStage >= 1 )
	{
		m_iVictoryStrategyHash |= AI_VICTORY_DIPLOMACY1;
	    if (iVictoryStage > 1)
	    {
	        m_iVictoryStrategyHash |= AI_VICTORY_DIPLOMACY2;
            if (iVictoryStage > 2 && !bStartedOtherLevel3)
            {
				bStartedOtherLevel3 = true;
                m_iVictoryStrategyHash |= AI_VICTORY_DIPLOMACY3;

				if (iVictoryStage > 3 && !bStartedOtherLevel4)
				{
					bStartedOtherLevel4 = true;
                	m_iVictoryStrategyHash |= AI_VICTORY_DIPLOMACY4;
                }
            }
	    }
	}

	return m_iVictoryStrategyHash;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/18/10                                jdog5000      */
/*                                                                                              */
/* War Strategy AI                                                                              */
/************************************************************************************************/
int CvPlayerAI::AI_getStrategyRand() const
{
    if( m_iStrategyRand <= 0 )
	{
		m_iStrategyRand = 1 + GC.getGameINLINE().getSorenRandNum(100000, "AI Strategy Rand");
	}

	return m_iStrategyRand;
}


bool CvPlayerAI::AI_isDoStrategy(int iStrategy) const
{
    if (isHuman())
    {
        return false;
    }

	if( isBarbarian() || isMinorCiv() || !isAlive() )
	{
		return false;
	}

    return (iStrategy & AI_getStrategyHash());
}

void CvPlayerAI::AI_forceUpdateStrategies()
{
	//this forces a recache.
	m_iStrategyHashCacheTurn = -1;
}

int CvPlayerAI::AI_getStrategyHash() const
{
    if ((m_iStrategyHash != 0) && (m_iStrategyHashCacheTurn == GC.getGameINLINE().getGameTurn()))
    {
        return m_iStrategyHash;        
    }
    
    const FlavorTypes AI_FLAVOR_MILITARY = (FlavorTypes)0;
	const FlavorTypes AI_FLAVOR_RELIGION = (FlavorTypes)1;
    const FlavorTypes AI_FLAVOR_PRODUCTION = (FlavorTypes)2;
    const FlavorTypes AI_FLAVOR_GOLD = (FlavorTypes)3;
    const FlavorTypes AI_FLAVOR_SCIENCE = (FlavorTypes)4;
    const FlavorTypes AI_FLAVOR_CULTURE = (FlavorTypes)5;
    const FlavorTypes AI_FLAVOR_GROWTH = (FlavorTypes)6;
    
    int iI, iJ, iK;
    UnitTypes eLoopUnit;

	int iLastStrategyHash = m_iStrategyHash;
    
    m_iStrategyHash = AI_DEFAULT_STRATEGY;
    m_iStrategyHashCacheTurn = GC.getGameINLINE().getGameTurn();
    
	if (AI_getFlavorValue(AI_FLAVOR_PRODUCTION) >= 2) // 0, 2, 5 or 10 in default xml [augustus 5, frederick 10, huayna 2, jc 2, chinese leader 2, qin 5, ramsess 2, roosevelt 5, stalin 2]
	{
		m_iStrategyHash |= AI_STRATEGY_PRODUCTION;
	}
	
	if (getCapitalCity() == NULL)
    {
        return m_iStrategyHash;
    }
    
    int iNonsense = AI_getStrategyRand();
    
	int iMetCount = GET_TEAM(getTeam()).getHasMetCivCount(true);
    
    //Unit Analysis
    int iBestSlowUnitCombat = -1;
    int iBestFastUnitCombat = -1;
    
    bool bHasMobileArtillery = false;
    bool bHasMobileAntiair = false;
    bool bHasBomber = false;
    
    int iNukeCount = 0;
    
    int iAttackUnitCount = 0;
	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));
		
		if (NO_UNIT != eLoopUnit)
		{
			if (getCapitalCity() != NULL)
			{
				if (getCapitalCity()->canTrain(eLoopUnit))
				{
					CvUnitInfo& kLoopUnit = GC.getUnitInfo(eLoopUnit);
					bool bIsUU = (GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex()) != (GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI));
					if (kLoopUnit.getUnitAIType(UNITAI_RESERVE) || kLoopUnit.getUnitAIType(UNITAI_ATTACK_CITY)
						|| kLoopUnit.getUnitAIType(UNITAI_COUNTER) || kLoopUnit.getUnitAIType(UNITAI_PILLAGE))
					{

						iAttackUnitCount++;

						//UU love
						if (bIsUU)
						{
							if (kLoopUnit.getUnitAIType(UNITAI_ATTACK_CITY) || 
								(kLoopUnit.getUnitAIType(UNITAI_ATTACK)	&& !kLoopUnit.getUnitAIType(UNITAI_CITY_DEFENSE)))
							{
								iAttackUnitCount++;					
							}
						}
						int iCombat = kLoopUnit.getCombat();
						int iMoves = kLoopUnit.getMoves();
						if (iMoves == 1)
						{
							iBestSlowUnitCombat = std::max(iBestSlowUnitCombat, iCombat);
						}
						else if (iMoves > 1)
						{
							iBestFastUnitCombat = std::max(iBestFastUnitCombat, iCombat);
							if (bIsUU)
							{
								iBestFastUnitCombat += 1;
							}
						}
					}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       09/10/08                                jdog5000      */
/*                                                                                              */
/* Bugfix				                                                                         */
/************************************************************************************************/
/* original BTS code
					if (kLoopUnit.getMoves() > 1)
*/
					// Mobile anti-air and artillery flags only meant for land units
					if ( kLoopUnit.getDomainType() == DOMAIN_LAND && kLoopUnit.getMoves() > 1 )
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
					{
						if (kLoopUnit.getInterceptionProbability() > 25)
						{
							bHasMobileAntiair = true;
						}
						if (kLoopUnit.getBombardRate() > 10)
						{
							bHasMobileArtillery = true;
						}
					}

					if (kLoopUnit.getAirRange() > 1)
					{
						if (!kLoopUnit.isSuicide())
						{
							if ((kLoopUnit.getBombRate() > 10) && (kLoopUnit.getAirCombat() > 0))
							{
								bHasBomber = true;								
							}
						}
					}
					
					if (kLoopUnit.getNukeRange() > 0)
					{
						iNukeCount++;
					}
				}
			}
		}
	}
	
/************************************************************************************************/
/* REVOLUTION_MOD                         06/03/08                                jdog5000      */
/*                                                                                              */
/* Revolution AI and for minor civs                                                             */
/************************************************************************************************/
/*
	if (iAttackUnitCount <= 1)
*/
	// This strategy is less appropriate for minor civs or rebels since they are at war
	if (iAttackUnitCount <= ((isMinorCiv() || isRebel()) ? 0 : 1))
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
	{
		m_iStrategyHash |= AI_STRATEGY_GET_BETTER_UNITS;
	}
	if (iBestFastUnitCombat > iBestSlowUnitCombat)
	{
		m_iStrategyHash |= AI_STRATEGY_FASTMOVERS;		
		if (bHasMobileArtillery && bHasMobileAntiair)
		{
			m_iStrategyHash |= AI_STRATEGY_LAND_BLITZ;
		}
	}
	if (iNukeCount > 0)
	{
		if ((GC.getLeaderHeadInfo(getPersonalityType()).getBuildUnitProb() + iNonsense % 15) >= (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) ? 37 : 43))
		{
			m_iStrategyHash |= AI_STRATEGY_OWABWNW;
		}
	}
	if (bHasBomber)
	{
		if (!(m_iStrategyHash & AI_STRATEGY_LAND_BLITZ))
		{
			m_iStrategyHash |= AI_STRATEGY_AIR_BLITZ;
		}
		else
		{
			if ((iNonsense % 2) == 0)
			{
				m_iStrategyHash |= AI_STRATEGY_AIR_BLITZ;
				m_iStrategyHash &= ~AI_STRATEGY_LAND_BLITZ;				
			}
		}
	}

	if( gPlayerLogLevel >= 2 )
	{
		if( (m_iStrategyHash & AI_STRATEGY_LAND_BLITZ) && !(iLastStrategyHash & AI_STRATEGY_LAND_BLITZ) )
		{
			logBBAI( "    Player %d (%S) starts strategy AI_STRATEGY_LAND_BLITZ on turn %d", getID(), getCivilizationDescription(0), GC.getGameINLINE().getGameTurn());
		}

		if( (m_iStrategyHash & AI_STRATEGY_AIR_BLITZ) && !(iLastStrategyHash & AI_STRATEGY_AIR_BLITZ) )
		{
			logBBAI( "    Player %d (%S) starts strategy AI_STRATEGY_AIR_BLITZ on turn %d", getID(), getCivilizationDescription(0), GC.getGameINLINE().getGameTurn());
		}
	}
    
	//missionary
	{
	    if (getStateReligion() != NO_RELIGION)
	    {
            int iHolyCityCount = countHolyCities();
            if ((iHolyCityCount > 0) && hasHolyCity(getStateReligion()))
            {
                int iMissionary = 0;
                //Missionary
                iMissionary += AI_getFlavorValue(AI_FLAVOR_GROWTH) * 2; // up to 10
                iMissionary += AI_getFlavorValue(AI_FLAVOR_CULTURE) * 4; // up to 40
                iMissionary += AI_getFlavorValue(AI_FLAVOR_RELIGION) * 6; // up to 60
                
                CivicTypes eCivic = (CivicTypes)GC.getLeaderHeadInfo(getPersonalityType()).getFavoriteCivic();
                if ((eCivic != NO_CIVIC) && (GC.getCivicInfo(eCivic).isNoNonStateReligionSpread()))
                {
                	iMissionary += 20;
                }
                
                iMissionary += (iHolyCityCount - 1) * 5;
                
				iMissionary += std::min(iMetCount, 5) * 7;

                for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
                {
					if (iI != getID())
                    {
						if (GET_PLAYER((PlayerTypes)iI).isAlive() && GET_TEAM(getTeam()).isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))
						{
                            if (GET_TEAM(getTeam()).isOpenBorders(GET_PLAYER((PlayerTypes)iI).getTeam()))
                            {
								if ((GET_PLAYER((PlayerTypes)iI).getStateReligion() == getStateReligion()))
								{
									iMissionary += 10;
								}
								else if( !GET_PLAYER((PlayerTypes)iI).isNoNonStateReligionSpread() )
								{
									iMissionary += (GET_PLAYER((PlayerTypes)iI).countHolyCities() == 0) ? 12 : 4;
								}
							}
                        }
                    }
                }
/************************************************************************************************/
/* Afforess	                  Start		 07/26/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
				bool bHasHolyBuilding = false;
				if (getStateReligion() != NO_RELIGION)
				{
					if (GC.getGameINLINE().getHolyCity(getStateReligion()) != NULL && GC.getGameINLINE().getHolyCity(getStateReligion())->getOwner() == getID())
					{
						for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
						{
							if (GC.getBuildingInfo((BuildingTypes)iI).getGlobalReligionCommerce() == getStateReligion())
							{
								if (GC.getGameINLINE().getHolyCity(getStateReligion())->getNumActiveBuilding((BuildingTypes)iI) > 0)
								{
									bHasHolyBuilding = true;
									break;
								}
							}
						}
					}
				}
				if (!bHasHolyBuilding)
				{
					iMissionary -= 10;
				}
				else
				{
					iMissionary += 10;
				}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
              //  iMissionary += (iNonsense % 7) * 3;
                
                if (iMissionary > 100)
                {
                    m_iStrategyHash |= AI_STRATEGY_MISSIONARY;
                }
            }
	    }
	}

	// Espionage
	if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
	{
		int iTempValue = 0;
		if (getCommercePercent(COMMERCE_ESPIONAGE) == 0)
		{
			iTempValue += 4;
		}

		if (GET_TEAM(getTeam()).getAnyWarPlanCount(true) == 0)
		{
			if( GET_TEAM(getTeam()).getBestKnownTechScorePercent() < 85 )
			{
				iTempValue += 5;
			}
			else
			{
				iTempValue += 3;
			}
		}
		
		iTempValue += (100 - AI_getEspionageWeight()) / 10;
		
		iTempValue += iNonsense % 12;
	
		if (iTempValue > 10)
		{
			m_iStrategyHash |= AI_STRATEGY_BIG_ESPIONAGE;	
		}
	}
    
	// Turtle strategy
	if( GET_TEAM(getTeam()).getAtWarCount(true) > 0 && getNumCities() > 0 )
	{
		int iMaxWarCounter = 0;
		for( int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++ )
		{
			if( iTeam != getTeam() )
			{
				if( GET_TEAM((TeamTypes)iTeam).isAlive() && !GET_TEAM((TeamTypes)iTeam).isMinorCiv() )
				{
					iMaxWarCounter = std::max( iMaxWarCounter, GET_TEAM(getTeam()).AI_getAtWarCounter((TeamTypes)iTeam) );
				}
			}
		}

		// Are we losing badly or recently attacked?
		if( GET_TEAM(getTeam()).AI_getWarSuccessCapitulationRatio() < -50 || iMaxWarCounter < 10 )
		{
			if( GET_TEAM(getTeam()).AI_getEnemyPowerPercent(true) > std::max(150, GC.getDefineINT("BBAI_TURTLE_ENEMY_POWER_RATIO")) )
			{
				m_iStrategyHash |= AI_STRATEGY_TURTLE;
			}
		}
	}

	if( gPlayerLogLevel >= 2 )
	{
		if( (m_iStrategyHash & AI_STRATEGY_TURTLE) && !(iLastStrategyHash & AI_STRATEGY_TURTLE) )
		{
			logBBAI( "    Player %d (%S) starts strategy AI_STRATEGY_TURTLE on turn %d", getID(), getCivilizationDescription(0), GC.getGameINLINE().getGameTurn());
		}

		if( !(m_iStrategyHash & AI_STRATEGY_TURTLE) && (iLastStrategyHash & AI_STRATEGY_TURTLE) )
		{
			logBBAI( "    Player %d (%S) stops strategy AI_STRATEGY_TURTLE on turn %d", getID(), getCivilizationDescription(0), GC.getGameINLINE().getGameTurn());
		}
	}
	
	int iCurrentEra = getCurrentEra();
	int iParanoia = 0;
	int iCloseTargets = 0;
	int iOurDefensivePower = GET_TEAM(getTeam()).getDefensivePower();

    for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
    {
		if( GET_PLAYER((PlayerTypes)iI).isAlive() && !GET_PLAYER((PlayerTypes)iI).isMinorCiv() && !GET_PLAYER((PlayerTypes)iI).isBarbarian() )
		{
    		if ((GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam()) && GET_TEAM(getTeam()).isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()) )
			{
				if (!GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isAVassal() && !GET_TEAM(getTeam()).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam()))
    			{
					if( GET_TEAM(getTeam()).AI_getWarPlan(GET_PLAYER((PlayerTypes)iI).getTeam()) != NO_WARPLAN )
					{
						iCloseTargets++;
					}
					else if( !GET_TEAM(getTeam()).isVassal(GET_PLAYER((PlayerTypes)iI).getTeam()) )
					{
						// Are they a threat?
						int iTempParanoia = 0;

						int iTheirPower = GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).getPower(true);
						if( 4*iTheirPower > 3*iOurDefensivePower )
						{
							if( GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).getAtWarCount(true) == 0 || GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).AI_getEnemyPowerPercent(false) < 140 )
							{
								// Memory of them declaring on us and our friends
								int iWarMemory = AI_getMemoryCount((PlayerTypes)iI, MEMORY_DECLARED_WAR);
								iWarMemory += (AI_getMemoryCount((PlayerTypes)iI, MEMORY_DECLARED_WAR_ON_FRIEND) + 1)/2;

								if (iWarMemory > 0)
								{
									//they are a snake
									iTempParanoia += 50 + 50 * iWarMemory;

									if( gPlayerLogLevel >= 2 )
									{
										logBBAI( "    Player %d (%S) wary of %S because of war memory %d", getID(), getCivilizationDescription(0), GET_PLAYER((PlayerTypes)iI).getCivilizationDescription(0), iWarMemory);
									}
								}
							}
						}

						// Do we think our relations are bad?
						int iCloseness = AI_playerCloseness((PlayerTypes)iI, DEFAULT_PLAYER_CLOSENESS);
						if (iCloseness > 0)
						{
							int iAttitudeWarProb = 100 - GC.getLeaderHeadInfo(getPersonalityType()).getNoWarAttitudeProb(AI_getAttitude((PlayerTypes)iI));
							if( iAttitudeWarProb > 10 )
							{
								if( 4*iTheirPower > 3*iOurDefensivePower )
								{
									iTempParanoia += iAttitudeWarProb/2;
								}

								iCloseTargets++;
							}

							if( iTheirPower > 2*iOurDefensivePower )
							{
								if( AI_getAttitude((PlayerTypes)iI) != ATTITUDE_FRIENDLY )
								{
									iTempParanoia += 25;
								}
							}
						}

						if( iTempParanoia > 0 )
						{
							iTempParanoia *= iTheirPower;
							iTempParanoia /= std::max(1, iOurDefensivePower);
						}

						// Do they look like they're going for militaristic victory?
						if( GET_PLAYER((PlayerTypes)iI).AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST4) )
						{
							iTempParanoia += 200;
						}
						else if( GET_PLAYER((PlayerTypes)iI).AI_isDoVictoryStrategy(AI_VICTORY_CONQUEST3) )
						{
							iTempParanoia += 100;
						}
						else if( GET_PLAYER((PlayerTypes)iI).AI_isDoVictoryStrategy(AI_VICTORY_DOMINATION3) )
						{
							iTempParanoia += 50;
						}

						if( iTempParanoia > 0 )
						{
							if( iCloseness == 0 )
							{
								iTempParanoia /= 2;
							}

							iParanoia += iTempParanoia;
						}
					}
				}
			}
    	}
    }

	if( m_iStrategyHash & AI_STRATEGY_GET_BETTER_UNITS )
	{
		iParanoia *= 3;
		iParanoia /= 2;
	}

	// Scale paranoia in later eras/larger games
	iParanoia -= (100*(iCurrentEra + 1)) / std::max(1, GC.getNumEraInfos());

	// Alert strategy
	if( iParanoia >= 200 )
	{
		m_iStrategyHash |= AI_STRATEGY_ALERT1;
		if( iParanoia >= 400 )
		{
			m_iStrategyHash |= AI_STRATEGY_ALERT2;
		}
	}

	if( gPlayerLogLevel >= 2 )
	{
		if( (m_iStrategyHash & AI_STRATEGY_ALERT1) && !(iLastStrategyHash & AI_STRATEGY_ALERT1) )
		{
			logBBAI( "    Player %d (%S) starts strategy AI_STRATEGY_ALERT1 on turn %d with iParanoia %d", getID(), getCivilizationDescription(0), GC.getGameINLINE().getGameTurn(), iParanoia);
		}

		if( !(m_iStrategyHash & AI_STRATEGY_ALERT1) && (iLastStrategyHash & AI_STRATEGY_ALERT1) )
		{
			logBBAI( "    Player %d (%S) stops strategy AI_STRATEGY_ALERT1 on turn %d with iParanoia %d", getID(), getCivilizationDescription(0), GC.getGameINLINE().getGameTurn(), iParanoia);
		}

		if( (m_iStrategyHash & AI_STRATEGY_ALERT2) && !(iLastStrategyHash & AI_STRATEGY_ALERT2) )
		{
			logBBAI( "    Player %d (%S) starts strategy AI_STRATEGY_ALERT2 on turn %d with iParanoia %d", getID(), getCivilizationDescription(0), GC.getGameINLINE().getGameTurn(), iParanoia);
		}

		if( !(m_iStrategyHash & AI_STRATEGY_ALERT2) && (iLastStrategyHash & AI_STRATEGY_ALERT2) )
		{
			logBBAI( "    Player %d (%S) stops strategy AI_STRATEGY_ALERT2 on turn %d with iParanoia %d", getID(), getCivilizationDescription(0), GC.getGameINLINE().getGameTurn(), iParanoia);
		}
	}
	
	// BBAI TODO: Integrate Dagger with new conquest victory strategy, have Dagger focus on early rushes
    //dagger
	if( !(AI_isDoVictoryStrategy(AI_VICTORY_CULTURE2)) 
	 && !(m_iStrategyHash & AI_STRATEGY_MISSIONARY)
     && (iCurrentEra <= (2+(iNonsense%2))) && (iCloseTargets > 0) )
    {	    
	    int iDagger = 0;
	    iDagger += 12000 / std::max(100, (50 + GC.getLeaderHeadInfo(getPersonalityType()).getMaxWarRand()));
	    iDagger *= (iNonsense % 11);
	    iDagger /= 10;
	    iDagger += 5 * std::min(8, AI_getFlavorValue(AI_FLAVOR_MILITARY));
	    
        for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
        {
            eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

            if ((eLoopUnit != NO_UNIT) && (GC.getUnitInfo(eLoopUnit).getCombat() > 0))
            {
                if ((GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex()) != (GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)))
                {
                	bool bIsDefensive = (GC.getUnitInfo(eLoopUnit).getUnitAIType(UNITAI_CITY_DEFENSE) &&
                		!GC.getUnitInfo(eLoopUnit).getUnitAIType(UNITAI_RESERVE));
					
					iDagger += bIsDefensive ? -10 : 0;
                       
                    if (getCapitalCity()->canTrain(eLoopUnit))
                    {
                        iDagger += bIsDefensive ? 10 : 40;
                        
                        int iUUStr = GC.getUnitInfo(eLoopUnit).getCombat();
                        int iNormalStr = GC.getUnitInfo((UnitTypes)(GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex())).getCombat();
                        iDagger += 20 * range((iUUStr - iNormalStr), 0, 2);
						if (GC.getUnitInfo(eLoopUnit).getPrereqAndTech() == NO_TECH)
                        {
                            iDagger += 20;
                        }
                    }
                    else
                    {
                        if (GC.getUnitInfo(eLoopUnit).getPrereqAndTech() != NO_TECH)
                        {
                            if (GC.getTechInfo((TechTypes)(GC.getUnitInfo(eLoopUnit).getPrereqAndTech())).getEra() <= (iCurrentEra + 1))
                            {
                                if (GET_TEAM(getTeam()).isHasTech((TechTypes)GC.getUnitInfo(eLoopUnit).getPrereqAndTech()))
                                {
                                	//we have the tech but can't train the unit, dejection.
                                    iDagger += 10;
                                }
                                else
                                {
                                	//we don't have the tech, it's understandable we can't train.
                                    iDagger += 30;
                                }
                            }
                        }
                                
                        bool bNeedsAndBonus = false;
                        int iOrBonusCount = 0;
                        int iOrBonusHave = 0;
                        
                        for (iJ = 0; iJ < GC.getNumBonusInfos(); iJ++)
                        {
                            BonusTypes eBonus = (BonusTypes)iJ;
                            if (eBonus != NO_BONUS)
                            {
                                if (GC.getUnitInfo(eLoopUnit).getPrereqAndBonus() == eBonus)
                                {
                                    if (getNumTradeableBonuses(eBonus) == 0)
                                    {
                                        bNeedsAndBonus = true;
                                    }
                                }

                                for (iK = 0; iK < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); iK++)
                                {
                                    if (GC.getUnitInfo(eLoopUnit).getPrereqOrBonuses(iK) == eBonus)
                                    {
                                        iOrBonusCount++;
                                        if (getNumTradeableBonuses(eBonus) > 0)
                                        {
                                            iOrBonusHave++;
                                        }
                                    }
                                }
                            }
                        }
                        
                        
                        iDagger += 20;
                        if (bNeedsAndBonus)
                        {
                            iDagger -= 20;
                        }
                        if ((iOrBonusCount > 0) && (iOrBonusHave == 0))
                        {
                            iDagger -= 20;
                        }
                    }
                }
            }
        }
        
        if (!GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
        {
			iDagger += range(100 - GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAITrainPercent(), 0, 15);
        }
        
        if ((getCapitalCity()->area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (getCapitalCity()->area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE))
        {
            iDagger += (iAttackUnitCount > 0) ? 40 : 20;
        }
/************************************************************************************************/
/* REVOLUTION_MOD                         05/22/08                                jdog5000      */
/*                                                                                              */
/* Revolution AI                                                                                */
/************************************************************************************************/
		if( isRebel() )
		{
			iDagger += 30;
		}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
        
        if (iDagger >= AI_DAGGER_THRESHOLD)
        {
            m_iStrategyHash |= AI_STRATEGY_DAGGER;            
        }
		else
		{
			if( iLastStrategyHash &= AI_STRATEGY_DAGGER )
			{
				if (iDagger >= (9*AI_DAGGER_THRESHOLD)/10)
				{
					m_iStrategyHash |= AI_STRATEGY_DAGGER;            
				}
			}
		}

		if( gPlayerLogLevel >= 2 )
		{
			if( (m_iStrategyHash & AI_STRATEGY_DAGGER) && !(iLastStrategyHash & AI_STRATEGY_DAGGER) )
			{
				logBBAI( "    Player %d (%S) starts strategy AI_STRATEGY_DAGGER on turn %d with iDagger %d", getID(), getCivilizationDescription(0), GC.getGameINLINE().getGameTurn(), iDagger);
			}

			if( !(m_iStrategyHash & AI_STRATEGY_DAGGER) && (iLastStrategyHash & AI_STRATEGY_DAGGER) )
			{
				logBBAI( "    Player %d (%S) stops strategy AI_STRATEGY_DAGGER on turn %d with iDagger %d", getID(), getCivilizationDescription(0), GC.getGameINLINE().getGameTurn(), iDagger);
			}
		}
	}
	
	if( m_iStrategyHash & !AI_STRATEGY_ALERT2 )
	{//Crush
		int iWarCount = 0;
		int iCrushValue = 0;
		
		iCrushValue += (iNonsense % 4);
		
		if (m_iStrategyHash & AI_STRATEGY_DAGGER)
		{
			iCrushValue += 3;			
		}
		if (GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
		{
			iCrushValue += 3;
		}

		for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
		{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       02/14/10                        denev & jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
			if ((GET_TEAM((TeamTypes)iI).isAlive()) && (iI != getID()))
*/
			if ((GET_TEAM((TeamTypes)iI).isAlive()) && (iI != getTeam()))
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
			{
				if (GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iI) != NO_WARPLAN)
				{
					if (!GET_TEAM((TeamTypes)iI).isAVassal())
					{
						if (GET_TEAM(getTeam()).AI_teamCloseness((TeamTypes)iI) > 0)
						{
							iWarCount++;
						}
					}
					
					if (GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iI) == WARPLAN_PREPARING_TOTAL)
					{
						iCrushValue += 6;					
					}
					else if ((GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iI) == WARPLAN_TOTAL) && (GET_TEAM(getTeam()).AI_getWarPlanStateCounter((TeamTypes)iI) < 20))
					{
						iCrushValue += 6;						
					}
					
					if ((GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iI) == WARPLAN_DOGPILE) && (GET_TEAM(getTeam()).AI_getWarPlanStateCounter((TeamTypes)iI) < 20))
					{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       02/14/10                             jdog5000         */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
						for (iJ = 0; iJ < MAX_TEAMS; iJ++)
						{
							if ((iJ != iI) && iJ != getID())
*/
						for (iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
						{
							if ((iJ != iI) && iJ != getTeam() && GET_TEAM((TeamTypes)iJ).isAlive())
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
							{
								if ((atWar((TeamTypes)iI, (TeamTypes)iJ)) && !GET_TEAM((TeamTypes)iI).isAVassal())
								{
									iCrushValue += 4;
								}
							}
						}
					}
/************************************************************************************************/
/* REVOLUTION_MOD                         05/18/08                                jdog5000      */
/*                                                                                              */
/* Revolution AI                                                                                */
/************************************************************************************************/
					if( GET_TEAM((TeamTypes)iI).isRebelAgainst(getTeam()) )
					{
						iCrushValue += 4;
					}
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
				}
			}
		}
		if ((iWarCount <= 1) && (iCrushValue >= ((iLastStrategyHash & AI_STRATEGY_CRUSH) ? 9 :10)))
		{
			m_iStrategyHash |= AI_STRATEGY_CRUSH;
		}

		if( gPlayerLogLevel >= 2 )
		{
			if( (m_iStrategyHash & AI_STRATEGY_CRUSH) && !(iLastStrategyHash & AI_STRATEGY_CRUSH) )
			{
				logBBAI( "    Player %d (%S) starts strategy AI_STRATEGY_CRUSH on turn %d with iCrushValue %d", getID(), getCivilizationDescription(0), GC.getGameINLINE().getGameTurn(), iCrushValue);
			}

			if( !(m_iStrategyHash & AI_STRATEGY_CRUSH) && (iLastStrategyHash & AI_STRATEGY_CRUSH) )
			{
				logBBAI( "    Player %d (%S) stops strategy AI_STRATEGY_CRUSH on turn %d with iCrushValue %d", getID(), getCivilizationDescription(0), GC.getGameINLINE().getGameTurn(), iCrushValue);
			}
		}
	}
	
	{
		CvTeamAI& kTeam = GET_TEAM(getTeam());
		int iOurVictoryCountdown = kTeam.AI_getLowestVictoryCountdown();

		int iTheirVictoryCountdown = MAX_INT;
		
		for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
		{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       02/14/10                             jdog5000         */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
			if ((GET_TEAM((TeamTypes)iI).isAlive()) && (iI != getID()))
*/
			if ((GET_TEAM((TeamTypes)iI).isAlive()) && (iI != getTeam()))
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
			{
				CvTeamAI& kOtherTeam = GET_TEAM((TeamTypes)iI);
				iTheirVictoryCountdown = std::min(iTheirVictoryCountdown, kOtherTeam.AI_getLowestVictoryCountdown());
			}
		}
		
		if (MAX_INT == iTheirVictoryCountdown)
		{
			iTheirVictoryCountdown = -1;
		}

		if ((iOurVictoryCountdown >= 0) && (iTheirVictoryCountdown < 0 || iOurVictoryCountdown <= iTheirVictoryCountdown))
		{
			m_iStrategyHash |= AI_STRATEGY_LAST_STAND;
		}
		else if ((iTheirVictoryCountdown >= 0))
		{
			if((iTheirVictoryCountdown < iOurVictoryCountdown))
			{
				m_iStrategyHash |= AI_STRATEGY_FINAL_WAR;			
			}
			else if( GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) )
			{
				m_iStrategyHash |= AI_STRATEGY_FINAL_WAR;
			}
			else if( AI_isDoVictoryStrategyLevel4() || AI_isDoVictoryStrategy(AI_VICTORY_SPACE3) )
			{
				m_iStrategyHash |= AI_STRATEGY_FINAL_WAR;
			}
		}
		
		if (iOurVictoryCountdown < 0)
		{
			if (isCurrentResearchRepeat())
			{
				int iStronger = 0;
				int iAlive = 1;
				for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++)
				{
					if (iTeam != getTeam())
					{
						CvTeamAI& kLoopTeam = GET_TEAM((TeamTypes)iTeam);
						if (kLoopTeam.isAlive())
						{
							iAlive++;
							if (kTeam.getPower(true) < kLoopTeam.getPower(true))
							{
								iStronger++;
							}
						}
					}
				}
				
				if ((iStronger <= 1) || (iStronger <= iAlive / 4))
				{
					m_iStrategyHash |= AI_STRATEGY_FINAL_WAR;
				}
			}
		}
		
	}

	if (isCurrentResearchRepeat())
	{
		int iTotalVictories = 0;
		int iAchieveVictories = 0;
		int iWarVictories = 0;
		
		
		int iThreshold = std::max(1, (GC.getGame().countCivTeamsAlive() + 1) / 4);
		
		CvTeamAI& kTeam = GET_TEAM(getTeam());
		for (int iVictory = 0; iVictory < GC.getNumVictoryInfos(); iVictory++)
		{
			CvVictoryInfo& kVictory = GC.getVictoryInfo((VictoryTypes)iVictory);
			if (GC.getGame().isVictoryValid((VictoryTypes)iVictory))
			{
				iTotalVictories ++;
				if (kVictory.isDiploVote())
				{
					//
				}
				else if (kVictory.isEndScore())
				{
					int iHigherCount = 0;
					int IWeakerCount = 0;
					for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++)
					{
						if (iTeam != getTeam())
						{
							CvTeamAI& kLoopTeam = GET_TEAM((TeamTypes)iTeam);
							if (kLoopTeam.isAlive())
							{
								if (GC.getGame().getTeamScore(getTeam()) < ((GC.getGame().getTeamScore((TeamTypes)iTeam) * 90) / 100))
								{
									iHigherCount++;
									if (kTeam.getPower(true) > kLoopTeam.getPower(true))
									{
										IWeakerCount++;
									}
								}
							}
						}
					}
						
					if (iHigherCount > 0)
					{
						if (IWeakerCount == iHigherCount)
						{
							iWarVictories++;
						}
					}
				}
				else if (kVictory.getCityCulture() > 0)
				{
					if (m_iStrategyHash & AI_VICTORY_CULTURE1)
					{
						iAchieveVictories++;
					}
				}
				else if (kVictory.getMinLandPercent() > 0 || kVictory.getLandPercent() > 0)
				{
					int iLargerCount = 0;
					for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++)
					{
						if (iTeam != getTeam())
						{
							CvTeamAI& kLoopTeam = GET_TEAM((TeamTypes)iTeam);
							if (kLoopTeam.isAlive())
							{
								if (kTeam.getTotalLand(true) < kLoopTeam.getTotalLand(true))
								{
									iLargerCount++;
								}
							}
						}
					}
					if (iLargerCount <= iThreshold)
					{
						iWarVictories++;
					}
				}
				else if (kVictory.isConquest())
				{
					int iStrongerCount = 0;
					for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; iTeam++)
					{
						if (iTeam != getTeam())
						{
							CvTeamAI& kLoopTeam = GET_TEAM((TeamTypes)iTeam);
							if (kLoopTeam.isAlive())
							{
								if (kTeam.getPower(true) < kLoopTeam.getPower(true))
								{
									iStrongerCount++;
								}
							}
						}
					}
					if (iStrongerCount <= iThreshold)
					{
						iWarVictories++;
					}
				}
				else
				{
					if (kTeam.getVictoryCountdown((VictoryTypes)iVictory) > 0)
					{
						iAchieveVictories++;
					}
				}
			}
		}
		
		if (iAchieveVictories == 0)
		{
			if (iWarVictories > 0)
			{
				m_iStrategyHash |= AI_STRATEGY_FINAL_WAR;
			}
		}
	}
	
	
	//Turn off inappropriate strategies.
	if (GC.getGameINLINE().isOption(GAMEOPTION_ALWAYS_PEACE))
	{
		m_iStrategyHash &= ~AI_STRATEGY_DAGGER;
		m_iStrategyHash &= ~AI_STRATEGY_CRUSH;
		m_iStrategyHash &= ~AI_STRATEGY_ALERT1;
		m_iStrategyHash &= ~AI_STRATEGY_ALERT2;
		m_iStrategyHash &= ~AI_STRATEGY_TURTLE;
		m_iStrategyHash &= ~AI_STRATEGY_FINAL_WAR;
		m_iStrategyHash &= ~AI_STRATEGY_LAST_STAND;

		m_iStrategyHash &= ~AI_STRATEGY_OWABWNW;
		m_iStrategyHash &= ~AI_STRATEGY_FASTMOVERS;
	}

	return m_iStrategyHash;   
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/


void CvPlayerAI::AI_nowHasTech(TechTypes eTech)
{
	// while its _possible_ to do checks, for financial trouble, and this tech adds financial buildings
	// if in war and this tech adds important war units
	// etc
	// it makes more sense to just redetermine what to produce
	// that is already done every time a civ meets a new civ, it makes sense to do it when a new tech is learned
	// if this is changed, then at a minimum, AI_isFinancialTrouble should be checked
	if (!isHuman())
	{
		int iGameTurn = GC.getGameINLINE().getGameTurn();
		
		// only reset at most every 10 turns
		if (iGameTurn > m_iTurnLastProductionDirty + 10)
		{
			// redeterimine the best things to build in each city
			AI_makeProductionDirty();
		
			m_iTurnLastProductionDirty = iGameTurn;
		}
	}

}


int CvPlayerAI::AI_countDeadlockedBonuses(CvPlot* pPlot) const
{
    CvPlot* pLoopPlot;
    CvPlot* pLoopPlot2;
    int iDX, iDY;
    int iI;
    
    int iMinRange = GC.getMIN_CITY_RANGE();
    int iRange = iMinRange * 2;
    int iCount = 0;

    for (iDX = -(iRange); iDX <= iRange; iDX++)
    {
        for (iDY = -(iRange); iDY <= iRange; iDY++)
        {
            if (plotDistance(iDX, iDY, 0, 0) > CITY_PLOTS_RADIUS)
            {
                pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

                if (pLoopPlot != NULL)
                {
                    if (pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
                    {
                        if (!pLoopPlot->isCityRadius() && ((pLoopPlot->area() == pPlot->area()) || pLoopPlot->isWater()))
                        {
                            bool bCanFound = false;
                            bool bNeverFound = true;
                            //potentially blockable resource
                            //look for a city site within a city radius
                            for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
                            {
                                pLoopPlot2 = plotCity(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), iI);
                                if (pLoopPlot2 != NULL)
                                {
                                    //canFound usually returns very quickly
                                    if (canFound(pLoopPlot2->getX_INLINE(), pLoopPlot2->getY_INLINE(), false))
                                    {
                                        bNeverFound = false;
                                        if (stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pLoopPlot2->getX_INLINE(), pLoopPlot2->getY_INLINE()) > iMinRange)
                                        {
                                            bCanFound = true;
                                            break;
                                        }
                                    }
                                }
                            }
                            if (!bNeverFound && !bCanFound)
                            {
                                iCount++;
                            }
                        }
                    }
                }
            }
        }
    }
    
    return iCount;
}

int CvPlayerAI::AI_getOurPlotStrength(CvPlot* pPlot, int iRange, bool bDefensiveBonuses, bool bTestMoves) const
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iValue;
	int iDistance;
	int iDX, iDY;

	iValue = 0;

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pPlot->area())
				{
				    iDistance = stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
					pUnitNode = pLoopPlot->headUnitNode();

					while (pUnitNode != NULL)
					{
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

						if (pLoopUnit->getOwnerINLINE() == getID())
						{
							if ((bDefensiveBonuses && pLoopUnit->canDefend()) || pLoopUnit->canAttack())
							{
								if (!(pLoopUnit->isInvisible(getTeam(), false)))
								{
								    if (pLoopUnit->atPlot(pPlot) || pLoopUnit->canMoveInto(pPlot) || pLoopUnit->canMoveInto(pPlot, /*bAttack*/ true))
								    {
                                        if (!bTestMoves)
                                        {
                                        	iValue += pLoopUnit->currEffectiveStr((bDefensiveBonuses ? pPlot : NULL), NULL);
                                        }
                                        else
                                        {
											if (pLoopUnit->baseMoves() >= iDistance)
                                            {
                                                iValue += pLoopUnit->currEffectiveStr((bDefensiveBonuses ? pPlot : NULL), NULL);
                                            }
                                        }
								    }
								}
							}
						}
					}
				}
			}
		}
	}


	return iValue;
}

int CvPlayerAI::AI_getEnemyPlotStrength(CvPlot* pPlot, int iRange, bool bDefensiveBonuses, bool bTestMoves) const
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iValue;
	int iDistance;
	int iDX, iDY;

	iValue = 0;

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pPlot->area())
				{
				    iDistance = stepDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
					pUnitNode = pLoopPlot->headUnitNode();

					while (pUnitNode != NULL)
					{
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

						if (atWar(pLoopUnit->getTeam(), getTeam()))
						{
							if ((bDefensiveBonuses && pLoopUnit->canDefend()) || pLoopUnit->canAttack())
							{
								if (!(pLoopUnit->isInvisible(getTeam(), false)))
								{
								    if (pPlot->isValidDomainForAction(*pLoopUnit))
								    {
                                        if (!bTestMoves)
                                        {
                                            iValue += pLoopUnit->currEffectiveStr((bDefensiveBonuses ? pPlot : NULL), NULL);
                                        }
                                        else
                                        {
                                            int iDangerRange = pLoopUnit->baseMoves();
                                            iDangerRange += ((pLoopPlot->isValidRoute(pLoopUnit)) ? 1 : 0);
                                            if (iDangerRange >= iDistance)
                                            {
                                                iValue += pLoopUnit->currEffectiveStr((bDefensiveBonuses ? pPlot : NULL), NULL);
                                            }
                                        }
								    }
								}
							}
						}
					}
				}
			}
		}
	}


	return iValue;
	
}

int CvPlayerAI::AI_goldToUpgradeAllUnits(int iExpThreshold) const
{
	if (m_iUpgradeUnitsCacheTurn == GC.getGameINLINE().getGameTurn() && m_iUpgradeUnitsCachedExpThreshold == iExpThreshold)
	{
		return m_iUpgradeUnitsCachedGold;
	}

	int iTotalGold = 0;
	
	CvCivilizationInfo& kCivilizationInfo = GC.getCivilizationInfo(getCivilizationType());
	
	// cache the value for each unit type
	std::vector<int> aiUnitUpgradePrice(GC.getNumUnitInfos(), 0);	// initializes to zeros

	int iLoop;
	for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		// if experience is below threshold, skip this unit
		if (pLoopUnit->getExperience() < iExpThreshold)
		{
			continue;
		}
		
		UnitTypes eUnitType = pLoopUnit->getUnitType();

		// check cached value for this unit type
		int iCachedUnitGold = aiUnitUpgradePrice[eUnitType];
		if (iCachedUnitGold != 0)
		{
			// if positive, add it to the sum
			if (iCachedUnitGold > 0)
			{
				iTotalGold += iCachedUnitGold;
			}
			
			// either way, done with this unit
			continue;
		}
		
		int iUnitGold = 0;
		int iUnitUpgradePossibilities = 0;
		
		UnitAITypes eUnitAIType = pLoopUnit->AI_getUnitAIType();
		CvArea* pUnitArea = pLoopUnit->area();
		int iUnitValue = AI_unitValue(eUnitType, eUnitAIType, pUnitArea);

		for (int iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			UnitClassTypes eUpgradeUnitClassType = (UnitClassTypes) iI;
			UnitTypes eUpgradeUnitType = (UnitTypes)(kCivilizationInfo.getCivilizationUnits(iI));

			if (NO_UNIT != eUpgradeUnitType)
			{
				// is it better?
				int iUpgradeValue = AI_unitValue(eUpgradeUnitType, eUnitAIType, pUnitArea);
				if (iUpgradeValue > iUnitValue)
				{
					// is this a valid upgrade?
					if (pLoopUnit->upgradeAvailable(eUnitType, eUpgradeUnitClassType))
					{
						// can we actually make this upgrade?
						bool bCanUpgrade = false;
						CvCity* pCapitalCity = getCapitalCity();
						if (pCapitalCity != NULL && pCapitalCity->canTrain(eUpgradeUnitType))
						{
							bCanUpgrade = true;
						}
						else
						{
							CvCity* pCloseCity = GC.getMapINLINE().findCity(pLoopUnit->getX_INLINE(), pLoopUnit->getY_INLINE(), getID(), NO_TEAM, true, (pLoopUnit->getDomainType() == DOMAIN_SEA));
							if (pCloseCity != NULL && pCloseCity->canTrain(eUpgradeUnitType))
							{
								bCanUpgrade = true;
							}
						}

						if (bCanUpgrade)
						{
							iUnitGold += pLoopUnit->upgradePrice(eUpgradeUnitType);
							iUnitUpgradePossibilities++; 
						}
					}
				}
			}
		}
		
		// if we found any, find average and add to total
		if (iUnitUpgradePossibilities > 0)
		{
			iUnitGold /= iUnitUpgradePossibilities;

			// add to cache
			aiUnitUpgradePrice[eUnitType] = iUnitGold;

			// add to sum
			iTotalGold += iUnitGold;
		}
		else
		{
			// add to cache, dont upgrade to this type
			aiUnitUpgradePrice[eUnitType] = -1;
		}
	}

	m_iUpgradeUnitsCacheTurn = GC.getGameINLINE().getGameTurn();
	m_iUpgradeUnitsCachedExpThreshold = iExpThreshold;
	m_iUpgradeUnitsCachedGold = iTotalGold;

	return iTotalGold;
}

int CvPlayerAI::AI_goldTradeValuePercent() const
{
	int iValue = 2;
	if (AI_isFinancialTrouble())
	{
		iValue += 1;
	}
	return 100 * iValue;
	
}

int CvPlayerAI::AI_averageYieldMultiplier(YieldTypes eYield) const
{
	FAssert(eYield > -1);
	FAssert(eYield < NUM_YIELD_TYPES);
	
	if (m_iAveragesCacheTurn != GC.getGameINLINE().getGameTurn())
	{
		AI_calculateAverages();
	}
	
	FAssert(m_aiAverageYieldMultiplier[eYield] > 0);
	return m_aiAverageYieldMultiplier[eYield];
}

int CvPlayerAI::AI_averageCommerceMultiplier(CommerceTypes eCommerce) const
{
	FAssert(eCommerce > -1);
	FAssert(eCommerce < NUM_COMMERCE_TYPES);
	
	if (m_iAveragesCacheTurn != GC.getGameINLINE().getGameTurn())
	{
		AI_calculateAverages();
	}
	
	return m_aiAverageCommerceMultiplier[eCommerce];	
}

int CvPlayerAI::AI_averageGreatPeopleMultiplier() const
{
	if (m_iAveragesCacheTurn != GC.getGameINLINE().getGameTurn())
	{
		AI_calculateAverages();
	}
	return m_iAverageGreatPeopleMultiplier;	
}

//"100 eCommerce is worth (return) raw YIELD_COMMERCE
int CvPlayerAI::AI_averageCommerceExchange(CommerceTypes eCommerce) const
{
	FAssert(eCommerce > -1);
	FAssert(eCommerce < NUM_COMMERCE_TYPES);
	
	if (m_iAveragesCacheTurn != GC.getGameINLINE().getGameTurn())
	{
		AI_calculateAverages();
	}
	
	return m_aiAverageCommerceExchange[eCommerce];
}

void CvPlayerAI::AI_calculateAverages() const
{
	CvCity* pLoopCity;
	int iLoop;
	int iI;
	
	int iPopulation;
	int iTotalPopulation;

	for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		m_aiAverageYieldMultiplier[iI] = 0;		
	}
	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		m_aiAverageCommerceMultiplier[iI] = 0;	
	}
	m_iAverageGreatPeopleMultiplier = 0;
	
	iTotalPopulation = 0;
	
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		iPopulation = std::max(pLoopCity->getPopulation(), NUM_CITY_PLOTS);
		iTotalPopulation += iPopulation;
			
		for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			m_aiAverageYieldMultiplier[iI] += iPopulation * pLoopCity->AI_yieldMultiplier((YieldTypes)iI);
		}
		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			m_aiAverageCommerceMultiplier[iI] += iPopulation * pLoopCity->getTotalCommerceRateModifier((CommerceTypes)iI);
		}
		m_iAverageGreatPeopleMultiplier += iPopulation * pLoopCity->getTotalGreatPeopleRateModifier();
	}
	
	
	if (iTotalPopulation > 0)
	{
		for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			m_aiAverageYieldMultiplier[iI] /= iTotalPopulation;
			FAssert(m_aiAverageYieldMultiplier[iI] > 0);
/************************************************************************************************/
/* Afforess	                  Start		 06/04/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			m_aiAverageYieldMultiplier[iI] = std::max(1, m_aiAverageYieldMultiplier[iI]);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		}
		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			m_aiAverageCommerceMultiplier[iI] /= iTotalPopulation;	
			FAssert(m_aiAverageCommerceMultiplier[iI] > 0);	
/************************************************************************************************/
/* Afforess	                  Start		 06/04/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			m_aiAverageCommerceMultiplier[iI] = std::max(1, m_aiAverageCommerceMultiplier[iI]);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		}
		m_iAverageGreatPeopleMultiplier /= iTotalPopulation;
		FAssert(m_iAverageGreatPeopleMultiplier > 0);
/************************************************************************************************/
/* Afforess	                  Start		 06/04/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		m_iAverageGreatPeopleMultiplier = std::max(1, m_iAverageGreatPeopleMultiplier);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	}
	else
	{
		for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
		{
			m_aiAverageYieldMultiplier[iI] = 100;
		}
		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			m_aiAverageCommerceMultiplier[iI] = 100;
		}
		m_iAverageGreatPeopleMultiplier = 100;
	}
	
	
	//Calculate Exchange Rate
	
	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		m_aiAverageCommerceExchange[iI] = 0;		
	}
	
	int iCommerce = 0;
	int iTotalCommerce = 0;
	
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		iCommerce = pLoopCity->getYieldRate(YIELD_COMMERCE);
		iTotalCommerce += iCommerce;
		
		int iExtraCommerce = 0;
		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			iExtraCommerce +=((pLoopCity->getSpecialistPopulation() + pLoopCity->getNumGreatPeople()) * getSpecialistExtraCommerce((CommerceTypes)iI));
			iExtraCommerce += (pLoopCity->getBuildingCommerce((CommerceTypes)iI) + pLoopCity->getSpecialistCommerce((CommerceTypes)iI) + pLoopCity->getReligionCommerce((CommerceTypes)iI) + getFreeCityCommerce((CommerceTypes)iI));
		}
		iTotalCommerce += iExtraCommerce;
		
		for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
		{
			m_aiAverageCommerceExchange[iI] += ((iCommerce + iExtraCommerce) * pLoopCity->getTotalCommerceRateModifier((CommerceTypes)iI)) / 100;		
		}
	}

	for (iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		if (m_aiAverageCommerceExchange[iI] > 0)
		{
			m_aiAverageCommerceExchange[iI] = (100 * iTotalCommerce) / m_aiAverageCommerceExchange[iI];
		}
		else
		{
			m_aiAverageCommerceExchange[iI] = 100;
		}
	}

	m_iAveragesCacheTurn = GC.getGameINLINE().getGameTurn();
}

void CvPlayerAI::AI_convertUnitAITypesForCrush()
{
	CvUnit* pLoopUnit;
	
	int iLoop;
	
	for(pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		bool bValid = false;
		if ((pLoopUnit->AI_getUnitAIType() == UNITAI_RESERVE)
			|| (pLoopUnit->AI_isCityAIType() && (pLoopUnit->getExtraCityDefensePercent() <= 0)))
		{
			bValid = true;
		}
		if ((pLoopUnit->area()->getAreaAIType(getTeam()) == AREAAI_ASSAULT)
			|| (pLoopUnit->area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE))
		{
			bValid = false;
		}
		
		if (!pLoopUnit->canAttack() || (pLoopUnit->AI_getUnitAIType() == UNITAI_CITY_SPECIAL))
		{
			bValid = false;
		}
	
		if (bValid)
		{
			if (pLoopUnit->plot()->isCity())
			{
				if (pLoopUnit->plot()->getPlotCity()->getOwner() == getID())
				{
					if (pLoopUnit->plot()->getBestDefender(getID()) == pLoopUnit)
					{
						bValid = false;
					}
				}
			}
		}
		
		if (bValid)
		{
			pLoopUnit->AI_setUnitAIType(UNITAI_ATTACK_CITY);			
		}
	}
}

int CvPlayerAI::AI_playerCloseness(PlayerTypes eIndex, int iMaxDistance) const
{
	PROFILE_FUNC();	
	FAssert(GET_PLAYER(eIndex).isAlive());
	FAssert(eIndex != getID());
	
	int iValue = 0;
	int iLoop;
	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		iValue += pLoopCity->AI_playerCloseness(eIndex, iMaxDistance);
	}
	
	return iValue;
}


int CvPlayerAI::AI_getTotalAreaCityThreat(CvArea* pArea) const
{
	PROFILE_FUNC();
	CvCity* pLoopCity;
	int iLoop;
	int iValue;
	
	iValue = 0;
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (pLoopCity->getArea() == pArea->getID())
		{
			iValue += pLoopCity->AI_cityThreat();
		}
	}
	return iValue;
}

int CvPlayerAI::AI_countNumAreaHostileUnits(CvArea* pArea, bool bPlayer, bool bTeam, bool bNeutral, bool bHostile) const
{
	PROFILE_FUNC();
	CvPlot* pLoopPlot;
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		if ((pLoopPlot->area() == pArea) && pLoopPlot->isVisible(getTeam(), false) && 
			((bPlayer && pLoopPlot->getOwnerINLINE() == getID()) || (bTeam && pLoopPlot->getTeam() == getTeam()) 
				|| (bNeutral && !pLoopPlot->isOwned()) || (bHostile && pLoopPlot->isOwned() && GET_TEAM(getTeam()).isAtWar(pLoopPlot->getTeam()))))
			{
			iCount += pLoopPlot->plotCount(PUF_isEnemy, getID(), false, NO_PLAYER, NO_TEAM, PUF_isVisible, getID());
		}
	}
	return iCount;
}

//this doesn't include the minimal one or two garrison units in each city.
int CvPlayerAI::AI_getTotalFloatingDefendersNeeded(CvArea* pArea) const
{
	PROFILE_FUNC();
	int iDefenders;
	int iCurrentEra = getCurrentEra();
	int iAreaCities = pArea->getCitiesPerPlayer(getID());
	
	iCurrentEra = std::max(0, iCurrentEra - GC.getGame().getStartEra() / 2);
	
	iDefenders = 1 + ((iCurrentEra + ((GC.getGameINLINE().getMaxCityElimination() > 0) ? 3 : 2)) * iAreaCities);
	iDefenders /= 3;
	iDefenders += pArea->getPopulationPerPlayer(getID()) / 7;

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/01/10                                jdog5000      */
/*                                                                                              */
/* War strategy AI, Victory Strategy AI                                                         */
/************************************************************************************************/
	if( pArea->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE || pArea->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE || pArea->getAreaAIType(getTeam()) == AREAAI_MASSING )
	{
		if( iAreaCities <= std::min(4, pArea->getNumCities()/3) )
		{
			// Land war here, as floating defenders are based on cities/population need to make sure
			// AI defends its footholds in new continents well.
			iDefenders += GET_TEAM(getTeam()).countEnemyPopulationByArea(pArea) / 14;
		}
	}

	if (pArea->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE)
	{
		iDefenders *= 2;
	}
	else 
	{
		if( AI_isDoStrategy(AI_STRATEGY_ALERT2) )
		{
			iDefenders *= 2;
		}
		else if( AI_isDoStrategy(AI_STRATEGY_ALERT1) )
		{
			iDefenders *= 3;
			iDefenders /= 2;
		}
		else if (pArea->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE)
		{
			iDefenders *= 2;
			iDefenders /= 3;
		}
		else if (pArea->getAreaAIType(getTeam()) == AREAAI_MASSING)
		{
			if( GET_TEAM(getTeam()).AI_getEnemyPowerPercent(true) < (10 + GC.getLeaderHeadInfo(getPersonalityType()).getMaxWarNearbyPowerRatio()) )
			{
				iDefenders *= 2;
				iDefenders /= 3;
			}
		}
	}
	
	if (AI_getTotalAreaCityThreat(pArea) == 0)
	{
		iDefenders /= 2;
	}
	
	if (!GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
	{
		iDefenders *= 2;
		iDefenders /= 3;
	}

	// Removed AI_STRATEGY_GET_BETTER_UNITS reduction, it was reducing defenses twice
	
	if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE3))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	{
		iDefenders += 2 * iAreaCities;
		if (pArea->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE)
		{
			iDefenders *= 2; //go crazy
		}
	}
	
	iDefenders *= 60;
	iDefenders /= std::max(30, (GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAITrainPercent() - 20));
	
	if ((iCurrentEra < 3) && (GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS)))
	{
		iDefenders += 2;
	}
	
	if (getCapitalCity() != NULL)
	{
		if (getCapitalCity()->area() != pArea)
		{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       01/23/09                                jdog5000      */
/*                                                                                              */
/* Bugfix, War tactics AI                                                                       */
/************************************************************************************************/
/* original BTS code
			//Defend offshore islands only lightly.
			iDefenders = std::min(iDefenders, iAreaCities * iAreaCities - 1);
*/
			// Lessen defensive requirements only if not being attacked locally
			if( pArea->getAreaAIType(getTeam()) != AREAAI_DEFENSIVE )
			{
				// This may be our first city captured on a large enemy continent, need defenses to scale up based
				// on total number of area cities not just ours
				iDefenders = std::min(iDefenders, iAreaCities * iAreaCities + pArea->getNumCities() - iAreaCities - 1);
			}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
		}
	}
	
	return iDefenders;
}

int CvPlayerAI::AI_getTotalFloatingDefenders(CvArea* pArea) const
{
	PROFILE_FUNC();
	int iCount = 0;

	iCount += AI_totalAreaUnitAIs(pArea, UNITAI_COLLATERAL);
	iCount += AI_totalAreaUnitAIs(pArea, UNITAI_RESERVE);
	iCount += std::max(0, (AI_totalAreaUnitAIs(pArea, UNITAI_CITY_DEFENSE) - (pArea->getCitiesPerPlayer(getID()) * 2)));
	iCount += AI_totalAreaUnitAIs(pArea, UNITAI_CITY_COUNTER);
	iCount += AI_totalAreaUnitAIs(pArea, UNITAI_CITY_SPECIAL);
	// BBAI TODO: Defense air?  Is this outdated?
	iCount += AI_totalAreaUnitAIs(pArea, UNITAI_DEFENSE_AIR);
	return iCount;
}

RouteTypes CvPlayerAI::AI_bestAdvancedStartRoute(CvPlot* pPlot, int* piYieldValue) const
{
	RouteTypes eBestRoute = NO_ROUTE;
	int iBestValue = -1;
    for (int iI = 0; iI < GC.getNumRouteInfos(); iI++)
    {
        RouteTypes eRoute = (RouteTypes)iI;

		int iValue = 0;
		int iCost = getAdvancedStartRouteCost(eRoute, true, pPlot);
		
		if (iCost >= 0)
		{
			iValue += GC.getRouteInfo(eRoute).getValue();
									
			if (iValue > 0)
			{
				int iYieldValue = 0;
				if (pPlot->getImprovementType() != NO_IMPROVEMENT)
				{
					iYieldValue += ((GC.getImprovementInfo(pPlot->getImprovementType()).getRouteYieldChanges(eRoute, YIELD_FOOD)) * 100);
					iYieldValue += ((GC.getImprovementInfo(pPlot->getImprovementType()).getRouteYieldChanges(eRoute, YIELD_PRODUCTION)) * 60);
					iYieldValue += ((GC.getImprovementInfo(pPlot->getImprovementType()).getRouteYieldChanges(eRoute, YIELD_COMMERCE)) * 40);
				}
				iValue *= 1000;
				iValue /= (1 + iCost);

				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					eBestRoute = eRoute;
					if (piYieldValue != NULL)
					{
						*piYieldValue = iYieldValue;
					}
				}
			}
		}
	}
	return eBestRoute;
}

UnitTypes CvPlayerAI::AI_bestAdvancedStartUnitAI(CvPlot* pPlot, UnitAITypes eUnitAI) const
{
	UnitTypes eLoopUnit;
	UnitTypes eBestUnit;
	int iValue;
	int iBestValue;
	int iI, iJ, iK;
	
	FAssertMsg(eUnitAI != NO_UNITAI, "UnitAI is not assigned a valid value");

	iBestValue = 0;
	eBestUnit = NO_UNIT;

	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

		if (eLoopUnit != NO_UNIT)
		{
			//if (!isHuman() || (GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType() == eUnitAI))
			{
				int iUnitCost = getAdvancedStartUnitCost(eLoopUnit, true, pPlot);
				if (iUnitCost >= 0)
				{
					iValue = AI_unitValue(eLoopUnit, eUnitAI, pPlot->area());

					if (iValue > 0)
					{
						//free promotions. slow?
						//only 1 promotion per source is counted (ie protective isn't counted twice)
						int iPromotionValue = 0;

						//special to the unit
						for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
						{
							if (GC.getUnitInfo(eLoopUnit).getFreePromotions(iJ))
							{
								iPromotionValue += 15;
								break;
							}
						}

						for (iK = 0; iK < GC.getNumPromotionInfos(); iK++)
						{
							if (isFreePromotion((UnitCombatTypes)GC.getUnitInfo(eLoopUnit).getUnitCombatType(), (PromotionTypes)iK))
							{
								iPromotionValue += 15;
								break;
							}

							if (isFreePromotion((UnitClassTypes)GC.getUnitInfo(eLoopUnit).getUnitClassType(), (PromotionTypes)iK))
							{
								iPromotionValue += 15;
								break;
							}
						}

						//traits
						for (iJ = 0; iJ < GC.getNumTraitInfos(); iJ++)
						{
							if (hasTrait((TraitTypes)iJ))
							{
								for (iK = 0; iK < GC.getNumPromotionInfos(); iK++)
								{
									if (GC.getTraitInfo((TraitTypes) iJ).isFreePromotion(iK))
									{
										if ((GC.getUnitInfo(eLoopUnit).getUnitCombatType() != NO_UNITCOMBAT) && GC.getTraitInfo((TraitTypes) iJ).isFreePromotionUnitCombat(GC.getUnitInfo(eLoopUnit).getUnitCombatType()))
										{
											iPromotionValue += 15;
											break;
										}
									}
								}
							}
						}

						iValue *= (iPromotionValue + 100);
						iValue /= 100;

						iValue *= (GC.getGameINLINE().getSorenRandNum(40, "AI Best Advanced Start Unit") + 100);
						iValue /= 100;

						iValue *= (getNumCities() + 2);
						iValue /= (getUnitClassCountPlusMaking((UnitClassTypes)iI) + getNumCities() + 2);

						FAssert((MAX_INT / 1000) > iValue);
						iValue *= 1000;
						
						iValue /= 1 + iUnitCost;

						iValue = std::max(1, iValue);

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							eBestUnit = eLoopUnit;
						}
					}
				}
			}
		}
	}

	return eBestUnit;
}

CvPlot* CvPlayerAI::AI_advancedStartFindCapitalPlot() const
{
	CvPlot* pBestPlot = NULL;
	int iBestValue = -1;
	
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (kPlayer.isAlive())
		{
			if (kPlayer.getTeam() == getTeam())
			{
				CvPlot* pLoopPlot = kPlayer.getStartingPlot();
				if (pLoopPlot != NULL)
				{
					if (getAdvancedStartCityCost(true, pLoopPlot) > 0)
					{
					int iX = pLoopPlot->getX_INLINE();
					int iY = pLoopPlot->getY_INLINE();
						
						int iValue = 1000;
						if (iPlayer == getID())
						{
							iValue += 1000;
						}
						else
						{
							iValue += GC.getGame().getSorenRandNum(100, "AI Advanced Start Choose Team Start");
						}
						CvCity * pNearestCity = GC.getMapINLINE().findCity(iX, iY, NO_PLAYER, getTeam());
						if (NULL != pNearestCity)
						{
							FAssert(pNearestCity->getTeam() == getTeam());
							int iDistance = stepDistance(iX, iY, pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE());
							if (iDistance < 10)
							{
								iValue /= (10 - iDistance);
							}
						}
						
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pLoopPlot;							
						}
					}
				}
				else
				{
					FAssertMsg(false, "StartingPlot for a live player is NULL!");
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		return pBestPlot;
	}
	
	FAssertMsg(false, "AS: Failed to find a starting plot for a player");
	
	//Execution should almost never reach here.
	
	//Update found values just in case - particulary important for simultaneous turns.
	AI_updateFoundValues();
	
	pBestPlot = NULL;
	iBestValue = -1;
	
	if (NULL != getStartingPlot())
	{
		for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
		{
			CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
			if (pLoopPlot->getArea() == getStartingPlot()->getArea())
			{
				int iValue = pLoopPlot->getFoundValue(getID());
				if (iValue > 0)
				{
					if (getAdvancedStartCityCost(true, pLoopPlot) > 0)
					{
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pLoopPlot;
						}
					}
				}
			}
		}
	}
	
	if (pBestPlot != NULL)
	{
		return pBestPlot;
	}
	
	//Commence panic.
	FAssertMsg(false, "Failed to find an advanced start starting plot");
	return NULL;
}


bool CvPlayerAI::AI_advancedStartPlaceExploreUnits(bool bLand)
{
	CvPlot* pBestExplorePlot = NULL;
	int iBestExploreValue = 0;
	UnitTypes eBestUnitType = NO_UNIT;
	
	UnitAITypes eUnitAI = NO_UNITAI;
	if (bLand)
	{
		eUnitAI = UNITAI_EXPLORE;
	}
	else
	{
		eUnitAI = UNITAI_EXPLORE_SEA;
	}
	
	int iLoop;
	CvCity* pLoopCity;
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		CvPlot* pLoopPlot = pLoopCity->plot();
		CvArea* pLoopArea = bLand ? pLoopCity->area() : pLoopPlot->waterArea();
		
		if (pLoopArea != NULL)
			{
			int iValue = std::max(0, pLoopArea->getNumUnrevealedTiles(getTeam()) - 10) * 10;
			iValue += std::max(0, pLoopArea->getNumTiles() - 50);
				
				if (iValue > 0)
				{
					int iOtherPlotCount = 0;
					int iGoodyCount = 0;
					int iExplorerCount = 0;
				int iAreaId = pLoopArea->getID();
					
				int iRange = 4;
					for (int iX = -iRange; iX <= iRange; iX++)
					{
						for (int iY = -iRange; iY <= iRange; iY++)
						{
							CvPlot* pLoopPlot2 = plotXY(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), iX, iY);
						if (NULL != pLoopPlot2) 
							{
								iExplorerCount += pLoopPlot2->plotCount(PUF_isUnitAIType, eUnitAI, -1, NO_PLAYER, getTeam());
							if (pLoopPlot2->getArea() == iAreaId)
							{
								if (pLoopPlot2->isGoody())
								{
									iGoodyCount++;
								}
								if (pLoopPlot2->getTeam() != getTeam())
								{
									iOtherPlotCount++;
								}
							}
						}
					}
				}
					
					iValue -= 300 * iExplorerCount;
					iValue += 200 * iGoodyCount;
					iValue += 10 * iOtherPlotCount;
					if (iValue > iBestExploreValue)
					{
						UnitTypes eUnit = AI_bestAdvancedStartUnitAI(pLoopPlot, eUnitAI);
						if (eUnit != NO_UNIT)
						{
							eBestUnitType = eUnit;
							iBestExploreValue = iValue;
							pBestExplorePlot = pLoopPlot;
						}
					}
				}
			}
		}
	
	if (pBestExplorePlot != NULL)
	{
		doAdvancedStartAction(ADVANCEDSTARTACTION_UNIT, pBestExplorePlot->getX_INLINE(), pBestExplorePlot->getY_INLINE(), eBestUnitType, true);
		return true;
	}
	return false;	
}

void CvPlayerAI::AI_advancedStartRevealRadius(CvPlot* pPlot, int iRadius)
{
	for (int iRange = 1; iRange <=iRadius; iRange++)
	{
		for (int iX = -iRange; iX <= iRange; iX++)
		{
			for (int iY = -iRange; iY <= iRange; iY++)
			{
				if (plotDistance(0, 0, iX, iY) <= iRadius)
				{
					CvPlot* pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iX, iY);

					if (NULL != pLoopPlot)
					{
						if (getAdvancedStartVisibilityCost(true, pLoopPlot) > 0)
						{
							doAdvancedStartAction(ADVANCEDSTARTACTION_VISIBILITY, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), -1, true);
						}
					}
				}
			}
		}
	}
}

bool CvPlayerAI::AI_advancedStartPlaceCity(CvPlot* pPlot)
{
	//If there is already a city, then improve it.
	CvCity* pCity = pPlot->getPlotCity();
	if (pCity == NULL)
	{
		doAdvancedStartAction(ADVANCEDSTARTACTION_CITY, pPlot->getX(), pPlot->getY(), -1, true);

		pCity = pPlot->getPlotCity();
		if ((pCity == NULL) || (pCity->getOwnerINLINE() != getID()))
		{
			//this should never happen since the cost for a city should be 0 if
			//the city can't be placed.
			//(It can happen if another player has placed a city in the fog)
			FAssertMsg(false, "ADVANCEDSTARTACTION_CITY failed in unexpected way");
			return false;
		}
	}
/************************************************************************************************/
/* Afforess	                  Start		 06/07/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	//Only expand culture when we have lots to spare. Never expand for the capital, the palace works fine on it's own
	if (pCity != getCapitalCity() && getAdvancedStartPoints() > getAdvancedStartCultureCost(true, pCity) * 50)
	{
		if (pCity->getCultureLevel() <= 1)
		{
			doAdvancedStartAction(ADVANCEDSTARTACTION_CULTURE, pPlot->getX(), pPlot->getY(), -1, true);
			//to account for culture expansion.
			pCity->AI_updateBestBuild();
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/	
	
	int iPlotsImproved = 0;
/************************************************************************************************/
/* Afforess	                  Start		 06/07/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	for (int iI = 0; iI < pCity->getNumCityPlots(); iI++)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	{
		if (iI != CITY_HOME_PLOT)
		{
			CvPlot* pLoopPlot = plotCity(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iI);
			if ((pLoopPlot != NULL) && (pLoopPlot->getWorkingCity() == pCity))
			{
				if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
				{
					iPlotsImproved++;
				}
			}
		}
	}
/************************************************************************************************/
/* Afforess	                  Start		 06/07/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	int iDivisor = std::max(1, 2000 / std::max(1, getAdvancedStartPoints()));

	int iTargetPopulation = pCity->happyLevel() + (getCurrentEra() / 2);
	iTargetPopulation /= iDivisor;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/	
	while (iPlotsImproved < iTargetPopulation)
	{
		CvPlot* pBestPlot;
		ImprovementTypes eBestImprovement = NO_IMPROVEMENT;
		int iBestValue = 0;
/************************************************************************************************/
/* Afforess	                  Start		 06/07/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		for (int iI = 0; iI < pCity->getNumCityPlots(); iI++)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		{
			int iValue = pCity->AI_getBestBuildValue(iI);
			if (iValue > iBestValue)
			{
				BuildTypes eBuild = pCity->AI_getBestBuild(iI);
				if (eBuild != NO_BUILD)
				{
					ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
					if (eImprovement != NO_IMPROVEMENT)
					{
						CvPlot* pLoopPlot = plotCity(pCity->getX_INLINE(), pCity->getY_INLINE(), iI);
						if ((pLoopPlot != NULL) && (pLoopPlot->getImprovementType() != eImprovement))
						{
							eBestImprovement = eImprovement;
							pBestPlot = pLoopPlot;
							iBestValue = iValue;
						}
					}
				}
			}
		}
		
		if (iBestValue > 0)
		{
			
			FAssert(pBestPlot != NULL);
			doAdvancedStartAction(ADVANCEDSTARTACTION_IMPROVEMENT, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), eBestImprovement, true);
			iPlotsImproved++;
			if (pCity->getPopulation() < iPlotsImproved)
			{
				doAdvancedStartAction(ADVANCEDSTARTACTION_POP, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), -1, true);
			}
		}
		else
		{
			break;
		}
	}
		

	while (iPlotsImproved > pCity->getPopulation())
	{
		int iPopCost = getAdvancedStartPopCost(true, pCity);
		if (iPopCost <= 0 || iPopCost > getAdvancedStartPoints())
		{
			break;
		}
		if (pCity->healthRate() < 0)
		{
			break;
		}
		doAdvancedStartAction(ADVANCEDSTARTACTION_POP, pPlot->getX_INLINE(), pPlot->getY_INLINE(), -1, true);
	}
	
	pCity->AI_updateAssignWork();

	return true;
}




//Returns false if we have no more points.
bool CvPlayerAI::AI_advancedStartDoRoute(CvPlot* pFromPlot, CvPlot* pToPlot)
{
	FAssert(pFromPlot != NULL);
	FAssert(pToPlot != NULL);
	
	FAStarNode* pNode;
	gDLL->getFAStarIFace()->ForceReset(&GC.getStepFinder());
	if (gDLL->getFAStarIFace()->GeneratePath(&GC.getStepFinder(), pFromPlot->getX_INLINE(), pFromPlot->getY_INLINE(), pToPlot->getX_INLINE(), pToPlot->getY_INLINE(), false, 0, true))
	{
		pNode = gDLL->getFAStarIFace()->GetLastNode(&GC.getStepFinder());
		if (pNode != NULL)
		{
			if (pNode->m_iData1 > (1 + stepDistance(pFromPlot->getX(), pFromPlot->getY(), pToPlot->getX(), pToPlot->getY())))
			{
				//Don't build convulted paths.
				return true;
			}
		}

		while (pNode != NULL)
		{
			CvPlot* pPlot = GC.getMapINLINE().plotSorenINLINE(pNode->m_iX, pNode->m_iY);
			RouteTypes eRoute = AI_bestAdvancedStartRoute(pPlot);
			if (eRoute != NO_ROUTE)
			{
				if (getAdvancedStartRouteCost(eRoute, true, pPlot) > getAdvancedStartPoints())
				{
					return false;
				}
				doAdvancedStartAction(ADVANCEDSTARTACTION_ROUTE, pNode->m_iX, pNode->m_iY, eRoute, true);
			}
			pNode = pNode->m_pParent;			
		}
	}
	return true;
}
void CvPlayerAI::AI_advancedStartRouteTerritory()
{
//	//This uses a heuristic to create a road network
//	//which is at least effecient if not all inclusive
//	//Basically a human will place roads where they connect
//	//the maximum number of trade groups and this
//	//mimics that.
//	
//	
//	CvPlot* pLoopPlot;
//	CvPlot* pLoopPlot2;
//	int iI, iJ;
//	int iPass;
//	
//
//	std::vector<int> aiPlotGroups;
//	for (iPass = 4; iPass > 1; --iPass)
//	{
//		for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
//		{
//			pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
//			if ((pLoopPlot != NULL) && (pLoopPlot->getOwner() == getID()) && (pLoopPlot->getRouteType() == NO_ROUTE))
//			{
//				aiPlotGroups.clear();
//				if (pLoopPlot->getPlotGroup(getID()) != NULL)
//				{
//					aiPlotGroups.push_back(pLoopPlot->getPlotGroup(getID())->getID());
//				}
//				for (iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
//				{
//					pLoopPlot2 = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), (DirectionTypes)iJ);
//					if ((pLoopPlot2 != NULL) && (pLoopPlot2->getRouteType() != NO_ROUTE))
//					{
//						CvPlotGroup* pPlotGroup = pLoopPlot2->getPlotGroup(getID());
//						if (pPlotGroup != NULL)
//						{
//							if (std::find(aiPlotGroups.begin(),aiPlotGroups.end(), pPlotGroup->getID()) == aiPlotGroups.end())
//							{
//								aiPlotGroups.push_back(pPlotGroup->getID());
//							}
//						}
//					}
//				}
//				if ((int)aiPlotGroups.size() >= iPass)
//				{
//					RouteTypes eBestRoute = AI_bestAdvancedStartRoute(pLoopPlot);
//					if (eBestRoute != NO_ROUTE)
//					{
//						doAdvancedStartAction(ADVANCEDSTARTACTION_ROUTE, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), eBestRoute, true);
//					}
//				}
//			}
//		}
//	}
//	
//	//Maybe try to build road network for mobility but bearing in mind
//	//that routes can't be built outside culture atm. I think workers
//	//can do that just fine.

	CvPlot* pLoopPlot;
	int iI;
	
	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		if ((pLoopPlot != NULL) && (pLoopPlot->getOwner() == getID()) && (pLoopPlot->getRouteType() == NO_ROUTE))
		{
			if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
			{
				BonusTypes eBonus = pLoopPlot->getBonusType(getTeam());
				if (eBonus != NO_BONUS)
				{
					if (GC.getImprovementInfo(pLoopPlot->getImprovementType()).isImprovementBonusTrade(eBonus))
					{
						int iBonusValue = AI_bonusVal(eBonus, 1);
						if (iBonusValue > 9)
						{
							int iBestValue = 0;
							CvPlot* pBestPlot = NULL;
							int iRange = 2;
							for (int iX = -iRange; iX <= iRange; iX++)
							{
								for (int iY = -iRange; iY <= iRange; iY++)
								{
									CvPlot* pLoopPlot2 = plotXY(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), iX, iY);
									if (pLoopPlot2 != NULL)
									{
										if (pLoopPlot2->getOwner() == getID())
										{
											if ((pLoopPlot2->isConnectedToCapital()) || pLoopPlot2->isCity())
											{
												int iValue = 1000;
												if (pLoopPlot2->isCity())
												{
													iValue += 100;
													if (pLoopPlot2->getPlotCity()->isCapital())
													{
														iValue += 100;
													}
												}
												if (pLoopPlot2->isRoute())
												{
													iValue += 100;
												}
												int iDistance = GC.getMapINLINE().calculatePathDistance(pLoopPlot, pLoopPlot2);
												if (iDistance > 0)
												{
													iValue /= (1 + iDistance);
													
													if (iValue > iBestValue)
													{
														iBestValue = iValue;
														pBestPlot = pLoopPlot2;
													}
												}
											}
										}
									}
								}
							}
							if (pBestPlot != NULL)
							{
								if (!AI_advancedStartDoRoute(pLoopPlot, pBestPlot))
								{
									return;
								}
							}
						}
					}
				}
				if (pLoopPlot->getRouteType() == NO_ROUTE)
				{
					int iRouteYieldValue = 0;
					RouteTypes eRoute = (AI_bestAdvancedStartRoute(pLoopPlot, &iRouteYieldValue));
					if (eRoute != NO_ROUTE && iRouteYieldValue > 0)
					{
						doAdvancedStartAction(ADVANCEDSTARTACTION_ROUTE, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), eRoute, true);
					}
				}
			}
		}
	}
	
	//Connect Cities
	int iLoop;
	CvCity* pLoopCity;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (!pLoopCity->isCapital() && !pLoopCity->isConnectedToCapital())
		{
			int iBestValue = 0;
			CvPlot* pBestPlot = NULL;
			int iRange = 5;
			for (int iX = -iRange; iX <= iRange; iX++)
			{
				for (int iY = -iRange; iY <= iRange; iY++)
				{
					CvPlot* pLoopPlot = plotXY(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), iX, iY);
					if ((pLoopPlot != NULL) && (pLoopPlot->getOwner() == getID()))
					{
						if ((pLoopPlot->isConnectedToCapital()) || pLoopPlot->isCity())
						{
							int iValue = 1000;
							if (pLoopPlot->isCity())
							{
								iValue += 500;
								if (pLoopPlot->getPlotCity()->isCapital())
								{
									iValue += 500;
								}
							}
							if (pLoopPlot->isRoute())
							{
								iValue += 100;
							}
							int iDistance = GC.getMapINLINE().calculatePathDistance(pLoopCity->plot(), pLoopPlot);
							if (iDistance > 0)
							{
								iValue /= (1 + iDistance);
								
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = pLoopPlot;
								}
							}
						}
					}
				}
			}
			if (NULL != pBestPlot)
			{
				if (!AI_advancedStartDoRoute(pBestPlot, pLoopCity->plot()))
				{
					return;
				}
			}
		}
	}
}


void CvPlayerAI::AI_doAdvancedStart(bool bNoExit)
{
	FAssertMsg(!isBarbarian(), "Should not be called for barbarians!");

	if (NULL == getStartingPlot())
	{
		FAssert(false);
		return;
	}

	int iLoop;
	CvCity* pLoopCity;
	
	int iStartingPoints = getAdvancedStartPoints();
	int iRevealPoints = (iStartingPoints * 10) / 100;
	int iMilitaryPoints = (iStartingPoints * (isHuman() ? 17 : (10 + (GC.getLeaderHeadInfo(getPersonalityType()).getBuildUnitProb() / 3)))) / 100;
	int iCityPoints = iStartingPoints - (iMilitaryPoints + iRevealPoints);

	if (getCapitalCity() != NULL)
	{
		AI_advancedStartPlaceCity(getCapitalCity()->plot());
	}
	else
	{
		for (int iPass = 0; iPass < 2 && NULL == getCapitalCity(); ++iPass)
		{
			CvPlot* pBestCapitalPlot = AI_advancedStartFindCapitalPlot();

			if (pBestCapitalPlot != NULL)
			{
				if (!AI_advancedStartPlaceCity(pBestCapitalPlot))
				{
					FAssertMsg(false, "AS AI: Unexpected failure placing capital");
				}
				break;
			}
			else
			{
				//If this point is reached, the advanced start system is broken.
				//Find a new starting plot for this player
				setStartingPlot(findStartingPlot(false), true);
				//Redo Starting visibility
				CvPlot* pStartingPlot = getStartingPlot();
				if (NULL != pStartingPlot)
				{
					for (int iPlotLoop = 0; iPlotLoop < GC.getMapINLINE().numPlots(); ++iPlotLoop)
					{
						CvPlot* pPlot = GC.getMapINLINE().plotByIndex(iPlotLoop);

						if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE()) <= GC.getDefineINT("ADVANCED_START_SIGHT_RANGE"))
						{
							pPlot->setRevealed(getTeam(), true, false, NO_TEAM, false);
						}
					}
				}
			}
		}

		if (getCapitalCity() == NULL)
		{
			if (!bNoExit)
			{
				doAdvancedStartAction(ADVANCEDSTARTACTION_EXIT, -1, -1, -1, true);
			}
			return;
		}
	}
	
	iCityPoints -= (iStartingPoints - getAdvancedStartPoints());
	
	int iLastPointsTotal = getAdvancedStartPoints();
	
	for (int iPass = 0; iPass < 6; iPass++)
	{
		for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
		{
			CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
			if (pLoopPlot->isRevealed(getTeam(), false))
			{
				if (pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
				{
					AI_advancedStartRevealRadius(pLoopPlot, CITY_PLOTS_RADIUS);					
				}
				else
				{
					for (int iJ = 0; iJ < NUM_CARDINALDIRECTION_TYPES; iJ++)
					{
						CvPlot* pLoopPlot2 = plotCardinalDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), (CardinalDirectionTypes)iJ);
						if ((pLoopPlot2 != NULL) && (getAdvancedStartVisibilityCost(true, pLoopPlot2) > 0))
						{
							//Mildly maphackery but any smart human can see the terrain type of a tile.
							pLoopPlot2->getTerrainType();
							int iFoodYield = GC.getTerrainInfo(pLoopPlot2->getTerrainType()).getYield(YIELD_FOOD);
							if (pLoopPlot2->getFeatureType() != NO_FEATURE)
							{
								iFoodYield += GC.getFeatureInfo(pLoopPlot2->getFeatureType()).getYieldChange(YIELD_FOOD);
							}
							if (((iFoodYield >= 2) && !pLoopPlot2->isFreshWater()) || pLoopPlot2->isHills() || pLoopPlot2->isRiver())
							{
								doAdvancedStartAction(ADVANCEDSTARTACTION_VISIBILITY, pLoopPlot2->getX_INLINE(), pLoopPlot2->getY_INLINE(), -1, true);						
							}
						}
					}
				}
			}
			if ((iLastPointsTotal - getAdvancedStartPoints()) > iRevealPoints)
			{
				break;
			}
		}
	}
	
	iLastPointsTotal = getAdvancedStartPoints();
	iCityPoints = std::min(iCityPoints, iLastPointsTotal);
	int iArea = -1; //getStartingPlot()->getArea();
	
	//Spend econ points on a tech?
	int iTechRand = 90 + GC.getGame().getSorenRandNum(20, "AI AS Buy Tech 1");
	int iTotalTechSpending = 0;
	
	if (getCurrentEra() == 0)
	{
		TechTypes eTech = AI_bestTech(1);
		if ((eTech != NO_TECH) && !GC.getTechInfo(eTech).isRepeat())
		{
			int iTechCost = getAdvancedStartTechCost(eTech, true);
			if (iTechCost > 0)
			{
				doAdvancedStartAction(ADVANCEDSTARTACTION_TECH, -1, -1, eTech, true);
				iTechRand -= 50;
				iTotalTechSpending += iTechCost;
			}
		}
	}
	
	bool bDonePlacingCities = false;	
	for (int iPass = 0; iPass < 100; ++iPass)
	{
		int iRand = iTechRand + 10 * getNumCities();
		if ((iRand > 0) && (GC.getGame().getSorenRandNum(100, "AI AS Buy Tech 2") < iRand))
		{
			TechTypes eTech = AI_bestTech(1);
			if ((eTech != NO_TECH) && !GC.getTechInfo(eTech).isRepeat())
			{
				int iTechCost = getAdvancedStartTechCost(eTech, true);
				if ((iTechCost > 0) && ((iTechCost + iTotalTechSpending) < (iCityPoints / 4)))
				{
					doAdvancedStartAction(ADVANCEDSTARTACTION_TECH, -1, -1, eTech, true);
					iTechRand -= 50;
					iTotalTechSpending += iTechCost;
					
					for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
					{	
						AI_advancedStartPlaceCity(pLoopCity->plot());
					}
				}
			}
		}
		int iBestFoundValue = 0;
		CvPlot* pBestFoundPlot = NULL;
		AI_updateFoundValues(false);
		for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
		{
			CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
			//if (pLoopPlot->area() == getStartingPlot()->area())
			{
				if (plotDistance(getStartingPlot()->getX_INLINE(), getStartingPlot()->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) < 9)
				{
					if (pLoopPlot->getFoundValue(getID()) > iBestFoundValue)
					{
						if (getAdvancedStartCityCost(true, pLoopPlot) > 0)
						{
							pBestFoundPlot = pLoopPlot;
							iBestFoundValue = pLoopPlot->getFoundValue(getID());
						}
					}
				}
			}
		}
		
		if (iBestFoundValue < ((getNumCities() == 0) ? 1 : (500 + 250 * getNumCities())))
		{
			bDonePlacingCities = true;			
		}
		if (!bDonePlacingCities)
		{
			int iCost = getAdvancedStartCityCost(true, pBestFoundPlot);
			if (iCost > getAdvancedStartPoints())
			{
				bDonePlacingCities = true;
			}// at 500pts, we have 200, we spend 100. 
			else if (((iLastPointsTotal - getAdvancedStartPoints()) + iCost) > iCityPoints)
			{
				bDonePlacingCities = true;
			}
		}
		
		if (!bDonePlacingCities)
		{
			if (!AI_advancedStartPlaceCity(pBestFoundPlot))
			{
				FAssertMsg(false, "AS AI: Failed to place city (non-capital)");
				bDonePlacingCities = true;
			}
		}

		if (bDonePlacingCities)
		{
			break;
		}
	}


	bool bDoneWithTechs = false;
	while (!bDoneWithTechs)
	{
		bDoneWithTechs = true;
		TechTypes eTech = AI_bestTech(1);
		if (eTech != NO_TECH && !GC.getTechInfo(eTech).isRepeat())
		{
			int iTechCost = getAdvancedStartTechCost(eTech, true);
			if ((iTechCost > 0) && ((iTechCost + iLastPointsTotal - getAdvancedStartPoints()) <= iCityPoints))
			{
				doAdvancedStartAction(ADVANCEDSTARTACTION_TECH, -1, -1, eTech, true);
				bDoneWithTechs = false;
			}
		}
	}
	
	{
		//Land
		AI_advancedStartPlaceExploreUnits(true);
		if (getCurrentEra() > 2)
		{
			//Sea
			AI_advancedStartPlaceExploreUnits(false);
			if (GC.getGameINLINE().circumnavigationAvailable())
			{
				if (GC.getGameINLINE().getSorenRandNum(GC.getGameINLINE().countCivPlayersAlive(), "AI AS buy 2nd sea explorer") < 2)
				{
					AI_advancedStartPlaceExploreUnits(false);
				}
			}
		}
	}
	
	AI_advancedStartRouteTerritory();
		
	bool bDoneBuildings = (iLastPointsTotal - getAdvancedStartPoints()) > iCityPoints;
	for (int iPass = 0; iPass < 10 && !bDoneBuildings; ++iPass)
	{
		for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			BuildingTypes eBuilding = pLoopCity->AI_bestAdvancedStartBuilding(iPass);
			if (eBuilding != NO_BUILDING)
			{
				bDoneBuildings = (iLastPointsTotal - (getAdvancedStartPoints() - getAdvancedStartBuildingCost(eBuilding, true, pLoopCity))) > iCityPoints;
				if (!bDoneBuildings)
				{
					doAdvancedStartAction(ADVANCEDSTARTACTION_BUILDING, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), eBuilding, true);
				}
				else
				{
					//continue there might be cheaper buildings in other cities we can afford
				}
			}
		}
	}
	
	//Units
	std::vector<UnitAITypes> aeUnitAITypes;
	aeUnitAITypes.push_back(UNITAI_CITY_DEFENSE);
	aeUnitAITypes.push_back(UNITAI_WORKER);
	aeUnitAITypes.push_back(UNITAI_RESERVE);
	aeUnitAITypes.push_back(UNITAI_COUNTER);
	
	
	bool bDone = false;
	for (int iPass = 0; iPass < 10; ++iPass)
	{
		for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			if ((iPass == 0) || (pLoopCity->getArea() == getStartingPlot()->getArea()))
			{
				CvPlot* pUnitPlot = pLoopCity->plot();
				//Token defender
				UnitTypes eBestUnit = AI_bestAdvancedStartUnitAI(pUnitPlot, aeUnitAITypes[iPass % aeUnitAITypes.size()]);
				if (eBestUnit != NO_UNIT)
				{
					if (getAdvancedStartUnitCost(eBestUnit, true, pUnitPlot) > getAdvancedStartPoints())
					{
						bDone = true;
						break;
					}
					doAdvancedStartAction(ADVANCEDSTARTACTION_UNIT, pUnitPlot->getX(), pUnitPlot->getY(), eBestUnit, true);
				}
			}
		}
	}
	
	if (isHuman())
	{
		// remove unhappy population
		for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			while (pLoopCity->angryPopulation() > 0 && getAdvancedStartPopCost(false, pLoopCity) > 0)
			{
				doAdvancedStartAction(ADVANCEDSTARTACTION_POP, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), -1, false);
			}
		}
	}

	if (!bNoExit)
	{
		doAdvancedStartAction(ADVANCEDSTARTACTION_EXIT, -1, -1, -1, true);
	}

}


void CvPlayerAI::AI_recalculateFoundValues(int iX, int iY, int iInnerRadius, int iOuterRadius) const
{
	CvPlot* pLoopPlot;
	int iLoopX, iLoopY;
	int iValue;
	
	for (iLoopX = -iOuterRadius; iLoopX <= iOuterRadius; iLoopX++)
	{
		for (iLoopY = -iOuterRadius; iLoopY <= iOuterRadius; iLoopY++)
		{
			pLoopPlot = plotXY(iX, iY, iLoopX, iLoopY);
			if ((NULL != pLoopPlot) && !AI_isPlotCitySite(pLoopPlot))
			{
				if (stepDistance(0, 0, iLoopX, iLoopY) <= iInnerRadius)
				{
					if (!((iLoopX == 0) && (iLoopY == 0)))
					{
						pLoopPlot->setFoundValue(getID(), 0);
					}
				}
				else
				{
					if ((pLoopPlot != NULL) && (pLoopPlot->isRevealed(getTeam(), false)))
					{
						long lResult=-1;
						if(GC.getUSE_GET_CITY_FOUND_VALUE_CALLBACK())
						{
							CyArgsList argsList;
							argsList.add((int)getID());
							argsList.add(pLoopPlot->getX());
							argsList.add(pLoopPlot->getY());
							gDLL->getPythonIFace()->callFunction(PYGameModule, "getCityFoundValue", argsList.makeFunctionArgs(), &lResult);
						}

						if (lResult == -1)
						{
							iValue = AI_foundValue(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
						}
						else
						{
							iValue = lResult;
						}

						pLoopPlot->setFoundValue(getID(), iValue);

						if (iValue > pLoopPlot->area()->getBestFoundValue(getID()))
						{
							pLoopPlot->area()->setBestFoundValue(getID(), iValue);
						}
					}
				}
			}
		}
	}
}

	
int CvPlayerAI::AI_getMinFoundValue() const
{
	int iValue = 600;
	int iNetCommerce = 1 + getCommerceRate(COMMERCE_GOLD) + getCommerceRate(COMMERCE_RESEARCH) + std::max(0, getGoldPerTurn());
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       06/11/09                       jdog5000 & DanF5771    */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original BTS code
	int iNetExpenses = calculateInflatedCosts() + std::min(0, getGoldPerTurn());
*/
	int iNetExpenses = calculateInflatedCosts() + std::max(0, -getGoldPerTurn());
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
	
	iValue *= iNetCommerce;
	iValue /= std::max(std::max(1, iNetCommerce / 4), iNetCommerce - iNetExpenses);
	
	if (GET_TEAM(getTeam()).getAnyWarPlanCount(1) > 0)
	{
		iValue *= 2;
	}
	
	return iValue;
}
	
void CvPlayerAI::AI_updateCitySites(int iMinFoundValueThreshold, int iMaxSites) const
{
	std::vector<int>::iterator it;
	int iValue;
	int iI;
	
	int iPass = 0;
	while (iPass < iMaxSites)
	{
		//Add a city to the list.
		int iBestFoundValue = 0;
		CvPlot* pBestFoundPlot = NULL;

		for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
		{
			CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
			if (pLoopPlot->isRevealed(getTeam(), false))
			{
				iValue = pLoopPlot->getFoundValue(getID());
				if (iValue > iMinFoundValueThreshold)
				{
					if (!AI_isPlotCitySite(pLoopPlot))
					{
						iValue *= std::min(NUM_CITY_PLOTS * 2, pLoopPlot->area()->getNumUnownedTiles());					

						if (iValue > iBestFoundValue)
						{
							iBestFoundValue = iValue;
							pBestFoundPlot = pLoopPlot;
						}
					}
				}
			}
		}
		if (pBestFoundPlot != NULL)
		{
			m_aiAICitySites.push_back(GC.getMapINLINE().plotNum(pBestFoundPlot->getX_INLINE(), pBestFoundPlot->getY_INLINE()));
			AI_recalculateFoundValues(pBestFoundPlot->getX_INLINE(), pBestFoundPlot->getY_INLINE(), CITY_PLOTS_RADIUS, 2 * CITY_PLOTS_RADIUS);
		}
		else
		{
			break;
		}
		iPass++;
	}
}

void CvPlayerAI::AI_invalidateCitySites(int iMinFoundValueThreshold) const
{
	m_aiAICitySites.clear();
}

int CvPlayerAI::AI_getNumCitySites() const
{
	return m_aiAICitySites.size();
	
}

bool CvPlayerAI::AI_isPlotCitySite(CvPlot* pPlot) const
{
	std::vector<int>::iterator it;
	int iPlotIndex = GC.getMapINLINE().plotNumINLINE(pPlot->getX_INLINE(), pPlot->getY_INLINE());
	
	for (it = m_aiAICitySites.begin(); it != m_aiAICitySites.end(); it++)
	{
		if ((*it) == iPlotIndex)
		{
			return true;
		}		
	}
	return false;
	
}

int CvPlayerAI::AI_getNumAreaCitySites(int iAreaID, int& iBestValue) const
{
	std::vector<int>::iterator it;
	int iCount = 0;
	iBestValue = 0;
	
	for (it = m_aiAICitySites.begin(); it != m_aiAICitySites.end(); it++)
	{
		CvPlot* pCitySitePlot = GC.getMapINLINE().plotByIndex((*it));
		if (pCitySitePlot->getArea() == iAreaID)
		{
			iCount++;
			iBestValue = std::max(iBestValue, pCitySitePlot->getFoundValue(getID()));
		}		
	}
	return iCount;
}

int CvPlayerAI::AI_getNumAdjacentAreaCitySites(int iWaterAreaID, int iExcludeArea, int& iBestValue) const
{
	std::vector<int>::iterator it;	
	int iCount = 0;
	iBestValue = 0;

	for (it = m_aiAICitySites.begin(); it != m_aiAICitySites.end(); it++)
	{
		CvPlot* pCitySitePlot = GC.getMapINLINE().plotByIndex((*it));
		if (pCitySitePlot->getArea() != iExcludeArea)
		{
			if (pCitySitePlot->isAdjacentToArea(iWaterAreaID))
			{
				iCount++;
				iBestValue = std::max(iBestValue, pCitySitePlot->getFoundValue(getID()));
			}	
		}
	}
	return iCount;	
	
	
}

CvPlot* CvPlayerAI::AI_getCitySite(int iIndex) const
{
	FAssert(iIndex < (int)m_aiAICitySites.size());
	return GC.getMapINLINE().plotByIndex(m_aiAICitySites[iIndex]);
}

int CvPlayerAI::AI_bestAreaUnitAIValue(UnitAITypes eUnitAI, CvArea* pArea, UnitTypes* peBestUnitType) const
{
	
	CvCity* pCity = NULL;
	
	if (pArea != NULL)
	{
		if (getCapitalCity() != NULL)
		{
			if (pArea->isWater())
			{
				if (getCapitalCity()->plot()->isAdjacentToArea(pArea))
				{
					pCity = getCapitalCity();
				}			
			}
			else
			{
				if (getCapitalCity()->getArea() == pArea->getID())
				{
					pCity = getCapitalCity();
				}
			}
		}
	
		if (NULL == pCity)
		{
			CvCity* pLoopCity;
			int iLoop;
			for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
			{
				if (pArea->isWater())
				{
					if (pLoopCity->plot()->isAdjacentToArea(pArea))
					{
						pCity = pLoopCity;
						break;
					}
				}
				else
				{
					if (pLoopCity->getArea() == pArea->getID())
					{
						pCity = pLoopCity;
						break;
					}
				}
			}
		}
	}
	
	return AI_bestCityUnitAIValue(eUnitAI, pCity, peBestUnitType);
	
}

int CvPlayerAI::AI_bestCityUnitAIValue(UnitAITypes eUnitAI, CvCity* pCity, UnitTypes* peBestUnitType) const
{
	UnitTypes eLoopUnit;
	int iValue;
	int iBestValue;
	int iI;
	
	FAssertMsg(eUnitAI != NO_UNITAI, "UnitAI is not assigned a valid value");
	
	iBestValue = 0;
	
	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

		if (eLoopUnit != NO_UNIT)
		{
			if (!isHuman() || (GC.getUnitInfo(eLoopUnit).getDefaultUnitAIType() == eUnitAI))
			{
				if (NULL == pCity ? canTrain(eLoopUnit) : pCity->canTrain(eLoopUnit))
				{
					iValue = AI_unitValue(eLoopUnit, eUnitAI, (pCity == NULL) ? NULL : pCity->area());
					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						if (peBestUnitType != NULL)
						{
							*peBestUnitType = eLoopUnit;
						}
					}
				}
			}
		}
	}

	return iBestValue;
}

int CvPlayerAI::AI_calculateTotalBombard(DomainTypes eDomain) const
{
	int iI;
	int iTotalBombard = 0;
	
	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		UnitTypes eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));
		if (eLoopUnit != NO_UNIT)
		{
			if (GC.getUnitInfo(eLoopUnit).getDomainType() == eDomain)
			{
				int iBombardRate = GC.getUnitInfo(eLoopUnit).getBombardRate();
				
				if (iBombardRate > 0)
				{
					if( GC.getUnitInfo(eLoopUnit).isIgnoreBuildingDefense() )
					{
						iBombardRate *= 3;
						iBombardRate /= 2;
					}

					iTotalBombard += iBombardRate * getUnitClassCount((UnitClassTypes)iI);
				}
				
				int iBombRate = GC.getUnitInfo(eLoopUnit).getBombRate();
				if (iBombRate > 0)
				{
					iTotalBombard += iBombRate * getUnitClassCount((UnitClassTypes)iI);
				}
			}
		}
	}
	
	return iTotalBombard;
}

void CvPlayerAI::AI_updateBonusValue(BonusTypes eBonus)
{
	FAssert(m_aiBonusValue != NULL);

	//reset
    m_aiBonusValue[eBonus] = -1;
}


void CvPlayerAI::AI_updateBonusValue()
{
	PROFILE_FUNC();

	FAssert(m_aiBonusValue != NULL);

	for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		AI_updateBonusValue((BonusTypes)iI);		
	}
}

int CvPlayerAI::AI_getUnitClassWeight(UnitClassTypes eUnitClass) const
{
	return m_aiUnitClassWeights[eUnitClass] / 100;
}

int CvPlayerAI::AI_getUnitCombatWeight(UnitCombatTypes eUnitCombat) const
{
	return m_aiUnitCombatWeights[eUnitCombat] / 100;
}

void CvPlayerAI::AI_doEnemyUnitData()
{
	std::vector<int> aiUnitCounts(GC.getNumUnitInfos(), 0);
	
	std::vector<int> aiDomainSums(NUM_DOMAIN_TYPES, 0);
	
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int iI;
	
	int iOldTotal = 0;
	int iNewTotal = 0;
	
	// Count enemy land and sea units visible to us
	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		int iAdjacentAttackers = -1;
		if (pLoopPlot->isVisible(getTeam(), false))
		{
			pUnitNode = pLoopPlot->headUnitNode();

			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
				
				if (pLoopUnit->canFight())
				{
					int iUnitValue = 1;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/18/10                                jdog5000      */
/*                                                                                              */
/* War Strategy AI                                                                              */
/************************************************************************************************/
					//if (atWar(getTeam(), pLoopUnit->getTeam()))
					if( GET_TEAM(getTeam()).AI_getWarPlan(pLoopUnit->getTeam()) != NO_WARPLAN )
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
					{
						iUnitValue += 10;
					
						if ((pLoopPlot->getOwnerINLINE() == getID()))
						{
							iUnitValue += 15;
						}
						else if (atWar(getTeam(), pLoopPlot->getTeam()))
						{
							if (iAdjacentAttackers == -1)
							{
								iAdjacentAttackers = GET_PLAYER(pLoopPlot->getOwnerINLINE()).AI_adjacentPotentialAttackers(pLoopPlot);
							}
							if (iAdjacentAttackers > 0)
							{
								iUnitValue += 15;
							}
						}
					}
					else if (pLoopUnit->getOwnerINLINE() != getID())
					{
						iUnitValue += pLoopUnit->canAttack() ? 4 : 1;
						if (pLoopPlot->getCulture(getID()) > 0)
						{
							iUnitValue += pLoopUnit->canAttack() ? 4 : 1;
						}	
					}
					
					// If we hadn't seen any of this class before
					if (m_aiUnitClassWeights[pLoopUnit->getUnitClassType()] == 0)
					{
						iUnitValue *= 4;
					}
					
					iUnitValue *= pLoopUnit->baseCombatStr();
					aiUnitCounts[pLoopUnit->getUnitType()] += iUnitValue;
					aiDomainSums[pLoopUnit->getDomainType()] += iUnitValue;
					iNewTotal += iUnitValue;
				}
			}
		}
	}

	if (iNewTotal == 0)
	{
		//This should rarely happen.
		return;
	}

	//Decay
	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		m_aiUnitClassWeights[iI] -= 100;
		m_aiUnitClassWeights[iI] *= 3;
		m_aiUnitClassWeights[iI] /= 4;
		m_aiUnitClassWeights[iI] = std::max(0, m_aiUnitClassWeights[iI]);
	}
	
	for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
	{
		if (aiUnitCounts[iI] > 0)
		{
			UnitTypes eLoopUnit = (UnitTypes)iI;

			TechTypes eTech = (TechTypes)GC.getUnitInfo((UnitTypes)iI).getPrereqAndTech();
			
			int iEraDiff = (eTech == NO_TECH) ? 4 : std::min(4, getCurrentEra() - GC.getTechInfo(eTech).getEra());

			if (iEraDiff > 1)
			{
				iEraDiff -= 1;
				aiUnitCounts[iI] *= 3 - iEraDiff;
				aiUnitCounts[iI] /= 3;
			}
			FAssert(aiDomainSums[GC.getUnitInfo(eLoopUnit).getDomainType()] > 0);
			m_aiUnitClassWeights[GC.getUnitInfo(eLoopUnit).getUnitClassType()] += (5000 * aiUnitCounts[iI]) / std::max(1, aiDomainSums[GC.getUnitInfo(eLoopUnit).getDomainType()]);
		}
	}
	
	for (iI = 0; iI < GC.getNumUnitCombatInfos(); ++iI)
	{
		m_aiUnitCombatWeights[iI] = 0;
	}
	
	for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		if (m_aiUnitClassWeights[iI] > 0)
		{
/************************************************************************************************/
/* Afforess	                  Start		 01/17/10                                               */
/*                                                                                              */
/*   TakBal Crash Fix when NO_UNITCOMBAT has a defense value                                    */
/************************************************************************************************/
			UnitTypes eUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex();
            int ctype = GC.getUnitInfo(eUnit).getUnitCombatType();
            if (ctype >= 0 && ctype < GC.getNumUnitCombatInfos())
                m_aiUnitCombatWeights[ctype] += m_aiUnitClassWeights[iI];
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		}
	}
	
	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		if (m_aiUnitCombatWeights[iI] > 25)
		{
			m_aiUnitCombatWeights[iI] += 2500;
		}
		else if (m_aiUnitCombatWeights[iI] > 0)
		{
			m_aiUnitCombatWeights[iI] += 1000;
		}
	}
}

int CvPlayerAI::AI_calculateUnitAIViability(UnitAITypes eUnitAI, DomainTypes eDomain) const
{
	int iBestUnitAIStrength = 0;
	int iBestOtherStrength = 0;
	
	for (int iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		UnitTypes eLoopUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex();
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       01/15/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original BTS code
		CvUnitInfo& kUnitInfo = GC.getUnitInfo((UnitTypes)iI);
*/
		//Afforess CTD Fix:
		if (eLoopUnit == NO_UNIT)
			continue;
		CvUnitInfo& kUnitInfo = GC.getUnitInfo(eLoopUnit);
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
		if (kUnitInfo.getDomainType() == eDomain)
		{

			TechTypes eTech = (TechTypes)kUnitInfo.getPrereqAndTech();
			if ((m_aiUnitClassWeights[iI] > 0) || GET_TEAM(getTeam()).isHasTech((eTech)))
			{
				if (kUnitInfo.getUnitAIType(eUnitAI))
				{
					iBestUnitAIStrength = std::max(iBestUnitAIStrength, kUnitInfo.getCombat());
				}

				iBestOtherStrength = std::max(iBestOtherStrength, kUnitInfo.getCombat());
			}
		}
	}
	
	return (100 * iBestUnitAIStrength) / std::max(1, iBestOtherStrength);
}

ReligionTypes CvPlayerAI::AI_chooseReligion()
{
	ReligionTypes eFavorite = (ReligionTypes)GC.getLeaderHeadInfo(getLeaderType()).getFavoriteReligion();
	if (NO_RELIGION != eFavorite && !GC.getGameINLINE().isReligionFounded(eFavorite))
	{
		return eFavorite;
	}

	std::vector<ReligionTypes> aeReligions;
	for (int iReligion = 0; iReligion < GC.getNumReligionInfos(); ++iReligion)
	{
		if (!GC.getGameINLINE().isReligionFounded((ReligionTypes)iReligion))
		{
			aeReligions.push_back((ReligionTypes)iReligion);
		}
	}

	if (aeReligions.size() > 0)
	{
		return aeReligions[GC.getGameINLINE().getSorenRandNum(aeReligions.size(), "AI pick religion")];
	}

	return NO_RELIGION;
}

int CvPlayerAI::AI_getAttitudeWeight(PlayerTypes ePlayer) const
{
	int iAttitudeWeight = 0;
	switch (AI_getAttitude(ePlayer))
	{
	case ATTITUDE_FURIOUS:
		iAttitudeWeight = -100;
		break;
	case ATTITUDE_ANNOYED:
		iAttitudeWeight = -50;
		break;
	case ATTITUDE_CAUTIOUS:
		iAttitudeWeight = 0;
		break;
	case ATTITUDE_PLEASED:
		iAttitudeWeight = 50;
		break;
	case ATTITUDE_FRIENDLY:
		iAttitudeWeight = 100;			
		break;
	}
	
	return iAttitudeWeight;
}

int CvPlayerAI::AI_getPlotAirbaseValue(CvPlot* pPlot) const
{
	PROFILE_FUNC();
	
	FAssert(pPlot != NULL);
/************************************************************************************************/
/* Afforess	                  Start		 02/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (pPlot->isOwned() && (pPlot->getTeam() != getTeam()))
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	{
		return 0;
	}
	
	if (pPlot->isCityRadius())
	{
		CvCity* pWorkingCity = pPlot->getWorkingCity();
		if (pWorkingCity != NULL)
		{
			if (pWorkingCity->AI_getBestBuild(pWorkingCity->getCityPlotIndex(pPlot)) != NO_BUILD)
			{
				return 0;
			}
			if (pPlot->getImprovementType() != NO_IMPROVEMENT)
			{
				CvImprovementInfo &kImprovementInfo = GC.getImprovementInfo(pPlot->getImprovementType());
				if (!kImprovementInfo.isActsAsCity())
				{
					return 0;
				}
			}
		}
	}
	
	int iMinOtherCityDistance = MAX_INT;
	CvPlot* iMinOtherCityPlot = NULL;
	
	int iMinFriendlyCityDistance = MAX_INT;
	CvPlot* iMinFriendlyCityPlot = NULL;
	
	int iOtherCityCount = 0;
	
	int iRange = 4;
	for (int iX = -iRange; iX <= iRange; iX++)
	{
		for (int iY = -iRange; iY <= iRange; iY++)
		{
			CvPlot* pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iX, iY);
			if ((pLoopPlot != NULL) && (pPlot != pLoopPlot))
			{
				int iDistance = plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
				
				if (pLoopPlot->getTeam() == getTeam())
				{
					if (pLoopPlot->isCity(true))
					{
						if (1 == iDistance)
						{
							return 0;
						}
						if (iDistance < iMinFriendlyCityDistance)
						{
							iMinFriendlyCityDistance = iDistance;
							iMinFriendlyCityPlot = pLoopPlot;
						}
					}
				}
				else
				{
					if (pLoopPlot->isOwned())
					{
						if (pLoopPlot->isCity(false))
						{
							if (iDistance < iMinOtherCityDistance)
							{
								iMinOtherCityDistance = iDistance;
								iMinOtherCityPlot = pLoopPlot;
								iOtherCityCount++;
							}
						}
					}
				}
			}
		}
	}
	
	if (0 == iOtherCityCount)
	{
		return 0;
	}
	
//	if (iMinFriendlyCityPlot != NULL)
//	{
//		FAssert(iMinOtherCityPlot != NULL);
//		if (plotDistance(iMinFriendlyCityPlot->getX_INLINE(), iMinFriendlyCityPlot->getY_INLINE(), iMinOtherCityPlot->getX_INLINE(), iMinOtherCityPlot->getY_INLINE()) < (iMinOtherCityDistance - 1))
//		{
//			return 0;
//		}
//	}

//	if (iMinOtherCityPlot != NULL)
//	{
//		CvCity* pNearestCity = GC.getMapINLINE().findCity(iMinOtherCityPlot->getX_INLINE(), iMinOtherCityPlot->getY_INLINE(), NO_PLAYER, getTeam(), false);
//		if (NULL == pNearestCity)
//		{
//			return 0;
//		}
//		if (plotDistance(pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE(), iMinOtherCityPlot->getX_INLINE(), iMinOtherCityPlot->getY_INLINE()) < iRange)
//		{
//			return 0;
//		}
//	}
		
	
	int iDefenseModifier = pPlot->defenseModifier(getTeam(), false);
//	if (iDefenseModifier <= 0)
//	{
//		return 0;
//	}
	
	int iValue = iOtherCityCount * 50;
	iValue *= 100 + (2 * (iDefenseModifier + (pPlot->isHills() ? 25 : 0)));
	iValue /= 100;
	
	return iValue;
}

int CvPlayerAI::AI_getPlotCanalValue(CvPlot* pPlot) const
{
	PROFILE_FUNC();
	
	FAssert(pPlot != NULL);
	
	if (pPlot->isOwned())
	{
		if (pPlot->getTeam() != getTeam())
		{
			return 0;
		}
		if (pPlot->isCityRadius())
		{
			CvCity* pWorkingCity = pPlot->getWorkingCity();
			if (pWorkingCity != NULL)
			{
				if (pWorkingCity->AI_getBestBuild(pWorkingCity->getCityPlotIndex(pPlot)) != NO_BUILD)
				{
					return 0;
				}
				if (pPlot->getImprovementType() != NO_IMPROVEMENT)
				{
					CvImprovementInfo &kImprovementInfo = GC.getImprovementInfo(pPlot->getImprovementType());
					if (!kImprovementInfo.isActsAsCity())
					{
						return 0;
					}
				}
			}
		}
	}
	
	for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		CvPlot* pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), (DirectionTypes)iI);
		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->isCity(true))
			{
				return 0;
			}
		}
	}
	
	CvArea* pSecondWaterArea = pPlot->secondWaterArea();
	if (pSecondWaterArea == NULL)
	{
		return 0;
	}
	
	return 10 * std::min(0, pSecondWaterArea->getNumTiles() - 2);
}

//This returns a positive number equal approximately to the sum
//of the percentage values of each unit (there is no need to scale the output by iHappy)
//100 * iHappy means a high value.
int CvPlayerAI::AI_getHappinessWeight(int iHappy, int iExtraPop) const
{
	int iWorstHappy = 0;
	int iBestHappy = 0;
	int iTotalUnhappy = 0;
	int iTotalHappy = 0;
	int iLoop;
	CvCity* pLoopCity;
	int iCount = 0;
	
	if (0 == iHappy)
	{
		iHappy = 1;
	}
	int iValue = 0;
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		int iCityHappy = pLoopCity->happyLevel() - pLoopCity->unhappyLevel(iExtraPop);
		
		iCityHappy -= std::max(0, pLoopCity->getCommerceHappiness());
		
		//
		// Mongoose AICivicFix BEGIN
		//
		// -- thanks to Fuyu for the min(5) requirements
		//
		//int iCityHappy = pLoopCity->happyDifferenceWithoutCivics(iExtraPop);
		int iHappyNow  = std::min(5, iCityHappy);
		int iHappyThen = std::min(5, iCityHappy + iHappy);
		//
		// Mongoose AICivicFix END
		//
		
		//Integration
		int iTempValue = (((100 * iHappyThen - 10 * iHappyThen * iHappyThen)) - (100 * iHappyNow - 10 * iHappyNow * iHappyNow));
/************************************************************************************************/
/* Afforess	                  Start		 07/22/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		if (pLoopCity->isCapital() && getNoCapitalUnhappiness())
		{
			iTempValue /= 3;
		}
		if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_REVOLUTION))
		{
			iTempValue *= 3;
			iTempValue /= 2;
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		if (iHappy > 0)
		{
			//
			// Mongoose AICivicFix BEGIN
			//
			// -- thanks to Fuyu for persuading me to weight this based on population
			//
			iValue += std::max(0, iTempValue) * (pLoopCity->getPopulation() + 5);
			//
			// Mongoose AICivicFix END
			//
		}
		else
		{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/21/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* orginal bts code
			iValue += std::max(0, -iTempValue);
*/
			// Negative happy changes should produce a negative value, not the same value as positive
			//
			// Mongoose AICivicFix BEGIN
			//
			// -- thanks to Fuyu for persuading me to weight this based on population
			//
			iValue += std::min(0, iTempValue) * (pLoopCity->getPopulation() + 5);
			//
			// Mongoose AICivicFix END
			//
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
		}

		
		iCount++;
/************************************************************************************************/
/* Afforess	                  Start		 07/22/10                                               */
/*                                                                                              */
/*  Why Stop at 7 Cities?                                                                        */
/************************************************************************************************/
/*
		if (iCount > 6)
		{
			break;
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	}
	
	return (0 == iCount) ? 50 * iHappy : iValue / iCount;
}

int CvPlayerAI::AI_getHealthWeight(int iHealth, int iExtraPop) const
{
	int iWorstHealth = 0;
	int iBestHealth = 0;
	int iTotalUnhappy = 0;
	int iTotalHealth = 0;
	int iLoop;
	CvCity* pLoopCity;
	int iCount = 0;
	
	if (0 == iHealth)
	{
		iHealth = 1;
	}
	int iValue = 0;
	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		int iCityHealth = pLoopCity->goodHealth() - pLoopCity->badHealth(false, iExtraPop);
		
		int iHealthNow = iCityHealth;
		int iHealthThen = iCityHealth + iHealth;
		
		//Integration
		int iTempValue = (((100 * iHealthThen - 6 * iHealthThen * iHealthThen)) - (100 * iHealthNow - 6 * iHealthNow * iHealthNow));
		if (iHealth > 0)
		{
			iValue += std::max(0, iTempValue);
		}
		else
		{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/21/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* orginal bts code
			iValue += std::max(0, -iTempValue);
*/
			// Negative health changes should produce a negative value, not the same value as positive
			iValue += std::min(0, iTempValue);
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
		}
		iCount++;
		if (iCount > 6)
		{
			break;
		}
	}
	
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/21/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* orginal bts code
	return (0 == iCount) ? 50 : iValue / iCount;
*/
	// Mirror happiness valuation code
	return (0 == iCount) ? 50*iHealth : iValue / iCount;
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
}
	
void CvPlayerAI::AI_invalidateCloseBordersAttitudeCache()
{
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		m_aiCloseBordersAttitudeCache[i] = MAX_INT;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/03/09                       poyuzhe & jdog5000     */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
		// From Sanguo Mod Performance, ie the CAR Mod
		// Attitude cache
		AI_invalidateAttitudeCache((PlayerTypes)i);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	}
}

bool CvPlayerAI::AI_isPlotThreatened(CvPlot* pPlot, int iRange, bool bTestMoves) const
{
	PROFILE_FUNC();

	CvArea *pPlotArea = pPlot->area();

	if (iRange == -1)
	{
		iRange = DANGER_RANGE;
	}

	for (int iDX = -iRange; iDX <= iRange; iDX++)
	{
		for (int iDY = -iRange; iDY <= iRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->area() == pPlotArea)
				{
					for (CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode(); pUnitNode != NULL; pUnitNode = pLoopPlot->nextUnitNode(pUnitNode))
					{
						CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
						if (pLoopUnit->isEnemy(getTeam()) && pLoopUnit->canAttack() && !pLoopUnit->isInvisible(getTeam(), false))
						{
							if (pLoopUnit->canMoveOrAttackInto(pPlot))
							{
								int iPathTurns = 0;
								if (bTestMoves)
								{
									if (!pLoopUnit->getGroup()->generatePath(pLoopPlot, pPlot, MOVE_MAX_MOVES | MOVE_IGNORE_DANGER, false, &iPathTurns))
									{
										iPathTurns = MAX_INT;
									}
								}

								if (iPathTurns <= 1)
								{
									return true;
								}
							}
						}
					}
				}
			}
		}
	}

	return false;
}

bool CvPlayerAI::AI_isFirstTech(TechTypes eTech) const
{
	for (int iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (GC.getReligionInfo((ReligionTypes)iI).getTechPrereq() == eTech)
		{
			if (!(GC.getGameINLINE().isReligionSlotTaken((ReligionTypes)iI)))
			{
				return true;
			}
		}
	}

	if (GC.getGameINLINE().countKnownTechNumTeams(eTech) == 0)
	{
		if ((getTechFreeUnit(eTech) != NO_UNIT) ||
			(GC.getTechInfo(eTech).getFirstFreeTechs() > 0))
		{
			return true;
		}
	}

	return false;
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/03/09                       poyuzhe & jdog5000     */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
// From Sanguo Mod Performance, ie the CAR Mod
// Attitude cache
void CvPlayerAI::AI_invalidateAttitudeCache(PlayerTypes ePlayer)
{
	m_aiAttitudeCache[ePlayer] = MAX_INT;
}

void CvPlayerAI::AI_invalidateAttitudeCache()
{
	for( int iI = 0; iI < MAX_PLAYERS; iI++ )
	{
		AI_invalidateAttitudeCache((PlayerTypes)iI);
	}
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* RevDCM	                  Start		 5/1/09                                                 */
/*                                                                                              */
/* Inquisitions                                                                                 */
/************************************************************************************************/
CvCity* CvPlayerAI::getInquisitionRevoltCity(CvUnit *pUnit, bool bNoUnit, int iRevIndexThreshold, int iTrendThreshold)
{
	FAssert(pUnit != NULL);
	if(!(hasInquisitionTarget()))
	{
		return NULL;
	}

	CvCity* pLoopCity;
	CvCity* pBestCity = NULL;
	CvPlot* pUnitPlot = pUnit->plot();
	int iLoop;
	int iBestRevoltIndex = 100;
	int iTempCityValue = 0;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (pLoopCity->isInquisitionConditions())
		{
			if( (pLoopCity->getRevTrend() > iTrendThreshold)
			|| (pLoopCity->getRevolutionIndex() > 1000) )
			{
				iTempCityValue = pLoopCity->getRevolutionIndex() + 7*(pLoopCity->getRevTrend());
				iTempCityValue -= 10*(pUnitPlot->calculatePathDistanceToPlot(getTeam(), pLoopCity->plot()));
				if(iTempCityValue > iBestRevoltIndex)
				{
					if ((bNoUnit) || (pUnit->generatePath(pLoopCity->plot(), 0, false)))
					{
						iBestRevoltIndex = iTempCityValue;
						pBestCity = pLoopCity;
					}
				}
			}
		}
	}

	return pBestCity;
}

CvCity* CvPlayerAI::getTeamInquisitionRevoltCity(CvUnit *pUnit, bool bNoUnit, int iRevIndexThreshold, int iTrendThreshold)
{
	FAssert(pUnit != NULL);
	if(!(hasInquisitionTarget()))
	{
		return NULL;
	}

	CvCity* pLoopCity;
	CvCity* pBestCity = NULL;
	CvPlot* pUnitPlot = pUnit->plot();
	int iI, iLoop;
	int iBestRevoltIndex = 100;
	int iTempCityValue = 0;

	for(iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER(PlayerTypes(iI));
		if(kLoopPlayer.isAlive())
		{
			if( (kLoopPlayer.getTeam() == getTeam()) ||
			(GET_TEAM(getTeam()).isVassal((TeamTypes)kLoopPlayer.getTeam())) )
			{
				for (pLoopCity = kLoopPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kLoopPlayer.nextCity(&iLoop))
				{
					if(pLoopCity->isInquisitionConditions())
					{
						if( (pLoopCity->getRevTrend() > iTrendThreshold)
						|| (pLoopCity->getRevolutionIndex() > 1000) )
						{
							iTempCityValue = pLoopCity->getRevolutionIndex() + 7*(pLoopCity->getRevTrend());
							iTempCityValue -= 10*(pUnitPlot->calculatePathDistanceToPlot(getTeam(), pLoopCity->plot()));
							if(iTempCityValue > iBestRevoltIndex)
							{
								if ((bNoUnit) || (pUnit->generatePath(pLoopCity->plot(), 0, false)))
								{
									iBestRevoltIndex = iTempCityValue;
									pBestCity = pLoopCity;
								}
							}
						}
					}
				}
			}
		}
	}

	return pBestCity;
}

CvCity* CvPlayerAI::getReligiousVictoryTarget(CvUnit *pUnit, bool bNoUnit)
{
	FAssert(pUnit != NULL);
	if(!(hasInquisitionTarget()))
	{
		return NULL;
	}
	if( !(isPushReligiousVictory()) && !(isConsiderReligiousVictory()) )
	{
		return NULL;
	}

	CvCity* pLoopCity;
	CvCity* pBestCity = NULL;
	int iI, iJ, iLoop;
	int iBestCityValue = MAX_INT;
	int tempCityValue;
	CvPlot* pUnitPlot = pUnit->plot();
	CvPlot* pLoopPlot = NULL;
	CvTeam& pTeam = GET_TEAM(getTeam());
	ReligionTypes eStateReligion = getStateReligion();

	for(iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER(PlayerTypes(iI));
		CvTeam& kLoopTeam = GET_TEAM(kLoopPlayer.getTeam());
		if(kLoopPlayer.isAlive())
		{
			if( (TeamTypes(kLoopPlayer.getTeam()) == getTeam()) || kLoopTeam.isVassal((TeamTypes)kLoopPlayer.getTeam()) )
			{
				if(pUnitPlot->isHasPathToPlayerCity(getTeam(), PlayerTypes(iI)))
				{
					for(pLoopCity = kLoopPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kLoopPlayer.nextCity(&iLoop))
					{
						if(pLoopCity->isInquisitionConditions())
						{
							pLoopPlot = pLoopCity->plot();
							if ((bNoUnit) || (pUnit->generatePath(pLoopPlot, 0, false)))
							{	
								tempCityValue = pUnitPlot->calculatePathDistanceToPlot(getTeam(), pLoopPlot);
								if( isNonStateReligionCommerce()
								&& (kLoopPlayer.getID() == getID()) )
								{
									tempCityValue *= 2;
								}
								if(kLoopTeam.isVassal((TeamTypes)kLoopPlayer.getTeam()))
								{
									tempCityValue -= 12;
								}
								for(iJ = 0; iJ < GC.getNumReligionInfos(); iJ++)
								{
									if(ReligionTypes(iJ) != eStateReligion)
									{
										if(hasHolyCity(ReligionTypes(iJ)))
										{
											if(pLoopCity->isHasReligion(ReligionTypes(iJ)))
											{
												tempCityValue += 13;
											}
										}
									}
								}
								if(tempCityValue < iBestCityValue)
								{
									pBestCity = pLoopCity;
									iBestCityValue = tempCityValue;
								}
							}
						}
					}
				}
			}
		}
	}

	return pBestCity;
}


bool CvPlayerAI::isPushReligiousVictory() const
{
	return m_bPushReligiousVictory;
}

void CvPlayerAI::AI_setPushReligiousVictory()
{
	m_bPushReligiousVictory = false;
	if(getStateReligion() == NO_RELIGION)
	{
		return;
	}
	if(AI_getCultureVictoryStage() > 1)
	{
		return;
	}

	ReligionTypes eStateReligion = getStateReligion();
	int iStateReligionInfluence = GC.getGameINLINE().calculateReligionPercent(eStateReligion);
	int iI;
	int iVictoryTarget;
	CvTeamAI& pTeamAI = GET_TEAM(getTeam());

	if(!hasHolyCity(eStateReligion))
	{
		return;
	}
	// Better way to determine if religious victory is valid?
	bool bValid = false;
	for (iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		if (GC.getGameINLINE().isVictoryValid((VictoryTypes) iI))
		{
			CvVictoryInfo& kVictoryInfo = GC.getVictoryInfo((VictoryTypes) iI);
			if( kVictoryInfo.getReligionPercent() > 0 )
			{
				iVictoryTarget = kVictoryInfo.getReligionPercent();
				bValid = true;
				break;
			}
		}
	}
	if (!bValid)
	{
		m_bPushReligiousVictory = false;
		return;
	}

	if(iStateReligionInfluence > (3*iVictoryTarget) / 4)
	{
		m_bPushReligiousVictory = true;
		return;
	}

	bool bStateReligionBest = true;
	for(iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if(eStateReligion != (ReligionTypes)iI)
		{
			if(GC.getGameINLINE().calculateReligionPercent((ReligionTypes)iI) > iStateReligionInfluence)
			{
				bStateReligionBest = false;
				break;
			}
		}
	}
/************************************************************************************************/
/* Afforess	                  Start		 07/26/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	int iPercentThreshold = (2*iVictoryTarget) / 3;
	bool bHasHolyBuilding = false;
	if (getStateReligion() != NO_RELIGION)
	{
		if (GC.getGameINLINE().getHolyCity(getStateReligion()) != NULL && GC.getGameINLINE().getHolyCity(getStateReligion())->getOwner() == getID())
		{
			for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
			{
				if (GC.getBuildingInfo((BuildingTypes)iI).getGlobalReligionCommerce() == getStateReligion())
				{
					if (GC.getGameINLINE().getHolyCity(getStateReligion())->getNumActiveBuilding((BuildingTypes)iI) > 0)
					{
						bHasHolyBuilding = true;
						break;
					}
				}
			}
		}
	}
	if (bHasHolyBuilding)
	{
		iPercentThreshold /= 2;
	}

	if(bStateReligionBest)
	{
		if( (iStateReligionInfluence > iPercentThreshold) || (pTeamAI.getTotalLand(true) > 50) )
		{
			m_bPushReligiousVictory = true;
			return;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


}


bool CvPlayerAI::isConsiderReligiousVictory() const
{
	return m_bConsiderReligiousVictory;
}

void CvPlayerAI::AI_setConsiderReligiousVictory()
{
	if(isPushReligiousVictory())
	{
		m_bConsiderReligiousVictory = true;
		return;
	}

	m_bConsiderReligiousVictory = false;
	if(getStateReligion() == NO_RELIGION)
	{
		return;
	}
	if(AI_getCultureVictoryStage() > 1)
	{
		return;
	}

	ReligionTypes eStateReligion = getStateReligion();
	int iStateReligionInfluence = GC.getGameINLINE().calculateReligionPercent(eStateReligion);
	int iI;
	int iVictoryTarget;
	CvTeamAI& pTeamAI = GET_TEAM(getTeam());

	if(!hasHolyCity(eStateReligion))
	{
		return;
	}
	// Better way to determine if religious victory is valid?
	bool bValid = false;
	for (iI = 0; iI < GC.getNumVictoryInfos(); iI++)
	{
		if (GC.getGameINLINE().isVictoryValid((VictoryTypes) iI))
		{
			CvVictoryInfo& kVictoryInfo = GC.getVictoryInfo((VictoryTypes) iI);
			if( kVictoryInfo.getReligionPercent() > 0 )
			{
				iVictoryTarget = kVictoryInfo.getReligionPercent();
				bValid = true;
				break;
			}
		}
	}
	if (!bValid)
	{
		return;
	}

	if(iStateReligionInfluence > ((2*iVictoryTarget)/ 3))
	{
		m_bConsiderReligiousVictory = true;
		return;
	}

	bool eStateReligionBest = true;
	for(iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if(eStateReligion != (ReligionTypes)iI)
		{
			if(GC.getGameINLINE().calculateReligionPercent((ReligionTypes)iI) > iStateReligionInfluence)
			{
				eStateReligionBest = false;
				break;
			}
		}
	}
	if(eStateReligionBest)
	{
		m_bConsiderReligiousVictory = true;
		return;
	} else
	{
		if( (iStateReligionInfluence > (iVictoryTarget / 2)))
		{
			m_bConsiderReligiousVictory = true;
			return;
		}
	}

}


bool CvPlayerAI::hasInquisitionTarget() const
{
	return m_bHasInquisitionTarget;
}

void CvPlayerAI::AI_setHasInquisitionTarget()
{

	m_bHasInquisitionTarget = false;
	if(!(isInquisitionConditions()))
	{
		return;
	}

	CvCity* pLoopCity = NULL;
	int iI, iLoop;

	for(iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER(PlayerTypes(iI));
		if(kLoopPlayer.isAlive())
		{
			if( (kLoopPlayer.getTeam() == getTeam()) ||
			(GET_TEAM(getTeam()).isVassal((TeamTypes)kLoopPlayer.getTeam())) )
			{
				for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
				{
					if (pLoopCity->isInquisitionConditions())
					{
						if( (pLoopCity->getRevTrend() > 0) && (pLoopCity->getRevolutionIndex() > 500) )
						{
							m_bHasInquisitionTarget = true;
							return;
						}else if(pLoopCity->getRevolutionIndex() > 1000)
						{
							m_bHasInquisitionTarget = true;
							return;
						}
					}
				}
			}
		}
	}

	pLoopCity = NULL;
	if(isPushReligiousVictory() || isConsiderReligiousVictory())
	{
		for(iI = 0; iI < MAX_PLAYERS; iI++)
		{
			CvPlayer& kLoopPlayer = GET_PLAYER(PlayerTypes(iI));
			CvTeam& kLoopTeam = GET_TEAM(kLoopPlayer.getTeam());
			if(kLoopPlayer.isAlive())
			{
				if( (TeamTypes(kLoopPlayer.getTeam()) == getTeam()) || kLoopTeam.isVassal((TeamTypes)kLoopPlayer.getTeam()) )
				{
					for(pLoopCity = kLoopPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kLoopPlayer.nextCity(&iLoop))
					{
						if(pLoopCity->isInquisitionConditions())
						{
							m_bHasInquisitionTarget = true;
							return;
						}
					}
				}
			}
		}
	}

}

int CvPlayerAI::countCityReligionRevolts() const
{
	CvCity* pLoopCity = NULL;
	int iLoop;
	int iCount = 0;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (pLoopCity->isInquisitionConditions())
		{
			if( (pLoopCity->getRevTrend() > 0) && (pLoopCity->getRevolutionIndex() > 500) )
			{
				iCount++;
			}else if(pLoopCity->getRevolutionIndex() > 1000)
			{
				iCount++;
			}
		}
	}

	return iCount;
}
/************************************************************************************************/
/* Inquisitions	                     END                                                        */
/************************************************************************************************/
/************************************************************************************************/
/* Afforess	                  Start		 12/9/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/

int CvPlayerAI::AI_getEmbassyAttitude(PlayerTypes ePlayer) const
{
    int iAttitude;
	bool bVictim = ((GET_TEAM(getTeam()).AI_getMemoryCount((GET_PLAYER(ePlayer).getTeam()), MEMORY_DECLARED_WAR) == 0) && (GET_TEAM(getTeam()).isAtWar(GET_PLAYER(ePlayer).getTeam())));

    iAttitude = 0;

	if (GET_TEAM(getTeam()).isHasEmbassy(GET_PLAYER(ePlayer).getTeam()))
	{
		iAttitude = 1;
	}
	else if (((GET_TEAM(getTeam())).AI_getMemoryCount(GET_PLAYER(ePlayer).getTeam(), MEMORY_RECALLED_AMBASSADOR) > 0) && !bVictim)
	{
		iAttitude = -2;
	}

    return iAttitude;
}

int CvPlayerAI::AI_workerTradeVal(CvUnit* pUnit) const
{
	int iValue = 0;
	int iLoop;
	int iProductionCost = (GC.getUnitInfo(pUnit->getUnitType()).getProductionCost() > 0) ? GC.getUnitInfo(pUnit->getUnitType()).getProductionCost() : 500;
	CvArea* pLoopArea;

	int iNeededWorkers = 0;
	
	if (!(GC.getUnitInfo(pUnit->getUnitType()).isWorkerTrade()))
	{//It's not a worker, so it's worthless
		return 0;
	}

	for (pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
	{
		if (pLoopArea->getCitiesPerPlayer(getID()) > 0)
		{
			iNeededWorkers += AI_neededWorkers(pLoopArea);
		}
	}

	if (iNeededWorkers > 1)
	{
		iValue = (iNeededWorkers * iProductionCost);
		iValue *= 2;
		iValue /= 3;
	}
	else
	{
		iValue = (iProductionCost * 7) / 8;
	}

	return iValue;
}

int CvPlayerAI::AI_militaryUnitTradeVal(CvUnit* pUnit) const
{
	int iValue;
	UnitTypes eUnit = pUnit->getUnitType();

	if (GC.getUnitInfo(eUnit).getProductionCost() > 0)
	{
		iValue = GC.getUnitInfo(eUnit).getProductionCost();
	}
	else
	{
		iValue = 200;
	}

	iValue += AI_unitValue(eUnit, (UnitAITypes)GC.getUnitInfo(eUnit).getDefaultUnitAIType(), getCapitalCity()->area());
	GC.getGameINLINE().logMsg("AI Unit Value For %s is %d", GC.getUnitInfo(eUnit).getDescription(), iValue);
	return iValue;
}

int CvPlayerAI::AI_pledgeVoteTradeVal(VoteTriggeredData* kData, PlayerVoteTypes ePlayerVote, PlayerTypes ePlayer) const
{
	return 1;
}

int CvPlayerAI::AI_corporationTradeVal(CorporationTypes eCorporation, PlayerTypes ePlayer) const
{
	int iValue = 100;
	CvCity* pLoopCity;
	int iLoop;

	for (pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if (pLoopCity->isHasCorporation(eCorporation))
		{
			iValue += AI_corporationValue(eCorporation, pLoopCity) / 50;
		}
		//if we can spread it to another city...
		else
		{
			for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
			{
				BonusTypes eBonus = (BonusTypes)GC.getCorporationInfo(eCorporation).getPrereqBonus(i);
				if (NO_BONUS != eBonus)
				{
					if (pLoopCity->hasBonus(eBonus))
					{
						iValue += 5;
					}
				}
			}
		}
	}

	iValue *= getNumCities();

	return iValue;
}

int CvPlayerAI::AI_secretaryGeneralTradeVal(VoteSourceTypes eVoteSource, PlayerTypes ePlayer) const
{
	TeamTypes eBestTeam;
	int iValue = 0;
	int iAttitude;
	int iBestAttitude;
	int aiVotes[MAX_TEAMS];

	for (int i = 0; i < MAX_TEAMS; i++)
	{
		aiVotes[i] = 0;
	}

	for (int iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (GET_TEAM((TeamTypes)iI).isVotingMember(eVoteSource) && !(getTeam() == iI))
			{
				for (int iJ = 0; iJ < MAX_TEAMS; iJ++)
				{
					iBestAttitude = 0;
					if (iI != iJ)
					{
						if (GET_TEAM((TeamTypes)iJ).isAlive())
						{
							if (GC.getGameINLINE().isTeamVoteEligible((TeamTypes)iJ, eVoteSource))
							{
								if (GET_TEAM((TeamTypes)iI).isVassal((TeamTypes)iJ))
								{
									aiVotes[iJ] += GET_TEAM((TeamTypes)iI).getVotes(NO_VOTE, eVoteSource);
								}

								iAttitude = GET_TEAM((TeamTypes)iI).AI_getAttitudeVal((TeamTypes)iJ);

								if (iAttitude > iBestAttitude)
								{
									iBestAttitude = iAttitude;
									eBestTeam = (TeamTypes)iJ;
								}
							}
						}
					}
				}
				aiVotes[eBestTeam] += GET_TEAM((TeamTypes)iI).getVotes(NO_VOTE, eVoteSource);
			}
		}
	}

	int iMostVotes = 0;
	TeamTypes eLikelyWinner = NO_TEAM;

	for (int iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (aiVotes[iI] > iMostVotes)
		{
			iMostVotes = aiVotes[iI];
			eLikelyWinner = (TeamTypes)iI;
		}
	}

	bool bKingMaker = false;
	int iOurVotes = 0;
	int iTheirVotes = 0;

	if (eLikelyWinner != GET_PLAYER(ePlayer).getTeam())
	{
		iOurVotes = GET_TEAM(getTeam()).getVotes(NO_VOTE, eVoteSource);
		iTheirVotes = aiVotes[GET_PLAYER(ePlayer).getTeam()];

		if ((iOurVotes + iTheirVotes) > iMostVotes)
		{
			bKingMaker = true;
		}
		
		int iOurSharedCivics = 0;
		int iTheirSharedCivics = 0;
		for (iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
		{
			if (getCivics((CivicOptionTypes)iI) == GET_PLAYER(ePlayer).getCivics((CivicOptionTypes)iI))
			{
				iOurSharedCivics++;
			}

			if (getCivics((CivicOptionTypes)iI) == GET_PLAYER(GET_TEAM(eLikelyWinner).getLeaderID()).getCivics((CivicOptionTypes)iI))
			{
				iTheirSharedCivics++;
			}
		}

		iValue *= std::max(1, iOurSharedCivics);
		iValue /= std::max(1, iTheirSharedCivics);



	}
	else
	{
		bKingMaker = false;
	}

	if (bKingMaker)
	{
		int iExtraVotes = ((iOurVotes + iTheirVotes) - iMostVotes);

		iValue *= iExtraVotes;
	}

	iValue *= 2;
	return iValue;
}

bool CvPlayerAI::containFixedBorders(CivicTypes* paeCivics) const
{
	int iI;

	if (!GC.getGameINLINE().isOption(GAMEOPTION_FIXED_BORDERS))
	{
		return false;
	}

	for (iI = 0; iI < GC.getNumCivicOptionInfos(); ++iI)
	{
		if (GC.getCivicInfo(paeCivics[iI]).IsFixedBorders())
		{
			return true;
		}
	}
	
	return false;
}

int CvPlayerAI::AI_getCivicAttitudeChange(PlayerTypes ePlayer) const
{
    int iAttitude = 0;
    
	for (int iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
	{
		CvCivicInfo& kCivicOption = GC.getCivicInfo(getCivics((CivicOptionTypes)iI));
		
		for (int iJ = 0; iJ < GC.getNumCivicOptionInfos(); iJ++)
		{
			iAttitude += kCivicOption.getCivicAttitudeChange(GET_PLAYER(ePlayer).getCivics((CivicOptionTypes)iJ));
		}
	}
    
    return iAttitude;
}

bool CvPlayerAI::AI_avoidIncreasingTaxes() const
{
	
    if ((getTaxationAnger() > 0) && !isGoldenAge())
    {
        return true;
    }

	return false;
}

int CvPlayerAI::AI_getCivicShareAttitude(PlayerTypes ePlayer) const
{
	int iAttitude = 0;
    for (int iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
    {
        if (getCivics((CivicOptionTypes)iI) == GET_PLAYER(ePlayer).getCivics((CivicOptionTypes)iI))
        {
            iAttitude += GC.getCivicInfo((CivicTypes)getCivics((CivicOptionTypes)iI)).getAttitudeShareMod();
        }
    }
	return iAttitude;
}
/************************************************************************************************/
/* Afforess	                  Start		 06/16/10                                               */
/*                                                                                              */
/* Advanced Diplomacy                                                                           */
/************************************************************************************************/
TeamTypes CvPlayerAI::AI_bestJoinWarTeam(PlayerTypes ePlayer)
{
	int iValue = 0;
	int iBestValue = 0;
	TeamTypes eWarTeam = NO_TEAM;

	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (atWar(GET_PLAYER((PlayerTypes)iI).getTeam(), getTeam()))
			{
				if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).canDeclareWar(GET_PLAYER((PlayerTypes)iI).getTeam()))
				{
					if (!GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isVassal(GET_PLAYER(ePlayer).getTeam()) && !GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).isDefensivePact(GET_PLAYER(ePlayer).getTeam()))
					{
						if (GET_PLAYER(ePlayer).isHuman() || 
							(GET_PLAYER(ePlayer).AI_getAttitude(getID()) > GC.getLeaderHeadInfo(GET_PLAYER(ePlayer).getPersonalityType()).getDeclareWarRefuseAttitudeThreshold()))
						{
							if (GET_PLAYER(ePlayer).isHuman() || (GET_PLAYER(ePlayer).AI_getAttitude((PlayerTypes)iI) < ATTITUDE_CAUTIOUS))
							{
								iValue = GET_TEAM(getTeam()).AI_declareWarTradeVal(GET_PLAYER((PlayerTypes)iI).getTeam(), GET_PLAYER(ePlayer).getTeam());
								if (iBestValue < iValue)
								{
									iBestValue = iValue;
									eWarTeam = GET_PLAYER((PlayerTypes)iI).getTeam();
								}
							}
						}
					}
				}
			}
		}
	}
	return eWarTeam;
}

TeamTypes CvPlayerAI::AI_bestMakePeaceTeam(PlayerTypes ePlayer)
{
	int iOurPower = GET_TEAM(getTeam()).getPower(true);
	int iTheirPower = GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getPower(true);
	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if ((iI != ePlayer) && (iI != getID()))
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (!GET_PLAYER((PlayerTypes)iI).isMinorCiv())
				{
					if (GET_TEAM(getTeam()).isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))
					{
						if (atWar(GET_PLAYER((PlayerTypes)iI).getTeam(), GET_PLAYER(ePlayer).getTeam()) && !atWar(getTeam(), GET_PLAYER((PlayerTypes)iI).getTeam()))
						{
							if (GET_PLAYER((PlayerTypes)iI).AI_isWillingToTalk(ePlayer))
							{
								if ((AI_getAttitude((PlayerTypes)iI) >= ATTITUDE_FRIENDLY)
									|| ((iOurPower < iTheirPower) && (iTheirPower > (GET_TEAM(GET_PLAYER((PlayerTypes)iI).getTeam()).getPower(true) * 3))))
								{
									return (TeamTypes)GET_PLAYER((PlayerTypes)iI).getTeam();
								}
							}
						}
					}
				}
			}
		}
	}
	return NO_TEAM;
}

TeamTypes CvPlayerAI::AI_bestStopTradeTeam(PlayerTypes ePlayer)
{
	int iValue;
	int iBestValue = 0;
	TeamTypes eBestTeam = NO_TEAM;

	TeamTypes eWorstEnemy = GET_TEAM(getTeam()).AI_getWorstEnemy();

	for (int iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		TeamTypes eTeam = ((TeamTypes)iI);
		bool bAtWar = atWar(eTeam, getTeam());
		if ((getTeam() != eTeam) && (eTeam != GET_PLAYER(ePlayer).getTeam()))
		{
			if ((eWorstEnemy == eTeam) || bAtWar || 
				((GET_TEAM(getTeam()).AI_getAttitude(eTeam) == ATTITUDE_FURIOUS) || (GET_TEAM(getTeam()).AI_getAttitude(eTeam) == ATTITUDE_ANNOYED)))
			{
				if (GET_PLAYER(ePlayer).getTeam() != eTeam)
				{
					if (GET_TEAM(eTeam).isAlive())
					{
						if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasMet(eTeam))
						{
							if (!atWar(GET_PLAYER(ePlayer).getTeam(), eTeam))
							{
								if (GET_TEAM(eTeam).isVassal(GET_PLAYER(ePlayer).getTeam()))
								{
									if (GET_PLAYER(ePlayer).canStopTradingWithTeam(eTeam))
									{
										if (GET_PLAYER(ePlayer).AI_stopTradingTrade(eTeam, getID()) == NO_DENIAL)
										{
											if ((bAtWar && GET_PLAYER(ePlayer).isTradingMilitaryBonus(GET_TEAM(eTeam).getLeaderID()))
												|| !(GET_TEAM(GET_PLAYER(ePlayer).getTeam()).AI_getAttitude(eTeam) == ATTITUDE_FRIENDLY))
											{
												iValue = 0;

												iValue = AI_stopTradingTradeVal(eTeam, ePlayer);
												if ((iBestValue == 0) || (iValue < iBestValue))
												{
													iBestValue = iValue;
													eBestTeam = eTeam;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return eBestTeam;
}

int CvPlayerAI::AI_militaryBonusVal(BonusTypes eBonus)
{
	int iValue = 0;
	int iHasOrBonusCount;
	bool bFound = false;
	UnitTypes eUnit = NO_UNIT;

	for (int iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
	{
		eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI);

		if (canTrain(eUnit))
		{
			if (GC.getUnitInfo(eUnit).getPrereqAndBonus() == eBonus)
			{
				iValue += 1000;
			}

			iHasOrBonusCount = 0;

			bFound = false;

			for (iI = 0; iI < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); ++iI)
			{
				if (GC.getUnitInfo(eUnit).getPrereqOrBonuses(iI) == eBonus)
				{
					bFound = true;
				}
				else if (hasBonus((BonusTypes)GC.getUnitInfo(eUnit).getPrereqOrBonuses(iI)))
				{
					iHasOrBonusCount++;
				}
			}

			if (bFound)
			{
				iValue += 300;
				iValue /= iHasOrBonusCount;
			}
		}
	}
	return iValue;
}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


//Slightly altered form of CvUnitAI::AI_promotionValue()
int CvPlayerAI::AI_promotionValue(PromotionTypes ePromotion, UnitTypes eUnit, const CvUnit* pUnit, UnitAITypes eUnitAI) const
{
	int iValue;
	int iTemp;
	int iExtra;
	int iI;

	iValue = 0;
	
	CvPromotionInfo &kPromotion = GC.getPromotionInfo(ePromotion);
	CvUnitInfo &kUnit = GC.getUnitInfo(eUnit);
	int iMoves;
	if (pUnit == NULL)
	{
		iMoves = kUnit.getMoves();
	}
	else
	{
		iMoves = pUnit->baseMoves();
	}
	if (eUnitAI == NO_UNITAI)
	{
		eUnitAI = (UnitAITypes)kUnit.getDefaultUnitAIType();
	}
/************************************************************************************************/
/* SUPER SPIES                            05/24/08                                TSheep        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	//TSHEEP Setup AI values for promotions
	if(kUnit.isSpy())
	{
	/*********************************************************************************************/
	/* REVOLUTIONDCM				24/09/09						glider1						 */
	/**																							 */
	/*********************************************************************************************/
	//Readjust promotion choices favouring security, deception, logistics, escape, improvise,
	//filling in other promotions very lightly because the AI does not yet have situational awareness
	//when using spy promotions at the moment of mission execution.

		//Logistics
		//I & III
		iValue += (kPromotion.getMovesChange()*20);
		//II
		if (kPromotion.isEnemyRoute()) iValue += 20;
		iValue += (kPromotion.getMoveDiscountChange()*10);
		//total 20, 30, 20 points

		//Deception
		if (kPromotion.getEvasionChange())
		{
			//Lean towards more deception if deception is already present
			iValue += ((kPromotion.getEvasionChange() * 2) + pUnit == NULL ? 0 : pUnit->evasionProbability());
		}//total 20, 30, 40 points

		//Security
		iValue += (kPromotion.getVisibilityChange()*10);
		//Lean towards more security if security is already present
		iValue += (kPromotion.getInterceptChange() +  pUnit == NULL ? kUnit.getInterceptionProbability() : pUnit->currInterceptionProbability());
		//total 20, 30, 40 points

		//Escape
		if (kPromotion.getWithdrawalChange())
		{
			iValue += 30;
		}

		//Improvise
		if (kPromotion.getUpgradeDiscount())
		{
			iValue += 20;
		}
		
		//Loyalty
		if (kPromotion.isAlwaysHeal()) 
		{
			iValue += 15;
		}

		//Instigator
		//I & II
		if (kPromotion.getEnemyHealChange()) 
		{
			iValue += 15;
		}
		//III
		if (kPromotion.getNeutralHealChange())
		{
			iValue += 15;
		}

		//Alchemist
		if (kPromotion.getFriendlyHealChange())
		{
			iValue += 15;
		}

		if (iValue > 0)
		{
			iValue += GC.getGameINLINE().getSorenRandNum(15, "AI Promote");
		}

		return iValue;
	/****************************************************************************************/
	/* REVOLUTIONDCM				END      						glider1                 */
	/****************************************************************************************/
	}
/********************************************************************************************/
/* SUPER SPIES                     END                             TSheep                  */
/********************************************************************************************/

	if (kPromotion.isLeader())
	{
		// Don't consume the leader as a regular promotion
		return 0;
	}

	if (kPromotion.isBlitz())
	{
		if ((eUnitAI == UNITAI_RESERVE  && (iMoves > 1) || 
			eUnitAI == UNITAI_PARADROP))
		{
			iValue += 10;
		}
		else
		{
			iValue += 2;
		}
	}

	if (kPromotion.isAmphib())
	{
		if ((eUnitAI == UNITAI_ATTACK) ||
			  (eUnitAI == UNITAI_ATTACK_CITY))
		{
			iValue += 5;
		}
		else
		{
			iValue++;
		}
	}

	if (kPromotion.isRiver())
	{
		if ((eUnitAI == UNITAI_ATTACK) ||
			  (eUnitAI == UNITAI_ATTACK_CITY))
		{
			iValue += 5;
		}
		else
		{
			iValue++;
		}
	}

	if (kPromotion.isEnemyRoute())
	{
		if (eUnitAI == UNITAI_PILLAGE)
		{
			iValue += 40;
		}
		else if ((eUnitAI == UNITAI_ATTACK) ||
			       (eUnitAI == UNITAI_ATTACK_CITY))
		{
			iValue += 20;
		}
		else if (eUnitAI == UNITAI_PARADROP)
		{
			iValue += 10;
		}
		else
		{
			iValue += 4;
		}
	}

	if (kPromotion.isAlwaysHeal())
	{
		if ((eUnitAI == UNITAI_ATTACK) ||
			  (eUnitAI == UNITAI_ATTACK_CITY) ||
				(eUnitAI == UNITAI_PILLAGE) ||
				(eUnitAI == UNITAI_COUNTER) ||
				(eUnitAI == UNITAI_ATTACK_SEA) ||
				(eUnitAI == UNITAI_PIRATE_SEA) ||
				(eUnitAI == UNITAI_ESCORT_SEA) ||
				(eUnitAI == UNITAI_PARADROP))
		{
			iValue += 10;
		}
		else
		{
			iValue += 8;
		}
	}

	if (kPromotion.isHillsDoubleMove())
	{
		if (eUnitAI == UNITAI_EXPLORE)
		{
			iValue += 20;
		}
		else
		{
			iValue += 10;
		}
	}
/************************************************************************************************/
/* Afforess	                  Start		 06/04/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (kPromotion.isCanMovePeaks())
	{
		if (GC.getGameINLINE().isOption(GAMEOPTION_MOUNTAINS))
		{
			for (iI = 0; iI < GC.getNumTechInfos(); iI++)
			{
				if (!(GET_TEAM(getTeam()).isHasTech((TechTypes)iI)))
				{
					if (GC.getTechInfo((TechTypes)iI).isCanPassPeaks())
					{
						iValue += 75;
					}
				}
			}
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/	


	if (kPromotion.isImmuneToFirstStrikes() 
		&& (pUnit == NULL || !pUnit->immuneToFirstStrikes()))
	{
		if ((eUnitAI == UNITAI_ATTACK_CITY))
		{
			iValue += 12;			
		}
		else if ((eUnitAI == UNITAI_ATTACK))
		{
			iValue += 8;
		}
		else
		{
			iValue += 4;
		}
	}

	iTemp = kPromotion.getVisibilityChange();
	if ((eUnitAI == UNITAI_EXPLORE_SEA) || 
		(eUnitAI == UNITAI_EXPLORE))
	{
		iValue += (iTemp * 40);
	}
	else if (eUnitAI == UNITAI_PIRATE_SEA) 
	{
		iValue += (iTemp * 20);
	}

	iTemp = kPromotion.getMovesChange();
	if ((eUnitAI == UNITAI_ATTACK_SEA) ||
		(eUnitAI == UNITAI_PIRATE_SEA) ||
		  (eUnitAI == UNITAI_RESERVE_SEA) ||
		  (eUnitAI == UNITAI_ESCORT_SEA) ||
			(eUnitAI == UNITAI_EXPLORE_SEA) ||
			(eUnitAI == UNITAI_ASSAULT_SEA) ||
			(eUnitAI == UNITAI_SETTLER_SEA) ||
			(eUnitAI == UNITAI_PILLAGE) ||
			(eUnitAI == UNITAI_ATTACK) ||
			(eUnitAI == UNITAI_PARADROP))
	{
		iValue += (iTemp * 20);
	}
	else
	{
		iValue += (iTemp * 4);
	}

	iTemp = kPromotion.getMoveDiscountChange();
	if (eUnitAI == UNITAI_PILLAGE)
	{
		iValue += (iTemp * 10);
	}
	else
	{
		iValue += (iTemp * 2);
	}

	iTemp = kPromotion.getAirRangeChange();
	if (eUnitAI == UNITAI_ATTACK_AIR ||
		eUnitAI == UNITAI_CARRIER_AIR)
	{
		iValue += (iTemp * 20);
	}
	else if (eUnitAI == UNITAI_DEFENSE_AIR)
	{
		iValue += (iTemp * 10);
	}

	iTemp = kPromotion.getInterceptChange();
	if (eUnitAI == UNITAI_DEFENSE_AIR)
	{
		iValue += (iTemp * 3);
	}
	else if (eUnitAI == UNITAI_CITY_SPECIAL || eUnitAI == UNITAI_CARRIER_AIR)
	{
		iValue += (iTemp * 2);
	}
	else
	{
		iValue += (iTemp / 10);
	}

	iTemp = kPromotion.getEvasionChange();
	if (eUnitAI == UNITAI_ATTACK_AIR || eUnitAI == UNITAI_CARRIER_AIR)
	{
		iValue += (iTemp * 3);
	}
	else
	{
		iValue += (iTemp / 10);
	}

	iTemp = kPromotion.getFirstStrikesChange() * 2;
	iTemp += kPromotion.getChanceFirstStrikesChange();
	if ((eUnitAI == UNITAI_RESERVE) ||
		  (eUnitAI == UNITAI_COUNTER) ||
			(eUnitAI == UNITAI_CITY_DEFENSE) ||
			(eUnitAI == UNITAI_CITY_COUNTER) ||
			(eUnitAI == UNITAI_CITY_SPECIAL) ||
			(eUnitAI == UNITAI_ATTACK))
	{
		iTemp *= 8;
		iExtra = pUnit == NULL ? kUnit.getChanceFirstStrikes() + kUnit.getFirstStrikes() * 2 : pUnit->getExtraChanceFirstStrikes() + pUnit->getExtraFirstStrikes() * 2;
		iTemp *= 100 + iExtra * 15;
		iTemp /= 100;
		iValue += iTemp;
	}
	else
	{
		iValue += (iTemp * 5);
	}


	iTemp = kPromotion.getWithdrawalChange();
	if (iTemp != 0)
	{
		iExtra = (kUnit.getWithdrawalProbability() + (pUnit == NULL ? 0 : pUnit->getExtraWithdrawal() * 4));
		iTemp *= (100 + iExtra);
		iTemp /= 100;
		if ((eUnitAI == UNITAI_ATTACK_CITY))
		{
			iValue += (iTemp * 4) / 3;
		}
		else if ((eUnitAI == UNITAI_COLLATERAL) ||
			  (eUnitAI == UNITAI_RESERVE) ||
			  (eUnitAI == UNITAI_RESERVE_SEA) ||
			  (pUnit != NULL && pUnit->getLeaderUnitType() != NO_UNIT))
		{
			iValue += iTemp * 1;
		}
		else
		{
			iValue += (iTemp / 4);
		}
	}

	iTemp = kPromotion.getCollateralDamageChange();
	if (iTemp != 0)
	{
		iExtra = pUnit == NULL ? kUnit.getCollateralDamage() : pUnit->getExtraCollateralDamage(); //collateral has no strong synergy (not like retreat)
		iTemp *= (100 + iExtra);
		iTemp /= 100;
		
		if (eUnitAI == UNITAI_COLLATERAL)
		{
			iValue += (iTemp * 1);
		}
		else if (eUnitAI == UNITAI_ATTACK_CITY)
		{
			iValue += ((iTemp * 2) / 3);
		}
		else
		{
			iValue += (iTemp / 8);
		}
	}

	iTemp = kPromotion.getBombardRateChange();
	if (eUnitAI == UNITAI_ATTACK_CITY)
	{
		iValue += (iTemp * 2);
	}
	else
	{
		iValue += (iTemp / 8);
	}

	iTemp = kPromotion.getEnemyHealChange();	
	if ((eUnitAI == UNITAI_ATTACK) ||
		(eUnitAI == UNITAI_ATTACK_SEA) ||
		(eUnitAI == UNITAI_PARADROP) ||
		(eUnitAI == UNITAI_PIRATE_SEA))
	{
		iValue += (iTemp / 4);
	}
	else
	{
		iValue += (iTemp / 8);
	}

	iTemp = kPromotion.getNeutralHealChange();
	iValue += (iTemp / 8);

	iTemp = kPromotion.getFriendlyHealChange();
	if ((eUnitAI == UNITAI_CITY_DEFENSE) ||
		  (eUnitAI == UNITAI_CITY_COUNTER) ||
		  (eUnitAI == UNITAI_CITY_SPECIAL))
	{
		iValue += (iTemp / 4);
	}
	else
	{
		iValue += (iTemp / 8);
	}


	if ( pUnit != NULL && ( pUnit->getDamage() > 0 || ((eUnitAI == UNITAI_COUNTER || 
															eUnitAI == UNITAI_PILLAGE ||
															eUnitAI == UNITAI_ATTACK_CITY ||
															eUnitAI == UNITAI_RESERVE )) ))
    {
        iTemp = kPromotion.getSameTileHealChange() + pUnit->getSameTileHeal();
        iExtra = pUnit->getSameTileHeal();
        
        iTemp *= (100 + iExtra * 5);
        iTemp /= 100;
        
        if (iTemp > 0)
        {
            if (pUnit->healRate(pUnit->plot()) < iTemp)
            {
                iValue += iTemp * ((pUnit->getGroup()->getNumUnits() > 4) ? 4 : 2);
            }
            else
            {
                iValue += (iTemp / 8);
            }
        }

        iTemp = kPromotion.getAdjacentTileHealChange();
        iExtra = pUnit->getAdjacentTileHeal();
        iTemp *= (100 + iExtra * 5);
        iTemp /= 100;
        if (pUnit->getSameTileHeal() >= iTemp)
        {
            iValue += (iTemp * ((pUnit->getGroup()->getNumUnits() > 9) ? 4 : 2));
        }
        else
        {
            iValue += (iTemp / 4);
        }
    }

	// try to use Warlords to create super-medic units
	if (pUnit != NULL && (kPromotion.getAdjacentTileHealChange() > 0 || kPromotion.getSameTileHealChange() > 0))
	{
		PromotionTypes eLeader = NO_PROMOTION;
		for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
		{
			if (GC.getPromotionInfo((PromotionTypes)iI).isLeader())
			{
				eLeader = (PromotionTypes)iI;
			}
		}
		
		if (eLeader != NO_PROMOTION && pUnit->isHasPromotion(eLeader))
		{
			iValue += kPromotion.getAdjacentTileHealChange() + kPromotion.getSameTileHealChange();
		}
	}
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	//@MOD Commanders: command range promotion AI value
	iTemp = kPromotion.getCommandRange();
	iValue += iTemp * 2;
	//end mod

	//@MOD Commanders: control points promotion AI value
	iTemp = kPromotion.getControlPoints();
	if (pUnit != NULL && pUnit->plot()->getNumUnits() > pUnit->controlPoints())
	{
		iValue += iTemp * 100;
	}
	//end mod

	iTemp = kPromotion.getCombatPercent();
	//@MOD Commanders: combat promotion value
	if (eUnitAI == UNITAI_GENERAL)
	{
		iValue += (iTemp * 3);
	}
	else 
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	if ((eUnitAI == UNITAI_ATTACK) ||
		(eUnitAI == UNITAI_COUNTER) ||
		(eUnitAI == UNITAI_CITY_COUNTER) ||
		  (eUnitAI == UNITAI_ATTACK_SEA) ||
		  (eUnitAI == UNITAI_RESERVE_SEA) ||
			(eUnitAI == UNITAI_ATTACK_SEA) ||
			(eUnitAI == UNITAI_PARADROP) ||
			(eUnitAI == UNITAI_PIRATE_SEA) ||
			(eUnitAI == UNITAI_RESERVE_SEA) ||
			(eUnitAI == UNITAI_ESCORT_SEA) ||
			(eUnitAI == UNITAI_CARRIER_SEA) ||
			(eUnitAI == UNITAI_ATTACK_AIR) ||
			(eUnitAI == UNITAI_CARRIER_AIR))
	{
		iValue += (iTemp * 2);
	}
	else
	{
		iValue += (iTemp * 1);
	}

	iTemp = kPromotion.getCityAttackPercent();
	if (iTemp != 0)
	{
		if (kUnit.getUnitAIType(UNITAI_ATTACK) || kUnit.getUnitAIType(UNITAI_ATTACK_CITY) || kUnit.getUnitAIType(UNITAI_ATTACK_CITY_LEMMING))
		{
			iExtra = (kUnit.getCityAttackModifier() + (pUnit == NULL ? 0 : pUnit->getExtraCityAttackPercent() * 2));
			iTemp *= (100 + iExtra);
			iTemp /= 100;
			if (eUnitAI == UNITAI_ATTACK_CITY)
			{
				iValue += (iTemp * 1);
			}
			else 
			{
				iValue -= iTemp / 4;
			}
		}
	}

	iTemp = kPromotion.getCityDefensePercent();
	if (iTemp != 0)
	{
		if ((eUnitAI == UNITAI_CITY_DEFENSE) ||
			  (eUnitAI == UNITAI_CITY_SPECIAL))
		{
			iExtra = kUnit.getCityDefenseModifier() + (pUnit == NULL ? 0 : pUnit->getExtraCityDefensePercent() * 2);
			iValue += ((iTemp * (100 + iExtra)) / 100);
		}
		else
		{
			iValue += (iTemp / 4);
		}
	}

	iTemp = kPromotion.getHillsAttackPercent();
	if (iTemp != 0)
	{
		iExtra = pUnit == NULL ? kUnit.getHillsAttackModifier() : pUnit->getExtraHillsAttackPercent();
		iTemp *= (100 + iExtra * 2);
		iTemp /= 100;
		if ((eUnitAI == UNITAI_ATTACK) ||
			(eUnitAI == UNITAI_COUNTER))
		{
			iValue += (iTemp / 4);
		}
		else
		{
			iValue += (iTemp / 16);
		}
	}

	iTemp = kPromotion.getHillsDefensePercent();
	if (iTemp != 0)
	{
		iExtra = (kUnit.getHillsDefenseModifier() + (pUnit == NULL ? 0 : pUnit->getExtraHillsDefensePercent() * 2)); 
		iTemp *= (100 + iExtra);
		iTemp /= 100;
		if (eUnitAI == UNITAI_CITY_DEFENSE)
		{
			if (pUnit != NULL && pUnit->plot()->isCity() && pUnit->plot()->isHills())
			{
				iValue += (iTemp * 4) / 3;
			}
		}
		else if (eUnitAI == UNITAI_COUNTER)
		{
			if (pUnit != NULL && pUnit->plot()->isHills())
			{
				iValue += (iTemp / 4);
			}
			else
			{
				iValue++;
			}
		}
		else
		{
			iValue += (iTemp / 16);
		}
	}

	iTemp = kPromotion.getRevoltProtection();
	if ((eUnitAI == UNITAI_CITY_DEFENSE) ||
		(eUnitAI == UNITAI_CITY_COUNTER) ||
		(eUnitAI == UNITAI_CITY_SPECIAL))
	{
		if (pUnit != NULL && iTemp > 0)
		{
			PlayerTypes eOwner = pUnit->plot()->calculateCulturalOwner();
			if (eOwner != NO_PLAYER && GET_PLAYER(eOwner).getTeam() != getTeam())
			{
				iValue += (iTemp / 2);
			}
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			if (pUnit->plot()->isCity())
			{
				iValue += pUnit->plot()->getPlotCity()->getRevolutionIndex() * iTemp / 1000;
			}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		}
	}

	iTemp = kPromotion.getCollateralDamageProtection();
	if ((eUnitAI == UNITAI_CITY_DEFENSE) ||
		(eUnitAI == UNITAI_CITY_COUNTER) ||
		(eUnitAI == UNITAI_CITY_SPECIAL))
	{
		iValue += (iTemp / 3);
	}
	else if ((eUnitAI == UNITAI_ATTACK) ||
		(eUnitAI == UNITAI_COUNTER))
	{
		iValue += (iTemp / 4);
	}
	else
	{
		iValue += (iTemp / 8);
	}

	iTemp = kPromotion.getPillageChange();
	if (eUnitAI == UNITAI_PILLAGE ||
		eUnitAI == UNITAI_ATTACK_SEA ||
		eUnitAI == UNITAI_PIRATE_SEA)
	{
		iValue += (iTemp / 4);
	}
	else
	{
		iValue += (iTemp / 16);
	}

	iTemp = kPromotion.getUpgradeDiscount();
	iValue += (iTemp / 16);

	iTemp = kPromotion.getExperiencePercent();
	if ((eUnitAI == UNITAI_ATTACK) ||
		(eUnitAI == UNITAI_ATTACK_SEA) ||
		(eUnitAI == UNITAI_PIRATE_SEA) ||
		(eUnitAI == UNITAI_RESERVE_SEA) ||
		(eUnitAI == UNITAI_ESCORT_SEA) ||
		(eUnitAI == UNITAI_CARRIER_SEA) ||
		(eUnitAI == UNITAI_MISSILE_CARRIER_SEA))
	{
		iValue += (iTemp * 1);
	}
	else
	{
		iValue += (iTemp / 2);
	}

	iTemp = kPromotion.getKamikazePercent();
	if (eUnitAI == UNITAI_ATTACK_CITY)
	{
		iValue += (iTemp / 16);
	}
	else
	{
		iValue += (iTemp / 64);
	}

	for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
	{
		iTemp = kPromotion.getTerrainAttackPercent(iI);
		if (iTemp != 0)
		{
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/* End Overvalue of Terrain Promotions                                                          */
/************************************************************************************************/
/*
			iExtra = pUnit == NULL ? kUnit.getTerrainAttackModifier(iI) : pUnit->getExtraTerrainAttackPercent((TerrainTypes)iI);
			iTemp *= (100 + iExtra * 2);
			iTemp /= 100;
*/
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			if ((eUnitAI == UNITAI_ATTACK) ||
				(eUnitAI == UNITAI_COUNTER))
			{
				iValue += (iTemp / 4);
			}
			else
			{
				iValue += (iTemp / 25);
			}
		}

		iTemp = kPromotion.getTerrainDefensePercent(iI);
		if (iTemp != 0)
		{
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/* End Overvalue of Terrain Promotions                                                          */
/************************************************************************************************/
/*
			iExtra = pUnit == NULL ? kUnit.getTerrainDefenseModifier(iI) : pUnit->getExtraTerrainDefensePercent((TerrainTypes)iI);
			iTemp *= (100 + iExtra);
			iTemp /= 100;
*/
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			if (eUnitAI == UNITAI_COUNTER)
			{
				if (pUnit != NULL && pUnit->plot()->getTerrainType() == (TerrainTypes)iI)
				{
					iValue += (iTemp / 4);
				}
				else
				{
					iValue++;
				}
			}
			else
			{
				iValue += (iTemp / 25);
			}
		}

		if (kPromotion.getTerrainDoubleMove(iI))
		{
			if (eUnitAI == UNITAI_EXPLORE)
			{
				iValue += 15;
			}
			else if ((eUnitAI == UNITAI_ATTACK) || (eUnitAI == UNITAI_PILLAGE))
			{
				iValue += 2;
			}
			else
			{
			    iValue += 1;
			}
		}
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
	{
		iTemp = kPromotion.getFeatureAttackPercent(iI);
		if (iTemp != 0)
		{
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/* End Overvalue of Terrain Promotions                                                          */
/************************************************************************************************/
/*
			iExtra = pUnit == NULL ? kUnit.getFeatureAttackModifier(iI) : pUnit->getExtraFeatureAttackPercent((FeatureTypes)iI);
			iTemp *= (100 + iExtra * 2);
			iTemp /= 100;
*/
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			if ((eUnitAI == UNITAI_ATTACK) ||
				(eUnitAI == UNITAI_COUNTER))
			{
				iValue += (iTemp / 4);
			}
			else
			{
				iValue += (iTemp / 16);
			}
		}

		iTemp = kPromotion.getFeatureDefensePercent(iI);
		if (iTemp != 0)
		{
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/* End Overvalue of Terrain Promotions                                                          */
/************************************************************************************************/
/*
			iExtra = pUnit == NULL ? kUnit.getFeatureDefenseModifier(iI) : pUnit->getExtraFeatureDefensePercent((FeatureTypes)iI);
			iTemp *= (100 + iExtra * 2);
			iTemp /= 100;
*/
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			if (pUnit != NULL && !pUnit->noDefensiveBonus() || (!kUnit.isNoDefensiveBonus()))
			{
				if (eUnitAI == UNITAI_COUNTER)
				{
					if (pUnit != NULL && pUnit->plot()->getFeatureType() == (FeatureTypes)iI)
					{
						iValue += (iTemp / 4);
					}
					else
					{
						iValue++;
					}
				}
				else
				{
					iValue += (iTemp / 16);
				}
			}
		}

		if (kPromotion.getFeatureDoubleMove(iI))
		{
			if (eUnitAI == UNITAI_EXPLORE)
			{
				iValue += 20;
			}
			else if ((eUnitAI == UNITAI_ATTACK) || (eUnitAI == UNITAI_PILLAGE))
			{
				iValue += 10;
			}
			else
			{
			    iValue += 1;
			}
		}
	}

    int iOtherCombat = 0; 
    int iSameCombat = 0;
    
    for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
    {
        if ((UnitCombatTypes)iI == kUnit.getUnitCombatType())
        {
            iSameCombat += pUnit == NULL ? kUnit.getUnitCombatModifier(iI) : pUnit->unitCombatModifier((UnitCombatTypes)iI);
        }
        else
        {
            iOtherCombat += pUnit == NULL ? kUnit.getUnitCombatModifier(iI) : pUnit->unitCombatModifier((UnitCombatTypes)iI);
        }
    }

	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		iTemp = kPromotion.getUnitCombatModifierPercent(iI);
		int iCombatWeight = 0;
        //Fighting their own kind
        if ((UnitCombatTypes)iI == kUnit.getUnitCombatType())
        {
            if (iSameCombat >= iOtherCombat)
            {
                iCombatWeight = 70;//"axeman takes formation"
            }
            else
            {
                iCombatWeight = 30;
            }
        }
        else
        {
            //fighting other kinds
            if ((pUnit != NULL && pUnit->unitCombatModifier((UnitCombatTypes)iI) > 10) || (pUnit == NULL && kUnit.getUnitCombatModifier(iI) > 10))
            {
                iCombatWeight = 70;//"spearman takes formation"
            }
            else
            {
                iCombatWeight = 30;
            }
        }

		iCombatWeight *= AI_getUnitCombatWeight((UnitCombatTypes)iI);
		iCombatWeight /= 100;		
		
		if ((eUnitAI == UNITAI_COUNTER) || (eUnitAI == UNITAI_CITY_COUNTER))
		{
		    iValue += (iTemp * iCombatWeight) / 50;
		}
		else if ((eUnitAI == UNITAI_ATTACK) ||
			       (eUnitAI == UNITAI_RESERVE))
		{
			iValue += (iTemp * iCombatWeight) / 100;
		}
		else
		{
			iValue += (iTemp * iCombatWeight) / 200;
		}
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		iTemp = kPromotion.getDomainModifierPercent(iI);
		if (eUnitAI == UNITAI_COUNTER)
		{
			iValue += (iTemp * 1);
		}
		else if ((eUnitAI == UNITAI_ATTACK) ||
			       (eUnitAI == UNITAI_RESERVE))
		{
			iValue += (iTemp / 2);
		}
		else
		{
			iValue += (iTemp / 8);
		}
	}
	
	if (kPromotion.getIgnoreTerrainDamage() != NO_TERRAIN)
	{
		iValue += -GC.getTerrainInfo((TerrainTypes)kPromotion.getIgnoreTerrainDamage()).getHealthPercent();
	}
	
	if (kPromotion.isZoneOfControl())
	{
		iValue += 250;
	}

	if (pUnit != NULL && iValue > 0)
	{
		iValue += GC.getGameINLINE().getSorenRandNum(15, "AI Promote");
	}
	
	return iValue;
}

TechTypes CvPlayerAI::AI_bestReligiousTech(int iMaxPathLength, TechTypes eIgnoreTech, AdvisorTypes eIgnoreAdvisor) const
{
	PROFILE("CvPlayerAI::AI_bestReligiousTech");

	int iValue;
	int iBestValue = 0;
	TechTypes eBestTech = NO_TECH;
	int iPathLength;
	
	for (int iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		if ((eIgnoreTech == NO_TECH) || (iI != eIgnoreTech))
		{
			if ((eIgnoreAdvisor == NO_ADVISOR) || (GC.getTechInfo((TechTypes)iI).getAdvisorType() != eIgnoreAdvisor))
			{
				if (canEverResearch((TechTypes)iI))
				{
					if (!GET_TEAM(getTeam()).isHasTech((TechTypes)iI))
					{
						if (GC.getTechInfo((TechTypes)iI).getEra() <= (getCurrentEra()))
						{
							iPathLength = findPathLength(((TechTypes)iI), false);

							if (iPathLength <= iMaxPathLength)
							{
								iValue = AI_religiousTechValue((TechTypes)iI);
								
								iValue /= std::max(1, iPathLength);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									eBestTech = ((TechTypes)iI);
								}
							}
						}
					}
				}
			}
		}
	}

	return eBestTech;
}

int CvPlayerAI::AI_religiousTechValue(TechTypes eTech) const
{
	PROFILE_FUNC();
	
	int iReligionValue = 0;
	
	if (GC.getGameINLINE().countKnownTechNumTeams(eTech) == 0)
	{
		for (int iJ = 0; iJ < GC.getNumReligionInfos(); iJ++)
		{
			TechTypes eReligionTech = (TechTypes)GC.getReligionInfo((ReligionTypes)iJ).getTechPrereq();
			if (eReligionTech == eTech)
			{
				if (!(GC.getGameINLINE().isReligionSlotTaken((ReligionTypes)iJ)))
				{
					if (!GC.getGame().isOption(GAMEOPTION_PICK_RELIGION))
					{
						ReligionTypes eFavorite = (ReligionTypes)GC.getLeaderHeadInfo(getLeaderType()).getFavoriteReligion();
						if (eFavorite != NO_RELIGION)
						{
							if (iJ == eFavorite)
							{
								iReligionValue += 1000;
							}
							else
							{
								iReligionValue += 750;
							}
						}
					}
					iReligionValue += 250;
				}
			}
		}
		
		if (iReligionValue > 0)
		{
			if (AI_isDoVictoryStrategy(AI_VICTORY_CULTURE1))
			{
				iReligionValue += 500;
			}
		}
	}

	return iReligionValue;
}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

