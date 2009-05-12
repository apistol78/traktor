#ifndef traktor_db_LocalInstance_H
#define traktor_db_LocalInstance_H

#include "Database/Provider.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class LocalContext;
class LocalGroup;

/*! \brief Local instance.
 * \ingroup Database
 */
class T_DLLCLASS LocalInstance : public IProviderInstance
{
	T_RTTI_CLASS(LocalInstance)

public:
	LocalInstance(LocalContext* context);

	bool internalCreateExisting(const Path& instancePath);

	bool internalCreateNew(const Path& instancePath, const Guid& instanceGuid);

	virtual std::wstring getName() const;

	virtual Guid getGuid() const;

	virtual std::wstring getPrimaryTypeName() const;

	virtual bool rename(const std::wstring& name);

	virtual bool remove();

	virtual bool lock();

	virtual bool unlock();

	virtual Serializable* readObject();

	virtual bool writeObject(Serializable* object);

	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const;

	virtual Stream* readData(const std::wstring& dataName);

	virtual Stream* writeData(const std::wstring& dataName);

private:
	Ref< LocalContext > m_context;
	Path m_instancePath;
	bool m_locked;
};

	}
}

#endif	// traktor_db_LocalInstance_H
