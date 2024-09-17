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
#include "Shape/Editor/Solid/SolidTypes.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::model
{

class Model;

}

namespace traktor::shape
{

class PrimitiveComponentData;

/*! Primitive entity.
 * \ingroup Shape
 */
class T_DLLCLASS PrimitiveComponent : public world::IEntityComponent
{
    T_RTTI_CLASS;

public:
    explicit PrimitiveComponent(const PrimitiveComponentData* data);

    virtual void destroy() override final;

    virtual void setOwner(world::Entity* owner) override final;

    virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

    virtual void update(const world::UpdateParams& update) override final;

	const PrimitiveComponentData* getData() const { return m_data; }

    const model::Model* getModel() const { return m_model; }

    void setSelectedMaterial(uint32_t selectedMaterial) { m_selectedMaterial = selectedMaterial; }

    uint32_t getSelectedMaterial() const { return m_selectedMaterial; }

    bool isDirty() const { return m_dirty; }

    void resetDirty() { m_dirty = false; }

protected:
    friend class PrimitiveComponentData;

	Ref< const PrimitiveComponentData > m_data;
	Ref< const model::Model > m_model;
    Transform m_transform;
	Aabb3 m_boundingBox;
    uint32_t m_selectedMaterial;
    bool m_dirty;
};

}
