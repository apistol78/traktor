#include <functional>
#include <radeon_rays.h>
#undef PI
#include "Core/Containers/CircularVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Timer.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Illuminate/Editor/GBuffer.h"
#include "Illuminate/Editor/IlluminateConfiguration.h"
#include "Illuminate/Editor/RayTracerRadeonRays.h"
#include "Model/Model.h"

using namespace RadeonRays;

namespace traktor
{
    namespace illuminate
    {
        namespace
        {

const uint32_t c_bufferPoolSize = 512;

class BufferPool : public Object
{
public:
    Buffer* acquire();

    void release(Buffer*);

private:
    CircularVector< Buffer*, c_bufferPoolSize + 1 > m_free;
};

Buffer* BufferPool::acquire()
{
    Buffer* b = nullptr;
    if (!m_free.empty())
    {
        b = m_free.front();
        m_free.pop_front();
    }
    return b;
}

void BufferPool::release(Buffer* b)
{
    T_FATAL_ASSERT(!m_free.full());
    m_free.push_back(b);
}


class Task : public Object
{
public:
    IntersectionApi* m_api;
    int32_t m_activeRayCount;
    Scalar m_attenutate;
	Buffer* m_rayBuffer;
	ray* m_rayPtr;
	Buffer* m_isectBuffer;
	Intersection* m_isectPtr;
    Event* m_event;

	Task(IntersectionApi* api);

	void add(const std::function< Event* () >& fn);

    bool ready();

	bool process();

    void retry();

private:
    std::vector< std::function< Event* () > > m_fns;
    size_t m_count;
};

Task::Task(IntersectionApi* api)
:   m_api(api)
,   m_activeRayCount(0)
,   m_attenutate(1.0f)
,   m_rayBuffer(nullptr)
,   m_rayPtr(nullptr)
,   m_isectBuffer(nullptr)
,   m_isectPtr(nullptr)
,   m_event(nullptr)
,   m_count(0)
{
    m_fns.reserve(8);
}

bool Task::ready()
{
    if (m_event)
    {
        if (!m_event->Complete())
            return false;

        m_api->DeleteEvent(m_event);
        m_event = nullptr;
    }
    return true;
}

void Task::add(const std::function< Event* () >& fn)
{
    m_fns.push_back(fn);
}

bool Task::process()
{
    if (m_count < m_fns.size())
    {
	    auto fn = m_fns[m_count++];
	    m_event = fn();
        return true;
    }
    else
        return false;
}

void Task::retry()
{
    --m_count;
}


class Scheduler : public Object
{
public:
    Scheduler(size_t reserveTasks);

	void add(Task* task);

