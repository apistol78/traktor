#ifndef traktor_amalgam_HostEnumerator_H
#define traktor_amalgam_HostEnumerator_H

#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

class PropertyGroup;

	namespace net
	{

class DiscoveryManager;

	}

	namespace amalgam
	{

/*! \brief Enumerator of hosts able to communicate with editor.
 * \ingroup Amalgam
 */
class HostEnumerator : public Object
{
	T_RTTI_CLASS;

public:
	HostEnumerator(const PropertyGroup* settings, net::DiscoveryManager* discoveryManager);

	int32_t count() const;

	bool getHost(int32_t index, std::wstring& outHost) const;

	bool getDescription(int32_t index, std::wstring& outDescription) const;

	bool supportPlatform(int32_t index, const std::wstring& platform) const;

	bool isLocal(int32_t index) const;

	void update();

private:
	struct Host
	{
		std::wstring host;
		std::wstring description;
		std::vector< std::wstring > platforms;
		bool local;

		Host();

		bool operator < (const Host& h) const;
	};

	Ref< net::DiscoveryManager > m_discoveryManager;
	mutable Semaphore m_lock;
	std::vector< Host > m_manual;
	std::vector< Host > m_hosts;
};

	}
}

#endif	// traktor_amalgam_HostEnumerator_H
