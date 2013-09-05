//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:	BoolExpr.cpp
//
//  PURPOSE: Boolean Expressions for Civ4 classes
//
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "IntExpr.h"
#include "CvDLLXMLIFaceBase.h"
#include <boost/bind.hpp>

IntExpr::~IntExpr()
{
}

IntExpr* IntExpr::read(CvXMLLoadUtility *pXML)
{
	// In general we assume no comments to simplify reading code

	TCHAR szTag[1024];
	if (!gDLL->getXMLIFace()->GetLastLocatedNodeTagName(pXML->GetXML(), szTag))
	{
		// No located node
		return NULL;
	}
	
	if (strcmp(szTag, "Plus") == 0)
	{
		// this is a Plus node, check number of children
		if (gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML()) < 2)
		{
			// no real Plus node
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// there is a subexpression, so no simple constant
				IntExpr* pExpr = read(pXML);
				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pExpr;
			}
			else
			{
				// constant
				int iConstant = 0;
				pXML->GetXmlVal(&iConstant);
				return new IntExprConstant(!iConstant);
			}
		}
		else
		{
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = read(pXML);
				
				// read nodes until there are no more siblings
				while (gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					pExpr = new IntExprPlus(pExpr, read(pXML));
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "Mult") == 0)
	{
		// this is a Mult node, check number of children
		if (gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML()) < 2)
		{
			// no real Mult node
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// there is a subexpression, so no simple constant
				IntExpr* pExpr = read(pXML);
				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pExpr;
			}
			else
			{
				// constant
				int iConstant = 0;
				pXML->GetXmlVal(&iConstant);
				return new IntExprConstant(!iConstant);
			}
		}
		else
		{
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = read(pXML);
				
				// read nodes until there are no more siblings
				while (gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					pExpr = new IntExprMult(pExpr, read(pXML));
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "Minus") == 0)
	{
		// this is a Minus node, check number of children
		if (gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML()) != 2)
		{
			// Minus nodes must have two subexpressions, make it a constant 0 node
			return new IntExprConstant(0);
		}
		else
		{
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = read(pXML);
				
				// read the second node
				if (gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					pExpr = new IntExprMinus(pExpr, read(pXML));
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "Div") == 0)
	{
		// this is a Div node, check number of children
		if (gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML()) != 2)
		{
			// Div nodes must have two subexpressions, make it a constant 0 node
			return new IntExprConstant(0);
		}
		else
		{
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = read(pXML);
				
				// read the second node
				if (gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					pExpr = new IntExprDiv(pExpr, read(pXML));
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "If") == 0)
	{
		// this is an if/then/else node, check number of children
		if (gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML()) != 3)
		{
			// if/then/else nodes must have three subexpressions, make it a constant 0 node
			return new IntExprConstant(0);
		}
		else
		{
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// read the if node
				BoolExpr* pIfExpr = BoolExpr::read(pXML);
				IntExpr* pThenExpr = NULL;
				IntExpr* pElseExpr = NULL;
				
				// read the then node
				if (gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					pThenExpr = read(pXML);
				}
				// read the else node
				if (gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					pElseExpr = read(pXML);
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return new IntExprIf(pIfExpr, pThenExpr, pElseExpr);
			}
		}
	}

	// Check for the integrators
	if ((strcmp(szTag, "IntegrateSum") == 0) || (strcmp(szTag, "IntegrateAvg") == 0) || (strcmp(szTag, "IntegrateCount") == 0))
	{
		CvString szTextVal;
		pXML->GetChildXmlValByName(szTextVal, "RelationType");
		RelationTypes eRelation = (RelationTypes) pXML->FindInInfoClass(szTextVal);
		int iData = -1;
		pXML->GetChildXmlValByName(&iData, "iDistance");
		pXML->GetChildXmlValByName(szTextVal, "GameObjectType");
		GameObjectTypes eType = (GameObjectTypes) pXML->FindInInfoClass(szTextVal);
		
		IntExpr* pExpr = NULL;
		BoolExpr* pBExpr = NULL;
		// Find the expression and read it
		if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
		{
			bool bFound = false;
			TCHAR szInnerTag[1024];
			while (gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
			{
				if (!gDLL->getXMLIFace()->IsLastLocatedNodeCommentNode(pXML->GetXML()))
				{
					if (gDLL->getXMLIFace()->GetLastLocatedNodeTagName(pXML->GetXML(), szInnerTag))
					{
						if (!((strcmp(szInnerTag, "RelationType") == 0) || (strcmp(szInnerTag, "iDistance") == 0) || (strcmp(szInnerTag, "GameObjectType") == 0)))
						{
							bFound = true;
							if (strcmp(szTag, "IntegrateCount") == 0)
							{
								pBExpr = BoolExpr::read(pXML);
							}
							else
							{
								pExpr = IntExpr::read(pXML);
							}
							break;
						}
					}
				}
			}

			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			if (strcmp(szTag, "IntegrateSum") == 0)
			{
				return new IntExprIntegrateSum(pExpr, eRelation, iData, eType);
			}
			else if (strcmp(szTag, "IntegrateAvg") == 0)
			{
				return new IntExprIntegrateAvg(pExpr, eRelation, iData, eType);
			}
			else
			{
				return new IntExprIntegrateCount(pBExpr, eRelation, iData, eType);
			}
		}
	}

	// Check for the different literals
	if (strcmp(szTag, "PropertyType") == 0)
	{
		// this is a Property node, querying a property of the game object
		CvString szTextVal;
		pXML->GetXmlVal(szTextVal);
		return new IntExprProperty((PropertyTypes)pXML->FindInInfoClass(szTextVal));
	}

	if (strcmp(szTag, "AttributeType") == 0)
	{
		// this is an Attribute node, querying an attribute of the game object
		CvString szTextVal;
		pXML->GetXmlVal(szTextVal);
		return new IntExprAttribute((AttributeTypes)pXML->FindInInfoClass(szTextVal));
	}

	// Check if there is a subexpression and use that
	if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
	{
		// there is a subexpression, so no simple constant
		IntExpr* pExpr = read(pXML);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		return pExpr;
	}
	else
	{
		// constant
		int iConstant = 0;
		pXML->GetXmlVal(&iConstant);
		return new IntExprConstant(iConstant);
	}
}

