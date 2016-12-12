#include "Core/Log/Log.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/ES2/ExtensionsGLES2.h"
#include "Render/OpenGL/ES2/StateCache.h"
#include "Render/OpenGL/ES2/VertexBufferStaticOpenGLES2.h"
#if defined(__ANDROID__)
#	include "Render/OpenGL/ES2/Android/ContextOpenGLES2.h"
#elif defined(__IOS__)
#	include "Render/OpenGL/ES2/iOS/EAGLContextWrapper.h"
#elif defined(__PNACL__)
#	include "Render/OpenGL/ES2/PNaCl/ContextOpenGLES2.h"
#elif defined(_WIN32)
#	include "Render/OpenGL/ES2/Win32/ContextOpenGLES2.h"
#endif
#include "Render/OpenGL/Glsl/GlslType.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteBufferCallback : public ContextOpenGLES2::IDeleteCallback
{
	GLuint m_bufferName;

	DeleteBufferCallback(GLuint bufferName)
	:	m_bufferName(bufferName)
	{
	}

	virtual ~DeleteBufferCallback()
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteBuffers(1, &m_bufferName));
		delete this;
	}
};

#if defined(GL_OES_vertex_array_object) && !defined(_WIN32)
struct DeleteVertexArrayCallback : public ContextOpenGLES2::IDeleteCallback
{
	GLuint m_arrayName;

	DeleteVertexArrayCallback(GLuint arrayName)
	:	m_arrayName(arrayName)
	{
	}

	virtual ~DeleteVertexArrayCallback()
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(g_glDeleteVertexArraysOES(1, &m_arrayName));
		delete this;
	}
};
#endif

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferStaticOpenGLES2", VertexBufferStaticOpenGLES2, VertexBufferOpenGLES2)

VertexBufferStaticOpenGLES2::VertexBufferStaticOpenGLES2(ContextOpenGLES2* context, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize)
:	VertexBufferOpenGLES2(bufferSize)
,	m_context(context)
,	m_arrayObject(0)
,	m_bufferObject(0)
,	m_vertexStride(0)
,	m_lockOffset(0)
,	m_lockSize(0)
,	m_dirty(false)
{
	m_vertexStride = getVertexSize(vertexElements);
	T_ASSERT (m_vertexStride > 0);
	
	for (size_t i = 0; i < vertexElements.size(); ++i)
	{
		if (vertexElements[i].getIndex() >= 4)
		{
			log::warning << L"Index out of bounds on vertex element " << uint32_t(i) << Endl;
			continue;
		}

		AttributeDesc desc;
		desc.location = glsl_vertex_attr_location(vertexElements[i].getDataUsage(), vertexElements[i].getIndex());
		
		switch (vertexElements[i].getDataType())
		{
		case DtFloat1:
			desc.size = 1;
			desc.type = GL_FLOAT;
			desc.normalized = GL_FALSE;
			break;

		case DtFloat2:
			desc.size = 2;
			desc.type = GL_FLOAT;
			desc.normalized = GL_FALSE;
			break;

		case DtFloat3:
			desc.size = 3;
			desc.type = GL_FLOAT;
			desc.normalized = GL_FALSE;
			break;

		case DtFloat4:
			desc.size = 4;
			desc.type = GL_FLOAT;
			desc.normalized = GL_FALSE;
			break;

		case DtByte4:
			desc.size = 4;
			desc.type = GL_UNSIGNED_BYTE;
			desc.normalized = GL_FALSE;
			break;

		case DtByte4N:
			desc.size = 4;
			desc.type = GL_UNSIGNED_BYTE;
			desc.normalized = GL_TRUE;
			break;

		case DtShort2:
			desc.size = 2;
			desc.type = GL_SHORT;
			desc.normalized = GL_FALSE;
			break;

		case DtShort4:
			desc.size = 4;
			desc.type = GL_SHORT;
			desc.normalized = GL_FALSE;
			break;

		case DtShort2N:
			desc.size = 2;
			desc.type = GL_SHORT;
			desc.normalized = GL_TRUE;
			break;

		case DtShort4N:
			desc.size = 4;
			desc.type = GL_SHORT;
			desc.normalized = GL_TRUE;
			break;

#if defined(GL_OES_vertex_half_float)
		case DtHalf2:
			desc.size = 2;
			desc.type = GL_HALF_FLOAT_OES;
			desc.normalized = GL_TRUE;
			break;

		case DtHalf4:
			desc.size = 4;
			desc.type = GL_HALF_FLOAT_OES;
			desc.normalized = GL_TRUE;
			break;
#endif

		default:
			log::warning << L"Unsupport vertex format" << Endl;
		}
		
		desc.offset = vertexElements[i].getOffset();
		
		m_attributes.push_back(desc);
	}
}

