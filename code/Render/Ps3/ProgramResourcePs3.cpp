#include "Render/Ps3/ProgramResourcePs3.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberComposite.h"
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

ProgramResourcePs3::ProgramResourcePs3(
	CGCbin* vertexShaderBin,
	CGCbin* pixelShaderBin,
	const std::vector< Parameter >& parameters,
	const RenderState& renderState
)
:	m_vertexShaderBin(vertexShaderBin)
,	m_pixelShaderBin(pixelShaderBin)
,	m_parameters(parameters)
,	m_renderState(renderState)
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
	if (!ProgramResource::serialize(s))
		return false;

	s >> MemberBin(L"vertexProgramBin", m_vertexShaderBin);
	s >> MemberBin(L"pixelProgramBin", m_pixelShaderBin);
	s >> MemberStlVector< Parameter, MemberComposite< Parameter > >(L"parameters", m_parameters);
	s >> MemberRenderState(L"renderState", m_renderState);

	return true;
}

bool ProgramResourcePs3::Parameter::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< bool >(L"sampler", sampler);
	s >> Member< int32_t >(L"size", size);
	s >> Member< int32_t >(L"count", count);
	return true;
}

	}
}
