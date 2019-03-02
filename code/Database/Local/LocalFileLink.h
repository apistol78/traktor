#pragma once

#include <string>
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace db
	{

/*! \brief Virtual file link.
 * \ingroup Database
 */
class LocalFileLink : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	inline const std::wstring& getPath() const { return m_path; }

private:
	std::wstring m_path;
};

	}
}

