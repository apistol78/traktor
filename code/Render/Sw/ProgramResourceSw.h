#ifndef traktor_render_ProgramResourceSw_H
#define traktor_render_ProgramResourceSw_H

#include "Core/Heap/Ref.h"
#include "Render/ProgramResource.h"

namespace traktor
{
	namespace render
	{

class ShaderGraph;

/*!
 * \ingroup SW
 */
class ProgramResourceSw : public ProgramResource
{
	T_RTTI_CLASS(ProgramResourceSw)

public:
	ProgramResourceSw(const ShaderGraph* shaderGraph = 0);

	Ref< const ShaderGraph > getShaderGraph() const { return m_shaderGraph; }

	virtual bool serialize(Serializer& s);

private:
	Ref< ShaderGraph > m_shaderGraph;
};

	}
}

#endif	// traktor_render_ProgramResourceSw_H
