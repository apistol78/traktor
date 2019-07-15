#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Render/OpenGL/Std/ProgramResourceOpenGL.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberRenderState : public MemberComplex
{
public:
	MemberRenderState(const wchar_t* const name, RenderState& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> MemberEnumByValue< CullMode >(L"cullMode", m_ref.cullMode);
		s >> Member< bool >(L"blendEnable", m_ref.blendEnable);
		s >> MemberEnumByValue< BlendOperation >(L"blendColorOperation", m_ref.blendColorOperation);
		s >> MemberEnumByValue< BlendFactor >(L"blendColorSource", m_ref.blendColorSource);
		s >> MemberEnumByValue< BlendFactor >(L"blendColorDestination", m_ref.blendColorDestination);
		s >> MemberEnumByValue< BlendOperation >(L"blendAlphaOperation", m_ref.blendAlphaOperation);
		s >> MemberEnumByValue< BlendFactor >(L"blendAlphaSource", m_ref.blendAlphaSource);
		s >> MemberEnumByValue< BlendFactor >(L"blendAlphaDestination", m_ref.blendAlphaDestination);
		s >> Member< uint32_t >(L"colorWriteMask", m_ref.colorWriteMask);
		s >> Member< bool >(L"depthEnable", m_ref.depthEnable);
		s >> Member< bool >(L"depthWriteEnable", m_ref.depthWriteEnable);
		s >> MemberEnumByValue< CompareFunction >(L"depthFunction", m_ref.depthFunction);
		s >> Member< bool >(L"alphaTestEnable", m_ref.alphaTestEnable);
		s >> MemberEnumByValue< CompareFunction >(L"alphaTestFunction", m_ref.alphaTestFunction);
		s >> Member< int32_t >(L"alphaTestReference", m_ref.alphaTestReference);
		s >> Member< bool >(L"alphaToCoverageEnable", m_ref.alphaToCoverageEnable);
		s >> Member< bool >(L"wireframe", m_ref.wireframe);
		s >> Member< bool >(L"stencilEnable", m_ref.stencilEnable);
		s >> MemberEnumByValue< StencilOperation >(L"stencilFail", m_ref.stencilFail);
		s >> MemberEnumByValue< StencilOperation >(L"stencilZFail", m_ref.stencilZFail);
		s >> MemberEnumByValue< StencilOperation >(L"stencilPass", m_ref.stencilPass);
		s >> MemberEnumByValue< CompareFunction >(L"stencilFunction", m_ref.stencilFunction);
		s >> Member< uint32_t >(L"stencilReference", m_ref.stencilReference);
		s >> Member< uint32_t >(L"stencilMask", m_ref.stencilMask);
	}

private:
	RenderState& m_ref;
};

class MemberSamplerState : public MemberComplex
{
public:
	MemberSamplerState(const wchar_t* const name, SamplerState& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> MemberEnumByValue< Filter >(L"minFilter", m_ref.minFilter);
		s >> MemberEnumByValue< Filter >(L"mipFilter", m_ref.mipFilter);
		s >> MemberEnumByValue< Filter >(L"magFilter", m_ref.magFilter);
		s >> MemberEnumByValue< Address >(L"addressU", m_ref.addressU);
		s >> MemberEnumByValue< Address >(L"addressV", m_ref.addressV);
		s >> MemberEnumByValue< Address >(L"addressW", m_ref.addressW);
		s >> MemberEnumByValue< CompareFunction >(L"compare", m_ref.compare);
		s >> Member< float >(L"mipBias", m_ref.mipBias);
		s >> Member< bool >(L"ignoreMips", m_ref.ignoreMips);
		s >> Member< bool >(L"useAnisotropic", m_ref.useAnisotropic);
	}

private:
	SamplerState& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceOpenGL", 12, ProgramResourceOpenGL, ProgramResource)

ProgramResourceOpenGL::ProgramResourceOpenGL()
:	m_texturesCount(0)
,	m_hash(0)
{
	m_uniformBufferSizes[0] =
	m_uniformBufferSizes[1] =
	m_uniformBufferSizes[2] = 0;
}

void ProgramResourceOpenGL::serialize(ISerializer& s)
{
	T_ASSERT(s.getVersion() >= 12);

	s >> MemberRenderState(L"renderState", m_renderState);
	s >> Member< std::string >(L"vertexShader", m_vertexShader);
	s >> Member< std::string >(L"fragmentShader", m_fragmentShader);
	s >> Member< std::string >(L"computeShader", m_computeShader);
	s >> MemberStaticArray< uint32_t, 3 >(L"uniformBufferSizes", m_uniformBufferSizes);
	s >> Member< uint32_t >(L"texturesCount", m_texturesCount);
	s >> MemberAlignedVector< ParameterDesc, MemberComposite< ParameterDesc > >(L"parameters", m_parameters);
	s >> MemberAlignedVector< SamplerDesc, MemberComposite< SamplerDesc > >(L"samplers", m_samplers);
	s >> MemberAlignedVector< SBufferDesc, MemberComposite< SBufferDesc > >(L"sbuffers", m_sbuffers);
	s >> Member< uint32_t >(L"hash", m_hash);
}

void ProgramResourceOpenGL::ParameterDesc::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< uint32_t >(L"buffer", buffer);
	s >> Member< uint32_t >(L"offset", offset);
	s >> Member< uint32_t >(L"size", size);
}

void ProgramResourceOpenGL::SamplerDesc::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"unit", unit);
	s >> MemberSamplerState(L"state", state);
	s >> Member< uint32_t >(L"textureIndex", textureIndex);
}

void ProgramResourceOpenGL::SBufferDesc::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"binding", binding);
}

	}
}
