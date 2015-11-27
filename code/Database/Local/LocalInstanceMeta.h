#ifndef traktor_db_LocalInstanceMeta_H
#define traktor_db_LocalInstanceMeta_H

#include "Core/Guid.h"
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
	LocalInstanceMeta();

	LocalInstanceMeta(const Guid& guid, const std::wstring& primaryType);

	void setGuid(const Guid& guid);

	const Guid& getGuid() const;

	void setPrimaryType(const std::wstring& primaryType);

	const std::wstring& getPrimaryType() const;

	void addBlob(const std::wstring& blob);

	void removeBlob(const std::wstring& blob);

	bool haveBlob(const std::wstring& blob) const;

	const std::vector< std::wstring >& getBlobs() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Guid m_guid;
	std::wstring m_primaryType;
	std::vector< std::wstring > m_blobs;
};

	}
}

#endif	// traktor_db_LocalInstanceMeta_H
