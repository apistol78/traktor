#ifndef traktor_ui_xtrme_BatchRenderer_H
#define traktor_ui_xtrme_BatchRenderer_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Render/Types.h"
#include "Ui/Size.h"

namespace traktor
{
	namespace render
	{

class RenderSystem;
class RenderView;
class Program;
class VertexBuffer;
class Texture;

	}

	namespace ui
	{
		namespace xtrme
		{

struct ScreenVertex;

class BatchRenderer : public Object
{
	T_RTTI_CLASS(BatchRenderer)

public:
	enum ProgramId
	{
		PiDefault	= 0,
		PiImage		= 1,
		PiFont		= 2
	};

	BatchRenderer(render::RenderSystem* renderSystem, render::RenderView* renderView, uint32_t bufferCount);

	virtual ~BatchRenderer();

	void batch(ProgramId programId, render::Texture* texture, render::PrimitiveType primitiveType);

	void add(int x, int y, uint32_t color);

	void add(int x, int y, float u, float v, uint32_t color);

	void begin();

	void end(const Size& size);

private:
	struct Batch
	{
		Ref< render::Program > program;
		Ref< render::Texture > texture;
		std::vector< render::Primitives > primitives;
	};

	Ref< render::RenderSystem > m_renderSystem;
	Ref< render::RenderView > m_renderView;
	Ref< render::Program > m_programs[3];
	Ref< render::VertexBuffer > m_vertexBuffer;
	ScreenVertex* m_vertexStart;
	ScreenVertex* m_vertex;
	std::vector< Batch > m_batches;
};

		}
	}
}

#endif	// traktor_ui_xtrme_BatchRenderer_H
