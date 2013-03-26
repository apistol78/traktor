#include "Core/Functor/Functor.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/JobManager.h"
#include "Render/Shader.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterInstance.h"
#include "Spray/Types.h"
#include "Spray/Modifier.h"
#include "Spray/PointRenderer.h"
#include "Spray/Source.h"

#if defined(T_MODIFIER_USE_PS3_SPURS)
#	include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#	include "Spray/Ps3/SprayJobQueue.h"
#	include "Spray/Ps3/Spu/JobModifierUpdate.h"
#endif

//#if !TARGET_OS_IPHONE && !defined(_WINCE)
//#	define T_USE_UPDATE_JOBS
//#endif

namespace traktor
{
	namespace spray
	{
		namespace
		{

const float c_warmUpDeltaTime = 1.0f / 5.0f;
#if TARGET_OS_IPHONE
const uint32_t c_maxEmitPerUpdate = 4;
const uint32_t c_maxEmitSingleShot = 10;
#elif defined(_PS3)
const uint32_t c_maxEmitPerUpdate = 6;
const uint32_t c_maxEmitSingleShot = 2000;
#else
const uint32_t c_maxEmitPerUpdate = 16;
const uint32_t c_maxEmitSingleShot = 3000;
#endif

struct PointPredicate
{
	const Plane& m_cameraPlane;

	PointPredicate(const Plane& cameraPlane)
	:	m_cameraPlane(cameraPlane)
	{
	}

