#include <algorithm>
#include <locale>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/ES2/CubeTextureOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderSystemOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderViewOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramCompilerOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramOpenGLES2.h"
#include "Render/OpenGL/ES2/IndexBufferOpenGLES2.h"
#include "Render/OpenGL/ES2/VertexBufferDynamicOpenGLES2.h"
#include "Render/OpenGL/ES2/VertexBufferStaticOpenGLES2.h"
#include "Render/OpenGL/ES2/VolumeTextureOpenGLES2.h"
#include "Render/OpenGL/ES2/SimpleTextureOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetSetOpenGLES2.h"
#include "Render/OpenGL/ES2/ContextOpenGLES2.h"
#if defined(__IOS__)
#	include "Render/OpenGL/ES2/iOS/EAGLContextWrapper.h"
#endif

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemOpenGLES2", 0, RenderSystemOpenGLES2, IRenderSystem)

RenderSystemOpenGLES2::RenderSystemOpenGLES2()
:	m_nativeHandle(0)
{
}

bool RenderSystemOpenGLES2::create(const RenderSystemDesc& desc)
{
#if !defined(T_OFFLINE_ONLY)
	m_globalContext = ContextOpenGLES2::createResourceContext(desc.nativeHandle);
	if (!m_globalContext)
		return false;
#endif

	m_nativeHandle = desc.nativeHandle;
	return true;
}

void RenderSystemOpenGLES2::destroy()
{
//#if defined(T_OPENGL_ES2_HAVE_EGL)
//	if (m_display == EGL_NO_DISPLAY)
//		return;
//
//	eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//
//	eglDestroyContext(m_display, m_context);
//	eglDestroySurface(m_display, m_surface);
//	eglTerminate(m_display);
//
//	m_display = EGL_NO_DISPLAY;
//	m_context = EGL_NO_CONTEXT;
//	m_surface = EGL_NO_SURFACE;
//#endif
}

bool RenderSystemOpenGLES2::reset(const RenderSystemDesc& desc)
{
	return true;
}

void RenderSystemOpenGLES2::getInformation(RenderSystemInformation& outInfo) const
{
}

uint32_t RenderSystemOpenGLES2::getDisplayModeCount() const
{
	return 0;
}

DisplayMode RenderSystemOpenGLES2::getDisplayMode(uint32_t index) const
{
	return DisplayMode();
}

DisplayMode RenderSystemOpenGLES2::getCurrentDisplayMode() const
{
#if defined(_WIN32)

	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmgl);

	DisplayMode dm;
	dm.width = dmgl.dmPelsWidth;
	dm.height = dmgl.dmPelsHeight;
	dm.refreshRate = (uint16_t)dmgl.dmDisplayFrequency;
	dm.colorBits = (uint16_t)dmgl.dmBitsPerPel;
	return dm;

#elif defined(__IOS__)

	DisplayMode dm;
	dm.width = EAGLContextWrapper::getCurrentWidth();
	dm.height = EAGLContextWrapper::getCurrentHeight();
	dm.refreshRate = 60;
	dm.colorBits = 32;
	return dm;

#else
	return DisplayMode();
#endif
}

float RenderSystemOpenGLES2::getDisplayAspectRatio() const
{
#if defined(__IOS__)
	bool landscape = m_globalContext->getLandscape();
	float ratio = float(EAGLContextWrapper::getCurrentWidth()) / EAGLContextWrapper::getCurrentHeight();
	return landscape ? 1.0f / ratio : ratio;
#else
	return 0.0f;
#endif
}

Ref< IRenderView > RenderSystemOpenGLES2::createRenderView(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32) || defined(__PNACL__) || defined(__EMSCRIPTEN__) || defined(__ANDROID__)
	Ref< ContextOpenGLES2 > context = ContextOpenGLES2::createContext(
		m_globalContext,
		m_nativeHandle,
		desc
	);
	return new RenderViewOpenGLES2(m_globalContext, context);
#else
	return 0;
#endif
}

Ref< IRenderView > RenderSystemOpenGLES2::createRenderView(const RenderViewEmbeddedDesc& desc)
{
#if !defined(T_OFFLINE_ONLY)
	Ref< ContextOpenGLES2 > context = ContextOpenGLES2::createContext(
		m_globalContext,
		m_nativeHandle,
		desc
	);
	return new RenderViewOpenGLES2(m_globalContext, context);
#else
	return 0;
#endif
}

Ref< VertexBuffer > RenderSystemOpenGLES2::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
#if !defined(T_OFFLINE_ONLY)
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	if (!dynamic)
		return new VertexBufferStaticOpenGLES2(m_globalContext, vertexElements, bufferSize);
	else
		return new VertexBufferDynamicOpenGLES2(m_globalContext, vertexElements, bufferSize);
#else
	return 0;
#endif
}

Ref< IndexBuffer > RenderSystemOpenGLES2::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
#if !defined(T_OFFLINE_ONLY)
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	return new IndexBufferOpenGLES2(m_globalContext, indexType, bufferSize, dynamic);
#else
	return 0;
#endif
}

Ref< ISimpleTexture > RenderSystemOpenGLES2::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
#if !defined(T_OFFLINE_ONLY)
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	Ref< SimpleTextureOpenGLES2 > texture = new SimpleTextureOpenGLES2(m_globalContext);
	if (texture->create(desc))
		return texture;
	else
		return texture;
#else
	return 0;
#endif
}

Ref< ICubeTexture > RenderSystemOpenGLES2::createCubeTexture(const CubeTextureCreateDesc& desc)
{
#if !defined(T_OFFLINE_ONLY)
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	Ref< CubeTextureOpenGLES2 > texture = new CubeTextureOpenGLES2(m_globalContext);
	if (texture->create(desc))
		return texture;
	else
		return texture;
#else
	return 0;
#endif
}

Ref< IVolumeTexture > RenderSystemOpenGLES2::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
#if !defined(T_OFFLINE_ONLY)
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	Ref< VolumeTextureOpenGLES2 > texture = new VolumeTextureOpenGLES2(m_globalContext);
	if (texture->create(desc))
		return texture;
	else
		return texture;
#else
	return 0;
#endif
}

Ref< RenderTargetSet > RenderSystemOpenGLES2::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
#if !defined(T_OFFLINE_ONLY)
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	Ref< RenderTargetSetOpenGLES2 > renderTargetSet = new RenderTargetSetOpenGLES2(m_globalContext);
	if (renderTargetSet->create(desc))
		return renderTargetSet;
	else
		return 0;
#else
	return 0;
#endif
}

Ref< IProgram > RenderSystemOpenGLES2::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
#if !defined(T_OFFLINE_ONLY)
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	return ProgramOpenGLES2::create(m_globalContext, programResource);
#else
	return 0;
#endif
}

Ref< IProgramCompiler > RenderSystemOpenGLES2::createProgramCompiler() const
{
	return new ProgramCompilerOpenGLES2();
}

Ref< ITimeQuery > RenderSystemOpenGLES2::createTimeQuery() const
{
	return 0;
}

void RenderSystemOpenGLES2::purge()
{
}

void RenderSystemOpenGLES2::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

	}
}
