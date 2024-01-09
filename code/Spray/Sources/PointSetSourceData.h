/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Range.h"
#include "Core/Math/Vector4.h"
#include "Resource/Id.h"
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

class PointSet;

/*! Point set particle source persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS PointSetSourceData : public SourceData
{
	T_RTTI_CLASS;

public:
	PointSetSourceData();

	virtual Ref< const Source > createSource(resource::IResourceManager* resourceManager) const override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< PointSet >& getPointSet() const { return m_pointSet; }

private:
	resource::Id< PointSet > m_pointSet;
	Vector4 m_offset;
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

}
