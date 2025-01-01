/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

/*! Program resource base class.
 * \ingroup Render
 */
class T_DLLCLASS ProgramResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	bool requireRayTracing() const { return m_requireRayTracing; }

	virtual void serialize(ISerializer& s) override;

protected:
	bool m_requireRayTracing = false;
};

}
