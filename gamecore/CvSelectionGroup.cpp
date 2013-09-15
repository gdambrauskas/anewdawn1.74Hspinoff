// selectionGroup.cpp

#include "CvGameCoreDLL.h"
#include "CvGlobals.h"
#include "CvSelectionGroup.h"
#include "CvGameAI.h"
#include "CvPlayerAI.h"
#include "CvTeamAI.h"
#include "CvUnit.h"
#include "CvGameCoreUtils.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"
#include "FAStarNode.h"
#include "CvInfos.h"
#include "FProfiler.h"
#include "CyPlot.h"
#include "CySelectionGroup.h"
#include "CyArgsList.h"
#include "CvDLLPythonIFaceBase.h"
#include <set>
#include "CvEventReporter.h"

// BUG - start
#include "CvBugOptions.h"
// BUG - end

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/30/10                                jdog5000      */
/*                                                                                              */
/* AI Logging                                                                                   */
/************************************************************************************************/
#include "BetterBTSAI.h"
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

CvSelectionGroup* CvSelectionGroup::m_pCachedMovementGroup = NULL;
std::map<int,edgeCosts>* CvSelectionGroup::m_pCachedNonEndTurnEdgeCosts = NULL;
std::map<int,edgeCosts>* CvSelectionGroup::m_pCachedEndTurnEdgeCosts = NULL;
CvPathGenerator*	CvSelectionGroup::m_generator = NULL;

// Public Functions...

CvSelectionGroup::CvSelectionGroup()
{
	reset(0, NO_PLAYER, true);
}


CvSelectionGroup::~CvSelectionGroup()
{
	uninit();
}


void CvSelectionGroup::init(int iID, PlayerTypes eOwner)
{
	//--------------------------------
	// Init saved data
	reset(iID, eOwner);

	//--------------------------------
	// Init non-saved data

	//--------------------------------
	// Init other game data
	AI_init();
}


void CvSelectionGroup::uninit()
{
	m_units.clear();

	m_missionQueue.clear();
}


// FUNCTION: reset()
// Initializes data members that are serialized.
void CvSelectionGroup::reset(int iID, PlayerTypes eOwner, bool bConstructorCall)
{
	//--------------------------------
	// Uninit class
	uninit();

	m_iID = iID;
	m_iMissionTimer = 0;

	m_bForceUpdate = false;

	m_eOwner = eOwner;

	m_eActivityType = ACTIVITY_AWAKE;
	m_eAutomateType = NO_AUTOMATE;
	m_bIsBusyCache = false;

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/19/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
	m_bIsStrandedCache = false;
	m_bIsStrandedCacheValid = false;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
// BUG - Safe Move - start
	m_bLastPathPlotChecked = false;
	m_bLastPlotVisible = false;
	m_bLastPlotRevealed = false;
// BUG - Safe Move - end

	if (!bConstructorCall)
	{
		AI_reset();
	}
}


void CvSelectionGroup::kill()
{
	FAssert(getOwnerINLINE() != NO_PLAYER);
	FAssertMsg(getID() != FFreeList::INVALID_INDEX, "getID() is not expected to be equal with FFreeList::INVALID_INDEX");
	FAssertMsg(getNumUnits() == 0, "The number of units is expected to be 0");

	GET_PLAYER(getOwnerINLINE()).removeGroupCycle(getID());

	GET_PLAYER(getOwnerINLINE()).deleteSelectionGroup(getID());
}

