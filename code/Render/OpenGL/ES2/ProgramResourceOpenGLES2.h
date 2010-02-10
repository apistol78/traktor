#ifndef traktor_render_ProgramResourceOpenGLES2_H
#define traktor_render_ProgramResourceOpenGLES2_H

#include "Core/Misc/AutoPtr.h"
#include "Render/OpenGL/TypesOpenGL.h"
#include "Render/Resource/ProgramResource.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class ProgramResourceOpenGLES2 : public ProgramResource
{
	T_RTTI_CLASS;

public:
	ProgramResourceOpenGLES2();

	ProgramResourceOpenGLES2(
		const void* buffer,
		uint32_t bufferSize,
		const std::vector< SamplerTexture >& samplerTextures,
		const RenderState& renderState
	);

	uint32_t getBufferSize() const { return m_bufferSize; }

	const uint8_t* getBuffer() const { return m_buffer.c_ptr(); }

	const std::vector< SamplerTexture >& getSamplerTextures() const { return m_samplerTextures; }

	const RenderState& getRenderState() const { return m_renderState; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_bufferSize;
	AutoArrayPtr< uint8_t > m_buffer;
	std::vector< SamplerTexture > m_samplerTextures;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_ProgramResourceOpenGLES2_H
