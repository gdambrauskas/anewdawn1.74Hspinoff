//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvPropertySource.cpp
//
//  PURPOSE: Source of generic properties for Civ4 classes
//
//------------------------------------------------------------------------------------------------

#include "CvGameCoreDLL.h"
#include "CvPropertySource.h"
#include "CvXMLLoadUtility.h"
#include "CvDLLXMLIFaceBase.h"

CvPropertySource::CvPropertySource() : 
									m_eProperty(NO_PROPERTY),
									m_eObjectType(NO_GAMEOBJECT),
									m_eRelation(NO_RELATION),
									m_iRelationData(0),
									m_pExprActive(NULL)
{
}

CvPropertySource::CvPropertySource(PropertyTypes eProperty) :
									m_eProperty(eProperty),
									m_eObjectType(NO_GAMEOBJECT),
									m_eRelation(NO_RELATION),
									m_iRelationData(0),
									m_pExprActive(NULL)
{
}

CvPropertySource::~CvPropertySource()
{
	GC.removeDelayedResolution((int*)&m_eProperty);
	delete m_pExprActive;
}

PropertyTypes CvPropertySource::getProperty() const
{
	return m_eProperty;
}

void CvPropertySource::setProperty(PropertyTypes eProperty)
{
	m_eProperty = eProperty;
}

GameObjectTypes CvPropertySource::getObjectType() const
{
	return m_eObjectType;
}

void CvPropertySource::setObjectType(GameObjectTypes eObjectType)
{
	m_eObjectType = eObjectType;
}

RelationTypes CvPropertySource::getRelation() const
{
	return m_eRelation;
}

void CvPropertySource::setRelation(RelationTypes eRelation)
{
	m_eRelation = eRelation;
}

int CvPropertySource::getRelationData() const
{
	return m_iRelationData;
}

void CvPropertySource::setRelationData(int iRelationData)
{
	m_iRelationData = iRelationData;
}

bool CvPropertySource::isActive(CvGameObject *pObject)
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

void CvPropertySource::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eProperty);
	pStream->Read((int*)&m_eObjectType);
	pStream->Read((int*)&m_eRelation);
	pStream->Read(&m_iRelationData);
	m_pExprActive = BoolExpr::readExpression(pStream);
}

void CvPropertySource::write(FDataStreamBase *pStream)
{
	pStream->Write(m_eProperty);
	pStream->Write(m_eObjectType);
	pStream->Write(m_eRelation);
	pStream->Write(m_iRelationData);
	m_pExprActive->write(pStream);
}

bool CvPropertySource::read(CvXMLLoadUtility *pXML)
{
	CvString szTextVal;
	pXML->GetChildXmlValByName(szTextVal, "PropertyType");
	GC.addDelayedResolution((int*)&m_eProperty, szTextVal);
	//m_eProperty = (PropertyTypes) pXML->FindInInfoClass(szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "GameObjectType");
	m_eObjectType = (GameObjectTypes) pXML->FindInInfoClass(szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "RelationType");
	m_eRelation = (RelationTypes) pXML->FindInInfoClass(szTextVal);
	if (m_eRelation == RELATION_NEAR)
		pXML->GetChildXmlValByName(&m_iRelationData, "iDistance");
	if (gDLL->getXMLIFace()->SetToChildByTagName(pXML->GetXML(),"Active"))
	{
		m_pExprActive = BoolExpr::read(pXML);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
	}
	return true;
}

void CvPropertySource::copyNonDefaults(CvPropertySource *pProp, CvXMLLoadUtility *pXML)
{
	//if (m_eProperty == NO_PROPERTY)
	//	m_eProperty = pProp->getProperty();
	GC.copyNonDefaultDelayedResolution((int*)&m_eProperty, (int*)&(pProp->m_eProperty));

	if (m_eObjectType == NO_GAMEOBJECT)
		m_eObjectType = pProp->getObjectType();
	if (m_eRelation == NO_RELATION)
		m_eRelation = pProp->getRelation();
	if (m_iRelationData == 0)
		m_iRelationData = pProp->getRelationData();
	if (m_pExprActive == NULL)
	{
		m_pExprActive = pProp->m_pExprActive;
		pProp->m_pExprActive = NULL;
	}
}



CvPropertySourceConstant::CvPropertySourceConstant() :	CvPropertySource(),
														m_iAmountPerTurn(0)
{
}

