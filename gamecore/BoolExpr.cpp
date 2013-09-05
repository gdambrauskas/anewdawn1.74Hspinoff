//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:	BoolExpr.cpp
//
//  PURPOSE: Boolean Expressions for Civ4 classes
//
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "BoolExpr.h"
#include "CvDLLXMLIFaceBase.h"
#include <boost/bind.hpp>

BoolExpr::~BoolExpr()
{
}

BoolExpr* BoolExpr::read(CvXMLLoadUtility *pXML)
{
	// In general we assume no comments to simplify reading code

	TCHAR szTag[1024];
	if (!gDLL->getXMLIFace()->GetLastLocatedNodeTagName(pXML->GetXML(), szTag))
	{
		// No located node
		return NULL;
	}
	
	if (strcmp(szTag, "Not") == 0)
	{
		// this is a Not node, read the first subexpression and generate a Not expression
		if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
		{
			// there is a subexpression, so no simple constant
			BoolExpr* pExpr = read(pXML);
			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			return new BoolExprNot(pExpr);
		}
		else
		{
			// constant, no need to generate a real Not
			bool bConstant = false;
			pXML->GetXmlVal(&bConstant);
			return new BoolExprConstant(!bConstant);
		}
	}
	
	if (strcmp(szTag, "And") == 0)
	{
		// this is an And node, check number of children
		if (gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML()) < 2)
		{
			// no real And node
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// there is a subexpression, so no simple constant
				BoolExpr* pExpr = read(pXML);
				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pExpr;
			}
			else
			{
				// constant
				bool bConstant = false;
				pXML->GetXmlVal(&bConstant);
				return new BoolExprConstant(!bConstant);
			}
		}
		else
		{
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// read the first node
				BoolExpr* pExpr = read(pXML);
				
				// read nodes until there are no more siblings
				while (gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					pExpr = new BoolExprAnd(pExpr, read(pXML));
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "Or") == 0)
	{
		// this is an Or node, check number of children
		if (gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML()) < 2)
		{
			// no real Or node
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// there is a subexpression, so no simple constant
				BoolExpr* pExpr = read(pXML);
				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pExpr;
			}
			else
			{
				// constant
				bool bConstant = false;
				pXML->GetXmlVal(&bConstant);
				return new BoolExprConstant(!bConstant);
			}
		}
		else
		{
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// read the first node
				BoolExpr* pExpr = read(pXML);
				
				// read nodes until there are no more siblings
				while (gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					pExpr = new BoolExprOr(pExpr, read(pXML));
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "BEqual") == 0)
	{
		// this is a Boolean Expression comparison node, check number of children
		if (gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML()) != 2)
		{
			// BEqual nodes must have two boolean expressions, make it a constant false node
			return new BoolExprConstant(false);
		}
		else
		{
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// read the first node
				BoolExpr* pExpr = read(pXML);
				
				// read the second node
				if (gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					pExpr = new BoolExprBEqual(pExpr, read(pXML));
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pExpr;
			}
		}
	}

	if (strcmp(szTag, "Greater") == 0)
	{
		// this is a comparison node, check number of children
		if (gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML()) != 2)
		{
			// Comparison nodes must have two boolean expressions, make it a constant false node
			return new BoolExprConstant(false);
		}
		else
		{
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = IntExpr::read(pXML);
				BoolExpr* pBExpr = NULL;
				
				// read the second node
				if (gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					pBExpr = new BoolExprGreater(pExpr, IntExpr::read(pXML));
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pBExpr;
			}
		}
	}

	if (strcmp(szTag, "GreaterEqual") == 0)
	{
		// this is a comparison node, check number of children
		if (gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML()) != 2)
		{
			// Comparison nodes must have two boolean expressions, make it a constant false node
			return new BoolExprConstant(false);
		}
		else
		{
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = IntExpr::read(pXML);
				BoolExpr* pBExpr = NULL;
				
				// read the second node
				if (gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					pBExpr = new BoolExprGreaterEqual(pExpr, IntExpr::read(pXML));
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pBExpr;
			}
		}
	}

	if (strcmp(szTag, "Equal") == 0)
	{
		// this is a comparison node, check number of children
		if (gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML()) != 2)
		{
			// Comparison nodes must have two boolean expressions, make it a constant false node
			return new BoolExprConstant(false);
		}
		else
		{
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// read the first node
				IntExpr* pExpr = IntExpr::read(pXML);
				BoolExpr* pBExpr = NULL;
				
				// read the second node
				if (gDLL->getXMLIFace()->NextSibling(pXML->GetXML()))
				{
					pBExpr = new BoolExprEqual(pExpr, IntExpr::read(pXML));
				}

				gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
				return pBExpr;
			}
		}
	}

	if (strcmp(szTag, "If") == 0)
	{
		// this is an if/then/else node, check number of children
		if (gDLL->getXMLIFace()->GetNumChildren(pXML->GetXML()) != 3)
		{
			// if/then/else nodes must have three boolean expressions, make it a constant false node
			return new BoolExprConstant(false);
		}
		else
		{
			if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
			{
				// read the if node
				BoolExpr* pIfExpr = read(pXML);
				BoolExpr* pThenExpr = NULL;
				BoolExpr* pElseExpr = NULL;
				
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
				return new BoolExprIf(pIfExpr, pThenExpr, pElseExpr);
			}
		}
	}

	// Check for the integrators
	if (strcmp(szTag, "IntegrateOr") == 0)
	{
		CvString szTextVal;
		pXML->GetChildXmlValByName(szTextVal, "RelationType");
		RelationTypes eRelation = (RelationTypes) pXML->FindInInfoClass(szTextVal);
		int iData = -1;
		pXML->GetChildXmlValByName(&iData, "iDistance");
		pXML->GetChildXmlValByName(szTextVal, "GameObjectType");
		GameObjectTypes eType = (GameObjectTypes) pXML->FindInInfoClass(szTextVal);
		
		BoolExpr* pExpr = NULL;
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
							pExpr = BoolExpr::read(pXML);
							break;
						}
					}
				}
			}

			gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
			return new BoolExprIntegrateOr(pExpr, eRelation, iData, eType);
		}
	}

	// Check for the different literals
	if (strcmp(szTag, "Has") == 0)
	{
		// this is a Has GOM node
		BoolExprHas* pExpr = new BoolExprHas();
		pExpr->readContent(pXML);
		return pExpr;
	}

	if (strcmp(szTag, "Is") == 0)
	{
		// this is an Is node, querying a tag of the game object
		CvString szTextVal;
		pXML->GetXmlVal(szTextVal);
		return new BoolExprIs((TagTypes)pXML->FindInInfoClass(szTextVal));
	}

	// Check if there is a subexpression and use that
	if (gDLL->getXMLIFace()->SetToChild(pXML->GetXML()))
	{
		// there is a subexpression, so no simple constant
		BoolExpr* pExpr = read(pXML);
		gDLL->getXMLIFace()->SetToParent(pXML->GetXML());
		return pExpr;
	}
	else
	{
		// constant
		bool bConstant = false;
		pXML->GetXmlVal(&bConstant);
		return new BoolExprConstant(bConstant);
	}
}

