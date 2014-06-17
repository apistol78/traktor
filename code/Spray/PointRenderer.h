#ifndef traktor_spray_PointRenderer_H
#define traktor_spray_PointRenderer_H

#include "Core/Object.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Vector2.h"
#include "Core/Containers/AlignedVector.h"
#include "Spray/Point.h"
#include "Render/Shader.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
#if defined(_PS3)
class SpursJobQueue;
#endif

	namespace render
	{

class IRenderSystem;
class RenderContext;
class VertexBuffer;
class IndexBuffer;

	}

	namespace world
	{

class IWorldRenderPass;

	}

	namespace spray
	{

struct EmitterVertex;

/*! \brief Particle renderer.
 * \ingroup Spray
 */
class T_DLLCLASS PointRenderer : public Object
{
	T_RTTI_CLASS;

public:
	PointRenderer(render::IRenderSystem* renderSystem, float lod1Distance, float lod2Distance);

	virtual ~PointRenderer();

	void destroy();

	void render(
		render::Shader* shader,
		const Plane& cameraPlane,
		const PointVector& points,
		float middleAge,
		float cullNearDistance,
		float fadeNearRange,
		float cameraOffset
	);

	void flush(
		render::RenderContext* renderContext,
		world::IWorldRenderPass& worldRenderPass
	);

	void setLodDistances(float lod1Distance, float lod2Distance)
	{
		m_lod1Distance = lod1Distance;
		m_lod2Distance = lod2Distance;
	}

	float getLod1Distance() const { return m_lod1Distance; }

	float getLod2Distance() const { return m_lod2Distance; }

private:
#pragma pack(1)
	struct Batch
	{
		uint32_t count;
		float distance;
		render::Shader* shader;
		uint32_t offset;

		Batch()
		:	count(0)
		,	distance(0.0f)
		,	shader(0)
		,	offset(0)
		{
		}
	};
#pragma pack()

	Ref< render::VertexBuffer > m_vertexBuffers[8];
	Ref< render::IndexBuffer > m_indexBuffer;
	float m_lod1Distance;
	float m_lod2Distance;
	int32_t m_count;
	EmitterVertex* m_vertexTop;
	EmitterVertex* m_vertex;
	int32_t m_pointOffset;
	AlignedVector< Batch > m_batches;

#if defined(_PS3)
	Ref< SpursJobQueue > m_jobQueue;
#endif
};

	}
}

#endif	// traktor_spray_PointRenderer_H
