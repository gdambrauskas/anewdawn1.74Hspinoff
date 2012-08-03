// plotGroup.cpp

#include "CvGameCoreDLL.h"
#include "CvPlotGroup.h"
#include "CvPlot.h"
#include "cvGlobals.h"
#include "CvPlayerAI.h"
#include "CvMap.h"
#include "CvCity.h"
#include "CvDLLFAStarIFaceBase.h"
#include "FProfiler.h"

//#define VALIDATION_FOR_PLOT_GROUPS

int CvPlotGroup::m_allocationSeqForSession = 0;

// Public Functions...

CvPlotGroup::CvPlotGroup()
{
	m_paiNumBonuses = NULL;
	m_paiNumFreeTradeRegionBuildings = NULL;
	m_numCities = -1;

	reset(0, NO_PLAYER, true);
}


CvPlotGroup::~CvPlotGroup()
{
	uninit();
}


void CvPlotGroup::init(int iID, PlayerTypes eOwner, CvPlot* pPlot, bool bRecalculateBonuses)
{
	//--------------------------------
	// Init saved data
	reset(iID, eOwner);

	//--------------------------------
	// Init non-saved data
	m_zobristHashes.allNodesHash = 0;
	m_zobristHashes.resourceNodesHash = 0;

	//--------------------------------
	// Init other game data
	addPlot(pPlot, bRecalculateBonuses);
}


void CvPlotGroup::uninit()
{
	SAFE_DELETE_ARRAY(m_paiNumBonuses);
	SAFE_DELETE_ARRAY(m_paiNumFreeTradeRegionBuildings);
}

// FUNCTION: reset()
// Initializes data members that are serialized.
void CvPlotGroup::reset(int iID, PlayerTypes eOwner, bool bConstructorCall)
{
	//--------------------------------
	// Uninit class
	uninit();

	m_iID = iID;
	m_eOwner = eOwner;
	m_numCities = -1;
	m_numPlots = 0;
	m_sessionAllocSeq = m_allocationSeqForSession++;

	if (!bConstructorCall)
	{
		SAFE_DELETE_ARRAY(m_paiNumBonuses);
		SAFE_DELETE_ARRAY(m_paiNumFreeTradeRegionBuildings);
	}
}


void CvPlotGroup::addPlot(CvPlot* pPlot, bool bRecalculateBonuses)
{
	PROFILE_FUNC();

	pPlot->setPlotGroup(getOwnerINLINE(), this, bRecalculateBonuses);

	//	Add the zobrist contribution of this plot to the hash
	m_zobristHashes.allNodesHash ^= pPlot->getZobristContribution();
	if ( pPlot->isCity() || 
		 (pPlot->getImprovementType() != NO_IMPROVEMENT && pPlot->getBonusType() != NO_BONUS) )
	{
		m_zobristHashes.resourceNodesHash ^= pPlot->getZobristContribution();
	}

	if ( m_numCities != -1 && pPlot->isCity() && pPlot->getPlotCity()->getOwnerINLINE() == getOwnerINLINE())
	{
		m_numCities++;
	}

	m_numPlots++;

	if ( m_seedPlotX == -1 )
	{
		m_seedPlotX = pPlot->getX_INLINE();
		m_seedPlotY = pPlot->getY_INLINE();
	}
}


void CvPlotGroup::removePlot(CvPlot* pPlot, bool bRecalculateBonuses)
{
	PROFILE_FUNC();

	FAssert(pPlot->getPlotGroup(m_eOwner) == this);
	if ( pPlot->getPlotGroup(m_eOwner) == this )
	{
		pPlot->setPlotGroup(getOwnerINLINE(), NULL, bRecalculateBonuses);

		if ( --m_numPlots == 0 )
		{
			GET_PLAYER(getOwnerINLINE()).deletePlotGroup(getID());
		}
		else
		{
			if ( m_numCities != -1 && pPlot->isCity() && pPlot->getPlotCity()->getOwnerINLINE() == getOwnerINLINE() )
			{
				m_numCities--;
			}

			//	Remove the zobrist contribution of this plot to the hash
			m_zobristHashes.allNodesHash ^= pPlot->getZobristContribution();
			if ( pPlot->isCity() || 
				 (pPlot->getImprovementType() != NO_IMPROVEMENT && pPlot->getBonusType() != NO_BONUS) )
			{
				m_zobristHashes.resourceNodesHash ^= pPlot->getZobristContribution();
			}
		}
	}
}

