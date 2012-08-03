#include "CvGameCoreDLL.h"
#include "CvGameCoreUtils.h"
#include <algorithm>
#include "CvUnit.h"
#include "CvGameAI.h"
#include "CvPlayerAI.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvRandom.h"
#include "FAStarNode.h"
#include "CvCity.h"
#include "CvTeamAI.h"
#include "CvInfos.h"
#include "cvGlobals.h"
#include "FProfiler.h"

#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"


#define PATH_MOVEMENT_WEIGHT									(1000)
#define PATH_RIVER_WEIGHT											(100)
#define PATH_CITY_WEIGHT											(100)
#define PATH_DEFENSE_WEIGHT										(10)
#define PATH_TERRITORY_WEIGHT									(3)
#define PATH_STEP_WEIGHT											(2)
#define PATH_STRAIGHT_WEIGHT									(1)

#define PATH_DAMAGE_WEIGHT										(500)
CvPlot* plotCity(int iX, int iY, int iIndex)
{
	return GC.getMapINLINE().plotINLINE((iX + GC.getCityPlotX()[iIndex]), (iY + GC.getCityPlotY()[iIndex]));
}

int plotCityXY(int iDX, int iDY)
{
	if ((abs(iDX) > CITY_PLOTS_RADIUS) || (abs(iDY) > CITY_PLOTS_RADIUS))
	{
		return -1;
	}
	else
	{
		return GC.getXYCityPlot((iDX + CITY_PLOTS_RADIUS), (iDY + CITY_PLOTS_RADIUS));
	}
}

int plotCityXY(const CvCity* pCity, const CvPlot* pPlot)
{
	return plotCityXY(dxWrap(pPlot->getX_INLINE() - pCity->getX_INLINE()), dyWrap(pPlot->getY_INLINE() - pCity->getY_INLINE()));
}

CardinalDirectionTypes getOppositeCardinalDirection(CardinalDirectionTypes eDir)
{
	return (CardinalDirectionTypes)((eDir + 2) % NUM_CARDINALDIRECTION_TYPES);
}

DirectionTypes cardinalDirectionToDirection(CardinalDirectionTypes eCard)
{
	switch (eCard)
	{
	case CARDINALDIRECTION_NORTH:
		return DIRECTION_NORTH;
	case CARDINALDIRECTION_EAST:
		return DIRECTION_EAST;
	case CARDINALDIRECTION_SOUTH:
		return DIRECTION_SOUTH;
	case CARDINALDIRECTION_WEST:
		return DIRECTION_WEST;
	}
	return NO_DIRECTION;
}

bool isCardinalDirection(DirectionTypes eDirection)
{
	switch( eDirection )
	{
	case DIRECTION_EAST:
	case DIRECTION_NORTH:
	case DIRECTION_SOUTH:
	case DIRECTION_WEST:
		return true;
	}
	return false;
}

DirectionTypes estimateDirection(int iDX, int iDY)
{
	const int displacementSize = 8;
	static float sqrt2 = 1 / sqrt(2.0f);
	//													N			NE			E			SE				S			SW				W			NW
	static float displacements[displacementSize][2] = {{0, 1}, {sqrt2, sqrt2}, {1, 0}, {sqrt2, -sqrt2}, {0, -1}, {-sqrt2, -sqrt2}, {-1, 0}, {-sqrt2, sqrt2}};
	float maximum = 0;
	int maximumIndex = -1;
	for(int i=0;i<displacementSize;i++)
	{
		float dotProduct = iDX * displacements[i][0] + iDY * displacements[i][1];
		if(dotProduct > maximum)
		{
			maximum = dotProduct;
			maximumIndex = i;
		}
	}

	return (DirectionTypes) maximumIndex;
}

DirectionTypes estimateDirection(const CvPlot* pFromPlot, const CvPlot* pToPlot)
{
	return estimateDirection(dxWrap(pToPlot->getX_INLINE() - pFromPlot->getX_INLINE()), dyWrap(pToPlot->getY_INLINE() - pFromPlot->getY_INLINE()));
}


float directionAngle( DirectionTypes eDirection )
{
	switch( eDirection )
	{
	case DIRECTION_NORTHEAST:	return fM_PI * 0.25f;
	case DIRECTION_EAST:			return fM_PI * 0.5f;
	case DIRECTION_SOUTHEAST:	return fM_PI * 0.75f;
	case DIRECTION_SOUTH:			return fM_PI * 1.0f;
	case DIRECTION_SOUTHWEST:	return fM_PI * 1.25f;
	case DIRECTION_WEST:			return fM_PI * 1.5f;
	case DIRECTION_NORTHWEST:	return fM_PI * 1.75f;
	default:
	case DIRECTION_NORTH:			return 0.0f;
	}
}

bool atWar(TeamTypes eTeamA, TeamTypes eTeamB)
{
	if ((eTeamA == NO_TEAM) || (eTeamB == NO_TEAM))
	{
		return false;
	}

	FAssert(GET_TEAM(eTeamA).isAtWar(eTeamB) == GET_TEAM(eTeamB).isAtWar(eTeamA));
	FAssert((eTeamA != eTeamB) || !(GET_TEAM(eTeamA).isAtWar(eTeamB)));

	return GET_TEAM(eTeamA).isAtWar(eTeamB);
}

bool isNonAlly(TeamTypes eOurTeam, TeamTypes eTheirTeam)
{
	FAssert(eOurTeam != NO_TEAM);

	if (eTheirTeam == NO_TEAM)
	{
		return true;
	}

	return (atWar(eOurTeam, eTheirTeam) || GET_TEAM(eTheirTeam).canDeclareWar(eOurTeam));
}

bool isPotentialEnemy(TeamTypes eOurTeam, TeamTypes eTheirTeam)
{
	FAssert(eOurTeam != NO_TEAM);

	if (eTheirTeam == NO_TEAM)
	{
		return false;
	}

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       05/05/09                                jdog5000      */
/*                                                                                              */
/* Bugfix, General AI                                                                           */
/************************************************************************************************/
/* original bts code
	return (atWar(eOurTeam, eTheirTeam) || GET_TEAM(eOurTeam).AI_isSneakAttackReady(eTheirTeam));
*/
	// Fixes bug where AI would launch invasion while unable to declare war
	// which caused units to be bumped once forced peace expired
	return (atWar(eOurTeam, eTheirTeam) || (GET_TEAM(eOurTeam).AI_isSneakAttackReady(eTheirTeam) && GET_TEAM(eOurTeam).canDeclareWar(eTheirTeam)));
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
}

CvCity* getCity(IDInfo city)
{
	if ((city.eOwner >= 0) && city.eOwner < MAX_PLAYERS)
	{
		return (GET_PLAYER((PlayerTypes)city.eOwner).getCity(city.iID));
	}

	return NULL;
}

CvUnit* getUnit(IDInfo unit)
{
	if ((unit.eOwner >= 0) && unit.eOwner < MAX_PLAYERS)
	{
		return (GET_PLAYER((PlayerTypes)unit.eOwner).getUnit(unit.iID));
	}

	return NULL;
}

bool isBeforeUnitCycle(const CvUnit* pFirstUnit, const CvUnit* pSecondUnit)
{
	FAssert(pFirstUnit != NULL);
	FAssert(pSecondUnit != NULL);
	FAssert(pFirstUnit != pSecondUnit);

	if (pFirstUnit->getOwnerINLINE() != pSecondUnit->getOwnerINLINE())
	{
		return (pFirstUnit->getOwnerINLINE() < pSecondUnit->getOwnerINLINE());
	}

	if (pFirstUnit->getDomainType() != pSecondUnit->getDomainType())
	{
		return (pFirstUnit->getDomainType() < pSecondUnit->getDomainType());
	}

	if (pFirstUnit->baseCombatStr() != pSecondUnit->baseCombatStr())
	{
		return (pFirstUnit->baseCombatStr() > pSecondUnit->baseCombatStr());
	}

	if (pFirstUnit->getUnitType() != pSecondUnit->getUnitType())
	{
		return (pFirstUnit->getUnitType() > pSecondUnit->getUnitType());
	}

	if (pFirstUnit->getLevel() != pSecondUnit->getLevel())
	{
		return (pFirstUnit->getLevel() > pSecondUnit->getLevel());
	}

	if (pFirstUnit->getExperience() != pSecondUnit->getExperience())
	{
		return (pFirstUnit->getExperience() > pSecondUnit->getExperience());
	}

	return (pFirstUnit->getID() < pSecondUnit->getID());
}

/*************************************************************************************************/
/** ADVANCED COMBAT ODDS                      11/7/09                           PieceOfMind      */
/** BEGIN                                                                       v?.?             */
/*************************************************************************************************/
bool isPromotionValid(PromotionTypes ePromotion, UnitTypes eUnit, bool bLeader)
{
	// RevolutionDCM - super spies
	// Disable spy promotions mechanism
	if (GC.getUnitInfo(eUnit).isSpy() && !GC.isSS_ENABLED()) {
		return false;
	}
	// RevolutionDCM - end
	
	CvUnitInfo& kUnit = GC.getUnitInfo(eUnit);
	CvPromotionInfo& kPromotion = GC.getPromotionInfo(ePromotion);

	if (kUnit.getFreePromotions(ePromotion))
	{
		return true;
	}

	if (kUnit.getUnitCombatType() == NO_UNITCOMBAT)
	{
		return false;
	}

	if (!bLeader && kPromotion.isLeader())
	{
		return false;
	}

	if (!(kPromotion.getUnitCombat(kUnit.getUnitCombatType())))
	{
		return false;
	}

/************************************************************************************************/
/* SUPER_SPIES                             04/05/08                                Faichele     */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/* original code
	if (kUnit.isOnlyDefensive())
	{
		if ((kPromotion.getCityAttackPercent() != 0) ||
			  (kPromotion.getWithdrawalChange() != 0) ||
			  (kPromotion.getCollateralDamageChange() != 0) ||
			  (kPromotion.isBlitz()) ||
			  (kPromotion.isAmphib()) ||
			  (kPromotion.isRiver()) ||
			  (kPromotion.getHillsAttackPercent() != 0))
		{
			return false;
		}
	}
*/
	//TSHEEP Override for Spy promotions
	//if (kUnit.isOnlyDefensive())
	if (kUnit.isOnlyDefensive() && !kUnit.isSpy())//TSHEEP End
	{
		if ((kPromotion.getCityAttackPercent() != 0) ||
			  (kPromotion.getWithdrawalChange() != 0) ||
			  (kPromotion.getCollateralDamageChange() != 0) ||
			  (kPromotion.isBlitz()) ||
			  (kPromotion.isAmphib()) ||
			  (kPromotion.isRiver()) ||
			  (kPromotion.getHillsAttackPercent() != 0))
		{
			return false;
		}
	}
/************************************************************************************************/
/* SUPER_SPIES                             END                                                  */
/************************************************************************************************/

	if (kUnit.isIgnoreTerrainCost())
	{
		if (kPromotion.getMoveDiscountChange() != 0)
		{
			return false;
		}
	}

	if (kUnit.getMoves() == 1)
	{
		if (kPromotion.isBlitz())
		{
			return false;
		}
	}

	if ((kUnit.getCollateralDamage() == 0) || (kUnit.getCollateralDamageLimit() == 0) || (kUnit.getCollateralDamageMaxUnits() == 0))
	{
		if (kPromotion.getCollateralDamageChange() != 0)
		{
			return false;
		}
	}
	
/************************************************************************************************/
/* SUPER_SPIES                             04/05/08                                Faichele     */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
/* original code
	if (kUnit.getInterceptionProbability() == 0)
	{
		if (kPromotion.getInterceptChange() != 0)
		{
			return false;
		}
	}
*/
	//TSHEEP - Spy Promotion Override
	//if (kUnit.getInterceptionProbability() == 0)
	if (kUnit.getInterceptionProbability() == 0 && !kUnit.isSpy())//TSHEEP End
	{
		if (kPromotion.getInterceptChange() != 0)
		{
			return false;
		}
	}
/************************************************************************************************/
/* SUPER_SPIES                             END                                                  */
/************************************************************************************************/

	if (NO_PROMOTION != kPromotion.getPrereqPromotion())
	{
		if (!isPromotionValid((PromotionTypes)kPromotion.getPrereqPromotion(), eUnit, bLeader))
		{
			return false;
		}
	}

	PromotionTypes ePrereq1 = (PromotionTypes)kPromotion.getPrereqOrPromotion1();
	PromotionTypes ePrereq2 = (PromotionTypes)kPromotion.getPrereqOrPromotion2();
	if (NO_PROMOTION != ePrereq1 || NO_PROMOTION != ePrereq2)
	{
		bool bValid = false;
		if (!bValid)
		{
			if (NO_PROMOTION != ePrereq1 && isPromotionValid(ePrereq1, eUnit, bLeader))
			{
				bValid = true;
			}
		}

		if (!bValid)
		{
			if (NO_PROMOTION != ePrereq2 && isPromotionValid(ePrereq2, eUnit, bLeader))
			{
				bValid = true;
			}
		}

		if (!bValid)
		{
			return false;
		}
	}

	return true;
}
/*************************************************************************************************/
/** ADVANCED COMBAT ODDS                      11/7/09                           PieceOfMind      */
/** END                                                                                          */
/*************************************************************************************************/

int getPopulationAsset(int iPopulation)
{
	return (iPopulation * 2);
}

int getLandPlotsAsset(int iLandPlots)
{
	return iLandPlots;
}

int getPopulationPower(int iPopulation)
{
	return (iPopulation / 2);
}

int getPopulationScore(int iPopulation)
{
	return iPopulation;
}

int getLandPlotsScore(int iLandPlots)
{
	return iLandPlots;
}

int getTechScore(TechTypes eTech)
{
	return (GC.getTechInfo(eTech).getEra() + 1);
}

int getWonderScore(BuildingClassTypes eWonderClass)
{
	if (isLimitedWonderClass(eWonderClass))
	{
		return 5;
	}
	else
	{
		return 0;
	}
}

ImprovementTypes finalImprovementUpgrade(ImprovementTypes eImprovement, int iCount)
{
	FAssertMsg(eImprovement != NO_IMPROVEMENT, "Improvement is not assigned a valid value");

	if (iCount > GC.getNumImprovementInfos())
	{
		return NO_IMPROVEMENT;
	}

	if (GC.getImprovementInfo(eImprovement).getImprovementUpgrade() != NO_IMPROVEMENT)
	{
		return finalImprovementUpgrade(((ImprovementTypes)(GC.getImprovementInfo(eImprovement).getImprovementUpgrade())), (iCount + 1));
	}
	else
	{
		return eImprovement;
	}
}

int getWorldSizeMaxConscript(CivicTypes eCivic)
{
	int iMaxConscript;

	iMaxConscript = GC.getCivicInfo(eCivic).getMaxConscript();

	iMaxConscript *= std::max(0, (GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getMaxConscriptModifier() + 100));
	iMaxConscript /= 100;

	return iMaxConscript;
}

bool isReligionTech(TechTypes eTech)
{
	int iI;

	for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		if (GC.getReligionInfo((ReligionTypes)iI).getTechPrereq() == eTech)
		{
			return true;
		}
	}

	return false;
}

bool isCorporationTech(TechTypes eTech)
{
	int iI;

	for (iI = 0; iI < GC.getNumCorporationInfos(); iI++)
	{
		if (GC.getCorporationInfo((CorporationTypes)iI).getTechPrereq() == eTech)
		{
			return true;
		}
	}

	return false;
}

bool isTechRequiredForUnit(TechTypes eTech, UnitTypes eUnit)
{
	int iI;
	CvUnitInfo& info = GC.getUnitInfo(eUnit);

	if (info.getPrereqAndTech() == eTech)
	{
		return true;
	}

	for (iI = 0; iI < GC.getNUM_UNIT_AND_TECH_PREREQS(); iI++)
	{
		if (info.getPrereqAndTechs(iI) == eTech)
		{
			return true;
		}
	}

	return false;
}

bool isTechRequiredForBuilding(TechTypes eTech, BuildingTypes eBuilding)
{
	int iI;
	CvBuildingInfo& info = GC.getBuildingInfo(eBuilding);

	if (info.getPrereqAndTech() == eTech)
	{
		return true;
	}

	for (iI = 0; iI < GC.getNUM_BUILDING_AND_TECH_PREREQS(); iI++)
	{
		if (info.getPrereqAndTechs(iI) == eTech)
		{
			return true;
		}
	}

	SpecialBuildingTypes eSpecial = (SpecialBuildingTypes)info.getSpecialBuildingType();
	if (NO_SPECIALBUILDING != eSpecial && GC.getSpecialBuildingInfo(eSpecial).getTechPrereq() == eTech)
	{
		return true;
	}

	return false;
}

bool isTechRequiredForProject(TechTypes eTech, ProjectTypes eProject)
{
	if (GC.getProjectInfo(eProject).getTechPrereq() == eTech)
	{
		return true;
	}

	return false;
}

bool isWorldUnitClass(UnitClassTypes eUnitClass)
{
	return (GC.getUnitClassInfo(eUnitClass).getMaxGlobalInstances() != -1);
}

