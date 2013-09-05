//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvPropertyPropagator.cpp
//
//  PURPOSE: Propagator of generic properties for Civ4 classes
//
//------------------------------------------------------------------------------------------------

#include "CvGameCoreDLL.h"
#include "CvPropertyPropagator.h"
#include "CvXMLLoadUtility.h"
#include "CvDLLXMLIFaceBase.h"

CvPropertyPropagator::CvPropertyPropagator() : 
									m_eProperty(NO_PROPERTY),
									m_eObjectType(NO_GAMEOBJECT),
									m_eRelation(NO_RELATION),
									m_iRelationData(0),
									m_eTargetRelation(NO_RELATION),
									m_eTargetObjectType(NO_GAMEOBJECT),
									m_iTargetRelationData(0),
									m_pExprActive(NULL)
{
}

CvPropertyPropagator::CvPropertyPropagator(PropertyTypes eProperty) :
									m_eProperty(eProperty),
									m_eObjectType(NO_GAMEOBJECT),
									m_eRelation(NO_RELATION),
									m_iRelationData(0),
									m_eTargetRelation(NO_RELATION),
									m_eTargetObjectType(NO_GAMEOBJECT),
									m_iTargetRelationData(0),
									m_pExprActive(NULL)
{
}

CvPropertyPropagator::~CvPropertyPropagator()
{
	GC.removeDelayedResolution((int*)&m_eProperty);
	delete m_pExprActive;
}

PropertyTypes CvPropertyPropagator::getProperty() const
{
	return m_eProperty;
}

void CvPropertyPropagator::setProperty(PropertyTypes eProperty)
{
	m_eProperty = eProperty;
}

GameObjectTypes CvPropertyPropagator::getObjectType() const
{
	return m_eObjectType;
}

void CvPropertyPropagator::setObjectType(GameObjectTypes eObjectType)
{
	m_eObjectType = eObjectType;
}

RelationTypes CvPropertyPropagator::getRelation() const
{
	return m_eRelation;
}

void CvPropertyPropagator::setRelation(RelationTypes eRelation)
{
	m_eRelation = eRelation;
}

int CvPropertyPropagator::getRelationData() const
{
	return m_iRelationData;
}

void CvPropertyPropagator::setRelationData(int iRelationData)
{
	m_iRelationData = iRelationData;
}

RelationTypes CvPropertyPropagator::getTargetRelation() const
{
	return m_eTargetRelation;
}

void CvPropertyPropagator::setTargetRelation(RelationTypes eTargetRelation)
{
	m_eTargetRelation = eTargetRelation;
}

int CvPropertyPropagator::getTargetRelationData() const
{
	return m_iTargetRelationData;
}

void CvPropertyPropagator::setTargetRelationData(int iRelationData)
{
	m_iTargetRelationData = iRelationData;
}

GameObjectTypes CvPropertyPropagator::getTargetObjectType() const
{
	return m_eTargetObjectType;
}

void CvPropertyPropagator::setTargetObjectType(GameObjectTypes eObjectType)
{
	m_eTargetObjectType = eObjectType;
}

bool CvPropertyPropagator::isActive(CvGameObject *pObject)
{
	if ((m_eObjectType == NO_GAMEOBJECT) || (m_eObjectType == pObject->getGameObjectType()))
	{
		if (m_pExprActive)
		{
			return m_pExprActive->evaluate(pObject);
		}
		else
		{
			return true;
		}
	}
	return false;
}

void CvPropertyPropagator::getTargetObjects(CvGameObject* pObject, std::vector<CvGameObject*>& apGameObjects)
{
	apGameObjects.push_back(pObject);
	if (m_eTargetObjectType != NO_GAMEOBJECT)
	{
		pObject->enumerateRelated(apGameObjects, m_eTargetObjectType, m_eTargetRelation, m_iTargetRelationData);
	}
	else
	{
		for (int i=0; i<NUM_GAMEOBJECTS; i++)
		{
			pObject->enumerateRelated(apGameObjects, (GameObjectTypes)i, m_eTargetRelation, m_iTargetRelationData);
		}
	}
	// TODO: Should still filter out the source object from the vector
}

