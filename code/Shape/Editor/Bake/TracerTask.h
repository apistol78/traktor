/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace shape
    {

class BakeConfiguration;
class TracerEnvironment;
class TracerIrradiance;
class TracerLight;
class TracerModel;
class TracerOutput;

class T_DLLCLASS TracerTask : public Object
{
    T_RTTI_CLASS;

public:
    explicit TracerTask(const Guid& sceneId, const BakeConfiguration* configuration);

	const Guid& getSceneId() const { return m_sceneId; }

    const BakeConfiguration* getConfiguration() const { return m_configuration; }

	void addTracerEnvironment(const TracerEnvironment* tracerEnvironment);

	const RefArray< const TracerEnvironment >& getTracerEnvironments() const { return m_tracerEnvironments; }

    void addTracerLight(const TracerLight* tracerLight);

	const RefArray< const TracerLight >& getTracerLights() const { return m_tracerLights; }

    void addTracerModel(const TracerModel* tracerModel);

	const RefArray< const TracerModel >& getTracerModels() const { return m_tracerModels; }

    void addTracerOutput(const TracerOutput* tracerOutput);

	const RefArray< const TracerOutput >& getTracerOutputs() const { return m_tracerOutputs; }

    void addTracerIrradiance(const TracerIrradiance* tracerIrradiance);

    const RefArray< const TracerIrradiance >& getTracerIrradiances() const { return m_tracerIrradiances; }

private:
    Guid m_sceneId;
    Ref< const BakeConfiguration > m_configuration;
	RefArray< const TracerEnvironment > m_tracerEnvironments;
    RefArray< const TracerLight > m_tracerLights;
    RefArray< const TracerModel > m_tracerModels;
    RefArray< const TracerOutput > m_tracerOutputs;
    RefArray< const TracerIrradiance > m_tracerIrradiances;
};

    }
}
