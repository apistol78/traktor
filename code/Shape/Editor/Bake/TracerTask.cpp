#include "Shape/Editor/Bake/TracerTask.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerTask", TracerTask, Object)

TracerTask::TracerTask(const Guid& sceneId, const BakeConfiguration* configuration)
:   m_sceneId(sceneId)
,   m_configuration(configuration)
{
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

    }
}