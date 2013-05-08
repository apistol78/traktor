#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/OpenGL/ProgramResourceOpenGL.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberSamplerBindingOpenGL : public MemberComplex
{
public:
	MemberSamplerBindingOpenGL(const wchar_t* const name, SamplerBindingOpenGL& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> Member< GLuint >(L"stage", m_ref.stage);
		s >> Member< GLenum >(L"target", m_ref.target);
		s >> Member< int32_t >(L"texture", m_ref.texture);
	}

private:
	SamplerBindingOpenGL& m_ref;
};

class MemberSamplerStateOpenGL : public MemberComplex
{
public:
	MemberSamplerStateOpenGL(const wchar_t* const name, SamplerStateOpenGL& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}
	
	virtual void serialize(ISerializer& s) const
	{
		s >> Member< GLenum >(L"minFilter", m_ref.minFilter);
		s >> Member< GLenum >(L"magFilter", m_ref.magFilter);
		s >> Member< GLenum >(L"wrapS", m_ref.wrapS);
		s >> Member< GLenum >(L"wrapT", m_ref.wrapT);
		s >> Member< GLenum >(L"wrapR", m_ref.wrapR);
	}
	
private:
	SamplerStateOpenGL& m_ref;
};

class MemberRenderStateOpenGL : public MemberComplex
{
public:
	MemberRenderStateOpenGL(const wchar_t* const name, RenderStateOpenGL& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}
	
	virtual void serialize(ISerializer& s) const
	{
		s >> Member< GLboolean >(L"cullFaceEnable", m_ref.cullFaceEnable);
		s >> Member< GLenum >(L"cullFace", m_ref.cullFace);
		s >> Member< GLboolean >(L"blendEnable", m_ref.blendEnable);
		s >> Member< GLenum >(L"blendEquation", m_ref.blendEquation);
		s >> Member< GLenum >(L"blendFuncSrc", m_ref.blendFuncSrc);
		s >> Member< GLenum >(L"blendFuncDest", m_ref.blendFuncDest);
		s >> Member< GLboolean >(L"depthTestEnable", m_ref.depthTestEnable);
		s >> Member< uint32_t >(L"colorMask", m_ref.colorMask);
		s >> Member< GLboolean >(L"depthMask", m_ref.depthMask);
		s >> Member< GLboolean >(L"alphaTestEnable", m_ref.alphaTestEnable);
		s >> Member< GLenum >(L"alphaFunc", m_ref.alphaFunc);
		s >> Member< GLclampf >(L"alphaRef", m_ref.alphaRef);
		s >> Member< GLboolean >(L"stencilTestEnable", m_ref.stencilTestEnable);
		s >> Member< GLenum >(L"stencilFunc", m_ref.stencilFunc);
		s >> Member< GLint >(L"stencilRef", m_ref.stencilRef);
		s >> Member< GLenum >(L"stencilOpFail", m_ref.stencilOpFail);
		s >> Member< GLenum >(L"stencilOpZFail", m_ref.stencilOpZFail);
		s >> Member< GLenum >(L"stencilOpZPass", m_ref.stencilOpZPass);
		s >> MemberStaticArray< SamplerStateOpenGL, 16, MemberSamplerStateOpenGL >(L"samplerStates", m_ref.samplerStates);
	}
	
private:
	RenderStateOpenGL& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceOpenGL", 6, ProgramResourceOpenGL, ProgramResource)

ProgramResourceOpenGL::ProgramResourceOpenGL()
:	m_hash(0)
{
}

ProgramResourceOpenGL::ProgramResourceOpenGL(
	const std::string& vertexShader,
	const std::string& fragmentShader,
	const std::vector< std::wstring >& textures,
	const std::vector< SamplerBindingOpenGL >& samplers,
	const RenderStateOpenGL& renderState
)
:	m_vertexShader(vertexShader)
,	m_fragmentShader(fragmentShader)
,	m_textures(textures)
,	m_samplers(samplers)
,	m_renderState(renderState)
,	m_hash(0)
{
}

void ProgramResourceOpenGL::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 6);

	s >> Member< std::string >(L"vertexShader", m_vertexShader);
	s >> Member< std::string >(L"fragmentShader", m_fragmentShader);
	s >> MemberStlVector< std::wstring >(L"textures", m_textures);
	s >> MemberStlVector< SamplerBindingOpenGL, MemberSamplerBindingOpenGL >(L"samplers", m_samplers);
	s >> MemberRenderStateOpenGL(L"renderState", m_renderState);
	s >> Member< uint32_t >(L"hash", m_hash);
}

	}
}
