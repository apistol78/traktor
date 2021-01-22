#include <numeric>
#include "Core/Io/FileSystem.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Winding3.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/DilateFilter.h"
#include "Drawing/Filters/EncodeRGBM.h"
#include "Drawing/Functions/BlendFunction.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Render/Types.h"
#include "Render/Editor/Texture/AstcCompressor.h"
#include "Render/Editor/Texture/DxtnCompressor.h"
#include "Render/Editor/Texture/UnCompressor.h"
#include "Render/Resource/TextureResource.h"
#include "Render/SH/SHCoeffs.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"
#include "Shape/Editor/Bake/GBuffer.h"
#include "Shape/Editor/Bake/IRayTracer.h"
#include "Shape/Editor/Bake/TracerEnvironment.h"
#include "Shape/Editor/Bake/TracerIrradiance.h"
#include "Shape/Editor/Bake/TracerLight.h"
#include "Shape/Editor/Bake/TracerModel.h"
#include "Shape/Editor/Bake/TracerOutput.h"
#include "Shape/Editor/Bake/TracerProcessor.h"
#include "Shape/Editor/Bake/TracerTask.h"
#include "World/IrradianceGridResource.h"

#if !defined(__RPI__)
#	include <OpenImageDenoise/oidn.h>
#endif

namespace traktor
{
	namespace shape
	{
		namespace
		{

Ref< drawing::Image > denoise(const GBuffer& gbuffer, drawing::Image* lightmap)
{
#if !defined(__RPI__)
	int32_t width = lightmap->getWidth();
	int32_t height = lightmap->getHeight();

	lightmap->convert(drawing::PixelFormat::getRGBAF32());

	drawing::Image albedo(
		drawing::PixelFormat::getRGBAF32(),
		width,
		height
	);
	albedo.clear(Color4f(1.0f, 1.0f, 1.0f, 1.0f));

	drawing::Image normals(
		drawing::PixelFormat::getRGBAF32(),
		width,
		height
	);
	for (int32_t y = 0; y < height; ++y)
	{
		for (int32_t x = 0; x < width; ++x)
		{
			const auto elm = gbuffer.get(x, y);
			normals.setPixelUnsafe(x, y, Color4f(elm.normal));
		}
	}

	Ref< drawing::Image > output = new drawing::Image(
		drawing::PixelFormat::getRGBAF32(),
		lightmap->getWidth(),
		lightmap->getHeight()
	);
	output->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

	OIDNDevice device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);
	oidnCommitDevice(device);

	OIDNFilter filter = oidnNewFilter(device, "RT"); // generic ray tracing filter
	oidnSetSharedFilterImage(filter, "color",  lightmap->getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0);
	oidnSetSharedFilterImage(filter, "albedo", albedo.getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0); // optional
	oidnSetSharedFilterImage(filter, "normal", normals.getData(), OIDN_FORMAT_FLOAT3, width, height, 0, 4 * sizeof(float), 0); // optional
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
#else
	return lightmap;
#endif
}

bool writeTexture(
	db::Database* outputDatabase,
	const std::wstring& compressionMethod,
	const Guid& lightmapId,
	const drawing::Image* lightmap
)
{
	render::TextureFormat textureFormat = render::TfInvalid;
	Ref< drawing::Image > lightmapFormat = lightmap->clone();
	Ref< render::ICompressor > compressor;
	bool needAlpha;

	// Convert image to match texture format.
	if (compareIgnoreCase(compressionMethod, L"DXTn") == 0)
	{
		const drawing::EncodeRGBM encodeRGBM(5.0f, 4, 4, 0.8f);
		lightmapFormat->apply(&encodeRGBM);
		lightmapFormat->convert(drawing::PixelFormat::getR8G8B8A8().endianSwapped());
		textureFormat = render::TfDXT5;
		compressor = new render::DxtnCompressor();
		needAlpha = true;
	}
	else if (compareIgnoreCase(compressionMethod, L"ASTC") == 0)
	{
		const drawing::EncodeRGBM encodeRGBM(5.0f, 4, 4, 0.8f);
		lightmapFormat->apply(&encodeRGBM);
		textureFormat = render::TfASTC4x4;
		compressor = new render::AstcCompressor();
		needAlpha = true;
	}
	else
	{
		const drawing::EncodeRGBM encodeRGBM(5.0f);
		lightmapFormat->apply(&encodeRGBM);
		lightmapFormat->convert(drawing::PixelFormat::getR8G8B8A8().endianSwapped());
		textureFormat = render::TfR8G8B8A8;
		compressor = new render::UnCompressor();
		needAlpha = true;
	}

	Ref< db::Instance > outputInstance = outputDatabase->createInstance(
		L"Generated/" + lightmapId.format(),
		db::CifReplaceExisting,
		&lightmapId
	);
	if (!outputInstance)
		return false;

	Ref< render::TextureResource > outputResource = new render::TextureResource();
	outputInstance->setObject(outputResource);

	// Create output data stream.
	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		outputInstance->revert();
		return false;
	}