bool CvSelectionGroup::sentryAlert() const
{
	CvUnit* pHeadUnit = NULL;
	int iMaxRange = 0;
	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		int iRange = pLoopUnit->visibilityRange() + 1;

		if (iRange > iMaxRange)
		{
			iMaxRange = iRange;
			pHeadUnit = pLoopUnit;
		}
	}

	if (NULL != pHeadUnit)
	{
		for (int iX = -iMaxRange; iX <= iMaxRange; ++iX)
		{
			for (int iY = -iMaxRange; iY <= iMaxRange; ++iY)
			{
				CvPlot* pPlot = ::plotXY(pHeadUnit->getX_INLINE(), pHeadUnit->getY_INLINE(), iX, iY);
				if (NULL != pPlot)
				{
					if (pHeadUnit->plot()->canSeePlot(pPlot, pHeadUnit->getTeam(), iMaxRange - 1, NO_DIRECTION))
					{
						if (pPlot->isVisibleEnemyUnit(pHeadUnit))
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
/*
 * Similar to sentryAlert() except only checks water/land plots based on the domain type of the head unit.
 */
bool CvSelectionGroup::sentryAlertSameDomainType() const
{
	int iMaxRange = 0;
	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	int iIndex = -1;

	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		int iRange = pLoopUnit->visibilityRange() + 1;

		if (iRange > iMaxRange)
		{
			iMaxRange = iRange;
			iIndex = getUnitIndex(pLoopUnit);
		}
	}

	CvUnit* pHeadUnit = ((iIndex == -1) ? NULL : getUnitAt(iIndex));
	if (NULL != pHeadUnit)
	{
		for (int iX = -iMaxRange; iX <= iMaxRange; ++iX)
		{
			for (int iY = -iMaxRange; iY <= iMaxRange; ++iY)
			{
				CvPlot* pPlot = ::plotXY(pHeadUnit->getX_INLINE(), pHeadUnit->getY_INLINE(), iX, iY);
				if (NULL != pPlot)
				{
					if (pHeadUnit->plot()->canSeePlot(pPlot, pHeadUnit->getTeam(), iMaxRange - 1, NO_DIRECTION))
					{
						if (pPlot->isVisibleEnemyUnit(pHeadUnit))
						{
							if ((getDomainType() == DOMAIN_SEA) && (pPlot->isWater()))
							{
								return true;
							}
							else if ((getDomainType() == DOMAIN_LAND) && (!(pPlot->isWater())))
							{
								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}
#endif
// BUG - Sentry Actions - end

void CvSelectionGroup::doTurn()
{
	PROFILE("CvSelectionGroup::doTurn()")

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int iWaitTurns;
	int iBestWaitTurns;

	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (getNumUnits() > 0)
	{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/19/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
		invalidateIsStrandedCache();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

		bool bHurt = false;
		
		// do unit's turns (checking for damage)
		pUnitNode = headUnitNode();
		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);
/************************************************************************************************/
/* Afforess	                  Start		 07/30/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			FAssertMsg((pLoopUnit->plot() == plot()), "Selectiongroup is not on the same tile!");
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			pLoopUnit->doTurn();

			if (pLoopUnit->isHurt())
			{
				bHurt = true;
			}
		}

		ActivityTypes eActivityType = getActivityType();

		// wake unit if skipped last turn 
		//		or healing and automated or no longer hurt (automated healing is one turn at a time)
		//		or on sentry and there is danger
		if ((eActivityType == ACTIVITY_HOLD) ||
			((eActivityType == ACTIVITY_HEAL) && (AI_isControlled() || !bHurt)) ||
			((eActivityType == ACTIVITY_SENTRY) && (sentryAlert())))
		{
			setActivityType(ACTIVITY_AWAKE);
		}

// BUG - Sentry Healing and Explorering Units - start
		if (isHuman())
		{
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
			if (((eActivityType == ACTIVITY_SENTRY_NAVAL_UNITS) && (sentryAlertSameDomainType())) ||
				((eActivityType == ACTIVITY_SENTRY_LAND_UNITS) && (sentryAlertSameDomainType())) ||
				((eActivityType == ACTIVITY_SENTRY_WHILE_HEAL) && (sentryAlertSameDomainType() || AI_isControlled() || !bHurt)))
			{
				setActivityType(ACTIVITY_AWAKE);
			}
#endif
// BUG - Sentry Actions - end

// BUG - Sentry Exploring Units - start
			if (isAutomated() && getAutomateType() == AUTOMATE_EXPLORE && getBugOptionBOOL("Actions__SentryHealing", true, "BUG_SENTRY_HEALING") && sentryAlert())
			{
				if (!(getBugOptionBOOL("Actions__SentryHealingOnlyNeutral", true, "BUG_SENTRY_HEALING_ONLY_NEUTRAL") && plot()->isOwned()))
				{
					setActivityType(ACTIVITY_AWAKE);
				}
			}
// BUG - Sentry Exploring Units - end

// BUG - Sentry Healing Units - start
			if (eActivityType == ACTIVITY_HEAL && getBugOptionBOOL("Actions__SentryHealing", true, "BUG_SENTRY_HEALING") && sentryAlert())
			{
				if (!(getBugOptionBOOL("Actions__SentryHealingOnlyNeutral", true, "BUG_SENTRY_HEALING_ONLY_NEUTRAL") && plot()->isOwned()))
				{
					setActivityType(ACTIVITY_AWAKE);
				}
			}
		}
// BUG - Sentry Healing and Explorering Units - end
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      09/21/08                                jdog5000      */
/*                                                                                              */
/* Air AI                                                                                       */
/************************************************************************************************/
		// with improvements to launching air patrols, now can wake every turn
		if ( (eActivityType == ACTIVITY_INTERCEPT) && !isHuman() )
		{
			setActivityType(ACTIVITY_AWAKE);
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/	

		if (AI_isControlled())
		{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/20/09                                jdog5000      */
/*                                                                                              */
/* Unit AI, Efficiency                                                                          */
/************************************************************************************************/
			//if ((getActivityType() != ACTIVITY_MISSION) || (!canFight() && (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0)))
			if ((getActivityType() != ACTIVITY_MISSION) || (!canFight() && (GET_PLAYER(getOwnerINLINE()).AI_getAnyPlotDanger(plot(), 2))))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/	
			{
				setForceUpdate(true);
			}
		}
		else
		{
			if (getActivityType() == ACTIVITY_MISSION)
			{
				bool bNonSpy = false;
				for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					if (!pLoopUnit->isSpy())
					{
						bNonSpy = true;
						break;
					}
				}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/20/09                                jdog5000      */
/*                                                                                              */
/* Unit AI, Efficiency                                                                          */
/************************************************************************************************/
				//if (bNonSpy && GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0)
				if (bNonSpy && GET_PLAYER(getOwnerINLINE()).AI_getAnyPlotDanger(plot(), 2))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
				{
					clearMissionQueue();
				}
			}
		}

		if (isHuman())
		{
			if (GC.getGameINLINE().isMPOption(MPOPTION_SIMULTANEOUS_TURNS))
			{
				iBestWaitTurns = 0;

				pUnitNode = headUnitNode();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = nextUnitNode(pUnitNode);

					iWaitTurns = (GC.getDefineINT("MIN_TIMER_UNIT_DOUBLE_MOVES") - (GC.getGameINLINE().getTurnSlice() - pLoopUnit->getLastMoveTurn()));

					if (iWaitTurns > iBestWaitTurns)
					{
						iBestWaitTurns = iWaitTurns;
					}
				}

				setMissionTimer(std::max(iBestWaitTurns, getMissionTimer()));

				if (iBestWaitTurns > 0)
				{
					// Cycle selection if the current group is selected
					CvUnit* pSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
					if (pSelectedUnit && pSelectedUnit->getGroup() == this)
					{
						gDLL->getInterfaceIFace()->selectGroup(pSelectedUnit, false, false, false);
					}
				}
			}
		}
	}

	doDelayedDeath();
}

bool CvSelectionGroup::showMoves() const
{
	if (GC.getGameINLINE().isMPOption(MPOPTION_SIMULTANEOUS_TURNS) || GC.getGameINLINE().isSimultaneousTeamTurns())
	{
		return false;
	}

	for (int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
		if (kLoopPlayer.isAlive())
		{
			if (kLoopPlayer.isHuman())
			{
				CvUnit* pHeadUnit = getHeadUnit();

				if (NULL != pHeadUnit)
				{
					if (pHeadUnit->isEnemy(kLoopPlayer.getTeam()))
					{
						if (kLoopPlayer.isOption(PLAYEROPTION_SHOW_ENEMY_MOVES))
						{
							return true;
						}
					}
					else
					{
						if (kLoopPlayer.isOption(PLAYEROPTION_SHOW_FRIENDLY_MOVES))
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}


void CvSelectionGroup::updateTimers()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	bool bCombat;

	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (getNumUnits() > 0)
	{
		bCombat = false;

		pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);

			if (pLoopUnit->isCombat())
			{
				if (pLoopUnit->isAirCombat())
				{
					pLoopUnit->updateAirCombat();
				}
				else
				{
					pLoopUnit->updateCombat();
				}

				bCombat = true;
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
				// Dale - SA: Stack Attack START
				if (!GC.getDCM_STACK_ATTACK())
				{
					break;
				}
				// Dale - SA: Stack Attack END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
			}
		}

		if (!bCombat)
		{
			updateMission();
		}
	}

	doDelayedDeath();
}


// Returns true if group was killed...
bool CvSelectionGroup::doDelayedDeath()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (isBusy())
	{
		return false;
	}

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		pLoopUnit->doDelayedDeath();
	}

	if (getNumUnits() == 0)
	{
		kill();
		return true;
	}

	return false;
}


void CvSelectionGroup::playActionSound()
{
	// Pitboss should not be playing sounds!
#ifndef PITBOSS

	CvUnit *pHeadUnit;
	int iScriptId = -1;

	pHeadUnit = getHeadUnit();
	if ( pHeadUnit )
	{
		iScriptId = pHeadUnit->getArtInfo(0, GET_PLAYER(getOwnerINLINE()).getCurrentEra())->getActionSoundScriptId();
	}

	if ( (iScriptId == -1) && pHeadUnit )
	{
		CvCivilizationInfo *pCivInfo;
		pCivInfo = &GC.getCivilizationInfo( pHeadUnit->getCivilizationType() );
		if ( pCivInfo )
		{
			iScriptId = pCivInfo->getActionSoundScriptId();
		}
	}

	if ( (iScriptId != -1) && pHeadUnit )
	{
		CvPlot *pPlot = GC.getMapINLINE().plotINLINE(pHeadUnit->getX_INLINE(),pHeadUnit->getY_INLINE());
		if ( pPlot )
		{
			gDLL->Do3DSound( iScriptId, pPlot->getPoint() );
		}
	}

#endif // n PITBOSS
}


void CvSelectionGroup::pushMission(MissionTypes eMission, int iData1, int iData2, int iFlags, bool bAppend, bool bManual, MissionAITypes eMissionAI, CvPlot* pMissionAIPlot, CvUnit* pMissionAIUnit)
{
	PROFILE_FUNC();

	MissionData mission;

	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (!bAppend)
	{
		if (isBusy())
		{
			return;
		}

		clearMissionQueue();
	}

	if (bManual)
	{
		setAutomateType(NO_AUTOMATE);
	}

	mission.eMissionType = eMission;
	mission.iData1 = iData1;
	mission.iData2 = iData2;
	mission.iFlags = iFlags;
	mission.iPushTurn = GC.getGameINLINE().getGameTurn();

	AI_setMissionAI(eMissionAI, pMissionAIPlot, pMissionAIUnit);

	insertAtEndMissionQueue(mission, !bAppend);

	if (bManual)
	{
		if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			if (isBusy() && GC.getMissionInfo(eMission).isSound())
			{
				playActionSound();
			}

			gDLL->getInterfaceIFace()->setHasMovedUnit(true);
		}

		CvEventReporter::getInstance().selectionGroupPushMission(this, eMission);

		doDelayedDeath();
	}
}


void CvSelectionGroup::popMission()
{
	CLLNode<MissionData>* pTailNode;

	FAssert(getOwnerINLINE() != NO_PLAYER);

	pTailNode = tailMissionQueueNode();

	if (pTailNode != NULL)
	{
		deleteMissionQueueNode(pTailNode);
	}
}


void CvSelectionGroup::autoMission()
{
	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (getNumUnits() > 0)
	{
		if (headMissionQueueNode() != NULL)
		{
			if (!isBusy())
			{
				bool bVisibleHuman = false;
				if (isHuman())
				{
					for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
					{
						CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
						if (!pLoopUnit->alwaysInvisible())
						{
							bVisibleHuman = true;
							break;
						}
					}
				}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/20/09                                jdog5000      */
/*                                                                                              */
/* Unit AI, Efficiency                                                                          */
/************************************************************************************************/
				//if (bVisibleHuman && GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 1) > 0)
				if (bVisibleHuman && GET_PLAYER(getOwnerINLINE()).AI_getAnyPlotDanger(plot(), 1))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/	
				{
					clearMissionQueue();
				}
				else
				{
					if (getActivityType() == ACTIVITY_MISSION)
					{
						continueMission();
					}
					else
					{
						startMission();
					}
				}
			}
		}
	}

	doDelayedDeath();
}


void CvSelectionGroup::updateMission()
{
	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (getMissionTimer() > 0)
	{
		changeMissionTimer(-1);

		if (getMissionTimer() == 0)
		{
			if (getActivityType() == ACTIVITY_MISSION)
			{
				continueMission();
			}
			else
			{
				if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
				{
					if (gDLL->getInterfaceIFace()->getHeadSelectedUnit() == NULL)
					{
						gDLL->getInterfaceIFace()->changeCycleSelectionCounter(1);
					}
				}
			}
		}
	}
}


CvPlot* CvSelectionGroup::lastMissionPlot()
{
	CLLNode<MissionData>* pMissionNode;
	CvUnit* pTargetUnit;

	pMissionNode = tailMissionQueueNode();

	while (pMissionNode != NULL)
	{
		switch (pMissionNode->m_data.eMissionType)
		{
		case MISSION_MOVE_TO:
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
		case MISSION_MOVE_TO_SENTRY:
#endif
// BUG - Sentry Actions - end
		case MISSION_ROUTE_TO:
			return GC.getMapINLINE().plotINLINE(pMissionNode->m_data.iData1, pMissionNode->m_data.iData2);
			break;

		case MISSION_MOVE_TO_UNIT:
			pTargetUnit = GET_PLAYER((PlayerTypes)pMissionNode->m_data.iData1).getUnit(pMissionNode->m_data.iData2);
			if (pTargetUnit != NULL)
			{
				return pTargetUnit->plot();
			}
			break;

		case MISSION_SKIP:
		case MISSION_SLEEP:
		case MISSION_FORTIFY:
		case MISSION_PLUNDER:
		case MISSION_AIRPATROL:
		case MISSION_SEAPATROL:
		case MISSION_HEAL:
		case MISSION_SENTRY:
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
		case MISSION_SENTRY_WHILE_HEAL:
		case MISSION_SENTRY_NAVAL_UNITS:
		case MISSION_SENTRY_LAND_UNITS:
#endif
// BUG - Sentry Actions - end
		case MISSION_AIRLIFT:
		case MISSION_NUKE:
		case MISSION_RECON:
		case MISSION_PARADROP:
		case MISSION_AIRBOMB:
		case MISSION_BOMBARD:
		case MISSION_RANGE_ATTACK:
		case MISSION_PILLAGE:
		case MISSION_SABOTAGE:
		case MISSION_DESTROY:
		case MISSION_STEAL_PLANS:
		case MISSION_FOUND:
		case MISSION_SPREAD:
		case MISSION_SPREAD_CORPORATION:
		case MISSION_JOIN:
		case MISSION_CONSTRUCT:
		case MISSION_DISCOVER:
		case MISSION_HURRY:
		case MISSION_TRADE:
		case MISSION_GREAT_WORK:
		case MISSION_INFILTRATE:
		case MISSION_GOLDEN_AGE:
		case MISSION_BUILD:
		case MISSION_LEAD:
		case MISSION_ESPIONAGE:
		case MISSION_DIE_ANIMATION:
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
		// Dale - AB: Bombing START
		case MISSION_AIRBOMB1:
		case MISSION_AIRBOMB2:
		case MISSION_AIRBOMB3:
		case MISSION_AIRBOMB4:
		case MISSION_AIRBOMB5:
		// Dale - AB: Bombing END
		// Dale - RB: Field Bombard START
		case MISSION_RBOMBARD:
		// Dale - RB: Field Bombard END
		// Dale - ARB: Archer Bombard START
		case MISSION_ABOMBARD:
		// Dale - ARB: Archer Bombard END
		// Dale - FE: Fighters START
		case MISSION_FENGAGE:
		// Dale - FE: Fighters END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* Afforess	                  Start		 06/11/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		case MISSION_CLAIM_TERRITORY:
		case MISSION_HURRY_FOOD:
		case MISSION_INQUISITION:
		case MISSION_ESPIONAGE_SLEEP:
		case MISSION_GREAT_COMMANDER:
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			break;

		default:
			FAssert(false);
			break;
		}

		pMissionNode = prevMissionQueueNode(pMissionNode);
	}

	return plot();
}


bool CvSelectionGroup::canStartMission(int iMission, int iData1, int iData2, CvPlot* pPlot, bool bTestVisible, bool bUseCache)
{
	//PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pTargetUnit;
	CvUnit* pLoopUnit;

	//cache isBusy
	if(bUseCache)
	{
		if(m_bIsBusyCache)
		{
			return false;
		}
	}
	else
	{
		if (isBusy())
		{
			return false;
		}
	}

	if (pPlot == NULL)
	{
		pPlot = plot();
	}

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		switch (iMission)
		{
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
		case MISSION_MOVE_TO_SENTRY:
			if (!pLoopUnit->canSentry(NULL))
			{
				return false;
			}
			// fall through to next case
#endif
// BUG - Sentry Actions - end

		case MISSION_MOVE_TO:
			if (!(pPlot->at(iData1, iData2)))
			{
				return true;
			}
			break;

		case MISSION_ROUTE_TO:
			if (!(pPlot->at(iData1, iData2)) || (getBestBuildRoute(pPlot) != NO_ROUTE))
			{
				return true;
			}
			break;

		case MISSION_MOVE_TO_UNIT:
			FAssertMsg(iData1 != NO_PLAYER, "iData1 should be a valid Player");
			pTargetUnit = GET_PLAYER((PlayerTypes)iData1).getUnit(iData2);
			if ((pTargetUnit != NULL) && !(pTargetUnit->atPlot(pPlot)))
			{
				return true;
			}
			break;

		case MISSION_SKIP:
			if (pLoopUnit->canHold(pPlot))
			{
				return true;
			}
			break;

		case MISSION_SLEEP:
			if (pLoopUnit->canSleep(pPlot))
			{
				return true;
			}
			break;

		case MISSION_FORTIFY:
			if (pLoopUnit->canFortify(pPlot))
			{
				return true;
			}
			break;

		case MISSION_AIRPATROL:
			if (pLoopUnit->canAirPatrol(pPlot))
			{
				return true;
			}
			break;

		case MISSION_SEAPATROL:
			if (pLoopUnit->canSeaPatrol(pPlot))
			{
				return true;
			}
			break;

		case MISSION_HEAL:
			if (pLoopUnit->canHeal(pPlot))
			{
				return true;
			}
			break;

		case MISSION_SENTRY:
			if (pLoopUnit->canSentry(pPlot))
			{
				return true;
			}
			break;
			
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
		case MISSION_SENTRY_WHILE_HEAL:
			if ((pLoopUnit->canSentry(pPlot)) && (pLoopUnit->canHeal(pPlot)))
			{
				return true;
			}
			break;

		case MISSION_SENTRY_NAVAL_UNITS:
			if ((getDomainType() == DOMAIN_SEA) && (pLoopUnit->canSentry(pPlot)))
			{
				return true;
			}
			break;

		case MISSION_SENTRY_LAND_UNITS:
			if ((getDomainType() == DOMAIN_LAND) && (pLoopUnit->canSentry(pPlot)))
			{
				return true;
			}
			break;
#endif
// BUG - Sentry Actions - end

		case MISSION_AIRLIFT:
			if (pLoopUnit->canAirliftAt(pPlot, iData1, iData2))
			{
				return true;
			}
			break;

		case MISSION_NUKE:
			if (pLoopUnit->canNukeAt(pPlot, iData1, iData2))
			{
				return true;
			}
			break;

		case MISSION_RECON:
			if (pLoopUnit->canReconAt(pPlot, iData1, iData2))
			{
				return true;
			}
			break;

		case MISSION_PARADROP:
			if (pLoopUnit->canParadropAt(pPlot, iData1, iData2))
			{
				return true;
			}
			break;

		case MISSION_AIRBOMB:
			if (pLoopUnit->canAirBombAt(pPlot, iData1, iData2))
			{
				return true;
			}
			break;

		case MISSION_BOMBARD:
			if (pLoopUnit->canBombard(pPlot))
			{
				return true;
			}
			break;
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
		// Dale - RB: Field Bombard START
		case MISSION_RBOMBARD:
			if(GC.getDCM_RANGE_BOMBARD() || GC.getDefineINT("DCM_NAVAL_BOMBARD"))
			{
				if (pLoopUnit->canBombardAtRanged(pPlot, iData1, iData2))
				{
					return true;
				}
			}
			break;
		// Dale - RB: Field Bombard END

		// Dale - ARB: Archer Bombard START
		case MISSION_ABOMBARD:
			if(GC.getDCM_ARCHER_BOMBARD())
			{
				if (pLoopUnit->canArcherBombardAt(pPlot, iData1, iData2))
				{
					return true;
				}
			}
			break;
		// Dale - ARB: Archer Bombard END
/************************************************************************************************/
/* DCM                                     END                                    Johny Smith  */
/************************************************************************************************/

		case MISSION_RANGE_ATTACK:
			if (pLoopUnit->canRangeStrikeAt(pPlot, iData1, iData2))
			{
				return true;
			}
			break;

		case MISSION_PLUNDER:
			if (pLoopUnit->canPlunder(pPlot, bTestVisible))
			{
				return true;
			}
			break;

		case MISSION_PILLAGE:
			if (pLoopUnit->canPillage(pPlot))
			{
				return true;
			}
			break;

		case MISSION_SABOTAGE:
			if (pLoopUnit->canSabotage(pPlot, bTestVisible))
			{
				return true;
			}
			break;

		case MISSION_DESTROY:
			if (pLoopUnit->canDestroy(pPlot, bTestVisible))
			{
				return true;
			}
			break;

		case MISSION_STEAL_PLANS:
			if (pLoopUnit->canStealPlans(pPlot, bTestVisible))
			{
				return true;
			}
			break;

		case MISSION_FOUND:
			if (pLoopUnit->canFound(pPlot, bTestVisible))
			{
				return true;
			}
			break;

		case MISSION_SPREAD:
			if (pLoopUnit->canSpread(pPlot, ((ReligionTypes)iData1), bTestVisible))
			{
				return true;
			}
			break;

		case MISSION_SPREAD_CORPORATION:
			if (pLoopUnit->canSpreadCorporation(pPlot, ((CorporationTypes)iData1), bTestVisible))
			{
				return true;
			}
			break;

		case MISSION_JOIN:
			if (pLoopUnit->canJoin(pPlot, ((SpecialistTypes)iData1)))
			{
				return true;
			}
			break;

		case MISSION_CONSTRUCT:
			if (pLoopUnit->canConstruct(pPlot, ((BuildingTypes)iData1), bTestVisible))
			{
				return true;
			}
			break;

		case MISSION_DISCOVER:
			if (pLoopUnit->canDiscover(pPlot))
			{
				return true;
			}
			break;

		case MISSION_HURRY:
			if (pLoopUnit->canHurry(pPlot, bTestVisible))
			{
				return true;
			}
			break;

		case MISSION_TRADE:
			if (pLoopUnit->canTrade(pPlot, bTestVisible))
			{
				return true;
			}
			break;

		case MISSION_GREAT_WORK:
			if (pLoopUnit->canGreatWork(pPlot))
			{
				return true;
			}
			break;

		case MISSION_INFILTRATE:
			if (pLoopUnit->canInfiltrate(pPlot, bTestVisible))
			{
				return true;
			}
			break;

		case MISSION_GOLDEN_AGE:
			//this means to play the animation only			
			if (iData1 != -1)
			{
				return true;
			}

			if (pLoopUnit->canGoldenAge(pPlot, bTestVisible))
			{
				return true;
			}
			break;

		case MISSION_BUILD:

            FAssertMsg(((BuildTypes)iData1) < GC.getNumBuildInfos(), "Invalid Build");
/************************************************************************************************/
/* Afforess	                  Start		 06/01/10                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
			if (pLoopUnit->canBuild(pPlot, ((BuildTypes)iData1), bTestVisible))
*/
			if (pLoopUnit->canBuild(pPlot, ((BuildTypes)iData1), bTestVisible && !GET_PLAYER(pLoopUnit->getOwnerINLINE()).isModderOption(MODDEROPTION_HIDE_UNAVAILBLE_BUILDS)))
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/			

            {
                return true;
            }
			break;

		case MISSION_LEAD:
			if (pLoopUnit->canLead(pPlot, iData1))
			{
				return true;
			}
			break;

		case MISSION_ESPIONAGE:
			if (pLoopUnit->canEspionage(pPlot, bTestVisible))
			{
				return true;
			}
			break;

		case MISSION_DIE_ANIMATION:
			return false;
			break;

		case MISSION_BEGIN_COMBAT:
		case MISSION_END_COMBAT:
		case MISSION_AIRSTRIKE:
		case MISSION_SURRENDER:
		case MISSION_IDLE:
		case MISSION_DIE:
		case MISSION_DAMAGE:
		case MISSION_MULTI_SELECT:
		case MISSION_MULTI_DESELECT:
			break;
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
		// Dale - AB: Bombing START
		case MISSION_AIRBOMB1:
			if (pLoopUnit->canAirBomb1At(pPlot, iData1, iData2))
			{
				return true;
			}
			break;

		case MISSION_AIRBOMB2:
			if (pLoopUnit->canAirBomb2At(pPlot, iData1, iData2))
			{
				return true;
			}
			break;

		case MISSION_AIRBOMB3:
			if (pLoopUnit->canAirBomb3At(pPlot, iData1, iData2))
			{
				return true;
			}
			break;

		case MISSION_AIRBOMB4:
			if (pLoopUnit->canAirBomb4At(pPlot, iData1, iData2))
			{
				return true;
			}
			break;

		case MISSION_AIRBOMB5:
			if (pLoopUnit->canAirBomb5At(pPlot, iData1, iData2))
			{
				return true;
			}
			break;
		// Dale - AB: Bombing END

		// Dale - FE: Fighters START
		case MISSION_FENGAGE:
			if (pLoopUnit->canFEngageAt(pPlot, iData1, iData2))
			{
				return true;
			}
			break;
		// Dale - FE: Fighters END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
	/************************************************************************************************/
/* Afforess	                  Start		 02/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		case MISSION_CLAIM_TERRITORY:
			if (pLoopUnit->canClaimTerritory(pPlot))
			{
				return true;
			}
			break;
			
		case MISSION_HURRY_FOOD:
			if (pLoopUnit->canHurryFood(pPlot))
			{
				return true;
			}
			break;
		case MISSION_INQUISITION:
			if (pLoopUnit->canPerformInquisition(pPlot))
			{
				return true;
			}
			break;
		case MISSION_ESPIONAGE_SLEEP:
			if (pLoopUnit->plot()->isCity() && pLoopUnit->canSleep(pPlot) && pLoopUnit->canEspionage(pPlot, true) && pLoopUnit->getFortifyTurns() != GC.getDefineINT("MAX_FORTIFY_TURNS"))
			{
				return true;
			}
			break;
		case MISSION_GREAT_COMMANDER:
			if (GC.getGameINLINE().isOption(GAMEOPTION_GREAT_COMMANDERS) && pLoopUnit->getUnitInfo().isGreatGeneral() && !pLoopUnit->isCommander())
			{
				return true;
			}
			break;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		default:
			FAssert(false);
			break;
		}
	}

	return false;
}


void CvSelectionGroup::startMission()
{
	//PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	bool bDelete;
	bool bAction;
	bool bNuke;
	bool bNotify;

	FAssert(!isBusy());
	FAssert(getOwnerINLINE() != NO_PLAYER);
	FAssert(headMissionQueueNode() != NULL);

	if (!GC.getGameINLINE().isMPOption(MPOPTION_SIMULTANEOUS_TURNS))
	{
		if (!GET_PLAYER(getOwnerINLINE()).isTurnActive())
		{
			if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
			{
				if (IsSelected())
				{
					gDLL->getInterfaceIFace()->changeCycleSelectionCounter(1);
				}
			}

			return;
		}
	}

	if (canAllMove())
	{
		setActivityType(ACTIVITY_MISSION);
	}
	else
	{
		setActivityType(ACTIVITY_HOLD);
	}


	bDelete = false;
	bAction = false;
	bNuke = false;
	bNotify = false;

	if (!canStartMission(headMissionQueueNode()->m_data.eMissionType, headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2, plot()))
	{
		bDelete = true;
	}
	else
	{
		FAssertMsg(GET_PLAYER(getOwnerINLINE()).isTurnActive() || GET_PLAYER(getOwnerINLINE()).isHuman(), "It's expected that either the turn is active for this player or the player is human");

		switch (headMissionQueueNode()->m_data.eMissionType)
		{
		case MISSION_MOVE_TO:
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
		case MISSION_MOVE_TO_SENTRY:
#endif
// BUG - Sentry Actions - end
// BUG - Safe Move - start
			// if player is human, save the visibility and reveal state of the last plot of the move path from the initial plot
			if (isHuman())
			{
				checkLastPathPlot(GC.getMapINLINE().plotINLINE(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2));
			}
			break;
// BUG - Safe Move - end

		case MISSION_ROUTE_TO:
		case MISSION_MOVE_TO_UNIT:
			break;

		case MISSION_SKIP:
			setActivityType(ACTIVITY_HOLD);
			bDelete = true;
			break;

		case MISSION_SLEEP:
			setActivityType(ACTIVITY_SLEEP);
			bNotify = true;
			bDelete = true;
			break;

		case MISSION_FORTIFY:
			setActivityType(ACTIVITY_SLEEP);
			bNotify = true;
			bDelete = true;
			break;

		case MISSION_PLUNDER:
			setActivityType(ACTIVITY_PLUNDER);
			bNotify = true;
			bDelete = true;
			break;

		case MISSION_AIRPATROL:
			setActivityType(ACTIVITY_INTERCEPT);
			bDelete = true;
			break;

		case MISSION_SEAPATROL:
			setActivityType(ACTIVITY_PATROL);
			bDelete = true;
			break;

		case MISSION_HEAL:
			setActivityType(ACTIVITY_HEAL);
			bNotify = true;
			bDelete = true;
			break;

		case MISSION_SENTRY:
			setActivityType(ACTIVITY_SENTRY);
			bNotify = true;
			bDelete = true;
			break;

// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
		case MISSION_SENTRY_WHILE_HEAL:
			setActivityType(ACTIVITY_SENTRY_WHILE_HEAL);
			bNotify = true;
			bDelete = true;
			break;

		case MISSION_SENTRY_NAVAL_UNITS:
			setActivityType(ACTIVITY_SENTRY_NAVAL_UNITS);
			bNotify = true;
			bDelete = true;
			break;

		case MISSION_SENTRY_LAND_UNITS:
			setActivityType(ACTIVITY_SENTRY_LAND_UNITS);
			bNotify = true;
			bDelete = true;
			break;
#endif
// BUG - Sentry Actions - end

		case MISSION_AIRLIFT:
		case MISSION_NUKE:
		case MISSION_RECON:
		case MISSION_PARADROP:
		case MISSION_AIRBOMB:
		case MISSION_BOMBARD:
		case MISSION_RANGE_ATTACK:
		case MISSION_PILLAGE:
		case MISSION_SABOTAGE:
		case MISSION_DESTROY:
		case MISSION_STEAL_PLANS:
		case MISSION_FOUND:
		case MISSION_SPREAD:
		case MISSION_SPREAD_CORPORATION:
		case MISSION_JOIN:
		case MISSION_CONSTRUCT:
		case MISSION_DISCOVER:
		case MISSION_HURRY:
		case MISSION_TRADE:
		case MISSION_GREAT_WORK:
		case MISSION_INFILTRATE:
		case MISSION_GOLDEN_AGE:
		case MISSION_BUILD:
		case MISSION_LEAD:
		case MISSION_ESPIONAGE:
		case MISSION_DIE_ANIMATION:
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
		// Dale - AB: Bombing START
		case MISSION_AIRBOMB1:
		case MISSION_AIRBOMB2:
		case MISSION_AIRBOMB3:
		case MISSION_AIRBOMB4:
		case MISSION_AIRBOMB5:
		// Dale - AB: Bombing END
		// Dale - RB: Field Bombard START
		case MISSION_RBOMBARD:
		// Dale - RB: Field Bombard END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* Afforess	                  Start		 06/05/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		case MISSION_CLAIM_TERRITORY:
		case MISSION_HURRY_FOOD:
		case MISSION_INQUISITION:
		case MISSION_GREAT_COMMANDER:
			break;
		case MISSION_ESPIONAGE_SLEEP:
			bDelete = true;
			break;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			break;

		default:
			FAssert(false);
			break;
		}

		if ( bNotify )
		{
			NotifyEntity( headMissionQueueNode()->m_data.eMissionType );
		}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/30/10                                jdog5000      */
/*                                                                                              */
/* War tactics AI                                                                               */
/************************************************************************************************/
		if( headMissionQueueNode()->m_data.eMissionType == MISSION_PILLAGE )
		{
			// Fast units pillage first
			pUnitNode = headUnitNode();
			int iMaxMovesLeft = 0;

			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = nextUnitNode(pUnitNode);

/************************************************************************************************/
/* Afforess	                  Start		 07/27/10                                               */
/*                                                                                              */
/* Infinite Loop Fix                                                                            */
/************************************************************************************************/
/*
				if( pLoopUnit->canMove() && pLoopUnit->canPillage(plot()) )
*/
				if( pLoopUnit->canMove() && pLoopUnit->canPillage(pLoopUnit->plot()) )
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				{
					int iMovesLeft = pLoopUnit->movesLeft();
					if( pLoopUnit->bombardRate() > 0 )
					{
						iMovesLeft /= 2;
					}
					iMovesLeft *= pLoopUnit->currHitPoints();
					iMovesLeft /= std::max(1, pLoopUnit->maxHitPoints());

					iMaxMovesLeft = std::max( iMaxMovesLeft, iMovesLeft );
				}
			}

			bool bDidPillage = false;
			while( iMaxMovesLeft > 0 && !bDidPillage )
			{
				pUnitNode = headUnitNode();
				int iNextMaxMovesLeft = 0;
				int iTries = getNumUnits() * 3;
				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = nextUnitNode(pUnitNode);
/************************************************************************************************/
/* Afforess	                  Start		 07/27/10                                               */
/*                                                                                              */
/* Infinite Loop Fix                                                                            */
/************************************************************************************************/
/*
					if( pLoopUnit->canMove() && pLoopUnit->canPillage(plot()) )
*/
					if( pLoopUnit->canMove() && pLoopUnit->canPillage(pLoopUnit->plot()) )
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
					if( pLoopUnit->canMove() && pLoopUnit->canPillage(pLoopUnit->plot()) )
					{
						int iMovesLeft = pLoopUnit->movesLeft();
						if( pLoopUnit->bombardRate() > 0 )
						{
							iMovesLeft /= 2;
						}
						iMovesLeft *= pLoopUnit->currHitPoints();
						iMovesLeft /= std::max(1, pLoopUnit->maxHitPoints());

						if( iMovesLeft >= iMaxMovesLeft )
						{
							if (pLoopUnit->pillage())
							{
								bAction = true;
								if( isHuman() || canAllMove() )
								{
									bDidPillage = true;
									break;
								}
							}
						}

						iNextMaxMovesLeft = std::max( iNextMaxMovesLeft, iMovesLeft );
					}
				}

				iMaxMovesLeft = iNextMaxMovesLeft;
			}
		}
		else
		{
			pUnitNode = headUnitNode();

			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = nextUnitNode(pUnitNode);

				if (pLoopUnit->canMove())
				{
					switch (headMissionQueueNode()->m_data.eMissionType)
					{
					case MISSION_MOVE_TO:
					case MISSION_ROUTE_TO:
					case MISSION_MOVE_TO_UNIT:
					case MISSION_SKIP:
					case MISSION_SLEEP:
					case MISSION_FORTIFY:
					case MISSION_AIRPATROL:
					case MISSION_SEAPATROL:
					case MISSION_HEAL:
					case MISSION_SENTRY:
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
					case MISSION_MOVE_TO_SENTRY:
					case MISSION_SENTRY_WHILE_HEAL:
					case MISSION_SENTRY_NAVAL_UNITS:
					case MISSION_SENTRY_LAND_UNITS:
#endif
// BUG - Sentry Actions - end
						break;

					case MISSION_AIRLIFT:
						if (pLoopUnit->airlift(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
						{
							bAction = true;
						}
						break;

					case MISSION_NUKE:
						if (pLoopUnit->nuke(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
						{
							bAction = true;

							if (GC.getMapINLINE().plotINLINE(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2)->isVisibleToWatchingHuman())
							{
								bNuke = true;
							}
						}
						break;

					case MISSION_RECON:
						if (pLoopUnit->recon(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
						{
							bAction = true;
						}
						break;

					case MISSION_PARADROP:
						if (pLoopUnit->paradrop(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
						{
							bAction = true;
						}
						break;

					case MISSION_AIRBOMB:
						if (pLoopUnit->airBomb(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
						{
							bAction = true;
						}
						break;

					case MISSION_BOMBARD:
						if (pLoopUnit->bombard())
						{
							bAction = true;
						}
						break;

				case MISSION_RBOMBARD:
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
						// Dale - RB: Field Bombard START
						if(GC.getDCM_RANGE_BOMBARD() || GC.getDefineINT("DCM_NAVAL_BOMBARD"))
						{
							if (pLoopUnit->bombardRanged(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
							{
								bAction = true;
							}
						}
				// Dale - RB: Field Bombard END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
					break;

				case MISSION_RANGE_ATTACK:
						if (pLoopUnit->rangeStrike(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
						{
							bAction = true;
						}
						break;

					case MISSION_PILLAGE:
						if (pLoopUnit->pillage())
						{
							bAction = true;
						}
						break;

					case MISSION_PLUNDER:
						if (pLoopUnit->plunder())
						{
							bAction = true;
						}
						break;

					case MISSION_SABOTAGE:
						if (pLoopUnit->sabotage())
						{
							bAction = true;
						}
						break;

					case MISSION_DESTROY:
						if (pLoopUnit->destroy())
						{
							bAction = true;
						}
						break;

					case MISSION_STEAL_PLANS:
						if (pLoopUnit->stealPlans())
						{
							bAction = true;
						}
						break;

					case MISSION_FOUND:
						if (pLoopUnit->found())
						{
							bAction = true;
						}
						break;

					case MISSION_SPREAD:
						if (pLoopUnit->spread((ReligionTypes)(headMissionQueueNode()->m_data.iData1)))
						{
							bAction = true;
						}
						break;

					case MISSION_SPREAD_CORPORATION:
						if (pLoopUnit->spreadCorporation((CorporationTypes)(headMissionQueueNode()->m_data.iData1)))
						{
							bAction = true;
						}
						break;

					case MISSION_JOIN:
						if (pLoopUnit->join((SpecialistTypes)(headMissionQueueNode()->m_data.iData1)))
						{
							bAction = true;
						}
						break;

					case MISSION_CONSTRUCT:
						if (pLoopUnit->construct((BuildingTypes)(headMissionQueueNode()->m_data.iData1)))
						{
							bAction = true;
						}
						break;

					case MISSION_DISCOVER:
						if (pLoopUnit->discover())
						{
							bAction = true;
						}
						break;

					case MISSION_HURRY:
						if (pLoopUnit->hurry())
						{
							bAction = true;
						}
						break;

					case MISSION_TRADE:
						if (pLoopUnit->trade())
						{
							bAction = true;
						}
						break;

					case MISSION_GREAT_WORK:
						if (pLoopUnit->greatWork())
						{
							bAction = true;
						}
						break;

					case MISSION_INFILTRATE:
						if (pLoopUnit->infiltrate())
						{
							bAction = true;
						}
						break;

					case MISSION_GOLDEN_AGE:
						//just play animation, not golden age - JW
						if (headMissionQueueNode()->m_data.iData1 != -1)
						{
							CvMissionDefinition kMission;
							kMission.setMissionTime(GC.getMissionInfo(MISSION_GOLDEN_AGE).getTime() * gDLL->getSecsPerTurn());
							kMission.setUnit(BATTLE_UNIT_ATTACKER, pLoopUnit);
							kMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
							kMission.setPlot(pLoopUnit->plot());
							kMission.setMissionType(MISSION_GOLDEN_AGE);
							gDLL->getEntityIFace()->AddMission(&kMission);
							pLoopUnit->NotifyEntity(MISSION_GOLDEN_AGE);
							bAction = true;
						}
						else
						{
							if (pLoopUnit->goldenAge())
							{
								bAction = true;
							}
						}
						break;

					case MISSION_BUILD:
						break;

					case MISSION_LEAD:
						if (pLoopUnit->lead(headMissionQueueNode()->m_data.iData1))
						{
							bAction = true;
						}
						break;

					case MISSION_ESPIONAGE:
						if (pLoopUnit->espionage((EspionageMissionTypes)headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
						{
							bAction = true;
						}
						pUnitNode = NULL; // allow one unit at a time to do espionage
						break;

/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
				// Dale - AB: Bombing START
					case MISSION_AIRBOMB1:
						if (pLoopUnit->airBomb1(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
						{
							bAction = true;
						}
						break;

					case MISSION_AIRBOMB2:
						if (pLoopUnit->airBomb2(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
						{
							bAction = true;
						}
						break;

					case MISSION_AIRBOMB3:
						if (pLoopUnit->airBomb3(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
						{
							bAction = true;
						}
						break;

					case MISSION_AIRBOMB4:
						if (pLoopUnit->airBomb4(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
						{
							bAction = true;
						}
						break;

					case MISSION_AIRBOMB5:
						if (pLoopUnit->airBomb5(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
						{
							bAction = true;
						}
						break;
						// Dale - AB: Bombing END

					case MISSION_ABOMBARD:
					// Dale - ARB: Archer Bombard START
						if(GC.getDCM_ARCHER_BOMBARD())
						{
							if (pLoopUnit->archerBombard(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
							{
								bAction = true;
							}
						}
					// Dale - ARB: Archer Bombard END
						break;

					// Dale - FE: Fighters START
					case MISSION_FENGAGE:
						if(GC.getDCM_FIGHTER_ENGAGE())
						{
							if (pLoopUnit->fighterEngage(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
							{
								bAction = true;
							}
						}
						break;
					// Dale - FE: Fighters END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* Afforess	                  Start		 02/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
					case MISSION_CLAIM_TERRITORY:
						if (pLoopUnit->claimTerritory())
						{
							bAction = true;
						}
						pUnitNode = NULL; // allow only one unit to claim territory (no need for more)
						break;
						
					case MISSION_HURRY_FOOD:
						if (pLoopUnit->hurryFood())
						{
							bAction = true;
						}
						break;
					case MISSION_INQUISITION:
						if (pLoopUnit->performInquisition())
						{
							bAction = true;
						}
						break;
					case MISSION_ESPIONAGE_SLEEP:
						if (pLoopUnit->sleepForEspionage())
						{
							//bAction = true;
						}
						break;
					case MISSION_GREAT_COMMANDER:
						pLoopUnit->setCommander(true);
						pUnitNode = NULL;
						break;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/	

					case MISSION_DIE_ANIMATION:
						bAction = true;
						break;

					default:
						FAssert(false);
						break;
					}

					if (getNumUnits() == 0)
					{
						break;
					}

					if (headMissionQueueNode() == NULL)
					{
						break;
					}
				}
			}
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	if ((getNumUnits() > 0) && (headMissionQueueNode() != NULL))
	{
		if (bAction)
		{
			if (isHuman())
			{
				if (plot()->isVisibleToWatchingHuman())
				{
					updateMissionTimer();
				}
			}
		}

		if (bNuke)
		{
			setMissionTimer(GC.getMissionInfo(MISSION_NUKE).getTime());
		}

		if (!isBusy())
		{
			if (bDelete)
			{
				if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
				{
					if (IsSelected())
					{
						gDLL->getInterfaceIFace()->changeCycleSelectionCounter((GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_QUICK_MOVES)) ? 1 : 2);
					}
				}

				deleteMissionQueueNode(headMissionQueueNode());
			}
			else if (getActivityType() == ACTIVITY_MISSION)
			{
				continueMission();
			}
		}
	}
}


void CvSelectionGroup::continueMission(int iSteps)
{
	CvUnit* pTargetUnit;
	bool bDone;
	bool bAction;

	FAssert(!isBusy());
	FAssert(headMissionQueueNode() != NULL);
	FAssert(getOwnerINLINE() != NO_PLAYER);
	FAssert(getActivityType() == ACTIVITY_MISSION);

	if (headMissionQueueNode() == NULL)
	{
		// just in case...
		setActivityType(ACTIVITY_AWAKE);
		return;
	}

	bDone = false;
	bAction = false;

	if (headMissionQueueNode()->m_data.iPushTurn == GC.getGameINLINE().getGameTurn() || (headMissionQueueNode()->m_data.iFlags & MOVE_THROUGH_ENEMY))
	{
		if (headMissionQueueNode()->m_data.eMissionType == MISSION_MOVE_TO)
		{
			bool bFailedAlreadyFighting;
			if (groupAttack(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2, headMissionQueueNode()->m_data.iFlags, bFailedAlreadyFighting))
			{
				bDone = true;
			}
		}
	}

	// extra crash protection, should never happen (but a previous bug in groupAttack was causing a NULL here)
	// while that bug is fixed, no reason to not be a little more careful
	if (headMissionQueueNode() == NULL)
	{
		setActivityType(ACTIVITY_AWAKE);
		return;
	}

	if (!bDone)
	{
		if (getNumUnits() > 0)
		{
			if (canAllMove())
			{
				switch (headMissionQueueNode()->m_data.eMissionType)
				{
				case MISSION_MOVE_TO:
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
				case MISSION_MOVE_TO_SENTRY:
#endif
// BUG - Sentry Actions - end
// BUG - Safe Move - start
					// if player is human, save the visibility and reveal state of the last plot of the move path from the initial plot
					// if it hasn't been saved already to handle units in motion when loading a game
					if (isHuman() && !isLastPathPlotChecked())
					{
						checkLastPathPlot(GC.getMapINLINE().plotINLINE(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2));
					}
// BUG - Safe Move - end

					if (getDomainType() == DOMAIN_AIR)
					{
						groupPathTo(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2, headMissionQueueNode()->m_data.iFlags);
						bDone = true;
					}
					else if (groupPathTo(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2, headMissionQueueNode()->m_data.iFlags))
					{
						bAction = true;

						if (getNumUnits() > 0)
						{
							if (!canAllMove())
							{
								if (headMissionQueueNode() != NULL)
								{
									if (groupAmphibMove(GC.getMapINLINE().plotINLINE(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2), headMissionQueueNode()->m_data.iFlags))
									{
										bAction = false;
										bDone = true;
									}
								}
							}
						}
					}
					else
					{
						bDone = true;
					}
					break;

				case MISSION_ROUTE_TO:
					if (groupRoadTo(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2, headMissionQueueNode()->m_data.iFlags))
					{
						bAction = true;
					}
					else
					{
						bDone = true;
					}
					break;

				case MISSION_MOVE_TO_UNIT:
					if ((getHeadUnitAI() == UNITAI_CITY_DEFENSE) && plot()->isCity() && (plot()->getTeam() == getTeam()))
					{
						if (plot()->getBestDefender(getOwnerINLINE())->getGroup() == this)
						{
							bAction = false;
							bDone = true;
							break;
						}
					}
					pTargetUnit = GET_PLAYER((PlayerTypes)headMissionQueueNode()->m_data.iData1).getUnit(headMissionQueueNode()->m_data.iData2);
					if (pTargetUnit != NULL)
					{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      12/07/08                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
						// Handling for mission to retrieve a unit
						if( AI_getMissionAIType() == MISSIONAI_PICKUP )
						{
							if( !(pTargetUnit->getGroup()->isStranded()) || isFull() || (pTargetUnit->plot() == NULL) )
							{
								bDone = true;
								bAction = false;
								break;
							}

							CvPlot* pPickupPlot = NULL;
							CvPlot* pAdjacentPlot = NULL;
							int iPathTurns;
							int iBestPathTurns = MAX_INT;

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/08/09                        Maniac & jdog5000     */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
/* original bts code
							if( (pTargetUnit->plot()->isWater() || pTargetUnit->plot()->isFriendlyCity(*getHeadUnit(), true)) && generatePath(plot(), pTargetUnit->plot(), 0, false, &iPathTurns) )
*/
							if( (canMoveAllTerrain() || pTargetUnit->plot()->isWater() || pTargetUnit->plot()->isFriendlyCity(*getHeadUnit(), true)) && generatePath(plot(), pTargetUnit->plot(), 0, false, &iPathTurns) )
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
							{
								pPickupPlot = pTargetUnit->plot();
							}
							else
							{
								for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
								{
									pAdjacentPlot = plotDirection(pTargetUnit->plot()->getX_INLINE(), pTargetUnit->plot()->getY_INLINE(), ((DirectionTypes)iI));

									if (pAdjacentPlot != NULL)
									{
										if( atPlot(pAdjacentPlot) )
										{
											pPickupPlot = pAdjacentPlot;
											break;
										}

										if( pAdjacentPlot->isWater() || pAdjacentPlot->isFriendlyCity(*getHeadUnit(), true) )
										{
											if( generatePath(plot(), pAdjacentPlot, 0, true, &iPathTurns) )
											{
												if( iPathTurns < iBestPathTurns )
												{
													pPickupPlot = pAdjacentPlot;
													iBestPathTurns = iPathTurns;
												}
											}
										}
									}
								}
							}

							if( pPickupPlot != NULL )
							{
								if( atPlot(pPickupPlot) )
								{
									CLLNode<IDInfo>* pEntityNode;
									CvUnit* pLoopUnit;

									pEntityNode = headUnitNode();

									while (pEntityNode != NULL)
									{
										pLoopUnit = ::getUnit(pEntityNode->m_data);
										pEntityNode = nextUnitNode(pEntityNode);

										if( !(pLoopUnit->isFull()) )
										{
											pTargetUnit->getGroup()->setRemoteTransportUnit(pLoopUnit);
										}
									}

									bAction = true;
									bDone = true;
								}
								else
								{
									if (groupPathTo(pPickupPlot->getX_INLINE(), pPickupPlot->getY_INLINE(), headMissionQueueNode()->m_data.iFlags))
									{
										bAction = true;
									}
									else
									{
										bDone = true;
									}
								}
							}
							else
							{
								bDone = true;
							}
							break;
						}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

						if (AI_getMissionAIType() != MISSIONAI_SHADOW && AI_getMissionAIType() != MISSIONAI_GROUP)
						{
							if (!plot()->isOwned() || plot()->getOwnerINLINE() == getOwnerINLINE())
							{
								CvPlot* pMissionPlot = pTargetUnit->getGroup()->AI_getMissionAIPlot();
								if (pMissionPlot != NULL && NO_TEAM != pMissionPlot->getTeam())
								{
									if (pMissionPlot->isOwned() && pTargetUnit->isPotentialEnemy(pMissionPlot->getTeam(), pMissionPlot))
									{
										bAction = false;
										bDone = true;
										break;								
									}
								}
							}
						}
							 
						if (groupPathTo(pTargetUnit->getX_INLINE(), pTargetUnit->getY_INLINE(), headMissionQueueNode()->m_data.iFlags))
						{
							bAction = true;
						}
						else
						{
							bDone = true;
						}
					}
					else
					{
						bDone = true;
					}
					break;

				case MISSION_SKIP:
				case MISSION_SLEEP:
				case MISSION_FORTIFY:
				case MISSION_PLUNDER:
				case MISSION_AIRPATROL:
				case MISSION_SEAPATROL:
				case MISSION_HEAL:
				case MISSION_SENTRY:
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
				case MISSION_SENTRY_WHILE_HEAL:
				case MISSION_SENTRY_NAVAL_UNITS:
				case MISSION_SENTRY_LAND_UNITS:
#endif
// BUG - Sentry Actions - end
					FAssert(false);
					break;

				case MISSION_AIRLIFT:
				case MISSION_NUKE:
				case MISSION_RECON:
				case MISSION_PARADROP:
				case MISSION_AIRBOMB:
				case MISSION_BOMBARD:
				case MISSION_RANGE_ATTACK:
				case MISSION_PILLAGE:
				case MISSION_SABOTAGE:
				case MISSION_DESTROY:
				case MISSION_STEAL_PLANS:
				case MISSION_FOUND:
				case MISSION_SPREAD:
				case MISSION_SPREAD_CORPORATION:
				case MISSION_JOIN:
				case MISSION_CONSTRUCT:
				case MISSION_DISCOVER:
				case MISSION_HURRY:
				case MISSION_TRADE:
				case MISSION_GREAT_WORK:
				case MISSION_INFILTRATE:
				case MISSION_GOLDEN_AGE:
				case MISSION_LEAD:
				case MISSION_ESPIONAGE:
				case MISSION_DIE_ANIMATION:
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
				// Dale - AB: Bombing START
				case MISSION_AIRBOMB1:
				case MISSION_AIRBOMB2:
				case MISSION_AIRBOMB3:
				case MISSION_AIRBOMB4:
				case MISSION_AIRBOMB5:
				// Dale - AB: Bombing END
				// Dale - RB: Field Bombard START
				case MISSION_RBOMBARD:
				// Dale - RB: Field Bombard END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
					break;

				case MISSION_BUILD:
					if (!groupBuild((BuildTypes)(headMissionQueueNode()->m_data.iData1)))
					{
						bDone = true;
					}
					break;

/************************************************************************************************/
/* Afforess	                  Start		 06/05/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
				case MISSION_HURRY_FOOD:
				case MISSION_INQUISITION:
				case MISSION_CLAIM_TERRITORY:
				case MISSION_ESPIONAGE_SLEEP:
				case MISSION_GREAT_COMMANDER:
					break;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				default:
					FAssert(false);
					break;
				}
			}
		}
	}

	if ((getNumUnits() > 0) && (headMissionQueueNode() != NULL))
	{
		if (!bDone)
		{
			switch (headMissionQueueNode()->m_data.eMissionType)
			{
			case MISSION_MOVE_TO:
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
			case MISSION_MOVE_TO_SENTRY:
#endif
// BUG - Sentry Actions - end
				if (at(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
				{
// BUG - Safe Move - start
					clearLastPathPlot();
// BUG - Safe Move - end
					bDone = true;
				}
				break;

			case MISSION_ROUTE_TO:
				if (at(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2))
				{
					if (getBestBuildRoute(plot()) == NO_ROUTE)
					{
						bDone = true;
					}
				}
				break;

			case MISSION_MOVE_TO_UNIT:
				pTargetUnit = GET_PLAYER((PlayerTypes)headMissionQueueNode()->m_data.iData1).getUnit(headMissionQueueNode()->m_data.iData2);
				if ((pTargetUnit == NULL) || atPlot(pTargetUnit->plot()))
				{
					bDone = true;
				}
				break;

			case MISSION_SKIP:
			case MISSION_SLEEP:
			case MISSION_FORTIFY:
			case MISSION_PLUNDER:
			case MISSION_AIRPATROL:
			case MISSION_SEAPATROL:
			case MISSION_HEAL:
			case MISSION_SENTRY:
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
			case MISSION_SENTRY_WHILE_HEAL:
			case MISSION_SENTRY_NAVAL_UNITS:
			case MISSION_SENTRY_LAND_UNITS:
#endif
// BUG - Sentry Actions - end
				FAssert(false);
				break;

			case MISSION_AIRLIFT:
			case MISSION_NUKE:
			case MISSION_RECON:
			case MISSION_PARADROP:
			case MISSION_AIRBOMB:
			case MISSION_BOMBARD:
			case MISSION_RANGE_ATTACK:
			case MISSION_PILLAGE:
			case MISSION_SABOTAGE:
			case MISSION_DESTROY:
			case MISSION_STEAL_PLANS:
			case MISSION_FOUND:
			case MISSION_SPREAD:
			case MISSION_SPREAD_CORPORATION:
			case MISSION_JOIN:
			case MISSION_CONSTRUCT:
			case MISSION_DISCOVER:
			case MISSION_HURRY:
			case MISSION_TRADE:
			case MISSION_GREAT_WORK:
			case MISSION_INFILTRATE:
			case MISSION_GOLDEN_AGE:
			case MISSION_LEAD:
			case MISSION_ESPIONAGE:
			case MISSION_DIE_ANIMATION:
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
			// Dale - AB: Bombing START
			case MISSION_AIRBOMB1:
			case MISSION_AIRBOMB2:
			case MISSION_AIRBOMB3:
			case MISSION_AIRBOMB4:
			case MISSION_AIRBOMB5:
			// Dale - AB: Bombing END
			// Dale - RB: Field Bombard START
			case MISSION_RBOMBARD:
			// Dale - RB: Field Bombard END
			// Dale - ARB: Archer Bombard START
			case MISSION_ABOMBARD:
			// Dale - ARB: Archer Bombard END
			// Dale - FE: Fighters START
			case MISSION_FENGAGE:
			// Dale - FE: Fighters END
				bDone = true;
				break;

			case MISSION_BUILD:
				// XXX what happens if two separate worker groups are both building the mine...
				/*if (plot()->getBuildType() != ((BuildTypes)(headMissionQueueNode()->m_data.iData1)))
				{
					bDone = true;
				}*/
				break;
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* Afforess	                  Start		 06/05/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			case MISSION_HURRY_FOOD:
			case MISSION_INQUISITION:
			case MISSION_CLAIM_TERRITORY:
			case MISSION_GREAT_COMMANDER:
				bDone = true;
				break;
			case MISSION_ESPIONAGE_SLEEP:
				break;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			default:
				FAssert(false);
				break;
			}
		}
	}

	if ((getNumUnits() > 0) && (headMissionQueueNode() != NULL))
	{
		if (bAction)
		{
			if (bDone || !canAllMove())
			{
				if (plot()->isVisibleToWatchingHuman())
				{
					updateMissionTimer(iSteps);

					if (showMoves())
					{
						if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
						{
							if (getOwnerINLINE() != GC.getGameINLINE().getActivePlayer())
							{
								if (plot()->isActiveVisible(false) && !isInvisible(GC.getGameINLINE().getActiveTeam()))
								{
									gDLL->getInterfaceIFace()->lookAt(plot()->getPoint(), CAMERALOOKAT_NORMAL);
								}
							}
						}
					}
				}
			}
		}

		if (bDone)
		{
			if (!isBusy())
			{
				if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
				{
					if (IsSelected())
					{
						if ((headMissionQueueNode()->m_data.eMissionType == MISSION_MOVE_TO) ||
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
							(headMissionQueueNode()->m_data.eMissionType == MISSION_MOVE_TO_SENTRY) ||
#endif
// BUG - Sentry Actions - end
							(headMissionQueueNode()->m_data.eMissionType == MISSION_ROUTE_TO) ||
							(headMissionQueueNode()->m_data.eMissionType == MISSION_MOVE_TO_UNIT))
						{
							gDLL->getInterfaceIFace()->changeCycleSelectionCounter((GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_QUICK_MOVES)) ? 1 : 2);
						}
					}
				}

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       08/04/09                                jdog5000      */
/*                                                                                              */
/* Player interface                                                                             */
/************************************************************************************************/
/* original bts code
				deleteMissionQueueNode(headMissionQueueNode());
*/
				if (!isHuman() || (headMissionQueueNode()->m_data.eMissionType != MISSION_MOVE_TO))
				{
					deleteMissionQueueNode(headMissionQueueNode());
				}
				else
				{
					if (canAllMove() || (nextMissionQueueNode(headMissionQueueNode()) == NULL))
					{
						deleteMissionQueueNode(headMissionQueueNode());
					}
				}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/				
			}
		}
		else
		{
			if (canAllMove())
			{
				continueMission(iSteps + 1);
			}
			else if (!isBusy())
			{
				if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
				{
					if (IsSelected())
					{
						gDLL->getInterfaceIFace()->changeCycleSelectionCounter(1);
					}
				}
			}
		}
	}
}


bool CvSelectionGroup::canDoCommand(CommandTypes eCommand, int iData1, int iData2, bool bTestVisible, bool bUseCache)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	//cache isBusy
	if(bUseCache)
	{
		if(m_bIsBusyCache)
		{
			return false;
		}
	}
	else
	{
		if (isBusy())
		{
			return false;
		}
	}

	if(!canEverDoCommand(eCommand, iData1, iData2, bTestVisible, bUseCache))
		return false;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit->canDoCommand(eCommand, iData1, iData2, bTestVisible, false))
		{
			return true;
		}
	}

	return false;
}

bool CvSelectionGroup::canEverDoCommand(CommandTypes eCommand, int iData1, int iData2, bool bTestVisible, bool bUseCache)
{
	if(eCommand == COMMAND_LOAD)
	{
		CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();

		while (pUnitNode != NULL)
		{
			CvUnit *pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = plot()->nextUnitNode(pUnitNode);

			if (!pLoopUnit->isFull())
			{
				return true;
			}
		}

		//no cargo space on this plot
		return false;
	}
	else if(eCommand == COMMAND_UNLOAD)
	{
		CLLNode<IDInfo>* pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			CvUnit *pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);

			if (pLoopUnit->isCargo())
			{
				return true;
			}
		}

		//no loaded unit
		return false;
	}
	else if(eCommand == COMMAND_UPGRADE)
	{
		if(bUseCache)
		{
			//see if any of the different units can upgrade to this unit type
			for(int i=0;i<(int)m_aDifferentUnitCache.size();i++)
			{
				CvUnit *unit = m_aDifferentUnitCache[i];
				if(unit->canDoCommand(eCommand, iData1, iData2, bTestVisible, false))
					return true;
			}

			return false;
		}
	}

	return true;
}

void CvSelectionGroup::setupActionCache()
{
	//cache busy calculation
	m_bIsBusyCache = isBusy();

    //cache different unit types
	m_aDifferentUnitCache.erase(m_aDifferentUnitCache.begin(), m_aDifferentUnitCache.end());
	CLLNode<IDInfo> *pUnitNode = headUnitNode();
	while(pUnitNode != NULL)
	{
		CvUnit *unit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if(unit->isReadyForUpgrade())
		{
			UnitTypes unitType = unit->getUnitType();
			bool bFound = false;
			for(int i=0;i<(int)m_aDifferentUnitCache.size();i++)
			{
				if(unitType == m_aDifferentUnitCache[i]->getUnitType())
				{
					bFound = true;
					break;
				}
			}

			if(!bFound)
				m_aDifferentUnitCache.push_back(unit);
		}
	}
}

// Returns true if one of the units can support the interface mode...
bool CvSelectionGroup::canDoInterfaceMode(InterfaceModeTypes eInterfaceMode)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	FAssertMsg(eInterfaceMode != NO_INTERFACEMODE, "InterfaceMode is not assigned a valid value");

	if (isBusy())
	{
		return false;
	}

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);

		switch (eInterfaceMode)
		{
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
		case INTERFACEMODE_GO_TO_SENTRY:
			if (sentryAlertSameDomainType())
			{
				return false;
			}
			// fall through to next case
#endif
// BUG - Sentry Actions - end
		case INTERFACEMODE_GO_TO:
			if ((getDomainType() != DOMAIN_AIR) && (getDomainType() != DOMAIN_IMMOBILE))
			{
				return true;
			}
			break;

		case INTERFACEMODE_GO_TO_TYPE:
			if ((getDomainType() != DOMAIN_AIR) && (getDomainType() != DOMAIN_IMMOBILE))
			{
				if (pLoopUnit->plot()->plotCount(PUF_isUnitType, pLoopUnit->getUnitType(), -1, pLoopUnit->getOwnerINLINE()) > 1)
				{
					return true;
				}
			}
			break;

		case INTERFACEMODE_GO_TO_ALL:
			if ((getDomainType() != DOMAIN_AIR) && (getDomainType() != DOMAIN_IMMOBILE))
			{
				if (pLoopUnit->plot()->plotCount(NULL, -1, -1, pLoopUnit->getOwnerINLINE()) > 1)
				{
					return true;
				}
			}
			break;

		case INTERFACEMODE_ROUTE_TO:
			if (pLoopUnit->AI_getUnitAIType() == UNITAI_WORKER)
			{
				if (pLoopUnit->canBuildRoute())
				{
					return true;
				}
			}
			break;

		case INTERFACEMODE_AIRLIFT:
			if (pLoopUnit->canAirlift(pLoopUnit->plot()))
			{
				return true;
			}
			break;

		case INTERFACEMODE_NUKE:
			if (pLoopUnit->canNuke(pLoopUnit->plot()))
			{
				return true;
			}
			break;

		case INTERFACEMODE_RECON:
			if (pLoopUnit->canRecon(pLoopUnit->plot()))
			{
				return true;
			}
			break;

		case INTERFACEMODE_PARADROP:
			if (pLoopUnit->canParadrop(pLoopUnit->plot()))
			{
				return true;
			}
			break;

		case INTERFACEMODE_AIRBOMB:
			if (pLoopUnit->canAirBomb(pLoopUnit->plot()))
			{
				return true;
			}
			break;

		case INTERFACEMODE_RANGE_ATTACK:
			if (pLoopUnit->canRangeStrike())
			{
				return true;
			}
			break;

		case INTERFACEMODE_AIRSTRIKE:
			if (pLoopUnit->getDomainType() == DOMAIN_AIR)
			{
				if (pLoopUnit->canAirAttack())
				{
					return true;
				}
			}
			break;

		case INTERFACEMODE_REBASE:
			if (pLoopUnit->getDomainType() == DOMAIN_AIR)
			{
				return true;
			}
			break;
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
		// Dale - AB: Bombing START
		case INTERFACEMODE_AIRBOMB1:
			if (pLoopUnit->canAirBomb1(pLoopUnit->plot()))
			{
				return true;
			}
			break;

		case INTERFACEMODE_AIRBOMB2:
			if (pLoopUnit->canAirBomb2(pLoopUnit->plot()))
			{
				return true;
			}
			break;

		case INTERFACEMODE_AIRBOMB3:
			if (pLoopUnit->canAirBomb3(pLoopUnit->plot()))
			{
				return true;
			}
			break;

		case INTERFACEMODE_AIRBOMB4:
			if (pLoopUnit->canAirBomb4(pLoopUnit->plot()))
			{
				return true;
			}
			break;

		case INTERFACEMODE_AIRBOMB5:
			if (pLoopUnit->canAirBomb5(pLoopUnit->plot()))
			{
				return true;
			}
			break;
		// Dale - AB: Bombing END

		// Dale - RB: Field Bombard START
		case INTERFACEMODE_BOMBARD:
			if (pLoopUnit->canRBombard(pLoopUnit->plot()))
			{
				return true;
			}
			break;
		// Dale - RB: Field Bombard END

		// Dale - ARB: Archer Bombard START
		case INTERFACEMODE_ABOMBARD:
			if (pLoopUnit->canArcherBombard(pLoopUnit->plot()) && GC.getDCM_ARCHER_BOMBARD())
			{
				return true;
			}
			break;
		// Dale - ARB: Archer Bombard END

		// Dale - FE: Fighters START
		case INTERFACEMODE_FENGAGE:
			if (pLoopUnit->canFEngage(pLoopUnit->plot()) && GC.getDCM_FIGHTER_ENGAGE())
			{
				return true;
			}
			break;
		// Dale - FE: Fighters END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
		}

		pUnitNode = nextUnitNode(pUnitNode);
	}

	return false;
}


// Returns true if one of the units can execute the interface mode at the specified plot...
bool CvSelectionGroup::canDoInterfaceModeAt(InterfaceModeTypes eInterfaceMode, CvPlot* pPlot)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	FAssertMsg(eInterfaceMode != NO_INTERFACEMODE, "InterfaceMode is not assigned a valid value");

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);

		switch (eInterfaceMode)
		{
		case INTERFACEMODE_AIRLIFT:
			if (pLoopUnit != NULL)
			{
				if (pLoopUnit->canAirliftAt(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
				{
					return true;
				}
			}
			break;

		case INTERFACEMODE_NUKE:
			if (pLoopUnit != NULL)
			{
				if (pLoopUnit->canNukeAt(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
				{
					return true;
				}
			}
			break;

		case INTERFACEMODE_RECON:
			if (pLoopUnit != NULL)
			{
				if (pLoopUnit->canReconAt(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
				{
					return true;
				}
			}
			break;

		case INTERFACEMODE_PARADROP:
			if (pLoopUnit != NULL)
			{
				if (pLoopUnit->canParadropAt(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
				{
					return true;
				}
			}
			break;

		case INTERFACEMODE_AIRBOMB:
			if (pLoopUnit != NULL)
			{
				if (pLoopUnit->canAirBombAt(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
				{
					return true;
				}
			}
			break;

		case INTERFACEMODE_RANGE_ATTACK:
			if (pLoopUnit != NULL)
			{
				if (pLoopUnit->canRangeStrikeAt(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
				{
					return true;
				}
			}
			break;

		case INTERFACEMODE_AIRSTRIKE:
			if (pLoopUnit != NULL)
			{
				if (pLoopUnit->canMoveInto(pPlot, true))
				{
					return true;
				}
			}
			break;

		case INTERFACEMODE_REBASE:
			if (pLoopUnit != NULL)
			{
				if (pLoopUnit->canMoveInto(pPlot))
				{
					return true;
				}
			}
			break;
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
		// Dale - AB: Bombing START
		case INTERFACEMODE_AIRBOMB1:
			if (pLoopUnit != NULL)
			{
				if (GC.getDCM_AIR_BOMBING())
				{
					if (pLoopUnit->canAirBomb1At(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
					{
						return true;
					}
				}
			}
			break;

		case INTERFACEMODE_AIRBOMB2:
			if (pLoopUnit != NULL)
			{
				if (GC.getDCM_AIR_BOMBING())
				{
					if (pLoopUnit->canAirBomb2At(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
					{
						return true;
					}
				}
			}
			break;

		case INTERFACEMODE_AIRBOMB3:
			if (pLoopUnit != NULL)
			{
				if (GC.getDCM_AIR_BOMBING())
				{
					if (pLoopUnit->canAirBomb3At(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
					{
						return true;
					}
				}
			}
			break;

		case INTERFACEMODE_AIRBOMB4:
			if (pLoopUnit != NULL)
			{
				if (GC.getDCM_AIR_BOMBING())
				{
					if (pLoopUnit->canAirBomb4At(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
					{
						return true;
					}
				}
			}
			break;

		case INTERFACEMODE_AIRBOMB5:
			if (pLoopUnit != NULL)
			{
				if (GC.getDCM_AIR_BOMBING())
				{
					if (pLoopUnit->canAirBomb5At(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
					{
						return true;
					}
				}
			}
			break;
		// Dale - AB: Bombing END

		// Dale - RB: Field Bombard START
		case INTERFACEMODE_BOMBARD:
			if (pLoopUnit != NULL)
			{
				if(GC.getDCM_RANGE_BOMBARD() || GC.getDefineINT("DCM_NAVAL_BOMBARD"))
				{
					if (pLoopUnit->canBombardAtRanged(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
					{
						return true;
					}
				}
			}
			break;
		// Dale - RB: Field Bombard END

		// Dale - ARB: Archer Bombard START
		case INTERFACEMODE_ABOMBARD:
			if (pLoopUnit != NULL)
			{
				if(GC.getDCM_ARCHER_BOMBARD())
				{
					if (pLoopUnit->canArcherBombardAt(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
					{
						return true;
					}
				}
			}
			break;
		// Dale - ARB: Archer Bombard END

		// Dale - FE: Fighters START
		case INTERFACEMODE_FENGAGE:
			if (pLoopUnit != NULL)
			{
				if(GC.getDCM_FIGHTER_ENGAGE())
				{
					if (pLoopUnit->canFEngageAt(pLoopUnit->plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()))
					{
						return true;
					}
				}
			}
			break;
		// Dale - FE: Fighters END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/

		default:
			return true;
			break;
		}

		pUnitNode = nextUnitNode(pUnitNode);
	}

	return false;
}


bool CvSelectionGroup::isHuman()
{
	if (getOwnerINLINE() != NO_PLAYER)
	{
		return GET_PLAYER(getOwnerINLINE()).isHuman();
	}

	return true;
}


bool CvSelectionGroup::isBusy()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;

	if (getNumUnits() == 0)
	{
		return false;
	}

	if (getMissionTimer() > 0)
	{
		return true;
	}

	pPlot = plot();

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit != NULL)
		{
			if (pLoopUnit->isCombat())
			{
				return true;
			}
		}
	}

	return false;
}


bool CvSelectionGroup::isCargoBusy()
{
	CLLNode<IDInfo>* pUnitNode1;
	CLLNode<IDInfo>* pUnitNode2;
	CvUnit* pLoopUnit1;
	CvUnit* pLoopUnit2;
	CvPlot* pPlot;

	if (getNumUnits() == 0)
	{
		return false;
	}

	pPlot = plot();

	pUnitNode1 = headUnitNode();

	while (pUnitNode1 != NULL)
	{
		pLoopUnit1 = ::getUnit(pUnitNode1->m_data);
		pUnitNode1 = nextUnitNode(pUnitNode1);

		if (pLoopUnit1 != NULL)
		{
			if (pLoopUnit1->getCargo() > 0)
			{
				pUnitNode2 = pPlot->headUnitNode();

				while (pUnitNode2 != NULL)
				{
					pLoopUnit2 = ::getUnit(pUnitNode2->m_data);
					pUnitNode2 = pPlot->nextUnitNode(pUnitNode2);

					if (pLoopUnit2->getTransportUnit() == pLoopUnit1)
					{
						if (pLoopUnit2->getGroup()->isBusy())
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}


int CvSelectionGroup::baseMoves()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int iValue;
	int iBestValue;

	iBestValue = MAX_INT;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		iValue = pLoopUnit->baseMoves();

		if (iValue < iBestValue)
		{
			iBestValue = iValue;
		}
	}

	return iBestValue;
}


bool CvSelectionGroup::isWaiting() const
{
	return ((getActivityType() == ACTIVITY_HOLD) ||
		      (getActivityType() == ACTIVITY_SLEEP) ||
					(getActivityType() == ACTIVITY_HEAL) ||
					(getActivityType() == ACTIVITY_SENTRY) ||
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
					(getActivityType() == ACTIVITY_SENTRY_WHILE_HEAL) ||
					(getActivityType() == ACTIVITY_SENTRY_NAVAL_UNITS) ||
					(getActivityType() == ACTIVITY_SENTRY_LAND_UNITS) ||
#endif
// BUG - Sentry Actions - end
					(getActivityType() == ACTIVITY_PATROL) ||
					(getActivityType() == ACTIVITY_PLUNDER) ||
					(getActivityType() == ACTIVITY_INTERCEPT));
}


bool CvSelectionGroup::isFull()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	if (getNumUnits() > 0)
	{
		// do two passes, the first pass, we ignore units with speical cargo
		int iSpecialCargoCount = 0;
		int iCargoCount = 0;
		
		// first pass, count but ignore special cargo units
		pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);
			
			if (pLoopUnit->cargoSpace() > 0)
			{
				iCargoCount++;
			}

			if (pLoopUnit->specialCargo() != NO_SPECIALUNIT)
			{
				iSpecialCargoCount++;
			}
			else if (!(pLoopUnit->isFull()))
			{
				return false;
			}
		}
		
		// if every unit in the group has special cargo, then check those, otherwise, consider ourselves full
		if (iSpecialCargoCount >= iCargoCount)
		{
			pUnitNode = headUnitNode();
			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = nextUnitNode(pUnitNode);
				
				if (!(pLoopUnit->isFull()))
				{
					return false;
				}
			}
		}

		return true;
	}

	return false;
}


bool CvSelectionGroup::hasCargo()
{
	CLLNode<IDInfo>* pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit->hasCargo())
		{
			return true;
		}
	}

	return false;
}

int CvSelectionGroup::getCargo() const
{
	int iCargoCount = 0;

	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		iCargoCount += pLoopUnit->getCargo();
	}

	return iCargoCount;
}

bool CvSelectionGroup::canAllMove()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	if (getNumUnits() > 0)
	{
		pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);

			FAssertMsg(pLoopUnit != NULL, "existing node, but NULL unit");

			if (pLoopUnit != NULL && !(pLoopUnit->canMove()))
			{
				return false;
			}
		}

		return true;
	}

	return false;
}


bool CvSelectionGroup::canAnyMove()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit->canMove())
		{
			return true;
		}
	}

	return false;
}

bool CvSelectionGroup::hasMoved()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit->hasMoved())
		{
			return true;
		}
	}

	return false;
}


bool CvSelectionGroup::canEnterTerritory(TeamTypes eTeam, bool bIgnoreRightOfPassage) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	if (getNumUnits() > 0)
	{
		pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);

			if (!(pLoopUnit->canEnterTerritory(eTeam, bIgnoreRightOfPassage)))
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

bool CvSelectionGroup::canEnterArea(TeamTypes eTeam, const CvArea* pArea, bool bIgnoreRightOfPassage) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	if (getNumUnits() > 0)
	{
		pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);

			if (!(pLoopUnit->canEnterArea(eTeam, pArea, bIgnoreRightOfPassage)))
			{
				return false;
			}
		}

		return true;
	}

	return false;
}


bool CvSelectionGroup::canMoveInto(CvPlot* pPlot, bool bAttack)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	if (getNumUnits() > 0)
	{
		pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);

			if (pLoopUnit->canMoveInto(pPlot, bAttack))
			{
				return true;
			}
		}
	}

	return false;
}


bool CvSelectionGroup::canMoveOrAttackInto(CvPlot* pPlot, bool bDeclareWar)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	if (getNumUnits() > 0)
	{
		pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);

			if (pLoopUnit->canMoveOrAttackInto(pPlot, bDeclareWar))
			{
				return true;
			}
		}
	}

	return false;
}


bool CvSelectionGroup::canMoveThrough(CvPlot* pPlot)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	if (getNumUnits() > 0)
	{
		pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);

			if (!(pLoopUnit->canMoveThrough(pPlot)))
			{
				return false;
			}
		}

		return true;
	}

	return false;
}


bool CvSelectionGroup::canFight()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit->canFight())
		{
			return true;
		}
	}

	return false;
}


bool CvSelectionGroup::canDefend()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit->canDefend())
		{
			return true;
		}
	}

	return false;
}

bool CvSelectionGroup::canBombard(const CvPlot* pPlot)
{
	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit->canBombard(pPlot))
		{
			return true;
		}
/************************************************************************************************/
/* DCM                               04/19/09                           Johny Smith      */
/************************************************************************************************/		
		// Dale - RB: Field Bombard START
		if (pLoopUnit->canRBombard(pPlot))
		{
			return true;
		}
		// Dale - RB: Field Bombard END
		// Dale - ARB: Archer Bombard START
		if (pLoopUnit->canArcherBombard(pPlot))
		{
			return true;
		}
		// Dale - ARB: Archer Bombard END
/************************************************************************************************/
/* DCM                               End                               Johny Smith        */
/************************************************************************************************/		
	}

	return false;
}

bool CvSelectionGroup::visibilityRange()
{
	int iMaxRange = 0;
	
	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		int iRange = pLoopUnit->visibilityRange();
		if (iRange > iMaxRange)
		{
			iMaxRange = iRange;
		}
	}

	return iMaxRange;
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/30/10                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
//
// Approximate how many turns this group would take to reduce pCity's defense modifier to zero
//
int CvSelectionGroup::getBombardTurns(CvCity* pCity)
{
	PROFILE_FUNC();

	bool bHasBomber = (getOwnerINLINE() != NO_PLAYER ? (GET_PLAYER(getOwnerINLINE()).AI_calculateTotalBombard(DOMAIN_AIR) > 0) : false);
	bool bIgnoreBuildingDefense = bHasBomber;
	int iTotalBombardRate = (bHasBomber ? 16 : 0);
	int iUnitBombardRate = 0;

	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if( pLoopUnit->bombardRate() > 0 )
		{
			iUnitBombardRate = pLoopUnit->bombardRate();

			if( pLoopUnit->ignoreBuildingDefense() )
			{
				bIgnoreBuildingDefense = true;
			}
			else
			{
				iUnitBombardRate *= std::max(25, (100 - pCity->getBuildingBombardDefense()));
				iUnitBombardRate /= 100;
			}

			iTotalBombardRate += iUnitBombardRate;
		}
	}


	if( pCity->getTotalDefense(bIgnoreBuildingDefense) == 0 )
	{
		return 0;
	}

	int iBombardTurns = pCity->getTotalDefense(bIgnoreBuildingDefense);

	if( iTotalBombardRate > 0 )
	{
		iBombardTurns = (GC.getMAX_CITY_DEFENSE_DAMAGE() - pCity->getDefenseDamage());
		iBombardTurns *= pCity->getTotalDefense(false);
		iBombardTurns += (GC.getMAX_CITY_DEFENSE_DAMAGE() * iTotalBombardRate) - 1;
		iBombardTurns /= std::max(1, (GC.getMAX_CITY_DEFENSE_DAMAGE() * iTotalBombardRate));
	}

	//if( gUnitLogLevel > 2 ) logBBAI("      Bombard of %S will take %d turns at rate %d and current damage %d with bombard def %d", pCity->getName().GetCString(), iBombardTurns, iTotalBombardRate, pCity->getDefenseDamage(), (bIgnoreBuildingDefense ? 0 : pCity->getBuildingBombardDefense()));

	return iBombardTurns;
}

bool CvSelectionGroup::isHasPathToAreaPlayerCity( PlayerTypes ePlayer, int iFlags, int iMaxPathTurns )
{
	PROFILE_FUNC();

	CvCity* pLoopCity = NULL;
	int iLoop;
	int iPathTurns;

	for (pLoopCity = GET_PLAYER(ePlayer).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(ePlayer).nextCity(&iLoop))
	{
		if( pLoopCity->area() == area() )
		{
			if( generatePath(plot(), pLoopCity->plot(), iFlags, true, &iPathTurns) )
			{
				if( (iMaxPathTurns < 0) || (iPathTurns <= iMaxPathTurns) )
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool CvSelectionGroup::isHasPathToAreaEnemyCity( bool bIgnoreMinors, int iFlags, int iMaxPathTurns )
{
	PROFILE_FUNC();

	int iI;
	int iPass = 0;

	for( iI = 0; iI < MAX_PLAYERS; iI++ )
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive() && isPotentialEnemy(getTeam(), GET_PLAYER((PlayerTypes)iI).getTeam()) )
		{
			if( !bIgnoreMinors || (!GET_PLAYER((PlayerTypes)iI).isBarbarian() && !GET_PLAYER((PlayerTypes)iI).isMinorCiv()) )
			{
				if( isHasPathToAreaPlayerCity((PlayerTypes)iI, iFlags, iMaxPathTurns) )
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool CvSelectionGroup::isStranded()
{
	PROFILE_FUNC();

	if( !(m_bIsStrandedCacheValid) )
	{
		m_bIsStrandedCache = calculateIsStranded();
		m_bIsStrandedCacheValid = true;
	}
		
	return m_bIsStrandedCache;
}

void CvSelectionGroup::invalidateIsStrandedCache()
{
	m_bIsStrandedCacheValid = false;
}

bool CvSelectionGroup::calculateIsStranded()
{
	PROFILE_FUNC();

	if( getNumUnits() <= 0 )
	{
		return false;
	}

	if( plot() == NULL )
	{
		return false;
	}

	if( getDomainType() != DOMAIN_LAND )
	{
		return false;
	}

	if( (getActivityType() != ACTIVITY_AWAKE) && (getActivityType() != ACTIVITY_HOLD) )
	{
		return false;
	}

	if( AI_getMissionAIType() != NO_MISSIONAI )
	{
		return false;
	}

	if( getLengthMissionQueue() > 0 )
	{
		return false;
	}
	
	if( !canAllMove() )
	{
		return false;
	}

	if( getHeadUnit()->isCargo() )
	{
		return false;
	}

	if( plot()->area()->getNumUnrevealedTiles(getTeam()) > 0 )
	{
		if( (getHeadUnitAI() == UNITAI_ATTACK) || (getHeadUnitAI() == UNITAI_EXPLORE) )
		{
			return false;
		}
	}

	int iBestValue;
	if( (getHeadUnitAI() == UNITAI_SETTLE) && (GET_PLAYER(getOwner()).AI_getNumAreaCitySites(getArea(), iBestValue) > 0) )
	{
		return false;
	}

	if( plot()->area()->getCitiesPerPlayer(getOwner()) == 0 )
	{
		int iBestValue;
		if( (plot()->area()->getNumAIUnits(getOwner(),UNITAI_SETTLE) > 0) && (GET_PLAYER(getOwner()).AI_getNumAreaCitySites(getArea(), iBestValue) > 0) )
		{
			return false;
		}
	}

	if( plot()->area()->getNumCities() > 0 )
	{
		if( getHeadUnit()->AI_getUnitAIType() == UNITAI_SPY )
		{
			return false;
		}

		if( plot()->getImprovementType() != NO_IMPROVEMENT )
		{
			if( GC.getImprovementInfo(plot()->getImprovementType()).isActsAsCity() && canDefend() )
			{
				return false;
			}
		}

		if( plot()->isCity() && (plot()->getOwner() == getOwner()) )
		{
			return false;
		}

		if( isHasPathToAreaPlayerCity(getOwner()) )
		{
			return false;
		}

		if( isHasPathToAreaEnemyCity(false) )
		{
			return false;
		}
	}

	return true;
}

bool CvSelectionGroup::canMoveAllTerrain() const
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (!(pLoopUnit->canMoveAllTerrain()))
		{
			return false;
		}
	}

	return true;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

void CvSelectionGroup::unloadAll()
{
	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit != NULL)
		{
			pLoopUnit->unloadAll();
		}
	}
}

bool CvSelectionGroup::alwaysInvisible() const
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	if (getNumUnits() > 0)
	{
		pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);

			if (!(pLoopUnit->alwaysInvisible()))
			{
				return false;
			}
		}

		return true;
	}

	return false;
}


bool CvSelectionGroup::isInvisible(TeamTypes eTeam) const
{
	if (getNumUnits() > 0)
	{
		CLLNode<IDInfo>* pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);

			if (!pLoopUnit->isInvisible(eTeam, false))
			{
				return false;
			}
		}

		return true;
	}

	return false;
}


int CvSelectionGroup::countNumUnitAIType(UnitAITypes eUnitAI)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int iCount;

	FAssertMsg(headUnitNode() != NULL, "headUnitNode() is not expected to be equal with NULL");

	iCount = 0;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);
		
		// count all units if NO_UNITAI passed in
		if (NO_UNITAI == eUnitAI || pLoopUnit->AI_getUnitAIType() == eUnitAI)
		{
			iCount++;
		}
	}

	return iCount;
}


bool CvSelectionGroup::hasWorker()
{
	return ((countNumUnitAIType(UNITAI_WORKER) > 0) || (countNumUnitAIType(UNITAI_WORKER_SEA) > 0));
}


bool CvSelectionGroup::IsSelected()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit->IsSelected())
		{
			return true;
		}
	}

	return false;
}


void CvSelectionGroup::NotifyEntity(MissionTypes eMission)
{
	CLLNode<IDInfo>* pUnitNode;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		::getUnit(pUnitNode->m_data)->NotifyEntity(eMission);
		pUnitNode = nextUnitNode(pUnitNode);
	}
}


void CvSelectionGroup::airCircle(bool bStart)
{
	CLLNode<IDInfo>* pUnitNode;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		::getUnit(pUnitNode->m_data)->airCircle(bStart);
		pUnitNode = nextUnitNode(pUnitNode);
	}
}


void CvSelectionGroup::setBlockading(bool bStart)
{
	CLLNode<IDInfo>* pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		::getUnit(pUnitNode->m_data)->setBlockading(bStart);
		pUnitNode = nextUnitNode(pUnitNode);
	}
}


int CvSelectionGroup::getX() const
{
	CvUnit* pHeadUnit;

	pHeadUnit = getHeadUnit();

	if (pHeadUnit != NULL)
	{
		return getHeadUnit()->getX_INLINE();
	}
	else
	{
		return INVALID_PLOT_COORD;
	}
}


int CvSelectionGroup::getY() const
{
	CvUnit* pHeadUnit;

	pHeadUnit = getHeadUnit();

	if (pHeadUnit != NULL)
	{
		return getHeadUnit()->getY_INLINE();
	}
	else
	{
		return INVALID_PLOT_COORD;
	}
}


bool CvSelectionGroup::at(int iX, int iY) const
{
	return((getX() == iX) && (getY() == iY));
}


bool CvSelectionGroup::atPlot( const CvPlot* pPlot) const
{
	return (plot() == pPlot);
}


CvPlot* CvSelectionGroup::plot() const
{
	CvUnit* pHeadUnit;

	pHeadUnit = getHeadUnit();

	if (pHeadUnit != NULL)
	{
		return getHeadUnit()->plot();
	}
	else
	{
		return NULL;
	}
}


int CvSelectionGroup::getArea() const
{
	CvUnit* pHeadUnit;

	pHeadUnit = getHeadUnit();

	if (pHeadUnit != NULL)
	{
		return getHeadUnit()->getArea();
	}
	else
	{
		return NULL;
	}
}

CvArea* CvSelectionGroup::area() const
{
	CvUnit* pHeadUnit;

	pHeadUnit = getHeadUnit();

	if (pHeadUnit != NULL)
	{
		return getHeadUnit()->area();
	}
	else
	{
		return NULL;
	}
}


DomainTypes CvSelectionGroup::getDomainType() const
{
	CvUnit* pHeadUnit;

	pHeadUnit = getHeadUnit();

	if (pHeadUnit != NULL)
	{
		return getHeadUnit()->getDomainType();
	}
	else
	{
		return NO_DOMAIN;
	}
}


RouteTypes CvSelectionGroup::getBestBuildRoute(CvPlot* pPlot, BuildTypes* peBestBuild) const
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	RouteTypes eRoute;
	RouteTypes eBestRoute;
	int iValue;
	int iBestValue;
	int iI;

	if (peBestBuild != NULL)
	{
		*peBestBuild = NO_BUILD;
	}

	iBestValue = 0;
	eBestRoute = NO_ROUTE;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
		{
			eRoute = ((RouteTypes)(GC.getBuildInfo((BuildTypes) iI).getRoute()));

			if (eRoute != NO_ROUTE)
			{
				if (pLoopUnit->canBuild(pPlot, ((BuildTypes)iI)))
				{
					iValue = GC.getRouteInfo(eRoute).getValue();

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						eBestRoute = eRoute;
						if (peBestBuild != NULL)
						{
							*peBestBuild = ((BuildTypes)iI);
						}
					}
				}
			}
		}
	}

	return eBestRoute;
}


