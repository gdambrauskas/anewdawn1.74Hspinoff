#pragma once

//  $Header:
//------------------------------------------------------------------------------------------------
//
//  FILE:    CvPropertySource.h
//
//  PURPOSE: Source of generic properties for Civ4 classes
//
//------------------------------------------------------------------------------------------------
#ifndef CV_PROPERTY_SOURCE_H
#define CV_PROPERTY_SOURCE_H

#include "CvXMLLoadUtilityModTools.h"
#include "CvGameObject.h"
#include "BoolExpr.h"

class CvGameObject;

// Base class of which the actual property source classes are derived
class CvPropertySource
{
public:
	CvPropertySource();
	CvPropertySource(PropertyTypes eProperty);
	virtual ~CvPropertySource();
	PropertyTypes getProperty() const;
	void setProperty(PropertyTypes eProperty);
	GameObjectTypes getObjectType() const;
	void setObjectType(GameObjectTypes eObjectType);
	RelationTypes getRelation() const;
	void setRelation(RelationTypes eRelation);
	int getRelationData() const;
	void setRelationData(int iRelationData);

	bool isActive(CvGameObject* pObject);

	virtual PropertySourceTypes getType() = 0;

	virtual int getSourcePredict(const CvGameObject* pObject, int iCurrentAmount) = 0;
	virtual int getSourceCorrect(const CvGameObject* pObject, int iCurrentAmount, int iPredictedAmount) = 0;

	virtual void buildDisplayString(CvWStringBuffer& szBuffer) const = 0;
	
	virtual void read(FDataStreamBase* pStream);
	//void readWrapper(FDataStreamBase* pStream);
	virtual void write(FDataStreamBase* pStream);
	//void writeWrapper(FDataStreamBase* pStream);
	virtual bool read(CvXMLLoadUtility* pXML);
	virtual void copyNonDefaults(CvPropertySource* pProp, CvXMLLoadUtility* pXML );

	virtual void getCheckSum(unsigned int& iSum);
protected:
	PropertyTypes m_eProperty;
	GameObjectTypes m_eObjectType;
	RelationTypes m_eRelation;
	int m_iRelationData;
	BoolExpr* m_pExprActive;
};


class CvPropertySourceConstant : public CvPropertySource
{
public:
	CvPropertySourceConstant();
	CvPropertySourceConstant(PropertyTypes eProperty);
	CvPropertySourceConstant(PropertyTypes eProperty, int iAmountPerTurn);

	int getAmountPerTurn();

	virtual PropertySourceTypes getType();

	virtual int getSourcePredict(const CvGameObject* pObject, int iCurrentAmount);
	virtual int getSourceCorrect(const CvGameObject* pObject, int iCurrentAmount, int iPredictedAmount);

	virtual void buildDisplayString(CvWStringBuffer& szBuffer) const;
	
	virtual void read(FDataStreamBase* pStream);
	//void readWrapper(FDataStreamBase* pStream);
	virtual void write(FDataStreamBase* pStream);
	//void writeWrapper(FDataStreamBase* pStream);
	virtual bool read(CvXMLLoadUtility* pXML);
	virtual void copyNonDefaults(CvPropertySource* pProp, CvXMLLoadUtility* pXML );

	virtual void getCheckSum(unsigned int& iSum);

protected:
	int m_iAmountPerTurn;
};

class CvPropertySourceConstantLimited : public CvPropertySource
{
public:
	CvPropertySourceConstantLimited();
	CvPropertySourceConstantLimited(PropertyTypes eProperty);
	CvPropertySourceConstantLimited(PropertyTypes eProperty, int iAmountPerTurn, int iLimit);

	int getAmountPerTurn();
	int getLimit();

	virtual PropertySourceTypes getType();

	virtual int getSourcePredict(const CvGameObject* pObject, int iCurrentAmount);
	virtual int getSourceCorrect(const CvGameObject* pObject, int iCurrentAmount, int iPredictedAmount);

	virtual void buildDisplayString(CvWStringBuffer& szBuffer) const;
	
	virtual void read(FDataStreamBase* pStream);
	//void readWrapper(FDataStreamBase* pStream);
	virtual void write(FDataStreamBase* pStream);
	//void writeWrapper(FDataStreamBase* pStream);
	virtual bool read(CvXMLLoadUtility* pXML);
	virtual void copyNonDefaults(CvPropertySource* pProp, CvXMLLoadUtility* pXML );

	virtual void getCheckSum(unsigned int& iSum);

protected:
	int m_iAmountPerTurn;
	int m_iLimit;
};

class CvPropertySourceDecay : public CvPropertySource
{
public:
	CvPropertySourceDecay();
	CvPropertySourceDecay(PropertyTypes eProperty);
	CvPropertySourceDecay(PropertyTypes eProperty, int iPercent, int iNoDecayAmount);

	int getPercent();
	int getNoDecayAmount();

	virtual PropertySourceTypes getType();

	virtual int getSourcePredict(const CvGameObject* pObject, int iCurrentAmount);
	virtual int getSourceCorrect(const CvGameObject* pObject, int iCurrentAmount, int iPredictedAmount);

	virtual void buildDisplayString(CvWStringBuffer& szBuffer) const;
	
	virtual void read(FDataStreamBase* pStream);
	//void readWrapper(FDataStreamBase* pStream);
	virtual void write(FDataStreamBase* pStream);
	//void writeWrapper(FDataStreamBase* pStream);
	virtual bool read(CvXMLLoadUtility* pXML);
	virtual void copyNonDefaults(CvPropertySource* pProp, CvXMLLoadUtility* pXML );

	virtual void getCheckSum(unsigned int& iSum);

protected:
	int m_iPercent;
	int m_iNoDecayAmount;
};

class CvPropertySourceAttributeConstant : public CvPropertySource
{
public:
	CvPropertySourceAttributeConstant();
	CvPropertySourceAttributeConstant(PropertyTypes eProperty);
	CvPropertySourceAttributeConstant(PropertyTypes eProperty, AttributeTypes eAttribute, int iAmountPerTurn);

	AttributeTypes getAttribute();
	int getAmountPerTurn();

	virtual PropertySourceTypes getType();

	virtual int getSourcePredict(const CvGameObject* pObject, int iCurrentAmount);
	virtual int getSourceCorrect(const CvGameObject* pObject, int iCurrentAmount, int iPredictedAmount);

	virtual void buildDisplayString(CvWStringBuffer& szBuffer) const;
	
	virtual void read(FDataStreamBase* pStream);
	//void readWrapper(FDataStreamBase* pStream);
	virtual void write(FDataStreamBase* pStream);
	//void writeWrapper(FDataStreamBase* pStream);
	virtual bool read(CvXMLLoadUtility* pXML);
	virtual void copyNonDefaults(CvPropertySource* pProp, CvXMLLoadUtility* pXML );

	virtual void getCheckSum(unsigned int& iSum);

protected:
	AttributeTypes m_eAttribute;
	int m_iAmountPerTurn;
};

#endif