/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Proxy.h"
#include "World/IWorldComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ImageGraph;

}

namespace traktor::world
{

/*! Post process world component.
 * \ingroup World
 */
class T_DLLCLASS PostProcessComponent : public IWorldComponent
{
	T_RTTI_CLASS;

public:
	explicit PostProcessComponent(const resource::Proxy< render::ImageGraph >& imageGraph);

	virtual void destroy() override final;

	virtual void update(World* world, const UpdateParams& update) override final;

	render::ImageGraph* getImageGraph() const;

private:
	resource::Proxy< render::ImageGraph > m_imageGraph;
};

}
