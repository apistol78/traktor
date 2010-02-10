#ifndef traktor_render_ProgramResourceSw_H
#define traktor_render_ProgramResourceSw_H

#include "Render/Resource/ProgramResource.h"

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
	T_RTTI_CLASS;

public:
	ProgramResourceSw(const ShaderGraph* shaderGraph = 0);

	Ref< const ShaderGraph > getShaderGraph() const { return m_shaderGraph; }

	virtual bool serialize(ISerializer& s);

private:
	Ref< ShaderGraph > m_shaderGraph;
};

	}
}

#endif	// traktor_render_ProgramResourceSw_H
