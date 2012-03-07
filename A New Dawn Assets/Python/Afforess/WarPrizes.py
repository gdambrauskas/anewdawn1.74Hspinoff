from CvPythonExtensions import *
import CvEventInterface
import CvUtil
import BugUtil
import PyHelpers
import BugCore

gc = CyGlobalContext()
localText = CyTranslator()
PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo
PyCity = PyHelpers.PyCity
PyGame = PyHelpers.PyGame
ANewDawnOpt = BugCore.game.RoMSettings

class WarPrizes:
	def __init__(self, eventManager):
	
		eventManager.addEventHandler("combatResult", self.onCombatResult)

	def onCombatResult(self, argsList):
		'Combat Result'
		pWinner,pLoser = argsList
		playerX = PyPlayer(pWinner.getOwner())
		unitX = PyInfo.UnitInfo(pWinner.getUnitType())
		playerY = PyPlayer(pLoser.getOwner())
		unitY = PyInfo.UnitInfo(pLoser.getUnitType())
		pPlayer = gc.getPlayer(pWinner.getOwner())
		pPlayerLoser = gc.getPlayer(pLoser.getOwner())
		if (ANewDawnOpt.isWarPrizes()):
			if not (gc.getPlayer(pWinner.getOwner()).isBarbarian()):
				if (unitX.getUnitCombatType() == gc.getInfoTypeForString("UNITCOMBAT_WOODEN_SHIPS")) or (unitX.getUnitCombatType() == gc.getInfoTypeForString("UNITCOMBAT_STEAM_SHIPS")) or (unitX.getUnitCombatType() == gc.getInfoTypeForString("UNITCOMBAT_DIESEL_SHIPS")) or (unitX.getUnitCombatType() == gc.getInfoTypeForString("UNITCOMBAT_NUCLEAR_SHIPS")):
					if (unitY.getUnitCombatType() == gc.getInfoTypeForString("UNITCOMBAT_WOODEN_SHIPS")) or (unitY.getUnitCombatType() == gc.getInfoTypeForString("UNITCOMBAT_STEAM_SHIPS")) or (unitY.getUnitCombatType() == gc.getInfoTypeForString("UNITCOMBAT_DIESEL_SHIPS")) or (unitY.getUnitCombatType() == gc.getInfoTypeForString("UNITCOMBAT_NUCLEAR_SHIPS")):
						if not (unitX.getUnitClassType() == gc.getInfoTypeForString("UNITCLASS_PRIVATEER")):
							if not (unitY.getUnitClassType() == gc.getInfoTypeForString("UNITCLASS_PRIVATEER")):
								if CyGame().getSorenRandNum(100, "Bob") <= 15:
									iUnit = pLoser.getUnitType()
									newUnit = pPlayer.initUnit(pLoser.getUnitType(), pWinner.getX(), pWinner.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.NO_DIRECTION)
									newUnit.finishMoves()
									newUnit.setDamage(50, pWinner.getOwner())
									CyInterface().addMessage(pWinner.getOwner(),false,20,BugUtil.getText("TXT_KEY_MISC_WARPRIZES_SUCCESS", gc.getUnitInfo(iUnit).getDescription()),'',0,gc.getUnitInfo(iUnit).getButton(),ColorTypes(gc.getInfoTypeForString("COLOR_GREEN")), pWinner.getX(), pWinner.getY(), True,True)
									CyInterface().addMessage(pLoser.getOwner(),false,20,BugUtil.getText("TXT_KEY_MISC_WARPRIZES_FAILURE",gc.getUnitInfo(iUnit).getDescription()),'',0,'Art/Interface/Buttons/General/warning_popup.dds',ColorTypes(gc.getInfoTypeForString("COLOR_RED")), pLoser.getX(), pLoser.getY(), True,True)
									if ((pLoser.getUnitClassType() == gc.getInfoTypeForString("UNITCLASS_GALLEY")) or (pLoser.getUnitClassType() == gc.getInfoTypeForString("UNITCLASS_FLUYT")) or (pLoser.getUnitClassType() == gc.getInfoTypeForString("UNITCLASS_GALLEON")) or (pLoser.getUnitClassType() == gc.getInfoTypeForString("UNITCLASS_TRANSPORT"))):
										Loot = CyGame().getSorenRandNum(100, "Loot")
										pPlayer.changeGold(Loot)
										if (pPlayerLoser.getGold() >= Loot):
											pPlayerLoser.changeGold(-Loot)
											CyInterface().addMessage(pLoser.getOwner(),false,20,BugUtil.getText("TXT_KEY_MISC_WARPRIZES_FAILURE_GOLD_LOST",gc.getUnitInfo(iUnit).getDescription()),'',0,'Art/Interface/Buttons/General/warning_popup.dds',ColorTypes(gc.getInfoTypeForString("COLOR_RED")), pLoser.getX(), pLoser.getY(), True,True)  
										CyInterface().addMessage(pWinner.getOwner(),false,20,BugUtil.getText("TXT_KEY_MISC_WARPRIZES_SUCCESS_GOLD_GAINED",gc.getUnitInfo(iUnit).getDescription() ),'',0,'Art/Interface/Buttons/process/processwealth.dds',ColorTypes(gc.getInfoTypeForString("COLOR_GREEN")), pWinner.getX(), pWinner.getY(), True,True)
## War Prize Modcomp END##