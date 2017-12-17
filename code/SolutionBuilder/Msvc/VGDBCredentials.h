/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef VGDBCredentials_H
#define VGDBCredentials_H

#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace sb
	{

class VGDBCredentials : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

	const std::wstring& getHost() const { return m_host; }

	const std::wstring& getUser() const { return m_user; }

	const std::wstring& getLocalPath() const { return m_localPath; }

	const std::wstring& getRemotePath() const { return m_remotePath; }

private:
	std::wstring m_host;
	std::wstring m_user;
	std::wstring m_localPath;
	std::wstring m_remotePath;
};

	}
}

#endif	// VGDBCredentials_H