#ifdef _DEBUG
void CvPlotGroup::Validate(void)
{
#if 0
	CLLNode<XYCoords>* pPlotNode;

	pPlotNode = headPlotsNode();

	while (pPlotNode != NULL)
	{
		CvPlot* pPlot = GC.getMapINLINE().plotSorenINLINE(pPlotNode->m_data.iX, pPlotNode->m_data.iY);

		FAssert(pPlot->getPlotGroup(m_eOwner) == this);
		pPlotNode = nextPlotsNode(pPlotNode);
	}
#endif
}
#endif

CvPlot* CvPlotGroup::getRepresentativePlot(void) const
{
	CvPlot*	result = NULL;

	if ( m_seedPlotX != -1 && m_seedPlotY != -1 )
	{
		result = GC.getMapINLINE().plotSorenINLINE(m_seedPlotX, m_seedPlotY);

		if ( result != NULL && result->getPlotGroup(m_eOwner) != this )
		{
			result = NULL;
		}
	}

	if ( result == NULL )
	{
		for(int iI = 0; iI < GC.getMapINLINE().getGridWidthINLINE(); iI++)
		{
			for(int iJ = 0; iJ < GC.getMapINLINE().getGridHeightINLINE(); iJ++)
			{
				result = GC.getMapINLINE().plotSorenINLINE(iI, iJ);

				if ( result->getPlotGroup(m_eOwner) == this )
				{
					m_seedPlotX = iI;
					m_seedPlotY = iJ;

					return result;
				}
			}
		}

		return NULL;
	}

	return result;
}

typedef struct buildRemovedPlotListParams
{
	CLinkList<XYCoords> removedPlots;
	int					groupGenNumber;
} buildRemovedPlotListParams;

static bool buildRemovedPlotList(CvPlotGroup* onBehalfOf, CvPlot* pLoopPlot, void* params)
{
	buildRemovedPlotListParams* parm = (buildRemovedPlotListParams*)params;

	//	If the plot was not visited in the generational check above it is no longer
	//	part of this plot group
	if ( pLoopPlot->m_groupGenerationNumber != parm->groupGenNumber)
	{
		XYCoords xy;

		xy.iX = pLoopPlot->getX_INLINE();
		xy.iY = pLoopPlot->getY_INLINE();

		parm->removedPlots.insertAtEnd(xy);
	}

	return true;
}

typedef struct buildAllPlotListParams
{
	CLinkList<XYCoords> allPlots;
} buildAllPlotListParams;

static bool buildAllPlotList(CvPlotGroup* onBehalfOf, CvPlot* pLoopPlot, void* params)
{
	buildAllPlotListParams* parm = (buildAllPlotListParams*)params;

	XYCoords xy;

	xy.iX = pLoopPlot->getX_INLINE();
	xy.iY = pLoopPlot->getY_INLINE();

	parm->allPlots.insertAtEnd(xy);

	//OutputDebugString(CvString::format("Enumerated plot: (%d,%d)\n", xy.iX,xy.iY).c_str());

	return true;
}

