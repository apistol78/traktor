/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Aabb3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace db
    {

class Instance;

    }

    namespace shape
    {

class T_DLLCLASS TracerIrradiance : public Object
{
    T_RTTI_CLASS;

public:
    TracerIrradiance(
        db::Instance* irradianceInstance,
		const Aabb3& boundingBox
    );


    db::Instance* getIrradianceInstance() const { return m_irradianceInstance; }

	const Aabb3& getBoundingBox() const { return m_boundingBox; }

private:
    Ref< db::Instance > m_irradianceInstance;
	Aabb3 m_boundingBox;
};

    }
}