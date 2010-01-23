#include "Core/Misc/WildCompare.h"
#include "Core/Misc/Split.h"

namespace traktor
{

WildCompare::WildCompare(const std::wstring& mask)
:	m_beginWild(false)
,	m_endWild(false)
{
	if (mask.length() > 0)
	{
		Split< std::wstring >::any(mask, L"*", m_nowild);
		m_beginWild = bool(mask[0] == L'*');
		m_endWild = bool(mask[mask.length() - 1] == L'*');
	}
}

bool WildCompare::match(const std::wstring& str, CompareMode mode, std::vector< std::wstring >* outPieces) const
{
	size_t p = 0;
	size_t i = 0;

	if (!m_nowild.size())
		return m_beginWild;
	
	if (!m_beginWild)
	{
		size_t ln = m_nowild[0].length();

		if (str.compare(0, ln, m_nowild[0]) != 0)
			return false;

		p = ln;
		i = 1;
	}

	for (; i < m_nowild.size(); ++i)
	{
		size_t f = str.find(m_nowild[i], p);
		if (f == std::wstring::npos)
			return false;
			
		if (outPieces)
			outPieces->push_back(str.substr(p, f - p));

		p = f + m_nowild[i].length();
	}

	if (!m_endWild)
	{
		if (p < str.length())
			return false;
	}
	else if (outPieces)
		outPieces->push_back(str.substr(p));

	return true;
}

std::wstring WildCompare::merge(const std::vector< std::wstring >& pieces) const
{
	std::vector< std::wstring >::const_iterator i = pieces.begin();
	std::wstring merged;
	
	if (m_beginWild && i != pieces.end())
	{
		merged += *i;
		++i;
	}
	
	for (std::vector< std::wstring >::const_iterator j = m_nowild.begin(); j != m_nowild.end(); ++j)
	{
		merged += *j;
		if (i != pieces.end())
		{
			merged += *i;
			++i;
		}
	}
		
	if (m_endWild && i != pieces.end())
		merged += *i;
	
	return merged;
}

}