// Returns true if group was bumped...
bool CvSelectionGroup::groupDeclareWar(CvPlot* pPlot, bool bForce)
{
	CvTeamAI& kTeam = GET_TEAM(getTeam());
	TeamTypes ePlotTeam = pPlot->getTeam();
	
	if (!AI_isDeclareWar(pPlot))
	{
		return false;
	}

	int iNumUnits = getNumUnits();

	if (bForce || !canEnterArea(ePlotTeam, pPlot->area(), true))
	{
		if (ePlotTeam != NO_TEAM && kTeam.AI_isSneakAttackReady(ePlotTeam))
		{
			if (kTeam.canDeclareWar(ePlotTeam))
			{
				kTeam.declareWar(ePlotTeam, true, NO_WARPLAN);
			}
		}
	}

	return (iNumUnits != getNumUnits());
}


// Returns true if attack was made...
bool CvSelectionGroup::groupAttack(int iX, int iY, int iFlags, bool& bFailedAlreadyFighting)
{
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - SA: Stack Attack START
	if (GC.getDCM_STACK_ATTACK())
	{
		return groupStackAttack(iX, iY, iFlags, bFailedAlreadyFighting);
	}
	// Dale - SA: Stack Attack END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* RevolutionDCM                               04/19/09                           Glider1       */
/************************************************************************************************/
	// RevolutionDCM - attack support
	CvPlot* pOrigPlot = plot();
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pDestPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	// RevolutionDCM - end
/************************************************************************************************/
/* RevolutionDCM                               END                                Glider1       */
/************************************************************************************************/

	if (iFlags & MOVE_THROUGH_ENEMY)
	{
		if (generatePath(plot(), pDestPlot, iFlags))
		{
			pDestPlot = getPathFirstPlot();
		}
	}


	FAssertMsg(pDestPlot != NULL, "DestPlot is not assigned a valid value");

	bool bStack = (isHuman() && ((getDomainType() == DOMAIN_AIR) || GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_STACK_ATTACK)));
    
	bool bAttack = false;
	bFailedAlreadyFighting = false;

	if (getNumUnits() > 0)
	{
		if ((getDomainType() == DOMAIN_AIR) || (stepDistance(getX(), getY(), pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE()) == 1))
		{
			if ((iFlags & MOVE_DIRECT_ATTACK) || (getDomainType() == DOMAIN_AIR) || (iFlags & MOVE_THROUGH_ENEMY) || (generatePath(plot(), pDestPlot, iFlags) && (getPathFirstPlot() == pDestPlot)))
			{
				int iAttackOdds;
				CvUnit* pBestAttackUnit = AI_getBestGroupAttacker(pDestPlot, true, iAttackOdds);

				if (pBestAttackUnit)
				{
					// if there are no defenders, do not attack
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Lead From Behind                                                                             */
/************************************************************************************************/
// From Lead From Behind by UncutDragon
/* original code
					CvUnit* pBestDefender = pDestPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), pBestAttackUnit, true);
					if (NULL == pBestDefender)
					{
						return false;
					}
*/					// modified
					if (!pDestPlot->hasDefender(false, NO_PLAYER, getOwnerINLINE(), pBestAttackUnit, true))
						return false;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

// BUG - Safe Move - start
					if (isHuman())
					{
						if (!(isLastPathPlotVisible()) && (getDomainType() != DOMAIN_AIR))
						{
							return false;
						}
					}
// BUG - Safe Move - end

					bool bNoBlitz = (!pBestAttackUnit->isBlitz() || !pBestAttackUnit->isMadeAttack());

					if (groupDeclareWar(pDestPlot))
					{
						return true;
					}

/************************************************************************************************/
/* DCM	                  Start		 05/31/10                        Johnny Smith               */
/*                                                                   Afforess                   */
/* Battle Effects                                                                               */
/************************************************************************************************/
					// Dale - BE: Battle Effect START
					CvUnit* pBestDefender = pDestPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), pBestAttackUnit, true);
					if (NULL == pBestDefender)
					{
						return false;
					}
					pBestAttackUnit->setBattlePlot(pDestPlot, pBestDefender);
/************************************************************************************************/
/* DCM                      Battle Effects END                                                  */
/************************************************************************************************/
					// RevolutionDCM - attack support
					if (GC.getDCM_ATTACK_SUPPORT())
					{
						if (pDestPlot->getNumUnits() > 0)
						{
							pUnitNode = pDestPlot->headUnitNode();
							while (pUnitNode != NULL)
							{
								pLoopUnit = ::getUnit(pUnitNode->m_data);
								pUnitNode = pDestPlot->nextUnitNode(pUnitNode);
								if (GET_TEAM(pLoopUnit->getTeam()).isAtWar(getTeam()))
								{
									if (pLoopUnit != NULL && this != NULL && pDestPlot != NULL && plot() != NULL)
									{
										if (pLoopUnit->canArcherBombardAt(pDestPlot, plot()->getX_INLINE(), plot()->getY_INLINE()))
										{
											if (pLoopUnit->archerBombard(plot()->getX_INLINE(), plot()->getY_INLINE(), true))
											{
											}
										}
										else if (pLoopUnit->canBombardAtRanged(pDestPlot, plot()->getX_INLINE(), plot()->getY_INLINE()))
										{
											if (pLoopUnit->bombardRanged(plot()->getX_INLINE(), plot()->getY_INLINE(), true))
											{
											}
										}
										else if (pLoopUnit->getDomainType() == DOMAIN_AIR && !pLoopUnit->isSuicide())
										{
											pLoopUnit->updateAirStrike(plot(), false, true);//airStrike(plot()))
										}
										pLoopUnit->setMadeAttack(false);
									}
								}
							}
						} else {
							return bAttack;
						}
						if (pOrigPlot->getNumUnits() > 0)
						{
							pUnitNode = pOrigPlot->headUnitNode();
							while (pUnitNode != NULL)
							{
								pLoopUnit = ::getUnit(pUnitNode->m_data);
								pUnitNode = pOrigPlot->nextUnitNode(pUnitNode);
								if (pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
								{
									if (pLoopUnit != NULL && this != NULL && pDestPlot != NULL && plot() != NULL)
									{
										if (pLoopUnit->canArcherBombardAt(plot(), pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE()))
										{
											if (pLoopUnit->archerBombard(pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE(), true))
											{
											}
										}
										else if (pLoopUnit->canBombardAtRanged(plot(), pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE()))
										{
											if (pLoopUnit->bombardRanged(pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE(), true))
											{
											}
										}
										else if (pLoopUnit->getDomainType() == DOMAIN_AIR && !pLoopUnit->isSuicide())
										{
											if (plotDistance(plot()->getX_INLINE(), plot()->getY_INLINE(), pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE()) == 1)
											{
												pLoopUnit->updateAirStrike(pDestPlot, false, false);//airStrike(pDestPlot))
											}
										}
										pLoopUnit->setMadeAttack(false);
									}
								}
							}
						} else {
							return bAttack;
						}
					}
					// RevolutionDCM - attack support end
/************************************************************************************************/
/* RevolutionDCM                               END                                              */
/************************************************************************************************/
					while (true)
					{
						pBestAttackUnit = AI_getBestGroupAttacker(pDestPlot, false, iAttackOdds, false, bNoBlitz);
						if (pBestAttackUnit == NULL)
						{
							break;
						}

						if (iAttackOdds < 68)
						{
						    CvUnit * pBestSacrifice = AI_getBestGroupSacrifice(pDestPlot, false, false, bNoBlitz);
						    if (pBestSacrifice != NULL)
						    {
                                pBestAttackUnit = pBestSacrifice;
						    }
						}

						bAttack = true;
/************************************************************************************************/
/* Afforess	                  Start		 04/29/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
						if (GC.getUSE_CAN_DO_COMBAT_CALLBACK())
						{
							CySelectionGroup* pyGroup = new CySelectionGroup(this);
							CyPlot* pyPlot = new CyPlot(pDestPlot);
							CyArgsList argsList;
							argsList.add(gDLL->getPythonIFace()->makePythonObject(pyGroup));	// pass in Selection Group class
							argsList.add(gDLL->getPythonIFace()->makePythonObject(pyPlot));	// pass in Plot class
							long lResult=0;
							gDLL->getPythonIFace()->callFunction(PYGameModule, "doCombat", argsList.makeFunctionArgs(), &lResult);
							delete pyGroup;	// python fxn must not hold on to this pointer 
							delete pyPlot;	// python fxn must not hold on to this pointer 
							if (lResult == 1)
							{
								break;
							} 
						}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

						if (getNumUnits() > 1)
						{
							if (pBestAttackUnit->plot()->isFighting() || pDestPlot->isFighting())
							{
								bFailedAlreadyFighting = true;
							}
							else
							{
								pBestAttackUnit->attack(pDestPlot, bStack);
							}
						}
						else
						{
							pBestAttackUnit->attack(pDestPlot, false);
							break;
						}

						if (bFailedAlreadyFighting || !bStack)
						{
							// if this is AI stack, follow through with the attack to the end
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/* Allow Automated Units to Stack Attack                                                        */
/************************************************************************************************/
/*
							if (!isHuman() && getNumUnits() > 1)
*/
							if ((!isHuman() || isAutomated()) && getNumUnits() > 1)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
							{
								AI_queueGroupAttack(iX, iY);
							}
							
							break;
						}
					}
				}
			}
		}
	}

	return bAttack;
}


