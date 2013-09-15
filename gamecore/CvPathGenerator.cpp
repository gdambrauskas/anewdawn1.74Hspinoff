//	Internal path generation engine

#include "CvGameCoreDLL.h"
#include "CvSelectionGroup.h"
#include "CvBugOptions.h"
#include "CvPathGenerator.h"

//#define	TRACE_PATHING

#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
#define	VALIDATE_TREE(x,y,z)	ValidateTree(x,y,z);
#else
#define	VALIDATE_TREE(x,y,z)	;
#endif

class CvPathGeneratorPlotInfo
{
public:
	CvPathGeneratorPlotInfo() {}

	CvPathNode* pNode;
	bool		bKnownInvalidNode;
};

//	Helper class representing a path tree node
class CvPathNode
{
public:
	CvPathNode()
	{
		m_firstChild	= NULL;
		m_prevSibling	= NULL;
		m_nextSibling	= NULL;
		m_plot			= NULL;
	}

	~CvPathNode()
	{
	}

	int			m_iPathTurns;
	int			m_iMovementRemaining;
	CvPathNode*	m_parent;
	CvPathNode*	m_firstChild;
	CvPathNode*	m_prevSibling;
	CvPathNode*	m_nextSibling;
	CvPlot*		m_plot;
	int			m_iCostTo;
	int			m_iCostFrom;
	int			m_iLowestPossibleCostFrom;
	int			m_iPathSeq;	//	Data updated for path generation that matches this seq
	int			m_iEdgesIncluded;	//	Bitmap by direction out
	bool		m_bProcessedAsTerminus;
#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
	int			m_iValidationSeq;
	bool		m_bIsQueued;
#endif
};


//	Helper class to store plot-related info during path generation
class CvPathPlotInfoStore
{
public:
	CvPathPlotInfoStore(CvMap* pMap)
	{
		m_seq = 0;
	}

	~CvPathPlotInfoStore()
	{
	}

	void reset(void)
	{
		m_seq++;
		m_allocationPool.reset();
	}

	CvPathGeneratorPlotInfo*	getPlotInfo(const CvPlot* pPlot)
	{
		//PROFILE_FUNC();

		if ( pPlot->m_pathGenerationSeq != m_seq )
		{
			pPlot->m_currentPathInfo = m_allocationPool.allocateNode();
			pPlot->m_pathGenerationSeq = m_seq;

			pPlot->m_currentPathInfo->pNode = NULL;
			pPlot->m_currentPathInfo->bKnownInvalidNode = false;
		}

		return pPlot->m_currentPathInfo;
	}

private:
	int											m_seq;
	CvAllocationPool<CvPathGeneratorPlotInfo>	m_allocationPool;
};

CvPathGenerator::CvPathGenerator(CvMap* pMap)
{
	m_iSeq = 0;
	m_iTurn = -1;
	m_currentGroupMembershipChecksum = 0;
	m_pReplacedNonTerminalNode = NULL;
	m_pBestTerminalNode = NULL;
	m_pFrom = NULL;
	m_map = pMap;
	//m_priorityQueue = new std::multimap<int, CvPathNode*>();
	m_plotInfo = new CvPathPlotInfoStore(pMap);
	m_nodeAllocationPool = new CvAllocationPool<CvPathNode>();
}

CvPathGenerator::~CvPathGenerator(void)
{
	//SAFE_DELETE(m_priorityQueue);
	SAFE_DELETE(m_plotInfo);
	SAFE_DELETE(m_nodeAllocationPool);
}

void CvPathGenerator::Initialize(HeuristicCost HeuristicFunc, EdgeCost CostFunc, EdgeValidity ValidFunc, TerminusValidity TerminusValidFunc)
{
	m_HeuristicFunc		= HeuristicFunc;
	m_CostFunc			= CostFunc;
	m_ValidFunc			= ValidFunc;
	m_TerminusValidFunc = TerminusValidFunc;
}