bool isTeamUnitClass(UnitClassTypes eUnitClass)
{
	return (GC.getUnitClassInfo(eUnitClass).getMaxTeamInstances() != -1);
}

bool isNationalUnitClass(UnitClassTypes eUnitClass)
{
	return (GC.getUnitClassInfo(eUnitClass).getMaxPlayerInstances() != -1);
}

bool isLimitedUnitClass(UnitClassTypes eUnitClass)
{
	return (isWorldUnitClass(eUnitClass) || isTeamUnitClass(eUnitClass) || isNationalUnitClass(eUnitClass));
}

bool isWorldWonderClass(BuildingClassTypes eBuildingClass)
{
	return (GC.getBuildingClassInfo(eBuildingClass).getMaxGlobalInstances() != -1);
}

bool isTeamWonderClass(BuildingClassTypes eBuildingClass)
{
	return (GC.getBuildingClassInfo(eBuildingClass).getMaxTeamInstances() != -1);
}

bool isNationalWonderClass(BuildingClassTypes eBuildingClass)
{
	return (GC.getBuildingClassInfo(eBuildingClass).getMaxPlayerInstances() != -1);
}

bool isLimitedWonderClass(BuildingClassTypes eBuildingClass)
{
	return (isWorldWonderClass(eBuildingClass) || isTeamWonderClass(eBuildingClass) || isNationalWonderClass(eBuildingClass));
}

int limitedWonderClassLimit(BuildingClassTypes eBuildingClass)
{
	int iMax;
	int iCount = 0;
	bool bIsLimited = false;

	iMax = GC.getBuildingClassInfo(eBuildingClass).getMaxGlobalInstances();
	if (iMax != -1)
	{
		iCount += iMax;
		bIsLimited = true;
	}

	iMax = GC.getBuildingClassInfo(eBuildingClass).getMaxTeamInstances();
	if (iMax != -1)
	{
		iCount += iMax;
		bIsLimited = true;
	}

	iMax = GC.getBuildingClassInfo(eBuildingClass).getMaxPlayerInstances();
	if (iMax != -1)
	{
		iCount += iMax;
		bIsLimited = true;
	}

	return bIsLimited ? iCount : -1;
}

bool isWorldProject(ProjectTypes eProject)
{
	return (GC.getProjectInfo(eProject).getMaxGlobalInstances() != -1);
}

bool isTeamProject(ProjectTypes eProject)
{
	return (GC.getProjectInfo(eProject).getMaxTeamInstances() != -1);
}

bool isLimitedProject(ProjectTypes eProject)
{
	return (isWorldProject(eProject) || isTeamProject(eProject));
}

// FUNCTION: getBinomialCoefficient
// Needed for getCombatOdds
// Returns int value, being the possible number of combinations 
// of k draws out of a population of n
// Written by DeepO 
// Modified by Jason Winokur to keep the intermediate factorials small
__int64 getBinomialCoefficient(int iN, int iK)
{
	__int64 iTemp = 1;
	//take advantage of symmetry in combination, eg. 15C12 = 15C3
	iK = std::min(iK, iN - iK);
	
	//eg. 15C3 = (15 * 14 * 13) / (1 * 2 * 3) = 15 / 1 * 14 / 2 * 13 / 3 = 455
	for(int i=1;i<=iK;i++)
		iTemp = (iTemp * (iN - i + 1)) / i;

	// Make sure iTemp fits in an integer (and thus doesn't overflow)
	FAssert(iTemp < MAX_INT);

	return iTemp;
}

