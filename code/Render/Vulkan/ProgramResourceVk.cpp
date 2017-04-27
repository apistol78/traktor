/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Vulkan/ProgramResourceVk.h"

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

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceVk", 0, ProgramResourceVk, ProgramResource)

ProgramResourceVk::ProgramResourceVk()
:	m_parameterScalarSize(0)
,	m_parameterTextureSize(0)
{
}

void ProgramResourceVk::serialize(ISerializer& s)
{
	s >> MemberRenderState(L"renderState", m_renderState);
	s >> MemberStlVector< uint32_t >(L"vertexShader", m_vertexShader);
	s >> MemberStlVector< uint32_t >(L"fragmentShader", m_fragmentShader);
	s >> MemberStaticArray< UniformBufferDesc, 3, MemberComposite< UniformBufferDesc > >(L"vertexUniformBuffers", m_vertexUniformBuffers);
	s >> MemberStaticArray< UniformBufferDesc, 3, MemberComposite< UniformBufferDesc > >(L"fragmentUniformBuffers", m_fragmentUniformBuffers);
	s >> MemberStlVector< ParameterDesc, MemberComposite< ParameterDesc > >(L"parameters", m_parameters);
	s >> Member< uint32_t >(L"parameterScalarSize", m_parameterScalarSize);
	s >> Member< uint32_t >(L"parameterTextureSize", m_parameterTextureSize);
}

void ProgramResourceVk::ParameterDesc::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< uint32_t >(L"offset", offset);
	s >> Member< uint32_t >(L"size", size);
}

void ProgramResourceVk::ParameterMappingDesc::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"uniformBufferOffset", uniformBufferOffset);
	s >> Member< uint32_t >(L"offset", offset);
	s >> Member< uint32_t >(L"size", size);
}

void ProgramResourceVk::UniformBufferDesc::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"size", size);
	s >> MemberStlVector< ParameterMappingDesc, MemberComposite< ParameterMappingDesc > >(L"parameters", parameters);
}

	}
}