#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
bool CvPathGenerator::ValidateTreeInternal(CvPathNode* root, int& iValidationSeq, CvPathNode* unreferencedNode, CvPathNode* referencedNode, int& iQueuedCount)
{
	bool bResult = (referencedNode == NULL || root == referencedNode);
	int	iStartSeq = iValidationSeq;

	if ( root != NULL )
	{
		CvPathNode*	child;

		FAssert(m_nodeAllocationPool->isAllocated(root));
		FAssert(root != unreferencedNode);

		if (root->m_bIsQueued)
		{
			iQueuedCount++;
		}

		for(child = root->m_firstChild; child != NULL; child = child->m_nextSibling)
		{
			FAssert(child->m_iValidationSeq == 0 || iStartSeq - child->m_iValidationSeq >= 0);	//	Couched this way to allow for seq wrapping
			FAssert(child->m_parent == root);

			if ( child->m_prevSibling == NULL )
			{
				FAssert(child == root->m_firstChild);
			}
			else
			{
				FAssert(child == child->m_prevSibling->m_nextSibling);
			}

			child->m_iValidationSeq = ++iValidationSeq;

			bResult |= ValidateTreeInternal(child, iValidationSeq, unreferencedNode, referencedNode, iQueuedCount);
		}
	}

	return bResult;
}

void CvPathGenerator::ValidateTree(CvPathNode* root, CvPathNode* unreferencedNode, CvPathNode* referencedNode)
{
	static int iSeq = 0;
	int iQueuedCount = 0;

	FAssert(ValidateTreeInternal(root, iSeq, unreferencedNode, referencedNode, iQueuedCount));
	FAssert(iQueuedCount == m_priorityQueue.size());
}
#endif

//	Unlink a node (well strictly a subtree)
static void UnlinkNode(CvPathNode* node)
{
	if ( node->m_parent != NULL )
	{
		if ( node->m_parent->m_firstChild == node )
		{
			node->m_parent->m_firstChild = node->m_nextSibling;
		}
		else
		{
			node->m_prevSibling->m_nextSibling = node->m_nextSibling;
		}

		if ( node->m_nextSibling != NULL )
		{
			node->m_nextSibling->m_prevSibling = node->m_prevSibling;
		}
	}
}

//	Link a node into a specified parent
static void LinkNode(CvPathNode* node, CvPathNode* parent)
{
	node->m_prevSibling = NULL;
	
	if ( (node->m_nextSibling = parent->m_firstChild) != NULL )
	{
		parent->m_firstChild->m_prevSibling = node;
	}

	parent->m_firstChild = node;
	node->m_parent = parent;
}

//	Move node to some other point in the tree
static void RelinkNode(CvPathNode* node, CvPathNode* parent)
{
	UnlinkNode(node);
	LinkNode(node, parent);
}
CvPathNode*	CvPathGenerator::allocatePathNode(void)
{
	CvPathNode* node = m_nodeAllocationPool->allocateNode();

	node->m_parent = NULL;
	node->m_firstChild = NULL;
	node->m_nextSibling = NULL;
	node->m_prevSibling = NULL;
	node->m_iPathTurns = 0;
	node->m_iMovementRemaining = 0;
	node->m_plot = NULL;
	node->m_iCostTo = 0;
	node->m_iCostFrom = 0;
	node->m_iEdgesIncluded = 0;
	node->m_bProcessedAsTerminus = false;
	node->m_iPathSeq = -1;
#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
	node->m_iValidationSeq = 0;
	node->m_bIsQueued = false;
#endif

	return node;
}

