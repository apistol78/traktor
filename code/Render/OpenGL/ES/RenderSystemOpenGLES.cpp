#include <algorithm>
#include <locale>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/ES/Platform.h"
#include "Render/OpenGL/ES/CubeTextureOpenGLES.h"
#include "Render/OpenGL/ES/RenderSystemOpenGLES.h"
#include "Render/OpenGL/ES/RenderViewOpenGLES.h"
#include "Render/OpenGL/ES/ProgramOpenGLES.h"
#include "Render/OpenGL/ES/IndexBufferOpenGLES.h"
#include "Render/OpenGL/ES/VertexBufferDynamicOpenGLES.h"
#include "Render/OpenGL/ES/VertexBufferStaticOpenGLES.h"
#include "Render/OpenGL/ES/VolumeTextureOpenGLES.h"
#include "Render/OpenGL/ES/SimpleTextureOpenGLES.h"
#include "Render/OpenGL/ES/RenderTargetSetOpenGLES.h"
#if defined(__ANDROID__)
#	include "Render/OpenGL/ES/Android/ContextOpenGLES.h"
#elif defined(__IOS__)
#	include "Render/OpenGL/ES/iOS/ContextOpenGLES.h"
#	include "Render/OpenGL/ES/iOS/EAGLContextWrapper.h"
#elif defined(__EMSCRIPTEN__)
#	include "Render/OpenGL/ES/Emscripten/ContextOpenGLES.h"
#elif defined(_WIN32)
#	include "Render/OpenGL/ES/Win32/ContextOpenGLES.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Render/OpenGL/ES/Linux/ContextOpenGLES.h"
#endif

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemOpenGLES", 0, RenderSystemOpenGLES, IRenderSystem)

RenderSystemOpenGLES::RenderSystemOpenGLES()
{
}

bool RenderSystemOpenGLES::create(const RenderSystemDesc& desc)
{
	m_sysapp = desc.sysapp;
	return true;
}

void RenderSystemOpenGLES::destroy()
{
}

bool RenderSystemOpenGLES::reset(const RenderSystemDesc& desc)
{
	return true;
}

void RenderSystemOpenGLES::getInformation(RenderSystemInformation& outInfo) const
{
}

uint32_t RenderSystemOpenGLES::getDisplayModeCount() const
{
#if defined(_WIN32)
	uint32_t count = 0;

	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	while (EnumDisplaySettings(NULL, count, &dmgl))
		++count;

	return count;
#else
	return 0;
#endif
}

DisplayMode RenderSystemOpenGLES::getDisplayMode(uint32_t index) const
{
#if defined(_WIN32)
	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	EnumDisplaySettings(NULL, index, &dmgl);

	DisplayMode dm;
	dm.width = dmgl.dmPelsWidth;
	dm.height = dmgl.dmPelsHeight;
	dm.refreshRate = (uint16_t)dmgl.dmDisplayFrequency;
	dm.colorBits = (uint16_t)dmgl.dmBitsPerPel;
	return dm;
#else
	return DisplayMode();
#endif
}

DisplayMode RenderSystemOpenGLES::getCurrentDisplayMode() const
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

#elif defined(__EMSCRIPTEN__)

	int width, height, fullScreen;
	emscripten_get_canvas_size(&width, &height, &fullScreen);

	DisplayMode dm;
	dm.width = width;
	dm.height = height;
	dm.refreshRate = 60;
	dm.colorBits = 32;
	return dm;

#else
	return DisplayMode();
#endif
}

float RenderSystemOpenGLES::getDisplayAspectRatio() const
{
	return 0.0f;
}

Ref< IRenderView > RenderSystemOpenGLES::createRenderView(const RenderViewDefaultDesc& desc)
{
	m_context = ContextOpenGLES::createContext(
		m_sysapp,
		desc
	);
	if (m_context)
		return new RenderViewOpenGLES(m_context);
	else
		return nullptr;
}

Ref< IRenderView > RenderSystemOpenGLES::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	m_context = ContextOpenGLES::createContext(
		m_sysapp,
		desc
	);
	if (m_context)
		return new RenderViewOpenGLES(m_context);
	else
		return nullptr;
}

Ref< VertexBuffer > RenderSystemOpenGLES::createVertexBuffer(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(ContextOpenGLES::Scope)(m_context);
	if (!dynamic)
		return new VertexBufferStaticOpenGLES(m_context, vertexElements, bufferSize);
	else
		return new VertexBufferDynamicOpenGLES(m_context, vertexElements, bufferSize);
}

Ref< IndexBuffer > RenderSystemOpenGLES::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(ContextOpenGLES::Scope)(m_context);
	return new IndexBufferOpenGLES(m_context, indexType, bufferSize, dynamic);
}

Ref< StructBuffer > RenderSystemOpenGLES::createStructBuffer(const AlignedVector< StructElement >& structElements, uint32_t bufferSize)
{
	return nullptr;
}

Ref< ISimpleTexture > RenderSystemOpenGLES::createSimpleTexture(const SimpleTextureCreateDesc& desc, const wchar_t* const tag)
{
	T_ANONYMOUS_VAR(ContextOpenGLES::Scope)(m_context);
	Ref< SimpleTextureOpenGLES > texture = new SimpleTextureOpenGLES(m_context);
	if (texture->create(desc))
		return texture;
	else
		return nullptr;
}

Ref< ICubeTexture > RenderSystemOpenGLES::createCubeTexture(const CubeTextureCreateDesc& desc, const wchar_t* const tag)
{
	T_ANONYMOUS_VAR(ContextOpenGLES::Scope)(m_context);
	Ref< CubeTextureOpenGLES > texture = new CubeTextureOpenGLES(m_context);
	if (texture->create(desc))
		return texture;
	else
		return nullptr;
}

Ref< IVolumeTexture > RenderSystemOpenGLES::createVolumeTexture(const VolumeTextureCreateDesc& desc, const wchar_t* const tag)
{
	T_ANONYMOUS_VAR(ContextOpenGLES::Scope)(m_context);
	Ref< VolumeTextureOpenGLES > texture = new VolumeTextureOpenGLES(m_context);
	if (texture->create(desc))
		return texture;
	else
		return nullptr;
}

Ref< IRenderTargetSet > RenderSystemOpenGLES::createRenderTargetSet(const RenderTargetSetCreateDesc& desc, IRenderTargetSet* sharedDepthStencil, const wchar_t* const tag)
{
	T_ANONYMOUS_VAR(ContextOpenGLES::Scope)(m_context);
	Ref< RenderTargetSetOpenGLES > renderTargetSet = new RenderTargetSetOpenGLES(m_context);
	if (renderTargetSet->create(desc, sharedDepthStencil))
		return renderTargetSet;
	else
		return nullptr;
}

Ref< IProgram > RenderSystemOpenGLES::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	T_ANONYMOUS_VAR(ContextOpenGLES::Scope)(m_context);
	return ProgramOpenGLES::create(m_context, programResource);
}

void RenderSystemOpenGLES::purge()
{
}

void RenderSystemOpenGLES::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

	}
}
