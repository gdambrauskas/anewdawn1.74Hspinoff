// unitAI.cpp

#include "CvGameCoreDLL.h"
#include "BetterBTSAI.h"
#include "CvPathGenerator.h"
#include "CvContractBroker.h"

CvContractBroker::CvContractBroker() : m_eOwner(NO_PLAYER)
{
	reset();
}

CvContractBroker::~CvContractBroker(void)
{
}

//	Delete all work requests and looking for work records
void	CvContractBroker::reset()
{
	m_workRequests.clear();
	m_advertisingUnits.clear();
	m_advertisingTenders.clear();
	m_contractedUnits.clear();

	m_iNextWorkRequestId = 0;
}

//	Initialize
void	CvContractBroker::init(PlayerTypes eOwner)
{
	m_eOwner = eOwner;
}

//	Note a unit looking for work
void	CvContractBroker::lookingForWork(CvUnit* pUnit)
{
	PROFILE_FUNC();

	advertisingUnit	unitDetails;
	int				iUnitStr = GC.getGameINLINE().AI_combatValue(pUnit->getUnitType());

	unitDetails.bIsWorker = (pUnit->AI_getUnitAIType() == UNITAI_WORKER);

	//	Combat values are just the crude value of the unit type for now - should add in promotions
	//	here for sure
	if ( pUnit->canDefend() )
	{
		unitDetails.iDefensiveValue = iUnitStr;
	}
	if ( pUnit->canAttack() )
	{
		unitDetails.iOffensiveValue = iUnitStr;
	}

	unitDetails.iUnitId = pUnit->getID();
	unitDetails.iAtX	= pUnit->getX_INLINE();
	unitDetails.iAtY	= pUnit->getY_INLINE();

	//	Initially not assigned to a contract
	unitDetails.iContractedWorkRequest = -1;
	//	and no attempt has been made yet to match any work requests
	unitDetails.iMatchedToRequestSeq = -1;

	{
		MEMORY_TRACK_EXEMPT();

		m_advertisingUnits.push_back(unitDetails);
	}
}

//	Make a work request
//		iPriority should be in the range 0-100 ideally
//		eUnitFlags indicate the type(s) of unit sought
//		(iAtX,iAtY) is (roughly) where the work will be
//		pJoinUnit may be NULL but if not it is a request to join that unit's group
void	CvContractBroker::advertiseWork(int iPriority, unitCapabilities eUnitFlags, int iAtX, int iAtY, CvUnit* pJoinUnit, UnitAITypes eAIType)
{
	PROFILE_FUNC();

	workRequest	newRequest;
	int			iLoop;

	//	First check that there are not already units on the way to meet this need
	//	else concurrent builds will get queued while they are in transit
	for(CvSelectionGroup* pLoopSelectionGroup = GET_PLAYER(m_eOwner).firstSelectionGroup(&iLoop); pLoopSelectionGroup; pLoopSelectionGroup = GET_PLAYER(m_eOwner).nextSelectionGroup(&iLoop))
	{
		CvPlot* pMissionPlot = pLoopSelectionGroup->AI_getMissionAIPlot();

		if ( pMissionPlot == GC.getMapINLINE().plotINLINE(iAtX, iAtY) &&
			 pLoopSelectionGroup->AI_getMissionAIUnit() == pJoinUnit &&
			 pLoopSelectionGroup->AI_getMissionAIType() == MISSIONAI_CONTRACT )
		{
			std::map<int,bool>::const_iterator itr = m_contractedUnits.find(pLoopSelectionGroup->getID());

			if ( itr == m_contractedUnits.end() )
			{
				m_contractedUnits[pLoopSelectionGroup->getID()] = true;
				break;
			}
			else
			{
				if( gUnitLogLevel >= 3 ) logBBAI("      Unit %S at (%d,%d) already responding to contract at (%d,%d)",
												 pLoopSelectionGroup->getHeadUnit()->getDescription(),
												 pLoopSelectionGroup->getX(), 
												 pLoopSelectionGroup->getY(), 
												 iAtX, iAtY);
			}
		}
	}

	newRequest.iPriority		= iPriority;
	newRequest.eUnitFlags		= eUnitFlags;
	newRequest.eAIType			= eAIType;
	newRequest.iAtX				= iAtX;
	newRequest.iAtY				= iAtY;
	newRequest.iWorkRequestId	= ++m_iNextWorkRequestId;
	newRequest.bFulfilled		= false;

	if ( pJoinUnit == NULL )
	{
		newRequest.iUnitId	= -1;
	}
	else
	{
		newRequest.iUnitId	= pJoinUnit->getID();
	}

	//	Insert in priority order, highest first
	std::vector<workRequest>::iterator insertAt;

	for( insertAt = m_workRequests.begin(); insertAt != m_workRequests.end(); insertAt++ )
	{
		if ( iPriority > (*insertAt).iPriority )
		{
			break;
		}
	}

	{
		MEMORY_TRACK_EXEMPT();

		m_workRequests.insert(insertAt, newRequest);
	}
}

