#include "Render/OpenGL/Std/TimeQueryOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TimeQueryOpenGL", TimeQueryOpenGL, ITimeQuery)

Ref< TimeQueryOpenGL > TimeQueryOpenGL::create()
{
	Ref< TimeQueryOpenGL > tq = new TimeQueryOpenGL();
	return tq;
}

void TimeQueryOpenGL::begin()
{
	if (m_current < 0)
	{
		T_OGL_SAFE(glGenQueries(MaxTimeQueries, m_query));
		m_current = 0;
	}
}

int32_t TimeQueryOpenGL::stamp()
{
	int32_t index = m_current;
	T_OGL_SAFE(glQueryCounter(m_query[index], GL_TIMESTAMP));
	m_last = index;
	m_current = (m_current + 1) % sizeof_array(m_query);
	return index;
}

void TimeQueryOpenGL::end()
{
}

bool TimeQueryOpenGL::ready() const
{
	GLint avail = 0;
	T_OGL_SAFE(glGetQueryObjectiv(m_query[m_last], GL_QUERY_RESULT_AVAILABLE, &avail));
	return bool(avail != 0);
}

uint64_t TimeQueryOpenGL::get(int32_t index) const
{
	GLint avail = 0;
	while (!avail)
	{
		T_OGL_SAFE(glGetQueryObjectiv(m_query[index], GL_QUERY_RESULT_AVAILABLE, &avail));
	}

	GLuint64 result = 0;
	T_OGL_SAFE(glGetQueryObjectui64v(m_query[index], GL_QUERY_RESULT, &result));

	return uint64_t(result / 1000);
}

TimeQueryOpenGL::TimeQueryOpenGL()
:	m_last(0)
,	m_current(-1)
{
	for (uint32_t i = 0; i < sizeof_array(m_query); ++i)
		m_query[i] = 0;
}

	}
}
