/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
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

namespace traktor::spray
{

class Source;

/*! Particle source persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS SourceData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< const Source > createSource(resource::IResourceManager* resourceManager) const = 0;

	virtual void serialize(ISerializer& s) override;

	float getConstantRate() const { return m_constantRate; }

	float getVelocityRate() const { return m_velocityRate; }

private:
	float m_constantRate = 0.0f;
	float m_velocityRate = 0.0f;
};

}