void CvSelectionGroup::groupMove(CvPlot* pPlot, bool bCombat, CvUnit* pCombatUnit, bool bEndMove)
{
	//PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	pUnitNode = headUnitNode();
	
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
	bool bSentryAlert = isHuman() && NULL != headMissionQueueNode() && headMissionQueueNode()->m_data.eMissionType == MISSION_MOVE_TO_SENTRY && sentryAlertSameDomainType();
#endif
// BUG - Sentry Actions - end

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
		// don't move if bSentryAlert set to true above
		if ((!bSentryAlert && pLoopUnit->canMove() && ((bCombat && (!(pLoopUnit->isNoCapture()) || !(pPlot->isEnemyCity(*pLoopUnit)))) ? pLoopUnit->canMoveOrAttackInto(pPlot) : pLoopUnit->canMoveInto(pPlot))) || (pLoopUnit == pCombatUnit))
#else
		if ((pLoopUnit->canMove() && ((bCombat && (!(pLoopUnit->isNoCapture()) || !(pPlot->isEnemyCity(*pLoopUnit)))) ? pLoopUnit->canMoveOrAttackInto(pPlot) : pLoopUnit->canMoveInto(pPlot))) || (pLoopUnit == pCombatUnit))
#endif
// BUG - Sentry Actions - end
		{
			pLoopUnit->move(pPlot, true);
		}
		else
		{
			pLoopUnit->joinGroup(NULL, true);
			pLoopUnit->ExecuteMove(((float)(GC.getMissionInfo(MISSION_MOVE_TO).getTime() * gDLL->getMillisecsPerTurn())) / 1000.0f, false);
/************************************************************************************************/
/* Afforess	                  Start		 07/31/10                                               */
/*                                                                                              */
/* Units Seem to be getting stuck here                                                          */
/************************************************************************************************/
			if (GC.iStuckUnitID != pLoopUnit->getID())
			{
				GC.iStuckUnitID = pLoopUnit->getID();
				GC.iStuckUnitCount = 0;
			}
			else
			{
				GC.iStuckUnitCount++;
				if (GC.iStuckUnitCount > 5)
				{
					FAssertMsg(false, "Unit Stuck in Loop!");
					CvUnit* pHeadUnit = getHeadUnit();
					if (NULL != pHeadUnit)
					{
						TCHAR szOut[1024];
						CvWString szTempString;
						getUnitAIString(szTempString, pHeadUnit->AI_getUnitAIType());
						sprintf(szOut, "Unit stuck in loop: %S(%S)[%d, %d] (%S)\n", pHeadUnit->getName().GetCString(), GET_PLAYER(pHeadUnit->getOwnerINLINE()).getName(),
							pHeadUnit->getX_INLINE(), pHeadUnit->getY_INLINE(), szTempString.GetCString());
						gDLL->messageControlLog(szOut);
					}
					pLoopUnit->finishMoves();
				}
			}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			
		}
	}

	//execute move
	if(bEndMove || !canAllMove())
	{
		pUnitNode = headUnitNode();
		while(pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);

			pLoopUnit->ExecuteMove(((float)(GC.getMissionInfo(MISSION_MOVE_TO).getTime() * gDLL->getMillisecsPerTurn())) / 1000.0f, false);
		}
	}
}


