//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvPropertyManipulators.cpp
//
//  PURPOSE: Stores manipulators of generic properties for Civ4 classes (sources, interactions, propagators)
//
//------------------------------------------------------------------------------------------------

#include "CvGameCoreDLL.h"
#include "CvPropertyManipulators.h"
#include "CvXMLLoadUtility.h"
#include "CvDLLXMLIFaceBase.h"

CvPropertyManipulators::~CvPropertyManipulators()
{
	for (int i=0; i<(int)m_apSources.size(); i++)
	{
		delete m_apSources[i];
	}
	for (int i=0; i<(int)m_apInteractions.size(); i++)
	{
		delete m_apInteractions[i];
	}
	for (int i=0; i<(int)m_apPropagators.size(); i++)
	{
		delete m_apPropagators[i];
	}
}

int CvPropertyManipulators::getNumSources() const
{
	return (int) m_apSources.size();
}

CvPropertySource* CvPropertyManipulators::getSource(int index)
{
	FAssert(0 <= index);
	FAssert(index < (int)m_apSources.size());
	return m_apSources[index];
}

int CvPropertyManipulators::addSource(PropertySourceTypes eType)
{
	switch (eType)
	{
		case PROPERTYSOURCE_CONSTANT:
			m_apSources.push_back(new CvPropertySourceConstant());
			return (int)m_apSources.size()-1;
			break;
		case PROPERTYSOURCE_CONSTANT_LIMITED:
			m_apSources.push_back(new CvPropertySourceConstantLimited());
			return (int)m_apSources.size()-1;
			break;
		case PROPERTYSOURCE_DECAY:
			m_apSources.push_back(new CvPropertySourceDecay());
			return (int)m_apSources.size()-1;
			break;
		case PROPERTYSOURCE_ATTRIBUTE_CONSTANT:
			m_apSources.push_back(new CvPropertySourceAttributeConstant());
			return (int)m_apSources.size()-1;
			break;
	}
	return -1;
}

int CvPropertyManipulators::getNumInteractions() const
{
	return (int) m_apInteractions.size();
}

CvPropertyInteraction* CvPropertyManipulators::getInteraction(int index)
{
	FAssert(0 <= index);
	FAssert(index < (int)m_apInteractions.size());
	return m_apInteractions[index];
}

int CvPropertyManipulators::addInteraction(PropertyInteractionTypes eType)
{
	switch (eType)
	{
		case PROPERTYINTERACTION_CONVERT_CONSTANT:
			m_apInteractions.push_back(new CvPropertyInteractionConvertConstant());
			return (int)m_apInteractions.size()-1;
	}
	return -1;
}

int CvPropertyManipulators::getNumPropagators() const
{
	return (int) m_apPropagators.size();
}

CvPropertyPropagator* CvPropertyManipulators::getPropagator(int index)
{
	FAssert(0 <= index);
	FAssert(index < (int)m_apPropagators.size());
	return m_apPropagators[index];
}

int CvPropertyManipulators::addPropagator(PropertyPropagatorTypes eType)
{
	switch (eType)
	{
		case PROPERTYPROPAGATOR_SPREAD:
			m_apPropagators.push_back(new CvPropertyPropagatorSpread());
			return (int)m_apPropagators.size()-1;
			break;

		case PROPERTYPROPAGATOR_GATHER:
			m_apPropagators.push_back(new CvPropertyPropagatorGather());
			return (int)m_apPropagators.size()-1;
			break;

		case PROPERTYPROPAGATOR_DIFFUSE:
			m_apPropagators.push_back(new CvPropertyPropagatorDiffuse());
			return (int)m_apPropagators.size()-1;
			break;
	}
	return -1;
}

void CvPropertyManipulators::buildDisplayString(CvWStringBuffer &szBuffer) const
{
	for (int i=0; i<(int)m_apSources.size(); i++)
	{
		szBuffer.append(NEWLINE);
		m_apSources[i]->buildDisplayString(szBuffer);
	}
	for (int i=0; i<(int)m_apInteractions.size(); i++)
	{
		szBuffer.append(NEWLINE);
		m_apInteractions[i]->buildDisplayString(szBuffer);
	}
	for (int i=0; i<(int)m_apPropagators.size(); i++)
	{
		szBuffer.append(NEWLINE);
		m_apPropagators[i]->buildDisplayString(szBuffer);
	}
}

void CvPropertyManipulators::read(FDataStreamBase *pStream)
{
	int iNum;
	// Sources
	pStream->Read(&iNum);
	for (int i=0; i<iNum; i++)
	{
		int iType;
		pStream->Read(&iType);
		int iPos = addSource((PropertySourceTypes)iType);
		if (iPos != -1)
			m_apSources[iPos]->read(pStream);
	}

	// Interactions
	pStream->Read(&iNum);
	for (int i=0; i<iNum; i++)
	{
		int iType;
		pStream->Read(&iType);
		int iPos = addInteraction((PropertyInteractionTypes)iType);
		if (iPos != -1)
			m_apInteractions[iPos]->read(pStream);
	}

	// Propagators
	pStream->Read(&iNum);
	for (int i=0; i<iNum; i++)
	{
		int iType;
		pStream->Read(&iType);
		int iPos = addPropagator((PropertyPropagatorTypes)iType);
		if (iPos != -1)
			m_apPropagators[iPos]->read(pStream);
	}
}

