#include "Core/Serialization/ISerializer.h"
#include "Render/Shader.h"
#include "Render/Editor/Image2/ImgStepSimple.h"
#include "Resource/Member.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ImgStepSimple", 0, ImgStepSimple, IImgStep)

void ImgStepSimple::serialize(ISerializer& s)
{
    s >> resource::Member< render::Shader >(L"shader", m_shader);
}

    }
}