// Returns true if move was made...
bool CvSelectionGroup::groupPathTo(int iX, int iY, int iFlags)
{
	CvPlot* pDestPlot;
	CvPlot* pPathPlot;

	if (at(iX, iY))
	{
		return false; // XXX is this necessary?
	}

	FAssert(!isBusy());
	FAssert(getOwnerINLINE() != NO_PLAYER);
	FAssert(headMissionQueueNode() != NULL);

	pDestPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	FAssertMsg(pDestPlot != NULL, "DestPlot is not assigned a valid value");

	FAssertMsg(canAllMove(), "canAllMove is expected to be true");

	if (getDomainType() == DOMAIN_AIR)
	{
		if (!canMoveInto(pDestPlot))
		{
			return false;
		}

		pPathPlot = pDestPlot;
	}
	else
	{
		if (!generatePath(plot(), pDestPlot, iFlags))
		{
			return false;
		}

		pPathPlot = getPathFirstPlot();

		if (groupAmphibMove(pPathPlot, iFlags))
		{
			return false;
		}
	}
	
	bool bForce = false;
	MissionAITypes eMissionAI = AI_getMissionAIType();
	
	/*** Dexy - Fixed Borders START ****/
	if (eMissionAI == MISSIONAI_BLOCKADE || eMissionAI == MISSIONAI_PILLAGE || eMissionAI == MISSIONAI_CLAIM_TERRITORY)
//	OLD CODE
//	if (eMissionAI == MISSIONAI_BLOCKADE || eMissionAI == MISSIONAI_PILLAGE)
	/*** Dexy - Fixed Borders  END  ****/
	{
		bForce = true;
	}

	if (groupDeclareWar(pPathPlot, bForce))
	{
		return false;
	}

	bool bEndMove = false;
	if(pPathPlot == pDestPlot)
		bEndMove = true;
    
	groupMove(pPathPlot, iFlags & MOVE_THROUGH_ENEMY, NULL, bEndMove);

	return true;
}


