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
class TracerLight;
class TracerModel;
class TracerOutput;

class T_DLLCLASS TracerTask : public Object
{
    T_RTTI_CLASS;

public:
    TracerTask(const Guid& sceneId, const BakeConfiguration* configuration);

	const Guid& getSceneId() const { return m_sceneId; }

    const BakeConfiguration* getConfiguration() const { return m_configuration; }

    void addTracerLight(const TracerLight* tracerLight);

	const RefArray< const TracerLight >& getTracerLights() const { return m_tracerLights; }

    void addTracerModel(const TracerModel* tracerModel);

	const RefArray< const TracerModel >& getTracerModels() const { return m_tracerModels; }

    void addTracerOutput(const TracerOutput* tracerOutput);

	const RefArray< const TracerOutput >& getTracerOutputs() const { return m_tracerOutputs; }

private:
    Guid m_sceneId;
    Ref< const BakeConfiguration > m_configuration;
    RefArray< const TracerLight > m_tracerLights;
    RefArray< const TracerModel > m_tracerModels;
    RefArray< const TracerOutput > m_tracerOutputs;
};

    }
}
