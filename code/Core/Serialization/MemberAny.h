#pragma once

#include "Core/Class/Any.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor
{

/*!
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

	virtual void serialize(ISerializer& s) const override final
	{
		const MemberEnum< Any::Type >::Key c_AnyType_Keys[] =
		{
			{ L"Void", Any::Type::Void },
			{ L"Boolean", Any::Type::Boolean },
			{ L"Int32", Any::Type::Int32 },
			{ L"Float", Any::Type::Float },
			{ L"String", Any::Type::String },
			{ L"Object", Any::Type::Object },
			{ 0 }
		};

		Any::AnyType type = m_ref.getType();
		s >> MemberEnum< Any::Type >(L"type", type, c_AnyType_Keys);

		if (s.getDirection() == ISerializer::Direction::Read)
		{
			switch (type)
			{
			case Any::Type::Void:
				break;
			case Any::Type::Boolean:
				{
					bool value;
					s >> Member< bool >(L"value", value);
					m_ref = Any::fromBoolean(value);
				}
				break;
			case Any::Type::Int32:
				{
					int32_t value;
					s >> Member< int32_t >(L"value", value);
					m_ref = Any::fromInt32(value);
				}
				break;
			case Any::Type::Float:
				{
					float value;
					s >> Member< float >(L"value", value);
					m_ref = Any::fromFloat(value);
				}
				break;
			case Any::Type::String:
				{
					std::wstring value;
					s >> Member< std::wstring >(L"value", value);
					m_ref = Any::fromString(value);
				}
				break;
			case Any::Type::Object:
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
			case Any::Type::Void:
				break;
			case Any::Type::Boolean:
				{
					bool value = m_ref.getBoolean();
					s >> Member< bool >(L"value", value);
				}
				break;
			case Any::Type::Int32:
				{
					int32_t value = m_ref.getInt32();
					s >> Member< int32_t >(L"value", value);
				}
				break;
			case Any::Type::Float:
				{
					float value = m_ref.getFloat();
					s >> Member< float >(L"value", value);
				}
				break;
			case Any::Type::String:
				{
					std::wstring value = m_ref.getWideString();
					s >> Member< std::wstring >(L"value", value);
				}
				break;
			case Any::Type::Object:
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

