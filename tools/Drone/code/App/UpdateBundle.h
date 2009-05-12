#ifndef traktor_drone_UpdateBundle_H
#define traktor_drone_UpdateBundle_H

#include <Core/Serialization/Serializable.h>
#include <Net/Url.h>

namespace traktor
{
	namespace drone
	{

class UpdateBundle : public Serializable
{
	T_RTTI_CLASS(UpdateBundle);

public:
	struct BundledItem
	{
		std::wstring url;
		std::wstring path;

		bool serialize(Serializer& s);
	};

	UpdateBundle();

	uint32_t getBundleVersion() const;

	const std::wstring& getDescription() const;

	const std::vector< BundledItem >& getItems() const;

	virtual bool serialize(Serializer& s);

private:
	uint32_t m_version;
	std::wstring m_description;
	std::vector< BundledItem > m_items;
};

	}
}

#endif	// traktor_drone_UpdateBundle_H