// FUNCTION: getCombatOdds
// Calculates combat odds, given two units
// Returns value from 0-1000
// Written by DeepO
int getCombatOdds(CvUnit* pAttacker, const CvUnit* pDefender)
{
	float fOddsEvent;
	float fOddsAfterEvent;
	int iAttackerStrength;
	int iAttackerFirepower;
	int iDefenderStrength;
	int iDefenderFirepower;
	int iDefenderOdds;
	int iAttackerOdds;
	int iStrengthFactor;
	int iDamageToAttacker;
	int iDamageToDefender;
	int iNeededRoundsAttacker;
	int iNeededRoundsDefender;
	int iMaxRounds;
	int iAttackerLowFS;
	int iAttackerHighFS;
	int iDefenderLowFS;
	int iDefenderHighFS;
	int iFirstStrikes;
	int iDefenderHitLimit;
	int iI;
	int iJ;
	int iI3;
	int iI4;
	int iOdds = 0;

	// setup battle, calculate strengths and odds
	//////

	//Added ST
	iAttackerStrength = pAttacker->currCombatStr(NULL, NULL);
	iAttackerFirepower = pAttacker->currFirepower(NULL, NULL);

	iDefenderStrength = pDefender->currCombatStr(pDefender->plot(), pAttacker);
	iDefenderFirepower = pDefender->currFirepower(pDefender->plot(), pAttacker);

	FAssert((iAttackerStrength + iDefenderStrength) > 0);
	FAssert((iAttackerFirepower + iDefenderFirepower) > 0);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Efficiency, Lead From Behind                                                                 */
/************************************************************************************************/
	// From Lead From Behind by UncutDragon
/* original code
	iDefenderOdds = ((GC.getDefineINT("COMBAT_DIE_SIDES") * iDefenderStrength) / (iAttackerStrength + iDefenderStrength));
*/	// modified
	iDefenderOdds = ((GC.getCOMBAT_DIE_SIDES() * iDefenderStrength) / (iAttackerStrength + iDefenderStrength));
	// /UncutDragon

	if (iDefenderOdds == 0)
	{
		return 1000;
	}

	// UncutDragon
/* original code
	iAttackerOdds = GC.getDefineINT("COMBAT_DIE_SIDES") - iDefenderOdds;	
*/	// modified
	iAttackerOdds = GC.getCOMBAT_DIE_SIDES() - iDefenderOdds;	
	// /UncutDragon

	if (iAttackerOdds == 0)
	{
		return 0;
	}

	iStrengthFactor = ((iAttackerFirepower + iDefenderFirepower + 1) / 2);

	// calculate damage done in one round
	//////

	// UncutDragon
/* original code
	iDamageToAttacker = std::max(1,((GC.getDefineINT("COMBAT_DAMAGE") * (iDefenderFirepower + iStrengthFactor)) / (iAttackerFirepower + iStrengthFactor)));
	iDamageToDefender = std::max(1,((GC.getDefineINT("COMBAT_DAMAGE") * (iAttackerFirepower + iStrengthFactor)) / (iDefenderFirepower + iStrengthFactor)));
*/	// modified
	iDamageToAttacker = std::max(1,((GC.getCOMBAT_DAMAGE() * (iDefenderFirepower + iStrengthFactor)) / (iAttackerFirepower + iStrengthFactor)));
	iDamageToDefender = std::max(1,((GC.getCOMBAT_DAMAGE() * (iAttackerFirepower + iStrengthFactor)) / (iDefenderFirepower + iStrengthFactor)));
	// /UncutDragon

	// calculate needed rounds.
	// Needed rounds = round_up(health/damage)
	//////

	iDefenderHitLimit = pDefender->maxHitPoints() - pAttacker->combatLimit();

	iNeededRoundsAttacker = (std::max(0, pDefender->currHitPoints() - iDefenderHitLimit) + iDamageToDefender - 1 ) / iDamageToDefender;
	iNeededRoundsDefender = (pAttacker->currHitPoints() + iDamageToAttacker - 1 ) / iDamageToAttacker;
	iMaxRounds = iNeededRoundsAttacker + iNeededRoundsDefender - 1;

	// calculate possible first strikes distribution.
	// We can't use the getCombatFirstStrikes() function (only one result,
	// no distribution), so we need to mimic it.
	//////

	iAttackerLowFS = (pDefender->immuneToFirstStrikes()) ? 0 : pAttacker->firstStrikes();
	iAttackerHighFS = (pDefender->immuneToFirstStrikes()) ? 0 : (pAttacker->firstStrikes() + pAttacker->chanceFirstStrikes());

	iDefenderLowFS = (pAttacker->immuneToFirstStrikes()) ? 0 : pDefender->firstStrikes();
	iDefenderHighFS = (pAttacker->immuneToFirstStrikes()) ? 0 : (pDefender->firstStrikes() + pDefender->chanceFirstStrikes());

	// UncutDragon
	if (GC.getLFBEnable())
		return LFBgetCombatOdds(iAttackerLowFS, iAttackerHighFS, iDefenderLowFS, iDefenderHighFS, iNeededRoundsAttacker, iNeededRoundsDefender, iAttackerOdds);
	// /UncutDragon

	// For every possible first strike event, calculate the odds of combat.
	// Then, add these to the total, weighted to the chance of that first 
	// strike event occurring
	//////

	for (iI = iAttackerLowFS; iI < iAttackerHighFS + 1; iI++)
	{
		for (iJ = iDefenderLowFS; iJ < iDefenderHighFS + 1; iJ++)
		{
			// for every possible combination of fs results, calculate the chance

			if (iI >= iJ)
			{
				// Attacker gets more or equal first strikes than defender

				iFirstStrikes = iI - iJ;

				// For every possible first strike getting hit, calculate both
				// the chance of that event happening, as well as the rest of 
				// the chance assuming the event has happened. Multiply these 
				// together to get the total chance (Bayes rule). 
				// iI3 counts the number of successful first strikes
				//////

				for (iI3 = 0; iI3 < (iFirstStrikes + 1); iI3++)
				{
					// event: iI3 first strikes hit the defender

					// calculate chance of iI3 first strikes hitting: fOddsEvent
					// f(k;n,p)=C(n,k)*(p^k)*((1-p)^(n-k)) 
					// this needs to be in floating point math
					//////

					// UncutDragon
/* original code
					fOddsEvent = ((float)getBinomialCoefficient(iFirstStrikes, iI3)) * pow((((float)iAttackerOdds) / GC.getDefineINT("COMBAT_DIE_SIDES")), iI3) * pow((1.0f - (((float)iAttackerOdds) / GC.getDefineINT("COMBAT_DIE_SIDES"))), (iFirstStrikes - iI3));
*/					// modified
					fOddsEvent = ((float)getBinomialCoefficient(iFirstStrikes, iI3)) * pow((((float)iAttackerOdds) / GC.getCOMBAT_DIE_SIDES()), iI3) * pow((1.0f - (((float)iAttackerOdds) / GC.getCOMBAT_DIE_SIDES())), (iFirstStrikes - iI3));
					// /UncutDragon

					// calculate chance assuming iI3 first strike hits: fOddsAfterEvent
					//////

					if (iI3 >= iNeededRoundsAttacker)
					{
						fOddsAfterEvent = 1;
					}
					else
					{
						fOddsAfterEvent = 0;

						// odds for _at_least_ (iNeededRoundsAttacker - iI3) (the remaining hits 
						// the attacker needs to make) out of (iMaxRounds - iI3) (the left over 
						// rounds) is the sum of each _exact_ draw
						//////

						for (iI4 = (iNeededRoundsAttacker - iI3); iI4 < (iMaxRounds - iI3 + 1); iI4++)
						{
							// odds of exactly iI4 out of (iMaxRounds - iI3) draws.
							// f(k;n,p)=C(n,k)*(p^k)*((1-p)^(n-k)) 
							// this needs to be in floating point math
							//////

							// UncutDragon
/* original code
							fOddsAfterEvent += ((float)getBinomialCoefficient((iMaxRounds - iI3), iI4)) * pow((((float)iAttackerOdds) / GC.getDefineINT("COMBAT_DIE_SIDES")), iI4) * pow((1.0f - (((float)iAttackerOdds) / GC.getDefineINT("COMBAT_DIE_SIDES"))), ((iMaxRounds - iI3) - iI4));
*/							// modified
							fOddsAfterEvent += ((float)getBinomialCoefficient((iMaxRounds - iI3), iI4)) * pow((((float)iAttackerOdds) / GC.getCOMBAT_DIE_SIDES()), iI4) * pow((1.0f - (((float)iAttackerOdds) / GC.getCOMBAT_DIE_SIDES())), ((iMaxRounds - iI3) - iI4));
							// /UncutDragon
						}
					}

					// Multiply these together, round them properly, and add 
					// the result to the total iOdds
					//////

					iOdds += ((int)(1000.0 * (fOddsEvent*fOddsAfterEvent + 0.0005)));
				}
			}
			else // (iI < iJ)
			{
				// Attacker gets less first strikes than defender

				iFirstStrikes = iJ - iI;

				// For every possible first strike getting hit, calculate both
				// the chance of that event happening, as well as the rest of 
				// the chance assuming the event has happened. Multiply these 
				// together to get the total chance (Bayes rule). 
				// iI3 counts the number of successful first strikes
				//////

				for (iI3 = 0; iI3 < (iFirstStrikes + 1); iI3++)
				{
					// event: iI3 first strikes hit the defender

					// First of all, check if the attacker is still alive.
					// Otherwise, no further calculations need to occur 
					/////

					if (iI3 < iNeededRoundsDefender)
					{
						// calculate chance of iI3 first strikes hitting: fOddsEvent
						// f(k;n,p)=C(n,k)*(p^k)*((1-p)^(n-k)) 
						// this needs to be in floating point math
						//////

						// UncutDragon
/* original code
						fOddsEvent = ((float)getBinomialCoefficient(iFirstStrikes, iI3)) * pow((((float)iDefenderOdds) / GC.getDefineINT("COMBAT_DIE_SIDES")), iI3) * pow((1.0f - (((float)iDefenderOdds) / GC.getDefineINT("COMBAT_DIE_SIDES"))), (iFirstStrikes - iI3));
*/						// modified
						fOddsEvent = ((float)getBinomialCoefficient(iFirstStrikes, iI3)) * pow((((float)iDefenderOdds) / GC.getCOMBAT_DIE_SIDES()), iI3) * pow((1.0f - (((float)iDefenderOdds) / GC.getCOMBAT_DIE_SIDES())), (iFirstStrikes - iI3));
						// /UncutDragon

						// calculate chance assuming iI3 first strike hits: fOddsAfterEvent
						//////

						fOddsAfterEvent = 0;

						// odds for _at_least_ iNeededRoundsAttacker (the remaining hits 
						// the attacker needs to make) out of (iMaxRounds - iI3) (the left over 
						// rounds) is the sum of each _exact_ draw
						//////

						for (iI4 = iNeededRoundsAttacker; iI4 < (iMaxRounds - iI3 + 1); iI4++)
						{

							// odds of exactly iI4 out of (iMaxRounds - iI3) draws.
							// f(k;n,p)=C(n,k)*(p^k)*((1-p)^(n-k)) 
							// this needs to be in floating point math
							//////

							// UncutDragon
/* original code
							fOddsAfterEvent += ((float)getBinomialCoefficient((iMaxRounds - iI3), iI4)) * pow((((float)iAttackerOdds) / GC.getDefineINT("COMBAT_DIE_SIDES")), iI4) * pow((1.0f - (((float)iAttackerOdds) / GC.getDefineINT("COMBAT_DIE_SIDES"))), ((iMaxRounds - iI3) - iI4));
*/							// modified
							fOddsAfterEvent += ((float)getBinomialCoefficient((iMaxRounds - iI3), iI4)) * pow((((float)iAttackerOdds) / GC.getCOMBAT_DIE_SIDES()), iI4) * pow((1.0f - (((float)iAttackerOdds) / GC.getCOMBAT_DIE_SIDES())), ((iMaxRounds - iI3) - iI4));
							// /UncutDragon
						}

						// Multiply these together, round them properly, and add 
						// the result to the total iOdds
						//////

						iOdds += ((int)(1000.0 * (fOddsEvent*fOddsAfterEvent + 0.0005)));
					}
				}				
			}
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	// Weigh the total to the number of possible combinations of first strikes events
	// note: the integer math breaks down when #FS > 656 (with a die size of 1000)
	//////

	iOdds /= (((pDefender->immuneToFirstStrikes()) ? 0 : pAttacker->chanceFirstStrikes()) + 1) * (((pAttacker->immuneToFirstStrikes()) ? 0 : pDefender->chanceFirstStrikes()) + 1); 

	// finished!
	//////

	return iOdds;
}

/*************************************************************************************************/
/** ADVANCED COMBAT ODDS                      11/7/09                           PieceOfMind      */
/** BEGIN                                                                       v1.1             */
/*************************************************************************************************/

//Calculates the probability of a particular combat outcome
//Returns a float value (between 0 and 1)
//Written by PieceOfMind
//n_A = hits taken by attacker, n_D = hits taken by defender.
float getCombatOddsSpecific(CvUnit* pAttacker, CvUnit* pDefender, int n_A, int n_D)
{
    int iAttackerStrength;
    int iAttackerFirepower;
    int iDefenderStrength;
    int iDefenderFirepower;
    int iDefenderOdds;
    int iAttackerOdds;
    int iStrengthFactor;
    int iDamageToAttacker;
    int iDamageToDefender;
    int iNeededRoundsAttacker;
    //int iNeededRoundsDefender;

    int AttFSnet;
    int AttFSC;
    int DefFSC;

    int iDefenderHitLimit;


    iAttackerStrength = pAttacker->currCombatStr(NULL, NULL);
    iAttackerFirepower = pAttacker->currFirepower(NULL, NULL);
    iDefenderStrength = pDefender->currCombatStr(pDefender->plot(), pAttacker);
    iDefenderFirepower = pDefender->currFirepower(pDefender->plot(), pAttacker);

    iStrengthFactor = ((iAttackerFirepower + iDefenderFirepower + 1) / 2);
    iDamageToAttacker = std::max(1,((GC.getDefineINT("COMBAT_DAMAGE") * (iDefenderFirepower + iStrengthFactor)) / (iAttackerFirepower + iStrengthFactor)));
    iDamageToDefender = std::max(1,((GC.getDefineINT("COMBAT_DAMAGE") * (iAttackerFirepower + iStrengthFactor)) / (iDefenderFirepower + iStrengthFactor)));

    iDefenderOdds = ((GC.getDefineINT("COMBAT_DIE_SIDES") * iDefenderStrength) / (iAttackerStrength + iDefenderStrength));
    iAttackerOdds = GC.getDefineINT("COMBAT_DIE_SIDES") - iDefenderOdds;

    if (GC.getDefineINT("ACO_IgnoreBarbFreeWins")==0)
    {
        if (pDefender->isBarbarian())
        {
            //defender is barbarian
            if (!GET_PLAYER(pAttacker->getOwnerINLINE()).isBarbarian() && GET_PLAYER(pAttacker->getOwnerINLINE()).getWinsVsBarbs() < GC.getHandicapInfo(GET_PLAYER(pAttacker->getOwnerINLINE()).getHandicapType()).getFreeWinsVsBarbs())
            {
                //attacker is not barb and attacker player has free wins left
                //I have assumed in the following code only one of the units (attacker and defender) can be a barbarian

                iDefenderOdds = std::min((10 * GC.getDefineINT("COMBAT_DIE_SIDES")) / 100, iDefenderOdds);
                iAttackerOdds = std::max((90 * GC.getDefineINT("COMBAT_DIE_SIDES")) / 100, iAttackerOdds);
            }
        }
        else if (pAttacker->isBarbarian())
        {
            //attacker is barbarian
            if (!GET_PLAYER(pDefender->getOwnerINLINE()).isBarbarian() && GET_PLAYER(pDefender->getOwnerINLINE()).getWinsVsBarbs() < GC.getHandicapInfo(GET_PLAYER(pDefender->getOwnerINLINE()).getHandicapType()).getFreeWinsVsBarbs())
            {
                //defender is not barbarian and defender has free wins left and attacker is barbarian
                iAttackerOdds = std::min((10 * GC.getDefineINT("COMBAT_DIE_SIDES")) / 100, iAttackerOdds);
                iDefenderOdds = std::max((90 * GC.getDefineINT("COMBAT_DIE_SIDES")) / 100, iDefenderOdds);
            }
        }
    }

    iDefenderHitLimit = pDefender->maxHitPoints() - pAttacker->combatLimit();

    //iNeededRoundsAttacker = (std::max(0, pDefender->currHitPoints() - iDefenderHitLimit) + iDamageToDefender - (((pAttacker->combatLimit())==GC.getMAX_HIT_POINTS())?1:0) ) / iDamageToDefender;
    iNeededRoundsAttacker = (pDefender->currHitPoints() - pDefender->maxHitPoints() + pAttacker->combatLimit() - (((pAttacker->combatLimit())==pDefender->maxHitPoints())?1:0))/iDamageToDefender + 1;

    int N_D = (std::max(0, pDefender->currHitPoints() - iDefenderHitLimit) + iDamageToDefender - (((pAttacker->combatLimit())==GC.getMAX_HIT_POINTS())?1:0) ) / iDamageToDefender;

    //int N_A = (pAttacker->currHitPoints() + iDamageToAttacker - 1 ) / iDamageToAttacker;  //same as next line
    int N_A = (pAttacker->currHitPoints() - 1)/iDamageToAttacker + 1;


    //int iRetreatOdds = std::max((pAttacker->withdrawalProbability()),100);
    float RetreatOdds = ((float)(std::min((pAttacker->withdrawalProbability()),100)))/100.0f ;

    AttFSnet = ( (pDefender->immuneToFirstStrikes()) ? 0 : pAttacker->firstStrikes() ) - ((pAttacker->immuneToFirstStrikes()) ? 0 : pDefender->firstStrikes());
    AttFSC = (pDefender->immuneToFirstStrikes()) ? 0 : (pAttacker->chanceFirstStrikes());
    DefFSC = (pAttacker->immuneToFirstStrikes()) ? 0 : (pDefender->chanceFirstStrikes());


    float P_A = (float)iAttackerOdds / GC.getDefineINT("COMBAT_DIE_SIDES");
    float P_D = (float)iDefenderOdds / GC.getDefineINT("COMBAT_DIE_SIDES");
    float answer = 0.0f;
    if (n_A < N_A && n_D == iNeededRoundsAttacker)   // (1) Defender dies or is taken to combat limit
    {
        float sum1 = 0.0f;
        for (int i = (-AttFSnet-AttFSC<1?1:-AttFSnet-AttFSC); i <= DefFSC - AttFSnet; i++)
        {
            for (int j = 0; j <= i; j++)
            {

                if (n_A >= j)
                {
                    sum1 += (float)getBinomialCoefficient(i,j) * pow(P_A,(float)(i-j)) * getBinomialCoefficient(iNeededRoundsAttacker-1+n_A-j,iNeededRoundsAttacker-1);

                } //if
            }//for j
        }//for i
        sum1 *= pow(P_D,(float)n_A)*pow(P_A,(float)iNeededRoundsAttacker);
        answer += sum1;


        float sum2 = 0.0f;


        for (int i = (0<AttFSnet-DefFSC?AttFSnet-DefFSC:0); i <= AttFSnet + AttFSC; i++)
        {

            for (int j = 0; j <= i; j++)
            {
                if (N_D > j)
                {
                    sum2 = sum2 + getBinomialCoefficient(n_A+iNeededRoundsAttacker-j-1,n_A) * (float)getBinomialCoefficient(i,j) * pow(P_A,(float)iNeededRoundsAttacker) * pow(P_D,(float)(n_A+i-j));

                }
                else if (n_A == 0)
                {
                    sum2 = sum2 + (float)getBinomialCoefficient(i,j) * pow(P_A,(float)j) * pow(P_D,(float)(i-j));
                }
                else
                {
                    sum2 = sum2 + 0.0f;
                }
            }//for j

        }//for i
        answer += sum2;

    }
    else if (n_D < N_D && n_A == N_A)  // (2) Attacker dies!
    {

        float sum1 = 0.0f;
        for (int i = (-AttFSnet-AttFSC<1?1:-AttFSnet-AttFSC); i <= DefFSC - AttFSnet; i++)
        {

            for (int j = 0; j <= i; j++)
            {
                if (N_A>j)
                {
                    sum1 += getBinomialCoefficient(n_D+N_A-j-1,n_D) * (float)getBinomialCoefficient(i,j) * pow(P_D,(float)(N_A)) * pow(P_A,(float)(n_D+i-j));
                }
                else
                {
                    if (n_D == 0)
                    {
                        sum1 += (float)getBinomialCoefficient(i,j) * pow(P_D,(float)(j)) * pow(P_A,(float)(i-j));
                    }//if (inside if) else sum += 0
                }//if
            }//for j

        }//for i
        answer += sum1;
        float sum2 = 0.0f;
        for (int i = (0<AttFSnet-DefFSC?AttFSnet-DefFSC:0); i <= AttFSnet + AttFSC; i++)
        {
            for (int j = 0; j <= i; j++)
            {
                if (n_D >= j)
                {
                    sum2 += (float)getBinomialCoefficient(i,j) * pow(P_D,(float)(i-j)) * getBinomialCoefficient(N_A-1+n_D-j,N_A-1);
                } //if
            }//for j
        }//for i
        sum2 *= pow(P_A,(float)(n_D))*pow(P_D,(float)(N_A));
        answer += sum2;
        answer = answer * (1.0f - RetreatOdds);

    }
    else if (n_A == (N_A-1) && n_D < N_D)  // (3) Attacker retreats!
    {
        float sum1 = 0.0f;
        for (int i = (AttFSnet+AttFSC>-1?1:-AttFSnet-AttFSC); i <= DefFSC - AttFSnet; i++)
        {

            for (int j = 0; j <= i; j++)
            {
                if (N_A>j)
                {
                    sum1 += getBinomialCoefficient(n_D+N_A-j-1,n_D) * (float)getBinomialCoefficient(i,j) * pow(P_D,(float)(N_A)) * pow(P_A,(float)(n_D+i-j));
                }
                else
                {
                    if (n_D == 0)
                    {
                        sum1 += (float)getBinomialCoefficient(i,j) * pow(P_D,(float)(j)) * pow(P_A,(float)(i-j));
                    }//if (inside if) else sum += 0
                }//if
            }//for j

        }//for i
        answer += sum1;

        float sum2 = 0.0f;
        for (int i = (0<AttFSnet-DefFSC?AttFSnet-DefFSC:0); i <= AttFSnet + AttFSC; i++)
        {
            for (int j = 0; j <= i; j++)
            {
                if (n_D >= j)
                {
                    sum2 += (float)getBinomialCoefficient(i,j) * pow(P_D,(float)(i-j)) * getBinomialCoefficient(N_A-1+n_D-j,N_A-1);
                } //if
            }//for j
        }//for i
        sum2 *= pow(P_A,(float)(n_D))*pow(P_D,(float)(N_A));
        answer += sum2;
        answer = answer * RetreatOdds;//
    }
    else
    {
        //Unexpected value.  Process should not reach here.
    }

    answer = answer / ((float)(AttFSC+DefFSC+1)); // dividing by (t+w+1) as is necessary
    return answer;
}// getCombatOddsSpecific

// I had to add this function to the header file CvGameCoreUtils.h
/*************************************************************************************************/
/** ADVANCED COMBAT ODDS                      11/7/09                           PieceOfMind      */
/** END                                                                                          */
/*************************************************************************************************/

int getEspionageModifier(TeamTypes eOurTeam, TeamTypes eTargetTeam)
{
	FAssert(eOurTeam != eTargetTeam);
	FAssert(eOurTeam != BARBARIAN_TEAM);
	FAssert(eTargetTeam != BARBARIAN_TEAM);

	int iTargetPoints = GET_TEAM(eTargetTeam).getEspionagePointsEver();
	int iOurPoints = GET_TEAM(eOurTeam).getEspionagePointsEver();

	int iModifier = GC.getDefineINT("ESPIONAGE_SPENDING_MULTIPLIER") * (2 * iTargetPoints + iOurPoints);
	iModifier /= std::max(1, iTargetPoints + 2 * iOurPoints);
	return iModifier;
}

void setTradeItem(TradeData* pItem, TradeableItems eItemType, int iData)
{
	pItem->m_eItemType = eItemType;
	pItem->m_iData = iData;
	pItem->m_bOffering = false;
	pItem->m_bHidden = false;
}

bool isPlotEventTrigger(EventTriggerTypes eTrigger)
{
	CvEventTriggerInfo& kTrigger = GC.getEventTriggerInfo(eTrigger);

	if (kTrigger.getNumPlotsRequired() > 0)
	{
		if (kTrigger.getPlotType() != NO_PLOT)
		{
			return true;
		}

		if (kTrigger.getNumFeaturesRequired() > 0)
		{
			return true;
		}

		if (kTrigger.getNumTerrainsRequired() > 0)
		{
			return true;
		}

		if (kTrigger.getNumImprovementsRequired() > 0)
		{
			return true;
		}

		if (kTrigger.getNumBonusesRequired() > 0)
		{
			return true;
		}

		if (kTrigger.getNumRoutesRequired() > 0)
		{
			return true;
		}

		if (kTrigger.isUnitsOnPlot() && kTrigger.getNumUnitsRequired() > 0)
		{
			return true;
		}

		if (kTrigger.isPrereqEventCity() && !kTrigger.isPickCity())
		{
			return true;
		}
	}

	return false;
}

TechTypes getDiscoveryTech(UnitTypes eUnit, PlayerTypes ePlayer)
{
	TechTypes eBestTech = NO_TECH;
	int iBestValue = 0;

	for (int iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		if (GET_PLAYER(ePlayer).canResearch((TechTypes)iI))
		{
			int iValue = 0;

			for (int iJ = 0; iJ < GC.getNumFlavorTypes(); iJ++)
			{
				iValue += (GC.getTechInfo((TechTypes) iI).getFlavorValue(iJ) * GC.getUnitInfo(eUnit).getFlavorValue(iJ));
			}

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestTech = ((TechTypes)iI);
			}
		}
	}

	return eBestTech;
}


void setListHelp(wchar* szBuffer, const wchar* szStart, const wchar* szItem, const wchar* szSeparator, bool bFirst)
{
	if (bFirst)
	{
		wcscat(szBuffer, szStart);
	}
	else
	{
		wcscat(szBuffer, szSeparator);
	}

	wcscat(szBuffer, szItem);
}

void setListHelp(CvWString& szBuffer, const wchar* szStart, const wchar* szItem, const wchar* szSeparator, bool bFirst)
{
	if (bFirst)
	{
		szBuffer += szStart;
	}
	else
	{
		szBuffer += szSeparator;
	}

	szBuffer += szItem;
}

void setListHelp(CvWStringBuffer& szBuffer, const wchar* szStart, const wchar* szItem, const wchar* szSeparator, bool bFirst)
{
	if (bFirst)
	{
		szBuffer.append(szStart);
	}
	else
	{
		szBuffer.append(szSeparator);
	}

	szBuffer.append(szItem);
}

bool PUF_isGroupHead(const CvUnit* pUnit, int iData1, int iData2)
{
	return (pUnit->isGroupHead());
}

bool PUF_isPlayer(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	return (pUnit->getOwnerINLINE() == iData1);
}

bool PUF_isTeam(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	return (pUnit->getTeam() == iData1);
}

bool PUF_isCombatTeam(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	FAssertMsg(iData2 != -1, "Invalid data argument, should be >= 0");

	return (GET_PLAYER(pUnit->getCombatOwner((TeamTypes)iData2, pUnit->plot())).getTeam() == iData1 && !pUnit->isInvisible((TeamTypes)iData2, false, false));
}

bool PUF_isOtherPlayer(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	return (pUnit->getOwnerINLINE() != iData1);
}

bool PUF_isOtherTeam(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	TeamTypes eTeam = GET_PLAYER((PlayerTypes)iData1).getTeam();
	if (pUnit->canCoexistWithEnemyUnit(eTeam))
	{
		return false;
	}

	return (pUnit->getTeam() != eTeam);
}

bool PUF_isEnemy(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	FAssertMsg(iData2 != -1, "Invalid data argument, should be >= 0");

	TeamTypes eOtherTeam = GET_PLAYER((PlayerTypes)iData1).getTeam();
	TeamTypes eOurTeam = GET_PLAYER(pUnit->getCombatOwner(eOtherTeam, pUnit->plot())).getTeam();

	if (pUnit->canCoexistWithEnemyUnit(eOtherTeam))
	{
		return false;
	}

	return (iData2 ? eOtherTeam != eOurTeam : atWar(eOtherTeam, eOurTeam));
}

bool PUF_isNonAlly(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	FAssertMsg(iData2 != -1, "Invalid data argument, should be >= 0");

	TeamTypes eOtherTeam = GET_PLAYER((PlayerTypes)iData1).getTeam();
	TeamTypes eOurTeam = GET_PLAYER(pUnit->getCombatOwner(eOtherTeam, pUnit->plot())).getTeam();

	if (pUnit->canCoexistWithEnemyUnit(eOtherTeam))
	{
		return false;
	}

	return (iData2 ? eOtherTeam == eOurTeam : isNonAlly(eOtherTeam, eOurTeam));
}

bool PUF_isVisible(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	return !(pUnit->isInvisible(GET_PLAYER((PlayerTypes)iData1).getTeam(), false));
}

bool PUF_isVisibleDebug(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	return !(pUnit->isInvisible(GET_PLAYER((PlayerTypes)iData1).getTeam(), true));
}

bool PUF_canSiege(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	return pUnit->canSiege(GET_PLAYER((PlayerTypes)iData1).getTeam());
}

bool PUF_isPotentialEnemy(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	FAssertMsg(iData2 != -1, "Invalid data argument, should be >= 0");

	TeamTypes eOtherTeam = GET_PLAYER((PlayerTypes)iData1).getTeam();
	TeamTypes eOurTeam = GET_PLAYER(pUnit->getCombatOwner(eOtherTeam, pUnit->plot())).getTeam();

	if (pUnit->canCoexistWithEnemyUnit(eOtherTeam))
	{
		return false;
	}
	return (iData2 ? eOtherTeam != eOurTeam : isPotentialEnemy(eOtherTeam, eOurTeam));
}

bool PUF_canDeclareWar( const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	FAssertMsg(iData2 != -1, "Invalid data argument, should be >= 0");

	TeamTypes eOtherTeam = GET_PLAYER((PlayerTypes)iData1).getTeam();
	TeamTypes eOurTeam = GET_PLAYER(pUnit->getCombatOwner(eOtherTeam, pUnit->plot())).getTeam();

	if (pUnit->canCoexistWithEnemyUnit(eOtherTeam))
	{
		return false;
	}

	return (iData2 ? false : GET_TEAM(eOtherTeam).canDeclareWar(eOurTeam));
}

bool PUF_canDefend(const CvUnit* pUnit, int iData1, int iData2)
{
	return pUnit->canDefend();
}

bool PUF_cannotDefend(const CvUnit* pUnit, int iData1, int iData2)
{
	return !(pUnit->canDefend());
}

bool PUF_canDefendGroupHead(const CvUnit* pUnit, int iData1, int iData2)
{
	return (PUF_canDefend(pUnit, iData1, iData2) && PUF_isGroupHead(pUnit, iData1, iData2));
}

bool PUF_canDefendEnemy(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	FAssertMsg(iData2 != -1, "Invalid data argument, should be >= 0");
	return (PUF_canDefend(pUnit, iData1, iData2) && PUF_isEnemy(pUnit, iData1, iData2));
}

bool PUF_canDefendPotentialEnemy(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	return (PUF_canDefend(pUnit, iData1, iData2) && PUF_isPotentialEnemy(pUnit, iData1, iData2));
}

bool PUF_canAirAttack(const CvUnit* pUnit, int iData1, int iData2)
{
	return pUnit->canAirAttack();
}

bool PUF_canAirDefend(const CvUnit* pUnit, int iData1, int iData2)
{
	return pUnit->canAirDefend();
}

bool PUF_isFighting(const CvUnit* pUnit, int iData1, int iData2)
{
	return pUnit->isFighting();
}

bool PUF_isAnimal( const CvUnit* pUnit, int iData1, int iData2)
{
	return pUnit->isAnimal();
}

bool PUF_isMilitaryHappiness(const CvUnit* pUnit, int iData1, int iData2)
{
	return pUnit->isMilitaryHappiness();
}

bool PUF_isInvestigate(const CvUnit* pUnit, int iData1, int iData2)
{
	return pUnit->isInvestigate();
}

bool PUF_isCounterSpy(const CvUnit* pUnit, int iData1, int iData2)
{
	return pUnit->isCounterSpy();
}

bool PUF_isSpy(const CvUnit* pUnit, int iData1, int iData2)
{
	return pUnit->isSpy();
}

bool PUF_isDomainType(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	return (pUnit->getDomainType() == iData1);
}

bool PUF_isUnitType(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	return (pUnit->getUnitType() == iData1);
}

bool PUF_isUnitAIType(const CvUnit* pUnit, int iData1, int iData2)
{
	FAssertMsg(iData1 != -1, "Invalid data argument, should be >= 0");
	return (pUnit->AI_getUnitAIType() == iData1);
}

bool PUF_isCityAIType(const CvUnit* pUnit, int iData1, int iData2)
{
	return pUnit->AI_isCityAIType();
}

bool PUF_isNotCityAIType(const CvUnit* pUnit, int iData1, int iData2)
{
	return !(PUF_isCityAIType(pUnit, iData1, iData2));
}

bool PUF_isSelected(const CvUnit* pUnit, int iData1, int iData2)
{
	return pUnit->IsSelected();
}

bool PUF_makeInfoBarDirty(CvUnit* pUnit, int iData1, int iData2)
{
	pUnit->setInfoBarDirty(true);
	return true;
}

bool PUF_isNoMission(const CvUnit* pUnit, int iData1, int iData2)
{
	return (pUnit->getGroup()->getActivityType() != ACTIVITY_MISSION);
}

bool PUF_isMissionAI(const CvUnit* pUnit, int iData1, int iData2)
{
	return (pUnit->getGroup()->AI_getMissionAIType() == (MissionAITypes)iData1);
}

/********************************************************************************/
/* 	Worker Counting						03.08.2010				Fuyu			*/
/********************************************************************************/
bool PUF_isNoMissionAI(const CvUnit* pUnit, int iData1, int iData2)
{
	return (pUnit->getGroup()->AI_getMissionAIType() == NO_MISSIONAI);
}
/********************************************************************************/
/* 	Worker Counting												END 			*/
/********************************************************************************/

bool PUF_isFiniteRange(const CvUnit* pUnit, int iData1, int iData2)
{
	return ((pUnit->getDomainType() != DOMAIN_AIR) || (pUnit->getUnitInfo().getAirRange() > 0));
}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      01/15/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
bool PUF_isAvailableUnitAITypeGroupie(const CvUnit* pUnit, int iData1, int iData2)
{
	return ((PUF_isUnitAITypeGroupie(pUnit,iData1,iData2)) && !(pUnit->isCargo()));
}

bool PUF_isUnitAITypeGroupie(const CvUnit* pUnit, int iData1, int iData2)
{
	CvUnit* pGroupHead = pUnit->getGroup()->getHeadUnit();
	return (PUF_isUnitAIType(pGroupHead,iData1,iData2));
}

bool PUF_isFiniteRangeAndNotJustProduced(const CvUnit* pUnit, int iData1, int iData2)
{
	return (PUF_isFiniteRange(pUnit,iData1,iData2) && ((GC.getGameINLINE().getGameTurn() - pUnit->getGameTurnCreated()) > 1));
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

/************************************************************************************************/
/* Afforess	                  Start		 08/19/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
bool PUF_isMissionary(const CvUnit* pUnit, int /*ReligionTypes*/ iData1, int /*PlayerTypes*/ iData2)
{
	if (iData2 != -1)
	{
		if (pUnit->getOwner() != iData2)
		{
			return false;
		}
	}
	if (!pUnit->canSpread(pUnit->plot(), (ReligionTypes)iData1, false))
	{
		return false;
	}
	return true;
}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/

int potentialIrrigation(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	if (parent == NULL)
	{
		return TRUE;
	}

	return ((GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY)->isPotentialIrrigation()) ? TRUE : FALSE);
}


