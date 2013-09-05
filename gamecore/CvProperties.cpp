//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:	CvProperties.cpp
//
//  PURPOSE: Generic properties for Civ4 classes
//
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "CvProperties.h"
//#include "CvGlobals.h"
//#include "CvArtFileMgr.h"
#include "CvXMLLoadUtility.h"
#include "CvDLLXMLIFaceBase.h"
//#include "CvGameTextMgr.h"
//#include "CvGameCoreUtils.h"
#include "CvTaggedSaveFormatWrapper.h"
#include <boost/bind.hpp>

CvProperties::CvProperties()
{
	m_pGameObject = NULL;
}

CvProperties::CvProperties(CvGame* pGame)
{
	m_pGameObject = pGame->getGameObject();
}

CvProperties::CvProperties(CvTeam* pTeam)
{
	m_pGameObject = pTeam->getGameObject();
}

CvProperties::CvProperties(CvPlayer* pPlayer)
{
	m_pGameObject = pPlayer->getGameObject();
}

CvProperties::CvProperties(CvCity* pCity)
{
	m_pGameObject = pCity->getGameObject();
}

CvProperties::CvProperties(CvUnit* pUnit)
{
	m_pGameObject = pUnit->getGameObject();
}

CvProperties::CvProperties(CvPlot* pPlot)
{
	m_pGameObject = pPlot->getGameObject();
}

int CvProperties::getProperty(int index) const
{
	FAssert(0 <= index);
	FAssert(index < (int)m_aiProperty.size());
	return m_aiProperty[index].first;
}

int CvProperties::getValue(int index) const
{
	FAssert(0 <= index);
	FAssert(index < (int)m_aiProperty.size());
	return m_aiProperty[index].second;
}

int CvProperties::getNumProperties() const
{
	return m_aiProperty.size();
}

int CvProperties::getPositionByProperty(int eProp) const
{
	for (std::vector<std::pair<int,int> >::const_iterator it = m_aiProperty.begin();it!=m_aiProperty.end(); it++)
		if ( it->first==eProp )
			return it - m_aiProperty.begin();
	return -1;
}

int CvProperties::getValueByProperty(int eProp) const
{
	int index = getPositionByProperty(eProp);
	if (index < 0)
		return 0;
	else
		return getValue(index);
}

int CvProperties::getChangeByProperty(int eProp) const
{
	for (std::vector<std::pair<int,int> >::const_iterator it = m_aiPropertyChange.begin();it!=m_aiPropertyChange.end(); it++)
		if ( it->first==eProp )
			return it->second;
	return 0;
}

void CvProperties::setChangeByProperty(int eProp, int iVal)
{
	for (std::vector<std::pair<int,int> >::iterator it = m_aiPropertyChange.begin();it!=m_aiPropertyChange.end(); it++)
		if ( it->first==eProp )
			it->second = iVal;
	m_aiPropertyChange.push_back(std::pair<int, int>(eProp,iVal));;
}

void CvProperties::changeChangeByProperty(int eProp, int iChange)
{
	for (std::vector<std::pair<int,int> >::iterator it = m_aiPropertyChange.begin();it!=m_aiPropertyChange.end(); it++)
		if ( it->first==eProp )
			it->second += iChange;
	m_aiPropertyChange.push_back(std::pair<int, int>(eProp,iChange));;
}

void CvProperties::setValue(int index, int iVal)
{
	FAssert(0 <= index);
	FAssert(index < (int)m_aiProperty.size());
	int iOldVal = m_aiProperty[index].second;
	if (iOldVal != iVal)
	{
		m_aiProperty[index].second = iVal;
		if (m_pGameObject)
			m_pGameObject->eventPropertyChanged((PropertyTypes)m_aiProperty[index].first, iVal);
		// If this sets the value to 0, remove the property
		if (iVal == 0)
			m_aiProperty.erase(m_aiProperty.begin()+index);
	}
}

void CvProperties::setValueByProperty(int eProp, int iVal)
{
	int index = getPositionByProperty(eProp);
	if (index < 0)
	{
		if (iVal != 0)
		{
			m_aiProperty.push_back(std::pair<int, int>(eProp,iVal));
			if (m_pGameObject)
				m_pGameObject->eventPropertyChanged((PropertyTypes)eProp, iVal);
		}
	}
	else
		setValue(index, iVal);
}

