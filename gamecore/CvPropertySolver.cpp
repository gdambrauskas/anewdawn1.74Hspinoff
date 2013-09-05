//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvPropertySolver.cpp
//
//  PURPOSE: Singleton class for solving the system of property manipulators
//
//------------------------------------------------------------------------------------------------

#include "CvGameCoreDLL.h"
#include "CvPropertySolver.h"

#include <boost/bind.hpp>

PropertySourceContext::PropertySourceContext(CvPropertySource *pSource, CvGameObject *pObject) : m_pSource(pSource), m_pObject(pObject)
{
	m_iCurrentAmount = pObject->getPropertiesConst()->getValueByProperty(pSource->getProperty());
}

CvPropertySource* PropertySourceContext::getSource()
{
	return m_pSource;
}

CvGameObject* PropertySourceContext::getObject()
{
	return m_pObject;
}

void PropertySourceContext::doPredict(PropertySolverMap *pmapProperties)
{
	int iChange = m_pSource->getSourcePredict(m_pObject, m_iCurrentAmount);
	pmapProperties->addChange(m_pObject, m_pSource->getProperty(), iChange);
}

void PropertySourceContext::doCorrect(PropertySolverMap *pmapProperties)
{
	PropertyTypes eProperty = m_pSource->getProperty();
	int iPredicted = pmapProperties->getPredictValue(m_pObject, eProperty);
	int iChange = m_pSource->getSourceCorrect(m_pObject, m_iCurrentAmount, iPredicted);
	pmapProperties->addChange(m_pObject, eProperty, iChange);
}


PropertyInteractionContext::PropertyInteractionContext(CvPropertyInteraction *pInteraction, CvGameObject *pObject) : m_pInteraction(pInteraction), m_pObject(pObject)
{
	m_iCurrentAmountSource = pObject->getPropertiesConst()->getValueByProperty(pInteraction->getSourceProperty());
	m_iCurrentAmountTarget = pObject->getPropertiesConst()->getValueByProperty(pInteraction->getTargetProperty());
}

CvPropertyInteraction* PropertyInteractionContext::getInteraction()
{
	return m_pInteraction;
}

CvGameObject* PropertyInteractionContext::getObject()
{
	return m_pObject;
}

void PropertyInteractionContext::doPredict(PropertySolverMap *pmapProperties)
{
	std::pair<int,int> iChange = m_pInteraction->getPredict(m_iCurrentAmountSource, m_iCurrentAmountTarget);
	pmapProperties->addChange(m_pObject, m_pInteraction->getSourceProperty(), iChange.first);
	pmapProperties->addChange(m_pObject, m_pInteraction->getTargetProperty(), iChange.second);
}

void PropertyInteractionContext::doCorrect(PropertySolverMap *pmapProperties)
{
	PropertyTypes ePropertySource = m_pInteraction->getSourceProperty();
	PropertyTypes ePropertyTarget = m_pInteraction->getTargetProperty();
	int iPredictedAmountSource = pmapProperties->getPredictValue(m_pObject, ePropertySource);
	int iPredictedAmountTarget = pmapProperties->getPredictValue(m_pObject, ePropertyTarget);
	std::pair<int,int> iChange = m_pInteraction->getCorrect(m_iCurrentAmountSource, m_iCurrentAmountTarget, iPredictedAmountSource, iPredictedAmountTarget);
	pmapProperties->addChange(m_pObject, ePropertySource, iChange.first);
	pmapProperties->addChange(m_pObject, ePropertyTarget, iChange.second);
}


PropertyPropagatorContext::PropertyPropagatorContext(CvPropertyPropagator *pPropagator, CvGameObject *pObject) : m_pPropagator(pPropagator), m_pObject(pObject)
{
	pPropagator->getTargetObjects(pObject, m_apTargetObjects);
	PropertyTypes eProperty = pPropagator->getProperty();

	for(int i=0; i<(int)m_apTargetObjects.size(); i++)
	{
		m_aiCurrentAmount.push_back(m_apTargetObjects[i]->getPropertiesConst()->getValueByProperty(eProperty));
	}
}

CvPropertyPropagator* PropertyPropagatorContext::getPropagator()
{
	return m_pPropagator;
}

CvGameObject* PropertyPropagatorContext::getObject()
{
	return m_pObject;
}

std::vector<CvGameObject*>* PropertyPropagatorContext::getTargetObjects()
{
	return &m_apTargetObjects;
}

void PropertyPropagatorContext::doPredict(PropertySolverMap *pmapProperties)
{
	std::vector<int> aiPredict;
	aiPredict.resize(m_aiCurrentAmount.size());
	m_pPropagator->getPredict(m_aiCurrentAmount, aiPredict);

	PropertyTypes eProperty = m_pPropagator->getProperty();
	for(int i=0; i<(int)aiPredict.size(); i++)
	{
		pmapProperties->addChange(m_apTargetObjects[i], eProperty, aiPredict[i]);
	}
}