bool	CvPathGenerator::generatePath(const CvPlot* pFrom, const CvPlot* pTo, CvSelectionGroup* pGroup, int iFlags, int iMaxTurns)
{
	CvPathNode*	root;

	PROFILE_FUNC();

	m_bFoundRoute = false;

	m_generatedPath.Set(NULL);
	m_iSeq++;

	unsigned int iGroupMembershipChecksum = 0;
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	pUnitNode = pGroup->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pGroup->nextUnitNode(pUnitNode);

		CheckSum(iGroupMembershipChecksum, pLoopUnit->getID());
	}

	if ( m_currentGroupMembershipChecksum != iGroupMembershipChecksum || m_iFlags != iFlags )
	{
		CvSelectionGroup::setGroupToCacheFor(pGroup);

		if ( !pGroup->AI_isControlled() )
		{
			m_useAIPathingAlways = getBugOptionBOOL("MainInterface__UseAIPathing", false);
		}
	}

	if ( !m_TerminusValidFunc(pGroup, pTo->getX_INLINE(), pTo->getY_INLINE(), iFlags) )
	{
		return false;
	}

#ifdef TRACE_PATHING
	OutputDebugString(CvString::format("Generate path from (%d,%d) to (%d,%d)\n", pFrom->getX_INLINE(), pFrom->getY_INLINE(), pTo->getX_INLINE(), pTo->getY_INLINE()).c_str());
