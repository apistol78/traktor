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

class WorldRenderView;

	}

	namespace spray
	{

struct Vertex;

/*! \brief Particle renderer.
 * \ingroup Spray
 */
class T_DLLCLASS PointRenderer : public Object
{
	T_RTTI_CLASS;

public:
	PointRenderer(render::IRenderSystem* renderSystem);

	virtual ~PointRenderer();

	void destroy();

	void render(
		render::Shader* shader,
		const Plane& cameraPlane,
		const PointVector& points,
		float middleAge,
		float cullNearDistance,
		float fadeNearRange
	);

	void flush(
		render::RenderContext* renderContext,
		world::WorldRenderView* worldRenderView
	);

private:
	enum { BufferCount = 4 };

#pragma pack(1)
	struct Batch
	{
		uint32_t count;
		float distance;
		render::Shader* shader;
		uint32_t offset;
	};
#pragma pack()

	Ref< render::VertexBuffer > m_vertexBuffer[BufferCount];
	Ref< render::IndexBuffer > m_indexBuffer;
	uint32_t m_currentBuffer;
	Vertex* m_vertex;
	uint32_t m_vertexOffset;
	AlignedVector< Batch > m_batches[BufferCount];

#if defined(_PS3)
	Ref< SpursJobQueue > m_jobQueue;
#endif
};

	}
}

#endif	// traktor_spray_PointRenderer_H
