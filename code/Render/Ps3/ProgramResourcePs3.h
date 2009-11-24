#ifndef traktor_render_ProgramResourceDx10_H
#define traktor_render_ProgramResourceDx10_H

#include "Render/ProgramResource.h"
#include "Render/Ps3/PlatformPs3.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup PS3
 */
class ProgramResourcePs3 : public ProgramResource
{
	T_RTTI_CLASS;

public:
	ProgramResourcePs3();

	ProgramResourcePs3(
		CGCbin* vertexShaderBin,
		CGCbin* pixelShaderBin
	);

	virtual ~ProgramResourcePs3();

	virtual bool serialize(ISerializer& s);

	CGCbin* getVertexShaderBin() const {
		return m_vertexShaderBin;
	}

	CGCbin* getPixelShaderBin() const {
		return m_pixelShaderBin;
	}

private:
	CGCbin* m_vertexShaderBin;
	CGCbin* m_pixelShaderBin;
};

	}
}

#endif	// traktor_render_ProgramResourceDx10_H
