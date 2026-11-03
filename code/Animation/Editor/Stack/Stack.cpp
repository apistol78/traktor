/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/Stack/Stack.h"

#include "Animation/Editor/Stack/StackLayer.h"
#include "Animation/Skeleton.h"
#include "Core/Serialization/AttributeNoHash.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Resource/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.Stack", 0, Stack, ISerializable)

void Stack::addLayer(StackLayer* layer)
{
	m_layers.push_back(layer);
}

void Stack::removeLayer(StackLayer* layer)
{
	m_layers.remove(layer);
}

const RefArray< StackLayer >& Stack::getLayers() const
{
	return m_layers;
}

void Stack::setPreviewPosition(const Vector4& position)
{
	m_previewPosition = position;
}

void Stack::setPreviewAngles(const Vector4& angles)
{
	m_previewAngles = angles;
}

void Stack::serialize(ISerializer& s)
{
	s >> MemberRefArray< StackLayer >(L"layers", m_layers); //, AttributePrivate());
	s >> resource::Member< Skeleton >(L"previewSkeleton", m_previewSkeleton);
	s >> resource::Member< mesh::SkinnedMesh >(L"previewMesh", m_previewMesh);
	s >> Member< Vector4 >(L"previewPosition", m_previewPosition, AttributePrivate() | AttributeNoHash());
	s >> Member< Vector4 >(L"previewAngles", m_previewAngles, AttributePrivate() | AttributeNoHash());
}

}
