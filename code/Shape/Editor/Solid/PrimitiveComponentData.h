/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::shape
{

class IShape;
class PrimitiveComponent;

/*! Primitive entity data.
 * \ingroup Shape
 */
class T_DLLCLASS PrimitiveComponentData : public world::IEntityComponentData
{
    T_RTTI_CLASS;

public:
    Ref< PrimitiveComponent > createComponent() const;

    void setMaterial(int32_t face, const Guid& material);

    virtual int32_t getOrdinal() const override final;

    virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

    virtual void serialize(ISerializer& s) override final;

    BooleanOperation getOperation() const { return m_operation; }

    const IShape* getShape() const { return m_shape; }

    const SmallMap< int32_t, Guid >& getMaterials() const { return m_materials; }

private:
    friend class PrimitiveEditModifier;

    BooleanOperation m_operation = BooleanOperation::Union;
	Ref< IShape > m_shape;
    SmallMap< int32_t, Guid > m_materials;
};

}
