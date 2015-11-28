#ifndef traktor_MemberAny_H
#define traktor_MemberAny_H

#include "Core/Class/Any.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor
{

/*! \brief
 * \ingroup Core
 */
class MemberAny : public MemberComplex
{
public:
	MemberAny(const wchar_t* const name, Any& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		const MemberEnum< Any::AnyType >::Key c_AnyType_Keys[] =
		{
			{ L"AtVoid", Any::AtVoid },
			{ L"AtBoolean", Any::AtBoolean },
			{ L"AtInteger", Any::AtInteger },
			{ L"AtFloat", Any::AtFloat },
			{ L"AtString", Any::AtString },
			{ L"AtObject", Any::AtObject },
			{ 0 }
		};

		Any::AnyType type = m_ref.getType();
		s >> MemberEnum< Any::AnyType >(L"type", type, c_AnyType_Keys);

		if (s.getDirection() == ISerializer::SdRead)
		{
			switch (type)
			{
			case Any::AtVoid:
				break;
			case Any::AtBoolean:
				{
					bool value;
					s >> Member< bool >(L"value", value);
					m_ref = Any::fromBoolean(value);
				}
				break;
			case Any::AtInteger:
				{
					int32_t value;
					s >> Member< int32_t >(L"value", value);
					m_ref = Any::fromInteger(value);
				}
				break;
			case Any::AtFloat:
				{
					float value;
					s >> Member< float >(L"value", value);
					m_ref = Any::fromFloat(value);
				}
				break;
			case Any::AtString:
				{
					std::wstring value;
					s >> Member< std::wstring >(L"value", value);
					m_ref = Any::fromString(value);
				}
				break;
			case Any::AtObject:
				{
					Ref< ISerializable > value;
					s >> Member< ISerializable* >(L"value", value);
					m_ref = Any::fromObject(value);
				}
				break;
			}
		}
		else
		{
			switch (type)
			{
			case Any::AtVoid:
				break;
			case Any::AtBoolean:
				{
					bool value = m_ref.getBoolean();
					s >> Member< bool >(L"value", value);
				}
				break;
			case Any::AtInteger:
				{
					int32_t value = m_ref.getInteger();
					s >> Member< int32_t >(L"value", value);
				}
				break;
			case Any::AtFloat:
				{
					float value = m_ref.getFloat();
					s >> Member< float >(L"value", value);
				}
				break;
			case Any::AtString:
				{
					std::wstring value = m_ref.getWideString();
					s >> Member< std::wstring >(L"value", value);
				}
				break;
			case Any::AtObject:
				{
					Ref< ISerializable > value = dynamic_type_cast< ISerializable* >(m_ref.getObject());
					s >> Member< ISerializable* >(L"value", value);
				}
				break;
			}			
		}
	}

private:
	Any& m_ref;
};

}

#endif	// traktor_MemberAny_H