BoolExpr* BoolExpr::readExpression(FDataStreamBase *pStream)
{
	BoolExpr* pExpr = NULL;
	BoolExprTypes eExprType = NO_BOOLEXPR;
	pStream->Read((int*)&eExprType);

	switch (eExprType)
	{
	case BOOLEXPR_CONSTANT:
		pExpr = new BoolExprConstant();
		break;

	case BOOLEXPR_HAS:
		pExpr = new BoolExprHas();
		break;

	case BOOLEXPR_IS:
		pExpr = new BoolExprIs();
		break;

	case BOOLEXPR_NOT:
		pExpr = new BoolExprNot();
		break;

	case BOOLEXPR_AND:
		pExpr = new BoolExprAnd();
		break;

	case BOOLEXPR_OR:
		pExpr = new BoolExprOr();
		break;

	case BOOLEXPR_BEQUAL:
		pExpr = new BoolExprBEqual();
		break;

	case BOOLEXPR_IF:
		pExpr = new BoolExprIf();
		break;

	case BOOLEXPR_INTEGRATE_OR:
		pExpr = new BoolExprIntegrateOr();
		break;

	case BOOLEXPR_GREATER:
		pExpr = new BoolExprGreater();
		break;

	case BOOLEXPR_GREATER_EQUAL:
		pExpr = new BoolExprGreaterEqual();
		break;

	case BOOLEXPR_EQUAL:
		pExpr = new BoolExprEqual();
		break;
	}

	if (pExpr)
		pExpr->read(pStream);

	return pExpr;
}