void CvPlotGroup::recalculatePlots()
{
	PROFILE_FUNC();

	CvPlot* pPlot;
	PlayerTypes eOwner;

#ifdef _DEBUG
	Validate();
#endif

	eOwner = getOwnerINLINE();

	pPlot = getRepresentativePlot();

	if (pPlot != NULL)
	{
		static int groupGenNumber = 0;
		CLLNode<XYCoords>* pPlotNode;

		plotGroupCheckInfo	checkInfo;

		checkInfo.hashInfo.allNodesHash = 0;
		checkInfo.hashInfo.resourceNodesHash = 0;
		checkInfo.groupGenerationNumber = ++groupGenNumber;

		//	Check whether the same set of cities and bonuses are still in the connected region
		//	If they are then there is no material change and we don't need to recalculate
		gDLL->getFAStarIFace()->SetData(&GC.getPlotGroupFinder(), &checkInfo);
		gDLL->getFAStarIFace()->GeneratePath(&GC.getPlotGroupFinder(), pPlot->getX_INLINE(), pPlot->getY_INLINE(), -1, -1, false, eOwner);

		if (checkInfo.hashInfo.allNodesHash == m_zobristHashes.allNodesHash)
		{
			return;
		}

		//	If the city+resource network has not changed then bonuses dont need to be
		//	recalculated and we can just use the pathing to update the plot groups on
		//	nodes that have been removed
		if (checkInfo.hashInfo.resourceNodesHash == m_zobristHashes.resourceNodesHash)
		{
			PROFILE_BEGIN("CvPlotGroup::recalculatePlots (unchanged bonuses)");

			buildRemovedPlotListParams removedPlotParams;

			removedPlotParams.removedPlots.clear();
			removedPlotParams.groupGenNumber = groupGenNumber;

			plotEnumerator(buildRemovedPlotList, &removedPlotParams);

			PROFILE_END();

			//	Just update the plot group for the removed plots
			pPlotNode = removedPlotParams.removedPlots.head();

			while (pPlotNode != NULL)
			{
				pPlot = GC.getMapINLINE().plotSorenINLINE(pPlotNode->m_data.iX, pPlotNode->m_data.iY);

				FAssertMsg(pPlot != NULL, "Plot is not assigned a valid value");

				removePlot(pPlot, false);
				pPlot->updatePlotGroup(eOwner);

				pPlotNode = removedPlotParams.removedPlots.next(pPlotNode);
			}

			while (pPlotNode != NULL)
			{
				pPlot = GC.getMapINLINE().plotSorenINLINE(pPlotNode->m_data.iX, pPlotNode->m_data.iY);

				FAssertMsg(pPlot != NULL, "Plot is not assigned a valid value");

				if ( pPlot->getPlotGroup(m_eOwner) == NULL )
				{
					colorRegion(pPlot, m_eOwner, false);
				}
				//pPlot->updatePlotGroup(eOwner, false, false);

				pPlotNode = removedPlotParams.removedPlots.deleteNode(pPlotNode);
			}
		}
		else
		{
			PROFILE("CvPlotGroup::recalculatePlots update");

			m_numCities = -1;

			buildAllPlotListParams allPlotParams;

			allPlotParams.allPlots.clear();

			plotEnumerator(buildAllPlotList, &allPlotParams);

			pPlotNode = allPlotParams.allPlots.head();

			//	Set them all to no plot group but without any update of bonuses for now
			while (pPlotNode != NULL)
			{
				PROFILE("CvPlotGroup::recalculatePlots update 1");

				pPlot = GC.getMapINLINE().plotSorenINLINE(pPlotNode->m_data.iX, pPlotNode->m_data.iY);

				FAssertMsg(pPlot != NULL, "Plot is not assigned a valid value");

				pPlot->setPlotGroup(eOwner, NULL, false);

				//OutputDebugString(CvString::format("Nulled plot group for: (%d,%d)\n", pPlot->getX_INLINE(), pPlot->getY_INLINE()).c_str());

				pPlotNode = allPlotParams.allPlots.next(pPlotNode);
			}

			pPlotNode = allPlotParams.allPlots.head();
			int	iStartingAllocSeq = m_allocationSeqForSession;

			//	Construct new plot groups (still without bonus adjustment)
			while (pPlotNode != NULL)
			{
				PROFILE("CvPlotGroup::recalculatePlots update 2");

				pPlot = GC.getMapINLINE().plotSorenINLINE(pPlotNode->m_data.iX, pPlotNode->m_data.iY);

				FAssertMsg(pPlot != NULL, "Plot is not assigned a valid value");

				if ( pPlot->getPlotGroup(m_eOwner) == NULL )
				{
					colorRegion(pPlot, m_eOwner, false);
				}
				//pPlot->updatePlotGroup(eOwner, true, false);

				pPlotNode = allPlotParams.allPlots.next(pPlotNode);
			}

			pPlotNode = allPlotParams.allPlots.head();

			//	Find the largest resulting new plot group
			int iLargest = 0;
			CvPlotGroup* pLargestGroup = NULL;

			while (pPlotNode != NULL)
			{
				PROFILE("CvPlotGroup::recalculatePlots update 2");

				pPlot = GC.getMapINLINE().plotSorenINLINE(pPlotNode->m_data.iX, pPlotNode->m_data.iY);

				FAssertMsg(pPlot != NULL, "Plot is not assigned a valid value");

				CvPlotGroup* pPlotGroup = pPlot->getPlotGroup(eOwner);

				if ( pPlotGroup != NULL && pPlotGroup->m_sessionAllocSeq >= iStartingAllocSeq && pPlotGroup->m_numPlots > iLargest )
				{
					pLargestGroup = pPlotGroup;
					iLargest = pLargestGroup->m_numPlots;
				}

				pPlotNode = allPlotParams.allPlots.next(pPlotNode);
			}

			pPlotNode = allPlotParams.allPlots.head();

			//	Go through the new set picking one of the new plot groups to
			//	be replaced by a shrunken version of the original - its members will
			//	end up just staying with the original and necessitating no bonus recalculation
			//	Others will get bonus adjustment to their new plot groups
			CvPlotGroup* newTransitionGroup = pLargestGroup;
			int iPlotsTransferred = 0;

			m_numCities = 0;

			while (pPlotNode != NULL)
			{
				PROFILE("CvPlotGroup::recalculatePlots update 3");

				pPlot = GC.getMapINLINE().plotSorenINLINE(pPlotNode->m_data.iX, pPlotNode->m_data.iY);

				FAssertMsg(pPlot != NULL, "Plot is not assigned a valid value");

				CvPlotGroup* plotGroup = pPlot->getPlotGroup(eOwner);

				if ( NULL == newTransitionGroup )
				{
					newTransitionGroup = plotGroup;
				}

				if ( plotGroup != NULL && newTransitionGroup == plotGroup )
				{
					//	Put this one back where it came from with no recalculation
					pPlot->setPlotGroup(eOwner, this, false);

					iPlotsTransferred++;
					newTransitionGroup->m_numPlots--;

					if ( pPlot->getPlotCity() != NULL && pPlot->getPlotCity()->getOwnerINLINE() == m_eOwner )
					{
						m_numCities++;
					}
				}
				else
				{
					//	Transiently put it back so we can take it out again WITH bonus
					//	recalculation
					pPlot->setPlotGroup(eOwner, this, false);
					pPlot->setPlotGroup(eOwner, plotGroup);
				}

				pPlotNode = allPlotParams.allPlots.deleteNode(pPlotNode);
			}

			if ( newTransitionGroup != NULL )
			{
				if ( newTransitionGroup->m_numPlots > 0 )
				{
					mergeIn(newTransitionGroup,true);
				}
				else
				{
					GET_PLAYER(eOwner).deletePlotGroup(newTransitionGroup->getID());
				}
			}
		}

#ifdef VALIDATION_FOR_PLOT_GROUPS
	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if ( pLoopPlot->getPlotGroupId(m_eOwner) != -1 && pLoopPlot->getPlotGroup(m_eOwner) == NULL )
		{
			::MessageBox(NULL, "Invalid plot group id found after recalc of specific plot group!", "CvGameCoreDLL", MB_OK);
		}
	}
#endif
	}
}

