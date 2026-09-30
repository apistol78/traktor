/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Id.h"
#include "World/IWorldComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ITexture;

}

namespace traktor::world
{

/*! Color correction world component data.
 * \ingroup World
 */
class T_DLLCLASS ColorCorrectionComponentData : public IWorldComponentData
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::ITexture >& getColorGrading() const { return m_colorGrading; }

private:
	resource::Id< render::ITexture > m_colorGrading;
};

}
