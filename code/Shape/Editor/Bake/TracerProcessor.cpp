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
#include "Model/ModelAdjacency.h"
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

void encodeRGBM(drawing::Image* image)
{
	const float c_multiplierRange = 16.0f;

	Color4f cl;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			image->getPixelUnsafe(x, y, cl);

			// Normalize all channels from our valid range into 0-1.
			cl /= Scalar(c_multiplierRange);

			float r = clamp< float >(cl.getRed(), 0.0f, 1.0f);
			float g = clamp< float >(cl.getGreen(), 0.0f, 1.0f);
			float b = clamp< float >(cl.getBlue(), 0.0f, 1.0f);
			float M = max(r, max(g, b));

			float bestError = std::numeric_limits< float >::max();
			int32_t bestM = M;

			int32_t iM = (int32_t)std::ceil(M * 255.0f);
			for (int32_t m = std::max(iM - 16, 0); m <= std::min(iM + 16, 255); ++m)
			{
				float Mchk = float(m) / 255.0f;

				int32_t R = (int32_t)std::ceil(255.0f * clamp(r / Mchk, 0.0f, 1.0f));
				int32_t G = (int32_t)std::ceil(255.0f * clamp(g / Mchk, 0.0f, 1.0f));
				int32_t B = (int32_t)std::ceil(255.0f * clamp(b / Mchk, 0.0f, 1.0f));

				float dr = ((float)R / 255.0f) * Mchk;
				float dg = ((float)G / 255.0f) * Mchk;
				float db = ((float)B / 255.0f) * Mchk;

				float error = (r - dr) * (r - dr) + (g - dg) * (g - dg) + (b - db) * (b - db);
				if (error < bestError)
				{
					bestError = error;
					bestM = M;
				}
			}

			cl.set(
				r / bestM,
				g / bestM,
				b / bestM,
				bestM
			);

			image->setPixel(x, y, cl);
		}
	}
}

void line(const Vector2& from, const Vector2& to, const std::function< void(const Vector2, float) >& fn)
{
	Vector2 ad = (to - from);
	ad.x = std::abs(ad.x);
	ad.y = std::abs(ad.y);
	int32_t ln = (int32_t)(std::max(ad.x, ad.y) + 0.5f);
	for (int32_t i = 0; i <= ln; ++i)
	{
		float fraction = (float)i / ln;
		Vector2 position = lerp(from, to, fraction);
		fn(position, fraction);
	}
}

        }

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.TracerProcessor", TracerProcessor, Object)