IntExpr* IntExpr::readExpression(FDataStreamBase *pStream)
{
	IntExpr* pExpr = NULL;
	IntExprTypes eExprType = NO_INTEXPR;
	pStream->Read((int*)&eExprType);

	switch (eExprType)
	{
	case INTEXPR_CONSTANT:
		pExpr = new IntExprConstant();
		break;

	case INTEXPR_PROPERTY:
		pExpr = new IntExprProperty();
		break;

	case INTEXPR_ATTRIBUTE:
		pExpr = new IntExprAttribute();
		break;

	case INTEXPR_PLUS:
		pExpr = new IntExprPlus();
		break;

	case INTEXPR_MINUS:
		pExpr = new IntExprMinus();
		break;

	case INTEXPR_MULT:
		pExpr = new IntExprMult();
		break;

	case INTEXPR_DIV:
		pExpr = new IntExprDiv();
		break;

	case INTEXPR_IF:
		pExpr = new IntExprIf();
		break;

	case INTEXPR_INTEGRATE_SUM:
		pExpr = new IntExprIntegrateSum();
		break;

	case INTEXPR_INTEGRATE_AVG:
		pExpr = new IntExprIntegrateAvg();
		break;

	case INTEXPR_INTEGRATE_COUNT:
		pExpr = new IntExprIntegrateCount();
	}

	if (pExpr)
		pExpr->read(pStream);

	return pExpr;
}


int IntExprConstant::evaluate(CvGameObject *pObject)
{
	return m_iValue;
}

void IntExprConstant::read(FDataStreamBase *pStream)
{
	pStream->Read(&m_iValue);
}

void IntExprConstant::write(FDataStreamBase *pStream)
{
	pStream->Write((int)INTEXPR_CONSTANT);
	pStream->Write(m_iValue);
}

void IntExprConstant::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString s;
	s.Format(L"%d", m_iValue);
	szBuffer.append(s);
}


