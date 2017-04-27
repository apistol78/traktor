/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstdlib>
#include <numeric>
#include "Core/Io/Path.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Core/System/OS.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Path", Path, Object)

Path::Path()
:	m_relative(false)
{
}

Path::Path(const Path& path)
:	m_original(path.m_original)
,	m_volume(path.m_volume)
,	m_relative(path.m_relative)
,	m_path(path.m_path)
,	m_file(path.m_file)
,	m_ext(path.m_ext)
{
}

Path::Path(const std::wstring& path)
:	m_original(path)
{
	resolve();
}

Path::Path(const wchar_t* path)
:	m_original(path)
{
	resolve();
}

bool Path::empty() const
{
	return m_original.empty();
}

std::wstring Path::getOriginal() const
{
	return m_original;
}

bool Path::hasVolume() const
{
	return bool(m_volume.empty() == false);
}

std::wstring Path::getVolume() const
{
	return m_volume;
}

bool Path::isRelative() const
{
	return m_relative;
}

std::wstring Path::getFileName() const
{
	return m_file;
}

std::wstring Path::getFileNameNoExtension() const
{
	std::wstring::size_type ext = m_file.find_last_of(L'.');
	return (ext != std::wstring::npos) ? m_file.substr(0, ext) : m_file; 
}

std::wstring Path::getPathOnly() const
{
	std::wstringstream ss;

	if (!m_volume.empty())
		ss << m_volume << L":";
	if (!m_relative)
		ss << L"/";

	if (!m_path.empty())
		ss << m_path;

	return ss.str();
}

std::wstring Path::getPathOnlyNoVolume() const
{
	std::wstringstream ss;

	if (!m_relative)
		ss << L"/";

	if (!m_path.empty())
		ss << m_path;

	return ss.str();
}

std::wstring Path::getPathName() const
{
	std::wstringstream ss;

	if (!m_volume.empty())
		ss << m_volume << L":";
	if (!m_relative)
		ss << L"/";

	if (!m_path.empty())
	{
		ss << m_path;
		if (!m_file.empty())
			ss << L"/";
	}
	if (!m_file.empty())
		ss << m_file;

	return ss.str();
}

std::wstring Path::getPathNameNoExtension() const
{
	std::wstring pathName = getPathName();
	std::wstring::size_type ext = pathName.find_last_of(L'.');
	return (ext != std::wstring::npos) ? pathName.substr(0, ext) : pathName; 
}

std::wstring Path::getExtension() const
{
	return m_ext;
}

std::wstring Path::getPathNameNoVolume() const
{
	std::wstringstream ss;

	if (!m_relative)
		ss << L"/";

	if (!m_path.empty())
	{
		ss << m_path;
		if (!m_file.empty())
			ss << L"/";
	}
	if (!m_file.empty())
		ss << m_file;

	return ss.str();
}

Path Path::normalized() const
{
	std::vector< std::wstring > p;
	p.reserve(32);

	StringSplit< std::wstring > ss(getPathNameNoVolume(), L"/");
	for (StringSplit< std::wstring >::const_iterator i = ss.begin(); i != ss.end(); ++i)
	{
		if (*i == L".")
			continue;

		if (*i == L".." && !p.empty())
		{
			p.pop_back();
			continue;
		}

		p.push_back(*i);
	}

	StringOutputStream s;

	if (hasVolume())
		s << getVolume() << L":";

	if (!p.empty())
	{
		if (!isRelative())
			s << L"/";
		for (std::vector< std::wstring >::const_iterator i = p.begin(); i != p.end() - 1; ++i)
			s << *i << L"/";
		s << p.back();
	}

	return Path(s.str());
}

Path Path::operator + (const Path& rh) const
{
	if (!rh.isRelative())
		return rh;

	return Path(getPathName() + L"/" + rh.getPathName());
}

bool Path::operator == (const Path& rh) const
{
	std::wstring pl = getPathName();
	std::wstring pr = rh.getPathName();
#if defined(_WIN32)
	return compareIgnoreCase(pl, pr) == 0;
#else
	return pl.compare(pr) == 0;
#endif
}

bool Path::operator < (const Path& rh) const
{
	return bool(getPathName() < rh.getPathName());
}

void Path::resolve()
{
	std::wstring tmp = replaceAll(m_original, L'\\', L'/');
	std::wstring env;

	for (;;)
	{
		size_t s = tmp.find(L"$(");
		if (s == std::string::npos)
			break;

		size_t e = tmp.find(L")", s + 2);
		if (e == std::string::npos)
			break;

		std::wstring name = tmp.substr(s + 2, e - s - 2);
		
		if (OS::getInstance().getEnvironment(name, env))
			tmp = tmp.substr(0, s) + replaceAll< std::wstring >(env, L'\\', L'/') + tmp.substr(e + 1);
		else
			tmp = tmp.substr(0, s) + tmp.substr(e + 1);
	}
	
	std::wstring::size_type vol = tmp.find(L':');
	if (vol != std::wstring::npos)
	{
		m_volume = toLower(tmp.substr(0, vol));
		tmp = tmp.substr(vol + 1);
	}

	m_relative = true;
	if (!tmp.empty())
	{
		if (tmp[0] == L'/')
		{
			m_relative = false;
			tmp = tmp.substr(1);
		}
		else if (tmp[0] == L'~')
			m_relative = false;
	}
	
	std::wstring::size_type sls = tmp.find_last_of(L'/');
	if (sls != std::wstring::npos)
	{
		m_path = replaceAll< std::wstring >(tmp.substr(0, sls), L"//", L"/");
		tmp = tmp.substr(sls + 1);
	}
	
	m_file = tmp;
	
	std::wstring::size_type ext = tmp.find_last_of(L'.');
	if (ext != std::wstring::npos)
		m_ext = toLower(tmp.substr(ext + 1));
}

}
