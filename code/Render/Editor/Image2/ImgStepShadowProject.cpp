#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Shader.h"
#include "Render/Editor/Image2/ImgStepShadowProject.h"
#include "Resource/Member.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ImgStepShadowProject", 0, ImgStepShadowProject, IImgStep)

std::wstring ImgStepShadowProject::getTitle() const
{
    return L"Shadow Project";
}

void ImgStepShadowProject::getInputs(std::set< std::wstring >& outInputs) const
{
    outInputs.insert(L"ShadowMap");
    for (const auto& parameter : m_parameters)
        outInputs.insert(parameter);
}

void ImgStepShadowProject::serialize(ISerializer& s)
{
    s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlList< std::wstring >(L"parameters", m_parameters);
}

    }
}
