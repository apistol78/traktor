/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef Aggregation_H
#define Aggregation_H

#include <Core/RefArray.h>
#include <Core/Serialization/ISerializable.h>

class AggregationItem;
class Dependency;

class Aggregation : public traktor::ISerializable
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

	const traktor::RefArray< AggregationItem >& getItems() const;

	//@}

	/*! \name Dependencies. */
	//@{

	void addDependency(Dependency* dependency);

	void removeDependency(Dependency* dependency);

	void setDependencies(const traktor::RefArray< Dependency >& dependencies);

	const traktor::RefArray< Dependency >& getDependencies() const;

	//@}

	virtual void serialize(traktor::ISerializer& s);

private:
	bool m_enable;
	std::wstring m_name;
	traktor::RefArray< AggregationItem > m_items;
	traktor::RefArray< Dependency > m_dependencies;
};

#endif	// Aggregation_H
