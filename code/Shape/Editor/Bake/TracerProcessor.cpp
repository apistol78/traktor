#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Winding3.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/DilateFilter.h"
#include "Drawing/Functions/BlendFunction.h"
#include "Model/Model.h"
#include "Render/Types.h"
#include "Render/Resource/TextureResource.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/GBuffer.h"
#include "Shape/Editor/Bake/IRayTracer.h"
#include "Shape/Editor/Bake/TracerLight.h"
#include "Shape/Editor/Bake/TracerModel.h"
#include "Shape/Editor/Bake/TracerOutput.h"
#include "Shape/Editor/Bake/TracerProcessor.h"
#include "Shape/Editor/Bake/TracerTask.h"

#if !defined(__RPI__) && !defined(__APPLE__)
#	include <OpenImageDenoise/oidn.h>
#endif

namespace traktor
{
    namespace shape
    {
        namespace
        {

#if !defined(__RPI__) && !defined(__APPLE__)
Ref< drawing::Image > denoise(const GBuffer& gbuffer, drawing::Image* lightmap)
{
	int32_t width = lightmap->getWidth();
	int32_t height = lightmap->getHeight();

	lightmap->convert(drawing::PixelFormat::getRGBAF32());

	Ref< drawing::Image > albedo = new drawing::Image(
		drawing::PixelFormat::getRGBAF32(),
		lightmap->getWidth(),
		lightmap->getHeight()
	);
	albedo->clear(Color4f(1, 1, 1, 1));

	Ref< drawing::Image > normals = new drawing::Image(
		drawing::PixelFormat::getRGBAF32(),
		lightmap->getWidth(),
		lightmap->getHeight()
	);
	for (int32_t y = 0; y < height; ++y)
	{
		for (int32_t x = 0; x < width; ++x)
		{
			const auto elm = gbuffer.get(x, y);
			normals->setPixel(x, y, Color4f(elm.normal));
		}
	}

	Ref< drawing::Image > output = new drawing::Image(
		drawing::PixelFormat::getRGBAF32(),
		lightmap->getWidth(),
		lightmap->getHeight()
	);

	OIDNDevice device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);
	oidnCommitDevice(device);

	OIDNFilter filter = oidnNewFilter(device, "RT"); // generic ray tracing filter
	oidnSetSharedFilterImage(filter, "color",  lightmap->getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0);
	oidnSetSharedFilterImage(filter, "albedo", albedo->getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0); // optional
	oidnSetSharedFilterImage(filter, "normal", normals->getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0); // optional
	oidnSetSharedFilterImage(filter, "output", output->getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0);
	oidnSetFilter1b(filter, "hdr", true); // image is HDR
	oidnCommitFilter(filter);

	oidnExecuteFilter(filter);	

	// Check for errors
	const char* errorMessage;
	if (oidnGetDeviceError(device, &errorMessage) != OIDN_ERROR_NONE)
		log::error << mbstows(errorMessage) << Endl;

