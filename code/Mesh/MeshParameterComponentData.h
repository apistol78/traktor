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
#include "Core/Containers/SmallMap.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ITexture;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::mesh
{

class MeshParameterComponent;

/*!
 * \ingroup Mesh
 */
class T_DLLCLASS MeshParameterComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	Ref< MeshParameterComponent > createComponent(resource::IResourceManager* resourceManager) const;

	void setTexture(const std::wstring& parameterName, const resource::Id< render::ITexture >& texture);

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	SmallMap< std::wstring, resource::Id< render::ITexture > > m_textures;
};

}
