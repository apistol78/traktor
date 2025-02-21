/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/DeepHash.h"

#include "Core/Containers/SmallMap.h"
#include "Core/Guid.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Path.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector2.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Serialization/AttributeNoHash.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/Serializer.h"

namespace traktor
{
namespace
{

template < typename AttributeType, typename MemberType >
const AttributeType* findAttribute(const MemberType& m)
{
	const Attribute* attributes = m.getAttributes();
	return attributes ? attributes->find< AttributeType >() : nullptr;
}

class HashSerializer : public Serializer
{
public:
	HashSerializer(Murmur3& hasher)
		: m_hasher(hasher)
		, m_writtenCount(0)
	{
	}

	virtual Direction getDirection() const override final
	{
		return Direction::Write;
	}

	virtual void operator>>(const Member< bool >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			m_hasher.feed< uint8_t >(m ? 0xff : 0x00);
	}

	virtual void operator>>(const Member< int8_t >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			m_hasher.feed< int8_t >(m);
	}

	virtual void operator>>(const Member< uint8_t >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			m_hasher.feed< uint8_t >(m);
	}

	virtual void operator>>(const Member< int16_t >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			m_hasher.feed< int64_t >(m);
	}

	virtual void operator>>(const Member< uint16_t >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			m_hasher.feed< uint16_t >(m);
	}

	virtual void operator>>(const Member< int32_t >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			m_hasher.feed< int32_t >(m);
	}

	virtual void operator>>(const Member< uint32_t >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			m_hasher.feed< uint32_t >(m);
	}

	virtual void operator>>(const Member< int64_t >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			m_hasher.feed< int64_t >(m);
	}

	virtual void operator>>(const Member< uint64_t >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			m_hasher.feed< uint64_t >(m);
	}

	virtual void operator>>(const Member< float >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			m_hasher.feed< float >(m);
	}

	virtual void operator>>(const Member< double >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			m_hasher.feed< double >(m);
	}

	virtual void operator>>(const Member< std::string >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
		{
			const std::string& str = m;
			m_hasher.feedBuffer(str.data(), str.size());
		}
	}

	virtual void operator>>(const Member< std::wstring >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
		{
			const std::wstring& str = m;
			m_hasher.feed(str);
		}
	}

	virtual void operator>>(const Member< Guid >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
		{
			const Guid& guid = m;
			m_hasher.feedBuffer((const uint8_t*)guid, 16);
		}
	}

	virtual void operator>>(const Member< Path >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
		{
			const std::wstring path = m->getOriginal();
			m_hasher.feed(path);
		}
	}

	virtual void operator>>(const Member< Color4ub >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
		{
			m_hasher.feed< uint8_t >(m->r);
			m_hasher.feed< uint8_t >(m->g);
			m_hasher.feed< uint8_t >(m->b);
			m_hasher.feed< uint8_t >(m->a);
		}
	}

	virtual void operator>>(const Member< Color4f >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
		{
			float T_MATH_ALIGN16 e[4];
			m->storeAligned(e);
			m_hasher.feedBuffer(e, sizeof(e));
		}
	}

	virtual void operator>>(const Member< Scalar >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
		{
			const float v = (float)(const Scalar&)m;
			m_hasher.feed< float >(v);
		}
	}

	virtual void operator>>(const Member< Vector2 >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
		{
			m_hasher.feed< float >(m->x);
			m_hasher.feed< float >(m->y);
		}
	}

	virtual void operator>>(const Member< Vector4 >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
		{
			float T_MATH_ALIGN16 e[4];
			m->storeAligned(e);
			m_hasher.feedBuffer(e, sizeof(e));
		}
	}

	virtual void operator>>(const Member< Matrix33 >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			for (int i = 0; i < 3 * 3; ++i)
				m_hasher.feed< float >(m->m[i]);
	}

	virtual void operator>>(const Member< Matrix44 >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
		{
			float T_MATH_ALIGN16 e[16];
			m->storeAligned(e);
			m_hasher.feedBuffer(e, sizeof(e));
		}
	}

	virtual void operator>>(const Member< Quaternion >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
		{
			float T_MATH_ALIGN16 e[4];
			m->e.storeAligned(e);
			m_hasher.feedBuffer(e, sizeof(e));
		}
	}

	virtual void operator>>(const Member< ISerializable* >& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
		{
			ISerializable* object = *m;
			if (object)
			{
				auto it = m_written.find(object);
				if (it == m_written.end())
				{
					m_written.insert(std::make_pair(object, ++m_writtenCount));

					const wchar_t* const typeName = type_of(object).getName();
					for (size_t i = 0; typeName[i]; ++i)
					{
						uint32_t ch = (uint32_t)typeName[i];
						m_hasher.feed(ch);
					}

					serialize(object);
				}
				else
					m_hasher.feed< uint32_t >(it->second);
			}
			else
				m_hasher.feed< uint32_t >(0);
		}
	}

	virtual void operator>>(const Member< void* >& m) override final
	{
		m_hasher.feedBuffer(m.getBlob(), m.getBlobSize());
	}

	virtual void operator>>(const MemberArray& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
		{
			const size_t size = m.size();
			m.write(*this, size);
		}
	}

	virtual void operator>>(const MemberComplex& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			m.serialize(*this);
	}

	virtual void operator>>(const MemberEnumBase& m) override final
	{
		if (findAttribute< AttributeNoHash >(m) == nullptr)
			m.serialize(*this);
	}

private:
	Murmur3& m_hasher;
	SmallMap< ISerializable*, uint32_t > m_written;
	uint32_t m_writtenCount;
};

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.DeepHash", DeepHash, Object)

DeepHash::DeepHash(const ISerializable* object)
	: m_hash(0)
{
	if (object)
	{
		Murmur3 a;
		a.begin();
		HashSerializer(a).writeObject(object);
		a.end();
		m_hash = a.get();
	}
}

uint32_t DeepHash::get() const
{
	return m_hash;
}

bool DeepHash::operator==(const DeepHash& hash) const
{
	return m_hash == hash.m_hash;
}

bool DeepHash::operator!=(const DeepHash& hash) const
{
	return m_hash != hash.m_hash;
}

bool DeepHash::operator==(const DeepHash* hash) const
{
	return m_hash == hash->m_hash;
}

bool DeepHash::operator!=(const DeepHash* hash) const
{
	return m_hash != hash->m_hash;
}

bool DeepHash::operator==(const ISerializable* object) const
{
	DeepHash hash(object);
	return m_hash == hash.m_hash;
}

bool DeepHash::operator!=(const ISerializable* object) const
{
	DeepHash hash(object);
	return m_hash != hash.m_hash;
}

bool DeepHash::operator==(uint32_t hash) const
{
	return m_hash == hash;
}

bool DeepHash::operator!=(uint32_t hash) const
{
	return m_hash != hash;
}

}