CvPropertySourceConstant::CvPropertySourceConstant(PropertyTypes eProperty) : CvPropertySource(eProperty),
																			m_iAmountPerTurn(0)
{
}

CvPropertySourceConstant::CvPropertySourceConstant(PropertyTypes eProperty, int iAmountPerTurn) : CvPropertySource(eProperty), m_iAmountPerTurn(iAmountPerTurn)
{
}

PropertySourceTypes CvPropertySourceConstant::getType()
{
	return PROPERTYSOURCE_CONSTANT;
}

int CvPropertySourceConstant::getAmountPerTurn()
{
	return m_iAmountPerTurn;
}

int CvPropertySourceConstant::getSourcePredict(const CvGameObject* pObject, int iCurrentAmount)
{
	return m_iAmountPerTurn;
}

int CvPropertySourceConstant::getSourceCorrect(const CvGameObject* pObject, int iCurrentAmount, int iPredictedAmount)
{
	return m_iAmountPerTurn;
}

void CvPropertySourceConstant::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString szTemp;
	szTemp.Format(L"%c: %+d / Turn", GC.getPropertyInfo(getProperty()).getChar(), m_iAmountPerTurn);
	szBuffer.append(szTemp);
}

void CvPropertySourceConstant::read(FDataStreamBase *pStream)
{
	CvPropertySource::read(pStream);
	pStream->Read(&m_iAmountPerTurn);
}

void CvPropertySourceConstant::write(FDataStreamBase *pStream)
{
	CvPropertySource::write(pStream);
	pStream->Write(m_iAmountPerTurn);
}

bool CvPropertySourceConstant::read(CvXMLLoadUtility *pXML)
{
	CvPropertySource::read(pXML);
	pXML->GetChildXmlValByName(&m_iAmountPerTurn, "iAmountPerTurn");
	return true;
}

void CvPropertySourceConstant::copyNonDefaults(CvPropertySource *pProp, CvXMLLoadUtility *pXML)
{
	CvPropertySource::copyNonDefaults(pProp, pXML);
	CvPropertySourceConstant* pOther = static_cast<CvPropertySourceConstant*>(pProp);
	if (m_iAmountPerTurn == 0)
		m_iAmountPerTurn = pOther->getAmountPerTurn();
}


CvPropertySourceConstantLimited::CvPropertySourceConstantLimited() :	CvPropertySource(),
														m_iAmountPerTurn(0), m_iLimit(0)
{
}

CvPropertySourceConstantLimited::CvPropertySourceConstantLimited(PropertyTypes eProperty) : CvPropertySource(eProperty),
																			m_iAmountPerTurn(0), m_iLimit(0)
{
}

CvPropertySourceConstantLimited::CvPropertySourceConstantLimited(PropertyTypes eProperty, int iAmountPerTurn, int iLimit) : CvPropertySource(eProperty), m_iAmountPerTurn(iAmountPerTurn), m_iLimit(iLimit)
{
}

PropertySourceTypes CvPropertySourceConstantLimited::getType()
{
	return PROPERTYSOURCE_CONSTANT_LIMITED;
}

int CvPropertySourceConstantLimited::getAmountPerTurn()
{
	return m_iAmountPerTurn;
}

int CvPropertySourceConstantLimited::getLimit()
{
	return m_iLimit;
}

int CvPropertySourceConstantLimited::getSourcePredict(const CvGameObject* pObject, int iCurrentAmount)
{
	return m_iAmountPerTurn + iCurrentAmount > m_iLimit  ?  std::max(m_iLimit - iCurrentAmount, 0)  :  m_iAmountPerTurn;
}

int CvPropertySourceConstantLimited::getSourceCorrect(const CvGameObject* pObject, int iCurrentAmount, int iPredictedAmount)
{
	if (iCurrentAmount >= m_iLimit)
	{
		return 0;
	}

	int iPredict = getSourcePredict(pObject, iCurrentAmount);
	if (iPredictedAmount <= m_iLimit)
	{
		return iPredict;
	}
	else
	{
		int iTotalPredicted = iPredictedAmount - iCurrentAmount;
		return (iPredict * (m_iLimit - iCurrentAmount)) / iTotalPredicted;
	}
}

void CvPropertySourceConstantLimited::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString szTemp;
	szTemp.Format(L"%c: %+d / Turn (limit %d)", GC.getPropertyInfo(getProperty()).getChar(), m_iAmountPerTurn, m_iLimit);
	szBuffer.append(szTemp);
}