void CvProperties::changeValue(int index, int iChange)
{
	if (iChange == 0)
		return;

	int eProperty = getProperty(index);

	setValue(index, getValue(index) + iChange);
	changeChangeByProperty(eProperty, iChange);
	if (m_pGameObject)
	{
		propagateChange(eProperty, iChange);
	}
}

void CvProperties::changeValueByProperty(int eProp, int iChange)
{
	if (iChange == 0)
		return;

	int index = getPositionByProperty(eProp);
	if (index < 0)
	{
		m_aiProperty.push_back(std::pair<int, int>(eProp,iChange));
		changeChangeByProperty(eProp, iChange);
		if (m_pGameObject)
		{
			propagateChange(eProp, iChange);
			m_pGameObject->eventPropertyChanged((PropertyTypes)eProp, iChange);
		}
	}
	else
		changeValue(index, iChange);
}

// helper function for propagating change
void callChangeValueByProperty(CvGameObject* pObject, int eProp, int iChange)
{
	pObject->getProperties()->changeValueByProperty(eProp, iChange);
}

void CvProperties::propagateChange(int eProp, int iChange)
{
	CvPropertyInfo& kInfo = GC.getPropertyInfo((PropertyTypes)eProp);
	for (int iI = 0; iI < NUM_GAMEOBJECTS; iI++)
	{
		int iChangePercent = kInfo.getChangePropagator(m_pGameObject->getGameObjectType(), (GameObjectTypes)iI);
		if (iChangePercent)
		{
			int iPropChange = (iChange * iChangePercent) / 100;
			m_pGameObject->foreach((GameObjectTypes)iI, boost::bind(callChangeValueByProperty, _1, eProp, iPropChange));
		}
	}
}

void CvProperties::addProperties(const CvProperties* pProp)
{
	int num = pProp->getNumProperties();
	for (int index = 0; index < num; index++)
	{
		changeValueByProperty(pProp->getProperty(index), pProp->getValue(index));
	}
}

void CvProperties::subtractProperties(const CvProperties* pProp)
{
	int num = pProp->getNumProperties();
	for (int index = 0; index < num; index++)
	{
		changeValueByProperty(pProp->getProperty(index), - pProp->getValue(index));
	}
}

bool CvProperties::isEmpty() const
{
	return m_aiProperty.empty();
}

void CvProperties::clear()
{
	m_aiProperty.clear();
}

void CvProperties::clearChange()
{
	m_aiPropertyChange.clear();
}

bool isNotSourceDrainProperty(std::pair<int,int> p)
{
	return !GC.getPropertyInfo((PropertyTypes)p.first).isSourceDrain();
}

void CvProperties::clearForRecalculate()
{
	m_aiProperty.erase(std::remove_if(m_aiProperty.begin(), m_aiProperty.end(), isNotSourceDrainProperty), m_aiProperty.end());
}

void CvProperties::read(FDataStreamBase *pStream)
{
	int num;
	int eProp;
	int iVal;
	pStream->Read(&num);
	for (int i = 0; i < num; i++)
	{
		pStream->Read(&eProp);
		pStream->Read(&iVal);
		setValueByProperty(eProp, iVal);
	}
}

void CvProperties::readWrapper(FDataStreamBase *pStream)
{
	int iPropertyNum = 0;
	int eProp;
	int iVal;

	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();
	wrapper.AttachToStream(pStream);

	//WRAPPER_READ_OBJECT_START(wrapper);

	WRAPPER_READ(wrapper, "CvProperties",&iPropertyNum);
	for (int i = 0; i < iPropertyNum; i++)
	{
		eProp = -1;
		WRAPPER_READ_CLASS_ENUM_ALLOW_MISSING(wrapper, "CvProperties", REMAPPED_CLASS_TYPE_PROPERTIES, &eProp);
		if (eProp == -1)
		{
			// Handle old save game before property remapping
			WRAPPER_READ(wrapper, "CvProperties",&eProp);
			if (eProp == 0) // crime
				eProp = GC.getInfoTypeForString("PROPERTY_CRIME");
			else if (eProp == 1) // flammability
				eProp = GC.getInfoTypeForString("PROPERTY_FLAMMABILITY");
			if (eProp == -1) // removed property
			{
				WRAPPER_SKIP_ELEMENT(wrapper, "CvProperties", "iVal", SAVE_VALUE_TYPE_INT);
				continue;
			}
		}
		WRAPPER_READ(wrapper, "CvProperties",&iVal);
		setValueByProperty(eProp, iVal);
	}

	//WRAPPER_READ_OBJECT_END(wrapper);
}

