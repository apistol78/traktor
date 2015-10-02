#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Resource/Member.h"
#include "Spark/Font.h"
#include "Spark/Text.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.Text", 0, Text, Character)

Text::Text()
:	m_height(0.0f)
,	m_origin(0.0f, 0.0f)
,	m_size(0.0f, 0.0f)
,	m_horizontalAlign(AnLeft)
,	m_verticalAlign(AnTop)
{
}

void Text::serialize(ISerializer& s)
{
	const MemberEnum< Align >::Key c_horizontalAlign_Keys[] =
	{
		{ L"AnLeft", AnLeft },
		{ L"AnCenter", AnCenter },
		{ L"AnRight", AnRight },
		{ 0 }
	};
	const MemberEnum< Align >::Key c_verticalAlign_Keys[] =
	{
		{ L"AnTop", AnTop },
		{ L"AnCenter", AnCenter },
		{ L"AnBottom", AnBottom },
		{ 0 }
	};

	Character::serialize(s);

	s >> resource::Member< Font >(L"font", m_font);
	s >> Member< std::wstring >(L"text", m_text);
	s >> Member< float >(L"height", m_height);
	s >> Member< Vector2 >(L"origin", m_origin);
	s >> Member< Vector2 >(L"size", m_size);
	s >> MemberEnum< Align >(L"horizontalAlign", m_horizontalAlign, c_horizontalAlign_Keys);
	s >> MemberEnum< Align >(L"verticalAlign", m_verticalAlign, c_verticalAlign_Keys);
}

	}
}
