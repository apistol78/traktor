#pragma once

#include <string>
#include <vector>
#include "Render/OpenGL/ES2/TypesOpenGLES2.h"
#include "Render/Resource/ProgramResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_ES2_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class T_DLLCLASS ProgramResourceOpenGLES2 : public ProgramResource
{
	T_RTTI_CLASS;

public:
	ProgramResourceOpenGLES2();

	ProgramResourceOpenGLES2(
		const std::string& vertexShader,
		const std::string& fragmentShader,
		const std::vector< std::wstring >& textures,
		const std::vector< NamedUniformType >& uniforms,
		const std::vector< SamplerBindingOpenGL >& samplers,
		const RenderStateOpenGL& renderState
	);

	const std::string& getVertexShader() const { return m_vertexShader; }

	const std::string& getFragmentShader() const { return m_fragmentShader; }

	const std::vector< std::wstring >& getTextures() const { return m_textures; }

	const std::vector< NamedUniformType >& getUniforms() const { return m_uniforms; }

	const std::vector< SamplerBindingOpenGL >& getSamplers() const { return m_samplers; }

	const RenderStateOpenGL& getRenderState() const { return m_renderState; }

	void setHash(uint32_t hash) { m_hash = hash; }

	uint32_t getHash() const { return m_hash; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::string m_vertexShader;
	std::string m_fragmentShader;
	std::vector< std::wstring > m_textures;
	std::vector< NamedUniformType > m_uniforms;
	std::vector< SamplerBindingOpenGL > m_samplers;
	RenderStateOpenGL m_renderState;
	uint32_t m_hash;
};

	}
}

