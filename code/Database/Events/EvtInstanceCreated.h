#pragma once

#include "Database/Events/EvtInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! Instance created event.
 * \ingroup Database
 */
class T_DLLCLASS EvtInstanceCreated : public EvtInstance
{
	T_RTTI_CLASS;

public:
	EvtInstanceCreated() = default;

	explicit EvtInstanceCreated(const std::wstring& groupPath, const Guid& instanceGuid);

	/*! Path to group which contain new instance.
	 *
	 * \return Group path.
	 */
	const std::wstring& getGroupPath() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_groupPath;
};

	}
}

