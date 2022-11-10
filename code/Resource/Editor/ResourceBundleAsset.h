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
#if defined(T_RESOURCE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

/*!
 * \ingroup Resource
 */
class T_DLLCLASS ResourceBundleAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	void add(const Guid& resource);

	const AlignedVector< Guid >& get() const;

	void setPersistent(bool persistent);

	bool persistent() const;

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< Guid > m_resources;
	bool m_persistent = false;
};

	}
}

