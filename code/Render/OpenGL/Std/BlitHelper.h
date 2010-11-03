#ifndef traktor_render_BlitHelper_H
#define traktor_render_BlitHelper_H

#include "Core/Object.h"
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

class BlitHelper : public Object
{
	T_RTTI_CLASS;
	
public:
	bool create();
	
	void destroy();

	void blit(GLint sourceTextureHandle);
	
private:
	GLhandleARB m_programObject;
};
	
	}
}

#endif	// traktor_render_BlitHelper_H
