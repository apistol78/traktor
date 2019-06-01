#pragma once

#include <embree3/rtcore.h>
#include "Illuminate/Editor/IRayTracer.h"

namespace traktor
{

class RandomGeometry;

    namespace illuminate
    {

class RayTracer;

class RayTracerEmbree : public IRayTracer
{
    T_RTTI_CLASS;

public:
    RayTracerEmbree();

    virtual bool create(const IlluminateConfiguration* configuration) override final;

    virtual void destroy() override final;

    virtual void addLight(const Light& light) override final;

    virtual void addModel(const model::Model* model, const Transform& transform) override final;

    virtual void commit() override final;

    virtual Ref< drawing::Image > traceDirect(const GBuffer* gbuffer) const override final;

    virtual Ref< drawing::Image > traceIndirect(const GBuffer* gbuffer) const override final;

private:
	struct Surface
	{
		Color4f albedo;
	};

	const IlluminateConfiguration* m_configuration;
	AlignedVector< Light > m_lights;

	RTCDevice m_device;
	RTCScene m_scene;
	float m_maxDistance;

	Color4f sampleAnalyticalLights(
        RandomGeometry& random,
        const Vector4& origin,
        const Vector4& normal,
		bool secondary
    ) const;
};

    }
}