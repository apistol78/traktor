#include "Render/Ps3/ProgramResourcePs3.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberBin : public MemberComplex
{
public:
	MemberBin(const std::wstring& name, CGCbin*& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		if (s.getDirection() == ISerializer::SdRead)
		{
			uint32_t size;
			if (!(s >> Member< uint32_t >(L"size", size)))
				return false;

			if (size > 0)
			{
				m_ref = sceCgcNewBin();
				if (sceCgcStoreBinData(m_ref, 0, size) != SCECGC_OK)
					return false;

				s >> Member< void* >(L"data", sceCgcGetBinData(m_ref), size);
			}
			else
				m_ref = 0;
		}
		else	// SdWrite
		{
			uint32_t size = m_ref ? sceCgcGetBinSize(m_ref) : 0;
			if (!(s >> Member< uint32_t >(L"size", size)))
				return false;

			if (size > 0)
				s >> Member< void* >(L"data", sceCgcGetBinData(m_ref), size);
		}
		return true;
	}

private:
	CGCbin*& m_ref;
};

class MemberFragmentOffset : public MemberComplex
{
public:
	MemberFragmentOffset(const std::wstring& name, FragmentOffset& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< uint32_t >(L"ucodeOffset", m_ref.ucodeOffset);
		s >> Member< uint32_t >(L"parameterOffset", m_ref.parameterOffset);
		return true;
	}

private:
	FragmentOffset& m_ref;
};

class MemberProgramScalar : public MemberComplex
{
public:
	MemberProgramScalar(const std::wstring& name, ProgramScalar& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< uint16_t >(L"vertexRegisterIndex", m_ref.vertexRegisterIndex);
		s >> Member< uint16_t >(L"vertexRegisterCount", m_ref.vertexRegisterCount);
		s >> MemberStlVector< FragmentOffset, MemberFragmentOffset >(L"fragmentOffsets", m_ref.fragmentOffsets);
		s >> Member< uint16_t >(L"offset", m_ref.offset);
		s >> Member< uint16_t >(L"length", m_ref.length);
		return true;
	}

private:
	ProgramScalar& m_ref;
};

class MemberProgramSampler : public MemberComplex
{
public:
	MemberProgramSampler(const std::wstring& name, ProgramSampler& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< uint16_t >(L"stage", m_ref.stage);
		s >> Member< uint16_t >(L"texture", m_ref.texture);
		return true;
	}

private:
	ProgramSampler& m_ref;
};

class MemberSamplerState : public MemberComplex
{
public:
	MemberSamplerState(const std::wstring& name, SamplerState& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< uint8_t >(L"minFilter", m_ref.minFilter);
		s >> Member< uint8_t >(L"magFilter", m_ref.magFilter);
		s >> Member< uint8_t >(L"wrapU", m_ref.wrapU);
		s >> Member< uint8_t >(L"wrapV", m_ref.wrapV);
		s >> Member< uint8_t >(L"wrapW", m_ref.wrapW);
		return true;
	}

private:
	SamplerState& m_ref;
};

class MemberRenderState : public MemberComplex
{
public:
	MemberRenderState(const std::wstring& name, RenderState& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< uint32_t >(L"cullFaceEnable", m_ref.cullFaceEnable);
		s >> Member< uint32_t >(L"cullFace", m_ref.cullFace);
		s >> Member< uint32_t >(L"blendEnable", m_ref.blendEnable);
		s >> Member< uint16_t >(L"blendEquation", m_ref.blendEquation);
		s >> Member< uint16_t >(L"blendFuncSrc", m_ref.blendFuncSrc);
		s >> Member< uint16_t >(L"blendFuncDest", m_ref.blendFuncDest);
		s >> Member< uint32_t >(L"depthTestEnable", m_ref.depthTestEnable);
		s >> Member< uint32_t >(L"colorMask", m_ref.colorMask);
		s >> Member< uint32_t >(L"depthMask", m_ref.depthMask);
		s >> Member< uint32_t >(L"depthFunc", m_ref.depthFunc);
		s >> Member< uint32_t >(L"alphaTestEnable", m_ref.alphaTestEnable);
		s >> Member< uint32_t >(L"alphaFunc", m_ref.alphaFunc);
		s >> Member< uint32_t >(L"alphaRef", m_ref.alphaRef);
		s >> MemberStaticArray< SamplerState, 8, MemberSamplerState >(L"samplerStates", m_ref.samplerStates);
		return true;
	}

private:
	RenderState& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourcePs3", 0, ProgramResourcePs3, ProgramResource)

ProgramResourcePs3::ProgramResourcePs3()
:	m_vertexShaderBin(0)
,	m_pixelShaderBin(0)
{
}

ProgramResourcePs3::~ProgramResourcePs3()
{
	if (m_vertexShaderBin)
		sceCgcDeleteBin(m_vertexShaderBin);
	if (m_pixelShaderBin)
		sceCgcDeleteBin(m_pixelShaderBin);
}

bool ProgramResourcePs3::serialize(ISerializer& s)
{
	s >> MemberBin(L"vertexProgramBin", m_vertexShaderBin);
	s >> MemberBin(L"pixelProgramBin", m_pixelShaderBin);
	s >> MemberStlVector< ProgramScalar, MemberProgramScalar >(L"vertexScalars", m_vertexScalars);
	s >> MemberStlVector< ProgramScalar, MemberProgramScalar >(L"pixelScalars", m_pixelScalars);
	s >> MemberStlVector< ProgramSampler, MemberProgramSampler >(L"vertexSamplers", m_vertexSamplers);
	s >> MemberStlVector< ProgramSampler, MemberProgramSampler >(L"pixelSamplers", m_pixelSamplers);
	s >> MemberStlMap< std::wstring, uint32_t >(L"scalarParameterMap", m_scalarParameterMap);
	s >> MemberStlMap< std::wstring, uint32_t >(L"textureParameterMap", m_textureParameterMap);
	s >> Member< uint32_t >(L"scalarParameterDataSize", m_scalarParameterDataSize);
	s >> Member< uint32_t >(L"textureParameterDataSize", m_textureParameterDataSize);
	s >> MemberRenderState(L"renderState", m_renderState);
	return true;
}

	}
}
