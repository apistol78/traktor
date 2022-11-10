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
#include "Core/Ref.h"
#include "Core/Containers/SmallMap.h"
#include "Shape/Editor/Solid/SolidTypes.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace shape
    {

class IShape;
class PrimitiveEntity;

/*! Primitive entity data.
 * \ingroup Shape
 */
class T_DLLCLASS PrimitiveEntityData : public world::EntityData
{
    T_RTTI_CLASS;

public:
    PrimitiveEntityData();
    
    Ref< PrimitiveEntity > createEntity() const;

    void setMaterial(int32_t face, const Guid& material);

    virtual void serialize(ISerializer& s) override;

    BooleanOperation getOperation() const { return m_operation; }

    const IShape* getShape() const { return m_shape; }

    const SmallMap< int32_t, Guid >& getMaterials() const { return m_materials; }

private:
    friend class PrimitiveEditModifier;

    BooleanOperation m_operation;
	Ref< IShape > m_shape;
    SmallMap< int32_t, Guid > m_materials;
};

    }
}