void CvProperties::write(FDataStreamBase *pStream)
{
	int iPropertyNum = getNumProperties();
	pStream->Write(iPropertyNum);
	for (int i = 0; i < iPropertyNum; i++)
	{
		pStream->Write(getProperty(i));
		pStream->Write(getValue(i));
	}
}

void CvProperties::writeWrapper(FDataStreamBase *pStream)
{
	int iPropertyNum = getNumProperties();
	int eProp;
	int iVal;

	CvTaggedSaveFormatWrapper&	wrapper = CvTaggedSaveFormatWrapper::getSaveFormatWrapper();
	wrapper.AttachToStream(pStream);

	//WRAPPER_WRITE_OBJECT_START(wrapper);

	WRAPPER_WRITE(wrapper, "CvProperties",iPropertyNum);
	for (int i = 0; i < iPropertyNum; i++)
	{
		eProp = getProperty(i);
		iVal = getValue(i);
		WRAPPER_WRITE_CLASS_ENUM(wrapper, "CvProperties", REMAPPED_CLASS_TYPE_PROPERTIES, eProp);
		WRAPPER_WRITE(wrapper, "CvProperties",iVal);
	}

	//WRAPPER_WRITE_OBJECT_END(wrapper);
}

bool CvProperties::read(CvXMLLoadUtility* pXML, const TCHAR* szTagName)
{
	if(gDLL->getXMLIFace()->SetToChildByTagName( pXML->GetXML(), szTagName))
	{
		if(gDLL->getXMLIFace()->SetToChild( pXML->GetXML() ))
		{

			if (gDLL->getXMLIFace()->LocateFirstSiblingNodeByTagName(pXML->GetXML(), "Property"))
			{
				do
				{
					int iVal;
					CvString szTextVal;
					pXML->GetChildXmlValByName(szTextVal, "PropertyType");
					int eProp = pXML->FindInInfoClass(szTextVal);
					pXML->GetChildXmlValByName(&iVal, "iPropertyValue");
					setValueByProperty(eProp, iVal);
				} while(gDLL->getXMLIFace()->NextSibling(pXML->GetXML()));
			}
			gDLL->getXMLIFace()->SetToParent( pXML->GetXML() );
		}
		gDLL->getXMLIFace()->SetToParent( pXML->GetXML() );
	}

	return true;
}

void CvProperties::copyNonDefaults(CvProperties* pProp, CvXMLLoadUtility* pXML )
{
	int num = pProp->getNumProperties();
	for (int index = 0; index < num; index++)
	{
		if (getPositionByProperty(pProp->getProperty(index)) < 0)
			setValueByProperty(pProp->getProperty(index), pProp->getValue(index));
	}
}

bool CvProperties::operator<(const CvProperties& prop) const
{
	int num = prop.getNumProperties();
	for (int index = 0; index < num; index++)
	{
		if (getValueByProperty(prop.getProperty(index)) >= prop.getValue(index))
			return false;
	}
	return true;
}

bool CvProperties::operator<=(const CvProperties& prop) const
{
	int num = prop.getNumProperties();
	for (int index = 0; index < num; index++)
	{
		if (getValueByProperty(prop.getProperty(index)) > prop.getValue(index))
			return false;
	}
	return true;
}

bool CvProperties::operator>(const CvProperties& prop) const
{
	int num = prop.getNumProperties();
	for (int index = 0; index < num; index++)
	{
		if (getValueByProperty(prop.getProperty(index)) <= prop.getValue(index))
			return false;
	}
	return true;
}

bool CvProperties::operator>=(const CvProperties& prop) const
{
	int num = prop.getNumProperties();
	for (int index = 0; index < num; index++)
	{
		if (getValueByProperty(prop.getProperty(index)) < prop.getValue(index))
			return false;
	}
	return true;
}

bool CvProperties::operator==(const CvProperties& prop) const
{
	int num = prop.getNumProperties();
	for (int index = 0; index < num; index++)
	{
		if (getValueByProperty(prop.getProperty(index)) != prop.getValue(index))
			return false;
	}
	return true;
}

