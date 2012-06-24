#include "CvGameCoreDLL.h"
#include "CyPlayer.h"
#include "CyUnit.h"
#include "CyCity.h"
#include "CyPlot.h"
#include "CySelectionGroup.h"
#include "CyArea.h"
//# include <boost/python/manage_new_object.hpp>
//# include <boost/python/return_value_policy.hpp>
//# include <boost/python/scope.hpp>

//
// published python interface for CyPlayer
//

void CyPlayerPythonInterface2(python::class_<CyPlayer>& x)
{
	OutputDebugString("Python Extension Module - CyPlayerPythonInterface2\n");

	// set the docstring of the current module scope 
	python::scope().attr("__doc__") = "Civilization IV Player Class"; 
	x
		.def("AI_updateFoundValues", &CyPlayer::AI_updateFoundValues, "void (bool bStartingLoc)")
		.def("AI_foundValue", &CyPlayer::AI_foundValue, "int (int, int, int, bool)")
		.def("AI_isFinancialTrouble", &CyPlayer::AI_isFinancialTrouble, "bool ()")
		.def("AI_demandRebukedWar", &CyPlayer::AI_demandRebukedWar, "bool (int /*PlayerTypes*/)")
		.def("AI_getAttitude", &CyPlayer::AI_getAttitude, "AttitudeTypes (int /*PlayerTypes*/) - Gets the attitude of the player towards the player passed in")
		.def("AI_unitValue", &CyPlayer::AI_unitValue, "int (int /*UnitTypes*/ eUnit, int /*UnitAITypes*/ eUnitAI, CyArea* pArea)")
		.def("AI_civicValue", &CyPlayer::AI_civicValue, "int (int /*CivicTypes*/ eCivic)")
		.def("AI_totalUnitAIs", &CyPlayer::AI_totalUnitAIs, "int (int /*UnitAITypes*/ eUnitAI)")
		.def("AI_totalAreaUnitAIs", &CyPlayer::AI_totalAreaUnitAIs, "int (CyArea* pArea, int /*UnitAITypes*/ eUnitAI)")
		.def("AI_totalWaterAreaUnitAIs", &CyPlayer::AI_totalWaterAreaUnitAIs, "int (CyArea* pArea, int /*UnitAITypes*/ eUnitAI)")
		.def("AI_getNumAIUnits", &CyPlayer::AI_getNumAIUnits, "int (UnitAIType) - Returns # of UnitAITypes the player current has of UnitAIType")
		.def("AI_getAttitudeExtra", &CyPlayer::AI_getAttitudeExtra, "int (int /*PlayerTypes*/ eIndex) - Returns the extra attitude for this player - usually scenario specific")
		.def("AI_setAttitudeExtra", &CyPlayer::AI_setAttitudeExtra, "void (int /*PlayerTypes*/ eIndex, int iNewValue) - Sets the extra attitude for this player - usually scenario specific")
		.def("AI_changeAttitudeExtra", &CyPlayer::AI_changeAttitudeExtra, "void (int /*PlayerTypes*/ eIndex, int iChange) - Changes the extra attitude for this player - usually scenario specific")
		.def("AI_getMemoryCount", &CyPlayer::AI_getMemoryCount, "int (/*PlayerTypes*/ eIndex1, /*MemoryTypes*/ eIndex2)")
		.def("AI_changeMemoryCount", &CyPlayer::AI_changeMemoryCount, "void (/*PlayerTypes*/ eIndex1, /*MemoryTypes*/ eIndex2, int iChange)")
		.def("AI_getExtraGoldTarget", &CyPlayer::AI_getExtraGoldTarget, "int ()")
		.def("AI_setExtraGoldTarget", &CyPlayer::AI_setExtraGoldTarget, "void (int)")
// BUG - Refuses to Talk - start
		.def("AI_isWillingToTalk", &CyPlayer::AI_isWillingToTalk, "bool (int /*PlayerTypes*/)")
// BUG - Refuses to Talk - end

		.def("getScoreHistory", &CyPlayer::getScoreHistory, "int (int iTurn)")
		.def("getEconomyHistory", &CyPlayer::getEconomyHistory, "int (int iTurn)")
		.def("getIndustryHistory", &CyPlayer::getIndustryHistory, "int (int iTurn)")
		.def("getAgricultureHistory", &CyPlayer::getAgricultureHistory, "int (int iTurn)")
		.def("getPowerHistory", &CyPlayer::getPowerHistory, "int (int iTurn)")
		.def("getCultureHistory", &CyPlayer::getCultureHistory, "int (int iTurn)")
		.def("getEspionageHistory", &CyPlayer::getEspionageHistory, "int (int iTurn)")

		.def("getScriptData", &CyPlayer::getScriptData, "str () - Get stored custom data (via pickle)")
		.def("setScriptData", &CyPlayer::setScriptData, "void (str) - Set stored custom data (via pickle)")

		.def("chooseTech", &CyPlayer::chooseTech, "void (int iDiscover, wstring szText, bool bFront)")

		.def("AI_maxGoldTrade", &CyPlayer::AI_maxGoldTrade, "int (int)")
		.def("AI_maxGoldPerTurnTrade", &CyPlayer::AI_maxGoldPerTurnTrade, "int (int)")

		.def("splitEmpire", &CyPlayer::splitEmpire, "bool (int iAreaId)")
		.def("canSplitEmpire", &CyPlayer::canSplitEmpire, "bool ()")
		.def("canSplitArea", &CyPlayer::canSplitArea, "bool (int)")
/************************************************************************************************/
/* REVOLUTION_MOD                         11/15/08                                jdog5000      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		.def("assimilatePlayer", &CyPlayer::assimilatePlayer, "bool ( int iPlayer ) - acquire iPlayer's units and cities")
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/
		.def("canHaveTradeRoutesWith", &CyPlayer::canHaveTradeRoutesWith, "bool (int)")
		.def("forcePeace", &CyPlayer::forcePeace, "void (int)")

/************************************************************************************************/
/* REVOLUTION_MOD                         06/11/08                                jdog5000      */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/

// BUG - Reminder Mod - start
		.def("addReminder", &CyPlayer::addReminder, "void (int iGameTurn, string szMessage)")
// BUG - Reminder Mod - end
		.def("setFoundedFirstCity", &CyPlayer::setFoundedFirstCity, "void (bool bNewValue)")
		.def("setAlive", &CyPlayer::setAlive, "void (bool bNewValue)")
		.def("setNewPlayerAlive", &CyPlayer::setNewPlayerAlive, "void (bool bNewValue) - like setAlive, but without firing turn logic")
		.def("changeTechScore", &CyPlayer::changeTechScore, "void (int iChange)" )
		.def("getStabilityIndex", &CyPlayer::getStabilityIndex, "int ( )")
		.def("setStabilityIndex", &CyPlayer::setStabilityIndex, "void ( int iNewValue )")
		.def("changeStabilityIndex", &CyPlayer::changeStabilityIndex, "void ( int iChange )")
		.def("getStabilityIndexAverage", &CyPlayer::getStabilityIndexAverage, "int ( )")
		.def("setStabilityIndexAverage", &CyPlayer::setStabilityIndexAverage, "void ( int iNewValue )")
		.def("updateStabilityIndexAverage", &CyPlayer::updateStabilityIndexAverage, "void ( )")																												// Exposed to Python
/************************************************************************************************/
/* REVOLUTION_MOD                          END                                                  */
/************************************************************************************************/

		// RevolutionDCM - revolution stability history
		.def("getRevolutionStabilityHistory", &CyPlayer::getRevolutionStabilityHistory, "int (int iTurn)")
/************************************************************************************************/
/* REVDCM                                 02/16/10                                phungus420    */
/*                                                                                              */
/* RevTrait Effects                                                                             */
/************************************************************************************************/
		.def("isNonStateReligionCommerce", &CyPlayer::isNonStateReligionCommerce, "bool ()")
		.def("isUpgradeAnywhere", &CyPlayer::isUpgradeAnywhere, "bool ()")
/************************************************************************************************/
/* REVDCM                                  END                                                  */
/************************************************************************************************/
/************************************************************************************************/
/* Afforess	                  Start		 04/15/10                                               */
/*                                                                                              */
/*                                                                                              */
/************************************************************************************************/
		.def("getDarkAgeTurns", &CyPlayer::getDarkAgeTurns, "int ()")
		.def("getDarkAgeLength", &CyPlayer::getDarkAgeLength, "int ()")
		.def("isDarkAge", &CyPlayer::isDarkAge, "bool ()")
		.def("changeDarkAgeTurns", &CyPlayer::changeDarkAgeTurns, "void (int iChange)")
		.def("isDarkAgeCapable", &CyPlayer::isDarkAgeCapable, "bool ()")
		.def("getDarkAgePointsFinal", &CyPlayer::getDarkAgePointsFinal, "int ()")
		.def("canHaveSlaves", &CyPlayer::canHaveSlaves, "bool ()")
		.def("hasValidBuildings", &CyPlayer::hasValidBuildings, "bool (int iTech)")
		.def("getBonusCommerceModifier", &CyPlayer::getBonusCommerceModifier, "int (int i, int j)")
		.def("isShowLandmarks", &CyPlayer::isShowLandmarks, "bool ()")
		.def("setShowLandmarks", &CyPlayer::setShowLandmarks, "void (bool bNewVal)")
		.def("getBuildingClassCountWithUpgrades", &CyPlayer::getBuildingClassCountWithUpgrades, "int (int iBuilding)")
		.def("setColor", &CyPlayer::setColor, "void (int iColor)")
		.def("setHandicap", &CyPlayer::setHandicap, "void (int iNewVal)")
		.def("isModderOption", &CyPlayer::isModderOption, "bool ()")
		.def("getModderOption", &CyPlayer::getModderOption, "bool ()")
		.def("setModderOption", &CyPlayer::setModderOption, "void ()")
		.def("doRevolution", &CyPlayer::doRevolution, "void (int (CivicTypes*) paeNewCivics, bool bForce)")
		.def("isAutomatedCanBuild", &CyPlayer::isAutomatedCanBuild, "bool ()")
		.def("setAutomatedCanBuild", &CyPlayer::setAutomatedCanBuild, "void ()")
		.def("setTeam", &CyPlayer::setTeam, "void ()")
/************************************************************************************************/
/* Afforess	                     END                                                            */
/************************************************************************************************/


		;
}
