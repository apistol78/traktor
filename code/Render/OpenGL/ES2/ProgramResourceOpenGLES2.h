#ifndef traktor_render_ProgramResourceOpenGLES2_H
#define traktor_render_ProgramResourceOpenGLES2_H

#include "Core/Heap/Ref.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/ProgramResource.h"
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class ProgramResourceOpenGLES2 : public ProgramResource
{
	T_RTTI_CLASS(ProgramResourceOpenGL)

public:
	ProgramResourceOpenGLES2();

	ProgramResourceOpenGLES2(
		const void* buffer,
		uint32_t bufferSize,
		const std::set< std::wstring >& vertexSamplers,
		const std::set< std::wstring >& fragmentSamplers,
		const RenderState& renderState
	);

	uint32_t getBufferSize() const { return m_bufferSize; }

	const uint8_t* getBuffer() const { return m_buffer.c_ptr(); }

	const std::set< std::wstring >& getVertexSamplers() const { return m_vertexSamplers; }

	const std::set< std::wstring >& getFragmentSamplers() const { return m_fragmentSamplers; }

	const RenderState& getRenderState() const { return m_renderState; }

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_bufferSize;
	AutoArrayPtr< uint8_t > m_buffer;
	std::set< std::wstring > m_vertexSamplers;
	std::set< std::wstring > m_fragmentSamplers;
	RenderState m_renderState;
};

	}
}

#endif	// traktor_render_ProgramResourceOpenGLES2_H
