#pragma once

#include <string>
#include "Core/Guid.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace db
	{

/*! \brief Local instance meta.
 * \ingroup Database
 */
class LocalInstanceMeta : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Blob
	{
		std::wstring name;
		std::wstring hash;

		void serialize(ISerializer& s);
	};

	LocalInstanceMeta();

	LocalInstanceMeta(const Guid& guid, const std::wstring& primaryType);

	void setGuid(const Guid& guid);

	const Guid& getGuid() const;

	void setPrimaryType(const std::wstring& primaryType);

	const std::wstring& getPrimaryType() const;

	void setBlob(const std::wstring& name, const std::wstring& hash);

	void removeBlob(const std::wstring& name);

	bool haveBlob(const std::wstring& name) const;

	const AlignedVector< Blob >& getBlobs() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_guid;
	std::wstring m_primaryType;
	AlignedVector< Blob > m_blobs;
};

	}
}

