//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:	CvOutcomeMission.cpp
//
//  PURPOSE: A mission that has a cost and a result depending on an outcome list
//
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "CvOutcomeMission.h"

#include "CvGameObject.h"

#include "CvXMLLoadUtility.h"
#include "CvDLLXMLIFaceBase.h"

#include <boost/bind.hpp>

CvOutcomeMission::CvOutcomeMission() :
m_eMission(NO_MISSION),
m_iCost(0),
m_bKill(true),
m_ePayerType(NO_GAMEOBJECT)
{
}

int CvOutcomeMission::getCost()
{
	return m_iCost;
}

MissionTypes CvOutcomeMission::getMission()
{
	return m_eMission;
}

CvOutcomeList* CvOutcomeMission::getOutcomeList()
{
	return &m_OutcomeList;
}

CvProperties* CvOutcomeMission::getPropertyCost()
{
	return &m_PropertyCost;
}

bool CvOutcomeMission::isKill()
{
	return m_bKill;
}

GameObjectTypes CvOutcomeMission::getPayerType()
{
	return m_ePayerType;
}

void callSetPayer(CvGameObject* pObject, CvGameObject** ppPayer)
{
	*ppPayer = pObject;
}

bool CvOutcomeMission::isPossible(CvUnit* pUnit)
{
	CvPlayer* pOwner = &GET_PLAYER(pUnit->getOwnerINLINE());
	if (pOwner->getGold() < getCost())
	{
		return false;
	}

	if (!getOutcomeList()->isPossible(*pUnit))
	{
		return false;
	}

	if (!getPropertyCost()->isEmpty())
	{
		CvGameObject* pPayer = NULL;
		if ((m_ePayerType == NO_GAMEOBJECT) || (m_ePayerType == GAMEOBJECT_UNIT))
		{
			pPayer = pUnit->getGameObject();
		}
		else
		{
			pUnit->getGameObject()->foreach(m_ePayerType, boost::bind(callSetPayer, _1, &pPayer));
		}

		if (!pPayer)
		{
			return false;
		}

		if (! (*(pPayer->getProperties()) > m_PropertyCost ))
		{
			return false;
		}
	}

	return true;
}

void CvOutcomeMission::buildDisplayString(CvWStringBuffer &szBuffer, CvUnit *pUnit)
{
	if ((m_iCost != 0) || !m_PropertyCost.isEmpty())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(L"Cost: ");
		if (m_iCost != 0)
		{
			CvWString szTempBuffer;
			szTempBuffer.Format(L"%d%c", m_iCost, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
			szBuffer.append(szTempBuffer);
		}
		
		m_PropertyCost.buildCompactChangesString(szBuffer);
	}

	if (m_bKill)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CONSUME_UNIT"));
	}

	m_OutcomeList.buildDisplayString(szBuffer, *pUnit);
}

void CvOutcomeMission::execute(CvUnit* pUnit)
{
	CvPlayer* pOwner = &GET_PLAYER(pUnit->getOwnerINLINE());
	pOwner->changeGold(-m_iCost);

	getOutcomeList()->execute(*pUnit);

	if (!getPropertyCost()->isEmpty())
	{
		CvGameObject* pPayer = NULL;
		if ((m_ePayerType == NO_GAMEOBJECT) || (m_ePayerType == GAMEOBJECT_UNIT))
		{
			pPayer = pUnit->getGameObject();
		}
		else
		{
			pUnit->getGameObject()->foreach(m_ePayerType, boost::bind(callSetPayer, _1, &pPayer));
		}

		if (pPayer)
		{
			pPayer->getProperties()->subtractProperties(&m_PropertyCost);
		}
	}

	pUnit->finishMoves();
}

void CvOutcomeMission::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eMission);
	pStream->Read(&m_bKill);
	pStream->Read(&m_iCost);
	pStream->Read((int*)&m_ePayerType);
	m_PropertyCost.read(pStream);
	m_OutcomeList.read(pStream);
}

void CvOutcomeMission::write(FDataStreamBase *pStream)
{
	pStream->Write((int)m_eMission);
	pStream->Write(m_bKill);
	pStream->Write(m_iCost);
	pStream->Write((int)m_ePayerType);
	m_PropertyCost.write(pStream);
	m_OutcomeList.write(pStream);
}

bool CvOutcomeMission::read(CvXMLLoadUtility *pXML)
{
	CvString szTextVal;
	pXML->GetChildXmlValByName(szTextVal, "MissionType");
	m_eMission = (MissionTypes) pXML->FindInInfoClass(szTextVal);
	pXML->GetChildXmlValByName(&m_bKill, "bKill", true);
	pXML->GetChildXmlValByName(&m_iCost, "iCost");
	pXML->GetChildXmlValByName(szTextVal, "PayerType");
	m_ePayerType = (GameObjectTypes) pXML->FindInInfoClass(szTextVal);
	m_PropertyCost.read(pXML, "PropertyCost");
	m_OutcomeList.read(pXML, "ActionOutcomes");
	return true;
}

void CvOutcomeMission::copyNonDefaults(CvOutcomeMission *pOutcomeMission, CvXMLLoadUtility *pXML)
{
	if (m_eMission == NO_MISSION)
	{
		m_eMission = pOutcomeMission->getMission();
	}

	if (m_bKill)
	{
		m_bKill = pOutcomeMission->isKill();
	}

	if (m_iCost == 0)
	{
		m_iCost = pOutcomeMission->getCost();
	}

	if (m_ePayerType == NO_GAMEOBJECT)
	{
		m_ePayerType = pOutcomeMission->getPayerType();
	}

	m_PropertyCost.copyNonDefaults(pOutcomeMission->getPropertyCost(), pXML);
	m_OutcomeList.copyNonDefaults(pOutcomeMission->getOutcomeList(), pXML);
}