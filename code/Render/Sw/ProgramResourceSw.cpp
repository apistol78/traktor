#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Sw/ProgramResourceSw.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class MemberRenderStateDesc : public MemberComplex
{
public:
	MemberRenderStateDesc(const wchar_t* const name, RenderStateDesc& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		const MemberEnum< PixelOutput::CullMode >::Key kCullMode[] =
		{
			{ L"CmNever", PixelOutput::CmNever },
			{ L"CmClockWise", PixelOutput::CmClockWise },
			{ L"CmCounterClockWise", PixelOutput::CmCounterClockWise },
			{ 0, 0 }
		};

		const MemberEnum< PixelOutput::BlendOperation >::Key kBlendOperations[] =
		{
			{ L"BoAdd", PixelOutput::BoAdd },
			{ L"BoSubtract", PixelOutput::BoSubtract },
			{ L"BoReverseSubtract", PixelOutput::BoReverseSubtract },
			{ L"BoMin", PixelOutput::BoMin },
			{ L"BoMax", PixelOutput::BoMax },
			{ 0, 0 }
		};

		const MemberEnum< PixelOutput::BlendFactor >::Key kBlendFactors[] =
		{
			{ L"BfOne", PixelOutput::BfOne },
			{ L"BfZero", PixelOutput::BfZero },
			{ L"BfSourceColor", PixelOutput::BfSourceColor },
			{ L"BfOneMinusSourceColor", PixelOutput::BfOneMinusSourceColor },
			{ L"BfDestinationColor", PixelOutput::BfDestinationColor },
			{ L"BfOneMinusDestinationColor", PixelOutput::BfOneMinusDestinationColor },
			{ L"BfSourceAlpha", PixelOutput::BfSourceAlpha },
			{ L"BfOneMinusSourceAlpha", PixelOutput::BfOneMinusSourceAlpha },
			{ L"BfDestinationAlpha", PixelOutput::BfDestinationAlpha },
			{ L"BfOneMinusDestinationAlpha", PixelOutput::BfOneMinusDestinationAlpha },
			{ 0, 0 }
		};

		s >> Member< bool >(L"depthEnable", m_ref.depthEnable);
		s >> Member< bool >(L"depthWriteEnable", m_ref.depthWriteEnable);
		s >> MemberEnum< PixelOutput::CullMode >(L"cullMode", m_ref.cullMode, kCullMode);
		s >> Member< bool >(L"blendEnable", m_ref.blendEnable);
		s >> MemberEnum< PixelOutput::BlendOperation >(L"blendOperation", m_ref.blendOperation, kBlendOperations);
		s >> MemberEnum< PixelOutput::BlendFactor >(L"blendSource", m_ref.blendSource, kBlendFactors);
		s >> MemberEnum< PixelOutput::BlendFactor >(L"blendDestination", m_ref.blendDestination, kBlendFactors);

		return true;
	}

private:
	RenderStateDesc& m_ref;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceSw", 1, ProgramResourceSw, ProgramResource)

ProgramResourceSw::ProgramResourceSw()
:	m_interpolatorCount(0)
{
}

ProgramResourceSw::ProgramResourceSw(
	const IntrProgram& vertexProgram,
	const IntrProgram& pixelProgram,
	const std::map< std::wstring, std::pair< int32_t, int32_t > >& parameterMap,
	const std::map< std::wstring, int32_t >& samplerMap,
	const RenderStateDesc& renderState,
	uint32_t interpolatorCount
)
:	m_vertexProgram(vertexProgram)
,	m_pixelProgram(pixelProgram)
,	m_parameterMap(parameterMap)
,	m_samplerMap(samplerMap)
,	m_renderState(renderState)
,	m_interpolatorCount(interpolatorCount)
{
}

bool ProgramResourceSw::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 1);

	s >> MemberComposite< IntrProgram >(L"vertexProgram", m_vertexProgram);
	s >> MemberComposite< IntrProgram >(L"pixelProgram", m_pixelProgram);
	s >> MemberStlMap<
		std::wstring,
		std::pair< int32_t, int32_t >,
		MemberStlPair<
			std::wstring,
			std::pair< int32_t, int32_t >,
			Member< std::wstring >,
			MemberStlPair< int32_t, int32_t >
		>
	>(L"parameterMap", m_parameterMap);
	s >> MemberStlMap<
		std::wstring,
		int32_t,
		MemberStlPair<
			std::wstring,
			int32_t
		>
	>(L"samplerMap", m_samplerMap);

	s >> MemberRenderStateDesc(L"renderState", m_renderState);
	s >> Member< uint32_t >(L"interpolatorCount", m_interpolatorCount);

	return true;
}

	}
}
