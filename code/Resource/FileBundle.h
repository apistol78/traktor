/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
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

/*! File bundle
 *  A file bundle is a serialized instance with a collection of named data streams attached.
 *
 * \ingroup Resource
 */
class T_DLLCLASS FileBundle : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Lookup name of data stream from identifier. */
	std::wstring lookup(const std::wstring& id) const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class FileBundlePipeline;

	std::map< std::wstring, std::wstring > m_dataIds;
};

}