	void execute();

private:
	RefArray< Task > m_tasks;
};

Scheduler::Scheduler(size_t reserveTasks)
{
    m_tasks.reserve(reserveTasks);
}

void Scheduler::add(Task* task)
{
    m_tasks.push_back(task);
}

void Scheduler::execute()
{
    //log::info << L"Executing " << int32_t(m_tasks.size()) << L" tasks..." << Endl;

    Timer timer;
    timer.start();

	while (!m_tasks.empty())
	{
		for (auto it = m_tasks.begin(); it != m_tasks.end(); )
		{
			auto task = *it;
            
            // Check if task is waiting.
            if (!task->ready())
            {
                ++it;
                continue;
            }
            
            // Task is ready, process it.
            double Ts = timer.getElapsedTime();
            bool result = task->process();
            double Te = timer.getElapsedTime();

            if (Te - Ts > 0.004)
                log::warning << L"Task exceed 4 ms process (" << int32_t((Te - Ts) * 1000) << L" ms)." << Endl;

			if (result)
				++it;
			else
				it = m_tasks.erase(it);
		}
	}
}



const Scalar p(1.0f / (2.0f * PI));
const Scalar c_epsilonOffset(0.1f);

Scalar attenuation(const Scalar& distance, const Scalar& range)
{
	Scalar k0 = clamp(Scalar(1.0f) / (distance * distance), Scalar(0.0f), Scalar(1.0f));
	Scalar k1 = clamp(Scalar(1.0f) - (distance / range), Scalar(0.0f), Scalar(1.0f));
	return k0 * k1;
}

        }

T_IMPLEMENT_RTTI_CLASS(L"traktor.illuminate.RayTracerRadeonRays", RayTracerRadeonRays, IRayTracer)

RayTracerRadeonRays::RayTracerRadeonRays()
:   m_configuration(nullptr)
,   m_api(nullptr)
{
}

bool RayTracerRadeonRays::create(const IlluminateConfiguration* configuration)
{
    IntersectionApi::SetPlatform(DeviceInfo::kOpenCL);

    int32_t deviceIndex = -1;
    for (int32_t index = 0; index < IntersectionApi::GetDeviceCount(); ++index)
    {
        DeviceInfo deviceInfo;
        IntersectionApi::GetDeviceInfo(index, deviceInfo);
        if (deviceInfo.type == DeviceInfo::kGpu)
        {
            deviceIndex = index;
            break;
        }
    }
    if (deviceIndex <= -1)
    {
        log::error << L"Unable to find Radeon Rays device; no suitable device found." << Endl;
        return false;
    }

    m_api = IntersectionApi::Create(deviceIndex);
    if (!m_api)
    {
        log::error << L"Unable to create Radeon Rays device." << Endl;
        return false;
    }

    m_configuration = configuration;
    return true;
}

void RayTracerRadeonRays::destroy()
{
    if (m_api)
    {
        IntersectionApi::Delete(m_api);
        m_api = nullptr;
    }
}

void RayTracerRadeonRays::addLight(const Light& light)
{
    m_lights.push_back(light);
}

void RayTracerRadeonRays::addModel(const model::Model* model, const Transform& transform)
{
    // Build vertex buffer.
    AlignedVector< float > vertices;
    for (auto position : model->getPositions())
    {
        Vector4 p = transform * position.xyz1();
        vertices.push_back(p.x());
        vertices.push_back(p.y());
        vertices.push_back(p.z());
    }

    // Build face buffers.
    AlignedVector< int > faceIndices;
    AlignedVector< int > faceNumIndices;
    for (auto polygon : model->getPolygons())
    {
        T_FATAL_ASSERT(polygon.getVertexCount() == 3);

        for (auto vertex : polygon.getVertices())
            faceIndices.push_back(
                model->getVertex(vertex).getPosition()
            );

        faceNumIndices.push_back(polygon.getVertexCount());
    }

    // Upload mesh shape.
    Shape* shape = m_api->CreateMesh(
        vertices.c_ptr(),       // Vertices
        vertices.size() / 3,    // Number of vertices.
        3 * sizeof(float),      // Vertex size (stride).
        faceIndices.c_ptr(),    // Indices
        0,                      // Index size (stride).
        faceNumIndices.c_ptr(), // Face counts.
        faceNumIndices.size()   // Number of faces.
    );
    if (shape != nullptr)
        m_api->AttachShape(shape);
}

void RayTracerRadeonRays::commit()
{
    m_api->Commit();
}

Ref< drawing::Image > RayTracerRadeonRays::traceDirect(const GBuffer* gbuffer) const
{
    const int32_t shadowSampleCount = m_configuration->getShadowSampleCount();
    const float pointLightShadowRadius = m_configuration->getPointLightShadowRadius();

    int32_t width = gbuffer->getWidth();
    int32_t height = gbuffer->getHeight();

    RandomGeometry random;

    Ref< drawing::Image > lightmapDirect = new drawing::Image(drawing::PixelFormat::getRGBAF32(), width, height);
    lightmapDirect->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

    BufferPool rayBufferPool;
    BufferPool isectBufferPool;

    for (int32_t i = 0; i < c_bufferPoolSize; ++i)
    {
        Buffer* rayBuffer = m_api->CreateBuffer(shadowSampleCount * sizeof(ray), nullptr);
        Buffer* isectBuffer = m_api->CreateBuffer(shadowSampleCount * sizeof(Intersection), nullptr);
        if (!rayBuffer || !isectBuffer)
            return nullptr;

        rayBufferPool.release(rayBuffer);
        isectBufferPool.release(isectBuffer);
    }

    for (int32_t ty = 0; ty < height; ty += 16)
    {
        for (int32_t tx = 0; tx < width; tx += 16)
        {
            Scheduler scheduler(16 * 16 * m_lights.size());

            for (int32_t y = ty; y < ty + 16; ++y)
            {
                for (int32_t x = tx; x < tx + 16; ++x)
                {

                    for (const auto& light : m_lights)
                    {
                        if (light.type == Light::LtDirectional)
                        {
                            Ref< Task > task = new Task(m_api);

                            task->add([&, task]() -> Event* {
                                task->m_rayBuffer = rayBufferPool.acquire();
                                task->m_isectBuffer = isectBufferPool.acquire();
                                if (!task->m_rayBuffer || !task->m_isectBuffer)
                                    task->retry();
                                return nullptr;
                            });

                            task->add([&, task]() -> Event* {
                                Event* mapEvent = nullptr;
                                
                                m_api->MapBuffer(
                                    task->m_rayBuffer,
                                    kMapWrite,
                                    0,
                                    1 * sizeof(ray),
                                    (void**)&task->m_rayPtr,
                                    &mapEvent
                                );
                                
                                return mapEvent;
                            });

                            task->add([&, task, x, y, light]() -> Event* {
                                const auto& elm = gbuffer->get(x, y);
                                if (elm.polygon == model::c_InvalidIndex)
                                    return nullptr;

                                Scalar phi = dot3(elm.normal, -light.direction);
                                if (phi <= 0.0f)
                                    return nullptr;

                                Vector4 direction = -light.direction;
                                Vector4 origin = elm.position + direction * Scalar(0.01f);

                                ray* wr = task->m_rayPtr;
                                wr->o = float4(origin.x(), origin.y(), origin.z(), 1000.0f);
                                wr->d = float3(direction.x(), direction.y(), direction.z());
                                wr->SetActive(true);
                                wr->SetMask(~0U);
                                wr++;

                                task->m_activeRayCount = (int32_t)(wr - task->m_rayPtr);
                                task->m_attenutate = phi;
                                return nullptr;
                            });

                            task->add([&, task]() -> Event* {
                                Event* unmapEvent = nullptr;

                                m_api->UnmapBuffer(
                                    task->m_rayBuffer,
                                    task->m_rayPtr,
                                    &unmapEvent
                                );

                                return unmapEvent;
                            });

                            task->add([&, task]() -> Event* {
                                if (task->m_activeRayCount <= 0)
                                    return nullptr;

                                Event* queryEvent = nullptr;

                                m_api->QueryIntersection(
                                    task->m_rayBuffer,
                                    task->m_activeRayCount,
                                    task->m_isectBuffer,
                                    nullptr,
                                    &queryEvent
                                );

                                return queryEvent;
                            });

                            task->add([&, task]() -> Event* {
                                if (task->m_activeRayCount <= 0)
                                    return nullptr;

                                Event* mapEvent = nullptr;
                                
                                m_api->MapBuffer(
                                    task->m_isectBuffer,
                                    kMapRead,
                                    0,
                                    1 * sizeof(Intersection),
                                    (void**)&task->m_isectPtr,
                                    &mapEvent
                                );
                                
                                return mapEvent;
                            });

                            task->add([&, task, x, y, light]() -> Event* {
                                if (task->m_activeRayCount <= 0)
                                    return nullptr;

                                Intersection* ri = task->m_isectPtr;
                                if (ri->primid == kNullId)
                                {
                                    Color4f lm;
                                    lightmapDirect->getPixelUnsafe(x, y, lm);
                                    lightmapDirect->setPixelUnsafe(x, y, lm + light.color * task->m_attenutate);
                                }

                                return nullptr;                     
                            });

                            task->add([&, task]() -> Event* {
                                if (task->m_activeRayCount <= 0)
                                    return nullptr;

                                Event* unmapEvent = nullptr;

                                m_api->UnmapBuffer(
                                    task->m_isectBuffer,
                                    task->m_isectPtr,
                                    &unmapEvent
                                );

                                return unmapEvent;
                            });

                            task->add([&, task]() -> Event* {
                                rayBufferPool.release(task->m_rayBuffer);
                                isectBufferPool.release(task->m_isectBuffer);
                                return nullptr;
                            });

                            scheduler.add(task);
                        }
                        else if (light.type == Light::LtPoint)
                        {
                            Ref< Task > task = new Task(m_api);

                            task->add([&, task]() -> Event* {
                                task->m_rayBuffer = rayBufferPool.acquire();
                                task->m_isectBuffer = isectBufferPool.acquire();
                                if (!task->m_rayBuffer || !task->m_isectBuffer)
                                    task->retry();
                                return nullptr;
                            });

                            task->add([&, task]() -> Event* {
                                Event* mapEvent = nullptr;
                                
                                m_api->MapBuffer(
                                    task->m_rayBuffer,
                                    kMapWrite,
                                    0,
                                    shadowSampleCount * sizeof(ray),
                                    (void**)&task->m_rayPtr,
                                    &mapEvent
                                );
                                
                                return mapEvent;
                            });

                            task->add([&, task, x, y, light]() -> Event* {
                                const auto& elm = gbuffer->get(x, y);
                                if (elm.polygon == model::c_InvalidIndex)
                                    return nullptr;

                                Vector4 origin = elm.position;
                                Vector4 normal = elm.normal;

                                Vector4 lightDirection = (light.position - origin).xyz0();
                                Scalar lightDistance = lightDirection.normalize();
                                if (lightDistance > light.range)
                                    return nullptr;

                                Scalar phi = dot3(normal, lightDirection);
                                if (phi <= 0.0f)
                                    return nullptr;

                                Scalar f = attenuation(lightDistance, light.range);
                                if (f <= 0.0f)
                                    return nullptr;

                                Vector4 u, v;
                                orthogonalFrame(lightDirection, u, v);

                                ray* wr = task->m_rayPtr;
                                for (int32_t i = 0; i < shadowSampleCount; ++i)
                                {
                                    float a = 0.0f, b = 0.0f;
                                    if (shadowSampleCount > 1)
                                    {
                                        do
                                        {
                                            a = random.nextFloat() * 2.0f - 1.0f;
                                            b = random.nextFloat() * 2.0f - 1.0f;
                                        }
                                        while ((a * a) + (b * b) > 1.0f);
                                    }

                                    Vector4 shadowDirection = (light.position + u * Scalar(a * pointLightShadowRadius) + v * Scalar(b * pointLightShadowRadius) - origin).xyz0();

                                    Vector4 O = origin + normal * Scalar(0.01f);
                                    Vector4 D = shadowDirection.normalized();

                                    wr->o = float4(O.x(), O.y(), O.z(), lightDistance - 0.01f);
                                    wr->d = float3(D.x(), D.y(), D.z());
                                    wr->SetActive(true);
                                    wr->SetMask(~0U);
                                    wr++;
                                }

                                task->m_activeRayCount = (int32_t)(wr - task->m_rayPtr);
                                task->m_attenutate = phi * min(f, Scalar(1.0f));
                                return nullptr;
                            });

                            task->add([&, task]() -> Event* {
                                Event* unmapEvent = nullptr;

                                m_api->UnmapBuffer(
                                    task->m_rayBuffer,
                                    task->m_rayPtr,
                                    &unmapEvent
                                );

                                return unmapEvent;
                            });

                            task->add([&, task]() -> Event* {
                                if (task->m_activeRayCount <= 0)
                                    return nullptr;

                                Event* queryEvent = nullptr;

                                m_api->QueryIntersection(
                                    task->m_rayBuffer,
                                    task->m_activeRayCount,
                                    task->m_isectBuffer,
                                    nullptr,
                                    &queryEvent
                                );

                                return queryEvent;
                            });

                            task->add([&, task]() -> Event* {
                                if (task->m_activeRayCount <= 0)
                                    return nullptr;

                                Event* mapEvent = nullptr;
                                
                                m_api->MapBuffer(
                                    task->m_isectBuffer,
                                    kMapRead,
                                    0,
                                    shadowSampleCount * sizeof(Intersection),
                                    (void**)&task->m_isectPtr,
                                    &mapEvent
                                );
                                
                                return mapEvent;
                            });

                            task->add([&, task, x, y, light]() -> Event* {
                                if (task->m_activeRayCount <= 0)
                                    return nullptr;

                                int32_t shadowCount = 0;

                                Intersection* ri = task->m_isectPtr;
                                for (int32_t i = 0; i < shadowSampleCount; ++i)
                                {
                                    if (ri->primid != kNullId)
                                        shadowCount++;
                                    ++ri;
                                }

                                Scalar shadowAttenuate(1.0f - float(shadowCount) / shadowSampleCount);

                                Color4f lm;
                                lightmapDirect->getPixelUnsafe(x, y, lm);
                                lightmapDirect->setPixelUnsafe(x, y, lm + light.color * task->m_attenutate * shadowAttenuate);

                                return nullptr;                     
                            });

                            task->add([&, task]() -> Event* {
                                if (task->m_activeRayCount <= 0)
                                    return nullptr;

                                Event* unmapEvent = nullptr;

                                m_api->UnmapBuffer(
                                    task->m_isectBuffer,
                                    task->m_isectPtr,
                                    &unmapEvent
                                );

                                return unmapEvent;
                            });

                            task->add([&, task]() -> Event* {
                                rayBufferPool.release(task->m_rayBuffer);
                                isectBufferPool.release(task->m_isectBuffer);
                                return nullptr;
                            });

                            scheduler.add(task);
                        }
                    }
                }
            }

            scheduler.execute();

        }
    }


    return lightmapDirect;
}

Ref< drawing::Image > RayTracerRadeonRays::traceIndirect(const GBuffer* gbuffer) const
{
    return nullptr;
}

    }
}