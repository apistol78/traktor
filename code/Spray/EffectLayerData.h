/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class IEntityEventData;
class IEntityFactory;

}

namespace traktor::spray
{

class EffectLayer;
class EmitterData;
class SequenceData;
class TrailData;

/*! Effect layer persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS EffectLayerData : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< EffectLayer > createEffectLayer(resource::IResourceManager* resourceManager, const world::IEntityFactory* entityFactory) const;

	virtual void serialize(ISerializer& s) override final;

	void setName(const std::wstring& name) { m_name = name; }

	const std::wstring& getName() const { return m_name; }

	void setTime(float time) { m_time = time; }

	float getTime() const { return m_time; }

	void setDuration(float duration) { m_duration = duration; }

	float getDuration() const { return m_duration; }

	EmitterData* getEmitter() const { return m_emitter; }

	TrailData* getTrail() const { return m_trail; }

	SequenceData* getSequence() const { return m_sequence; }

	world::IEntityEventData* getTriggerEnable() const { return m_triggerEnable; }

	world::IEntityEventData* getTriggerDisable() const { return m_triggerDisable; }

private:
	std::wstring m_name;
	float m_time = 0.0f;
	float m_duration = 0.0f;
	Ref< EmitterData > m_emitter;
	Ref< TrailData > m_trail;
	Ref< SequenceData > m_sequence;
	Ref< world::IEntityEventData > m_triggerEnable;
	Ref< world::IEntityEventData > m_triggerDisable;
};

}
