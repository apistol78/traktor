/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Color4f.h"
#include "Resource/Id.h"
#include "World/IWorldComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::render
{

class IRenderSystem;
class Shader;

}

namespace traktor::world
{

class FogComponent;

/*!
 * \ingroup World
 */
class T_DLLCLASS FogComponentData : public IWorldComponentData
{
	T_RTTI_CLASS;

public:
	Ref< FogComponent > createComponent() const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class FogComponent;

	Color4f m_mediumColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	float m_mediumDensity = 0.2f;

	// Distance fog.
	bool m_distanceFogEnable = false;
	float m_fogDistance = 90.0f;
	float m_fogElevation = 0.0f;

	// Volumetric fog.
	bool m_volumetricFogEnable = false;
	float m_maxDistance = 100.0f;
	float m_maxScattering = 10.0f;
};

}
