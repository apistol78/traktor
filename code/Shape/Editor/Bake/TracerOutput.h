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
#include "Core/Math/Transform.h"

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

    namespace model
    {

class Model;

    }

    namespace shape
    {

class T_DLLCLASS TracerOutput : public Object
{
    T_RTTI_CLASS;

public:
    TracerOutput(
		db::Instance* lightmapDiffuseInstance,
        const model::Model* model,
		const Transform& transform,
		int32_t lightmapSize
    );

	db::Instance* getLightmapDiffuseInstance() const { return m_lightmapDiffuseInstance; }

    const model::Model* getModel() const { return m_model; }

	const Transform& getTransform() const { return m_transform; }

	int32_t getLightmapSize() const { return m_lightmapSize; }

private:
	Ref< db::Instance > m_lightmapDiffuseInstance;
	Ref< const model::Model > m_model;
	Transform m_transform;
	int32_t m_lightmapSize;
};

    }
}