int checkFreshWater(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	if (data == ASNL_ADDCLOSED)
	{
		if (GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY)->isFreshWater())
		{
			*((bool *)pointer) = true;
		}
	}

	return 1;
}


int changeIrrigated(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder) 
{
	if (data == ASNL_ADDCLOSED)
	{
		GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY)->setIrrigated(*((bool *)pointer));
	}

	return 1;
}

int pathDestValid(int iToX, int iToY, const void* pointer, FAStar* finder)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode1;
	CLLNode<IDInfo>* pUnitNode2;
	CvSelectionGroup* pSelectionGroup;
	CvUnit* pLoopUnit1;
	CvUnit* pLoopUnit2;
	CvPlot* pToPlot;
	bool bAIControl;
	bool bValid;

	pToPlot = GC.getMapINLINE().plotSorenINLINE(iToX, iToY);
	FAssert(pToPlot != NULL);

	pSelectionGroup = ((CvSelectionGroup *)pointer);

	if (pSelectionGroup->atPlot(pToPlot))
	{
		return TRUE;
	}

	if (pSelectionGroup->getDomainType() == DOMAIN_IMMOBILE)
	{
		return FALSE;
	}

	bAIControl = pSelectionGroup->AI_isControlled();

	if (bAIControl)
	{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      11/04/09                                jdog5000      */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
		// BBAI efficiency: switch order, getPlotDanger is more expensive
		if (pSelectionGroup->getDomainType() == DOMAIN_LAND)
		{
			int iGroupAreaID = pSelectionGroup->getArea();
			if (pToPlot->getArea() != iGroupAreaID)
			{
				if( !(pSelectionGroup->canMoveAllTerrain()) )
				{
					if (!(pToPlot->isAdjacentToArea(iGroupAreaID)))
					{
						return FALSE;
					}
				}
			}
		}	

		if (!(gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_IGNORE_DANGER))
		{
			if (!(pSelectionGroup->canFight()) && !(pSelectionGroup->alwaysInvisible()))
			{
				//if (GET_PLAYER(pSelectionGroup->getHeadOwner()).AI_getPlotDanger(pToPlot) > 0)
				if (GET_PLAYER(pSelectionGroup->getHeadOwner()).AI_getAnyPlotDanger(pToPlot))
				{
					return FALSE;
				}
			}
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	}

	if (bAIControl || pToPlot->isRevealed(pSelectionGroup->getHeadTeam(), false))
	{
		if (pSelectionGroup->isAmphibPlot(pToPlot))
		{
			pUnitNode1 = pSelectionGroup->headUnitNode();

			while (pUnitNode1 != NULL)
			{
				pLoopUnit1 = ::getUnit(pUnitNode1->m_data);
				pUnitNode1 = pSelectionGroup->nextUnitNode(pUnitNode1);

				if ((pLoopUnit1->getCargo() > 0) && (pLoopUnit1->domainCargo() == DOMAIN_LAND))
				{
					bValid = false;

					pUnitNode2 = pLoopUnit1->plot()->headUnitNode();

					while (pUnitNode2 != NULL)
					{
						pLoopUnit2 = ::getUnit(pUnitNode2->m_data);
						pUnitNode2 = pLoopUnit1->plot()->nextUnitNode(pUnitNode2);

						if (pLoopUnit2->getTransportUnit() == pLoopUnit1)
						{
							if (pLoopUnit2->isGroupHead())
							{
								if (pLoopUnit2->getGroup()->canMoveOrAttackInto(pToPlot, (pSelectionGroup->AI_isDeclareWar(pToPlot) || (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_DECLARE_WAR))))
								{
									bValid = true;
									break;
								}
							}
						}
					}

					if (bValid)
					{
						return TRUE;
					}
				}
			}

			return FALSE;
		}
		else
		{
			if (!(pSelectionGroup->canMoveOrAttackInto(pToPlot, (pSelectionGroup->AI_isDeclareWar(pToPlot) || (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_DECLARE_WAR)))))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


int pathHeuristic(int iFromX, int iFromY, int iToX, int iToY)
{
	return (plotDistance(iFromX, iFromY, iToX, iToY) * PATH_MOVEMENT_WEIGHT);
}


int pathCost(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvSelectionGroup* pSelectionGroup;
	CvUnit* pLoopUnit;
	CvPlot* pFromPlot;
	CvPlot* pToPlot;
	int iWorstCost;
	int iCost;
	int iWorstMovesLeft;
	int iMovesLeft;
	int iWorstMax;
	int iMax;

	pFromPlot = GC.getMapINLINE().plotSorenINLINE(parent->m_iX, parent->m_iY);
	FAssert(pFromPlot != NULL);
	pToPlot = GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY);
	FAssert(pToPlot != NULL);

	pSelectionGroup = ((CvSelectionGroup *)pointer);

	iWorstCost = MAX_INT;
	iWorstMovesLeft = MAX_INT;
	iWorstMax = MAX_INT;

	int iWorstMovement = MAX_INT;
	int iLargestBaseCost = 0;
	int iLargestMax = 0;
	int iMaxTerrainDamage = 0;
	static bool bHasAlwaysHostileUnit;
	static bool bHasCanFightUnit;
	static bool bHasCanAttackUnit;
	static bool bHasDefensiveBonusUnit;
	static CvSelectionGroup* gLastSelectionGroup = NULL;
	bool bHasVisibleEnemy = false;
#define	MAX_UNIQUE_UNIT_CATEGORIES_CONSIDERED	3
	int	unitCharacteristics[MAX_UNIQUE_UNIT_CATEGORIES_CONSIDERED];
	int	movesLeft[MAX_UNIQUE_UNIT_CATEGORIES_CONSIDERED];
	int numUniqueUnitCategories = 0;

	if ( gLastSelectionGroup != pSelectionGroup )
	{
		bHasAlwaysHostileUnit = false;
		bHasCanFightUnit = false;
		bHasCanAttackUnit = false;
		bHasDefensiveBonusUnit = false;

		gLastSelectionGroup = pSelectionGroup;
	}

	pUnitNode = pSelectionGroup->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pSelectionGroup->nextUnitNode(pUnitNode);

		if (parent->m_iData1 > 0)
		{
			iMax = parent->m_iData1;
		}
		else
		{
			iMax = pLoopUnit->maxMoves();
		}

		int unitMovementCharacteristics = pLoopUnit->getMovementCharacteristicsHash();

		//	If we've already considred a unit with these characteristics no need to
		//	check this one too
		for(int iI = 0; iI < numUniqueUnitCategories; iI++)
		{
			if ( unitMovementCharacteristics == unitCharacteristics[iI] )
			{
				break;
			}
		}

		if ( iI == numUniqueUnitCategories )
		{
			if ( !bHasAlwaysHostileUnit && pLoopUnit->isAlwaysHostile(NULL) )
			{
				bHasAlwaysHostileUnit = true;
			}

			if ( !bHasCanFightUnit && pLoopUnit->canFight())
			{
				bHasCanFightUnit = true;
			}

			if ( bHasCanFightUnit && !bHasCanAttackUnit && pLoopUnit->canAttack())
			{
				bHasCanAttackUnit = true;
			}

			if ( bHasCanAttackUnit && !bHasVisibleEnemy && pToPlot->isVisibleEnemyDefender(pLoopUnit) )
			{
				bHasVisibleEnemy = true;
			}

			if ( !bHasDefensiveBonusUnit && !pLoopUnit->noDefensiveBonus() )
			{
				bHasDefensiveBonusUnit = true;
			}

			if ( pToPlot->getTerrainTurnDamage(pLoopUnit) > iMaxTerrainDamage )
			{
				iMaxTerrainDamage = pToPlot->getTerrainTurnDamage(pLoopUnit);
			}

			iCost = pToPlot->movementCost(pLoopUnit, pFromPlot);

			iMovesLeft = std::max(0, (iMax - iCost));

			if ( iMovesLeft < iWorstMovement )
			{
				iWorstMovement = iMovesLeft;
			}

			if ( iMax > iLargestMax )
			{
				iLargestMax = iMax;
			}

			if ( iCost > iLargestBaseCost )
			{
				iLargestBaseCost = iCost;
			}

			if ( numUniqueUnitCategories < MAX_UNIQUE_UNIT_CATEGORIES_CONSIDERED )
			{
				movesLeft[numUniqueUnitCategories] = iMax;
				unitCharacteristics[numUniqueUnitCategories++] = unitMovementCharacteristics;
			}
		}
		else if ( movesLeft[iI] > iMax )
		{
			iCost = pToPlot->movementCost(pLoopUnit, pFromPlot);

			iMovesLeft = std::max(0, (iMax - iCost));

			if ( iMovesLeft < iWorstMovement )
			{
				iWorstMovement = iMovesLeft;
			}

			movesLeft[iI] = iMovesLeft;
		}
	}

#define NEW_ALGORITHM
#ifdef NEW_ALGORITHM
	iCost = iLargestBaseCost;

	if ( iWorstMovement == 0)
	{
		PROFILE("pathCost.noMovement");

		iCost = (PATH_MOVEMENT_WEIGHT * iLargestMax);

		if (pToPlot->getTeam() != pSelectionGroup->getTeam())
		{
			iCost += PATH_TERRITORY_WEIGHT;
		}

		// Damage caused by features (mods)
		if (0 != GC.getPATH_DAMAGE_WEIGHT())
		{
			if (pToPlot->getFeatureType() != NO_FEATURE)
			{
				iCost += (GC.getPATH_DAMAGE_WEIGHT() * std::max(0, GC.getFeatureInfo(pToPlot->getFeatureType()).getTurnDamage())) / GC.getMAX_HIT_POINTS();
			}
/************************************************************************************************/
/* Afforess	                  Start		 05/17/10                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
			if (iMaxTerrainDamage > 0)
			{
				iCost += (GC.getPATH_DAMAGE_WEIGHT() * std::max(0, iMaxTerrainDamage * 2)) / GC.getMAX_HIT_POINTS();
			}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
			if (pToPlot->getExtraMovePathCost() > 0)
			{
				iCost += (PATH_MOVEMENT_WEIGHT * pToPlot->getExtraMovePathCost());
			}
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/03/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
		// Add additional cost for ending turn in or adjacent to enemy territory based on flags
		if (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_AVOID_ENEMY_WEIGHT_3)
		{
			if (pToPlot->isOwned() && ((GET_TEAM(pSelectionGroup->getHeadTeam()).AI_getWarPlan(pToPlot->getTeam()) != NO_WARPLAN) || (pToPlot->getTeam() != pSelectionGroup->getTeam() && bHasAlwaysHostileUnit)))
			{
				iCost *= 3;
			}
			else
			{
				CvPlot* pAdjacentPlot;
				int iI;
				for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
				{
					pAdjacentPlot = plotDirection(pToPlot->getX_INLINE(), pToPlot->getY_INLINE(), ((DirectionTypes)iI));

					if( pAdjacentPlot != NULL )
					{
						if (pAdjacentPlot->isOwned() && (atWar(pAdjacentPlot->getTeam(), pSelectionGroup->getHeadTeam()) || (pAdjacentPlot->getTeam() != pSelectionGroup->getTeam() && bHasAlwaysHostileUnit)))
						{
							iCost *= 3;
							iCost /= 2;
						}
					}
				}
			}
		}
		else if (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_AVOID_ENEMY_WEIGHT_2)
		{
			if (pToPlot->isOwned() && ((GET_TEAM(pSelectionGroup->getHeadTeam()).AI_getWarPlan(pToPlot->getTeam()) != NO_WARPLAN) || (pToPlot->getTeam() != pSelectionGroup->getTeam() && bHasAlwaysHostileUnit)))
			{
				iCost *= 2;
			}
			else
			{
				CvPlot* pAdjacentPlot;
				int iI;
				for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
				{
					pAdjacentPlot = plotDirection(pToPlot->getX_INLINE(), pToPlot->getY_INLINE(), ((DirectionTypes)iI));
					
					if( pAdjacentPlot != NULL )
					{
						if (pAdjacentPlot->isOwned() && (atWar(pAdjacentPlot->getTeam(), pSelectionGroup->getHeadTeam()) || (pAdjacentPlot->getTeam() != pSelectionGroup->getTeam() && bHasAlwaysHostileUnit)))
						{
							iCost *= 4;
							iCost /= 3;
						}
					}
				}
			}
		}
	}
	else
	{
		iCost = (PATH_MOVEMENT_WEIGHT * iCost);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/03/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
/*					// Add additional cost for movement in or adjacent to enemy territory based on flags
		if (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_AVOID_ENEMY_WEIGHT_3)
		{
			if (pToPlot->isOwned() && ((GET_TEAM(pSelectionGroup->getHeadTeam()).AI_getWarPlan(pToPlot->getTeam()) != NO_WARPLAN) || (pToPlot->getTeam() != pLoopUnit->getTeam() && pLoopUnit->isAlwaysHostile(pToPlot))))
			{
				iCost *= 2;
			}
			else
			{
				CvPlot* pAdjacentPlot;
				int iI;
				for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
				{
					pAdjacentPlot = plotDirection(pToPlot->getX_INLINE(), pToPlot->getY_INLINE(), ((DirectionTypes)iI));

					if( pAdjacentPlot != NULL )
					{
						if (pAdjacentPlot->isOwned() && (atWar(pAdjacentPlot->getTeam(), pSelectionGroup->getHeadTeam()) || (pAdjacentPlot->getTeam() != pLoopUnit->getTeam() && pLoopUnit->isAlwaysHostile(pAdjacentPlot))))
						{
							iCost *= 4;
							iCost /= 3;
						}
					}
				}
			}
		}
		else if (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_AVOID_ENEMY_WEIGHT_2)
		{
			if (pToPlot->isOwned() && ((GET_TEAM(pSelectionGroup->getHeadTeam()).AI_getWarPlan(pToPlot->getTeam()) != NO_WARPLAN) || (pToPlot->getTeam() != pLoopUnit->getTeam() && pLoopUnit->isAlwaysHostile(pToPlot))))
			{
				iCost *= 3;
				iCost /= 2;
			}
			else
			{
				CvPlot* pAdjacentPlot;
				int iI;
				for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
				{
					pAdjacentPlot = plotDirection(pToPlot->getX_INLINE(), pToPlot->getY_INLINE(), ((DirectionTypes)iI));
					
					if( pAdjacentPlot != NULL )
					{
						if (pAdjacentPlot->isOwned() && (atWar(pAdjacentPlot->getTeam(), pSelectionGroup->getHeadTeam()) || (pAdjacentPlot->getTeam() != pLoopUnit->getTeam() && pLoopUnit->isAlwaysHostile(pAdjacentPlot))))
						{
							iCost *= 6;
							iCost /= 5;
						}
					}
				}
			}
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	}

	if (bHasCanFightUnit)
	{
		PROFILE("pathCost.bHasCanFightUnit");

		if (iWorstMovement == 0)
		{
			iCost += (PATH_DEFENSE_WEIGHT * std::max(0, (200 - (!bHasDefensiveBonusUnit ? 0 : pToPlot->defenseModifier(pSelectionGroup->getTeam(), false)))));
		}

		if (pSelectionGroup->AI_isControlled())
		{
			if (bHasCanAttackUnit)
			{
				if (gDLL->getFAStarIFace()->IsPathDest(finder, pToPlot->getX_INLINE(), pToPlot->getY_INLINE()))
				{
					if (bHasVisibleEnemy)
					{
						iCost += (PATH_DEFENSE_WEIGHT * std::max(0, (200 - (!bHasDefensiveBonusUnit ? 0 : pFromPlot->defenseModifier(pSelectionGroup->getTeam(), false)))));

						if (!(pFromPlot->isCity()))
						{
							iCost += PATH_CITY_WEIGHT;
						}

						if (pFromPlot->isRiverCrossing(directionXY(pFromPlot, pToPlot)))
						{
							if (!(pLoopUnit->isRiver()))
							{
								iCost += (PATH_RIVER_WEIGHT * -(GC.getRIVER_ATTACK_MODIFIER()));
								iCost += (PATH_MOVEMENT_WEIGHT * iMovesLeft);
							}
						}
					}
				}
			}
		}
	}

	iWorstCost = iCost;
#else
	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pSelectionGroup->nextUnitNode(pUnitNode);
		FAssertMsg(pLoopUnit->getDomainType() != DOMAIN_AIR, "pLoopUnit->getDomainType() is not expected to be equal with DOMAIN_AIR");

		if (parent->m_iData1 > 0)
		{
			iMax = parent->m_iData1;
		}
		else
		{
			iMax = pLoopUnit->maxMoves();
		}

		iCost = pToPlot->movementCost(pLoopUnit, pFromPlot);

		iMovesLeft = std::max(0, (iMax - iCost));

		if (iMovesLeft <= iWorstMovesLeft)
		{
			if ((iMovesLeft < iWorstMovesLeft) || (iMax <= iWorstMax))
			{
				if (iMovesLeft == 0)
				{
					iCost = (PATH_MOVEMENT_WEIGHT * iMax);

					if (pToPlot->getTeam() != pLoopUnit->getTeam())
					{
						iCost += PATH_TERRITORY_WEIGHT;
					}

					// Damage caused by features (mods)
					if (0 != GC.getPATH_DAMAGE_WEIGHT())
					{
						if (pToPlot->getFeatureType() != NO_FEATURE)
						{
							iCost += (GC.getPATH_DAMAGE_WEIGHT() * std::max(0, GC.getFeatureInfo(pToPlot->getFeatureType()).getTurnDamage())) / GC.getMAX_HIT_POINTS();
						}
/************************************************************************************************/
/* Afforess	                  Start		 05/17/10                                                */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
						if (pToPlot->getTerrainTurnDamage(pLoopUnit) != 0)
						{
							iCost += (GC.getPATH_DAMAGE_WEIGHT() * std::max(0, pToPlot->getTerrainTurnDamage(pLoopUnit) * 2)) / GC.getMAX_HIT_POINTS();
						}
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
						if (pToPlot->getExtraMovePathCost() > 0)
						{
							iCost += (PATH_MOVEMENT_WEIGHT * pToPlot->getExtraMovePathCost());
						}
					}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/03/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
					// Add additional cost for ending turn in or adjacent to enemy territory based on flags
					if (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_AVOID_ENEMY_WEIGHT_3)
					{
						if (pToPlot->isOwned() && ((GET_TEAM(pSelectionGroup->getHeadTeam()).AI_getWarPlan(pToPlot->getTeam()) != NO_WARPLAN) || (pToPlot->getTeam() != pLoopUnit->getTeam() && pLoopUnit->isAlwaysHostile(pToPlot))))
						{
							iCost *= 3;
						}
						else
						{
							CvPlot* pAdjacentPlot;
							int iI;
							for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
							{
								pAdjacentPlot = plotDirection(pToPlot->getX_INLINE(), pToPlot->getY_INLINE(), ((DirectionTypes)iI));

								if( pAdjacentPlot != NULL )
								{
									if (pAdjacentPlot->isOwned() && (atWar(pAdjacentPlot->getTeam(), pSelectionGroup->getHeadTeam()) || (pAdjacentPlot->getTeam() != pLoopUnit->getTeam() && pLoopUnit->isAlwaysHostile(pAdjacentPlot))))
									{
										iCost *= 3;
										iCost /= 2;
									}
								}
							}
						}
					}
					else if (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_AVOID_ENEMY_WEIGHT_2)
					{
						if (pToPlot->isOwned() && ((GET_TEAM(pSelectionGroup->getHeadTeam()).AI_getWarPlan(pToPlot->getTeam()) != NO_WARPLAN) || (pToPlot->getTeam() != pLoopUnit->getTeam() && pLoopUnit->isAlwaysHostile(pToPlot))))
						{
							iCost *= 2;
						}
						else
						{
							CvPlot* pAdjacentPlot;
							int iI;
							for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
							{
								pAdjacentPlot = plotDirection(pToPlot->getX_INLINE(), pToPlot->getY_INLINE(), ((DirectionTypes)iI));
								
								if( pAdjacentPlot != NULL )
								{
									if (pAdjacentPlot->isOwned() && (atWar(pAdjacentPlot->getTeam(), pSelectionGroup->getHeadTeam()) || (pAdjacentPlot->getTeam() != pLoopUnit->getTeam() && pLoopUnit->isAlwaysHostile(pAdjacentPlot))))
									{
										iCost *= 4;
										iCost /= 3;
									}
								}
							}
						}
					}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
				}
				else
				{
					iCost = (PATH_MOVEMENT_WEIGHT * iCost);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      04/03/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
/*					// Add additional cost for movement in or adjacent to enemy territory based on flags
					if (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_AVOID_ENEMY_WEIGHT_3)
					{
						if (pToPlot->isOwned() && ((GET_TEAM(pSelectionGroup->getHeadTeam()).AI_getWarPlan(pToPlot->getTeam()) != NO_WARPLAN) || (pToPlot->getTeam() != pLoopUnit->getTeam() && pLoopUnit->isAlwaysHostile(pToPlot))))
						{
							iCost *= 2;
						}
						else
						{
							CvPlot* pAdjacentPlot;
							int iI;
							for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
							{
								pAdjacentPlot = plotDirection(pToPlot->getX_INLINE(), pToPlot->getY_INLINE(), ((DirectionTypes)iI));

								if( pAdjacentPlot != NULL )
								{
									if (pAdjacentPlot->isOwned() && (atWar(pAdjacentPlot->getTeam(), pSelectionGroup->getHeadTeam()) || (pAdjacentPlot->getTeam() != pLoopUnit->getTeam() && pLoopUnit->isAlwaysHostile(pAdjacentPlot))))
									{
										iCost *= 4;
										iCost /= 3;
									}
								}
							}
						}
					}
					else if (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_AVOID_ENEMY_WEIGHT_2)
					{
						if (pToPlot->isOwned() && ((GET_TEAM(pSelectionGroup->getHeadTeam()).AI_getWarPlan(pToPlot->getTeam()) != NO_WARPLAN) || (pToPlot->getTeam() != pLoopUnit->getTeam() && pLoopUnit->isAlwaysHostile(pToPlot))))
						{
							iCost *= 3;
							iCost /= 2;
						}
						else
						{
							CvPlot* pAdjacentPlot;
							int iI;
							for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
							{
								pAdjacentPlot = plotDirection(pToPlot->getX_INLINE(), pToPlot->getY_INLINE(), ((DirectionTypes)iI));
								
								if( pAdjacentPlot != NULL )
								{
									if (pAdjacentPlot->isOwned() && (atWar(pAdjacentPlot->getTeam(), pSelectionGroup->getHeadTeam()) || (pAdjacentPlot->getTeam() != pLoopUnit->getTeam() && pLoopUnit->isAlwaysHostile(pAdjacentPlot))))
									{
										iCost *= 6;
										iCost /= 5;
									}
								}
							}
						}
					}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
				}

				if (pLoopUnit->canFight())
				{
					if (iMovesLeft == 0)
					{
						iCost += (PATH_DEFENSE_WEIGHT * std::max(0, (200 - ((pLoopUnit->noDefensiveBonus()) ? 0 : pToPlot->defenseModifier(pLoopUnit->getTeam(), false)))));
					}

					if (pSelectionGroup->AI_isControlled())
					{
						if (pLoopUnit->canAttack())
						{
							if (gDLL->getFAStarIFace()->IsPathDest(finder, pToPlot->getX_INLINE(), pToPlot->getY_INLINE()))
							{
								if (pToPlot->isVisibleEnemyDefender(pLoopUnit))
								{
									iCost += (PATH_DEFENSE_WEIGHT * std::max(0, (200 - ((pLoopUnit->noDefensiveBonus()) ? 0 : pFromPlot->defenseModifier(pLoopUnit->getTeam(), false)))));

									if (!(pFromPlot->isCity()))
									{
										iCost += PATH_CITY_WEIGHT;
									}

									if (pFromPlot->isRiverCrossing(directionXY(pFromPlot, pToPlot)))
									{
										if (!(pLoopUnit->isRiver()))
										{
											iCost += (PATH_RIVER_WEIGHT * -(GC.getRIVER_ATTACK_MODIFIER()));
											iCost += (PATH_MOVEMENT_WEIGHT * iMovesLeft);
										}
									}
								}
							}
						}
					}
				}

				if (iCost < iWorstCost)
				{
					iWorstCost = iCost;
					iWorstMovesLeft = iMovesLeft;
					iWorstMax = iMax;
				}
			}
		}
	}
#endif

	FAssert(iWorstCost != MAX_INT);

	iWorstCost += PATH_STEP_WEIGHT;

	if ((pFromPlot->getX_INLINE() == pToPlot->getX_INLINE()) || (pFromPlot->getY_INLINE() == pToPlot->getY_INLINE()))
	{
		iWorstCost += PATH_STRAIGHT_WEIGHT;
	}

	FAssert(iWorstCost > 0);

	return iWorstCost;
}

int pathValidInternal(FAStarNode* parent, CvPlot* pFromPlot, int data, CvSelectionGroup* pSelectionGroup, FAStar* finder)
{
	bool bAIControl;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      03/03/10                                jdog5000      */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
	if (pSelectionGroup->atPlot(pFromPlot))
	{
		return TRUE;
	}

	if (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_SAFE_TERRITORY)
	{
		PROFILE("pathValid move save");

		if (pFromPlot->isOwned())
		{
			if (pFromPlot->getTeam() != pSelectionGroup->getHeadTeam())
			{
				return FALSE;
			}
		}

		if (!(pFromPlot->isRevealed(pSelectionGroup->getHeadTeam(), false)))
		{
			return FALSE;
		}
	}

	if (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_OUR_TERRITORY)
	{
		PROFILE("pathValid move save");

		if (!pFromPlot->isOwned())
		{
			return FALSE;
		}
		else if (pFromPlot->getTeam() != pSelectionGroup->getHeadTeam())
		{
			return FALSE;
		}

		if (!(pFromPlot->isRevealed(pSelectionGroup->getHeadTeam(), false)))
		{
			return FALSE;
		}
	}

	if (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_NO_ENEMY_TERRITORY)
	{
		PROFILE("pathValid no enemy");

		if (pFromPlot->isOwned())
		{
			if (atWar(pFromPlot->getTeam(), pSelectionGroup->getHeadTeam()))
			{
				return FALSE;
			}
		}
	}

	bAIControl = pSelectionGroup->AI_isControlled();

	if (bAIControl)
	{
		PROFILE("pathValid danger & invisible");
		if ((parent->m_iData2 > 1) || (parent->m_iData1 == 0))
		{
			if (!(gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_IGNORE_DANGER))
			{
				if (!(pSelectionGroup->canFight()) && !(pSelectionGroup->alwaysInvisible()))
				{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      08/20/09                                jdog5000      */
/*                                                                                              */
/* Unit AI, Efficiency                                                                          */
/************************************************************************************************/
					//if (GET_PLAYER(pSelectionGroup->getHeadOwner()).AI_getPlotDanger(pFromPlot) > 0)
					if (GET_PLAYER(pSelectionGroup->getHeadOwner()).AI_getAnyPlotDanger(pFromPlot))
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
					{
						return FALSE;
					}
				}
			}
		}
	}

	if (bAIControl || pFromPlot->isRevealed(pSelectionGroup->getHeadTeam(), false))
	{
		PROFILE("pathValid move through");

		if (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_THROUGH_ENEMY)
		{
			if (!(pSelectionGroup->canMoveOrAttackInto(pFromPlot)))
			{
				return FALSE;
			}
		}
		else
		{
			if (!(pSelectionGroup->canMoveThrough(pFromPlot)))
			{
				return FALSE;
			}
		}
	}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

	return TRUE;
}


int pathValid(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	PROFILE_FUNC();

	CvSelectionGroup* pSelectionGroup;
	CvPlot* pFromPlot;
	CvPlot* pToPlot;

	if (parent == NULL)
	{
		return TRUE;
	}

	pFromPlot = GC.getMapINLINE().plotSorenINLINE(parent->m_iX, parent->m_iY);
	FAssert(pFromPlot != NULL);
	pToPlot = GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY);
	FAssert(pToPlot != NULL);

	pSelectionGroup = ((CvSelectionGroup *)pointer);

	// XXX might want to take this out...
	if (pSelectionGroup->getDomainType() == DOMAIN_SEA)
	{
		PROFILE("pathValid domain sea");

#if 0
		//	Optimisation short-circuit some invalid pathing choices quickly
		if (!pToPlot->isWater() && !pSelectionGroup->canMoveAllTerrain() && !pToPlot->isCanMoveSeaUnits())
		{
			if (!pToPlot->isCity()) 
			{
				return FALSE;
			}
		}
#endif
		//	Can't cross diagonally across 'land'
		if (pFromPlot->isWater() && pToPlot->isWater())
		{
			if (!(GC.getMapINLINE().plotINLINE(pFromPlot->getX_INLINE(), pToPlot->getY_INLINE())->isWater()) && !(GC.getMapINLINE().plotINLINE(pToPlot->getX_INLINE(), pFromPlot->getY_INLINE())->isWater()))
			{
				if( !(pSelectionGroup->canMoveAllTerrain()) )
				{
					return FALSE;
				}
			}
		}
	}

	//	KOSHLING MOD - none of the rest of the calculation depends on pToPlot, 
	//	so we can cache the results from one request for each parent/pFromPlot.
	//	The cache is reset prior to each path generation
	bool bResult;
	if ( pSelectionGroup->HaveCachedPathValidityResult( pFromPlot, &bResult ) )
	{
		return bResult;
	}

	bResult = pathValidInternal(parent, pFromPlot, data, pSelectionGroup, finder);

	pSelectionGroup->CachePathValidityResult( pFromPlot, bResult );

	return bResult;
}


int pathAdd(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	PROFILE_FUNC();

	CvSelectionGroup* pSelectionGroup = ((CvSelectionGroup *)pointer);
	FAssert(pSelectionGroup->getNumUnits() > 0);

	int iTurns = 1;
	int iMoves = MAX_INT;

	if (data == ASNC_INITIALADD)
	{
		bool bMaxMoves = (gDLL->getFAStarIFace()->GetInfo(finder) & MOVE_MAX_MOVES);
		if (bMaxMoves)
		{
			iMoves = 0;
		}

		for (CLLNode<IDInfo>* pUnitNode = pSelectionGroup->headUnitNode(); pUnitNode != NULL; pUnitNode = pSelectionGroup->nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			if (bMaxMoves)
			{
				iMoves = std::max(iMoves, pLoopUnit->maxMoves());
			}
			else
			{
				iMoves = std::min(iMoves, pLoopUnit->movesLeft());
			}
		}
	}
	else
	{
		CvPlot* pFromPlot = GC.getMapINLINE().plotSorenINLINE(parent->m_iX, parent->m_iY);
		FAssertMsg(pFromPlot != NULL, "FromPlot is not assigned a valid value");
		CvPlot* pToPlot = GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY);
		FAssertMsg(pToPlot != NULL, "ToPlot is not assigned a valid value");

		int iStartMoves = parent->m_iData1;
		iTurns = parent->m_iData2;
		if (iStartMoves == 0)
		{
			iTurns++;
		}

		iMoves = pSelectionGroup->movesRemainingAfterMovingTo((iStartMoves == 0 ? -1 : iStartMoves), pFromPlot, pToPlot);
	}

	FAssertMsg(iMoves >= 0, "iMoves is expected to be non-negative (invalid Index)");

	node->m_iData1 = iMoves;
	node->m_iData2 = iTurns;

	return 1;
}


