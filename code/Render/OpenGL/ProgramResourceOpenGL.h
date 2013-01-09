#ifndef traktor_render_ProgramResourceOpenGL_H
#define traktor_render_ProgramResourceOpenGL_H

#include "Render/OpenGL/TypesOpenGL.h"
#include "Render/Resource/ProgramResource.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class ProgramResourceOpenGL : public ProgramResource
{
	T_RTTI_CLASS;

public:
	ProgramResourceOpenGL();

	ProgramResourceOpenGL(
		const std::string& vertexShader,
		const std::string& fragmentShader,
		const std::vector< std::wstring >& textures,
		const std::vector< SamplerBinding >& samplers,
		const RenderState& renderState
	);

	const std::string& getVertexShader() const { return m_vertexShader; }

	const std::string& getFragmentShader() const { return m_fragmentShader; }

	const std::vector< std::wstring >& getTextures() const { return m_textures; }

	const std::vector< SamplerBinding >& getSamplers() const { return m_samplers; }

	const RenderState& getRenderState() const { return m_renderState; }

	void setHash(uint32_t hash) { m_hash = hash; }

	uint32_t getHash() const { return m_hash; }

	virtual bool serialize(ISerializer& s);

private:
	std::string m_vertexShader;
	std::string m_fragmentShader;
	std::vector< std::wstring > m_textures;
	std::vector< SamplerBinding > m_samplers;
	RenderState m_renderState;
	uint32_t m_hash;
};

	}
}

#endif	// traktor_render_ProgramResourceOpenGL_H
