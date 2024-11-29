/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Io/Path.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

/*!
 * \ingroup Resource
 */
class T_DLLCLASS FileBundleAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Pattern
	{
		Path sourceBase;
		Path outputBase;
		std::wstring sourceMask;
		bool recursive = false;

		void serialize(ISerializer& s);
	};

	const std::list< Pattern >& getPatterns() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::list< Pattern > m_patterns;
};

}