int stepDestValid(int iToX, int iToY, const void* pointer, FAStar* finder)
{
	PROFILE_FUNC();

	CvPlot* pFromPlot;
	CvPlot* pToPlot;

	pFromPlot = GC.getMapINLINE().plotSorenINLINE(gDLL->getFAStarIFace()->GetStartX(finder), gDLL->getFAStarIFace()->GetStartY(finder));
	FAssert(pFromPlot != NULL);
	pToPlot = GC.getMapINLINE().plotSorenINLINE(iToX, iToY);
	FAssert(pToPlot != NULL);

	if (pFromPlot->area() != pToPlot->area())
	{
		//	If the info int is not -1 it is the plot coord of a land plot to allow transition to
		//	from a water plot (used to find assault locations for transports)
		if ( (pFromPlot->getPlotType() == PLOT_OCEAN) != (pToPlot->getPlotType() == PLOT_OCEAN) )
		{
			int	iPlot = gDLL->getFAStarIFace()->GetInfo(finder);

			if ( iPlot != -1 )
			{
				CvPlot*	destPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);

				if ( destPlot == pToPlot )
				{
					return TRUE;
				}
			}
		}
		return FALSE;
	}

	return TRUE;
}


int stepHeuristic(int iFromX, int iFromY, int iToX, int iToY)
{
	return stepDistance(iFromX, iFromY, iToX, iToY);
}