void CvPropertySourceConstantLimited::read(FDataStreamBase *pStream)
{
	CvPropertySource::read(pStream);
	pStream->Read(&m_iAmountPerTurn);
	pStream->Read(&m_iLimit);
}

void CvPropertySourceConstantLimited::write(FDataStreamBase *pStream)
{
	CvPropertySource::write(pStream);
	pStream->Write(m_iAmountPerTurn);
	pStream->Write(m_iLimit);
}

bool CvPropertySourceConstantLimited::read(CvXMLLoadUtility *pXML)
{
	CvPropertySource::read(pXML);
	pXML->GetChildXmlValByName(&m_iAmountPerTurn, "iAmountPerTurn");
	pXML->GetChildXmlValByName(&m_iLimit, "iLimit");
	return true;
}

void CvPropertySourceConstantLimited::copyNonDefaults(CvPropertySource *pProp, CvXMLLoadUtility *pXML)
{
	CvPropertySource::copyNonDefaults(pProp, pXML);
	CvPropertySourceConstantLimited* pOther = static_cast<CvPropertySourceConstantLimited*>(pProp);
	if (m_iAmountPerTurn == 0)
		m_iAmountPerTurn = pOther->getAmountPerTurn();
	if (m_iLimit == 0)
		m_iLimit = pOther->getLimit();
}


CvPropertySourceDecay::CvPropertySourceDecay() :	CvPropertySource(),
														m_iPercent(0), m_iNoDecayAmount(0)
{
}

CvPropertySourceDecay::CvPropertySourceDecay(PropertyTypes eProperty) : CvPropertySource(eProperty),
																			m_iPercent(0), m_iNoDecayAmount(0)
{
}

CvPropertySourceDecay::CvPropertySourceDecay(PropertyTypes eProperty, int iPercent, int iNoDecayAmount) : CvPropertySource(eProperty), m_iPercent(iPercent), m_iNoDecayAmount(iNoDecayAmount)
{
}

PropertySourceTypes CvPropertySourceDecay::getType()
{
	return PROPERTYSOURCE_DECAY;
}

int CvPropertySourceDecay::getPercent()
{
	return m_iPercent;
}

int CvPropertySourceDecay::getNoDecayAmount()
{
	return m_iNoDecayAmount;
}

int CvPropertySourceDecay::getSourcePredict(const CvGameObject* pObject, int iCurrentAmount)
{
	if (iCurrentAmount >= 0)
		return - (m_iPercent * std::max(iCurrentAmount - m_iNoDecayAmount, 0)) / 100;
	else
		return (m_iPercent * std::max(-iCurrentAmount - m_iNoDecayAmount, 0)) / 100;
}

int CvPropertySourceDecay::getSourceCorrect(const CvGameObject* pObject, int iCurrentAmount, int iPredictedAmount)
{
	if (iCurrentAmount >= 0)
	{
		int iDiff = iCurrentAmount - m_iNoDecayAmount;
		if (iDiff <= 0)
		{
			return 0;
		}

		int iPredicted = iCurrentAmount - (iDiff * m_iPercent) / 100;
		int iExtra = iPredictedAmount - iPredicted;
		//use half of extra to base decay on
		iDiff += iExtra / 2;
		return - std::max(0, (iDiff * m_iPercent) / 100);
	}
	else
	{
		int iDiff = -iCurrentAmount - m_iNoDecayAmount;
		if (iDiff <= 0)
		{
			return 0;
		}

		int iPredicted = iCurrentAmount + (iDiff * m_iPercent) / 100;
		int iExtra = iPredictedAmount - iPredicted;
		//use half of extra to base decay on
		iDiff += iExtra / 2;
		return std::max(0, (iDiff * m_iPercent) / 100);
	}
}

void CvPropertySourceDecay::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString szTemp;
	szTemp.Format(L"%c: %d%% decay / Turn (above %d)", GC.getPropertyInfo(getProperty()).getChar(), m_iPercent, m_iNoDecayAmount);
	szBuffer.append(szTemp);
}

void CvPropertySourceDecay::read(FDataStreamBase *pStream)
{
	CvPropertySource::read(pStream);
	pStream->Read(&m_iPercent);
	pStream->Read(&m_iNoDecayAmount);
}

