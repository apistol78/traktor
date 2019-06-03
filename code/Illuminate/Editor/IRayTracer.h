#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{

class Transform;
class Vector4;

    namespace drawing
    {

class Image;

    }

    namespace model
    {

class Model;

    }

    namespace render
    {

class SHCoeffs;

    }

    namespace illuminate
    {

struct Light;
class GBuffer;
class IlluminateConfiguration;

/*! \brief Ray tracer interface.
 * \ingroup Illuminate
 */
class IRayTracer : public Object
{
    T_RTTI_CLASS;

public:
    virtual bool create(const IlluminateConfiguration* configuration) = 0;

    virtual void destroy() = 0;

    virtual void addLight(const Light& light) = 0;

    virtual void addModel(const model::Model* model, const Transform& transform) = 0;

    virtual void commit() = 0;

    /*! \brief Preprocess GBuffer.
     *         Preprocess GBuffer so it's more optimal for tracing.
     *
     * Preprocessing can include functions which modify gbuffer data to reduce
     * fake shadowing artifacts etc.
     *
     * \param gbuffer GBuffer to preprocess.
     */
    virtual void preprocess(GBuffer* gbuffer) const = 0;

    virtual Ref< render::SHCoeffs > traceProbe(const Vector4& position) const = 0;

    virtual Ref< drawing::Image > traceDirect(const GBuffer* gbuffer) const = 0;

    virtual Ref< drawing::Image > traceIndirect(const GBuffer* gbuffer) const = 0;
};

    }
}