int stepCost(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	return 1;
}


int stepValid(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	CvPlot* pNewPlot;

	if (parent == NULL)
	{
		return TRUE;
	}

	pNewPlot = GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY);

	if (pNewPlot->isImpassable())
	{
		return FALSE;
	}

/********************************************************************************/
/* 	BETTER_BTS_AI_MOD					12/12/08				jdog5000	*/
/* 																			*/
/* 	Bugfix																	*/
/********************************************************************************/
/* original BTS code
	if (GC.getMapINLINE().plotSorenINLINE(parent->m_iX, parent->m_iY)->area() != pNewPlot->area())
	{
		return FALSE;
	}
*/
	CvPlot* pFromPlot = GC.getMapINLINE().plotSorenINLINE(parent->m_iX, parent->m_iY);
	if (pFromPlot->area() != pNewPlot->area())
	{
		//	If the info int is not -1 it is the plot coord of a land plot to allow transition to
		//	from a water plot (used to find assault locations for transports)
		if ( (pFromPlot->getPlotType() == PLOT_OCEAN) != (pNewPlot->getPlotType() == PLOT_OCEAN) )
		{
			int	iPlot = gDLL->getFAStarIFace()->GetInfo(finder);

			if ( iPlot != -1 )
			{
				CvPlot*	destPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);

				if ( destPlot == pNewPlot || destPlot == pFromPlot )
				{
					return TRUE;
				}
			}
		}

		return FALSE;
	}

	// Don't count diagonal hops across land isthmus
	if (pFromPlot->isWater() && pNewPlot->isWater())
	{
		if (!(GC.getMapINLINE().plotINLINE(parent->m_iX, node->m_iY)->isWater()) && !(GC.getMapINLINE().plotINLINE(node->m_iX, parent->m_iY)->isWater()))
		{
			return FALSE;
		}
	}
/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						END								*/
/********************************************************************************/

	return TRUE;
}

/********************************************************************************/
/* 	BETTER_BTS_AI_MOD					02/02/09				jdog5000	*/
/* 																			*/
/* 																			*/
/********************************************************************************/
// Find paths that a team's units could follow without declaring war
int teamStepValid(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	CvPlot* pNewPlot;

	if (parent == NULL)
	{
		return TRUE;
	}

	pNewPlot = GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY);

	if (pNewPlot->isImpassable())
	{
		return FALSE;
	}

	CvPlot* pFromPlot = GC.getMapINLINE().plotSorenINLINE(parent->m_iX, parent->m_iY);
	if (pFromPlot->area() != pNewPlot->area())
	{
		return FALSE;
	}

	// Don't count diagonal hops across land isthmus
	if (pFromPlot->isWater() && pNewPlot->isWater())
	{
		if (!(GC.getMapINLINE().plotINLINE(parent->m_iX, node->m_iY)->isWater()) && !(GC.getMapINLINE().plotINLINE(node->m_iX, parent->m_iY)->isWater()))
		{
			return FALSE;
		}
	}

	TeamTypes ePlotTeam = pNewPlot->getTeam();
	std::vector<TeamTypes> teamVec = *((std::vector<TeamTypes> *)pointer);
	TeamTypes eTeam = teamVec[0];
	TeamTypes eTargetTeam = teamVec[1];
	CvTeamAI& kTeam = GET_TEAM(eTeam);

	if (ePlotTeam == NO_TEAM)
	{
		return TRUE;
	}

	if (ePlotTeam == eTargetTeam)
	{
		return TRUE;
	}

	if (kTeam.isFriendlyTerritory(ePlotTeam))
	{
		return TRUE;
	}

	if (kTeam.AI_getWarPlan(ePlotTeam) != NO_WARPLAN)
	{
		return TRUE;
	}

	if (kTeam.isOpenBorders(ePlotTeam))
	{
		return TRUE;
	}



	return FALSE;
}
/********************************************************************************/
/* 	BETTER_BTS_AI_MOD						END								*/
/********************************************************************************/

int stepAdd(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	if (data == ASNC_INITIALADD)
	{
		node->m_iData1 = 0;
	}
	else
	{
		node->m_iData1 = (parent->m_iData1 + 1);
	}

	FAssertMsg(node->m_iData1 >= 0, "node->m_iData1 is expected to be non-negative (invalid Index)");

	return 1;
}


int routeValid(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	CvPlot* pNewPlot;
	PlayerTypes ePlayer;

	if (parent == NULL)
	{
		return TRUE;
	}

	pNewPlot = GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY);

	ePlayer = ((PlayerTypes)(gDLL->getFAStarIFace()->GetInfo(finder)));

	if (!(pNewPlot->isOwned()) || (pNewPlot->getTeam() == GET_PLAYER(ePlayer).getTeam()))
	{
		if (pNewPlot->getRouteType() == GET_PLAYER(ePlayer).getBestRoute(pNewPlot))
		{
			return TRUE;
		}
	}

	return FALSE;
}


int borderValid(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	CvPlot* pNewPlot;
	PlayerTypes ePlayer;

	if (parent == NULL)
	{
		return TRUE;
	}

	pNewPlot = GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY);

	ePlayer = ((PlayerTypes)(gDLL->getFAStarIFace()->GetInfo(finder)));

	if (pNewPlot->getTeam() == GET_PLAYER(ePlayer).getTeam())
	{
		return TRUE;
	}

	return FALSE;
}


int areaValid(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	if (parent == NULL)
	{
		return TRUE;
	}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/02/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
// original BTS code
	return ((GC.getMapINLINE().plotSorenINLINE(parent->m_iX, parent->m_iY)->isWater() == GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY)->isWater()) ? TRUE : FALSE);

	// BBAI TODO: Why doesn't this work to break water and ice into separate area?
