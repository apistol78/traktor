/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IWorldComponent.h"

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
class T_DLLCLASS IrradianceGridComponent : public IWorldComponent
{
	T_RTTI_CLASS;

public:
	virtual void destroy() override final;

	virtual void update(World* world, const UpdateParams& update) override final;

	void setIrradianceGrid(const IrradianceGrid* irradianceGrid);

	const IrradianceGrid* getIrradianceGrid() const;

private:
	Ref< const IrradianceGrid > m_irradianceGrid;
};

}
