/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Ref.h"
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

class IRenderSystem;
class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class DisplacementWorldComponent;

/*! Displacement world component data.
 * \ingroup World
 */
class T_DLLCLASS DisplacementWorldComponentData : public IWorldComponentData
{
	T_RTTI_CLASS;

public:
	Ref< DisplacementWorldComponent > createComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class DisplacementWorldComponent;

	int32_t m_resolution = 512;
	float m_extent = 64.0f;
	float m_fadeRate = 1.5f;
};

}