	Writer writer(stream);

	// Write texture resource header.
	writer << uint32_t(12);
	writer << int32_t(lightmapFormat->getWidth());
	writer << int32_t(lightmapFormat->getHeight());
	writer << int32_t(1);
	writer << int32_t(1);
	writer << int32_t(textureFormat);
	writer << bool(false);
	writer << uint8_t(render::Tt2D);
	writer << bool(false);
	writer << bool(false);

	// Write texture data.
	RefArray< drawing::Image > mipImages(1);
	mipImages[0] = lightmapFormat;
	compressor->compress(writer, mipImages, textureFormat, needAlpha, 3);

	stream->close();

	if (!outputInstance->commit())
		return false;
	
	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerProcessor", TracerProcessor, Object)

TracerProcessor::TracerProcessor(const TypeInfo* rayTracerType, const std::wstring& compressionMethod, bool editor)
:   m_rayTracerType(rayTracerType)
,	m_compressionMethod(compressionMethod)
,	m_editor(editor)
,   m_thread(nullptr)
{
	T_FATAL_ASSERT(m_rayTracerType != nullptr);

	m_thread = ThreadManager::getInstance().create(makeFunctor(this, &TracerProcessor::processorThread), L"Tracer");
	m_thread->start();

	if (!m_editor)
		m_queue = &JobManager::getInstance().getQueue();
	else
	{
		m_queue = new JobQueue();
		m_queue->create(4, Thread::Below);
	}

	FileSystem::getInstance().makeAllDirectories(L"data/Temp/Lightmaps");
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

	if (m_editor)
		safeDestroy(m_queue);
}

void TracerProcessor::enqueue(const TracerTask* task)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	
	cancel(task->getSceneId());

	// Add our task and issue processing thread.
	m_tasks.push_back(task);
	m_event.broadcast();
}

void TracerProcessor::cancel(const Guid& sceneId)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	
	// Remove any pending task which reference the same scene.
	auto it = std::find_if(m_tasks.begin(), m_tasks.end(), [=](const TracerTask* hs) {
		return hs->getSceneId() == sceneId;
	});
	if (it != m_tasks.end())
		m_tasks.erase(it);

	// Check if currently processing task is same scene.
	if (m_activeTask != nullptr && m_activeTask->getSceneId() == sceneId)
	{
		// Currently processing same scene, abort and restart.
		m_cancelled = true;
	}
}

void TracerProcessor::cancelAll()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_tasks.clear();
	m_cancelled = true;
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
	int32_t pending;

	Timer timer;
	timer.start();

	while (!m_thread->stopped())
	{
		m_event.wait(100);

		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			if (!m_tasks.empty())
			{
				m_activeTask = m_tasks.front();
				m_tasks.pop_front();
				pending = (int32_t)m_tasks.size();
			}
		}

		if (m_activeTask)
		{
			m_status.active = true;
			m_cancelled = false;

			log::info << L"Lightmap task " << m_activeTask->getSceneId().format() << L" started (" << pending << L" pending)." << Endl;
			double Tstart = timer.getElapsedTime();

			process(m_activeTask);

			double Tend = timer.getElapsedTime();
			if (!m_cancelled)
				log::info << L"Lightmap task " << m_activeTask->getSceneId().format() << L" finished in " << (int32_t)(Tend - Tstart) << L" seconds." << Endl;

			m_status.active = false;
			m_activeTask = nullptr;
		}
	}

	cancelAll();
}

