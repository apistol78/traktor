/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::spray
{

class Trail;

/*! Ribbon trail persistent data.
 * \ingroup Spray
 */
class TrailData : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< Trail > createTrail(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Id< render::Shader > m_shader;
	float m_width = 1.0f;
	float m_age = 1.0f;
	float m_lengthThreshold = 1.0f;
	float m_breakThreshold = 0.0f;
};

}