#endif

	if ( m_iTurn != GC.getGameINLINE().getGameTurn() || m_currentGroupMembershipChecksum != iGroupMembershipChecksum || m_iFlags != iFlags || pFrom != m_pFrom )
	{
		m_nodeAllocationPool->reset();
		m_plotInfo->reset();
		m_iTurn = GC.getGameINLINE().getGameTurn();

		m_pReplacedNonTerminalNode = NULL;
		m_pBestTerminalNode = NULL;
	}

	CvPathGeneratorPlotInfo* plotInfo = m_plotInfo->getPlotInfo(pFrom);

	if ( m_pBestTerminalNode == NULL || m_pBestTerminalNode->m_plot != pTo )
	{
		if ( plotInfo->pNode == NULL )
		{
			root = allocatePathNode();

			root->m_plot = (CvPlot*)pFrom;
			root->m_iCostFrom = m_HeuristicFunc(pGroup, pFrom->getX_INLINE(), pFrom->getY_INLINE(), pTo->getX_INLINE(), pTo->getY_INLINE(), root->m_iLowestPossibleCostFrom);

			plotInfo->pNode = root;
		}
		else
		{
			root = plotInfo->pNode;

			FAssert(pFrom == root->m_plot);
		}

		VALIDATE_TREE(root, m_pReplacedNonTerminalNode, m_pBestTerminalNode);

		if ( m_pBestTerminalNode != NULL && m_pBestTerminalNode->m_plot != pTo )
		{
			if ( m_pBestTerminalNode->m_plot != pFrom )
			{
				//	Put back a non-terminal variant of the old terminal node if we have one
				CvPathGeneratorPlotInfo* oldTerminalPlotInfo = m_plotInfo->getPlotInfo(m_pBestTerminalNode->m_plot);
				
				FAssert(oldTerminalPlotInfo->pNode == m_pBestTerminalNode);

				oldTerminalPlotInfo->pNode = m_pReplacedNonTerminalNode;

				if ( m_pBestTerminalNode->m_parent != NULL )
				{
					FAssert( m_pBestTerminalNode->m_parent->m_firstChild == m_pBestTerminalNode );
					FAssert( m_pBestTerminalNode->m_firstChild == NULL );

					UnlinkNode(m_pBestTerminalNode);
				}

				if ( m_pReplacedNonTerminalNode != NULL )
				{
					if ( m_pReplacedNonTerminalNode->m_parent != NULL )
					{
						LinkNode(m_pReplacedNonTerminalNode, m_pReplacedNonTerminalNode->m_parent);
					}
				}

				VALIDATE_TREE(root, m_pBestTerminalNode, m_pReplacedNonTerminalNode);
			}

			m_pBestTerminalNode = NULL;
			m_pReplacedNonTerminalNode = NULL;
		}

		CvPathGeneratorPlotInfo* terminalPlotInfo = m_plotInfo->getPlotInfo(pTo);

		if ( terminalPlotInfo->pNode != NULL && pFrom != pTo )
		{
			VALIDATE_TREE(root, NULL, terminalPlotInfo->pNode);

			//	Check its within the allowed range - INVALID CHECK AS IT STANDS
			//if ( terminalPlotInfo->pNode->m_iPathTurns > iMaxTurns )
			//{
			//	return false;
			//}

			//	If we already know a route to the terminal plot recalculate its final
			//	edge for the plot now being terminal and seed the best route with it
			m_pReplacedNonTerminalNode = terminalPlotInfo->pNode;

			CvPlot* pParentPlot = m_pReplacedNonTerminalNode->m_parent->m_plot;
			int		iMovementRemaining = m_pReplacedNonTerminalNode->m_parent->m_iMovementRemaining;
			int		iPathTurns = m_pReplacedNonTerminalNode->m_parent->m_iPathTurns;

			UnlinkNode(m_pReplacedNonTerminalNode);

			int iNodeCost;
			int	iEdgeCost = m_CostFunc( this,
										pGroup,
										pParentPlot->getX_INLINE(),
										pParentPlot->getY_INLINE(),
										pTo->getX_INLINE(),
										pTo->getY_INLINE(),
										iFlags,
										iMovementRemaining,
										iPathTurns,
										iNodeCost,
										true);
			
			m_pBestTerminalNode = allocatePathNode();

			m_pBestTerminalNode->m_iCostTo = iEdgeCost + m_pReplacedNonTerminalNode->m_parent->m_iCostTo;
			m_pBestTerminalNode->m_iPathTurns = m_pReplacedNonTerminalNode->m_parent->m_iPathTurns + (m_pReplacedNonTerminalNode->m_parent->m_iMovementRemaining == 0 ? 1 : 0);
			m_pBestTerminalNode->m_plot = (CvPlot*)pTo;
			m_pBestTerminalNode->m_bProcessedAsTerminus = true;

			LinkNode(m_pBestTerminalNode, m_pReplacedNonTerminalNode->m_parent);

			terminalPlotInfo->pNode = m_pBestTerminalNode;
			m_iTerminalNodeCost = iNodeCost;

			VALIDATE_TREE(root, m_pReplacedNonTerminalNode, m_pBestTerminalNode);
		}

		m_iFlags = iFlags;
		m_currentGroupMembershipChecksum = iGroupMembershipChecksum;
		m_pFrom = pFrom;

		root->m_iPathSeq = m_iSeq;

		//	Special-case pFrom == pTo
		if ( pFrom == pTo )
		{
			m_pBestTerminalNode = root;

			root->m_bProcessedAsTerminus = true;
		}
		else
		{
			root->m_bProcessedAsTerminus = false;

			MEMORY_TRACK_EXEMPT();

			m_priorityQueue.push(root);
#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
			root->m_bIsQueued = true;
#endif
		}

		//while(m_priorityQueue->size() > 0)
		while(m_priorityQueue.size() > 0)
		{
			CvPathNode* node;

			//	Dequeue this node now we are dealing with it
			{
				PROFILE("CvPathGenerator::generatePath.popNode");

				node = m_priorityQueue.top();
				m_priorityQueue.pop();
#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
				node->m_bIsQueued = false;
#endif
#ifdef TRACE_PATHING
				OutputDebugString(CvString::format("Dequeue (%d,%d): %d\n", node->m_plot->getX_INLINE(), node->m_plot->getY_INLINE(), node->m_iCostTo).c_str());
#endif
			}

			const CvPlot*				nodePlot = node->m_plot;
			CvPathGeneratorPlotInfo*	nodePlotInfo = m_plotInfo->getPlotInfo(nodePlot);
			int							iPathTurns = node->m_iPathTurns + (node->m_iMovementRemaining == 0 ? 1 : 0);

			FAssert(nodePlotInfo->pNode == node);

			if ( iPathTurns <= iMaxTurns )
			{
				//PROFILE("CvPathGenerator::generatePath.processNode");

				if ( m_pBestTerminalNode != NULL )
				{
					if ( node->m_iCostTo + node->m_iLowestPossibleCostFrom + m_iTerminalNodeCost >= m_pBestTerminalNode->m_iCostTo )
					{
#ifdef TRACE_PATHING
						OutputDebugString(CvString::format("Reject because costTo(%d)+minCostFrom(%d)+finalNodeCost(%d) > existingPathCost(%d)\n",
										  node->m_iCostTo,
										  node->m_iLowestPossibleCostFrom,
										  m_iTerminalNodeCost,
										  m_pBestTerminalNode->m_iCostTo).c_str());
#endif
						//	This branch cannot lead to a better solution than the one we already have
						continue;
					}
				}

				for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
				{
					VALIDATE_TREE(root, m_pReplacedNonTerminalNode, m_pBestTerminalNode);
					VALIDATE_TREE(root, NULL, node);

					CvPlot* pAdjacentPlot = plotDirection(nodePlot->getX_INLINE(), nodePlot->getY_INLINE(), (DirectionTypes)iI);
					if (pAdjacentPlot != NULL)
					{
						CvPathGeneratorPlotInfo*	pAdjacentPlotInfo = m_plotInfo->getPlotInfo(pAdjacentPlot);
						bool isTerminus = (pAdjacentPlot == pTo);

						if ( isTerminus || !pAdjacentPlotInfo->bKnownInvalidNode )
						{
							int iMovementRemaining = node->m_iMovementRemaining;
							bool bValid;
							bool bUseExistingNode = false;

							FAssert( pAdjacentPlotInfo->pNode == NULL || pAdjacentPlotInfo->pNode->m_plot == pAdjacentPlot );
#if 0
							//	If we're inserting a terminal node over an existing non-terminal node trim out
							//	the non-terminal version (it will be replaced on next generation to a different target)
							if ( pAdjacentPlotInfo->pNode != NULL && pAdjacentPlotInfo->pNode->m_bProcessedAsTerminus != isTerminus )
							{
								FAssert(pAdjacentPlot != pFrom);
								FAssert(isTerminus);
								FAssert(m_pBestTerminalNode == NULL);
								FAssert(m_pReplacedNonTerminalNode == NULL);

								m_pReplacedNonTerminalNode = pAdjacentPlotInfo->pNode;
								pAdjacentPlotInfo->pNode = NULL;

								UnlinkNode(m_pReplacedNonTerminalNode);

								VALIDATE_TREE(root, m_pReplacedNonTerminalNode, NULL);
							}
#endif

							if ( pAdjacentPlotInfo->pNode != NULL && (pAdjacentPlotInfo->pNode->m_iEdgesIncluded & (1<<iI)) != 0 )
							{
								//	If the adjacent node already has a tree node associated with it and this edge
								//	has already been taken into account then eitehr:
								//	1) This edge is the one that is to the lowest cost route to the adjacent node, in which case use it
								//	2) This edge was not the lowest cost route to the adjactn route in which case we don't need to
								//	   process it

								if ( pAdjacentPlotInfo->pNode->m_parent == node )
								{
									bValid = true;
									bUseExistingNode = true;
								}
								else
								{
#ifdef TRACE_PATHING
									if ( pAdjacentPlotInfo->pNode->m_parent != NULL )
									{
										OutputDebugString(CvString::format("\tReject (%d,%d) - lower cost route (%d) known from (%d,%d) [%d]\n", pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE(), pAdjacentPlotInfo->pNode->m_iCostTo, pAdjacentPlotInfo->pNode->m_parent->m_plot->getX_INLINE(), pAdjacentPlotInfo->pNode->m_parent->m_plot->getY_INLINE(), pAdjacentPlotInfo->pNode->m_parent->m_iCostTo).c_str());
									}
#endif
									continue;
								}
							}
							else
							{
								bValid = m_ValidFunc(pGroup,
													 nodePlot->getX_INLINE(),
													 nodePlot->getY_INLINE(),
													 pAdjacentPlot->getX_INLINE(),
													 pAdjacentPlot->getY_INLINE(),
													 iFlags,
													 isTerminus,
													 iMovementRemaining,
													 iPathTurns,
													 pAdjacentPlotInfo->bKnownInvalidNode);
							}

							if ( bValid )
							{
								CvPathNode* newNode;

								if ( bUseExistingNode )
								{
									newNode = pAdjacentPlotInfo->pNode;

									if ( m_pBestTerminalNode != NULL )
									{
										int iMinFinalCost = newNode->m_iCostTo + (isTerminus ? 0 : m_iTerminalNodeCost);

										if ( iMinFinalCost > m_pBestTerminalNode->m_iCostTo )
										{
											//	This branch cannot lead to a better solution than the one we already have
#ifdef TRACE_PATHING
											OutputDebugString(CvString::format("\tReject (%d,%d): min final (existing node) cost %d > %d\n", pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE(), iMinFinalCost, m_pBestTerminalNode->m_iCostTo).c_str());
#endif
											continue;
										}
									}
								}
								else
								{
									int iNodeCost;
									int	iEdgeCost = m_CostFunc( this,
																pGroup,
																nodePlot->getX_INLINE(),
																nodePlot->getY_INLINE(),
																pAdjacentPlot->getX_INLINE(),
																pAdjacentPlot->getY_INLINE(),
																iFlags,
																iMovementRemaining,
																iPathTurns,
																iNodeCost,
																(pAdjacentPlot == pTo));
									if ( m_pBestTerminalNode != NULL )
									{
										int iMinFinalCost = node->m_iCostTo + std::max(iEdgeCost, node->m_iLowestPossibleCostFrom) + (isTerminus ? 0 : m_iTerminalNodeCost);

										if ( iMinFinalCost > m_pBestTerminalNode->m_iCostTo )
										{
											//	This branch cannot lead to a better solution than the one we already have
#ifdef TRACE_PATHING
											OutputDebugString(CvString::format("\tReject (%d,%d): min final cost %d > %d\n", pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE(), iMinFinalCost, m_pBestTerminalNode->m_iCostTo).c_str());
#endif
											continue;
										}
									}

									VALIDATE_TREE(root, NULL, NULL);

									if ( pAdjacentPlotInfo->pNode == NULL )
									{
										//	Not yet visited - queue a new node for it
										newNode = allocatePathNode();

										VALIDATE_TREE(root, newNode, NULL);

										LinkNode(newNode, node);

										VALIDATE_TREE(root, NULL, newNode);
									}
									else
									{
										newNode = pAdjacentPlotInfo->pNode;

										//	Visited previously - is this route here superior to the one we already
										//	have
										if ( pAdjacentPlotInfo->pNode->m_iCostTo > node->m_iCostTo + iEdgeCost )
										{
											//	For now just trim the old tree rooted here and recalculate it,
											//	but (TODO) adjust the existing tree nodes if end turn boundaries
											//	align
											newNode = pAdjacentPlotInfo->pNode;
										}
										else
										{
											//	Equal cost is considered in precisely one case - retracing the (best) steps
											//	taken by a previous path
											if ( pAdjacentPlotInfo->pNode->m_iCostTo < node->m_iCostTo + iEdgeCost ||
												 pAdjacentPlotInfo->pNode->m_iPathSeq == m_iSeq ||
												 newNode->m_parent != node )
											{
#ifdef TRACE_PATHING
												OutputDebugString(CvString::format("\tReject (%d,%d): %d > %d\n", pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE(), node->m_iCostTo + iEdgeCost, newNode->m_iCostTo).c_str());
#endif
												continue;
											}
										}

										RelinkNode(newNode, node);

										VALIDATE_TREE(root, NULL, newNode);
									}

									pAdjacentPlotInfo->pNode = newNode;

									newNode->m_iCostTo = node->m_iCostTo + iEdgeCost;
									newNode->m_iMovementRemaining = iMovementRemaining;
									newNode->m_iPathTurns = node->m_iPathTurns + (node->m_iMovementRemaining == 0 ? 1 : 0);
									newNode->m_plot = pAdjacentPlot;

									if ( isTerminus )
									{
										m_iTerminalNodeCost = iNodeCost;
									}

									VALIDATE_TREE(root, m_pReplacedNonTerminalNode, m_pBestTerminalNode);
								}

								newNode->m_iEdgesIncluded |= (1<<iI);
								newNode->m_iCostFrom = m_HeuristicFunc(pGroup, pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE(), pTo->getX_INLINE(), pTo->getY_INLINE(), newNode->m_iLowestPossibleCostFrom);

								//	If this reaches the destination then set current least cost info
								if ( isTerminus )
								{
									newNode->m_bProcessedAsTerminus = true;

									if ( m_pBestTerminalNode == NULL || pAdjacentPlotInfo->pNode->m_iCostTo < m_pBestTerminalNode->m_iCostTo )
									{
#ifdef TRACE_PATHING
										OutputDebugString(CvString::format("New best cost to terminus @(%d,%d): %d\n", pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE(), newNode->m_iCostTo).c_str());
#endif
										m_pBestTerminalNode = newNode;

										//	TODO - maybe back-propagate actual costs to neighbour heuristic costs at this point
									}
#ifdef TRACE_PATHING
									else
									{
										OutputDebugString(CvString::format("New route to terminus @(%d,%d): %d, greater than existing best cost %d\n", pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE(), newNode->m_iCostTo, m_pBestTerminalNode->m_iCostTo).c_str());
									}
#endif
								}
								//	Queue up the node unless it's the terminus
								else
								{
									newNode->m_bProcessedAsTerminus = false;

									if ( newNode->m_iPathSeq != m_iSeq )
									{
										PROFILE("CvPathGenerator::generatePath.insertNode");

#ifdef TRACE_PATHING
										OutputDebugString(CvString::format("\tQueue (%d,%d): %d\n", pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE(), newNode->m_iCostTo).c_str());
#endif
#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
										newNode->m_bIsQueued = true;
#endif
										newNode->m_iPathSeq = m_iSeq;

										{
											MEMORY_TRACK_EXEMPT();

											m_priorityQueue.push(newNode);
										}
									}
								}
							}
						}
					}
				}
			}
		}

		VALIDATE_TREE(root, m_pReplacedNonTerminalNode, m_pBestTerminalNode);
	}
	else
	{
		root = plotInfo->pNode;

		FAssert(pFrom == root->m_plot);

		VALIDATE_TREE(root, m_pReplacedNonTerminalNode, m_pBestTerminalNode);
	}

	if ( m_pBestTerminalNode != NULL )
	{
		PROFILE("CvPathGenerator::generatePath.finalizePath");

		//	Relink the optimal path so that the best branch is the first child at each step
		CvPathNode*	node;
		CvPathNode*	descendantNode = NULL;

		FAssert( m_pBestTerminalNode->m_bProcessedAsTerminus );

		for(node = m_pBestTerminalNode; node != NULL; node = node->m_parent)
		{
			if ( descendantNode != NULL && descendantNode != node->m_firstChild )
			{
				FAssert(descendantNode->m_prevSibling != NULL);

				UnlinkNode(descendantNode);
				LinkNode(descendantNode, node);
			}

			descendantNode = node;
		}

		FAssert(descendantNode == root);

		m_generatedPath.Set(descendantNode);

		VALIDATE_TREE(root, m_pReplacedNonTerminalNode, m_pBestTerminalNode);
		return true;
	}
	else
	{
		return false;
	}
}

