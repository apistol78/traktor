/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spray/SourceData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spray
{

/*! Virtual particle source persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS VirtualSourceData : public SourceData
{
	T_RTTI_CLASS;

public:
	VirtualSourceData();

	virtual Ref< const Source > createSource(resource::IResourceManager* resourceManager) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

}
