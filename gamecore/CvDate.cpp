//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvDate.h
//
//  PURPOSE: Class to keep a Civ4 date and methods related to the time/turn relationship
//
//------------------------------------------------------------------------------------------------
#include "CvGameCoreDLL.h"
#include "CvDate.h"

int CvDate::m_aiDaysPerMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

//	Externally (fed to the constructor or provided by the getDay/Mobnth/Year methods) days are 1-based,
//	months and years are 0-based
CvDate::CvDate(int iYear, int iMonth, int iDay) :
m_iYear(iYear),
m_iMonth(iMonth),
m_iDay(iDay - 1)
{
}

int CvDate::getYear() const
{
	return m_iYear;
}

int CvDate::getDay() const
{
	return m_iDay + 1;
}

int CvDate::getMonth() const
{
	return m_iMonth;
}

int CvDate::getWeek() const
{
	return (m_iDay / 7) + 1;
}

SeasonTypes CvDate::getSeason() const
{
	if (m_iMonth <= 1 || m_iMonth >= 11)
	{
		return SEASON_WINTER; // Winter
	}
	if (m_iMonth >= 2 && m_iMonth <= 4)
	{
		return SEASON_SPRING; // Spring
	}
	if (m_iMonth >= 5 && m_iMonth <= 7)
	{
		return SEASON_SUMMER; // Summer
	}
	if (m_iMonth >= 8 && m_iMonth <= 10)
	{
		return SEASON_AUTUMN; // Autumn
	}
	return NO_SEASON; // This will never be executed
}

CvDateIncrement CvDate::getIncrement(GameSpeedTypes eGameSpeed) const
{
	GameSpeedTypes eActualGameSpeed = eGameSpeed;
	if (eGameSpeed == NO_GAMESPEED)
	{
		eActualGameSpeed = GC.getGameINLINE().getGameSpeedType();
	}
	CvGameSpeedInfo& kInfo = GC.getGameSpeedInfo(eActualGameSpeed);
	std::vector<CvDateIncrement>& aIncrements = kInfo.getIncrements();
	if (!kInfo.getEndDatesCalculated())
	{
		calculateEndDates(eActualGameSpeed);
	}

	for (std::vector<CvDateIncrement>::iterator it = aIncrements.begin(); it != aIncrements.end(); it++)
	{
		if (*this <= it->m_endDate)
		{
			return *it;
		}
	}
	return aIncrements[aIncrements.size()-1];
}

void CvDate::increment(GameSpeedTypes eGameSpeed)
{
	CvDateIncrement inc = getIncrement(eGameSpeed);
	int iYears = inc.m_iIncrementMonth / 12;
	int iMonths = inc.m_iIncrementMonth % 12;
	int iDays = inc.m_iIncrementDay;

	m_iDay += iDays;
	while (m_iDay >= m_aiDaysPerMonth[m_iMonth])
	{
		m_iDay -= m_aiDaysPerMonth[m_iMonth];
		m_iMonth += 1;
		if (m_iMonth >= 12)
		{
			m_iMonth -= 12;
			m_iYear += 1;
		}
	}

	m_iMonth += iMonths;
	if (m_iMonth >= 12)
	{
		m_iMonth -= 12;
		m_iYear += 1;
	}

	if (m_iDay >= m_aiDaysPerMonth[m_iMonth])
	{
		m_iDay -= m_aiDaysPerMonth[m_iMonth];
		m_iMonth += 1;
		if (m_iMonth >= 12)
		{
			m_iMonth -= 12;
			m_iYear += 1;
		}
	}

	m_iYear += iYears;
}

void CvDate::increment(int iTurns, GameSpeedTypes eGameSpeed)
{
	for (int i=0; i<iTurns; i++)
	{
		increment(eGameSpeed);
	}
}

bool CvDate::operator !=(const CvDate &kDate) const
{
	return !(*this == kDate);
}

bool CvDate::operator ==(const CvDate &kDate) const
{
	return m_iYear == kDate.getYear() && m_iMonth == kDate.getMonth() && m_iDay == kDate.getDay();
}

bool CvDate::operator <(const CvDate &kDate) const
{
	if (m_iYear < kDate.getYear())
		return true;
	if (m_iYear > kDate.getYear())
		return false;
	if (m_iMonth < kDate.getMonth())
		return true;
	if (m_iMonth > kDate.getMonth())
		return false;
	return m_iDay < kDate.getDay();
}

bool CvDate::operator >(const CvDate &kDate) const
{
	if (m_iYear > kDate.getYear())
		return true;
	if (m_iYear < kDate.getYear())
		return false;
	if (m_iMonth > kDate.getMonth())
		return true;
	if (m_iMonth < kDate.getMonth())
		return false;
	return m_iDay > kDate.getDay();
}

bool CvDate::operator <=(const CvDate &kDate) const
{
	return ! (*this > kDate);
}

bool CvDate::operator >=(const CvDate &kDate) const
{
	return ! (*this < kDate);
}

CvDate CvDate::getStartingDate()
{
	return CvDate(GC.getGameINLINE().getStartYear());
}

CvDate CvDate::getDate(int iTurn, GameSpeedTypes eGameSpeed)
{
	CvDate date;
	int iRemainingTurns;

	GameSpeedTypes eActualGameSpeed = eGameSpeed;
	if (eGameSpeed == NO_GAMESPEED)
	{
		eActualGameSpeed = GC.getGameINLINE().getGameSpeedType();
	}
	CvGameSpeedInfo& kInfo = GC.getGameSpeedInfo(eActualGameSpeed);
	std::vector<CvDateIncrement>& aIncrements = kInfo.getIncrements();
	if (!kInfo.getEndDatesCalculated())
	{
		calculateEndDates(eActualGameSpeed);
	}
	

	for (int i=0; i<(int)aIncrements.size(); i++)
	{
		if (iTurn <= aIncrements[i].m_iendTurn)
		{
			if (i==0)
			{
				iRemainingTurns = iTurn;
				date = getStartingDate();
			}
			else
			{
				iRemainingTurns = iTurn - aIncrements[i-1].m_iendTurn;
				date = aIncrements[i-1].m_endDate;
			}
			break;
		}
		else if (i==(int)aIncrements.size())
		{
			iRemainingTurns = iTurn - aIncrements[i].m_iendTurn;
			date = aIncrements[i].m_endDate;
		}
	}
	date.increment(iRemainingTurns, eGameSpeed);
	return date;
}

void CvDate::calculateEndDates(GameSpeedTypes eGameSpeed)
{
	CvGameSpeedInfo& kInfo = GC.getGameSpeedInfo(eGameSpeed);
	std::vector<CvDateIncrement>& aIncrements = kInfo.getIncrements();
	kInfo.setEndDatesCalculated(true);
	for (int i=0; i<(int)aIncrements.size(); i++)
	{
		aIncrements[i].m_endDate = CvDate(1000000);
		aIncrements[i].m_endDate = getDate(aIncrements[i].m_iendTurn);
	}
}