bool CvPathGenerator::CvPathNodeComparer::operator() (const CvPathNode*& lhs, const CvPathNode*&rhs) const
{
	return lhs->m_iCostFrom + lhs->m_iCostTo > rhs->m_iCostFrom + rhs->m_iCostTo;
}

	
CvPath&	CvPathGenerator::getLastPath(void)
{
	return m_generatedPath;
}

bool CvPathGenerator::generatePathForHypotheticalUnit(const CvPlot* pFrom, const CvPlot* pTo, PlayerTypes ePlayer, UnitTypes eUnit, int iFlags, int iMaxTurns)
{
	bool bResult;
	CvUnit*	pTempUnit = GET_PLAYER(ePlayer).getTempUnit(eUnit, pFrom->getX_INLINE(), pFrom->getY_INLINE());

	pTempUnit->finishMoves();

	//	Force flush of pathing cache since although we are using the same (pseudo) group
	//	all the time its starting plot varies
	m_currentGroupMembershipChecksum = 0;

	bResult = generatePath(pFrom, pTo, pTempUnit->getGroup(), iFlags, iMaxTurns);

	GET_PLAYER(ePlayer).releaseTempUnit();

	return bResult;
}

CvPath::const_iterator::const_iterator(CvPathNode* cursorNode)
{
	m_cursorNode = cursorNode;
}

