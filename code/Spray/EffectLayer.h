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

namespace traktor::world
{

class IEntityEvent;

}

namespace traktor::spray
{

class EffectLayerInstance;
class Emitter;
class Sequence;
class Trail;

/*! Effect layer.
 * \ingroup Spray
 */
class T_DLLCLASS EffectLayer : public Object
{
	T_RTTI_CLASS;

public:
	explicit EffectLayer(
		float time,
		float duration,
		Emitter* emitter,
		Trail* trail,
		Sequence* sequence,
		const world::IEntityEvent* triggerEnable,
		const world::IEntityEvent* triggerDisable
	);

	Ref< EffectLayerInstance > createInstance(render::IRenderSystem* renderSystem, resource::IResourceManager* resourceManager) const;

	float getTime() const { return m_time; }

	float getDuration() const { return m_duration; }

	const Emitter* getEmitter() const { return m_emitter; }

	const Trail* getTrail() const { return m_trail; }

	const Sequence* getSequence() const { return m_sequence; }

	const world::IEntityEvent* getTriggerEnable() const { return m_triggerEnable; }

	const world::IEntityEvent* getTriggerDisable() const { return m_triggerDisable; }

private:
	float m_time;
	float m_duration;
	Ref< Emitter > m_emitter;
	Ref< Trail > m_trail;
	Ref< Sequence > m_sequence;
	Ref< const world::IEntityEvent > m_triggerEnable;
	Ref< const world::IEntityEvent > m_triggerDisable;
};

}