TracerProcessor::TracerProcessor(const TypeInfo* rayTracerType, db::Database* outputDatabase)
:   m_outputDatabase(outputDatabase)
,   m_rayTracerType(rayTracerType)
,   m_thread(nullptr)
{
	T_FATAL_ASSERT(m_outputDatabase != nullptr);
	T_FATAL_ASSERT(m_rayTracerType != nullptr);

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
    
	// Remove any pending task which reference the same scene.
    auto it = std::find_if(m_tasks.begin(), m_tasks.end(), [=](const TracerTask* hs) {
        return hs->getSceneId() == task->getSceneId();
    });
    if (it != m_tasks.end())
        m_tasks.erase(it);

	// Check if currently processing task is same scene.
	if (m_activeTask != nullptr && m_activeTask->getSceneId() == task->getSceneId())
	{
		// \tbd Currently processing same scene, abort and restart.
	}

	// Add our task and issue processing thread.
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
        rayTracer->addModel(tracerModel->getModel(), tracerModel->getTransform());

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
        gbuffer.create(outputSize, outputSize, *renderModel, tracerOutput->getTransform(), channel);

        // Preprocess GBuffer.
        rayTracer->preprocess(&gbuffer);

		gbuffer.saveAsImages(L"data/Temp/Bake/" + tracerOutput->getName() + L"_GBuffer");

        Ref< drawing::Image > lightmapDirect;
        Ref< drawing::Image > lightmapIndirect;

        if (configuration->traceDirect())
            lightmapDirect = rayTracer->traceDirect(&gbuffer);

        if (configuration->traceIndirect())
            lightmapIndirect = rayTracer->traceIndirect(&gbuffer);

		// if (lightmapDirect)
		// 	lightmapDirect->save(L"data/Temp/Bake/" + tracerOutput->getName() + L"_Direct.exr");
		// if (lightmapIndirect)
		// 	lightmapIndirect->save(L"data/Temp/Bake/" + tracerOutput->getName() + L"_Indirect.exr");

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

		// Filter seams.
		if (configuration->getEnableSeamFilter())
		{
			model::ModelAdjacency adjacency(renderModel, model::ModelAdjacency::MdByPosition);
			for (uint32_t i = 0; i < adjacency.getEdgeCount(); ++i)
			{
				// Get shared edges of this polygon's edge.
				model::ModelAdjacency::share_vector_t shared;
				adjacency.getSharedEdges(i, shared);
				if (shared.size() != 1)
					continue;

				// Get attributes of this edge.
				const model::Polygon& polygonA = renderModel->getPolygon(adjacency.getPolygon(i));
				uint32_t Aivx0 = polygonA.getVertex(adjacency.getPolygonEdge(i));
				uint32_t Aivx1 = polygonA.getVertex((Aivx0 + 1) % polygonA.getVertexCount());

				// Get attributes of shared edge.
				const model::Polygon& polygonB = renderModel->getPolygon(adjacency.getPolygon(shared[0]));
				uint32_t Bivx0 = polygonB.getVertex(adjacency.getPolygonEdge(shared[0]));
				uint32_t Bivx1 = polygonB.getVertex((Bivx0 + 1) % polygonB.getVertexCount());

				model::Vertex Avx0 = renderModel->getVertex(Aivx0);
				model::Vertex Avx1 = renderModel->getVertex(Aivx1);
				model::Vertex Bvx0 = renderModel->getVertex(Bivx0);
				model::Vertex Bvx1 = renderModel->getVertex(Bivx1);

				// Swap indices if order is reversed.
				if (Bvx0.getPosition() == Avx1.getPosition())
				{
					std::swap(Bivx0, Bivx1);
					std::swap(Bvx0, Bvx1);
				}

				// Check for lightmap seam.
				if (
					Avx0.getPosition() == Bvx0.getPosition() &&
					Avx1.getPosition() == Bvx1.getPosition() &&
					Avx0.getNormal() == Bvx0.getNormal() &&
					Avx1.getNormal() == Bvx1.getNormal() &&
					(
						Avx0.getTexCoord(channel) != Bvx0.getTexCoord(channel) ||
						Avx1.getTexCoord(channel) != Bvx1.getTexCoord(channel)
					)
				)
				{
					Vector2 imageSize(lightmap->getWidth() - 1, lightmap->getHeight() - 1);

					Vector4 Ap0 = renderModel->getPosition(Avx0.getPosition());
					Vector4 Ap1 = renderModel->getPosition(Avx1.getPosition());
					Vector4 An0 = renderModel->getNormal(Avx0.getNormal());
					Vector4 An1 = renderModel->getNormal(Avx1.getNormal());
					Vector2 Auv0 = renderModel->getTexCoord(Avx0.getTexCoord(channel)) * imageSize;
					Vector2 Auv1 = renderModel->getTexCoord(Avx1.getTexCoord(channel)) * imageSize;

					Vector4 Bp0 = renderModel->getPosition(Bvx0.getPosition());
					Vector4 Bp1 = renderModel->getPosition(Bvx1.getPosition());
					Vector4 Bn0 = renderModel->getNormal(Bvx0.getNormal());
					Vector4 Bn1 = renderModel->getNormal(Bvx1.getNormal());
					Vector2 Buv0 = renderModel->getTexCoord(Bvx0.getTexCoord(channel)) * imageSize;
					Vector2 Buv1 = renderModel->getTexCoord(Bvx1.getTexCoord(channel)) * imageSize;

					float Auvln = (Auv1 - Auv0).length();
					float Buvln = (Buv1 - Buv0).length();

					if (Auvln >= Buvln)
					{
						line(Auv0, Auv1, [&](const Vector2& Auv, float fraction) {
							Vector2 Buv = lerp(Buv0, Buv1, fraction);

							int32_t Ax = (int32_t)(Auv.x);
							int32_t Ay = (int32_t)(Auv.y);
							int32_t Bx = (int32_t)(Buv.x);
							int32_t By = (int32_t)(Buv.y);

							Color4f Aclr, Bclr;
							if (lightmap->getPixel(Ax, Ay, Aclr) && lightmap->getPixel(Bx, By, Bclr))
							{
								lightmap->setPixel(Ax, Ay, Aclr * Scalar(0.75f) + Bclr * Scalar(0.25f));
								lightmap->setPixel(Bx, By, Aclr * Scalar(0.25f) + Bclr * Scalar(0.75f));
							}
						});
					}
					else
					{
						line(Buv0, Buv1, [&](const Vector2& Buv, float fraction) {
							Vector2 Auv = lerp(Auv0, Auv1, fraction);

							int32_t Ax = (int32_t)(Auv.x);
							int32_t Ay = (int32_t)(Auv.y);
							int32_t Bx = (int32_t)(Buv.x);
							int32_t By = (int32_t)(Buv.y);

							Color4f Aclr, Bclr;
							if (lightmap->getPixel(Ax, Ay, Aclr) && lightmap->getPixel(Bx, By, Bclr))
							{
								lightmap->setPixel(Ax, Ay, Aclr * Scalar(0.75f) + Bclr * Scalar(0.25f));
								lightmap->setPixel(Bx, By, Aclr * Scalar(0.25f) + Bclr * Scalar(0.75f));
							}
						});
					}
				}
			}
		}

        // Discard alpha.
        lightmap->clearAlpha(1.0f);

		// lightmap->save(L"data/Temp/Bake/" + tracerOutput->getName() + L".exr");

		// Convert into format which our lightmap texture will be.
		if (true)
		{
			encodeRGBM(lightmap);
			lightmap->convert(drawing::PixelFormat::getR8G8B8A8().endianSwapped());
		}
		else
			lightmap->convert(drawing::PixelFormat::getABGRF16().endianSwapped());

		// Create output instance.
		Guid lightmapId = tracerOutput->getLightmapId();
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

		Ref< render::TextureResource > outputResource = new render::TextureResource();
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

		// Write texture resource header.
		writer << uint32_t(12);
		writer << int32_t(lightmap->getWidth());
		writer << int32_t(lightmap->getHeight());
		writer << int32_t(1);
		writer << int32_t(1);
		writer << int32_t(render::TfR8G8B8A8); // int32_t(render::TfR16G16B16A16F);
		writer << bool(false);
		writer << uint8_t(render::Tt2D);
		writer << bool(false);
		writer << bool(false);

		// Write texture data.
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
