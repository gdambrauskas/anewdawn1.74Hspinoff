//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvPropertyInteraction.cpp
//
//  PURPOSE: Interaction of generic properties for Civ4 classes
//
//------------------------------------------------------------------------------------------------

#include "CvGameCoreDLL.h"
#include "CvPropertyInteraction.h"
#include "CvXMLLoadUtility.h"
#include "CvDLLXMLIFaceBase.h"

CvPropertyInteraction::CvPropertyInteraction() : 
									m_eSourceProperty(NO_PROPERTY),
									m_eTargetProperty(NO_PROPERTY),
									m_eObjectType(NO_GAMEOBJECT),
									m_eRelation(NO_RELATION),
									m_iRelationData(0),
									m_pExprActive(NULL)
{
}

CvPropertyInteraction::CvPropertyInteraction(PropertyTypes eSourceProperty, PropertyTypes eTargetProperty) :
									m_eSourceProperty(eSourceProperty),
									m_eTargetProperty(eTargetProperty),
									m_eObjectType(NO_GAMEOBJECT),
									m_eRelation(NO_RELATION),
									m_iRelationData(0),
									m_pExprActive(NULL)
{
}

CvPropertyInteraction::~CvPropertyInteraction()
{
	GC.removeDelayedResolution((int*)&m_eSourceProperty);
	GC.removeDelayedResolution((int*)&m_eTargetProperty);
	delete m_pExprActive;
}

PropertyTypes CvPropertyInteraction::getSourceProperty() const
{
	return m_eSourceProperty;
}

PropertyTypes CvPropertyInteraction::getTargetProperty() const
{
	return m_eTargetProperty;
}

void CvPropertyInteraction::setSourceProperty(PropertyTypes eProperty)
{
	m_eSourceProperty = eProperty;
}

void CvPropertyInteraction::setTargetProperty(PropertyTypes eProperty)
{
	m_eTargetProperty = eProperty;
}

GameObjectTypes CvPropertyInteraction::getObjectType() const
{
	return m_eObjectType;
}

void CvPropertyInteraction::setObjectType(GameObjectTypes eObjectType)
{
	m_eObjectType = eObjectType;
}

RelationTypes CvPropertyInteraction::getRelation() const
{
	return m_eRelation;
}

void CvPropertyInteraction::setRelation(RelationTypes eRelation)
{
	m_eRelation = eRelation;
}

int CvPropertyInteraction::getRelationData() const
{
	return m_iRelationData;
}

void CvPropertyInteraction::setRelationData(int iRelationData)
{
	m_iRelationData = iRelationData;
}

bool CvPropertyInteraction::isActive(CvGameObject *pObject)
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

void CvPropertyInteraction::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eSourceProperty);
	pStream->Read((int*)&m_eTargetProperty);
	pStream->Read((int*)&m_eObjectType);
	pStream->Read((int*)&m_eRelation);
	pStream->Read(&m_iRelationData);
	m_pExprActive = BoolExpr::readExpression(pStream);
}

void CvPropertyInteraction::write(FDataStreamBase *pStream)
{
	pStream->Write(m_eSourceProperty);
	pStream->Write(m_eTargetProperty);
	pStream->Write(m_eObjectType);
	pStream->Write(m_eRelation);
	pStream->Write(m_iRelationData);
	m_pExprActive->write(pStream);
}

bool CvPropertyInteraction::read(CvXMLLoadUtility *pXML)
{
	CvString szTextVal;
	pXML->GetChildXmlValByName(szTextVal, "SourcePropertyType");
	//m_eSourceProperty = (PropertyTypes) pXML->FindInInfoClass(szTextVal);
	GC.addDelayedResolution((int*)&m_eSourceProperty,szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "TargetPropertyType");
	//m_eTargetProperty = (PropertyTypes) pXML->FindInInfoClass(szTextVal);
	GC.addDelayedResolution((int*)&m_eTargetProperty,szTextVal);
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

void CvPropertyInteraction::copyNonDefaults(CvPropertyInteraction *pProp, CvXMLLoadUtility *pXML)
{
//	if (m_eSourceProperty == NO_PROPERTY)
//		m_eSourceProperty = pProp->getSourceProperty();
	GC.copyNonDefaultDelayedResolution((int*)&m_eSourceProperty,(int*)&(pProp->m_eSourceProperty));
//	if (m_eTargetProperty == NO_PROPERTY)
//		m_eTargetProperty = pProp->getTargetProperty();
	GC.copyNonDefaultDelayedResolution((int*)&m_eTargetProperty,(int*)&(pProp->m_eTargetProperty));
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



CvPropertyInteractionConvertConstant::CvPropertyInteractionConvertConstant() :	CvPropertyInteraction(),
																				m_iAmountPerTurn(0)
{
}

CvPropertyInteractionConvertConstant::CvPropertyInteractionConvertConstant(PropertyTypes eSourceProperty, PropertyTypes eTargetProperty) : CvPropertyInteraction(eSourceProperty, eTargetProperty),
																																		m_iAmountPerTurn(0)
{
}

CvPropertyInteractionConvertConstant::CvPropertyInteractionConvertConstant(PropertyTypes eSourceProperty, PropertyTypes eTargetProperty, int iAmountPerTurn) : CvPropertyInteraction(eSourceProperty, eTargetProperty), m_iAmountPerTurn(iAmountPerTurn)
{
}

PropertyInteractionTypes CvPropertyInteractionConvertConstant::getType()
{
	return PROPERTYINTERACTION_CONVERT_CONSTANT;
}

int CvPropertyInteractionConvertConstant::getAmountPerTurn()
{
	return m_iAmountPerTurn;
}

std::pair<int,int> CvPropertyInteractionConvertConstant::getPredict(int iCurrentAmountSource, int iCurrentAmountTarget)
{
	return std::pair<int,int>(-m_iAmountPerTurn,m_iAmountPerTurn);
}

std::pair<int,int> CvPropertyInteractionConvertConstant::getCorrect(int iCurrentAmountSource, int iCurrentAmountTarget, int iPredictedAmountSource, int iPredictedAmountTarget)
{
	return std::pair<int,int>(-m_iAmountPerTurn,m_iAmountPerTurn);
}

void CvPropertyInteractionConvertConstant::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString szTemp;
	szTemp.Format(L"Convert %+d %c to %c / Turn", m_iAmountPerTurn, GC.getPropertyInfo(getSourceProperty()).getChar(), GC.getPropertyInfo(getTargetProperty()).getChar());
	szBuffer.append(szTemp);
}

void CvPropertyInteractionConvertConstant::read(FDataStreamBase *pStream)
{
	CvPropertyInteraction::read(pStream);
	pStream->Read(&m_iAmountPerTurn);
}

void CvPropertyInteractionConvertConstant::write(FDataStreamBase *pStream)
{
	CvPropertyInteraction::write(pStream);
	pStream->Write(m_iAmountPerTurn);
}

bool CvPropertyInteractionConvertConstant::read(CvXMLLoadUtility *pXML)
{
	CvPropertyInteraction::read(pXML);
	pXML->GetChildXmlValByName(&m_iAmountPerTurn, "iAmountPerTurn");
	return true;
}

void CvPropertyInteractionConvertConstant::copyNonDefaults(CvPropertyInteraction *pProp, CvXMLLoadUtility *pXML)
{
	CvPropertyInteraction::copyNonDefaults(pProp, pXML);
	CvPropertyInteractionConvertConstant* pOther = static_cast<CvPropertyInteractionConvertConstant*>(pProp);
	if (m_iAmountPerTurn == 0)
		m_iAmountPerTurn = pOther->getAmountPerTurn();
}