//	Advertise a tender to build units
//		iMinPriority indicaes the lowest priority request this tender is appropriate for
void	CvContractBroker::advertiseTender(CvCity* pCity, int iMinPriority)
{
	PROFILE_FUNC();

	if( gCityLogLevel >= 3 ) logBBAI("      City %S tenders for unit builds at priority %d", pCity->getName().GetCString(), iMinPriority);
	
	cityTender	newTender;

	newTender.iMinPriority		= iMinPriority;
	newTender.iCityId			= pCity->getID();

	{
		MEMORY_TRACK_EXEMPT();

		m_advertisingTenders.push_back(newTender);
	}
}

void CvContractBroker::finalizeTenderContracts(void)
{
	std::map<unsigned int,int>	tenderAllocations;

	for(int iI = 0; iI < (int)m_workRequests.size(); iI++)
	{
		if ( !m_workRequests[iI].bFulfilled )
		{
			int iBestValue = 0;
			int iBestCityTenderKey;
			int iValue;
			UnitTypes eUnit;
			UnitTypes eBestUnit = NO_UNIT;
			CvCity*	pBestCity = NULL;
			CvPlot* pDestPlot = GC.getMapINLINE().plotINLINE(m_workRequests[iI].iAtX, m_workRequests[iI].iAtY);

			if( gCityLogLevel >= 3 )
			{
				CvString	unitAIType;

				if ( m_workRequests[iI].eAIType == NO_UNITAI )
				{
					unitAIType = "NO_UNITAI";
				}
				else
				{
					CvInfoBase& AIType = GC.getUnitAIInfo(m_workRequests[iI].eAIType);
					unitAIType = AIType.getType();
				}

				logBBAI("      Processing bids for tender for unitAI %s at (%d,%d) with priority %d",
						unitAIType.c_str(),
						m_workRequests[iI].iAtX, m_workRequests[iI].iAtY,
						m_workRequests[iI].iPriority);
			}

			for(unsigned int iJ = 0; iJ < m_advertisingTenders.size(); iJ++)
			{
				if ( m_advertisingTenders[iJ].iMinPriority <= m_workRequests[iI].iPriority )
				{
					CvCity* pCity = GET_PLAYER(m_eOwner).getCity(m_advertisingTenders[iJ].iCityId);

					if ( pCity != NULL && pDestPlot != NULL )
					{
						if ( pCity->area() == pDestPlot->area() || (pDestPlot->getPlotCity() != NULL && pCity->waterArea() != pDestPlot->getPlotCity()->waterArea()) )
						{
							int	iTendersAlreadyInProcess = pCity->numQueuedUnits(m_workRequests[iI].eAIType, pDestPlot);
							unsigned int iTenderAllocationKey = 0;
							unsigned int iAnyAITenderAllocationKey = 0;

							CheckSum(iTenderAllocationKey, pCity->getID());
							CheckSum(iTenderAllocationKey, GC.getMapINLINE().plotNumINLINE(m_workRequests[iI].iAtX, m_workRequests[iI].iAtY));
							CheckSum(iTenderAllocationKey, (int)m_workRequests[iI].eAIType);

							std::map<unsigned int, int>::const_iterator itr = tenderAllocations.find(iTenderAllocationKey);
							if ( itr != tenderAllocations.end() )
							{
								iTendersAlreadyInProcess -= itr->second;
							}
							else
							{
								tenderAllocations[iTenderAllocationKey] = 0;
							}

							FAssert(iTendersAlreadyInProcess >= 0);

							if ( iTendersAlreadyInProcess == 0 )
							{
								if ( m_workRequests[iI].eAIType == NO_UNITAI )
								{
									UnitAITypes*	pUnitAIs = NULL;
									int				iNumAIs = -1;

									if ( (m_workRequests[iI].eUnitFlags & DEFENSIVE_UNITCAPABILITIES) != 0 )
									{
										static UnitAITypes defensiveAIs[] = { UNITAI_CITY_DEFENSE, UNITAI_ATTACK };
										pUnitAIs = defensiveAIs;
										iNumAIs = sizeof(defensiveAIs)/sizeof(UnitAITypes);
									}
									if ( (m_workRequests[iI].eUnitFlags & OFFENSIVE_UNITCAPABILITIES) != 0 )
									{
										static UnitAITypes offensiveAIs[] = { UNITAI_ATTACK };
										pUnitAIs = offensiveAIs;
										iNumAIs = sizeof(offensiveAIs)/sizeof(UnitAITypes);
									}
									if ( (m_workRequests[iI].eUnitFlags & WORKER_UNITCAPABILITIES) != 0 )
									{
										static UnitAITypes workerAIs[] = { UNITAI_WORKER };
										pUnitAIs = workerAIs;
										iNumAIs = sizeof(workerAIs)/sizeof(UnitAITypes);
									}

									// gvd eUnit = pCity->AI_bestUnit(iValue, iNumAIs, pUnitAIs, false, NO_ADVISOR, NULL, false, true);
								}
								else
								{
									eUnit = pCity->AI_bestUnitAI(m_workRequests[iI].eAIType, iValue);
								}
								if ( eUnit != NO_UNIT )
								{
									//	Adjust value for production time and distance
									int iTurns = 0;
									int iBaseValue = iValue;
									
									if ( (pCity->isProduction() && pCity->getOrderData(0).eOrderType == ORDER_TRAIN) )
									{
										iTurns = pCity->getTotalProductionQueueTurnsLeft() + pCity->getProductionTurnsLeft(eUnit, 1);
									}
									else
									{
										iTurns = pCity->getProductionTurnsLeft(eUnit, 1);
									}

									//	Decrease the value 10% per (standard speed) turn
									iValue *= 100 - 10*std::min((iTurns*100)/GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent(),10);
									iValue /= 100;

									//	Decrease by 5% per turn of separation from destination
									//int iDistance = GC.getMapINLINE().calculatePathDistance(pCity->plot(),pDestPlot);
									if ( CvSelectionGroup::getPathGenerator()->generatePathForHypotheticalUnit(pCity->plot(), pDestPlot, m_eOwner, eUnit, MOVE_NO_ENEMY_TERRITORY, 19) )
									{
										int iDistance = CvSelectionGroup::getPathGenerator()->getLastPath().length();
										iValue *= 100 - 5*std::min(20, iDistance);
										iValue /= 100;
										
										if( gCityLogLevel >= 3 )
										{
											logBBAI("      City %S could supply unit %S with base value %d, depreciated value (aftr %d turn production at distance %d) to %d",
													pCity->getName().GetCString(),
													GC.getUnitInfo(eUnit).getDescription(),
													iBaseValue,
													iTurns,
													iDistance,
													iValue);
										}

										if ( iValue > iBestValue )
										{
											iBestValue = iValue;
											iBestCityTenderKey = iTenderAllocationKey;
											eBestUnit = eUnit;
											pBestCity = pCity;
										}
									}
								}
								else
								{
									if( gCityLogLevel >= 3 )
									{
										logBBAI("      City %S has no suitable units to offer",
												pCity->getName().GetCString());
									}
								}
							}
							else
							{
								//	Already being built
								m_workRequests[iI].bFulfilled = true;
								eBestUnit = NO_UNIT;

								tenderAllocations[iTenderAllocationKey] = tenderAllocations[iTenderAllocationKey] + 1;
								
								if( gCityLogLevel >= 3 )
								{
									logBBAI("      City %S is already building a unit",
											pCity->getName().GetCString());
								}
								break;
							}
						}
					}
				}
			}

			if ( eBestUnit != NO_UNIT )
			{
				if( gCityLogLevel >= 2 )
				{
					CvString	unitAIType;

					if ( m_workRequests[iI].eAIType == NO_UNITAI )
					{
						unitAIType = "NO_UNITAI";
					}
					else
					{
						CvInfoBase& AIType = GC.getUnitAIInfo(m_workRequests[iI].eAIType);
						unitAIType = AIType.getType();
					}

					logBBAI("      City %S wins business for unitAI build %s (training %S)",
							pBestCity->getName().GetCString(),
							unitAIType.c_str(),
							GC.getUnitInfo(eBestUnit).getDescription());
				}
				
				m_workRequests[iI].bFulfilled = true;
				tenderAllocations[iBestCityTenderKey] = tenderAllocations[iBestCityTenderKey] + 1;

				//	Queue up the build.  Add to queue head if the current build is not a unit (implies
				//	a local build below the priority of work the city tendered for)
				bool bAppend = (pBestCity->isProduction() && pBestCity->getOrderData(0).eOrderType == ORDER_TRAIN);
				/* gvd
				pBestCity->pushOrder(ORDER_TRAIN,
									 eBestUnit,
									 m_workRequests[iI].eAIType,
									 false,
									 !bAppend,
									 bAppend,
									 false,
									 pDestPlot,
									 m_workRequests[iI].eAIType);*/
			}
		}
	}
}

