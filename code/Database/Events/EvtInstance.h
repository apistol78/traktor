#pragma once

#include "Core/Guid.h"
#include "Database/IEvent.h"

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

/*! Instance base event.
 * \ingroup Database
 */
class T_DLLCLASS EvtInstance : public IEvent
{
	T_RTTI_CLASS;

public:
	EvtInstance() = default;

	explicit EvtInstance(const Guid& instanceGuid);

	/*! Instance's guid.
	 *
	 * \return Instance's guid.
	 */
	const Guid& getInstanceGuid() const;

	virtual void serialize(ISerializer& s) override;

private:
	Guid m_instanceGuid;
};

	}
}

