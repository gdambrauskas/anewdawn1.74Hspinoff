########################################################################################################
##						zCivics						      ##
##												      ##
##	Created By:		Zebra 9								      ##
##	Created:		5 / 21 / 07							      ##
########################################################################################################

from CvPythonExtensions import *

# globals
gc = CyGlobalContext()

class zCivics:
	def __init__(self):
		self.civicInfo = dict()
		self.unitInfo = dict()
		self.buildingInfo = dict()
########################################################################################################
################			Add Your Code Below Here			################
########################################################################################################
		self.addUnitPre("UNIT_INQUISITOR", "CIVIC_INTOLERANT")
		self.addBuildingPre("BUILDING_SLAVE_MARKET", "CIVIC_SLAVERY")
		self.addBuildingPre("BUILDING_MANOR", "CIVIC_ARISTOCRACY")
		self.addBuildingPre("BUILDING_AGORA", "CIVIC_REPUBLIC")
		self.addBuildingPre("BUILDING_STATUE_OF_LIBERTY", "CIVIC_LIBERAL")
		##generalstaff Start
		self.addBuildingPre("BUILDING_DICTATOR_MONUMENT", "CIVIC_DESPOTISM")
		self.addBuildingPre("BUILDING_ROYAL_MONUMENT", "CIVIC_HEREDITARY_RULE")
		self.addBuildingPre("BUILDING_POLLING_PLACE", "CIVIC_DEMOCRACY")
		self.addBuildingPre("BUILDING_REEDUCATION_CENTER", "CIVIC_COMMUNIST")
		self.addBuildingPre("BUILDING_CHECKPOINT", "CIVIC_FASCIST")
		self.addBuildingPre("BUILDING_TOWN_HALL", "CIVIC_FEDERAL")
		self.addBuildingPre("BUILDING_VILLA", "CIVIC_PATRICIAN")
		self.addBuildingPre("BUILDING_POLIS_COUNCIL", "CIVIC_SENATE")
		self.addBuildingPre("BUILDING_WARLORD_KEEP", "CIVIC_VASSALAGE")
		self.addBuildingPre("BUILDING_CIVIL_SERVANT_SCHOOL", "CIVIC_BUREAUCRACY")
		self.addBuildingPre("BUILDING_SINGLE_ISSUE_PARTY", "CIVIC_PARLIAMENT")
		self.addBuildingPre("BUILDING_PRESIDENTIAL_MONUMENT", "CIVIC_PRESIDENT")
		self.addBuildingPre("BUILDING_BRAHMIN_LIBRARY", "CIVIC_CASTE_SYSTEM")
		self.addBuildingPre("BUILDING_ART_PATRONAGE", "CIVIC_BOURGEOIS")
		self.addBuildingPre("BUILDING_FESTIVAL", "CIVIC_PROLETARIAT")
		self.addBuildingPre("BUILDING_ESTATE", "CIVIC_FEUDAL")
		self.addBuildingPre("BUILDING_PARAMILITARY_CORPS", "CIVIC_NATIONALIST")
		self.addBuildingPre("BUILDING_DISSIDENT_PRESS", "CIVIC_LIBERAL")
		self.addBuildingPre("BUILDING_SOCIAL_JUSTICE_ORGANIZATION", "CIVIC_MARXIST")
		self.addBuildingPre("BUILDING_IMPERIAL_MINT", "CIVIC_MERCANTILISM")
		self.addBuildingPre("BUILDING_FREE_PORT", "CIVIC_LAISSEZ_FAIRE")
		self.addBuildingPre("BUILDING_PLANNING_OFFICE", "CIVIC_PLANNED")
		self.addBuildingPre("BUILDING_CHAIN_STORE", "CIVIC_CORPORATIST")
		self.addBuildingPre("BUILDING_HIGHWAY_PROJECT", "CIVIC_KEYNESIAN")
		self.addBuildingPre("BUILDING_WASTE_TO_ENERGY", "CIVIC_ENVIRONMENTALISM")
		self.addBuildingPre("BUILDING_MYSTIC_HUT", "CIVIC_PROPHETS")
		self.addBuildingPre("BUILDING_DIVINE_MONUMENT", "CIVIC_DIVINE_RULE")
		self.addBuildingPre("BUILDING_SEMINARY", "CIVIC_STATE_CHURCH")
		self.addBuildingPre("BUILDING_CHURCH_SCHOOL", "CIVIC_FREE_CHURCH")
		self.addBuildingPre("BUILDING_REVIVALIST_CHURCH", "CIVIC_INTOLERANT")
		self.addBuildingPre("BUILDING_UNIVERSAL_CHURCH", "CIVIC_SECULAR")
		self.addBuildingPre("BUILDING_CHURCH_OF_STATE", "CIVIC_ATHEIST")
		self.addBuildingPre("BUILDING_PLAGUE_HOSPITAL", "CIVIC_CHARITY")
		self.addBuildingPre("BUILDING_FOUNDLING_HOSPITAL", "CIVIC_CHURCH")
		self.addBuildingPre("BUILDING_WORKHOUSE", "CIVIC_PUBLIC_WORKS")
		self.addBuildingPre("BUILDING_FOUNDATION", "CIVIC_PRIVATE")
		self.addBuildingPre("BUILDING_CHAMBER_OF_COMMERCE", "CIVIC_CORPORATE")
		self.addBuildingPre("BUILDING_SOCIAL_SERVICES", "CIVIC_SUBSIDIZED")
		self.addBuildingPre("BUILDING_WELFARE_OFFICE", "CIVIC_SOCIALIZED")
		self.addBuildingPre("BUILDING_THINK_TANK", "CIVIC_SUPREMACY")
		self.addBuildingPre("BUILDING_AI_SURVEILLANCE", "CIVIC_TECHNOCRACY")
		self.addBuildingPre("BUILDING_BODY_EXCHANGE", "CIVIC_SUPERHUMAN")
		self.addBuildingPre("BUILDING_AUTOMATED_DEFENSES", "CIVIC_POST_SCARCITY")
		self.addBuildingPre("BUILDING_PARK_ARCOLOGY", "CIVIC_PARADISE")
		self.addBuildingPre("BUILDING_FORTIFIED_ENCAMPMENT", "CIVIC_CHIEFDOM")
		self.addBuildingPre("BUILDING_INTERROGATION_BUILDING", "CIVIC_OLIGARCHY")
		self.addBuildingPre("BUILDING_WARRIOR_HUT", "CIVIC_TRIBAL")
		self.addBuildingPre("BUILDING_CARAVAN_POST", "CIVIC_BARTER")
		self.addBuildingPre("BUILDING_IDOL_SHRINE", "CIVIC_FOLKLORE")
		self.addBuildingPre("BUILDING_MERCENARY_CAMP", "CIVIC_SURVIVAL")
		self.addUnitPre("UNIT_FUNDAMENTALIST_GUERRILLA", "CIVIC_INTOLERANT")
		self.addUnitPre("UNIT_SWAT", "CIVIC_SUPREMACY")
		self.addUnitPre("UNIT_IMPERIAL_GUARD", "CIVIC_HEREDITARY_RULE")
		self.addUnitPre("UNIT_GUARD_DU_CORPS", "CIVIC_HEREDITARY_RULE")
		self.addUnitPre("UNIT_MODERN_IMPERIAL_GUARD", "CIVIC_HEREDITARY_RULE")
		self.addUnitPre("UNIT_ROBOTIC_IMPERIAL_GUARD", "CIVIC_HEREDITARY_RULE")
		self.addUnitPre("UNIT_MASS_PRODUCED_ROBOT", "CIVIC_POST_SCARCITY")
		self.addUnitPre("UNIT_PARAMILITARY", "CIVIC_NATIONALIST")
		self.addUnitPre("UNIT_LEFTIST_GUERRILLA", "CIVIC_COMMUNIST")
		##generalstaff End
		##Afforess Start
		self.addBuildingPre("BUILDING_WATCH_TOWER", "CIVIC_BANDITS")
		self.addBuildingPre("BUILDING_DRAFT_OFFICE", "CIVIC_CONSCRIPTION1")
		self.addBuildingPre("BUILDING_SHANTYTOWN", "CIVIC_FUEDALISM")
		self.addBuildingPre("BUILDING_MARCHING_PAVILION", "CIVIC_STANDING_ARMY")
		self.addBuildingPre("BUILDING_RECRUITMENT_OFFICE", "CIVIC_VOLUNTEER_ARMY")
		self.addBuildingPre("BUILDING_ART_GALLERY", "CIVIC_PACIFISM")
		self.addBuildingPre("BUILDING_MISSILE_SILO", "CIVIC_MUTUALLY_ASSURED_DESTRUCTION")
		self.addBuildingPre("BUILDING_CONTROL_CENTER", "CIVIC_UNMANNED_WARFARE")
		##Afforess End
		