// Returns true if move was made...
bool CvSelectionGroup::groupRoadTo(int iX, int iY, int iFlags)
{
	CvPlot* pPlot;
	RouteTypes eBestRoute;
	BuildTypes eBestBuild;

	if (!AI_isControlled() || !at(iX, iY) || (getLengthMissionQueue() == 1))
	{
		pPlot = plot();

		eBestRoute = getBestBuildRoute(pPlot, &eBestBuild);

		if (eBestBuild != NO_BUILD)
		{
			groupBuild(eBestBuild);
			return true;
		}
	}

	return groupPathTo(iX, iY, iFlags);
}


// Returns true if build should continue...
bool CvSelectionGroup::groupBuild(BuildTypes eBuild)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;
	bool bContinue;

	FAssert(getOwnerINLINE() != NO_PLAYER);
	FAssertMsg(eBuild < GC.getNumBuildInfos(), "Invalid Build");

	bContinue = false;

	pPlot = plot();

    ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
	if (eImprovement != NO_IMPROVEMENT)
	{
		if (AI_isControlled())
		{
			if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION))
			{
				if ((pPlot->getImprovementType() != NO_IMPROVEMENT) && (pPlot->getImprovementType() != (ImprovementTypes)(GC.getDefineINT("RUINS_IMPROVEMENT"))))
				{
				    BonusTypes eBonus = (BonusTypes)pPlot->getNonObsoleteBonusType(GET_PLAYER(getOwnerINLINE()).getTeam());
				    if ((eBonus == NO_BONUS) || !GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eBonus))
				    {
                        if (GC.getImprovementInfo(eImprovement).getImprovementPillage() != NO_IMPROVEMENT)
                        {
                            return false;
                        }
				    }
				}
			}
//
//			if (AI_getMissionAIType() == MISSION_BUILD)
//			{
//                CvCity* pWorkingCity = pPlot->getWorkingCity();
//                if ((pWorkingCity != NULL) && (AI_getMissionAIPlot() == pPlot))
//                {
//                    if (pWorkingCity->AI_getBestBuild(pWorkingCity->getCityPlotIndex(pPlot)) != eBuild)
//                    {
//                        return false;
//                    }
//                }
//			}
		}
	}
	
// BUG - Pre-Chop - start
	bool bCheckChop = false;

	FeatureTypes eFeature = pPlot->getFeatureType();
	CvBuildInfo& kBuildInfo = GC.getBuildInfo(eBuild);
	if (eFeature != NO_FEATURE && isHuman() && kBuildInfo.isFeatureRemove(eFeature) && kBuildInfo.getFeatureProduction(eFeature) != 0)
	{
		if (kBuildInfo.getImprovement() == NO_IMPROVEMENT)
		{
			// clearing a forest or jungle
			if (getBugOptionBOOL("Actions__PreChopForests", true, "BUG_PRECHOP_FORESTS"))
			{
				bCheckChop = true;
			}
		}
		else
		{
			if (getBugOptionBOOL("Actions__PreChopImprovements", true, "BUG_PRECHOP_IMPROVEMENTS"))
			{
				bCheckChop = true;
			}
		}
	}
// BUG - Pre-Chop - end

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		FAssertMsg(pLoopUnit->atPlot(pPlot), "pLoopUnit is expected to be at pPlot");

		if (pLoopUnit->canBuild(pPlot, eBuild))
		{
			bContinue = true;

			if (pLoopUnit->build(eBuild))
			{
				bContinue = false;
				break;
			}

// BUG - Pre-Chop - start
			if (bCheckChop && pPlot->getBuildTurnsLeft(eBuild, getOwnerINLINE()) == 1)
			{
				// TODO: stop other worker groups
				CvCity* pCity;
				int iProduction = plot()->getFeatureProduction(eBuild, getTeam(), &pCity);

				if (iProduction > 0)
				{
					CvWString szBuffer = gDLL->getText("TXT_KEY_BUG_PRECLEARING_FEATURE_BONUS", GC.getFeatureInfo(eFeature).getTextKeyWide(), iProduction, pCity->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer,  ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_CITY_EDIT")->getPath(), MESSAGE_TYPE_INFO, GC.getFeatureInfo(eFeature).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), getX(), getY(), true, true);
				}
				bContinue = false;
				break;
			}
// BUG - Pre-Chop - end
		}
	}

	return bContinue;
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/18/10                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
void CvSelectionGroup::setTransportUnit(CvUnit* pTransportUnit, CvSelectionGroup** pOtherGroup)
{
	// if we are loading
	if (pTransportUnit != NULL)
	{
		CvUnit* pHeadUnit = getHeadUnit();
		FAssertMsg(pHeadUnit != NULL, "non-zero group without head unit");
		
		int iCargoSpaceAvailable = pTransportUnit->cargoSpaceAvailable(pHeadUnit->getSpecialUnitType(), pHeadUnit->getDomainType());
		
		// if no space at all, give up
		if (iCargoSpaceAvailable < 1)
		{
			return;
		}

		// if there is space, but not enough to fit whole group, then split us, and set on the new group
		if (iCargoSpaceAvailable < getNumUnits())
		{
			CvSelectionGroup* pSplitGroup = splitGroup(iCargoSpaceAvailable, NULL, pOtherGroup);
			if (pSplitGroup != NULL)
			{
				pSplitGroup->setTransportUnit(pTransportUnit);
			}
			return;
		}
		
		FAssertMsg(iCargoSpaceAvailable >= getNumUnits(), "cargo size too small");
		
		// setTransportUnit removes the unit from the current group (at least currently), so we have to be careful in the loop here
		// so, loop until we do not load one
		bool bLoadedOne;
		do
		{
			bLoadedOne = false;

			// loop over all the units, finding one to load
			CLLNode<IDInfo>* pUnitNode = headUnitNode();
			while (pUnitNode != NULL && !bLoadedOne)
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = nextUnitNode(pUnitNode);
				
				// just in case implementation of setTransportUnit changes, check to make sure this unit is not already loaded
				if (pLoopUnit != NULL && pLoopUnit->getTransportUnit() != pTransportUnit)
				{
					// if there is room, load the unit and stop the loop (since setTransportUnit ungroups this unit currently)
					bool bSpaceAvailable = pTransportUnit->cargoSpaceAvailable(pLoopUnit->getSpecialUnitType(), pLoopUnit->getDomainType());
					if (bSpaceAvailable)
					{
						pLoopUnit->setTransportUnit(pTransportUnit);
						bLoadedOne = true;

					}
				}
			}
		}
		while (bLoadedOne);
	}
	// otherwise we are unloading
	else
	{
		// loop over all the units, unloading them
		CLLNode<IDInfo>* pUnitNode = headUnitNode();
		while (pUnitNode != NULL)
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);
			
			if (pLoopUnit != NULL)
			{
				// unload unit
				pLoopUnit->setTransportUnit(NULL);
			}
		}
	}
}


/// \brief Function for loading stranded units onto an offshore transport
///
void CvSelectionGroup::setRemoteTransportUnit(CvUnit* pTransportUnit)
{
	// if we are loading
	if (pTransportUnit != NULL)
	{
		CvUnit* pHeadUnit = getHeadUnit();
		FAssertMsg(pHeadUnit != NULL, "non-zero group without head unit");
		
		int iCargoSpaceAvailable = pTransportUnit->cargoSpaceAvailable(pHeadUnit->getSpecialUnitType(), pHeadUnit->getDomainType());
		
		// if no space at all, give up
		if (iCargoSpaceAvailable < 1)
		{
			return;
		}

		// if there is space, but not enough to fit whole group, then split us, and set on the new group
		if (iCargoSpaceAvailable < getNumUnits())
		{
			CvSelectionGroup* pSplitGroup = splitGroup(iCargoSpaceAvailable);
			if (pSplitGroup != NULL)
			{
				pSplitGroup->setRemoteTransportUnit(pTransportUnit);
			}
			return;
		}
		
		FAssertMsg(iCargoSpaceAvailable >= getNumUnits(), "cargo size too small");

		bool bLoadedOne;
		do
		{
			bLoadedOne = false;

			// loop over all the units on the plot, looping through this selection group did not work
			CLLNode<IDInfo>* pUnitNode = headUnitNode();
			while (pUnitNode != NULL && !bLoadedOne)
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = nextUnitNode(pUnitNode);
				
				if (pLoopUnit != NULL && pLoopUnit->getTransportUnit() != pTransportUnit && pLoopUnit->getOwnerINLINE() == pTransportUnit->getOwnerINLINE())
				{
					bool bSpaceAvailable = pTransportUnit->cargoSpaceAvailable(pLoopUnit->getSpecialUnitType(), pLoopUnit->getDomainType());
					if (bSpaceAvailable)
					{
						if( !(pLoopUnit->atPlot(pTransportUnit->plot())) )
						{
							// Putting a land unit on water automatically loads it
							pLoopUnit->setXY(pTransportUnit->getX_INLINE(),pTransportUnit->getY_INLINE());
						}

						if( pLoopUnit->getTransportUnit() != pTransportUnit ) 
						{
							pLoopUnit->setTransportUnit(pTransportUnit);
						}

						bLoadedOne = true;
					}
				}
			}
		}
		while (bLoadedOne);
	}
	// otherwise we are unloading
	else
	{
		// loop over all the units, unloading them
		CLLNode<IDInfo>* pUnitNode = headUnitNode();
		while (pUnitNode != NULL)
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);
			
			if (pLoopUnit != NULL)
			{
				// unload unit
				pLoopUnit->setTransportUnit(NULL);
			}
		}
	}
}

