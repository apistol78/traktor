/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sb
{

class AggregationItem;
class Dependency;

// !DEPRECATED!
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

	virtual void serialize(ISerializer& s) override final;

private:
	bool m_enable;
	std::wstring m_name;
	RefArray< AggregationItem > m_items;
	RefArray< Dependency > m_dependencies;
};

}
