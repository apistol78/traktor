#ifndef traktor_render_TimeQueryOpenGL_H
#define traktor_render_TimeQueryOpenGL_H

#include "Render/ITimeQuery.h"
#include "Render/OpenGL/Platform.h"

namespace traktor
{
	namespace render
	{

/*! \brief GPU time stamp recording query.
 * \ingroup Render
 */
class TimeQueryOpenGL : public ITimeQuery
{
	T_RTTI_CLASS;

public:
	static Ref< TimeQueryOpenGL > create();

	virtual void begin();

	virtual int32_t stamp();

	virtual void end();	

	virtual bool ready() const;

	virtual uint64_t get(int32_t index) const;

private:
	enum
	{
		MaxTimeQueries = 2000
	};

	GLuint m_query[MaxTimeQueries];
	int32_t m_last;
	int32_t m_current;

	TimeQueryOpenGL();
};

	}
}

#endif	// traktor_render_TimeQueryOpenGL_H
