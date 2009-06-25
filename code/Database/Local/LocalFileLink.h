#ifndef traktor_db_LocalFileLink_H
#define traktor_db_LocalFileLink_H

#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace db
	{

/*! \brief Virtual file link.
 * \ingroup Database
 */
class LocalFileLink : public Serializable
{
	T_RTTI_CLASS(LocalFileLink)

public:
	virtual bool serialize(Serializer& s);

	inline const std::wstring& getPath() const { return m_path; }

private:
	std::wstring m_path;
};

	}
}

#endif	// traktor_db_LocalFileLink_H
