#pragma once

#if 0
#	include "Core/Serialization/DeepHash.h"
#	include "Core/Serialization/ISerializable.h"

namespace traktor
{

class ImmutableCheck
{
public:
	ImmutableCheck(const ISerializable* imm)
	:	m_imm(imm)
	{
		m_hash = DeepHash(m_imm).get();
	}

	~ImmutableCheck()
	{
		uint32_t hash = DeepHash(m_imm).get();
		T_FATAL_ASSERT_M(hash == m_hash, L"Immutable object modified.");
	}

private:
	const ISerializable* m_imm;
	uint32_t m_hash;
};

}

#	define T_IMMUTABLE_CHECK(imm) T_ANONYMOUS_VAR(ImmutableCheck)(imm);
#else
#	define T_IMMUTABLE_CHECK(imm)
#endif
