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

    namespace shape
    {

struct Light;
class GBuffer;
class BakeConfiguration;

/*! \brief Ray tracer interface.
 * \ingroup Illuminate
 */
class IRayTracer : public Object
{
    T_RTTI_CLASS;

public:
    virtual bool create(const BakeConfiguration* configuration) = 0;

    virtual void destroy() = 0;

    virtual void addLight(const Light& light) = 0;

    virtual void addModel(const model::Model* model, const Transform& transform) = 0;

    virtual void commit() = 0;

    /*! Preprocess GBuffer.
     *
     * Preprocessing can include functions which modify gbuffer data to reduce
     * fake shadowing artifacts etc.
     *
     * \param gbuffer GBuffer to preprocess.
     */
    virtual void preprocess(GBuffer* gbuffer) const = 0;

    virtual Ref< render::SHCoeffs > traceProbe(const Vector4& position) const = 0;

    virtual void traceLightmap(const GBuffer* gbuffer, drawing::Image* lightmap, const int32_t region[4]) const = 0;
};

    }
}