VertexBufferStaticOpenGLES2::~VertexBufferStaticOpenGLES2()
{
	destroy();
}

void VertexBufferStaticOpenGLES2::destroy()
{
	if (m_bufferObject)
	{
		if (m_context)
			m_context->deleteResource(new DeleteBufferCallback(m_bufferObject));
		m_bufferObject = 0;
	}
#if defined(GL_OES_vertex_array_object) && !defined(_WIN32)
	if (m_arrayObject)
	{
		if (m_context)
			m_context->deleteResource(new DeleteVertexArrayCallback(m_arrayObject));
		m_arrayObject = 0;
	}
#endif
}

void* VertexBufferStaticOpenGLES2::lock()
{
	m_lockOffset = 0;
	m_lockSize = getBufferSize();
	
	if (!m_buffer.ptr())
		m_buffer.reset((uint8_t*)Alloc::acquireAlign(m_lockSize, 16, "VB"));

	return m_buffer.ptr();
}

void* VertexBufferStaticOpenGLES2::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_FATAL_ASSERT (m_buffer.ptr() == 0);
	T_FATAL_ASSERT (!m_dirty);

	m_lockOffset = vertexOffset * m_vertexStride;
	m_lockSize = vertexCount * m_vertexStride;
	m_buffer.reset((uint8_t*)Alloc::acquireAlign(m_lockSize, 16, "VB"));

	return m_buffer.ptr();
}

void VertexBufferStaticOpenGLES2::unlock()
{
	m_dirty = true;
	setContentValid(true);
}

void VertexBufferStaticOpenGLES2::activate(StateCache* stateCache)
{
	if (m_dirty)
	{
		if (!m_bufferObject)
		{
			T_OGL_SAFE(glGenBuffers(1, &m_bufferObject));
			T_FATAL_ASSERT (m_bufferObject != 0);
		}

		stateCache->setArrayBuffer(m_bufferObject);

		int32_t bufferSize = getBufferSize();
		if (m_lockOffset <= 0 && m_lockSize >= bufferSize)
		{
			T_OGL_SAFE(glBufferData(
				GL_ARRAY_BUFFER,
				m_lockSize,
				m_buffer.ptr(),
				GL_STATIC_DRAW
			));
		}
		else
		{
			T_OGL_SAFE(glBufferSubData(
				GL_ARRAY_BUFFER,
				m_lockOffset,
				m_lockSize,
				m_buffer.ptr()
			));		
		}
		m_buffer.release();
	
#if defined(GL_OES_vertex_array_object) && !defined(_WIN32)
		if (m_arrayObject)
		{
			if (m_context)
				m_context->deleteResource(new DeleteVertexArrayCallback(m_arrayObject));
			m_arrayObject = 0;
		}
#endif
		m_dirty = false;
	}

#if defined(GL_OES_vertex_array_object) && !defined(_WIN32)
	if (m_arrayObject == 0 && g_glGenVertexArraysOES != 0)
	{
		T_OGL_SAFE(g_glGenVertexArraysOES(1, &m_arrayObject));
		stateCache->setVertexArrayObject(m_arrayObject);

		if (m_bufferObject)
			stateCache->setArrayBuffer(m_bufferObject);

		for (AlignedVector< AttributeDesc >::const_iterator i = m_attributes.begin(); i != m_attributes.end(); ++i)
		{
			T_OGL_SAFE(glEnableVertexAttribArray(i->location));
			T_OGL_SAFE(glVertexAttribPointer(
				i->location,
				i->size,
				i->type,
				i->normalized,
				m_vertexStride,
				(const GLvoid*)i->offset
			));
		}
	}
	else
	{
		stateCache->setVertexArrayObject(m_arrayObject);
	}
#else
	{
		stateCache->setVertexArrayObject(0);
		
		GLint maxAttributeIndex = 0;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttributeIndex);
		for (GLint i = 0; i < maxAttributeIndex; ++i)
		{
			T_OGL_SAFE(glDisableVertexAttribArray(i));
		}

		for (AlignedVector< AttributeDesc >::const_iterator i = m_attributes.begin(); i != m_attributes.end(); ++i)
		{
			T_OGL_SAFE(glEnableVertexAttribArray(i->location));
			T_OGL_SAFE(glVertexAttribPointer(
				i->location,
				i->size,
				i->type,
				i->normalized,
				m_vertexStride,
				(const GLvoid*)i->offset
			));
		}
	}
#endif
}

	}
}
