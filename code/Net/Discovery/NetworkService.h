#ifndef traktor_net_NetworkService_H
#define traktor_net_NetworkService_H

#include "Net/Discovery/IService.h"

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

/*! \brief Generic network service.
 * \ingroup Net
 */
class T_DLLCLASS NetworkService : public IService
{
	T_RTTI_CLASS;

public:
	NetworkService();

	NetworkService(
		const std::wstring& type,
		const std::wstring& host,
		const std::wstring& description
	);

	const std::wstring& getType() const;

	const std::wstring& getHost() const;

	virtual std::wstring getDescription() const;

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_type;
	std::wstring m_host;
	std::wstring m_description;
};

	}
}

#endif	// traktor_net_NetworkService_H
