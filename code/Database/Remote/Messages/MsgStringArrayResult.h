#ifndef traktor_db_MsgStringArrayResult_H
#define traktor_db_MsgStringArrayResult_H

#include <vector>
#include "Database/Remote/IMessage.h"

namespace traktor
{
	namespace db
	{

/*! \brief Array of string result.
 * \ingroup Database
 */
class MsgStringArrayResult : public IMessage
{
	T_RTTI_CLASS;

public:
	MsgStringArrayResult();

	MsgStringArrayResult(const std::vector< std::wstring >& values);

	void add(const std::wstring& value);

	uint32_t count();

	const std::wstring& get(uint32_t index) const;

	const std::vector< std::wstring >& get() const { return m_values; }

	virtual bool serialize(ISerializer& s);

private:
	std::vector< std::wstring > m_values;
};

	}
}

#endif	// traktor_db_MsgStringArrayResult_H