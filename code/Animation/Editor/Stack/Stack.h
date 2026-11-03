/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector4.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class SkinnedMesh;

}

namespace traktor::animation
{

class StackLayer;
class Skeleton;

/*! Animation stack.
 * \ingroup Animation
 */
class T_DLLCLASS Stack : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addLayer(StackLayer* layer);

	void removeLayer(StackLayer* layer);

	const RefArray< StackLayer >& getLayers() const;

	const resource::Id< Skeleton >& getPreviewSkeleton() const { return m_previewSkeleton; }

	const resource::Id< mesh::SkinnedMesh >& getPreviewMesh() const { return m_previewMesh; }

	void setPreviewPosition(const Vector4& position);

	const Vector4& getPreviewPosition() const { return m_previewPosition; }

	void setPreviewAngles(const Vector4& angles);

	const Vector4& getPreviewAngles() const { return m_previewAngles; }

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< StackLayer > m_layers;
	resource::Id< Skeleton > m_previewSkeleton;
	resource::Id< mesh::SkinnedMesh > m_previewMesh;
	Vector4 m_previewPosition = Vector4(0.0f, -2.0f, 7.0f, 1.0f);
	Vector4 m_previewAngles = Vector4::zero();
};

}