bool TracerProcessor::process(const TracerTask* task)
{
	auto configuration = task->getConfiguration();
	T_FATAL_ASSERT(configuration != nullptr);

	// Update status.
	m_status.description = L"Preparing...";

   	// Create raytracer implementation.
	Ref< IRayTracer > rayTracer = checked_type_cast< IRayTracer* >(m_rayTracerType->createInstance());
	if (!rayTracer->create(configuration))
		return false;

	// Setup raytracer scene.
	for (auto tracerEnvironment : task->getTracerEnvironments())
		rayTracer->addEnvironment(tracerEnvironment->getEnvironment());
	for (auto tracerLight : task->getTracerLights())
		rayTracer->addLight(tracerLight->getLight());
	for (auto tracerModel : task->getTracerModels())
		rayTracer->addModel(tracerModel->getModel(), tracerModel->getTransform());

	rayTracer->commit();

	// Get output tasks and sort them by priority.
	auto tracerOutputs = task->getTracerOutputs();
	tracerOutputs.sort([](const TracerOutput* lh, const TracerOutput* rh) {
		return lh->getPriority() > rh->getPriority();
	});

	// Calculate total progress.
	m_status.total = std::accumulate(tracerOutputs.begin(), tracerOutputs.end(), (int32_t)0, [](int32_t acc, const TracerOutput* iter) {
		return acc + (iter->getLightmapSize() / 16) * (iter->getLightmapSize() / 16);
	});
	m_status.current = 0;

	// Trace each lightmap in task.
	for (uint32_t i = 0; !m_cancelled && i < tracerOutputs.size(); ++i)
	{
		auto tracerOutput = tracerOutputs[i];
		auto renderModel = tracerOutput->getModel();
		T_FATAL_ASSERT(renderModel != nullptr);

		const int32_t width = tracerOutput->getLightmapSize();
		const int32_t height = width;
		const uint32_t channel = renderModel->getTexCoordChannel(L"Lightmap");

		// Update status.
		m_status.description = str(L"%s (%d)...", tracerOutput->getName().c_str(), width);

		// Create GBuffer of mesh's geometry.
		GBuffer gbuffer;
		gbuffer.create(width, height, *renderModel, tracerOutput->getTransform(), channel);

		// Trace lightmap.
		Ref< drawing::Image > lightmap = new drawing::Image(
			drawing::PixelFormat::getRGBAF32(),
			width,
			height
		);
		lightmap->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

		RefArray< Job > jobs;
		for (int32_t ty = 0; !m_cancelled && ty < height; ty += 16)
		{
			Ref< Job > job = m_queue->add(makeFunctor([&, ty](){
				for (int32_t tx = 0; tx < width; tx += 16)
				{
					int32_t region[] = { tx, ty, tx + 16, ty + 16 };
					rayTracer->traceLightmap(renderModel, &gbuffer, lightmap, region);
					++m_status.current;
				}
			}));
			jobs.push_back(job);
		}
		while (!jobs.empty())
		{
			jobs.back()->wait();
			jobs.pop_back();
		}

		if (m_cancelled)
			break;

		// Blur lightmap to reduce noise from path tracing.
		if (configuration->getEnableDenoise())
			lightmap = denoise(gbuffer, lightmap);

		// Discard alpha.
		lightmap->clearAlpha(1.0f);

		// Save lightmap for debugging.
		lightmap->save(L"data/Temp/Lightmaps/" + tracerOutput->getLightmapId().format() + L".png");

		// Create final output instance.
		bool result = writeTexture(
			task->getOutputDatabase(),
			m_compressionMethod,
			tracerOutput->getLightmapId(),
			lightmap
		);
		if (!result)
		{
			log::error << L"Trace failed; unable to create output lightmap texture for \"" << tracerOutput->getName() << L"\"." << Endl;
			return false;
		}
	}

	// Trace irradiance grids.
	auto tracerIrradiances = task->getTracerIrradiances();
	for (uint32_t i = 0; !m_cancelled && i < tracerIrradiances.size(); ++i)
	{
		auto tracerIrradiance = tracerIrradiances[i];
		Guid irradianceGridId = tracerIrradiance->getIrradianceGridId();

		// Create output instance.
		Ref< world::IrradianceGridResource > outputResource = new world::IrradianceGridResource();
		Ref< db::Instance > outputInstance = task->getOutputDatabase()->createInstance(
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

		const Scalar gridDensity(configuration->getIrradianceGridDensity());

		// Determine bounding box from all trace models if noone is already provided.
		Aabb3 boundingBox = tracerIrradiance->getBoundingBox();
		if (boundingBox.empty())
		{
			for (auto tracerModel : task->getTracerModels())
				boundingBox.contain(tracerModel->getModel()->getBoundingBox().transform(tracerModel->getTransform()));
			boundingBox.expand(gridDensity);
		}

		Vector4 worldSize = boundingBox.getExtent() * Scalar(2.0f);

		int32_t gridX = std::max((int32_t)(worldSize.x() * gridDensity + 0.5f), 2);
		int32_t gridY = std::max((int32_t)(worldSize.y() * gridDensity + 0.5f), 2);
		int32_t gridZ = std::max((int32_t)(worldSize.z() * gridDensity + 0.5f), 2);

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

		m_status.description = str(L"Irradiance grid (%d, %d, %d)", gridX, gridY, gridZ);

		uint32_t progress = 0;
		for (int32_t x = 0; !m_cancelled && x < gridX; ++x)
		{
			float fx = x / (float)(gridX - 1.0f);
			for (int32_t y = 0; !m_cancelled && y < gridY; ++y)
			{
				float fy = y / (float)(gridY - 1.0f);
				for (int32_t z = 0; !m_cancelled && z < gridZ; ++z)
				{
					float fz = z / (float)(gridZ - 1.0f);

					m_status.current = progress++;
					m_status.total = gridX * gridY * gridZ;

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

		if (!m_cancelled)
		{
			if (!outputInstance->commit())
			{
				log::error << L"Trace failed; unable to commit output instance." << Endl;
				return false;
			}
		}
		else
			outputInstance->revert();
	}

	return true;
}

	}
}
