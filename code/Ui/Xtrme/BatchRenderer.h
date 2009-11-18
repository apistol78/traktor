#ifndef traktor_ui_xtrme_BatchRenderer_H
#define traktor_ui_xtrme_BatchRenderer_H

#include "Core/Object.h"
#include "Render/Types.h"
#include "Ui/Size.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class IRenderView;
class IProgram;
class VertexBuffer;
class ITexture;

	}

	namespace ui
	{
		namespace xtrme
		{

struct ScreenVertex;

class BatchRenderer : public Object
{
	T_RTTI_CLASS;

public:
	enum ProgramId
	{
		PiDefault	= 0,
		PiImage		= 1,
		PiFont		= 2
	};

	BatchRenderer(render::IRenderSystem* renderSystem, render::IRenderView* renderView, uint32_t bufferCount);

	virtual ~BatchRenderer();

	void batch(ProgramId programId, render::ITexture* texture, render::PrimitiveType primitiveType);

	void add(int x, int y, uint32_t color);

	void add(int x, int y, float u, float v, uint32_t color);

	void begin();

	void end(const Size& size);

private:
	struct Batch
	{
		Ref< render::IProgram > program;
		Ref< render::ITexture > texture;
		std::vector< render::Primitives > primitives;
	};

	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	Ref< render::IProgram > m_programs[3];
	Ref< render::VertexBuffer > m_vertexBuffer;
	ScreenVertex* m_vertexStart;
	ScreenVertex* m_vertex;
	std::vector< Batch > m_batches;
};

		}
	}
}

#endif	// traktor_ui_xtrme_BatchRenderer_H
