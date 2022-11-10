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
class BakeConfiguration;
class GBuffer;
class IblProbe;

/*! Ray tracer interface.
 * \ingroup Illuminate
 */
class IRayTracer : public Object
{
    T_RTTI_CLASS;

public:
    virtual bool create(const BakeConfiguration* configuration) = 0;

    virtual void destroy() = 0;

	virtual void addEnvironment(const IblProbe* environment) = 0;

    virtual void addLight(const Light& light) = 0;

    virtual void addModel(const model::Model* model, const Transform& transform) = 0;

    virtual void commit() = 0;

    virtual Ref< render::SHCoeffs > traceProbe(const Vector4& position) const = 0;

    virtual void traceLightmap(const model::Model* model, const GBuffer* gbuffer, drawing::Image* lightmapDiffuse, drawing::Image* lightmapDirectional, const int32_t region[4]) const = 0;
};

    }
}