bool BoolExprConstant::evaluate(CvGameObject *pObject)
{
	return m_bValue;
}

void BoolExprConstant::read(FDataStreamBase *pStream)
{
	pStream->Read(&m_bValue);
}

void BoolExprConstant::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_CONSTANT);
	pStream->Write(m_bValue);
}

void BoolExprConstant::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append(m_bValue ? "True" : "False");
}


BoolExprHas::~BoolExprHas()
{
	GC.removeDelayedResolution((int*)&m_eGOM);
	GC.removeDelayedResolution(&m_iID);
}

bool BoolExprHas::evaluate(CvGameObject *pObject)
{
	return pObject->hasGOM(m_eGOM, m_iID);
}

void BoolExprHas::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eGOM);
	pStream->Read(&m_iID);
}

void BoolExprHas::write(FDataStreamBase* pStream)
{
	pStream->Write((int)BOOLEXPR_HAS);
	pStream->Write((int)m_eGOM);
	pStream->Write(m_iID);
}

void BoolExprHas::readContent(CvXMLLoadUtility* pXML)
{
	CvString szTextVal;
	pXML->GetChildXmlValByName(szTextVal, "GOMType");
	GC.addDelayedResolution((int*)&m_eGOM, szTextVal);
	pXML->GetChildXmlValByName(szTextVal, "ID");
	GC.addDelayedResolution(&m_iID, szTextVal);
}

void BoolExprHas::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append(L"<link=literal>");
	switch (m_eGOM)
	{
		case GOM_BUILDING:
			szBuffer.append(GC.getBuildingInfo((BuildingTypes)m_iID).getDescription());
			break;
		case GOM_PROMOTION:
			szBuffer.append(GC.getPromotionInfo((PromotionTypes)m_iID).getDescription());
			break;
		case GOM_TRAIT:
			szBuffer.append(GC.getTraitInfo((TraitTypes)m_iID).getDescription());
			break;
		case GOM_FEATURE:
			szBuffer.append(GC.getFeatureInfo((FeatureTypes)m_iID).getDescription());
			break;
		case GOM_OPTION:
			szBuffer.append(GC.getGameOptionInfo((GameOptionTypes)m_iID).getDescription());
			break;
		case GOM_TERRAIN:
			szBuffer.append(GC.getTerrainInfo((TerrainTypes)m_iID).getDescription());
			break;
		case GOM_GAMESPEED:
			szBuffer.append(GC.getGameSpeedInfo((GameSpeedTypes)m_iID).getDescription());
			break;
		case GOM_ROUTE:
			szBuffer.append(GC.getRouteInfo((RouteTypes)m_iID).getDescription());
			break;
		case GOM_BONUS:
			szBuffer.append(GC.getBonusInfo((BonusTypes)m_iID).getDescription());
			break;
		case GOM_UNITTYPE:
			szBuffer.append(GC.getUnitInfo((UnitTypes)m_iID).getDescription());
			break;
		case GOM_TECH:
			szBuffer.append(GC.getTechInfo((TechTypes)m_iID).getDescription());
			break;
		case GOM_CIVIC:
			szBuffer.append(GC.getCivicInfo((CivicTypes)m_iID).getDescription());
			break;
		case GOM_RELIGION:
			szBuffer.append(GC.getReligionInfo((ReligionTypes)m_iID).getDescription());
			break;
		case GOM_CORPORATION:
			szBuffer.append(GC.getCorporationInfo((CorporationTypes)m_iID).getDescription());
			break;
		case GOM_IMPROVEMENT:
			szBuffer.append(GC.getImprovementInfo((ImprovementTypes)m_iID).getDescription());
			break;
	}
	szBuffer.append(L"</link>");
}


