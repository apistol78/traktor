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

	virtual void serialize(ISerializer& s) const
	{
		const MemberEnum< CullMode >::Key kCullMode[] =
		{
			{ L"CmNever", CmNever },
			{ L"CmClockWise", CmClockWise },
			{ L"CmCounterClockWise", CmCounterClockWise },
			{ 0, 0 }
		};

		const MemberEnum< BlendOperation >::Key kBlendOperations[] =
		{
			{ L"BoAdd", BoAdd },
			{ L"BoSubtract", BoSubtract },
			{ L"BoReverseSubtract", BoReverseSubtract },
			{ L"BoMin", BoMin },
			{ L"BoMax", BoMax },
			{ 0, 0 }
		};

		const MemberEnum< BlendFactor >::Key kBlendFactors[] =
		{
			{ L"BfOne", BfOne },
			{ L"BfZero", BfZero },
			{ L"BfSourceColor", BfSourceColor },
			{ L"BfOneMinusSourceColor", BfOneMinusSourceColor },
			{ L"BfDestinationColor", BfDestinationColor },
			{ L"BfOneMinusDestinationColor", BfOneMinusDestinationColor },
			{ L"BfSourceAlpha", BfSourceAlpha },
			{ L"BfOneMinusSourceAlpha", BfOneMinusSourceAlpha },
			{ L"BfDestinationAlpha", BfDestinationAlpha },
			{ L"BfOneMinusDestinationAlpha", BfOneMinusDestinationAlpha },
			{ 0, 0 }
		};

		s >> Member< bool >(L"depthEnable", m_ref.depthEnable);
		s >> Member< bool >(L"depthWriteEnable", m_ref.depthWriteEnable);
		s >> MemberEnum< CullMode >(L"cullMode", m_ref.cullMode, kCullMode);
		s >> Member< bool >(L"blendEnable", m_ref.blendEnable);
		s >> MemberEnum< BlendOperation >(L"blendOperation", m_ref.blendOperation, kBlendOperations);
		s >> MemberEnum< BlendFactor >(L"blendSource", m_ref.blendSource, kBlendFactors);
		s >> MemberEnum< BlendFactor >(L"blendDestination", m_ref.blendDestination, kBlendFactors);
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

void ProgramResourceSw::serialize(ISerializer& s)
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
}

	}
}