void CvPropertyPropagator::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eProperty);
	pStream->Read((int*)&m_eObjectType);
	pStream->Read((int*)&m_eRelation);
	pStream->Read(&m_iRelationData);
	pStream->Read((int*)&m_eTargetRelation);
	pStream->Read(&m_iTargetRelationData);
	pStream->Read((int*)&m_eTargetObjectType);
	m_pExprActive = BoolExpr::readExpression(pStream);
}

void CvPropertyPropagator::write(FDataStreamBase *pStream)
{
	pStream->Write(m_eProperty);
	pStream->Write(m_eObjectType);
	pStream->Write(m_eRelation);
	pStream->Write(m_iRelationData);
	pStream->Write(m_eTargetRelation);
	pStream->Write(m_iTargetRelationData);
	pStream->Write(m_eTargetObjectType);
	m_pExprActive->write(pStream);
}

bool CvPropertyPropagator::read(CvXMLLoadUtility *pXML)
{
	CvString szTextVal;
	pXML->GetChildXmlValByName(szTextVal, "PropertyType");
//	m_eProperty = (PropertyTypes) pXML->FindInInfoClass(szTextVal);
	GC.addDelayedResolution((int*)&m_eProperty, szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "GameObjectType");
	m_eObjectType = (GameObjectTypes) pXML->FindInInfoClass(szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "RelationType");
	m_eRelation = (RelationTypes) pXML->FindInInfoClass(szTextVal);
	if (m_eRelation == RELATION_NEAR)
		pXML->GetChildXmlValByName(&m_iRelationData, "iDistance");
	pXML->GetChildXmlValByName(szTextVal, "TargetObjectType");
	m_eTargetObjectType = (GameObjectTypes) pXML->FindInInfoClass(szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "TargetRelationType");
	m_eTargetRelation = (RelationTypes) pXML->FindInInfoClass(szTextVal);
	if (m_eTargetRelation == RELATION_NEAR)
		pXML->GetChildXmlValByName(&m_iTargetRelationData, "iTargetDistance");
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"Active"))
	{
		m_pExprActive = BoolExpr::read(pXML);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	return true;
}

void CvPropertyPropagator::copyNonDefaults(CvPropertyPropagator *pProp, CvXMLLoadUtility *pXML)
{
//	if (m_eProperty == NO_PROPERTY)
//		m_eProperty = pProp->getProperty();
	GC.copyNonDefaultDelayedResolution((int*)&m_eProperty,(int*)&(pProp->m_eProperty));
	if (m_eObjectType == NO_GAMEOBJECT)
		m_eObjectType = pProp->getObjectType();
	if (m_eRelation == NO_RELATION)
		m_eRelation = pProp->getRelation();
	if (m_iRelationData == 0)
		m_iRelationData = pProp->getRelationData();
	if (m_eTargetObjectType == NO_GAMEOBJECT)
		m_eTargetObjectType = pProp->getTargetObjectType();
	if (m_eTargetRelation == NO_RELATION)
		m_eTargetRelation = pProp->getTargetRelation();
	if (m_iTargetRelationData == 0)
		m_iTargetRelationData = pProp->getTargetRelationData();
	if (m_pExprActive == NULL)
	{
		m_pExprActive = pProp->m_pExprActive;
		pProp->m_pExprActive = NULL;
	}
}



CvPropertyPropagatorSpread::CvPropertyPropagatorSpread() :	CvPropertyPropagator(),
															m_iPercent(0)
{
}

CvPropertyPropagatorSpread::CvPropertyPropagatorSpread(PropertyTypes eProperty) : CvPropertyPropagator(eProperty),
																				m_iPercent(0)
{
}

CvPropertyPropagatorSpread::CvPropertyPropagatorSpread(PropertyTypes eProperty, int iPercent) : CvPropertyPropagator(eProperty), m_iPercent(iPercent)
{
}

PropertyPropagatorTypes CvPropertyPropagatorSpread::getType()
{
	return PROPERTYPROPAGATOR_SPREAD;
}

