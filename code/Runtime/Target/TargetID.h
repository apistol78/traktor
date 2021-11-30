#pragma once

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

/*! Running target identification.
 * \ingroup Runtime
 */
class T_DLLCLASS TargetID : public ISerializable
{
	T_RTTI_CLASS;

public:
	TargetID() = default;

	explicit TargetID(const Guid& id, const std::wstring& name);

	const Guid& getId() const;

	const std::wstring& getName() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_id;
	std::wstring m_name;
};

	}
}

