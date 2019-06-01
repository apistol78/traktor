#pragma once

#include "Illuminate/Editor/IRayTracer.h"

namespace RadeonRays
{

class IntersectionApi;
    
}

namespace traktor
{
    namespace illuminate
    {

class RayTracerRadeonRays : public IRayTracer
{
    T_RTTI_CLASS;

public:
    RayTracerRadeonRays();

    virtual bool create(const IlluminateConfiguration* configuration) override final;

    virtual void destroy() override final;

    virtual void addLight(const Light& light) override final;

    virtual void addModel(const model::Model* model, const Transform& transform) override final;

    virtual void commit() override final;

    virtual Ref< drawing::Image > traceDirect(const GBuffer* gbuffer) const override final;

    virtual Ref< drawing::Image > traceIndirect(const GBuffer* gbuffer) const override final;

private:
	const IlluminateConfiguration* m_configuration;
    RadeonRays::IntersectionApi* m_api;
	AlignedVector< Light > m_lights;
};

    }
}