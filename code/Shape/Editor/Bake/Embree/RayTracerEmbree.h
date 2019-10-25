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
    RayTracerEmbree();

    virtual bool create(const BakeConfiguration* configuration) override final;

    virtual void destroy() override final;

    virtual void addLight(const Light& light) override final;

    virtual void addModel(const model::Model* model, const Transform& transform) override final;

    virtual void commit() override final;

    virtual void preprocess(GBuffer* gbuffer) const override final;

    virtual Ref< render::SHCoeffs > traceProbe(const Vector4& position) const override final;

    virtual Ref< drawing::Image > traceDirect(const GBuffer* gbuffer) const override final;

    virtual Ref< drawing::Image > traceIndirect(const GBuffer* gbuffer) const override final;

private:
	struct Surface
	{
		Color4f albedo;
	};

	const BakeConfiguration* m_configuration;
	AlignedVector< Light > m_lights;
	RTCDevice m_device;
	RTCScene m_scene;
	model::Model m_model;
    Ref< render::SHEngine > m_shEngine;
	float m_maxDistance;

	Color4f sampleAnalyticalLights(
        RandomGeometry& random,
        const Vector4& origin,
        const Vector4& normal,
		uint8_t mask,
		bool bounce
    ) const;
};

    }
}