bool CvProperties::operator!=(const CvProperties& prop) const
{
	int num = prop.getNumProperties();
	for (int index = 0; index < num; index++)
	{
		if (getValueByProperty(prop.getProperty(index)) == prop.getValue(index))
			return false;
	}
	return true;
}

void CvProperties::buildRequiresMinString(CvWStringBuffer& szBuffer, const CvProperties& prop) const
{
	int num = getNumProperties();
	for (int index = 0; index < num; index++)
	{
		if (prop.getValueByProperty(getProperty(index)) < getValue(index))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText(GC.getPropertyInfo((PropertyTypes)getProperty(index)).getPrereqMinDisplayText(), getValue(index)));
		}
	}
}

void CvProperties::buildRequiresMaxString(CvWStringBuffer& szBuffer, const CvProperties& prop) const
{
	int num = getNumProperties();
	for (int index = 0; index < num; index++)
	{
		if (prop.getValueByProperty(getProperty(index)) > getValue(index))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText(GC.getPropertyInfo((PropertyTypes)getProperty(index)).getPrereqMaxDisplayText(), getValue(index)));
		}
	}
}

void CvProperties::buildChangesString(CvWStringBuffer& szBuffer, CvWString* pszCity) const
{
	int num = getNumProperties();
	for (int iI = 0; iI < num; iI++)
	{
		szBuffer.append(NEWLINE);
		if (pszCity)
		{
			szBuffer.append(*pszCity);
			szBuffer.append(": ");
		}
		CvWString szTemp;
		szTemp.Format(L"%c: %+d", GC.getPropertyInfo((PropertyTypes)getProperty(iI)).getChar(), getValue(iI));
		szBuffer.append(szTemp);
		//szBuffer.append(gDLL->getText(GC.getPropertyInfo((PropertyTypes)getProperty(iI)).getChangeDisplayText(), getValue(iI), GC.getPropertyInfo((PropertyTypes)getProperty(iI)).getChar()));
	}
}

void CvProperties::buildCompactChangesString(CvWStringBuffer& szBuffer) const
{
	int num = getNumProperties();
	for (int iI = 0; iI < num; iI++)
	{
		CvWString szTemp;
		szTemp.Format(L" %d%c", getValue(iI), GC.getPropertyInfo((PropertyTypes)getProperty(iI)).getChar());
		szBuffer.append(szTemp);
	}
}

void CvProperties::buildChangesAllCitiesString(CvWStringBuffer& szBuffer) const
{
	int num = getNumProperties();
	for (int iI = 0; iI < num; iI++)
	{
		szBuffer.append(NEWLINE);
		CvWString szTemp;
		szTemp.Format(L"%c (All Cities): %+d", GC.getPropertyInfo((PropertyTypes)getProperty(iI)).getChar(), getValue(iI));
		szBuffer.append(szTemp);
		//szBuffer.append(gDLL->getText(GC.getPropertyInfo((PropertyTypes)getProperty(iI)).getChangeAllCitiesDisplayText(), getValue(iI), GC.getPropertyInfo((PropertyTypes)getProperty(iI)).getChar()));
	}
}

void CvProperties::buildDisplayString(CvWStringBuffer& szBuffer) const
{
	int num = getNumProperties();
	for (int iI = 0; iI < num; iI++)
	{
		szBuffer.append(NEWLINE);
		CvWString szTemp;
		szTemp.Format(L"%c: %d", GC.getPropertyInfo((PropertyTypes)getProperty(iI)).getChar(), getValue(iI));
		szBuffer.append(szTemp);
		//szBuffer.append(gDLL->getText(GC.getPropertyInfo((PropertyTypes)getProperty(iI)).getValueDisplayText(), getValue(iI), CvWString::format(L"%c", GC.getPropertyInfo((PropertyTypes) iI).getChar())));
	}
}

std::wstring CvProperties::getPropertyDisplay(int index) const
{
	CvWString szTemp;
	if (index < getNumProperties())
		szTemp.Format(L"%c %s", GC.getPropertyInfo((PropertyTypes)getProperty(index)).getChar(), GC.getPropertyInfo((PropertyTypes)getProperty(index)).getText());
	return szTemp.GetCString();
}
