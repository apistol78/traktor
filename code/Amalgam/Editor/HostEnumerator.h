#ifndef traktor_amalgam_HostEnumerator_H
#define traktor_amalgam_HostEnumerator_H

#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{
	namespace net
	{

class DiscoveryManager;

	}

	namespace amalgam
	{

class HostEnumerator : public Object
{
	T_RTTI_CLASS;

public:
	HostEnumerator(net::DiscoveryManager* discoveryManager);

	int32_t count() const;

	bool getHost(int32_t index, std::wstring& outHost) const;

	bool getDescription(int32_t index, std::wstring& outDescription) const;

	void update();

private:
	struct Host
	{
		std::wstring host;
		std::wstring description;

		bool operator < (const Host& h) const;
	};

	Ref< net::DiscoveryManager > m_discoveryManager;
	mutable Semaphore m_lock;
	std::vector< Host > m_hosts;
};

	}
}

#endif	// traktor_amalgam_HostEnumerator_H
