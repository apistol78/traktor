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
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
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
	PointRenderer(render::IRenderSystem* renderSystem, float cullNearDistance, float fadeNearRange);

	virtual ~PointRenderer();

	void destroy();

	void render(
		render::Shader* shader,
		const Plane& cameraPlane,
		const PointVector& points,
		float middleAge
	);

	void flush(
		render::RenderContext* renderContext,
		world::WorldRenderView* worldRenderView
	);

private:
	enum { BufferCount = 4 };

	struct Batch
	{
		render::Shader* shader;
		uint32_t offset;
		uint32_t count;
		float distance;
	};

	float m_cullNearDistance;
	float m_fadeNearRange;
	Ref< render::VertexBuffer > m_vertexBuffer[BufferCount];
	Ref< render::IndexBuffer > m_indexBuffer;
	uint32_t m_currentBuffer;
	Vertex* m_vertex;
	uint32_t m_vertexOffset;
	AlignedVector< Batch > m_batches[BufferCount];
};

	}
}

#endif	// traktor_spray_PointRenderer_H
