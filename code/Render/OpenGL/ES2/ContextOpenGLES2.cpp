#include "Render/OpenGL/ES2/ContextOpenGLES2.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ContextOpenGLES2", ContextOpenGLES2, IContext)

bool ContextOpenGLES2::enter()
{
	return true;
}

void ContextOpenGLES2::leave()
{
}

void ContextOpenGLES2::deleteResource(IDeleteCallback* callback)
{
	m_deleteResources.push_back(callback);
}

void ContextOpenGLES2::deleteResources()
{
	for (std::vector< IDeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
		(*i)->deleteResource();
	m_deleteResources.resize(0);
}

	}
}
