#pragma once

#include "Core/Containers/SmallMap.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"

namespace traktor
{
	namespace render
	{

/*! \brief OpenGL resource context.
 * \ingroup OGL
 */
class ResourceContextOpenGL : public ContextOpenGL
{
	T_RTTI_CLASS;

public:
	/*! \brief Delete callback. 
	 * \ingroup OGL
	 *
	 * These are enqueued in the context
	 * and are invoked as soon as it's
	 * safe to actually delete resources.
	 */
	struct IDeleteCallback
	{
		virtual void deleteResource() = 0;
	};

	struct SamplerStateObject
	{
		GLuint samplers[2];	// 0 = with mips, 1 = no mips
	};

#if defined(_WIN32)
	ResourceContextOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);
#elif defined(__APPLE__)
	ResourceContextOpenGL(void* context);
#elif defined(__LINUX__)
	ResourceContextOpenGL(::Display* display, ::Window window, GLXContext context);
#endif

	void setMaxAnisotropy(GLfloat maxAnisotropy);

	GLuint createShaderObject(const char* shader, GLenum shaderType);

	uint32_t createRenderStateObject(const RenderStateOpenGL& renderState);

	uint32_t createSamplerStateObject(const SamplerStateOpenGL& samplerState);

	void deleteResource(IDeleteCallback* callback);

	void deleteResources();

	const SmallMap< uint32_t, SamplerStateObject >& getSamplerStateObjects() const { return m_samplerStateObjects; }

	const AlignedVector< RenderStateOpenGL >& getRenderStateList() const { return m_renderStateList; }

private:
	SmallMap< uint32_t, GLuint > m_shaderObjects;
	SmallMap< uint32_t, uint32_t > m_renderStateListCache;
	SmallMap< uint32_t, SamplerStateObject > m_samplerStateObjects;
	AlignedVector< RenderStateOpenGL > m_renderStateList;
	AlignedVector< IDeleteCallback* > m_deleteResources;
	GLfloat m_maxAnisotropy;
};

	}
}