/*
	if( GC.getMapINLINE().plotSorenINLINE(parent->m_iX, parent->m_iY)->isWater() != GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY)->isWater() )
	{
		return FALSE;
	}

	// Ice blocks become their own area
	if( GC.getMapINLINE().plotSorenINLINE(parent->m_iX, parent->m_iY)->isWater() && GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY)->isWater() )
	{
		if( GC.getMapINLINE().plotSorenINLINE(parent->m_iX, parent->m_iY)->isImpassable() != GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY)->isImpassable() )
		{
			return FALSE;
		}
	}

	return TRUE;
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
}


int joinArea(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder) 
{
	if (data == ASNL_ADDCLOSED)
	{
		GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY)->setArea(gDLL->getFAStarIFace()->GetInfo(finder));
	}

	return 1;
}


int plotGroupValid(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	CvPlot* pOldPlot;
	CvPlot* pNewPlot;
	PlayerTypes ePlayer;

	if (parent == NULL)
	{
		return TRUE;
	}

	pOldPlot = GC.getMapINLINE().plotSorenINLINE(parent->m_iX, parent->m_iY);
	pNewPlot = GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY);

	ePlayer = ((PlayerTypes)(gDLL->getFAStarIFace()->GetInfo(finder)));
	TeamTypes eTeam = GET_PLAYER(ePlayer).getTeam();

	if (pOldPlot->getPlotGroup(ePlayer) == pNewPlot->getPlotGroup(ePlayer))
	{
		if (pNewPlot->isTradeNetwork(eTeam))
		{
			if (pNewPlot->isTradeNetworkConnected(pOldPlot, eTeam))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}


int countPlotGroup(FAStarNode* parent, FAStarNode* node, int data, const void* pointer, FAStar* finder)
{
	PROFILE_FUNC();

	if (data == ASNL_ADDCLOSED)
	{
		CvPlot* pPlot = GC.getMapINLINE().plotSorenINLINE(node->m_iX, node->m_iY);
		plotGroupCheckInfo* checkInfo = (plotGroupCheckInfo*)pointer;

		pPlot->m_groupGenerationNumber = checkInfo->groupGenerationNumber;
		checkInfo->hashInfo.allNodesHash ^= pPlot->getZobristContribution();
		if ( pPlot->isCity() || 
			 (pPlot->getImprovementType() != NO_IMPROVEMENT && pPlot->getBonusType() != NO_BONUS) )
		{
			checkInfo->hashInfo.resourceNodesHash ^= pPlot->getZobristContribution();
		}
	}

	return 1;
}


int baseYieldToSymbol(int iNumYieldTypes, int iYieldStack)
{
	int iReturn;	// holds the return value we will be calculating

	// get the base value for the iReturn value
	iReturn = iNumYieldTypes * GC.getDefineINT("MAX_YIELD_STACK");
	// then add the offset to the return value
	iReturn += iYieldStack;

	// return the value we have calculated
	return iReturn;
}


bool isPickableName(const TCHAR* szName)
{
	if (szName)
	{
		int iLen = _tcslen(szName);

		if (!_tcsicmp(&szName[iLen-6], "NOPICK"))
		{
			return false;
		}
	}

	return true;
}


// create an array of shuffled numbers
int* shuffle(int iNum, CvRandom& rand)
{
	int* piShuffle = new int[iNum];
	shuffleArray(piShuffle, iNum, rand);
	return piShuffle;
}


void shuffleArray(int* piShuffle, int iNum, CvRandom& rand)
{
	int iI, iJ;

	for (iI = 0; iI < iNum; iI++)
	{
		piShuffle[iI] = iI;
	}

	for (iI = 0; iI < iNum; iI++)
	{
		iJ = (rand.get(iNum - iI, NULL) + iI);

		if (iI != iJ)
		{
			int iTemp = piShuffle[iI];
			piShuffle[iI] = piShuffle[iJ];
			piShuffle[iJ] = iTemp;
		}
	}
}

int getTurnYearForGame(int iGameTurn, int iStartYear, CalendarTypes eCalendar, GameSpeedTypes eSpeed)
{
	return (getTurnMonthForGame(iGameTurn, iStartYear, eCalendar, eSpeed) / GC.getNumMonthInfos());
}


int getTurnMonthForGame(int iGameTurn, int iStartYear, CalendarTypes eCalendar, GameSpeedTypes eSpeed)
{
	int iTurnMonth;
	int iTurnCount;
	int iI;

	iTurnMonth = iStartYear * GC.getNumMonthInfos();

	switch (eCalendar)
	{
	case CALENDAR_DEFAULT:
		iTurnCount = 0;

		for (iI = 0; iI < GC.getGameSpeedInfo(eSpeed).getNumTurnIncrements(); iI++)
		{
			if (iGameTurn > (iTurnCount + GC.getGameSpeedInfo(eSpeed).getGameTurnInfo(iI).iNumGameTurnsPerIncrement))
			{
				iTurnMonth += (GC.getGameSpeedInfo(eSpeed).getGameTurnInfo(iI).iMonthIncrement * GC.getGameSpeedInfo(eSpeed).getGameTurnInfo(iI).iNumGameTurnsPerIncrement);
				iTurnCount += GC.getGameSpeedInfo(eSpeed).getGameTurnInfo(iI).iNumGameTurnsPerIncrement;
			}
			else
			{
				iTurnMonth += (GC.getGameSpeedInfo(eSpeed).getGameTurnInfo(iI).iMonthIncrement * (iGameTurn - iTurnCount));
				iTurnCount += (iGameTurn - iTurnCount);
				break;
			}
		}

		if (iGameTurn > iTurnCount)
		{
			iTurnMonth += (GC.getGameSpeedInfo(eSpeed).getGameTurnInfo(GC.getGameSpeedInfo(eSpeed).getNumTurnIncrements() - 1).iMonthIncrement * (iGameTurn - iTurnCount));
		}
		break;

	case CALENDAR_BI_YEARLY:
		iTurnMonth += (2 * iGameTurn * GC.getNumMonthInfos());
		break;

	case CALENDAR_YEARS:
	case CALENDAR_TURNS:
		iTurnMonth += iGameTurn * GC.getNumMonthInfos();
		break;

	case CALENDAR_SEASONS:
		iTurnMonth += (iGameTurn * GC.getNumMonthInfos()) / GC.getNumSeasonInfos();
		break;

	case CALENDAR_MONTHS:
		iTurnMonth += iGameTurn;
		break;

	case CALENDAR_WEEKS:
		iTurnMonth += iGameTurn / GC.getDefineINT("WEEKS_PER_MONTHS");
		break;

	default:
		FAssert(false);
	}

	return iTurnMonth;
}

// these string functions should only be used under chipotle cheat code (not internationalized)

void getDirectionTypeString(CvWString& szString, DirectionTypes eDirectionType)
{
	switch (eDirectionType)
	{
	case NO_DIRECTION: szString = L"NO_DIRECTION"; break;

	case DIRECTION_NORTH: szString = L"north"; break;
	case DIRECTION_NORTHEAST: szString = L"northeast"; break;
	case DIRECTION_EAST: szString = L"east"; break;
	case DIRECTION_SOUTHEAST: szString = L"southeast"; break;
	case DIRECTION_SOUTH: szString = L"south"; break;
	case DIRECTION_SOUTHWEST: szString = L"southwest"; break;
	case DIRECTION_WEST: szString = L"west"; break;
	case DIRECTION_NORTHWEST: szString = L"northwest"; break;

	default: szString = CvWString::format(L"UNKNOWN_DIRECTION(%d)", eDirectionType); break;
	}
}

void getCardinalDirectionTypeString(CvWString& szString, CardinalDirectionTypes eDirectionType)
{
	getDirectionTypeString(szString, cardinalDirectionToDirection(eDirectionType));
}

void getActivityTypeString(CvWString& szString, ActivityTypes eActivityType)
{
	switch (eActivityType)
	{
	case NO_ACTIVITY: szString = L"NO_ACTIVITY"; break;

	case ACTIVITY_AWAKE: szString = L"ACTIVITY_AWAKE"; break;
	case ACTIVITY_HOLD: szString = L"ACTIVITY_HOLD"; break;
	case ACTIVITY_SLEEP: szString = L"ACTIVITY_SLEEP"; break;
	case ACTIVITY_HEAL: szString = L"ACTIVITY_HEAL"; break;
	case ACTIVITY_SENTRY: szString = L"ACTIVITY_SENTRY"; break;
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
	case ACTIVITY_SENTRY_WHILE_HEAL: szString = L"ACTIVITY_SENTRY_WHILE_HEAL"; break;
	case ACTIVITY_SENTRY_NAVAL_UNITS: szString = L"ACTIVITY_SENTRY_NAVAL_UNITS"; break;
	case ACTIVITY_SENTRY_LAND_UNITS: szString = L"ACTIVITY_SENTRY_LAND_UNITS"; break;
#endif
// BUG - Sentry Actions - end
	case ACTIVITY_INTERCEPT: szString = L"ACTIVITY_INTERCEPT"; break;
	case ACTIVITY_MISSION: szString = L"ACTIVITY_MISSION"; break;

	default: szString = CvWString::format(L"UNKNOWN_ACTIVITY(%d)", eActivityType); break;
	}
}

void getMissionTypeString(CvWString& szString, MissionTypes eMissionType)
{
	switch (eMissionType)
	{
	case NO_MISSION: szString = L"NO_MISSION"; break;

	case MISSION_MOVE_TO: szString = L"MISSION_MOVE_TO"; break;
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
	case MISSION_MOVE_TO_SENTRY: szString = L"MISSION_MOVE_TO_SENTRY"; break;
#endif
// BUG - Sentry Actions - end
	case MISSION_ROUTE_TO: szString = L"MISSION_ROUTE_TO"; break;
	case MISSION_MOVE_TO_UNIT: szString = L"MISSION_MOVE_TO_UNIT"; break;
	case MISSION_SKIP: szString = L"MISSION_SKIP"; break;
	case MISSION_SLEEP: szString = L"MISSION_SLEEP"; break;
	case MISSION_FORTIFY: szString = L"MISSION_FORTIFY"; break;
	case MISSION_PLUNDER: szString = L"MISSION_PLUNDER"; break;
	case MISSION_AIRPATROL: szString = L"MISSION_AIRPATROL"; break;
	case MISSION_SEAPATROL: szString = L"MISSION_SEAPATROL"; break;
	case MISSION_HEAL: szString = L"MISSION_HEAL"; break;
	case MISSION_SENTRY: szString = L"MISSION_SENTRY"; break;
// BUG - Sentry Actions - start
#ifdef _MOD_SENTRY
	case MISSION_SENTRY_WHILE_HEAL: szString = L"MISSION_SENTRY_WHILE_HEAL"; break;
	case MISSION_SENTRY_NAVAL_UNITS: szString = L"MISSION_SENTRY_NAVAL_UNITS"; break;
	case MISSION_SENTRY_LAND_UNITS: szString = L"MISSION_SENTRY_LAND_UNITS"; break;
#endif
// BUG - Sentry Actions - end
	case MISSION_AIRLIFT: szString = L"MISSION_AIRLIFT"; break;
	case MISSION_NUKE: szString = L"MISSION_NUKE"; break;
	case MISSION_RECON: szString = L"MISSION_RECON"; break;
	case MISSION_PARADROP: szString = L"MISSION_PARADROP"; break;
	case MISSION_AIRBOMB: szString = L"MISSION_AIRBOMB"; break;
	case MISSION_BOMBARD: szString = L"MISSION_BOMBARD"; break;
	case MISSION_PILLAGE: szString = L"MISSION_PILLAGE"; break;
	case MISSION_SABOTAGE: szString = L"MISSION_SABOTAGE"; break;
	case MISSION_DESTROY: szString = L"MISSION_DESTROY"; break;
	case MISSION_STEAL_PLANS: szString = L"MISSION_STEAL_PLANS"; break;
	case MISSION_FOUND: szString = L"MISSION_FOUND"; break;
	case MISSION_SPREAD: szString = L"MISSION_SPREAD"; break;
	case MISSION_SPREAD_CORPORATION: szString = L"MISSION_SPREAD_CORPORATION"; break;
	case MISSION_JOIN: szString = L"MISSION_JOIN"; break;
	case MISSION_CONSTRUCT: szString = L"MISSION_CONSTRUCT"; break;
	case MISSION_DISCOVER: szString = L"MISSION_DISCOVER"; break;
	case MISSION_HURRY: szString = L"MISSION_HURRY"; break;
	case MISSION_TRADE: szString = L"MISSION_TRADE"; break;
	case MISSION_GREAT_WORK: szString = L"MISSION_GREAT_WORK"; break;
	case MISSION_INFILTRATE: szString = L"MISSION_INFILTRATE"; break;
	case MISSION_GOLDEN_AGE: szString = L"MISSION_GOLDEN_AGE"; break;
	case MISSION_BUILD: szString = L"MISSION_BUILD"; break;
	case MISSION_LEAD: szString = L"MISSION_LEAD"; break;
	case MISSION_ESPIONAGE: szString = L"MISSION_ESPIONAGE"; break;
	case MISSION_DIE_ANIMATION: szString = L"MISSION_DIE_ANIMATION"; break;

	case MISSION_BEGIN_COMBAT: szString = L"MISSION_BEGIN_COMBAT"; break;
	case MISSION_END_COMBAT: szString = L"MISSION_END_COMBAT"; break;
	case MISSION_AIRSTRIKE: szString = L"MISSION_AIRSTRIKE"; break;
	case MISSION_SURRENDER: szString = L"MISSION_SURRENDER"; break;
	case MISSION_CAPTURED: szString = L"MISSION_CAPTURED"; break;
	case MISSION_IDLE: szString = L"MISSION_IDLE"; break;
	case MISSION_DIE: szString = L"MISSION_DIE"; break;
	case MISSION_DAMAGE: szString = L"MISSION_DAMAGE"; break;
	case MISSION_MULTI_SELECT: szString = L"MISSION_MULTI_SELECT"; break;
	case MISSION_MULTI_DESELECT: szString = L"MISSION_MULTI_DESELECT"; break;
/************************************************************************************************/
/* Afforess	                  Start		 06/05/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	case MISSION_HURRY_FOOD: szString = L"MISSION_HURRY_FOOD"; break;
	case MISSION_INQUISITION: szString = L"MISSION_INQUISITION"; break;
	case MISSION_CLAIM_TERRITORY: szString = L"MISSION_CLAIM_TERRITORY";
	case MISSION_ESPIONAGE_SLEEP: szString = L"MISSION_ESPIONAGE_SLEEP"; break;
	case MISSION_GREAT_COMMANDER: szString = L"MISSION_GREAT_COMMANDER"; break;
	case MISSION_SHADOW: szString = L"MISSION_SHADOW"; break;
	case MISSION_WAIT_FOR_TECH: szString = L"MISSION_WAIT_FOR_TECH"; break;
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/
	default: szString = CvWString::format(L"UNKOWN_MISSION(%d)", eMissionType); break;
	}
}

void getMissionAIString(CvWString& szString, MissionAITypes eMissionAI)
{
	switch (eMissionAI)
	{
	case NO_MISSIONAI: szString = L"NO_MISSIONAI"; break;

	case MISSIONAI_SHADOW: szString = L"MISSIONAI_SHADOW"; break;
	case MISSIONAI_GROUP: szString = L"MISSIONAI_GROUP"; break;
	case MISSIONAI_LOAD_ASSAULT: szString = L"MISSIONAI_LOAD_ASSAULT"; break;
	case MISSIONAI_LOAD_SETTLER: szString = L"MISSIONAI_LOAD_SETTLER"; break;
	case MISSIONAI_LOAD_SPECIAL: szString = L"MISSIONAI_LOAD_SPECIAL"; break;
	case MISSIONAI_GUARD_CITY: szString = L"MISSIONAI_GUARD_CITY"; break;
	case MISSIONAI_GUARD_BONUS: szString = L"MISSIONAI_GUARD_BONUS"; break;
	case MISSIONAI_GUARD_SPY: szString = L"MISSIONAI_GUARD_SPY"; break;
	case MISSIONAI_ATTACK_SPY: szString = L"MISSIONAI_ATTACK_SPY"; break;
	case MISSIONAI_SPREAD: szString = L"MISSIONAI_SPREAD"; break;
	case MISSIONAI_CONSTRUCT: szString = L"MISSIONAI_CONSTRUCT"; break;
	case MISSIONAI_HURRY: szString = L"MISSIONAI_HURRY"; break;
	case MISSIONAI_GREAT_WORK: szString = L"MISSIONAI_GREAT_WORK"; break;
	case MISSIONAI_EXPLORE: szString = L"MISSIONAI_EXPLORE"; break;
	case MISSIONAI_BLOCKADE: szString = L"MISSIONAI_BLOCKADE"; break;
	case MISSIONAI_PILLAGE: szString = L"MISSIONAI_PILLAGE"; break;
	case MISSIONAI_FOUND: szString = L"MISSIONAI_FOUND"; break;
	case MISSIONAI_BUILD: szString = L"MISSIONAI_BUILD"; break;
	case MISSIONAI_ASSAULT: szString = L"MISSIONAI_ASSAULT"; break;
	case MISSIONAI_CARRIER: szString = L"MISSIONAI_CARRIER"; break;
	case MISSIONAI_PICKUP: szString = L"MISSIONAI_PICKUP"; break;
/************************************************************************************************/
/* Afforess                               12/7/09                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
	case MISSIONAI_CLAIM_TERRITORY: szString = L"MISSIONAI_CLAIM_TERRITORY"; break;
	case MISSIONAI_HURRY_FOOD: szString = L"MISSIONAI_HURRY_FOOD"; break;
	case MISSIONAI_INQUISITION: szString = L"MISSIONAI_INQUISITION"; break;
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	

	default: szString = CvWString::format(L"UNKOWN_MISSION_AI(%d)", eMissionAI); break;
	}
}

void getUnitAIString(CvWString& szString, UnitAITypes eUnitAI)
{
	// note, GC.getUnitAIInfo(eUnitAI).getDescription() is a international friendly way to get string (but it will be longer)
	
	switch (eUnitAI)
	{
	case NO_UNITAI: szString = L"no unitAI"; break;

	case UNITAI_UNKNOWN: szString = L"unknown"; break;
	case UNITAI_ANIMAL: szString = L"animal"; break;
	case UNITAI_SETTLE: szString = L"settle"; break;
	case UNITAI_WORKER: szString = L"worker"; break;
	case UNITAI_ATTACK: szString = L"attack"; break;
	case UNITAI_ATTACK_CITY: szString = L"attack city"; break;
	case UNITAI_COLLATERAL: szString = L"collateral"; break;
	case UNITAI_PILLAGE: szString = L"pillage"; break;
	case UNITAI_RESERVE: szString = L"reserve"; break;
	case UNITAI_COUNTER: szString = L"counter"; break;
	case UNITAI_CITY_DEFENSE: szString = L"city defense"; break;
	case UNITAI_CITY_COUNTER: szString = L"city counter"; break;
	case UNITAI_CITY_SPECIAL: szString = L"city special"; break;
	case UNITAI_EXPLORE: szString = L"explore"; break;
	case UNITAI_MISSIONARY: szString = L"missionary"; break;
	case UNITAI_PROPHET: szString = L"prophet"; break;
	case UNITAI_ARTIST: szString = L"artist"; break;
	case UNITAI_SCIENTIST: szString = L"scientist"; break;
	case UNITAI_GENERAL: szString = L"general"; break;
	case UNITAI_MERCHANT: szString = L"merchant"; break;
	case UNITAI_ENGINEER: szString = L"engineer"; break;
	case UNITAI_SPY: szString = L"spy"; break;
	case UNITAI_ICBM: szString = L"icbm"; break;
	case UNITAI_WORKER_SEA: szString = L"worker sea"; break;
	case UNITAI_ATTACK_SEA: szString = L"attack sea"; break;
	case UNITAI_RESERVE_SEA: szString = L"reserve sea"; break;
	case UNITAI_ESCORT_SEA: szString = L"escort sea"; break;
	case UNITAI_EXPLORE_SEA: szString = L"explore sea"; break;
	case UNITAI_ASSAULT_SEA: szString = L"assault sea"; break;
	case UNITAI_SETTLER_SEA: szString = L"settler sea"; break;
	case UNITAI_MISSIONARY_SEA: szString = L"missionary sea"; break;
	case UNITAI_SPY_SEA: szString = L"spy sea"; break;
	case UNITAI_CARRIER_SEA: szString = L"carrier sea"; break;
	case UNITAI_MISSILE_CARRIER_SEA: szString = L"missile carrier"; break;
	case UNITAI_PIRATE_SEA: szString = L"pirate sea"; break;
	case UNITAI_ATTACK_AIR: szString = L"attack air"; break;
	case UNITAI_DEFENSE_AIR: szString = L"defense air"; break;
	case UNITAI_CARRIER_AIR: szString = L"carrier air"; break;
	case UNITAI_PARADROP: szString = L"paradrop"; break;
	case UNITAI_ATTACK_CITY_LEMMING: szString = L"attack city lemming"; break;

	default: szString = CvWString::format(L"unknown(%d)", eUnitAI); break;
	}
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      02/21/10                                jdog5000      */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
// From Lead From Behind by UncutDragon
typedef std::vector<int> LFBoddsAttOdds;
typedef std::vector<LFBoddsAttOdds> LFBoddsDefRounds;
typedef std::vector<LFBoddsDefRounds> LFBoddsAttRounds;
typedef std::vector<LFBoddsAttRounds> LFBoddsFirstStrike;
int LFBlookupCombatOdds(int iFirstStrikes, int iNeededRoundsAttacker, int iNeededRoundsDefender, int iAttackerOdds);
int LFBlookupCombatOdds(LFBoddsFirstStrike* pOdds, int iFSIndex, int iFirstStrikes, int iNeededRoundsAttacker, int iNeededRoundsDefender, int iAttackerOdds);
int LFBlookupCombatOdds(LFBoddsAttOdds* pOdds, int iOddsIndex, int iFirstStrikes, int iNeededRoundsAttacker, int iNeededRoundsDefender, int iAttackerOdds);
int LFBcalculateCombatOdds(int iFirstStrikes, int iNeededRoundsAttacker, int iNeededRoundsDefender, int iAttackerOdds);

const int LFB_ODDS_INTERVAL_SIZE = 16;
const int LFB_ODDS_EXTRA_ACCURACY = 32;
LFBoddsFirstStrike* pOddsCacheFSPos = NULL;
LFBoddsFirstStrike* pOddsCacheFSNeg = NULL;

static LFBoddsFirstStrike* GetOddsCacheFSPos()
{
	if ( pOddsCacheFSPos == NULL )
	{
		pOddsCacheFSPos = new LFBoddsFirstStrike();
	}

	return pOddsCacheFSPos;
}

static LFBoddsFirstStrike* GetOddsCacheFSNeg()
{
	if ( pOddsCacheFSNeg == NULL )
	{
		pOddsCacheFSNeg = new LFBoddsFirstStrike();
	}

	return pOddsCacheFSNeg;
}

