//------------------------------------------------------------------------------------------------
//
//  FILE:    CvOutcomeMission.h
//
//  PURPOSE: A mission that has a cost and a result depending on an outcome list
//
//------------------------------------------------------------------------------------------------
#ifndef CV_OUTCOME_MISSION_H
#define CV_OUTCOME_MISSION_H

#include "CvXMLLoadUtilityModTools.h"
#include "CvOutcomeList.h"

class CvOutcomeMission
{
public:
	CvOutcomeMission();
	MissionTypes getMission();
	CvOutcomeList* getOutcomeList();
	CvProperties* getPropertyCost();
	bool isKill();
	int getCost();
	GameObjectTypes getPayerType();

	bool isPossible(CvUnit* pUnit);
	void buildDisplayString(CvWStringBuffer& szBuffer, CvUnit* pUnit);
	void execute(CvUnit* pUnit);
	
	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);
	bool read(CvXMLLoadUtility* pXML);
	void copyNonDefaults(CvOutcomeMission* pOutcomeMission, CvXMLLoadUtility* pXML );

	void getCheckSum(unsigned int& iSum);

protected:
	MissionTypes m_eMission;
	CvOutcomeList m_OutcomeList;
	CvProperties m_PropertyCost;
	GameObjectTypes m_ePayerType;
	bool m_bKill;
	int m_iCost;
};

#endif