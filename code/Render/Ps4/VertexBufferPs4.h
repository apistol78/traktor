#ifndef traktor_render_VertexBufferPs4_H
#define traktor_render_VertexBufferPs4_H

#include "Render/VertexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup GNM
 */
class VertexBufferPs4 : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	virtual void prepare();

protected:
	VertexBufferPs4(uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_VertexBufferPs4_H