bool BoolExprIs::evaluate(CvGameObject *pObject)
{
	return pObject->isTag(m_eTag);
}

void BoolExprIs::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eTag);
}

void BoolExprIs::write(FDataStreamBase* pStream)
{
	pStream->Write((int)BOOLEXPR_IS);
	pStream->Write((int)m_eTag);
}

void BoolExprIs::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	// This is only the quick and dirty variant. Needs some proper text usage still.
	switch (m_eTag)
	{
		case TAG_WATER:
			szBuffer.append("Is on or near water");
			break;
		case TAG_FRESH_WATER:
			szBuffer.append("Has Fresh Water");
			break;
	}
}


BoolExprNot::~BoolExprNot()
{
	SAFE_DELETE(m_pExpr);
}

bool BoolExprNot::evaluate(CvGameObject *pObject)
{
	return !m_pExpr->evaluate(pObject);
}

void BoolExprNot::read(FDataStreamBase *pStream)
{
	m_pExpr = BoolExpr::readExpression(pStream);
}

void BoolExprNot::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_NOT);
	m_pExpr->write(pStream);
}

void BoolExprNot::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("Not (");
	m_pExpr->buildDisplayString(szBuffer);
	szBuffer.append(")");
}


BoolExprAnd::~BoolExprAnd()
{
	SAFE_DELETE(m_pExpr1);
	SAFE_DELETE(m_pExpr2);
}

bool BoolExprAnd::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) && m_pExpr2->evaluate(pObject);
}

void BoolExprAnd::read(FDataStreamBase *pStream)
{
	m_pExpr1 = BoolExpr::readExpression(pStream);
	m_pExpr2 = BoolExpr::readExpression(pStream);
}

void BoolExprAnd::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_AND);
	m_pExpr1->write(pStream);
	m_pExpr2->write(pStream);
}

void BoolExprAnd::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("(");
	m_pExpr1->buildDisplayString(szBuffer);
	szBuffer.append(") And (");
	m_pExpr2->buildDisplayString(szBuffer);
	szBuffer.append(")");
}


BoolExprOr::~BoolExprOr()
{
	SAFE_DELETE(m_pExpr1);
	SAFE_DELETE(m_pExpr2);
}

bool BoolExprOr::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) || m_pExpr2->evaluate(pObject);
}

void BoolExprOr::read(FDataStreamBase *pStream)
{
	m_pExpr1 = BoolExpr::readExpression(pStream);
	m_pExpr2 = BoolExpr::readExpression(pStream);
}

void BoolExprOr::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_OR);
	m_pExpr1->write(pStream);
	m_pExpr2->write(pStream);
}

void BoolExprOr::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("(");
	m_pExpr1->buildDisplayString(szBuffer);
	szBuffer.append(") Or (");
	m_pExpr2->buildDisplayString(szBuffer);
	szBuffer.append(")");
}


BoolExprBEqual::~BoolExprBEqual()
{
	SAFE_DELETE(m_pExpr1);
	SAFE_DELETE(m_pExpr2);
}

bool BoolExprBEqual::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) == m_pExpr2->evaluate(pObject);
}

void BoolExprBEqual::read(FDataStreamBase *pStream)
{
	m_pExpr1 = BoolExpr::readExpression(pStream);
	m_pExpr2 = BoolExpr::readExpression(pStream);
}

void BoolExprBEqual::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_BEQUAL);
	m_pExpr1->write(pStream);
	m_pExpr2->write(pStream);
}

void BoolExprBEqual::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("(");
	m_pExpr1->buildDisplayString(szBuffer);
	szBuffer.append(") Equals (");
	m_pExpr2->buildDisplayString(szBuffer);
	szBuffer.append(")");
}


BoolExprIf::~BoolExprIf()
{
	SAFE_DELETE(m_pExprIf);
	SAFE_DELETE(m_pExprThen);
	SAFE_DELETE(m_pExprElse);
}

