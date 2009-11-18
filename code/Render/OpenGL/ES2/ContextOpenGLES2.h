#ifndef traktor_render_ContextOpenGLES2_H
#define traktor_render_ContextOpenGLES2_H

#include <map>
#include "Core/Object.h"
#include "Core/Thread/ThreadLocal.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/IContext.h"

namespace traktor
{
	namespace render
	{

/*! \brief OpenGL ES2 context.
 * \ingroup OGL
 */
class ContextOpenGLES2 : public IContext
{
	T_RTTI_CLASS;

public:
	virtual bool enter();

	virtual void leave();

	virtual void deleteResource(IDeleteCallback* callback);

	virtual void deleteResources();

private:
	std::vector< IDeleteCallback* > m_deleteResources;
};

	}
}

#endif	// traktor_render_ContextOpenGLES2_H