int CvPlotGroup::getID() const
{
	return m_iID;
}


void CvPlotGroup::setID(int iID)
{
	m_iID = iID;
}


PlayerTypes CvPlotGroup::getOwner() const
{
	return getOwnerINLINE();
}


int CvPlotGroup::getNumBonuses(BonusTypes eBonus) const
{
	FAssertMsg(eBonus >= 0, "eBonus is expected to be non-negative (invalid Index)");
	FAssertMsg(eBonus < GC.getNumBonusInfos(), "eBonus is expected to be within maximum bounds (invalid Index)");
	return (m_paiNumBonuses == NULL ? 0 : m_paiNumBonuses[eBonus]);
}


bool CvPlotGroup::hasBonus(BonusTypes eBonus)
{
	return(getNumBonuses(eBonus) > 0);
}


void CvPlotGroup::changeNumBonuses(BonusTypes eBonus, int iChange)
{
	PROFILE_FUNC();

	FAssertMsg(eBonus >= 0, "eBonus is expected to be non-negative (invalid Index)");
	FAssertMsg(eBonus < GC.getNumBonusInfos(), "eBonus is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		if ( m_paiNumBonuses == NULL )
		{
			m_paiNumBonuses = new int[GC.getNumBonusInfos()];
			memset(m_paiNumBonuses, 0, sizeof(int)*GC.getNumBonusInfos());
		}

		m_paiNumBonuses[eBonus] = (m_paiNumBonuses[eBonus] + iChange);

		int iLoop;
		CvCity* pLoopCity;
		for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
		{
			if (pLoopCity->plotGroup(getOwnerINLINE()) == this)
			{
				pLoopCity->changeNumBonuses(eBonus, iChange);
			}
		}
	}
}

