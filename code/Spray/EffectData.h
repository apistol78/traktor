/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
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

class IEntityFactory;

}

namespace traktor::spray
{

class Effect;
class EffectLayerData;

/*! Effect persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS EffectData : public ISerializable
{
	T_RTTI_CLASS;

public:
	EffectData() = default;

	explicit EffectData(
		float duration,
		float loopStart,
		float loopEnd,
		const RefArray< EffectLayerData >& layers
	);

	Ref< Effect > createEffect(resource::IResourceManager* resourceManager, const world::IEntityFactory* entityFactory) const;

	void addLayer(EffectLayerData* layer);

	virtual void serialize(ISerializer& s) override final;

	float getDuration() const { return m_duration; }

	float getLoopStart() const { return m_loopStart; }

	float getLoopEnd() const { return m_loopEnd; }

	void setLayers(const RefArray< EffectLayerData >& layers) { m_layers = layers; }

	const RefArray< EffectLayerData >& getLayers() const { return m_layers; }

private:
	float m_duration = 0.0f;
	float m_loopStart = 0.0f;
	float m_loopEnd = 0.0f;
	RefArray< EffectLayerData > m_layers;
};

}
