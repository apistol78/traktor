/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IndexBufferIAR_H
#define traktor_render_IndexBufferIAR_H

#include "Render/OpenGL/IndexBufferOpenGL.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGL;

/*!
 * \ingroup OGL
 */
class IndexBufferIAR : public IndexBufferOpenGL
{
	T_RTTI_CLASS;

public:
	IndexBufferIAR(ContextOpenGL* resourceContext, IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferIAR();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;
	
	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void bind() T_OVERRIDE T_FINAL;

	virtual const GLvoid* getIndexData() const T_OVERRIDE T_FINAL;
	
private:
	Ref< ContextOpenGL > m_resourceContext;
	GLubyte* m_data;
};
	
	}
}

#endif	// traktor_render_IndexBufferIAR_H
