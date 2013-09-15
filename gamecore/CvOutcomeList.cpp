//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:	CvOutcomeList.cpp
//
//  PURPOSE: A list of possible outcomes with a relative chance
//
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "CvOutcomeList.h"
//#include "CvGlobals.h"
//#include "CvArtFileMgr.h"
#include "CvXMLLoadUtility.h"
#include "CvDLLXMLIFaceBase.h"
//#include "CvGameTextMgr.h"
//#include "CvGameCoreUtils.h"
#include <set>


CvOutcome* CvOutcomeList::getOutcome(int index)
{
	FAssert(0 <= index);
	FAssert(index < (int)m_aOutcome.size());
	return &(m_aOutcome[index]);
}

int CvOutcomeList::getNumOutcomes() const
{
	return m_aOutcome.size();
}

bool CvOutcomeList::isPossible(const CvUnit &kUnit) const
{
	int iNum = getNumOutcomes();
	if (iNum <= 0)
		return false;

	for (int i=0; i<iNum; i++)
	{
		if (m_aOutcome[i].isPossible(kUnit))
		{
			return true;
		}
	}

	return false;
}

bool CvOutcomeList::isPossibleSomewhere(const CvUnit &kUnit) const
{
	int iNum = getNumOutcomes();
	if (iNum <= 0)
		return false;

	for (int i=0; i<iNum; i++)
	{
		if (m_aOutcome[i].isPossibleSomewhere(kUnit))
		{
			return true;
		}
	}

	return false;
}

bool CvOutcomeList::isPossibleInPlot(const CvUnit &kUnit, const CvPlot& kPlot, bool bForTrade) const
{
	int iNum = getNumOutcomes();
	if (iNum <= 0)
		return false;

	for (int i=0; i<iNum; i++)
	{
		if (m_aOutcome[i].isPossibleInPlot(kUnit, kPlot, bForTrade))
		{
			return true;
		}
	}

	return false;
}

bool CvOutcomeList::isEmpty() const
{
	return m_aOutcome.empty();
}

void CvOutcomeList::clear()
{
	m_aOutcome.clear();
}

void insertReplaceOutcomesRecursive(std::set<OutcomeTypes>& aeReplacedOutcomes, OutcomeTypes eOutcome)
{
	CvOutcomeInfo& kInfo = GC.getOutcomeInfo(eOutcome);
	int iNumReplaced = kInfo.getNumReplaceOutcomes();
	for (int j=0; j<iNumReplaced; j++)
	{
		OutcomeTypes eReplOutcome = kInfo.getReplaceOutcome(j);
		aeReplacedOutcomes.insert(eReplOutcome);
		insertReplaceOutcomesRecursive(aeReplacedOutcomes, eReplOutcome);
	}
}

bool CvOutcomeList::execute(CvUnit &kUnit, PlayerTypes eDefeatedUnitPlayer, UnitTypes eDefeatedUnitType)
{
	PROFILE_FUNC();

	std::vector<std::pair<CvOutcome*, int> > apOutcome;
	std::set<OutcomeTypes> aeReplacedOutcomes;
	int iChanceSum = 0;
	for (int i=0; i<getNumOutcomes(); i++)
	{
		CvOutcome* pOutcome = getOutcome(i);
		if (pOutcome->isPossible(kUnit))
		{
			int iChance = pOutcome->getChance(kUnit);
			iChanceSum += iChance;
			apOutcome.push_back(std::pair<CvOutcome*,int>(pOutcome, iChance));
			insertReplaceOutcomesRecursive(aeReplacedOutcomes, pOutcome->getType());
		}
	}
	
	for (int i=(int)apOutcome.size()-1; i>=0; i--)
	{
		if (aeReplacedOutcomes.find(apOutcome[i].first->getType()) != aeReplacedOutcomes.end())
		{
			iChanceSum -= apOutcome[i].second;
			apOutcome.erase(apOutcome.begin()+i);
		}
	}


	if (iChanceSum <= 0)
	{
		return false;
	}

	// Use absolute percentages when chance sum is smaller than 100
	if (iChanceSum < 100)
	{
		iChanceSum = 100;
	}

	int iRoll = GC.getGameINLINE().getSorenRandNum( iChanceSum, "Outcome roll");
	iChanceSum = 0;

	for (int i=0; i<(int)apOutcome.size(); i++)
	{
		iChanceSum += apOutcome[i].second;
		if (iRoll < iChanceSum)
		{
			apOutcome[i].first->execute(kUnit, eDefeatedUnitPlayer, eDefeatedUnitType);
			return true;
		}
	}

	// Nothing happens
	return false;
}

