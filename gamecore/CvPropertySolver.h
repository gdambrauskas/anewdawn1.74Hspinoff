#pragma once

//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvPropertySolver.h
//
//  PURPOSE: Singleton class for solving the system of property manipulators
//
//------------------------------------------------------------------------------------------------
#ifndef CV_PROPERTY_SOLVER_H
#define CV_PROPERTY_SOLVER_H

#include <vector>
#include <map>
#include "CvProperties.h"
#include "CvPropertySource.h"
#include "CvPropertyInteraction.h"
#include "CvPropertyPropagator.h"

typedef std::map<CvGameObject*, std::pair<CvProperties,CvProperties> > PropertySolverMapType;

class PropertySolverMap
{
public:
	int getPredictValue(CvGameObject* pObject, PropertyTypes eProperty);
	int getChangeValue(CvGameObject* pObject, PropertyTypes eProperty);
	void addChange(CvGameObject* pObject, PropertyTypes eProperty, int iChange);

	void computePredictValues();
	void applyChanges();

protected:
	PropertySolverMapType m_mapProperties;
};

class PropertySourceContext
{
public:
	PropertySourceContext(CvPropertySource* pSource, CvGameObject* pObject);
	CvPropertySource* getSource();
	CvGameObject* getObject();

	void doPredict(PropertySolverMap* pmapProperties);
	void doCorrect(PropertySolverMap* pmapProperties);

protected:
	CvPropertySource* m_pSource;
	CvGameObject* m_pObject;
	int m_iCurrentAmount;
};

class PropertyInteractionContext
{
public:
	PropertyInteractionContext(CvPropertyInteraction* pInteraction, CvGameObject* pObject);
	CvPropertyInteraction* getInteraction();
	CvGameObject* getObject();

	void doPredict(PropertySolverMap* pmapProperties);
	void doCorrect(PropertySolverMap* pmapProperties);

protected:
	CvPropertyInteraction* m_pInteraction;
	CvGameObject* m_pObject;
	int m_iCurrentAmountSource;
	int m_iCurrentAmountTarget;
};

class PropertyPropagatorContext
{
public:
	PropertyPropagatorContext(CvPropertyPropagator* pPropagator, CvGameObject* pObject);
	CvPropertyPropagator* getPropagator();
	CvGameObject* getObject();
	std::vector<CvGameObject*>* getTargetObjects();

	void doPredict(PropertySolverMap* pmapProperties);
	void doCorrect(PropertySolverMap* pmapProperties);

protected:
	CvPropertyPropagator* m_pPropagator;
	CvGameObject* m_pObject;
	std::vector<CvGameObject*> m_apTargetObjects;
	std::vector<int> m_aiCurrentAmount;
};

class CvPropertySolver
{
public:
	void addGlobalManipulators(CvPropertyManipulators* pMani);
	void instantiateSource(CvGameObject* pObject, CvPropertySource* pSource);
	void instantiateInteraction(CvGameObject* pObject, CvPropertyInteraction* pInteraction);
	void instantiatePropagator(CvGameObject* pObject, CvPropertyPropagator* pPropagator);
	void instantiateManipulators(CvGameObject* pObject, CvPropertyManipulators* pMani);
	void instantiateGlobalManipulators(CvGameObject* pObject);
	void gatherActiveManipulators();
	void resetPropertyChanges();
	
	void predictSources();
	void correctSources();
	
	void predictInteractions();
	void correctInteractions();

	void predictPropagators();
	void correctPropagators();

	void doTurn();

protected:
	std::vector<PropertySourceContext> m_aSourceContexts;
	std::vector<PropertyInteractionContext> m_aInteractionContexts;
	std::vector<PropertyPropagatorContext> m_aPropagatorContexts;
	std::vector<CvPropertyManipulators*> m_apGlobalManipulators;
	PropertySolverMap m_mapProperties;
};


#endif