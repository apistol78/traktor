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
#include <string>
#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Render
 */
class T_DLLCLASS TextureSet : public ISerializable
{
	T_RTTI_CLASS;

public:
	const std::map< std::wstring, Guid >& get() const { return m_textures; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::map< std::wstring, Guid > m_textures;
};

	}
}
