/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

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

namespace traktor::spray
{

class Effect;
class EffectComponent;

/*! Effect component persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS EffectComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	EffectComponentData() = default;

	explicit EffectComponentData(const resource::Id< Effect >& effect);

	Ref< EffectComponent > createComponent(resource::IResourceManager* resourceManager) const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< Effect >& getEffect() const { return m_effect; }

private:
	resource::Id< Effect > m_effect;
};

}