int CvPlotGroup::getNumFreeTradeRegionBuildings(BuildingTypes eBuilding) const
{
	FAssertMsg(eBuilding >= 0, "eBuilding is expected to be non-negative (invalid Index)");
	FAssertMsg(eBuilding < GC.getNumBuildingInfos(), "eBuilding is expected to be within maximum bounds (invalid Index)");
	return (m_paiNumFreeTradeRegionBuildings == NULL ? 0 :m_paiNumFreeTradeRegionBuildings[eBuilding]);
}

void CvPlotGroup::changeNumFreeTradeRegionBuildings(BuildingTypes eBuilding, int iChange)
{
	PROFILE_FUNC();

	FAssertMsg(eBuilding >= 0, "eBuilding is expected to be non-negative (invalid Index)");
	FAssertMsg(eBuilding < GC.getNumBuildingInfos(), "eBuilding is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		//	Can go negative in games that started under assets that didn't have the same
		//	free building sets as current assets - prevent this
		int iNewValue = std::max(0,m_paiNumFreeTradeRegionBuildings[eBuilding] + iChange);

		if ( iNewValue != 0 && m_paiNumFreeTradeRegionBuildings == NULL )
		{
			m_paiNumFreeTradeRegionBuildings = new int[GC.getNumBuildingInfos()];

			for(int iI = 0; iI < GC.getNumBuildingInfos(); iI++)
			{
				m_paiNumFreeTradeRegionBuildings[iI] = 0;
			}
		}
		if ( m_paiNumFreeTradeRegionBuildings != NULL )
		{
			m_paiNumFreeTradeRegionBuildings[eBuilding] = iNewValue;
		}

		int iLoop;
		CvCity* pLoopCity;
		for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
		{
			if (pLoopCity->plotGroup(getOwnerINLINE()) == this)
			{
				pLoopCity->changeNumFreeTradeRegionBuilding(eBuilding, iChange);
			}
		}
	}
}

