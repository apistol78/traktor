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
#include "Render/SH/SHCoeffs.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/GBuffer.h"
#include "Shape/Editor/Bake/IRayTracer.h"
#include "Shape/Editor/Bake/TracerIrradiance.h"
#include "Shape/Editor/Bake/TracerLight.h"
#include "Shape/Editor/Bake/TracerModel.h"
#include "Shape/Editor/Bake/TracerOutput.h"
#include "Shape/Editor/Bake/TracerProcessor.h"
#include "Shape/Editor/Bake/TracerTask.h"
#include "World/IrradianceGridResource.h"

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

	// settings->getProperty< std::wstring >(L"BakePipelineOperator.RayTracerType", L"traktor.shape.RayTracerEmbree").c_str());
	m_rayTracerType = TypeInfo::find(L"traktor.shape.RayTracerEmbree");
	//m_rayTracerType = TypeInfo::find(L"traktor.shape.RayTracerLocal");
    
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

        const int32_t outputSize = tracerOutput->getLightmapSize();

		// Update status.
		m_status.current = i;
		m_status.total = tracerOutputs.size();
		m_status.description = tracerOutput->getName() + L" (" + toString(outputSize) + L" * " + toString(outputSize) + L")";

        uint32_t channel = renderModel->getTexCoordChannel(L"Lightmap");

        // Create GBuffer of mesh's geometry.
        GBuffer gbuffer;
        gbuffer.create(outputSize, outputSize, *renderModel, Transform::identity(), channel);

        // Preprocess GBuffer.
        rayTracer->preprocess(&gbuffer);

		gbuffer.saveAsImages(L"data/Temp/Bake/" + tracerOutput->getName() + L"_GBuffer");

        Ref< drawing::Image > lightmapDirect;
        Ref< drawing::Image > lightmapIndirect;

        if (configuration->traceDirect())
            lightmapDirect = rayTracer->traceDirect(&gbuffer);

        if (configuration->traceIndirect())
            lightmapIndirect = rayTracer->traceIndirect(&gbuffer);

		if (lightmapDirect)
			lightmapDirect->save(L"data/Temp/Bake/" + tracerOutput->getName() + L"_Direct.exr");
		if (lightmapIndirect)
			lightmapIndirect->save(L"data/Temp/Bake/" + tracerOutput->getName() + L"_Indirect.exr");

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

	// Trace irradiance grids.
	auto tracerIrradiances = task->getTracerIrradiances();
	for (uint32_t i = 0; i < tracerIrradiances.size(); ++i)
	{
		auto tracerIrradiance = tracerIrradiances[i];
		Guid irradianceGridId = tracerIrradiance->getIrradianceGridId();

		// Create output instance.
		Ref< world::IrradianceGridResource > outputResource = new world::IrradianceGridResource();
		Ref< db::Instance > outputInstance = m_outputDatabase->createInstance(
			L"Generated/" + tracerIrradiance->getIrradianceGridId().format(),
			db::CifReplaceExisting,
			&irradianceGridId
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

		const Scalar c_gridPerUnit(2.0f);

		// Determine bounding box from all trace models if noone is already provided.
		Aabb3 boundingBox = tracerIrradiance->getBoundingBox();
		if (boundingBox.empty())
		{
			for (auto tracerModel : task->getTracerModels())
				boundingBox.contain(tracerModel->getModel()->getBoundingBox());
			boundingBox.expand(c_gridPerUnit);
		}

		Vector4 worldSize = boundingBox.getExtent() * Scalar(2.0f);

		int32_t gridX = std::max((int32_t)(worldSize.x() * c_gridPerUnit + 0.5f), 2);
		int32_t gridY = std::max((int32_t)(worldSize.y() * c_gridPerUnit + 0.5f), 2);
		int32_t gridZ = std::max((int32_t)(worldSize.z() * c_gridPerUnit + 0.5f), 2);

		Writer writer(stream);

		writer << uint32_t(2);

		writer << (uint32_t)gridX;	// width
		writer << (uint32_t)gridY;	// height
		writer << (uint32_t)gridZ;	// depth

		writer << boundingBox.mn.x();
		writer << boundingBox.mn.y();
		writer << boundingBox.mn.z();
		writer << boundingBox.mx.x();
		writer << boundingBox.mx.y();
		writer << boundingBox.mx.z();

		uint32_t progress = 0;
		for (int32_t x = 0; x < gridX; ++x)
		{
			float fx = x / (float)(gridX - 1.0f);
			for (int32_t y = 0; y < gridY; ++y)
			{
				float fy = y / (float)(gridY - 1.0f);
				for (int32_t z = 0; z < gridZ; ++z)
				{
					float fz = z / (float)(gridZ - 1.0f);

					m_status.current = progress++;
					m_status.total = gridX * gridY * gridZ;
					m_status.description = L"Irradiance grid";

					Vector4 position = boundingBox.mn + (boundingBox.mx - boundingBox.mn) * Vector4(fx, fy, fz);

					Ref< render::SHCoeffs > sh = rayTracer->traceProbe(position.xyz1());
					if (!sh)
					{
						log::error << L"Trace failed; unable to trace irradiance probe." << Endl;
						return false;
					}
					T_FATAL_ASSERT(sh->get().size() == 9);

					for (int32_t i = 0; i < 9; ++i)
					{
						auto c = (*sh)[i];
						writer << c.x();
						writer << c.y();
						writer << c.z();
					}
				}
			}
		}

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