int CvPropertyPropagatorSpread::getPercent()
{
	return m_iPercent;
}

void CvPropertyPropagatorSpread::getPredict(std::vector<int>& aiCurrentAmount, std::vector<int>& aiPredict)
{
	int iCurrentAmount = aiCurrentAmount[0];
	aiPredict[0] = 0;
	for(int iI=1; iI<(int)aiCurrentAmount.size(); iI++)
	{
		int iDiff = iCurrentAmount - aiCurrentAmount[iI];
		aiPredict[iI] = std::max(0, (iDiff * m_iPercent) / 100);
	}
}

void CvPropertyPropagatorSpread::getCorrect(std::vector<int>& aiCurrentAmount, std::vector<int>& aiPredictedAmount, std::vector<int>& aiCorrect)
{
	int iCurrentAmount = aiCurrentAmount[0];
	aiCorrect[0] = 0;
	for(int iI=1; iI<(int)aiCurrentAmount.size(); iI++)
	{
		int iDiff = iCurrentAmount - aiCurrentAmount[iI];
		if (iDiff < 0)
		{
			aiCorrect[iI] = 0;
		}
		else
		{
			int iPredicted = aiCurrentAmount[iI] + (iDiff * m_iPercent) / 100;
			int iExtra = aiPredictedAmount[iI] - iPredicted;
			if (iExtra > 0)
			{
				//use half of extra to base spreading on
				iDiff -= iExtra / 2;
				aiCorrect[iI] = std::max(0, (iDiff * m_iPercent) / 100);
			}
			else
			{
				aiCorrect[iI] =(iDiff * m_iPercent) / 100;
			}
		}
	}
}

void CvPropertyPropagatorSpread::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString szTemp;
	szTemp.Format(L"Spreads %d%% %c difference / Turn", m_iPercent, GC.getPropertyInfo(getProperty()).getChar());
	szBuffer.append(szTemp);
}

void CvPropertyPropagatorSpread::read(FDataStreamBase *pStream)
{
	CvPropertyPropagator::read(pStream);
	pStream->Read(&m_iPercent);
}

void CvPropertyPropagatorSpread::write(FDataStreamBase *pStream)
{
	CvPropertyPropagator::write(pStream);
	pStream->Write(m_iPercent);
}

bool CvPropertyPropagatorSpread::read(CvXMLLoadUtility *pXML)
{
	CvPropertyPropagator::read(pXML);
	pXML->GetChildXmlValByName(&m_iPercent, "iPercent");
	return true;
}

void CvPropertyPropagatorSpread::copyNonDefaults(CvPropertyPropagator *pProp, CvXMLLoadUtility *pXML)
{
	CvPropertyPropagator::copyNonDefaults(pProp, pXML);
	CvPropertyPropagatorSpread* pOther = static_cast<CvPropertyPropagatorSpread*>(pProp);
	if (m_iPercent == 0)
		m_iPercent = pOther->getPercent();
}


CvPropertyPropagatorGather::CvPropertyPropagatorGather() :	CvPropertyPropagator(),
															m_iAmountPerTurn(0)
{
}

CvPropertyPropagatorGather::CvPropertyPropagatorGather(PropertyTypes eProperty) : CvPropertyPropagator(eProperty),
																				m_iAmountPerTurn(0)
{
}

CvPropertyPropagatorGather::CvPropertyPropagatorGather(PropertyTypes eProperty, int iAmountPerTurn) : CvPropertyPropagator(eProperty), m_iAmountPerTurn(iAmountPerTurn)
{
}

PropertyPropagatorTypes CvPropertyPropagatorGather::getType()
{
	return PROPERTYPROPAGATOR_GATHER;
}

int CvPropertyPropagatorGather::getAmountPerTurn()
{
	return m_iAmountPerTurn;
}

