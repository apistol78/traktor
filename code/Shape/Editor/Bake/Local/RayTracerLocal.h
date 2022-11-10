/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/RandomGeometry.h"
#include "Core/Math/SahTree.h"
#include "Shape/Editor/Bake/IRayTracer.h"

namespace traktor
{
    namespace shape
    {

class RayTracer;

class RayTracerLocal : public IRayTracer
{
    T_RTTI_CLASS;

public:
    RayTracerLocal();

    virtual bool create(const BakeConfiguration* configuration) override final;

    virtual void destroy() override final;

	virtual void addEnvironment(const IblProbe* environment) override final;

    virtual void addLight(const Light& light) override final;

    virtual void addModel(const model::Model* model, const Transform& transform) override final;

    virtual void commit() override final;

    virtual Ref< render::SHCoeffs > traceProbe(const Vector4& position) const override final;

    virtual void traceLightmap(const model::Model* model, const GBuffer* gbuffer, drawing::Image* lightmapDiffuse, drawing::Image* lightmapDirectional, const int32_t region[4]) const override final;

private:
	struct Surface
	{
		Color4f albedo;
	};

	const BakeConfiguration* m_configuration;
	SahTree m_sah;
	AlignedVector< Light > m_lights;
	AlignedVector< Winding3 > m_windings;
	AlignedVector< Surface > m_surfaces;
	float m_maxDistance;

    void cullLights(const GBuffer* gbuffer, AlignedVector< Light >& outLights) const;

	Color4f sampleAnalyticalLights(
        RandomGeometry& random,
        SahTree::QueryCache& sahCache,
        const AlignedVector< Light >& lights,
        const Vector4& origin,
        const Vector4& normal,
        uint32_t shadowSampleCount,
        float pointLightShadowRadius
    ) const;
};

    }
}