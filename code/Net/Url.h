#ifndef traktor_net_Url_H
#define traktor_net_Url_H

#include <string>
#include <vector>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

/*! \brief Unified resource locator.
 * \ingroup Net
 */
class T_DLLCLASS Url : public Object
{
	T_RTTI_CLASS;

public:
	Url();

	Url(const std::wstring& spec);

	bool set(const std::wstring& spec);
	
	bool valid() const;

	int getDefaultPort() const;

	std::wstring getFile() const;

	std::wstring getHost() const;

	std::wstring getPath() const;

	int getPort() const;

	std::wstring getProtocol() const;

	std::wstring getQuery() const;

	std::wstring getRef() const;

	std::wstring getUserInfo() const;

	std::wstring getString() const;

	static std::wstring encode(const std::vector< uint8_t >& bytes);

	static std::wstring encode(const uint8_t* bytes, uint32_t len);

	static std::wstring encode(const std::wstring& text);

	static std::vector< uint8_t > decodeBytes(const std::wstring& text);

	static std::wstring decodeString(const std::wstring& text);
	
private:
	bool m_valid;
	std::wstring m_protocol;
	std::wstring m_host;
	int m_port;
	std::wstring m_path;
	std::wstring m_file;
	std::wstring m_query;
	std::wstring m_ref;
	std::wstring m_userInfo;
};

	}
}

#endif	// traktor_net_Url_H