void PropertyPropagatorContext::doCorrect(PropertySolverMap *pmapProperties)
{
	PropertyTypes eProperty = m_pPropagator->getProperty();
	std::vector<int> aiPredict;
	for(int i=0; i<(int)m_apTargetObjects.size(); i++)
	{
		aiPredict.push_back(pmapProperties->getPredictValue(m_apTargetObjects[i], eProperty));
	}
	std::vector<int> aiCorrect;
	aiCorrect.resize(m_aiCurrentAmount.size());
	m_pPropagator->getCorrect(m_aiCurrentAmount, aiPredict, aiCorrect);
	for(int i=0; i<(int)aiCorrect.size(); i++)
	{
		pmapProperties->addChange(m_apTargetObjects[i], eProperty, aiCorrect[i]);
	}
}


void PropertySolverMap::addChange(CvGameObject *pObject, PropertyTypes eProperty, int iChange)
{
	m_mapProperties[pObject].second.changeValueByProperty(eProperty, iChange);
}

int PropertySolverMap::getChangeValue(CvGameObject *pObject, PropertyTypes eProperty)
{
	return m_mapProperties[pObject].second.getValueByProperty(eProperty);
}

int PropertySolverMap::getPredictValue(CvGameObject *pObject, PropertyTypes eProperty)
{
	return m_mapProperties[pObject].first.getValueByProperty(eProperty);
}

void PropertySolverMap::computePredictValues()
{
	for(PropertySolverMapType::iterator it = m_mapProperties.begin(); it != m_mapProperties.end(); it++)
	{
		CvProperties* pProp = it->first->getProperties();
		it->second.first.addProperties(pProp);
		it->second.first.addProperties(&(it->second.second));
		it->second.second.clear();
	}
}

void PropertySolverMap::applyChanges()
{
	for(PropertySolverMapType::iterator it = m_mapProperties.begin(); it != m_mapProperties.end(); it++)
	{
		CvProperties* pProp = it->first->getProperties();
		pProp->addProperties(&(it->second.second));
	}
	// Changes are applied, clear the intermediate values
	m_mapProperties.clear();
}


void CvPropertySolver::addGlobalManipulators(CvPropertyManipulators *pMani)
{
	m_apGlobalManipulators.push_back(pMani);
}

void CvPropertySolver::instantiateSource(CvGameObject* pObject, CvPropertySource* pSource)
{
	if (pSource->isActive(pObject))
	{
		m_aSourceContexts.push_back(PropertySourceContext(pSource, pObject));
	}
}

void callInstantiateSource(CvGameObject* pObject, CvPropertySource* pSource, CvPropertySolver* pSolver)
{
	pSolver->instantiateSource(pObject, pSource);
}

void CvPropertySolver::instantiateInteraction(CvGameObject* pObject, CvPropertyInteraction* pInteraction)
{
	if (pInteraction->isActive(pObject))
	{
		m_aInteractionContexts.push_back(PropertyInteractionContext(pInteraction, pObject));
	}
}

void callInstantiateInteraction(CvGameObject* pObject, CvPropertyInteraction* pInteraction, CvPropertySolver* pSolver)
{
	pSolver->instantiateInteraction(pObject, pInteraction);
}

void CvPropertySolver::instantiatePropagator(CvGameObject* pObject, CvPropertyPropagator* pPropagator)
{
	if (pPropagator->isActive(pObject))
	{
		m_aPropagatorContexts.push_back(PropertyPropagatorContext(pPropagator, pObject));
	}
}

void callInstantiatePropagator(CvGameObject* pObject, CvPropertyPropagator* pPropagator, CvPropertySolver* pSolver)
{
	pSolver->instantiatePropagator(pObject, pPropagator);
}