// gets the combat odds using precomputed attacker/defender values instead of unit pointers
int LFBgetCombatOdds(int iAttackerLowFS,	int iAttackerHighFS, int iDefenderLowFS, int iDefenderHighFS, int iNeededRoundsAttacker, int iNeededRoundsDefender, int iAttackerOdds)
{
	int iDefenderOdds;
	bool bFlip = false;
	int iFirstStrikes;
	int iI;
	int iJ;
	int iOdds = 0;

	// Essentially, this means we're attacking with a seige engine and the defender is already at or below the max combat limit
	// We're not allowed to attack regardless, since we can't do any damage - just return 100%
	if (iNeededRoundsAttacker == 0)
		return 1000;
	// Because the best defender code calls us from the defender's perspective, we also need to check 'defender' rounds zero
	if (iNeededRoundsDefender == 0)
		return 0;

	// If attacker has better than even chance to hit, we just flip it and calculate defender's chance to win
	// This reduces how much we cache considerably (by half just from the fact we're only dealing with half the odds
	// - but additionally, iNeededRounds'Defender' is guaranteed to stay low - at most 5 with standard settings).
	iDefenderOdds = GC.getCOMBAT_DIE_SIDES() - iAttackerOdds;	
	if (iAttackerOdds > iDefenderOdds)
		bFlip = true;

	// This is basically the two outside loops at the end of the standard getCombatOdds
	// We just call our cache lookup in the middle (flipped if necessary) instead of the actual computation
	for (iI = iAttackerLowFS; iI < iAttackerHighFS + 1; iI++)
	{
		for (iJ = iDefenderLowFS; iJ < iDefenderHighFS + 1; iJ++)
		{
			iFirstStrikes = iI - iJ;
			if (bFlip)
				iOdds += LFBlookupCombatOdds(-iFirstStrikes, iNeededRoundsDefender, iNeededRoundsAttacker, iDefenderOdds);
			else
				iOdds += LFBlookupCombatOdds(iFirstStrikes, iNeededRoundsAttacker, iNeededRoundsDefender, iAttackerOdds);
		}
	}

	// Odds are a straight average of all the FS combinations (since all are equally possible)
	iOdds /= ((iAttackerHighFS - iAttackerLowFS + 1) * (iDefenderHighFS - iDefenderLowFS + 1));

	// Now that we have the final odds, we can remove the extra accuracy, rounding off
	iOdds = (iOdds + (LFB_ODDS_EXTRA_ACCURACY/2)) / LFB_ODDS_EXTRA_ACCURACY;

	// If we flipped the perspective in the computation/lookup, need to flip it back now
	if (bFlip)
		iOdds = 1000 - iOdds;

	return iOdds;
}

// lookup the combat odds in the cache for a specific sub-result
int LFBlookupCombatOdds(int iFirstStrikes, int iNeededRoundsAttacker, int iNeededRoundsDefender, int iAttackerOdds)
{
	int iOdds = 0;

	// We actually maintain two caches - one for positive first strikes (plus zero), and one for negative
	// This just makes the indices (and growing the array as needed) easy
	if (iFirstStrikes < 0)
		iOdds = LFBlookupCombatOdds(GetOddsCacheFSNeg(), (-iFirstStrikes)-1, iFirstStrikes, iNeededRoundsAttacker, iNeededRoundsDefender, iAttackerOdds);
	else
		iOdds = LFBlookupCombatOdds(GetOddsCacheFSPos(), iFirstStrikes, iFirstStrikes, iNeededRoundsAttacker, iNeededRoundsDefender, iAttackerOdds);

	return iOdds;
}

int LFBlookupCombatOdds(LFBoddsFirstStrike* pOdds, int iFSIndex, int iFirstStrikes, int iNeededRoundsAttacker, int iNeededRoundsDefender, int iAttackerOdds)
{
	// Grow the arrays as needed
	// First dimension is the first strikes
	int iInsert = iFSIndex - (int)(*pOdds).size() + 1;
	if (iInsert > 0)
	{
		LFBoddsAttRounds pAdd;
		(*pOdds).insert((*pOdds).end(), iInsert, pAdd);
	}

	// Second dimension is the attacker rounds (starting at 1)
	LFBoddsAttRounds* pAttRounds = &((*pOdds)[iFSIndex]);
	iInsert = iNeededRoundsAttacker - (int)(*pAttRounds).size();
	if (iInsert > 0)
	{
		LFBoddsDefRounds pAdd;
		(*pAttRounds).insert((*pAttRounds).end(), iInsert, pAdd);
	}

	// Third dimension is the defender rounds (starting at 1)
	LFBoddsDefRounds* pDefRounds = &((*pAttRounds)[iNeededRoundsAttacker-1]);
	iInsert = iNeededRoundsDefender - (int)(*pDefRounds).size();
	if (iInsert > 0)
	{
		LFBoddsAttOdds pAdd;
		(*pDefRounds).insert((*pDefRounds).end(), iInsert, pAdd);
	}

	// Fourth (last) dimension is the odds index (odds/16)
	LFBoddsAttOdds* pAttOdds = &((*pDefRounds)[iNeededRoundsDefender-1]);

	// Round down to the nearest interval
	int iMinOddsIndex = iAttackerOdds / LFB_ODDS_INTERVAL_SIZE;
	int iMinOddsValue = iMinOddsIndex * LFB_ODDS_INTERVAL_SIZE;

	// Lookup the odds for the rounded down value
	int iOdds = LFBlookupCombatOdds(pAttOdds, iMinOddsIndex, iFirstStrikes, iNeededRoundsAttacker, iNeededRoundsDefender, iMinOddsValue);

	// If we happened to hit an interval exactly, we're done
	if (iMinOddsValue < iAttackerOdds)
	{
		// 'Round up' to the nearest interval - we don't actually need to compute it, we know
		// it's one more than the rounded down interval
		int iMaxOddsIndex = iMinOddsIndex+1;
		int iMaxOddsValue = iMinOddsValue+LFB_ODDS_INTERVAL_SIZE;

		// Lookup the odds for the rounded up value
		int iMaxOdds = LFBlookupCombatOdds(pAttOdds, iMaxOddsIndex, iFirstStrikes, iNeededRoundsAttacker, iNeededRoundsDefender, iMaxOddsValue);
		
		// Do a simple weighted average on the two odds
		iOdds += (((iAttackerOdds - iMinOddsValue) * (iMaxOdds - iOdds)) / LFB_ODDS_INTERVAL_SIZE);
	}

	return iOdds;
}

int LFBlookupCombatOdds(LFBoddsAttOdds* pOdds, int iOddsIndex, int iFirstStrikes, int iNeededRoundsAttacker, int iNeededRoundsDefender, int iAttackerOdds)
{
	int iNotComputed = -1;

	// Index 0 -> AttackerOdds 0 -> no chance to win
	if (iOddsIndex == 0)
		return 0;

	// We don't store all possible indices, just what we need/use
	// So use position 0 to keep track of what index we start with
	int iFirstIndex = iOddsIndex;
	if ((*pOdds).size() == 0)
		(*pOdds).push_back(iFirstIndex);
	else
		iFirstIndex = (*pOdds)[0];

	int iRealIndex = iOddsIndex - iFirstIndex + 1;

	// Index is before the start of our array
	int iInsert = -iRealIndex+1;
	if (iInsert > 0)
	{
		(*pOdds).insert((*pOdds).begin()+1, iInsert, iNotComputed);
		iFirstIndex -= iInsert;
		iRealIndex = 1;
		(*pOdds)[0] = iFirstIndex;
	}

	// Index is past the end of our array
	iInsert = iRealIndex - (int)(*pOdds).size() + 1;
	if (iInsert > 0)
		(*pOdds).insert((*pOdds).end(), iInsert, iNotComputed);

	// Retrieve the odds from the array
	int iOdds = (*pOdds)[iRealIndex];

	// Odds aren't cached yet - need to actually calculate them
	if (iOdds == iNotComputed)
	{
		iOdds = LFBcalculateCombatOdds(iFirstStrikes, iNeededRoundsAttacker, iNeededRoundsDefender, iAttackerOdds);
		(*pOdds)[iRealIndex] = iOdds;
	}

	return iOdds;
}

// Perform the actual odds calculation (basically identical to the default algorithm, except that we retain a little more accuracy)
int LFBcalculateCombatOdds(int iFirstStrikes, int iNeededRoundsAttacker, int iNeededRoundsDefender, int iAttackerOdds)
{
	float fOddsEvent;
	float fOddsAfterEvent;
	int iMaxRounds = iNeededRoundsAttacker + iNeededRoundsDefender - 1;
	int iOdds = 0;
	int iI3;
	int iI4;

	// This part is basically the inside of the outer two loops at the end of the standard getCombatOdds
	if (iFirstStrikes > 0)
	{
		// Attacker gets more or equal first strikes than defender

		// For every possible first strike getting hit, calculate both
		// the chance of that event happening, as well as the rest of 
		// the chance assuming the event has happened. Multiply these 
		// together to get the total chance (Bayes rule). 
		// iI3 counts the number of successful first strikes
		//////

		for (iI3 = 0; iI3 < (iFirstStrikes + 1); iI3++)
		{
			// event: iI3 first strikes hit the defender

			// calculate chance of iI3 first strikes hitting: fOddsEvent
			// f(k;n,p)=C(n,k)*(p^k)*((1-p)^(n-k)) 
			// this needs to be in floating point math
			//////

			fOddsEvent = ((float)getBinomialCoefficient(iFirstStrikes, iI3)) * std::pow((((float)iAttackerOdds) / GC.getCOMBAT_DIE_SIDES()), iI3) * std::pow((1.0f - (((float)iAttackerOdds) / GC.getCOMBAT_DIE_SIDES())), (iFirstStrikes - iI3));

			// calculate chance assuming iI3 first strike hits: fOddsAfterEvent
			//////

			if (iI3 >= iNeededRoundsAttacker)
			{
				fOddsAfterEvent = 1;
			}
			else
			{
				fOddsAfterEvent = 0;

				// odds for _at_least_ (iNeededRoundsAttacker - iI3) (the remaining hits 
				// the attacker needs to make) out of (iMaxRounds - iI3) (the left over 
				// rounds) is the sum of each _exact_ draw
				//////

				for (iI4 = (iNeededRoundsAttacker - iI3); iI4 < (iMaxRounds - iI3 + 1); iI4++)
				{
					// odds of exactly iI4 out of (iMaxRounds - iI3) draws.
					// f(k;n,p)=C(n,k)*(p^k)*((1-p)^(n-k)) 
					// this needs to be in floating point math
					//////

					fOddsAfterEvent += ((float)getBinomialCoefficient((iMaxRounds - iI3), iI4)) * std::pow((((float)iAttackerOdds) / GC.getCOMBAT_DIE_SIDES()), iI4) * std::pow((1.0f - (((float)iAttackerOdds) / GC.getCOMBAT_DIE_SIDES())), ((iMaxRounds - iI3) - iI4));
				}
			}

			// Multiply these together, round them properly, and add 
			// the result to the total iOdds
			//////

			iOdds += ((int)((1000.0 * fOddsEvent * fOddsAfterEvent * (float)LFB_ODDS_EXTRA_ACCURACY) + 0.5));
		}
	}
	else // (iI < iJ)
	{
		// Attacker gets less first strikes than defender
		int iDefenderOdds = GC.getCOMBAT_DIE_SIDES() - iAttackerOdds;
		iFirstStrikes *= -1;

		// For every possible first strike getting hit, calculate both
		// the chance of that event happening, as well as the rest of 
		// the chance assuming the event has happened. Multiply these 
		// together to get the total chance (Bayes rule). 
		// iI3 counts the number of successful first strikes
		//////

		for (iI3 = 0; iI3 < (iFirstStrikes + 1); iI3++)
		{
			// event: iI3 first strikes hit the defender

			// First of all, check if the attacker is still alive.
			// Otherwise, no further calculations need to occur 
			/////

			if (iI3 < iNeededRoundsDefender)
			{
				// calculate chance of iI3 first strikes hitting: fOddsEvent
				// f(k;n,p)=C(n,k)*(p^k)*((1-p)^(n-k)) 
				// this needs to be in floating point math
				//////

				fOddsEvent = ((float)getBinomialCoefficient(iFirstStrikes, iI3)) * std::pow((((float)iDefenderOdds) / GC.getCOMBAT_DIE_SIDES()), iI3) * std::pow((1.0f - (((float)iDefenderOdds) / GC.getCOMBAT_DIE_SIDES())), (iFirstStrikes - iI3));

				// calculate chance assuming iI3 first strike hits: fOddsAfterEvent
				//////

				fOddsAfterEvent = 0;

				// odds for _at_least_ iNeededRoundsAttacker (the remaining hits 
				// the attacker needs to make) out of (iMaxRounds - iI3) (the left over 
				// rounds) is the sum of each _exact_ draw
				//////

				for (iI4 = iNeededRoundsAttacker; iI4 < (iMaxRounds - iI3 + 1); iI4++)
				{

					// odds of exactly iI4 out of (iMaxRounds - iI3) draws.
					// f(k;n,p)=C(n,k)*(p^k)*((1-p)^(n-k)) 
					// this needs to be in floating point math
					//////

					fOddsAfterEvent += ((float)getBinomialCoefficient((iMaxRounds - iI3), iI4)) * std::pow((((float)iAttackerOdds) / GC.getCOMBAT_DIE_SIDES()), iI4) * std::pow((1.0f - (((float)iAttackerOdds) / GC.getCOMBAT_DIE_SIDES())), ((iMaxRounds - iI3) - iI4));
				}

				// Multiply these together, round them properly, and add 
				// the result to the total iOdds
				//////

				iOdds += ((int)((1000.0 * fOddsEvent * fOddsAfterEvent * (float)LFB_ODDS_EXTRA_ACCURACY)+0.5));
			}
		}				
	}

	return iOdds;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/


// BUG - Unit Experience - start
#include "CyArgsList.h"

/*
 * Calculates the experience needed to reach the next level after the given level.
 */
int calculateExperience(int iLevel, PlayerTypes ePlayer)
{
	FAssertMsg(ePlayer != NO_PLAYER, "ePlayer must be a valid player");
	FAssertMsg(iLevel > 0, "iLevel must be greater than zero");

	long lExperienceNeeded = 0;

	CyArgsList argsList;
	argsList.add(iLevel);
	argsList.add(ePlayer);

	PYTHON_CALL_FUNCTION4(__FUNCTION__, PYGameModule, "getExperienceNeeded", argsList.makeFunctionArgs(), &lExperienceNeeded);

	return (int)lExperienceNeeded;
}

/*
 * Calculates the level for a unit with the given experience.
 */
int calculateLevel(int iExperience, PlayerTypes ePlayer)
{
	FAssertMsg(ePlayer != NO_PLAYER, "ePlayer must be a valid player");

	if (iExperience <= 0)
	{
		return 1;
	}

	int iLevel = 1;
	while (true)
	{
		int iNextLevelExperience = calculateExperience(iLevel, ePlayer);
		if (iNextLevelExperience > iExperience)
		{
			break;
		}
		++iLevel;
		if (iNextLevelExperience == iExperience)
		{
			break;
		}
	}

	return iLevel;
}
// BUG - Unit Experience - end
/************************************************************************************************/
/* Afforess	                  Start		 06/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/

DirectionTypes getOppositeDirection(DirectionTypes eDirection)
{
	switch(eDirection)
	{
	case DIRECTION_NORTH:
		return DIRECTION_SOUTH;
	case DIRECTION_NORTHEAST:
		return DIRECTION_SOUTHWEST;
	case DIRECTION_EAST:
		return DIRECTION_WEST;
	case DIRECTION_SOUTHEAST:
		return DIRECTION_NORTHWEST;
	case DIRECTION_SOUTH:
		return DIRECTION_NORTH;
	case DIRECTION_SOUTHWEST:
		return DIRECTION_NORTHEAST;
	case DIRECTION_WEST:
		return DIRECTION_EAST;
	case DIRECTION_NORTHWEST:
		return DIRECTION_SOUTHEAST;
	default:
		return NO_DIRECTION;
	}
}

bool isAdjacentDirection(DirectionTypes eFacingDirection, DirectionTypes eOtherDirection)
{
	switch(eFacingDirection)
	{
	case DIRECTION_NORTH:
		return eOtherDirection == DIRECTION_NORTHEAST || eOtherDirection == DIRECTION_NORTHWEST;
	case DIRECTION_NORTHEAST:
		return eOtherDirection == DIRECTION_NORTH || eOtherDirection == DIRECTION_EAST;
	case DIRECTION_EAST:
		return eOtherDirection == DIRECTION_NORTHEAST || eOtherDirection == DIRECTION_SOUTHEAST;
	case DIRECTION_SOUTHEAST:
		return eOtherDirection == DIRECTION_EAST || eOtherDirection == DIRECTION_SOUTH;
	case DIRECTION_SOUTH:
		return eOtherDirection == DIRECTION_SOUTHEAST || eOtherDirection == DIRECTION_SOUTHWEST;
	case DIRECTION_SOUTHWEST:
		return eOtherDirection == DIRECTION_SOUTH || eOtherDirection == DIRECTION_WEST;
	case DIRECTION_WEST:
		return eOtherDirection == DIRECTION_SOUTHWEST || eOtherDirection == DIRECTION_NORTHWEST;
	case DIRECTION_NORTHWEST:
		return eOtherDirection == DIRECTION_NORTH || eOtherDirection == DIRECTION_WEST;
	}
	return false;
}
/************************************************************************************************/
/* Afforess	                         END                                                        */
/************************************************************************************************/	

