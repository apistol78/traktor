#include "Avalanche/Key.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace avalanche
	{

Key::Key()
:	m_kv(0, 0, 0, 0)
{
}

Key::Key(uint32_t k0, uint32_t k1, uint32_t k2, uint32_t k3)
:	m_kv(k0, k1, k2, k3)
{
}

bool Key::valid() const
{
	return !(*this == Key(0, 0, 0, 0));
}

std::wstring Key::format() const
{
	return str(L"%08x:%08x:%08x:%08x", std::get< 0 >(m_kv), std::get< 1 >(m_kv), std::get< 2 >(m_kv), std::get< 3 >(m_kv));
}

Key Key::read(IStream* stream)
{
	uint32_t kv[4];
	if (stream->read(kv, sizeof(kv)) == sizeof(kv))
		return Key(kv[0], kv[1], kv[2], kv[3]);
	else
		return Key();
}

bool Key::write(IStream* stream) const
{
	uint32_t kv[4] = { std::get< 0 >(m_kv), std::get< 1 >(m_kv), std::get< 2 >(m_kv), std::get< 3 >(m_kv) };
	return stream->write(kv, sizeof(kv)) == sizeof(kv);
}

bool Key::operator == (const Key& rh) const
{
	return m_kv == rh.m_kv;
}

bool Key::operator < (const Key& rh) const
{
	return m_kv < rh.m_kv;
}

bool Key::operator > (const Key& rh) const
{
	return m_kv > rh.m_kv;
}

	}
}
