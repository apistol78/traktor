#ifndef VGDBCredentials_H
#define VGDBCredentials_H

#include <Core/Serialization/ISerializable.h>

class VGDBCredentials : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(traktor::ISerializer& s);

	const std::wstring& getHost() const { return m_host; }

	const std::wstring& getUser() const { return m_user; }

private:
	std::wstring m_host;
	std::wstring m_user;
};

#endif	// VGDBCredentials_H
