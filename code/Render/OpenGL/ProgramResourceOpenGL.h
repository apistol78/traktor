#ifndef traktor_render_ProgramResourceOpenGL_H
#define traktor_render_ProgramResourceOpenGL_H

#include "Core/Heap/Ref.h"
#include "Render/ProgramResource.h"

namespace traktor
{
	namespace render
	{

class ShaderGraph;

/*!
 * \ingroup OGL
 */
class ProgramResourceOpenGL : public ProgramResource
{
	T_RTTI_CLASS(ProgramResourceOpenGL)

public:
	ProgramResourceOpenGL(const ShaderGraph* shaderGraph = 0);

	const ShaderGraph* getShaderGraph() const { return m_shaderGraph; }

	virtual bool serialize(Serializer& s);

private:
	Ref< ShaderGraph > m_shaderGraph;
};

	}
}

#endif	// traktor_render_ProgramResourceOpenGL_H
