/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_TimeQueryOpenGL_H
#define traktor_render_TimeQueryOpenGL_H

#include "Core/Ref.h"
#include "Render/ITimeQuery.h"
#include "Render/OpenGL/Std/Platform.h"

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

	virtual void begin() override final;

	virtual int32_t stamp() override final;

	virtual void end() override final;	

	virtual bool ready() const override final;

	virtual uint64_t get(int32_t index) const override final;

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