void CvPropertySourceDecay::write(FDataStreamBase *pStream)
{
	CvPropertySource::write(pStream);
	pStream->Write(m_iPercent);
	pStream->Write(m_iNoDecayAmount);
}

bool CvPropertySourceDecay::read(CvXMLLoadUtility *pXML)
{
	CvPropertySource::read(pXML);
	pXML->GetChildXmlValByName(&m_iPercent, "iPercent");
	pXML->GetChildXmlValByName(&m_iNoDecayAmount, "iNoDecayAmount");
	return true;
}

void CvPropertySourceDecay::copyNonDefaults(CvPropertySource *pProp, CvXMLLoadUtility *pXML)
{
	CvPropertySource::copyNonDefaults(pProp, pXML);
	CvPropertySourceDecay* pOther = static_cast<CvPropertySourceDecay*>(pProp);
	if (m_iPercent == 0)
		m_iPercent = pOther->getPercent();
	if (m_iNoDecayAmount == 0)
		m_iNoDecayAmount = pOther->getNoDecayAmount();
}

CvPropertySourceAttributeConstant::CvPropertySourceAttributeConstant() :	CvPropertySource(),
														m_iAmountPerTurn(0),
														m_eAttribute(NO_ATTRIBUTE)
{
}

CvPropertySourceAttributeConstant::CvPropertySourceAttributeConstant(PropertyTypes eProperty) : CvPropertySource(eProperty),
																			m_iAmountPerTurn(0),
																			m_eAttribute(NO_ATTRIBUTE)
{
}

CvPropertySourceAttributeConstant::CvPropertySourceAttributeConstant(PropertyTypes eProperty, AttributeTypes eAttribute, int iAmountPerTurn) : CvPropertySource(eProperty), m_iAmountPerTurn(iAmountPerTurn), m_eAttribute(eAttribute)
{
}

PropertySourceTypes CvPropertySourceAttributeConstant::getType()
{
	return PROPERTYSOURCE_ATTRIBUTE_CONSTANT;
}

AttributeTypes CvPropertySourceAttributeConstant::getAttribute()
{
	return m_eAttribute;
}

int CvPropertySourceAttributeConstant::getAmountPerTurn()
{
	return m_iAmountPerTurn;
}

int CvPropertySourceAttributeConstant::getSourcePredict(const CvGameObject* pObject, int iCurrentAmount)
{
	return pObject->getAttribute(m_eAttribute) * m_iAmountPerTurn;
}

int CvPropertySourceAttributeConstant::getSourceCorrect(const CvGameObject* pObject, int iCurrentAmount, int iPredictedAmount)
{
	return pObject->getAttribute(m_eAttribute) * m_iAmountPerTurn;
}

void CvPropertySourceAttributeConstant::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString szTemp;
	szTemp.Format(L"%c: %+d / Turn", GC.getPropertyInfo(getProperty()).getChar(), m_iAmountPerTurn); // TODO: proper display of attribute
	szBuffer.append(szTemp);
}

void CvPropertySourceAttributeConstant::read(FDataStreamBase *pStream)
{
	CvPropertySource::read(pStream);
	pStream->Read((int*)&m_eAttribute);
	pStream->Read(&m_iAmountPerTurn);
}

void CvPropertySourceAttributeConstant::write(FDataStreamBase *pStream)
{
	CvPropertySource::write(pStream);
	pStream->Write((int)m_eAttribute);
	pStream->Write(m_iAmountPerTurn);
}

bool CvPropertySourceAttributeConstant::read(CvXMLLoadUtility *pXML)
{
	CvPropertySource::read(pXML);
	CvString szTextVal;
	pXML->GetChildXmlValByName(szTextVal, "AttributeType");
	m_eAttribute = (AttributeTypes)pXML->FindInInfoClass(szTextVal);
	pXML->GetChildXmlValByName(&m_iAmountPerTurn, "iAmountPerTurn");
	return true;
}

void CvPropertySourceAttributeConstant::copyNonDefaults(CvPropertySource *pProp, CvXMLLoadUtility *pXML)
{
	CvPropertySource::copyNonDefaults(pProp, pXML);
	CvPropertySourceAttributeConstant* pOther = static_cast<CvPropertySourceAttributeConstant*>(pProp);
	if (m_eAttribute == NO_ATTRIBUTE)
		m_eAttribute = pOther->getAttribute();
	if (m_iAmountPerTurn == 0)
		m_iAmountPerTurn = pOther->getAmountPerTurn();
}