CvPath::const_iterator& CvPath::const_iterator::operator++()
{
	if ( m_cursorNode != NULL )
	{	
		if ( m_cursorNode->m_bProcessedAsTerminus )
		{
			m_cursorNode = NULL;
		}
		else
		{
			m_cursorNode = m_cursorNode->m_firstChild;
		}
	}

	return (*this);
}

bool CvPath::const_iterator::operator==(const_iterator& other)
{
	return m_cursorNode == other.m_cursorNode;
}

bool CvPath::const_iterator::operator!=(const_iterator& other)
{
	return m_cursorNode != other.m_cursorNode;
}

CvPlot*	CvPath::const_iterator::plot(void)
{
	return (m_cursorNode == NULL ? NULL : m_cursorNode->m_plot);
}

int	CvPath::const_iterator::turn(void)
{
	return (m_cursorNode == NULL ? NULL : m_cursorNode->m_iPathTurns);
}

CvPath::CvPath()
{
	m_startNode = NULL;
}

void CvPath::Set(CvPathNode* startNode)
{
	m_startNode = startNode;
	m_endNode = m_startNode;

	if ( startNode != NULL )
	{
#ifdef TRACE_PATHING
		OutputDebugString("Path generated:\n");
#endif

		while(m_endNode != NULL && m_endNode->m_firstChild != NULL && !m_endNode->m_bProcessedAsTerminus )
		{
#ifdef TRACE_PATHING
			OutputDebugString(CvString::format("\t->(%d,%d)\n", m_endNode->m_plot->getX_INLINE(), m_endNode->m_plot->getY_INLINE()).c_str());
#endif
			m_endNode = m_endNode->m_firstChild;
		}
	}
}

