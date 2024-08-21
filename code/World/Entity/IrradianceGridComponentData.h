/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
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

namespace traktor::world
{

class IrradianceGrid;

/*!
 * \ingroup World
 */
class T_DLLCLASS IrradianceGridComponentData : public IWorldComponentData
{
	T_RTTI_CLASS;

public:
	IrradianceGridComponentData() = default;

	explicit IrradianceGridComponentData(const resource::Id< IrradianceGrid >& irradianceGrid);

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< IrradianceGrid >& getIrradianceGrid() const { return m_irradianceGrid; }

private:
	resource::Id< IrradianceGrid > m_irradianceGrid;
};

}
