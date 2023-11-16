/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Bake/TracerTask.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerTask", TracerTask, Object)

TracerTask::TracerTask(const Guid& sceneId, const BakeConfiguration* configuration)
:   m_sceneId(sceneId)
,   m_configuration(configuration)
{
}

void TracerTask::addTracerEnvironment(const TracerEnvironment* tracerEnvironment)
{
	m_tracerEnvironments.push_back(tracerEnvironment);
}

void TracerTask::addTracerLight(const TracerLight* tracerLight)
{
    m_tracerLights.push_back(tracerLight);
}

void TracerTask::addTracerModel(const TracerModel* tracerModel)
{
    m_tracerModels.push_back(tracerModel);
}

void TracerTask::addTracerOutput(const TracerOutput* tracerOutput)
{
    m_tracerOutputs.push_back(tracerOutput);
}

void TracerTask::addTracerIrradiance(const TracerIrradiance* tracerIrradiance)
{
    m_tracerIrradiances.push_back(tracerIrradiance);
}

void TracerTask::addTracerCamera(const TracerCamera* tracerCamera)
{
    m_tracerCameras.push_back(tracerCamera);
}

}
