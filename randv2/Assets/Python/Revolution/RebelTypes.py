# RebelTypes.py
#
# by jdog5000
# Version 1.1

# This file sets up the most likely rebel civ types to appear when a revolution occurs in a particular civ.

from CvPythonExtensions import *
import CvUtil

gc = CyGlobalContext()

# Initialize list to empty
RebelTypeList = list()

# This function actually sets up the lists of most preferable rebel types for each motherland civ type
# All rebel types in this list are equally likely
# If none of these are available, defaults to a similar art style civ
def setup( ) :

    #print "Setting up rebel type list"

    global RebelTypeList
    
    RebelTypeList = list()

    for idx in range(0,gc.getNumCivilizationInfos()) :
        RebelTypeList.append( list() )

    try :
        iAmerica        = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_AMERICA')
        iArabia         = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_ARABIA')
        iAztec          = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_AZTEC')
        iBabylon        = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_BABYLON')
        iByzantium      = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_BYZANTIUM')
        iCarthage       = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_CARTHAGE')
        iCelt           = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_CELT')
        iChina          = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_CHINA')
        iEgypt          = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_EGYPT')
        iEngland        = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_ENGLAND')
        iEthiopia        = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_ETHIOPIA')
        iFrance         = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_FRANCE')
        iGermany        = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_GERMANY')
        iGreece         = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_GREECE')
        iHolyRoman      = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_HOLY_ROMAN')
        iInca           = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_INCA')
        iIndia          = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_INDIA')
        iJapan          = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_JAPAN')
        iKhmer          = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_KHMER')
        iKorea          = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_KOREA')
        iMali           = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_MALI')
        iMaya           = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_MAYA')
        iMongol         = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_MONGOL')
        iNativeAmerica  = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_NATIVE_AMERICA')
        iNetherlands    = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_NETHERLANDS')
        iOttoman        = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_OTTOMAN')
        iPersia         = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_PERSIA')
        iPortugal       = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_PORTUGAL')
        iRome           = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_ROME')
        iRussia         = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_RUSSIA')
        iSpain          = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_SPAIN')
        iSumeria        = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_SUMERIA')
        iViking         = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_VIKING')
        iZulu           = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_ZULU')
# Rise of Mankind 2.5
       # iAbyssinia      = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_ABYSSINIANS')
        iAssyria        = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_ASSYRIA')
        iHittites       = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_HITTITES')
        iIroquois       = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_IROQUOIS')
        iSiam           = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_SIAM')


        # Format is:
        # RebelTypeList[iHomeland] = [iRebel1, iRebel2, iRebel3]
        # No limit on length of rebel list, can be zero

        RebelTypeList[iAmerica]     = [iEngland, iAztec, iNativeAmerica,iIroquois]
      #  RebelTypeList[iAbyssinia]   = [iEgypt,iMali,iZulu,iArabia,iEthiopia]
        RebelTypeList[iArabia]      = [iEgypt,iPersia,iOttoman,iBabylon,iSumeria,iAbyssinia,iAssyria]
        RebelTypeList[iAssyria]     = [iPersia,iBabylon,iSumeria,iHittites,iEgypt,iArabia]
        RebelTypeList[iAztec]       = [iInca,iSpain,iNativeAmerica,iMaya,iIroquois]
        RebelTypeList[iBabylon]     = [iSumeria,iPersia,iGreece,iEgypt,iArabia,iAssyria,iHittites]
        RebelTypeList[iByzantium]   = [iGreece,iRome,iOttoman,iHolyRoman,iHittites]
        RebelTypeList[iCarthage]    = [iRome,iGreece,iMali,iSpain]
        RebelTypeList[iCelt]        = [iFrance,iEngland,iGermany,iSpain]
        RebelTypeList[iChina]       = [iKorea,iMongol,iIndia,iJapan,iKhmer,iSiam]
        RebelTypeList[iEgypt]       = [iBabylon,iArabia,iPersia,iGreece,iEthiopia,iAssyria,iAbyssinia,iHittites]
        RebelTypeList[iEngland]     = [iAmerica,iIndia,iZulu,iNetherlands,iCelt]
        RebelTypeList[iEthiopia]    = [iEgypt,iMali,iZulu,iArabia,iAbyssinia]
        RebelTypeList[iFrance]      = [iGermany,iCelt,iEngland,iMali,iHolyRoman]
        RebelTypeList[iGermany]     = [iFrance,iRussia,iViking,iHolyRoman,iNetherlands]
        RebelTypeList[iGreece]      = [iRome,iPersia,iCarthage,iOttoman,iHittites]
        RebelTypeList[iHittites]    = [iAssyria,iEgypt,iPersia,iOttoman,iByzantium,iGreece,iBabylon]
        RebelTypeList[iHolyRoman]   = [iGermany,iFrance,iSpain,iByzantium]
        RebelTypeList[iInca]        = [iAztec,iSpain,iMaya,iNativeAmerica,iIroquois]
        RebelTypeList[iIndia]       = [iPersia,iSiam,iChina,iEngland,iKhmer]
        RebelTypeList[iIroquois]    = [iNativeAmerica,iAztec,iMaya,iInca,iAmerica]
        RebelTypeList[iJapan]       = [iKorea,iChina,iMongol,iKhmer,iSiam]
        RebelTypeList[iKhmer]       = [iSiam,iIndia,iChina,iMongol,iJapan]
        RebelTypeList[iKorea]       = [iJapan,iChina,iMongol,iKhmer]
        RebelTypeList[iMali]        = [iCarthage,iEgypt,iFrance,iZulu,iEthiopia,iAbyssinia]
        RebelTypeList[iMaya]        = [iAztec,iInca,iSpain,iNativeAmerica,iIroquois]
        RebelTypeList[iMongol]      = [iChina,iRussia,iPersia,iKorea,iSiam]
        RebelTypeList[iNativeAmerica]   = [iIroquois,iAztec,iMaya,iAmerica,iInca]
        RebelTypeList[iNetherlands] = [iPortugal,iGermany,iEngland,iAmerica]
        RebelTypeList[iOttoman]     = [iPersia,iGreece,iGermany,iArabia,iByzantium,iHittites]
        RebelTypeList[iPersia]      = [iOttoman,iIndia,iMongol,iGreece,iSumeria,iBabylon,iAssyria,iHittites]
        RebelTypeList[iPortugal]    = [iSpain,iFrance,iNetherlands]
        RebelTypeList[iRome]        = [iGreece,iCarthage,iCelt,iEgypt,iByzantium]
        RebelTypeList[iRussia]      = [iViking,iGermany,iMongol,iPersia]
        RebelTypeList[iSiam]        = [iKhmer,iIndia,iChina,iMongol,iJapan]
        RebelTypeList[iSpain]       = [iPortugal,iArabia,iAztec,iInca,iHolyRoman]
        RebelTypeList[iSumeria]     = [iBabylon,iOttoman,iGreece,iPersia,iAssyria]
        RebelTypeList[iViking]      = [iRussia,iGermany,iEngland,iAmerica]
        RebelTypeList[iZulu]        = [iMali,iArabia,iEgypt,iEthiopia,iAbyssinia]
# Rise of Mankind 2.5        
        #print "Completed rebel type list"

    except:
        print "Error!  Rebel types not found, no short lists available"