int CvOutcomeList::AI_getValueInPlot(const CvUnit& kUnit, const CvPlot& kPlot, bool bForTrade)
{
	std::vector<std::pair<CvOutcome*, int> > apOutcome;
	std::set<OutcomeTypes> aeReplacedOutcomes;
	int iChanceSum = 0;
	for (int i=0; i<getNumOutcomes(); i++)
	{
		CvOutcome* pOutcome = getOutcome(i);
		if (pOutcome->isPossibleInPlot(kUnit, kPlot, bForTrade))
		{
			int iChance = pOutcome->getChance(kUnit);
			iChanceSum += iChance;
			apOutcome.push_back(std::pair<CvOutcome*,int>(pOutcome, iChance));
			insertReplaceOutcomesRecursive(aeReplacedOutcomes, pOutcome->getType());
		}
	}
	
	for (int i=(int)apOutcome.size()-1; i>=0; i--)
	{
		if (aeReplacedOutcomes.find(apOutcome[i].first->getType()) != aeReplacedOutcomes.end())
		{
			iChanceSum -= apOutcome[i].second;
			apOutcome.erase(apOutcome.begin()+i);
		}
	}


	if (iChanceSum <= 0)
	{
		return 0;
	}

	// Use absolute percentages when chance sum is smaller than 100
	if (iChanceSum < 100)
	{
		iChanceSum = 100;
	}

	int iValue = 0;

	for (int i=0; i<(int)apOutcome.size(); i++)
	{
		iValue += ((100 * apOutcome[i].second)/iChanceSum) * apOutcome[i].first->AI_getValueInPlot(kUnit, kPlot, bForTrade);
	}

	return iValue / 100;
}

void CvOutcomeList::read(FDataStreamBase *pStream)
{
	int num;
	pStream->Read(&num);
	for (int i = 0; i < num; i++)
	{
		m_aOutcome.push_back(CvOutcome());
		m_aOutcome[i].read(pStream);
	}
}

void CvOutcomeList::write(FDataStreamBase *pStream)
{
	int num = getNumOutcomes();
	pStream->Write(num);
	for (int i = 0; i < num; i++)
	{
		getOutcome(i)->write(pStream);
	}
}

bool CvOutcomeList::read(CvXMLLoadUtility* pXML, const TCHAR* szTagName)
{
	if(gDLL->getXMLIFace()->SetToChildByTagName( pXML->GetXML(), szTagName))
	{
		if(gDLL->getXMLIFace()->SetToChild( pXML->GetXML() ))
		{

			if (gDLL->getXMLIFace()->LocateFirstSiblingNodeByTagName(pXML->GetXML(), "Outcome"))
			{
				do
				{
					m_aOutcome.push_back(CvOutcome());
					m_aOutcome[m_aOutcome.size()-1].read(pXML);
				} while(gDLL->getXMLIFace()->NextSibling(pXML->GetXML()));
			}
			gDLL->getXMLIFace()->SetToParent( pXML->GetXML() );
		}
		gDLL->getXMLIFace()->SetToParent( pXML->GetXML() );
	}

	return true;
}

void CvOutcomeList::copyNonDefaults(CvOutcomeList* pOutcomeList, CvXMLLoadUtility* pXML )
{
	if (isEmpty())
	{
		int num = pOutcomeList->getNumOutcomes();
		for (int index = 0; index < num; index++)
		{
			m_aOutcome.push_back(CvOutcome(*(pOutcomeList->getOutcome(index))));
		}
	}
}

void CvOutcomeList::getCheckSum(unsigned int& iSum)
{
	int num = getNumOutcomes();
	for (int index = 0; index < num; index++)
	{
		m_aOutcome[index].getCheckSum(iSum);
	}
}

void CvOutcomeList::buildDisplayString(CvWStringBuffer& szBuffer, const CvUnit& kUnit)
{
	std::vector<std::pair<CvOutcome*, int> > apOutcome;
	std::set<OutcomeTypes> aeReplacedOutcomes;
	int iChanceSum = 0;

	for (int i=0; i<getNumOutcomes(); i++)
	{
		CvOutcome* pOutcome = getOutcome(i);
		if (pOutcome->isPossible(kUnit))
		{
			int iChance = pOutcome->getChance(kUnit);
			iChanceSum += iChance;
			apOutcome.push_back(std::pair<CvOutcome*,int>(pOutcome, iChance));
			insertReplaceOutcomesRecursive(aeReplacedOutcomes, pOutcome->getType());
		}
	}
	
	for (int i=(int)apOutcome.size()-1; i>=0; i--)
	{
		if (aeReplacedOutcomes.find(apOutcome[i].first->getType()) != aeReplacedOutcomes.end())
		{
			iChanceSum -= apOutcome[i].second;
			apOutcome.erase(apOutcome.begin()+i);
		}
	}

	if (iChanceSum <= 0)
	{
		return;
	}

	// Use absolute percentages when chance sum is smaller than 100
	if (iChanceSum < 100)
	{
		iChanceSum = 100;
	}

	for (int i=0; i<(int)apOutcome.size(); i++)
	{
		szBuffer.append(NEWLINE);
		CvWString s;
		s.Format(L"%d%%: ",(100 * apOutcome[i].second)/iChanceSum);
		szBuffer.append(s);
		apOutcome[i].first->buildDisplayString(szBuffer, kUnit);
	}
}