bool CvSelectionGroup::isAmphibPlot(const CvPlot* pPlot) const
{
	bool bFriendly = true;
	CvUnit* pUnit = getHeadUnit();
	if (NULL != pUnit)
	{
		bFriendly = pPlot->isFriendlyCity(*pUnit, true);
	}

	//return ((getDomainType() == DOMAIN_SEA) && pPlot->isCoastalLand() && !bFriendly && !canMoveAllTerrain());

	if (getDomainType() == DOMAIN_SEA)
	{
		if (pPlot->isCity() && !bFriendly && (pPlot->isCoastalLand() || pPlot->isWater() || canMoveAllTerrain()))
		{
			return true;
		}
		return (pPlot->isCoastalLand() && !bFriendly && !canMoveAllTerrain());
	}
	return false;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

// Returns true if attempted an amphib landing...
bool CvSelectionGroup::groupAmphibMove(CvPlot* pPlot, int iFlags)
{
	CLLNode<IDInfo>* pUnitNode1;
	CvUnit* pLoopUnit1;
	bool bLanding = false;

	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (groupDeclareWar(pPlot))
	{
		return true;
	}

	if (isAmphibPlot(pPlot))
	{
// BUG - Safe Move - start
		// don't perform amphibious landing on plot that was unrevealed when goto order was issued
		if (isHuman())
		{
			if (!isLastPathPlotRevealed())
			{
				return false;
			}
		}
// BUG - Safe Move - end

		if (stepDistance(getX(), getY(), pPlot->getX_INLINE(), pPlot->getY_INLINE()) == 1)
		{
			pUnitNode1 = headUnitNode();

			// BBAI TODO: Bombard with warships if invading

			while (pUnitNode1 != NULL)
			{
				pLoopUnit1 = ::getUnit(pUnitNode1->m_data);
				pUnitNode1 = nextUnitNode(pUnitNode1);

				if ((pLoopUnit1->getCargo() > 0) && (pLoopUnit1->domainCargo() == DOMAIN_LAND))
				{
					std::vector<CvUnit*> aCargoUnits;
					pLoopUnit1->getCargoUnits(aCargoUnits);
					std::vector<CvSelectionGroup*> aCargoGroups;
					for (uint i = 0; i < aCargoUnits.size(); ++i)
					{
						CvSelectionGroup* pGroup = aCargoUnits[i]->getGroup();
						if (std::find(aCargoGroups.begin(), aCargoGroups.end(), pGroup) == aCargoGroups.end())
						{
							aCargoGroups.push_back(aCargoUnits[i]->getGroup());
						}
					}

					for (uint i = 0; i < aCargoGroups.size(); ++i)
					{
						CvSelectionGroup* pGroup = aCargoGroups[i];
						if (pGroup->canAllMove())
						{
							FAssert(!pGroup->at(pPlot->getX_INLINE(), pPlot->getY_INLINE()));
							pGroup->pushMission(MISSION_MOVE_TO, pPlot->getX_INLINE(), pPlot->getY_INLINE(), (MOVE_IGNORE_DANGER | iFlags));
							bLanding = true;
						}
					}
				}
			}
		}
	}

	return bLanding;
}


bool CvSelectionGroup::readyToSelect(bool bAny)
{
	return (readyToMove(bAny) && !isAutomated());
}


bool CvSelectionGroup::readyToMove(bool bAny)
{
	return (((bAny) ? canAnyMove() : canAllMove()) && (headMissionQueueNode() == NULL) && (getActivityType() == ACTIVITY_AWAKE) && !isBusy() && !isCargoBusy());
}


bool CvSelectionGroup::readyToAuto()
{
	return (canAllMove() && (headMissionQueueNode() != NULL));
}


int CvSelectionGroup::getID() const
{
	return m_iID;
}


void CvSelectionGroup::setID(int iID)
{
	m_iID = iID;
}


PlayerTypes CvSelectionGroup::getOwner() const
{
	return getOwnerINLINE();
}


TeamTypes CvSelectionGroup::getTeam() const
{
	if (getOwnerINLINE() != NO_PLAYER)
	{
		return GET_PLAYER(getOwnerINLINE()).getTeam();
	}

	return NO_TEAM;
}


int CvSelectionGroup::getMissionTimer() const
{
	return m_iMissionTimer;
}


void CvSelectionGroup::setMissionTimer(int iNewValue)
{
	FAssert(getOwnerINLINE() != NO_PLAYER);

	m_iMissionTimer = iNewValue;
	FAssert(getMissionTimer() >= 0);
}


void CvSelectionGroup::changeMissionTimer(int iChange)
{
	setMissionTimer(getMissionTimer() + iChange);
}


void CvSelectionGroup::updateMissionTimer(int iSteps)
{
	CvUnit* pTargetUnit;
	CvPlot* pTargetPlot;
	int iTime;

	if (!isHuman() && !showMoves())
	{
		iTime = 0;
	}
	else if (headMissionQueueNode() != NULL)
	{
		iTime = GC.getMissionInfo((MissionTypes)(headMissionQueueNode()->m_data.eMissionType)).getTime();

		if ((headMissionQueueNode()->m_data.eMissionType == MISSION_MOVE_TO) ||
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
				(headMissionQueueNode()->m_data.eMissionType == MISSION_MOVE_TO_SENTRY) ||
#endif
// BUG - Sentry Actions - end
				(headMissionQueueNode()->m_data.eMissionType == MISSION_ROUTE_TO) ||
				(headMissionQueueNode()->m_data.eMissionType == MISSION_MOVE_TO_UNIT))
		{
			if (headMissionQueueNode()->m_data.eMissionType == MISSION_MOVE_TO_UNIT)
			{
				pTargetUnit = GET_PLAYER((PlayerTypes)headMissionQueueNode()->m_data.iData1).getUnit(headMissionQueueNode()->m_data.iData2);
				if (pTargetUnit != NULL)
				{
					pTargetPlot = pTargetUnit->plot();
				}
				else
				{
					pTargetPlot = NULL;
				}
			}
			else
			{
				pTargetPlot = GC.getMapINLINE().plotINLINE(headMissionQueueNode()->m_data.iData1, headMissionQueueNode()->m_data.iData2);
			}

			if (atPlot(pTargetPlot))
			{
				iTime += iSteps;
			}
			else
			{
				iTime = std::min(iTime, 2);
			}
		}

		if (isHuman() && (isAutomated() || (GET_PLAYER((GC.getGameINLINE().isNetworkMultiPlayer()) ? getOwnerINLINE() : GC.getGameINLINE().getActivePlayer()).isOption(PLAYEROPTION_QUICK_MOVES))))
		{
			iTime = std::min(iTime, 1);
		}
	}
	else
	{
		iTime = 0;
	}

	setMissionTimer(iTime);
}


bool CvSelectionGroup::isForceUpdate()
{
	return m_bForceUpdate;
}


void CvSelectionGroup::setForceUpdate(bool bNewValue)
{
	m_bForceUpdate = bNewValue;
}


ActivityTypes CvSelectionGroup::getActivityType() const
{
	return m_eActivityType;
}


void CvSelectionGroup::setActivityType(ActivityTypes eNewValue)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;

	FAssert(getOwnerINLINE() != NO_PLAYER);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/28/10                                jdog5000      */
/*                                                                                              */
/* Unit AI                                                                                      */
/************************************************************************************************/
//	FAssert(isHuman() || getHeadUnit()->isCargo() || eNewValue != ACTIVITY_SLEEP);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	ActivityTypes eOldActivity = getActivityType();

	if (eOldActivity != eNewValue)
	{
		pPlot = plot();

		if (eOldActivity == ACTIVITY_INTERCEPT)
		{
			airCircle(false);
		}

		setBlockading(false);

		m_eActivityType = eNewValue;

		if (getActivityType() == ACTIVITY_INTERCEPT)
		{
			airCircle(true);
		}

		if (getActivityType() != ACTIVITY_MISSION)
		{
			pUnitNode = headUnitNode();

			if (getActivityType() != ACTIVITY_INTERCEPT)
			{
				//don't idle intercept animation
				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = nextUnitNode(pUnitNode);

					pLoopUnit->NotifyEntity(MISSION_IDLE);
				}
			}

			if (getTeam() == GC.getGameINLINE().getActiveTeam())
			{
				if (pPlot != NULL)
				{
					pPlot->setFlagDirty(true);
				}
			}
		}

		if (pPlot == gDLL->getInterfaceIFace()->getSelectionPlot())
		{
			gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		}
	}
}


AutomateTypes CvSelectionGroup::getAutomateType() const														
{
	return m_eAutomateType;
}


bool CvSelectionGroup::isAutomated()
{
	return (getAutomateType() != NO_AUTOMATE);
}


void CvSelectionGroup::setAutomateType(AutomateTypes eNewValue)
{
	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (getAutomateType() != eNewValue)
	{
		m_eAutomateType = eNewValue;

		clearMissionQueue();
		setActivityType(ACTIVITY_AWAKE);

		// if canceling automation, cancel on cargo as well
		if (eNewValue == NO_AUTOMATE)
		{
			CvPlot* pPlot = plot();
			if (pPlot != NULL)
			{
				CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
				while (pUnitNode != NULL)
				{
					CvUnit* pCargoUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pPlot->nextUnitNode(pUnitNode);
					
					CvUnit* pTransportUnit = pCargoUnit->getTransportUnit();
					if (pTransportUnit != NULL && pTransportUnit->getGroup() == this)
					{
						pCargoUnit->getGroup()->setAutomateType(NO_AUTOMATE);
						pCargoUnit->getGroup()->setActivityType(ACTIVITY_AWAKE);
					}
				}
			}
		}
	}
}


FAStarNode* CvSelectionGroup::getPathLastNode() const
{
	return gDLL->getFAStarIFace()->GetLastNode(&GC.getPathFinder());
}


CvPlot* CvSelectionGroup::getPathFirstPlot() const
{
	FAStarNode* pNode;

	pNode = getPathLastNode();

	if (pNode->m_pParent == NULL)
	{
		return GC.getMapINLINE().plotSorenINLINE(pNode->m_iX, pNode->m_iY);
	}

	while (pNode != NULL)
	{
		if (pNode->m_pParent->m_pParent == NULL)
		{
			return GC.getMapINLINE().plotSorenINLINE(pNode->m_iX, pNode->m_iY);
		}

		pNode = pNode->m_pParent;
	}

	FAssert(false);

	return NULL;
}


CvPlot* CvSelectionGroup::getPathEndTurnPlot() const
{
	FAStarNode* pNode;

	pNode = getPathLastNode();

	if (NULL != pNode)
	{
		if ((pNode->m_pParent == NULL) || (pNode->m_iData2 == 1))
		{
			return GC.getMapINLINE().plotSorenINLINE(pNode->m_iX, pNode->m_iY);
		}

		while (pNode->m_pParent != NULL)
		{
			if (pNode->m_pParent->m_iData2 == 1)
			{
				return GC.getMapINLINE().plotSorenINLINE(pNode->m_pParent->m_iX, pNode->m_pParent->m_iY);
			}

			pNode = pNode->m_pParent;
		}
	}

	FAssert(false);

	return NULL;
}


bool CvSelectionGroup::generatePath( const CvPlot* pFromPlot, const CvPlot* pToPlot, int iFlags, bool bReuse, int* piPathTurns) const
{
	PROFILE("CvSelectionGroup::generatePath()")

	FAStarNode* pNode;
	bool bSuccess;
/************************************************************************************************/
/* Afforess	                  Start		 04/09/10                                               */
/*                                                                                              */
/* CTD Fix                                                                                      */
/************************************************************************************************/
	if (pFromPlot == NULL || pToPlot == NULL) return false;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	gDLL->getFAStarIFace()->SetData(&GC.getPathFinder(), this);

	bSuccess = gDLL->getFAStarIFace()->GeneratePath(&GC.getPathFinder(), pFromPlot->getX_INLINE(), pFromPlot->getY_INLINE(), pToPlot->getX_INLINE(), pToPlot->getY_INLINE(), false, iFlags, bReuse);

	if (piPathTurns != NULL)
	{
		*piPathTurns = MAX_INT;

		if (bSuccess)
		{
			pNode = getPathLastNode();

			if (pNode != NULL)
			{
				*piPathTurns = pNode->m_iData2;
			}
		}
	}

	return bSuccess;
}

CvPathGenerator*	CvSelectionGroup::getPathGenerator()
{
	if ( m_generator == NULL )
	{
		m_generator = new	CvPathGenerator(&GC.getMapINLINE());

		// gvd m_generator->Initialize(NewPathHeuristicFunc, NewPathCostFunc, NewPathValidFunc, NewPathDestValid);
	}

	return m_generator;
}



void CvSelectionGroup::setGroupToCacheFor(CvSelectionGroup* group)
{
	MEMORY_TRACK_EXEMPT();
	PROFILE_FUNC();

	m_pCachedMovementGroup = group;

	if ( m_pCachedNonEndTurnEdgeCosts == NULL )
	{
		m_pCachedNonEndTurnEdgeCosts = new std::map<int,edgeCosts>();
	}
	if ( m_pCachedEndTurnEdgeCosts == NULL )
	{
		m_pCachedEndTurnEdgeCosts = new std::map<int,edgeCosts>();
	}

	m_pCachedNonEndTurnEdgeCosts->clear();
	m_pCachedEndTurnEdgeCosts->clear();
}

bool CvSelectionGroup::HaveCachedPathEdgeCosts(CvPlot* pFromPlot, CvPlot* pToPlot, bool bIsEndTurnElement, int& iResult, int& iBestMoveCost, int& iWorstMoveCost, int& iToPlotNodeCost)
{
	//	Could use Zobrist hashes of the plots, but actually since we're only combining two sets of coordinates we can
	//	fit it all in an int for any reasonable map
	int cacheKey = GC.getMapINLINE().plotNumINLINE(pFromPlot->getX_INLINE(),pFromPlot->getY_INLINE()) + (GC.getMapINLINE().plotNumINLINE(pToPlot->getX_INLINE(),pToPlot->getY_INLINE()) << 16);

	if ( this != m_pCachedMovementGroup )
	{
		return false;
	}

	if ( bIsEndTurnElement )
	{
		std::map<int,edgeCosts>::const_iterator itr = m_pCachedEndTurnEdgeCosts->find(cacheKey);

		if ( itr == m_pCachedEndTurnEdgeCosts->end() )
		{
			return false;
		}
		else
		{
			iResult = (itr->second).iCost;
			iBestMoveCost = (itr->second).iBestMoveCost;
			iWorstMoveCost = (itr->second).iWorstMoveCost;
			iToPlotNodeCost = (itr->second).iToPlotNodeCost;
#ifdef _DEBUG
			FAssert((itr->second).pFromPlot == pFromPlot);
			FAssert((itr->second).pToPlot == pToPlot);
#endif
			return true;
		}
	}
	else
	{
		std::map<int,edgeCosts>::const_iterator itr = m_pCachedNonEndTurnEdgeCosts->find(cacheKey);

		if ( itr == m_pCachedNonEndTurnEdgeCosts->end() )
		{
			return false;
		}
		else
		{
			iResult = (itr->second).iCost;
			iBestMoveCost = (itr->second).iBestMoveCost;
			iWorstMoveCost = (itr->second).iWorstMoveCost;
			iToPlotNodeCost = (itr->second).iToPlotNodeCost;
#ifdef _DEBUG
			FAssert((itr->second).pFromPlot == pFromPlot);
			FAssert((itr->second).pToPlot == pToPlot);
#endif
			return true;
		}
	}
}

void CvSelectionGroup::CachePathEdgeCosts(CvPlot* pFromPlot, CvPlot* pToPlot, bool bIsEndTurnElement, int iCost, int iBestMoveCost, int iWorstMoveCost, int iToPlotNodeCost)
{
	MEMORY_TRACK_EXEMPT();

	if ( this == m_pCachedMovementGroup )
	{
		//	Could use Zobrist hashes of the plots, but actually since we're only combining two sets of coordinates we can
		//	fit it all in an int for any reasonable map
		FAssert(GC.getMapINLINE().getGridHeightINLINE()*GC.getMapINLINE().getGridHeightINLINE()*GC.getMapINLINE().getGridWidthINLINE()*(GC.getMapINLINE().getGridWidthINLINE()/2) < MAXINT);
		int cacheKey = GC.getMapINLINE().plotNumINLINE(pFromPlot->getX_INLINE(),pFromPlot->getY_INLINE()) + (GC.getMapINLINE().plotNumINLINE(pToPlot->getX_INLINE(),pToPlot->getY_INLINE()) << 16);

		edgeCosts costs;

		costs.iCost = iCost;
		costs.iBestMoveCost = iBestMoveCost;
		costs.iWorstMoveCost = iWorstMoveCost;
		costs.iToPlotNodeCost = iToPlotNodeCost;
#ifdef _DEBUG
		costs.pFromPlot = pFromPlot;
		costs.pToPlot = pToPlot;
#endif

		if ( bIsEndTurnElement )
		{
			(*m_pCachedEndTurnEdgeCosts)[cacheKey] = costs;
		}
		else
		{
			(*m_pCachedNonEndTurnEdgeCosts)[cacheKey] = costs;
		}
	}
}

void CvSelectionGroup::resetPath()
{
	gDLL->getFAStarIFace()->ForceReset(&GC.getPathFinder());
}


void CvSelectionGroup::clearUnits()
{
	CLLNode<IDInfo>* pUnitNode;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pUnitNode = deleteUnitNode(pUnitNode);
	}
}


// Returns true if the unit is added...
bool CvSelectionGroup::addUnit(CvUnit* pUnit, bool bMinimalChange)
{
	//PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	bool bAdded;
 
	if (!(pUnit->canJoinGroup(pUnit->plot(), this)))
	{
		return false;
	}

	bAdded = false;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		if ((pUnit->AI_groupFirstVal() > pLoopUnit->AI_groupFirstVal()) ||
			  ((pUnit->AI_groupFirstVal() == pLoopUnit->AI_groupFirstVal()) &&
				 (pUnit->AI_groupSecondVal() > pLoopUnit->AI_groupSecondVal())))
		{
			m_units.insertBefore(pUnit->getIDInfo(), pUnitNode);
			bAdded = true;
			break;
		}
		pUnitNode = nextUnitNode(pUnitNode);
	}

	if (!bAdded)
	{
		m_units.insertAtEnd(pUnit->getIDInfo());
	}

	if(!bMinimalChange)
	{
		if (getOwnerINLINE() == NO_PLAYER)
		{
			if (getNumUnits() > 0)
			{
				pUnitNode = headUnitNode();
				while (pUnitNode != NULL)
				{
					//if (pUnitNode != headUnitNode())
					//{
						::getUnit(pUnitNode->m_data)->NotifyEntity(MISSION_MULTI_SELECT);
					//}
					pUnitNode = nextUnitNode(pUnitNode);
				}
			}
		}
	}

	return true;
}


void CvSelectionGroup::removeUnit(CvUnit* pUnit)
{
	CLLNode<IDInfo>* pUnitNode;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		if (::getUnit(pUnitNode->m_data) == pUnit)
		{
			deleteUnitNode(pUnitNode);
			break;
		}
		else
		{
			pUnitNode = nextUnitNode(pUnitNode);
		}
	}
}


CLLNode<IDInfo>* CvSelectionGroup::deleteUnitNode(CLLNode<IDInfo>* pNode)
{
	CLLNode<IDInfo>* pNextUnitNode;

	if (getOwnerINLINE() != NO_PLAYER)
	{
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/*  Allow Automated Units to Stack Attack                                                       */
/************************************************************************************************/
/*
		setAutomateType(NO_AUTOMATE);
*/
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		clearMissionQueue();

		switch (getActivityType())
		{
		case ACTIVITY_SLEEP:
		case ACTIVITY_INTERCEPT:
		case ACTIVITY_PATROL:
		case ACTIVITY_PLUNDER:
			break;
		default:
			setActivityType(ACTIVITY_AWAKE);
			break;
		}
	}

	pNextUnitNode = m_units.deleteNode(pNode);

	return pNextUnitNode;
}


CLLNode<IDInfo>* CvSelectionGroup::nextUnitNode(CLLNode<IDInfo>* pNode) const
{
	return m_units.next(pNode);
}


int CvSelectionGroup::getNumUnits() const
{
	return m_units.getLength();
}

void CvSelectionGroup::mergeIntoGroup(CvSelectionGroup* pSelectionGroup)
{
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());

	// merge groups, but make sure we do not change the head unit AI
	// this means that if a new unit is going to become the head, change its AI to match, if possible
	// AI_setUnitAIType removes the unit from the current group (at least currently), so we have to be careful in the loop here
	// so, loop until we have not changed unit AIs
	bool bChangedUnitAI;
	do
	{
		bChangedUnitAI = false;

		// loop over all the units, moving them to the new group, 
		// stopping if we had to change a unit AI, because doing so removes that unit from our group, so we have to start over
		CLLNode<IDInfo>* pUnitNode = headUnitNode();
		while (pUnitNode != NULL && !bChangedUnitAI)
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);
			
			if (pLoopUnit != NULL)
			{
				UnitAITypes eUnitAI = pLoopUnit->AI_getUnitAIType();

				// if the unitAIs are different, and the loop unit has a higher val, then the group unitAI would change
				// change this UnitAI to the old group UnitAI if possible
				CvUnit* pNewHeadUnit = pSelectionGroup->getHeadUnit();
				UnitAITypes eNewHeadUnitAI = pSelectionGroup->getHeadUnitAI();
				if (pNewHeadUnit!= NULL && eUnitAI != eNewHeadUnitAI && pLoopUnit->AI_groupFirstVal() > pNewHeadUnit->AI_groupFirstVal())
				{
					// non-zero AI_unitValue means that this UnitAI is valid for this unit (that is the check used everywhere)
					if (kPlayer.AI_unitValue(pLoopUnit->getUnitType(), eNewHeadUnitAI, NULL) > 0)
					{
						// this will remove pLoopUnit from the current group
						pLoopUnit->AI_setUnitAIType(eNewHeadUnitAI);

						bChangedUnitAI = true;
					}
				}

				pLoopUnit->joinGroup(pSelectionGroup);
			}
		}
	}
	while (bChangedUnitAI);
}

