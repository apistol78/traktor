#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Shader.h"
#include "Render/Editor/Image2/ImgStepSimple.h"
#include "Resource/Member.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ImgStepSimple", 0, ImgStepSimple, IImgStep)

void ImgStepSimple::getInputs(std::set< std::wstring >& outInputs) const
{
    for (const auto& source : m_sources)
        outInputs.insert(source.targetSetId);
}

void ImgStepSimple::serialize(ISerializer& s)
{
    s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberAlignedVector< Source, MemberComposite< Source > >(L"sources", m_sources);
}

ImgStepSimple::Source::Source()
:	colorIndex(0)
{
}

void ImgStepSimple::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"parameter", parameter);
	s >> Member< std::wstring >(L"targetSetId", targetSetId);
	s >> Member< int32_t >(L"colorIndex", colorIndex);
}

    }
}
