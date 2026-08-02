/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <embree4/rtcore.h>
#include "Core/Math/Quaternion.h"
#include "Core/Math/Transform.h"
#include "Model/Model.h"
#include "Shape/Editor/Bake/IRayTracer.h"

namespace traktor
{

class RandomGeometry;

}

namespace traktor::render
{

class SHEngine;

}

namespace traktor::shape
{

class RayTracer;

class RayTracerEmbree : public IRayTracer
{
	T_RTTI_CLASS;

public:
	virtual bool create(const BakeOperationData* configuration) override final;

	virtual void destroy() override final;

	virtual void addEnvironment(const IProbe* environment) override final;

	virtual void addLight(const Light& light) override final;

	virtual void addModel(const model::Model* model, const Transform& transform) override final;

	virtual void commit() override final;

	virtual Ref< render::SHCoeffs > traceProbe(const Vector4& position, const Vector4& size) const override final;

	virtual void traceLightmap(const model::Model* model, const GBuffer* gbuffer, drawing::Image* lightmapDiffuse, const int32_t region[4]) const override final;

	virtual Color4f traceRay(const Vector4& position, const Vector4& direction) const override final;

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

	/*! Placement of a model in the traced world, as registered through addModel. */
	struct Placement
	{
		Ref< const model::Model > model;
		Transform transform;
	};

	/*! Embree representation of a single source model.
	 *
	 * Vertex buffers and the flattened material table are created once per unique
	 * model and shared by every placement of it. \a scene is only created when a
	 * model is placed more than once; the buffers are then kept in model local
	 * space and each placement becomes an instance of \a scene. A model placed
	 * only once has its placement transform baked into the buffers instead, and
	 * its geometry is attached directly to the top level scene.
	 */
	struct Geometry
	{
		Ref< const model::Model > model;
		RTCGeometry mesh = nullptr;
		RTCScene scene = nullptr;
		AlignedVector< float* > buffers;
		AlignedVector< const model::Material* > materials;
		Aabb3 boundingBox;
	};

	/*! Geometry placed in the top level scene, indexed by its top level geometry ID. */
	struct Instance
	{
		const Geometry* geometry = nullptr;
		Quaternion rotation = Quaternion::identity();
	};

	const BakeOperationData* m_configuration = nullptr;
	Ref< const IProbe > m_environment;
	AlignedVector< Vector2 > m_shadowSampleOffsets;
	AlignedVector< Light > m_lights;
	RTCDevice m_device = nullptr;
	RTCScene m_scene = nullptr;
	AlignedVector< Placement > m_placements;
	AlignedVector< Geometry* > m_geometries;
	AlignedVector< Instance > m_instances;
	Ref< render::SHEngine > m_shEngine;
	Aabb3 m_boundingBox;

	Geometry* createGeometry(const model::Model* model, const Transform& transform);

	/*! Resolve which placement was hit.
	 *
	 * Instanced hits report the instance in the top level scene through \a instanceID
	 * and the geometry within the instanced scene through \a geometryID, non instanced
	 * hits leave \a instanceID invalid and report the top level geometry directly.
	 */
	const Instance& getInstance(uint32_t instanceID, uint32_t geometryID) const
	{
		return m_instances[(instanceID != RTC_INVALID_GEOMETRY_ID) ? instanceID : geometryID];
	}

	Color4f tracePath0(
		const Vector4& origin,
		const Vector4& normal,
		RandomGeometry& random,
		uint32_t extraLightMask
	) const;

	Color4f traceSinglePath(
		const Vector4& origin,
		const Vector4& direction,
		float maxDistance,
		RandomGeometry& random,
		uint32_t extraLightMask,
		int32_t depth
	) const;

	Scalar traceOcclusion(
		const Vector4& origin,
		const Vector4& normal,
		float maxDistance,
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

	static void shadowOccluded(const RTCFilterFunctionNArguments* args);
};

}