// split this group into two new groups, one of iSplitSize, the other the remaining units
// split up each unit AI type as evenly as possible
CvSelectionGroup* CvSelectionGroup::splitGroup(int iSplitSize, CvUnit* pNewHeadUnit, CvSelectionGroup** ppOtherGroup)
{
	FAssertMsg(iSplitSize > 0, "non-positive splitGroup size");
	if (!(iSplitSize > 0))
	{
		return NULL;
	}

	// are we already small enough?
	if (getNumUnits() <= iSplitSize)
	{
		return this;
	}
	
	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	CvUnit* pOldHeadUnit = ::getUnit(pUnitNode->m_data);
	FAssertMsg(pOldHeadUnit != NULL, "non-zero group without head unit");
	if (pOldHeadUnit == NULL)
	{
		return NULL;
	}

	UnitAITypes eOldHeadAI = pOldHeadUnit->AI_getUnitAIType();

	// if pNewHeadUnit NULL, then we will use our current head to head the new split group of target size
	if (pNewHeadUnit == NULL)
	{
		pNewHeadUnit = pOldHeadUnit;
	}

	// the AI of the new head (the remainder will get the AI of the old head)
	// UnitAITypes eNewHeadAI = pNewHeadUnit->AI_getUnitAIType();

	// pRemainderHeadUnit is the head unit of the group that contains the remainder of units 
	CvUnit* pRemainderHeadUnit = NULL;

	// if the new head is not the old head, then make the old head the remainder head
	bool bSplitingHead = (pOldHeadUnit == pNewHeadUnit);
	if (!bSplitingHead)
	{
		pRemainderHeadUnit = pOldHeadUnit;
	}
	
	// try to find remainder head with same AI as head, if we cannot find one, we will split the rest of the group up
	if (pRemainderHeadUnit == NULL)
	{
		// loop over all the units
		pUnitNode = headUnitNode();
		while (pUnitNode != NULL && pRemainderHeadUnit == NULL)
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);
			
			if (pLoopUnit != NULL && pLoopUnit != pNewHeadUnit)
			{
				UnitAITypes eLoopUnitAI = pLoopUnit->AI_getUnitAIType();
				if (eLoopUnitAI == eOldHeadAI)
				{
					pRemainderHeadUnit = pLoopUnit;
				}
			}
		}
	}
	
	CvSelectionGroup* pSplitGroup = NULL;
	CvSelectionGroup* pRemainderGroup = NULL;
	
	// make the new group for the new head
	pNewHeadUnit->joinGroup(NULL);
	pSplitGroup = pNewHeadUnit->getGroup();
	FAssertMsg(pSplitGroup != NULL, "join resulted in NULL group");

	// make a new group for the remainder, if non-null
	if (pRemainderHeadUnit != NULL)
	{
		pRemainderHeadUnit->joinGroup(NULL);
		pRemainderGroup = pRemainderHeadUnit->getGroup();
		FAssertMsg(pRemainderGroup != NULL, "join resulted in NULL group");
	}

	// loop until this group is empty, trying to move different AI types each time
	
	
	//Exhibit of why i HATE iustus code sometimes
	//unsigned int unitAIBitField = 0;
	//setBit(unitAIBitField, eNewHeadAI);
	
	bool abUnitAIField[NUM_UNITAI_TYPES];
	for (int iI = 0; iI < NUM_UNITAI_TYPES; iI++)
	{
		abUnitAIField[iI] = false;
	}
	
	while (getNumUnits())
	{
		UnitAITypes eTargetUnitAI = NO_UNITAI;
	
		// loop over all the units, find the next different UnitAI and move one of each
		bool bDestinationSplit = (pSplitGroup->getNumUnits() < iSplitSize);
		pUnitNode = headUnitNode();
		while (pUnitNode != NULL)
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);
			
			if (pLoopUnit != NULL)
			{
				UnitAITypes eLoopUnitAI = pLoopUnit->AI_getUnitAIType();
				
				// if we have not found a new type to move, is this a new unitai?
				// note, if there eventually are unitAIs above 31, we will just always move those, which is fine
				if (eTargetUnitAI == NO_UNITAI && !abUnitAIField[eLoopUnitAI])
				{
					eTargetUnitAI =  eLoopUnitAI;
					abUnitAIField[eLoopUnitAI] = true;
				}
				
				// is this the right UnitAI?
				if (eLoopUnitAI == eTargetUnitAI)
				{
					// move this unit to the appropriate group 
					if (bDestinationSplit)
					{
						pLoopUnit->joinGroup(pSplitGroup);
					}
					else
					{
						pLoopUnit->joinGroup(pRemainderGroup);
						// (if pRemainderGroup NULL, it gets its own group)
						pRemainderGroup = pLoopUnit->getGroup();
					}
					
					// if we moved to remainder, try for next unit AI
					if (!bDestinationSplit)
					{
						eTargetUnitAI = NO_UNITAI;

						bDestinationSplit = (pSplitGroup->getNumUnits() < iSplitSize);
					}
					else
					{
						// next unit goes to the remainder group
						bDestinationSplit = false;
					}
				}
			}

		}
		
		// clear bitfield, all types are valid again
		for (int iI = 0; iI < NUM_UNITAI_TYPES; iI++)
		{
			abUnitAIField[iI] = false;
		}
	}

	FAssertMsg(pSplitGroup->getNumUnits() <= iSplitSize, "somehow our split group is too large");
	
	if (ppOtherGroup != NULL)
	{
		*ppOtherGroup = pRemainderGroup;
	}

	return pSplitGroup;
}


//------------------------------------------------------------------------------------------------
// FUNCTION:    CvSelectionGroup::getUnitIndex
//! \brief      Returns the index of the given unit in the selection group
//! \param      pUnit The unit to find the index of within the group
//! \retval     The zero-based index of the unit within the group, or -1 if it is not in the group.
//------------------------------------------------------------------------------------------------
int CvSelectionGroup::getUnitIndex(CvUnit* pUnit, int maxIndex /* = -1 */) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int iIndex;

	iIndex = 0;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit == pUnit)
		{
			return iIndex;
		}

		iIndex++;

		//early out if not interested beyond maxIndex
		if((maxIndex >= 0) && (iIndex >= maxIndex))
			return -1;
	}

	return -1;
}

CLLNode<IDInfo>* CvSelectionGroup::headUnitNode() const
{
	return m_units.head();
}


CvUnit* CvSelectionGroup::getHeadUnit() const
{
	CLLNode<IDInfo>* pUnitNode = headUnitNode();

	if (pUnitNode != NULL)
	{
		return ::getUnit(pUnitNode->m_data);
	}
	else
	{
		return NULL;
	}
}

CvUnit* CvSelectionGroup::getUnitAt(int index) const
{
	int numUnits = getNumUnits();
	if(index >= numUnits)
	{
		FAssertMsg(false, "[Jason] Selectiongroup unit index out of bounds.");
		return NULL;
	}
	else
	{
		CLLNode<IDInfo>* pUnitNode = headUnitNode();
		for(int i=0;i<index;i++)
			pUnitNode = nextUnitNode(pUnitNode);
		
		CvUnit *pUnit = ::getUnit(pUnitNode->m_data);
		return pUnit;
	}
}


UnitAITypes CvSelectionGroup::getHeadUnitAI() const
{
	CvUnit* pHeadUnit;

	pHeadUnit = getHeadUnit();

	if (pHeadUnit != NULL)
	{
		return pHeadUnit->AI_getUnitAIType();
	}

	return NO_UNITAI;
}


PlayerTypes CvSelectionGroup::getHeadOwner() const
{
	CvUnit* pHeadUnit;

	pHeadUnit = getHeadUnit();

	if (pHeadUnit != NULL)
	{
		return pHeadUnit->getOwnerINLINE();
	}

	return NO_PLAYER;
}


TeamTypes CvSelectionGroup::getHeadTeam() const
{
	CvUnit* pHeadUnit;

	pHeadUnit = getHeadUnit();

	if (pHeadUnit != NULL)
	{
		return pHeadUnit->getTeam();
	}

	return NO_TEAM;
}


void CvSelectionGroup::clearMissionQueue()
{
	FAssert(getOwnerINLINE() != NO_PLAYER);

	deactivateHeadMission();

	m_missionQueue.clear();

	if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && IsSelected())
	{
		gDLL->getInterfaceIFace()->setDirty(Waypoints_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
	}
}


int CvSelectionGroup::getLengthMissionQueue() const																
{
	return m_missionQueue.getLength();
}


MissionData* CvSelectionGroup::getMissionFromQueue(int iIndex) const
{
	CLLNode<MissionData>* pMissionNode;

	pMissionNode = m_missionQueue.nodeNum(iIndex);

	if (pMissionNode != NULL)
	{
		return &(pMissionNode->m_data);
	}
	else
	{
		return NULL;
	}
}


void CvSelectionGroup::insertAtEndMissionQueue(MissionData mission, bool bStart)
{
	//PROFILE_FUNC();

	FAssert(getOwnerINLINE() != NO_PLAYER);

	m_missionQueue.insertAtEnd(mission);

	if ((getLengthMissionQueue() == 1) && bStart)
	{
		activateHeadMission();
	}

	if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && IsSelected())
	{
		gDLL->getInterfaceIFace()->setDirty(Waypoints_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
	}
}


CLLNode<MissionData>* CvSelectionGroup::deleteMissionQueueNode(CLLNode<MissionData>* pNode)
{
	CLLNode<MissionData>* pNextMissionNode;

	FAssertMsg(pNode != NULL, "Node is not assigned a valid value");
	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (pNode == headMissionQueueNode())
	{
		deactivateHeadMission();
	}

	pNextMissionNode = m_missionQueue.deleteNode(pNode);

	if (pNextMissionNode == headMissionQueueNode())
	{
		activateHeadMission();
	}

	if ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && IsSelected())
	{
		gDLL->getInterfaceIFace()->setDirty(Waypoints_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
	}

	return pNextMissionNode;
}


CLLNode<MissionData>* CvSelectionGroup::nextMissionQueueNode(CLLNode<MissionData>* pNode) const
{
	return m_missionQueue.next(pNode);
}


CLLNode<MissionData>* CvSelectionGroup::prevMissionQueueNode(CLLNode<MissionData>* pNode) const
{
	return m_missionQueue.prev(pNode);
}


CLLNode<MissionData>* CvSelectionGroup::headMissionQueueNode() const
{
	return m_missionQueue.head();
}


CLLNode<MissionData>* CvSelectionGroup::tailMissionQueueNode() const
{
	return m_missionQueue.tail();
}


int CvSelectionGroup::getMissionType(int iNode) const
{
	int iCount = 0;
	CLLNode<MissionData>* pMissionNode;

	pMissionNode = headMissionQueueNode();

	while (pMissionNode != NULL)
	{
		if ( iNode == iCount )
		{
			return pMissionNode->m_data.eMissionType;
		}

		iCount++;

		pMissionNode = nextMissionQueueNode(pMissionNode);
	}

	return -1;
}


int CvSelectionGroup::getMissionData1(int iNode) const
{
	int iCount = 0;
	CLLNode<MissionData>* pMissionNode;

	pMissionNode = headMissionQueueNode();

	while (pMissionNode != NULL)
	{
		if ( iNode == iCount )
		{
			return pMissionNode->m_data.iData1;
		}

		iCount++;

		pMissionNode = nextMissionQueueNode(pMissionNode);
	}

	return -1;
}


int CvSelectionGroup::getMissionData2(int iNode) const
{
	int iCount = 0;
	CLLNode<MissionData>* pMissionNode;

	pMissionNode = headMissionQueueNode();

	while (pMissionNode != NULL)
	{
		if ( iNode == iCount )
		{
			return pMissionNode->m_data.iData2;
		}

		iCount++;

		pMissionNode = nextMissionQueueNode(pMissionNode);
	}

	return -1;
}


void CvSelectionGroup::read(FDataStreamBase* pStream)
{
	// Init saved data
	reset();

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iID);
	pStream->Read(&m_iMissionTimer);

	pStream->Read(&m_bForceUpdate);

	pStream->Read((int*)&m_eOwner);
	pStream->Read((int*)&m_eActivityType);
	pStream->Read((int*)&m_eAutomateType);

	m_units.Read(pStream);
	m_missionQueue.Read(pStream);
}


void CvSelectionGroup::write(FDataStreamBase* pStream)
{
	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iID);
	pStream->Write(m_iMissionTimer);

	pStream->Write(m_bForceUpdate);

	pStream->Write(m_eOwner);
	pStream->Write(m_eActivityType);
	pStream->Write(m_eAutomateType);

	m_units.Write(pStream);
	m_missionQueue.Write(pStream);
}

// Protected Functions...

void CvSelectionGroup::activateHeadMission()
{
	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (headMissionQueueNode() != NULL)
	{
		if (!isBusy())
		{
			startMission();
		}
	}
}


void CvSelectionGroup::deactivateHeadMission()
{
	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (headMissionQueueNode() != NULL)
	{
		if (getActivityType() == ACTIVITY_MISSION)
		{
			setActivityType(ACTIVITY_AWAKE);
		}

		setMissionTimer(0);

		if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			if (IsSelected())
			{
				gDLL->getInterfaceIFace()->changeCycleSelectionCounter(1);
			}
		}
	}
}

/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
// Dale - SA: Stack Attack START
bool CvSelectionGroup::groupStackAttack(int iX, int iY, int iFlags, bool& bFailedAlreadyFighting)
{
	CvPlot* pDestPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	CvPlot* pOrigPlot = plot();
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	if (iFlags & MOVE_THROUGH_ENEMY)
	{
		if (generatePath(plot(), pDestPlot, iFlags))
		{
			pDestPlot = getPathFirstPlot();
		}
	}
	FAssertMsg(pDestPlot != NULL, "DestPlot is not assigned a valid value");
	bool bStack = (isHuman() && ((getDomainType() == DOMAIN_AIR) || true));//GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_STACK_ATTACK)));
	bool bAttack = false;
	bool bAction = false;
	bFailedAlreadyFighting = false;
	if (getNumUnits() > 0)
	{
		if ((getDomainType() == DOMAIN_AIR) || (stepDistance(getX(), getY(), pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE()) == 1))
		{
			if ((iFlags & MOVE_DIRECT_ATTACK) || (getDomainType() == DOMAIN_AIR) || (iFlags & MOVE_THROUGH_ENEMY) || (generatePath(plot(), pDestPlot, iFlags) && (getPathFirstPlot() == pDestPlot)))
			{
				int iAttackOdds;
				CvUnit* pBestAttackUnit = AI_getBestGroupAttacker(pDestPlot, true, iAttackOdds);
				if (pBestAttackUnit != NULL)
				{
					// if there are no defenders, do not attack
					CvUnit* pBestDefender = pDestPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), pBestAttackUnit, true);
					if (NULL == pBestDefender)
					{
						return false;
					}
					bool bNoBlitz = (!pBestAttackUnit->isBlitz() || !pBestAttackUnit->isMadeAttack());
					if (groupDeclareWar(pDestPlot))
					{
						return true;
					}
/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
					pBestAttackUnit->setBattlePlot(pDestPlot, pBestDefender);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/

					// RevolutionDCM - attack support
					if (GC.getDCM_ATTACK_SUPPORT())
					{
						if (pDestPlot->getNumUnits() > 0)
						{
							pUnitNode = pDestPlot->headUnitNode();
							while (pUnitNode != NULL)
							{
								pLoopUnit = ::getUnit(pUnitNode->m_data);
								pUnitNode = pDestPlot->nextUnitNode(pUnitNode);
								if (GET_TEAM(pLoopUnit->getTeam()).isAtWar(getTeam()))
								{
									if (pLoopUnit != NULL && this != NULL && pDestPlot != NULL && plot() != NULL)
									{
										if (pLoopUnit->canArcherBombardAt(pDestPlot, plot()->getX_INLINE(), plot()->getY_INLINE()))
										{
											if (pLoopUnit->archerBombard(plot()->getX_INLINE(), plot()->getY_INLINE(), true))
											{
												bAction = true;
											}
										}
										else if (pLoopUnit->canBombardAtRanged(pDestPlot, plot()->getX_INLINE(), plot()->getY_INLINE()))
										{
											if (pLoopUnit->bombardRanged(plot()->getX_INLINE(), plot()->getY_INLINE(), true))
											{
												bAction = true;
											}
										}
										else if (pLoopUnit->getDomainType() == DOMAIN_AIR && !pLoopUnit->isSuicide())
										{
											pLoopUnit->updateAirStrike(plot(), false, true);//airStrike(plot()))
										}
										pLoopUnit->setMadeAttack(false);
									}
								}
							}
						} else {
							return bAttack;
						}
						if (pOrigPlot->getNumUnits() > 0)
						{
							pUnitNode = pOrigPlot->headUnitNode();
							while (pUnitNode != NULL)
							{
								pLoopUnit = ::getUnit(pUnitNode->m_data);
								pUnitNode = pOrigPlot->nextUnitNode(pUnitNode);
								if (pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
								{
									if (pLoopUnit != NULL && this != NULL && pDestPlot != NULL && plot() != NULL)
									{
										if (pLoopUnit->canArcherBombardAt(plot(), pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE()))
										{
											if (pLoopUnit->archerBombard(pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE(), true))
											{
												bAction = true;
											}
										}
										else if (pLoopUnit->canBombardAtRanged(plot(), pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE()))
										{
											if (pLoopUnit->bombardRanged(pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE(), true))
											{
												bAction = true;
											}
										}
										else if (pLoopUnit->getDomainType() == DOMAIN_AIR && !pLoopUnit->isSuicide())
										{
											if (plotDistance(plot()->getX_INLINE(), plot()->getY_INLINE(), pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE()) == 1)
											{
												pLoopUnit->updateAirStrike(pDestPlot, false, false);//airStrike(pDestPlot))
											}
										}
										pLoopUnit->setMadeAttack(false);
									}
								}
							}
						} else {
							return bAttack;
						}
					}
					// RevolutionDCM - attack support end
					while (true)
					{
						pBestAttackUnit = AI_getBestGroupAttacker(pDestPlot, false, iAttackOdds, false, bNoBlitz);
						if (pBestAttackUnit == NULL)
						{
							break;
						}
						if (iAttackOdds < 68)
						{
						    CvUnit * pBestSacrifice = AI_getBestGroupSacrifice(pDestPlot, false, false, bNoBlitz);
						    if (pBestSacrifice != NULL)
						    {
                                pBestAttackUnit = pBestSacrifice;
						    }
						}
						bAttack = true;
/************************************************************************************************/
/* Afforess	                  Start		 04/29/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
						if (GC.getUSE_CAN_DO_COMBAT_CALLBACK())
						{
							CySelectionGroup* pyGroup = new CySelectionGroup(this);
							CyPlot* pyPlot = new CyPlot(pDestPlot);
							CyArgsList argsList;
							argsList.add(gDLL->getPythonIFace()->makePythonObject(pyGroup));	// pass in Selection Group class
							argsList.add(gDLL->getPythonIFace()->makePythonObject(pyPlot));	// pass in Plot class
							long lResult=0;
							gDLL->getPythonIFace()->callFunction(PYGameModule, "doCombat", argsList.makeFunctionArgs(), &lResult);
							delete pyGroup;	// python fxn must not hold on to this pointer 
							delete pyPlot;	// python fxn must not hold on to this pointer 
							if (lResult == 1)
							{
								break;
							}
						}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

						pBestAttackUnit->attack(pDestPlot, false);
						if (bFailedAlreadyFighting || !bStack)
						{
							// if this is AI stack, follow through with the attack to the end
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/* Allow Automated Units to Stack Attack                                                        */
/************************************************************************************************/
/*
							if (!isHuman() && getNumUnits() > 1)
*/
							if ((!isHuman() | isAutomated()) && getNumUnits() > 1)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
							{
								AI_queueGroupAttack(iX, iY);
							}
							
							break;
						}
					}
				}
			}
		}
	}

	return bAttack;
}
// Dale - SA: Stack Attack END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/

// BUG - All Units Actions - start
bool CvSelectionGroup::allMatch(UnitTypes eUnit) const
{
	FAssertMsg(eUnit >= 0, "eUnit expected to be >= 0");
	FAssertMsg(eUnit < GC.getNumUnitInfos(), "eUnit expected to be < GC.getNumUnitInfos()");

	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	CvUnit* pLoopUnit;

	FAssertMsg(pUnitNode != NULL, "headUnitNode() expected to be non-NULL");

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit->getUnitType() != eUnit)
		{
			return false;
		}
	}

	return true;
}
// BUG - All Units Actions - end

// BUG - Safe Move - start
void CvSelectionGroup::checkLastPathPlot(CvPlot* pPlot)
{
	m_bLastPathPlotChecked = true;
	if (pPlot != NULL)
	{
		m_bLastPlotVisible = pPlot->isVisible(getTeam(), false);
		m_bLastPlotRevealed = pPlot->isRevealed(getTeam(), false);
	}
	else
	{
		m_bLastPlotVisible = false;
		m_bLastPlotRevealed = false;
	}
}

void CvSelectionGroup::clearLastPathPlot()
{
	m_bLastPathPlotChecked = false;
}

bool CvSelectionGroup::isLastPathPlotChecked() const
{
	return m_bLastPathPlotChecked;
}

bool CvSelectionGroup::isLastPathPlotVisible() const
{
	return m_bLastPathPlotChecked ? m_bLastPlotVisible : false;
}

bool CvSelectionGroup::isLastPathPlotRevealed() const
{
	return m_bLastPathPlotChecked ? m_bLastPlotRevealed : false;
}
// BUG - Safe Move - end