void CvPropertyManipulators::write(FDataStreamBase *pStream)
{
	// Sources
	int iNum = (int)m_apSources.size();
	pStream->Write(iNum);
	for (int i=0; i<iNum; i++)
	{
		int iType = (int)m_apSources[i]->getType();
		pStream->Write(iType);
		m_apSources[i]->write(pStream);
	}

	// Interactions
	iNum = (int)m_apInteractions.size();
	pStream->Write(iNum);
	for (int i=0; i<iNum; i++)
	{
		int iType = (int)m_apInteractions[i]->getType();
		pStream->Write(iType);
		m_apInteractions[i]->write(pStream);
	}

	// Propagators
	iNum = (int)m_apPropagators.size();
	pStream->Write(iNum);
	for (int i=0; i<iNum; i++)
	{
		int iType = (int)m_apPropagators[i]->getType();
		pStream->Write(iType);
		m_apPropagators[i]->write(pStream);
	}
}

bool CvPropertyManipulators::read(CvXMLLoadUtility *pXML, const TCHAR* szTagName)
{
	if(gDLL->getXMLIFace()->SetToChildByTagName( pXML->GetXML(), szTagName))
	{
		if(gDLL->getXMLIFace()->SetToChild( pXML->GetXML() ))
		{

			if (gDLL->getXMLIFace()->LocateFirstSiblingNodeByTagName(pXML->GetXML(), "PropertySource"))
			{
				do
				{
					CvString szTextVal;
					pXML->GetChildXmlValByName(szTextVal, "PropertySourceType");
					int iType = pXML->FindInInfoClass(szTextVal);
					int iPos = addSource((PropertySourceTypes)iType);
					if (iPos != -1)
						m_apSources[iPos]->read(pXML);
				} while(gDLL->getXMLIFace()->LocateNextSiblingNodeByTagName(pXML->GetXML(), "PropertySource"));
			}
			if (gDLL->getXMLIFace()->LocateFirstSiblingNodeByTagName(pXML->GetXML(), "PropertyInteraction"))
			{
				do
				{
					CvString szTextVal;
					pXML->GetChildXmlValByName(szTextVal, "PropertyInteractionType");
					int iType = pXML->FindInInfoClass(szTextVal);
					int iPos = addInteraction((PropertyInteractionTypes)iType);
					if (iPos != -1)
						m_apInteractions[iPos]->read(pXML);
				} while(gDLL->getXMLIFace()->LocateNextSiblingNodeByTagName(pXML->GetXML(), "PropertyInteraction"));
			}
			if (gDLL->getXMLIFace()->LocateFirstSiblingNodeByTagName(pXML->GetXML(), "PropertyPropagator"))
			{
				do
				{
					CvString szTextVal;
					pXML->GetChildXmlValByName(szTextVal, "PropertyPropagatorType");
					int iType = pXML->FindInInfoClass(szTextVal);
					int iPos = addPropagator((PropertyPropagatorTypes)iType);
					if (iPos != -1)
						m_apPropagators[iPos]->read(pXML);
				} while(gDLL->getXMLIFace()->LocateNextSiblingNodeByTagName(pXML->GetXML(), "PropertyPropagator"));
			}
			gDLL->getXMLIFace()->SetToParent( pXML->GetXML() );
		}
		gDLL->getXMLIFace()->SetToParent( pXML->GetXML() );
	}

	return true;
}

void CvPropertyManipulators::copyNonDefaults(CvPropertyManipulators *pProp, CvXMLLoadUtility *pXML)
{
	if (m_apSources.size() == 0)
	{
		int iNum = pProp->getNumSources();
		for (int i=0; i<iNum; i++)
		{
			CvPropertySource* pSource = pProp->getSource(i);
			int iPos = addSource(pSource->getType());
			if (iPos != -1)
				m_apSources[iPos]->copyNonDefaults(pSource, pXML);
		}
	}
	if (m_apInteractions.size() == 0)
	{
		int iNum = pProp->getNumInteractions();
		for (int i=0; i<iNum; i++)
		{
			CvPropertyInteraction* pInteraction = pProp->getInteraction(i);
			int iPos = addInteraction(pInteraction->getType());
			if (iPos != -1)
				m_apInteractions[iPos]->copyNonDefaults(pInteraction, pXML);
		}
	}
	if (m_apPropagators.size() == 0)
	{
		int iNum = pProp->getNumPropagators();
		for (int i=0; i<iNum; i++)
		{
			CvPropertyPropagator* pPropagator = pProp->getPropagator(i);
			int iPos = addPropagator(pPropagator->getType());
			if (iPos != -1)
				m_apPropagators[iPos]->copyNonDefaults(pPropagator, pXML);
		}
	}
}