void CvPropertyPropagatorGather::getPredict(std::vector<int>& aiCurrentAmount, std::vector<int>& aiPredict)
{
	aiPredict[0] = 0;
	for(int iI=1; iI<(int)aiCurrentAmount.size(); iI++)
	{
		if (aiCurrentAmount[iI] < m_iAmountPerTurn)
		{
			aiPredict[0] += aiCurrentAmount[iI];
			aiPredict[iI] = -aiCurrentAmount[iI];
		}
		else
		{
			aiPredict[0] += m_iAmountPerTurn;
			aiPredict[iI] = -m_iAmountPerTurn;
		}
	}
}

void CvPropertyPropagatorGather::getCorrect(std::vector<int>& aiCurrentAmount, std::vector<int>& aiPredictedAmount, std::vector<int>& aiCorrect)
{
	aiCorrect[0] = 0;
	for(int iI=1; iI<(int)aiCurrentAmount.size(); iI++)
	{
		if (aiPredictedAmount[iI] >= 0)
		{
			if (aiCurrentAmount[iI] < m_iAmountPerTurn)
			{
				aiCorrect[0] += aiCurrentAmount[iI];
				aiCorrect[iI] = -aiCurrentAmount[iI];
			}
			else
			{
				aiCorrect[0] += m_iAmountPerTurn;
				aiCorrect[iI] = -m_iAmountPerTurn;
			}
		}
		else
		{
			int iPredicted = (aiCurrentAmount[iI] < m_iAmountPerTurn) ? aiCurrentAmount[iI] : m_iAmountPerTurn;
			int iCorrected = (iPredicted * aiCurrentAmount[iI]) / (aiCurrentAmount[iI] - aiPredictedAmount[iI]);
			aiCorrect[0] += iCorrected;
			aiCorrect[iI] = -iCorrected;
		}
	}
}

void CvPropertyPropagatorGather::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString szTemp;
	szTemp.Format(L"Gathers %d %c / Turn", m_iAmountPerTurn, GC.getPropertyInfo(getProperty()).getChar());
	szBuffer.append(szTemp);
}

void CvPropertyPropagatorGather::read(FDataStreamBase *pStream)
{
	CvPropertyPropagator::read(pStream);
	pStream->Read(&m_iAmountPerTurn);
}

void CvPropertyPropagatorGather::write(FDataStreamBase *pStream)
{
	CvPropertyPropagator::write(pStream);
	pStream->Write(m_iAmountPerTurn);
}

bool CvPropertyPropagatorGather::read(CvXMLLoadUtility *pXML)
{
	CvPropertyPropagator::read(pXML);
	pXML->GetChildXmlValByName(&m_iAmountPerTurn, "iAmountPerTurn");
	return true;
}

void CvPropertyPropagatorGather::copyNonDefaults(CvPropertyPropagator *pProp, CvXMLLoadUtility *pXML)
{
	CvPropertyPropagator::copyNonDefaults(pProp, pXML);
	CvPropertyPropagatorGather* pOther = static_cast<CvPropertyPropagatorGather*>(pProp);
	if (m_iAmountPerTurn == 0)
		m_iAmountPerTurn = pOther->getAmountPerTurn();
}


CvPropertyPropagatorDiffuse::CvPropertyPropagatorDiffuse() :	CvPropertyPropagator(),
															m_iPercent(0)
{
}

CvPropertyPropagatorDiffuse::CvPropertyPropagatorDiffuse(PropertyTypes eProperty) : CvPropertyPropagator(eProperty),
																				m_iPercent(0)
{
}

CvPropertyPropagatorDiffuse::CvPropertyPropagatorDiffuse(PropertyTypes eProperty, int iPercent) : CvPropertyPropagator(eProperty), m_iPercent(iPercent)
{
}

PropertyPropagatorTypes CvPropertyPropagatorDiffuse::getType()
{
	return PROPERTYPROPAGATOR_DIFFUSE;
}

int CvPropertyPropagatorDiffuse::getPercent()
{
	return m_iPercent;
}