void CvPropertySolver::instantiateManipulators(CvGameObject* pObject, CvPropertyManipulators* pMani)
{
	// Sources
	for (int j=0; j<pMani->getNumSources(); j++)
	{
		CvPropertySource* pSource = pMani->getSource(j);
		RelationTypes eRelation = pSource->getRelation();
		if (eRelation == NO_RELATION)
		{
			instantiateSource(pObject, pSource);
		}
		else
		{
			pObject->foreachRelated(pSource->getObjectType(), eRelation, boost::bind(callInstantiateSource, _1, pSource, this), pSource->getRelationData());
		}
	}
	// Interactions
	for (int j=0; j<pMani->getNumInteractions(); j++)
	{
		CvPropertyInteraction* pInteraction = pMani->getInteraction(j);
		RelationTypes eRelation = pInteraction->getRelation();
		if (eRelation == NO_RELATION)
		{
			instantiateInteraction(pObject, pInteraction);
		}
		else
		{
			pObject->foreachRelated(pInteraction->getObjectType(), eRelation, boost::bind(callInstantiateInteraction, _1, pInteraction, this), pInteraction->getRelationData());
		}
	}
	// Propagators
	for (int j=0; j<pMani->getNumPropagators(); j++)
	{
		CvPropertyPropagator* pPropagator = pMani->getPropagator(j);
		RelationTypes eRelation = pPropagator->getRelation();
		if (eRelation == NO_RELATION)
		{
			instantiatePropagator(pObject, pPropagator);
		}
		else
		{
			pObject->foreachRelated(pPropagator->getObjectType(), eRelation, boost::bind(callInstantiatePropagator, _1, pPropagator, this), pPropagator->getRelationData());
		}
	}
}

void CvPropertySolver::instantiateGlobalManipulators(CvGameObject *pObject)
{
	for (int i=0; i<(int)m_apGlobalManipulators.size(); i++)
	{
		instantiateManipulators(pObject, m_apGlobalManipulators[i]);
	}
}

// helper functions
void callInstantiateManipulators(CvGameObject* pObject, CvPropertyManipulators* pMani, CvPropertySolver* pSolver)
{
	pSolver->instantiateManipulators(pObject, pMani);
}

void callInstantiateGlobalManipulators(CvGameObject* pObject, CvPropertySolver* pSolver)
{
	pSolver->instantiateGlobalManipulators(pObject);
	pObject->foreachManipulator(boost::bind(callInstantiateManipulators, _1, _2, pSolver));
}

void CvPropertySolver::gatherActiveManipulators()
{
	// Global manipulators first
	for (int i=0; i<GC.getNumPropertyInfos(); i++)
	{
		addGlobalManipulators(GC.getPropertyInfo((PropertyTypes)i).getPropertyManipulators());
	}

	for (int i=0; i<NUM_GAMEOBJECTS; i++)
	{
		GC.getGameINLINE().getGameObject()->foreach((GameObjectTypes)i, boost::bind(callInstantiateGlobalManipulators, _1, this));
	}
}

void CvPropertySolver::predictSources()
{
	for (int i=0; i<(int)m_aSourceContexts.size(); i++)
	{
		m_aSourceContexts[i].doPredict(&m_mapProperties);
	}
}

void CvPropertySolver::correctSources()
{
	for (int i=0; i<(int)m_aSourceContexts.size(); i++)
	{
		m_aSourceContexts[i].doCorrect(&m_mapProperties);
	}
}

void CvPropertySolver::predictInteractions()
{
	for (int i=0; i<(int)m_aInteractionContexts.size(); i++)
	{
		m_aInteractionContexts[i].doPredict(&m_mapProperties);
	}
}

void CvPropertySolver::correctInteractions()
{
	for (int i=0; i<(int)m_aInteractionContexts.size(); i++)
	{
		m_aInteractionContexts[i].doCorrect(&m_mapProperties);
	}
}

void CvPropertySolver::predictPropagators()
{
	for (int i=0; i<(int)m_aPropagatorContexts.size(); i++)
	{
		m_aPropagatorContexts[i].doPredict(&m_mapProperties);
	}
}

void CvPropertySolver::correctPropagators()
{
	for (int i=0; i<(int)m_aPropagatorContexts.size(); i++)
	{
		m_aPropagatorContexts[i].doCorrect(&m_mapProperties);
	}
}

void callResetPropertyChange(CvGameObject* pObject)
{
	pObject->getProperties()->clearChange();
}

void CvPropertySolver::resetPropertyChanges()
{
	for (int i=0; i<NUM_GAMEOBJECTS; i++)
	{
		GC.getGameINLINE().getGameObject()->foreach((GameObjectTypes)i, callResetPropertyChange);
	}
}

void CvPropertySolver::doTurn()
{
	MEMORY_TRACE_FUNCTION();
	PROFILE_FUNC();

	gatherActiveManipulators();
	resetPropertyChanges();

	// Propagators first
	predictPropagators();
	m_mapProperties.computePredictValues();
	correctPropagators();
	m_mapProperties.applyChanges();
	m_aPropagatorContexts.clear();

	// Interactions next
	predictInteractions();
	m_mapProperties.computePredictValues();
	correctInteractions();
	m_mapProperties.applyChanges();
	m_aInteractionContexts.clear();

	// Sources last
	predictSources();
	m_mapProperties.computePredictValues();
	correctSources();
	m_mapProperties.applyChanges();
	m_aSourceContexts.clear();
	m_apGlobalManipulators.clear();
}
	
