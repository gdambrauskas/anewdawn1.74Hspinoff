// unitAI.cpp

#include "CvGameCoreDLL.h"
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

	m_advertisingUnits.push_back(unitDetails);
}

//	Make a work request
//		iPriority should be in the range 0-100 ideally
//		eUnitFlags indicate the type(s) of unit sought
//		(iAtX,iAtY) is (roughly) where the work will be
//		pJoinUnit may be NULL but if not it is a request to join that unit's group
void	CvContractBroker::advertiseWork(int iPriority, unitCapabilities eUnitFlags, int iAtX, int iAtY, CvUnit* pJoinUnit)
{
	PROFILE_FUNC();

	workRequest	newRequest;

	newRequest.iPriority		= iPriority;
	newRequest.eUnitFlags		= eUnitFlags;
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
	m_workRequests.insert(insertAt, newRequest);
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
			if ( unitInfo.bIsWorker || (request.eUnitFlags & WORKER_UNITCAPABILITIES) == 0 )
			{
				if ( unitInfo.iDefensiveValue > 0 && (request.eUnitFlags & DEFENSIVE_UNITCAPABILITIES) != 0 )
				{
					iValue += unitInfo.iDefensiveValue;
				}
				if ( unitInfo.iDefensiveValue > 0 && (request.eUnitFlags & OFFENSIVE_UNITCAPABILITIES) != 0 )
				{
					iValue += unitInfo.iOffensiveValue;
				}
			}

			if ( iValue*1000 > iBestValue )
			{
				CvUnit* pLoopUnit = findUnit(unitInfo.iUnitId);
				CvPlot*	pTargetPlot = GC.getMapINLINE().plotINLINE(request.iAtX, request.iAtY);
				int		iPathTurns = 0;

				if ( pLoopUnit != NULL && (pLoopUnit->atPlot(pTargetPlot) || (!bThisPlotOnly && pLoopUnit->generatePath(pTargetPlot, 0, true, &iPathTurns))) )
				{
					//	For low priority work never try to satisfy it with a distant unit
					if ( request.iPriority > 25 || iPathTurns < 5 )
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