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

CvProperties::CvProperties()
{
	m_eParentType = NO_GAMEOBJECT;
}

CvProperties::CvProperties(CvGame* pGame) : m_pGame(pGame)
{
	m_eParentType = GAMEOBJECT_GAME;
}

CvProperties::CvProperties(CvTeam* pTeam) : m_pTeam(pTeam)
{
	m_eParentType = GAMEOBJECT_TEAM;
}

CvProperties::CvProperties(CvPlayer* pPlayer) : m_pPlayer(pPlayer)
{
	m_eParentType = GAMEOBJECT_PLAYER;
}

CvProperties::CvProperties(CvCity* pCity) : m_pCity(pCity)
{
	m_eParentType = GAMEOBJECT_CITY;
}

CvProperties::CvProperties(CvUnit* pUnit) : m_pUnit(pUnit)
{
	m_eParentType = GAMEOBJECT_UNIT;
}

CvProperties::CvProperties(CvPlot* pPlot) : m_pPlot(pPlot)
{
	m_eParentType = GAMEOBJECT_PLOT;
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
		if ( (*it).first==eProp )
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

void CvProperties::setValue(int index, int iVal)
{
	FAssert(0 <= index);
	FAssert(index < (int)m_aiProperty.size());
	m_aiProperty[index].second = iVal;
}

void CvProperties::setValueByProperty(int eProp, int iVal)
{
	int index = getPositionByProperty(eProp);
	if (index < 0)
	{
		m_aiProperty.push_back(std::pair<int, int>(eProp,iVal));
	}
	else
		setValue(index, iVal);
}

void CvProperties::changeValue(int index, int iChange)
{
	setValue(index, getValue(index) + iChange);
	if (m_eParentType != NO_GAMEOBJECT)
	{
		propagateChange(getProperty(index), iChange);
	}
}

void CvProperties::changeValueByProperty(int eProp, int iChange)
{
	int index = getPositionByProperty(eProp);
	if (index < 0)
	{
		m_aiProperty.push_back(std::pair<int, int>(eProp,iChange));
		if (m_eParentType != NO_GAMEOBJECT)
		{
			propagateChange(eProp, iChange);
		}
	}
	else
		changeValue(index, iChange);
}

void CvProperties::propagateChange(int eProp, int iChange)
{
	CvPropertyInfo& kInfo = GC.getPropertyInfo((PropertyTypes)eProp);
	int iChangePercent = kInfo.getChangePropagator(m_eParentType, GAMEOBJECT_GAME);
	if (iChangePercent)
	{
		GC.getGameINLINE().getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
	}
	
	iChangePercent = kInfo.getChangePropagator(m_eParentType, GAMEOBJECT_TEAM);
	if (iChangePercent)
	{
		switch(m_eParentType)
		{
			case GAMEOBJECT_GAME:
				for (int iI = 0; iI < MAX_TEAMS; iI++)
				{
					if (GET_TEAM((TeamTypes)iI).isAlive())
					{
						GET_TEAM((TeamTypes)iI).getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
					}
				}
				break;

			case GAMEOBJECT_PLAYER:
				GET_TEAM(m_pPlayer->getTeam()).getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				break;

			case GAMEOBJECT_CITY:
				GET_TEAM(m_pCity->getTeam()).getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				break;

			case GAMEOBJECT_UNIT:
				GET_TEAM(m_pUnit->getTeam()).getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				break;

			case GAMEOBJECT_PLOT:
				if (m_pPlot->getTeam() != NO_TEAM)
					GET_TEAM(m_pPlot->getTeam()).getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				break;
		}
	}

	iChangePercent = kInfo.getChangePropagator(m_eParentType, GAMEOBJECT_PLAYER);
	if (iChangePercent)
	{
		switch(m_eParentType)
		{
			case GAMEOBJECT_GAME:
				for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
				{
					CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
					if (kLoopPlayer.isAlive())
					{
						kLoopPlayer.getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
					}
				}
				break;

			case GAMEOBJECT_TEAM:
				for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
				{
					CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
					if (kLoopPlayer.isAlive())
					{
						if (kLoopPlayer.getTeam() == m_pTeam->getID())
						{
							kLoopPlayer.getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
						}
					}
				}
				break;

			case GAMEOBJECT_CITY:
				GET_PLAYER(m_pCity->getOwnerINLINE()).getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				break;

			case GAMEOBJECT_UNIT:
				GET_PLAYER(m_pUnit->getOwnerINLINE()).getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				break;

			case GAMEOBJECT_PLOT:
				if (m_pPlot->getOwnerINLINE() != NO_PLAYER)
					GET_PLAYER(m_pPlot->getOwnerINLINE()).getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				break;
		}
	}

	iChangePercent = kInfo.getChangePropagator(m_eParentType, GAMEOBJECT_CITY);
	if (iChangePercent)
	{
		switch(m_eParentType)
		{
			case GAMEOBJECT_GAME:
				for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
				{
					CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
					if (kLoopPlayer.isAlive())
					{
						int iLoop;
						for (CvCity* pCity = kLoopPlayer.firstCity(&iLoop); pCity != NULL; pCity = kLoopPlayer.nextCity(&iLoop))
						{
							pCity->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
						}
					}
				}
				break;

			case GAMEOBJECT_TEAM:
				for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
				{
					CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
					if (kLoopPlayer.isAlive())
					{
						if (kLoopPlayer.getTeam() == m_pTeam->getID())
						{
							int iLoop;
							for (CvCity* pCity = kLoopPlayer.firstCity(&iLoop); pCity != NULL; pCity = kLoopPlayer.nextCity(&iLoop))
							{
								pCity->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
							}
						}
					}
				}
				break;

			case GAMEOBJECT_PLAYER:
				int iLoop;
				for (CvCity* pCity = m_pPlayer->firstCity(&iLoop); pCity != NULL; pCity = m_pPlayer->nextCity(&iLoop))
				{
					pCity->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				}
				break;

			case GAMEOBJECT_UNIT:
				if (m_pUnit->plot()->getPlotCity())
					m_pUnit->plot()->getPlotCity()->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				break;

			case GAMEOBJECT_PLOT:
				if (m_pPlot->getPlotCity())
					m_pPlot->getPlotCity()->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				break;
		}
	}

	iChangePercent = kInfo.getChangePropagator(m_eParentType, GAMEOBJECT_UNIT);
	if (iChangePercent)
	{
		switch(m_eParentType)
		{
			case GAMEOBJECT_GAME:
				for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
				{
					CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
					if (kLoopPlayer.isAlive())
					{
						int iLoop;
						for (CvUnit* pUnit = kLoopPlayer.firstUnit(&iLoop); pUnit != NULL; pUnit = kLoopPlayer.nextUnit(&iLoop))
						{
							pUnit->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
						}
					}
				}
				break;

			case GAMEOBJECT_TEAM:
				for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
				{
					CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
					if (kLoopPlayer.isAlive())
					{
						if (kLoopPlayer.getTeam() == m_pTeam->getID())
						{
							int iLoop;
							for (CvUnit* pUnit = kLoopPlayer.firstUnit(&iLoop); pUnit != NULL; pUnit = kLoopPlayer.nextUnit(&iLoop))
							{
								pUnit->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
							}
						}
					}
				}
				break;

			case GAMEOBJECT_PLAYER:
				int iLoop;
				for (CvUnit* pUnit = m_pPlayer->firstUnit(&iLoop); pUnit != NULL; pUnit = m_pPlayer->nextUnit(&iLoop))
				{
					pUnit->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				}
				break;

			case GAMEOBJECT_CITY:
				{
					CLLNode<IDInfo>* pUnitNode = m_pCity->plot()->headUnitNode();
					while (pUnitNode != NULL)
					{
						CvUnit* pUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = m_pCity->plot()->nextUnitNode(pUnitNode);
						pUnit->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
					}
				}
				break;

			case GAMEOBJECT_PLOT:
				{
					CLLNode<IDInfo>* pUnitNode = m_pPlot->headUnitNode();
					while (pUnitNode != NULL)
					{
						CvUnit* pUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = m_pPlot->nextUnitNode(pUnitNode);
						pUnit->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
					}
				}
				break;
		}
	}

	iChangePercent = kInfo.getChangePropagator(m_eParentType, GAMEOBJECT_PLOT);
	if (iChangePercent)
	{
		switch(m_eParentType)
		{
			case GAMEOBJECT_GAME:
				for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
				{
					CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
					pLoopPlot->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				}
				break;

			case GAMEOBJECT_TEAM:
				for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
				{
					CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
					if (pLoopPlot->getTeam() == m_pTeam->getID())
					{
						pLoopPlot->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
					}
				}
				break;

			case GAMEOBJECT_PLAYER:
				for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
				{
					CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
					if (pLoopPlot->getOwnerINLINE() == m_pPlayer->getID())
					{
						pLoopPlot->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
					}
				}
				break;

			case GAMEOBJECT_CITY:
				m_pCity->plot()->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				break;

			case GAMEOBJECT_UNIT:
				m_pUnit->plot()->getProperties()->changeValueByProperty(eProp, (iChange * iChangePercent) / 100);
				break;
		}
	}
}

void CvProperties::addProperties(CvProperties* pProp)
{
	int num = pProp->getNumProperties();
	for (int index = 0; index < num; index++)
	{
		changeValueByProperty(pProp->getProperty(index), pProp->getValue(index));
	}
}

void CvProperties::subtractProperties(CvProperties* pProp)
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

void CvProperties::clearForRecalculate()
{
	m_aiProperty.clear();
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
		WRAPPER_READ(wrapper, "CvProperties",&eProp);
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
		WRAPPER_WRITE(wrapper, "CvProperties",eProp);
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
		szTemp.Format(L"%c %s: %d", GC.getPropertyInfo((PropertyTypes)getProperty(index)).getChar(), GC.getPropertyInfo((PropertyTypes)getProperty(index)).getText(), getValue(index));
	return szTemp.GetCString();
}
