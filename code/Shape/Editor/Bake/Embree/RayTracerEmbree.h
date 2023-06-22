/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <embree3/rtcore.h>
#include "Model/Model.h"
#include "Shape/Editor/Bake/IRayTracer.h"

namespace traktor
{

class RandomGeometry;

	namespace render
	{

class SHEngine;

	}

	namespace shape
	{

class RayTracer;

class RayTracerEmbree : public IRayTracer
{
	T_RTTI_CLASS;

public:
	virtual bool create(const BakeConfiguration* configuration) override final;

	virtual void destroy() override final;

	virtual void addEnvironment(const IblProbe* environment) override final;

	virtual void addLight(const Light& light) override final;

	virtual void addModel(const model::Model* model, const Transform& transform) override final;

	virtual void commit() override final;

	virtual Ref< render::SHCoeffs > traceProbe(const Vector4& position) const override final;

	virtual void traceLightmap(const model::Model* model, const GBuffer* gbuffer, drawing::Image* lightmapDiffuse, const int32_t region[4]) const override final;

private:
	struct Surface
	{
		Color4f albedo;
	};

	struct Irradiance
	{
		Vector4 position;
		Vector4 normal;
		Color4f irradiance;

		operator const Vector4& () const { return position; }
	};

	const BakeConfiguration* m_configuration;
	Ref< const IblProbe > m_environment;
	AlignedVector< Vector2 > m_shadowSampleOffsets;
	AlignedVector< Light > m_lights;
	RTCDevice m_device = nullptr;
	RTCScene m_scene = nullptr;
	AlignedVector< float* > m_buffers;
	RefArray< const model::Model > m_models;
	AlignedVector< uint32_t > m_materialOffset;
	AlignedVector< const model::Material* > m_materials;
	Ref< render::SHEngine > m_shEngine;
	float m_maxDistance = 0.0f;
	Aabb3 m_boundingBox;

	Color4f tracePath0(
		const Vector4& origin,
		const Vector4& normal,
		RandomGeometry& random
	) const;

	Color4f traceSinglePath(
		const Vector4& origin,
		const Vector4& direction,
		RandomGeometry& random,
		int32_t depth
	) const;

	Scalar traceAmbientOcclusion(
		const Vector4& origin,
		const Vector4& normal,
		RandomGeometry& random
	) const;

	Color4f sampleAnalyticalLights(
		RandomGeometry& random,
		const Vector4& origin,
		const Vector4& normal,
		uint8_t mask,
		bool bounce
	) const;

	static void alphaTestFilter(const RTCFilterFunctionNArguments* args);
};

	}
}