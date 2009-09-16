#include <algorithm>
#include <locale>
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/ProgramResourceOpenGL.h"
#include "Render/OpenGL/Glsl.h"
#include "Render/OpenGL/GlslProgram.h"
#include "Render/OpenGL/ES2/RenderSystemOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderViewOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramOpenGLES2.h"
#include "Render/OpenGL/ES2/IndexBufferOpenGLES2.h"
#include "Render/OpenGL/ES2/VertexBufferOpenGLES2.h"
#include "Render/OpenGL/ES2/SimpleTextureOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetSetOpenGLES2.h"
#include "Render/DisplayMode.h"
#include "Render/VertexElement.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.RenderSystemOpenGLES2", RenderSystemOpenGLES2, IRenderSystem)

RenderSystemOpenGLES2::RenderSystemOpenGLES2()
{
}

bool RenderSystemOpenGLES2::create()
{
	return true;
}

void RenderSystemOpenGLES2::destroy()
{
}

int RenderSystemOpenGLES2::getDisplayModeCount() const
{
	return 0;
}

DisplayMode* RenderSystemOpenGLES2::getDisplayMode(int index)
{
	return 0;
}

DisplayMode* RenderSystemOpenGLES2::getCurrentDisplayMode()
{
	return 0;
}

bool RenderSystemOpenGLES2::handleMessages()
{
	return true;
}

IRenderView* RenderSystemOpenGLES2::createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc)
{
	return 0;
}

IRenderView* RenderSystemOpenGLES2::createRenderView(void* windowHandle, const RenderViewCreateDesc& desc)
{
	return 0;
}

VertexBuffer* RenderSystemOpenGLES2::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	return gc_new< VertexBufferOpenGLES2 >(cref(vertexElements), bufferSize, dynamic);
}

IndexBuffer* RenderSystemOpenGLES2::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	return gc_new< IndexBufferOpenGLES2 >(indexType, bufferSize, dynamic);
}

ISimpleTexture* RenderSystemOpenGLES2::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureOpenGLES2 > texture = gc_new< SimpleTextureOpenGLES2 >();
	if (texture->create(desc))
		return texture;
	else
		return texture;
}

ICubeTexture* RenderSystemOpenGLES2::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	return 0;
}

IVolumeTexture* RenderSystemOpenGLES2::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	return 0;
}

RenderTargetSet* RenderSystemOpenGLES2::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	Ref< RenderTargetSetOpenGLES2 > renderTargetSet = gc_new< RenderTargetSetOpenGLES2 >();
	if (renderTargetSet->create(desc))
		return renderTargetSet;
	else
		return 0;
}

ProgramResource* RenderSystemOpenGLES2::compileProgram(const ShaderGraph* shaderGraph, int optimize, bool validate)
{
	return gc_new< ProgramResourceOpenGL >(shaderGraph);
}

IProgram* RenderSystemOpenGLES2::createProgram(const ProgramResource* programResource)
{
	Ref< const ProgramResourceOpenGL > resource = dynamic_type_cast< const ProgramResourceOpenGL* >(programResource);
	if (!resource)
		return 0;

	const ShaderGraph* shaderGraph = resource->getShaderGraph();

	GlslProgram glslProgram;
	if (!Glsl().generate(shaderGraph, glslProgram))
		return 0;

	Ref< ProgramOpenGLES2 > shader = gc_new< ProgramOpenGLES2 >();
	if (!shader->create(glslProgram))
		return 0;

	return shader;
}

	}
}
