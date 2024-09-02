/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IRenderSystem;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::spray
{

class EffectInstance;
class EffectLayer;

/*! Effect immutable descriptor.
 * \ingroup Spray
 */
class T_DLLCLASS Effect : public Object
{
	T_RTTI_CLASS;

public:
	explicit Effect(
		render::IRenderSystem* renderSystem,
		resource::IResourceManager* resourceManager,
		float duration,
		float loopStart,
		float loopEnd,
		const RefArray< EffectLayer >& layers
	);

	Ref< EffectInstance > createInstance() const;

	float getDuration() const { return m_duration; }

	float getLoopStart() const { return m_loopStart; }

	float getLoopEnd() const { return m_loopEnd; }

	const RefArray< EffectLayer >& getLayers() const { return m_layers; }

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< resource::IResourceManager > m_resourceManager;
	float m_duration;
	float m_loopStart;
	float m_loopEnd;
	RefArray< EffectLayer > m_layers;
};

}
