/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ITexture;
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
    void associateTexture(handle_t textureId, ITexture* texture);

	void associateTextureTargetSet(handle_t textureId, handle_t targetSetId, int32_t colorIndex);

    void associateTextureTargetSetDepth(handle_t textureId, handle_t targetSetId);

    handle_t findTextureTargetSetId(handle_t textureId) const;

    ITexture* findTexture(const RenderGraph& renderGraph, handle_t textureId) const;

	void setFloatParameter(handle_t handle, float value);

    const SmallMap< handle_t, float >& getFloatParameters() const { return m_scalarParameters; }

	void setVectorParameter(handle_t handle, const Vector4& value);

    const SmallMap< handle_t, Vector4 >& getVectorParameters() const { return m_vectorParameters; }

private:
	struct TextureTargetSet
	{
		handle_t targetSetId;
		int32_t colorIndex;
        ITexture* texture;
	};

	SmallMap< handle_t, TextureTargetSet > m_textureTargetSet;
	SmallMap< handle_t, float > m_scalarParameters;
	SmallMap< handle_t, Vector4 > m_vectorParameters;
};

	}
}