########################################################################################################
################			Change Nothing Below Here			################
########################################################################################################

	def addCivicPre(self, szCivic, szReligion):
		if self.civicInfo.has_key(szCivic):
			self.civicInfo[szCivic] = (szReligion, self.civicInfo[szCivic][1])
		else:
			self.civicInfo[szCivic] = (szReligion, False)

	def addUnitPre(self, szUnit, szCivic):
		self.unitInfo[szUnit] = szCivic

	def addBuildingPre(self, szBuilding, szCivic):
		self.buildingInfo[szBuilding] = szCivic

	def addLockCivic(self, szCivic, bNewValue):
		if self.civicInfo.has_key(szCivic):
			self.civicInfo[szCivic] = (self.civicInfo[szCivic][0], bNewValue)
		else:
			self.civicInfo[szCivic] = ("", bNewValue)

	def cannotDoCivic(self, iPlayer, iCivic):
		civicType = gc.getCivicInfo(iCivic).getType()
		if self.civicInfo.has_key(civicType):
			if self.civicInfo[civicType][0] != "":
				if gc.getPlayer(iPlayer).getStateReligion() != gc.getInfoTypeForString(self.civicInfo[civicType][0]):
					return True

		pPlayer = gc.getPlayer(iPlayer)
		civicOptType = gc.getCivicInfo(iCivic).getCivicOptionType()
		testCivic = gc.getCivicInfo(pPlayer.getCivics(civicOptType)).getType()
		if self.civicInfo.has_key(testCivic) and testCivic != civicType:
			if self.civicInfo[testCivic][1]:
				return True

		return False

	def cannotTrainUnit(self, iPlayer, iUnit):
		unitType = gc.getUnitInfo(iUnit).getType()
		if self.unitInfo.has_key(unitType):
			if not gc.getPlayer(iPlayer).isCivic(gc.getInfoTypeForString(self.unitInfo[unitType])):
				return True
		return False

	def cannotConstructBuilding(self, iPlayer, iBuilding):
		buildingType = gc.getBuildingInfo(iBuilding).getType()
		if self.buildingInfo.has_key(buildingType):
			if not gc.getPlayer(iPlayer).isCivic(gc.getInfoTypeForString(self.buildingInfo[buildingType])):
				return True
		return False