int IntExprAttribute::evaluate(CvGameObject *pObject)
{
	return pObject->getAttribute(m_eAttribute);
}

void IntExprAttribute::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eAttribute);
}

void IntExprAttribute::write(FDataStreamBase* pStream)
{
	pStream->Write((int)INTEXPR_ATTRIBUTE);
	pStream->Write((int)m_eAttribute);
}

void IntExprAttribute::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString s;
	switch (m_eAttribute)
	{
		case ATTRIBUTE_POPULATION:
			s.Format(L"%c", gDLL->getSymbolID(CITIZEN_CHAR));
			szBuffer.append(s);
			break;
		case ATTRIBUTE_HEALTH:
			s.Format(L"%c", gDLL->getSymbolID(HEALTHY_CHAR));
			szBuffer.append(s);
			break;
		case ATTRIBUTE_HAPPINESS:
			s.Format(L"%c", gDLL->getSymbolID(HAPPY_CHAR));
			szBuffer.append(s);
			break;
	}
}


int IntExprProperty::evaluate(CvGameObject *pObject)
{
	return pObject->getProperties()->getValueByProperty((int)m_eProperty);
}

void IntExprProperty::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eProperty);
}

void IntExprProperty::write(FDataStreamBase* pStream)
{
	pStream->Write((int)INTEXPR_PROPERTY);
	pStream->Write((int)m_eProperty);
}

void IntExprProperty::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	CvWString s;
	s.Format(L"%c", GC.getPropertyInfo(m_eProperty).getChar());
	szBuffer.append(s);
}


IntExprOp::~IntExprOp()
{
	SAFE_DELETE(m_pExpr1);
	SAFE_DELETE(m_pExpr2);
}

void IntExprOp::read(FDataStreamBase *pStream)
{
	m_pExpr1 = IntExpr::readExpression(pStream);
	m_pExpr2 = IntExpr::readExpression(pStream);
}

void IntExprOp::write(FDataStreamBase *pStream)
{
	pStream->Write((int)getType());
	m_pExpr1->write(pStream);
	m_pExpr2->write(pStream);
}

void IntExprOp::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("(");
	m_pExpr1->buildDisplayString(szBuffer);
	szBuffer.append(") ");
	buildOpNameString(szBuffer);
	szBuffer.append(" (");
	m_pExpr2->buildDisplayString(szBuffer);
	szBuffer.append(")");
}


int IntExprPlus::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) + m_pExpr2->evaluate(pObject);
}

IntExprTypes IntExprPlus::getType() const
{
	return INTEXPR_PLUS;
}

void IntExprPlus::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("+");
}


int IntExprMinus::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) - m_pExpr2->evaluate(pObject);
}

IntExprTypes IntExprMinus::getType() const
{
	return INTEXPR_MINUS;
}

void IntExprMinus::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("-");
}


int IntExprMult::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) * m_pExpr2->evaluate(pObject);
}

IntExprTypes IntExprMult::getType() const
{
	return INTEXPR_MULT;
}

void IntExprMult::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("*");
}


int IntExprDiv::evaluate(CvGameObject *pObject)
{
	int iDiv = m_pExpr2->evaluate(pObject);
	return iDiv ? m_pExpr1->evaluate(pObject) / iDiv : m_pExpr1->evaluate(pObject);
}

IntExprTypes IntExprDiv::getType() const
{
	return INTEXPR_DIV;
}

void IntExprDiv::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("/");
}


IntExprIf::~IntExprIf()
{
	SAFE_DELETE(m_pExprIf);
	SAFE_DELETE(m_pExprThen);
	SAFE_DELETE(m_pExprElse);
}

int IntExprIf::evaluate(CvGameObject *pObject)
{
	return m_pExprIf->evaluate(pObject) ? m_pExprThen->evaluate(pObject) : m_pExprElse->evaluate(pObject);
}

void IntExprIf::read(FDataStreamBase *pStream)
{
	m_pExprIf = BoolExpr::readExpression(pStream);
	m_pExprThen = IntExpr::readExpression(pStream);
	m_pExprElse = IntExpr::readExpression(pStream);
}