void CvPlotGroup::plotEnumerator(bool (*pfFunc)(CvPlotGroup* onBehalfOf, CvPlot*, void*), void* param)
{
	CvPlot*	pStartPlot;
	bool bSeedValid = false;
	int expectedPlotsRemaining = m_numPlots;

	if ( m_seedPlotX != -1 && m_seedPlotY != -1 )
	{
		pStartPlot = GC.getMapINLINE().plotSorenINLINE(m_seedPlotX, m_seedPlotY);

		if ( pStartPlot->getPlotGroup(m_eOwner) == this )
		{
			bSeedValid = true;
		}
		else
		{
			m_seedPlotX = -1;
		}
	}

	if ( !bSeedValid )
	{
		pStartPlot = GC.getMapINLINE().plotSorenINLINE(GC.getMapINLINE().getGridWidthINLINE()/2,GC.getMapINLINE().getGridHeightINLINE()/2);
	}

	int iMaxXDistance = GC.getMapINLINE().getGridWidthINLINE();
	int iMaxYDistance = GC.getMapINLINE().getGridHeightINLINE();

	if ( GC.getMapINLINE().isWrapXINLINE() )
	{
		iMaxXDistance /= 2;
	}

	if ( GC.getMapINLINE().isWrapYINLINE() )
	{
		iMaxYDistance /= 2;
	}

	int iMinX = pStartPlot->getX_INLINE() - iMaxXDistance + (GC.getMapINLINE().getGridWidthINLINE()%2 == 0 ? 1 : 0);
	int iMaxX = pStartPlot->getX_INLINE() + iMaxXDistance;
	int iMinY = pStartPlot->getY_INLINE() - iMaxYDistance + (GC.getMapINLINE().getGridHeightINLINE()%2 == 0 ? 1 : 0);
	int iMaxY = pStartPlot->getY_INLINE() + iMaxYDistance;

	for(int iRadius = 0;
			iRadius <= std::max(iMaxXDistance, iMaxYDistance) && expectedPlotsRemaining > 0;
			iRadius++)
	{
		//	Examine the ring of plots iDistance away from us (this is a square)
		for(int iPerimeter = -iRadius;
		    iPerimeter < iRadius || (iRadius == 0 && iPerimeter == 0);
			iPerimeter++)
		{
			for( int iSide = 0; iSide < 4; iSide++ )
			{
				int iX = pStartPlot->getX_INLINE();
				int iY = pStartPlot->getY_INLINE();

				switch(iSide)
				{
				case 0:
					//	North side
					iX = iX - iPerimeter;
					iY = iY + iRadius;
					break;
				case 1:
					//	East side
					iX = iX + iRadius;
					iY = iY + iPerimeter;
					break;
				case 2:
					//	South side
					iX = iX + iPerimeter;
					iY = iY - iRadius;
					break;
				case 3:
					//	West side
					iX = iX - iRadius;
					iY = iY - iPerimeter;
					break;
				}

				if ( iX < iMinX || iY < iMinY || iX > iMaxX || iY > iMaxY)
				{
					continue;
				}

				CvPlot* pLoopPlot = GC.getMapINLINE().plotINLINE(iX,iY);

				if ( pLoopPlot != NULL && pLoopPlot->getPlotGroup(m_eOwner) == this )
				{
					expectedPlotsRemaining--;
					if ( m_seedPlotX == -1 )
					{
						m_seedPlotX = pLoopPlot->getX_INLINE();
						m_seedPlotY = pLoopPlot->getY_INLINE();
					}

					if ( !(*pfFunc)(this, pLoopPlot, param) )
					{
						return;
					}
				}

				if ( iRadius == 0 )
				{
					break;
				}
			}
		}
	}
}

static bool countCitiesCallback(CvPlotGroup* onBehalfOf, CvPlot* pLoopPlot, void* dummy)
{
	CvCity* pCity = pLoopPlot->getPlotCity();

	if (pCity != NULL && pCity->getOwnerINLINE() == onBehalfOf->getOwnerINLINE())
	{
		onBehalfOf->m_numCities++;
	}

	return true;
}


int CvPlotGroup::getNumCities(void)
{
	if ( m_numCities == -1 )
	{
		m_numCities = 0;

		plotEnumerator(countCitiesCallback, NULL);
	}

	return m_numCities;
}

