#include "Spray/EmitterInstance.h"
#include "Spray/Emitter.h"
#include "Spray/Source.h"
#include "Spray/Modifier.h"
#include "Spray/PointRenderer.h"
#include "Spray/EmitterUpdateContext.h"
#include "Render/Shader.h"

#define T_USE_UPDATE_JOBS

namespace traktor
{
	namespace spray
	{
		namespace
		{

const float c_warmUpDeltaTime = 1.0f / 10.0f;
const int c_maxEmitPerUpdate = 8;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EmitterInstance", EmitterInstance, Object)

EmitterInstance::EmitterInstance(Emitter* emitter)
:	m_emitter(emitter)
,	m_totalTime(0.0f)
,	m_emitted(0)
,	m_warm(false)
{
}

EmitterInstance::~EmitterInstance()
{
	synchronize();
}

void EmitterInstance::update(EmitterUpdateContext& context, const Transform& transform, bool emit, bool singleShot)
{
	// Warm up instance.
	if (!m_warm)
	{
		m_warm = true;
		if (m_emitter->getWarmUp() >= FUZZY_EPSILON)
		{
			EmitterUpdateContext warmContext(c_warmUpDeltaTime);

			float time = 0.0f;
			for (; time < m_emitter->getWarmUp(); time += c_warmUpDeltaTime)
				update(warmContext, transform, true, false);

			warmContext.deltaTime = m_emitter->getWarmUp() - time;
			if (warmContext.deltaTime >= FUZZY_EPSILON)
				update(warmContext, transform, true, false);
		}
	}

	synchronize();

	// Erase dead particles.
	for (PointVector::iterator i = m_points.begin(); i != m_points.end(); )
	{
		if ((i->age += context.deltaTime) < i->maxAge)
			++i;
		else
			i = m_points.erase(i);
	}

	m_totalTime += context.deltaTime;

	// Emit particles.
	if (emit)
	{
		Source* source = m_emitter->getSource();
		if (source)
		{
			if (!singleShot)
			{
				// Emit in multiple frames; estimate number of particles to emit.
				uint32_t goal = uint32_t(m_totalTime * source->getRate());
				uint32_t emitCount = std::min< uint32_t >(goal - m_emitted, c_maxEmitPerUpdate);
				if (emitCount > 0)
					source->emit(context, transform, emitCount, *this);
			}
			else
			{
				// Single shot emit; emit all particles in one frame and then no more.
				source->emit(
					context,
					transform,
					uint32_t(source->getRate()),
					*this
				);
			}
		}
	}

	// Calculate bounding box; do this before modifiers as modifiers are executed
	// asynchronously.
	m_boundingBox = Aabb();
	for (PointVector::iterator i = m_points.begin(); i != m_points.end(); ++i)
		m_boundingBox.contain(i->position);

#if defined(T_USE_UPDATE_JOBS)
	// Execute modifiers.
	size_t size = m_points.size();
	if (size >= 4)
	{
		size_t pivots[] =
		{
			0,
			size / 4,
			(size * 2) / 4,
			(size * 3) / 4,
			size
		};

		m_jobs[0] = makeFunctor< EmitterInstance, float, const Transform&, size_t, size_t >(this, &EmitterInstance::updateTask, context.deltaTime, cref(transform), pivots[0], pivots[1]);
		m_jobs[1] = makeFunctor< EmitterInstance, float, const Transform&, size_t, size_t >(this, &EmitterInstance::updateTask, context.deltaTime, cref(transform), pivots[1], pivots[2]);
		m_jobs[2] = makeFunctor< EmitterInstance, float, const Transform&, size_t, size_t >(this, &EmitterInstance::updateTask, context.deltaTime, cref(transform), pivots[2], pivots[3]);
		m_jobs[3] = makeFunctor< EmitterInstance, float, const Transform&, size_t, size_t >(this, &EmitterInstance::updateTask, context.deltaTime, cref(transform), pivots[3], pivots[4]);

		JobManager& jobManager = JobManager::getInstance();
		jobManager.add(m_jobs[0]);
		jobManager.add(m_jobs[1]);
		jobManager.add(m_jobs[2]);
		jobManager.add(m_jobs[3]);
	}
#else
	updateTask(context.deltaTime, transform, 0, m_points.size());
#endif
}

void EmitterInstance::render(PointRenderer* pointRenderer, const Plane& cameraPlane) const
{
	synchronize();

	if (m_points.empty())
		return;

	resource::Proxy< render::Shader >& shader = m_emitter->getShader();
	if (!shader.validate())
		return;

	pointRenderer->render(
		shader,
		cameraPlane,
		m_points,
		m_emitter->getMiddleAge()
	);
}

void EmitterInstance::synchronize() const
{
#if defined(T_USE_UPDATE_JOBS)
	m_jobs[0].wait(); m_jobs[0] = 0;
	m_jobs[1].wait(); m_jobs[1] = 0;
	m_jobs[2].wait(); m_jobs[2] = 0;
	m_jobs[3].wait(); m_jobs[3] = 0;
#endif
}

void EmitterInstance::updateTask(float deltaTime, const Transform& transform, size_t first, size_t last)
{
	Scalar deltaTimeScalar(deltaTime);
	const RefArray< Modifier >& modifiers = m_emitter->getModifiers();
	for (RefArray< Modifier >::const_iterator i = modifiers.begin(); i != modifiers.end(); ++i)
	{
		if (*i)
			(*i)->update(deltaTimeScalar, transform, m_points, first, last);
	}
}

	}
}
