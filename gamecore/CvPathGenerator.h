#pragma once
#include <queue>
#include "FProfiler.h"

//	Forward declarations of helper classes
class CvPathNode;
class CvPathPlotInfoStore;
class CvPathGenerator;

// Function prototype for Cost and Validity functions
typedef int(*HeuristicCost)(CvSelectionGroup* pGroup, int iFromX, int iFromY, int iToX, int iToY, int& iLimitCost);
typedef int(*EdgeCost)(CvPathGenerator* generator, CvSelectionGroup* pGroup, int iFromX, int iFromY, int iToX, int iToY, int iFlags, int& iMovementRemaining, int iPathTurns, int& iToNodeCost, bool bIsTerminalNode);
typedef bool(*EdgeValidity)(CvSelectionGroup* pGroup, int iFromX, int iFromY, int iToX, int iToY, int iFlags, bool isTerminus, int iMovementRemaining, int iPathTurns, bool& bToNodeInvalidity);
typedef bool(*TerminusValidity)(CvSelectionGroup* pGroup, int iToX, int iToY, int iFlags);

//	Define this for extensive inline validation (DEBUG at least)
#ifdef _DEBUG
//#define	DYNAMIC_PATH_STRUCTURE_VALIDATION
#endif

class CvPath
{
friend CvPathGenerator;
public:
	class const_iterator
	{
	friend CvPath;
	protected:
		const_iterator(CvPathNode* cursorNode);

	public:
		const_iterator& operator++();

		bool operator==(const_iterator& other);

		bool operator!=(const_iterator& other);

		CvPlot*	plot(void);
		int		turn(void);

	private:
		CvPathNode*	m_cursorNode;
	};

protected:
	CvPath();
	
	void Set(CvPathNode* startNode);

public:
	const_iterator begin(void);
	const_iterator end(void);

	int	length(void) const;
	CvPlot*	lastPlot(void) const;
	bool	containsEdge(const CvPlot* pFromPlot, const CvPlot* pToPlot) const;
	bool	containsNode(const CvPlot* pPlot) const;
	int		movementRemaining(void) const;

private:
	CvPathNode* m_startNode;
	CvPathNode* m_endNode;
};

//	Helper class to manage allocation of tree nodes efficiently
typedef std::vector<CvPathNode> PoolBucket;

template <class T>
class CvAllocationPool
{
private:
	class AllocationType
	{
	public:
		AllocationType() {}

#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
		int	m_seq;
#endif
		T	m_data;
	};

public:
	CvAllocationPool()
	{
		m_nextBucketToAllocate = 0;
#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
		m_iSeq = 0;
#endif
		reset();
	}
	~CvAllocationPool()
	{
		while(m_nextBucketToAllocate-- > 0)
		{
			SAFE_DELETE(m_pool[m_nextBucketToAllocate]);
		}
	}

	T*		allocateNode(void)
	{
		//PROFILE("CvAllocationPool.allocateNode");

		int iCurrentBucketCapacity;

		if ( m_nextBucketIndex == 0 )
		{
			iCurrentBucketCapacity = 0;
		}
		else
		{
			iCurrentBucketCapacity = m_pool[m_nextBucketIndex-1]->capacity();
		}

		if ( m_nextIndex >= iCurrentBucketCapacity )
		{
			if ( m_nextBucketToAllocate == m_nextBucketIndex )
			{
				MEMORY_TRACK_EXEMPT();

				std::vector<AllocationType>* newBucket = new std::vector<AllocationType>();

				newBucket->reserve(range(iCurrentBucketCapacity*2, 16, 1024));
				m_pool.push_back(newBucket);

				m_nextBucketToAllocate++;
			}

			m_nextIndex = 0;
			m_nextBucketIndex++;
		}

		AllocationType& allocated = (*m_pool[m_nextBucketIndex-1])[m_nextIndex++];

#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
		allocated.m_seq = m_iSeq;
#endif
		return &allocated.m_data;
	}

	void	reset(void)
	{
		m_nextIndex = 0;
		m_nextBucketIndex = 0;
#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
		m_iSeq++;
#endif

		//	No point freeing the allocated pools - we'll just need them again
		//	for another use
	}

#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
	bool	isAllocated(T* data) const
	{
		AllocationType*	alloc = (AllocationType*)(((int*)data) - 1);

		return (m_iSeq == alloc->m_seq);
	}
#endif

private:
	int											m_nextBucketIndex;
	int											m_nextIndex;
	int											m_nextBucketToAllocate;
#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
	int											m_iSeq;
#endif
	std::vector<std::vector<AllocationType>*>	m_pool;
};

class CvPathGenerator
{
public:
	CvPathGenerator(CvMap* pMap);
	virtual ~CvPathGenerator(void);

	void Initialize(HeuristicCost HeuristicFunc, EdgeCost CostFunc, EdgeValidity ValidFunc, TerminusValidity TerminusValidFunc);
	bool generatePath(const CvPlot* pFrom, const CvPlot* pTo, CvSelectionGroup* pGroup, int iFlags, int iMaxTurns);
	bool generatePathForHypotheticalUnit(const CvPlot* pFrom, const CvPlot* pTo, PlayerTypes ePlayer, UnitTypes eUnit, int iFlags, int iMaxTurns);

	inline bool useAIPathingAlways(void) const { return m_useAIPathingAlways; };

	CvPath&	getLastPath(void);
private:
	class CvPathNodeComparer
	{
	public:
		CvPathNodeComparer()
		{
		}
		bool operator() (const CvPathNode*& lhs, const CvPathNode*&rhs) const;
	};

	CvPathNode*	allocatePathNode(void);

#ifdef DYNAMIC_PATH_STRUCTURE_VALIDATION
	bool ValidateTreeInternal(CvPathNode* root, int& iValidationSeq, CvPathNode* unreferencedNode, CvPathNode* referencedNode, int& iQueuedCount);
	void ValidateTree(CvPathNode* root, CvPathNode* unreferencedNode, CvPathNode* referencedNode);
#endif

private:
	CvMap*								m_map;
	std::priority_queue<CvPathNode*,std::vector<CvPathNode*>,CvPathNodeComparer> m_priorityQueue;
	CvPathPlotInfoStore*				m_plotInfo;
	CvAllocationPool<CvPathNode>*		m_nodeAllocationPool;
	bool								m_bFoundRoute;
	int									m_iTerminalNodeCost;
	CvPathNode*							m_pBestTerminalNode;
	CvPathNode*							m_pReplacedNonTerminalNode;
	CvPath								m_generatedPath;
	unsigned int						m_currentGroupMembershipChecksum;
	const CvPlot*						m_pFrom;
	int									m_iFlags;
	int									m_iTurn;
	int									m_iSeq;
	bool								m_useAIPathingAlways;
	//	Callbacks
	HeuristicCost						m_HeuristicFunc;
	EdgeCost							m_CostFunc;
	EdgeValidity						m_ValidFunc;
	TerminusValidity					m_TerminusValidFunc;
};