void CvPlotGroup::read(FDataStreamBase* pStream)
{
	MEMORY_TRACE_FUNCTION();

	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

	wrapper.AttachToStream(pStream);

	WRAPPER_READ_OBJECT_START(wrapper);

	// Init saved data
	reset();

	uint uiFlag=0;
	WRAPPER_READ(wrapper, "CvPlotGroup", &uiFlag);	// flags for expansion

	WRAPPER_READ(wrapper, "CvPlotGroup", &m_iID);

	WRAPPER_READ(wrapper, "CvPlotGroup", (int*)&m_eOwner);

	bool arrayPresent = true;
	WRAPPER_READ_DECORATED(wrapper, "CvPlotGroup", &arrayPresent, "bonusesPresent");
	if ( arrayPresent )
	{
		if ( m_paiNumBonuses == NULL )
		{
			m_paiNumBonuses = new int[GC.getNumBonusInfos()];
		}
		WRAPPER_READ_CLASS_ARRAY(wrapper, "CvPlotGroup", REMAPPED_CLASS_TYPE_BONUSES, GC.getNumBonusInfos(), m_paiNumBonuses);
	}
	else
	{
		SAFE_DELETE_ARRAY(m_paiNumBonuses);
	}

	arrayPresent = true;
	WRAPPER_READ_DECORATED(wrapper, "CvPlotGroup", &arrayPresent, "freeBuildingsPresent");
	if ( arrayPresent )
	{
		if ( m_paiNumFreeTradeRegionBuildings == NULL )
		{
			m_paiNumFreeTradeRegionBuildings = new int[GC.getNumBuildingInfos()];
		}
		WRAPPER_READ_CLASS_ARRAY(wrapper, "CvPlotGroup", REMAPPED_CLASS_TYPE_BUILDINGS, GC.getNumBuildingInfos(), m_paiNumFreeTradeRegionBuildings);
	}
	else
	{
		SAFE_DELETE_ARRAY(m_paiNumFreeTradeRegionBuildings);
	}

	m_numPlots = -1;
	WRAPPER_READ(wrapper, "CvPlotGroup", &m_numPlots);

	//	To maintain backwrd compatibility read the plot list from the old format
	//	that didn't record m_numPlots
	if ( m_numPlots == -1 )
	{
		CLinkList<XYCoords> dummyPlots;
		dummyPlots.Read(pStream);

		m_numPlots = dummyPlots.getLength();

		FAssert(m_numPlots > 0);
	}

	int iI;

	if ( m_paiNumBonuses != NULL )
	{
		for(iI = 0; iI < GC.getNumBonusInfos(); iI++)
		{
			if ( m_paiNumBonuses[iI] != 0 )
			{
				break;
			}
		}

		if ( iI == GC.getNumBonusInfos() )
		{
			SAFE_DELETE_ARRAY(m_paiNumBonuses);
		}
	}

	if ( m_paiNumFreeTradeRegionBuildings != NULL )
	{
		for(iI= 0; iI < GC.getNumBuildingInfos(); iI++)
		{
			if ( m_paiNumFreeTradeRegionBuildings[iI] != 0 )
			{
				break;
			}
		}

		if ( iI == GC.getNumBuildingInfos() )
		{
			SAFE_DELETE_ARRAY(m_paiNumFreeTradeRegionBuildings);
		}
	}

	WRAPPER_READ(wrapper, "CvPlotGroup", &m_seedPlotX);
	WRAPPER_READ(wrapper, "CvPlotGroup", &m_seedPlotY);

	WRAPPER_READ_OBJECT_END(wrapper);
}


void CvPlotGroup::write(FDataStreamBase* pStream)
{
	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();

	wrapper.AttachToStream(pStream);

	WRAPPER_WRITE_OBJECT_START(wrapper);

	uint uiFlag=0;
	WRAPPER_WRITE(wrapper, "CvPlotGroup", uiFlag);		// flag for expansion

	WRAPPER_WRITE(wrapper, "CvPlotGroup", m_iID);

	WRAPPER_WRITE(wrapper, "CvPlotGroup", m_eOwner);

	WRAPPER_WRITE_DECORATED(wrapper, "CvPlotGroup", (bool)(m_paiNumBonuses != NULL), "bonusesPresent");
	if ( m_paiNumBonuses != NULL )
	{
		WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvPlotGroup", REMAPPED_CLASS_TYPE_BONUSES, GC.getNumBonusInfos(), m_paiNumBonuses);
	}
	WRAPPER_WRITE_DECORATED(wrapper, "CvPlotGroup", (bool)(m_paiNumFreeTradeRegionBuildings != NULL), "freeBuildingsPresent");
	if ( m_paiNumFreeTradeRegionBuildings != NULL )
	{
		WRAPPER_WRITE_CLASS_ARRAY(wrapper, "CvPlotGroup", REMAPPED_CLASS_TYPE_BUILDINGS, GC.getNumBuildingInfos(), m_paiNumFreeTradeRegionBuildings);
	}

	WRAPPER_WRITE(wrapper, "CvPlotGroup", m_numPlots);

	WRAPPER_WRITE(wrapper, "CvPlotGroup", m_seedPlotX);
	WRAPPER_WRITE(wrapper, "CvPlotGroup", m_seedPlotY);

	WRAPPER_WRITE_OBJECT_END(wrapper);
}