	// Cleanup
	oidnReleaseFilter(filter);
	oidnReleaseDevice(device);	
	return output;
}
#endif

        }

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerProcessor", TracerProcessor, Object)

TracerProcessor::TracerProcessor(db::Database* outputDatabase)
:   m_outputDatabase(outputDatabase)
,   m_rayTracerType(nullptr)
,   m_thread(nullptr)
{
	T_FATAL_ASSERT(m_outputDatabase != nullptr);

	m_rayTracerType = TypeInfo::find(L"traktor.shape.RayTracerEmbree"); // settings->getProperty< std::wstring >(L"BakePipelineOperator.RayTracerType", L"traktor.shape.RayTracerEmbree").c_str());
    
    m_thread = ThreadManager::getInstance().create(makeFunctor(this, &TracerProcessor::processorThread), L"Tracer");
    m_thread->start();
}

TracerProcessor::~TracerProcessor()
{
	// Ensure all tasks has finished until we stop thread.
	waitUntilIdle();

	// Stop task thread.
	if (m_thread != nullptr)
	{
		m_thread->stop();
		ThreadManager::getInstance().destroy(m_thread);
		m_thread = nullptr;
	}
}

void TracerProcessor::enqueue(const TracerTask* task)
{
    T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
    
    auto it = std::find_if(m_tasks.begin(), m_tasks.end(), [=](const TracerTask* hs) {
        return hs->getSceneId() == task->getSceneId();
    });
    if (it != m_tasks.end())
        m_tasks.erase(it);
    
    m_tasks.push_back(task);

    m_event.broadcast();
}

void TracerProcessor::cancelAll()
{
    T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
    m_tasks.clear();
}

void TracerProcessor::waitUntilIdle()
{
    Thread* thread = ThreadManager::getInstance().getCurrentThread();
	while (!m_tasks.empty() || m_activeTask != nullptr)
		thread->yield();
}

TracerProcessor::Status TracerProcessor::getStatus() const
{
	return m_status;
}

void TracerProcessor::processorThread()
{
    while (!m_thread->stopped())
    {
        if (!m_event.wait(100))
            continue;

        {
            T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
            if (!m_tasks.empty())
            {
                m_activeTask = m_tasks.front();
                m_tasks.pop_front();
            }
        }

        if (m_activeTask)
        {
			m_status.active = true;
            process(m_activeTask);
			m_status.active = false;
            m_activeTask = nullptr;
        }
    }
	cancelAll();
}

bool TracerProcessor::process(const TracerTask* task) const
{
    auto configuration = task->getConfiguration();
    T_FATAL_ASSERT(configuration != nullptr);

   	// Create raytracer implementation.
	Ref< IRayTracer > rayTracer = checked_type_cast< IRayTracer* >(m_rayTracerType->createInstance());
	if (!rayTracer->create(configuration))
		return false;

    // Setup raytracer scene.
    for (auto tracerLight : task->getTracerLights())
        rayTracer->addLight(tracerLight->getLight());
    for (auto tracerModel : task->getTracerModels())
        rayTracer->addModel(tracerModel->getModel(), Transform::identity());

    rayTracer->commit();

	// Get output tasks and sort them by priority.
	auto tracerOutputs = task->getTracerOutputs();
	tracerOutputs.sort([](const TracerOutput* lh, const TracerOutput* rh) {
		return lh->getPriority() > rh->getPriority();
	});

    // Trace each lightmap in task.
	for (uint32_t i = 0; i < tracerOutputs.size(); ++i)
    {
		auto tracerOutput = tracerOutputs[i];
        auto renderModel = tracerOutput->getModel();
        T_FATAL_ASSERT(renderModel != nullptr);

		// Update status.
		m_status.current = i;
		m_status.total = tracerOutputs.size();
		m_status.description = tracerOutput->getName() + L" (" + toString(tracerOutput->getPriority()) + L")";

        // Calculate output size from lumel density.
        float totalWorldArea = 0.0f;
        for (const auto& polygon : renderModel->getPolygons())
        {
            Winding3 polygonWinding;
            for (const auto index : polygon.getVertices())
                polygonWinding.push(renderModel->getVertexPosition(index));
            totalWorldArea += abs(polygonWinding.area());
        }

        const float totalLightMapArea = configuration->getLumelDensity() * configuration->getLumelDensity() * totalWorldArea;
        const float size = std::sqrt(totalLightMapArea);
        
        const int32_t outputSize = alignUp(std::max< int32_t >(
            configuration->getMinimumLightMapSize(),
                (int32_t)(size + 0.5f)
        ), 16);

        uint32_t channel = renderModel->getTexCoordChannel(L"Lightmap");

        // Create GBuffer of mesh's geometry.
        GBuffer gbuffer;
        gbuffer.create(outputSize, outputSize, *renderModel, Transform::identity(), channel);

		gbuffer.saveAsImages(tracerOutput->getName() + L"_Lightmap_Pre_");

        // Preprocess GBuffer.
        rayTracer->preprocess(&gbuffer);

		gbuffer.saveAsImages(tracerOutput->getName() + L"_Lightmap_Post_");

        Ref< drawing::Image > lightmapDirect;
        Ref< drawing::Image > lightmapIndirect;

        if (configuration->traceDirect())
            lightmapDirect = rayTracer->traceDirect(&gbuffer);

        if (configuration->traceIndirect())
            lightmapIndirect = rayTracer->traceIndirect(&gbuffer);

        if (configuration->getEnableDilate())
        {
            // Dilate lightmap to prevent leaking.
            drawing::DilateFilter dilateFilter(3);
            if (lightmapDirect)
                lightmapDirect->apply(&dilateFilter);
            if (lightmapIndirect)
                lightmapIndirect->apply(&dilateFilter);
        }

        // Blur indirect lightmap to reduce noise from path tracing.
#if !defined(__RPI__) && !defined(__APPLE__)
        if (configuration->getEnableDenoise())
        {
            if (lightmapDirect)
                lightmapDirect = denoise(gbuffer, lightmapDirect);
            if (lightmapIndirect)
                lightmapIndirect = denoise(gbuffer, lightmapIndirect);
        }
#endif

        // Merge direct and indirect lightmaps.
        Ref< drawing::Image > lightmap;
        if (lightmapDirect && lightmapIndirect)
        {
            lightmap = lightmapDirect;
            lightmap->copy(lightmapIndirect, 0, 0, outputSize, outputSize, drawing::BlendFunction(
                drawing::BlendFunction::BfOne,
                drawing::BlendFunction::BfOne,
                drawing::BlendFunction::BoAdd
            ));
        }
        else if (lightmapDirect)
            lightmap = lightmapDirect;
        else if (lightmapIndirect)
            lightmap = lightmapIndirect;

        lightmapDirect = nullptr;
        lightmapIndirect = nullptr;

        if (!lightmap)
            return false;

        // Clamp shadow below threshold; to prevent tonemap to bring up noise.
        if (configuration->getClampShadowThreshold() > FUZZY_EPSILON)
        {
            for (uint32_t y = 0; y < lightmap->getHeight(); ++y)
            {
                for (uint32_t x = 0; x < lightmap->getWidth(); ++x)
                {
                    Color4f lumel;
                    lightmap->getPixelUnsafe(x, y, lumel);

                    Scalar intensity = dot3(lumel, Vector4(1.0f, 1.0f, 1.0f, 0.0f));

                    intensity = (intensity - Scalar(configuration->getClampShadowThreshold())) / Scalar(1.0f - configuration->getClampShadowThreshold());
                    if (intensity < 0.0f)
                        intensity = Scalar(0.0f);

                    lightmap->setPixelUnsafe(x, y, lumel * intensity);
                }
            }
        }

        // Discard alpha.
        lightmap->clearAlpha(1.0f);

		// Convert into format which our lightmap texture will be.
		lightmap->convert(drawing::PixelFormat::getABGRF16().endianSwapped());

		Guid lightmapId = tracerOutput->getLightmapId();

		// Create output instance.
		Ref< render::TextureResource > outputResource = new render::TextureResource();
		Ref< db::Instance > outputInstance = m_outputDatabase->createInstance(
			L"Generated/" + tracerOutput->getLightmapId().format(),
			db::CifReplaceExisting,
			&lightmapId
		);
		if (!outputInstance)
		{
			log::error << L"Trace failed; unable to create output instance." << Endl;
			return false;
		}

		outputInstance->setObject(outputResource);

		// Create output data stream.
		Ref< IStream > stream = outputInstance->writeData(L"Data");
		if (!stream)
		{
			log::error << L"Trace failed; unable to create texture data stream." << Endl;
			outputInstance->revert();
			return false;
		}

		Writer writer(stream);

		writer << uint32_t(12);
		writer << int32_t(lightmap->getWidth());
		writer << int32_t(lightmap->getHeight());
		writer << int32_t(1);
		writer << int32_t(1);
		writer << int32_t(render::TfR16G16B16A16F);
		writer << bool(false);
		writer << uint8_t(render::Tt2D);
		writer << bool(false);
		writer << bool(false);

		uint32_t dataSize = render::getTextureMipPitch(
			render::TfR16G16B16A16F,
			lightmap->getWidth(),
			lightmap->getHeight()
		);

		const uint8_t* data = static_cast< const uint8_t* >(lightmap->getData());
		if (writer.write(data, dataSize, 1) != dataSize)
			return false;

		stream->close();

		if (!outputInstance->commit())
		{
			log::error << L"Trace failed; unable to commit output instance." << Endl;
			return false;
		}
    }

    return true;
}

    }
}
