/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef Aggregation_H
#define Aggregation_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sb
	{

class AggregationItem;
class Dependency;

class T_DLLCLASS Aggregation : public ISerializable
{
	T_RTTI_CLASS;

public:
	Aggregation();

	void setEnable(bool enable);

	bool getEnable() const;

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	/*! \name Aggregation items. */
	//@{

	void addItem(AggregationItem* item);

	void removeItem(AggregationItem* item);

	const RefArray< AggregationItem >& getItems() const;

	//@}

	/*! \name Dependencies. */
	//@{

	void addDependency(Dependency* dependency);

	void removeDependency(Dependency* dependency);

	void setDependencies(const RefArray< Dependency >& dependencies);

	const RefArray< Dependency >& getDependencies() const;

	//@}

	virtual void serialize(ISerializer& s);

private:
	bool m_enable;
	std::wstring m_name;
	RefArray< AggregationItem > m_items;
	RefArray< Dependency > m_dependencies;
};

	}
}

#endif	// Aggregation_H