static bool zobristHashSetter(CvPlotGroup* onBehalfOf, CvPlot* pLoopPlot, void* params)
{
	plotGroupHashInfo* parm = (plotGroupHashInfo*)params;

	parm->allNodesHash ^= pLoopPlot->getZobristContribution();
	if ( pLoopPlot->isCity() || 
		 (pLoopPlot->getImprovementType() != NO_IMPROVEMENT && pLoopPlot->getBonusType() != NO_BONUS) )
	{
		parm->resourceNodesHash ^= pLoopPlot->getZobristContribution();
	}

	return true;
}

//	Calculate the hashes after a fresh game load
void CvPlotGroup::RecalculateHashes()
{
	//	Set up the Zobrist hashes
	plotEnumerator(zobristHashSetter, &m_zobristHashes);
}

typedef struct plotGroupMergerParams
{
	CvPlotGroup* mergeTo;
	bool		 bRecalculateBonuses;
} plotGroupMergerParams;

static bool plotGroupMerger(CvPlotGroup* onBehalfOf, CvPlot* pLoopPlot, void* params)
{
	plotGroupMergerParams* parm = (plotGroupMergerParams*)params;

	parm->mergeTo->addPlot(pLoopPlot, parm->bRecalculateBonuses);

	return true;
}
	
void CvPlotGroup::mergeIn(CvPlotGroup* from, bool bRecalculateBonuses)
{
	PROFILE_FUNC();

	plotGroupMergerParams params;

	params.mergeTo = this;
	params.bRecalculateBonuses = bRecalculateBonuses;

	from->plotEnumerator(plotGroupMerger, &params);
	
	GET_PLAYER(getOwnerINLINE()).deletePlotGroup(from->getID());
}

void CvPlotGroup::colorRegion(CvPlot* pStartPlot, PlayerTypes eOwner, bool bRecalculateBonuses)
{
	FAssert(pStartPlot->getPlotGroup(eOwner) == NULL);

	colorRegionInternal(pStartPlot, eOwner, NULL, bRecalculateBonuses);
}

CvPlotGroup* CvPlotGroup::colorRegionInternal(CvPlot* pPlot, PlayerTypes eOwner, CvPlotGroup* pPlotGroup, bool bRecalculateBonuses)
{
	PROFILE_FUNC();

	if (pPlot->isTradeNetwork(GET_PLAYER(eOwner).getTeam()))
	{
		std::vector<CvPlot*> queue;

		queue.reserve(100);
		queue.push_back(pPlot);

		int iWaterMark = 0;

		while( iWaterMark < (int) queue.size() )
		{
			CvPlot* pLoopPlot = queue[iWaterMark++];

			if ( pLoopPlot->getPlotGroup(eOwner) == NULL )
			{
				if ( pPlotGroup == NULL )
				{
					pPlotGroup = GET_PLAYER(eOwner).initPlotGroup(pLoopPlot, bRecalculateBonuses);
				}
				else
				{
					pPlotGroup->addPlot(pLoopPlot, bRecalculateBonuses);
				}

				for (int iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
				{
					CvPlot* pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), ((DirectionTypes)iI));

					if (pAdjacentPlot != NULL)
					{
						if (pLoopPlot->isTradeNetworkConnected(pAdjacentPlot, GET_PLAYER(eOwner).getTeam()))
						{
							CvPlotGroup* pAdjacentPlotGroup = pAdjacentPlot->getPlotGroup(eOwner);

							if (pAdjacentPlotGroup != NULL)
							{
								if ( pAdjacentPlotGroup != pPlotGroup )
								{
									if ( pPlotGroup->getLengthPlots() > pAdjacentPlotGroup->getLengthPlots() )
									{
										pPlotGroup->mergeIn(pAdjacentPlotGroup, bRecalculateBonuses);
									}
									else
									{
										pAdjacentPlotGroup->mergeIn(pPlotGroup, bRecalculateBonuses);

										pPlotGroup = pAdjacentPlotGroup;
									}
								}
							}
							else
							{
								queue.push_back(pAdjacentPlot);
							}
						}
					}
				}
			}
		}
	}

	return pPlotGroup;
}