void IntExprIf::write(FDataStreamBase *pStream)
{
	pStream->Write((int)INTEXPR_IF);
	m_pExprIf->write(pStream);
	m_pExprThen->write(pStream);
	m_pExprElse->write(pStream);
}

void IntExprIf::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("If (");
	m_pExprIf->buildDisplayString(szBuffer);
	szBuffer.append(") Then (");
	m_pExprThen->buildDisplayString(szBuffer);
	szBuffer.append(") Else (");
	m_pExprElse->buildDisplayString(szBuffer);
	szBuffer.append(")");
}


IntExprIntegrateOp::~IntExprIntegrateOp()
{
	SAFE_DELETE(m_pExpr);
}

int IntExprIntegrateOp::evaluate(CvGameObject *pObject)
{
	int iAcc = 0;
	pObject->foreachRelated(m_eType, m_eRelation, boost::bind(getOp(), _1, m_pExpr, &iAcc));
	return iAcc;
}

void IntExprIntegrateOp::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eRelation);
	pStream->Read(&m_iData);
	pStream->Read((int*)&m_eType);
	m_pExpr = IntExpr::readExpression(pStream);
}

void IntExprIntegrateOp::write(FDataStreamBase *pStream)
{
	pStream->Write((int)getType());
	pStream->Write((int)m_eRelation);
	pStream->Write(m_iData);
	pStream->Write((int)m_eType);
	m_pExpr->write(pStream);
}

void IntExprIntegrateOp::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	// TODO: Proper rendering of relations
	m_pExpr->buildDisplayString(szBuffer);
}


void evalExprIntegrateSum(CvGameObject* pObject, IntExpr* pExpr, int* iAcc)
{
	*iAcc = *iAcc + pExpr->evaluate(pObject);
}

IntExprTypes IntExprIntegrateSum::getType() const
{
	return INTEXPR_INTEGRATE_SUM;
}

IntegrateOpFunc IntExprIntegrateSum::getOp()
{
	return &evalExprIntegrateSum;
}


void evalExprIntegrateAvg(CvGameObject* pObject, IntExpr* pExpr, int* iAcc, int* iCount)
{
	*iAcc = *iAcc + pExpr->evaluate(pObject);
	++*iCount;
}

IntExprTypes IntExprIntegrateAvg::getType() const
{
	return INTEXPR_INTEGRATE_AVG;
}

int IntExprIntegrateAvg::evaluate(CvGameObject *pObject)
{
	int iAcc = 0;
	int iCount = 0;
	pObject->foreachRelated(m_eType, m_eRelation, boost::bind(evalExprIntegrateAvg, _1, m_pExpr, &iAcc, &iCount));
	return iCount ? iAcc/iCount : 0;
}

IntegrateOpFunc IntExprIntegrateAvg::getOp()
{
	return NULL;
}


void evalExprIntegrateCount(CvGameObject* pObject, BoolExpr* pExpr, int* iAcc)
{
	if (pExpr->evaluate(pObject))
	{
		++*iAcc;
	}
}

IntExprIntegrateCount::~IntExprIntegrateCount()
{
	SAFE_DELETE(m_pExpr);
}

int IntExprIntegrateCount::evaluate(CvGameObject *pObject)
{
	int iAcc = 0;
	pObject->foreachRelated(m_eType, m_eRelation, boost::bind(evalExprIntegrateCount, _1, m_pExpr, &iAcc));
	return iAcc;
}

void IntExprIntegrateCount::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eRelation);
	pStream->Read(&m_iData);
	pStream->Read((int*)&m_eType);
	m_pExpr = BoolExpr::readExpression(pStream);
}

void IntExprIntegrateCount::write(FDataStreamBase *pStream)
{
	pStream->Write((int)INTEXPR_INTEGRATE_COUNT);
	pStream->Write((int)m_eRelation);
	pStream->Write(m_iData);
	pStream->Write((int)m_eType);
	m_pExpr->write(pStream);
}

void IntExprIntegrateCount::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	// TODO: Proper rendering of relations
	m_pExpr->buildDisplayString(szBuffer);
}