void CvPropertyPropagatorDiffuse::getPredict(std::vector<int>& aiCurrentAmount, std::vector<int>& aiPredict)
{
	int iCurrentAmount = aiCurrentAmount[0];
	aiPredict[0] = 0;
	for(int iI=1; iI<(int)aiCurrentAmount.size(); iI++)
	{
		int iDiff = iCurrentAmount - aiCurrentAmount[iI];
		int iChange = std::max(0, (iDiff * m_iPercent) / 100);
		aiPredict[iI] = iChange;
		aiPredict[0] -= iChange;
	}
}

void CvPropertyPropagatorDiffuse::getCorrect(std::vector<int>& aiCurrentAmount, std::vector<int>& aiPredictedAmount, std::vector<int>& aiCorrect)
{
	int iCurrentAmount = aiCurrentAmount[0];
	int iPredictedAmount = aiPredictedAmount[0];
	int iPredictedSelf = 0;
	int iPredictedTotalSelf = iPredictedAmount - iCurrentAmount;
	aiCorrect[0] = 0;
	for(int iI=1; iI<(int)aiCurrentAmount.size(); iI++)
	{
		int iDiff = iCurrentAmount - aiCurrentAmount[iI];
		if (iDiff < 0)
		{
			aiCorrect[iI] = 0;
		}
		else
		{
			int iChange = (iDiff * m_iPercent) / 100;
			int iPredicted = aiCurrentAmount[iI] + iChange;
			iPredictedSelf -= iChange;
			int iExtra = aiPredictedAmount[iI] - iPredicted;
			if (iExtra > 0)
			{
				//use half of extra to base diffusion on
				iDiff -= iExtra / 2;
			}
			
			iChange = std::max(0, (iDiff * m_iPercent) / 100);
			aiCorrect[iI] = iChange;
			aiCorrect[0] -= iChange;
		}
	}
	
	if (iPredictedTotalSelf < iPredictedSelf)
	{
		int iSelfChangeByOthers = iPredictedTotalSelf - iPredictedSelf;
		// use half of other change to base diffusion on
		int iAssumedAmount = iCurrentAmount + iSelfChangeByOthers / 2;

		aiCorrect[0] = 0;

		for(int iI=1; iI<(int)aiCurrentAmount.size(); iI++)
		{
			int iDiff = iAssumedAmount - aiCurrentAmount[iI];
			if (iDiff < 0)
			{
				aiCorrect[iI] = 0;
			}
			else
			{
				int iChange = (iDiff * m_iPercent) / 100;
				int iPredicted = aiCurrentAmount[iI] + iChange;
				int iExtra = aiPredictedAmount[iI] - iPredicted;
				if (iExtra > 0)
				{
					//use half of extra to base diffusion on
					iDiff -= iExtra / 2;
				}
				
				iChange = std::max(0, (iDiff * m_iPercent) / 100);
				aiCorrect[iI] = iChange;
				aiCorrect[0] -= iChange;
			}
		}
	}
}

void CvPropertyPropagatorDiffuse::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString szTemp;
	szTemp.Format(L"%d%% %c diffusion / Turn", m_iPercent, GC.getPropertyInfo(getProperty()).getChar());
	szBuffer.append(szTemp);
}

void CvPropertyPropagatorDiffuse::read(FDataStreamBase *pStream)
{
	CvPropertyPropagator::read(pStream);
	pStream->Read(&m_iPercent);
}

void CvPropertyPropagatorDiffuse::write(FDataStreamBase *pStream)
{
	CvPropertyPropagator::write(pStream);
	pStream->Write(m_iPercent);
}

bool CvPropertyPropagatorDiffuse::read(CvXMLLoadUtility *pXML)
{
	CvPropertyPropagator::read(pXML);
	pXML->GetChildXmlValByName(&m_iPercent, "iPercent");
	return true;
}

void CvPropertyPropagatorDiffuse::copyNonDefaults(CvPropertyPropagator *pProp, CvXMLLoadUtility *pXML)
{
	CvPropertyPropagator::copyNonDefaults(pProp, pXML);
	CvPropertyPropagatorDiffuse* pOther = static_cast<CvPropertyPropagatorDiffuse*>(pProp);
	if (m_iPercent == 0)
		m_iPercent = pOther->getPercent();
}