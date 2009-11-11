#ifndef traktor_db_IProviderInstance_H
#define traktor_db_IProviderInstance_H

#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Serializable;
class Stream;

	namespace db
	{

/*! \brief Provider instance interface.
 * \ingroup Database
 */
class T_DLLCLASS IProviderInstance : public Object
{
	T_RTTI_CLASS(IProviderInstance)

public:
	virtual std::wstring getPrimaryTypeName() const = 0;

	virtual bool openTransaction() = 0;

	virtual bool commitTransaction() = 0;

	virtual bool closeTransaction() = 0;

	virtual std::wstring getName() const = 0;

	virtual bool setName(const std::wstring& name) = 0;

	virtual Guid getGuid() const = 0;

	virtual bool setGuid(const Guid& guid) = 0;

	virtual bool remove() = 0;

	virtual Ref< Stream > readObject(const Type*& outSerializerType) = 0;

	virtual Ref< Stream > writeObject(const std::wstring& primaryTypeName, const Type*& outSerializerType) = 0;

	virtual uint32_t getDataNames(std::vector< std::wstring >& outDataNames) const = 0;

	virtual Ref< Stream > readData(const std::wstring& dataName) = 0;

	virtual Ref< Stream > writeData(const std::wstring& dataName) = 0;
};

	}
}

#endif	// traktor_db_IProviderInstance_H
