#ifndef traktor_db_LocalManifest_H
#define traktor_db_LocalManifest_H

#include <string>
#include "Core/Serialization/Serializable.h"

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

/*! \brief Local database manifest.
 * \ingroup Database
 */
class T_DLLCLASS LocalManifest : public Serializable
{
	T_RTTI_CLASS(LocalManifest)

public:
	LocalManifest();

	void setRootGroupPath(const std::wstring& userDomainPath);

	const std::wstring& getRootGroupPath() const;

	void setEventMonitorEnable(bool eventMonitorEnable);

	bool getEventMonitorEnable() const;

	void setEventFile(const std::wstring& eventFile);

	const std::wstring& getEventFile() const;

	void setUseBinary(bool useBinary);

	bool getUseBinary() const;

	virtual bool serialize(Serializer& s);

private:
	std::wstring m_rootGroupPath;
	bool m_eventMonitorEnable;
	std::wstring m_eventFile;
	bool m_useBinary;
};

	}
}

#endif	// traktor_db_LocalManifest_H
