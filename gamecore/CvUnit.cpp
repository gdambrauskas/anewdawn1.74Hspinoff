// unit.cpp

#include "CvGameCoreDLL.h"
#include "CvUnit.h"
#include "CvArea.h"
#include "CvPlot.h"
#include "CvCity.h"
#include "CvGlobals.h"
#include "CvGameCoreUtils.h"
#include "CvGameAI.h"
#include "CvMap.h"
#include "CvPlayerAI.h"
#include "CvRandom.h"
#include "CvTeamAI.h"
#include "CvGameCoreUtils.h"
#include "CyUnit.h"
#include "CyArgsList.h"
#include "CyPlot.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLEngineIFaceBase.h"
#include "CvEventReporter.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"
#include "CvInfos.h"
#include "FProfiler.h"
#include "CvPopupInfo.h"
#include "CvArtFileMgr.h"

// BUG - start
#include "CvBugOptions.h"
// BUG - end

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/24/10                                jdog5000      */
/*                                                                                              */
/* AI logging                                                                                   */
/************************************************************************************************/
#include "BetterBTSAI.h"
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

// Public Functions...


// Public Functions...
#pragma warning( disable : 4355 )
CvUnit::CvUnit() : m_GameObject(this)
{
	m_aiExtraDomainModifier = new int[NUM_DOMAIN_TYPES];
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	m_pUsedCommander = NULL;
	m_paiTerrainProtected = NULL;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	m_pabHasPromotion = NULL;

	m_paiTerrainDoubleMoveCount = NULL;
	m_paiFeatureDoubleMoveCount = NULL;
	m_paiExtraTerrainAttackPercent = NULL;
	m_paiExtraTerrainDefensePercent = NULL;
	m_paiExtraFeatureAttackPercent = NULL;
	m_paiExtraFeatureDefensePercent = NULL;
	m_paiExtraUnitCombatModifier = NULL;

	CvDLLEntity::createUnitEntity(this);		// create and attach entity to unit

	reset(0, NO_UNIT, NO_PLAYER, true);
}


CvUnit::~CvUnit()
{
	if (!gDLL->GetDone() && GC.IsGraphicsInitialized())						// don't need to remove entity when the app is shutting down, or crash can occur
	{
		gDLL->getEntityIFace()->RemoveUnitFromBattle(this);
		CvDLLEntity::removeEntity();		// remove entity from engine
	}

	CvDLLEntity::destroyEntity();			// delete CvUnitEntity and detach from us

	uninit();

	SAFE_DELETE_ARRAY(m_aiExtraDomainModifier);
}

void CvUnit::reloadEntity()
{
	//destroy old entity
	if (!gDLL->GetDone() && GC.IsGraphicsInitialized())						// don't need to remove entity when the app is shutting down, or crash can occur
	{
		gDLL->getEntityIFace()->RemoveUnitFromBattle(this);
		CvDLLEntity::removeEntity();		// remove entity from engine
	}
	
	CvDLLEntity::destroyEntity();			// delete CvUnitEntity and detach from us

	//creat new one
	CvDLLEntity::createUnitEntity(this);		// create and attach entity to unit
	setupGraphical();
}


void CvUnit::init(int iID, UnitTypes eUnit, UnitAITypes eUnitAI, PlayerTypes eOwner, int iX, int iY, DirectionTypes eFacingDirection)
{
	CvWString szBuffer;
	int iUnitName;
	int iI, iJ;

	FAssert(NO_UNIT != eUnit);

	//--------------------------------
	// Init saved data
	reset(iID, eUnit, eOwner);

	if(eFacingDirection == NO_DIRECTION)
		m_eFacingDirection = DIRECTION_SOUTH;
	else
		m_eFacingDirection = eFacingDirection;

	//--------------------------------
	// Init containers

	//--------------------------------
	// Init pre-setup() data
	setXY(iX, iY, false, false);

	//--------------------------------
	// Init non-saved data
	setupGraphical();

	//--------------------------------
	// Init other game data
	plot()->updateCenterUnit();

	plot()->setFlagDirty(true);

	iUnitName = GC.getGameINLINE().getUnitCreatedCount(getUnitType());
	int iNumNames = m_pUnitInfo->getNumUnitNames();
	if (iUnitName < iNumNames)
	{
		int iOffset = GC.getGameINLINE().getSorenRandNum(iNumNames, "Unit name selection");

		for (iI = 0; iI < iNumNames; iI++)
		{
			int iIndex = (iI + iOffset) % iNumNames;
			CvWString szName = gDLL->getText(m_pUnitInfo->getUnitNames(iIndex));
			if (!GC.getGameINLINE().isGreatPersonBorn(szName))
			{
				setName(szName);
				GC.getGameINLINE().addGreatPersonBornName(szName);
				break;
			}
		}
	}

	setGameTurnCreated(GC.getGameINLINE().getGameTurn());

	GC.getGameINLINE().incrementUnitCreatedCount(getUnitType());

	GC.getGameINLINE().incrementUnitClassCreatedCount((UnitClassTypes)(m_pUnitInfo->getUnitClassType()));
	GET_TEAM(getTeam()).changeUnitClassCount(((UnitClassTypes)(m_pUnitInfo->getUnitClassType())), 1);
	GET_PLAYER(getOwnerINLINE()).changeUnitClassCount(((UnitClassTypes)(m_pUnitInfo->getUnitClassType())), 1);

	GET_PLAYER(getOwnerINLINE()).changeExtraUnitCost(m_pUnitInfo->getExtraCost());

	if (m_pUnitInfo->getNukeRange() != -1)
	{
		GET_PLAYER(getOwnerINLINE()).changeNumNukeUnits(1);
	}

	if (m_pUnitInfo->isMilitarySupport())
	{
		GET_PLAYER(getOwnerINLINE()).changeNumMilitaryUnits(1);
	}

	GET_PLAYER(getOwnerINLINE()).changeAssets(m_pUnitInfo->getAssetValue());

	GET_PLAYER(getOwnerINLINE()).changePower(m_pUnitInfo->getPowerValue());

	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (m_pUnitInfo->getFreePromotions(iI))
		{
			setHasPromotion(((PromotionTypes)iI), true);
		}
	}

	FAssertMsg((GC.getNumTraitInfos() > 0), "GC.getNumTraitInfos() is less than or equal to zero but is expected to be larger than zero in CvUnit::init");
	for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
	{
		if (GET_PLAYER(getOwnerINLINE()).hasTrait((TraitTypes)iI))
		{
			for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
			{
				if (GC.getTraitInfo((TraitTypes) iI).isFreePromotion(iJ))
				{
					if ((getUnitCombatType() != NO_UNITCOMBAT) && GC.getTraitInfo((TraitTypes) iI).isFreePromotionUnitCombat(getUnitCombatType()))
					{
						setHasPromotion(((PromotionTypes)iJ), true);
					}
				}
			}
		}
	}

	if (NO_UNITCOMBAT != getUnitCombatType())
	{
		for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
		{
			if (GET_PLAYER(getOwnerINLINE()).isFreePromotion(getUnitCombatType(), (PromotionTypes)iJ))
			{
				setHasPromotion(((PromotionTypes)iJ), true);
			}
		}
	}

	if (NO_UNITCLASS != getUnitClassType())
	{
		for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
		{
			if (GET_PLAYER(getOwnerINLINE()).isFreePromotion(getUnitClassType(), (PromotionTypes)iJ))
			{
				setHasPromotion(((PromotionTypes)iJ), true);
			}
		}
	}

	if (getDomainType() == DOMAIN_LAND)
	{
		if (baseCombatStr() > 0)
		{
			if ((GC.getGameINLINE().getBestLandUnit() == NO_UNIT) || (baseCombatStr() > GC.getGameINLINE().getBestLandUnitCombat()))
			{
				GC.getGameINLINE().setBestLandUnit(getUnitType());
			}
		} 
	}

	if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
	{
		gDLL->getInterfaceIFace()->setDirty(GameData_DIRTY_BIT, true);
	}

	if (isWorldUnitClass((UnitClassTypes)(m_pUnitInfo->getUnitClassType())))
	{
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_TEAM(getTeam()).isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_CREATED_UNIT", GET_PLAYER(getOwnerINLINE()).getNameKey(), getNameKey());
					gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_WONDER_UNIT_BUILD", MESSAGE_TYPE_MAJOR_EVENT, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"), getX_INLINE(), getY_INLINE(), true, true);
				}
				else
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_UNKNOWN_CREATED_UNIT", getNameKey());
					gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_WONDER_UNIT_BUILD", MESSAGE_TYPE_MAJOR_EVENT, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"));
				}
			}
		}

		szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_CREATED_UNIT", GET_PLAYER(getOwnerINLINE()).getNameKey(), getNameKey());
		GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getOwnerINLINE(), szBuffer, getX_INLINE(), getY_INLINE(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"));
	}

	AI_init(eUnitAI);

	CvEventReporter::getInstance().unitCreated(this);
}


void CvUnit::uninit()
{
	SAFE_DELETE_ARRAY(m_pabHasPromotion);
/************************************************************************************************/
/* Afforess	                  Start		 06/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	SAFE_DELETE_ARRAY(m_paiTerrainProtected);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	SAFE_DELETE_ARRAY(m_paiTerrainDoubleMoveCount);
	SAFE_DELETE_ARRAY(m_paiFeatureDoubleMoveCount);
	SAFE_DELETE_ARRAY(m_paiExtraTerrainAttackPercent);
	SAFE_DELETE_ARRAY(m_paiExtraTerrainDefensePercent);
	SAFE_DELETE_ARRAY(m_paiExtraFeatureAttackPercent);
	SAFE_DELETE_ARRAY(m_paiExtraFeatureDefensePercent);
	SAFE_DELETE_ARRAY(m_paiExtraUnitCombatModifier);
}


// FUNCTION: reset()
// Initializes data members that are serialized.
void CvUnit::reset(int iID, UnitTypes eUnit, PlayerTypes eOwner, bool bConstructorCall)
{
	int iI;

	//--------------------------------
	// Uninit class
	uninit();

/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - RB: Field Bombard START
	m_iDCMBombRange = 0;
	m_iDCMBombAccuracy = 0;
	// Dale - RB: Field Bombard END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/

	m_iID = iID;
	m_iGroupID = FFreeList::INVALID_INDEX;
	m_iHotKeyNumber = -1;
	m_iX = INVALID_PLOT_COORD;
	m_iY = INVALID_PLOT_COORD;
	m_iLastMoveTurn = 0;
	m_iReconX = INVALID_PLOT_COORD;
	m_iReconY = INVALID_PLOT_COORD;
	m_iGameTurnCreated = 0;
	m_iDamage = 0;
	m_iMoves = 0;
	m_iExperience = 0;
	m_iLevel = 1;
	m_iCargo = 0;
	m_iAttackPlotX = INVALID_PLOT_COORD;
	m_iAttackPlotY = INVALID_PLOT_COORD;
	m_iCombatTimer = 0;
	m_iCombatFirstStrikes = 0;
	m_iFortifyTurns = 0;
	m_iBlitzCount = 0;
	m_iAmphibCount = 0;
	m_iRiverCount = 0;
	m_iEnemyRouteCount = 0;
	m_iAlwaysHealCount = 0;
	m_iHillsDoubleMoveCount = 0;
	m_iImmuneToFirstStrikesCount = 0;
	m_iExtraVisibilityRange = 0;
	m_iExtraMoves = 0;
	m_iExtraMoveDiscount = 0;
	m_iExtraAirRange = 0;
	m_iExtraIntercept = 0;
	m_iExtraEvasion = 0;
	m_iExtraFirstStrikes = 0;
	m_iExtraChanceFirstStrikes = 0;
	m_iExtraWithdrawal = 0;
	m_iExtraCollateralDamage = 0;
	m_iExtraBombardRate = 0;
	m_iExtraEnemyHeal = 0;
	m_iExtraNeutralHeal = 0;
	m_iExtraFriendlyHeal = 0;
	m_iSameTileHeal = 0;
	m_iAdjacentTileHeal = 0;
	m_iExtraCombatPercent = 0;
	m_iExtraCityAttackPercent = 0;
	m_iExtraCityDefensePercent = 0;
	m_iExtraHillsAttackPercent = 0;
	m_iExtraHillsDefensePercent = 0;
	m_iRevoltProtection = 0;
	m_iCollateralDamageProtection = 0;
	m_iPillageChange = 0;
	m_iUpgradeDiscount = 0;
	m_iExperiencePercent = 0;
	m_iKamikazePercent = 0;
	m_eFacingDirection = DIRECTION_SOUTH;
	m_iImmobileTimer = 0;

	m_bMadeAttack = false;
	m_bMadeInterception = false;
	m_bPromotionReady = false;
	m_bDeathDelay = false;
	m_bCombatFocus = false;
	m_bInfoBarDirty = false;
	m_bBlockading = false;
	m_bAirCombat = false;
/************************************************************************************************/
/* Afforess	                  Start		 02/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	m_iCanMovePeaksCount = 0;
	m_iSleepTimer = 0;
	//@MOD Commanders: reset parameters
	m_iOnslaughtCount = 0;
	m_iExtraCommandRange = 0;
	m_iExtraControlPoints = 0;
	m_iControlPointsLeft = 0;
	m_iCommanderID = -1;
	m_pUsedCommander = NULL;
	m_eOriginalOwner = eOwner;
	m_bCommander = false;
	m_iZoneOfControlCount = 0;
	m_bAutoPromoting = false;
	m_bAutoUpgrading = false;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	m_eOwner = eOwner;
	m_eCapturingPlayer = NO_PLAYER;
	m_eUnitType = eUnit;
	m_pUnitInfo = (NO_UNIT != m_eUnitType) ? &GC.getUnitInfo(m_eUnitType) : NULL;
	m_iBaseCombat = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->getCombat() : 0;
	m_eLeaderUnitType = NO_UNIT;
	m_iCargoCapacity = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->getCargoSpace() : 0;

	m_combatUnit.reset();
	m_transportUnit.reset();

	for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		m_aiExtraDomainModifier[iI] = 0;
	}

	m_szName.clear();
	m_szScriptData ="";

	if (!bConstructorCall)
	{
		FAssertMsg((0 < GC.getNumPromotionInfos()), "GC.getNumPromotionInfos() is not greater than zero but an array is being allocated in CvUnit::reset");
		m_pabHasPromotion = new bool[GC.getNumPromotionInfos()];
		for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
		{
			m_pabHasPromotion[iI] = false;
		}

		FAssertMsg((0 < GC.getNumTerrainInfos()), "GC.getNumTerrainInfos() is not greater than zero but a float array is being allocated in CvUnit::reset");
		m_paiTerrainDoubleMoveCount = new int[GC.getNumTerrainInfos()];
		m_paiExtraTerrainAttackPercent = new int[GC.getNumTerrainInfos()];
		m_paiExtraTerrainDefensePercent = new int[GC.getNumTerrainInfos()];
/************************************************************************************************/
/* Afforess	                  Start		 06/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		m_paiTerrainProtected = new int[GC.getNumTerrainInfos()];
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
		{
			m_paiTerrainDoubleMoveCount[iI] = 0;
			m_paiExtraTerrainAttackPercent[iI] = 0;
			m_paiExtraTerrainDefensePercent[iI] = 0;
/************************************************************************************************/
/* Afforess	                  Start		 06/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			m_paiTerrainProtected[iI] = 0;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		}

		FAssertMsg((0 < GC.getNumFeatureInfos()), "GC.getNumFeatureInfos() is not greater than zero but a float array is being allocated in CvUnit::reset");
		m_paiFeatureDoubleMoveCount = new int[GC.getNumFeatureInfos()];
		m_paiExtraFeatureDefensePercent = new int[GC.getNumFeatureInfos()];
		m_paiExtraFeatureAttackPercent = new int[GC.getNumFeatureInfos()];
		for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
		{
			m_paiFeatureDoubleMoveCount[iI] = 0;
			m_paiExtraFeatureAttackPercent[iI] = 0;
			m_paiExtraFeatureDefensePercent[iI] = 0;
		}

		FAssertMsg((0 < GC.getNumUnitCombatInfos()), "GC.getNumUnitCombatInfos() is not greater than zero but an array is being allocated in CvUnit::reset");
		m_paiExtraUnitCombatModifier = new int[GC.getNumUnitCombatInfos()];
		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			m_paiExtraUnitCombatModifier[iI] = 0;
		}

		AI_reset();
	}
}


//////////////////////////////////////
// graphical only setup
//////////////////////////////////////
void CvUnit::setupGraphical()
{
	if (!GC.IsGraphicsInitialized())
	{
		return;
	}

	CvDLLEntity::setup();

	if (getGroup()->getActivityType() == ACTIVITY_INTERCEPT)
	{
		airCircle(true);
	}
}


void CvUnit::convert(CvUnit* pUnit)
{
	CvPlot* pPlot = plot();

	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		setHasPromotion(((PromotionTypes)iI), (pUnit->isHasPromotion((PromotionTypes)iI) || m_pUnitInfo->getFreePromotions(iI)));
	}

	setGameTurnCreated(pUnit->getGameTurnCreated());
	setDamage(pUnit->getDamage());
	setMoves(pUnit->getMoves());
/************************************************************************************************/
/* Afforess	                  Start		 06/06/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	m_eOriginalOwner = pUnit->getOriginalOwner();
	setAutoPromoting(pUnit->isAutoPromoting());
	setAutoUpgrading(pUnit->isAutoUpgrading());
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	setLevel(pUnit->getLevel());
	int iOldModifier = std::max(1, 100 + GET_PLAYER(pUnit->getOwnerINLINE()).getLevelExperienceModifier());
	int iOurModifier = std::max(1, 100 + GET_PLAYER(getOwnerINLINE()).getLevelExperienceModifier());
	setExperience(std::max(0, (pUnit->getExperience() * iOurModifier) / iOldModifier));

	setName(pUnit->getNameNoDesc());
// BUG - Unit Name - start
	if (pUnit->isDescInName() && getBugOptionBOOL("MiscHover__UpdateUnitNameOnUpgrade", true, "BUG_UPDATE_UNIT_NAME_ON_UPGRADE"))
	{
		CvWString szUnitType(pUnit->m_pUnitInfo->getDescription());

		//szUnitType.Format(L"%s", pUnit->m_pUnitInfo->getDescription());
		m_szName.replace(m_szName.find(szUnitType), szUnitType.length(), m_pUnitInfo->getDescription());
	}
// BUG - Unit Name - end
	setLeaderUnitType(pUnit->getLeaderUnitType());

	CvUnit* pTransportUnit = pUnit->getTransportUnit();
	if (pTransportUnit != NULL)
	{
		pUnit->setTransportUnit(NULL);
		setTransportUnit(pTransportUnit);
	}

	std::vector<CvUnit*> aCargoUnits;
	pUnit->getCargoUnits(aCargoUnits);
	for (uint i = 0; i < aCargoUnits.size(); ++i)
	{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/30/09                     Mongoose & jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original BTS code
		aCargoUnits[i]->setTransportUnit(this);
*/
		// From Mongoose SDK
		// Check cargo types and capacity when upgrading transports
		if (cargoSpaceAvailable(aCargoUnits[i]->getSpecialUnitType(), aCargoUnits[i]->getDomainType()) > 0)
		{
			aCargoUnits[i]->setTransportUnit(this);
		}
		else
		{
			aCargoUnits[i]->setTransportUnit(NULL);
			aCargoUnits[i]->jumpToNearestValidPlot();
		}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
	}

	pUnit->kill(true);
}


void CvUnit::kill(bool bDelay, PlayerTypes ePlayer)
{
	PROFILE_FUNC();
	
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pTransportUnit;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;
	CvWString szBuffer;
	PlayerTypes eOwner;
	PlayerTypes eCapturingPlayer;
	UnitTypes eCaptureUnitType;

	pPlot = plot();
	FAssertMsg(pPlot != NULL, "Plot is not assigned a valid value");

	static std::vector<IDInfo> oldUnits;
	oldUnits.clear();
	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		oldUnits.push_back(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);
	}

	for (uint i = 0; i < oldUnits.size(); i++)
	{
		pLoopUnit = ::getUnit(oldUnits[i]);
		
		if (pLoopUnit != NULL)
		{
			if (pLoopUnit->getTransportUnit() == this)
			{
				//save old units because kill will clear the static list
				std::vector<IDInfo> tempUnits = oldUnits;

				if (pPlot->isValidDomainForLocation(*pLoopUnit))
				{
					pLoopUnit->setCapturingPlayer(NO_PLAYER);
				}
				
				pLoopUnit->kill(false, ePlayer);

				oldUnits = tempUnits;
			}
		}
	}

	if (ePlayer != NO_PLAYER)
	{
		CvEventReporter::getInstance().unitKilled(this, ePlayer);

		if ( (NO_UNIT != getLeaderUnitType())
		|| (GC.getUnitClassInfo(getUnitClassType()).getMaxGlobalInstances() == 1) )
		{
			for (int iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_GENERAL_KILLED", getNameKey());
					gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_MAJOR_EVENT);
				}
			}
		}
	}

	if (bDelay)
	{
		startDelayedDeath();
		return;
	}

	if (isMadeAttack() && nukeRange() != -1)
	{
		CvPlot* pTarget = getAttackPlot();
		if (pTarget)
		{
			pTarget->nukeExplosion(nukeRange(), this);
			setAttackPlot(NULL, false);
		}
	}

	finishMoves();

	if (IsSelected())
	{
		if (gDLL->getInterfaceIFace()->getLengthSelectionList() == 1)
		{
			if (!(gDLL->getInterfaceIFace()->isFocused()) && !(gDLL->getInterfaceIFace()->isCitySelection()) && !(gDLL->getInterfaceIFace()->isDiploOrPopupWaiting()))
			{
				GC.getGameINLINE().updateSelectionList();
			}

			if (IsSelected())
			{
				gDLL->getInterfaceIFace()->setCycleSelectionCounter(1);
			}
			else
			{
				gDLL->getInterfaceIFace()->setDirty(SelectionCamera_DIRTY_BIT, true);
			}
		}
	}

	gDLL->getInterfaceIFace()->removeFromSelectionList(this);

	// XXX this is NOT a hack, without it, the game crashes.
	gDLL->getEntityIFace()->RemoveUnitFromBattle(this);

	FAssertMsg(!isCombat(), "isCombat did not return false as expected");

	pTransportUnit = getTransportUnit();

	if (pTransportUnit != NULL)
	{
		setTransportUnit(NULL);
	}

	setReconPlot(NULL);
	setBlockading(false);

	FAssertMsg(getAttackPlot() == NULL, "The current unit instance's attack plot is expected to be NULL");
	FAssertMsg(getCombatUnit() == NULL, "The current unit instance's combat unit is expected to be NULL");

	GET_TEAM(getTeam()).changeUnitClassCount((UnitClassTypes)m_pUnitInfo->getUnitClassType(), -1);
	GET_PLAYER(getOwnerINLINE()).changeUnitClassCount((UnitClassTypes)m_pUnitInfo->getUnitClassType(), -1);

	GET_PLAYER(getOwnerINLINE()).changeExtraUnitCost(-(m_pUnitInfo->getExtraCost()));

	if (m_pUnitInfo->getNukeRange() != -1)
	{
		GET_PLAYER(getOwnerINLINE()).changeNumNukeUnits(-1);
	}

	if (m_pUnitInfo->isMilitarySupport())
	{
		GET_PLAYER(getOwnerINLINE()).changeNumMilitaryUnits(-1);
	}

	GET_PLAYER(getOwnerINLINE()).changeAssets(-(m_pUnitInfo->getAssetValue()));

	GET_PLAYER(getOwnerINLINE()).changePower(-(m_pUnitInfo->getPowerValue()));
/************************************************************************************************/
/* Afforess	                  Start		 04/16/10                                               */
/*                                                                                              */
/*  Promotions affect iAsset and iPower values, so they must be removed on unit death           */
/************************************************************************************************/
	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		setHasPromotion(((PromotionTypes)iI), false);
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


	GET_PLAYER(getOwnerINLINE()).AI_changeNumAIUnits(AI_getUnitAIType(), -1);

	eOwner = getOwnerINLINE();
	eCapturingPlayer = getCapturingPlayer();
	eCaptureUnitType = ((eCapturingPlayer != NO_PLAYER) ? getCaptureUnitType(GET_PLAYER(eCapturingPlayer).getCivilizationType()) : NO_UNIT);
// BUG - Unit Captured Event - start
	PlayerTypes eFromPlayer = getOwner();
	UnitTypes eCapturedUnitType = getUnitType();
// BUG - Unit Captured Event - end

	setXY(INVALID_PLOT_COORD, INVALID_PLOT_COORD, true);

	joinGroup(NULL, false, false);

	CvEventReporter::getInstance().unitLost(this);

	GET_PLAYER(getOwnerINLINE()).deleteUnit(getID());

	if ((eCapturingPlayer != NO_PLAYER) && (eCaptureUnitType != NO_UNIT) && !(GET_PLAYER(eCapturingPlayer).isBarbarian()))
	{
		if (GET_PLAYER(eCapturingPlayer).isHuman() || GET_PLAYER(eCapturingPlayer).AI_captureUnit(eCaptureUnitType, pPlot) || 0 == GC.getDefineINT("AI_CAN_DISBAND_UNITS"))
		{
			CvUnit* pkCapturedUnit = GET_PLAYER(eCapturingPlayer).initUnit(eCaptureUnitType, pPlot->getX_INLINE(), pPlot->getY_INLINE());

			if (pkCapturedUnit != NULL)
			{
// BUG - Unit Captured Event - start
				CvEventReporter::getInstance().unitCaptured(eFromPlayer, eCapturedUnitType, pkCapturedUnit);
// BUG - Unit Captured Event - end

				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_CAPTURED_UNIT", GC.getUnitInfo(eCaptureUnitType).getTextKeyWide());
				gDLL->getInterfaceIFace()->addMessage(eCapturingPlayer, true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_UNITCAPTURE", MESSAGE_TYPE_INFO, pkCapturedUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

				// Add a captured mission
				CvMissionDefinition kMission;
				kMission.setMissionTime(GC.getMissionInfo(MISSION_CAPTURED).getTime() * gDLL->getSecsPerTurn());
				kMission.setUnit(BATTLE_UNIT_ATTACKER, pkCapturedUnit);
				kMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
				kMission.setPlot(pPlot);
				kMission.setMissionType(MISSION_CAPTURED);
				gDLL->getEntityIFace()->AddMission(&kMission);

				pkCapturedUnit->finishMoves();

				if (!GET_PLAYER(eCapturingPlayer).isHuman())
				{
					CvPlot* pPlot = pkCapturedUnit->plot();
					if (pPlot && !pPlot->isCity(false))
					{
						if (GET_PLAYER(eCapturingPlayer).AI_getPlotDanger(pPlot) && GC.getDefineINT("AI_CAN_DISBAND_UNITS"))
						{
							pkCapturedUnit->kill(false);
						}
					}
				}
			}
		}
	}
}


void CvUnit::NotifyEntity(MissionTypes eMission)
{
	gDLL->getEntityIFace()->NotifyEntity(getUnitEntity(), eMission);
}


void CvUnit::doTurn()
{
	PROFILE("CvUnit::doTurn()")

	FAssertMsg(!isDead(), "isDead did not return false as expected");
	FAssertMsg(getGroup() != NULL, "getGroup() is not expected to be equal with NULL");

	testPromotionReady();
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	m_iControlPointsLeft = controlPoints();	//restore control points for commander
	gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
	m_pUsedCommander = NULL;	//reset used commander for combat units
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	if (isBlockading())
	{
		collectBlockadeGold();
	}

	if (isSpy() && isIntruding() && !isCargo())
	{
		TeamTypes eTeam = plot()->getTeam();
		if (NO_TEAM != eTeam)
		{
			if (GET_TEAM(getTeam()).isOpenBorders(eTeam))
			{
				testSpyIntercepted(plot()->getOwnerINLINE(), GC.getDefineINT("ESPIONAGE_SPY_NO_INTRUDE_INTERCEPT_MOD"));
			}
			else
			{
				testSpyIntercepted(plot()->getOwnerINLINE(), GC.getDefineINT("ESPIONAGE_SPY_INTERCEPT_MOD"));
			}
		}
	}

	if (baseCombatStr() > 0)
	{
		FeatureTypes eFeature = plot()->getFeatureType();
		if (NO_FEATURE != eFeature)
		{
			if (0 != GC.getFeatureInfo(eFeature).getTurnDamage())
			{
				changeDamage(GC.getFeatureInfo(eFeature).getTurnDamage(), NO_PLAYER);
			}

		}
/************************************************************************************************/
/* Afforess	                  Start		 05/17/10                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		if (plot()->getTerrainTurnDamage(this) != 0)
		{
			changeDamage(plot()->getTerrainTurnDamage(this), NO_PLAYER);
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	}

	if (hasMoved())
	{
		if (isAlwaysHeal())
		{
			doHeal();
		}
	}
	else
	{
		if (isHurt())
		{
			doHeal();
		}

		if (!isCargo())
		{
			changeFortifyTurns(1);
		}
	}
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (isSpy() && m_iSleepTimer > 0)
	{
		if (getFortifyTurns() == GC.getDefineINT("MAX_FORTIFY_TURNS"))
		{
			getGroup()->setActivityType(ACTIVITY_AWAKE);
			m_iSleepTimer = 0;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	changeImmobileTimer(-1);

	setMadeAttack(false);
	setMadeInterception(false);

	setReconPlot(NULL);


	setMoves(0);
/************************************************************************************************/
/* Afforess	                  Start		 08/20/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (isAutoUpgrading())
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 3) == 0 || plot()->isCity())
		{
			AI_upgrade();
			//Could Delete Unit!!!
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
}


void CvUnit::updateAirStrike(CvPlot* pPlot, bool bQuick, bool bFinish)
{
	bool bVisible = false;

	if (!bFinish)
	{
		if (isFighting())
		{
			return;
		}

		if (!bQuick)
		{
			bVisible = isCombatVisible(NULL);
		}

/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
		// Dale - NB: A-Bomb START
		if(canNuke(pPlot)){
			kill(true);
			return;
		}
		// Dale - NB: A-Bomb END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/

		if (!airStrike(pPlot))
		{
			return;
		}

		if (bVisible)
		{
			CvAirMissionDefinition kAirMission;
			kAirMission.setMissionType(MISSION_AIRSTRIKE);
			kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
			kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
			kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
			kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
			kAirMission.setPlot(pPlot);
			setCombatTimer(GC.getMissionInfo(MISSION_AIRSTRIKE).getTime());
			GC.getGameINLINE().incrementTurnTimer(getCombatTimer());
			kAirMission.setMissionTime(getCombatTimer() * gDLL->getSecsPerTurn());

			if (pPlot->isActiveVisible(false))
			{
				gDLL->getEntityIFace()->AddMission(&kAirMission);
			}

			return;
		}
	}

	CvUnit *pDefender = getCombatUnit();
	if (pDefender != NULL)
	{
		pDefender->setCombatUnit(NULL);
	}
	setCombatUnit(NULL);
	setAttackPlot(NULL, false);

	getGroup()->clearMissionQueue();

	if (isSuicide() && !isDead())
	{
		kill(true);
	}
}

void CvUnit::resolveAirCombat(CvUnit* pInterceptor, CvPlot* pPlot, CvAirMissionDefinition& kBattle)
{
	CvWString szBuffer;

	int iTheirStrength = (DOMAIN_AIR == pInterceptor->getDomainType() ? pInterceptor->airCurrCombatStr(this) : pInterceptor->currCombatStr(NULL, NULL));
	int iOurStrength = (DOMAIN_AIR == getDomainType() ? airCurrCombatStr(pInterceptor) : currCombatStr(NULL, NULL));
	int iTotalStrength = iOurStrength + iTheirStrength;
	if (0 == iTotalStrength)
	{
		FAssert(false);
		return;
	}

/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						10/19/08	Roland J & jdog5000	*/
/* 																			*/
/* 	Combat mechanics														*/
/********************************************************************************/
	/*
	int iOurOdds = (100 * iOurStrength) / std::max(1, iTotalStrength);

	int iOurRoundDamage = (pInterceptor->currInterceptionProbability() * GC.getDefineINT("MAX_INTERCEPTION_DAMAGE")) / 100;
	int iTheirRoundDamage = (currInterceptionProbability() * GC.getDefineINT("MAX_INTERCEPTION_DAMAGE")) / 100;
	if (getDomainType() == DOMAIN_AIR)
	{
		iTheirRoundDamage = std::max(GC.getDefineINT("MIN_INTERCEPTION_DAMAGE"), iTheirRoundDamage);
	}

	//original BTS code
	int iTheirDamage = 0;
	int iOurDamage = 0;

	for (int iRound = 0; iRound < GC.getDefineINT("INTERCEPTION_MAX_ROUNDS"); ++iRound)
	*/
	// For air v air, more rounds and factor in strength for per round damage
	int iOurOdds = (100 * iOurStrength) / std::max(1, iTotalStrength);
	int iMaxRounds = 0;
	int iOurRoundDamage = 0;
	int iTheirRoundDamage = 0;

	// Air v air is more like standard combat
	// Round damage in this case will now depend on strength and interception probability
	if( GC.getBBAI_AIR_COMBAT() && (DOMAIN_AIR == pInterceptor->getDomainType() && DOMAIN_AIR == getDomainType()) )
	{
		int iBaseDamage = GC.getDefineINT("AIR_COMBAT_DAMAGE");
		int iOurFirepower = ((airMaxCombatStr(pInterceptor) + iOurStrength + 1) / 2);
		int iTheirFirepower = ((pInterceptor->airMaxCombatStr(this) + iTheirStrength + 1) / 2);

		int iStrengthFactor = ((iOurFirepower + iTheirFirepower + 1) / 2);

		int iTheirInterception = std::max(pInterceptor->maxInterceptionProbability(),2*GC.getDefineINT("MIN_INTERCEPTION_DAMAGE"));
		int iOurInterception = std::max(maxInterceptionProbability(),2*GC.getDefineINT("MIN_INTERCEPTION_DAMAGE"));

		iOurRoundDamage = std::max(1, ((iBaseDamage * (iTheirFirepower + iStrengthFactor) * iTheirInterception) / ((iOurFirepower + iStrengthFactor) * 100)));
		iTheirRoundDamage = std::max(1, ((iBaseDamage * (iOurFirepower + iStrengthFactor) * iOurInterception) / ((iTheirFirepower + iStrengthFactor) * 100)));

		iMaxRounds = 2*GC.getDefineINT("INTERCEPTION_MAX_ROUNDS") - 1;
	}
	else
	{
		iOurRoundDamage = (pInterceptor->currInterceptionProbability() * GC.getDefineINT("MAX_INTERCEPTION_DAMAGE")) / 100;
		iTheirRoundDamage = (currInterceptionProbability() * GC.getDefineINT("MAX_INTERCEPTION_DAMAGE")) / 100;
		if (getDomainType() == DOMAIN_AIR)
		{
			iTheirRoundDamage = std::max(GC.getDefineINT("MIN_INTERCEPTION_DAMAGE"), iTheirRoundDamage);
		}

		iMaxRounds = GC.getDefineINT("INTERCEPTION_MAX_ROUNDS");
	}

	int iTheirDamage = 0;
	int iOurDamage = 0;

	for (int iRound = 0; iRound < iMaxRounds; ++iRound)
/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						END								*/
/********************************************************************************/
	{
		if (GC.getGameINLINE().getSorenRandNum(100, "Air combat") < iOurOdds)
		{
			if (DOMAIN_AIR == pInterceptor->getDomainType())
			{
				iTheirDamage += iTheirRoundDamage;
				pInterceptor->changeDamage(iTheirRoundDamage, getOwnerINLINE());
				if (pInterceptor->isDead())
				{
					break;
				}
			}
		}
		else
		{
			iOurDamage += iOurRoundDamage;
			changeDamage(iOurRoundDamage, pInterceptor->getOwnerINLINE());
			if (isDead())
			{
				break;
			}
		}
	}

	if (isDead())
	{
		if (iTheirRoundDamage > 0)
		{
			int iExperience = attackXPValue();
			iExperience = (iExperience * iOurStrength) / std::max(1, iTheirStrength);
			iExperience = range(iExperience, GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), GC.getDefineINT("MAX_EXPERIENCE_PER_COMBAT"));
			pInterceptor->changeExperience(iExperience, maxXPValue(), true, pPlot->getOwnerINLINE() == pInterceptor->getOwnerINLINE(), !isBarbarian());
		}
	}
	else if (pInterceptor->isDead())
	{
		int iExperience = pInterceptor->defenseXPValue();
		iExperience = (iExperience * iTheirStrength) / std::max(1, iOurStrength);
		iExperience = range(iExperience, GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), GC.getDefineINT("MAX_EXPERIENCE_PER_COMBAT"));
		changeExperience(iExperience, pInterceptor->maxXPValue(), true, pPlot->getOwnerINLINE() == getOwnerINLINE(), !pInterceptor->isBarbarian());
	}
	else if (iOurDamage > 0)
	{
		if (iTheirRoundDamage > 0)
		{
			pInterceptor->changeExperience(GC.getDefineINT("EXPERIENCE_FROM_WITHDRAWL"), maxXPValue(), true, pPlot->getOwnerINLINE() == pInterceptor->getOwnerINLINE(), !isBarbarian());
		}
	}
	else if (iTheirDamage > 0)
	{
		changeExperience(GC.getDefineINT("EXPERIENCE_FROM_WITHDRAWL"), pInterceptor->maxXPValue(), true, pPlot->getOwnerINLINE() == getOwnerINLINE(), !pInterceptor->isBarbarian());
	}

	kBattle.setDamage(BATTLE_UNIT_ATTACKER, iOurDamage);
	kBattle.setDamage(BATTLE_UNIT_DEFENDER, iTheirDamage);
}


void CvUnit::updateAirCombat(bool bQuick)
{
	CvUnit* pInterceptor = NULL;
	bool bFinish = false;

	FAssert(getDomainType() == DOMAIN_AIR || getDropRange() > 0);

	if (getCombatTimer() > 0)
	{
		changeCombatTimer(-1);

		if (getCombatTimer() > 0)
		{
			return;
		}
		else
		{
			bFinish = true;
		}
	}

	CvPlot* pPlot = getAttackPlot();
	if (pPlot == NULL)
	{
		return;
	}

	if (bFinish)
	{
		pInterceptor = getCombatUnit();
	}
	else
	{
		pInterceptor = bestInterceptor(pPlot);
	}


	if (pInterceptor == NULL)
	{
		setAttackPlot(NULL, false);
		setCombatUnit(NULL);

		getGroup()->clearMissionQueue();

		return;
	}

	//check if quick combat
	bool bVisible = false;
	if (!bQuick)
	{
		bVisible = isCombatVisible(pInterceptor);
	}

	//if not finished and not fighting yet, set up combat damage and mission
	if (!bFinish)
	{
		if (!isFighting())
		{
			if (plot()->isFighting() || pPlot->isFighting())
			{
				return;
			}

			setMadeAttack(true);

			setCombatUnit(pInterceptor, true);
			pInterceptor->setCombatUnit(this, false);
		}

		FAssertMsg(pInterceptor != NULL, "Defender is not assigned a valid value");

		FAssertMsg(plot()->isFighting(), "Current unit instance plot is not fighting as expected");
		FAssertMsg(pInterceptor->plot()->isFighting(), "pPlot is not fighting as expected");

		CvAirMissionDefinition kAirMission;
		if (DOMAIN_AIR != getDomainType())
		{
			kAirMission.setMissionType(MISSION_PARADROP);
		}
		else
		{
			kAirMission.setMissionType(MISSION_AIRSTRIKE);
		}
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, pInterceptor);

		resolveAirCombat(pInterceptor, pPlot, kAirMission);

		if (!bVisible)
		{
			bFinish = true;
		}
		else
		{
			kAirMission.setPlot(pPlot);
			kAirMission.setMissionTime(GC.getMissionInfo(MISSION_AIRSTRIKE).getTime() * gDLL->getSecsPerTurn());
			setCombatTimer(GC.getMissionInfo(MISSION_AIRSTRIKE).getTime());
			GC.getGameINLINE().incrementTurnTimer(getCombatTimer());

			if (pPlot->isActiveVisible(false))
			{
				gDLL->getEntityIFace()->AddMission(&kAirMission);
			}
		}

		changeMoves(GC.getMOVE_DENOMINATOR());
		if (DOMAIN_AIR != pInterceptor->getDomainType())
		{
			pInterceptor->setMadeInterception(true);
		}

		if (isDead())
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_SHOT_DOWN_ENEMY", pInterceptor->getNameKey(), getNameKey(), getVisualCivAdjective(pInterceptor->getTeam()));
			gDLL->getInterfaceIFace()->addMessage(pInterceptor->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_SHOT_DOWN", getNameKey(), pInterceptor->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, pInterceptor->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
		else if (kAirMission.getDamage(BATTLE_UNIT_ATTACKER) > 0)
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_HURT_ENEMY_AIR", pInterceptor->getNameKey(), getNameKey(), -(kAirMission.getDamage(BATTLE_UNIT_ATTACKER)), getVisualCivAdjective(pInterceptor->getTeam()));
			gDLL->getInterfaceIFace()->addMessage(pInterceptor->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIR_UNIT_HURT", getNameKey(), pInterceptor->getNameKey(), -(kAirMission.getDamage(BATTLE_UNIT_ATTACKER)));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, pInterceptor->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}

		if (pInterceptor->isDead())
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_SHOT_DOWN_ENEMY", getNameKey(), pInterceptor->getNameKey(), pInterceptor->getVisualCivAdjective(getTeam()));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, pInterceptor->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_SHOT_DOWN", pInterceptor->getNameKey(), getNameKey());
			gDLL->getInterfaceIFace()->addMessage(pInterceptor->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
		else if (kAirMission.getDamage(BATTLE_UNIT_DEFENDER) > 0)
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_DAMAGED_ENEMY_AIR", getNameKey(), pInterceptor->getNameKey(), -(kAirMission.getDamage(BATTLE_UNIT_DEFENDER)), pInterceptor->getVisualCivAdjective(getTeam()));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, pInterceptor->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_AIR_UNIT_DAMAGED", pInterceptor->getNameKey(), getNameKey(), -(kAirMission.getDamage(BATTLE_UNIT_DEFENDER)));
			gDLL->getInterfaceIFace()->addMessage(pInterceptor->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}

		if (0 == kAirMission.getDamage(BATTLE_UNIT_ATTACKER) + kAirMission.getDamage(BATTLE_UNIT_DEFENDER))
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ABORTED_ENEMY_AIR", pInterceptor->getNameKey(), getNameKey(), getVisualCivAdjective(getTeam()));
			gDLL->getInterfaceIFace()->addMessage(pInterceptor->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, pInterceptor->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_AIR_UNIT_ABORTED", getNameKey(), pInterceptor->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
	}

	if (bFinish)
	{
		setAttackPlot(NULL, false);
		setCombatUnit(NULL);
		pInterceptor->setCombatUnit(NULL);

		if (!isDead() && isSuicide())
		{
			kill(true);
		}
	}
}

void CvUnit::resolveCombat(CvUnit* pDefender, CvPlot* pPlot, CvBattleDefinition& kBattle)
{
	CombatDetails cdAttackerDetails;
	CombatDetails cdDefenderDetails;

	int iAttackerStrength = currCombatStr(NULL, NULL, &cdAttackerDetails);
	int iAttackerFirepower = currFirepower(NULL, NULL);
	int iDefenderStrength;
	int iAttackerDamage;
	int iDefenderDamage;
	int iDefenderOdds;
/************************************************************************************************/
/* Afforess	                  Start		 03/15/10                      Coded By: KillMePlease   */
/*                                                                                              */
/* Occasional Promotions                                                                        */
/************************************************************************************************/
	bool bAttackerWithdrawn = false;
	bool bAttackerHasLostNoHP = true;
	int iAttackerInitialDamage = getDamage();
	int iDefenderInitialDamage = pDefender->getDamage();
	int iInitialDefXP = pDefender->getExperience100();
	int iInitialAttXP = getExperience100();
	int iInitialAttGGXP = GET_PLAYER(getOwnerINLINE()).getCombatExperience();
	int iInitialDefGGXP = GET_PLAYER(pDefender->getOwnerINLINE()).getCombatExperience();
	bool bDynamicXP = GC.getGameINLINE().isModderGameOption(MODDERGAMEOPTION_IMPROVED_XP);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	getDefenderCombatValues(*pDefender, pPlot, iAttackerStrength, iAttackerFirepower, iDefenderOdds, iDefenderStrength, iAttackerDamage, iDefenderDamage, &cdDefenderDetails);
	int iAttackerKillOdds = iDefenderOdds * (100 - withdrawalProbability()) / 100;

	if (isHuman() || pDefender->isHuman())
	{
		//Added ST
		CyArgsList pyArgsCD;
		pyArgsCD.add(gDLL->getPythonIFace()->makePythonObject(&cdAttackerDetails));
		pyArgsCD.add(gDLL->getPythonIFace()->makePythonObject(&cdDefenderDetails));
		pyArgsCD.add(getCombatOdds(this, pDefender));
		CvEventReporter::getInstance().genericEvent("combatLogCalc", pyArgsCD.makeFunctionArgs());
	}

	collateralCombat(pPlot, pDefender);
/************************************************************************************************/
/* Afforess	                  Start		 02/22/10                Coded by: KillMePlease         */
/*                                                                                              */
/*   Defender Withdraw                                                                          */
/************************************************************************************************/
	int iCloseCombatRoundNum = -1;
	bool bTryMobileWithdraw = false;	//if unit will be trying to withdraw from a plot it occupies
	if (pPlot->getNumDefenders(pDefender->getOwner()) == 1 && pDefender->baseMoves() > baseMoves())	//must be faster than attacker
	{
		bTryMobileWithdraw = true;
	}
	int iWinningOdds = getCombatOdds(this, pDefender);
	bool bDefenderSkirmish = false; //iWinningOdds > 60;
	m_combatResult.bDefenderWithdrawn = false;
	m_combatResult.pPlot = NULL;
	m_combatResult.iAttacksCount++;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	
	while (true)
	{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Lead From Behind                                                                             */
/************************************************************************************************/
		// From Lead From Behind by UncutDragon
/* original code
		if (GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("COMBAT_DIE_SIDES"), "Combat") < iDefenderOdds)
*/		// modified
		if (GC.getGameINLINE().getSorenRandNum(GC.getCOMBAT_DIE_SIDES(), "Combat") < iDefenderOdds)
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

		{
			if (getCombatFirstStrikes() == 0)
			{
				if (getDamage() + iAttackerDamage >= maxHitPoints() && GC.getGameINLINE().getSorenRandNum(100, "Withdrawal") < withdrawalProbability())
				{
					flankingStrikeCombat(pPlot, iAttackerStrength, iAttackerFirepower, iAttackerKillOdds, iDefenderDamage, pDefender);
/************************************************************************************************/
/* Afforess	                  Start		 03/15/10                      Coded By: KillMePlease   */
/*                                                                                              */
/* Occasional Promotions                                                                        */
/************************************************************************************************/
					bAttackerWithdrawn = true;
	
/**	Great Generals From Barbarian Combat Start													**/

				if (!bDynamicXP)
					changeExperience(GC.getDefineINT("EXPERIENCE_FROM_WITHDRAWL"), pDefender->maxXPValue(), true, pPlot->getOwnerINLINE() == getOwnerINLINE(), (!pDefender->isBarbarian() || GC.getGameINLINE().isOption(GAMEOPTION_BARBARIAN_GENERALS)));
					
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
// BUG - Combat Events - start
					CvEventReporter::getInstance().combatRetreat(this, pDefender);
// BUG - Combat Events - end
					break;
				}

				changeDamage(iAttackerDamage, pDefender->getOwnerINLINE());
/************************************************************************************************/
/* Afforess	                  Start		 03/15/10                      Coded By: KillMePlease   */
/*                                                                                              */
/* Occasional Promotions                                                                        */
/************************************************************************************************/
				bAttackerHasLostNoHP = false;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				if (pDefender->getCombatFirstStrikes() > 0 && pDefender->isRanged())
				{
					kBattle.addFirstStrikes(BATTLE_UNIT_DEFENDER, 1);
					kBattle.addDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_RANGED, iAttackerDamage);
				}

				cdAttackerDetails.iCurrHitPoints = currHitPoints();

				if (isHuman() || pDefender->isHuman())
				{
					CyArgsList pyArgs;
					pyArgs.add(gDLL->getPythonIFace()->makePythonObject(&cdAttackerDetails));
					pyArgs.add(gDLL->getPythonIFace()->makePythonObject(&cdDefenderDetails));
					pyArgs.add(1);
					pyArgs.add(iAttackerDamage);
					CvEventReporter::getInstance().genericEvent("combatLogHit", pyArgs.makeFunctionArgs());
				}
			}
		}
		else
		{
			if (pDefender->getCombatFirstStrikes() == 0)
			{
/************************************************************************************************/
/* Afforess	                  Start		 02/22/10                        Coded by: KillMePlease */
/*                                                                                              */
/*   Defender Withdraw                                                                          */
/************************************************************************************************/
				if (GC.getGameINLINE().isModderGameOption(MODDERGAMEOPTION_DEFENDER_WITHDRAW))
				{
					iCloseCombatRoundNum++;
				}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				if (std::min(GC.getMAX_HIT_POINTS(), pDefender->getDamage() + iDefenderDamage) > combatLimit())
				{
					if (!bDynamicXP)
						changeExperience(GC.getDefineINT("EXPERIENCE_FROM_WITHDRAWL"), pDefender->maxXPValue(), true, pPlot->getOwnerINLINE() == getOwnerINLINE(), !pDefender->isBarbarian());
					pDefender->setDamage(combatLimit(), getOwnerINLINE());
// BUG - Combat Events - start
					CvEventReporter::getInstance().combatWithdrawal(this, pDefender);
// BUG - Combat Events - end
					break;
				}
				
/************************************************************************************************/
/* Afforess	                  Start		 02/22/10                Coded by: KillMePlease         */
/*                                                                                              */
/* Defender Withdraw                                                                            */
/************************************************************************************************/
				else if (GC.getGameINLINE().isModderGameOption(MODDERGAMEOPTION_DEFENDER_WITHDRAW) && (
				(pDefender->getDamage() + iDefenderDamage >= maxHitPoints() || bDefenderSkirmish) && 
				GC.getGameINLINE().getSorenRandNum(100, "Withdrawal") < pDefender->withdrawalProbability() 
				&& !isSuicide() && iCloseCombatRoundNum > 0) )	//can not to escape at close combat round 1
				{
					//attacker got experience
					int iExperience = pDefender->attackXPValue();
					iExperience = ((iExperience * iDefenderStrength) / iAttackerStrength);
					iExperience = range(iExperience, GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), GC.getDefineINT("MAX_EXPERIENCE_PER_COMBAT"));
					changeExperience(iExperience, pDefender->maxXPValue(), true, pPlot->getOwnerINLINE() == getOwnerINLINE(), (!pDefender->isBarbarian() || GC.getGameINLINE().isOption(GAMEOPTION_BARBARIAN_GENERALS)));

					//if (pPlot->getNumDefenders(pDefender->getOwner()) == 1 && pDefender->baseMoves() > baseMoves())	//must be faster to flee a battle
					{
						bool bEnemy = true;
						for (int iPlot = 0; iPlot < NUM_DIRECTION_TYPES; iPlot++)
						{
							CvPlot* pAdjacentPlot = plotDirection(pDefender->plot()->getX_INLINE(), pDefender->plot()->getY_INLINE(), ((DirectionTypes)iPlot));
							if (pAdjacentPlot != NULL)
							{
								if (pDefender->canMoveInto(pAdjacentPlot))
								{
									//Check that this tile is safe (ie, no attackers next to it)
									for (int iPlot2 = 0; iPlot2 < NUM_DIRECTION_TYPES; iPlot2++)
									{
										CvPlot* pAdjacentPlot2 = plotDirection(pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE(), ((DirectionTypes)iPlot2));
										if (pAdjacentPlot2 != NULL)
										{
											CLLNode<IDInfo>* pUnitNode;
											pUnitNode = pAdjacentPlot2->headUnitNode();
											CvUnit* pLoopUnit;
											bEnemy = false;
											while (pUnitNode != NULL)
											{
												pLoopUnit = ::getUnit(pUnitNode->m_data);
												pUnitNode = pAdjacentPlot2->nextUnitNode(pUnitNode);	
												if (GET_TEAM(pLoopUnit->getTeam()).isAtWar(pDefender->getTeam()))
												{
													bEnemy = true;
													break;
												}
											}
											if (bEnemy)
											{
												break;
											}
										}
									}
									if (!bEnemy)
									{
										m_combatResult.pPlot = pAdjacentPlot;
										m_combatResult.bDefenderWithdrawn = true;
										
										if (!bDynamicXP)
											pDefender->changeExperience(GC.getDefineINT("EXPERIENCE_FROM_WITHDRAWL"), pDefender->maxXPValue(), 
								true, pPlot->getOwnerINLINE() == pDefender->getOwnerINLINE(), (!isBarbarian() || GC.getGameINLINE().isOption(GAMEOPTION_BARBARIAN_GENERALS)));

										doDynamicXP(pDefender, pPlot, iAttackerInitialDamage, iWinningOdds, iDefenderInitialDamage, iInitialAttXP, iInitialDefXP, iInitialAttGGXP, iInitialDefGGXP, false, false);
										return;
									}
								}
							}
						}
					}
				}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

				pDefender->changeDamage(iDefenderDamage, getOwnerINLINE());

				if (getCombatFirstStrikes() > 0 && isRanged())
				{
					kBattle.addFirstStrikes(BATTLE_UNIT_ATTACKER, 1);
					kBattle.addDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_RANGED, iDefenderDamage);
				}

				cdDefenderDetails.iCurrHitPoints=pDefender->currHitPoints();

				if (isHuman() || pDefender->isHuman())
				{
					CyArgsList pyArgs;
					pyArgs.add(gDLL->getPythonIFace()->makePythonObject(&cdAttackerDetails));
					pyArgs.add(gDLL->getPythonIFace()->makePythonObject(&cdDefenderDetails));
					pyArgs.add(0);
					pyArgs.add(iDefenderDamage);
					CvEventReporter::getInstance().genericEvent("combatLogHit", pyArgs.makeFunctionArgs());
				}
			}
		}

		if (getCombatFirstStrikes() > 0)
		{
			changeCombatFirstStrikes(-1);
		}

		if (pDefender->getCombatFirstStrikes() > 0)
		{
			pDefender->changeCombatFirstStrikes(-1);
		}

		if (isDead() || pDefender->isDead())
		{
			if (isDead())
			{
				int iExperience = defenseXPValue();
				iExperience = ((iExperience * iAttackerStrength) / iDefenderStrength);
				iExperience = range(iExperience, GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), GC.getDefineINT("MAX_EXPERIENCE_PER_COMBAT"));
/*************************************************************************************************/
/**	Great Generals From Barbarian Combat Start													**/
/**			Oct 19 2009																			**/
/**																								**/
/*************************************************************************************************/
			if (!bDynamicXP)
				pDefender->changeExperience(iExperience, maxXPValue(), true, pPlot->getOwnerINLINE() == pDefender->getOwnerINLINE(), (!isBarbarian() || GC.getGameINLINE().isOption(GAMEOPTION_BARBARIAN_GENERALS)));
/*************************************************************************************************/
/**	Great Generals From Barbarian Combat End													**/
/*************************************************************************************************/
			}
			else
			{
				flankingStrikeCombat(pPlot, iAttackerStrength, iAttackerFirepower, iAttackerKillOdds, iDefenderDamage, pDefender);

				int iExperience = pDefender->attackXPValue();
				iExperience = ((iExperience * iDefenderStrength) / iAttackerStrength);
				iExperience = range(iExperience, GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), GC.getDefineINT("MAX_EXPERIENCE_PER_COMBAT"));
/*************************************************************************************************/
/**	Great Generals From Barbarian Combat Start													**/
/**			Oct 19 2009																			**/
/**																								**/
/*************************************************************************************************/
				if (!bDynamicXP)
					changeExperience(iExperience, pDefender->maxXPValue(), true, pPlot->getOwnerINLINE() == getOwnerINLINE(), (!pDefender->isBarbarian() || GC.getGameINLINE().isOption(GAMEOPTION_BARBARIAN_GENERALS)));
/*************************************************************************************************/
/**	Great Generals From Barbarian Combat End													**/
/*************************************************************************************************/
			}
			break;
		}
	}

/************************************************************************************************/
/* Afforess	                  Start		 05/6/10                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	bool bPromotion = false;
	bool bDefPromotion = false;
	doBattleFieldPromotions(pDefender, cdDefenderDetails, pPlot, bAttackerHasLostNoHP, bAttackerWithdrawn, iAttackerInitialDamage, iWinningOdds, iInitialAttXP, iInitialAttGGXP, iDefenderInitialDamage, iInitialDefXP, iInitialDefGGXP, bPromotion, bDefPromotion);
	doDynamicXP(pDefender, pPlot, iAttackerInitialDamage, iWinningOdds, iDefenderInitialDamage, iInitialAttXP, iInitialDefXP, iInitialAttGGXP, iInitialDefGGXP, bPromotion, bDefPromotion);
	
	doCommerceAttacks(pDefender, pPlot);
	doCommerceAttacks(this, pPlot);		
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
}


void CvUnit::updateCombat(bool bQuick)
{
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - SA: Stack Attack START
	if (GC.getDCM_STACK_ATTACK())
	{
		updateStackCombat(bQuick);
		return;
	}
	// Dale - SA: Stack Attack END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/

	CvWString szBuffer;

	bool bFinish = false;
	bool bVisible = false;

	if (getCombatTimer() > 0)
	{
		changeCombatTimer(-1);

		if (getCombatTimer() > 0)
		{
			return;
		}
		else
		{
			bFinish = true;
		}
	}

	CvPlot* pPlot = getAttackPlot();

	if (pPlot == NULL)
	{
		return;
	}

	if (getDomainType() == DOMAIN_AIR)
	{
		updateAirStrike(pPlot, bQuick, bFinish);
		return;
	}

	CvUnit* pDefender = NULL;
	if (bFinish)
	{
		pDefender = getCombatUnit();
	}
	else
	{
		pDefender = pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);
	}

	if (pDefender == NULL)
	{
		setAttackPlot(NULL, false);
		setCombatUnit(NULL);

		getGroup()->groupMove(pPlot, true, ((canAdvance(pPlot, 0)) ? this : NULL));

		getGroup()->clearMissionQueue();

		return;
	}

	//check if quick combat
	if (!bQuick)
	{
		bVisible = isCombatVisible(pDefender);
	}

	//FAssertMsg((pPlot == pDefender->plot()), "There is not expected to be a defender or the defender's plot is expected to be pPlot (the attack plot)");

	//if not finished and not fighting yet, set up combat damage and mission
	if (!bFinish)
	{
		if (!isFighting())
		{
			if (plot()->isFighting() || pPlot->isFighting())
			{
				return;
			}

			setMadeAttack(true);

			//rotate to face plot
			DirectionTypes newDirection = estimateDirection(this->plot(), pDefender->plot());
			if (newDirection != NO_DIRECTION)
			{
				setFacingDirection(newDirection);
			}

			//rotate enemy to face us
			newDirection = estimateDirection(pDefender->plot(), this->plot());
			if (newDirection != NO_DIRECTION)
			{
				pDefender->setFacingDirection(newDirection);
			}

			setCombatUnit(pDefender, true);
			pDefender->setCombatUnit(this, false);

			pDefender->getGroup()->clearMissionQueue();

			bool bFocused = (bVisible && isCombatFocus() && gDLL->getInterfaceIFace()->isCombatFocus());

			if (bFocused)
			{
				DirectionTypes directionType = directionXY(plot(), pPlot);
				//								N			NE				E				SE					S				SW					W				NW
				NiPoint2 directions[8] = {NiPoint2(0, 1), NiPoint2(1, 1), NiPoint2(1, 0), NiPoint2(1, -1), NiPoint2(0, -1), NiPoint2(-1, -1), NiPoint2(-1, 0), NiPoint2(-1, 1)};
				NiPoint3 attackDirection = NiPoint3(directions[directionType].x, directions[directionType].y, 0);
				float plotSize = GC.getPLOT_SIZE();
				NiPoint3 lookAtPoint(plot()->getPoint().x + plotSize / 2 * attackDirection.x, plot()->getPoint().y + plotSize / 2 * attackDirection.y, (plot()->getPoint().z + pPlot->getPoint().z) / 2);
				attackDirection.Unitize();
				gDLL->getInterfaceIFace()->lookAt(lookAtPoint, (((getOwnerINLINE() != GC.getGameINLINE().getActivePlayer()) || gDLL->getGraphicOption(GRAPHICOPTION_NO_COMBAT_ZOOM)) ? CAMERALOOKAT_BATTLE : CAMERALOOKAT_BATTLE_ZOOM_IN), attackDirection);
			}
			else
			{
				PlayerTypes eAttacker = getVisualOwner(pDefender->getTeam());
				CvWString szMessage;
				if (BARBARIAN_PLAYER != eAttacker)
				{
					szMessage = gDLL->getText("TXT_KEY_MISC_YOU_UNITS_UNDER_ATTACK", GET_PLAYER(getOwnerINLINE()).getNameKey());
				}
				else
				{
					szMessage = gDLL->getText("TXT_KEY_MISC_YOU_UNITS_UNDER_ATTACK_UNKNOWN");
				}

				gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szMessage, "AS2D_COMBAT", MESSAGE_TYPE_DISPLAY_ONLY, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true);
			}
		}

		FAssertMsg(pDefender != NULL, "Defender is not assigned a valid value");

		FAssertMsg(plot()->isFighting(), "Current unit instance plot is not fighting as expected");
		FAssertMsg(pPlot->isFighting(), "pPlot is not fighting as expected");

		if (!pDefender->canDefend())
		{
			if (!bVisible)
			{
				bFinish = true;
			}
			else
			{
				CvMissionDefinition kMission;
				kMission.setMissionTime(getCombatTimer() * gDLL->getSecsPerTurn());
				kMission.setMissionType(MISSION_SURRENDER);
				kMission.setUnit(BATTLE_UNIT_ATTACKER, this);
				kMission.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
				kMission.setPlot(pPlot);
				gDLL->getEntityIFace()->AddMission(&kMission);

				// Surrender mission
				setCombatTimer(GC.getMissionInfo(MISSION_SURRENDER).getTime());

				GC.getGameINLINE().incrementTurnTimer(getCombatTimer());
			}

			// Kill them!
			pDefender->setDamage(GC.getMAX_HIT_POINTS());
		}
		else
		{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
			//USE commanders here (so their command points will be decreased) for attacker and defender:
			this->tryUseCommander();
			pDefender->tryUseCommander();
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			CvBattleDefinition kBattle;
			kBattle.setUnit(BATTLE_UNIT_ATTACKER, this);
			kBattle.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
			kBattle.setDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_BEGIN, getDamage());
			kBattle.setDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_BEGIN, pDefender->getDamage());

			resolveCombat(pDefender, pPlot, kBattle);

			if (!bVisible)
			{
				bFinish = true;
			}
			else
			{
				kBattle.setDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_END, getDamage());
				kBattle.setDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_END, pDefender->getDamage());
				kBattle.setAdvanceSquare(canAdvance(pPlot, 1));

				if (isRanged() && pDefender->isRanged())
				{
					kBattle.setDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_END));
					kBattle.setDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_END));
				}
				else
				{
					kBattle.addDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_BEGIN));
					kBattle.addDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_BEGIN));
				}

				int iTurns = planBattle( kBattle);
				kBattle.setMissionTime(iTurns * gDLL->getSecsPerTurn());
				setCombatTimer(iTurns);

				GC.getGameINLINE().incrementTurnTimer(getCombatTimer());

				if (pPlot->isActiveVisible(false))
				{
					ExecuteMove(0.5f, true);
					gDLL->getEntityIFace()->AddMission(&kBattle);
				}
			}
		}
	}

	if (bFinish)
	{
		if (bVisible)
		{
			if (isCombatFocus() && gDLL->getInterfaceIFace()->isCombatFocus())
			{
				if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
				{
					gDLL->getInterfaceIFace()->releaseLockedCamera();
				}
			}
		}

		//end the combat mission if this code executes first
		gDLL->getEntityIFace()->RemoveUnitFromBattle(this);
		gDLL->getEntityIFace()->RemoveUnitFromBattle(pDefender);

		setAttackPlot(NULL, false);

		setCombatUnit(NULL);
		pDefender->setCombatUnit(NULL);
		NotifyEntity(MISSION_DAMAGE);
		pDefender->NotifyEntity(MISSION_DAMAGE);

		if (isDead())
		{
			if (isBarbarian())
			{
				GET_PLAYER(pDefender->getOwnerINLINE()).changeWinsVsBarbs(1);
			}

			if (!m_pUnitInfo->isHiddenNationality() && !pDefender->getUnitInfo().isHiddenNationality())
			{
				GET_TEAM(getTeam()).changeWarWeariness(pDefender->getTeam(), *pPlot, GC.getDefineINT("WW_UNIT_KILLED_ATTACKING"));
				GET_TEAM(pDefender->getTeam()).changeWarWeariness(getTeam(), *pPlot, GC.getDefineINT("WW_KILLED_UNIT_DEFENDING"));
				GET_TEAM(pDefender->getTeam()).AI_changeWarSuccess(getTeam(), GC.getDefineINT("WAR_SUCCESS_DEFENDING"));
			}

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_DIED_ATTACKING", getNameKey(), pDefender->getNameKey());
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/
			// ------ BEGIN InfluenceDrivenWar -------------------------------
			float fInfluenceRatio = 0.0;
			if (GC.getDefineINT("IDW_ENABLED"))
			{
				fInfluenceRatio = pDefender->doVictoryInfluence(this, false, false);
/************************************************************************************************/
/* Afforess	                  Start		 02/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
				if (fInfluenceRatio > 0.0) 
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				{
				CvWString szTempBuffer;
				szTempBuffer.Format(L" Influence: -%.1f%%", fInfluenceRatio);
				szBuffer += szTempBuffer;
				}
			}
			// ------ END InfluenceDrivenWar ---------------------------------
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_KILLED_ENEMY_UNIT", pDefender->getNameKey(), getNameKey(), getVisualCivAdjective(pDefender->getTeam()));
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/
			// ------ BEGIN InfluenceDrivenWar -------------------------------
			if (GC.getDefineINT("IDW_ENABLED"))
			{
/************************************************************************************************/
/* Afforess	                  Start		 02/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
				if (fInfluenceRatio > 0.0) 
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				{
				CvWString szTempBuffer;
				szTempBuffer.Format(L" Influence: +%.1f%%", fInfluenceRatio);
				szBuffer += szTempBuffer;
				}
			}
			// ------ END InfluenceDrivenWar ---------------------------------
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/27/09                                                        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
            pDefender->combatWon(this, false);
/************************************************************************************************/
/* JOOYO_ADDON                          END                                                     */
/************************************************************************************************/
			// report event to Python, along with some other key state
			CvEventReporter::getInstance().combatResult(pDefender, this);
		}
		else if (pDefender->isDead())
		{
			if (pDefender->isBarbarian())
			{
				GET_PLAYER(getOwnerINLINE()).changeWinsVsBarbs(1);
			}

			if (!m_pUnitInfo->isHiddenNationality() && !pDefender->getUnitInfo().isHiddenNationality())
			{
				GET_TEAM(pDefender->getTeam()).changeWarWeariness(getTeam(), *pPlot, GC.getDefineINT("WW_UNIT_KILLED_DEFENDING"));
				GET_TEAM(getTeam()).changeWarWeariness(pDefender->getTeam(), *pPlot, GC.getDefineINT("WW_KILLED_UNIT_ATTACKING"));
				GET_TEAM(getTeam()).AI_changeWarSuccess(pDefender->getTeam(), GC.getDefineINT("WAR_SUCCESS_ATTACKING"));
			}

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_DESTROYED_ENEMY", getNameKey(), pDefender->getNameKey());

/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/
			// ------ BEGIN InfluenceDrivenWar -------------------------------
			float fInfluenceRatio = 0.0;			
			if (GC.getDefineINT("IDW_ENABLED"))
			{
				fInfluenceRatio = doVictoryInfluence(pDefender, true, false);
/************************************************************************************************/
/* Afforess	                  Start		 02/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
				if (fInfluenceRatio > 0.0) 
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				{
				CvWString szTempBuffer;
				szTempBuffer.Format(L" Influence: +%.1f%%", fInfluenceRatio);
				szBuffer += szTempBuffer;
				}
			}
			// ------ END InfluenceDrivenWar ---------------------------------
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			if (getVisualOwner(pDefender->getTeam()) != getOwnerINLINE())
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED_UNKNOWN", pDefender->getNameKey(), getNameKey());
			}
			else
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED", pDefender->getNameKey(), getNameKey(), getVisualCivAdjective(pDefender->getTeam()));
			}
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/
			// ------ BEGIN InfluenceDrivenWar -------------------------------
			if (GC.getDefineINT("IDW_ENABLED"))
			{
/************************************************************************************************/
/* Afforess	                  Start		 02/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
				if (fInfluenceRatio > 0.0) 
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				{
				CvWString szTempBuffer;
				szTempBuffer.Format(L" Influence: -%.1f%%", fInfluenceRatio);
				szBuffer += szTempBuffer;
				}
			}
			// ------ END InfluenceDrivenWar ---------------------------------
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer,GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 06/27/09                                                        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
            combatWon(pDefender, true);
/************************************************************************************************/
/* JOOYO_ADDON                          END                                                     */
/************************************************************************************************/
			// report event to Python, along with some other key state
			CvEventReporter::getInstance().combatResult(this, pDefender);

			bool bAdvance = false;

			if (isSuicide())
			{
				kill(true);

				pDefender->kill(false);
				pDefender = NULL;
			}
			else
			{
				bAdvance = canAdvance(pPlot, ((pDefender->canDefend()) ? 1 : 0));

				if (bAdvance)
				{
					if (!isNoCapture())
					{
						pDefender->setCapturingPlayer(getOwnerINLINE());
					}
				}


				pDefender->kill(false);
				pDefender = NULL;

				if (!bAdvance)
				{
					changeMoves(std::max(GC.getMOVE_DENOMINATOR(), pPlot->movementCost(this, plot())));
					checkRemoveSelectionAfterAttack();
				}
			}

/****************************************************************************************/
/* REVOLUTIONDCM				09/06/09						jdog5000                */
/**																						*/
/**																						*/
/****************************************************************************************/
			// Fix rare crash bug
			if( getGroup() != NULL )
			{
				if (pPlot->getNumVisibleEnemyDefenders(this) == 0)
				{
					getGroup()->groupMove(pPlot, true, ((bAdvance) ? this : NULL));
				}

				// This is is put before the plot advancement, the unit will always try to walk back
				// to the square that they came from, before advancing.
				getGroup()->clearMissionQueue();
			}
/****************************************************************************************/
/* REVOLUTIONDCM				END      						jdog5000                */
/****************************************************************************************/
		}
/************************************************************************************************/
/* Afforess	                  Start		 02/22/10                    Coded By: KillMePlease     */
/*                                                                                              */
/*  Defender Withdraw                                                                           */
/************************************************************************************************/
		else if (m_combatResult.bDefenderWithdrawn)
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_UNIT_WITHDRAW", pDefender->getNameKey(), getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_OUR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_WITHDRAW", pDefender->getNameKey(), getNameKey());
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_THEIR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			
			if (pPlot->isCity())
			{
				m_combatResult.pPlot = NULL;
			}

			if (m_combatResult.pPlot != NULL)
			{
				//defender escapes to a safe plot
				pDefender->move(m_combatResult.pPlot, true);
				changeMoves(std::max(GC.getMOVE_DENOMINATOR(), pPlot->movementCost(this, plot())));
				checkRemoveSelectionAfterAttack();
				getGroup()->clearMissionQueue();
			}
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		else
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_WITHDRAW", getNameKey(), pDefender->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_OUR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_UNIT_WITHDRAW", getNameKey(), pDefender->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_THEIR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

			changeMoves(std::max(GC.getMOVE_DENOMINATOR(), pPlot->movementCost(this, plot())));
			checkRemoveSelectionAfterAttack();
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/
			// ------ BEGIN InfluenceDrivenWar -------------------------------
			if (GC.getDefineINT("IDW_ENABLED"))
			{	
				if (!canMove() || !isBlitz())
				{
					if (IsSelected())
					{
						if (gDLL->getInterfaceIFace()->getLengthSelectionList() > 1)
						{
							gDLL->getInterfaceIFace()->removeFromSelectionList(this);
						}
					}
				}
			}
			// ------ END InfluenceDrivenWar -------------------------------
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/

			getGroup()->clearMissionQueue();
		}
	}
}

void CvUnit::checkRemoveSelectionAfterAttack()
{
	if (!canMove() || !isBlitz())
	{
		if (IsSelected())
		{
			if (gDLL->getInterfaceIFace()->getLengthSelectionList() > 1)
			{
				gDLL->getInterfaceIFace()->removeFromSelectionList(this);
			}
		}
	}
}


bool CvUnit::isActionRecommended(int iAction)
{
	CvCity* pWorkingCity;
	CvPlot* pPlot;
	ImprovementTypes eImprovement;
	ImprovementTypes eFinalImprovement;
	BuildTypes eBuild;
	RouteTypes eRoute;
	BonusTypes eBonus;
	int iIndex;

	if (getOwnerINLINE() != GC.getGameINLINE().getActivePlayer())
	{
		return false;
	}

	if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_NO_UNIT_RECOMMENDATIONS))
	{
		return false;
	}

	CyUnit* pyUnit = new CyUnit(this);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
	argsList.add(iAction);
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "isActionRecommended", argsList.makeFunctionArgs(), &lResult);
	delete pyUnit;	// python fxn must not hold on to this pointer 
	if (lResult == 1)
	{
		return true;
	}

	pPlot = gDLL->getInterfaceIFace()->getGotoPlot();

	if (pPlot == NULL)
	{
		if (gDLL->shiftKey())
		{
			pPlot = getGroup()->lastMissionPlot();
		}
	}

	if (pPlot == NULL)
	{
		pPlot = plot();
	}

// BUFFY - Don't Recommend Actions in Fog of War - start
#ifdef _BUFFY
	// from HOF Mod - Denniz
	if (!pPlot->isVisible(GC.getGameINLINE().getActiveTeam(), false))
	{
		return false;
	}
#endif
// BUFFY - Don't Recommend Actions in Fog of War - end

	if (GC.getActionInfo(iAction).getMissionType() == MISSION_FORTIFY)
	{
		if (pPlot->isCity(true, getTeam()))
		{
			if (canDefend(pPlot))
			{
				if (pPlot->getNumDefenders(getOwnerINLINE()) < ((atPlot(pPlot)) ? 2 : 1))
				{
					return true;
				}
			}
		}
	}

// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
	if ((GC.getActionInfo(iAction).getMissionType() == MISSION_HEAL) || (GC.getActionInfo(iAction).getMissionType() == MISSION_SENTRY_WHILE_HEAL))
#else
	if (GC.getActionInfo(iAction).getMissionType() == MISSION_HEAL)
#endif
// BUG - Sentry Actions - end
	{
		if (isHurt())
		{
			if (!hasMoved())
			{
				if ((pPlot->getTeam() == getTeam()) || (healTurns(pPlot) < 4))
				{
					return true;
				}
			}
		}
	}

	if (GC.getActionInfo(iAction).getMissionType() == MISSION_FOUND)
	{
		if (canFound(pPlot))
		{
			if (pPlot->isBestAdjacentFound(getOwnerINLINE()))
			{
				return true;
			}
		}
	}

	if (GC.getActionInfo(iAction).getMissionType() == MISSION_BUILD)
	{
		if (pPlot->getOwnerINLINE() == getOwnerINLINE())
		{
			eBuild = ((BuildTypes)(GC.getActionInfo(iAction).getMissionData()));
			FAssert(eBuild != NO_BUILD);
			FAssertMsg(eBuild < GC.getNumBuildInfos(), "Invalid Build");

			if (canBuild(pPlot, eBuild))
			{
				eImprovement = ((ImprovementTypes)(GC.getBuildInfo(eBuild).getImprovement()));
				eRoute = ((RouteTypes)(GC.getBuildInfo(eBuild).getRoute()));
				eBonus = pPlot->getBonusType(getTeam());
				pWorkingCity = pPlot->getWorkingCity();

				if (pPlot->getImprovementType() == NO_IMPROVEMENT)
				{
					if (pWorkingCity != NULL)
					{
						iIndex = pWorkingCity->getCityPlotIndex(pPlot);

						if (iIndex != -1)
						{
							if (pWorkingCity->AI_getBestBuild(iIndex) == eBuild)
							{
								return true;
							}
						}
					}

					if (eImprovement != NO_IMPROVEMENT)
					{
						if (eBonus != NO_BONUS)
						{
							if (GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eBonus))
							{
								return true;
							}
						}

						if (pPlot->getImprovementType() == NO_IMPROVEMENT)
						{
							if (!(pPlot->isIrrigated()) && pPlot->isIrrigationAvailable(true))
							{
								if (GC.getImprovementInfo(eImprovement).isCarriesIrrigation())
								{
									return true;
								}
							}

							if (pWorkingCity != NULL)
							{
								if (GC.getImprovementInfo(eImprovement).getYieldChange(YIELD_FOOD) > 0)
								{
									return true;
								}

								if (pPlot->isHills())
								{
									if (GC.getImprovementInfo(eImprovement).getYieldChange(YIELD_PRODUCTION) > 0)
									{
										return true;
									}
								}
								else
								{
									if (GC.getImprovementInfo(eImprovement).getYieldChange(YIELD_COMMERCE) > 0)
									{
										return true;
									}
								}
							}
						}
					}
				}

				if (eRoute != NO_ROUTE)
				{
					if (!(pPlot->isRoute()))
					{
						if (eBonus != NO_BONUS)
						{
							return true;
						}

						if (pWorkingCity != NULL)
						{
							if (pPlot->isRiver())
							{
								return true;
							}
						}
					}

					eFinalImprovement = eImprovement;

					if (eFinalImprovement == NO_IMPROVEMENT)
					{
						eFinalImprovement = pPlot->getImprovementType();
					}

					if (eFinalImprovement != NO_IMPROVEMENT)
					{
						if ((GC.getImprovementInfo(eFinalImprovement).getRouteYieldChanges(eRoute, YIELD_FOOD) > 0) ||
							(GC.getImprovementInfo(eFinalImprovement).getRouteYieldChanges(eRoute, YIELD_PRODUCTION) > 0) ||
							(GC.getImprovementInfo(eFinalImprovement).getRouteYieldChanges(eRoute, YIELD_COMMERCE) > 0))
						{
							return true;
						}
					}
				}
			}
		}
	}

	if (GC.getActionInfo(iAction).getCommandType() == COMMAND_PROMOTION)
	{
		return true;
	}

	return false;
}


/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Efficiency, Lead From Behind                                                                 */
/************************************************************************************************/
// From Lead From Behind by UncutDragon
/* original code
bool CvUnit::isBetterDefenderThan(const CvUnit* pDefender, const CvUnit* pAttacker) const
*/ // modified (with extra parameter)
bool CvUnit::isBetterDefenderThan(const CvUnit* pDefender, const CvUnit* pAttacker, int* pBestDefenderRank) const
{
	int iOurDefense;
	int iTheirDefense;

	if (pDefender == NULL)
	{
		return true;
	}

	TeamTypes eAttackerTeam = NO_TEAM;
	if (NULL != pAttacker)
	{
		eAttackerTeam = pAttacker->getTeam();
	}

	if (canCoexistWithEnemyUnit(eAttackerTeam))
	{
		return false;
	}

	if (!canDefend())
	{
		return false;
	}

	if (canDefend() && !(pDefender->canDefend()))
	{
		return true;
	}

	if (pAttacker)
	{
		if (isTargetOf(*pAttacker) && !pDefender->isTargetOf(*pAttacker))
		{
			return true;
		}

		if (!isTargetOf(*pAttacker) && pDefender->isTargetOf(*pAttacker))
		{
			return false;
		}

		if (pAttacker->canAttack(*pDefender) && !pAttacker->canAttack(*this))
		{
			return false;
		}

		if (pAttacker->canAttack(*this) && !pAttacker->canAttack(*pDefender))
		{
			return true;
		}
	}

	// UncutDragon
	// To cut down on changes to existing code, we just short-circuit the method
	// and this point and call our own version instead
	if (GC.getLFBEnable())
		return LFBisBetterDefenderThan(pDefender, pAttacker, pBestDefenderRank);
	// /UncutDragon

	iOurDefense = currCombatStr(plot(), pAttacker);
	if (::isWorldUnitClass(getUnitClassType()))
	{
		iOurDefense /= 2;
	}

	if (NULL == pAttacker)
	{
		if (pDefender->collateralDamage() > 0)
		{
			iOurDefense *= (100 + pDefender->collateralDamage());
			iOurDefense /= 100;
		}

		if (pDefender->currInterceptionProbability() > 0)
		{
			iOurDefense *= (100 + pDefender->currInterceptionProbability());
			iOurDefense /= 100;
		}
	}
	else
	{
		if (!(pAttacker->immuneToFirstStrikes()))
		{
			// UncutDragon
/* original code
			iOurDefense *= ((((firstStrikes() * 2) + chanceFirstStrikes()) * ((GC.getDefineINT("COMBAT_DAMAGE") * 2) / 5)) + 100);
*/			// modified
			iOurDefense *= ((((firstStrikes() * 2) + chanceFirstStrikes()) * ((GC.getCOMBAT_DAMAGE() * 2) / 5)) + 100);
			// /UncutDragon
			iOurDefense /= 100;
		}

		if (immuneToFirstStrikes())
		{
			// UncutDragon
/* original code
			iOurDefense *= ((((pAttacker->firstStrikes() * 2) + pAttacker->chanceFirstStrikes()) * ((GC.getDefineINT("COMBAT_DAMAGE") * 2) / 5)) + 100);
*/			// modified
			iOurDefense *= ((((pAttacker->firstStrikes() * 2) + pAttacker->chanceFirstStrikes()) * ((GC.getCOMBAT_DAMAGE() * 2) / 5)) + 100);
			// /UncutDragon
			iOurDefense /= 100;
		}
	}

	int iAssetValue = std::max(1, getUnitInfo().getAssetValue());
	int iCargoAssetValue = 0;
	std::vector<CvUnit*> aCargoUnits;
	getCargoUnits(aCargoUnits);
	for (uint i = 0; i < aCargoUnits.size(); ++i)
	{
		iCargoAssetValue += aCargoUnits[i]->getUnitInfo().getAssetValue();
	}
	iOurDefense = iOurDefense * iAssetValue / std::max(1, iAssetValue + iCargoAssetValue);

	iTheirDefense = pDefender->currCombatStr(plot(), pAttacker);
	if (::isWorldUnitClass(pDefender->getUnitClassType()))
	{
		iTheirDefense /= 2;
	}

	if (NULL == pAttacker)
	{
		if (collateralDamage() > 0)
		{
			iTheirDefense *= (100 + collateralDamage());
			iTheirDefense /= 100;
		}

		if (currInterceptionProbability() > 0)
		{
			iTheirDefense *= (100 + currInterceptionProbability());
			iTheirDefense /= 100;
		}
	}
	else
	{
		if (!(pAttacker->immuneToFirstStrikes()))
		{
			// UncutDragon
/* original code
			iTheirDefense *= ((((pDefender->firstStrikes() * 2) + pDefender->chanceFirstStrikes()) * ((GC.getDefineINT("COMBAT_DAMAGE") * 2) / 5)) + 100);
*/			// modified
			iTheirDefense *= ((((pDefender->firstStrikes() * 2) + pDefender->chanceFirstStrikes()) * ((GC.getCOMBAT_DAMAGE() * 2) / 5)) + 100);
			// /UncutDragon
			iTheirDefense /= 100;
		}

		if (pDefender->immuneToFirstStrikes())
		{
			// UncutDragon
/* original code
			iTheirDefense *= ((((pAttacker->firstStrikes() * 2) + pAttacker->chanceFirstStrikes()) * ((GC.getDefineINT("COMBAT_DAMAGE") * 2) / 5)) + 100);
*/			// modified
			iTheirDefense *= ((((pAttacker->firstStrikes() * 2) + pAttacker->chanceFirstStrikes()) * ((GC.getCOMBAT_DAMAGE() * 2) / 5)) + 100);
			// /UncutDragon
			iTheirDefense /= 100;
		}
	}

	iAssetValue = std::max(1, pDefender->getUnitInfo().getAssetValue());
	iCargoAssetValue = 0;
	pDefender->getCargoUnits(aCargoUnits);
	for (uint i = 0; i < aCargoUnits.size(); ++i)
	{
		iCargoAssetValue += aCargoUnits[i]->getUnitInfo().getAssetValue();
	}
	iTheirDefense = iTheirDefense * iAssetValue / std::max(1, iAssetValue + iCargoAssetValue);

	if (iOurDefense == iTheirDefense)
	{
		if (NO_UNIT == getLeaderUnitType() && NO_UNIT != pDefender->getLeaderUnitType())
		{
			++iOurDefense;
		}
		else if (NO_UNIT != getLeaderUnitType() && NO_UNIT == pDefender->getLeaderUnitType())
		{
			++iTheirDefense;
		}
		else if (isBeforeUnitCycle(this, pDefender))
		{
			++iOurDefense;
		}
	}

	return (iOurDefense > iTheirDefense);
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/


bool CvUnit::canDoCommand(CommandTypes eCommand, int iData1, int iData2, bool bTestVisible, bool bTestBusy)
{
	CvUnit* pUnit;

	if (bTestBusy && getGroup()->isBusy())
	{
		return false;
	}

	switch (eCommand)
	{
	case COMMAND_PROMOTION:
		if (canPromote((PromotionTypes)iData1, iData2))
		{
			return true;
		}
		break;

	case COMMAND_UPGRADE:
		if (canUpgrade(((UnitTypes)iData1), bTestVisible))
		{
			return true;
		}
		break;

	case COMMAND_AUTOMATE:
		if (canAutomate((AutomateTypes)iData1))
		{
			return true;
		}
		break;

	case COMMAND_WAKE:
		if (!isAutomated() && isWaiting())
		{
			return true;
		}
		break;

	case COMMAND_CANCEL:
	case COMMAND_CANCEL_ALL:
		if (!isAutomated() && (getGroup()->getLengthMissionQueue() > 0))
		{
			return true;
		}
		break;

	case COMMAND_STOP_AUTOMATION:
		if (isAutomated())
		{
			return true;
		}
		break;

	case COMMAND_DELETE:
		if (canScrap())
		{
			return true;
		}
		break;

	case COMMAND_GIFT:
		if (canGift(bTestVisible))
		{
			return true;
		}
		break;

	case COMMAND_LOAD:
		if (canLoad(plot()))
		{
			return true;
		}
		break;

	case COMMAND_LOAD_UNIT:
		pUnit = ::getUnit(IDInfo(((PlayerTypes)iData1), iData2));
		if (pUnit != NULL)
		{
			if (canLoadUnit(pUnit, plot()))
			{
				return true;
			}
		}
		break;

	case COMMAND_UNLOAD:
		if (canUnload())
		{
			return true;
		}
		break;

	case COMMAND_UNLOAD_ALL:
		if (canUnloadAll())
		{
			return true;
		}
		break;

	case COMMAND_HOTKEY:
		if (isGroupHead())
		{
			return true;
		}
		break;

	default:
		FAssert(false);
		break;
	}

	return false;
}


void CvUnit::doCommand(CommandTypes eCommand, int iData1, int iData2)
{
	CvUnit* pUnit;
	bool bCycle;

	bCycle = false;

	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (canDoCommand(eCommand, iData1, iData2))
	{
		switch (eCommand)
		{
		case COMMAND_PROMOTION:
			promote((PromotionTypes)iData1, iData2);
			break;

		case COMMAND_UPGRADE:
			upgrade((UnitTypes)iData1);
			bCycle = true;
			break;

		case COMMAND_AUTOMATE:
			automate((AutomateTypes)iData1);
			bCycle = true;
			break;

		case COMMAND_WAKE:
			getGroup()->setActivityType(ACTIVITY_AWAKE);
			break;

		case COMMAND_CANCEL:
			getGroup()->popMission();
			break;

		case COMMAND_CANCEL_ALL:
			getGroup()->clearMissionQueue();
			break;

		case COMMAND_STOP_AUTOMATION:
			getGroup()->setAutomateType(NO_AUTOMATE);
			break;

		case COMMAND_DELETE:
			scrap();
			bCycle = true;
			break;

		case COMMAND_GIFT:
			gift();
			bCycle = true;
			break;

		case COMMAND_LOAD:
			load();
			bCycle = true;
			break;

		case COMMAND_LOAD_UNIT:
			pUnit = ::getUnit(IDInfo(((PlayerTypes)iData1), iData2));
			if (pUnit != NULL)
			{
				loadUnit(pUnit);
				bCycle = true;
			}
			break;

		case COMMAND_UNLOAD:
			unload();
			bCycle = true;
			break;

		case COMMAND_UNLOAD_ALL:
			unloadAll();
			bCycle = true;
			break;

		case COMMAND_HOTKEY:
			setHotKeyNumber(iData1);
			break;

		default:
			FAssert(false);
			break;
		}
	}

	if (bCycle)
	{
		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setCycleSelectionCounter(1);
		}
	}

	getGroup()->doDelayedDeath();
}


FAStarNode* CvUnit::getPathLastNode() const
{
	return getGroup()->getPathLastNode();
}


CvPlot* CvUnit::getPathEndTurnPlot() const
{
	return getGroup()->getPathEndTurnPlot();
}


bool CvUnit::generatePath(const CvPlot* pToPlot, int iFlags, bool bReuse, int* piPathTurns) const
{
	return getGroup()->generatePath(plot(), pToPlot, iFlags, bReuse, piPathTurns);
}


bool CvUnit::canEnterTerritory(TeamTypes eTeam, bool bIgnoreRightOfPassage) const
{
	if (GET_TEAM(getTeam()).isFriendlyTerritory(eTeam))
	{
		return true;
	}

	if (eTeam == NO_TEAM)
	{
		return true;
	}

	if (isEnemy(eTeam))
	{
		return true;
	}

	if (isRivalTerritory())
	{
		return true;
	}

	if (alwaysInvisible())
	{
		return true;
	}

	if (m_pUnitInfo->isHiddenNationality())
	{
		return true;
	}

	if (!bIgnoreRightOfPassage)
	{
		if (GET_TEAM(getTeam()).isOpenBorders(eTeam))
		{
			return true;
		}
/************************************************************************************************/
/* Afforess	                  Start		 02/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		if (GET_TEAM(getTeam()).isLimitedBorders(eTeam))
		{
			if (isOnlyDefensive() || !canFight())
			{
				return true;
			}
		}
	}
	if (!GET_TEAM(eTeam).isAlive())
	{
		return true;
	}	
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	return false;
}


bool CvUnit::canEnterArea(TeamTypes eTeam, const CvArea* pArea, bool bIgnoreRightOfPassage) const
{
	if (!canEnterTerritory(eTeam, bIgnoreRightOfPassage))
	{
		return false;
	}

	if (isBarbarian() && DOMAIN_LAND == getDomainType())
	{
		if (eTeam != NO_TEAM && eTeam != getTeam())
		{
			if (pArea && pArea->isBorderObstacle(eTeam))
			{
				return false;
			}
		}
	}

	return true;
}


// Returns the ID of the team to declare war against
TeamTypes CvUnit::getDeclareWarMove(const CvPlot* pPlot) const
{
	CvUnit* pUnit;
	TeamTypes eRevealedTeam;

	FAssert(isHuman());

	if (getDomainType() != DOMAIN_AIR)
	{
		eRevealedTeam = pPlot->getRevealedTeam(getTeam(), false);

		if (eRevealedTeam != NO_TEAM)
		{
			if (!canEnterArea(eRevealedTeam, pPlot->area()) || (getDomainType() == DOMAIN_SEA && !canCargoEnterArea(eRevealedTeam, pPlot->area(), false) && getGroup()->isAmphibPlot(pPlot)))
			{
				if (GET_TEAM(getTeam()).canDeclareWar(pPlot->getTeam()))
				{
					return eRevealedTeam;
				}
			}
		}
		else
		{
			if (pPlot->isActiveVisible(false))
			{
				if (canMoveInto(pPlot, true, true, true))
				{
					pUnit = pPlot->plotCheck(PUF_canDeclareWar, getOwnerINLINE(), isAlwaysHostile(pPlot), NO_PLAYER, NO_TEAM, PUF_isVisible, getOwnerINLINE());

					if (pUnit != NULL)
					{
						return pUnit->getTeam();
					}
				}
			}
		}
	}

	return NO_TEAM;
}

bool CvUnit::willRevealByMove(const CvPlot* pPlot) const
{
	int iRange = visibilityRange() + 1;
	for (int i = -iRange; i <= iRange; ++i)
	{
		for (int j = -iRange; j <= iRange; ++j)
		{
			CvPlot* pLoopPlot = ::plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), i, j);
			if (NULL != pLoopPlot)
			{
				if (!pLoopPlot->isRevealed(getTeam(), false) && pPlot->canSeePlot(pLoopPlot, getTeam(), visibilityRange(), NO_DIRECTION))
				{
					return true;
				}
			}
		}
	}

	return false;
}
/************************************************************************************************/
/* Afforess	                  Start		 06/17/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
bool CvUnit::canMoveInto(const CvPlot* pPlot, bool bAttack, bool bDeclareWar, bool bIgnoreLoad, bool bIgnoreTileLimit, bool bIgnoreLocation) const
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
{
	PROFILE_FUNC();

	FAssertMsg(pPlot != NULL, "Plot is not assigned a valid value");
/************************************************************************************************/
/* Afforess	                  Start		 06/22/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	int iHill = GC.getInfoTypeForString("TERRAIN_HILL");
	int iPeak = GC.getInfoTypeForString("TERRAIN_PEAK");
	
	if (!bIgnoreLocation)
	{
		if (atPlot(pPlot))
		{
			return false;
		}
	}
	
	//Forces damaged AI units to avoid deserts and tundra
	if (!isHuman())
	{
		if (plot()->getTerrainTurnDamage() <= 0)
		{
			if (getDamage() > 50)
			{
				if (pPlot->getTerrainTurnDamage() > 0)
				{
					return false;
				}
			}
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	// Cannot move around in unrevealed land freely
	if (m_pUnitInfo->isNoRevealMap() && willRevealByMove(pPlot))
	{
		return false;
	}

	if (GC.getUSE_SPIES_NO_ENTER_BORDERS())
	{
		if (isSpy() && NO_PLAYER != pPlot->getOwnerINLINE())
		{
			if (!GET_PLAYER(getOwnerINLINE()).canSpiesEnterBorders(pPlot->getOwnerINLINE()))
			{
				return false;
			}
		}
	}

	CvArea *pPlotArea = pPlot->area();
	TeamTypes ePlotTeam = pPlot->getTeam();
	bool bCanEnterArea = canEnterArea(ePlotTeam, pPlotArea);
	if (bCanEnterArea)
	{
		if (pPlot->getFeatureType() != NO_FEATURE)
		{
			if (m_pUnitInfo->getFeatureImpassable(pPlot->getFeatureType()))
			{
				TechTypes eTech = (TechTypes)m_pUnitInfo->getFeaturePassableTech(pPlot->getFeatureType());
				if (NO_TECH == eTech || !GET_TEAM(getTeam()).isHasTech(eTech))
				{
					if (DOMAIN_SEA != getDomainType() || pPlot->getTeam() != getTeam())  // sea units can enter impassable in own cultural borders
					{
						return false;
					}
				}
			}
		}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       09/17/09                         TC01 & jdog5000      */
/*                                                                                              */
/* Bugfix				                                                                        */
/************************************************************************************************/
/* original bts code
		else
*/
		// always check terrain also
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
		{
/************************************************************************************************/
/* Afforess	                  Start		 02/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			bool bPeak = false;
			bool bHill = false;	
			if (m_pUnitInfo->getTerrainImpassable(iHill))
				bHill = true;
			if (m_pUnitInfo->getTerrainImpassable(iPeak))
				bPeak = true;
			if (m_pUnitInfo->getTerrainImpassable(pPlot->getTerrainType()) || (pPlot->isPeak() && bPeak) || (pPlot->isHills() && bHill))
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			{
				TechTypes eTech = (TechTypes)m_pUnitInfo->getTerrainPassableTech(pPlot->getTerrainType());
				if (NO_TECH == eTech || !GET_TEAM(getTeam()).isHasTech(eTech))
				{
					if (DOMAIN_SEA != getDomainType() || pPlot->getTeam() != getTeam())  // sea units can enter impassable in own cultural borders
					{
						if (bIgnoreLoad || !canLoad(pPlot)) 
						{ 
							return false;
						}
					}
				}
			}
		}
	}

	switch (getDomainType())
	{
	case DOMAIN_SEA:
/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 07/07/09                                                        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		if (!pPlot->isWater() && !canMoveAllTerrain() && !pPlot->isCanMoveSeaUnits())
/************************************************************************************************/
/* JOOYO_ADDON                          END                                                     */
/************************************************************************************************/
		{
			if (!pPlot->isFriendlyCity(*this, true) || !pPlot->isCoastalLand()) 
			{
				return false;
			}
		}
		break;

	case DOMAIN_AIR:
		if (!bAttack)
		{
			bool bValid = false;

			if (pPlot->isFriendlyCity(*this, true))
			{
				bValid = true;

				if (m_pUnitInfo->getAirUnitCap() > 0)
				{
					if (pPlot->airUnitSpaceAvailable(getTeam()) <= 0)
					{
						bValid = false;
					}
				}
			}

			if (!bValid)
			{
				if (bIgnoreLoad || !canLoad(pPlot))
				{
					return false;
				}
			}
/************************************************************************************************/
/* Afforess	                  Start		 03/7/10                                                */
/*                                                                                              */
/*  Rebase Limit                                                                                */
/************************************************************************************************/
			if (!GET_TEAM(getTeam()).isRebaseAnywhere())
			{
				if ((GC.getGameINLINE().isModderGameOption(MODDERGAMEOPTION_AIRLIFT_RANGE)))
				{
					if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), getX_INLINE(), getY_INLINE()) > (GC.getGameINLINE().getModderGameOption(MODDERGAMEOPTION_AIRLIFT_RANGE)))
					{
						return false;
					}
				}
			}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/			
		}

		break;

	case DOMAIN_LAND:
/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 07/07/09                                                        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		if (pPlot->isWater() && !canMoveAllTerrain() && !pPlot->isCanMoveLandUnits())
/************************************************************************************************/
/* JOOYO_ADDON                          END                                                     */
/************************************************************************************************/
		{
			if (!pPlot->isCity() || 0 == GC.getDefineINT("LAND_UNITS_CAN_ATTACK_WATER_CITIES"))
			{
				if (bIgnoreLoad || !isHuman() || plot()->isWater() || !canLoad(pPlot))
				{
					return false;
				}
			}
		}
		break;

	case DOMAIN_IMMOBILE:
		return false;
		break;

	default:
		FAssert(false);
		break;
	}
	
/************************************************************************************************/
/* Afforess   Route Restricter   Start       08/03/09                                		     */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (m_pUnitInfo->getPassableRouteNeeded(0) || m_pUnitInfo->getPassableRouteNeeded(1))
	{
		if (!(m_pUnitInfo->getPassableRouteNeeded(pPlot->getRouteType()) && pPlot->isRoute())) 
		{
			return false;
		}
	}
/************************************************************************************************/
/* Afforess   Route Restricter End               END                                            */
/************************************************************************************************/

	if (isAnimal())
	{
		if (pPlot->isOwned())
		{
			return false;
		}

		if (!bAttack)
		{
			if (pPlot->getBonusType() != NO_BONUS)
			{
				return false;
			}

			if (pPlot->getImprovementType() != NO_IMPROVEMENT)
			{
				return false;
			}

			if (pPlot->getNumUnits() > 0)
			{
				return false;
			}
		}
	}

	if (isNoCapture())
	{
		if (!bAttack)
		{
			if (pPlot->isEnemyCity(*this))
			{
				return false;
			}
		}
	}

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       07/23/09                                jdog5000      */
/*                                                                                              */
/* Consistency                                                                                  */
/************************************************************************************************/
/* original bts code
	if (bAttack)
	{
		if (isMadeAttack() && !isBlitz())
		{
			return false;
		}
	}
*/
	// The following change makes capturing an undefended city like a attack action, it
	// cannot be done after another attack or a paradrop
	/*
	if (bAttack || (pPlot->isEnemyCity(*this) && !canCoexistWithEnemyUnit(NO_TEAM)) )
	{
		if (isMadeAttack() && !isBlitz())
		{
			return false;
		}
	}
	*/

	// The following change makes it possible to capture defenseless units after having 
	// made a previous attack or paradrop
	if( bAttack )
	{
		if (isMadeAttack() && !isBlitz() && (pPlot->getNumVisibleEnemyDefenders(this) > 0))
		{
			return false;
		}
	}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

	if (getDomainType() == DOMAIN_AIR)
	{
		if (bAttack)
		{
			if (!canAirStrike(pPlot))
			{
				return false;
			}
		}
	}
	else
	{
		if (canAttack())
		{
			if (bAttack || !canCoexistWithEnemyUnit(NO_TEAM))
			{
				if (!isHuman() || (pPlot->isVisible(getTeam(), false)))
				{
					if (pPlot->isVisibleEnemyUnit(this) != bAttack)
					{
						//FAssertMsg(isHuman() || (!bDeclareWar || (pPlot->isVisibleOtherUnit(getOwnerINLINE()) != bAttack)), "hopefully not an issue, but tracking how often this is the case when we dont want to really declare war");
						if (!bDeclareWar || (pPlot->isVisibleOtherUnit(getOwnerINLINE()) != bAttack && !(bAttack && pPlot->getPlotCity() && !isNoCapture())))
						{
							return false;
						}
					}
				}
			}

			if (bAttack)
			{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
				// From Lead From Behind by UncutDragon
/* original code
				CvUnit* pDefender = pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);
				if (NULL != pDefender)
				{
					if (!canAttack(*pDefender))
					{
						return false;
					}
				}
*/				// modified
				if (!pPlot->hasDefender(true, NO_PLAYER, getOwnerINLINE(), this, true))
						return false;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
			}
		}
		else
		{
			if (bAttack)
			{
				return false;
			}

			if (!canCoexistWithEnemyUnit(NO_TEAM))
			{
				if (!isHuman() || pPlot->isVisible(getTeam(), false))
				{
					if (pPlot->isEnemyCity(*this))
					{
						return false;
					}

					if (pPlot->isVisibleEnemyUnit(this))
					{
						return false;
					}
				}
			}
		}

		if (isHuman())
		{
			ePlotTeam = pPlot->getRevealedTeam(getTeam(), false);
			bCanEnterArea = canEnterArea(ePlotTeam, pPlotArea);
		}

		if (!bCanEnterArea)
		{
			FAssert(ePlotTeam != NO_TEAM);

			if (!(GET_TEAM(getTeam()).canDeclareWar(ePlotTeam)))
			{
				return false;
			}

			if (isHuman())
			{
				if (!bDeclareWar)
				{
					return false;
				}
			}
			else
			{
				if (GET_TEAM(getTeam()).AI_isSneakAttackReady(ePlotTeam))
				{
					if (!(getGroup()->AI_isDeclareWar(pPlot)))
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
		}
	}
/************************************************************************************************/
/* Afforess	                  Start		 08/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	bool bValid = false;
	if (pPlot->isImpassable(getTeam()))
	{
		CLLNode<IDInfo>* pUnitNode;
		CvUnit* pLoopUnit;

		//Check our current tile
		if (plot()->isPeak())
		{
			pUnitNode = plot()->headUnitNode();
			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = plot()->nextUnitNode(pUnitNode);
				if (pLoopUnit->isCanMovePeaks() && pLoopUnit->getTeam() == getTeam())
				{
					bValid = true;
					break;
				}
			}
		}
		if (pPlot->isPeak())
		{
			//Check the impassible tile	
			if (!bValid)
			{
				pUnitNode = pPlot->headUnitNode();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pPlot->nextUnitNode(pUnitNode);
					if (pLoopUnit->isCanMovePeaks() && pLoopUnit->getTeam() == getTeam())
					{
						bValid = true;
						break;
					}
				}
			}
		}
		if (!bValid)
		{
			if (!canMoveImpassable())
			{
				return false;
			}
		}
	}

	if (GC.getGameINLINE().getModderGameOption(MODDERGAMEOPTION_MAX_UNITS_PER_TILES) > 0)
	{
		if (!bIgnoreTileLimit)
		{
			if (!getUnitInfo().isOnlyDefensive() && baseCombatStr() > 0)
			{
				if (getDomainType() == DOMAIN_LAND && !pPlot->isWater() || getDomainType() == DOMAIN_SEA && pPlot->isWater() || getDomainType() == DOMAIN_AIR)
				{
					int iCount = 0;
					CLLNode<IDInfo>* pUnitNode;
					CvUnit* pLoopUnit;

					//Check our current tile
					pUnitNode = pPlot->headUnitNode();

					while (pUnitNode != NULL)
					{
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pPlot->nextUnitNode(pUnitNode);
						if (pLoopUnit->getTeam() == getTeam())
						{
							//Ignore workers, Missionaries, etc...
							if (!pLoopUnit->getUnitInfo().isOnlyDefensive() && pLoopUnit->baseCombatStr() > 0)
							{
								//No counting cargo for ships, or harbors
								if (pLoopUnit->getDomainType() == getDomainType())
								{
									iCount++;
								}
							}
						}
					}//Unit is already on the tile, ignore it in the count
					if (bIgnoreLocation)
					{
						iCount--;
					}
					if (GC.getGameINLINE().getModderGameOption(MODDERGAMEOPTION_MAX_UNITS_PER_TILES) <= iCount)
					{
						return false;
					}
				}
			}
		}
	}
	if (GC.getGameINLINE().isOption(GAMEOPTION_FIXED_BORDERS))
	{
		//Fort ZOC
		PlayerTypes eDefender = plot()->controlsAdjacentFort(getTeam());
		if (eDefender != NO_PLAYER)
		{
			const CvPlot* pZoneOfControl = plot()->isInFortControl(true, eDefender, getOwnerINLINE());
			const CvPlot* pForwardZoneOfControl = pPlot->isInFortControl(true, eDefender, getOwnerINLINE());
			if (pZoneOfControl != NULL && pForwardZoneOfControl != NULL)
			{
				if (pZoneOfControl == pPlot->isInFortControl(true, eDefender, getOwnerINLINE(), pZoneOfControl))
				{
					return false;
				}
			}
		}
	}
	
	//City ZoC		
	if (plot()->isInCityZoneOfControl(getOwnerINLINE()) && pPlot->isInCityZoneOfControl(getOwnerINLINE()))
	{
		return false;
	}
	//City Minimum Defense Level
	if (pPlot->getPlotCity() != NULL)
	{
		if (GET_TEAM(getTeam()).isAtWar(pPlot->getTeam()))
		{
			int iMinimumDefenseLevel = pPlot->getPlotCity()->getMinimumDefenseLevel();
			if (iMinimumDefenseLevel == 0)
			{
				iMinimumDefenseLevel = MAX_INT;
			}
			if (pPlot->getPlotCity()->getDefenseModifier(false) > iMinimumDefenseLevel)
			{
				return false;
			}
		}
	}
	//Promotion ZoC
	if (plot()->isInUnitZoneOfControl(getOwnerINLINE()) && pPlot->isInUnitZoneOfControl(getOwnerINLINE()))
	{
		return false;
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	if (GC.getUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK())
	{
		// Python Override
		CyArgsList argsList;
		argsList.add(getOwnerINLINE());	// Player ID
		argsList.add(getID());	// Unit ID
		argsList.add(pPlot->getX());	// Plot X
		argsList.add(pPlot->getY());	// Plot Y
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "unitCannotMoveInto", argsList.makeFunctionArgs(), &lResult);

		if (lResult != 0)
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::canMoveOrAttackInto(const CvPlot* pPlot, bool bDeclareWar) const
{
	return (canMoveInto(pPlot, false, bDeclareWar) || canMoveInto(pPlot, true, bDeclareWar));
}


bool CvUnit::canMoveThrough(const CvPlot* pPlot) const
{
	return canMoveInto(pPlot, false, false, true);
}


void CvUnit::attack(CvPlot* pPlot, bool bQuick)
{
	FAssert(canMoveInto(pPlot, true));
	FAssert(getCombatTimer() == 0);
/************************************************************************************************/
/* Afforess	                  Start		 02/22/10                    Coded by: KillMePlease     */
/*                                                                                              */
/*  Defender Withdraw                                                                           */
/************************************************************************************************/
	m_combatResult.iAttacksCount = 0;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	setAttackPlot(pPlot, false);

	updateCombat(bQuick);
}

void CvUnit::fightInterceptor(const CvPlot* pPlot, bool bQuick)
{
	FAssert(getCombatTimer() == 0);

	setAttackPlot(pPlot, true);

	updateAirCombat(bQuick);
}

void CvUnit::attackForDamage(CvUnit *pDefender, int attackerDamageChange, int defenderDamageChange)
{
	FAssert(getCombatTimer() == 0);
	FAssert(pDefender != NULL);
	FAssert(!isFighting());

	if(pDefender == NULL)
	{
		return;
	}

	setAttackPlot(pDefender->plot(), false);

	CvPlot* pPlot = getAttackPlot();
	if (pPlot == NULL)
	{
		return;
	}

	//rotate to face plot
	DirectionTypes newDirection = estimateDirection(this->plot(), pDefender->plot());
	if(newDirection != NO_DIRECTION)
	{
		setFacingDirection(newDirection);
	}

	//rotate enemy to face us
	newDirection = estimateDirection(pDefender->plot(), this->plot());
	if(newDirection != NO_DIRECTION)
	{
		pDefender->setFacingDirection(newDirection);
	}

	//check if quick combat
	bool bVisible = isCombatVisible(pDefender);

	//if not finished and not fighting yet, set up combat damage and mission
	if (!isFighting())
	{
		if (plot()->isFighting() || pPlot->isFighting())
		{
			return;
		}

		setCombatUnit(pDefender, true);
		pDefender->setCombatUnit(this, false);

		pDefender->getGroup()->clearMissionQueue();

		bool bFocused = (bVisible && isCombatFocus() && gDLL->getInterfaceIFace()->isCombatFocus());

		if (bFocused)
		{
			DirectionTypes directionType = directionXY(plot(), pPlot);
			//								N			NE				E				SE					S				SW					W				NW
			NiPoint2 directions[8] = {NiPoint2(0, 1), NiPoint2(1, 1), NiPoint2(1, 0), NiPoint2(1, -1), NiPoint2(0, -1), NiPoint2(-1, -1), NiPoint2(-1, 0), NiPoint2(-1, 1)};
			NiPoint3 attackDirection = NiPoint3(directions[directionType].x, directions[directionType].y, 0);
			float plotSize = GC.getPLOT_SIZE();
			NiPoint3 lookAtPoint(plot()->getPoint().x + plotSize / 2 * attackDirection.x, plot()->getPoint().y + plotSize / 2 * attackDirection.y, (plot()->getPoint().z + pPlot->getPoint().z) / 2);
			attackDirection.Unitize();
			gDLL->getInterfaceIFace()->lookAt(lookAtPoint, (((getOwnerINLINE() != GC.getGameINLINE().getActivePlayer()) || gDLL->getGraphicOption(GRAPHICOPTION_NO_COMBAT_ZOOM)) ? CAMERALOOKAT_BATTLE : CAMERALOOKAT_BATTLE_ZOOM_IN), attackDirection);
		}
		else
		{
			PlayerTypes eAttacker = getVisualOwner(pDefender->getTeam());
			CvWString szMessage;
			if (BARBARIAN_PLAYER != eAttacker)
			{
				szMessage = gDLL->getText("TXT_KEY_MISC_YOU_UNITS_UNDER_ATTACK", GET_PLAYER(getOwnerINLINE()).getNameKey());
			}
			else
			{
				szMessage = gDLL->getText("TXT_KEY_MISC_YOU_UNITS_UNDER_ATTACK_UNKNOWN");
			}

			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szMessage, "AS2D_COMBAT", MESSAGE_TYPE_DISPLAY_ONLY, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true);
		}
	}

	FAssertMsg(plot()->isFighting(), "Current unit instance plot is not fighting as expected");
	FAssertMsg(pPlot->isFighting(), "pPlot is not fighting as expected");

	//setup battle object
	CvBattleDefinition kBattle;
	kBattle.setUnit(BATTLE_UNIT_ATTACKER, this);
	kBattle.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
	kBattle.setDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_BEGIN, getDamage());
	kBattle.setDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_BEGIN, pDefender->getDamage());

	changeDamage(attackerDamageChange, pDefender->getOwnerINLINE());
	pDefender->changeDamage(defenderDamageChange, getOwnerINLINE());

	if (bVisible)
	{
		kBattle.setDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_END, getDamage());
		kBattle.setDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_END, pDefender->getDamage());
		kBattle.setAdvanceSquare(canAdvance(pPlot, 1));

		kBattle.addDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_BEGIN));
		kBattle.addDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_BEGIN));

		int iTurns = planBattle( kBattle);
		kBattle.setMissionTime(iTurns * gDLL->getSecsPerTurn());
		setCombatTimer(iTurns);

		GC.getGameINLINE().incrementTurnTimer(getCombatTimer());

		if (pPlot->isActiveVisible(false))
		{
			ExecuteMove(0.5f, true);
			gDLL->getEntityIFace()->AddMission(&kBattle);
		}
	}
	else
	{
		setCombatTimer(1);
	}
}


void CvUnit::move(CvPlot* pPlot, bool bShow)
{
	FAssert(canMoveOrAttackInto(pPlot) || isMadeAttack());

	CvPlot* pOldPlot = plot();

	changeMoves(pPlot->movementCost(this, plot()));

	setXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true, bShow && pPlot->isVisibleToWatchingHuman(), bShow);

	//change feature
	FeatureTypes featureType = pPlot->getFeatureType();
	if(featureType != NO_FEATURE)
	{
		CvString featureString(GC.getFeatureInfo(featureType).getOnUnitChangeTo());
		if(!featureString.IsEmpty())
		{
			FeatureTypes newFeatureType = (FeatureTypes) GC.getInfoTypeForString(featureString);
			pPlot->setFeatureType(newFeatureType);
		}
	}

	if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
	{
		if (!(pPlot->isOwned()))
		{
			//spawn birds if trees present - JW
			if (featureType != NO_FEATURE)
			{
				if (GC.getASyncRand().get(100) < GC.getFeatureInfo(featureType).getEffectProbability())
				{
					EffectTypes eEffect = (EffectTypes)GC.getInfoTypeForString(GC.getFeatureInfo(featureType).getEffectType());
					gDLL->getEngineIFace()->TriggerEffect(eEffect, pPlot->getPoint(), (float)(GC.getASyncRand().get(360)));
					gDLL->getInterfaceIFace()->playGeneralSound("AS3D_UN_BIRDS_SCATTER", pPlot->getPoint());
				}
			}
		}
	}

	CvEventReporter::getInstance().unitMove(pPlot, this, pOldPlot);
}

// false if unit is killed
/************************************************************************************************/
/* Afforess	                  Start		 06/13/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
bool CvUnit::jumpToNearestValidPlot(bool bKill)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
{
	CvCity* pNearestCity;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	FAssertMsg(!isAttacking(), "isAttacking did not return false as expected");
	FAssertMsg(!isFighting(), "isFighting did not return false as expected");

	pNearestCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), getOwnerINLINE());

	iBestValue = MAX_INT;
	pBestPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->isValidDomainForLocation(*this))
		{
			if (canMoveInto(pLoopPlot))
			{
				if (canEnterArea(pLoopPlot->getTeam(), pLoopPlot->area()) && !isEnemy(pLoopPlot->getTeam(), pLoopPlot))
				{
					FAssertMsg(!atPlot(pLoopPlot), "atPlot(pLoopPlot) did not return false as expected");

					if ((getDomainType() != DOMAIN_AIR) || pLoopPlot->isFriendlyCity(*this, true))
					{
						if (pLoopPlot->isRevealed(getTeam(), false))
						{
							iValue = (plotDistance(getX_INLINE(), getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) * 2);

							if (pNearestCity != NULL)
							{
								iValue += plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE());
							}

							if (getDomainType() == DOMAIN_SEA && !plot()->isWater())
							{
								if (!pLoopPlot->isWater() || !pLoopPlot->isAdjacentToArea(area()))
								{
									iValue *= 3;
								}
							}
							else
							{
								if (pLoopPlot->area() != area())
								{
									iValue *= 3;
								}
							}
/************************************************************************************************/
/* Afforess	                  Start		 06/20/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
							iValue *= std::max(1, (pLoopPlot->getTerrainTurnDamage(this) / 2));
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

							if (iValue < iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = pLoopPlot;
							}
						}
					}
				}
			}
		}
	}

	bool bValid = true;
	if (pBestPlot != NULL)
	{
		setXY(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
	}
/************************************************************************************************/
/* Afforess	                  Start		 06/13/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	else if (bKill)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	{
		kill(false);
		bValid = false;
	}
/************************************************************************************************/
/* Afforess	                  Start		 06/13/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	else
	{
		bValid = false;
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	return bValid;
}


bool CvUnit::canAutomate(AutomateTypes eAutomate) const
{
	if (eAutomate == NO_AUTOMATE)
	{
		return false;
	}

	if (!isGroupHead())
	{
		return false;
	}
/************************************************************************************************/
/* Afforess	                  Start		 02/14/10                                               */
/*                                                                                              */
/*  Clicking on the Automate button with an Inquisitor causes a CTD                             */
/************************************************************************************************/
	if (m_pUnitInfo->isInquisitor())
	{
		return false;
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	switch (eAutomate)
	{
	case AUTOMATE_BUILD:
		if ((AI_getUnitAIType() != UNITAI_WORKER) && (AI_getUnitAIType() != UNITAI_WORKER_SEA))
		{
			return false;
		}
		break;

	case AUTOMATE_NETWORK:
		if ((AI_getUnitAIType() != UNITAI_WORKER) || !canBuildRoute())
		{
			return false;
		}
		break;

	case AUTOMATE_CITY:
		if (AI_getUnitAIType() != UNITAI_WORKER)
		{
			return false;
		}
		break;

	case AUTOMATE_EXPLORE:
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/25/10                                jdog5000      */
/*                                                                                              */
/* Player Interface                                                                             */
/************************************************************************************************/
		if ( !canFight() )
		{
			// Enable exploration for air units
			if((getDomainType() != DOMAIN_SEA) && (getDomainType() != DOMAIN_AIR))
			{
				if( !(alwaysInvisible()) || !(isSpy()) )
				{
					return false;
				}
			}
		}

		if( (getDomainType() == DOMAIN_IMMOBILE) )
		{
			return false;
		}

		if( getDomainType() == DOMAIN_AIR && !canRecon(NULL) )
		{
			return false;
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/		

		break;

	case AUTOMATE_RELIGION:
		if (AI_getUnitAIType() != UNITAI_MISSIONARY)
		{
			return false;
		}
		break;
/************************************************************************************************/
/* Afforess	                  Start		 08/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	case AUTOMATE_PILLAGE:
		if (!getUnitInfo().isPillage())
		{
			return false;
		}
		break;
	case AUTOMATE_HUNT:
	case AUTOMATE_CITY_DEFENSE:
	case AUTOMATE_BORDER_PATROL:
		if (!canAttack())
		{
			return false;
		}
		break;
	case AUTOMATE_PIRATE:
		if (getDomainType() != DOMAIN_SEA)
		{
			return false;
		}
		if (!canAttack())
		{
			return false;
		}
		if (!m_pUnitInfo->isHiddenNationality() || !m_pUnitInfo->isAlwaysHostile())
		{
			return false;
		}
		break;
	case AUTOMATE_HURRY:
		if (m_pUnitInfo->getBaseHurry() <= 0)
		{
			return false;
		}
		//Do not give ability to great people
		if (m_pUnitInfo->getProductionCost() < 0)
		{
			return false;
		}
		break;
	case AUTOMATE_AIRSTRIKE:
		if (getDomainType() != DOMAIN_AIR)
		{
			return false;
		}
		if (!canAirAttack())
		{
			return false;
		}
		//Jets and Fighters can intercept, modders, if you have fighters with 0 interception, feel free to get rid of this check
		if (m_pUnitInfo->getInterceptionProbability() <= 0)
		{
			return false;
		}
		break;
	case AUTOMATE_AIRBOMB:
		if (getDomainType() != DOMAIN_AIR)
		{
			return false;
		}

		if (airBombBaseRate() == 0)
		{
			return false;
		}
		
		if (canAutomate(AUTOMATE_AIRSTRIKE))
		{
			return false;
		}
	case AUTOMATE_AIR_RECON:
		if (!canRecon(NULL))
		{
			return false;
		}
		break;
	case AUTOMATE_UPGRADING:
		if (GC.getUnitInfo(getUnitType()).getUpgradeUnitClassTypes().size() == 0)
		{
			return false;
		}
		if (isAutoUpgrading())
		{
			return false;
		}
		break;
	case AUTOMATE_CANCEL_UPGRADING:
		if (GC.getUnitInfo(getUnitType()).getUpgradeUnitClassTypes().size() == 0)
		{
			return false;
		}
		if (!isAutoUpgrading())
		{
			return false;
		}
		break;
	case AUTOMATE_PROMOTIONS:
		if (!canAcquirePromotionAny())
		{
			return false;
		}
		if (isAutoPromoting())
		{
			return false;
		}
		break;
	case AUTOMATE_CANCEL_PROMOTIONS:
		if (!canAcquirePromotionAny())
		{
			return false;
		}
		if (!isAutoPromoting())
		{
			return false;
		}
		break;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	default:
		FAssert(false);
		break;
	}

	return true;
}


void CvUnit::automate(AutomateTypes eAutomate)
{
	if (!canAutomate(eAutomate))
	{
		return;
	}
/************************************************************************************************/
/* Afforess	                  Start		 08/20/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	CvUnit* pLoopUnit = NULL;
	CLLNode<IDInfo>* pUnitNode = NULL;
	if (eAutomate == AUTOMATE_UPGRADING || eAutomate == AUTOMATE_CANCEL_UPGRADING)
	{
		pUnitNode = getGroup()->headUnitNode();
		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = getGroup()->nextUnitNode(pUnitNode);
			pLoopUnit->setAutoUpgrading((eAutomate == AUTOMATE_UPGRADING));
		}
		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		}
		return;
	}
	if (eAutomate == AUTOMATE_PROMOTIONS || eAutomate == AUTOMATE_CANCEL_PROMOTIONS)
	{
		pUnitNode = getGroup()->headUnitNode();
		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = getGroup()->nextUnitNode(pUnitNode);
			pLoopUnit->setAutoPromoting((eAutomate == AUTOMATE_PROMOTIONS));
		}
		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		}
		return;
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	getGroup()->setAutomateType(eAutomate);
}


bool CvUnit::canScrap() const
{
	if (plot()->isFighting())
	{
		return false;
	}

	return true;
}


void CvUnit::scrap()
{
	if (!canScrap())
	{
		return;
	}
	
/************************************************************************************************/
/* Afforess	                  Start		 01/13/10                                               */
/*                                                                                              */
/*    Gold when Disbanding                                                                      */
/************************************************************************************************/
	if (GC.getDefineINT("DISBANDING_EARNS_GOLD") && plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		int iCost = (getUnitInfo().getProductionCost() * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent()) / 100;
		GET_PLAYER(getOwnerINLINE()).changeGold(iCost / std::max(1, (GC.getDefineINT("UNIT_GOLD_DISBAND_DIVISOR"))));
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	kill(true);
}


bool CvUnit::canGift(bool bTestVisible, bool bTestTransport)
{
	CvPlot* pPlot = plot();
	CvUnit* pTransport = getTransportUnit();

	if (!(pPlot->isOwned()))
	{
		return false;
	}

	if (pPlot->getOwnerINLINE() == getOwnerINLINE())
	{
		return false;
	}

	if (pPlot->isVisibleEnemyUnit(this))
	{
		return false;
	}

	if (pPlot->isVisibleEnemyUnit(pPlot->getOwnerINLINE()))
	{
		return false;
	}

	if (!pPlot->isValidDomainForLocation(*this) && NULL == pTransport)
	{
		return false;
	}

	for (int iCorp = 0; iCorp < GC.getNumCorporationInfos(); ++iCorp)
	{
		if (m_pUnitInfo->getCorporationSpreads(iCorp) > 0)
		{
			return false;
		}
	}

	if (bTestTransport)
	{
		if (pTransport && pTransport->getTeam() != pPlot->getTeam())
		{
			return false;
		}
	}

	if (!bTestVisible)
	{
		if (GET_TEAM(pPlot->getTeam()).isUnitClassMaxedOut(getUnitClassType(), GET_TEAM(pPlot->getTeam()).getUnitClassMaking(getUnitClassType())))
		{
			return false;
		}

		if (GET_PLAYER(pPlot->getOwnerINLINE()).isUnitClassMaxedOut(getUnitClassType(), GET_PLAYER(pPlot->getOwnerINLINE()).getUnitClassMaking(getUnitClassType())))
		{
			return false;
		}

		if (!(GET_PLAYER(pPlot->getOwnerINLINE()).AI_acceptUnit(this)))
		{
			return false;
		}
	}

	return !atWar(pPlot->getTeam(), getTeam());
}


void CvUnit::gift(bool bTestTransport)
{
	CvUnit* pGiftUnit;
	CvWString szBuffer;
	PlayerTypes eOwner;

	if (!canGift(false, bTestTransport))
	{
		return;
	}

	std::vector<CvUnit*> aCargoUnits;
	getCargoUnits(aCargoUnits);
	for (uint i = 0; i < aCargoUnits.size(); ++i)
	{
		aCargoUnits[i]->gift(false);
	}

	FAssertMsg(plot()->getOwnerINLINE() != NO_PLAYER, "plot()->getOwnerINLINE() is not expected to be equal with NO_PLAYER");
	pGiftUnit = GET_PLAYER(plot()->getOwnerINLINE()).initUnit(getUnitType(), getX_INLINE(), getY_INLINE(), AI_getUnitAIType());

	FAssertMsg(pGiftUnit != NULL, "GiftUnit is not assigned a valid value");

	eOwner = getOwnerINLINE();

	pGiftUnit->convert(this);

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/03/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
	//GET_PLAYER(pGiftUnit->getOwnerINLINE()).AI_changePeacetimeGrantValue(eOwner, (pGiftUnit->getUnitInfo().getProductionCost() / 5));
	if( pGiftUnit->isCombat() )
	{
		GET_PLAYER(pGiftUnit->getOwnerINLINE()).AI_changePeacetimeGrantValue(eOwner, (pGiftUnit->getUnitInfo().getProductionCost() * 3 * GC.getGameINLINE().AI_combatValue(pGiftUnit->getUnitType()))/100);
	}
	else
	{
		GET_PLAYER(pGiftUnit->getOwnerINLINE()).AI_changePeacetimeGrantValue(eOwner, (pGiftUnit->getUnitInfo().getProductionCost()));
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	szBuffer = gDLL->getText("TXT_KEY_MISC_GIFTED_UNIT_TO_YOU", GET_PLAYER(eOwner).getNameKey(), pGiftUnit->getNameKey());
	gDLL->getInterfaceIFace()->addMessage(pGiftUnit->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_UNITGIFTED", MESSAGE_TYPE_INFO, pGiftUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pGiftUnit->getX_INLINE(), pGiftUnit->getY_INLINE(), true, true);

	// Python Event
	CvEventReporter::getInstance().unitGifted(pGiftUnit, getOwnerINLINE(), plot());
}


bool CvUnit::canLoadUnit(const CvUnit* pUnit, const CvPlot* pPlot) const
{
	FAssert(pUnit != NULL);
	FAssert(pPlot != NULL);

	if (pUnit == this)
	{
		return false;
	}

	if (pUnit->getTeam() != getTeam())
	{
		return false;
	}

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       11/06/09                     Mongoose & jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
	// From Mongoose SDK
	/*if (isCargo())
	{
		return false;
	}*/
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

	if (getCargo() > 0)
	{
		return false;
	}

	if (pUnit->isCargo())
	{
		return false;
	}

	if (!(pUnit->cargoSpaceAvailable(getSpecialUnitType(), getDomainType())))
	{
		return false;
	}

	if (!(pUnit->atPlot(pPlot)))
	{
		return false;
	}

	if (!m_pUnitInfo->isHiddenNationality() && pUnit->getUnitInfo().isHiddenNationality())
	{
		return false;
	}

	if (NO_SPECIALUNIT != getSpecialUnitType())
	{
		if (GC.getSpecialUnitInfo(getSpecialUnitType()).isCityLoad())
		{
			if (!pPlot->isCity(true, getTeam()))
			{
				return false;
			}
		}
	}

	return true;
}


void CvUnit::loadUnit(CvUnit* pUnit)
{
	if (!canLoadUnit(pUnit, plot()))
	{
		return;
	}

	setTransportUnit(pUnit);
}

bool CvUnit::shouldLoadOnMove(const CvPlot* pPlot) const
{
	if (isCargo())
	{
		return false;
	}

	switch (getDomainType())
	{
	case DOMAIN_LAND:
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/30/09                     Mongoose & jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
		if (pPlot->isWater())
*/
		// From Mongoose SDK
/************************************************************************************************/
/* Afforess	                  Start		 08/18/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
		if (pPlot->isWater() && !canMoveAllTerrain())
*/
		if ((pPlot->isWater() && !canMoveAllTerrain()) && !pPlot->isCanMoveLandUnits())
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		{
			return true;
		}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
		break;
	case DOMAIN_AIR:
		if (!pPlot->isFriendlyCity(*this, true))
		{
			return true;
		}

		if (m_pUnitInfo->getAirUnitCap() > 0)
		{
			if (pPlot->airUnitSpaceAvailable(getTeam()) <= 0)
			{
				return true;
			}
		}
		break;
	default:
		break;
	}

	if (m_pUnitInfo->getTerrainImpassable(pPlot->getTerrainType())) 
	{ 
		TechTypes eTech = (TechTypes)m_pUnitInfo->getTerrainPassableTech(pPlot->getTerrainType()); 
		if (NO_TECH == eTech || !GET_TEAM(getTeam()).isHasTech(eTech)) 
		{ 
			return true; 
		} 
	} 

	return false;
}


bool CvUnit::canLoad(const CvPlot* pPlot) const
{
	PROFILE_FUNC();

	FAssert(pPlot != NULL);

	CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (canLoadUnit(pLoopUnit, pPlot))
		{
			return true;
		}
	}

	return false;
}


void CvUnit::load()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;
	int iPass;

	if (!canLoad(plot()))
	{
		return;
	}

	pPlot = plot();

	for (iPass = 0; iPass < 2; iPass++)
	{
		pUnitNode = pPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);

			if (canLoadUnit(pLoopUnit, pPlot))
			{
				if ((iPass == 0) ? (pLoopUnit->getOwnerINLINE() == getOwnerINLINE()) : (pLoopUnit->getTeam() == getTeam()))
				{
					setTransportUnit(pLoopUnit);
					break;
				}
			}
		}

		if (isCargo())
		{
			break;
		}
	}
}


bool CvUnit::canUnload() const
{
	CvPlot& kPlot = *(plot());

	if (getTransportUnit() == NULL)
	{
		return false;
	}

	if (!kPlot.isValidDomainForLocation(*this))
	{
		return false;
	}

	if (getDomainType() == DOMAIN_AIR)
	{
		if (kPlot.isFriendlyCity(*this, true))
		{
			int iNumAirUnits = kPlot.countNumAirUnits(getTeam());
			CvCity* pCity = kPlot.getPlotCity();
			if (NULL != pCity)
			{
				if (iNumAirUnits >= pCity->getAirUnitCapacity(getTeam()))
				{
					return false;
				}
			}
			else
			{
				if (iNumAirUnits >= GC.getDefineINT("CITY_AIR_UNIT_CAPACITY"))
				{
					return false;
				}
			}
		}
	}

	return true;
}


void CvUnit::unload()
{
	if (!canUnload())
	{
		return;
	}

	setTransportUnit(NULL);
}


bool CvUnit::canUnloadAll() const
{
	if (getCargo() == 0)
	{
		return false;
	}

	return true;
}


void CvUnit::unloadAll()
{
	if (!canUnloadAll())
	{
		return;
	}

	std::vector<CvUnit*> aCargoUnits;
	getCargoUnits(aCargoUnits);
	for (uint i = 0; i < aCargoUnits.size(); ++i)
	{
		CvUnit* pCargo = aCargoUnits[i];
		if (pCargo->canUnload())
		{
			pCargo->setTransportUnit(NULL);
		}
		else
		{
			FAssert(isHuman() || pCargo->getDomainType() == DOMAIN_AIR);
			pCargo->getGroup()->setActivityType(ACTIVITY_AWAKE);
		}
	}
}


bool CvUnit::canHold(const CvPlot* pPlot) const
{
	return true;
}


bool CvUnit::canSleep(const CvPlot* pPlot) const
{
	if (isFortifyable())
	{
		return false;
	}

	if (isWaiting())
	{
		return false;
	}

	return true;
}


bool CvUnit::canFortify(const CvPlot* pPlot) const
{
	if (!isFortifyable())
	{
		return false;
	}

	if (isWaiting())
	{
		return false;
	}

	return true;
}


bool CvUnit::canAirPatrol(const CvPlot* pPlot) const
{
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}

	if (!canAirDefend(pPlot))
	{
		return false;
	}

	if (isWaiting())
	{
		return false;
	}

	return true;
}


bool CvUnit::canSeaPatrol(const CvPlot* pPlot) const
{
	if (!pPlot->isWater())
	{
		return false;
	}

	if (getDomainType() != DOMAIN_SEA)
	{
		return false;
	}

	if (!canFight() || isOnlyDefensive())
	{
		return false;
	}

	if (isWaiting())
	{
		return false;
	}

	return true;
}


void CvUnit::airCircle(bool bStart)
{
	if (!GC.IsGraphicsInitialized())
	{
		return;
	}

	if ((getDomainType() != DOMAIN_AIR) || (maxInterceptionProbability() == 0))
	{
		return;
	}

	//cancel previos missions
	gDLL->getEntityIFace()->RemoveUnitFromBattle( this );

	if (bStart)
	{
		CvAirMissionDefinition kDefinition;
		kDefinition.setPlot(plot());
		kDefinition.setUnit(BATTLE_UNIT_ATTACKER, this);
		kDefinition.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		kDefinition.setMissionType(MISSION_AIRPATROL);
		kDefinition.setMissionTime(1.0f); // patrol is indefinite - time is ignored

		gDLL->getEntityIFace()->AddMission( &kDefinition );
	}
}


bool CvUnit::canHeal(const CvPlot* pPlot) const
{
	if (!isHurt())
	{
		return false;
	}

	if (isWaiting())
	{
		return false;
	}

	if (healRate(pPlot) <= 0)
	{
		return false;
	}

	return true;
}


bool CvUnit::canSentry(const CvPlot* pPlot) const
{
	if (!canDefend(pPlot))
	{
		return false;
	}

	if (isWaiting())
	{
		return false;
	}

	return true;
}


int CvUnit::healRate(const CvPlot* pPlot) const
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvCity* pCity;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iTotalHeal;
	int iHeal;
	int iBestHeal;
	int iI;

	pCity = pPlot->getPlotCity();

	iTotalHeal = 0;

	if (pPlot->isCity(true, getTeam()))
	{
		iTotalHeal += GC.getDefineINT("CITY_HEAL_RATE") + (GET_TEAM(getTeam()).isFriendlyTerritory(pPlot->getTeam()) ? getExtraFriendlyHeal() : getExtraNeutralHeal());
		if (pCity && !pCity->isOccupation())
		{
			iTotalHeal += pCity->getHealRate();
		}
	}
	else
	{
		if (!GET_TEAM(getTeam()).isFriendlyTerritory(pPlot->getTeam()))
		{
			if (isEnemy(pPlot->getTeam(), pPlot))
			{
				iTotalHeal += (GC.getDefineINT("ENEMY_HEAL_RATE") + getExtraEnemyHeal());
			}
			else
			{
				iTotalHeal += (GC.getDefineINT("NEUTRAL_HEAL_RATE") + getExtraNeutralHeal());
			}
		}
		else
		{
			iTotalHeal += (GC.getDefineINT("FRIENDLY_HEAL_RATE") + getExtraFriendlyHeal());
		}
	}

	// XXX optimize this (save it?)
	iBestHeal = 0;

	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTeam() == getTeam()) // XXX what about alliances?
		{
			iHeal = pLoopUnit->getSameTileHeal();

			if (iHeal > iBestHeal)
			{
				iBestHeal = iHeal;
			}
		}
	}

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

					if (pLoopUnit->getTeam() == getTeam()) // XXX what about alliances?
					{
						iHeal = pLoopUnit->getAdjacentTileHeal();

						if (iHeal > iBestHeal)
						{
							iBestHeal = iHeal;
						}
					}
				}
			}
		}
	}

	iTotalHeal += iBestHeal;
	// XXX

	return iTotalHeal;
}


int CvUnit::healTurns(const CvPlot* pPlot) const
{
	int iHeal;
	int iTurns;

	if (!isHurt())
	{
		return 0;
	}

	iHeal = healRate(pPlot);

	if (iHeal > 0)
	{
		iTurns = (getDamage() / iHeal);

		if ((getDamage() % iHeal) != 0)
		{
			iTurns++;
		}

		return iTurns;
	}
	else
	{
		return MAX_INT;
	}
}


void CvUnit::doHeal()
{
	changeDamage(-(healRate(plot())));
}


bool CvUnit::canAirlift(const CvPlot* pPlot) const
{
	CvCity* pCity;

	if (getDomainType() != DOMAIN_LAND)
	{
		return false;
	}

	if (hasMoved())
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getCurrAirlift() >= pCity->getMaxAirlift())
	{
		return false;
	}

	if (pCity->getTeam() != getTeam())
	{
		return false;
	}

	return true;
}


bool CvUnit::canAirliftAt(const CvPlot* pPlot, int iX, int iY) const
{
	CvPlot* pTargetPlot;
	CvCity* pTargetCity;

	if (!canAirlift(pPlot))
	{
		return false;
	}

	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (!canMoveInto(pTargetPlot))
	{
		return false;
	}

	pTargetCity = pTargetPlot->getPlotCity();

	if (pTargetCity == NULL)
	{
		return false;
	}

	if (pTargetCity->isAirliftTargeted())
	{
		return false;
	}

	if (pTargetCity->getTeam() != getTeam() && !GET_TEAM(pTargetCity->getTeam()).isVassal(getTeam()))
	{
		return false;
	}
/************************************************************************************************/
/* Afforess	                  Start		 03/7/10                                                */
/*                                                                                              */
/*  Airlift Range                                                                               */
/************************************************************************************************/
	if (!GET_TEAM(getTeam()).isRebaseAnywhere())
	{
		if (GC.getGameINLINE().isModderGameOption(MODDERGAMEOPTION_AIRLIFT_RANGE))
		{
			if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iX, iY) > (GC.getGameINLINE().getModderGameOption(MODDERGAMEOPTION_AIRLIFT_RANGE)))
			{
				return false;
			}
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return true;
}


bool CvUnit::airlift(int iX, int iY)
{
	CvCity* pCity;
	CvCity* pTargetCity;
	CvPlot* pTargetPlot;

	if (!canAirliftAt(plot(), iX, iY))
	{
		return false;
	}

	pCity = plot()->getPlotCity();
	FAssert(pCity != NULL);
	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	FAssert(pTargetPlot != NULL);
	pTargetCity = pTargetPlot->getPlotCity();
	FAssert(pTargetCity != NULL);
	FAssert(pCity != pTargetCity);

	pCity->changeCurrAirlift(1);
	if (pTargetCity->getMaxAirlift() == 0)
	{
		pTargetCity->setAirliftTargeted(true);
	}

	finishMoves();

	setXY(pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE());

	return true;
}


bool CvUnit::isNukeVictim(const CvPlot* pPlot, TeamTypes eTeam) const
{
	CvPlot* pLoopPlot;
	int iDX, iDY;

	if (!(GET_TEAM(eTeam).isAlive()))
	{
		return false;
	}

	if (eTeam == getTeam())
	{
		return false;
	}

	for (iDX = -(nukeRange()); iDX <= nukeRange(); iDX++)
	{
		for (iDY = -(nukeRange()); iDY <= nukeRange(); iDY++)
		{
			pLoopPlot	= plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->getTeam() == eTeam)
				{
					return true;
				}

				if (pLoopPlot->plotCheck(PUF_isCombatTeam, eTeam, getTeam()) != NULL)
				{
					return true;
				}
			}
		}
	}

	return false;
}


bool CvUnit::canNuke(const CvPlot* pPlot) const
{
	if (nukeRange() == -1)
	{
		return false;
	}

	return true;
}


bool CvUnit::canNukeAt(const CvPlot* pPlot, int iX, int iY) const
{
	CvPlot* pTargetPlot;
	int iI;

	if (!canNuke(pPlot))
	{
		return false;
	}

	int iDistance = plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iX, iY);
	if (iDistance <= nukeRange())
	{
		return false;
	}

	if (airRange() > 0 && iDistance > airRange())
	{
		return false;
	}

	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
		// Dale - NB: A-Bomb START
		if (!((TeamTypes)iI == GET_TEAM(getTeam()).getID()))
		{
			if (isNukeVictim(pTargetPlot, ((TeamTypes)iI)))
			{
				if (!isEnemy((TeamTypes)iI))
				{
					return false;
				}
			}
		}
		// Dale - NB: A-Bomb END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
	}

	return true;
}


bool CvUnit::nuke(int iX, int iY)
{
	CvPlot* pPlot;
	CvWString szBuffer;
	bool abTeamsAffected[MAX_TEAMS];
	TeamTypes eBestTeam;
	int iBestInterception;
	int iI, iJ, iK;

	if (!canNukeAt(plot(), iX, iY))
	{
		return false;
	}

	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		abTeamsAffected[iI] = isNukeVictim(pPlot, ((TeamTypes)iI));
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (abTeamsAffected[iI])
		{
			if (!isEnemy((TeamTypes)iI))
			{
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
				// Dale - NB: A-Bomb START
				if (!((TeamTypes)iI == GET_TEAM(getTeam()).getID()))
				{
					GET_TEAM(getTeam()).declareWar(((TeamTypes)iI), false, WARPLAN_TOTAL);
				}
				// Dale - NB: A-Bomb END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
			}
		}
	}

/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - NB: A-Bomb START
	if(airBaseCombatStr() != 0)
	{
		if (interceptTest(pPlot))
		{
			return true;
		}
	}
	// Dale - NB: A-Bomb END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
	iBestInterception = 0;
	eBestTeam = NO_TEAM;

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (abTeamsAffected[iI])
		{
			if (GET_TEAM((TeamTypes)iI).getNukeInterception() > iBestInterception)
			{
				iBestInterception = GET_TEAM((TeamTypes)iI).getNukeInterception();
				eBestTeam = ((TeamTypes)iI);
			}
		}
	}

	iBestInterception *= (100 - m_pUnitInfo->getEvasionProbability());
	iBestInterception /= 100;

	setReconPlot(pPlot);

	if (GC.getGameINLINE().getSorenRandNum(100, "Nuke") < iBestInterception)
	{
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_NUKE_INTERCEPTED", GET_PLAYER(getOwnerINLINE()).getNameKey(), getNameKey(), GET_TEAM(eBestTeam).getName().GetCString());
				gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), (((PlayerTypes)iI) == getOwnerINLINE()), GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_NUKE_INTERCEPTED", MESSAGE_TYPE_MAJOR_EVENT, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
			}
		}

		if (pPlot->isActiveVisible(false))
		{
			// Nuke entity mission
			CvMissionDefinition kDefiniton;
			kDefiniton.setMissionTime(GC.getMissionInfo(MISSION_NUKE).getTime() * gDLL->getSecsPerTurn());
			kDefiniton.setMissionType(MISSION_NUKE);
			kDefiniton.setPlot(pPlot);
			kDefiniton.setUnit(BATTLE_UNIT_ATTACKER, this);
			kDefiniton.setUnit(BATTLE_UNIT_DEFENDER, this);

			// Add the intercepted mission (defender is not NULL)
			gDLL->getEntityIFace()->AddMission(&kDefiniton);
		}

		kill(true);
		return true; // Intercepted!!! (XXX need special event for this...)
	}

	if (pPlot->isActiveVisible(false))
	{
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
		// Dale - NB: A-Bomb START
		if(airBaseCombatStr() != 0){
			CvAirMissionDefinition kAirMission;

			kAirMission.setMissionTime(GC.getMissionInfo(MISSION_AIRSTRIKE).getTime() * gDLL->getSecsPerTurn());
			kAirMission.setMissionType(MISSION_AIRSTRIKE);
			kAirMission.setPlot(pPlot);
			kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
			kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
			kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
			kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);

			gDLL->getEntityIFace()->AddMission(&kAirMission);
		}
		else {
			// Nuke entity mission
			CvMissionDefinition kDefiniton;

			kDefiniton.setMissionTime(GC.getMissionInfo(MISSION_NUKE).getTime() * gDLL->getSecsPerTurn());
			kDefiniton.setMissionType(MISSION_NUKE);
			kDefiniton.setPlot(pPlot);
			kDefiniton.setUnit(BATTLE_UNIT_ATTACKER, this);
			kDefiniton.setUnit(BATTLE_UNIT_DEFENDER, NULL);

			// Add the non-intercepted mission (defender is NULL)
			gDLL->getEntityIFace()->AddMission(&kDefiniton);
		}
		// Dale - NB: A-Bomb END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
	}

	setMadeAttack(true);
	setAttackPlot(pPlot, false);

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (abTeamsAffected[iI])
		{
			GET_TEAM((TeamTypes)iI).changeWarWeariness(getTeam(), 100 * GC.getDefineINT("WW_HIT_BY_NUKE"));
			GET_TEAM(getTeam()).changeWarWeariness(((TeamTypes)iI), 100 * GC.getDefineINT("WW_ATTACKED_WITH_NUKE"));
			GET_TEAM(getTeam()).AI_changeWarSuccess(((TeamTypes)iI), GC.getDefineINT("WAR_SUCCESS_NUKE"));
		}
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (iI != getTeam())
			{
				if (abTeamsAffected[iI])
				{
					for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
					{
						if (GET_PLAYER((PlayerTypes)iJ).isAlive())
						{
							if (GET_PLAYER((PlayerTypes)iJ).getTeam() == ((TeamTypes)iI))
							{
								GET_PLAYER((PlayerTypes)iJ).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_NUKED_US, 1);
							}
						}
					}
				}
				else
				{
					for (iJ = 0; iJ < MAX_TEAMS; iJ++)
					{
						if (GET_TEAM((TeamTypes)iJ).isAlive())
						{
							if (abTeamsAffected[iJ])
							{
								if (GET_TEAM((TeamTypes)iI).isHasMet((TeamTypes)iJ))
								{
									if (GET_TEAM((TeamTypes)iI).AI_getAttitude((TeamTypes)iJ) >= ATTITUDE_CAUTIOUS)
									{
										for (iK = 0; iK < MAX_PLAYERS; iK++)
										{
											if (GET_PLAYER((PlayerTypes)iK).isAlive())
											{
												if (GET_PLAYER((PlayerTypes)iK).getTeam() == ((TeamTypes)iI))
												{
													GET_PLAYER((PlayerTypes)iK).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_NUKED_FRIEND, 1);
												}
											}
										}
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// XXX some AI should declare war here...

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_NUKE_LAUNCHED", GET_PLAYER(getOwnerINLINE()).getNameKey(), getNameKey());
			gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), (((PlayerTypes)iI) == getOwnerINLINE()), GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_NUKE_EXPLODES", MESSAGE_TYPE_MAJOR_EVENT, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
		}
	}

	if (isSuicide())
	{
		kill(true);
	}

	return true;
}


bool CvUnit::canRecon(const CvPlot* pPlot) const
{
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}

	if (airRange() == 0)
	{
		return false;
	}

	if (m_pUnitInfo->isSuicide())
	{
		return false;
	}

	return true;
}



bool CvUnit::canReconAt(const CvPlot* pPlot, int iX, int iY) const
{
	if (!canRecon(pPlot))
	{
		return false;
	}

	int iDistance = plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iX, iY);
	if (iDistance > airRange() || 0 == iDistance)
	{
		return false;
	}

	return true;
}


bool CvUnit::recon(int iX, int iY)
{
	CvPlot* pPlot;

	if (!canReconAt(plot(), iX, iY))
	{
		return false;
	}

	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	setReconPlot(pPlot);

	finishMoves();

	if (pPlot->isActiveVisible(false))
	{
		CvAirMissionDefinition kAirMission;
		kAirMission.setMissionType(MISSION_RECON);
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
		kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
		kAirMission.setPlot(pPlot);
		kAirMission.setMissionTime(GC.getMissionInfo((MissionTypes)MISSION_RECON).getTime() * gDLL->getSecsPerTurn());
		gDLL->getEntityIFace()->AddMission(&kAirMission);
	}

	return true;
}


bool CvUnit::canParadrop(const CvPlot* pPlot) const
{
	if (getDropRange() <= 0)
	{
		return false;
	}

	if (hasMoved())
	{
		return false;
	}

	if (!pPlot->isFriendlyCity(*this, true))
	{
		return false;
	}

	return true;
}



bool CvUnit::canParadropAt(const CvPlot* pPlot, int iX, int iY) const
{
	if (!canParadrop(pPlot))
	{
		return false;
	}

	CvPlot* pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	if (NULL == pTargetPlot || pTargetPlot == pPlot)
	{
		return false;
	}

	if (!pTargetPlot->isVisible(getTeam(), false))
	{
		return false;
	}

	if (!canMoveInto(pTargetPlot, false, false, true))
	{
		return false;
	}

	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iX, iY) > getDropRange())
	{
		return false;
	}

	if (!canCoexistWithEnemyUnit(NO_TEAM))
	{
		if (pTargetPlot->isEnemyCity(*this))
		{
			return false;
		}

		if (pTargetPlot->isVisibleEnemyUnit(this))
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::paradrop(int iX, int iY)
{
	if (!canParadropAt(plot(), iX, iY))
	{
		return false;
	}

	CvPlot* pPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	changeMoves(GC.getMOVE_DENOMINATOR() / 2);
	setMadeAttack(true);

	setXY(pPlot->getX_INLINE(), pPlot->getY_INLINE());

	//check if intercepted
	if(interceptTest(pPlot))
	{
		return true;
	}
	
	//play paradrop animation by itself
	if (pPlot->isActiveVisible(false))
	{
		CvAirMissionDefinition kAirMission;
		kAirMission.setMissionType(MISSION_PARADROP);
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
		kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
		kAirMission.setPlot(pPlot);
		kAirMission.setMissionTime(GC.getMissionInfo((MissionTypes)MISSION_PARADROP).getTime() * gDLL->getSecsPerTurn());
		gDLL->getEntityIFace()->AddMission(&kAirMission);
	}

	return true;
}


bool CvUnit::canAirBomb(const CvPlot* pPlot) const
{
/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - AB: Bombing START
	if (GC.getDCM_AIR_BOMBING())
	{
		if (isHuman())
		{
			return false;
		}
	}
	// Dale - AB: Bombing END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}

	if (airBombBaseRate() == 0)
	{
		return false;
	}

	if (isMadeAttack())
	{
		return false;
	}

	return true;
}


bool CvUnit::canAirBombAt(const CvPlot* pPlot, int iX, int iY) const
{
	CvCity* pCity;
	CvPlot* pTargetPlot;

	if (!canAirBomb(pPlot))
	{
		return false;
	}

	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) > airRange())
	{
		return false;
	}

	if (pTargetPlot->isOwned())
	{
		if (!potentialWarAction(pTargetPlot))
		{
			return false;
		}
	}

	pCity = pTargetPlot->getPlotCity();

/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
		// Dale - AB: Bombing START
	if (pTargetPlot->getImprovementType() != NO_IMPROVEMENT)
	{
		if (GC.getImprovementInfo(pTargetPlot->getImprovementType()).isActsAsCity() && pCity == NULL)
		{
			if (GC.getUnitInfo(getUnitType()).getDCMAirBomb4())
			{
				int iCount = 0;
				CvUnit* pUnit = NULL;
				CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
				CvUnit* pLoopUnit;
				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pPlot->nextUnitNode(pUnitNode);
					if (pLoopUnit->getDomainType() == DOMAIN_SEA && atWar(pLoopUnit->getTeam(), getTeam()))
					{
						iCount++;
					}
				}
				if (iCount > 0)
				{
					return true;
				}
			}
		}
	}
	else if (pCity != NULL)
	{
		if (GC.getDCM_AIR_BOMBING())
		{
			int iI, iLoop;
			CvUnit* pLoopUnit;
			for (iI = 0; iI < MAX_PLAYERS; ++iI)
			{
				if (atWar(GET_PLAYER((PlayerTypes)iI).getTeam(), getTeam()))
				{
					for(pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
					{
						if (pLoopUnit->plot() == pTargetPlot)
						{
							if (pLoopUnit->getDomainType() == DOMAIN_SEA)
							{
								return true;
							}
						}
					}
				}
			}
			if (pCity->isBombardable(this))
			{
				return true;
			}
			return false;
		} else {
			if (!(pCity->isBombardable(this)))
			{
				return false;
			}
		}
		// Dale - AB: Bombing END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
	}
	else
	{
		if (pTargetPlot->getImprovementType() == NO_IMPROVEMENT)
		{
			return false;
		}

		if (GC.getImprovementInfo(pTargetPlot->getImprovementType()).isPermanent())
		{
			return false;
		}

		if (GC.getImprovementInfo(pTargetPlot->getImprovementType()).getAirBombDefense() == -1)
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::airBomb(int iX, int iY)
{
	CvCity* pCity;
	CvPlot* pPlot;
	CvWString szBuffer;

/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - AB: Bombing START
	bool bNoTarget = true;
	int iMis0, iMis1, iMis2, iMis3, iMis4, iMis5;
	iMis0 = iMis1 = iMis2 = iMis3 = iMis4 = iMis5 = 0;
	int iI, iCount = 0;
	CvUnit* pUnit = NULL;
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	// Dale - AB: Bombing END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
	if (!canAirBombAt(plot(), iX, iY))
	{
		return false;
	}

	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (!isEnemy(pPlot->getTeam()))
	{
		getGroup()->groupDeclareWar(pPlot, true);
	}

	if (!isEnemy(pPlot->getTeam()))
	{
		return false;
	}

	if (interceptTest(pPlot))
	{
		return true;
	}

/************************************************************************************************/
/* DCM	                  Start		 05/31/10                        Johnny Smith               */
/*                                                                   Afforess                   */
/* Battle Effects                                                                               */
/************************************************************************************************/
	setBattlePlot(pPlot);
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/

	pCity = pPlot->getPlotCity();

	// RevolutionDCM start - AB Bombing<->BTS interaction bug fix
	if (pPlot->getImprovementType() != NO_IMPROVEMENT)
	{
		if (!GC.getDCM_AIR_BOMBING())
		{
			// RevolutionDCM start - vanilla airbomb behaviour
			if (GC.getGameINLINE().getSorenRandNum(airBombCurrRate(), "Air Bomb - Offense") >=
					GC.getGameINLINE().getSorenRandNum(GC.getImprovementInfo(pPlot->getImprovementType()).getAirBombDefense(), "Air Bomb - Defense"))
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_DESTROYED_IMP", getNameKey(), GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

				if (pPlot->isOwned())
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_IMP_WAS_DESTROYED", GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide(), getNameKey(), getVisualCivAdjective(pPlot->getTeam()));
					gDLL->getInterfaceIFace()->addMessage(pPlot->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
				}

				pPlot->setImprovementType((ImprovementTypes)(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage()));
			}
			else
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_FAIL_DESTROY_IMP", getNameKey(), GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMB_FAILS", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			}
			// RevolutionDCM end - vanilla airbomb behaviour
		} else
		// RevolutionDCM end - AB Bombing<->BTS interaction bug fix
		{
			// Dale - AB: AI Bombing START
			if (GC.getImprovementInfo(pPlot->getImprovementType()).isActsAsCity() && pCity == NULL)
			{
				if (GC.getUnitInfo(getUnitType()).getDCMAirBomb4())
				{
					iMis4 = 10;
					iCount = 0;
					pUnit = NULL;
					pUnitNode = pPlot->headUnitNode();
					while (pUnitNode != NULL)
					{
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pPlot->nextUnitNode(pUnitNode);
						if (pLoopUnit->getDomainType() == DOMAIN_SEA)
						{
							iCount++;
						}
					}
					if (iCount > 0)
					{
						airBomb4(iX, iY);
					}
				}	
			}
			// Dale - AB: AI Bombing END
		}
	} else
	{
		// RevolutionDCM start - AB Bombing<->BTS interaction bug fix					
		if (pCity != NULL)
		{
			if (!GC.getDCM_AIR_BOMBING())
			{
				// RevolutionDCM start - vanilla airbomb behaviour
				pCity->changeDefenseModifier(-airBombCurrRate());
				
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_DEFENSES_REDUCED_TO", pCity->getNameKey(), pCity->getDefenseModifier(false), getNameKey());
				gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
				
				szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_DEFENSES_REDUCED_TO", getNameKey(), pCity->getNameKey(), pCity->getDefenseModifier(false));
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE());
				// RevolutionDCM end - vanilla airbomb behaviour
			} else
			{
				// Dale - AB: AI Bombing START
				if (GC.getUnitInfo(getUnitType()).getDCMAirBomb1())
				{
					iMis1 = 10;
					iCount = 0;
					for (int iDX = -2; iDX <= 2; iDX++)
					{
						for (int iDY = 2; iDY <= 2; iDY++)
						{
							pLoopPlot = plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), iDX, iDY);
							if (pLoopPlot != NULL)
							{
								pUnit = NULL;
								pUnitNode = pPlot->headUnitNode();
								while (pUnitNode != NULL)
								{
									pLoopUnit = ::getUnit(pUnitNode->m_data);
									pUnitNode = pPlot->nextUnitNode(pUnitNode);
									if (pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
									{
										iCount++;
									}
								}
							}
						}
					}
					iMis1 *= (iCount * 2);
				}
				if (GC.getUnitInfo(getUnitType()).getDCMAirBomb2())
				{
					iMis2 = 10;
					iCount = 0;
					for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
					{
						if (GC.getBuildingInfo((BuildingTypes)iI).getDCMAirbombMission() == 2)
						{
							if (pCity->getNumRealBuilding((BuildingTypes)iI))
							{
								iCount++;
							}
						}
					}
					iMis2 *= iCount;
				}
				if (GC.getUnitInfo(getUnitType()).getDCMAirBomb3())
				{
					iMis3 = 10;
					iCount = 0;
					for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
					{
						if (GC.getBuildingInfo((BuildingTypes)iI).getDCMAirbombMission() == 3)
						{
							if (pCity->getNumRealBuilding((BuildingTypes)iI))
							{
								iCount++;
							}
						}
					}
					iMis3 *= (iCount * 2);
				}
				if (GC.getUnitInfo(getUnitType()).getDCMAirBomb4())
				{
					iMis4 = 10;
					iCount = 0;
					pUnit = NULL;
					pUnitNode = pPlot->headUnitNode();
					while (pUnitNode != NULL)
					{
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pPlot->nextUnitNode(pUnitNode);
						if (pLoopUnit->getDomainType() == DOMAIN_SEA)
						{
							iCount++;
						}
					}
					iMis4 *= (iCount * 4);
				}
				if (GC.getUnitInfo(getUnitType()).getDCMAirBomb5())
				{
					iMis5 = 10;
					iMis5 *= GC.getGame().getSorenRandNum(20, "Strat Bombing");
				}
				int temp = iMis1;
				iMis0 = 1;
				if (iMis2 > temp)
				{
					temp = iMis2;
					iMis0 = 2;
				}
				if (iMis3 > temp)
				{
					temp = iMis3;
					iMis0 = 3;
				}
				if (iMis4 > temp)
				{
					temp = iMis4;
					iMis0 = 4;
				}
				if (iMis5 > temp)
				{
					temp = iMis5;
					iMis0 = 5;
				}
				switch(iMis0)
				{
				case 1:
					if (airBomb1(iX, iY))
					{
						bNoTarget = false;
					}
					break;
				case 2:
					if (airBomb2(iX, iY))
					{
						bNoTarget = false;
					}
					break;
				case 3:
					if (airBomb3(iX, iY))
					{
						bNoTarget = false;
					}
					break;
				case 4:
					if (airBomb4(iX, iY))
					{
						bNoTarget = false;
					}
					break;
				case 5:
					if (airBomb5(iX, iY))
					{
						bNoTarget = false;
					}
					break;
				}
				if(bNoTarget)
				{
					if(pCity->getPopulation() > 1)
					{
						if(GC.getGameINLINE().getSorenRandNum(5, "Airbomb population") < 2)
						{
							pCity->changePopulation(-1);
							szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIRBOMB_POP");
							gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
							szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_AIRBOMB_POP");
							gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
						}
					}
				}
				// Dale - AB: AI Bombing END
			}
		// RevolutionDCM end - AB Bombing<->BTS interaction bug fix
		}
	}

	setReconPlot(pPlot);

	setMadeAttack(true);
	changeMoves(GC.getMOVE_DENOMINATOR());

	if (pPlot->isActiveVisible(false))
	{
		CvAirMissionDefinition kAirMission;
		kAirMission.setMissionType(MISSION_AIRBOMB);
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
		kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
		kAirMission.setPlot(pPlot);
		kAirMission.setMissionTime(GC.getMissionInfo((MissionTypes)MISSION_AIRBOMB).getTime() * gDLL->getSecsPerTurn());

		gDLL->getEntityIFace()->AddMission(&kAirMission);
	}

	if (isSuicide())
	{
		kill(true);
	}

	return true;
}


CvCity* CvUnit::bombardTarget(const CvPlot* pPlot) const
{
	int iBestValue = MAX_INT;
	CvCity* pBestCity = NULL;

	for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		CvPlot* pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

		if (pLoopPlot != NULL)
		{
			CvCity* pLoopCity = pLoopPlot->getPlotCity();

			if (pLoopCity != NULL)
			{
				if (pLoopCity->isBombardable(this))
				{
					int iValue = pLoopCity->getDefenseDamage();

					// always prefer cities we are at war with
					if (isEnemy(pLoopCity->getTeam(), pPlot))
					{
						iValue *= 128;
					}

					if (iValue < iBestValue)
					{
						iBestValue = iValue;
						pBestCity = pLoopCity;
					}
				}
			}
		}
	}

	return pBestCity;
}


bool CvUnit::canBombard(const CvPlot* pPlot) const
{
	// RevolutionDCM - return to vanilla
	// Dale - RB: Field Bombard START
//	if (GC.getDCM_RANGE_BOMBARD())
//	{
//		if(getDCMBombRange() > 0)
//		{
//			return false;
//		}
//	} /*else {  // RevolutionDCM - This is unnecessary, checked below
//		if (bombardTarget(pPlot) == NULL)
//		{
//			return false;
//		}
//	}*/
	// Dale - RB: Field Bombard END

	if (bombardRate() <= 0)
	{
		return false;
	}

	if (isMadeAttack())
	{
		return false;
	}

	if (isCargo())
	{
		return false;
	}

	if (bombardTarget(pPlot) == NULL)
	{
		return false;
	}

	return true;
}


bool CvUnit::bombard()
{
	CvPlot* pPlot = plot();
	if (!canBombard(pPlot))
	{
		return false;
	}

	CvCity* pBombardCity = bombardTarget(pPlot);
	FAssertMsg(pBombardCity != NULL, "BombardCity is not assigned a valid value");

	// Dale - RB: Bug Fix (RevolutionDCM - just checks for a null value)
	if (pBombardCity != NULL)
	{
		CvPlot* pTargetPlot = pBombardCity->plot();

		if (!isEnemy(pTargetPlot->getTeam()))
		{
			getGroup()->groupDeclareWar(pTargetPlot, true);
		}

		if (!isEnemy(pTargetPlot->getTeam()))
		{
			return false;
		}

		int iBombardModifier = 0;
		if (!ignoreBuildingDefense())
		{
			iBombardModifier -= pBombardCity->getBuildingBombardDefense();
		}

		pBombardCity->changeDefenseModifier(-(bombardRate() * std::max(0, 100 + iBombardModifier)) / 100);

		setMadeAttack(true);
		changeMoves(GC.getMOVE_DENOMINATOR());

		CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_DEFENSES_IN_CITY_REDUCED_TO", pBombardCity->getNameKey(), pBombardCity->getDefenseModifier(false), GET_PLAYER(getOwnerINLINE()).getNameKey());
		gDLL->getInterfaceIFace()->addMessage(pBombardCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pBombardCity->getX_INLINE(), pBombardCity->getY_INLINE(), true, true);

		szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_REDUCE_CITY_DEFENSES", getNameKey(), pBombardCity->getNameKey(), pBombardCity->getDefenseModifier(false));
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pBombardCity->getX_INLINE(), pBombardCity->getY_INLINE());
/************************************************************************************************/
/* Afforess	                  Start		 07/22/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		if (GC.getGameINLINE().isModderGameOption(MODDERGAMEOPTION_IMPROVED_XP))
		{
			 setExperience100(getExperience100() + 25, -1);
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

		if (pPlot->isActiveVisible(false))
		{
			CvUnit *pDefender = pBombardCity->plot()->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);

			// Bombard entity mission
			CvMissionDefinition kDefiniton;
			kDefiniton.setMissionTime(GC.getMissionInfo(MISSION_BOMBARD).getTime() * gDLL->getSecsPerTurn());
			kDefiniton.setMissionType(MISSION_BOMBARD);
			kDefiniton.setPlot(pBombardCity->plot());
			kDefiniton.setUnit(BATTLE_UNIT_ATTACKER, this);
			kDefiniton.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
			gDLL->getEntityIFace()->AddMission(&kDefiniton);
		}
	}
	return true;
}


bool CvUnit::canPillage(const CvPlot* pPlot) const
{
	if (!(m_pUnitInfo->isPillage()))
	{
		return false;
	}
/************************************************************************************************/
/* Afforess	                  Start		 06/01/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (pPlot == NULL)
	{
		return false;
	}

	if (GET_PLAYER(getOwnerINLINE()).isModderOption(MODDEROPTION_NO_FRIENDLY_PILLAGING))
	{
		if (pPlot->getTeam() == getTeam())
		{
			return false;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	if (pPlot->isCity())
	{
		return false;
	}

	if (pPlot->getImprovementType() == NO_IMPROVEMENT)
	{
		if (!(pPlot->isRoute()))
		{
			return false;
		}
	}
	else
	{
		if (GC.getImprovementInfo(pPlot->getImprovementType()).isPermanent())
		{
			return false;
		}
	}

	if (pPlot->isOwned())
	{
		if (!potentialWarAction(pPlot))
		{
			if ((pPlot->getImprovementType() == NO_IMPROVEMENT) || (pPlot->getOwnerINLINE() != getOwnerINLINE()))
			{
				return false;
			}
		}
	}

	if (!(pPlot->isValidDomainForAction(*this)))
	{
		return false;
	}

	return true;
}


bool CvUnit::pillage()
{
	CvWString szBuffer;
	int iPillageGold;
	long lPillageGold;
	ImprovementTypes eTempImprovement = NO_IMPROVEMENT;
	RouteTypes eTempRoute = NO_ROUTE;

	CvPlot* pPlot = plot();

	if (!canPillage(pPlot))
	{
		return false;
	}

	if (pPlot->isOwned())
	{
		// we should not be calling this without declaring war first, so do not declare war here
		if (!isEnemy(pPlot->getTeam(), pPlot))
		{
			if ((pPlot->getImprovementType() == NO_IMPROVEMENT) || (pPlot->getOwnerINLINE() != getOwnerINLINE()))
			{
				return false;
			}
		}
	}

/************************************************************************************************/
/* JOOYO_ADDON, Added by Jooyo, 07/07/09                                                        */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*	
	if (getDomainType() == DOMAIN_LAND && pPlot->isCanUseRouteLandUnits() && pPlot->isWater())
	{
		return false;
	}

	if (getDomainType() == DOMAIN_SEA && pPlot->isCanUseRouteSeaUnits() && !pPlot->isWater())
	{
		return false;
	}
*/
/************************************************************************************************/
/* JOOYO_ADDON                          END                                                     */
/************************************************************************************************/
	
	if (pPlot->isWater())
	{
		// UncutDragon
/* original code
		CvUnit* pInterceptor = bestSeaPillageInterceptor(this, GC.getDefineINT("COMBAT_DIE_SIDES") / 2);
*/		// modified
		CvUnit* pInterceptor = bestSeaPillageInterceptor(this, GC.getCOMBAT_DIE_SIDES() / 2);
		// /UncutDragon
		if (NULL != pInterceptor)
		{
			setMadeAttack(false);

			int iWithdrawal = withdrawalProbability();
			changeExtraWithdrawal(-iWithdrawal); // no withdrawal since we are really the defender
			attack(pInterceptor->plot(), false);
			changeExtraWithdrawal(iWithdrawal);

			return false;
		}
	}

	if (pPlot->getImprovementType() != NO_IMPROVEMENT)
	{
		eTempImprovement = pPlot->getImprovementType();

		if (pPlot->getTeam() != getTeam())
		{
			// Use python to determine pillage amounts...
			lPillageGold = 0;
			
			CyPlot* pyPlot = new CyPlot(pPlot);
			CyUnit* pyUnit = new CyUnit(this);

			CyArgsList argsList;
			argsList.add(gDLL->getPythonIFace()->makePythonObject(pyPlot));	// pass in plot class
			argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class

			gDLL->getPythonIFace()->callFunction(PYGameModule, "doPillageGold", argsList.makeFunctionArgs(),&lPillageGold);

			delete pyPlot;	// python fxn must not hold on to this pointer 
			delete pyUnit;	// python fxn must not hold on to this pointer 

			iPillageGold = (int)lPillageGold;

			if (iPillageGold > 0)
			{
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/
				// ------ BEGIN InfluenceDrivenWar -------------------------------
				float fInfluenceRatio = 0.0f;
				if (GC.getDefineINT("IDW_ENABLED") && GC.getDefineINT("IDW_PILLAGE_INFLUENCE_ENABLED"))
				{
					if (atWar(pPlot->getTeam(), getTeam()))
					{
						fInfluenceRatio = doPillageInfluence();
					}
				}
				// ------ END InfluenceDrivenWar -------------------------------

/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/
				GET_PLAYER(getOwnerINLINE()).changeGold(iPillageGold);

				szBuffer = gDLL->getText("TXT_KEY_MISC_PLUNDERED_GOLD_FROM_IMP", iPillageGold, GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/
				// ------ BEGIN InfluenceDrivenWar -------------------------------
				if (fInfluenceRatio > 0.0f)
				{
					CvWString szInfluence;
					szInfluence.Format(L" %s: +%.1f%%", gDLL->getText("TXT_KEY_TILE_INFLUENCE").GetCString(), fInfluenceRatio);
					szBuffer += szInfluence;
				}
				// ------ END InfluenceDrivenWar -------------------------------
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

				if (pPlot->isOwned())
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_IMP_DESTROYED", GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide(), getNameKey(), getVisualCivAdjective(pPlot->getTeam()));
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/
					// ------ BEGIN InfluenceDrivenWar -------------------------------
					if (fInfluenceRatio > 0.0f)
					{
						CvWString szInfluence;
						szInfluence.Format(L" %s: -%.1f%%", gDLL->getText("TXT_KEY_TILE_INFLUENCE").GetCString(), fInfluenceRatio);
						szBuffer += szInfluence;
					}
					// ------ END InfluenceDrivenWar -------------------------------
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/
					gDLL->getInterfaceIFace()->addMessage(pPlot->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
				}
			}
		}

		pPlot->setImprovementType((ImprovementTypes)(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage()));
	}
	else if (pPlot->isRoute())
	{
		eTempRoute = pPlot->getRouteType();
		pPlot->setRouteType(NO_ROUTE, true); // XXX downgrade rail???
	}

	changeMoves(GC.getMOVE_DENOMINATOR());

	if (pPlot->isActiveVisible(false))
	{
		// Pillage entity mission
		CvMissionDefinition kDefiniton;
		kDefiniton.setMissionTime(GC.getMissionInfo(MISSION_PILLAGE).getTime() * gDLL->getSecsPerTurn());
		kDefiniton.setMissionType(MISSION_PILLAGE);
		kDefiniton.setPlot(pPlot);
		kDefiniton.setUnit(BATTLE_UNIT_ATTACKER, this);
		kDefiniton.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		gDLL->getEntityIFace()->AddMission(&kDefiniton);
	}

	if (eTempImprovement != NO_IMPROVEMENT || eTempRoute != NO_ROUTE)
	{
		CvEventReporter::getInstance().unitPillage(this, eTempImprovement, eTempRoute, getOwnerINLINE());
	}

	return true;
}


bool CvUnit::canPlunder(const CvPlot* pPlot, bool bTestVisible) const
{
	if (getDomainType() != DOMAIN_SEA)
	{
		return false;
	}

	if (!(m_pUnitInfo->isPillage()))
	{
		return false;
	}

	if (!pPlot->isWater())
	{
		return false;
	}

	if (pPlot->isFreshWater())
	{
		return false;
	}

	if (!pPlot->isValidDomainForAction(*this))
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (pPlot->getTeam() == getTeam())
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::plunder()
{
	CvPlot* pPlot = plot();

	if (!canPlunder(pPlot))
	{
		return false;
	}

	setBlockading(true);

	finishMoves();

	return true;
}


void CvUnit::updatePlunder(int iChange, bool bUpdatePlotGroups)
{
	int iBlockadeRange = GC.getDefineINT("SHIP_BLOCKADE_RANGE");

	bool bOldTradeNet;
	bool bChanged = false;

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      06/01/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
	//gDLL->getFAStarIFace()->ForceReset(&GC.getStepFinder());
	bool bValid = false;
	for (int i = -iBlockadeRange; i <= iBlockadeRange; ++i)
	{
		for (int j = -iBlockadeRange; j <= iBlockadeRange; ++j)
		{
			CvPlot* pLoopPlot = ::plotXY(getX_INLINE(), getY_INLINE(), i, j);

			if (NULL != pLoopPlot && pLoopPlot->isWater() && pLoopPlot->area() == area())
			{

				int iPathDist = GC.getMapINLINE().calculatePathDistance(plot(),pLoopPlot);
				
				// BBAI NOTES:  There are rare issues where the path finder will return incorrect results
				// for unknown reasons.  Seems to find a suboptimal path sometimes in partially repeatable 
				// circumstances.  The fix below is a hack to address the permanent one or two tile blockades which 
				// would appear randomly, it should cause extra blockade clearing only very rarely.
				/*
				if( iPathDist > iBlockadeRange )
				{
					// No blockading on other side of an isthmus
					continue;
				}
				*/
				
				if( (iPathDist >= 0) && (iPathDist <= iBlockadeRange + 2) )
				{
					for (int iTeam = 0; iTeam < MAX_TEAMS; ++iTeam)
					{
						if (isEnemy((TeamTypes)iTeam))
						{
							bValid = (iPathDist <= iBlockadeRange);
							if( !bValid && (iChange == -1 && pLoopPlot->getBlockadedCount((TeamTypes)iTeam) > 0) )
							{
								bValid = true;
							}

							if( bValid )
							{
								if (!bChanged)
								{
									bOldTradeNet = pLoopPlot->isTradeNetwork((TeamTypes)iTeam);
								}

								pLoopPlot->changeBlockadedCount((TeamTypes)iTeam, iChange);

								if (!bChanged)
								{
									bChanged = (bOldTradeNet != pLoopPlot->isTradeNetwork((TeamTypes)iTeam));
								}
							}
						}
					}
				}
			}
		}
	}

	if (bChanged)
	{
		gDLL->getInterfaceIFace()->setDirty(BlockadedPlots_DIRTY_BIT, true);

		if (bUpdatePlotGroups)
		{
			GC.getGameINLINE().updatePlotGroups();
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
}


int CvUnit::sabotageCost(const CvPlot* pPlot) const
{
	return GC.getDefineINT("BASE_SPY_SABOTAGE_COST");
}


// XXX compare with destroy prob...
int CvUnit::sabotageProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle) const
{
	CvPlot* pLoopPlot;
	int iDefenseCount;
	int iCounterSpyCount;
	int iProb;
	int iI;

	iProb = 0; // XXX

	if (pPlot->isOwned())
	{
		iDefenseCount = pPlot->plotCount(PUF_canDefend, -1, -1, NO_PLAYER, pPlot->getTeam());
		iCounterSpyCount = pPlot->plotCount(PUF_isCounterSpy, -1, -1, NO_PLAYER, pPlot->getTeam());

		for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

			if (pLoopPlot != NULL)
			{
				iCounterSpyCount += pLoopPlot->plotCount(PUF_isCounterSpy, -1, -1, NO_PLAYER, pPlot->getTeam());
			}
		}
	}
	else
	{
		iDefenseCount = 0;
		iCounterSpyCount = 0;
	}

	if (eProbStyle == PROBABILITY_HIGH)
	{
		iCounterSpyCount = 0;
	}

	iProb += (40 / (iDefenseCount + 1)); // XXX

	if (eProbStyle != PROBABILITY_LOW)
	{
		iProb += (50 / (iCounterSpyCount + 1)); // XXX
	}

	return iProb;
}


bool CvUnit::canSabotage(const CvPlot* pPlot, bool bTestVisible) const
{
	if (!(m_pUnitInfo->isSabotage()))
	{
		return false;
	}

	if (pPlot->getTeam() == getTeam())
	{
		return false;
	}

	if (pPlot->isCity())
	{
		return false;
	}

	if (pPlot->getImprovementType() == NO_IMPROVEMENT)
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (GET_PLAYER(getOwnerINLINE()).getGold() < sabotageCost(pPlot))
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::sabotage()
{
	CvCity* pNearestCity;
	CvPlot* pPlot;
	CvWString szBuffer;
	bool bCaught;

	if (!canSabotage(plot()))
	{
		return false;
	}

	pPlot = plot();

	bCaught = (GC.getGameINLINE().getSorenRandNum(100, "Spy: Sabotage") > sabotageProb(pPlot));

	GET_PLAYER(getOwnerINLINE()).changeGold(-(sabotageCost(pPlot)));

	if (!bCaught)
	{
		pPlot->setImprovementType((ImprovementTypes)(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage()));

		finishMoves();

		pNearestCity = GC.getMapINLINE().findCity(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pPlot->getOwnerINLINE(), NO_TEAM, false);

		if (pNearestCity != NULL)
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_SPY_SABOTAGED", getNameKey(), pNearestCity->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_SABOTAGE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

			if (pPlot->isOwned())
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_SABOTAGE_NEAR", pNearestCity->getNameKey());
				gDLL->getInterfaceIFace()->addMessage(pPlot->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_SABOTAGE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
			}
		}

		if (pPlot->isActiveVisible(false))
		{
			NotifyEntity(MISSION_SABOTAGE);
		}
	}
	else
	{
		if (pPlot->isOwned())
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_SPY_CAUGHT_AND_KILLED", GET_PLAYER(getOwnerINLINE()).getCivilizationAdjective(), getNameKey());
			gDLL->getInterfaceIFace()->addMessage(pPlot->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSE", MESSAGE_TYPE_INFO);
		}

		szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_SPY_CAUGHT", getNameKey());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSED", MESSAGE_TYPE_INFO);

		if (plot()->isActiveVisible(false))
		{
			NotifyEntity(MISSION_SURRENDER);
		}

		if (pPlot->isOwned())
		{
			if (!isEnemy(pPlot->getTeam(), pPlot))
			{
				GET_PLAYER(pPlot->getOwnerINLINE()).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_SPY_CAUGHT, 1);
			}
		}

		kill(true, pPlot->getOwnerINLINE());
	}

	return true;
}


int CvUnit::destroyCost(const CvPlot* pPlot) const
{
	CvCity* pCity;
	bool bLimited;

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return 0;
	}

	bLimited = false;

	if (pCity->isProductionUnit())
	{
		bLimited = isLimitedUnitClass((UnitClassTypes)(GC.getUnitInfo(pCity->getProductionUnit()).getUnitClassType()));
	}
	else if (pCity->isProductionBuilding())
	{
		bLimited = isLimitedWonderClass((BuildingClassTypes)(GC.getBuildingInfo(pCity->getProductionBuilding()).getBuildingClassType()));
	}
	else if (pCity->isProductionProject())
	{
		bLimited = isLimitedProject(pCity->getProductionProject());
	}

	return (GC.getDefineINT("BASE_SPY_DESTROY_COST") + (pCity->getProduction() * ((bLimited) ? GC.getDefineINT("SPY_DESTROY_COST_MULTIPLIER_LIMITED") : GC.getDefineINT("SPY_DESTROY_COST_MULTIPLIER"))));
}


int CvUnit::destroyProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle) const
{
	CvCity* pCity;
	CvPlot* pLoopPlot;
	int iDefenseCount;
	int iCounterSpyCount;
	int iProb;
	int iI;

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return 0;
	}

	iProb = 0; // XXX

	iDefenseCount = pPlot->plotCount(PUF_canDefend, -1, -1, NO_PLAYER, pPlot->getTeam());

	iCounterSpyCount = pPlot->plotCount(PUF_isCounterSpy, -1, -1, NO_PLAYER, pPlot->getTeam());

	for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

		if (pLoopPlot != NULL)
		{
			iCounterSpyCount += pLoopPlot->plotCount(PUF_isCounterSpy, -1, -1, NO_PLAYER, pPlot->getTeam());
		}
	}

	if (eProbStyle == PROBABILITY_HIGH)
	{
		iCounterSpyCount = 0;
	}

	iProb += (25 / (iDefenseCount + 1)); // XXX

	if (eProbStyle != PROBABILITY_LOW)
	{
		iProb += (50 / (iCounterSpyCount + 1)); // XXX
	}

	iProb += std::min(25, pCity->getProductionTurnsLeft()); // XXX

	return iProb;
}


bool CvUnit::canDestroy(const CvPlot* pPlot, bool bTestVisible) const
{
	CvCity* pCity;

	if (!(m_pUnitInfo->isDestroy()))
	{
		return false;
	}

	if (pPlot->getTeam() == getTeam())
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getProduction() == 0)
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (GET_PLAYER(getOwnerINLINE()).getGold() < destroyCost(pPlot))
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::destroy()
{
	CvCity* pCity;
	CvWString szBuffer;
	bool bCaught;

	if (!canDestroy(plot()))
	{
		return false;
	}

	bCaught = (GC.getGameINLINE().getSorenRandNum(100, "Spy: Destroy") > destroyProb(plot()));

	pCity = plot()->getPlotCity();
	FAssertMsg(pCity != NULL, "City is not assigned a valid value");

	GET_PLAYER(getOwnerINLINE()).changeGold(-(destroyCost(plot())));

	if (!bCaught)
	{
		pCity->setProduction(pCity->getProduction() / 2);

		finishMoves();

		szBuffer = gDLL->getText("TXT_KEY_MISC_SPY_DESTROYED_PRODUCTION", getNameKey(), pCity->getProductionNameKey(), pCity->getNameKey());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_DESTROY", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE());

		szBuffer = gDLL->getText("TXT_KEY_MISC_CITY_PRODUCTION_DESTROYED", pCity->getProductionNameKey(), pCity->getNameKey());
		gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_DESTROY", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);

		if (plot()->isActiveVisible(false))
		{
			NotifyEntity(MISSION_DESTROY);
		}
	}
	else
	{
		szBuffer = gDLL->getText("TXT_KEY_MISC_SPY_CAUGHT_AND_KILLED", GET_PLAYER(getOwnerINLINE()).getCivilizationAdjective(), getNameKey());
		gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSE", MESSAGE_TYPE_INFO);

		szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_SPY_CAUGHT", getNameKey());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSED", MESSAGE_TYPE_INFO);

		if (plot()->isActiveVisible(false))
		{
			NotifyEntity(MISSION_SURRENDER);
		}

		if (!isEnemy(pCity->getTeam()))
		{
			GET_PLAYER(pCity->getOwnerINLINE()).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_SPY_CAUGHT, 1);
		}

		kill(true, pCity->getOwnerINLINE());
	}

	return true;
}


int CvUnit::stealPlansCost(const CvPlot* pPlot) const
{
	CvCity* pCity;

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return 0;
	}

	return (GC.getDefineINT("BASE_SPY_STEAL_PLANS_COST") + ((GET_TEAM(pCity->getTeam()).getTotalLand() + GET_TEAM(pCity->getTeam()).getTotalPopulation()) * GC.getDefineINT("SPY_STEAL_PLANS_COST_MULTIPLIER")));
}


// XXX compare with destroy prob...
int CvUnit::stealPlansProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle) const
{
	CvCity* pCity;
	CvPlot* pLoopPlot;
	int iDefenseCount;
	int iCounterSpyCount;
	int iProb;
	int iI;

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return 0;
	}

	iProb = ((pCity->isGovernmentCenter()) ? 20 : 0); // XXX

	iDefenseCount = pPlot->plotCount(PUF_canDefend, -1, -1, NO_PLAYER, pPlot->getTeam());

	iCounterSpyCount = pPlot->plotCount(PUF_isCounterSpy, -1, -1, NO_PLAYER, pPlot->getTeam());

	for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

		if (pLoopPlot != NULL)
		{
			iCounterSpyCount += pLoopPlot->plotCount(PUF_isCounterSpy, -1, -1, NO_PLAYER, pPlot->getTeam());
		}
	}

	if (eProbStyle == PROBABILITY_HIGH)
	{
		iCounterSpyCount = 0;
	}

	iProb += (20 / (iDefenseCount + 1)); // XXX

	if (eProbStyle != PROBABILITY_LOW)
	{
		iProb += (50 / (iCounterSpyCount + 1)); // XXX
	}

	return iProb;
}


bool CvUnit::canStealPlans(const CvPlot* pPlot, bool bTestVisible) const
{
	CvCity* pCity;

	if (!(m_pUnitInfo->isStealPlans()))
	{
		return false;
	}

	if (pPlot->getTeam() == getTeam())
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (GET_PLAYER(getOwnerINLINE()).getGold() < stealPlansCost(pPlot))
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::stealPlans()
{
	CvCity* pCity;
	CvWString szBuffer;
	bool bCaught;

	if (!canStealPlans(plot()))
	{
		return false;
	}

	bCaught = (GC.getGameINLINE().getSorenRandNum(100, "Spy: Steal Plans") > stealPlansProb(plot()));

	pCity = plot()->getPlotCity();
	FAssertMsg(pCity != NULL, "City is not assigned a valid value");

	GET_PLAYER(getOwnerINLINE()).changeGold(-(stealPlansCost(plot())));

	if (!bCaught)
	{
		GET_TEAM(getTeam()).changeStolenVisibilityTimer(pCity->getTeam(), 2);

		finishMoves();

		szBuffer = gDLL->getText("TXT_KEY_MISC_SPY_STOLE_PLANS", getNameKey(), pCity->getNameKey());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_STEALPLANS", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE());

		szBuffer = gDLL->getText("TXT_KEY_MISC_PLANS_STOLEN", pCity->getNameKey());
		gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_STEALPLANS", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);

		if (plot()->isActiveVisible(false))
		{
			NotifyEntity(MISSION_STEAL_PLANS);
		}
	}
	else
	{
		szBuffer = gDLL->getText("TXT_KEY_MISC_SPY_CAUGHT_AND_KILLED", GET_PLAYER(getOwnerINLINE()).getCivilizationAdjective(), getNameKey());
		gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSE", MESSAGE_TYPE_INFO);

		szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_SPY_CAUGHT", getNameKey());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSED", MESSAGE_TYPE_INFO);

		if (plot()->isActiveVisible(false))
		{
			NotifyEntity(MISSION_SURRENDER);
		}

		if (!isEnemy(pCity->getTeam()))
		{
			GET_PLAYER(pCity->getOwnerINLINE()).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_SPY_CAUGHT, 1);
		}

		kill(true, pCity->getOwnerINLINE());
	}

	return true;
}


bool CvUnit::canFound(const CvPlot* pPlot, bool bTestVisible) const
{
	if (!isFound())
	{
		return false;
	}

	if (!(GET_PLAYER(getOwnerINLINE()).canFound(pPlot->getX_INLINE(), pPlot->getY_INLINE(), bTestVisible)))
	{
		return false;
	}

	return true;
}


bool CvUnit::found()
{
	if (!canFound(plot()))
	{
		return false;
	}

	if (GC.getGameINLINE().getActivePlayer() == getOwnerINLINE())
	{
		gDLL->getInterfaceIFace()->lookAt(plot()->getPoint(), CAMERALOOKAT_NORMAL);
	}

	GET_PLAYER(getOwnerINLINE()).found(getX_INLINE(), getY_INLINE());

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_FOUND);
	}

	kill(true);

	return true;
}


bool CvUnit::canSpread(const CvPlot* pPlot, ReligionTypes eReligion, bool bTestVisible) const
{
	CvCity* pCity;

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       08/19/09                                jdog5000      */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
/* orginal bts code
	if (GC.getUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK())
	{
		CyArgsList argsList;
		argsList.add(getOwnerINLINE());
		argsList.add(getID());
		argsList.add((int) eReligion);
		argsList.add(pPlot->getX());
		argsList.add(pPlot->getY());
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotSpreadReligion", argsList.makeFunctionArgs(), &lResult);
		if (lResult > 0)
		{
			return false;
		}
	}
*/
				// UP efficiency: Moved below faster calls
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

	if (eReligion == NO_RELIGION)
	{
		return false;
	}

	if (m_pUnitInfo->getReligionSpreads(eReligion) <= 0)
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->isHasReligion(eReligion))
	{
		return false;
	}

	if (!canEnterArea(pPlot->getTeam(), pPlot->area()))
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (pCity->getTeam() != getTeam())
		{
			if (GET_PLAYER(pCity->getOwnerINLINE()).isNoNonStateReligionSpread())
			{
				if (eReligion != GET_PLAYER(pCity->getOwnerINLINE()).getStateReligion())
				{
					return false;
				}
			}
		}
	}

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       08/19/09                                jdog5000      */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
	if (GC.getUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK())
	{
		CyArgsList argsList;
		argsList.add(getOwnerINLINE());
		argsList.add(getID());
		argsList.add((int) eReligion);
		argsList.add(pPlot->getX());
		argsList.add(pPlot->getY());
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotSpreadReligion", argsList.makeFunctionArgs(), &lResult);
		if (lResult > 0)
		{
			return false;
		}
	}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

	return true;
}


bool CvUnit::spread(ReligionTypes eReligion)
{
	CvCity* pCity;
	CvWString szBuffer;
	int iSpreadProb;

	if (!canSpread(plot(), eReligion))
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		iSpreadProb = m_pUnitInfo->getReligionSpreads(eReligion);

		if (pCity->getTeam() != getTeam())
		{
			iSpreadProb /= 2;
		}

		bool bSuccess;

		iSpreadProb += (((GC.getNumReligionInfos() - pCity->getReligionCount()) * (100 - iSpreadProb)) / GC.getNumReligionInfos());

		if (GC.getGameINLINE().getSorenRandNum(100, "Unit Spread Religion") < iSpreadProb)
		{
			pCity->setHasReligion(eReligion, true, true, false);
			bSuccess = true;
		}
		else
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_RELIGION_FAILED_TO_SPREAD", getNameKey(), GC.getReligionInfo(eReligion).getChar(), pCity->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_NOSPREAD", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE());
			bSuccess = false;
		}

		// Python Event
		CvEventReporter::getInstance().unitSpreadReligionAttempt(this, eReligion, bSuccess);
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_SPREAD);
	}

	kill(true);

	return true;
}


bool CvUnit::canSpreadCorporation(const CvPlot* pPlot, CorporationTypes eCorporation, bool bTestVisible) const
{
	if (NO_CORPORATION == eCorporation)
	{
		return false;
	}

	if (!GET_PLAYER(getOwnerINLINE()).isActiveCorporation(eCorporation))
	{
		return false;
	}

	if (m_pUnitInfo->getCorporationSpreads(eCorporation) <= 0)
	{
		return false;
	}

	CvCity* pCity = pPlot->getPlotCity();

	if (NULL == pCity)
	{
		return false;
	}

	if (pCity->isHasCorporation(eCorporation))
	{
		return false;
	}
/************************************************************************************************/
/* Afforess	                  Start		 01/17/10                                               */
/*                                                                                              */
/*   Blocks obsolete Corps from spreading                                                       */
/************************************************************************************************/
	if (GC.getCorporationInfo(eCorporation).getObsoleteTech() != NO_TECH)
	{
		if (GET_TEAM(GET_PLAYER(pCity->getOwnerINLINE()).getTeam()).isHasTech((TechTypes)GC.getCorporationInfo(eCorporation).getObsoleteTech()))
		{
			return false;
		}
	}
	if (GC.getGame().isOption(GAMEOPTION_REALISTIC_CORPORATIONS))
	{
		if (!GC.getGame().isModderGameOption(MODDERGAMEOPTION_NO_AUTO_CORPORATION_FOUNDING))
		{
			return false;
		}
	}
	if (!GC.getGameINLINE().canEverSpread(eCorporation))
	{
		return false;
	}
	if (!bTestVisible)
	{
		for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
		{
			if (GC.getCorporationInfo(eCorporation).getPrereqBuildingClass(iI) > 0)
			{
				if (GET_PLAYER(pCity->getOwnerINLINE()).getBuildingClassCount((BuildingClassTypes)iI) < GC.getCorporationInfo(eCorporation).getPrereqBuildingClass(iI))
				{
					return false;
				}
			}
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	if (!canEnterArea(pPlot->getTeam(), pPlot->area()))
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (!GET_PLAYER(pCity->getOwnerINLINE()).isActiveCorporation(eCorporation))
		{
			return false;
		}

		for (int iCorporation = 0; iCorporation < GC.getNumCorporationInfos(); ++iCorporation)
		{
			if (pCity->isHeadquarters((CorporationTypes)iCorporation))
			{
				if (GC.getGameINLINE().isCompetingCorporation((CorporationTypes)iCorporation, eCorporation))
				{
					return false;
				}
			}
		}
/************************************************************************************************/
/* Afforess	                  Start		 02/17/10                                               */
/*                                                                                              */
/* Some corporations don't require any resources...                                             */
/************************************************************************************************/
		bool bValid = false;
		bool bRequiresBonus = false;
		for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
		{
			BonusTypes eBonus = (BonusTypes)GC.getCorporationInfo(eCorporation).getPrereqBonus(i);
			if (NO_BONUS != eBonus)
			{
				bRequiresBonus = true;
				if (pCity->hasBonus(eBonus))
				{
					bValid = true;
					break;
				}
			}
		}
		if (!bValid && bRequiresBonus)
		{
			return false;
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

		if (GET_PLAYER(getOwnerINLINE()).getGold() < spreadCorporationCost(eCorporation, pCity))
		{
			return false;
		}
	}

	return true;
}

int CvUnit::spreadCorporationCost(CorporationTypes eCorporation, CvCity* pCity) const
{
	int iCost = std::max(0, GC.getCorporationInfo(eCorporation).getSpreadCost() * (100 + GET_PLAYER(getOwnerINLINE()).calculateInflationRate()));
	iCost /= 100;

	if (NULL != pCity)
	{
		if (getTeam() != pCity->getTeam() && !GET_TEAM(pCity->getTeam()).isVassal(getTeam()))
		{
			iCost *= GC.getDefineINT("CORPORATION_FOREIGN_SPREAD_COST_PERCENT");
			iCost /= 100;
		}

		for (int iCorp = 0; iCorp < GC.getNumCorporationInfos(); ++iCorp)
		{
			if (iCorp != eCorporation)
			{
				if (pCity->isActiveCorporation((CorporationTypes)iCorp))
				{
					if (GC.getGameINLINE().isCompetingCorporation(eCorporation, (CorporationTypes)iCorp))
					{
						iCost *= 100 + GC.getCorporationInfo((CorporationTypes)iCorp).getSpreadFactor();
						iCost /= 100;
					}
				}
			}
		}
	}

	return iCost;
}

bool CvUnit::spreadCorporation(CorporationTypes eCorporation)
{
	int iSpreadProb;

	if (!canSpreadCorporation(plot(), eCorporation))
	{
		return false;
	}

	CvCity* pCity = plot()->getPlotCity();

	if (NULL != pCity)
	{
		GET_PLAYER(getOwnerINLINE()).changeGold(-spreadCorporationCost(eCorporation, pCity));

		iSpreadProb = m_pUnitInfo->getCorporationSpreads(eCorporation);

		if (pCity->getTeam() != getTeam())
		{
			iSpreadProb /= 2;
		}

		iSpreadProb += (((GC.getNumCorporationInfos() - pCity->getCorporationCount()) * (100 - iSpreadProb)) / GC.getNumCorporationInfos());

		if (GC.getGameINLINE().getSorenRandNum(100, "Unit Spread Corporation") < iSpreadProb)
		{
			pCity->setHasCorporation(eCorporation, true, true, false);
		}
		else
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_CORPORATION_FAILED_TO_SPREAD", getNameKey(), GC.getCorporationInfo(eCorporation).getChar(), pCity->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_NOSPREAD", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE());
		}
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_SPREAD_CORPORATION);
	}

	kill(true);

	return true;
}


bool CvUnit::canJoin(const CvPlot* pPlot, SpecialistTypes eSpecialist) const
{
	CvCity* pCity;
/************************************************************************************************/
/* Afforess	                  Start		 06/05/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (isCommander())
	{
		return false;
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	if (eSpecialist == NO_SPECIALIST)
	{
		return false;
	}

	if (!(m_pUnitInfo->getGreatPeoples(eSpecialist)))
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (!(pCity->canJoin()))
	{
		return false;
	}

	if (pCity->getTeam() != getTeam())
	{
		return false;
	}

	if (isDelayedDeath())
	{
		return false;
	}

	return true;
}


bool CvUnit::join(SpecialistTypes eSpecialist)
{
	CvCity* pCity;

	if (!canJoin(plot(), eSpecialist))
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		pCity->changeFreeSpecialistCount(eSpecialist, 1);
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_JOIN);
	}

	kill(true);

	return true;
}


bool CvUnit::canConstruct(const CvPlot* pPlot, BuildingTypes eBuilding, bool bTestVisible) const
{
	CvCity* pCity;

	if (eBuilding == NO_BUILDING)
	{
		return false;
	}
/************************************************************************************************/
/* Afforess	                  Start		 06/05/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (isCommander())
	{
		return false;
	}
	if (GC.getBuildingInfo(eBuilding).getGlobalReligionCommerce() > NO_RELIGION)
	{
		if (GC.getBuildingInfo(eBuilding).getProductionCost() == -1)
		{
			if (GC.getGameINLINE().getBuildingClassCreatedCount((BuildingClassTypes)GC.getBuildingInfo(eBuilding).getBuildingClassType()) > 0)
			{
				return false;
			}
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (getTeam() != pCity->getTeam())
	{
		return false;
	}

	if (pCity->getNumRealBuilding(eBuilding) > 0)
	{
		return false;
	}

	if (!(m_pUnitInfo->getForceBuildings(eBuilding)))
	{
		if (!(m_pUnitInfo->getBuildings(eBuilding)))
		{
			return false;
		}

		if (!(pCity->canConstruct(eBuilding, false, bTestVisible, true)))
		{
			return false;
		}
	}

	if (isDelayedDeath())
	{
		return false;
	}

	return true;
}


bool CvUnit::construct(BuildingTypes eBuilding)
{
	CvCity* pCity;

	if (!canConstruct(plot(), eBuilding))
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		pCity->setNumRealBuilding(eBuilding, pCity->getNumRealBuilding(eBuilding) + 1);

		CvEventReporter::getInstance().buildingBuilt(pCity, eBuilding);
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_CONSTRUCT);
	}

	kill(true);

	return true;
}


TechTypes CvUnit::getDiscoveryTech() const
{
	return ::getDiscoveryTech(getUnitType(), getOwnerINLINE());
}


int CvUnit::getDiscoverResearch(TechTypes eTech) const
{
	int iResearch;

	iResearch = (m_pUnitInfo->getBaseDiscover() + (m_pUnitInfo->getDiscoverMultiplier() * GET_TEAM(getTeam()).getTotalPopulation()));

	iResearch *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitDiscoverPercent();
	iResearch /= 100;

    if (eTech != NO_TECH)
    {
        iResearch = std::min(GET_TEAM(getTeam()).getResearchLeft(eTech), iResearch);
    }

	return std::max(0, iResearch);
}


bool CvUnit::canDiscover(const CvPlot* pPlot) const
{
	TechTypes eTech;

	eTech = getDiscoveryTech();

	if (eTech == NO_TECH)
	{
		return false;
	}

	if (getDiscoverResearch(eTech) == 0)
	{
		return false;
	}

	if (isDelayedDeath())
	{
		return false;
	}

	return true;
}


bool CvUnit::discover()
{
	TechTypes eDiscoveryTech;

	if (!canDiscover(plot()))
	{
		return false;
	}

	eDiscoveryTech = getDiscoveryTech();
	FAssertMsg(eDiscoveryTech != NO_TECH, "DiscoveryTech is not assigned a valid value");

	GET_TEAM(getTeam()).changeResearchProgress(eDiscoveryTech, getDiscoverResearch(eDiscoveryTech), getOwnerINLINE());

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_DISCOVER);
	}

	kill(true);

	return true;
}


int CvUnit::getMaxHurryProduction(CvCity* pCity) const
{
	int iProduction;

	iProduction = (m_pUnitInfo->getBaseHurry() + (m_pUnitInfo->getHurryMultiplier() * pCity->getPopulation()));

	iProduction *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitHurryPercent();
	iProduction /= 100;

	return std::max(0, iProduction);
}


int CvUnit::getHurryProduction(const CvPlot* pPlot) const
{
	CvCity* pCity;
	int iProduction;

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return 0;
	}

	iProduction = getMaxHurryProduction(pCity);

	iProduction = std::min(pCity->productionLeft(), iProduction);

	return std::max(0, iProduction);
}


bool CvUnit::canHurry(const CvPlot* pPlot, bool bTestVisible) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	CvCity* pCity;

	if (getHurryProduction(pPlot) == 0)
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getProductionTurnsLeft() == 1)
	{
		return false;
	}

/************************************************************************************************/
/* Afforess	                  Start		 04/23/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (getTeam() != pCity->getTeam())
	{
		return false;
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	if (!bTestVisible)
	{
		if (!(pCity->isProductionBuilding()))
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::hurry()
{
	CvCity* pCity;

	if (!canHurry(plot()))
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		pCity->changeProduction(getHurryProduction(plot()));
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_HURRY);
	}

	kill(true);

	return true;
}


int CvUnit::getTradeGold(const CvPlot* pPlot) const
{
	CvCity* pCapitalCity;
	CvCity* pCity;
	int iGold;

	pCity = pPlot->getPlotCity();
	pCapitalCity = GET_PLAYER(getOwnerINLINE()).getCapitalCity();

	if (pCity == NULL)
	{
		return 0;
	}

	iGold = (m_pUnitInfo->getBaseTrade() + (m_pUnitInfo->getTradeMultiplier() * ((pCapitalCity != NULL) ? pCity->calculateTradeProfit(pCapitalCity) : 0)));

	iGold *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitTradePercent();
	iGold /= 100;
	
/************************************************************************************************/
/* Afforess	                  Start		 03/9/10                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	iGold *= (GET_TEAM(getTeam()).getTradeMissionModifier() + 100);
	iGold /= 100;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/	

	return std::max(0, iGold);
}


bool CvUnit::canTrade(const CvPlot* pPlot, bool bTestVisible) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	CvCity* pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (getTradeGold(pPlot) == 0)
	{
		return false;
	}

	if (!canEnterArea(pPlot->getTeam(), pPlot->area()))
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (pCity->getTeam() == getTeam())
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::trade()
{
	if (!canTrade(plot()))
	{
		return false;
	}

	GET_PLAYER(getOwnerINLINE()).changeGold(getTradeGold(plot()));

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_TRADE);
	}

	kill(true);

	return true;
}


int CvUnit::getGreatWorkCulture(const CvPlot* pPlot) const
{
	int iCulture;

	iCulture = m_pUnitInfo->getGreatWorkCulture();

	iCulture *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitGreatWorkPercent();
	iCulture /= 100;

	return std::max(0, iCulture);
}


bool CvUnit::canGreatWork(const CvPlot* pPlot) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	CvCity* pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getOwnerINLINE() != getOwnerINLINE())
	{
		return false;
	}

	if (getGreatWorkCulture(pPlot) == 0)
	{
		return false;
	}

	return true;
}


bool CvUnit::greatWork()
{
	if (!canGreatWork(plot()))
	{
		return false;
	}

	CvCity* pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		pCity->setCultureUpdateTimer(0);
		pCity->setOccupationTimer(0);

		int iCultureToAdd = 100 * getGreatWorkCulture(plot());
		int iNumTurnsApplied = (GC.getDefineINT("GREAT_WORKS_CULTURE_TURNS") * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitGreatWorkPercent()) / 100;

		for (int i = 0; i < iNumTurnsApplied; ++i)
		{
			pCity->changeCultureTimes100(getOwnerINLINE(), iCultureToAdd / iNumTurnsApplied, true, true);
		}

		if (iNumTurnsApplied > 0)
		{
			pCity->changeCultureTimes100(getOwnerINLINE(), iCultureToAdd % iNumTurnsApplied, false, true);
		}
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_GREAT_WORK);
	}

	kill(true);

	return true;
}


int CvUnit::getEspionagePoints(const CvPlot* pPlot) const
{
	int iEspionagePoints;

	iEspionagePoints = m_pUnitInfo->getEspionagePoints();

	iEspionagePoints *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitGreatWorkPercent();
	iEspionagePoints /= 100;

	return std::max(0, iEspionagePoints);
}

bool CvUnit::canInfiltrate(const CvPlot* pPlot, bool bTestVisible) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	if (GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
	{
		return false;
	}

	if (getEspionagePoints(NULL) == 0)
	{
		return false;
	}

	CvCity* pCity = pPlot->getPlotCity();
	if (pCity == NULL || pCity->isBarbarian())
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (NULL != pCity && pCity->getTeam() == getTeam())
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::infiltrate()
{
	if (!canInfiltrate(plot()))
	{
		return false;
	}

	int iPoints = getEspionagePoints(NULL);
	GET_TEAM(getTeam()).changeEspionagePointsAgainstTeam(GET_PLAYER(plot()->getOwnerINLINE()).getTeam(), iPoints);
	GET_TEAM(getTeam()).changeEspionagePointsEver(iPoints);

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_INFILTRATE);
	}

	kill(true);

	return true;
}


bool CvUnit::canEspionage(const CvPlot* pPlot, bool bTestVisible) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	if (!isSpy())
	{
		return false;
	}

	if (GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
	{
		return false;
	}

	PlayerTypes ePlotOwner = pPlot->getOwnerINLINE();
	if (NO_PLAYER == ePlotOwner)
	{
		return false;
	}

	CvPlayer& kTarget = GET_PLAYER(ePlotOwner);

	if (kTarget.isBarbarian())
	{
		return false;
	}

	if (kTarget.getTeam() == getTeam())
	{
		return false;
	}

	if (GET_TEAM(getTeam()).isVassal(kTarget.getTeam()))
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (isMadeAttack())
		{
			return false;
		}

		if (hasMoved())
		{
			return false;
		}

		if (kTarget.getTeam() != getTeam() && !isInvisible(kTarget.getTeam(), false))
		{
			return false;
		}
	}

	return true;
}

//TSHEEP start
bool CvUnit::awardSpyExperience(TeamTypes eTargetTeam, int iModifier)
{
	if (GC.getDefineINT("SS_ENABLED"))
	{
		int iDifficulty = (getSpyInterceptPercent(eTargetTeam) * (100 + iModifier))/100;
		if (iDifficulty < 1)
			changeExperience(1);
		else if (iDifficulty < 10)
			changeExperience(2);
		else if (iDifficulty < 25)
			changeExperience(3);
		else if (iDifficulty < 50)
			changeExperience(4);
		else if (iDifficulty < 75)
			changeExperience(5);
		else if (iDifficulty >= 75)
			changeExperience(6);
		testPromotionReady();
		return true;
	} else return false;
}
//TSHEEP End

/************************************************************************************************/
/* RevolutionDCM                               04/19/09                           Glider1       */
/************************************************************************************************/
//RevolutionDCM - Super Spies
bool CvUnit::canAssassin(const CvPlot* pPlot, bool bTestVisible) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	if (!isSpy())
	{
		return false;
	}

	CvCity* pCity = pPlot->getPlotCity();
	if (NULL == pCity)
	{
		return false;
	}
	
	int numGreatPeople = pCity->getNumGreatPeople();
	if (numGreatPeople <= 0)
	{
		return false;
	} 

	CvPlayer& kTarget = GET_PLAYER(pCity->getOwnerINLINE());

	if (kTarget.getTeam() == getTeam())
	{
		return false;
	}

	if (kTarget.isBarbarian())
	{
		return false;
	}

	if (GET_TEAM(getTeam()).isVassal(kTarget.getTeam()))
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (isMadeAttack())
		{
			return false;
		}

		if (hasMoved())
		{
			return false;
		}

		if (kTarget.getTeam() != getTeam() && !isInvisible(kTarget.getTeam(), false))
		{
			return false;
		}
	}

	return true;
}

bool CvUnit::canBribe(const CvPlot* pPlot, bool bTestVisible) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	if (!isSpy())
	{
		return false;
	}

	if(pPlot->plotCount(PUF_isOtherTeam, getOwnerINLINE(), -1, NO_PLAYER, NO_TEAM, PUF_isVisible, getOwnerINLINE()) < 1)
	{
		return false;
	}	

	if (pPlot->plotCount(PUF_isUnitAIType, UNITAI_WORKER, -1) < 1)
	{
		return false;
	}

	CvUnit* pTargetUnit;
	pTargetUnit = pPlot->plotCheck(PUF_isOtherTeam, getOwnerINLINE(), -1, NO_PLAYER, NO_TEAM, PUF_isVisible, getOwnerINLINE());
	CvPlayer& kTarget = GET_PLAYER(pTargetUnit->getOwnerINLINE());

	if (kTarget.getTeam() == getTeam())
	{
		return false;
	}

	if (kTarget.isBarbarian())
	{
		return false;
	}

	if (GET_TEAM(getTeam()).isVassal(kTarget.getTeam()))
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (isMadeAttack())
		{
			return false;
		}

		if (hasMoved())
		{
			return false;
		}

		if (kTarget.getTeam() != getTeam() && !isInvisible(kTarget.getTeam(), false))
		{
			return false;
		}
	}

	return true;
}
// RevolutionDCM end
/************************************************************************************************/
/* RevolutionDCM                               END                               Glider1       */
/************************************************************************************************/

bool CvUnit::espionage(EspionageMissionTypes eMission, int iData)
{
	if (!canEspionage(plot()))
	{
		return false;
	}

	PlayerTypes eTargetPlayer = plot()->getOwnerINLINE();

	if (NO_ESPIONAGEMISSION == eMission)
	{
		FAssert(GET_PLAYER(getOwnerINLINE()).isHuman());
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DOESPIONAGE);
		if (NULL != pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo, getOwnerINLINE(), true);
		}
	}
	else if (GC.getEspionageMissionInfo(eMission).isTwoPhases() && -1 == iData)
	{
		FAssert(GET_PLAYER(getOwnerINLINE()).isHuman());
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DOESPIONAGE_TARGET);
		if (NULL != pInfo)
		{
			pInfo->setData1(eMission);
			gDLL->getInterfaceIFace()->addPopup(pInfo, getOwnerINLINE(), true);
		}
	}
	else
	{
		if (testSpyIntercepted(eTargetPlayer, GC.getEspionageMissionInfo(eMission).getDifficultyMod()))
		{
			return false;
		}

/************************************************************************************************/
/* Afforess	                  Start		 01/31/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		bool bCaught = testSpyIntercepted(eTargetPlayer, GC.getDefineINT("ESPIONAGE_SPY_MISSION_ESCAPE_MOD"));

		if (GET_PLAYER(getOwnerINLINE()).doEspionageMission(eMission, eTargetPlayer, plot(), iData, this, (bCaught && !isAlwaysHeal())))
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		{
			if (plot()->isActiveVisible(false))
			{
				NotifyEntity(MISSION_ESPIONAGE);
			}

			if (!bCaught)
			{
				setFortifyTurns(0);
				setMadeAttack(true);
				finishMoves();

				CvCity* pCapital = GET_PLAYER(getOwnerINLINE()).getCapitalCity();
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/*Spy actions that aren't in a city don't cause the spy to be sent back                         */
/************************************************************************************************/
				if (GC.getGameINLINE().isOption(GAMEOPTION_ADVANCED_ESPIONAGE))
				{
					if (!plot()->isCity())
					{
						pCapital = NULL;
					}
				}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				if (NULL != pCapital)
				{
					setXY(pCapital->getX_INLINE(), pCapital->getY_INLINE(), false, false, false);

					CvWString szBuffer = gDLL->getText("TXT_KEY_ESPIONAGE_SPY_SUCCESS", getNameKey(), pCapital->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_POSITIVE_DINK", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pCapital->getX_INLINE(), pCapital->getY_INLINE(), true, true);
				}
				//TSHEEP Give spies xp for successful missions
				awardSpyExperience(GET_PLAYER(eTargetPlayer).getTeam(),GC.getEspionageMissionInfo(eMission).getDifficultyMod());
				//TSHEEP end
			}

			return true;
		}
	}

	return false;
}

bool CvUnit::testSpyIntercepted(PlayerTypes eTargetPlayer, int iModifier)
{
	CvPlayer& kTargetPlayer = GET_PLAYER(eTargetPlayer);

	if (kTargetPlayer.isBarbarian())
	{
		return false;
	}

	if (GC.getGameINLINE().getSorenRandNum(10000, "Spy Interception") >= getSpyInterceptPercent(kTargetPlayer.getTeam()) * (100 + iModifier))
	{
		return false;
	}

	CvString szFormatNoReveal;
	CvString szFormatReveal;

	if (GET_TEAM(kTargetPlayer.getTeam()).getCounterespionageModAgainstTeam(getTeam()) > 0)
	{
		szFormatNoReveal = "TXT_KEY_SPY_INTERCEPTED_MISSION";
		szFormatReveal = "TXT_KEY_SPY_INTERCEPTED_MISSION_REVEAL";
	}
	else if (plot()->isEspionageCounterSpy(kTargetPlayer.getTeam()))
	{
		szFormatNoReveal = "TXT_KEY_SPY_INTERCEPTED_SPY";
		szFormatReveal = "TXT_KEY_SPY_INTERCEPTED_SPY_REVEAL";
	}
	else
	{
		szFormatNoReveal = "TXT_KEY_SPY_INTERCEPTED";
		szFormatReveal = "TXT_KEY_SPY_INTERCEPTED_REVEAL";
	}

	CvWString szCityName = kTargetPlayer.getCivilizationShortDescription();
	CvCity* pClosestCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), eTargetPlayer, kTargetPlayer.getTeam(), true, false);
	if (pClosestCity != NULL)
	{
		szCityName = pClosestCity->getName();
	}

	CvWString szBuffer = gDLL->getText(szFormatReveal.GetCString(), GET_PLAYER(getOwnerINLINE()).getCivilizationAdjectiveKey(), getNameKey(), kTargetPlayer.getCivilizationAdjectiveKey(), szCityName.GetCString());
	gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);

	//TSHEEP Enable Loyalty Promotion
	//if (GC.getGameINLINE().getSorenRandNum(100, "Spy Reveal identity") < GC.getDefineINT("ESPIONAGE_SPY_REVEAL_IDENTITY_PERCENT"))
	if (GC.getGameINLINE().getSorenRandNum(100, "Spy Reveal identity") < GC.getDefineINT("ESPIONAGE_SPY_REVEAL_IDENTITY_PERCENT") && !isAlwaysHeal())//TSHEEP End
	{
		if (!isEnemy(kTargetPlayer.getTeam()))
		{
			GET_PLAYER(eTargetPlayer).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_SPY_CAUGHT, 1);
		}

		gDLL->getInterfaceIFace()->addMessage(eTargetPlayer, true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
	}
	else
	{
		szBuffer = gDLL->getText(szFormatNoReveal.GetCString(), getNameKey(), kTargetPlayer.getCivilizationAdjectiveKey(), szCityName.GetCString());
		gDLL->getInterfaceIFace()->addMessage(eTargetPlayer, true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_SURRENDER);
	}

	//TSHEEP - Give xp to spy who catches spy
	CvUnit* pCounterUnit;

	pCounterUnit = plot()->plotCheck(PUF_isCounterSpy, -1, -1, NO_PLAYER, kTargetPlayer.getTeam());

	if(NULL != pCounterUnit)
	{
		pCounterUnit->changeExperience(1);
		//RevolutionDCM - just ensure that promotion readiness is tested
		pCounterUnit->testPromotionReady();
	}
	//TSHEEP End

	//TSHEEP Implement Escape Promotion
	if(GC.getGameINLINE().getSorenRandNum(100, "Spy Reveal identity") < withdrawalProbability())
	{
		setFortifyTurns(0);
		setMadeAttack(true);
		finishMoves();

		CvCity* pCapital = GET_PLAYER(getOwnerINLINE()).getCapitalCity();
		if (NULL != pCapital)
		{
			setXY(pCapital->getX_INLINE(), pCapital->getY_INLINE(), false, false, false);
		}
		szFormatReveal = "TXT_KEY_SPY_ESCAPED_REVEAL";
		szFormatNoReveal = "TXT_KEY_SPY_ESCAPED";
		szBuffer = gDLL->getText(szFormatReveal.GetCString(), GET_PLAYER(getOwnerINLINE()).getCivilizationAdjectiveKey(), getNameKey(), kTargetPlayer.getCivilizationAdjectiveKey(), szCityName.GetCString());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
		szBuffer = gDLL->getText(szFormatNoReveal.GetCString(), getNameKey(), kTargetPlayer.getCivilizationAdjectiveKey(), szCityName.GetCString());
		gDLL->getInterfaceIFace()->addMessage(eTargetPlayer, true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);

		changeExperience(1);
		testPromotionReady();

		return true;
	}
	//TSHEEP End

	kill(true);

	return true;
}

int CvUnit::getSpyInterceptPercent(TeamTypes eTargetTeam) const
{
	FAssert(isSpy());
	FAssert(getTeam() != eTargetTeam);

	int iSuccess = 0;

	int iTargetPoints = GET_TEAM(eTargetTeam).getEspionagePointsEver();
	int iOurPoints = GET_TEAM(getTeam()).getEspionagePointsEver();
	iSuccess += (GC.getDefineINT("ESPIONAGE_INTERCEPT_SPENDING_MAX") * iTargetPoints) / std::max(1, iTargetPoints + iOurPoints);

	//TSHEEP - add evasion attribute to spy chances
	if(getExtraEvasion())
	{
		iSuccess -= getExtraEvasion();
	}
	//TSHEEP end

	if (plot()->isEspionageCounterSpy(eTargetTeam))
	{
		iSuccess += GC.getDefineINT("ESPIONAGE_INTERCEPT_COUNTERSPY");
		//TSHEEP - Add intercept attribute of any enemy spies present to chances
		if(plot()->plotCheck(PUF_isCounterSpy, -1, -1, NO_PLAYER, eTargetTeam))
		{
			CvUnit* pCounterUnit = plot()->plotCheck(PUF_isCounterSpy, -1, -1, NO_PLAYER, eTargetTeam);
			if(pCounterUnit->getExtraIntercept())
				iSuccess += pCounterUnit->getExtraIntercept();
		}
		//TSHEEP end
	}

	if (GET_TEAM(eTargetTeam).getCounterespionageModAgainstTeam(getTeam()) > 0)
	{
		iSuccess += GC.getDefineINT("ESPIONAGE_INTERCEPT_COUNTERESPIONAGE_MISSION");
	}

	//TSHEEP - This check was always returning true since there is always at least one friendly spy in the tile
	//if (0 == getFortifyTurns() || plot()->plotCount(PUF_isSpy, -1, -1, NO_PLAYER, getTeam()) > 0)
	if (0 == getFortifyTurns() || plot()->plotCount(PUF_isSpy, -1, -1, NO_PLAYER, getTeam()) > 1)//TSHEEP - End
	{
		iSuccess += GC.getDefineINT("ESPIONAGE_INTERCEPT_RECENT_MISSION");
	}

	return std::min(100, std::max(0, iSuccess));
}

bool CvUnit::isIntruding() const
{
	TeamTypes eLocalTeam = plot()->getTeam();
	
	if (NO_TEAM == eLocalTeam || eLocalTeam == getTeam())
	{
		return false;
	}

	// UNOFFICIAL_PATCH Start
	// * Vassal's spies no longer caught in master's territory
	//if (GET_TEAM(eLocalTeam).isVassal(getTeam()))
	if (GET_TEAM(eLocalTeam).isVassal(getTeam()) || GET_TEAM(getTeam()).isVassal(eLocalTeam))
	// UNOFFICIAL_PATCH End
	{
		return false;
	}

	return true;
}

bool CvUnit::canGoldenAge(const CvPlot* pPlot, bool bTestVisible) const
{
	if (!isGoldenAge())
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (GET_PLAYER(getOwnerINLINE()).unitsRequiredForGoldenAge() > GET_PLAYER(getOwnerINLINE()).unitsGoldenAgeReady())
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::goldenAge()
{
	if (!canGoldenAge(plot()))
	{
		return false;
	}

	GET_PLAYER(getOwnerINLINE()).killGoldenAgeUnits(this);

	GET_PLAYER(getOwnerINLINE()).changeGoldenAgeTurns(GET_PLAYER(getOwnerINLINE()).getGoldenAgeLength());
	GET_PLAYER(getOwnerINLINE()).changeNumUnitGoldenAges(1);

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_GOLDEN_AGE);
	}

	kill(true);

	return true;
}


bool CvUnit::canBuild(const CvPlot* pPlot, BuildTypes eBuild, bool bTestVisible) const
{
    FAssertMsg(eBuild < GC.getNumBuildInfos(), "Index out of bounds");
	if (!(m_pUnitInfo->getBuilds(eBuild)))
	{
		return false;
	}
	
/************************************************************************************************/
/* Afforess	                  Start		 07/12/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (getGroup()->isAutomated())
	{
		if (!GET_PLAYER(getOwnerINLINE()).isAutomatedCanBuild(eBuild))
		{
			return false;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	if (!(GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild, false, bTestVisible)))
	{
		return false;
	}

	if (!pPlot->isValidDomainForAction(*this))
	{
		return false;
	}
	
	return true;
}

// Returns true if build finished...
bool CvUnit::build(BuildTypes eBuild)
{
	bool bFinished;

	FAssertMsg(eBuild < GC.getNumBuildInfos(), "Invalid Build");

	if (!canBuild(plot(), eBuild))
	{
		return false;
	}

	// Note: notify entity must come before changeBuildProgress - because once the unit is done building,
	// that function will notify the entity to stop building.
	NotifyEntity((MissionTypes)GC.getBuildInfo(eBuild).getMissionType());

	GET_PLAYER(getOwnerINLINE()).changeGold(-(GET_PLAYER(getOwnerINLINE()).getBuildCost(plot(), eBuild)));
/************************************************************************************************/
/* Afforess	                  Start		 02/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	bFinished = plot()->changeBuildProgress(eBuild, workRate(false), getOwnerINLINE());
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	finishMoves(); // needs to be at bottom because movesLeft() can affect workRate()...

	if (bFinished)
	{
		if (GC.getBuildInfo(eBuild).isKill())
		{
			kill(true);
		}
	}

	// Python Event
	CvEventReporter::getInstance().unitBuildImprovement(this, eBuild, bFinished);

	return bFinished;
}


bool CvUnit::canPromote(PromotionTypes ePromotion, int iLeaderUnitId) const
{
	if (iLeaderUnitId >= 0)
	{
		if (iLeaderUnitId == getID())
		{
			return false;
		}

		// The command is always possible if it's coming from a Warlord unit that gives just experience points
		CvUnit* pWarlord = GET_PLAYER(getOwnerINLINE()).getUnit(iLeaderUnitId);
		if (pWarlord && 
			NO_UNIT != pWarlord->getUnitType() && 
			pWarlord->getUnitInfo().getLeaderExperience() > 0 && 
			NO_PROMOTION == pWarlord->getUnitInfo().getLeaderPromotion() &&
			canAcquirePromotionAny())
		{
			return true;
		}
	}

	if (ePromotion == NO_PROMOTION)
	{
		return false;
	}

	if (!canAcquirePromotion(ePromotion))
	{
		return false;
	}

	if (GC.getPromotionInfo(ePromotion).isLeader())
	{
		if (iLeaderUnitId >= 0)
		{
			CvUnit* pWarlord = GET_PLAYER(getOwnerINLINE()).getUnit(iLeaderUnitId);
			if (pWarlord && NO_UNIT != pWarlord->getUnitType())
			{
				return (pWarlord->getUnitInfo().getLeaderPromotion() == ePromotion);
			}
		}
		return false;
	}
	else
	{
		if (!isPromotionReady())
		{
			return false;
		}
	}

	return true;
}

void CvUnit::promote(PromotionTypes ePromotion, int iLeaderUnitId)
{
	if (!canPromote(ePromotion, iLeaderUnitId))
	{
		return;
	}

	if (iLeaderUnitId >= 0)
	{
		CvUnit* pWarlord = GET_PLAYER(getOwnerINLINE()).getUnit(iLeaderUnitId);
		if (pWarlord)
		{
			pWarlord->giveExperience();
			if (!pWarlord->getNameNoDesc().empty())
			{
				setName(pWarlord->getNameKey());
			}

			//update graphics models
			m_eLeaderUnitType = pWarlord->getUnitType();
			reloadEntity();
		}
	}

	if (!GC.getPromotionInfo(ePromotion).isLeader())
	{
		changeLevel(1);
		changeDamage(-(getDamage() / 2));
	}

	setHasPromotion(ePromotion, true);

	testPromotionReady();

	if (IsSelected())
	{
		gDLL->getInterfaceIFace()->playGeneralSound(GC.getPromotionInfo(ePromotion).getSound());

		gDLL->getInterfaceIFace()->setDirty(UnitInfo_DIRTY_BIT, true);

// BUG - Update Plot List - start
		gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
// BUG - Update Plot List - end
	}
	else
	{
		setInfoBarDirty(true);
	}

	CvEventReporter::getInstance().unitPromoted(this, ePromotion);
}

bool CvUnit::lead(int iUnitId)
{
	if (!canLead(plot(), iUnitId))
	{
		return false;
	}

	PromotionTypes eLeaderPromotion = (PromotionTypes)m_pUnitInfo->getLeaderPromotion();

	if (-1 == iUnitId)
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_LEADUNIT, eLeaderPromotion, getID());
		if (pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo, getOwnerINLINE(), true);
		}
		return false;
	}
	else
	{
		CvUnit* pUnit = GET_PLAYER(getOwnerINLINE()).getUnit(iUnitId);

		if (!pUnit || !pUnit->canPromote(eLeaderPromotion, getID()))			
		{
			return false;
		}

		pUnit->joinGroup(NULL, true, true);

		pUnit->promote(eLeaderPromotion, getID());

		if (plot()->isActiveVisible(false))
		{
			NotifyEntity(MISSION_LEAD);
		}

		kill(true);

		return true;
	}
}


int CvUnit::canLead(const CvPlot* pPlot, int iUnitId) const
{
	PROFILE_FUNC();

	if (isDelayedDeath())
	{
		return 0;
	}

	if (NO_UNIT == getUnitType())
	{
		return 0;
	}
/************************************************************************************************/
/* Afforess	                  Start		 05/21/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (isCommander())
	{
		return 0;
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	int iNumUnits = 0;
	CvUnitInfo& kUnitInfo = getUnitInfo();

	if (-1 == iUnitId)
	{
		CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
		while(pUnitNode != NULL)
		{
			CvUnit* pUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);

			if (pUnit && pUnit != this && pUnit->getOwnerINLINE() == getOwnerINLINE() && pUnit->canPromote((PromotionTypes)kUnitInfo.getLeaderPromotion(), getID()))
			{
				++iNumUnits;
			}
		}
	}
	else
	{
		CvUnit* pUnit = GET_PLAYER(getOwnerINLINE()).getUnit(iUnitId);
		if (pUnit && pUnit != this && pUnit->canPromote((PromotionTypes)kUnitInfo.getLeaderPromotion(), getID()))
		{
			iNumUnits = 1;
		}
	}
	return iNumUnits;
}


int CvUnit::canGiveExperience(const CvPlot* pPlot) const
{
	int iNumUnits = 0;

	if (NO_UNIT != getUnitType() && m_pUnitInfo->getLeaderExperience() > 0)
	{
		CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
		while(pUnitNode != NULL)
		{
			CvUnit* pUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);

			if (pUnit && pUnit != this && pUnit->getOwnerINLINE() == getOwnerINLINE() && pUnit->canAcquirePromotionAny())
			{
/************************************************************************************************/
/* Afforess	                  Start		 03/30/10                                               */
/*                                                                                              */
/* Great Commanders: Do Not give commanders free XP                                             */
/************************************************************************************************/
				if (pUnit->getUnitInfo().isGreatGeneral())
					continue;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
				++iNumUnits;
			}
		}
	}

	return iNumUnits;
}

bool CvUnit::giveExperience()
{
	CvPlot* pPlot = plot();

	if (pPlot)
	{
		int iNumUnits = canGiveExperience(pPlot);
		if (iNumUnits > 0)
		{
			int iTotalExperience = getStackExperienceToGive(iNumUnits);

			int iMinExperiencePerUnit = iTotalExperience / iNumUnits;
			int iRemainder = iTotalExperience % iNumUnits;

			CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
			int i = 0;
			while(pUnitNode != NULL)
			{
				CvUnit* pUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = pPlot->nextUnitNode(pUnitNode);

				if (pUnit && pUnit != this && pUnit->getOwnerINLINE() == getOwnerINLINE() && pUnit->canAcquirePromotionAny())
				{
/************************************************************************************************/
/* Afforess	                  Start		 03/30/10                                               */
/*                                                                                              */
/* Great Commanders: Do Not give commanders free XP                                             */
/************************************************************************************************/
				if (pUnit->isCommander())
					continue;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
					pUnit->changeExperience(i < iRemainder ? iMinExperiencePerUnit+1 : iMinExperiencePerUnit);
					pUnit->testPromotionReady();
				}

				i++;
			}

			return true;
		}
	}

	return false;
}

int CvUnit::getStackExperienceToGive(int iNumUnits) const
{
	return (m_pUnitInfo->getLeaderExperience() * (100 + std::min(50, (iNumUnits - 1) * GC.getDefineINT("WARLORD_EXTRA_EXPERIENCE_PER_UNIT_PERCENT")))) / 100;
}

int CvUnit::upgradePrice(UnitTypes eUnit) const
{
	int iPrice;

/************************************************************************************************/
/* Afforess	                  Start		 12/21/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if(GC.getUSE_UPGRADE_UNIT_PRICE_CALLBACK())
	{
		CyArgsList argsList;
		argsList.add(getOwnerINLINE());
		argsList.add(getID());
		argsList.add((int) eUnit);
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "getUpgradePriceOverride", argsList.makeFunctionArgs(), &lResult);
		if (lResult >= 0)
		{
			return lResult;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	if (isBarbarian())
	{
		return 0;
	}

	iPrice = GC.getDefineINT("BASE_UNIT_UPGRADE_COST");

	iPrice += (std::max(0, (GET_PLAYER(getOwnerINLINE()).getProductionNeeded(eUnit) - GET_PLAYER(getOwnerINLINE()).getProductionNeeded(getUnitType()))) * GC.getDefineINT("UNIT_UPGRADE_COST_PER_PRODUCTION"));

	if (!isHuman() && !isBarbarian())
	{
		iPrice *= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIUnitUpgradePercent();
		iPrice /= 100;

		iPrice *= std::max(0, ((GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIPerEraModifier() * GET_PLAYER(getOwnerINLINE()).getCurrentEra()) + 100));
		iPrice /= 100;
	}

/************************************************************************************************/
/* REVDCM                                 02/16/10                                phungus420    */
/*                                                                                              */
/* Leonardo's Workshop                                                                          */
/************************************************************************************************/
	iPrice = iPrice * (100 + GET_PLAYER(getOwnerINLINE()).getUnitUpgradePriceModifier()) / 100;
/************************************************************************************************/
/* REVDCM                                  END                                                  */
/************************************************************************************************/

	iPrice -= (iPrice * getUpgradeDiscount()) / 100;

	return iPrice;
}


bool CvUnit::upgradeAvailable(UnitTypes eFromUnit, UnitClassTypes eToUnitClass, int iCount) const
{
	UnitTypes eLoopUnit;
	int iI;
	int numUnitClassInfos = GC.getNumUnitClassInfos();

	if (iCount > numUnitClassInfos)
	{
		return false;
	}

	CvUnitInfo &fromUnitInfo = GC.getUnitInfo(eFromUnit);

	if (fromUnitInfo.getUpgradeUnitClass(eToUnitClass))
	{
		return true;
	}

	for (iI = 0; iI < numUnitClassInfos; iI++)
	{
		if (fromUnitInfo.getUpgradeUnitClass(iI))
		{
			eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

			if (eLoopUnit != NO_UNIT)
			{
				if (upgradeAvailable(eLoopUnit, eToUnitClass, (iCount + 1)))
				{
					return true;
				}
			}
		}
	}

	return false;
}


bool CvUnit::canUpgrade(UnitTypes eUnit, bool bTestVisible) const
{
	if (eUnit == NO_UNIT)
	{
		return false;
	}

	if(!isReadyForUpgrade())
	{
		return false;
	}
	
	if (!bTestVisible)
	{
		if (GET_PLAYER(getOwnerINLINE()).getGold() < upgradePrice(eUnit))
		{
			return false;
		}
	}
	
	if (hasUpgrade(eUnit))
	{
		return true;
	}

	return false;
}

bool CvUnit::isReadyForUpgrade() const
{

/************************************************************************************************/
/* REVDCM                                 02/16/10                                phungus420    */
/*                                                                                              */
/* RevTrait Effects                                                                             */
/************************************************************************************************/
	if ( !GET_PLAYER(getOwnerINLINE()).isUpgradeAnywhere())
	{
		if (!canMove())
		{
			return false;
		}
	}

	if ( !GET_PLAYER(getOwnerINLINE()).isUpgradeAnywhere())
	{
		if (plot()->getTeam() != getTeam())
		{
			return false;
		}
	}
/************************************************************************************************/
/* REVDCM                                  END                                                  */
/************************************************************************************************/

	return true;
}

// has upgrade is used to determine if an upgrade is possible,
// it specifically does not check whether the unit can move, whether the current plot is owned, enough gold
// those are checked in canUpgrade()
// does not search all cities, only checks the closest one
bool CvUnit::hasUpgrade(bool bSearch) const
{
	return (getUpgradeCity(bSearch) != NULL);
}

// has upgrade is used to determine if an upgrade is possible,
// it specifically does not check whether the unit can move, whether the current plot is owned, enough gold
// those are checked in canUpgrade()
// does not search all cities, only checks the closest one
bool CvUnit::hasUpgrade(UnitTypes eUnit, bool bSearch) const
{
	return (getUpgradeCity(eUnit, bSearch) != NULL);
}

// finds the 'best' city which has a valid upgrade for the unit,
// it specifically does not check whether the unit can move, or if the player has enough gold to upgrade
// those are checked in canUpgrade()
// if bSearch is true, it will check every city, if not, it will only check the closest valid city
// NULL result means the upgrade is not possible
CvCity* CvUnit::getUpgradeCity(bool bSearch) const
{
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	UnitAITypes eUnitAI = AI_getUnitAIType();
	CvArea* pArea = area();

	int iCurrentValue = kPlayer.AI_unitValue(getUnitType(), eUnitAI, pArea);

	int iBestSearchValue = MAX_INT;
	CvCity* pBestUpgradeCity = NULL;
	
	for (int iI = 0; iI < GC.getNumUnitInfos(); iI++)
	{
		int iNewValue = kPlayer.AI_unitValue(((UnitTypes)iI), eUnitAI, pArea);
		if (iNewValue > iCurrentValue)
		{
			int iSearchValue;
			CvCity* pUpgradeCity = getUpgradeCity((UnitTypes)iI, bSearch, &iSearchValue);
			if (pUpgradeCity != NULL)
			{
				// if not searching or close enough, then this match will do
				if (!bSearch || iSearchValue < 16)
				{
					return pUpgradeCity;
				}
				
				if (iSearchValue < iBestSearchValue)
				{
					iBestSearchValue = iSearchValue;
					pBestUpgradeCity = pUpgradeCity;
				}
			}
		}
	}

	return pBestUpgradeCity;
}

// finds the 'best' city which has a valid upgrade for the unit, to eUnit type
// it specifically does not check whether the unit can move, or if the player has enough gold to upgrade
// those are checked in canUpgrade()
// if bSearch is true, it will check every city, if not, it will only check the closest valid city
// if iSearchValue non NULL, then on return it will be the city's proximity value, lower is better
// NULL result means the upgrade is not possible
CvCity* CvUnit::getUpgradeCity(UnitTypes eUnit, bool bSearch, int* iSearchValue) const
{
	if (eUnit == NO_UNIT)
	{
		return false;
	}
	
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	CvUnitInfo& kUnitInfo = GC.getUnitInfo(eUnit);
/************************************************************************************************/
/* Afforess	                  Start		 02/17/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (!GC.getGameINLINE().isOption(GAMEOPTION_ASSIMILATION))
	{
		if (GC.getCivilizationInfo(kPlayer.getCivilizationType()).getCivilizationUnits(kUnitInfo.getUnitClassType()) != eUnit)
		{
			return false;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	if (!upgradeAvailable(getUnitType(), ((UnitClassTypes)(kUnitInfo.getUnitClassType()))))
	{
		return false;
	}

	if (kUnitInfo.getCargoSpace() < getCargo())
	{
		return false;
	}

	CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = plot()->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTransportUnit() == this)
		{
			if (kUnitInfo.getSpecialCargo() != NO_SPECIALUNIT)
			{
				if (kUnitInfo.getSpecialCargo() != pLoopUnit->getSpecialUnitType())
				{
					return false;
				}
			}

			if (kUnitInfo.getDomainCargo() != NO_DOMAIN)
			{
				if (kUnitInfo.getDomainCargo() != pLoopUnit->getDomainType())
				{
					return false;
				}
			}
		}
	}
	
	// sea units must be built on the coast
	bool bCoastalOnly = (getDomainType() == DOMAIN_SEA);

	// results
	int iBestValue = MAX_INT;
	CvCity* pBestCity = NULL;
		
	// if search is true, check every city for our team
	if (bSearch)
	{
		// air units can travel any distance
		bool bIgnoreDistance = (getDomainType() == DOMAIN_AIR);

		TeamTypes eTeam = getTeam();
		int iArea = getArea();
		int iX = getX_INLINE(), iY = getY_INLINE();

		// check every player on our team's cities
		for (int iI = 0; iI < MAX_PLAYERS; iI++)
		{
			// is this player on our team?
			CvPlayerAI& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
			if (kLoopPlayer.isAlive() && kLoopPlayer.getTeam() == eTeam)
			{
				int iLoop;
				for (CvCity* pLoopCity = kLoopPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kLoopPlayer.nextCity(&iLoop))
				{
					// if coastal only, then make sure we are coast
					CvArea* pWaterArea = NULL;
					if (!bCoastalOnly || ((pWaterArea = pLoopCity->waterArea()) != NULL && !pWaterArea->isLake()))
					{
						// can this city tran this unit?
						if (pLoopCity->canTrain(eUnit, false, false, true))
						{
							// if we do not care about distance, then the first match will do
							if (bIgnoreDistance)
							{
								// if we do not care about distance, then return 1 for value
								if (iSearchValue != NULL)
								{
									*iSearchValue = 1;
								}

								return pLoopCity;
							}

							int iValue = plotDistance(iX, iY, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());

							// if not same area, not as good (lower numbers are better)
							if (iArea != pLoopCity->getArea() && (!bCoastalOnly || iArea != pWaterArea->getID()))
							{
								iValue *= 16;
							}

							// if we cannot path there, not as good (lower numbers are better)
							if (!generatePath(pLoopCity->plot(), 0, true))
							{
								iValue *= 16;
							}

							if (iValue < iBestValue)
							{
								iBestValue = iValue;
								pBestCity = pLoopCity;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		// find the closest city
		CvCity* pClosestCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), NO_PLAYER, getTeam(), true, bCoastalOnly);
		if (pClosestCity != NULL)
		{
			// if we can train, then return this city (otherwise it will return NULL)
			if (pClosestCity->canTrain(eUnit, false, false, true))
			{
				// did not search, always return 1 for search value
				iBestValue = 1;

				pBestCity = pClosestCity;
			}
		}
	}

	// return the best value, if non-NULL
	if (iSearchValue != NULL)
	{
		*iSearchValue = iBestValue;
	}

	return pBestCity;
}

void CvUnit::upgrade(UnitTypes eUnit)
{
	CvUnit* pUpgradeUnit;

	if (!canUpgrade(eUnit))
	{
		return;
	}

// BUG - Upgrade Unit Event - start
	int iPrice = upgradePrice(eUnit);
	GET_PLAYER(getOwnerINLINE()).changeGold(-iPrice);
// BUG - Upgrade Unit Event - end

	pUpgradeUnit = GET_PLAYER(getOwnerINLINE()).initUnit(eUnit, getX_INLINE(), getY_INLINE(), AI_getUnitAIType());

	FAssertMsg(pUpgradeUnit != NULL, "UpgradeUnit is not assigned a valid value");

	pUpgradeUnit->joinGroup(getGroup());

	pUpgradeUnit->convert(this);

	pUpgradeUnit->finishMoves();

	if (pUpgradeUnit->getLeaderUnitType() == NO_UNIT)
	{
		if (pUpgradeUnit->getExperience() > GC.getDefineINT("MAX_EXPERIENCE_AFTER_UPGRADE"))
		{
			pUpgradeUnit->setExperience(GC.getDefineINT("MAX_EXPERIENCE_AFTER_UPGRADE"));
		}
	}

// BUG - Upgrade Unit Event - start
	CvEventReporter::getInstance().unitUpgraded(this, pUpgradeUnit, iPrice);
// BUG - Upgrade Unit Event - end

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/24/10                                jdog5000      */
/*                                                                                              */
/* AI Logging                                                                                   */
/************************************************************************************************/
	if( gUnitLogLevel > 2 )
	{
		CvWString szString;
		getUnitAIString(szString, AI_getUnitAIType());
		logBBAI("    %S spends %d to upgrade %S to %S, unit AI %S", GET_PLAYER(getOwnerINLINE()).getCivilizationDescription(0), upgradePrice(eUnit), getName(0).GetCString(), pUpgradeUnit->getName(0).GetCString(), szString.GetCString());
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
}


HandicapTypes CvUnit::getHandicapType() const
{
	return GET_PLAYER(getOwnerINLINE()).getHandicapType();
}


CivilizationTypes CvUnit::getCivilizationType() const
{
	return GET_PLAYER(getOwnerINLINE()).getCivilizationType();
}

const wchar* CvUnit::getVisualCivAdjective(TeamTypes eForTeam) const
{
	if (getVisualOwner(eForTeam) == getOwnerINLINE())
	{
		return GC.getCivilizationInfo(getCivilizationType()).getAdjectiveKey();
	}

	return L"";
}

SpecialUnitTypes CvUnit::getSpecialUnitType() const
{
	return ((SpecialUnitTypes)(m_pUnitInfo->getSpecialUnitType()));
}


UnitTypes CvUnit::getCaptureUnitType(CivilizationTypes eCivilization) const
{
	FAssert(eCivilization != NO_CIVILIZATION);
	return ((m_pUnitInfo->getUnitCaptureClassType() == NO_UNITCLASS) ? NO_UNIT : (UnitTypes)GC.getCivilizationInfo(eCivilization).getCivilizationUnits(m_pUnitInfo->getUnitCaptureClassType()));
}


UnitCombatTypes CvUnit::getUnitCombatType() const
{
	return ((UnitCombatTypes)(m_pUnitInfo->getUnitCombatType()));
}


DomainTypes CvUnit::getDomainType() const
{
	return ((DomainTypes)(m_pUnitInfo->getDomainType()));
}


InvisibleTypes CvUnit::getInvisibleType() const
{
	return ((InvisibleTypes)(m_pUnitInfo->getInvisibleType()));
}

int CvUnit::getNumSeeInvisibleTypes() const
{
	return m_pUnitInfo->getNumSeeInvisibleTypes();
}

InvisibleTypes CvUnit::getSeeInvisibleType(int i) const
{
	return (InvisibleTypes)(m_pUnitInfo->getSeeInvisibleType(i));
}


int CvUnit::flavorValue(FlavorTypes eFlavor) const
{
	return m_pUnitInfo->getFlavorValue(eFlavor);
}


bool CvUnit::isBarbarian() const
{
	return GET_PLAYER(getOwnerINLINE()).isBarbarian();
}


bool CvUnit::isHuman() const
{
	return GET_PLAYER(getOwnerINLINE()).isHuman();
}


int CvUnit::visibilityRange() const
{
	return (GC.getDefineINT("UNIT_VISIBILITY_RANGE") + getExtraVisibilityRange());
}


int CvUnit::baseMoves() const
{/************************************************************************************************/
/* Afforess	                  Start		 07/16/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
	return (m_pUnitInfo->getMoves() + getExtraMoves() + GET_TEAM(getTeam()).getExtraMoves(getDomainType()));
*/
	return (m_pUnitInfo->getMoves() + getExtraMoves() + (getDomainType() != DOMAIN_AIR ? GET_TEAM(getTeam()).getExtraMoves(getDomainType()) : 0));
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

}

int CvUnit::maxMoves() const
{
	return (baseMoves() * GC.getMOVE_DENOMINATOR());
}


int CvUnit::movesLeft() const
{
	return std::max(0, (maxMoves() - getMoves()));
}


bool CvUnit::canMove() const
{
	if (isDead())
	{
		return false;
	}

	if (getMoves() >= maxMoves())
	{
		return false;
	}

	if (getImmobileTimer() > 0)
	{
		return false;
	}

	return true;
}


bool CvUnit::hasMoved()	const
{
	return (getMoves() > 0);
}


int CvUnit::airRange() const
{
/************************************************************************************************/
/* Afforess	                  Start		 08/5/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/*
	return (m_pUnitInfo->getAirRange() + getExtraAirRange());
*/
	if (getDomainType() == DOMAIN_AIR && nukeRange() == -1)
	{
		return (m_pUnitInfo->getAirRange() + getExtraAirRange() + GET_TEAM(getTeam()).getExtraMoves(DOMAIN_AIR));
	}
	return (m_pUnitInfo->getAirRange() + getExtraAirRange());
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
}


int CvUnit::nukeRange() const
{
	return m_pUnitInfo->getNukeRange();
}


// XXX should this test for coal?
bool CvUnit::canBuildRoute() const
{
	int iI;

	for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
	{
		if (GC.getBuildInfo((BuildTypes)iI).getRoute() != NO_ROUTE)
		{
			if (m_pUnitInfo->getBuilds(iI))
			{
				if (GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getBuildInfo((BuildTypes)iI).getTechPrereq())))
				{
					return true;
				}
			}
		}
	}

	return false;
}

BuildTypes CvUnit::getBuildType() const
{
	BuildTypes eBuild;

	if (getGroup()->headMissionQueueNode() != NULL)
	{
		switch (getGroup()->headMissionQueueNode()->m_data.eMissionType)
		{
		case MISSION_MOVE_TO:
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
		case MISSION_MOVE_TO_SENTRY:
#endif
// BUG - Sentry Actions - end
			break;

		case MISSION_ROUTE_TO:
			if (getGroup()->getBestBuildRoute(plot(), &eBuild) != NO_ROUTE)
			{
				return eBuild;
			}
			break;

		case MISSION_MOVE_TO_UNIT:
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
		case MISSION_LEAD:
		case MISSION_ESPIONAGE:
		case MISSION_DIE_ANIMATION:
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
/* Afforess	                  Start		 06/05/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		case MISSION_HURRY_FOOD:
		case MISSION_INQUISITION:
		case MISSION_CLAIM_TERRITORY:
		case MISSION_ESPIONAGE_SLEEP:
		case MISSION_GREAT_COMMANDER:
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			break;

		case MISSION_BUILD:
			return (BuildTypes)getGroup()->headMissionQueueNode()->m_data.iData1;
			break;

		default:
			FAssert(false);
			break;
		}
	}

	return NO_BUILD;
}


int CvUnit::workRate(bool bMax) const
{
	int iRate;

	if (!bMax)
	{
		if (!canMove())
		{
			return 0;
		}
	}

	iRate = m_pUnitInfo->getWorkRate();

	iRate *= std::max(0, (GET_PLAYER(getOwnerINLINE()).getWorkerSpeedModifier() + 100));
	iRate /= 100;

	if (!isHuman() && !isBarbarian())
	{
		iRate *= std::max(0, (GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIWorkRateModifier() + 100));
		iRate /= 100;
	}

/************************************************************************************************/
/* Afforess	                  Start		 02/05/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (GET_PLAYER(getOwnerINLINE()).isDarkAge())
	{
		iRate *= std::max(0, (GC.getDefineINT("DARK_AGE_WORKER_SLOWDOWN_RATE") + 100));
		iRate /= 100;
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


	return iRate;
}

bool CvUnit::isAnimal() const
{
	return m_pUnitInfo->isAnimal();
}


bool CvUnit::isNoBadGoodies() const
{
	return m_pUnitInfo->isNoBadGoodies();
}


bool CvUnit::isOnlyDefensive() const
{
	return m_pUnitInfo->isOnlyDefensive();
}


bool CvUnit::isNoCapture() const
{
	return m_pUnitInfo->isNoCapture();
}


bool CvUnit::isRivalTerritory() const
{
	return m_pUnitInfo->isRivalTerritory();
}


bool CvUnit::isMilitaryHappiness() const
{
	return m_pUnitInfo->isMilitaryHappiness();
}


bool CvUnit::isInvestigate() const
{
	return m_pUnitInfo->isInvestigate();
}


bool CvUnit::isCounterSpy() const
{
	return m_pUnitInfo->isCounterSpy();
}


bool CvUnit::isSpy() const
{
	return m_pUnitInfo->isSpy();
}


bool CvUnit::isFound() const
{
	return m_pUnitInfo->isFound();
}

/********************************************************************************/
/**		REVOLUTION_MOD							1/1/08				DPII		*/
/**																				*/
/**		 																		*/
/********************************************************************************/
/*
bool CvUnit::isCanBeRebel() const
{
	return GC.getUnitInfo(getUnitType()).isCanBeRebel();
}

bool CvUnit::isCanRebelCapture() const
{
	return GC.getUnitInfo(getUnitType()).isCanRebelCapture();
}

bool CvUnit::isCannotDefect() const
{
	return GC.getUnitInfo(getUnitType()).isCannotDefect();
}

bool CvUnit::isCanQuellRebellion() const
{
	return GC.getUnitInfo(getUnitType()).isCanQuellRebellion();
}
*/
/********************************************************************************/
/**		REVOLUTION_MOD							END								*/
/********************************************************************************/

bool CvUnit::isGoldenAge() const
{
	if (isDelayedDeath())
	{
		return false;
	}

	return m_pUnitInfo->isGoldenAge();
}

bool CvUnit::canCoexistWithEnemyUnit(TeamTypes eTeam) const
{
	if (NO_TEAM == eTeam)
	{
		if(alwaysInvisible())
		{
			return true;
		}
		
		return false;
	}

	if(isInvisible(eTeam, false))
	{
		return true;
	}

	return false;
}

bool CvUnit::isFighting() const
{
	return (getCombatUnit() != NULL);
}


bool CvUnit::isAttacking() const
{
	return (getAttackPlot() != NULL && !isDelayedDeath());
}


bool CvUnit::isDefending() const
{
	return (isFighting() && !isAttacking());
}


bool CvUnit::isCombat() const
{
	return (isFighting() || isAttacking());
}


int CvUnit::maxHitPoints() const
{
	return GC.getMAX_HIT_POINTS();
}


int CvUnit::currHitPoints()	const
{
	return (maxHitPoints() - getDamage());
}


bool CvUnit::isHurt() const
{
	return (getDamage() > 0);
}


bool CvUnit::isDead() const
{
	return (getDamage() >= maxHitPoints());
}


void CvUnit::setBaseCombatStr(int iCombat)
{
	m_iBaseCombat = iCombat;
}

int CvUnit::baseCombatStr() const
{
	return m_iBaseCombat;
}


// maxCombatStr can be called in four different configurations
//		pPlot == NULL, pAttacker == NULL for combat when this is the attacker
//		pPlot valid, pAttacker valid for combat when this is the defender
/*** Dexy - Surround and Destroy START ****/
//		pPlot == NULL, pAttacker valid for combat when this is the defender, attacker is just surrounding us (then defender gets no plot defensive bonuses)
/*** Dexy - Surround and Destroy  END  ****/
//		pPlot valid, pAttacker == NULL (new case), when this is the defender, attacker unknown
//		pPlot valid, pAttacker == this (new case), when the defender is unknown, but we want to calc approx str
//			note, in this last case, it is expected pCombatDetails == NULL, it does not have to be, but some 
//			values may be unexpectedly reversed in this case (iModifierTotal will be the negative sum)
/*** Dexy - Surround and Destroy START ****/
int CvUnit::maxCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails, bool bSurroundedModifier) const
// OLD CODE
// int CvUnit::maxCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails) const
/*** Dexy - Surround and Destroy  END  ****/
{
	int iCombat;

	FAssertMsg((pPlot == NULL) || (pPlot->getTerrainType() != NO_TERRAIN), "(pPlot == NULL) || (pPlot->getTerrainType() is not expected to be equal with NO_TERRAIN)");
	
	// handle our new special case
	const	CvPlot*	pAttackedPlot = NULL;
	bool	bAttackingUnknownDefender = false;
	if (pAttacker == this)
	{
		bAttackingUnknownDefender = true;
		pAttackedPlot = pPlot;
		
		// reset these values, we will fiddle with them below
		pPlot = NULL;
		pAttacker = NULL;
	}
	// otherwise, attack plot is the plot of us (the defender)
	else if (pAttacker != NULL)
	{
		pAttackedPlot = plot();
	}

	if (pCombatDetails != NULL)
	{
		pCombatDetails->iExtraCombatPercent = 0;
		pCombatDetails->iAnimalCombatModifierTA = 0;
		pCombatDetails->iAIAnimalCombatModifierTA = 0;
		pCombatDetails->iAnimalCombatModifierAA = 0;
		pCombatDetails->iAIAnimalCombatModifierAA = 0;
		pCombatDetails->iBarbarianCombatModifierTB = 0;
		pCombatDetails->iAIBarbarianCombatModifierTB = 0;
		pCombatDetails->iBarbarianCombatModifierAB = 0;
		pCombatDetails->iAIBarbarianCombatModifierAB = 0;
		pCombatDetails->iPlotDefenseModifier = 0;
		pCombatDetails->iFortifyModifier = 0;
		pCombatDetails->iCityDefenseModifier = 0;
		pCombatDetails->iHillsAttackModifier = 0;
		pCombatDetails->iHillsDefenseModifier = 0;
		pCombatDetails->iFeatureAttackModifier = 0;
		pCombatDetails->iFeatureDefenseModifier = 0;
		pCombatDetails->iTerrainAttackModifier = 0;
		pCombatDetails->iTerrainDefenseModifier = 0;
		pCombatDetails->iCityAttackModifier = 0;
		pCombatDetails->iDomainDefenseModifier = 0;
		pCombatDetails->iCityBarbarianDefenseModifier = 0;
		pCombatDetails->iClassDefenseModifier = 0;
		pCombatDetails->iClassAttackModifier = 0;
		pCombatDetails->iCombatModifierA = 0;
		pCombatDetails->iCombatModifierT = 0;
		pCombatDetails->iDomainModifierA = 0;
		pCombatDetails->iDomainModifierT = 0;
		pCombatDetails->iAnimalCombatModifierA = 0;
		pCombatDetails->iAnimalCombatModifierT = 0;
		pCombatDetails->iRiverAttackModifier = 0;
		pCombatDetails->iAmphibAttackModifier = 0;
		pCombatDetails->iKamikazeModifier = 0;
		pCombatDetails->iModifierTotal = 0;
		pCombatDetails->iBaseCombatStr = 0;
		pCombatDetails->iCombat = 0;
		pCombatDetails->iMaxCombatStr = 0;
		pCombatDetails->iCurrHitPoints = 0;
		pCombatDetails->iMaxHitPoints = 0;
		pCombatDetails->iCurrCombatStr = 0;
		pCombatDetails->eOwner = getOwnerINLINE();
		pCombatDetails->eVisualOwner = getVisualOwner();
		pCombatDetails->sUnitName = getName().GetCString();
	}

	if (baseCombatStr() == 0)
	{
		return 0;
	}

	int iModifier = 0;
	int iExtraModifier;

	iExtraModifier = getExtraCombatPercent();
	iModifier += iExtraModifier;
	if (pCombatDetails != NULL)
	{
		pCombatDetails->iExtraCombatPercent = iExtraModifier;
	}
	
	// do modifiers for animals and barbarians (leaving these out for bAttackingUnknownDefender case)
	if (pAttacker != NULL)
	{
		if (isAnimal())
		{
			if (pAttacker->isHuman())
			{
				iExtraModifier = GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAnimalCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAnimalCombatModifierTA = iExtraModifier;
				}
			}
			else
			{
				iExtraModifier = GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIAnimalCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAIAnimalCombatModifierTA = iExtraModifier;
				}
			}
		}

		if (pAttacker->isAnimal())
		{
			if (isHuman())
			{
				iExtraModifier = -GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAnimalCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAnimalCombatModifierAA = iExtraModifier;
				}
			}
			else
			{
				iExtraModifier = -GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIAnimalCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAIAnimalCombatModifierAA = iExtraModifier;
				}
			}
		}

		if (isBarbarian())
		{
			if (pAttacker->isHuman())
			{
				iExtraModifier = GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getBarbarianCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iBarbarianCombatModifierTB = iExtraModifier;
				}
			}
			else
			{
				iExtraModifier = GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIBarbarianCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAIBarbarianCombatModifierTB = iExtraModifier;
				}
			}
		}

		if (pAttacker->isBarbarian())
		{
			if (isHuman())
			{
				iExtraModifier = -GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getBarbarianCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iBarbarianCombatModifierAB = iExtraModifier;
				}
			}
			else
			{
				iExtraModifier = -GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIBarbarianCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAIBarbarianCombatModifierTB = iExtraModifier;
				}
			}
		}
	}
	
	// add defensive bonuses (leaving these out for bAttackingUnknownDefender case)
	if (pPlot != NULL)
	{
		if (!noDefensiveBonus())
		{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/30/10                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
			// When pAttacker is NULL but pPlot is not, this is a computation for this units defensive value
			// against an unknown attacker.  Always ignoring building defense in this case is a conservative estimate,
			// but causes AI to suicide against castle walls of low culture cities in early game.  Using this units
			// ignoreBuildingDefense does a little better ... in early game it corrects undervalue of castles.  One
			// downside is when medieval unit is defending a walled city against gunpowder.  Here, the over value
			// makes attacker a little more cautious, but with their tech lead it shouldn't matter too much.  Also
			// makes vulnerable units (ships, etc) feel safer in this case and potentially not leave, but ships
			// leave when ratio is pretty low anyway.

			//iExtraModifier = pPlot->defenseModifier(getTeam(), (pAttacker != NULL) ? pAttacker->ignoreBuildingDefense() : true);
			iExtraModifier = pPlot->defenseModifier(getTeam(), (pAttacker != NULL) ? pAttacker->ignoreBuildingDefense() : ignoreBuildingDefense());
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
			iModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iPlotDefenseModifier = iExtraModifier;
			}
		}

		iExtraModifier = fortifyModifier();
		iModifier += iExtraModifier;
		if (pCombatDetails != NULL)
		{
			pCombatDetails->iFortifyModifier = iExtraModifier;
		}

		if (pPlot->isCity(true, getTeam()))
		{
			iExtraModifier = cityDefenseModifier();

/************************************************************************************************/
/* Afforess	                  Start		 05/22/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			if (pPlot->isCity(false) && getUnitCombatType() != NO_UNITCOMBAT)
			{
				iExtraModifier += pPlot->getPlotCity()->getUnitCombatExtraStrength(getUnitCombatType());
			}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			iModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iCityDefenseModifier = iExtraModifier;
			}

		}

		if (pPlot->isHills())
		{
			iExtraModifier = hillsDefenseModifier();
			iModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iHillsDefenseModifier = iExtraModifier;
			}
		}

		if (pPlot->getFeatureType() != NO_FEATURE)
		{
			iExtraModifier = featureDefenseModifier(pPlot->getFeatureType());
			iModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iFeatureDefenseModifier = iExtraModifier;
			}
		}
		else
		{
			iExtraModifier = terrainDefenseModifier(pPlot->getTerrainType());
			iModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iTerrainDefenseModifier = iExtraModifier;
			}
		}
	}

	// if we are attacking to an plot with an unknown defender, the calc the modifier in reverse
	if (bAttackingUnknownDefender)
	{
		pAttacker = this;
	}

	// calc attacker bonueses
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       09/20/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original code
	if (pAttacker != NULL)
*/
	if (pAttacker != NULL && pAttackedPlot != NULL)
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
	{
		int iTempModifier = 0;		

		if (pAttackedPlot->isCity(true, getTeam()))
		{
			iExtraModifier = -pAttacker->cityAttackModifier();
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iCityAttackModifier = iExtraModifier;
			}

			if (pAttacker->isBarbarian())
			{
				iExtraModifier = GC.getDefineINT("CITY_BARBARIAN_DEFENSE_MODIFIER");
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iCityBarbarianDefenseModifier = iExtraModifier;
				}
			}
		}

		if (pAttackedPlot->isHills())
		{
			iExtraModifier = -pAttacker->hillsAttackModifier();
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iHillsAttackModifier = iExtraModifier;
			}
		}
		
		if (pAttackedPlot->getFeatureType() != NO_FEATURE)
		{
			iExtraModifier = -pAttacker->featureAttackModifier(pAttackedPlot->getFeatureType());
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iFeatureAttackModifier = iExtraModifier;
			}
		}
		else
		{
			iExtraModifier = -pAttacker->terrainAttackModifier(pAttackedPlot->getTerrainType());
			/*** Dexy - Others' bug fixes START ****/
			iTempModifier += iExtraModifier;
			// OLD CODE
			// iModifier += iExtraModifier;
			/*** Dexy - Others' bug fixes  END  ****/
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iTerrainAttackModifier = iExtraModifier;
			}
		}

		// only compute comparisions if we are the defender with a known attacker
		if (!bAttackingUnknownDefender)
		{
			FAssertMsg(pAttacker != this, "pAttacker is not expected to be equal with this");

			iExtraModifier = unitClassDefenseModifier(pAttacker->getUnitClassType());
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iClassDefenseModifier = iExtraModifier;
			}

			iExtraModifier = -pAttacker->unitClassAttackModifier(getUnitClassType());
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iClassAttackModifier = iExtraModifier;
			}

			if (pAttacker->getUnitCombatType() != NO_UNITCOMBAT)
			{
				iExtraModifier = unitCombatModifier(pAttacker->getUnitCombatType());
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iCombatModifierA = iExtraModifier;
				}
			}
			if (getUnitCombatType() != NO_UNITCOMBAT)
			{
				iExtraModifier = -pAttacker->unitCombatModifier(getUnitCombatType());
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iCombatModifierT = iExtraModifier;
				}
			}

			iExtraModifier = domainModifier(pAttacker->getDomainType());
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iDomainModifierA = iExtraModifier;
			}

			iExtraModifier = -pAttacker->domainModifier(getDomainType());
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iDomainModifierT = iExtraModifier;
			}

			if (pAttacker->isAnimal())
			{
				iExtraModifier = animalCombatModifier();
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAnimalCombatModifierA = iExtraModifier;
				}
			}

			if (isAnimal())
			{
				iExtraModifier = -pAttacker->animalCombatModifier();
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAnimalCombatModifierT = iExtraModifier;
				}
			}
		}

		if (!(pAttacker->isRiver()))
		{
			if (pAttacker->plot()->isRiverCrossing(directionXY(pAttacker->plot(), pAttackedPlot)))
			{
				iExtraModifier = -GC.getRIVER_ATTACK_MODIFIER();
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iRiverAttackModifier = iExtraModifier;
				}
			}
		}

		if (!(pAttacker->isAmphib()))
		{
			if (!(pAttackedPlot->isWater()) && pAttacker->plot()->isWater())
			{
				iExtraModifier = -GC.getAMPHIB_ATTACK_MODIFIER();
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAmphibAttackModifier = iExtraModifier;
				}
			}
		}
		
/************************************************************************************************/
/* Afforess	                  Start		 02/05/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		if (GET_PLAYER(pAttacker->getOwnerINLINE()).isDarkAge())
		{
			iTempModifier += GC.getDefineINT("DARK_AGE_ATTACK_STRENGTH_PERCENT_DECREASE");
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

		if (pAttacker->getKamikazePercent() != 0)
		{
/************************************************************************************************/
/* Afforess	                  Start		 02/05/10                                    Dexy       */
/*                                                                                              */
/*       Unofficial Patch                                                                       */
/************************************************************************************************/
			iExtraModifier = -pAttacker->getKamikazePercent();
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iKamikazeModifier = iExtraModifier;
			}
		}
		
/************************************************************************************************/
/* Afforess	                  Start		 02/05/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		if (bSurroundedModifier)
		{
			// the stronger the surroundings -> decrease the iModifier more
			iExtraModifier = -pAttacker->surroundedDefenseModifier(pAttackedPlot, bAttackingUnknownDefender ? NULL : this);
			iTempModifier += iExtraModifier;
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		
		// if we are attacking an unknown defender, then use the reverse of the modifier
		if (bAttackingUnknownDefender)
		{
			iModifier -= iTempModifier;
		}
		else
		{
			iModifier += iTempModifier;
		}
	}

	if (pCombatDetails != NULL)
	{
		pCombatDetails->iModifierTotal = iModifier;
		pCombatDetails->iBaseCombatStr = baseCombatStr();
	}

	if (iModifier > 0)
	{
		iCombat = (baseCombatStr() * (iModifier + 100));
	}
	else
	{
		iCombat = ((baseCombatStr() * 10000) / (100 - iModifier));
	}

	if (pCombatDetails != NULL)
	{
		pCombatDetails->iCombat = iCombat;
		pCombatDetails->iMaxCombatStr = std::max(1, iCombat);
		pCombatDetails->iCurrHitPoints = currHitPoints();
		pCombatDetails->iMaxHitPoints = maxHitPoints();
		pCombatDetails->iCurrCombatStr = ((pCombatDetails->iMaxCombatStr * pCombatDetails->iCurrHitPoints) / pCombatDetails->iMaxHitPoints);
	}

	return std::max(1, iCombat);
}


/*** Dexy - Surround and Destroy START ****/
int CvUnit::currCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails, bool bSurroundedModifier) const
{
	return ((maxCombatStr(pPlot, pAttacker, pCombatDetails, bSurroundedModifier) * currHitPoints()) / maxHitPoints());
}
// OLD CODE
// int CvUnit::currCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails) const
// {
// 	   return ((maxCombatStr(pPlot, pAttacker, pCombatDetails) * currHitPoints()) / maxHitPoints());
// }
/*** Dexy - Surround and Destroy  END  ****/


int CvUnit::currFirepower(const CvPlot* pPlot, const CvUnit* pAttacker) const
{
	return ((maxCombatStr(pPlot, pAttacker) + currCombatStr(pPlot, pAttacker) + 1) / 2);
}

// this nomalizes str by firepower, useful for quick odds calcs
// the effect is that a damaged unit will have an effective str lowered by firepower/maxFirepower
// doing the algebra, this means we mulitply by 1/2(1 + currHP)/maxHP = (maxHP + currHP) / (2 * maxHP)
int CvUnit::currEffectiveStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails) const
{
	int currStr = currCombatStr(pPlot, pAttacker, pCombatDetails);

	currStr *= (maxHitPoints() + currHitPoints());
	currStr /= (2 * maxHitPoints());
	
	return currStr;
}

float CvUnit::maxCombatStrFloat(const CvPlot* pPlot, const CvUnit* pAttacker) const
{
	return (((float)(maxCombatStr(pPlot, pAttacker))) / 100.0f);
}


float CvUnit::currCombatStrFloat(const CvPlot* pPlot, const CvUnit* pAttacker) const
{
	return (((float)(currCombatStr(pPlot, pAttacker))) / 100.0f);
}


bool CvUnit::canFight() const
{
	return (baseCombatStr() > 0);
}


bool CvUnit::canAttack() const
{
	if (!canFight())
	{
		return false;
	}

	if (isOnlyDefensive())
	{
		return false;
	}

	return true;
}
bool CvUnit::canAttack(const CvUnit& defender) const
{
	if (!canAttack())
	{
		return false;
	}

	if (defender.getDamage() >= combatLimit())
	{
		return false;
	}

	// Artillery can't amphibious attack
	if (plot()->isWater() && !defender.plot()->isWater())
	{
		if (combatLimit() < 100)
		{
			return false;
		}
	}

	return true;
}

bool CvUnit::canDefend(const CvPlot* pPlot) const
{
	if (pPlot == NULL)
	{
		pPlot = plot();
	}

	if (!canFight())
	{
		return false;
	}

	if (!pPlot->isValidDomainForAction(*this))
	{
		if (GC.getDefineINT("LAND_UNITS_CAN_ATTACK_WATER_CITIES") == 0)
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::canSiege(TeamTypes eTeam) const
{
	if (!canDefend())
	{
		return false;
	}

	if (!isEnemy(eTeam))
	{
		return false;
	}

	if (!isNeverInvisible())
	{
		return false;
	}

	return true;
}


int CvUnit::airBaseCombatStr() const
{
	return m_pUnitInfo->getAirCombat();
}


int CvUnit::airMaxCombatStr(const CvUnit* pOther) const
{
	int iModifier;
	int iCombat;

	if (airBaseCombatStr() == 0)
	{
		return 0;
	}

	iModifier = getExtraCombatPercent();

	if (getKamikazePercent() != 0)
	{
		iModifier += getKamikazePercent();
	}

/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						8/16/08		DanF5771 & jdog5000	*/
/* 																			*/
/* 	Bugfix																	*/
/********************************************************************************/
/* original BTS code
	if (getExtraCombatPercent() != 0)
	{
		iModifier += getExtraCombatPercent();
	}
*/
	// ExtraCombatPercent already counted above
/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						END								*/
/********************************************************************************/

	if (NULL != pOther)
	{
		if (pOther->getUnitCombatType() != NO_UNITCOMBAT)
		{
			iModifier += unitCombatModifier(pOther->getUnitCombatType());
		}

		iModifier += domainModifier(pOther->getDomainType());

		if (pOther->isAnimal())
		{
			iModifier += animalCombatModifier();
		}
	}

	if (iModifier > 0)
	{
		iCombat = (airBaseCombatStr() * (iModifier + 100));
	}
	else
	{
		iCombat = ((airBaseCombatStr() * 10000) / (100 - iModifier));
	}

	return std::max(1, iCombat);
}


int CvUnit::airCurrCombatStr(const CvUnit* pOther) const
{
	return ((airMaxCombatStr(pOther) * currHitPoints()) / maxHitPoints());
}


float CvUnit::airMaxCombatStrFloat(const CvUnit* pOther) const
{
	return (((float)(airMaxCombatStr(pOther))) / 100.0f);
}


float CvUnit::airCurrCombatStrFloat(const CvUnit* pOther) const
{
	return (((float)(airCurrCombatStr(pOther))) / 100.0f);
}


int CvUnit::combatLimit() const
{
	return m_pUnitInfo->getCombatLimit();
}


int CvUnit::airCombatLimit() const
{
	return m_pUnitInfo->getAirCombatLimit();
}


bool CvUnit::canAirAttack() const
{
	return (airBaseCombatStr() > 0);
}


bool CvUnit::canAirDefend(const CvPlot* pPlot) const
{
	if (pPlot == NULL)
	{
		pPlot = plot();
	}

	//TSHEEP - prevent spies from being used as SAMs
	if(isSpy())
	{
		return false;
	}
	//TSHEEP end

	if (maxInterceptionProbability() == 0)
	{
		return false;
	}

	if (getDomainType() != DOMAIN_AIR)
	{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/30/09                     Mongoose & jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
		if (!pPlot->isValidDomainForLocation(*this))
*/
		// From Mongoose SDK
		// Land units which are cargo cannot intercept
		if (!pPlot->isValidDomainForLocation(*this) || isCargo())
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
		{
			return false;
		}
	}

	return true;
}


int CvUnit::airCombatDamage(const CvUnit* pDefender) const
{
	CvCity* pCity;
	CvPlot* pPlot;
	int iOurStrength;
	int iTheirStrength;
	int iStrengthFactor;
	int iDamage;

	pPlot = pDefender->plot();

	iOurStrength = airCurrCombatStr(pDefender);
	FAssertMsg(iOurStrength > 0, "Air combat strength is expected to be greater than zero");
	iTheirStrength = pDefender->maxCombatStr(pPlot, this);

	iStrengthFactor = ((iOurStrength + iTheirStrength + 1) / 2);

	iDamage = std::max(1, ((GC.getDefineINT("AIR_COMBAT_DAMAGE") * (iOurStrength + iStrengthFactor)) / (iTheirStrength + iStrengthFactor)));

	pCity = pPlot->getPlotCity();

	if (pCity != NULL)
	{
		iDamage *= std::max(0, (pCity->getAirModifier() + 100));
		iDamage /= 100;
	}

	return iDamage;
}


int CvUnit::rangeCombatDamage(const CvUnit* pDefender) const
{
	CvPlot* pPlot;
	int iOurStrength;
	int iTheirStrength;
	int iStrengthFactor;
	int iDamage;

	pPlot = pDefender->plot();

	iOurStrength = airCurrCombatStr(pDefender);
	FAssertMsg(iOurStrength > 0, "Combat strength is expected to be greater than zero");
	iTheirStrength = pDefender->maxCombatStr(pPlot, this);

	iStrengthFactor = ((iOurStrength + iTheirStrength + 1) / 2);

	iDamage = std::max(1, ((GC.getDefineINT("RANGE_COMBAT_DAMAGE") * (iOurStrength + iStrengthFactor)) / (iTheirStrength + iStrengthFactor)));

	return iDamage;
}


CvUnit* CvUnit::bestInterceptor(const CvPlot* pPlot) const
{
	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	iBestValue = 0;
	pBestUnit = NULL;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (isEnemy(GET_PLAYER((PlayerTypes)iI).getTeam()) && !isInvisible(GET_PLAYER((PlayerTypes)iI).getTeam(), false, false))
			{
				for(pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
				{
					if (pLoopUnit->canAirDefend())
					{
						if (!pLoopUnit->isMadeInterception())
						{
							if ((pLoopUnit->getDomainType() != DOMAIN_AIR) || !(pLoopUnit->hasMoved()))
							{
								if ((pLoopUnit->getDomainType() != DOMAIN_AIR) || (pLoopUnit->getGroup()->getActivityType() == ACTIVITY_INTERCEPT))
								{
									if (plotDistance(pLoopUnit->getX_INLINE(), pLoopUnit->getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE()) <= pLoopUnit->airRange())
									{
										iValue = pLoopUnit->currInterceptionProbability();

										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											pBestUnit = pLoopUnit;
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

	return pBestUnit;
}


CvUnit* CvUnit::bestSeaPillageInterceptor(CvUnit* pPillager, int iMinOdds) const
{
	CvUnit* pBestUnit = NULL;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Lead From Behind                                                                             */
/************************************************************************************************/
	// From Lead From Behind by UncutDragon
	int pBestUnitRank = -1;

	for (int iDX = -1; iDX <= 1; ++iDX)
	{
		for (int iDY = -1; iDY <= 1; ++iDY)
		{
			CvPlot* pLoopPlot = plotXY(pPillager->getX_INLINE(), pPillager->getY_INLINE(), iDX, iDY);

			if (NULL != pLoopPlot)
			{
				CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();

				while (NULL != pUnitNode)
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

					if (NULL != pLoopUnit)
					{
						if (pLoopUnit->area() == pPillager->plot()->area())
						{
							if (!pLoopUnit->isInvisible(getTeam(), false))
							{
								if (isEnemy(pLoopUnit->getTeam()))
								{
									if (DOMAIN_SEA == pLoopUnit->getDomainType())
									{
										if (ACTIVITY_PATROL == pLoopUnit->getGroup()->getActivityType())
										{
											// UncutDragon
/* original code
											if (NULL == pBestUnit || pLoopUnit->isBetterDefenderThan(pBestUnit, this))
*/											// modified (added extra parameter)
											if (NULL == pBestUnit || pLoopUnit->isBetterDefenderThan(pBestUnit, this, &pBestUnitRank))
											// /UncutDragon
											{
												if (getCombatOdds(pPillager, pLoopUnit) < iMinOdds)
												{
													pBestUnit = pLoopUnit;
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
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	return pBestUnit;
}


bool CvUnit::isAutomated() const
{
	return getGroup()->isAutomated();
}


bool CvUnit::isWaiting() const
{
	return getGroup()->isWaiting();
}


bool CvUnit::isFortifyable() const
{
	if (!canFight() || noDefensiveBonus() || ((getDomainType() != DOMAIN_LAND) && (getDomainType() != DOMAIN_IMMOBILE)))
	{
		return false;
	}

	return true;
}


int CvUnit::fortifyModifier() const
{
	if (!isFortifyable())
	{
		return 0;
	}

	return (getFortifyTurns() * GC.getFORTIFY_MODIFIER_PER_TURN());
}


int CvUnit::experienceNeeded() const
{

	int iExperienceNeeded = calculateExperience(getLevel(), getOwnerINLINE());

/************************************************************************************************/
/* Afforess	                  Start		 04/2/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	//adjust level thresholds to a world size.
	if (isCommander())
	{
//		iExperienceNeeded *= GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getCommandersLevelThresholdsPercent() / 100;
		iExperienceNeeded *= 3;
		iExperienceNeeded /= 2;
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	return iExperienceNeeded;
}


int CvUnit::attackXPValue() const
{
	return m_pUnitInfo->getXPValueAttack();
}


int CvUnit::defenseXPValue() const
{
	return m_pUnitInfo->getXPValueDefense();
}


int CvUnit::maxXPValue() const
{
	int iMaxValue;

	iMaxValue = MAX_INT;

	if (isAnimal())
	{
		iMaxValue = std::min(iMaxValue, GC.getDefineINT("ANIMAL_MAX_XP_VALUE"));
	}

	if (isBarbarian())
	{
		iMaxValue = std::min(iMaxValue, GC.getDefineINT("BARBARIAN_MAX_XP_VALUE"));
	}

	return iMaxValue;
}


int CvUnit::firstStrikes() const
{
	return std::max(0, (m_pUnitInfo->getFirstStrikes() + getExtraFirstStrikes()));
}


int CvUnit::chanceFirstStrikes() const
{
	return std::max(0, (m_pUnitInfo->getChanceFirstStrikes() + getExtraChanceFirstStrikes()));
}


int CvUnit::maxFirstStrikes() const
{
	return (firstStrikes() + chanceFirstStrikes());
}


bool CvUnit::isRanged() const
{
	int i;
	CvUnitInfo * pkUnitInfo = &getUnitInfo();
	for ( i = 0; i < pkUnitInfo->getGroupDefinitions(); i++ )
	{
		if ( !getArtInfo(i, GET_PLAYER(getOwnerINLINE()).getCurrentEra())->getActAsRanged() )
		{
			return false;
		}
	}	
	return true;
}


bool CvUnit::alwaysInvisible() const
{
	return m_pUnitInfo->isInvisible();
}


bool CvUnit::immuneToFirstStrikes() const
{
	return (m_pUnitInfo->isFirstStrikeImmune() || (getImmuneToFirstStrikesCount() > 0));
}


bool CvUnit::noDefensiveBonus() const
{
	return m_pUnitInfo->isNoDefensiveBonus();
}


bool CvUnit::ignoreBuildingDefense() const
{
	return m_pUnitInfo->isIgnoreBuildingDefense();
}


bool CvUnit::canMoveImpassable() const
{
	return m_pUnitInfo->isCanMoveImpassable();
}

bool CvUnit::canMoveAllTerrain() const
{
	return m_pUnitInfo->isCanMoveAllTerrain();
}

bool CvUnit::flatMovementCost() const
{
	return m_pUnitInfo->isFlatMovementCost();
}


bool CvUnit::ignoreTerrainCost() const
{
	return m_pUnitInfo->isIgnoreTerrainCost();
}


bool CvUnit::isNeverInvisible() const
{
	return (!alwaysInvisible() && (getInvisibleType() == NO_INVISIBLE));
}


bool CvUnit::isInvisible(TeamTypes eTeam, bool bDebug, bool bCheckCargo) const
{
	if (bDebug && GC.getGameINLINE().isDebugMode())
	{
		return false;
	}

	if (getTeam() == eTeam)
	{
		return false;
	}

	if (alwaysInvisible())
	{
		return true;
	}

	if (bCheckCargo && isCargo())
	{
		return true;
	}

	if (getInvisibleType() == NO_INVISIBLE)
	{
		return false;
	}

	return !(plot()->isInvisibleVisible(eTeam, getInvisibleType()));
}


bool CvUnit::isNukeImmune() const
{
	return m_pUnitInfo->isNukeImmune();
}

/************************************************************************************************/
/* REVDCM_OC                              02/16/10                                phungus420    */
/*                                                                                              */
/* Inquisitions                                                                                 */
/************************************************************************************************/
bool CvUnit::isInquisitor() const
{
	return m_pUnitInfo->isInquisitor();
}
/************************************************************************************************/
/* REVDCM_OC                               END                                                  */
/************************************************************************************************/

int CvUnit::maxInterceptionProbability() const
{
	return std::max(0, m_pUnitInfo->getInterceptionProbability() + getExtraIntercept());
}


int CvUnit::currInterceptionProbability() const
{
	if (getDomainType() != DOMAIN_AIR && !GC.getGameINLINE().isModderGameOption(MODDERGAMEOPTION_BETTER_INTERCETION))
	{
		return maxInterceptionProbability();
	}
	else
	{
		return ((maxInterceptionProbability() * currHitPoints()) / maxHitPoints());
	}
}


int CvUnit::evasionProbability() const
{
	return std::max(0, m_pUnitInfo->getEvasionProbability() + getExtraEvasion());
}


int CvUnit::withdrawalProbability() const
{
	if (getDomainType() == DOMAIN_LAND && plot()->isWater())
	{
		return 0;
	}
/************************************************************************************************/
/* Afforess	                  Start		 04/02/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	return std::min(100, std::max(0, (m_pUnitInfo->getWithdrawalProbability() + getExtraWithdrawal())));
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

}


int CvUnit::collateralDamage() const
{
	return std::max(0, (m_pUnitInfo->getCollateralDamage()));
}


int CvUnit::collateralDamageLimit() const
{
	return std::max(0, m_pUnitInfo->getCollateralDamageLimit() * GC.getMAX_HIT_POINTS() / 100);
}


int CvUnit::collateralDamageMaxUnits() const
{
	return std::max(0, m_pUnitInfo->getCollateralDamageMaxUnits());
}


int CvUnit::cityAttackModifier() const
{
	return (m_pUnitInfo->getCityAttackModifier() + getExtraCityAttackPercent());
}


int CvUnit::cityDefenseModifier() const
{
	return (m_pUnitInfo->getCityDefenseModifier() + getExtraCityDefensePercent());
}


int CvUnit::animalCombatModifier() const
{
	return m_pUnitInfo->getAnimalCombatModifier();
}


int CvUnit::hillsAttackModifier() const
{
	return (m_pUnitInfo->getHillsAttackModifier() + getExtraHillsAttackPercent());
}


int CvUnit::hillsDefenseModifier() const
{
	return (m_pUnitInfo->getHillsDefenseModifier() + getExtraHillsDefensePercent());
}


int CvUnit::terrainAttackModifier(TerrainTypes eTerrain) const
{
	FAssertMsg(eTerrain >= 0, "eTerrain is expected to be non-negative (invalid Index)");
	FAssertMsg(eTerrain < GC.getNumTerrainInfos(), "eTerrain is expected to be within maximum bounds (invalid Index)");
	return (m_pUnitInfo->getTerrainAttackModifier(eTerrain) + getExtraTerrainAttackPercent(eTerrain));
}


int CvUnit::terrainDefenseModifier(TerrainTypes eTerrain) const
{
	FAssertMsg(eTerrain >= 0, "eTerrain is expected to be non-negative (invalid Index)");
	FAssertMsg(eTerrain < GC.getNumTerrainInfos(), "eTerrain is expected to be within maximum bounds (invalid Index)");
	return (m_pUnitInfo->getTerrainDefenseModifier(eTerrain) + getExtraTerrainDefensePercent(eTerrain));
}


int CvUnit::featureAttackModifier(FeatureTypes eFeature) const
{
	FAssertMsg(eFeature >= 0, "eFeature is expected to be non-negative (invalid Index)");
	FAssertMsg(eFeature < GC.getNumFeatureInfos(), "eFeature is expected to be within maximum bounds (invalid Index)");
	return (m_pUnitInfo->getFeatureAttackModifier(eFeature) + getExtraFeatureAttackPercent(eFeature));
}

int CvUnit::featureDefenseModifier(FeatureTypes eFeature) const
{
	FAssertMsg(eFeature >= 0, "eFeature is expected to be non-negative (invalid Index)");
	FAssertMsg(eFeature < GC.getNumFeatureInfos(), "eFeature is expected to be within maximum bounds (invalid Index)");
	return (m_pUnitInfo->getFeatureDefenseModifier(eFeature) + getExtraFeatureDefensePercent(eFeature));
}

int CvUnit::unitClassAttackModifier(UnitClassTypes eUnitClass) const
{
	FAssertMsg(eUnitClass >= 0, "eUnitClass is expected to be non-negative (invalid Index)");
	FAssertMsg(eUnitClass < GC.getNumUnitClassInfos(), "eUnitClass is expected to be within maximum bounds (invalid Index)");
	return m_pUnitInfo->getUnitClassAttackModifier(eUnitClass);
}


int CvUnit::unitClassDefenseModifier(UnitClassTypes eUnitClass) const
{
	FAssertMsg(eUnitClass >= 0, "eUnitClass is expected to be non-negative (invalid Index)");
	FAssertMsg(eUnitClass < GC.getNumUnitClassInfos(), "eUnitClass is expected to be within maximum bounds (invalid Index)");
	return m_pUnitInfo->getUnitClassDefenseModifier(eUnitClass);
}


int CvUnit::unitCombatModifier(UnitCombatTypes eUnitCombat) const
{
	FAssertMsg(eUnitCombat >= 0, "eUnitCombat is expected to be non-negative (invalid Index)");
	FAssertMsg(eUnitCombat < GC.getNumUnitCombatInfos(), "eUnitCombat is expected to be within maximum bounds (invalid Index)");
	return (m_pUnitInfo->getUnitCombatModifier(eUnitCombat) + getExtraUnitCombatModifier(eUnitCombat));
}


int CvUnit::domainModifier(DomainTypes eDomain) const
{
	FAssertMsg(eDomain >= 0, "eDomain is expected to be non-negative (invalid Index)");
	FAssertMsg(eDomain < NUM_DOMAIN_TYPES, "eDomain is expected to be within maximum bounds (invalid Index)");
	return (m_pUnitInfo->getDomainModifier(eDomain) + getExtraDomainModifier(eDomain));
}


int CvUnit::bombardRate() const
{
	return (m_pUnitInfo->getBombardRate() + getExtraBombardRate());
}


int CvUnit::airBombBaseRate() const
{
	return m_pUnitInfo->getBombRate();
}


int CvUnit::airBombCurrRate() const
{
	return ((airBombBaseRate() * currHitPoints()) / maxHitPoints());
}


SpecialUnitTypes CvUnit::specialCargo() const
{
	return ((SpecialUnitTypes)(m_pUnitInfo->getSpecialCargo()));
}


DomainTypes CvUnit::domainCargo() const
{
	return ((DomainTypes)(m_pUnitInfo->getDomainCargo()));
}


int CvUnit::cargoSpace() const
{
	return m_iCargoCapacity;
}

void CvUnit::changeCargoSpace(int iChange)
{
	if (iChange != 0)
	{
		m_iCargoCapacity += iChange;
		FAssert(m_iCargoCapacity >= 0);
		setInfoBarDirty(true);
	}
}

bool CvUnit::isFull() const
{
	return (getCargo() >= cargoSpace());
}


int CvUnit::cargoSpaceAvailable(SpecialUnitTypes eSpecialCargo, DomainTypes eDomainCargo) const
{
	if (specialCargo() != NO_SPECIALUNIT)
	{
		if (specialCargo() != eSpecialCargo)
		{
			return 0;
		}
	}

	if (domainCargo() != NO_DOMAIN)
	{
		if (domainCargo() != eDomainCargo)
		{
			return 0;
		}
	}

	return std::max(0, (cargoSpace() - getCargo()));
}


bool CvUnit::hasCargo() const
{
	return (getCargo() > 0);
}


bool CvUnit::canCargoAllMove() const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;

	pPlot = plot();

	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTransportUnit() == this)
		{
			if (pLoopUnit->getDomainType() == DOMAIN_LAND)
			{
				if (!(pLoopUnit->canMove()))
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool CvUnit::canCargoEnterArea(TeamTypes eTeam, const CvArea* pArea, bool bIgnoreRightOfPassage) const
{
	CvPlot* pPlot = plot();

	CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTransportUnit() == this)
		{
			if (!pLoopUnit->canEnterArea(eTeam, pArea, bIgnoreRightOfPassage))
			{
				return false;
			}
		}
	}

	return true;
}

int CvUnit::getUnitAICargo(UnitAITypes eUnitAI) const
{
	int iCount = 0;

	std::vector<CvUnit*> aCargoUnits;
	getCargoUnits(aCargoUnits);
	for (uint i = 0; i < aCargoUnits.size(); ++i)
	{
		if (aCargoUnits[i]->AI_getUnitAIType() == eUnitAI)
		{
			++iCount;
		}
	}

	return iCount;
}


int CvUnit::getID() const
{
	return m_iID;
}


int CvUnit::getIndex() const
{
	return (getID() & FLTA_INDEX_MASK);
}


IDInfo CvUnit::getIDInfo() const
{
	IDInfo unit(getOwnerINLINE(), getID());
	return unit;
}


void CvUnit::setID(int iID)
{
	m_iID = iID;
}


int CvUnit::getGroupID() const
{
	return m_iGroupID;
}


bool CvUnit::isInGroup() const
{
	return(getGroupID() != FFreeList::INVALID_INDEX);
}


bool CvUnit::isGroupHead() const // XXX is this used???
{
	return (getGroup()->getHeadUnit() == this);
}


CvSelectionGroup* CvUnit::getGroup() const
{
	return GET_PLAYER(getOwnerINLINE()).getSelectionGroup(getGroupID());
}


bool CvUnit::canJoinGroup(const CvPlot* pPlot, CvSelectionGroup* pSelectionGroup) const
{
	CvUnit* pHeadUnit;
	
	// do not allow someone to join a group that is about to be split apart
	// this prevents a case of a never-ending turn
	if (pSelectionGroup->AI_isForceSeparate())
	{
		return false;
	}
	
	if (pSelectionGroup->getOwnerINLINE() == NO_PLAYER)
	{
		pHeadUnit = pSelectionGroup->getHeadUnit();

		if (pHeadUnit != NULL)
		{
			if (pHeadUnit->getOwnerINLINE() != getOwnerINLINE())
			{
				return false;
			}
		}
	}
	else
	{
		if (pSelectionGroup->getOwnerINLINE() != getOwnerINLINE())
		{
			return false;
		}
	}

	if (pSelectionGroup->getNumUnits() > 0)
	{
		if (!(pSelectionGroup->atPlot(pPlot)))
		{
			return false;
		}

		if (pSelectionGroup->getDomainType() != getDomainType())
		{
			return false;
		}
	}

	return true;
}


void CvUnit::joinGroup(CvSelectionGroup* pSelectionGroup, bool bRemoveSelected, bool bRejoin)
{
	CvSelectionGroup* pOldSelectionGroup;
	CvSelectionGroup* pNewSelectionGroup;
	CvPlot* pPlot;

	pOldSelectionGroup = GET_PLAYER(getOwnerINLINE()).getSelectionGroup(getGroupID());

	if ((pSelectionGroup != pOldSelectionGroup) || (pOldSelectionGroup == NULL))
	{
		pPlot = plot();

		if (pSelectionGroup != NULL)
		{
			pNewSelectionGroup = pSelectionGroup;
		}
		else
		{
			if (bRejoin)
			{
				pNewSelectionGroup = GET_PLAYER(getOwnerINLINE()).addSelectionGroup();
				pNewSelectionGroup->init(pNewSelectionGroup->getID(), getOwnerINLINE());
			}
			else
			{
				pNewSelectionGroup = NULL;
			}
		}

		if ((pNewSelectionGroup == NULL) || canJoinGroup(pPlot, pNewSelectionGroup))
		{
			if (pOldSelectionGroup != NULL)
			{
				bool bWasHead = false;
				if (!isHuman())
				{
					if (pOldSelectionGroup->getNumUnits() > 1)
					{
						if (pOldSelectionGroup->getHeadUnit() == this)
						{
							bWasHead = true;
						}
					}
				}

				pOldSelectionGroup->removeUnit(this);

				// if we were the head, if the head unitAI changed, then force the group to separate (non-humans)
				if (bWasHead)
				{
					FAssert(pOldSelectionGroup->getHeadUnit() != NULL);
					if (pOldSelectionGroup->getHeadUnit()->AI_getUnitAIType() != AI_getUnitAIType())
					{
						pOldSelectionGroup->AI_makeForceSeparate();
					}
				}
			}

			if ((pNewSelectionGroup != NULL) && pNewSelectionGroup->addUnit(this, false))
			{
				m_iGroupID = pNewSelectionGroup->getID();
			}
			else
			{
				m_iGroupID = FFreeList::INVALID_INDEX;
			}

			if (getGroup() != NULL)
			{
				if (getGroup()->getNumUnits() > 1)
				{
					getGroup()->setActivityType(ACTIVITY_AWAKE);
				}
				else
				{
					GET_PLAYER(getOwnerINLINE()).updateGroupCycle(this);
				}
			}

			if (getTeam() == GC.getGameINLINE().getActiveTeam())
			{
				if (pPlot != NULL)
				{
					pPlot->setFlagDirty(true);
				}
			}

			if (pPlot == gDLL->getInterfaceIFace()->getSelectionPlot())
			{
				gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
			}
		}

		if (bRemoveSelected)
		{
			if (IsSelected())
			{
				gDLL->getInterfaceIFace()->removeFromSelectionList(this);
			}
		}
	}
}


int CvUnit::getHotKeyNumber()
{
	return m_iHotKeyNumber;
}


void CvUnit::setHotKeyNumber(int iNewValue)
{
	CvUnit* pLoopUnit;
	int iLoop;

	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (getHotKeyNumber() != iNewValue)
	{
		if (iNewValue != -1)
		{
			for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
			{
				if (pLoopUnit->getHotKeyNumber() == iNewValue)
				{
					pLoopUnit->setHotKeyNumber(-1);
				}
			}
		}

		m_iHotKeyNumber = iNewValue;

		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
		}
	}
}


int CvUnit::getX() const
{
	return m_iX;
}


int CvUnit::getY() const
{
	return m_iY;
}


void CvUnit::setXY(int iX, int iY, bool bGroup, bool bUpdate, bool bShow, bool bCheckPlotVisible)
{
	CLLNode<IDInfo>* pUnitNode;
	CvCity* pOldCity;
	CvCity* pNewCity;
	CvCity* pWorkingCity;
	CvUnit* pTransportUnit;
	CvUnit* pLoopUnit;
	CvPlot* pOldPlot;
	CvPlot* pNewPlot;
	CvPlot* pLoopPlot;
	CLinkList<IDInfo> oldUnits;
	ActivityTypes eOldActivityType;
	int iI;

	// OOS!! Temporary for Out-of-Sync madness debugging...
	if (GC.getLogging())
	{
		if (gDLL->getChtLvl() > 0)
		{
			char szOut[1024];
			sprintf(szOut, "Player %d Unit %d (%S's %S) moving from %d:%d to %d:%d\n", getOwnerINLINE(), getID(), GET_PLAYER(getOwnerINLINE()).getNameKey(), getName().GetCString(), getX_INLINE(), getY_INLINE(), iX, iY);
			gDLL->messageControlLog(szOut);
		}
	}

	FAssert(!at(iX, iY));
	FAssert(!isFighting());
	FAssert((iX == INVALID_PLOT_COORD) || (GC.getMapINLINE().plotINLINE(iX, iY)->getX_INLINE() == iX));
	FAssert((iY == INVALID_PLOT_COORD) || (GC.getMapINLINE().plotINLINE(iX, iY)->getY_INLINE() == iY));

	if (getGroup() != NULL)
	{
		eOldActivityType = getGroup()->getActivityType();
	}
	else
	{
		eOldActivityType = NO_ACTIVITY;
	}

	setBlockading(false);

	if (!bGroup || isCargo())
	{
		joinGroup(NULL, true);
		bShow = false;
	}

	pNewPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (pNewPlot != NULL)
	{
		pTransportUnit = getTransportUnit();

		if (pTransportUnit != NULL)
		{
			if (!(pTransportUnit->atPlot(pNewPlot)))
			{
				setTransportUnit(NULL);
			}
		}

		if (canFight())
		{
			oldUnits.clear();

			pUnitNode = pNewPlot->headUnitNode();

			while (pUnitNode != NULL)
			{
				oldUnits.insertAtEnd(pUnitNode->m_data);
				pUnitNode = pNewPlot->nextUnitNode(pUnitNode);
			}

			pUnitNode = oldUnits.head();

			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = oldUnits.next(pUnitNode);

				if (pLoopUnit != NULL)
				{
					if (isEnemy(pLoopUnit->getTeam(), pNewPlot) || pLoopUnit->isEnemy(getTeam()))
					{
						if (!pLoopUnit->canCoexistWithEnemyUnit(getTeam()))
						{
							if (NO_UNITCLASS == pLoopUnit->getUnitInfo().getUnitCaptureClassType() && pLoopUnit->canDefend(pNewPlot))
							{
								pLoopUnit->jumpToNearestValidPlot(); // can kill unit
							}
							else
							{
								if (!m_pUnitInfo->isHiddenNationality() && !pLoopUnit->getUnitInfo().isHiddenNationality())
								{
									GET_TEAM(pLoopUnit->getTeam()).changeWarWeariness(getTeam(), *pNewPlot, GC.getDefineINT("WW_UNIT_CAPTURED"));
									GET_TEAM(getTeam()).changeWarWeariness(pLoopUnit->getTeam(), *pNewPlot, GC.getDefineINT("WW_CAPTURED_UNIT"));
									GET_TEAM(getTeam()).AI_changeWarSuccess(pLoopUnit->getTeam(), GC.getDefineINT("WAR_SUCCESS_UNIT_CAPTURING"));
								}

								if (!isNoCapture())
								{
									pLoopUnit->setCapturingPlayer(getOwnerINLINE());
								}

								pLoopUnit->kill(false, getOwnerINLINE());
							}
						}
					}
				}
			}
		}

		if (pNewPlot->isGoody(getTeam()))
		{
			GET_PLAYER(getOwnerINLINE()).doGoody(pNewPlot, this);
		}
	}

	pOldPlot = plot();

	if (pOldPlot != NULL)
	{
		pOldPlot->removeUnit(this, bUpdate && !hasCargo());

		pOldPlot->changeAdjacentSight(getTeam(), visibilityRange(), false, this, true);

		pOldPlot->area()->changeUnitsPerPlayer(getOwnerINLINE(), -1);
		pOldPlot->area()->changePower(getOwnerINLINE(), -(m_pUnitInfo->getPowerValue()));

		if (AI_getUnitAIType() != NO_UNITAI)
		{
			pOldPlot->area()->changeNumAIUnits(getOwnerINLINE(), AI_getUnitAIType(), -1);
		}

		if (isAnimal())
		{
			pOldPlot->area()->changeAnimalsPerPlayer(getOwnerINLINE(), -1);
		}

		if (pOldPlot->getTeam() != getTeam() && (pOldPlot->getTeam() == NO_TEAM || !GET_TEAM(pOldPlot->getTeam()).isVassal(getTeam())))
		{
			GET_PLAYER(getOwnerINLINE()).changeNumOutsideUnits(-1);
		}

		setLastMoveTurn(GC.getGameINLINE().getTurnSlice());

		pOldCity = pOldPlot->getPlotCity();

		if (pOldCity != NULL)
		{
			if (isMilitaryHappiness())
			{
				pOldCity->changeMilitaryHappinessUnits(-1);
			}
		}

		pWorkingCity = pOldPlot->getWorkingCity();

		if (pWorkingCity != NULL)
		{
			if (canSiege(pWorkingCity->getTeam()))
			{
				pWorkingCity->AI_setAssignWorkDirty(true);
			}
		}

		if (pOldPlot->isWater())
		{
			for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
			{
				pLoopPlot = plotDirection(pOldPlot->getX_INLINE(), pOldPlot->getY_INLINE(), ((DirectionTypes)iI));

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->isWater())
					{
						pWorkingCity = pLoopPlot->getWorkingCity();

						if (pWorkingCity != NULL)
						{
							if (canSiege(pWorkingCity->getTeam()))
							{
								pWorkingCity->AI_setAssignWorkDirty(true);
							}
						}
					}
				}
			}
		}

		if (pOldPlot->isActiveVisible(true))
		{
			pOldPlot->updateMinimapColor();
		}

		if (pOldPlot == gDLL->getInterfaceIFace()->getSelectionPlot())
		{
			gDLL->getInterfaceIFace()->verifyPlotListColumn();

			gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
		}
	}

	if (pNewPlot != NULL)
	{
		m_iX = pNewPlot->getX_INLINE();
		m_iY = pNewPlot->getY_INLINE();
	}
	else
	{
		m_iX = INVALID_PLOT_COORD;
		m_iY = INVALID_PLOT_COORD;
	}

	FAssertMsg(plot() == pNewPlot, "plot is expected to equal pNewPlot");

	if (pNewPlot != NULL)
	{
		pNewCity = pNewPlot->getPlotCity();

		if (pNewCity != NULL)
		{
			if (isEnemy(pNewCity->getTeam()) && !canCoexistWithEnemyUnit(pNewCity->getTeam()) && canFight())
			{
				GET_TEAM(getTeam()).changeWarWeariness(pNewCity->getTeam(), *pNewPlot, GC.getDefineINT("WW_CAPTURED_CITY"));
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      06/14/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
/* original bts code
				GET_TEAM(getTeam()).AI_changeWarSuccess(pNewCity->getTeam(), GC.getDefineINT("WAR_SUCCESS_CITY_CAPTURING"));
*/
				// Double war success if capturing capital city, always a significant blow to enemy
				// pNewCity still points to old city here, hasn't been acquired yet
				GET_TEAM(getTeam()).AI_changeWarSuccess(pNewCity->getTeam(), (pNewCity->isCapital() ? 2 : 1)*GC.getWAR_SUCCESS_CITY_CAPTURING());
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

				PlayerTypes eNewOwner = GET_PLAYER(getOwnerINLINE()).pickConqueredCityOwner(*pNewCity);

				if (NO_PLAYER != eNewOwner)
				{
					GET_PLAYER(eNewOwner).acquireCity(pNewCity, true, false, true); // will delete the pointer
					pNewCity = NULL;
				}
			}
		}
/************************************************************************************************/
/* Afforess	                  Start		 02/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		if (pNewPlot->isActsAsCity() && GC.getGameINLINE().isOption(GAMEOPTION_FIXED_BORDERS))
		{
			bool bDoAcquireFort = false;
			
			if (pNewPlot->getOwnerINLINE() == NO_PLAYER)
			{
				bDoAcquireFort = true;
			}
			else
			{
				CvPlayer& pNewPlotOwner = GET_PLAYER(pNewPlot->getOwnerINLINE());
				if ((isEnemy(pNewPlotOwner.getTeam()) || !pNewPlotOwner.isAlive()) && !canCoexistWithEnemyUnit(pNewPlotOwner.getTeam()) && canFight())
				{
					bDoAcquireFort = true;
				}				
			}
			
			if (bDoAcquireFort)
			{
				GET_PLAYER(getOwnerINLINE()).acquireFort(pNewPlot);
			}
		}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

		//update facing direction
		if(pOldPlot != NULL)
		{
			DirectionTypes newDirection = estimateDirection(pOldPlot, pNewPlot);
			if(newDirection != NO_DIRECTION)
				m_eFacingDirection = newDirection;
		}

		//update cargo mission animations
		if (isCargo())
		{
			if (eOldActivityType != ACTIVITY_MISSION)
			{
				getGroup()->setActivityType(eOldActivityType);
			}
		}

		setFortifyTurns(0);

		pNewPlot->changeAdjacentSight(getTeam(), visibilityRange(), true, this, true); // needs to be here so that the square is considered visible when we move into it...

		pNewPlot->addUnit(this, bUpdate && !hasCargo());

		pNewPlot->area()->changeUnitsPerPlayer(getOwnerINLINE(), 1);
		pNewPlot->area()->changePower(getOwnerINLINE(), m_pUnitInfo->getPowerValue());

		if (AI_getUnitAIType() != NO_UNITAI)
		{
			pNewPlot->area()->changeNumAIUnits(getOwnerINLINE(), AI_getUnitAIType(), 1);
		}

		if (isAnimal())
		{
			pNewPlot->area()->changeAnimalsPerPlayer(getOwnerINLINE(), 1);
		}

		if (pNewPlot->getTeam() != getTeam() && (pNewPlot->getTeam() == NO_TEAM || !GET_TEAM(pNewPlot->getTeam()).isVassal(getTeam())))
		{
			GET_PLAYER(getOwnerINLINE()).changeNumOutsideUnits(1);
		}

		if (shouldLoadOnMove(pNewPlot))
		{
			load();
		}

		for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (!isInvisible(((TeamTypes)iI), false))
				{
					if (pNewPlot->isVisible((TeamTypes)iI, false))
					{
						GET_TEAM((TeamTypes)iI).meet(getTeam(), true);
					}
				}
			}
		}

		pNewCity = pNewPlot->getPlotCity();

		if (pNewCity != NULL)
		{
			if (isMilitaryHappiness())
			{
				pNewCity->changeMilitaryHappinessUnits(1);
			}
		}

		pWorkingCity = pNewPlot->getWorkingCity();

		if (pWorkingCity != NULL)
		{
			if (canSiege(pWorkingCity->getTeam()))
			{
				pWorkingCity->verifyWorkingPlot(pWorkingCity->getCityPlotIndex(pNewPlot));
			}
		}

		if (pNewPlot->isWater())
		{
			for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
			{
				pLoopPlot = plotDirection(pNewPlot->getX_INLINE(), pNewPlot->getY_INLINE(), ((DirectionTypes)iI));

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->isWater())
					{
						pWorkingCity = pLoopPlot->getWorkingCity();

						if (pWorkingCity != NULL)
						{
							if (canSiege(pWorkingCity->getTeam()))
							{
								pWorkingCity->verifyWorkingPlot(pWorkingCity->getCityPlotIndex(pLoopPlot));
							}
						}
					}
				}
			}
		}

		if (pNewPlot->isActiveVisible(true))
		{
			pNewPlot->updateMinimapColor();
		}

		if (GC.IsGraphicsInitialized())
		{
			//override bShow if check plot visible
			if(bCheckPlotVisible && pNewPlot->isVisibleToWatchingHuman())
				bShow = true;

			if (bShow)
			{
				QueueMove(pNewPlot);
			}
			else
			{
				SetPosition(pNewPlot);
			}
		}

		if (pNewPlot == gDLL->getInterfaceIFace()->getSelectionPlot())
		{
			gDLL->getInterfaceIFace()->verifyPlotListColumn();

			gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
		}
	}

	if (pOldPlot != NULL)
	{
		if (hasCargo())
		{
			pUnitNode = pOldPlot->headUnitNode();

			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = pOldPlot->nextUnitNode(pUnitNode);

				if (pLoopUnit->getTransportUnit() == this)
				{
					pLoopUnit->setXY(iX, iY, bGroup, false);
				}
			}
		}
	}

	if (bUpdate && hasCargo())
	{
		if (pOldPlot != NULL)
		{
			pOldPlot->updateCenterUnit();
			pOldPlot->setFlagDirty(true);
		}

		if (pNewPlot != NULL)
		{
			pNewPlot->updateCenterUnit();
			pNewPlot->setFlagDirty(true);
		}
	}

	FAssert(pOldPlot != pNewPlot);
	GET_PLAYER(getOwnerINLINE()).updateGroupCycle(this);

	setInfoBarDirty(true);

	if (IsSelected())
	{
		if (isFound())
		{
			gDLL->getInterfaceIFace()->setDirty(GlobeLayer_DIRTY_BIT, true);
			gDLL->getEngineIFace()->updateFoundingBorder();
		}

		gDLL->getInterfaceIFace()->setDirty(ColoredPlots_DIRTY_BIT, true);
	}

	//update glow
	if (pNewPlot != NULL)
	{
		gDLL->getEntityIFace()->updateEnemyGlow(getUnitEntity());
	}

	// report event to Python, along with some other key state
	CvEventReporter::getInstance().unitSetXY(pNewPlot, this);
}


bool CvUnit::at(int iX, int iY) const
{
	return((getX_INLINE() == iX) && (getY_INLINE() == iY));
}


bool CvUnit::atPlot(const CvPlot* pPlot) const
{
	return (plot() == pPlot);
}


CvPlot* CvUnit::plot() const
{
	return GC.getMapINLINE().plotSorenINLINE(getX_INLINE(), getY_INLINE());
}


int CvUnit::getArea() const
{
	return plot()->getArea();
}


CvArea* CvUnit::area() const
{
	return plot()->area();
}


bool CvUnit::onMap() const
{
	return (plot() != NULL);
}


int CvUnit::getLastMoveTurn() const
{
	return m_iLastMoveTurn;
}


void CvUnit::setLastMoveTurn(int iNewValue)
{
	m_iLastMoveTurn = iNewValue;
	FAssert(getLastMoveTurn() >= 0);
}


CvPlot* CvUnit::getReconPlot() const
{
	return GC.getMapINLINE().plotSorenINLINE(m_iReconX, m_iReconY);
}


void CvUnit::setReconPlot(CvPlot* pNewValue)
{
	CvPlot* pOldPlot;

	pOldPlot = getReconPlot();

	if (pOldPlot != pNewValue)
	{
		if (pOldPlot != NULL)
		{
			pOldPlot->changeAdjacentSight(getTeam(), GC.getDefineINT("RECON_VISIBILITY_RANGE"), false, this, true);
			pOldPlot->changeReconCount(-1); // changeAdjacentSight() tests for getReconCount()
		}

		if (pNewValue == NULL)
		{
			m_iReconX = INVALID_PLOT_COORD;
			m_iReconY = INVALID_PLOT_COORD;
		}
		else
		{
			m_iReconX = pNewValue->getX_INLINE();
			m_iReconY = pNewValue->getY_INLINE();

			pNewValue->changeReconCount(1); // changeAdjacentSight() tests for getReconCount()
			pNewValue->changeAdjacentSight(getTeam(), GC.getDefineINT("RECON_VISIBILITY_RANGE"), true, this, true);
		}
	}
}


int CvUnit::getGameTurnCreated() const
{
	return m_iGameTurnCreated;
}


void CvUnit::setGameTurnCreated(int iNewValue)
{
	m_iGameTurnCreated = iNewValue;
	FAssert(getGameTurnCreated() >= 0);
}


int CvUnit::getDamage() const
{
	return m_iDamage;
}


void CvUnit::setDamage(int iNewValue, PlayerTypes ePlayer, bool bNotifyEntity)
{
	int iOldValue;

	iOldValue = getDamage();

	m_iDamage = range(iNewValue, 0, maxHitPoints());

	FAssertMsg(currHitPoints() >= 0, "currHitPoints() is expected to be non-negative (invalid Index)");

	if (iOldValue != getDamage())
	{
		if (GC.getGameINLINE().isFinalInitialized() && bNotifyEntity)
		{
			NotifyEntity(MISSION_DAMAGE);
		}

		setInfoBarDirty(true);

		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
		}

		if (plot() == gDLL->getInterfaceIFace()->getSelectionPlot())
		{
			gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
		}
	}

	if (isDead())
	{
		kill(true, ePlayer);
	}
}


void CvUnit::changeDamage(int iChange, PlayerTypes ePlayer) 
{
	setDamage((getDamage() + iChange), ePlayer);
}


int CvUnit::getMoves() const
{
/************************************************************************************************/
/* Afforess	                  Start		 12/13/09                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	int iMoves = m_iMoves;

	iMoves *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitMovementPercent() + 100;
	iMoves /= 100;
	
	return iMoves;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
}


void CvUnit::setMoves(int iNewValue)
{
	CvPlot* pPlot;

	if (getMoves() != iNewValue)
	{
		pPlot = plot();

		m_iMoves = iNewValue;

		FAssert(getMoves() >= 0);

		if (getTeam() == GC.getGameINLINE().getActiveTeam())
		{
			if (pPlot != NULL)
			{
				pPlot->setFlagDirty(true);
			}
		}

		if (IsSelected())
		{
			gDLL->getFAStarIFace()->ForceReset(&GC.getInterfacePathFinder());

			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
		}

		if (pPlot == gDLL->getInterfaceIFace()->getSelectionPlot())
		{
			gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
		}
	}
}


void CvUnit::changeMoves(int iChange)														
{
	setMoves(getMoves() + iChange);
}


void CvUnit::finishMoves()																			
{
	setMoves(maxMoves());
}

/************************************************************************************************/
/* Afforess	                  Start		 04/25/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
int CvUnit::getExperience100() const
{
	return m_iExperience;
}

void CvUnit::setExperience100(int iNewValue, int iMax)
{
	if ((getExperience100() != iNewValue) && (getExperience100() < ((iMax == -1) ? MAX_INT : iMax)))
	{
		m_iExperience = std::min(((iMax == -1) ? MAX_INT : iMax), iNewValue);
		FAssert(getExperience100() >= 0);
		testPromotionReady();

		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
		}
	}
}

void CvUnit::changeExperience100(int iChange, int iMax, bool bFromCombat, bool bInBorders, bool bUpdateGlobal)
{
	int iUnitExperience = iChange;

	if (bFromCombat)
	{
		CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());

		int iCombatExperienceMod = 100 + kPlayer.getGreatGeneralRateModifier();

		if (bInBorders)
		{
			iCombatExperienceMod += kPlayer.getDomesticGreatGeneralRateModifier() + kPlayer.getExpInBorderModifier();
			iUnitExperience += (iChange * kPlayer.getExpInBorderModifier()) / 100;
		}

		if (bUpdateGlobal)
		{
			kPlayer.changeFractionalCombatExperience((iChange * iCombatExperienceMod) / 100);
		}

		if (getExperiencePercent() != 0)
		{
			iUnitExperience *= std::max(0, 100 + getExperiencePercent());
			iUnitExperience /= 100;
		}
		
/* Great Commanders                                                                             */
		if (m_pUsedCommander != NULL && bFromCombat)
		{
			m_pUsedCommander->setExperience100(m_pUsedCommander->getExperience100() + 100);	//1 xp every time
		}
	}

	setExperience100((getExperience100() + iUnitExperience), iMax);
}

int CvUnit::getExperience() const
{
	return getExperience100() / 100;
}

void CvUnit::setExperience(int iNewValue, int iMax)
{
	setExperience100(iNewValue * 100, iMax > 0 && iMax != MAX_INT ? iMax * 100 : -1);
}

void CvUnit::changeExperience(int iChange, int iMax, bool bFromCombat, bool bInBorders, bool bUpdateGlobal)
{
	changeExperience100(iChange * 100, iMax > 0 && iMax != MAX_INT ? iMax * 100 : -1, bFromCombat, bInBorders, bUpdateGlobal);
}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
int CvUnit::getLevel() const
{
	return m_iLevel;
}

void CvUnit::setLevel(int iNewValue)
{
	if (getLevel() != iNewValue)
	{
		m_iLevel = iNewValue;
		FAssert(getLevel() >= 0);

		if (getLevel() > GET_PLAYER(getOwnerINLINE()).getHighestUnitLevel())
		{
			GET_PLAYER(getOwnerINLINE()).setHighestUnitLevel(getLevel());
		}

		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
		}
	}
}

void CvUnit::changeLevel(int iChange)
{
	setLevel(getLevel() + iChange);
}

int CvUnit::getCargo() const
{
	return m_iCargo;
}

void CvUnit::changeCargo(int iChange)																
{
	m_iCargo += iChange;
	FAssert(getCargo() >= 0);
}

void CvUnit::getCargoUnits(std::vector<CvUnit*>& aUnits) const
{
	aUnits.clear();

	if (hasCargo())
	{
		CvPlot* pPlot = plot();
		CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
		while (pUnitNode != NULL)
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);
			if (pLoopUnit->getTransportUnit() == this)
			{
				aUnits.push_back(pLoopUnit);
			}
		}
	}

	FAssert(getCargo() == aUnits.size());
}

CvPlot* CvUnit::getAttackPlot() const
{
	return GC.getMapINLINE().plotSorenINLINE(m_iAttackPlotX, m_iAttackPlotY);
}


void CvUnit::setAttackPlot(const CvPlot* pNewValue, bool bAirCombat)
{
	if (getAttackPlot() != pNewValue)
	{
		if (pNewValue != NULL)
		{
			m_iAttackPlotX = pNewValue->getX_INLINE();
			m_iAttackPlotY = pNewValue->getY_INLINE();
		}
		else
		{
			m_iAttackPlotX = INVALID_PLOT_COORD;
			m_iAttackPlotY = INVALID_PLOT_COORD;
		}
	}

	m_bAirCombat = bAirCombat;
}

bool CvUnit::isAirCombat() const
{
	return m_bAirCombat;
}

int CvUnit::getCombatTimer() const
{
	return m_iCombatTimer;
}

void CvUnit::setCombatTimer(int iNewValue)			
{
	m_iCombatTimer = iNewValue;
	FAssert(getCombatTimer() >= 0);
}

void CvUnit::changeCombatTimer(int iChange)			
{
	setCombatTimer(getCombatTimer() + iChange);
}

int CvUnit::getCombatFirstStrikes() const
{
	return m_iCombatFirstStrikes;
}

void CvUnit::setCombatFirstStrikes(int iNewValue)			
{
	m_iCombatFirstStrikes = iNewValue;
	FAssert(getCombatFirstStrikes() >= 0);
}

void CvUnit::changeCombatFirstStrikes(int iChange)			
{
	setCombatFirstStrikes(getCombatFirstStrikes() + iChange);
}

int CvUnit::getFortifyTurns() const
{
	return m_iFortifyTurns;
}

void CvUnit::setFortifyTurns(int iNewValue)
{
	iNewValue = range(iNewValue, 0, GC.getDefineINT("MAX_FORTIFY_TURNS"));

	if (iNewValue != getFortifyTurns())
	{
		m_iFortifyTurns = iNewValue;
		setInfoBarDirty(true);
	}
}

void CvUnit::changeFortifyTurns(int iChange)
{
	setFortifyTurns(getFortifyTurns() + iChange);
}

int CvUnit::getBlitzCount() const
{
	return m_iBlitzCount;
}

bool CvUnit::isBlitz() const
{
	return (getBlitzCount() > 0);
}

void CvUnit::changeBlitzCount(int iChange)			
{
	m_iBlitzCount += iChange;
	FAssert(getBlitzCount() >= 0);
}

int CvUnit::getAmphibCount() const
{
	return m_iAmphibCount;
}

bool CvUnit::isAmphib() const
{
	return (getAmphibCount() > 0);
}

void CvUnit::changeAmphibCount(int iChange)
{
	m_iAmphibCount += iChange;
	FAssert(getAmphibCount() >= 0);
}

int CvUnit::getRiverCount() const
{
	return m_iRiverCount;
}

bool CvUnit::isRiver() const
{
	return (getRiverCount() > 0);
}

void CvUnit::changeRiverCount(int iChange)
{
	m_iRiverCount += iChange;
	FAssert(getRiverCount() >= 0);
}

int CvUnit::getEnemyRouteCount() const
{
	return m_iEnemyRouteCount;
}

bool CvUnit::isEnemyRoute() const
{
	return (getEnemyRouteCount() > 0);
}

void CvUnit::changeEnemyRouteCount(int iChange)
{
	m_iEnemyRouteCount += iChange;
	FAssert(getEnemyRouteCount() >= 0);
}

int CvUnit::getAlwaysHealCount() const
{
	return m_iAlwaysHealCount;
}

bool CvUnit::isAlwaysHeal() const
{
	return (getAlwaysHealCount() > 0);
}

void CvUnit::changeAlwaysHealCount(int iChange)
{
	m_iAlwaysHealCount += iChange;
	FAssert(getAlwaysHealCount() >= 0);
}

int CvUnit::getHillsDoubleMoveCount() const
{
	return m_iHillsDoubleMoveCount;
}

bool CvUnit::isHillsDoubleMove() const
{
	return (getHillsDoubleMoveCount() > 0);
}

void CvUnit::changeHillsDoubleMoveCount(int iChange)
{
	m_iHillsDoubleMoveCount += iChange;
	FAssert(getHillsDoubleMoveCount() >= 0);
}

int CvUnit::getImmuneToFirstStrikesCount() const
{
	return m_iImmuneToFirstStrikesCount;
}

void CvUnit::changeImmuneToFirstStrikesCount(int iChange)
{
	m_iImmuneToFirstStrikesCount += iChange;
	FAssert(getImmuneToFirstStrikesCount() >= 0);
}


int CvUnit::getExtraVisibilityRange() const																	
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraVisibilityRange + pCommander->getExtraVisibilityRange();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraVisibilityRange;
}

void CvUnit::changeExtraVisibilityRange(int iChange)
{
	if (iChange != 0)
	{
		plot()->changeAdjacentSight(getTeam(), visibilityRange(), false, this, true);

		m_iExtraVisibilityRange += iChange;
		FAssert(getExtraVisibilityRange() >= 0);

		plot()->changeAdjacentSight(getTeam(), visibilityRange(), true, this, true);
	}
}

int CvUnit::getExtraMoves() const												
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraMoves + pCommander->getExtraMoves();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraMoves;
}


void CvUnit::changeExtraMoves(int iChange)			
{
	m_iExtraMoves += iChange;
	FAssert(getExtraMoves() >= 0);
}


int CvUnit::getExtraMoveDiscount() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraMoveDiscount + pCommander->getExtraMoveDiscount();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraMoveDiscount;
}


void CvUnit::changeExtraMoveDiscount(int iChange)			
{
	m_iExtraMoveDiscount += iChange;
	FAssert(getExtraMoveDiscount() >= 0);
}

int CvUnit::getExtraAirRange() const												
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraAirRange + pCommander->getExtraAirRange();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraAirRange;
}

void CvUnit::changeExtraAirRange(int iChange)			
{
	m_iExtraAirRange += iChange;
}

int CvUnit::getExtraIntercept() const												
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraIntercept + pCommander->getExtraIntercept();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraIntercept;
}

void CvUnit::changeExtraIntercept(int iChange)			
{
	m_iExtraIntercept += iChange;
}

int CvUnit::getExtraEvasion() const												
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraEvasion + pCommander->getExtraEvasion();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraEvasion;
}

void CvUnit::changeExtraEvasion(int iChange)			
{
	m_iExtraEvasion += iChange;
}

int CvUnit::getExtraFirstStrikes() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraFirstStrikes + pCommander->getExtraFirstStrikes();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraFirstStrikes;
}

void CvUnit::changeExtraFirstStrikes(int iChange)			
{
	m_iExtraFirstStrikes += iChange;
	FAssert(getExtraFirstStrikes() >= 0);
}

int CvUnit::getExtraChanceFirstStrikes() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraChanceFirstStrikes + pCommander->getExtraChanceFirstStrikes();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraChanceFirstStrikes;
}

void CvUnit::changeExtraChanceFirstStrikes(int iChange)			
{
	m_iExtraChanceFirstStrikes += iChange;
	FAssert(getExtraChanceFirstStrikes() >= 0);
}


int CvUnit::getExtraWithdrawal() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraWithdrawal + pCommander->getExtraWithdrawal();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraWithdrawal;
}


void CvUnit::changeExtraWithdrawal(int iChange)															
{
	m_iExtraWithdrawal += iChange;
	FAssert(getExtraWithdrawal() >= 0);
}

int CvUnit::getExtraCollateralDamage() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraCollateralDamage + pCommander->getExtraCollateralDamage();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraCollateralDamage;
}

void CvUnit::changeExtraCollateralDamage(int iChange)													
{
	m_iExtraCollateralDamage += iChange;
	FAssert(getExtraCollateralDamage() >= 0);
}

int CvUnit::getExtraBombardRate() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraBombardRate + pCommander->getExtraBombardRate();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraBombardRate;
}

void CvUnit::changeExtraBombardRate(int iChange)													
{
	m_iExtraBombardRate += iChange;
	FAssert(getExtraBombardRate() >= 0);
}

int CvUnit::getExtraEnemyHeal() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraEnemyHeal + pCommander->getExtraEnemyHeal();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraEnemyHeal;
}

void CvUnit::changeExtraEnemyHeal(int iChange)						
{
	m_iExtraEnemyHeal += iChange;
	FAssert(getExtraEnemyHeal() >= 0);
}

int CvUnit::getExtraNeutralHeal() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraNeutralHeal + pCommander->getExtraNeutralHeal();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraNeutralHeal;
}

void CvUnit::changeExtraNeutralHeal(int iChange)			
{
	m_iExtraNeutralHeal += iChange;
	FAssert(getExtraNeutralHeal() >= 0);
}

int CvUnit::getExtraFriendlyHeal() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraFriendlyHeal + pCommander->getExtraFriendlyHeal();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraFriendlyHeal;
}


void CvUnit::changeExtraFriendlyHeal(int iChange)			
{
	m_iExtraFriendlyHeal += iChange;
	FAssert(getExtraFriendlyHeal() >= 0);
}

int CvUnit::getSameTileHeal() const
{
	return m_iSameTileHeal;
}

void CvUnit::changeSameTileHeal(int iChange)
{
	m_iSameTileHeal += iChange;
	FAssert(getSameTileHeal() >= 0);
}

int CvUnit::getAdjacentTileHeal() const
{
	return m_iAdjacentTileHeal;
}

void CvUnit::changeAdjacentTileHeal(int iChange)
{
	m_iAdjacentTileHeal += iChange;
	FAssert(getAdjacentTileHeal() >= 0);
}

int CvUnit::getExtraCombatPercent() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraCombatPercent + pCommander->getExtraCombatPercent();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraCombatPercent;
}

void CvUnit::changeExtraCombatPercent(int iChange)
{
	if (iChange != 0)
	{
		m_iExtraCombatPercent += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraCityAttackPercent() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraCityAttackPercent + pCommander->getExtraCityAttackPercent();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraCityAttackPercent;
}

void CvUnit::changeExtraCityAttackPercent(int iChange)
{
	if (iChange != 0)
	{
		m_iExtraCityAttackPercent += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraCityDefensePercent() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraCityDefensePercent + pCommander->getExtraCityDefensePercent();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraCityDefensePercent;
}

void CvUnit::changeExtraCityDefensePercent(int iChange)
{
	if (iChange != 0)
	{
		m_iExtraCityDefensePercent += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraHillsAttackPercent() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraHillsAttackPercent + pCommander->getExtraHillsAttackPercent();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraHillsAttackPercent;
}

void CvUnit::changeExtraHillsAttackPercent(int iChange)
{
	if (iChange != 0)
	{
		m_iExtraHillsAttackPercent += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraHillsDefensePercent() const
{
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_iExtraHillsDefensePercent + pCommander->getExtraHillsDefensePercent();
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExtraHillsDefensePercent;
}

void CvUnit::changeExtraHillsDefensePercent(int iChange)
{
	if (iChange != 0)
	{
		m_iExtraHillsDefensePercent += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getRevoltProtection() const
{
	return m_iRevoltProtection;
}

void CvUnit::changeRevoltProtection(int iChange)
{
	if (iChange != 0)
	{
		m_iRevoltProtection += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getCollateralDamageProtection() const
{
	return m_iCollateralDamageProtection;
}

void CvUnit::changeCollateralDamageProtection(int iChange)
{
	if (iChange != 0)
	{
		m_iCollateralDamageProtection += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getPillageChange() const
{
	return m_iPillageChange;
}

void CvUnit::changePillageChange(int iChange)
{
	if (iChange != 0)
	{
		m_iPillageChange += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getUpgradeDiscount() const
{
	return m_iUpgradeDiscount;
}

void CvUnit::changeUpgradeDiscount(int iChange)
{
	if (iChange != 0)
	{
		m_iUpgradeDiscount += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExperiencePercent() const
{
/************************************************************************************************/
/* Afforess	                  Start		 05/4/10                                                */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander())
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return m_iExperiencePercent + pCommander->getExperiencePercent();
	}
	else
	{
		return 0; //Great Commanders can not gain XP faster
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_iExperiencePercent;
}

void CvUnit::changeExperiencePercent(int iChange)
{
	if (iChange != 0)
	{
		m_iExperiencePercent += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getKamikazePercent() const
{
	return m_iKamikazePercent;
}

void CvUnit::changeKamikazePercent(int iChange)
{
	if (iChange != 0)
	{
		m_iKamikazePercent += iChange;

		setInfoBarDirty(true);
	}
}

DirectionTypes CvUnit::getFacingDirection(bool checkLineOfSightProperty) const
{
	if (checkLineOfSightProperty)
	{
		if (m_pUnitInfo->isLineOfSight())
		{
			return m_eFacingDirection; //only look in facing direction
		}
		else
		{
			return NO_DIRECTION; //look in all directions
		}
	}
	else
	{
		return m_eFacingDirection;
	}
}

void CvUnit::setFacingDirection(DirectionTypes eFacingDirection)
{
	if (eFacingDirection != m_eFacingDirection)
	{
		if (m_pUnitInfo->isLineOfSight())
		{
			//remove old fog
			plot()->changeAdjacentSight(getTeam(), visibilityRange(), false, this, true);

			//change direction
			m_eFacingDirection = eFacingDirection;

			//clear new fog
			plot()->changeAdjacentSight(getTeam(), visibilityRange(), true, this, true);

			gDLL->getInterfaceIFace()->setDirty(ColoredPlots_DIRTY_BIT, true);
		}
		else
		{
			m_eFacingDirection = eFacingDirection;
		}

		//update formation
		NotifyEntity(NO_MISSION);
	}
}

void CvUnit::rotateFacingDirectionClockwise()
{
	//change direction
	DirectionTypes eNewDirection = (DirectionTypes) ((m_eFacingDirection + 1) % NUM_DIRECTION_TYPES);
	setFacingDirection(eNewDirection);
}

void CvUnit::rotateFacingDirectionCounterClockwise()
{
	//change direction
	DirectionTypes eNewDirection = (DirectionTypes) ((m_eFacingDirection + NUM_DIRECTION_TYPES - 1) % NUM_DIRECTION_TYPES);
	setFacingDirection(eNewDirection);
}

int CvUnit::getImmobileTimer() const
{
	return m_iImmobileTimer;
}

void CvUnit::setImmobileTimer(int iNewValue)
{
	if (iNewValue != getImmobileTimer())
	{
		m_iImmobileTimer = iNewValue;

		setInfoBarDirty(true);
	}
}

void CvUnit::changeImmobileTimer(int iChange)
{
	if (iChange != 0)
	{
		setImmobileTimer(std::max(0, getImmobileTimer() + iChange));
	}
}

bool CvUnit::isMadeAttack() const
{
	return m_bMadeAttack;
}


void CvUnit::setMadeAttack(bool bNewValue)
{
	m_bMadeAttack = bNewValue;
}


bool CvUnit::isMadeInterception() const
{
	return m_bMadeInterception;
}


void CvUnit::setMadeInterception(bool bNewValue)
{
	m_bMadeInterception = bNewValue;
}


bool CvUnit::isPromotionReady() const
{
	return m_bPromotionReady;
}


void CvUnit::setPromotionReady(bool bNewValue)
{
	if (isPromotionReady() != bNewValue)
	{
		m_bPromotionReady = bNewValue;

		if (m_bPromotionReady)
		{
/************************************************************************************************/
/* Afforess	                  Start		 08/20/10                                               */
/*                                                                                              */
/* Advanced Automations                                                                         */
/************************************************************************************************/
			if (isAutoPromoting())
			{
				AI_promote();
				setPromotionReady(false);
				testPromotionReady();
			}
			else
			{
				getGroup()->setAutomateType(NO_AUTOMATE);
				getGroup()->clearMissionQueue();
				getGroup()->setActivityType(ACTIVITY_AWAKE);
			}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
		}

		gDLL->getEntityIFace()->showPromotionGlow(getUnitEntity(), bNewValue);

		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		}
	}
}


void CvUnit::testPromotionReady()
{
	setPromotionReady((getExperience() >= experienceNeeded()) && canAcquirePromotionAny());
}


bool CvUnit::isDelayedDeath() const
{
	return m_bDeathDelay;
}


void CvUnit::startDelayedDeath()			
{
	m_bDeathDelay = true;
}


// Returns true if killed...
bool CvUnit::doDelayedDeath()
{
	if (m_bDeathDelay && !isFighting())
	{
		kill(false);
		return true;
	}

	return false;
}


bool CvUnit::isCombatFocus() const
{
	return m_bCombatFocus;
}


bool CvUnit::isInfoBarDirty() const
{
	return m_bInfoBarDirty;
}


void CvUnit::setInfoBarDirty(bool bNewValue)
{
	m_bInfoBarDirty = bNewValue;
}

bool CvUnit::isBlockading() const
{
	return m_bBlockading;
}

void CvUnit::setBlockading(bool bNewValue)
{
	if (bNewValue != isBlockading())
	{
		m_bBlockading = bNewValue;

		updatePlunder(isBlockading() ? 1 : -1, true);
	}
}

void CvUnit::collectBlockadeGold()
{
	if (plot()->getTeam() == getTeam())
	{
		return;
	}

	int iBlockadeRange = GC.getDefineINT("SHIP_BLOCKADE_RANGE");

	for (int i = -iBlockadeRange; i <= iBlockadeRange; ++i)
	{
		for (int j = -iBlockadeRange; j <= iBlockadeRange; ++j)
		{
			CvPlot* pLoopPlot = ::plotXY(getX_INLINE(), getY_INLINE(), i, j);

			if (NULL != pLoopPlot && pLoopPlot->isRevealed(getTeam(), false))
			{
				CvCity* pCity = pLoopPlot->getPlotCity();

				if (NULL != pCity && !pCity->isPlundered() && isEnemy(pCity->getTeam()) && !atWar(pCity->getTeam(), getTeam()))
				{
					if (pCity->area() == area() || pCity->plot()->isAdjacentToArea(area()))
					{
						int iGold = pCity->calculateTradeProfit(pCity) * pCity->getTradeRoutes();
/************************************************************************************************/
/* Afforess	                  Start		 06/20/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
						iGold *= (100 + GET_PLAYER(getOwnerINLINE()).calculateInflationRate());
						iGold /= 100;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
						if (iGold > 0)
						{
							pCity->setPlundered(true);
							GET_PLAYER(getOwnerINLINE()).changeGold(iGold);
							GET_PLAYER(pCity->getOwnerINLINE()).changeGold(-iGold);

							CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_TRADE_ROUTE_PLUNDERED", getNameKey(), pCity->getNameKey(), iGold);
							gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BUILD_BANK", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE());

							szBuffer = gDLL->getText("TXT_KEY_MISC_TRADE_ROUTE_PLUNDER", getNameKey(), pCity->getNameKey(), iGold);
							gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BUILD_BANK", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE());
						}
					}
				}
			}
		}
	}
}


PlayerTypes CvUnit::getOwner() const
{
	return getOwnerINLINE();
}

PlayerTypes CvUnit::getVisualOwner(TeamTypes eForTeam) const
{
	if (NO_TEAM == eForTeam)
	{
		eForTeam = GC.getGameINLINE().getActiveTeam();
	}

	if (getTeam() != eForTeam && eForTeam != BARBARIAN_TEAM)
	{
		if (m_pUnitInfo->isHiddenNationality())
		{
			if (!plot()->isCity(true, getTeam()))
			{
				return BARBARIAN_PLAYER;
			}
		}
	}

	return getOwnerINLINE();
}


PlayerTypes CvUnit::getCombatOwner(TeamTypes eForTeam, const CvPlot* pPlot) const
{
	if (eForTeam != NO_TEAM && getTeam() != eForTeam && eForTeam != BARBARIAN_TEAM)
	{
		if (isAlwaysHostile(pPlot))
		{
			return BARBARIAN_PLAYER;
		}
	}

	return getOwnerINLINE();
}

TeamTypes CvUnit::getTeam() const
{
	return GET_PLAYER(getOwnerINLINE()).getTeam();
}


PlayerTypes CvUnit::getCapturingPlayer() const
{
	return m_eCapturingPlayer;
}


void CvUnit::setCapturingPlayer(PlayerTypes eNewValue)
{
	m_eCapturingPlayer = eNewValue;
}


const UnitTypes CvUnit::getUnitType() const
{
	return m_eUnitType;
}

CvUnitInfo &CvUnit::getUnitInfo() const
{
	return *m_pUnitInfo;
}


UnitClassTypes CvUnit::getUnitClassType() const
{
	return (UnitClassTypes)m_pUnitInfo->getUnitClassType();
}

const UnitTypes CvUnit::getLeaderUnitType() const
{
	return m_eLeaderUnitType;
}

void CvUnit::setLeaderUnitType(UnitTypes leaderUnitType)
{
	if(m_eLeaderUnitType != leaderUnitType)
	{
		m_eLeaderUnitType = leaderUnitType;
		reloadEntity();
	}
}

CvUnit* CvUnit::getCombatUnit() const
{
	return getUnit(m_combatUnit);
}


void CvUnit::setCombatUnit(CvUnit* pCombatUnit, bool bAttacking)
{
	if (isCombatFocus())
	{
		gDLL->getInterfaceIFace()->setCombatFocus(false);
	}

	if (pCombatUnit != NULL)
	{
		if (bAttacking)
		{
			if (GC.getLogging())
			{
				if (gDLL->getChtLvl() > 0)
				{
					// Log info about this combat...
					char szOut[1024];
					sprintf( szOut, "*** KOMBAT!\n     ATTACKER: Player %d Unit %d (%S's %S), CombatStrength=%d\n     DEFENDER: Player %d Unit %d (%S's %S), CombatStrength=%d\n",
						getOwnerINLINE(), getID(), GET_PLAYER(getOwnerINLINE()).getName(), getName().GetCString(), currCombatStr(NULL, NULL),
						pCombatUnit->getOwnerINLINE(), pCombatUnit->getID(), GET_PLAYER(pCombatUnit->getOwnerINLINE()).getName(), pCombatUnit->getName().GetCString(), pCombatUnit->currCombatStr(pCombatUnit->plot(), this));
					gDLL->messageControlLog(szOut);
				}
			}

			if (getDomainType() == DOMAIN_LAND
				&& !m_pUnitInfo->isIgnoreBuildingDefense()
				&& pCombatUnit->plot()->getPlotCity() 
				&& pCombatUnit->plot()->getPlotCity()->getBuildingDefense() > 0 
				&& cityAttackModifier() >= GC.getDefineINT("MIN_CITY_ATTACK_MODIFIER_FOR_SIEGE_TOWER"))
			{
				CvDLLEntity::SetSiegeTower(true);
			}
		}

		FAssertMsg(getCombatUnit() == NULL, "Combat Unit is not expected to be assigned");
		FAssertMsg(!(plot()->isFighting()), "(plot()->isFighting()) did not return false as expected");
		m_bCombatFocus = (bAttacking && !(gDLL->getInterfaceIFace()->isFocusedWidget()) && ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) || ((pCombatUnit->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && !(GC.getGameINLINE().isMPOption(MPOPTION_SIMULTANEOUS_TURNS)))));
		m_combatUnit = pCombatUnit->getIDInfo();
		setCombatFirstStrikes((pCombatUnit->immuneToFirstStrikes()) ? 0 : (firstStrikes() + GC.getGameINLINE().getSorenRandNum(chanceFirstStrikes() + 1, "First Strike")));
	}
	else
	{
		if(getCombatUnit() != NULL)
		{
			FAssertMsg(getCombatUnit() != NULL, "getCombatUnit() is not expected to be equal with NULL");
			FAssertMsg(plot()->isFighting(), "plot()->isFighting is expected to be true");
			m_bCombatFocus = false;
			m_combatUnit.reset();
			setCombatFirstStrikes(0);

			if (IsSelected())
			{
				gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
			}

			if (plot() == gDLL->getInterfaceIFace()->getSelectionPlot())
			{
				gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
			}

			CvDLLEntity::SetSiegeTower(false);
		}
	}

	setCombatTimer(0);
	setInfoBarDirty(true);

	if (isCombatFocus())
	{
		gDLL->getInterfaceIFace()->setCombatFocus(true);
	}
}


CvUnit* CvUnit::getTransportUnit() const
{
	return getUnit(m_transportUnit);
}


bool CvUnit::isCargo() const
{
	return (getTransportUnit() != NULL);
}


void CvUnit::setTransportUnit(CvUnit* pTransportUnit)
{
	CvUnit* pOldTransportUnit;

	pOldTransportUnit = getTransportUnit();

	if (pOldTransportUnit != pTransportUnit)
	{
		if (pOldTransportUnit != NULL)
		{
			pOldTransportUnit->changeCargo(-1);
		}

		if (pTransportUnit != NULL)
		{
			FAssertMsg(pTransportUnit->cargoSpaceAvailable(getSpecialUnitType(), getDomainType()) > 0, "Cargo space is expected to be available");

			joinGroup(NULL, true); // Because what if a group of 3 tries to get in a transport which can hold 2...

			m_transportUnit = pTransportUnit->getIDInfo();

			if (getDomainType() != DOMAIN_AIR)
			{
				getGroup()->setActivityType(ACTIVITY_SLEEP);
			}

			if (GC.getGameINLINE().isFinalInitialized())
			{
				finishMoves();
			}

			pTransportUnit->changeCargo(1);
			pTransportUnit->getGroup()->setActivityType(ACTIVITY_AWAKE);
		}
		else
		{
			m_transportUnit.reset();

			getGroup()->setActivityType(ACTIVITY_AWAKE);
		}

#ifdef _DEBUG
		std::vector<CvUnit*> aCargoUnits;
		if (pOldTransportUnit != NULL)
		{
			pOldTransportUnit->getCargoUnits(aCargoUnits);
		}
		if (pTransportUnit != NULL)
		{
			pTransportUnit->getCargoUnits(aCargoUnits);
		}
#endif

	}
}


int CvUnit::getExtraDomainModifier(DomainTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_DOMAIN_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_aiExtraDomainModifier[eIndex] + pCommander->getExtraDomainModifier(eIndex);
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_aiExtraDomainModifier[eIndex];
}


void CvUnit::changeExtraDomainModifier(DomainTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_DOMAIN_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiExtraDomainModifier[eIndex] = (m_aiExtraDomainModifier[eIndex] + iChange);
}


const CvWString CvUnit::getName(uint uiForm) const
{
	CvWString szBuffer;

	if (m_szName.empty())
	{
		return m_pUnitInfo->getDescription(uiForm);
	}
// BUG - Unit Name - start
	else if (isDescInName())
	{
		return m_szName;
	}
// BUG - Unit Name - end

	szBuffer.Format(L"%s (%s)", m_szName.GetCString(), m_pUnitInfo->getDescription(uiForm));

	return szBuffer;
}

// BUG - Unit Name - start
bool CvUnit::isDescInName() const
{
	return (m_szName.find(m_pUnitInfo->getDescription()) != -1);
}
// BUG - Unit Name - end


const wchar* CvUnit::getNameKey() const
{
	if (m_szName.empty())
	{
		return m_pUnitInfo->getTextKeyWide();
	}
	else
	{
		return m_szName.GetCString();
	}
}


const CvWString& CvUnit::getNameNoDesc() const
{
	return m_szName;
}


void CvUnit::setName(CvWString szNewValue)
{
	gDLL->stripSpecialCharacters(szNewValue);

	m_szName = szNewValue;

	if (IsSelected())
	{
		gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
	}
}


std::string CvUnit::getScriptData() const
{
	return m_szScriptData;
}


void CvUnit::setScriptData(std::string szNewValue)
{
	m_szScriptData = szNewValue;
}


int CvUnit::getTerrainDoubleMoveCount(TerrainTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiTerrainDoubleMoveCount[eIndex];
}


bool CvUnit::isTerrainDoubleMove(TerrainTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return (getTerrainDoubleMoveCount(eIndex) > 0);
}


void CvUnit::changeTerrainDoubleMoveCount(TerrainTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiTerrainDoubleMoveCount[eIndex] = (m_paiTerrainDoubleMoveCount[eIndex] + iChange);
	FAssert(getTerrainDoubleMoveCount(eIndex) >= 0);
}


int CvUnit::getFeatureDoubleMoveCount(FeatureTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiFeatureDoubleMoveCount[eIndex];
}


bool CvUnit::isFeatureDoubleMove(FeatureTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return (getFeatureDoubleMoveCount(eIndex) > 0);
}


void CvUnit::changeFeatureDoubleMoveCount(FeatureTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiFeatureDoubleMoveCount[eIndex] = (m_paiFeatureDoubleMoveCount[eIndex] + iChange);
	FAssert(getFeatureDoubleMoveCount(eIndex) >= 0);
}


int CvUnit::getExtraTerrainAttackPercent(TerrainTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_paiExtraTerrainAttackPercent[eIndex] + pCommander->getExtraTerrainAttackPercent(eIndex);
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_paiExtraTerrainAttackPercent[eIndex];
}


void CvUnit::changeExtraTerrainAttackPercent(TerrainTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_paiExtraTerrainAttackPercent[eIndex] += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraTerrainDefensePercent(TerrainTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_paiExtraTerrainDefensePercent[eIndex] + pCommander->getExtraTerrainDefensePercent(eIndex);
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_paiExtraTerrainDefensePercent[eIndex];
}


void CvUnit::changeExtraTerrainDefensePercent(TerrainTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_paiExtraTerrainDefensePercent[eIndex] += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraFeatureAttackPercent(FeatureTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_paiExtraFeatureAttackPercent[eIndex] + pCommander->getExtraFeatureAttackPercent(eIndex);
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_paiExtraFeatureAttackPercent[eIndex];
}


void CvUnit::changeExtraFeatureAttackPercent(FeatureTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_paiExtraFeatureAttackPercent[eIndex] += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraFeatureDefensePercent(FeatureTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_paiExtraFeatureDefensePercent[eIndex] + pCommander->getExtraFeatureDefensePercent(eIndex);
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_paiExtraFeatureDefensePercent[eIndex];
}


void CvUnit::changeExtraFeatureDefensePercent(FeatureTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_paiExtraFeatureDefensePercent[eIndex] += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraUnitCombatModifier(UnitCombatTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitCombatInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
	if (!isCommander()) //this is not a commander
	{
		CvUnit* pCommander = getCommander();
		if (pCommander != NULL)
			return	m_paiExtraUnitCombatModifier[eIndex] + pCommander->getExtraUnitCombatModifier(eIndex);
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	return m_paiExtraUnitCombatModifier[eIndex];
}


void CvUnit::changeExtraUnitCombatModifier(UnitCombatTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitCombatInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiExtraUnitCombatModifier[eIndex] = (m_paiExtraUnitCombatModifier[eIndex] + iChange);
}


bool CvUnit::canAcquirePromotion(PromotionTypes ePromotion) const
{
	FAssertMsg(ePromotion >= 0, "ePromotion is expected to be non-negative (invalid Index)");
	FAssertMsg(ePromotion < GC.getNumPromotionInfos(), "ePromotion is expected to be within maximum bounds (invalid Index)");

	if (isHasPromotion(ePromotion))
	{
		return false;
	}

	if (GC.getPromotionInfo(ePromotion).getPrereqPromotion() != NO_PROMOTION)
	{
		if (!isHasPromotion((PromotionTypes)(GC.getPromotionInfo(ePromotion).getPrereqPromotion())))
		{
			return false;
		}
	}

	if (GC.getPromotionInfo(ePromotion).getPrereqOrPromotion1() != NO_PROMOTION)
	{
		if (!isHasPromotion((PromotionTypes)(GC.getPromotionInfo(ePromotion).getPrereqOrPromotion1())))
		{
			if ((GC.getPromotionInfo(ePromotion).getPrereqOrPromotion2() == NO_PROMOTION) || !isHasPromotion((PromotionTypes)(GC.getPromotionInfo(ePromotion).getPrereqOrPromotion2())))
			{
				return false;
			}
		}
	}

	if (GC.getPromotionInfo(ePromotion).getTechPrereq() != NO_TECH)
	{
		if (!(GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getPromotionInfo(ePromotion).getTechPrereq()))))
		{
			return false;
		}
	}

/************************************************************************************************/
/* Afforess	                  Start		 01/01/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (GC.getPromotionInfo(ePromotion).getObsoleteTech() != NO_TECH)
	{
		if ((GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getPromotionInfo(ePromotion).getTechPrereq()))))
		{
			return false;
		}
	}
	if (getUnitCombatType() == NO_UNITCOMBAT)
		return false;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	if (GC.getPromotionInfo(ePromotion).getStateReligionPrereq() != NO_RELIGION)
	{
		if (GET_PLAYER(getOwnerINLINE()).getStateReligion() != GC.getPromotionInfo(ePromotion).getStateReligionPrereq())
		{
			return false;
		}
	}

	if (!isPromotionValid(ePromotion))
	{
		return false;
	}

	return true;
}

bool CvUnit::isPromotionValid(PromotionTypes ePromotion) const
{
	if (!::isPromotionValid(ePromotion, getUnitType(), true))
	{
		return false;
	}

/************************************************************************************************/
/* SUPER_SPIES                             05/24/08                                TSheep       */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (isSpy())
	{
		return true;
	}
/************************************************************************************************/
/* SUPER_SPIES                             END                                     TSheep              */
/************************************************************************************************/

	CvPromotionInfo& promotionInfo = GC.getPromotionInfo(ePromotion);

	if (promotionInfo.getWithdrawalChange() + m_pUnitInfo->getWithdrawalProbability() + getExtraWithdrawal() > GC.getDefineINT("MAX_WITHDRAWAL_PROBABILITY"))
	{
		return false;
	}

	if (promotionInfo.getInterceptChange() + maxInterceptionProbability() > GC.getDefineINT("MAX_INTERCEPTION_PROBABILITY"))
	{
		return false;
	}

	if (promotionInfo.getEvasionChange() + evasionProbability() > GC.getDefineINT("MAX_EVASION_PROBABILITY"))
	{
		return false;
	}

	return true;
}


bool CvUnit::canAcquirePromotionAny() const
{
	int iI;

	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (canAcquirePromotion((PromotionTypes)iI))
		{
			return true;
		}
	}

	return false;
}


bool CvUnit::isHasPromotion(PromotionTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumPromotionInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_pabHasPromotion[eIndex];
}


void CvUnit::setHasPromotion(PromotionTypes eIndex, bool bNewValue)
{
	int iChange;
	int iI;

/************************************************************************************************/
/* SUPER_SPIES                             05/24/08                                TSheep       */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	// RevolutionDCM - super spies
	// Disable spy promotions mechanism
	bool canPromote = true;
	if (isSpy()  && !GC.getDefineINT("SS_ENABLED") && !GC.getPromotionInfo(eIndex).isEnemyRoute())//exempt commando promotion
	{
		canPromote = false;
	}

	if (isHasPromotion(eIndex) != bNewValue && canPromote)
	// RevolutionDCM - end
/************************************************************************************************/
/* SUPER_SPIES                             END                                     TSheep       */
/************************************************************************************************/
	{
		m_pabHasPromotion[eIndex] = bNewValue;

		iChange = ((isHasPromotion(eIndex)) ? 1 : -1);

		changeBlitzCount((GC.getPromotionInfo(eIndex).isBlitz()) ? iChange : 0);
		changeAmphibCount((GC.getPromotionInfo(eIndex).isAmphib()) ? iChange : 0);
		changeRiverCount((GC.getPromotionInfo(eIndex).isRiver()) ? iChange : 0);
		changeEnemyRouteCount((GC.getPromotionInfo(eIndex).isEnemyRoute()) ? iChange : 0);
		changeAlwaysHealCount((GC.getPromotionInfo(eIndex).isAlwaysHeal()) ? iChange : 0);
		changeHillsDoubleMoveCount((GC.getPromotionInfo(eIndex).isHillsDoubleMove()) ? iChange : 0);
/************************************************************************************************/
/* Afforess  Mountaineering Promotion                              10/13/09                     */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		changeCanMovePeaksCount((GC.getPromotionInfo(eIndex).isCanMovePeaks()) ? iChange : 0);

/************************************************************************************************/
/* Afforess	                  Start		 03/1/10                       Coded By: KillMePlease   */
/*                                                                                              */
/* Great Commanders                                                                             */
/************************************************************************************************/
		changeExtraControlPoints(GC.getPromotionInfo(eIndex).getControlPoints() * iChange);
		changeExtraCommandRange(GC.getPromotionInfo(eIndex).getCommandRange() * iChange);
		changeOnslaughtCount((GC.getPromotionInfo(eIndex).isOnslaught()) ? iChange : 0);
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/	
		changeImmuneToFirstStrikesCount((GC.getPromotionInfo(eIndex).isImmuneToFirstStrikes()) ? iChange : 0);

		changeExtraVisibilityRange(GC.getPromotionInfo(eIndex).getVisibilityChange() * iChange);
		changeExtraMoves(GC.getPromotionInfo(eIndex).getMovesChange() * iChange);
		changeExtraMoveDiscount(GC.getPromotionInfo(eIndex).getMoveDiscountChange() * iChange);
		changeExtraAirRange(GC.getPromotionInfo(eIndex).getAirRangeChange() * iChange);
		changeExtraIntercept(GC.getPromotionInfo(eIndex).getInterceptChange() * iChange);
		changeExtraEvasion(GC.getPromotionInfo(eIndex).getEvasionChange() * iChange);
		changeExtraFirstStrikes(GC.getPromotionInfo(eIndex).getFirstStrikesChange() * iChange);
		changeExtraChanceFirstStrikes(GC.getPromotionInfo(eIndex).getChanceFirstStrikesChange() * iChange);
		changeExtraWithdrawal(GC.getPromotionInfo(eIndex).getWithdrawalChange() * iChange);
		changeExtraCollateralDamage(GC.getPromotionInfo(eIndex).getCollateralDamageChange() * iChange);
		changeExtraBombardRate(GC.getPromotionInfo(eIndex).getBombardRateChange() * iChange);
		changeExtraEnemyHeal(GC.getPromotionInfo(eIndex).getEnemyHealChange() * iChange);
		changeExtraNeutralHeal(GC.getPromotionInfo(eIndex).getNeutralHealChange() * iChange);
		changeExtraFriendlyHeal(GC.getPromotionInfo(eIndex).getFriendlyHealChange() * iChange);
		changeSameTileHeal(GC.getPromotionInfo(eIndex).getSameTileHealChange() * iChange);
		changeAdjacentTileHeal(GC.getPromotionInfo(eIndex).getAdjacentTileHealChange() * iChange);
		changeExtraCombatPercent(GC.getPromotionInfo(eIndex).getCombatPercent() * iChange);
		changeExtraCityAttackPercent(GC.getPromotionInfo(eIndex).getCityAttackPercent() * iChange);
		changeExtraCityDefensePercent(GC.getPromotionInfo(eIndex).getCityDefensePercent() * iChange);
		changeExtraHillsAttackPercent(GC.getPromotionInfo(eIndex).getHillsAttackPercent() * iChange);
		changeExtraHillsDefensePercent(GC.getPromotionInfo(eIndex).getHillsDefensePercent() * iChange);
		changeRevoltProtection(GC.getPromotionInfo(eIndex).getRevoltProtection() * iChange);
		changeCollateralDamageProtection(GC.getPromotionInfo(eIndex).getCollateralDamageProtection() * iChange);
		changePillageChange(GC.getPromotionInfo(eIndex).getPillageChange() * iChange);
		changeUpgradeDiscount(GC.getPromotionInfo(eIndex).getUpgradeDiscount() * iChange);
		changeExperiencePercent(GC.getPromotionInfo(eIndex).getExperiencePercent() * iChange);
		changeKamikazePercent((GC.getPromotionInfo(eIndex).getKamikazePercent()) * iChange);
		changeCargoSpace(GC.getPromotionInfo(eIndex).getCargoChange() * iChange);
/************************************************************************************************/
/* Afforess Promotion Changes                             12/5/09                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		CvPromotionInfo &kPromotion = GC.getPromotionInfo(eIndex);
		if (bNewValue)
        {
			for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
			{
                if (kPromotion.getNumPromotionOverwrites() > 0)
				{
					for (iI = 0; iI < kPromotion.getNumPromotionOverwrites(); iI++)
					{
						if (isHasPromotion(kPromotion.getPromotionOverwrites(iI)))
						{
							setHasPromotion(kPromotion.getPromotionOverwrites(iI), false);
						}
					}
				}
			}
		}

		int iAsset = m_pUnitInfo->getAssetValue();
		int iMultiplier = 100 + kPromotion.getAssetMultiplier();
		iAsset *= iMultiplier;
		iAsset /= 100;
		iAsset -= m_pUnitInfo->getAssetValue();
		GET_PLAYER(getOwnerINLINE()).changeAssets(iAsset * iChange);

		int iPower = m_pUnitInfo->getPowerValue();
		iMultiplier = 100 + kPromotion.getPowerMultiplier();
		iPower *= iMultiplier;
		iPower /= 100;
		iPower -= m_pUnitInfo->getPowerValue();
		GET_PLAYER(getOwnerINLINE()).changePower(iPower * iChange);
		
		if (kPromotion.isZoneOfControl())
		{
			changeZoneOfControlCount(iChange > 0 ? 1 : -1);
		}
		
		if (kPromotion.getIgnoreTerrainDamage() != NO_TERRAIN)
		{
			changeTerrainProtected((TerrainTypes)kPromotion.getIgnoreTerrainDamage(), iChange);
		}
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	

		for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
		{
			changeExtraTerrainAttackPercent(((TerrainTypes)iI), (GC.getPromotionInfo(eIndex).getTerrainAttackPercent(iI) * iChange));
			changeExtraTerrainDefensePercent(((TerrainTypes)iI), (GC.getPromotionInfo(eIndex).getTerrainDefensePercent(iI) * iChange));
			changeTerrainDoubleMoveCount(((TerrainTypes)iI), ((GC.getPromotionInfo(eIndex).getTerrainDoubleMove(iI)) ? iChange : 0));
		}

		for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
		{
			changeExtraFeatureAttackPercent(((FeatureTypes)iI), (GC.getPromotionInfo(eIndex).getFeatureAttackPercent(iI) * iChange));
			changeExtraFeatureDefensePercent(((FeatureTypes)iI), (GC.getPromotionInfo(eIndex).getFeatureDefensePercent(iI) * iChange));
			changeFeatureDoubleMoveCount(((FeatureTypes)iI), ((GC.getPromotionInfo(eIndex).getFeatureDoubleMove(iI)) ? iChange : 0));
		}

		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			changeExtraUnitCombatModifier(((UnitCombatTypes)iI), (GC.getPromotionInfo(eIndex).getUnitCombatModifierPercent(iI) * iChange));
		}

		for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
		{
			changeExtraDomainModifier(((DomainTypes)iI), (GC.getPromotionInfo(eIndex).getDomainModifierPercent(iI) * iChange));
		}

		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
		}

		//update graphics
		gDLL->getEntityIFace()->updatePromotionLayers(getUnitEntity());
	}
}


int CvUnit::getSubUnitCount() const
{
	return m_pUnitInfo->getGroupSize();
}


int CvUnit::getSubUnitsAlive() const
{
	return getSubUnitsAlive( getDamage());
}


int CvUnit::getSubUnitsAlive(int iDamage) const
{
	if (iDamage >= maxHitPoints())
	{
		return 0;
	}
	else
	{
		return std::max(1, (((getSubUnitCount() * (maxHitPoints() - iDamage)) + (maxHitPoints() / ((getSubUnitCount() * 2) + 1))) / maxHitPoints()));
	}
}
// returns true if unit can initiate a war action with plot (possibly by declaring war)
bool CvUnit::potentialWarAction(const CvPlot* pPlot) const
{
	TeamTypes ePlotTeam = pPlot->getTeam();
	TeamTypes eUnitTeam = getTeam();
	
	if (ePlotTeam == NO_TEAM)
	{
		return false;
	}

	if (isEnemy(ePlotTeam, pPlot))
	{
		return true;
	}
	
	if (getGroup()->AI_isDeclareWar(pPlot) && GET_TEAM(eUnitTeam).AI_getWarPlan(ePlotTeam) != NO_WARPLAN)
	{
		return true;
	}

	return false;
}

void CvUnit::read(FDataStreamBase* pStream)
{
	// Init data before load
	reset();

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - RB: Field Bombard START
	pStream->Read(&m_iDCMBombRange);
	pStream->Read(&m_iDCMBombAccuracy);
	// Dale - RB: Field Bombard END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/

	pStream->Read(&m_iID);
	pStream->Read(&m_iGroupID);
	pStream->Read(&m_iHotKeyNumber);
	pStream->Read(&m_iX);
	pStream->Read(&m_iY);
	pStream->Read(&m_iLastMoveTurn);
	pStream->Read(&m_iReconX);
	pStream->Read(&m_iReconY);
	pStream->Read(&m_iGameTurnCreated);
	pStream->Read(&m_iDamage);
	pStream->Read(&m_iMoves);
	pStream->Read(&m_iExperience);
	pStream->Read(&m_iLevel);
	pStream->Read(&m_iCargo);
	pStream->Read(&m_iCargoCapacity);
	pStream->Read(&m_iAttackPlotX);
	pStream->Read(&m_iAttackPlotY);
	pStream->Read(&m_iCombatTimer);
	pStream->Read(&m_iCombatFirstStrikes);
	if (uiFlag < 2)
	{
		int iCombatDamage;
		pStream->Read(&iCombatDamage);
	}
	pStream->Read(&m_iFortifyTurns);
	pStream->Read(&m_iBlitzCount);
	pStream->Read(&m_iAmphibCount);
	pStream->Read(&m_iRiverCount);
	pStream->Read(&m_iEnemyRouteCount);
	pStream->Read(&m_iAlwaysHealCount);
	pStream->Read(&m_iHillsDoubleMoveCount);
/************************************************************************************************/
/* Afforess  Mountaineering Promotion                              10/13/09                     */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	pStream->Read(&m_iCanMovePeaksCount);
	pStream->Read(&m_iSleepTimer);
	pStream->Read(&m_iExtraControlPoints);
	pStream->Read(&m_iExtraCommandRange);
	pStream->Read(&m_iControlPointsLeft);
	pStream->Read(&m_iCommanderID);			//id will be used later on player initialization to get m_pUsedCommander pointer
	pStream->Read((int*)&m_eOriginalOwner);
	pStream->Read(&m_bCommander);
	
	if (uiFlag > 2)
	{
		pStream->Read(&m_bAutoPromoting);
		pStream->Read(&m_bAutoUpgrading);
	}
	
	pStream->Read(GC.getNumTerrainInfos(), m_paiTerrainProtected);
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	

	pStream->Read(&m_iImmuneToFirstStrikesCount);
	pStream->Read(&m_iExtraVisibilityRange);
	pStream->Read(&m_iExtraMoves);
	pStream->Read(&m_iExtraMoveDiscount);
	pStream->Read(&m_iExtraAirRange);
	pStream->Read(&m_iExtraIntercept);
	pStream->Read(&m_iExtraEvasion);
	pStream->Read(&m_iExtraFirstStrikes);
	pStream->Read(&m_iExtraChanceFirstStrikes);
	pStream->Read(&m_iExtraWithdrawal);
	pStream->Read(&m_iExtraCollateralDamage);
	pStream->Read(&m_iExtraBombardRate);
	pStream->Read(&m_iExtraEnemyHeal);
	pStream->Read(&m_iExtraNeutralHeal);
	pStream->Read(&m_iExtraFriendlyHeal);
	pStream->Read(&m_iSameTileHeal);
	pStream->Read(&m_iAdjacentTileHeal);
	pStream->Read(&m_iExtraCombatPercent);
	pStream->Read(&m_iExtraCityAttackPercent);
	pStream->Read(&m_iExtraCityDefensePercent);
	pStream->Read(&m_iExtraHillsAttackPercent);
	pStream->Read(&m_iExtraHillsDefensePercent);
	pStream->Read(&m_iRevoltProtection);
	pStream->Read(&m_iCollateralDamageProtection);
	pStream->Read(&m_iPillageChange);
	pStream->Read(&m_iUpgradeDiscount);
	pStream->Read(&m_iExperiencePercent);
	pStream->Read(&m_iKamikazePercent);
	pStream->Read(&m_iBaseCombat);
	pStream->Read((int*)&m_eFacingDirection);
	pStream->Read(&m_iImmobileTimer);

	pStream->Read(&m_bMadeAttack);
	pStream->Read(&m_bMadeInterception);
	pStream->Read(&m_bPromotionReady);
	pStream->Read(&m_bDeathDelay);
	pStream->Read(&m_bCombatFocus);
	// m_bInfoBarDirty not saved...
	pStream->Read(&m_bBlockading);
	if (uiFlag > 0)
	{
		pStream->Read(&m_bAirCombat);
	}

	pStream->Read((int*)&m_eOwner);
	pStream->Read((int*)&m_eCapturingPlayer);
	pStream->Read((int*)&m_eUnitType);
	FAssert(NO_UNIT != m_eUnitType);
	m_pUnitInfo = (NO_UNIT != m_eUnitType) ? &GC.getUnitInfo(m_eUnitType) : NULL;
	pStream->Read((int*)&m_eLeaderUnitType);

	pStream->Read((int*)&m_combatUnit.eOwner);
	pStream->Read(&m_combatUnit.iID);
	pStream->Read((int*)&m_transportUnit.eOwner);
	pStream->Read(&m_transportUnit.iID);

	pStream->Read(NUM_DOMAIN_TYPES, m_aiExtraDomainModifier);

	pStream->ReadString(m_szName);
	pStream->ReadString(m_szScriptData);

	pStream->Read(GC.getNumPromotionInfos(), m_pabHasPromotion);

	pStream->Read(GC.getNumTerrainInfos(), m_paiTerrainDoubleMoveCount);
	pStream->Read(GC.getNumFeatureInfos(), m_paiFeatureDoubleMoveCount);
	pStream->Read(GC.getNumTerrainInfos(), m_paiExtraTerrainAttackPercent);
	pStream->Read(GC.getNumTerrainInfos(), m_paiExtraTerrainDefensePercent);
	pStream->Read(GC.getNumFeatureInfos(), m_paiExtraFeatureAttackPercent);
	pStream->Read(GC.getNumFeatureInfos(), m_paiExtraFeatureDefensePercent);
	pStream->Read(GC.getNumUnitCombatInfos(), m_paiExtraUnitCombatModifier);
}


void CvUnit::write(FDataStreamBase* pStream)
{
	uint uiFlag=3;
	pStream->Write(uiFlag);		// flag for expansion

/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
	// Dale - RB: Field Bombard START
	pStream->Write(m_iDCMBombRange);
	pStream->Write(m_iDCMBombAccuracy);
	// Dale - RB: Field Bombard END
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/

	pStream->Write(m_iID);
	pStream->Write(m_iGroupID);
	pStream->Write(m_iHotKeyNumber);
	pStream->Write(m_iX);
	pStream->Write(m_iY);
	pStream->Write(m_iLastMoveTurn);
	pStream->Write(m_iReconX);
	pStream->Write(m_iReconY);
	pStream->Write(m_iGameTurnCreated);
	pStream->Write(m_iDamage);
	pStream->Write(m_iMoves);
	pStream->Write(m_iExperience);
	pStream->Write(m_iLevel);
	pStream->Write(m_iCargo);
	pStream->Write(m_iCargoCapacity);
	pStream->Write(m_iAttackPlotX);
	pStream->Write(m_iAttackPlotY);
	pStream->Write(m_iCombatTimer);
	pStream->Write(m_iCombatFirstStrikes);
	pStream->Write(m_iFortifyTurns);
	pStream->Write(m_iBlitzCount);
	pStream->Write(m_iAmphibCount);
	pStream->Write(m_iRiverCount);
	pStream->Write(m_iEnemyRouteCount);
	pStream->Write(m_iAlwaysHealCount);
	pStream->Write(m_iHillsDoubleMoveCount);
/************************************************************************************************/
/* Afforess  Mountaineering Promotion                              10/13/09                     */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	pStream->Write(m_iCanMovePeaksCount);
	pStream->Write(m_iSleepTimer);
	pStream->Write(m_iExtraControlPoints);
	pStream->Write(m_iExtraCommandRange);
	pStream->Write(m_iControlPointsLeft);
	if (m_pUsedCommander != NULL)
	{
		pStream->Write(m_pUsedCommander->getID());
	}
	else
	{
		int badID = -1;
		pStream->Write(badID);	//-1 means there is no used commander
	}
	pStream->Write(m_eOriginalOwner);
	pStream->Write(m_bCommander);
	
	pStream->Write(m_bAutoPromoting);
	pStream->Write(m_bAutoUpgrading);

	pStream->Write(GC.getNumTerrainInfos(), m_paiTerrainProtected);
/************************************************************************************************/
/* Afforess	                         END                                                     */
/************************************************************************************************/	
	pStream->Write(m_iImmuneToFirstStrikesCount);
	pStream->Write(m_iExtraVisibilityRange);
	pStream->Write(m_iExtraMoves);
	pStream->Write(m_iExtraMoveDiscount);
	pStream->Write(m_iExtraAirRange);
	pStream->Write(m_iExtraIntercept);
	pStream->Write(m_iExtraEvasion);
	pStream->Write(m_iExtraFirstStrikes);
	pStream->Write(m_iExtraChanceFirstStrikes);
	pStream->Write(m_iExtraWithdrawal);
	pStream->Write(m_iExtraCollateralDamage);
	pStream->Write(m_iExtraBombardRate);
	pStream->Write(m_iExtraEnemyHeal);
	pStream->Write(m_iExtraNeutralHeal);
	pStream->Write(m_iExtraFriendlyHeal);
	pStream->Write(m_iSameTileHeal);
	pStream->Write(m_iAdjacentTileHeal);
	pStream->Write(m_iExtraCombatPercent);
	pStream->Write(m_iExtraCityAttackPercent);
	pStream->Write(m_iExtraCityDefensePercent);
	pStream->Write(m_iExtraHillsAttackPercent);
	pStream->Write(m_iExtraHillsDefensePercent);
	pStream->Write(m_iRevoltProtection);
	pStream->Write(m_iCollateralDamageProtection);
	pStream->Write(m_iPillageChange);
	pStream->Write(m_iUpgradeDiscount);
	pStream->Write(m_iExperiencePercent);
	pStream->Write(m_iKamikazePercent);
	pStream->Write(m_iBaseCombat);
	pStream->Write(m_eFacingDirection);
	pStream->Write(m_iImmobileTimer);

	pStream->Write(m_bMadeAttack);
	pStream->Write(m_bMadeInterception);
	pStream->Write(m_bPromotionReady);
	pStream->Write(m_bDeathDelay);
	pStream->Write(m_bCombatFocus);
	// m_bInfoBarDirty not saved...
	pStream->Write(m_bBlockading);
	pStream->Write(m_bAirCombat);

	pStream->Write(m_eOwner);
	pStream->Write(m_eCapturingPlayer);
	pStream->Write(m_eUnitType);
	pStream->Write(m_eLeaderUnitType);

	pStream->Write(m_combatUnit.eOwner);
	pStream->Write(m_combatUnit.iID);
	pStream->Write(m_transportUnit.eOwner);
	pStream->Write(m_transportUnit.iID);

	pStream->Write(NUM_DOMAIN_TYPES, m_aiExtraDomainModifier);

	pStream->WriteString(m_szName);
	pStream->WriteString(m_szScriptData);

	pStream->Write(GC.getNumPromotionInfos(), m_pabHasPromotion);

	pStream->Write(GC.getNumTerrainInfos(), m_paiTerrainDoubleMoveCount);
	pStream->Write(GC.getNumFeatureInfos(), m_paiFeatureDoubleMoveCount);
	pStream->Write(GC.getNumTerrainInfos(), m_paiExtraTerrainAttackPercent);
	pStream->Write(GC.getNumTerrainInfos(), m_paiExtraTerrainDefensePercent);
	pStream->Write(GC.getNumFeatureInfos(), m_paiExtraFeatureAttackPercent);
	pStream->Write(GC.getNumFeatureInfos(), m_paiExtraFeatureDefensePercent);
	pStream->Write(GC.getNumUnitCombatInfos(), m_paiExtraUnitCombatModifier);
}

// Protected Functions...

bool CvUnit::canAdvance(const CvPlot* pPlot, int iThreshold) const
{
	FAssert(canFight());
	FAssert(!(isAnimal() && pPlot->isCity()));
	FAssert(getDomainType() != DOMAIN_AIR);
	FAssert(getDomainType() != DOMAIN_IMMOBILE);

	if (pPlot->getNumVisibleEnemyDefenders(this) > iThreshold)
	{
		return false;
	}

	if (isNoCapture())
	{
		if (pPlot->isEnemyCity(*this))
		{
			return false;
		}
	}

	return true;
}


void CvUnit::collateralCombat(const CvPlot* pPlot, CvUnit* pSkipUnit)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	CvWString szBuffer;
	int iTheirStrength;
	int iStrengthFactor;
	int iCollateralDamage;
	int iUnitDamage;
	int iDamageCount;
	int iPossibleTargets;
	int iCount;
	int iValue;
	int iBestValue;
	std::map<CvUnit*, int> mapUnitDamage;
	std::map<CvUnit*, int>::iterator it;

	int iCollateralStrength = (getDomainType() == DOMAIN_AIR ? airBaseCombatStr() : baseCombatStr()) * collateralDamage() / 100;
	// UNOFFICIAL_PATCH Start
	// * Barrage promotions made working again on Tanks and other units with no base collateral ability
	if (iCollateralStrength == 0 && getExtraCollateralDamage() == 0)
	// UNOFFICIAL_PATCH End
	{
		return;
	}

	iPossibleTargets = std::min((pPlot->getNumVisibleEnemyDefenders(this) - 1), collateralDamageMaxUnits());

	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit != pSkipUnit)
		{
			if (isEnemy(pLoopUnit->getTeam(), pPlot))
			{
				if (!(pLoopUnit->isInvisible(getTeam(), false)))
				{
					if (pLoopUnit->canDefend())
					{
						iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "Collateral Damage"));

						iValue *= pLoopUnit->currHitPoints();

						mapUnitDamage[pLoopUnit] = iValue;
					}
				}
			}
		}
	}

	CvCity* pCity = NULL;
	if (getDomainType() == DOMAIN_AIR)
	{
		pCity = pPlot->getPlotCity();
	}

	iDamageCount = 0;
	iCount = 0;

	while (iCount < iPossibleTargets)
	{
		iBestValue = 0;
		pBestUnit = NULL;

		for (it = mapUnitDamage.begin(); it != mapUnitDamage.end(); it++)
		{
			if (it->second > iBestValue)
			{
				iBestValue = it->second;
				pBestUnit = it->first;
			}
		}

		if (pBestUnit != NULL)
		{
			mapUnitDamage.erase(pBestUnit);

			if (NO_UNITCOMBAT == getUnitCombatType() || !pBestUnit->getUnitInfo().getUnitCombatCollateralImmune(getUnitCombatType()))
			{
				iTheirStrength = pBestUnit->baseCombatStr();

				iStrengthFactor = ((iCollateralStrength + iTheirStrength + 1) / 2);

				iCollateralDamage = (GC.getDefineINT("COLLATERAL_COMBAT_DAMAGE") * (iCollateralStrength + iStrengthFactor)) / (iTheirStrength + iStrengthFactor);

				iCollateralDamage *= 100 + getExtraCollateralDamage();

				iCollateralDamage *= std::max(0, 100 - pBestUnit->getCollateralDamageProtection());
				iCollateralDamage /= 100;

				if (pCity != NULL)
				{
					iCollateralDamage *= 100 + pCity->getAirModifier();
					iCollateralDamage /= 100;
				}

				iCollateralDamage /= 100;

				iCollateralDamage = std::max(0, iCollateralDamage);

				int iMaxDamage = std::min(collateralDamageLimit(), (collateralDamageLimit() * (iCollateralStrength + iStrengthFactor)) / (iTheirStrength + iStrengthFactor));
				iUnitDamage = std::max(pBestUnit->getDamage(), std::min(pBestUnit->getDamage() + iCollateralDamage, iMaxDamage));

				if (pBestUnit->getDamage() != iUnitDamage)
				{
// BUG - Combat Events - start
					int iDamageDone = iUnitDamage - pBestUnit->getDamage();
					pBestUnit->setDamage(iUnitDamage, getOwnerINLINE());
					CvEventReporter::getInstance().combatLogCollateral(this, pBestUnit, iDamageDone);
// BUG - Combat Events - end
					iDamageCount++;
				}
			}

			iCount++;
		}
		else
		{
			break;
		}
	}

	if (iDamageCount > 0)
	{
		szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_SUFFER_COL_DMG", iDamageCount);
		gDLL->getInterfaceIFace()->addMessage(pSkipUnit->getOwnerINLINE(), (pSkipUnit->getDomainType() != DOMAIN_AIR), GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_COLLATERAL", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pSkipUnit->getX_INLINE(), pSkipUnit->getY_INLINE(), true, true);

		szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_INFLICT_COL_DMG", getNameKey(), iDamageCount);
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_COLLATERAL", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pSkipUnit->getX_INLINE(), pSkipUnit->getY_INLINE());
	}
}


void CvUnit::flankingStrikeCombat(const CvPlot* pPlot, int iAttackerStrength, int iAttackerFirepower, int iDefenderOdds, int iDefenderDamage, CvUnit* pSkipUnit)
{
	if (pPlot->isCity(true, pSkipUnit->getTeam()))
	{
		return;
	}

	CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();

	std::vector< std::pair<CvUnit*, int> > listFlankedUnits;
	while (NULL != pUnitNode)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit != pSkipUnit)
		{
			if (!pLoopUnit->isDead() && isEnemy(pLoopUnit->getTeam(), pPlot))
			{
				if (!(pLoopUnit->isInvisible(getTeam(), false)))
				{
					if (pLoopUnit->canDefend())
					{
						int iFlankingStrength = m_pUnitInfo->getFlankingStrikeUnitClass(pLoopUnit->getUnitClassType());

						if (iFlankingStrength > 0)
						{
							int iFlankedDefenderStrength;
							int iFlankedDefenderOdds;
							int iAttackerDamage;
							int iFlankedDefenderDamage;

							getDefenderCombatValues(*pLoopUnit, pPlot, iAttackerStrength, iAttackerFirepower, iFlankedDefenderOdds, iFlankedDefenderStrength, iAttackerDamage, iFlankedDefenderDamage);

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
							// From Lead From Behind by UncutDragon
/* original code
							if (GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("COMBAT_DIE_SIDES"), "Flanking Combat") >= iDefenderOdds)
*/							// modified
							if (GC.getGameINLINE().getSorenRandNum(GC.getCOMBAT_DIE_SIDES(), "Flanking Combat") >= iDefenderOdds)
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
							{
								int iCollateralDamage = (iFlankingStrength * iDefenderDamage) / 100;
								int iUnitDamage = std::max(pLoopUnit->getDamage(), std::min(pLoopUnit->getDamage() + iCollateralDamage, collateralDamageLimit()));

								if (pLoopUnit->getDamage() != iUnitDamage)
								{
									listFlankedUnits.push_back(std::make_pair(pLoopUnit, iUnitDamage));
								}
							}
						}
					}
				}
			}
		}
	}

	int iNumUnitsHit = std::min((int)listFlankedUnits.size(), collateralDamageMaxUnits());

	for (int i = 0; i < iNumUnitsHit; ++i)
	{
		int iIndexHit = GC.getGameINLINE().getSorenRandNum(listFlankedUnits.size(), "Pick Flanked Unit");
		CvUnit* pUnit = listFlankedUnits[iIndexHit].first;
		int iDamage = listFlankedUnits[iIndexHit].second;
// BUG - Combat Events - start
		int iDamageDone = iDamage - pUnit->getDamage();
// BUG - Combat Events - end
		pUnit->setDamage(iDamage, getOwnerINLINE());
		if (pUnit->isDead())
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_KILLED_UNIT_BY_FLANKING", getNameKey(), pUnit->getNameKey(), pUnit->getVisualCivAdjective(getTeam()));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_UNIT_DIED_BY_FLANKING", pUnit->getNameKey(), getNameKey(), getVisualCivAdjective(pUnit->getTeam()));
			gDLL->getInterfaceIFace()->addMessage(pUnit->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

			pUnit->kill(false); 
		}
// BUG - Combat Events - start
		CvEventReporter::getInstance().combatLogFlanking(this, pUnit, iDamageDone);
// BUG - Combat Events - end
		
		listFlankedUnits.erase(std::remove(listFlankedUnits.begin(), listFlankedUnits.end(), listFlankedUnits[iIndexHit]));
	}

	if (iNumUnitsHit > 0)
	{
		CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_DAMAGED_UNITS_BY_FLANKING", getNameKey(), iNumUnitsHit);
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

		if (NULL != pSkipUnit)
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_UNITS_DAMAGED_BY_FLANKING", getNameKey(), iNumUnitsHit);
			gDLL->getInterfaceIFace()->addMessage(pSkipUnit->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
	}
}


// Returns true if we were intercepted...
bool CvUnit::interceptTest(const CvPlot* pPlot)
{
	if (GC.getGameINLINE().getSorenRandNum(100, "Evasion Rand") >= evasionProbability())
	{
		CvUnit* pInterceptor = bestInterceptor(pPlot);
		if (pInterceptor != NULL)
		{
/************************************************************************************************/
/* Afforess	                  Start		 03/6/10                                                */
/*                                                                                              */
/*  Better Air Interception                                                                     */
/************************************************************************************************/
			int iInterceptionOdds; 
			if (GC.getGameINLINE().isModderGameOption(MODDERGAMEOPTION_BETTER_INTERCETION))
			{
				iInterceptionOdds = interceptionChance(pPlot);
			}
			else
			{
				iInterceptionOdds = pInterceptor->currInterceptionProbability();
			}
			if (GC.getGameINLINE().getSorenRandNum(100, "Intercept Rand (Air)") < iInterceptionOdds)
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			{
				fightInterceptor(pPlot, false);

				return true;
			}
		}
	}


	return false;
}


CvUnit* CvUnit::airStrikeTarget(const CvPlot* pPlot) const
{
	CvUnit* pDefender;

	pDefender = pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);

	if (pDefender != NULL)
	{
		if (!pDefender->isDead())
		{
			if (pDefender->canDefend())
			{
				return pDefender;
			}
		}
	}

	return NULL;
}


bool CvUnit::canAirStrike(const CvPlot* pPlot) const
{
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}

	if (!canAirAttack())
	{
		return false;
	}

	if (pPlot == plot())
	{
		return false;
	}

	if (!pPlot->isVisible(getTeam(), false))
	{
		return false;
	}

	if (plotDistance(getX_INLINE(), getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE()) > airRange())
	{
		return false;
	}

	if (airStrikeTarget(pPlot) == NULL)
	{
		return false;
	}

	return true;
}


bool CvUnit::airStrike(CvPlot* pPlot)
{
	if (!canAirStrike(pPlot))
	{
		return false;
	}

	if (interceptTest(pPlot))
	{
		return false;
	}

	CvUnit* pDefender = airStrikeTarget(pPlot);

/************************************************************************************************/
/* DCM	                  Start		 05/31/10                        Johnny Smith               */
/*                                                                   Afforess                   */
/* Battle Effects                                                                               */
/************************************************************************************************/
	setBattlePlot(pPlot, pDefender);
/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/
	FAssert(pDefender != NULL);
	FAssert(pDefender->canDefend());

	setReconPlot(pPlot);

	setMadeAttack(true);
	changeMoves(GC.getMOVE_DENOMINATOR());

	int iDamage = airCombatDamage(pDefender);

	int iUnitDamage = std::max(pDefender->getDamage(), std::min((pDefender->getDamage() + iDamage), airCombatLimit()));

	CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_AIR", pDefender->getNameKey(), getNameKey(), -(((iUnitDamage - pDefender->getDamage()) * 100) / pDefender->maxHitPoints()));
	gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_AIR_ATTACK", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);

	szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR", getNameKey(), pDefender->getNameKey(), -(((iUnitDamage - pDefender->getDamage()) * 100) / pDefender->maxHitPoints()));
	gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_AIR_ATTACKED", MESSAGE_TYPE_INFO, pDefender->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

	collateralCombat(pPlot, pDefender);

	pDefender->setDamage(iUnitDamage, getOwnerINLINE());
/************************************************************************************************/
/* Afforess	                  Start		 08/03/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (GC.getGameINLINE().isModderGameOption(MODDERGAMEOPTION_IMPROVED_XP))
	{
		setExperience100(getExperience100() + 25);
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

	
	return true;
}

bool CvUnit::canRangeStrike() const
{
	if (getDomainType() == DOMAIN_AIR)
	{
		return false;
	}

	if (airRange() <= 0)
	{
		return false;
	}

	if (airBaseCombatStr() <= 0)
	{
		return false;
	}

	if (!canFight())
	{
		return false;
	}

	if (isMadeAttack() && !isBlitz())
	{
		return false;
	}

	if (!canMove() && getMoves() > 0)
	{
		return false;
	}

	return true;
}

bool CvUnit::canRangeStrikeAt(const CvPlot* pPlot, int iX, int iY) const
{
	if (!canRangeStrike())
	{
		return false;
	}

	CvPlot* pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (NULL == pTargetPlot)
	{
		return false;
	}

	if (!pPlot->isVisible(getTeam(), false))
	{
		return false;
	}

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       05/10/10                             jdog5000         */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
	// Need to check target plot too
	if (!pTargetPlot->isVisible(getTeam(), false))
	{
		return false;
	}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) > airRange())
	{
		return false;
	}

	CvUnit* pDefender = airStrikeTarget(pTargetPlot);
	if (NULL == pDefender)
	{
		return false;
	}

	if (!pPlot->canSeePlot(pTargetPlot, getTeam(), airRange(), getFacingDirection(true)))
	{
		return false;
	}
	
	return true;
}


bool CvUnit::rangeStrike(int iX, int iY)
{
	CvUnit* pDefender;
	CvWString szBuffer;
	int iUnitDamage;
	int iDamage;

	CvPlot* pPlot = GC.getMapINLINE().plot(iX, iY);
	if (NULL == pPlot)
	{
		return false;
	}

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       05/10/10                             jdog5000         */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
	if (!canRangeStrikeAt(pPlot, iX, iY))
	{
		return false;
	}
*/
	if (!canRangeStrikeAt(plot(), iX, iY))
	{
		return false;
	}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

	pDefender = airStrikeTarget(pPlot);

	FAssert(pDefender != NULL);
	FAssert(pDefender->canDefend());

/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
	setBattlePlot(pDefender->plot(), pDefender);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
	if (GC.getDefineINT("RANGED_ATTACKS_USE_MOVES") == 0)
	{
		setMadeAttack(true);
	}
	changeMoves(GC.getMOVE_DENOMINATOR());

	iDamage = rangeCombatDamage(pDefender);

	iUnitDamage = std::max(pDefender->getDamage(), std::min((pDefender->getDamage() + iDamage), airCombatLimit()));

	szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_AIR", pDefender->getNameKey(), getNameKey(), -(((iUnitDamage - pDefender->getDamage()) * 100) / pDefender->maxHitPoints()));
	//red icon over attacking unit
	gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_COMBAT", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), this->getX_INLINE(), this->getY_INLINE(), true, true);
	//white icon over defending unit
	gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), false, 0, L"", "AS2D_COMBAT", MESSAGE_TYPE_DISPLAY_ONLY, pDefender->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pDefender->getX_INLINE(), pDefender->getY_INLINE(), true, true);

	szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR", getNameKey(), pDefender->getNameKey(), -(((iUnitDamage - pDefender->getDamage()) * 100) / pDefender->maxHitPoints()));
	gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_COMBAT", MESSAGE_TYPE_INFO, pDefender->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

	collateralCombat(pPlot, pDefender);

	//set damage but don't update entity damage visibility
	pDefender->setDamage(iUnitDamage, getOwnerINLINE(), false);

	if (pPlot->isActiveVisible(false))
	{
		// Range strike entity mission
		CvMissionDefinition kDefiniton;
		kDefiniton.setMissionTime(GC.getMissionInfo(MISSION_RANGE_ATTACK).getTime() * gDLL->getSecsPerTurn());
		kDefiniton.setMissionType(MISSION_RANGE_ATTACK);
		kDefiniton.setPlot(pDefender->plot());
		kDefiniton.setUnit(BATTLE_UNIT_ATTACKER, this);
		kDefiniton.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
		gDLL->getEntityIFace()->AddMission(&kDefiniton);

		//delay death
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       05/10/10                             jdog5000         */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
		pDefender->getGroup()->setMissionTimer(GC.getMissionInfo(MISSION_RANGE_ATTACK).getTime());
*/
		// mission timer is not used like this in any other part of code, so it might cause OOS
		// issues ... at worst I think unit dies before animation is complete, so no real
		// harm in commenting it out.
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
	}

	return true;
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvUnit::planBattle
//! \brief      Determines in general how a battle will progress.
//!
//!				Note that the outcome of the battle is not determined here. This function plans
//!				how many sub-units die and in which 'rounds' of battle.
//! \param      kBattleDefinition The battle definition, which receives the battle plan.
//! \retval     The number of game turns that the battle should be given.
//------------------------------------------------------------------------------------------------
int CvUnit::planBattle( CvBattleDefinition & kBattleDefinition ) const
{
#define BATTLE_TURNS_SETUP 4
#define BATTLE_TURNS_ENDING 4
#define BATTLE_TURNS_MELEE 6
#define BATTLE_TURNS_RANGED 6
#define BATTLE_TURN_RECHECK 4

	int								aiUnitsBegin[BATTLE_UNIT_COUNT];
	int								aiUnitsEnd[BATTLE_UNIT_COUNT];
	int								aiToKillMelee[BATTLE_UNIT_COUNT];
	int								aiToKillRanged[BATTLE_UNIT_COUNT];
	CvBattleRoundVector::iterator	iIterator;
	int								i, j;
	bool							bIsLoser;
	int								iRoundIndex;
	int								iTotalRounds = 0;
	int								iRoundCheck = BATTLE_TURN_RECHECK;

	// Initial conditions
	kBattleDefinition.setNumRangedRounds(0);
	kBattleDefinition.setNumMeleeRounds(0);

	int iFirstStrikesDelta = kBattleDefinition.getFirstStrikes(BATTLE_UNIT_ATTACKER) - kBattleDefinition.getFirstStrikes(BATTLE_UNIT_DEFENDER);
	if (iFirstStrikesDelta > 0) // Attacker first strikes
	{
		int iKills = computeUnitsToDie( kBattleDefinition, true, BATTLE_UNIT_DEFENDER );
		kBattleDefinition.setNumRangedRounds(std::max(iFirstStrikesDelta, iKills / iFirstStrikesDelta));
	}
	else if (iFirstStrikesDelta < 0) // Defender first strikes
	{
		int iKills = computeUnitsToDie( kBattleDefinition, true, BATTLE_UNIT_ATTACKER );
		iFirstStrikesDelta = -iFirstStrikesDelta;
		kBattleDefinition.setNumRangedRounds(std::max(iFirstStrikesDelta, iKills / iFirstStrikesDelta));
	}
	increaseBattleRounds( kBattleDefinition);


	// Keep randomizing until we get something valid
	do 
	{
		iRoundCheck++;
		if ( iRoundCheck >= BATTLE_TURN_RECHECK )
		{
			increaseBattleRounds( kBattleDefinition);
			iTotalRounds = kBattleDefinition.getNumRangedRounds() + kBattleDefinition.getNumMeleeRounds();
			iRoundCheck = 0;
		}

		// Make sure to clear the battle plan, we may have to do this again if we can't find a plan that works.
		kBattleDefinition.clearBattleRounds();

		// Create the round list
		CvBattleRound kRound;
		kBattleDefinition.setBattleRound(iTotalRounds, kRound);

		// For the attacker and defender
		for ( i = 0; i < BATTLE_UNIT_COUNT; i++ )
		{
			// Gather some initial information
			BattleUnitTypes unitType = (BattleUnitTypes) i;
			aiUnitsBegin[unitType] = kBattleDefinition.getUnit(unitType)->getSubUnitsAlive(kBattleDefinition.getDamage(unitType, BATTLE_TIME_BEGIN));
			aiToKillRanged[unitType] = computeUnitsToDie( kBattleDefinition, true, unitType);
			aiToKillMelee[unitType] = computeUnitsToDie( kBattleDefinition, false, unitType);
			aiUnitsEnd[unitType] = aiUnitsBegin[unitType] - aiToKillMelee[unitType] - aiToKillRanged[unitType];

			// Make sure that if they aren't dead at the end, they have at least one unit left
			if ( aiUnitsEnd[unitType] == 0 && !kBattleDefinition.getUnit(unitType)->isDead() )
			{
				aiUnitsEnd[unitType]++;
				if ( aiToKillMelee[unitType] > 0 )
				{
					aiToKillMelee[unitType]--;
				}
				else
				{
					aiToKillRanged[unitType]--;
				}
			}

			// If one unit is the loser, make sure that at least one of their units dies in the last round
			if ( aiUnitsEnd[unitType] == 0 )
			{
				kBattleDefinition.getBattleRound(iTotalRounds - 1).addNumKilled(unitType, 1);
				if ( aiToKillMelee[unitType] > 0)
				{
					aiToKillMelee[unitType]--;
				}
				else
				{
					aiToKillRanged[unitType]--;
				}
			}

			// Randomize in which round each death occurs
			bIsLoser = aiUnitsEnd[unitType] == 0;

			// Randomize the ranged deaths
			for ( j = 0; j < aiToKillRanged[unitType]; j++ )
			{
				iRoundIndex = GC.getGameINLINE().getSorenRandNum( range( kBattleDefinition.getNumRangedRounds(), 0, kBattleDefinition.getNumRangedRounds()), "Ranged combat death");
				kBattleDefinition.getBattleRound(iRoundIndex).addNumKilled(unitType, 1);
			}

			// Randomize the melee deaths
			for ( j = 0; j < aiToKillMelee[unitType]; j++ )
			{
				iRoundIndex = GC.getGameINLINE().getSorenRandNum( range( kBattleDefinition.getNumMeleeRounds() - (bIsLoser ? 1 : 2 ), 0, kBattleDefinition.getNumMeleeRounds()), "Melee combat death");
				kBattleDefinition.getBattleRound(kBattleDefinition.getNumRangedRounds() + iRoundIndex).addNumKilled(unitType, 1);
			}

			// Compute alive sums
			int iNumberKilled = 0;
			for(int j=0;j<kBattleDefinition.getNumBattleRounds();j++)
			{
				CvBattleRound &round = kBattleDefinition.getBattleRound(j);
				round.setRangedRound(j < kBattleDefinition.getNumRangedRounds());
				iNumberKilled += round.getNumKilled(unitType);
				round.setNumAlive(unitType, aiUnitsBegin[unitType] - iNumberKilled);
			}
		}

		// Now compute wave sizes
		for(int i=0;i<kBattleDefinition.getNumBattleRounds();i++)
		{
			CvBattleRound &round = kBattleDefinition.getBattleRound(i);
			round.setWaveSize(computeWaveSize(round.isRangedRound(), round.getNumAlive(BATTLE_UNIT_ATTACKER) + round.getNumKilled(BATTLE_UNIT_ATTACKER), round.getNumAlive(BATTLE_UNIT_DEFENDER) + round.getNumKilled(BATTLE_UNIT_DEFENDER)));
		}

		if ( iTotalRounds > 400 )
		{
			kBattleDefinition.setNumMeleeRounds(1);
			kBattleDefinition.setNumRangedRounds(0);
			break;
		}
	} 
	while ( !verifyRoundsValid( kBattleDefinition ));

	//add a little extra time for leader to surrender
	bool attackerLeader = false;
	bool defenderLeader = false;
	bool attackerDie = false;
	bool defenderDie = false;
	int lastRound = kBattleDefinition.getNumBattleRounds() - 1;
	if(kBattleDefinition.getUnit(BATTLE_UNIT_ATTACKER)->getLeaderUnitType() != NO_UNIT)
		attackerLeader = true;
	if(kBattleDefinition.getUnit(BATTLE_UNIT_DEFENDER)->getLeaderUnitType() != NO_UNIT)
		defenderLeader = true;
	if(kBattleDefinition.getBattleRound(lastRound).getNumAlive(BATTLE_UNIT_ATTACKER) == 0)
		attackerDie = true;
	if(kBattleDefinition.getBattleRound(lastRound).getNumAlive(BATTLE_UNIT_DEFENDER) == 0)
		defenderDie = true;

	int extraTime = 0;
	if((attackerLeader && attackerDie) || (defenderLeader && defenderDie))
		extraTime = BATTLE_TURNS_MELEE;
	if(gDLL->getEntityIFace()->GetSiegeTower(kBattleDefinition.getUnit(BATTLE_UNIT_ATTACKER)->getUnitEntity()) || gDLL->getEntityIFace()->GetSiegeTower(kBattleDefinition.getUnit(BATTLE_UNIT_DEFENDER)->getUnitEntity()))
		extraTime = BATTLE_TURNS_MELEE;

	return BATTLE_TURNS_SETUP + BATTLE_TURNS_ENDING + kBattleDefinition.getNumMeleeRounds() * BATTLE_TURNS_MELEE + kBattleDefinition.getNumRangedRounds() * BATTLE_TURNS_MELEE + extraTime;
}

//------------------------------------------------------------------------------------------------
// FUNCTION:	CvBattleManager::computeDeadUnits
//! \brief		Computes the number of units dead, for either the ranged or melee portion of combat.
//! \param		kDefinition The battle definition.
//! \param		bRanged true if computing the number of units that die during the ranged portion of combat,
//!					false if computing the number of units that die during the melee portion of combat.
//! \param		iUnit The index of the unit to compute (BATTLE_UNIT_ATTACKER or BATTLE_UNIT_DEFENDER).
//! \retval		The number of units that should die for the given unit in the given portion of combat
//------------------------------------------------------------------------------------------------
int CvUnit::computeUnitsToDie( const CvBattleDefinition & kDefinition, bool bRanged, BattleUnitTypes iUnit ) const
{
	FAssertMsg( iUnit == BATTLE_UNIT_ATTACKER || iUnit == BATTLE_UNIT_DEFENDER, "Invalid unit index");

	BattleTimeTypes iBeginIndex = bRanged ? BATTLE_TIME_BEGIN : BATTLE_TIME_RANGED;
	BattleTimeTypes iEndIndex = bRanged ? BATTLE_TIME_RANGED : BATTLE_TIME_END;
	return kDefinition.getUnit(iUnit)->getSubUnitsAlive(kDefinition.getDamage(iUnit, iBeginIndex)) - 
		kDefinition.getUnit(iUnit)->getSubUnitsAlive( kDefinition.getDamage(iUnit, iEndIndex));
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvUnit::verifyRoundsValid
//! \brief      Verifies that all rounds in the battle plan are valid
//! \param      vctBattlePlan The battle plan
//! \retval     true if the battle plan (seems) valid, false otherwise
//------------------------------------------------------------------------------------------------
bool CvUnit::verifyRoundsValid( const CvBattleDefinition & battleDefinition ) const
{
	for(int i=0;i<battleDefinition.getNumBattleRounds();i++)
	{
		if(!battleDefinition.getBattleRound(i).isValid())
			return false;
	}
	return true;
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvUnit::increaseBattleRounds
//! \brief      Increases the number of rounds in the battle.
//! \param      kBattleDefinition The definition of the battle
//------------------------------------------------------------------------------------------------
void CvUnit::increaseBattleRounds( CvBattleDefinition & kBattleDefinition ) const
{
	if ( kBattleDefinition.getUnit(BATTLE_UNIT_ATTACKER)->isRanged() && kBattleDefinition.getUnit(BATTLE_UNIT_DEFENDER)->isRanged())
	{
		kBattleDefinition.addNumRangedRounds(1);
	}
	else
	{
		kBattleDefinition.addNumMeleeRounds(1);
	}
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvUnit::computeWaveSize
//! \brief      Computes the wave size for the round.
//! \param      bRangedRound true if the round is a ranged round
//! \param		iAttackerMax The maximum number of attackers that can participate in a wave (alive)
//! \param		iDefenderMax The maximum number of Defenders that can participate in a wave (alive)
//! \retval     The desired wave size for the given parameters
//------------------------------------------------------------------------------------------------
int CvUnit::computeWaveSize( bool bRangedRound, int iAttackerMax, int iDefenderMax ) const
{
	FAssertMsg( getCombatUnit() != NULL, "You must be fighting somebody!" );
	int aiDesiredSize[BATTLE_UNIT_COUNT];
	if ( bRangedRound )
	{
		aiDesiredSize[BATTLE_UNIT_ATTACKER] = getUnitInfo().getRangedWaveSize();
		aiDesiredSize[BATTLE_UNIT_DEFENDER] = getCombatUnit()->getUnitInfo().getRangedWaveSize();
	}
	else
	{
		aiDesiredSize[BATTLE_UNIT_ATTACKER] = getUnitInfo().getMeleeWaveSize();
		aiDesiredSize[BATTLE_UNIT_DEFENDER] = getCombatUnit()->getUnitInfo().getMeleeWaveSize();
	}

	aiDesiredSize[BATTLE_UNIT_DEFENDER] = aiDesiredSize[BATTLE_UNIT_DEFENDER] <= 0 ? iDefenderMax : aiDesiredSize[BATTLE_UNIT_DEFENDER];
	aiDesiredSize[BATTLE_UNIT_ATTACKER] = aiDesiredSize[BATTLE_UNIT_ATTACKER] <= 0 ? iDefenderMax : aiDesiredSize[BATTLE_UNIT_ATTACKER];
	return std::min( std::min( aiDesiredSize[BATTLE_UNIT_ATTACKER], iAttackerMax ), std::min( aiDesiredSize[BATTLE_UNIT_DEFENDER],
		iDefenderMax) );
}

bool CvUnit::isTargetOf(const CvUnit& attacker) const
{
	CvUnitInfo& attackerInfo = attacker.getUnitInfo();
	CvUnitInfo& ourInfo = getUnitInfo();

	if (!plot()->isCity(true, getTeam()))
	{
		if (NO_UNITCLASS != getUnitClassType() && attackerInfo.getTargetUnitClass(getUnitClassType()))
		{
			return true;
		}

		if (NO_UNITCOMBAT != getUnitCombatType() && attackerInfo.getTargetUnitCombat(getUnitCombatType()))
		{
			return true;
		}
	}

	if (NO_UNITCLASS != attackerInfo.getUnitClassType() && ourInfo.getDefenderUnitClass(attackerInfo.getUnitClassType()))
	{
		return true;
	}

	if (NO_UNITCOMBAT != attackerInfo.getUnitCombatType() && ourInfo.getDefenderUnitCombat(attackerInfo.getUnitCombatType()))
	{
		return true;
	}

	return false;
}

bool CvUnit::isEnemy(TeamTypes eTeam, const CvPlot* pPlot) const
{
	if (NULL == pPlot)
	{
		pPlot = plot();
	}

	return (atWar(GET_PLAYER(getCombatOwner(eTeam, pPlot)).getTeam(), eTeam));
}

bool CvUnit::isPotentialEnemy(TeamTypes eTeam, const CvPlot* pPlot) const
{
	if (NULL == pPlot)
	{
		pPlot = plot();
	}

	return (::isPotentialEnemy(GET_PLAYER(getCombatOwner(eTeam, pPlot)).getTeam(), eTeam));
}

bool CvUnit::isSuicide() const
{
	return (m_pUnitInfo->isSuicide() || getKamikazePercent() != 0);
}

int CvUnit::getDropRange() const
{
	return (m_pUnitInfo->getDropRange());
}

void CvUnit::getDefenderCombatValues(CvUnit& kDefender, const CvPlot* pPlot, int iOurStrength, int iOurFirepower, int& iTheirOdds, int& iTheirStrength, int& iOurDamage, int& iTheirDamage, CombatDetails* pTheirDetails) const
{
	iTheirStrength = kDefender.currCombatStr(pPlot, this, pTheirDetails);
	int iTheirFirepower = kDefender.currFirepower(pPlot, this);

	FAssert((iOurStrength + iTheirStrength) > 0);
	FAssert((iOurFirepower + iTheirFirepower) > 0);

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
	// From Lead From Behind by UncutDragon
/* original code
	iTheirOdds = ((GC.getDefineINT("COMBAT_DIE_SIDES") * iTheirStrength) / (iOurStrength + iTheirStrength));
*/	// modified
	iTheirOdds = ((GC.getCOMBAT_DIE_SIDES() * iTheirStrength) / (iOurStrength + iTheirStrength));

	if (kDefender.isBarbarian())
	{
		if (GET_PLAYER(getOwnerINLINE()).getWinsVsBarbs() < GC.getHandicapInfo(GET_PLAYER(getOwnerINLINE()).getHandicapType()).getFreeWinsVsBarbs())
		{
			// UncutDragon
/* original code
			iTheirOdds = std::min((10 * GC.getDefineINT("COMBAT_DIE_SIDES")) / 100, iTheirOdds);
*/			// modified
			iTheirOdds = std::min((10 * GC.getCOMBAT_DIE_SIDES()) / 100, iTheirOdds);
			// /UncutDragon
		}
	}
	if (isBarbarian())
	{
		if (GET_PLAYER(kDefender.getOwnerINLINE()).getWinsVsBarbs() < GC.getHandicapInfo(GET_PLAYER(kDefender.getOwnerINLINE()).getHandicapType()).getFreeWinsVsBarbs())
		{
			// UncutDragon
/* original code
			iTheirOdds =  std::max((90 * GC.getDefineINT("COMBAT_DIE_SIDES")) / 100, iTheirOdds);
*/			// modified
			iTheirOdds =  std::max((90 * GC.getCOMBAT_DIE_SIDES()) / 100, iTheirOdds);
			// /UncutDragon
		}
	}

	int iStrengthFactor = ((iOurFirepower + iTheirFirepower + 1) / 2);

	// UncutDragon
/* original code
	iOurDamage = std::max(1, ((GC.getDefineINT("COMBAT_DAMAGE") * (iTheirFirepower + iStrengthFactor)) / (iOurFirepower + iStrengthFactor)));
	iTheirDamage = std::max(1, ((GC.getDefineINT("COMBAT_DAMAGE") * (iOurFirepower + iStrengthFactor)) / (iTheirFirepower + iStrengthFactor)));
*/	// modified
	iOurDamage = std::max(1, ((GC.getCOMBAT_DAMAGE() * (iTheirFirepower + iStrengthFactor)) / (iOurFirepower + iStrengthFactor)));
	iTheirDamage = std::max(1, ((GC.getCOMBAT_DAMAGE() * (iOurFirepower + iStrengthFactor)) / (iTheirFirepower + iStrengthFactor)));
	// /UncutDragon
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
}

int CvUnit::getTriggerValue(EventTriggerTypes eTrigger, const CvPlot* pPlot, bool bCheckPlot) const
{
	CvEventTriggerInfo& kTrigger = GC.getEventTriggerInfo(eTrigger);
	if (kTrigger.getNumUnits() <= 0)
	{
		return MIN_INT;
	}

	if (isDead())
	{
		return MIN_INT;
	}

	if (!CvString(kTrigger.getPythonCanDoUnit()).empty())
	{
		long lResult;

		CyArgsList argsList;
		argsList.add(eTrigger);
		argsList.add(getOwnerINLINE());
		argsList.add(getID());

		gDLL->getPythonIFace()->callFunction(PYRandomEventModule, kTrigger.getPythonCanDoUnit(), argsList.makeFunctionArgs(), &lResult);

		if (0 == lResult)
		{
			return MIN_INT;
		}
	}

	if (kTrigger.getNumUnitsRequired() > 0)
	{
		bool bFoundValid = false;
		for (int i = 0; i < kTrigger.getNumUnitsRequired(); ++i)
		{
			if (getUnitClassType() == kTrigger.getUnitRequired(i))
			{
				bFoundValid = true;
				break;
			}
		}

		if (!bFoundValid)
		{
			return MIN_INT;
		}
	}

	if (bCheckPlot)
	{
		if (kTrigger.isUnitsOnPlot())
		{
			if (!plot()->canTrigger(eTrigger, getOwnerINLINE()))
			{
				return MIN_INT;
			}
		}
	}

	int iValue = 0;

	if (0 == getDamage() && kTrigger.getUnitDamagedWeight() > 0)
	{
		return MIN_INT;
	}

	iValue += getDamage() * kTrigger.getUnitDamagedWeight();

	iValue += getExperience() * kTrigger.getUnitExperienceWeight();

	if (NULL != pPlot)
	{
		iValue += plotDistance(getX_INLINE(), getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE()) * kTrigger.getUnitDistanceWeight();
	}

	return iValue;
}

bool CvUnit::canApplyEvent(EventTypes eEvent) const
{
	CvEventInfo& kEvent = GC.getEventInfo(eEvent);

	if (0 != kEvent.getUnitExperience())
	{
		if (!canAcquirePromotionAny())
		{
			return false;
		}
	}

	if (NO_PROMOTION != kEvent.getUnitPromotion())
	{
		if (!canAcquirePromotion((PromotionTypes)kEvent.getUnitPromotion()))
		{
			return false;
		}
	}

	if (kEvent.getUnitImmobileTurns() > 0)
	{
		if (!canAttack())
		{
			return false;
		}
	}

	return true;
}

void CvUnit::applyEvent(EventTypes eEvent)
{
	if (!canApplyEvent(eEvent))
	{
		return;
	}
	
	CvEventInfo& kEvent = GC.getEventInfo(eEvent);

	if (0 != kEvent.getUnitExperience())
	{
		setDamage(0);
		changeExperience(kEvent.getUnitExperience());
	}

	if (NO_PROMOTION != kEvent.getUnitPromotion())
	{
		setHasPromotion((PromotionTypes)kEvent.getUnitPromotion(), true);
	}

	if (kEvent.getUnitImmobileTurns() > 0)
	{
		changeImmobileTimer(kEvent.getUnitImmobileTurns());
		CvWString szText = gDLL->getText("TXT_KEY_EVENT_UNIT_IMMOBILE", getNameKey(), kEvent.getUnitImmobileTurns());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szText, "AS2D_UNITGIFTED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"), getX_INLINE(), getY_INLINE(), true, true);
	}

	CvWString szNameKey(kEvent.getUnitNameKey());

	if (!szNameKey.empty())
	{
		setName(gDLL->getText(kEvent.getUnitNameKey()));
	}

	if (kEvent.isDisbandUnit())
	{
		kill(false);
	}
}

const CvArtInfoUnit* CvUnit::getArtInfo(int i, EraTypes eEra) const
{	
	// gdam: bug fix by me(not sure if it was real bug), lion would get here with civ type = -1
	UnitArtStyleTypes unitArtStyleType = NO_UNIT_ARTSTYLE;
	if (NO_CIVILIZATION != getCivilizationType()) {
		unitArtStyleType = (UnitArtStyleTypes) GC.getCivilizationInfo(getCivilizationType()).getUnitArtStyleType();
	}
	return m_pUnitInfo->getArtInfo(i, eEra, unitArtStyleType);
}

const TCHAR* CvUnit::getButton() const
{
	const CvArtInfoUnit* pArtInfo = getArtInfo(0, GET_PLAYER(getOwnerINLINE()).getCurrentEra());

	if (NULL != pArtInfo)
	{
		return pArtInfo->getButton();
	}

	return m_pUnitInfo->getButton();
}

int CvUnit::getGroupSize() const
{
	return m_pUnitInfo->getGroupSize();
}

int CvUnit::getGroupDefinitions() const
{
	return m_pUnitInfo->getGroupDefinitions();
}

int CvUnit::getUnitGroupRequired(int i) const
{
	return m_pUnitInfo->getUnitGroupRequired(i);
}

bool CvUnit::isRenderAlways() const
{
	return m_pUnitInfo->isRenderAlways();
}

float CvUnit::getAnimationMaxSpeed() const
{
	return m_pUnitInfo->getUnitMaxSpeed();
}

float CvUnit::getAnimationPadTime() const
{
	return m_pUnitInfo->getUnitPadTime();
}

const char* CvUnit::getFormationType() const
{
	return m_pUnitInfo->getFormationType();
}

bool CvUnit::isMechUnit() const
{
	return m_pUnitInfo->isMechUnit();
}

bool CvUnit::isRenderBelowWater() const
{
	return m_pUnitInfo->isRenderBelowWater();
}

int CvUnit::getRenderPriority(UnitSubEntityTypes eUnitSubEntity, int iMeshGroupType, int UNIT_MAX_SUB_TYPES) const
{
	if (eUnitSubEntity == UNIT_SUB_ENTITY_SIEGE_TOWER)
	{
		return (getOwner() * (GC.getNumUnitInfos() + 2) * UNIT_MAX_SUB_TYPES) + iMeshGroupType;
	}
	else
	{
		return (getOwner() * (GC.getNumUnitInfos() + 2) * UNIT_MAX_SUB_TYPES) + m_eUnitType * UNIT_MAX_SUB_TYPES + iMeshGroupType;
	}
}

bool CvUnit::isAlwaysHostile(const CvPlot* pPlot) const
{
	if (!m_pUnitInfo->isAlwaysHostile())
	{
		return false;
	}

	if (NULL != pPlot && pPlot->isCity(true, getTeam()))
	{
		return false;
	}

	return true;
}

bool CvUnit::verifyStackValid()
{
	if (!alwaysInvisible())
	{
		if (plot()->isVisibleEnemyUnit(this))
		{
			return jumpToNearestValidPlot();
		}
	}

	return true;
}


// Private Functions...

//check if quick combat
bool CvUnit::isCombatVisible(const CvUnit* pDefender) const
{
	bool bVisible = false;

	if (!m_pUnitInfo->isQuickCombat())
	{
		if (NULL == pDefender || !pDefender->getUnitInfo().isQuickCombat())
		{
			if (isHuman())
			{
				if (!GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_QUICK_ATTACK))
				{
					bVisible = true;
				}
			}
			else if (NULL != pDefender && pDefender->isHuman())
			{
				if (!GET_PLAYER(pDefender->getOwnerINLINE()).isOption(PLAYEROPTION_QUICK_DEFENSE))
				{
					bVisible = true;
				}
			}
		}
	}

	return bVisible;
}

// used by the executable for the red glow and plot indicators
bool CvUnit::shouldShowEnemyGlow(TeamTypes eForTeam) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	if (getDomainType() == DOMAIN_AIR)
	{
		return false;
	}

	if (!canFight())
	{
		return false;
	}

	CvPlot* pPlot = plot();
	if (pPlot == NULL)
	{
		return false;
	}

	TeamTypes ePlotTeam = pPlot->getTeam();
	if (ePlotTeam != eForTeam)
	{
		return false;
	}

	if (!isEnemy(ePlotTeam))
	{
		return false;
	}

	return true;
}

bool CvUnit::shouldShowFoundBorders() const
{
	return isFound();
}


void CvUnit::cheat(bool bCtrl, bool bAlt, bool bShift)
{
	if (gDLL->getChtLvl() > 0)
	{
		if (bCtrl)
		{
			setPromotionReady(true);
		}
	}
}

float CvUnit::getHealthBarModifier() const
{
	return (GC.getDefineFLOAT("HEALTH_BAR_WIDTH") / (GC.getGameINLINE().getBestLandUnitCombat() * 2));
}

void CvUnit::getLayerAnimationPaths(std::vector<AnimationPathTypes>& aAnimationPaths) const
{
	for (int i=0; i < GC.getNumPromotionInfos(); ++i)
	{
		PromotionTypes ePromotion = (PromotionTypes) i;
		if (isHasPromotion(ePromotion))
		{
			AnimationPathTypes eAnimationPath = (AnimationPathTypes) GC.getPromotionInfo(ePromotion).getLayerAnimationPath();
			if(eAnimationPath != ANIMATIONPATH_NONE)
			{
				aAnimationPaths.push_back(eAnimationPath);
			}
		}
	}
}

int CvUnit::getSelectionSoundScript() const
{
	int iScriptId = getArtInfo(0, GET_PLAYER(getOwnerINLINE()).getCurrentEra())->getSelectionSoundScriptId();
	if (iScriptId == -1)
	{
		iScriptId = GC.getCivilizationInfo(getCivilizationType()).getSelectionSoundScriptId();
	}
	return iScriptId;
}

/************************************************************************************************/
/* DCM                                     04/19/09                                Johny Smith  */
/************************************************************************************************/
// Dale - AB: Bombing START
bool CvUnit::canAirBomb1(const CvPlot* pPlot) const
{
	if (!GC.getDCM_AIR_BOMBING())
	{
		return false;
	}
	if (!GC.getUnitInfo(getUnitType()).getDCMAirBomb1())
	{
		return false;
	}
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}

	if (airBombBaseRate() == 0)
	{
		return false;
	}

	if (isMadeAttack())
	{
		return false;
	}

	return true;
}


bool CvUnit::canAirBomb1At(const CvPlot* pPlot, int iX, int iY) const
{
	CvCity* pCity;
	CvPlot* pTargetPlot;

	if (!canAirBomb1(pPlot))
	{
		return false;
	}

	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) > airRange())
	{
		return false;
	}

	if (pTargetPlot->isOwned())
	{
		if (!atWar(pTargetPlot->getTeam(), getTeam()))
		{
			return false;
		}
	}

	pCity = pTargetPlot->getPlotCity();

	if (pCity != NULL)
	{
		if (!(pCity->isBombardable(this)))
		{
			return false;
		}
	}
	else
	{
		if (pTargetPlot->getImprovementType() == NO_IMPROVEMENT)
		{
			return false;
		}

		if (GC.getImprovementInfo(pTargetPlot->getImprovementType()).isPermanent())
		{
			return false;
		}

		if (GC.getImprovementInfo(pTargetPlot->getImprovementType()).getAirBombDefense() == -1)
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::airBomb1(int iX, int iY)
{
	CvCity* pCity;
	CvPlot* pPlot;
	CvWString szBuffer;
	bool bBitter = true;

	if (!canAirBomb1At(plot(), iX, iY))
	{
		return false;
	}
	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	if (interceptTest(pPlot))
	{
		return true;
	}

/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
	setBattlePlot(pPlot);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
	pCity = pPlot->getPlotCity();
	if (bBitter)
	{
		if (pCity != NULL)
		{
			pCity->changeDefenseDamage(airBombCurrRate());
			bool bBarb = pCity->isBarbarian();
			changeExperience(GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), bBarb ? GC.getDefineINT("BARBARIAN_MAX_XP_VALUE") : -1, !bBarb, pCity->getOwnerINLINE() == getOwnerINLINE());

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_DEFENSES_REDUCED_TO", pCity->getNameKey(), pCity->getDefenseModifier(false), getNameKey());
			gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_DEFENSES_REDUCED_TO", getNameKey(), pCity->getNameKey(), pCity->getDefenseModifier(false));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE());
		}
		else
		{
			if (pPlot->getImprovementType() != NO_IMPROVEMENT)
			{
				if (GC.getGameINLINE().getSorenRandNum(airBombCurrRate(), "Air Bomb - Offense") >=
						GC.getGameINLINE().getSorenRandNum(GC.getImprovementInfo(pPlot->getImprovementType()).getAirBombDefense(), "Air Bomb - Defense"))
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_DESTROYED_IMP", getNameKey(), GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_PILLAGE", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

					if (pPlot->isOwned())
					{
						szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_IMP_WAS_DESTROYED", GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide(), getNameKey(), GET_PLAYER(getOwnerINLINE()).getCivilizationAdjectiveKey());
						gDLL->getInterfaceIFace()->addMessage(pPlot->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_PILLAGED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
						bool bBarb = GET_PLAYER(pPlot->getOwnerINLINE()).isBarbarian();
						changeExperience(GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), bBarb ? GC.getDefineINT("BARBARIAN_MAX_XP_VALUE") : -1, !bBarb, pPlot->getOwnerINLINE() == getOwnerINLINE());
					}

					pPlot->setImprovementType((ImprovementTypes)(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage()));
				}
				else
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_FAIL_DESTROY_IMP", getNameKey(), GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMB_FAILS", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
				}
			}
		}
	}

	setReconPlot(pPlot);

	setMadeAttack(true);
	changeMoves(GC.getMOVE_DENOMINATOR());

	if (pPlot->isActiveVisible(false))
	{
		CvAirMissionDefinition kAirMission;
		kAirMission.setMissionType(MISSION_AIRBOMB);
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
		kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
		kAirMission.setPlot(pPlot);
		kAirMission.setMissionTime(GC.getMissionInfo((MissionTypes)MISSION_AIRBOMB).getTime() * gDLL->getSecsPerTurn());

		gDLL->getEntityIFace()->AddMission(&kAirMission);
	}
	if (isSuicide())
	{
		kill(true);
	}
	return true;
}

bool CvUnit::canAirBomb2(const CvPlot* pPlot) const
{
	if (!GC.getDCM_AIR_BOMBING())
	{
		return false;
	}
	if (!GC.getUnitInfo(getUnitType()).getDCMAirBomb2())
	{
		return false;
	}
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}
	if (airBombBaseRate() == 0)
	{
		return false;
	}
	if (isMadeAttack())
	{
		return false;
	}
	return true;
}


bool CvUnit::canAirBomb2At(const CvPlot* pPlot, int iX, int iY) const
{
	CvCity* pCity;
	CvPlot* pTargetPlot;
	if (!canAirBomb2(pPlot))
	{
		return false;
	}
	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) > airRange())
	{
		return false;
	}
	if (pTargetPlot->isOwned())
	{
		if (!atWar(pTargetPlot->getTeam(), getTeam()))
		{
			return false;
		}
	}
	pCity = pTargetPlot->getPlotCity();
	if (pCity == NULL)
	{
		return false;
	}
	return true;
}


bool CvUnit::airBomb2(int iX, int iY)
{
	CvCity* pCity;
	CvPlot* pPlot;
	CvWString szBuffer;
	int build, iI, iAttempts, iMaxAttempts;
	bool bBitter = true;
	bool bNoTarget = true;
	bool abTech1 = false;
	bool abTech2 = false;
	CLinkList<int> buildingList;

	if (!canAirBomb2At(plot(), iX, iY))
	{
		return false;
	}
	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	if (interceptTest(pPlot))
	{
		return true;
	}

/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
	setBattlePlot(pPlot);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
	pCity = pPlot->getPlotCity();

	for (iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		if (GC.getTechInfo((TechTypes)iI).getDCMAirBombTech1())
		{
            if (GET_TEAM(GET_PLAYER(getOwnerINLINE()).getTeam()).isHasTech((TechTypes)iI))
			{
				abTech1 = true;
			}
		}
		if (GC.getTechInfo((TechTypes)iI).getDCMAirBombTech2())
		{
            if (GET_TEAM(GET_PLAYER(getOwnerINLINE()).getTeam()).isHasTech((TechTypes)iI))
			{
				abTech2 = true;
			}
		}
	}
	if (bBitter)
	{
		if (pCity != NULL)
		{
			buildingList.clear();
			for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
			{
				if (GC.getBuildingInfo((BuildingTypes)iI).getDCMAirbombMission() == 2)
				{
					buildingList.insertAtEnd(iI);
				}
			}
			if (buildingList.getLength() > 0)
			{
				iI = GC.getGameINLINE().getSorenRandNum(buildingList.getLength(), "Airbomb building");
				build = buildingList.nodeNum(iI)->m_data;
				if (pCity->getNumRealBuilding((BuildingTypes)build) > 0)
				{
					bNoTarget = false;
				}
				if (abTech1)
				{
					iAttempts = 0;
					if (abTech2)
					{
						iMaxAttempts = 8;
					}
					else
					{
						iMaxAttempts = 4;
					}
					while (bNoTarget)
					{
						iAttempts++;
						iI = GC.getGameINLINE().getSorenRandNum(buildingList.getLength(), "Airbomb building");
						build = buildingList.nodeNum(iI)->m_data;
						if (pCity->getNumRealBuilding((BuildingTypes)build) > 0 || iAttempts > iMaxAttempts)
						{
							bNoTarget = false;
						}
					}
				}
				if (pCity->getNumRealBuilding((BuildingTypes)build) > 0)
				{
					bNoTarget = false;
					bool bBarb = pCity->isBarbarian();
					changeExperience(GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), bBarb ? GC.getDefineINT("BARBARIAN_MAX_XP_VALUE") : -1, !bBarb, pCity->getOwnerINLINE() == getOwnerINLINE());
					pCity->setNumRealBuilding((BuildingTypes)build, 0);
					szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_AIRBOMB2SUCCESS", GC.getBuildingInfo((BuildingTypes)build).getTextKeyWide(), pCity->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIRBOMB2SUCCESS", GC.getBuildingInfo((BuildingTypes)build).getTextKeyWide(), pCity->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
				} else {
					szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_AIRBOMB2FAIL", pCity->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIRBOMB2FAIL", pCity->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
				}
			}
			if(bNoTarget)
			{
				if(pCity->getPopulation() > 1)
				{
					if(GC.getGameINLINE().getSorenRandNum(5, "Airbomb population") < 2)
					{
						pCity->changePopulation(-1);
						szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIRBOMB_POP");
						gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
						szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_AIRBOMB_POP");
						gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
					}
				}
			}
		}
	}
	setReconPlot(pPlot);
	setMadeAttack(true);
	changeMoves(GC.getMOVE_DENOMINATOR());
	if (pPlot->isActiveVisible(false))
	{
		CvAirMissionDefinition kAirMission;
		kAirMission.setMissionType(MISSION_AIRBOMB);
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
		kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
		kAirMission.setPlot(pPlot);
		kAirMission.setMissionTime(GC.getMissionInfo((MissionTypes)MISSION_AIRBOMB).getTime() * gDLL->getSecsPerTurn());
		gDLL->getEntityIFace()->AddMission(&kAirMission);
	}
	if (isSuicide())
	{
		kill(true);
	}
	return true;
}

bool CvUnit::canAirBomb3(const CvPlot* pPlot) const
{
	if (!GC.getDCM_AIR_BOMBING())
	{
		return false;
	}
	if (!GC.getUnitInfo(getUnitType()).getDCMAirBomb3())
	{
		return false;
	}
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}
	if (airBombBaseRate() == 0)
	{
		return false;
	}
	if (isMadeAttack())
	{
		return false;
	}
	return true;
}


bool CvUnit::canAirBomb3At(const CvPlot* pPlot, int iX, int iY) const
{
	CvCity* pCity;
	CvPlot* pTargetPlot;
	if (!canAirBomb3(pPlot))
	{
		return false;
	}
	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) > airRange())
	{
		return false;
	}
	if (pTargetPlot->isOwned())
	{
		if (!atWar(pTargetPlot->getTeam(), getTeam()))
		{
			return false;
		}
	}
	pCity = pTargetPlot->getPlotCity();
	if (pCity == NULL)
	{
		return false;
	}
	return true;
}


bool CvUnit::airBomb3(int iX, int iY)
{
	CvCity* pCity;
	CvPlot* pPlot;
	CvWString szBuffer;
	int build, iI, iAttempts, iMaxAttempts;
	bool bNoTarget = true;
	bool bBitter = true;
	bool abTech1 = false;
	bool abTech2 = false;
	CLinkList<int> buildingList;

	if (!canAirBomb3At(plot(), iX, iY))
	{
		return false;
	}
	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	if (interceptTest(pPlot))
	{
		return true;
	}

/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
	setBattlePlot(pPlot);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
	pCity = pPlot->getPlotCity();

	for (iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		if (GC.getTechInfo((TechTypes)iI).getDCMAirBombTech1())
		{
            if (GET_TEAM(GET_PLAYER(getOwnerINLINE()).getTeam()).isHasTech((TechTypes)iI))
			{
				abTech1 = true;
			}
		}
		if (GC.getTechInfo((TechTypes)iI).getDCMAirBombTech2())
		{
            if (GET_TEAM(GET_PLAYER(getOwnerINLINE()).getTeam()).isHasTech((TechTypes)iI))
			{
				abTech2 = true;
			}
		}
	}
	if (bBitter)
	{
		if (pCity != NULL)
		{
			buildingList.clear();
			for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
			{
				if (GC.getBuildingInfo((BuildingTypes)iI).getDCMAirbombMission() == 3)
				{
					buildingList.insertAtEnd(iI);
				}
			}
			if (buildingList.getLength() > 0)
			{
				iI = GC.getGameINLINE().getSorenRandNum(buildingList.getLength(), "Airbomb building");
				build = buildingList.nodeNum(iI)->m_data;
				if (pCity->getNumRealBuilding((BuildingTypes)build) > 0)
				{
					bNoTarget = false;
				}
				if (abTech1)
				{
					iAttempts = 0;
					if (abTech2)
					{
						iMaxAttempts = 8;
					}
					else
					{
						iMaxAttempts = 4;
					}
					while (bNoTarget)
					{
						iAttempts++;
						iI = GC.getGameINLINE().getSorenRandNum(buildingList.getLength(), "Airbomb building");
						build = buildingList.nodeNum(iI)->m_data;
						if (pCity->getNumRealBuilding((BuildingTypes)build) > 0 || iAttempts > iMaxAttempts)
						{
							bNoTarget = false;
						}
					}
				}
				if (pCity->getNumRealBuilding((BuildingTypes)build) > 0)
				{
					pCity->setNumRealBuilding((BuildingTypes)build, 0);
					bool bBarb = pCity->isBarbarian();
					changeExperience(GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), bBarb ? GC.getDefineINT("BARBARIAN_MAX_XP_VALUE") : -1, !bBarb, pCity->getOwnerINLINE() == getOwnerINLINE());
					szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_AIRBOMB3SUCCESS", GC.getBuildingInfo((BuildingTypes)build).getTextKeyWide(), pCity->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIRBOMB3SUCCESS", GC.getBuildingInfo((BuildingTypes)build).getTextKeyWide(), pCity->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
				} else {
					szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_AIRBOMB3FAIL", pCity->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIRBOMB3FAIL", pCity->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
				}
			}
			if(bNoTarget)
			{
				if(pCity->getPopulation() > 1)
				{
					if(GC.getGameINLINE().getSorenRandNum(5, "Airbomb population") < 1)
					{
						pCity->changePopulation(-1);
						szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIRBOMB_POP");
						gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
						szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_AIRBOMB_POP");
						gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
					}
				}
			}
		}
	}
	setReconPlot(pPlot);
	setMadeAttack(true);
	changeMoves(GC.getMOVE_DENOMINATOR());
	if (pPlot->isActiveVisible(false))
	{
		CvAirMissionDefinition kAirMission;
		kAirMission.setMissionType(MISSION_AIRBOMB);
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
		kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
		kAirMission.setPlot(pPlot);
		kAirMission.setMissionTime(GC.getMissionInfo((MissionTypes)MISSION_AIRBOMB).getTime() * gDLL->getSecsPerTurn());
		gDLL->getEntityIFace()->AddMission(&kAirMission);
	}
	if (isSuicide())
	{
		kill(true);
	}
	return true;
}

bool CvUnit::canAirBomb4(const CvPlot* pPlot) const
{
	if (!GC.getDCM_AIR_BOMBING())
	{
		return false;
	}
	if (!GC.getUnitInfo(getUnitType()).getDCMAirBomb4())
	{
		return false;
	}
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}
	if (airBombBaseRate() == 0)
	{
		return false;
	}
	if (isMadeAttack())
	{
		return false;
	}
	return true;
}


bool CvUnit::canAirBomb4At(const CvPlot* pPlot, int iX, int iY) const
{
	CvCity* pCity;
	CvPlot* pTargetPlot;
	CvUnit* pLoopUnit;
	int iI, iLoop;
	if (!canAirBomb4(pPlot))
	{
		return false;
	}
	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) > airRange())
	{
		return false;
	}
	if (pTargetPlot->isOwned())
	{
		if (!atWar(pTargetPlot->getTeam(), getTeam()))
		{
			return false;
		}
	}
	pCity = pTargetPlot->getPlotCity();
	if (pCity != NULL)
	{
		for (iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			if (atWar(GET_PLAYER((PlayerTypes)iI).getTeam(), getTeam()))
			{
				for(pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
				{
					if (pLoopUnit->plot() == pTargetPlot)
					{
						if (pLoopUnit->getDomainType() == DOMAIN_SEA)
						{
							return true;
						}
					}
				}
			}
		}
	}
	if (pTargetPlot->getImprovementType() != NO_IMPROVEMENT)
	{
		if (GC.getImprovementInfo(pTargetPlot->getImprovementType()).isActsAsCity() && pCity == NULL)
		{
			for (iI = 0; iI < MAX_PLAYERS; ++iI)
			{
				if (atWar(GET_PLAYER((PlayerTypes)iI).getTeam(), getTeam()))
				{
					for(pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
					{
						if (pLoopUnit->plot() == pTargetPlot)
						{
							if (pLoopUnit->getDomainType() == DOMAIN_SEA)
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


bool CvUnit::airBomb4(int iX, int iY)
{
	CvCity* pCity;
	CvUnit* pUnit;
	CvPlot* pPlot;
	CvWString szBuffer;
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int iDamage, iCount;
	int iUnitDamage;
	bool bBitter = true;
	bool bNoTarget = true;

	if (!canAirBomb4At(plot(), iX, iY))
	{
		return false;
	}
	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	if (interceptTest(pPlot))
	{
		return true;
	}

/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
	setBattlePlot(pPlot);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
	pCity = pPlot->getPlotCity();
	iCount = 0;
	pUnit = NULL;
	pUnitNode = pPlot->headUnitNode();
	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);
		if (pLoopUnit->getDomainType() == DOMAIN_SEA)
		{
			iCount++;
		}
	}
	iCount = (GC.getGameINLINE().getSorenRandNum(iCount, "Choose ship") + 1);
	pUnitNode = pPlot->headUnitNode();
	while (iCount > 0)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);
		if (pLoopUnit->getDomainType() == DOMAIN_SEA)
		{
			iCount--;
			pUnit = pLoopUnit;
		}
	}
	if (bBitter)
	{
//		if (pCity != NULL)
		{
			if (pUnit != NULL)
			{
				bNoTarget = false;
				if (pCity != NULL)
				{
					bool bBarb = pCity->isBarbarian();
					changeExperience(GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), bBarb ? GC.getDefineINT("BARBARIAN_MAX_XP_VALUE") : -1, !bBarb, pCity->getOwnerINLINE() == getOwnerINLINE());
				}
				iDamage = (airCombatDamage(pUnit) * 2);
				iUnitDamage = std::max(pUnit->getDamage(), std::min((pUnit->getDamage() + iDamage), airCombatLimit()));
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_AIR", pUnit->getNameKey(), getNameKey(), -(((iUnitDamage - pUnit->getDamage()) * 100) / pUnit->maxHitPoints()));
				gDLL->getInterfaceIFace()->addMessage(pUnit->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_AIR_ATTACK", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR", getNameKey(), pUnit->getNameKey(), -(((iUnitDamage - pUnit->getDamage()) * 100) / pUnit->maxHitPoints()));
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_AIR_ATTACKED", MESSAGE_TYPE_INFO, pUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
				pUnit->setDamage(iUnitDamage, getOwnerINLINE());
				if (GC.getGameINLINE().getSorenRandNum(100, "Spin the dice") < 50)
				{
					pUnit->setDamage(GC.getMAX_HIT_POINTS());
					szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMYSINK_AIRBOMB4SUCCESS", pUnit->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(pUnit->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOUSINK_AIRBOMB4SUCCESS", pUnit->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
				}
			} else {
				szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_AIRBOMB4FAIL", pCity->getNameKey());
				gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIRBOMB4FAIL", pCity->getNameKey());
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
			}
			if(bNoTarget)
			{
				if (pCity != NULL)
				{
					if(pCity->getPopulation() > 1)
					{
						if(GC.getGameINLINE().getSorenRandNum(5, "Airbomb population") < 1)
						{
							pCity->changePopulation(-1);
							szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIRBOMB_POP");
							gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
							szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_AIRBOMB_POP");
							gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
						}
					}
				}
			}
		}
	}
	setReconPlot(pPlot);
	setMadeAttack(true);
	changeMoves(GC.getMOVE_DENOMINATOR());
	if (pPlot->isActiveVisible(false))
	{
		CvAirMissionDefinition kAirMission;
		kAirMission.setMissionType(MISSION_AIRBOMB);
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
		kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
		kAirMission.setPlot(pPlot);
		kAirMission.setMissionTime(GC.getMissionInfo((MissionTypes)MISSION_AIRBOMB).getTime() * gDLL->getSecsPerTurn());
		gDLL->getEntityIFace()->AddMission(&kAirMission);
	}
	if (isSuicide())
	{
		kill(true);
	}
	return true;
}

bool CvUnit::canAirBomb5(const CvPlot* pPlot) const
{
	if (!GC.getDCM_AIR_BOMBING())
	{
		return false;
	}
	if (!GC.getUnitInfo(getUnitType()).getDCMAirBomb5())
	{
		return false;
	}
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}
	if (airBombBaseRate() == 0)
	{
		return false;
	}
	if (isMadeAttack())
	{
		return false;
	}
	return true;
}


bool CvUnit::canAirBomb5At(const CvPlot* pPlot, int iX, int iY) const
{
	CvCity* pCity;
	CvPlot* pTargetPlot;
	if (!canAirBomb5(pPlot))
	{
		return false;
	}
	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) > airRange())
	{
		return false;
	}
	if (pTargetPlot->isOwned())
	{
		if (!atWar(pTargetPlot->getTeam(), getTeam()))
		{
			return false;
		}
	}
	pCity = pTargetPlot->getPlotCity();
	if (pCity == NULL)
	{
		return false;
	}
	return true;
}


bool CvUnit::airBomb5(int iX, int iY)
{
	CvCity* pCity;
	CvPlot* pPlot;
	CvWString szBuffer;
	bool bNoTarget = true;
	bool bBitter = true;

	if (!canAirBomb5At(plot(), iX, iY))
	{
		return false;
	}
	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	if (interceptTest(pPlot))
	{
		return true;
	}

/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
	setBattlePlot(pPlot);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
	pCity = pPlot->getPlotCity();

	if (bBitter)
	{
		if (pCity != NULL)
		{
			if (GC.getGameINLINE().getSorenRandNum(100, "Airbomb") < 50)
			{
				bNoTarget = false;
				pCity->setProduction(pCity->getProduction() / 2);
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIRBOMB5SUCCESS", pCity->getNameKey());
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
				szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_AIRBOMB5SUCCESS", pCity->getNameKey());
				gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
				bool bBarb = pCity->isBarbarian();
				changeExperience(GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), bBarb ? GC.getDefineINT("BARBARIAN_MAX_XP_VALUE") : -1, !bBarb, pCity->getOwnerINLINE() == getOwnerINLINE());
			} else {
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIRBOMB5FAIL", pCity->getNameKey());
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
				szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_AIRBOMB5FAIL", pCity->getNameKey());
				gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
			}
			if(bNoTarget)
			{
				if(pCity->getPopulation() > 1)
				{
					if(GC.getGameINLINE().getSorenRandNum(5, "Airbomb population") < 1)
					{
						pCity->changePopulation(-1);
						szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIRBOMB_POP");
						gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
						szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_AIRBOMB_POP");
						gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);
					}
				}
			}
		}
	}
	setReconPlot(pPlot);
	setMadeAttack(true);
	changeMoves(GC.getMOVE_DENOMINATOR());
	if (pPlot->isActiveVisible(false))
	{
		CvAirMissionDefinition kAirMission;
		kAirMission.setMissionType(MISSION_AIRBOMB);
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
		kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
		kAirMission.setPlot(pPlot);
		kAirMission.setMissionTime(GC.getMissionInfo((MissionTypes)MISSION_AIRBOMB).getTime() * gDLL->getSecsPerTurn());
		gDLL->getEntityIFace()->AddMission(&kAirMission);
	}
	if (isSuicide())
	{
		kill(true);
	}
	return true;
}
// Dale - AB: Bombing END

// Dale - RB: Field Bombard START
bool CvUnit::canRBombard(const CvPlot* pPlot) const
{
	// RevolutionDCM added tests
	bool seaBombardTile = (pPlot->isWater() || plot()->isWater());

	if (!GC.getDCM_RANGE_BOMBARD() && !GC.getDefineINT("DCM_NAVAL_BOMBARD"))
	{
		return false;
	}

	if (!GC.getDCM_RANGE_BOMBARD() && GC.getDefineINT("DCM_NAVAL_BOMBARD"))
	{
		if (!seaBombardTile)
		{
			return false;
		}
	}

	if (GC.getDCM_RANGE_BOMBARD() && !GC.getDefineINT("DCM_NAVAL_BOMBARD"))
	{
		if (seaBombardTile) 
		{
			return false;
		}
	}

	if(getDCMBombRange() < 1)
	{
        return false;
	}

	if (getDomainType() == DOMAIN_AIR)
	{
		return false;
	}
	// RevolutionDCM - end

	if (bombardRate() <= 0)
	{
		return false;
	}

	if (isMadeAttack())
	{
		return false;
	}

	if (isCargo())
	{
		return false;
	}

	return true;
}

bool CvUnit::canBombardAtRanged(const CvPlot* pPlot, int iX, int iY) const
{
	CvCity* pCity;
	CvPlot* pTargetPlot;
	if (!canRBombard(pPlot))
	{
		return false;
	}

	if(iX < 0 || iY < 0)
	{
		return false;
	}
	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) > getDCMBombRange())
	{
		return false;
	}

	// RevolutionDCM - ranged bombardment/sea bombardment
	if (GC.getDCM_RANGE_BOMBARD() && pTargetPlot->isWater())
	{
		if (!GC.getDefineINT("DCM_NAVAL_BOMBARD"))
		{
			return false;
		}
	}
	// RevolutionDCM - end

	if (pTargetPlot->isOwned())
	{
		if(pTargetPlot->getTeam() != getTeam())
		{
			if (!atWar(pTargetPlot->getTeam(), getTeam()))
			{
				return false;
			}
		}
	}
	pCity = pTargetPlot->getPlotCity();
	if (pCity != NULL)
	{
		if (!(pCity->isBombardable(this)))
		{
			if(pTargetPlot->getNumVisibleEnemyDefenders(this) == 0)
			{
                return false;
			}
		}
	} else {
		if(pTargetPlot->getNumVisibleEnemyDefenders(this) == 0)
		{
			if (pTargetPlot->getImprovementType() == NO_IMPROVEMENT)
			{
				return false;
			}
			if (GC.getImprovementInfo(pTargetPlot->getImprovementType()).isPermanent())
			{
				return false;
			}
			if (GC.getImprovementInfo(pTargetPlot->getImprovementType()).getAirBombDefense() == -1)
			{
				return false;
			}
		}
	}
	return true;
}

// RevolutionDCM - significant chances to this function
bool CvUnit::bombardRanged(int iX, int iY, bool sAttack)
{
	CvCity* pCity;
	CvPlot* pPlot;
	CvWString szBuffer;
	CvUnit* pLoopUnit = NULL;

	if (!canBombardAtRanged(plot(), iX, iY))
	{
		return false;
	}

	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	if (pPlot == NULL)
	{
		return false;
	}
		
	int modified_accuracy = GC.getDefineINT("DCM_RB_CITY_INACCURACY");
	if (modified_accuracy <= 0)
	{
		modified_accuracy = 350; // default
	}

	int modified_miss = GC.getDefineINT("DCM_RB_CITYBOMBARD_CHANCE");
	if (modified_miss <= 0)
	{
		modified_miss = 5; // default
	}

	pCity = pPlot->getPlotCity();
	if (pCity != NULL)
	{
		int bombardCity = GC.getGameINLINE().getSorenRandNum(modified_miss, "Range Bombard City");
		// Introduce a slight chance that ranged bombardment hits city defenders rather than defenses
		if(pCity->isBombardable(this) && bombardCity > 0)
		{
			// RevolutionDCM - city bombard no different from vanilla in essense.
			int iBombardModifier = 0;

			if (!ignoreBuildingDefense())
			{
				iBombardModifier -= pCity->getBuildingBombardDefense();
			}
			// RevolutionDCM - only difference to standard city bombardment is to scale back 
			// bombard damage according to distance from the city.
			int distance = plotDistance(plot()->getX_INLINE(), plot()->getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			if (distance > 1)
			{
				iBombardModifier = -50;
			}

			pCity->changeDefenseModifier(-(bombardRate() * std::max(0, 100 + iBombardModifier)) / 100);

			setMadeAttack(true);
			changeMoves(GC.getMOVE_DENOMINATOR());

			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_DEFENSES_IN_CITY_REDUCED_TO", pCity->getNameKey(), pCity->getDefenseModifier(false), GET_PLAYER(getOwnerINLINE()).getNameKey());
			gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_REDUCE_CITY_DEFENSES", getNameKey(), pCity->getNameKey(), pCity->getDefenseModifier(false));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE());

/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
			setBattlePlot(pPlot);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
		} else		
		{
			// Give a reduced chance of range bombarding city defenders by default
			int odds = modified_accuracy;
			// Occasionally give city bombard a better chance at hitting city defenders if the city defenses
			// are still bombardable. This produces differentiation from standard bombard that seige also have
			// available to them as an option, and compensates range bombard for not lowering city defenses.
			if (pCity->isBombardable(this) && bombardCity == 0)
			{
				odds = 100; 
			}
			// standard odds made worse if greater than one tile out 
			int shotDistance = plotDistance(plot()->getX_INLINE(), plot()->getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			odds += (shotDistance - 1) * 50;

			pLoopUnit = pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);
			if (pLoopUnit != NULL)
			{
				// RevolutionDCM - change proposal to ranged bombardment. Only collateral damage can be issued.
				if (GC.getGameINLINE().getSorenRandNum(odds, "Bombard Accuracy") <= getDCMBombAccuracy())
				{
					szBuffer = gDLL->getText("TXT_KEY_HAS_RANGED_BOMBARD_ATTACKED", getNameKey());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE());
					szBuffer = gDLL->getText("TXT_KEY_HAS_BEEN_RANGED_BOMBARD_ATTACKED", getNameKey());
					gDLL->getInterfaceIFace()->addMessage(pLoopUnit->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
					if (GC.getDefineINT("DCM_NAVAL_BOMBARD"))
					{	//standard DCM damage calculation
						int iUnitDamage = GC.getGameINLINE().getSorenRandNum(bombardRate(), "Bombard damage");//(rand() % (bombardRate()));
						collateralCombat(pPlot, pLoopUnit);
						pLoopUnit->changeDamage(iUnitDamage, getOwner());
					} else
					{
						collateralCombat(pPlot, pLoopUnit);
					}
				} else
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_BOMB_MISSED", getNameKey());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE());
					szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_BOMB_MISSED", getNameKey());
					gDLL->getInterfaceIFace()->addMessage(pLoopUnit->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
				}
			}
/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
			setBattlePlot(pPlot);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
		}
	} else
	{
		// Field bombard case. If bombarding from a city, odds of success are reduced
		// For the sake of game balance by default.
		int odds = 100;
		if (plot()->getPlotCity() != NULL)
		{
			odds = modified_accuracy;			
		}
		// standard odds made worse if greater than one tile out 
		int shotDistance = plotDistance(plot()->getX_INLINE(), plot()->getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		odds += (shotDistance - 1) * 50;

		pLoopUnit = pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);
		if (pLoopUnit != NULL)
		{
			//RevolutionDCM - change proposal to ranged bombardment. Only collateral damage can be issued.
			if (GC.getGameINLINE().getSorenRandNum(odds, "Bombard Accuracy") <= getDCMBombAccuracy())
			{
				szBuffer = gDLL->getText("TXT_KEY_HAS_RANGED_BOMBARD_ATTACKED", getNameKey());
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE());
				szBuffer = gDLL->getText("TXT_KEY_HAS_BEEN_RANGED_BOMBARD_ATTACKED", getNameKey());
				gDLL->getInterfaceIFace()->addMessage(pLoopUnit->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
				if (GC.getDefineINT("DCM_NAVAL_BOMBARD"))
				{	//standard DCM damage calculation
					int iUnitDamage = GC.getGameINLINE().getSorenRandNum(bombardRate(), "Bombard damage");//(rand() % (bombardRate()));
					collateralCombat(pPlot, pLoopUnit);
					pLoopUnit->changeDamage(iUnitDamage, getOwner());
				} else
				{
					collateralCombat(pPlot, pLoopUnit);
				}
			} else
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_BOMB_MISSED", getNameKey());
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE());
				szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_BOMB_MISSED", getNameKey());
				gDLL->getInterfaceIFace()->addMessage(pLoopUnit->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
			}
/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
			setBattlePlot(pPlot);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
		} else
		{
			// Plot bombardment
			if (pPlot->getImprovementType() != NO_IMPROVEMENT) 
			{
				if (GC.getGameINLINE().getSorenRandNum(bombardRate(), "Bomb - Offense") >=
						GC.getGameINLINE().getSorenRandNum(GC.getImprovementInfo(pPlot->getImprovementType()).getAirBombDefense(), "Bomb - Defense"))
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_DESTROYED_IMP", getNameKey(), GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_PILLAGE", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
					if (pPlot->isOwned())
					{
						szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_IMP_WAS_DESTROYED", GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide(), getNameKey(), GET_PLAYER(getOwnerINLINE()).getCivilizationAdjectiveKey());
						gDLL->getInterfaceIFace()->addMessage(pPlot->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_PILLAGE", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
					}
					pPlot->setImprovementType((ImprovementTypes)(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage()));
				}
				else
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_FAIL_DESTROY_IMP", getNameKey(), GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMB_FAILS", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
				}
			}
/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
			setBattlePlot(pPlot);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
		}
	}
	if (!sAttack)
	{
		setMadeAttack(true);
		changeMoves(GC.getMOVE_DENOMINATOR());
	}

	if (pPlot->isActiveVisible(false))
	{
		// Bombard entity mission
		CvMissionDefinition kDefiniton;
		kDefiniton.setMissionTime(GC.getMissionInfo(MISSION_RBOMBARD).getTime() * gDLL->getSecsPerTurn());
		kDefiniton.setMissionType(MISSION_BOMBARD);
		kDefiniton.setPlot(pPlot);
		kDefiniton.setUnit(BATTLE_UNIT_ATTACKER, this);
		kDefiniton.setUnit(BATTLE_UNIT_DEFENDER, pLoopUnit);
		gDLL->getEntityIFace()->AddMission(&kDefiniton);
	}
	return true;
}

// RevolutionDCM - ranged bombard
// Estimate if a unit stack is worth range bombarding
bool CvUnit::isRbombardable(int iMinStack)
{
	int collateralCount = 0;
	int averageDamage = 0;
	int averageProtection = 0;
	int seigeCount = 0;
	
	CvUnit* nextUnit = NULL;
	int unitCount = plot()->getNumUnits();
	if (unitCount >= iMinStack)
	{
		for (int i = 0; i < unitCount; i++)
		{
			nextUnit = plot()->getUnitByIndex(i);
			if (nextUnit != NULL)
			{
				if (nextUnit->canRBombard(plot()))
				{
					seigeCount++;
				}
				averageDamage += nextUnit->getDamage();
				averageProtection += nextUnit->getCollateralDamageProtection();
			}
		}
		if (unitCount > 0)
		{
			collateralCount = unitCount - seigeCount;
			averageDamage /= unitCount;
			averageProtection /= unitCount;
			if (collateralCount > 1 && collateralCount < 8 && averageDamage < 40 && averageProtection < 10)
			{
				return true;
			}
		}
	}
	return false;
}

int CvUnit::getRbombardSeigeCount(CvPlot* pPlot)
{
	CvUnit* nextUnit = NULL;
	int seigeCount = 0;

	if (pPlot == NULL)
	{
		return 0;
	}

	int unitCount = pPlot->getNumUnits();
	for (int i = 0; i < unitCount; i++)
	{
		nextUnit = pPlot->getUnitByIndex(i);
		if (nextUnit != NULL)
		{
			if (nextUnit->canRBombard(pPlot))
			{
				seigeCount++;
			}
		}
	}
	return seigeCount;
}
// RevolutionDCM - end

int CvUnit::getDCMBombRange() const
{
	return GC.getUnitInfo(getUnitType()).getDCMBombRange();
}

int CvUnit::getDCMBombAccuracy() const
{
	return GC.getUnitInfo(getUnitType()).getDCMBombAccuracy();
}
// Dale - RB: Field Bombard END

// Dale - SA: Stack Attack START
void CvUnit::updateStackCombat(bool bQuick)
{
	CvWString szBuffer;

	bool bFinish = false;
	bool bVisible = false;

	if (getCombatTimer() > 0)
	{
		changeCombatTimer(-1);

		if (getCombatTimer() > 0)
		{
			return;
		}
		else
		{
			bFinish = true;
		}
	}

	CvPlot* pPlot = getAttackPlot();

	if (pPlot == NULL)
	{
		return;
	}

	if (getDomainType() == DOMAIN_AIR)
	{
		updateAirStrike(pPlot, bQuick, bFinish);
		return;
	}

	CvUnit* pDefender = NULL;
	if (bFinish)
	{
		pDefender = getCombatUnit();
	}
	else
	{
		pDefender = pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);
	}

	if (pDefender == NULL)
	{
		setAttackPlot(NULL, false);
		setCombatUnit(NULL);

		getGroup()->groupMove(pPlot, true, ((canAdvance(pPlot, 0)) ? this : NULL));

		getGroup()->clearMissionQueue();

		return;
	}

	//check if quick combat
	if (!bQuick)
	{
		bVisible = isCombatVisible(pDefender);
	}

	//FAssertMsg((pPlot == pDefender->plot()), "There is not expected to be a defender or the defender's plot is expected to be pPlot (the attack plot)");

	//if not finished and not fighting yet, set up combat damage and mission
	if (!bFinish)
	{
		if (!isFighting())
		{
//			if (plot()->isFighting() || pPlot->isFighting())
//			{
//				return;
//			}

			setMadeAttack(true);

			//rotate to face plot
			DirectionTypes newDirection = estimateDirection(this->plot(), pDefender->plot());
			if (newDirection != NO_DIRECTION)
			{
				setFacingDirection(newDirection);
			}

			//rotate enemy to face us
			newDirection = estimateDirection(pDefender->plot(), this->plot());
			if (newDirection != NO_DIRECTION)
			{
				pDefender->setFacingDirection(newDirection);
			}

			setCombatUnit(pDefender, true);
			pDefender->setCombatUnit(this, false);

			pDefender->getGroup()->clearMissionQueue();

			bool bFocused = (bVisible && isCombatFocus() && gDLL->getInterfaceIFace()->isCombatFocus());

			if (bFocused)
			{
				DirectionTypes directionType = directionXY(plot(), pPlot);
				//								N			NE				E				SE					S				SW					W				NW
				NiPoint2 directions[8] = {NiPoint2(0, 1), NiPoint2(1, 1), NiPoint2(1, 0), NiPoint2(1, -1), NiPoint2(0, -1), NiPoint2(-1, -1), NiPoint2(-1, 0), NiPoint2(-1, 1)};
				NiPoint3 attackDirection = NiPoint3(directions[directionType].x, directions[directionType].y, 0);
				float plotSize = GC.getPLOT_SIZE();
				NiPoint3 lookAtPoint(plot()->getPoint().x + plotSize / 2 * attackDirection.x, plot()->getPoint().y + plotSize / 2 * attackDirection.y, (plot()->getPoint().z + pPlot->getPoint().z) / 2);
				attackDirection.Unitize();
				gDLL->getInterfaceIFace()->lookAt(lookAtPoint, (((getOwnerINLINE() != GC.getGameINLINE().getActivePlayer()) || gDLL->getGraphicOption(GRAPHICOPTION_NO_COMBAT_ZOOM)) ? CAMERALOOKAT_BATTLE : CAMERALOOKAT_BATTLE_ZOOM_IN), attackDirection);
			}
			else
			{
				PlayerTypes eAttacker = getVisualOwner(pDefender->getTeam());
				CvWString szMessage;
				if (BARBARIAN_PLAYER != eAttacker)
				{
					szMessage = gDLL->getText("TXT_KEY_MISC_YOU_UNITS_UNDER_ATTACK", GET_PLAYER(getOwnerINLINE()).getNameKey());
				}
				else
				{
					szMessage = gDLL->getText("TXT_KEY_MISC_YOU_UNITS_UNDER_ATTACK_UNKNOWN");
				}

				gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szMessage, "AS2D_COMBAT", MESSAGE_TYPE_DISPLAY_ONLY, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true);
			}
		}

		FAssertMsg(pDefender != NULL, "Defender is not assigned a valid value");

		FAssertMsg(plot()->isFighting(), "Current unit instance plot is not fighting as expected");
		FAssertMsg(pPlot->isFighting(), "pPlot is not fighting as expected");

		if (!pDefender->canDefend())
		{
			if (!bVisible)
			{
				bFinish = true;
			}
			else
			{
				CvMissionDefinition kMission;
				kMission.setMissionTime(getCombatTimer() * gDLL->getSecsPerTurn());
				kMission.setMissionType(MISSION_SURRENDER);
				kMission.setUnit(BATTLE_UNIT_ATTACKER, this);
				kMission.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
				kMission.setPlot(pPlot);
				gDLL->getEntityIFace()->AddMission(&kMission);

				// Surrender mission
				setCombatTimer(GC.getMissionInfo(MISSION_SURRENDER).getTime());

				GC.getGameINLINE().incrementTurnTimer(getCombatTimer());
			}

			// Kill them!
			pDefender->setDamage(GC.getMAX_HIT_POINTS());
		}
		else
		{
			CvBattleDefinition kBattle;
			kBattle.setUnit(BATTLE_UNIT_ATTACKER, this);
			kBattle.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
			kBattle.setDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_BEGIN, getDamage());
			kBattle.setDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_BEGIN, pDefender->getDamage());

			resolveCombat(pDefender, pPlot, kBattle);

			if (!bVisible)
			{
				bFinish = true;
			}
			else
			{
				kBattle.setDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_END, getDamage());
				kBattle.setDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_END, pDefender->getDamage());
				kBattle.setAdvanceSquare(canAdvance(pPlot, 1));

				if (isRanged() && pDefender->isRanged())
				{
					kBattle.setDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_END));
					kBattle.setDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_END));
				}
				else
				{
					kBattle.addDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_BEGIN));
					kBattle.addDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_BEGIN));
				}

				int iTurns = planBattle( kBattle);

				kBattle.setMissionTime(iTurns * gDLL->getSecsPerTurn());
				setCombatTimer(iTurns);

				GC.getGameINLINE().incrementTurnTimer(getCombatTimer());

				if (pPlot->isActiveVisible(false))
				{
					ExecuteMove(0.5f, true);
					// RevolutionDCM - stack attack - this next line is one source of stack attack CTD's
					gDLL->getEntityIFace()->AddMission(&kBattle);
				}
			}
		}
	}

	if (bFinish)
	{
		if (bVisible)
		{
			if (isCombatFocus() && gDLL->getInterfaceIFace()->isCombatFocus())
			{
				if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
				{
					gDLL->getInterfaceIFace()->releaseLockedCamera();
				}
			}
		}

		//end the combat mission if this code executes first
		gDLL->getEntityIFace()->RemoveUnitFromBattle(this);
		gDLL->getEntityIFace()->RemoveUnitFromBattle(pDefender);
		setAttackPlot(NULL, false);
		setCombatUnit(NULL);
		pDefender->setCombatUnit(NULL);
		NotifyEntity(MISSION_DAMAGE);
		pDefender->NotifyEntity(MISSION_DAMAGE);

		if (isDead())
		{
			if (isBarbarian())
			{
				GET_PLAYER(pDefender->getOwnerINLINE()).changeWinsVsBarbs(1);
			}

			if (!m_pUnitInfo->isHiddenNationality() && !pDefender->getUnitInfo().isHiddenNationality())
			{
				GET_TEAM(getTeam()).changeWarWeariness(pDefender->getTeam(), *pPlot, GC.getDefineINT("WW_UNIT_KILLED_ATTACKING"));
				GET_TEAM(pDefender->getTeam()).changeWarWeariness(getTeam(), *pPlot, GC.getDefineINT("WW_KILLED_UNIT_DEFENDING"));
				GET_TEAM(pDefender->getTeam()).AI_changeWarSuccess(getTeam(), GC.getDefineINT("WAR_SUCCESS_DEFENDING"));
			}

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_DIED_ATTACKING", getNameKey(), pDefender->getNameKey());
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/
			// ------ BEGIN InfluenceDrivenWar -------------------------------
			float fInfluenceRatio = 0.0;
			if (GC.getDefineINT("IDW_ENABLED"))
			{
				fInfluenceRatio = pDefender->doVictoryInfluence(this, false, false);
				/*** Dexy - Fixed Borders START ****/
				if (fInfluenceRatio > 0.0f)
				/*** Dexy - Fixed Borders  END  ****/
				{
				CvWString szTempBuffer;
				szTempBuffer.Format(L" Influence: -%.1f%%", fInfluenceRatio);
				szBuffer += szTempBuffer;
			}
			}
			// ------ END InfluenceDrivenWar ---------------------------------
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_KILLED_ENEMY_UNIT", pDefender->getNameKey(), getNameKey(), getVisualCivAdjective(pDefender->getTeam()));
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/
			// ------ BEGIN InfluenceDrivenWar -------------------------------
			if (GC.getDefineINT("IDW_ENABLED"))
			{
				/*** Dexy - Fixed Borders START ****/
				if (fInfluenceRatio > 0.0f)
				/*** Dexy - Fixed Borders  END  ****/
				{
				CvWString szTempBuffer;
				szTempBuffer.Format(L" Influence: +%.1f%%", fInfluenceRatio);
				szBuffer += szTempBuffer;
			}
			}
			// ------ END InfluenceDrivenWar ---------------------------------
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

			// report event to Python, along with some other key state
			CvEventReporter::getInstance().combatResult(pDefender, this);
		}
		else if (pDefender->isDead())
		{
			if (pDefender->isBarbarian())
			{
				GET_PLAYER(getOwnerINLINE()).changeWinsVsBarbs(1);
			}

			if (!m_pUnitInfo->isHiddenNationality() && !pDefender->getUnitInfo().isHiddenNationality())
			{
				GET_TEAM(pDefender->getTeam()).changeWarWeariness(getTeam(), *pPlot, GC.getDefineINT("WW_UNIT_KILLED_DEFENDING"));
				GET_TEAM(getTeam()).changeWarWeariness(pDefender->getTeam(), *pPlot, GC.getDefineINT("WW_KILLED_UNIT_ATTACKING"));
				GET_TEAM(getTeam()).AI_changeWarSuccess(pDefender->getTeam(), GC.getDefineINT("WAR_SUCCESS_ATTACKING"));
			}

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_DESTROYED_ENEMY", getNameKey(), pDefender->getNameKey());
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/
			// ------ BEGIN InfluenceDrivenWar -------------------------------
			float fInfluenceRatio = 0.0;			
			if (GC.getDefineINT("IDW_ENABLED"))
			{
				fInfluenceRatio = doVictoryInfluence(pDefender, true, false);
				/*** Dexy - Fixed Borders START ****/
				if (fInfluenceRatio > 0.0f)
				/*** Dexy - Fixed Borders  END  ****/
				{
				CvWString szTempBuffer;
				szTempBuffer.Format(L" Influence: +%.1f%%", fInfluenceRatio);
				szBuffer += szTempBuffer;
			}
			}
			// ------ END InfluenceDrivenWar ---------------------------------
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			if (getVisualOwner(pDefender->getTeam()) != getOwnerINLINE())
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED_UNKNOWN", pDefender->getNameKey(), getNameKey());
			}
			else
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED", pDefender->getNameKey(), getNameKey(), getVisualCivAdjective(pDefender->getTeam()));
			}
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/
			// ------ BEGIN InfluenceDrivenWar -------------------------------
			if (GC.getDefineINT("IDW_ENABLED"))
			{
				/*** Dexy - Fixed Borders START ****/
				if (fInfluenceRatio > 0.0f)
				/*** Dexy - Fixed Borders  END  ****/
				{
					CvWString szTempBuffer;
					szTempBuffer.Format(L" Influence: -%.1f%%", fInfluenceRatio);
					szBuffer += szTempBuffer;
				}
			}
			// ------ END InfluenceDrivenWar ---------------------------------
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer,GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

			// report event to Python, along with some other key state
			CvEventReporter::getInstance().combatResult(this, pDefender);

			bool bAdvance = false;

			if (isSuicide())
			{
				kill(true);

				pDefender->kill(false);
				pDefender = NULL;
			}
			else
			{
				bAdvance = canAdvance(pPlot, ((pDefender->canDefend()) ? 1 : 0));

				if (bAdvance)
				{
					if (!isNoCapture())
					{
						pDefender->setCapturingPlayer(getOwnerINLINE());
					}
				}


				pDefender->kill(false);
				pDefender = NULL;

				if (!bAdvance)
				{
					changeMoves(std::max(GC.getMOVE_DENOMINATOR(), pPlot->movementCost(this, plot())));
					checkRemoveSelectionAfterAttack();

					if (!canMove() || !isBlitz())
					{
						if (IsSelected())
						{
							if (gDLL->getInterfaceIFace()->getLengthSelectionList() > 1)
							{
								gDLL->getInterfaceIFace()->removeFromSelectionList(this);
							}
						}
					}
				}
			}

			if (pPlot->getNumVisibleEnemyDefenders(this) == 0)
			{
				getGroup()->groupMove(pPlot, true, ((bAdvance) ? this : NULL));
			}

			// This is is put before the plot advancement, the unit will always try to walk back
			// to the square that they came from, before advancing.
			getGroup()->clearMissionQueue();
		}
		else
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_WITHDRAW", getNameKey(), pDefender->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_OUR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_UNIT_WITHDRAW", getNameKey(), pDefender->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_THEIR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

			changeMoves(std::max(GC.getMOVE_DENOMINATOR(), pPlot->movementCost(this, plot())));
			checkRemoveSelectionAfterAttack();

			getGroup()->clearMissionQueue();
		}
	}
}
// Dale - SA: Stack Attack END

// Dale - SA: Opp Fire START
void CvUnit::doOpportunityFire()
{
	int iI, iUnitDamage;
	CvPlot* pLoopPlot;
	CvPlot* pAttackPlot = NULL;
	CvUnit* pDefender = NULL;
	CvWString szBuffer;
	if (!GC.getDCM_OPP_FIRE())
	{
		return;
	}
	if (bombardRate() <= 0)
	{
		return;
	}
	if (getFortifyTurns() > 0)
	{
		for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			pLoopPlot = plotDirection(plot()->getX_INLINE(), plot()->getY_INLINE(), ((DirectionTypes)iI));
			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->getNumUnits() > 0)
				{
					pDefender = pLoopPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);
					if (pDefender != NULL)
					{
/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
						setBattlePlot(pLoopPlot, pDefender);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
						iUnitDamage = (GC.getGameINLINE().getSorenRandNum(bombardRate(), "Bombard damage") * 5);
						pDefender->changeDamage(iUnitDamage, getOwner());
						szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_OPP_FIRE", getNameKey(), pDefender->getNameKey());
						gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_OUR_WITHDRAWL", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), plot()->getX_INLINE(), plot()->getY_INLINE(), true, true);
						szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_OPP_FIRE", getNameKey(), pDefender->getNameKey());
						gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_THEIR_WITHDRAWL", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), plot()->getX_INLINE(), plot()->getY_INLINE(), true, true);
						if (pLoopPlot->isActiveVisible(false))
						{
							// Bombard entity mission
							CvMissionDefinition kDefiniton;
							kDefiniton.setMissionTime(GC.getMissionInfo(MISSION_BOMBARD).getTime() * gDLL->getSecsPerTurn());
							kDefiniton.setMissionType(MISSION_BOMBARD);
							kDefiniton.setPlot(pLoopPlot);
							kDefiniton.setUnit(BATTLE_UNIT_ATTACKER, this);
							kDefiniton.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
							gDLL->getEntityIFace()->AddMission(&kDefiniton);
						}
					}
				}
			}
		}
	}
}
// Dale - SA: Opp Fire END

// Dale - SA: Active Defense START
void CvUnit::doActiveDefense()
{
	int iDamage, iUnitDamage, iSearchRange, iDX, iDY;
	CvPlot* pLoopPlot;
	CvPlot* pAttackPlot = NULL;
	CvUnit* pDefender = NULL;
	CvWString szBuffer;
	if (!GC.getDCM_ACTIVE_DEFENSE())
	{
		return;
	}
	if (getGroup()->getActivityType() != ACTIVITY_INTERCEPT)
	{
		return;
	}
	iSearchRange = 2;
	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->getNumUnits() > 0)
				{
					pDefender = airStrikeTarget(pLoopPlot);
					if (pDefender != NULL)
					{
/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
						setBattlePlot(pLoopPlot, pDefender);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
						iDamage = airCombatDamage(pDefender);
						iUnitDamage = std::max(pDefender->getDamage(), std::min((pDefender->getDamage() + iDamage), airCombatLimit()));
						szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_AIR", pDefender->getNameKey(), getNameKey(), -(((iUnitDamage - pDefender->getDamage()) * 100) / pDefender->maxHitPoints()));
						gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_AIR_ATTACK", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), true, true);
						szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR", getNameKey(), pDefender->getNameKey(), -(((iUnitDamage - pDefender->getDamage()) * 100) / pDefender->maxHitPoints()));
						gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_AIR_ATTACKED", MESSAGE_TYPE_INFO, pDefender->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
						collateralCombat(pLoopPlot, pDefender);
						pDefender->setDamage(iUnitDamage, getOwnerINLINE());
						if (pLoopPlot->isActiveVisible(false))
						{
							CvAirMissionDefinition kAirMission;
							kAirMission.setMissionType(MISSION_AIRSTRIKE);
							kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
							kAirMission.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
							kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
							kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
							kAirMission.setPlot(pLoopPlot);
							setCombatTimer(GC.getMissionInfo(MISSION_AIRSTRIKE).getTime());
							GC.getGameINLINE().incrementTurnTimer(getCombatTimer());
							kAirMission.setMissionTime(getCombatTimer() * gDLL->getSecsPerTurn());
							gDLL->getEntityIFace()->AddMission(&kAirMission);
						}
					}
				}
			}
		}
	}
}
// Dale - SA: Active Defense END

// Dale - ARB: Archer Bombard START
bool CvUnit::canArcherBombard(const CvPlot* pPlot) const
{
	if(!GC.getDCM_ARCHER_BOMBARD())
	{
		return false;
	}
	if (!(GC.getUnitInfo(getUnitType()).getUnitCombatType() == (UnitCombatTypes)1))
	{
		return false;
	}
	if (isMadeAttack())
	{
		return false;
	}
	if (isCargo())
	{
		return false;
	}
	return true;
}

bool CvUnit::canArcherBombardAt(const CvPlot* pPlot, int iX, int iY) const
{
	CvPlot* pTargetPlot;
	if (!canArcherBombard(pPlot))
	{
		return false;
	}

	if(iX < 0 || iY < 0)
	{
		return false;
	}
	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) > 1)
	{
		return false;
	}
	if(pTargetPlot->getNumVisibleEnemyDefenders(this) == 0)
	{
		return false;
	}
	if (pTargetPlot->isOwned())
	{
		if(pTargetPlot->getTeam() != getTeam())
		{
			if (!atWar(pTargetPlot->getTeam(), getTeam()))
			{
				return false;
			}
		}
	}
	return true;
}

bool CvUnit::archerBombard(int iX, int iY, bool sAttack)
{
	CvPlot* pPlot;
	CvWString szBuffer;
	CvUnit* pLoopUnit = NULL;
	int iDefenderNum, iUnitDamage;
	bool bTarget = false;
	if (!canArcherBombardAt(plot(), iX, iY))
	{
		return false;
	}
	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	iDefenderNum = pPlot->getNumVisibleEnemyDefenders(this);
	if(iDefenderNum == 0)
	{
		return false;
	}

	// RevolutionDCM - getBestDefender() now used in all cases
	pLoopUnit = pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);

	if (GC.getGameINLINE().getSorenRandNum(100, "Bombard Accuracy") <= getDCMBombAccuracy())
	{
		bTarget = true;
	} else {
		szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_BOMB_MISSED", getNameKey());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
		szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_BOMB_MISSED", getNameKey());
		// RevolutionDCM - Archer Bombard fix - text display
		gDLL->getInterfaceIFace()->addMessage(pLoopUnit->getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE());
	}
	if (bTarget)
	{
		//RevolutionDCM - Archer Bombard fix - report killed units
		iUnitDamage = std::max(1, ((GC.getDefineINT("COMBAT_DAMAGE") * (currFirepower(NULL, NULL) + ((currFirepower(NULL, NULL) + pLoopUnit->currFirepower(NULL, NULL) + 1) / 2))) / (pLoopUnit->currFirepower(pPlot, this) + ((currFirepower(NULL, NULL) + pLoopUnit->currFirepower(NULL, NULL) + 1) / 2))));
		pLoopUnit->changeDamage(iUnitDamage, getOwner());
		if (pLoopUnit->isDead())
		{
			if (pLoopUnit->isBarbarian())
			{
				GET_PLAYER(getOwnerINLINE()).changeWinsVsBarbs(1);
			}
			if (!m_pUnitInfo->isHiddenNationality() && !pLoopUnit->getUnitInfo().isHiddenNationality())
			{
				GET_TEAM(pLoopUnit->getTeam()).changeWarWeariness(getTeam(), *pPlot, GC.getDefineINT("WW_UNIT_KILLED_DEFENDING"));
				GET_TEAM(getTeam()).changeWarWeariness(pLoopUnit->getTeam(), *pPlot, GC.getDefineINT("WW_KILLED_UNIT_ATTACKING"));
				GET_TEAM(getTeam()).AI_changeWarSuccess(pLoopUnit->getTeam(), GC.getDefineINT("WAR_SUCCESS_ATTACKING"));
			}
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_DESTROYED_ENEMY", getNameKey(), pLoopUnit->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			if (getVisualOwner(pLoopUnit->getTeam()) != getOwnerINLINE())
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED_UNKNOWN", pLoopUnit->getNameKey(), getNameKey());
			}
			else
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED", pLoopUnit->getNameKey(), getNameKey(), getVisualCivAdjective(pLoopUnit->getTeam()));
			}
			gDLL->getInterfaceIFace()->addMessage(pLoopUnit->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer,GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			// report event to Python, along with some other key state
			CvEventReporter::getInstance().combatResult(this, pLoopUnit);
		}
		else
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR", getNameKey(), pLoopUnit->getNameKey(), -(((iUnitDamage - pLoopUnit->getDamage()) * 100) / pLoopUnit->maxHitPoints()));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_AIR_ATTACKED", MESSAGE_TYPE_INFO, pLoopUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_AIR", pLoopUnit->getNameKey(), getNameKey(), -(((iUnitDamage - pLoopUnit->getDamage()) * 100) / pLoopUnit->maxHitPoints()));
			gDLL->getInterfaceIFace()->addMessage(pLoopUnit->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_AIR_ATTACK", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
		}
		// RevolutionDCM - end
/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
		setBattlePlot(pPlot, pLoopUnit);
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/
	}

	if (!sAttack)
	{
		setMadeAttack(true);
		changeMoves(GC.getMOVE_DENOMINATOR());
	}
	if (pPlot->isActiveVisible(false))
	{
		// Bombard entity mission
		CvMissionDefinition kDefiniton;
		kDefiniton.setMissionTime(GC.getMissionInfo(MISSION_ABOMBARD).getTime() * gDLL->getSecsPerTurn());
		kDefiniton.setMissionType(MISSION_BOMBARD);
		kDefiniton.setPlot(pPlot);
		kDefiniton.setUnit(BATTLE_UNIT_ATTACKER, this);
		kDefiniton.setUnit(BATTLE_UNIT_DEFENDER, pLoopUnit);
		gDLL->getEntityIFace()->AddMission(&kDefiniton);
	}
	return true;
}
// Dale - ARB: Archer Bombard END

// Dale - FE: Fighters START
bool CvUnit::canFEngage(const CvPlot* pPlot) const
{
	if(!GC.getDCM_FIGHTER_ENGAGE())
	{
		return false;
	}
	if (!GC.getUnitInfo(getUnitType()).getDCMFighterEngage())
	{
		return false;
	}
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}
	if (isMadeAttack())
	{
		return false;
	}
//	if (isCargo())
//	{
//		return false;
//	}
	return true;
}

bool CvUnit::canFEngageAt(const CvPlot* pPlot, int iX, int iY) const
{
	CvPlot* pTargetPlot;
	int iI, iLoop;
	CvUnit* pLoopUnit;
	if (!canFEngage(pPlot))
	{
		return false;
	}

	if(iX < 0 || iY < 0)
	{
		return false;
	}
	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) > airRange())
	{
		return false;
	}
	if (pTargetPlot->isOwned())
	{
		if(pTargetPlot->getTeam() != getTeam())
		{
			if (!atWar(pTargetPlot->getTeam(), getTeam()))
			{
				return false;
			}
		}
	}
	for (iI = 0; iI < MAX_PLAYERS; ++iI)
	{
		if (atWar(GET_PLAYER((PlayerTypes)iI).getTeam(), getTeam()))
		{
			for(pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
			{
				if (pLoopUnit->plot() == pTargetPlot)
				{
					if (pLoopUnit->getDomainType() == DOMAIN_AIR)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool CvUnit::fighterEngage(int iX, int iY)
{
	CvPlot* pPlot;
	CvWString szBuffer;
	CvUnit* pDefender = NULL;
	CvUnit* pLoopUnit;
	CLLNode<IDInfo>* pUnitNode;
	int iCount;
	bool bTarget = false;
	if (!canFEngageAt(plot(), iX, iY))
	{
		return false;
	}
	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	if (interceptTest(pPlot))
	{
		return true;
	}
	iCount = 0;
	pDefender = NULL;
	pUnitNode = pPlot->headUnitNode();
	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);
		if (pLoopUnit->getDomainType() == DOMAIN_AIR)
		{
			iCount++;
		}
	}
	iCount = (GC.getGameINLINE().getSorenRandNum(iCount, "Choose plane") + 1);
	pUnitNode = pPlot->headUnitNode();
	while (iCount > 0)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);
		if (pLoopUnit->getDomainType() == DOMAIN_AIR)
		{
			iCount--;
			pDefender = pLoopUnit;
		}
	}
	if (pDefender != NULL)
	{
		CvAirMissionDefinition kAirMission;
		kAirMission.setMissionType(MISSION_AIRSTRIKE);
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
		resolveAirCombat(pDefender, pPlot, kAirMission);
		if (pPlot->isActiveVisible(false))
		{
			kAirMission.setPlot(pPlot);
			kAirMission.setMissionTime(GC.getMissionInfo(MISSION_AIRSTRIKE).getTime() * gDLL->getSecsPerTurn());
			setCombatTimer(GC.getMissionInfo(MISSION_AIRSTRIKE).getTime());
			GC.getGameINLINE().incrementTurnTimer(getCombatTimer());
			gDLL->getEntityIFace()->AddMission(&kAirMission);
		}
		if (isDead())
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_SHOT_DOWN_ENEMY", pDefender->getNameKey(), getNameKey(), getVisualCivAdjective(pDefender->getTeam()));
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_SHOT_DOWN", getNameKey(), pDefender->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, pDefender->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
		else if (kAirMission.getDamage(BATTLE_UNIT_ATTACKER) > 0)
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_HURT_ENEMY_AIR", pDefender->getNameKey(), getNameKey(), -(kAirMission.getDamage(BATTLE_UNIT_ATTACKER)), getVisualCivAdjective(pDefender->getTeam()));
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIR_UNIT_HURT", getNameKey(), pDefender->getNameKey(), -(kAirMission.getDamage(BATTLE_UNIT_ATTACKER)));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, pDefender->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
		if (pDefender->isDead())
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_SHOT_DOWN_ENEMY", getNameKey(), pDefender->getNameKey(), pDefender->getVisualCivAdjective(getTeam()));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, pDefender->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_SHOT_DOWN", pDefender->getNameKey(), getNameKey());
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
		else if (kAirMission.getDamage(BATTLE_UNIT_DEFENDER) > 0)
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_DAMAGED_ENEMY_AIR", getNameKey(), pDefender->getNameKey(), -(kAirMission.getDamage(BATTLE_UNIT_DEFENDER)), pDefender->getVisualCivAdjective(getTeam()));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, pDefender->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_AIR_UNIT_DAMAGED", pDefender->getNameKey(), getNameKey(), -(kAirMission.getDamage(BATTLE_UNIT_DEFENDER)));
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
		if (0 == kAirMission.getDamage(BATTLE_UNIT_ATTACKER) + kAirMission.getDamage(BATTLE_UNIT_DEFENDER))
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ABORTED_ENEMY_AIR", pDefender->getNameKey(), getNameKey(), getVisualCivAdjective(getTeam()));
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, pDefender->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_AIR_UNIT_ABORTED", getNameKey(), pDefender->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
	}
	setMadeAttack(true);
	changeMoves(GC.getMOVE_DENOMINATOR());
	return true;
}
// Dale - FE: Fighters END

/************************************************************************************************/
/* DCM                                     END                                                  */
/************************************************************************************************/


/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              Start                                                 */
/************************************************************************************************/
// ------ BEGIN InfluenceDrivenWar -------------------------------

// unit influences combat area after victory 
// returns influence % in defended plot
float CvUnit::doVictoryInfluence(CvUnit* pLoserUnit, bool bAttacking, bool bWithdrawal)
{
	/*** Dexy - Fixed Borders START ****/
	if (GET_PLAYER(pLoserUnit->getOwnerINLINE()).hasFixedBorders())
	{
		return 0.0f;
	}
	/*** Dexy - Fixed Borders  END  ****/
/************************************************************************************************/
/* Afforess	                  Start		 06/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (plot()->getWorkingCity() != NULL)
	{
		if (plot()->getWorkingCity()->isProtectedCulture())
		{
			return 0.0f;
		}
	}
	if (pLoserUnit->plot()->getWorkingCity() != NULL)
	{
		if (pLoserUnit->plot()->getWorkingCity()->isProtectedCulture())
		{
			return 0.0f;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/	
	if( isAnimal() || pLoserUnit->isAnimal() )
	{
		return 0.0f;
	}
	if( isAlwaysHostile(plot()) || pLoserUnit->isAlwaysHostile(pLoserUnit->plot()) )
	{
		return 0.0f;
	}
	if (GC.getDefineINT("IDW_NO_BARBARIAN_INFLUENCE"))
	{
		if (isBarbarian() || pLoserUnit->isBarbarian())
		{
			return 0.0f;
		}
	}
	if (GC.getDefineINT("IDW_NO_NAVAL_INFLUENCE"))
	{
		if (DOMAIN_SEA == getDomainType())
		{
			return 0.0f;
		}
	}

	CvPlot* pWinnerPlot = plot();
	CvPlot* pLoserPlot = pLoserUnit->plot();
	CvPlot* pDefenderPlot = NULL;
	if (!bAttacking)
	{
		pDefenderPlot = pWinnerPlot;
	}
	else
	{
		pDefenderPlot = pLoserPlot;
	}
	int iWinnerCultureBefore = pDefenderPlot->getCulture(getOwnerINLINE()); //used later for influence %

	float fWinnerPlotMultiplier = 1.0f; // by default: same influence in WinnerPlot and LoserPlot
	if (GC.getDefineFLOAT("IDW_WINNER_PLOT_MULTIPLIER"))
		fWinnerPlotMultiplier = GC.getDefineFLOAT("IDW_WINNER_PLOT_MULTIPLIER");

	float fLoserPlotMultiplier = 1.0f; // by default: same influence in WinnerPlot and LoserPlot
	if (GC.getDefineFLOAT("IDW_LOSER_PLOT_MULTIPLIER"))
		fLoserPlotMultiplier = GC.getDefineFLOAT("IDW_LOSER_PLOT_MULTIPLIER");

	float bWithdrawalMultiplier = 0.5f;
	if (bWithdrawal)
	{
		fWinnerPlotMultiplier *= bWithdrawalMultiplier;
		fLoserPlotMultiplier *= bWithdrawalMultiplier;
	}

	if (pLoserPlot->isEnemyCity(*this)) // city combat 
	{         	
		if (pLoserPlot->getNumVisibleEnemyDefenders(this) > 1)
		{
			// if there are still some city defenders ->
			// we use same influence rules as for field combat
			influencePlots(pLoserPlot, pLoserUnit->getOwnerINLINE(), fLoserPlotMultiplier);
			influencePlots(pWinnerPlot, pLoserUnit->getOwnerINLINE(), fWinnerPlotMultiplier);			
		}		
		else // last defender is dead
		{			
			float fNoCityDefenderMultiplier = 2.5; // default: 250%
			if (GC.getDefineFLOAT("IDW_NO_CITY_DEFENDER_MULTIPLIER"))
				fNoCityDefenderMultiplier = GC.getDefineFLOAT("IDW_NO_CITY_DEFENDER_MULTIPLIER");

			// last city defender is dead -> influence is increased
			influencePlots(pLoserPlot, pLoserUnit->getOwnerINLINE(), fLoserPlotMultiplier * fNoCityDefenderMultiplier);
			influencePlots(pWinnerPlot, pLoserUnit->getOwnerINLINE(), fWinnerPlotMultiplier * fNoCityDefenderMultiplier);

			if (GC.getDefineINT("IDW_EMERGENCY_DRAFT_ENABLED"))
			{
				int iDefenderCulture = pLoserPlot->getCulture(pLoserUnit->getOwnerINLINE());
				int iAttackerCulture = pLoserPlot->getCulture(getOwnerINLINE());
				
				if (iDefenderCulture >= iAttackerCulture)
				{
					// if defender culture in city's central tile is still higher then atacker culture 
					// -> city is not captured yet but emergency militia is drafted
					pLoserPlot->getPlotCity()->emergencyConscript();
					
					// calculate city resistence % (to be displayed in game log)
					float fResistence = ((iDefenderCulture-iAttackerCulture)*100.0f)/(2*pDefenderPlot->countTotalCulture());

					CvWString szBuffer;
					szBuffer.Format(L"City militia has emerged! Resistance: %.1f%%", fResistence);	
					gDLL->getInterfaceIFace()->addMessage(pLoserUnit->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_UNIT_BUILD_UNIT", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pLoserPlot->getX_INLINE(), pLoserPlot->getY_INLINE(), true, true);
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_UNIT_BUILD_UNIT", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pLoserPlot->getX_INLINE(), pLoserPlot->getY_INLINE());

				}
			}
		}
	}
	else // field combat
	{				
		if (!pLoserUnit->canDefend()) 
		{
			// no influence from worker capture
			return 0.0f;
		}

		if (pLoserPlot->getImprovementType() != NO_IMPROVEMENT 
			&& GC.getImprovementInfo(pLoserPlot->getImprovementType()).getDefenseModifier() > 0
			&& pLoserPlot->getNumVisibleEnemyDefenders(this) > 1)
		{		
			// fort captured
			float fFortCaptureMultiplier = 2.0f; // default: 200%
			if (GC.getDefineFLOAT("IDW_FORT_CAPTURE_MULTIPLIER"))
				fFortCaptureMultiplier = GC.getDefineFLOAT("IDW_FORT_CAPTURE_MULTIPLIER");

			// influence is increased
			influencePlots(pLoserPlot, pLoserUnit->getOwnerINLINE(), fLoserPlotMultiplier * fFortCaptureMultiplier);
			influencePlots(pWinnerPlot, pLoserUnit->getOwnerINLINE(), fWinnerPlotMultiplier * fFortCaptureMultiplier);

		}
		else
		{			
			influencePlots(pLoserPlot, pLoserUnit->getOwnerINLINE(), fLoserPlotMultiplier);
			influencePlots(pWinnerPlot, pLoserUnit->getOwnerINLINE(), fWinnerPlotMultiplier);
		}
	}

	// calculate influence % in defended plot (to be displayed in game log)
    
	int iWinnerCultureAfter = pDefenderPlot->getCulture(getOwnerINLINE());
	int iTotalCulture = pDefenderPlot->countTotalCulture();
	float fInfluenceRatio = 0.0f;
	if (iTotalCulture > 0)
	{
		fInfluenceRatio = ((iWinnerCultureAfter-iWinnerCultureBefore)*100.0f)/iTotalCulture;	
	}
    return fInfluenceRatio;
}

// unit influences given plot and surounding area i.e. transfers culture from target civ to unit's owner
void CvUnit::influencePlots(CvPlot* pCentralPlot, PlayerTypes eTargetPlayer, float fLocationMultiplier)
{
	float fBaseCombatInfluence = 4.0f;    
	if (GC.getDefineFLOAT("IDW_BASE_COMBAT_INFLUENCE"))
        fBaseCombatInfluence = GC.getDefineFLOAT("IDW_BASE_COMBAT_INFLUENCE");

	// calculate base multiplier used for all plots
	float fGameSpeedMultiplier = (float) GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
	fGameSpeedMultiplier /= 100;
	fGameSpeedMultiplier *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getConstructPercent();
	fGameSpeedMultiplier /= 100;
	fGameSpeedMultiplier = sqrt(fGameSpeedMultiplier);

	float fExperienceFactor = 0.01f;  // default: each point of experience increases influence by 1%
	if (GC.getDefineFLOAT("IDW_EXPERIENCE_FACTOR"))
        fExperienceFactor = GC.getDefineFLOAT("IDW_EXPERIENCE_FACTOR");
	float fExperienceMultiplier = 1.0f + (getExperience() * 0.01f);

	float fWarlordMultiplier = 1.0; 
	if (NO_UNIT != getLeaderUnitType()) // warlord is here
	{
		fWarlordMultiplier = 1.5; // default: +50% 
		if (GC.getDefineFLOAT("IDW_WARLORD_MULTIPLIER"))
			fWarlordMultiplier = GC.getDefineFLOAT("IDW_WARLORD_MULTIPLIER");
	}

	float fCityPlotMultiplier = 1.0f; 
	if (!(GC.getDefineINT("IDW_EMERGENCY_DRAFT_ENABLED")))
	{
		if (GC.getDefineFLOAT("IDW_CITY_TILE_MULTIPLIER"))
		{
			fCityPlotMultiplier = GC.getDefineFLOAT("IDW_CITY_TILE_MULTIPLIER");
		}
	}

	float fBaseMultiplier = fBaseCombatInfluence * fGameSpeedMultiplier * fLocationMultiplier * fExperienceMultiplier * fWarlordMultiplier;
	if (fBaseMultiplier <= 0.0f)
		return;

	// get influence radius
	int iInfluenceRadius = 2; // default: like 2square city workable radius
	if (GC.getDefineINT("IDW_INFLUENCE_RADIUS"))
		iInfluenceRadius = GC.getDefineINT("IDW_INFLUENCE_RADIUS");
	if (iInfluenceRadius < 0)
		return;

	float fPlotDistanceFactor = 0.2f; // default: influence decreases by 20% with plot distance
	if (GC.getDefineFLOAT("IDW_PLOT_DISTANCE_FACTOR"))
        fPlotDistanceFactor = GC.getDefineFLOAT("IDW_PLOT_DISTANCE_FACTOR");

//	CvWString szBuffer;
//	szBuffer.Format(L"Factors: %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.3f, %d", fBaseCombatInfluence, fLocationMultiplier, fGameSpeedMultiplier, fPlotDistanceFactor, fExperienceMultiplier, fWarlordMultiplier, fBaseMultiplier, iInfluenceRadius);	
//	gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_UNIT_BUILD_UNIT", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCentralPlot->getX_INLINE(), pCentralPlot->getY_INLINE());

	for (int iDX = -iInfluenceRadius; iDX <= iInfluenceRadius; iDX++)
	{
		for (int iDY = -iInfluenceRadius; iDY <= iInfluenceRadius; iDY++)
		{
			int iDistance = plotDistance(0, 0, iDX, iDY);

			if (iDistance <= iInfluenceRadius)
			{
				CvPlot* pLoopPlot = plotXY(pCentralPlot->getX_INLINE(), pCentralPlot->getY_INLINE(), iDX, iDY);

				if (pLoopPlot != NULL)
				{	
					// calculate distance multiplier for current plot
					float fDistanceMultiplier = 0.5f+0.5f*fPlotDistanceFactor-fPlotDistanceFactor*iDistance;
					if (fDistanceMultiplier <= 0.0f)
						continue;
					int iTargetCulture = pLoopPlot->getCulture(eTargetPlayer);
					if (iTargetCulture <= 0)
						continue;
					if ( pLoopPlot->isCity() )
					{
						fBaseMultiplier *= fCityPlotMultiplier;
					}
					if (fBaseMultiplier <= 0.0f)
						continue;
					int iCultureTransfer = int (fBaseMultiplier * fDistanceMultiplier * sqrt((float) iTargetCulture));
					if (iTargetCulture < iCultureTransfer)
					{
						// cannot transfer more culture than remaining target culure
						iCultureTransfer = iTargetCulture;
					}
					if (iCultureTransfer == 0 && iTargetCulture > 0)
					{
						// always at least 1 point of culture must be transfered
						// othervise we may have the problems with capturing of very low culture cities. 
						iCultureTransfer = 1; 
					}
/************************************************************************************************/
/* Afforess	                  Start		 02/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
					/* plot that is not a city can't lose the last point of culture if its owner has fixed borders, otherwise it'd flip */
					if ((pLoopPlot->getPlotCity() == NULL) && GET_PLAYER(eTargetPlayer).hasFixedBorders())
					{
						if (iCultureTransfer == iTargetCulture)
						{
							iCultureTransfer = iTargetCulture - 1;
						}
					}			
					/* plots with forts can't lose the last point of culture, otherwise they could become unowned with some units remaining inside */
					if (pLoopPlot->isActsAsCity())
					{
						if (iCultureTransfer == iTargetCulture)
						{
							iCultureTransfer = iTargetCulture - 1;
						}
					}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


					if (iCultureTransfer > 0)
					{
						// target player's culture in plot is lowered
						pLoopPlot->changeCulture(eTargetPlayer, -iCultureTransfer, false);
						if( iTargetCulture > 0 && pLoopPlot->getCulture(eTargetPlayer) <= 0 )
						{
							// Don't allow complete loss of all culture
							pLoopPlot->setCulture(eTargetPlayer,1,false,false);
						}
						// owners's culture in plot is raised
						pLoopPlot->changeCulture(getOwnerINLINE(), iCultureTransfer, true);
					}
				}	
			}
		}
	}		
}


// unit influences current tile via pillaging 
// returns influence % in current plot
float CvUnit::doPillageInfluence()
{
	if (isBarbarian() && GC.getDefineINT("IDW_NO_BARBARIAN_INFLUENCE"))
	{
		return 0.0f;
	}
	if ((DOMAIN_SEA == getDomainType()) && GC.getDefineINT("IDW_NO_NAVAL_INFLUENCE"))
	{
		return 0.0f;
	}
	
	CvPlot* pPlot = plot();
	if (pPlot == NULL)
	{
		//should not happen
		return 0.0f;	
	}
/************************************************************************************************/
/* Afforess	                  Start		 06/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	if (pPlot->getWorkingCity() != NULL)
	{
		if (pPlot->getWorkingCity()->isProtectedCulture())
		{
			return 0.0f;
		}
	}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	int iOurCultureBefore = pPlot->getCulture(getOwnerINLINE()); //used later for influence %

	float fBasePillageInfluence = 2.0f;    
	if (GC.getDefineFLOAT("IDW_BASE_PILLAGE_INFLUENCE"))
        fBasePillageInfluence = GC.getDefineFLOAT("IDW_BASE_PILLAGE_INFLUENCE");

	float fGameSpeedMultiplier = (float) GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
	fGameSpeedMultiplier /= 100;
	fGameSpeedMultiplier *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getConstructPercent();
	fGameSpeedMultiplier /= 100;
	fGameSpeedMultiplier = sqrt(fGameSpeedMultiplier);

	PlayerTypes eTargetPlayer = pPlot->getOwner();
	int iTargetCulture = pPlot->getCulture(eTargetPlayer);
	if (iTargetCulture <= 0)
	{
		//should not happen
		return 0.0f;		
	}
	int iCultureTransfer = int (fBasePillageInfluence * fGameSpeedMultiplier * sqrt((float) iTargetCulture));
	if (iTargetCulture < iCultureTransfer)
	{
		// cannot transfer more culture than remaining target culure
		iCultureTransfer = iTargetCulture;
	}

	// target player's culture in plot is lowered
	pPlot->changeCulture(eTargetPlayer, -iCultureTransfer, false);
	// owners's culture in plot is raised
	pPlot->changeCulture(getOwnerINLINE(), iCultureTransfer, true);

	// calculate influence % in pillaged plot (to be displayed in game log)
    int iOurCultureAfter = pPlot->getCulture(getOwnerINLINE());
	float fInfluenceRatio = ((iOurCultureAfter-iOurCultureBefore)*100.0f)/pPlot->countTotalCulture();	
 
//	CvWString szBuffer;
//	szBuffer.Format(L"Factors: %.1f, %.1f, %d, Result: %.3f, ", fGameSpeedMultiplier, fBasePillageInfluence, iTargetCulture, fInfluenceRatio);	
//	gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_UNIT_BUILD_UNIT", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), plot()->getX_INLINE(), plot()->getY_INLINE());

	return fInfluenceRatio;
}

// ------ END InfluenceDrivenWar ---------------------------------
/************************************************************************************************/
/* INFLUENCE_DRIVEN_WAR                   04/16/09                                johnysmith    */
/*                                                                                              */
/* Original Author Moctezuma              End                                                   */
/************************************************************************************************/

/************************************************************************************************/
/* Afforess	                  Start		 02/14/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/

bool CvUnit::canPerformInquisition(const CvPlot* pPlot) const
{

	CvCity* pCity;

	pCity = pPlot->getPlotCity();

	if (!m_pUnitInfo->isInquisitor())
	{
		return false;
	}
	if (pCity == NULL)
	{
		return false;
	}
	if( GET_PLAYER(getOwnerINLINE()).getStateReligion() == NO_RELIGION )
	{
		return false;
	}
	if (!pCity->isHasReligion(GET_PLAYER(getOwnerINLINE()).getStateReligion()))
	{
		return false;
	}

	//Allow inquisitions in vassals
	if( (pCity->getTeam() != getTeam())  && !(GET_TEAM(pCity->getTeam()).isVassal(getTeam())) )
	{
		return false;
	}
	if (!pCity->isInquisitionConditions() || !GET_PLAYER(getOwnerINLINE()).isInquisitionConditions())
	{
		return false;
	}
	if(GET_PLAYER(getOwnerINLINE()).getStateReligion() != GET_PLAYER(pCity->getOwnerINLINE()).getStateReligion())
	{
		return false;
	}

	return true;
}


bool CvUnit::performInquisition()
{
	CvCity* pCity;
	int iI, iJ;
	int iReligionCount = 0;
	int iHolyCityVal = 0;
	int iCompensationGold = 0;
	int iCount;
	int iBestCount = 0;
	PlayerTypes eBestPlayer = NO_PLAYER;
	bool bRelocateHolyCity;
	int iLoop;
	CvCity* pLoopCity;
	CvWString szBuffer;

	if (!canPerformInquisition(plot()))
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
			if (kLoopPlayer.isAlive())
			{
				if( plot()->isVisible(kLoopPlayer.getTeam(), true)
				|| plot()->isRevealed(kLoopPlayer.getTeam(), true) )
				{
					gDLL->getInterfaceIFace()->playGeneralSound("AS3D_UN_CHRIST_MISSIONARY_ACTIVATE", plot()->getPoint());
				}
			}
		}
		
		for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
		{
			if ((ReligionTypes)iI != GET_PLAYER(getOwnerINLINE()).getStateReligion())
			{
				if (pCity->isHasReligion((ReligionTypes)iI))
				{
					iReligionCount++;
					if (pCity->isHolyCity((ReligionTypes)iI))
					{
						iHolyCityVal = 50;
					}
				}
			}
		}
		if (GC.getGameINLINE().getSorenRandNum(100, "Inquisition Persection Chance") < std::max(25, (95 - iHolyCityVal - (5 * iReligionCount))))
		{
			//Change memory if we are removing a religion that is another player's state religion
			for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
			{
				CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
				if (kLoopPlayer.isAlive())
				{
					if( plot()->isVisible(kLoopPlayer.getTeam(), false)
					|| plot()->isRevealed(kLoopPlayer.getTeam(), false) )
					{
						if (GET_TEAM(kLoopPlayer.getTeam()).isHasMet(GET_PLAYER(getOwnerINLINE()).getTeam()))
						{
							for (iJ = 0; iJ < GC.getNumReligionInfos(); iJ++)
							{
								if (GET_PLAYER(getOwnerINLINE()).getStateReligion() != (ReligionTypes)iJ)
								{
									//if the player has the holy city, or has the religion as a state religion.
									if (kLoopPlayer.hasHolyCity((ReligionTypes)iJ) || ((pCity->isHasReligion((ReligionTypes)iJ)) && (kLoopPlayer.getStateReligion() == (ReligionTypes)iJ)))
									{
										kLoopPlayer.AI_changeMemoryCount(getOwnerINLINE(), MEMORY_INQUISITION, 1);
										break;
									}
								}
							}
						}
					}
				}
			}
			//Remove temples, monasteries, etc...
			for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
			{
				if (pCity->getNumRealBuilding((BuildingTypes)iI) > 0)
				{
					CvBuildingInfo& kLoopBuilding = GC.getBuildingInfo((BuildingTypes)iI);
					for (iJ = 0; iJ < GC.getNumReligionInfos(); iJ++)
					{
						if (GET_PLAYER(getOwnerINLINE()).getStateReligion() != (ReligionTypes)iJ)
						{
							if (kLoopBuilding.getPrereqReligion() == (ReligionTypes)iJ)
							{
								pCity->setNumRealBuilding((BuildingTypes)iI, 0);
								iCompensationGold += kLoopBuilding.getProductionCost() * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent() / std::max(1, GC.getDefineINT("INQUISITION_BUILDING_GOLD_DIVISOR"));
							}
						}
					}
				}
			}
			//Remove the Religion & Holy Cities
			for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
			{
				if (GET_PLAYER(getOwnerINLINE()).getStateReligion() != (ReligionTypes)iI)
				{
					if (pCity->isHolyCity((ReligionTypes)iI))
					{
						//TODO: This value needs to be set from python
						if (GC.getDefineINT("OC_RESPAWN_HOLY_CITIES"))
						{
							GC.getGameINLINE().setHolyCity((ReligionTypes)iI, NULL , false);
							iCompensationGold += GC.getDefineINT("HOLYCITY_REMOVAL_GOLD");
							pCity->setHasReligion((ReligionTypes)iI, false, false, false);

							//Find the best place to replace the holy city
							for (iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
							{
								CvPlayerAI& kLoopPlayer = GET_PLAYER((PlayerTypes)iJ);
								
								if (kLoopPlayer.isAlive())
								{
									iCount = kLoopPlayer.getHasReligionCount((ReligionTypes)iI);
									if (iCount > iBestCount)
									{
										iBestCount = iCount;
										eBestPlayer = (PlayerTypes)iJ;
									}
								}
							}
							//Relocate the holy city
							bRelocateHolyCity = false;
							if (eBestPlayer != NO_PLAYER)
							{
								CvPlayerAI& kPlayer = GET_PLAYER(eBestPlayer);
								for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
								{
									if (pCity->isHasReligion((ReligionTypes)iI))
									{
										bRelocateHolyCity = true;
										break;
									}
								}
								if (bRelocateHolyCity)
								{
									GC.getGameINLINE().setHolyCity((ReligionTypes)iI, pLoopCity, true);
									//TODO: Create a text entry: "A Holy City Religion has been Respawned"
									szBuffer = gDLL->getText("TXT_KEY_MESSAGE_HOLY_CITY_RESPAWNED");
									gDLL->getInterfaceIFace()->addMessage(GC.getGameINLINE().getActivePlayer(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_DISCOVERBONUS", MESSAGE_TYPE_MAJOR_EVENT, "Art/Interface/Buttons/TerrainFeatures/Forest.dds", ColorTypes(8), getX_INLINE(), getY_INLINE(), false, false);
								}
							}
						}
					}
					else if (pCity->isHasReligion((ReligionTypes)iI))
					{
						pCity->setHasReligion((ReligionTypes)iI, false, false, false);
						iCompensationGold += GC.getDefineINT("RELIGION_REMOVAL_GOLD");
					}
				}
			}

			for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
			{
				CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
				if (kLoopPlayer.isAlive())
				{
					if( plot()->isVisible(kLoopPlayer.getTeam(), true)
					|| plot()->isRevealed(kLoopPlayer.getTeam(), true) )
					{
						szBuffer = gDLL->getText("TXT_KEY_MESSAGE_INQUISITION", pCity->getNameKey());
						gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PLAGUE", MESSAGE_TYPE_MAJOR_EVENT, m_pUnitInfo->getButton() , ColorTypes(8), getX_INLINE(), getY_INLINE(), true, true);
					}
				}
			}
			
			//Increase Temp Anger
			pCity->changeHurryAngerTimer(pCity->flatHurryAngerLength());
			if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_REVOLUTION))
			{
				//Avoid setting the Rev Index below 0...
				pCity->changeLocalRevIndex(-std::min(pCity->getRevolutionIndex(), iCompensationGold));
			}
			else
			{
				GET_PLAYER(getOwnerINLINE()).changeGold(iCompensationGold);
			}
		}
		//Inquisition Fails...
		else
		{
			for(iI = 0; iI < MAX_CIV_PLAYERS; iI++)
			{
				CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
				if (kLoopPlayer.isAlive())
				{
					if( plot()->isVisible(kLoopPlayer.getTeam(), true)
					|| plot()->isRevealed(kLoopPlayer.getTeam(), true) )
					{
						szBuffer = gDLL->getText("TXT_KEY_MESSAGE_INQUISITION_FAIL", pCity->getNameKey());
						gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_SABOTAGE", MESSAGE_TYPE_MAJOR_EVENT, m_pUnitInfo->getButton() , ColorTypes(8), getX_INLINE(), getY_INLINE(), true, true);
					}
				}
			}
			
			pCity->changeHurryAngerTimer(pCity->flatHurryAngerLength());
			if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_REVOLUTION))
			{
				pCity->changeLocalRevIndex(iCompensationGold / 2);
			}
		}

		if (plot()->isActiveVisible(false))
		{
			NotifyEntity(MISSION_INQUISITION);
		}	
	}

	kill(true);
	return true;
}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

/************************************************************************************************/
/* RevolutionDCM	                  Start		 05/31/10                        Afforess       */
/*                                                                                              */
/* Battle Effects                                                                               */
/************************************************************************************************/
void CvUnit::setBattlePlot(CvPlot* pPlot, const CvUnit* pDefender)
{
	if (pPlot->canHaveBattleEffect(this, pDefender))
	{
		if (pPlot->getBattleCountdown() < GC.getDefineINT("MAX_BATTLE_TURNS"))
		{
			pPlot->changeBattleCountdown(GC.getDefineINT("BATTLE_EFFECTS_MINIMUM_TURN_INCREMENTS"));
		}
	}
}
/************************************************************************************************/
/* RevolutionDCM	             Battle Effects END                                             */
/************************************************************************************************/


/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Lead From Behind                                                                             */
/************************************************************************************************/
// From Lead From Behind by UncutDragon

// Original isBetterDefenderThan call (without the extra parameter) - now just a pass-through
bool CvUnit::isBetterDefenderThan(const CvUnit* pDefender, const CvUnit* pAttacker) const
{
	return isBetterDefenderThan(pDefender, pAttacker, NULL);
}

// Modified version of best defender code (minus the initial boolean tests,
// which we still check in the original method)
bool CvUnit::LFBisBetterDefenderThan(const CvUnit* pDefender, const CvUnit* pAttacker, int* pBestDefenderRank) const
{
	// We adjust ranking based on ratio of our adjusted strength compared to twice that of attacker
	// Effect is if we're over twice as strong as attacker, we increase our ranking
	// (more likely to be picked as defender) - otherwise, we reduce our ranking (less likely)

	// Get our adjusted rankings based on combat odds
	int iOurRanking = LFBgetDefenderRank(pAttacker);
	int iTheirRanking = -1;
	if (pBestDefenderRank)
		iTheirRanking = (*pBestDefenderRank);
	if (iTheirRanking == -1)
		iTheirRanking = pDefender->LFBgetDefenderRank(pAttacker);

	// In case of equal value, fall back on unit cycle order
	if (iOurRanking == iTheirRanking)
	{
		if (isBeforeUnitCycle(this, pDefender))
			iTheirRanking--;
		else
			iTheirRanking++;
	}

	// Retain the basic rank (before value adjustment) for the best defender
	if (pBestDefenderRank)
		if (iOurRanking > iTheirRanking)
			(*pBestDefenderRank) = iOurRanking;

	return (iOurRanking > iTheirRanking);
}

// Get the (adjusted) odds of attacker winning to use in deciding best attacker
int CvUnit::LFBgetAttackerRank(const CvUnit* pDefender, int& iUnadjustedRank) const
{
	if (pDefender)
	{
		int iDefOdds = pDefender->LFBgetDefenderOdds(this);
		iUnadjustedRank = 1000 - iDefOdds;
		// If attacker has a chance to withdraw, factor that in as well
		if (withdrawalProbability() > 0)
			iUnadjustedRank += ((iDefOdds * withdrawalProbability()) / 100);
	} else {
		// No defender ... just use strength, but try to make it a number out of 1000
		iUnadjustedRank = currCombatStr(NULL, NULL) / 5;
	}
	int iRank = LFBgetValueAdjustedOdds(iUnadjustedRank);

	return iRank;
}

// Get the (adjusted) odds of defender winning to use in deciding best defender
int CvUnit::LFBgetDefenderRank(const CvUnit* pAttacker) const
{
	int iRank = LFBgetDefenderOdds(pAttacker);
	// Don't adjust odds for value if attacker is limited in their damage (i.e: no risk of death)
	if ((pAttacker != NULL) && (maxHitPoints() == pAttacker->combatLimit()))
		iRank = LFBgetValueAdjustedOdds(iRank);

	return iRank;
}

// Get the unadjusted odds of defender winning (used for both best defender and best attacker)
int CvUnit::LFBgetDefenderOdds(const CvUnit* pAttacker) const
{
	// Check if we have a valid attacker
	bool bUseAttacker = false;
	int iAttStrength = 0;
	if (pAttacker)
		iAttStrength = pAttacker->currCombatStr(NULL, NULL);
	if (iAttStrength > 0)
		bUseAttacker = true;

	int iDefense = 0;

	if (bUseAttacker && GC.getLFBUseCombatOdds())
	{
		// We start with straight combat odds
		iDefense = LFBgetDefenderCombatOdds(pAttacker);
	} else {
		// Lacking a real opponent (or if combat odds turned off) fall back on just using strength
		iDefense = currCombatStr(plot(), pAttacker);
		if (bUseAttacker)
		{
			// Similiar to the standard method, except I reduced the affect (cut it in half) handle attacker
			// and defender together (instead of applying one on top of the other) and substract the
			// attacker first strikes (instead of adding attacker first strikes when defender is immune)
			int iFirstStrikes = 0;

			if (!pAttacker->immuneToFirstStrikes())
				iFirstStrikes += (firstStrikes() * 2) + chanceFirstStrikes();
			if (!immuneToFirstStrikes())
				iFirstStrikes -= ((pAttacker->firstStrikes() * 2) + pAttacker->chanceFirstStrikes());

			if (iFirstStrikes != 0)
			{
				// With COMBAT_DAMAGE=20, this makes each first strike worth 8% (and each chance worth 4%)
				iDefense *= ((iFirstStrikes * GC.getCOMBAT_DAMAGE() / 5) + 100);
				iDefense /= 100;
			}

			// Make it a number out of 1000, taking attacker into consideration
			iDefense = (iDefense * 1000) / (iDefense + iAttStrength);			
		}
	}

	if (hasCargo())
	{
		// This part is taken directly from the standard method
		// Reduces value if a unit is carrying other units
		int iAssetValue = std::max(1, getUnitInfo().getAssetValue());
		int iCargoAssetValue = 0;
		std::vector<CvUnit*> aCargoUnits;
		getCargoUnits(aCargoUnits);
		for (uint i = 0; i < aCargoUnits.size(); ++i)
		{
			iCargoAssetValue += aCargoUnits[i]->getUnitInfo().getAssetValue();
		}
		iDefense = iDefense * iAssetValue / std::max(1, iAssetValue + iCargoAssetValue);
	}

	return iDefense;
}

// Take the unadjusted odds and adjust them based on unit value
int CvUnit::LFBgetValueAdjustedOdds(int iOdds) const
{
	// Adjust odds based on value
	int iValue = LFBgetRelativeValueRating();
	long iAdjustment = -250;
	if (GC.getLFBUseSlidingScale())
		iAdjustment = (iOdds - 990);
	// Value Adjustment = (odds-990)*(value*num/denom)^2
	long iValueAdj = (long)(iValue * GC.getLFBAdjustNumerator());
	iValueAdj *= iValueAdj;
	iValueAdj *= iAdjustment;
	iValueAdj /= (long)(GC.getLFBAdjustDenominator() * GC.getLFBAdjustDenominator());
	int iRank = iOdds + iValueAdj + 10000;
	// Note that the +10000 is just to try keeping it > 0 - doesn't really matter, other than that -1
	// would be interpreted later as not computed yet, which would cause us to compute it again each time

	return iRank;
}

// Method to evaluate the value of a unit relative to another
int CvUnit::LFBgetRelativeValueRating() const
{
	int iValueRating = 0;

	// Check if led by a Great General
	if (GC.getLFBBasedOnGeneral() > 0)
		if (NO_UNIT != getLeaderUnitType())
			iValueRating += GC.getLFBBasedOnGeneral();

	// Assign experience value in tiers
	if (GC.getLFBBasedOnExperience() > 0)
	{
		int iTier = 10;
		while (getExperience() >= iTier)
		{
			iValueRating += GC.getLFBBasedOnExperience();
			iTier *= 2;
		}
	}

	// Check if unit is limited in how many can exist
	if (GC.getLFBBasedOnLimited() > 0)
		if (isLimitedUnitClass(getUnitClassType()))
			iValueRating += GC.getLFBBasedOnLimited();

	// Check if unit has ability to heal
	if (GC.getLFBBasedOnHealer() > 0)
		if (getSameTileHeal() > 0)
			iValueRating += GC.getLFBBasedOnHealer();

	return iValueRating;
}

int CvUnit::LFBgetDefenderCombatOdds(const CvUnit* pAttacker) const
{
	int iAttackerStrength;
	int iAttackerFirepower;
	int iDefenderStrength;
	int iDefenderFirepower;
	int iDefenderOdds;
	int iStrengthFactor;
	int iDamageToAttacker;
	int iDamageToDefender;
	int iNeededRoundsAttacker;
	int iNeededRoundsDefender;
	int iAttackerLowFS;
	int iAttackerHighFS;
	int iDefenderLowFS;
	int iDefenderHighFS;
	int iDefenderHitLimit;

	iAttackerStrength = pAttacker->currCombatStr(NULL, NULL);
	iAttackerFirepower = pAttacker->currFirepower(NULL, NULL);

	iDefenderStrength = currCombatStr(plot(), pAttacker);
	iDefenderFirepower = currFirepower(plot(), pAttacker);

	FAssert((iAttackerStrength + iDefenderStrength) > 0);
	FAssert((iAttackerFirepower + iDefenderFirepower) > 0);

	iDefenderOdds = ((GC.getCOMBAT_DIE_SIDES() * iDefenderStrength) / (iAttackerStrength + iDefenderStrength));
	iStrengthFactor = ((iAttackerFirepower + iDefenderFirepower + 1) / 2);

	// calculate damage done in one round
	//////

	iDamageToAttacker = std::max(1,((GC.getCOMBAT_DAMAGE() * (iDefenderFirepower + iStrengthFactor)) / (iAttackerFirepower + iStrengthFactor)));
	iDamageToDefender = std::max(1,((GC.getCOMBAT_DAMAGE() * (iAttackerFirepower + iStrengthFactor)) / (iDefenderFirepower + iStrengthFactor)));

	// calculate needed rounds.
	// Needed rounds = round_up(health/damage)
	//////

	iDefenderHitLimit = maxHitPoints() - pAttacker->combatLimit();

	iNeededRoundsAttacker = (std::max(0, currHitPoints() - iDefenderHitLimit) + iDamageToDefender - 1 ) / iDamageToDefender;
	iNeededRoundsDefender = (pAttacker->currHitPoints() + iDamageToAttacker - 1 ) / iDamageToAttacker;

	// calculate possible first strikes distribution.
	// We can't use the getCombatFirstStrikes() function (only one result,
	// no distribution), so we need to mimic it.
	//////

	iAttackerLowFS = (immuneToFirstStrikes()) ? 0 : pAttacker->firstStrikes();
	iAttackerHighFS = (immuneToFirstStrikes()) ? 0 : (pAttacker->firstStrikes() + pAttacker->chanceFirstStrikes());

	iDefenderLowFS = (pAttacker->immuneToFirstStrikes()) ? 0 : firstStrikes();
	iDefenderHighFS = (pAttacker->immuneToFirstStrikes()) ? 0 : (firstStrikes() + chanceFirstStrikes());

	return LFBgetCombatOdds(iDefenderLowFS, iDefenderHighFS, iAttackerLowFS, iAttackerHighFS, iNeededRoundsDefender, iNeededRoundsAttacker, iDefenderOdds);
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* Afforess	                  Start		 06/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/

bool CvUnit::canTradeUnit(PlayerTypes eReceivingPlayer)
{
	CvArea* pWaterArea = NULL;
	CvCity* pCapitalCity;
	int iLoop;
	bool bShip = false;
	bool bCoast = false;

	pCapitalCity = GET_PLAYER(eReceivingPlayer).getCapitalCity();

	if (eReceivingPlayer == NO_PLAYER || eReceivingPlayer > MAX_PLAYERS)
	{
		return false;
	}
	
	if (getCargo() > 0)
	{
		return false;
	}
	
	if (getDomainType() == DOMAIN_SEA)
	{
		bShip = true;
		for (CvCity* pLoopCity = GET_PLAYER(eReceivingPlayer).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(eReceivingPlayer).nextCity(&iLoop))
		{
			if (((pWaterArea = pLoopCity->waterArea()) != NULL && !pWaterArea->isLake()))
			{
				bCoast = true;
			}
		}
	}
	
	if (bShip && !bCoast)
	{
		return false;
	}

	return true;
}
	
void CvUnit::tradeUnit(PlayerTypes eReceivingPlayer)
{
	CvUnit* pTradeUnit;
	CvWString szBuffer;
	CvCity* pBestCity;
	//CvArea* pWaterArea = NULL;
	PlayerTypes eOwner;
	int iLoop;
	
	eOwner = getOwnerINLINE();
	
	if (eReceivingPlayer != NO_PLAYER)
	{
		pBestCity = GET_PLAYER(eReceivingPlayer).getCapitalCity();
		
		if (getDomainType() == DOMAIN_SEA)
		//Find the first coastal city, and put the ship there
		{
			for (CvCity* pLoopCity = GET_PLAYER(eReceivingPlayer).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(eReceivingPlayer).nextCity(&iLoop))
			{
				if (pLoopCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
				{
					pBestCity = pLoopCity;
					break;
				}
			}
		}
		
		pTradeUnit = GET_PLAYER(eReceivingPlayer).initUnit(getUnitType(), pBestCity->getX_INLINE(), pBestCity->getY_INLINE(), AI_getUnitAIType());

		pTradeUnit->convert(this);
		
		 szBuffer = gDLL->getText("TXT_KEY_MISC_TRADED_UNIT_TO_YOU", GET_PLAYER(eOwner).getNameKey(), pTradeUnit->getNameKey());
		 gDLL->getInterfaceIFace()->addMessage(pTradeUnit->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_UNITGIFTED", MESSAGE_TYPE_INFO, pTradeUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pTradeUnit->getX_INLINE(), pTradeUnit->getY_INLINE(), true, true);
		 
	 }
}

bool CvUnit::spyNukeAffected(const CvPlot* pPlot, TeamTypes eTeam, int iRange) const
{
	CvPlot* pLoopPlot;
	int iDX, iDY;

	if (!(GET_TEAM(eTeam).isAlive()))
	{
		return false;
	}

	if (eTeam == getTeam())
	{
		return false;
	}

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->getTeam() == eTeam)
				{
					return true;
				}

				if (pLoopPlot->plotCheck(PUF_isCombatTeam, eTeam, getTeam()) != NULL)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool CvUnit::spyNuke(int iX, int iY, bool bCaught)
{
	CvPlot* pPlot;
	CvWString szBuffer;
	bool abTeamsAffected[MAX_TEAMS];
	int iI, iJ, iK;

	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		abTeamsAffected[iI] = spyNukeAffected(pPlot, (TeamTypes)iI, 1);
	}

	if (bCaught)
	{
		for (iI = 0; iI < MAX_TEAMS; iI++)
		{
			if (abTeamsAffected[iI])
			{
				if (!isEnemy((TeamTypes)iI))
				{
					if (!((TeamTypes)iI == GET_TEAM(getTeam()).getID()))
					{
						GET_TEAM(getTeam()).declareWar(((TeamTypes)iI), false, WARPLAN_TOTAL);
					}
				}
			}
		}
	}

	if (bCaught)
	{
		for (iI = 0; iI < MAX_TEAMS; iI++)
		{
			if (abTeamsAffected[iI])
			{
				GET_TEAM((TeamTypes)iI).changeWarWeariness(getTeam(), 100 * GC.getDefineINT("WW_HIT_BY_NUKE"));
				GET_TEAM(getTeam()).changeWarWeariness(((TeamTypes)iI), 100 * GC.getDefineINT("WW_ATTACKED_WITH_NUKE"));
				GET_TEAM(getTeam()).AI_changeWarSuccess(((TeamTypes)iI), GC.getDefineINT("WAR_SUCCESS_NUKE"));
			}
		}
	}

	if (bCaught)
	{
		for (iI = 0; iI < MAX_TEAMS; iI++)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (iI != getTeam())
				{
					if (abTeamsAffected[iI])
					{
						for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
						{
							if (GET_PLAYER((PlayerTypes)iJ).isAlive())
							{
								if (GET_PLAYER((PlayerTypes)iJ).getTeam() == ((TeamTypes)iI))
								{
									GET_PLAYER((PlayerTypes)iJ).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_NUKED_US, 1);
								}
							}
						}
					}
					else
					{
						for (iJ = 0; iJ < MAX_TEAMS; iJ++)
						{
							if (GET_TEAM((TeamTypes)iJ).isAlive())
							{
								if (abTeamsAffected[iJ])
								{
									if (GET_TEAM((TeamTypes)iI).isHasMet((TeamTypes)iJ))
									{
										if (GET_TEAM((TeamTypes)iI).AI_getAttitude((TeamTypes)iJ) >= ATTITUDE_CAUTIOUS)
										{
											for (iK = 0; iK < MAX_PLAYERS; iK++)
											{
												if (GET_PLAYER((PlayerTypes)iK).isAlive())
												{
													if (GET_PLAYER((PlayerTypes)iK).getTeam() == ((TeamTypes)iI))
													{
														GET_PLAYER((PlayerTypes)iK).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_NUKED_FRIEND, 1);
													}
												}
											}
											break;
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
	szBuffer = gDLL->getText("TXT_KEY_MISC_NUKE_UNKNOWN", GET_PLAYER(pPlot->getOwnerINLINE()).getNameKey());
	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (bCaught)
				szBuffer = gDLL->getText("TXT_KEY_MISC_NUKE_ENEMY_SPY", GET_PLAYER(getOwnerINLINE()).getNameKey(), GET_PLAYER(pPlot->getOwnerINLINE()).getNameKey());
			gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), (((PlayerTypes)iI) == getOwnerINLINE()), GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_NUKE_EXPLODES", MESSAGE_TYPE_MAJOR_EVENT, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
		}
	}

	//This is just so the espionage mission makes the cool explosion effect.
	if (GC.getInfoTypeForString("EFFECT_ICBM_NUCLEAR_EXPLOSION") != -1)
	{
		gDLL->getEngineIFace()->TriggerEffect((EffectTypes)GC.getInfoTypeForString("EFFECT_ICBM_NUCLEAR_EXPLOSION"), pPlot->getPoint(), 0);
		gDLL->getInterfaceIFace()->playGeneralSound("AS2D_NUKE_EXPLODES", pPlot->getPoint());
	}
	pPlot->nukeExplosion(1);
	return true;
}


bool CvUnit::canClaimTerritory(const CvPlot* pPlot) const
{

	if (!GET_PLAYER(getOwnerINLINE()).hasFixedBorders())
	{
		return false;
	}
	
	if (getOwnerINLINE() == BARBARIAN_PLAYER)
	{
		return false;
	}
	
	if (m_pUnitInfo->isAlwaysHostile())
	{
		return false;
	}

	if (!(m_pUnitInfo->isPillage()))
	{
		return false;
	}
	
	if (m_pUnitInfo->isHiddenNationality())
	{
		return false;
	}

	if (pPlot != NULL)
	{
	
		if (getOwnerINLINE() == pPlot->getOwnerINLINE())
		{
			return false;
		}
		/* if we or someone else (a friend) already claimed this plot in this turn */
		if (pPlot->getClaimingOwner() != NO_PLAYER)
		{
			return false;
		}

		if (!pPlot->isPotentialCityWork())
		{
			return false;
		}
		
		if (pPlot->isCity())
		{
			return false;
		}
		else if (pPlot->isCity(true))
		{
			if (!GET_TEAM(getTeam()).isAtWar(pPlot->getTeam()))
			{
				return false;
			}
		}

		if (GC.getGameINLINE().getModderGameOption(MODDERGAMEOPTION_CANNOT_CLAIM_OCEAN) && pPlot->isWater())
		{
			return false;
		}
		
		/* plots adjacent to cities always belong to those cities' owners */
		if (pPlot->getAdjacentCity() != NULL)
		{
			return false;
		}

		if (!pPlot->isOwned())
		{
			return true;
		}
		else if (potentialWarAction(pPlot))
		{
			return true;
		}
		
		return false;
	}
	
	return true;
}

bool CvUnit::claimTerritory()
{
	//logMsg("%S claims territory from %S at (%d, %d)", GET_PLAYER(getOwner()).getCivilizationShortDescription(), GET_PLAYER(plot()->getOwner()).getCivilizationShortDescription(), plot()->getX(), plot()->getY());
	

	CvPlot* pPlot = plot();
	bool bWasOwned = false;
	PlayerTypes pPlayerThatLostTerritory;
	
	if (!canClaimTerritory(pPlot))
	{
		return false;
	}
	if (pPlot->isOwned())
	{
		pPlayerThatLostTerritory = pPlot->getOwnerINLINE();
		TeamTypes tTeamThatLostTerritory = GET_PLAYER(pPlayerThatLostTerritory).getTeam();
		
		GET_TEAM(tTeamThatLostTerritory).changeWarWeariness(getTeam(), *pPlot, GC.getDefineINT("WW_PLOT_CAPTURED"));
		GET_TEAM(getTeam()).changeWarWeariness(tTeamThatLostTerritory, *pPlot, GC.getDefineINT("WW_CAPTURED_PLOT"));
		GET_TEAM(getTeam()).AI_changeWarSuccess(tTeamThatLostTerritory, GC.getDefineINT("WAR_SUCCESS_PLOT_CAPTURING"));
		
		bWasOwned = true;
	}
	
	pPlot->setClaimingOwner(getOwnerINLINE());
	
	if (bWasOwned)
	{
		CvWString szBuffer;
		CvCity *pNearestCity = GC.getMapINLINE().findCity(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pPlayerThatLostTerritory, NO_TEAM, false);				
		
		if (pNearestCity != NULL)
		{
			szBuffer = gDLL->getText("TXT_KEY_MISSION_CLAIM_TERRITORY_CIV_SUCCESS_NEAR", GET_PLAYER(getOwnerINLINE()).getCivilizationAdjectiveKey(), pNearestCity->getName().GetCString());
		}
		else 
		{
			szBuffer = gDLL->getText("TXT_KEY_MISSION_CLAIM_TERRITORY_CIV_SUCCESS", GET_PLAYER(getOwnerINLINE()).getCivilizationAdjectiveKey());
		}
		gDLL->getInterfaceIFace()->addMessage(pPlayerThatLostTerritory, true, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
	}

	finishMoves();

	return true;
}

int CvUnit::surroundedDefenseModifier(const CvPlot *pPlot, const CvUnit *pDefender) const
{
	if (!GC.getGameINLINE().isOption(GAMEOPTION_SAD))
	{
		return 0;
	}
	CvPlot *pLoopPlot;
	DirectionTypes dtDirectionAttacker = directionXY(pPlot, plot());		
	int iExtraModifier = 0, iI;
	
	for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

		if ((pLoopPlot != NULL) && (pLoopPlot->isWater() == pPlot->isWater()) && (iI != dtDirectionAttacker))
		{
			CLLNode<IDInfo>* pUnitNode;
			CvUnit* pLoopUnit;
			CvUnit* pBestUnit;
			int iBestCurrCombatStr, iLowestCurrCombatStr;

			pBestUnit = NULL;
			iBestCurrCombatStr = 0;
			iLowestCurrCombatStr = INT_MAX;
			pUnitNode = pLoopPlot->headUnitNode();

			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);	
				
				if (pLoopUnit->getTeam() == getTeam())
				{
					// if pDefender == null - use the last config of maxCombatStr() where pAttacker == this, else - use the pDefender to find the best attacker
					if (pDefender != NULL)
					{
						// pPlot == NULL -> defender gets no plot defense bonuses (hills, forest, fortify, etc.) against surrounding units (will have them for the attacker)
						int iTmpCurrCombatStr = pDefender->currCombatStr(NULL, pLoopUnit, NULL, false);
						
						if (iTmpCurrCombatStr < iLowestCurrCombatStr)
						{
							iLowestCurrCombatStr = iTmpCurrCombatStr;
							pBestUnit = pLoopUnit;
						}
					} 
					else
					{
						int iTmpCurrCombatStr = pLoopUnit->currCombatStr(pPlot, pLoopUnit, NULL, false);
						
						if (iTmpCurrCombatStr > iBestCurrCombatStr)
						{
							iBestCurrCombatStr = iTmpCurrCombatStr;
							pBestUnit = pLoopUnit;
						}
					}
				}
			}

			double fAttDeffFactor = 0.0;
			if (pDefender != NULL)
			{
				if (pBestUnit != NULL)
				{
					fAttDeffFactor = ((double) pBestUnit->currCombatStr(pPlot, pBestUnit, NULL, false)) / iLowestCurrCombatStr;
				}
			} 
			else 
			{
				fAttDeffFactor = (double) iBestCurrCombatStr;
			}

			/* surrounding distance = 1, 2, 3 or 4; the bigger the better */
			int iSurroundingDistance = abs(std::min(abs(iI - dtDirectionAttacker), abs(abs(iI - dtDirectionAttacker) - 8)));
		
			double fSurroundingDistanceFactor;
			
			switch (iSurroundingDistance)
			{
			case 1:
				fSurroundingDistanceFactor = GC.getDefineFLOAT("SAD_FACTOR_1");
				break;
			case 2:
				fSurroundingDistanceFactor = GC.getDefineFLOAT("SAD_FACTOR_2");
				break;
			case 3:
				fSurroundingDistanceFactor = GC.getDefineFLOAT("SAD_FACTOR_3");
				break;
			case 4:
				fSurroundingDistanceFactor = GC.getDefineFLOAT("SAD_FACTOR_4");
				break;
			default:
				fSurroundingDistanceFactor = GC.getDefineFLOAT("SAD_FACTOR_1");
				break;
			}

			if (fAttDeffFactor == 1)
			{
				iExtraModifier += int(fSurroundingDistanceFactor);
			}
			else if (fAttDeffFactor != 0)
			{
				iExtraModifier += int(fSurroundingDistanceFactor * ((fAttDeffFactor - 1) * pow(abs(fAttDeffFactor - 1), -0.75) + 1));				
			}
		}
	}
	
	return std::min(GC.getDefineINT("SAD_MAX_MODIFIER"), iExtraModifier);
}

int CvUnit::getCanMovePeaksCount() const
{
	return m_iCanMovePeaksCount;
}

bool CvUnit::isCanMovePeaks() const
{
	return (getCanMovePeaksCount() > 0);
}

void CvUnit::changeCanMovePeaksCount(int iChange)
{
	m_iCanMovePeaksCount += iChange;
	FAssert(getCanMovePeaksCount() >= 0);
}
void CvUnit::combatWon(CvUnit* pLoser, bool bAttacking)
{
 /*   //PromotionTypes ePromotion;
    //bool bConvert = false;
    int iUnit = NO_UNIT;
	int POW = NO_UNIT;
	//CvUnit* pLoopUnit;
    //CLLNode<IDInfo>* pUnitNode;
    //CvUnit* pLoopUnit;
    //CvPlot* pPlot;
    CvUnit* pUnit;

    if ((//m_pUnitInfo->getEnslavementChance() + 
	GET_PLAYER(getOwnerINLINE()).getEnslavementChance()) > 0)
    {
        if (//getDuration() == 0 && pLoser->isAlive() && !pLoser->isAnimal() && 
		iUnit == NO_UNIT)
        {
            if (GC.getGameINLINE().getSorenRandNum(100, "Enslavement") <= (//m_pUnitInfo->getEnslavementChance() + 
			GET_PLAYER(getOwnerINLINE()).getEnslavementChance()))
            {
                iUnit = ((UnitTypes)(GC.getDefineINT("SLAVE_UNIT")));
				//GC.getInfoTypeForString("UNIT_SLAVE");
            }
        }
    }
    if (iUnit != NO_UNIT)
    {
        if ((//!pLoser->isImmuneToCapture() && 
		!isNoCapture()// && !pLoser->isImmortal()
		)
		//|| GC.getUnitInfo((UnitTypes)pLoser->getUnitType()).getEquipmentPromotion() != NO_PROMOTION
		)
        {
            pUnit = GET_PLAYER(getOwnerINLINE()).initUnit((UnitTypes)iUnit, plot()->getX_INLINE(), plot()->getY_INLINE());
        }
    }*/
}

int CvUnit::getMaxHurryFood(CvCity* pCity) const
{
	int iFood;

	iFood = (m_pUnitInfo->getBaseFoodChange());

	iFood *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitHurryPercent();
	iFood /= 100;

	return std::max(0, iFood);
}


int CvUnit::getHurryFood(const CvPlot* pPlot) const
{
	CvCity* pCity;
	int iFood;
	int iFoodLeft;

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return 0;
	}

	iFood = getMaxHurryFood(pCity);
	iFoodLeft = (pCity->growthThreshold() - pCity->getFood());
	iFood = std::min(iFoodLeft, iFood);

	return std::max(0, iFood);
}


bool CvUnit::canHurryFood(const CvPlot* pPlot) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	CvCity* pCity;

	if (getHurryFood(pPlot) == 0)
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}
	
	if (pCity->getOwnerINLINE() != getOwnerINLINE())
	{
		return false;
	}

	if (pCity->getFoodTurnsLeft() == 1)
	{
		return false;
	}

	return true;
}


bool CvUnit::hurryFood()
{
	CvCity* pCity;

	if (!canHurryFood(plot()))
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		pCity->changeFood(getHurryFood(plot()));
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_HURRY_FOOD);
	}

	kill(true);

	return true;
}

bool CvUnit::sleepForEspionage()
{
	if (!canSleep(plot()) || !canEspionage(plot(), true))
	{
		return false;
	}
	
	if (getFortifyTurns() == GC.getDefineINT("MAX_FORTIFY_TURNS"))
	{
		return false;
	}
	
	getGroup()->setActivityType(ACTIVITY_SLEEP);
	
	m_iSleepTimer = 1;

	return true;
}

CvUnit* CvUnit::getCommander() const
{
	if (!GC.getGameINLINE().isOption(GAMEOPTION_GREAT_COMMANDERS)) {return NULL;}
	
	if (m_pUsedCommander != NULL)	//return already used one if it is not dead.
	//???? maybe also it have to be chosen an another commander if pUsedCommander get out of range
	{
		return m_pUsedCommander;	//??error prone? commander can be already dead or settled or etc and not being NULL? although it seems no problem here - no crashes during testing.
	}

	int iBestCommanderDistance = 9999999;
	CvUnit* pBestCommander = NULL;
	if (getOwnerINLINE() != NO_PLAYER)
	{
		CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());
		
		for (int i=0; i < (int)kPlayer.Commanders.size(); i++)		//loop through player's commanders
		{
			CvUnit* pCommander = kPlayer.Commanders[i];
			CvPlot* pCommPlot = pCommander->plot();
			FAssertMsg(pCommPlot != NULL, "Commander Should Exist!");
			if (pCommPlot == NULL)
			{
				continue;
			}
			int iDistance = plotDistance(pCommPlot->getX(), pCommPlot->getY(), plot()->getX(), plot()->getY());

			if (pCommander->controlPointsLeft() <= 0 || iDistance > pCommander->commandRange()) 
			{
				continue;
			}

			if (pBestCommander == NULL || 
				//best commander is at shorter distance, or at same distance but has more XP:
				iBestCommanderDistance <= iDistance || 
				iBestCommanderDistance == iDistance && pCommander->getExperience() > pBestCommander->getExperience())
				{
					pBestCommander = pCommander;
					iBestCommanderDistance = iDistance;
				}
		}
	}
	return pBestCommander;
}

void CvUnit::tryUseCommander()
{
	if (m_pUsedCommander != NULL)	//this unit is already using some commander
	{
		return;
	}

	CvUnit* pCommander = getCommander();
	
	if (pCommander != NULL)	//commander is used when any unit under his command fights in combat
	{
		pCommander->m_iControlPointsLeft -= 1;
		m_pUsedCommander = pCommander;
	}
}

bool CvUnit::isCommander() const
{
	return m_bCommander;
}

void CvUnit::setCommander(bool bNewVal)
{
	m_bCommander = bNewVal;
	if (isCommander())
	{
		GET_PLAYER(getOwnerINLINE()).Commanders.push_back(this);
		m_iControlPointsLeft = controlPoints();
	}
}

void CvUnit::nullUsedCommander()
{
	m_pUsedCommander = NULL;
}

CvUnit* CvUnit::getUsedCommander()
{
	return m_pUsedCommander;
}

int CvUnit::getExtraControlPoints() const	//control points
{
	return m_iExtraControlPoints;
}

void CvUnit::changeExtraControlPoints(int iChange)			
{
	m_iExtraControlPoints += iChange;
	m_iControlPointsLeft += iChange;
}

int CvUnit::controlPoints() const
{
	return m_pUnitInfo->getControlPoints() + getExtraControlPoints();
}

int CvUnit::controlPointsLeft() const
{
	return m_iControlPointsLeft;
}

int CvUnit::getExtraCommandRange() const	//command range											
{
	return m_iExtraCommandRange;
}

void CvUnit::changeExtraCommandRange(int iChange)			
{
	m_iExtraCommandRange += iChange;
}

int CvUnit::commandRange() const
{
	return m_pUnitInfo->getCommandRange() + getExtraCommandRange();
}

int CvUnit::getOnslaughtCount() const	//onslaught
{
	return m_iOnslaughtCount;
}

bool CvUnit::isOnslaught() const
{
	return (getOnslaughtCount() > 0);
}

void CvUnit::changeOnslaughtCount(int iChange)			
{
	m_iOnslaughtCount += iChange;
	FAssert(getOnslaughtCount() >= 0);
}

void CvUnit::gameLoadingAssignUsedCommander()
{
	if (m_iCommanderID != -1)
	{
		m_pUsedCommander = GET_PLAYER(getOwnerINLINE()).getUnit(m_iCommanderID);
	}
}

int CvUnit::interceptionChance(const CvPlot* pPlot) const
{
	int iValue;
	int iLoop;
	int iI;
	int iNoInterceptionChanceTimes100 = 10000;
	CvUnit* pLoopUnit;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (isEnemy(GET_PLAYER((PlayerTypes)iI).getTeam()) && !isInvisible(GET_PLAYER((PlayerTypes)iI).getTeam(), false, false))
			{
				for(pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
				{
					if (pLoopUnit->canAirDefend())
					{
						if (!pLoopUnit->isMadeInterception())
						{
							if ((pLoopUnit->getDomainType() != DOMAIN_AIR) || !(pLoopUnit->hasMoved()))
							{
								if ((pLoopUnit->getDomainType() != DOMAIN_AIR) || (pLoopUnit->getGroup()->getActivityType() == ACTIVITY_INTERCEPT))
								{
									if (plotDistance(pLoopUnit->getX_INLINE(), pLoopUnit->getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE()) <= pLoopUnit->airRange())
									{
										iValue = pLoopUnit->currInterceptionProbability();
										if (iValue > 0 && iValue < 100)
										{
											iNoInterceptionChanceTimes100 *= (100 - iValue);
											iNoInterceptionChanceTimes100 /= 100;
										}
										else if (iValue > 100)
											return 100;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return 100 - (iNoInterceptionChanceTimes100 / 100);
}

PlayerTypes CvUnit::getOriginalOwner() const
{
	return m_eOriginalOwner;
}

void CvUnit::doBattleFieldPromotions(CvUnit* pDefender, CombatDetails cdDefenderDetails, const CvPlot* pPlot, bool bAttackerHasLostNoHP, bool bAttackerWithdrawn, int iAttackerInitialDamage, int iWinningOdds, int iInitialAttXP, int iInitialAttGGXP, int iDefenderInitialDamage, int iInitialDefXP, int iInitialDefGGXP, bool &bAttackerPromoted, bool &bDefenderPromoted)
{
	if (GC.getGameINLINE().getModderGameOption(MODDERGAMEOPTION_BATTLEFIELD_PROMOTIONS))
	{
		if (getUnitCombatType() != NO_UNITCOMBAT && pDefender->getUnitCombatType() != NO_UNITCOMBAT)
		{
			std::vector<PromotionTypes> aAttackerAvailablePromotions;
			std::vector<PromotionTypes> aDefenderAvailablePromotions;
			int iI;
			for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)	//loop through promotions
			{
				CvPromotionInfo &kPromotion = GC.getPromotionInfo((PromotionTypes)iI);
				/* Block These Promotions */	
				if (kPromotion.getKamikazePercent() > 0)
					continue;
				if (kPromotion.getStateReligionPrereq() != NO_RELIGION)
				{
					if (GET_PLAYER(getOwnerINLINE()).getStateReligion() != kPromotion.getStateReligionPrereq())
						continue;
				}
				if (kPromotion.isLeader())
					continue;
				/* Block These Promotions */
				
				if (pDefender->isDead() || m_combatResult.bDefenderWithdrawn)
				{
					//* defender withdrawn, give him withdrawal promo
					if (m_combatResult.bDefenderWithdrawn && kPromotion.getWithdrawalChange() > 0 &&
						pDefender->canAcquirePromotion((PromotionTypes)iI))
					{
						aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
					}
					
					if (!canAcquirePromotion((PromotionTypes)iI)) //attacker can not acquire this promotion
					{
						continue;
					}
					//* attacker was crossing river
					if (kPromotion.isRiver() && cdDefenderDetails.iRiverAttackModifier != 0)	//this bonus is being applied to defender
					{
						aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* attack from water
					else if (kPromotion.isAmphib() && cdDefenderDetails.iAmphibAttackModifier != 0)
					{
						aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* attack terrain
					else if (kPromotion.getTerrainAttackPercent((int)pPlot->getTerrainType()) > 0)
					{
						aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* attack feature
					else if (pPlot->getFeatureType() != NO_FEATURE && 
						kPromotion.getFeatureAttackPercent((int)pPlot->getFeatureType()) > 0)
					{
						aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* attack hills
					else if (kPromotion.getHillsAttackPercent() > 0 && pPlot->isHills())
					{
						aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* attack city
					else if (kPromotion.getCityAttackPercent() > 0 && pPlot->isCity(true))	//count forts too
					{
						aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* first strikes/chanses promotions
					else if ((kPromotion.getFirstStrikesChange() > 0 || 
						kPromotion.getChanceFirstStrikesChange() > 0) && (firstStrikes() > 0 || chanceFirstStrikes() > 0)	)
					{
						aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* unit combat mod
					else if (kPromotion.getUnitCombatModifierPercent((int)pDefender->getUnitCombatType()) > 0)
					{
						aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* combat strength promotions
					else if (kPromotion.getCombatPercent() > 0)
					{
						aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* domain mod
					else if (kPromotion.getDomainModifierPercent((int)pDefender->getDomainType()))
					{
						aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* blitz
					else if (kPromotion.isBlitz() && bAttackerHasLostNoHP)
					{
						aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
					}
				}	//if defender is dead or withdrawn
				else	//attacker is dead or withdrawn
				{
					//* attacker withdrawn
					if (bAttackerWithdrawn && kPromotion.getWithdrawalChange() > 0 && canAcquirePromotion((PromotionTypes)iI))
					{
						aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
					}
					
					if (!pDefender->canAcquirePromotion((PromotionTypes)iI)) 
					{
						continue;
					}

					//* defend terrain
					if (kPromotion.getTerrainDefensePercent((int)pPlot->getTerrainType()) > 0)
					{
						aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* defend feature
					else if (pPlot->getFeatureType() != NO_FEATURE && 
						kPromotion.getFeatureDefensePercent((int)pPlot->getFeatureType()) > 0)
					{
						aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* defend hills
					else if (kPromotion.getHillsDefensePercent() > 0 && pPlot->isHills())
					{
						aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* defend city
					else if (kPromotion.getCityDefensePercent() > 0 && pPlot->isCity(true))	//count forts too
					{
						aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* first strikes/chanses promotions
					else if ((kPromotion.getFirstStrikesChange() > 0 || 
						kPromotion.getChanceFirstStrikesChange() > 0) && 
						(pDefender->firstStrikes() > 0 || pDefender->chanceFirstStrikes() > 0))
					{
						aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* unit combat mod vs attacker unit type
					else if (kPromotion.getUnitCombatModifierPercent((int)getUnitCombatType()) > 0)
					{
						aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* combat strength promotions
					else if (kPromotion.getCombatPercent() > 0)
					{
						aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
					}
					//* domain mod
					else if (kPromotion.getDomainModifierPercent((int)getDomainType()))
					{
						aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
					}
				}	//if attacker withdrawn
			}	//end promotion types cycle

			//promote attacker:
			if (!isDead() && aAttackerAvailablePromotions.size() > 0)
			{
				FAssertMsg(maxHitPoints() - iAttackerInitialDamage > 0, "Attacker is Dead!");
				int iHealthPercent = (maxHitPoints() - getDamage()) * 100 / std::max(1,(maxHitPoints() - iAttackerInitialDamage));
				int iPromotionChanceModifier = iHealthPercent * iHealthPercent / maxHitPoints();
				int iPromotionChance = (GC.getCOMBAT_DIE_SIDES() - iWinningOdds) * (100 + iPromotionChanceModifier) / 100;
				
				if (GC.getGameINLINE().getSorenRandNum(GC.getCOMBAT_DIE_SIDES(), "Occasional Promotion") < iPromotionChance)
				{
					//select random promotion from available
					PromotionTypes ptPromotion = aAttackerAvailablePromotions[
						GC.getGameINLINE().getSorenRandNum(aAttackerAvailablePromotions.size(), "Select Promotion Type")];
					//promote
					setHasPromotion(ptPromotion, true);
					bAttackerPromoted = true;
					//Reset XP
					setExperience100(iInitialAttXP);
					GET_PLAYER(getOwnerINLINE()).setCombatExperience(iInitialAttGGXP);
					getGroup()->setAutomateType(NO_AUTOMATE);
					//show message
					CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_UNIT_PROMOTED_IN_BATTLE", getNameKey(), GC.getPromotionInfo(ptPromotion).getText());
					gDLL->getInterfaceIFace()->addMessage(
						getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, 
						GC.getPromotionInfo((PromotionTypes)0).getSound(), MESSAGE_TYPE_INFO, NULL, 
						(ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), this->plot()->getX_INLINE(), this->plot()->getY_INLINE());
				}
			}
			//promote defender:
			if (!pDefender->isDead() && aDefenderAvailablePromotions.size() > 0)
			{
				FAssertMsg(pDefender->maxHitPoints() - iDefenderInitialDamage > 0, "Defender is Dead!");
				int iHealthPercent = (pDefender->maxHitPoints() - pDefender->getDamage()) * 100 / std::max(1,(pDefender->maxHitPoints() - iDefenderInitialDamage));
				int iPromotionChanceModifier = iHealthPercent * iHealthPercent / pDefender->maxHitPoints();
				int iPromotionChance = iWinningOdds * (100 + iPromotionChanceModifier) / 100;
			
				if (GC.getGameINLINE().getSorenRandNum(GC.getCOMBAT_DIE_SIDES(), "Occasional Promotion") < iPromotionChance)
				{
					//select random promotion from available
					PromotionTypes ptPromotion = aDefenderAvailablePromotions[
						GC.getGameINLINE().getSorenRandNum(aDefenderAvailablePromotions.size(), "Select Promotion Type")];
					//promote
					pDefender->setHasPromotion(ptPromotion, true);
					//Reset XP
					pDefender->setExperience100(iInitialDefXP);
					GET_PLAYER(pDefender->getOwnerINLINE()).setCombatExperience(iInitialDefGGXP);
					bDefenderPromoted = true;
					pDefender->getGroup()->setAutomateType(NO_AUTOMATE);
					//show message
					CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_UNIT_PROMOTED_IN_BATTLE", pDefender->getNameKey(), 
						GC.getPromotionInfo(ptPromotion).getText());
					gDLL->getInterfaceIFace()->addMessage(
						pDefender->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, 
						GC.getPromotionInfo((PromotionTypes)0).getSound(), MESSAGE_TYPE_INFO, NULL, 
						(ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
				}
			}
		}
	}
}
void CvUnit::doDynamicXP(CvUnit* pDefender, const CvPlot* pPlot, int iAttackerInitialDamage, int iWinningOdds, int iDefenderInitialDamage, int iInitialAttXP, int iInitialDefXP, int iInitialAttGGXP, int iInitialDefGGXP, bool bPromotion, bool bDefPromotion)
{
	if (GC.getGameINLINE().isModderGameOption(MODDERGAMEOPTION_IMPROVED_XP))
	{
		if (!isDead() && !bPromotion && getUnitCombatType() != NO_UNITCOMBAT)
		{
			//reset XP
			 setExperience100(iInitialAttXP);
			// GET_PLAYER(getOwnerINLINE()).setCombatExperience(iInitialAttGGXP);
			// if (m_pUsedCommander != NULL)
			// {
				// m_pUsedCommander->setExperience100(m_pUsedCommander->getExperience100() - 100);
			// }
			FAssertMsg(maxHitPoints() - iAttackerInitialDamage > 0, "Attacker is Dead!");
			int iHealthPercent = (maxHitPoints() - getDamage()) * 100 / std::max(1,(maxHitPoints() - iAttackerInitialDamage));
			int iExperienceModifier = (100 - iHealthPercent) * (100 - iHealthPercent) / maxHitPoints();
			//Chance of losing
			int iOdds = GC.getCOMBAT_DIE_SIDES() - iWinningOdds;
			int iExperience = iOdds * (100 + iExperienceModifier) / 100;
			
			iExperience = range(iExperience, GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT") * 25, attackXPValue() * 100);
			changeExperience100(iExperience, maxXPValue() == MAX_INT ? MAX_INT : maxXPValue() * 100, true, pPlot->getOwnerINLINE() == getOwnerINLINE(), (!pDefender->isBarbarian() || GC.getGameINLINE().isOption(GAMEOPTION_BARBARIAN_GENERALS)));
		}
		if (!pDefender->isDead() && !bDefPromotion && pDefender->getUnitCombatType() != NO_UNITCOMBAT)
		{
			//reset XP
			 pDefender->setExperience100(iInitialDefXP);
			// GET_PLAYER(pDefender->getOwnerINLINE()).setCombatExperience(iInitialDefGGXP);
			// if (pDefender->m_pUsedCommander != NULL)
			// {
				// pDefender->m_pUsedCommander->setExperience100(pDefender->m_pUsedCommander->getExperience100() - 100);
			// }
			FAssertMsg(pDefender->maxHitPoints() - iDefenderInitialDamage > 0, "Defender is Dead!");
			int iHealthPercent = (pDefender->maxHitPoints() - pDefender->getDamage()) * 100 / std::max(1,(pDefender->maxHitPoints() - iDefenderInitialDamage));
			int iExperienceModifier = (100 - iHealthPercent) * (100 - iHealthPercent) / pDefender->maxHitPoints();
			//Chance of Losing
			int iOdds = iWinningOdds;
			//iOdds = std::max(iOdds, -iOdds);
			int iExperience = iOdds * (100 + iExperienceModifier) / 100;
			iExperience = range(iExperience, GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT") * 25, pDefender->defenseXPValue() * 100);
			pDefender->changeExperience100(iExperience, pDefender->maxXPValue() == MAX_INT ? MAX_INT : pDefender->maxXPValue() * 100, true, pPlot->getOwnerINLINE() == pDefender->getOwnerINLINE(), (!isBarbarian() || GC.getGameINLINE().isOption(GAMEOPTION_BARBARIAN_GENERALS)));
			
		}
	}
}

bool CvUnit::isTerrainProtected(TerrainTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiTerrainProtected[eIndex] > 0;
}

int CvUnit::getTerrainProtectedCount(TerrainTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiTerrainProtected[eIndex];
}


void CvUnit::changeTerrainProtected(TerrainTypes eIndex, int iNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiTerrainProtected[eIndex] += iNewValue;
}

void CvUnit::doCommerceAttacks(const CvUnit* pDefender, const CvPlot* pPlot)
{
	if (pDefender->isDead())
	{
		if (pPlot->getPlotCity() != NULL)
		{
			for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
			{
				if (pPlot->getPlotCity()->getMaxCommerceAttacks((CommerceTypes)iI) > 0)
				{
					pPlot->getPlotCity()->changeCommerceAttacks((CommerceTypes)iI, 1);
				}
			}
		}
	}
}

int CvUnit::getZoneOfControlCount() const
{
	return m_iZoneOfControlCount;
}

bool CvUnit::isZoneOfControl() const
{
	return (getZoneOfControlCount() > 0);
}

void CvUnit::changeZoneOfControlCount(int iChange)			
{
	m_iZoneOfControlCount += iChange;
	FAssert(getOnslaughtCount() >= 0);
}

bool CvUnit::isAutoPromoting() const
{
	return m_bAutoPromoting;
}
void CvUnit::setAutoPromoting(bool bNewValue)
{
	m_bAutoPromoting = bNewValue;
	if (bNewValue)
	{
		//Force recalculation
		setPromotionReady(false);
		testPromotionReady();
	}
}

bool CvUnit::isAutoUpgrading() const
{
	return m_bAutoUpgrading;
}
void CvUnit::setAutoUpgrading(bool bNewValue)
{
	m_bAutoUpgrading = bNewValue;
}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


CvProperties* CvUnit::getProperties()
{
	return &m_Properties;
}

const CvProperties* CvUnit::getPropertiesConst() const
{
	return &m_Properties;
}