	bool operator () (const Point& pl, const Point& pr) const
	{
		return m_cameraPlane.distance(pl.position) > m_cameraPlane.distance(pr.position);
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EmitterInstance", EmitterInstance, Object)

EmitterInstance::EmitterInstance(const Emitter* emitter, float duration)
:	m_emitter(emitter)
//,	m_emitted(0)
,	m_totalTime(0.0f)
,	m_emitFraction(0.0f)
,	m_warm(false)
,	m_count(std::rand() & 15)
{
	
	int32_t pointCount = int32_t(c_maxEmitPerUpdate * 60 * duration);
	if (duration < 0.01f)
		pointCount = c_maxEmitSingleShot;

	m_points.reserve(pointCount);
	m_renderPoints[0].reserve(pointCount);
	m_renderPoints[1].reserve(pointCount);
	m_renderPoints[2].reserve(pointCount);
	m_renderPoints[3].reserve(pointCount);
}

EmitterInstance::~EmitterInstance()
{
	synchronize();
}

void EmitterInstance::update(Context& context, const Transform& transform, bool emit, bool singleShot)
{
	Transform T = m_emitter->worldSpace() ? transform : Transform::identity();

	// Warm up instance.
	if (!m_warm)
	{
		m_warm = true;
		m_position = T.translation();

		if (m_emitter->getWarmUp() >= FUZZY_EPSILON)
		{
			Context warmContext;
			warmContext.deltaTime = c_warmUpDeltaTime;
			warmContext.soundPlayer = 0;

			float time = 0.0f;
			for (; time < m_emitter->getWarmUp(); time += c_warmUpDeltaTime)
				update(warmContext, transform, true, false);

			warmContext.deltaTime = m_emitter->getWarmUp() - time;
			if (warmContext.deltaTime >= FUZZY_EPSILON)
				update(warmContext, transform, true, false);
		}
	}

	//synchronize();

	// Erase dead particles.
	size_t size = m_points.size();
	for (size_t i = 0; i < size; )
	{
		Point& point = m_points[i];
		if ((point.age += context.deltaTime) < point.maxAge)
			++i;
		else if (i < --size)
			point = m_points[size];
	}
	m_points.resize(size);

	// Emit particles.
	if (emit)
	{
		const Source* source = m_emitter->getSource();
		if (source)
		{
			uint32_t avail = m_points.capacity() - size;
			Vector4 dm = T.translation() - m_position;

			if (!singleShot)
			{
				float emitVelocity = context.deltaTime > FUZZY_EPSILON ? source->getVelocityRate() * (dm.length() / context.deltaTime) : 0.0f;
				float emitConstant = source->getConstantRate() * context.deltaTime;
				float emit = emitVelocity + emitConstant + m_emitFraction;
				uint32_t emitCountFrame = uint32_t(emit + 0.5f);

				// Emit in multiple frames; estimate number of particles to emit.
				if (emitCountFrame > 0)
				{
					uint32_t emitCount = min< uint32_t >(emitCountFrame, avail, c_maxEmitPerUpdate);
					if (emitCount > 0)
					{
						source->emit(
							context,
							T,
							dm,
							emitCount,
							*this
						);
					}
				}

				// Preserve fraction of non-emitted particles.
				m_emitFraction = emit - emitCountFrame;
			}
			else
			{
				// Single shot emit; emit all particles in one frame and then no more.
				uint32_t emitCount = min< uint32_t >(uint32_t(source->getConstantRate()), avail, c_maxEmitSingleShot);
				if (emitCount > 0)
				{
					source->emit(
						context,
						T,
						dm,
						emitCount,
						*this
					);
				}
			}
		}
	}

	// Save current position as we need it to calculate velocity next update.
	m_position = T.translation();
	m_totalTime += context.deltaTime;

	// Calculate bounding box; do this before modifiers as modifiers are executed
	// asynchronously.
	if ((m_count & 15) == 0)
	{
		m_boundingBox = Aabb3();
		Scalar deltaTime16 = Scalar(context.deltaTime * 16.0f);
		for (PointVector::iterator i = m_points.begin(); i != m_points.end(); ++i)
		{
			m_boundingBox.contain(i->position);
			m_boundingBox.contain(i->position + i->velocity * deltaTime16);
		}
		m_boundingBox = m_boundingBox.expand(Scalar(1.0f));
		if (!m_emitter->worldSpace())
			m_boundingBox = m_boundingBox.transform(transform);
	}

#if defined(T_MODIFIER_USE_PS3_SPURS)
	//
	// Update particles on SPU
	//
	if (!m_points.empty())
	{
		SpursJobQueue* jobQueue = SprayJobQueue::getInstance().getJobQueue();
		T_ASSERT (jobQueue);

		const RefArray< Modifier >& modifiers = m_emitter->getModifiers();
		for (RefArray< Modifier >::const_iterator i = modifiers.begin(); i != modifiers.end(); ++i)
		{
			if (*i)
				(*i)->update(
					jobQueue,
					Scalar(context.deltaTime),
					T,
					m_points
				);
		}
	}
#else
	//
	// Update particles on CPU
	//
	size = m_points.size();
#	if defined(T_USE_UPDATE_JOBS)
	// Execute modifiers.
	if (size >= 16)
	{
		JobManager& jobManager = JobManager::getInstance();
		m_job = jobManager.add(makeFunctor< EmitterInstance, float, const Transform&, size_t >(
			this,
			&EmitterInstance::updateTask,
			context.deltaTime,
			T,
			size
		));
	}
	else
		updateTask(context.deltaTime, T, size);
#	else
	updateTask(context.deltaTime, T, size);
#	endif
#endif

	// \fixme Should transform into render points as a "tail" job.
	synchronize();

	PointVector& renderPoints = m_renderPoints[m_count & 3];
	renderPoints = m_points;

	if (!m_emitter->worldSpace())
	{
		for (uint32_t i = 0; i < renderPoints.size(); ++i)
		{
			renderPoints[i].position = transform * renderPoints[i].position.xyz1();
			renderPoints[i].velocity = transform * renderPoints[i].velocity.xyz0();
		}
	}

	m_count++;
}

void EmitterInstance::render(PointRenderer* pointRenderer, const Transform& transform, const Plane& cameraPlane)
{
	T_ASSERT (m_count > 0);

	PointVector& renderPoints = m_renderPoints[(m_count - 1) & 3];

	if (renderPoints.empty())
		return;

	if (m_emitter->getSort())
		std::sort(renderPoints.begin(), renderPoints.end(), PointPredicate(cameraPlane));

	pointRenderer->render(
		m_emitter->getShader(),
		cameraPlane,
		renderPoints,
		m_emitter->getMiddleAge(),
		m_emitter->getCullNearDistance(),
		m_emitter->getFadeNearRange()
	);
}

void EmitterInstance::synchronize() const
{
#if defined(T_MODIFIER_USE_PS3_SPURS)

	SpursJobQueue* jobQueue = SprayJobQueue::getInstance().getJobQueue();
	T_ASSERT (jobQueue);

	jobQueue->wait();

#else

#	if defined(T_USE_UPDATE_JOBS)
	if (m_job)
	{
		m_job->wait();
		m_job = 0;
	}
#	endif

#endif
}

#if !defined(T_MODIFIER_USE_PS3_SPURS)
void EmitterInstance::updateTask(float deltaTime, const Transform& transform, size_t last)
{
	Scalar deltaTimeScalar(deltaTime);
	const RefArray< Modifier >& modifiers = m_emitter->getModifiers();
	for (RefArray< Modifier >::const_iterator i = modifiers.begin(); i != modifiers.end(); ++i)
	{
		if (*i)
			(*i)->update(deltaTimeScalar, transform, m_points, 0, last);
	}
}
#endif

	}
}