//	Make a contract
//	This will attempt to make the best contracts between currently
//	advertising units and work, then search the resuilting set for the work 
//	of the requested unit
//	returns true if a contract is made along with the details of what to do
bool	CvContractBroker::makeContract(CvUnit* pUnit, int& iAtX, int& iAtY, CvUnit*& pJoinUnit, bool bThisPlotOnly)
{
	PROFILE_FUNC();

	int			iI;

	//	Satisfy the highest priority requests first (sort order of m_workRequests)
	for(iI = 0; iI < (int)m_workRequests.size(); iI++)
	{
		if ( !m_workRequests[iI].bFulfilled )
		{
			advertisingUnit*	suitableUnit = findBestUnit( m_workRequests[iI], bThisPlotOnly );

			if ( NULL != suitableUnit )
			{
				suitableUnit->iContractedWorkRequest = m_workRequests[iI].iWorkRequestId;
				m_workRequests[iI].bFulfilled = true;
			}
		}
	}

	for(iI = 0; iI < (int)m_advertisingUnits.size(); iI++)
	{
		//	Note that all existing advertising units have attempetd to match
		//	against existing work requests
		m_advertisingUnits[iI].iMatchedToRequestSeq = m_iNextWorkRequestId;
	}

	//	Now see if this unit has work assigned
	for(iI = 0; iI < (int)m_advertisingUnits.size(); iI++)
	{
		if ( m_advertisingUnits[iI].iUnitId == pUnit->getID() )
		{
			int	iWorkRequest = m_advertisingUnits[iI].iContractedWorkRequest;

			if ( -1 != iWorkRequest )
			{
				const workRequest*	contractedRequest = findWorkRequest( iWorkRequest );

				FAssert(NULL != contractedRequest);

				iAtX		= contractedRequest->iAtX;
				iAtY		= contractedRequest->iAtY;

				pJoinUnit = findUnit(contractedRequest->iUnitId);
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	return false;
}

const workRequest*	CvContractBroker::findWorkRequest(int iWorkRequestId) const
{
	PROFILE_FUNC();

	for(int iI = 0; iI < (int)m_workRequests.size(); iI++ )
	{
		if ( m_workRequests[iI].iWorkRequestId == iWorkRequestId )
		{
			return &m_workRequests[iI];
		}
	}

	return NULL;
}

advertisingUnit*	CvContractBroker::findBestUnit(workRequest& request, bool bThisPlotOnly)
{
	PROFILE_FUNC();

	int	iBestValue = 0;
	int iBestUnitIndex = -1;

	for(int iI = 0; iI < (int)m_advertisingUnits.size(); iI++)
	{
		advertisingUnit&	unitInfo = m_advertisingUnits[iI];
		int	iValue = 0;

		//	Don't bother recalculating this advertiser/requestor pair if they have already been
		//	calculated previously
		if ( unitInfo.iMatchedToRequestSeq < request.iWorkRequestId &&
			 unitInfo.iContractedWorkRequest == -1 )
		{
			CvUnit* pLoopUnit = findUnit(unitInfo.iUnitId);

			if ( (request.eUnitFlags & WORKER_UNITCAPABILITIES) == 0 )
			{
				if ( request.eAIType == NO_UNITAI || pLoopUnit->getUnitInfo().getUnitAIType(request.eAIType) )
				{
					iValue += 10;

					if ( unitInfo.iDefensiveValue > 0 && (request.eUnitFlags == 0 || (request.eUnitFlags & DEFENSIVE_UNITCAPABILITIES) != 0) )
					{
						iValue += unitInfo.iDefensiveValue;
					}
					if ( unitInfo.iDefensiveValue > 0 && (request.eUnitFlags == 0 || (request.eUnitFlags & OFFENSIVE_UNITCAPABILITIES) != 0) )
					{
						iValue += unitInfo.iOffensiveValue;
					}
				}
			}
			else if ( unitInfo.bIsWorker )
			{
				iValue = 100;
			}

			if ( iValue*1000 > iBestValue )
			{
				CvPlot*	pTargetPlot = GC.getMapINLINE().plotINLINE(request.iAtX, request.iAtY);
				int		iPathTurns = 0;
				int		iMaxPathTurns = std::min((request.iPriority > 25 ? MAX_INT : 5), iBestValue/(1000*iValue));

				//	For low priority work never try to satisfy it with a distant unit
				if ( pLoopUnit != NULL &&
					 (pLoopUnit->atPlot(pTargetPlot) /* gvd ||
					  (!bThisPlotOnly && pLoopUnit->generatePath(pTargetPlot,
																 MOVE_SAFE_TERRITORY | MOVE_AVOID_ENEMY_UNITS,
																 true,
																 &iPathTurns,
																 iMaxPathTurns))*/) )
				{
					iValue *= 1000;
					iValue /= (iPathTurns+1);

					if ( iValue > iBestValue )
					{
						iBestValue		= iValue;
						iBestUnitIndex	= iI;
					}
				}
			}
		}
	}

	if ( iBestUnitIndex == -1 )
	{
		return NULL;
	}
	else
	{
		return &m_advertisingUnits[iBestUnitIndex];
	}
}

CvUnit* CvContractBroker::findUnit(int iUnitId) const
{
	if ( iUnitId == -1 )
	{
		return NULL;
	}
	else
	{
		return GET_PLAYER((PlayerTypes)m_eOwner).getUnit(iUnitId);
	}
}