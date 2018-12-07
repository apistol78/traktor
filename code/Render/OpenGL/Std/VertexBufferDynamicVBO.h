/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VertexBufferDynamicVBO_H
#define traktor_render_VertexBufferDynamicVBO_H

#include "Core/Containers/AlignedVector.h"
#include "Render/OpenGL/Std/VertexBufferOpenGL.h"

namespace traktor
{
	namespace render
	{

class ResourceContextOpenGL;
class VertexElement;

/*!
 * \ingroup OGL
 */
class VertexBufferDynamicVBO : public VertexBufferOpenGL
{
	T_RTTI_CLASS;

public:
	VertexBufferDynamicVBO(ResourceContextOpenGL* resourceContext, const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize);

	virtual ~VertexBufferDynamicVBO();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) T_OVERRIDE T_FINAL;

	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void activate(RenderContextOpenGL* renderContext, const GLint* attributeLocs, uint32_t attributeHash) T_OVERRIDE T_FINAL;

private:
	struct AttributeDesc
	{
		GLint size;
		GLenum type;
		GLboolean normalized;
		GLuint offset;
	};

	Ref< ResourceContextOpenGL > m_resourceContext;
	GLuint m_buffer;
	GLuint m_vertexStride;
	AttributeDesc m_attributeDesc[T_OGL_MAX_USAGE_INDEX];
	uint32_t m_attributeHash;
	uint8_t* m_lock;
	GLvoid* m_mapped;
	AlignedVector< uint8_t > m_data;
	bool m_dirty;
};

	}
}

#endif	// traktor_render_VertexBufferDynamicVBO_H
