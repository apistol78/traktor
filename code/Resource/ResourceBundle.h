/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

/*! Bundle of resource identifiers.
 * \ingroup Resource
 */
class T_DLLCLASS ResourceBundle : public ISerializable
{
	T_RTTI_CLASS;

public:
	ResourceBundle() = default;

	explicit ResourceBundle(const AlignedVector< std::pair< const TypeInfo*, Guid > >& resources, bool persistent);

	const AlignedVector< std::pair< const TypeInfo*, Guid > >& get() const;

	bool persistent() const;

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< std::pair< const TypeInfo*, Guid > > m_resources;
	bool m_persistent = false;
};

}
