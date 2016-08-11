#ifndef traktor_render_ProgramResourceVk_H
#define traktor_render_ProgramResourceVk_H

#include "Render/Resource/ProgramResource.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class ProgramResourceVk : public ProgramResource
{
	T_RTTI_CLASS;

public:
	ProgramResourceVk();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class ProgramVk;
	friend class ProgramCompilerVk;

	std::vector< uint32_t > m_vertexShader;
	std::vector< uint32_t > m_fragmentShader;
};

	}
}

#endif	// traktor_render_ProgramResourceVk_H