CvPath::const_iterator CvPath::begin(void)
{
	return CvPath::const_iterator(m_startNode);
}

CvPath::const_iterator CvPath::end(void)
{
	return CvPath::const_iterator(NULL);
}

int	CvPath::length(void) const
{
	return (m_endNode == NULL ? MAX_INT : m_endNode->m_iPathTurns);
}

CvPlot*	CvPath::lastPlot(void) const
{
	return (m_endNode == NULL ? NULL : m_endNode->m_plot);
}
	
bool	CvPath::containsEdge(const CvPlot* pFromPlot, const CvPlot* pToPlot) const
{
	CvPathNode*	pNode = m_startNode;

	while( pNode != NULL && pNode->m_firstChild != NULL )
	{
		if ( pNode->m_plot == pFromPlot && pNode->m_firstChild->m_plot == pToPlot )
		{
			return true;
		}

		pNode = pNode->m_firstChild;
	}

	return false;
}
	
bool	CvPath::containsNode(const CvPlot* pPlot) const
{
	CvPathNode*	pNode = m_startNode;

	while( pNode != NULL )
	{
		if ( pNode->m_plot == pPlot )
		{
			return true;
		}

		pNode = pNode->m_firstChild;
	}

	return false;
}

int		CvPath::movementRemaining(void) const
{
	if ( m_endNode == NULL )
	{
		return 0;
	}
	else
	{
		return m_endNode->m_iMovementRemaining;
	}
}
