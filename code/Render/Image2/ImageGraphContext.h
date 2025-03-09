/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

// #include <functional>
#include "Core/Containers/SmallMap.h"
#include "Core/Containers/StaticMap.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Core/Object.h"
#include "Render/Image2/ImageGraphTypes.h"
#include "Render/Shader.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Buffer;
class ITexture;
class ProgramParameters;
class RenderGraph;

/*!
 * \ingroup Render
 */
struct ImageGraphView
{
	Frustum viewFrustum;
	Matrix44 view;
	Matrix44 lastView;
	Matrix44 viewToLight;
	Matrix44 projection;
	Vector4 godRayDirection = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
	Vector4 shadowMapUvTransform = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
	int32_t sliceCount = 0;
	int32_t sliceIndex = 0;
	float sliceNearZ = 0.0f;
	float sliceFarZ = 0.0f;
	float shadowFarZ = 0.0f;
	float shadowMapBias = 0.0f;
	float deltaTime = 0.0f;
	float time = 0.0f;
	int32_t frame = 0;
};

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImageGraphContext : public Object
{
	T_RTTI_CLASS;

public:
	void associateExplicitTexture(img_handle_t textureId, ITexture* texture);

	void associateTexture(img_handle_t textureId, handle_t rgTextureId);

	void associateTextureTargetSet(img_handle_t textureId, RGTargetSet targetSetId, int32_t colorIndex);

	void associateTextureTargetSetDepth(img_handle_t textureId, RGTargetSet targetSetId);

	RGTargetSet findTextureTargetSetId(img_handle_t textureId) const;

	ITexture* findTexture(const RenderGraph& renderGraph, img_handle_t textureId) const;

	void associateSBuffer(img_handle_t sbufferId, RGBuffer frameSbufferId);

	RGBuffer findSBufferId(img_handle_t sbufferId) const;

	Buffer* findSBuffer(const RenderGraph& renderGraph, img_handle_t sbufferId) const;

	void setTechniqueFlag(const handle_t techniqueId, bool flag);

	const StaticMap< handle_t, bool, 16 >& getTechniqueFlags() const;

	void applyTechniqueFlags(const Shader* shader, Shader::Permutation& inoutPermutation) const;

private:
	struct TextureTargetSet
	{
		RGTargetSet targetSetId = RGTargetSet::Invalid;
		int32_t colorIndex = -1;
		handle_t textureId = 0;
		ITexture* texture = nullptr;
	};

	SmallMap< img_handle_t, TextureTargetSet > m_textureTargetSet;
	SmallMap< img_handle_t, RGBuffer > m_sbufferHandles;
	StaticMap< handle_t, bool, 16 > m_techniqueFlags;
};

}
