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
	Ref< FogComponent > createComponent(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s) override final;

private:
	friend class FogComponent;
	friend class FogComponentEntityPipeline;

	//! Seed permuted with m_mediumShader to name the generated, complete scattering
	//! shader; the pipeline builds it under that guid and createComponent binds it.
	static const Guid ms_generatedShaderSeed;

	Color4f m_mediumColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	float m_mediumDensity = 0.2f;
	Guid m_mediumShader = Guid(L"{B9E1127F-C81B-AF40-8989-368748993908}");

	// Distance fog.
	bool m_distanceFogEnable = false;
	float m_fogDistance = 90.0f;
	float m_fogElevation = 0.0f;

	// Volumetric fog.
	bool m_volumetricFogEnable = false;
	float m_maxDistance = 100.0f;

	// Phase function; two Henyey-Greenstein lobes mixed by phaseBlend. The
	// asymmetry g is the mean cosine of the scattering angle - 0 isotropic,
	// positive forward, negative backward - and decides how much brighter fog
	// reads looking into a light than away from it.
	float m_phaseForward = 0.7f;
	float m_phaseBackward = -0.4f;
	float m_phaseBlend = 0.2f;
};

}