bool BoolExprIf::evaluate(CvGameObject *pObject)
{
	return m_pExprIf->evaluate(pObject) ? m_pExprThen->evaluate(pObject) : m_pExprElse->evaluate(pObject);
}

void BoolExprIf::read(FDataStreamBase *pStream)
{
	m_pExprIf = BoolExpr::readExpression(pStream);
	m_pExprThen = BoolExpr::readExpression(pStream);
	m_pExprElse = BoolExpr::readExpression(pStream);
}

void BoolExprIf::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_IF);
	m_pExprIf->write(pStream);
	m_pExprThen->write(pStream);
	m_pExprElse->write(pStream);
}

void BoolExprIf::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("If (");
	m_pExprIf->buildDisplayString(szBuffer);
	szBuffer.append(") Then (");
	m_pExprThen->buildDisplayString(szBuffer);
	szBuffer.append(") Else (");
	m_pExprElse->buildDisplayString(szBuffer);
	szBuffer.append(")");
}


void evalExprIntegrateOr(CvGameObject* pObject, BoolExpr* pExpr, bool* bAcc)
{
	*bAcc = *bAcc || pExpr->evaluate(pObject);
}

BoolExprIntegrateOr::~BoolExprIntegrateOr()
{
	SAFE_DELETE(m_pExpr);
}

bool BoolExprIntegrateOr::evaluate(CvGameObject *pObject)
{
	bool bAcc = false;
	pObject->foreachRelated(m_eType, m_eRelation, boost::bind(evalExprIntegrateOr, _1, m_pExpr, &bAcc));
	return bAcc;
}

void BoolExprIntegrateOr::read(FDataStreamBase *pStream)
{
	pStream->Read((int*)&m_eRelation);
	pStream->Read(&m_iData);
	pStream->Read((int*)&m_eType);
	m_pExpr = BoolExpr::readExpression(pStream);
}

void BoolExprIntegrateOr::write(FDataStreamBase *pStream)
{
	pStream->Write((int)BOOLEXPR_INTEGRATE_OR);
	pStream->Write((int)m_eRelation);
	pStream->Write(m_iData);
	pStream->Write((int)m_eType);
	m_pExpr->write(pStream);
}

void BoolExprIntegrateOr::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	// TODO: Proper rendering of relations
	m_pExpr->buildDisplayString(szBuffer);
}


BoolExprComp::~BoolExprComp()
{
	SAFE_DELETE(m_pExpr1);
	SAFE_DELETE(m_pExpr2);
}

void BoolExprComp::read(FDataStreamBase *pStream)
{
	m_pExpr1 = IntExpr::readExpression(pStream);
	m_pExpr2 = IntExpr::readExpression(pStream);
}

void BoolExprComp::write(FDataStreamBase *pStream)
{
	pStream->Write((int)getType());
	m_pExpr1->write(pStream);
	m_pExpr2->write(pStream);
}

void BoolExprComp::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("(");
	m_pExpr1->buildDisplayString(szBuffer);
	szBuffer.append(") ");
	buildOpNameString(szBuffer);
	szBuffer.append(" (");
	m_pExpr2->buildDisplayString(szBuffer);
	szBuffer.append(")");
}


bool BoolExprGreater::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) > m_pExpr2->evaluate(pObject);
}

BoolExprTypes BoolExprGreater::getType() const
{
	return BOOLEXPR_GREATER;
}

void BoolExprGreater::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append(">");
}


bool BoolExprGreaterEqual::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) >= m_pExpr2->evaluate(pObject);
}

BoolExprTypes BoolExprGreaterEqual::getType() const
{
	return BOOLEXPR_GREATER_EQUAL;
}

void BoolExprGreaterEqual::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append(">=");
}


bool BoolExprEqual::evaluate(CvGameObject *pObject)
{
	return m_pExpr1->evaluate(pObject) == m_pExpr2->evaluate(pObject);
}

BoolExprTypes BoolExprEqual::getType() const
{
	return BOOLEXPR_EQUAL;
}

void BoolExprEqual::buildOpNameString(CvWStringBuffer &szBuffer) const
{
	szBuffer.append("=");
}
