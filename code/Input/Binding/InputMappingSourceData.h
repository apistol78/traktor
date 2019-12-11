#pragma once

#include <map>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class IInputSourceData;

/*! Input mapping source data.
 * \ingroup Input
 *
 * Serializable description of input sources.
 */
class T_DLLCLASS InputMappingSourceData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setSourceData(const std::wstring& id, IInputSourceData* data);

	IInputSourceData* getSourceData(const std::wstring& id);

	const std::map< std::wstring, Ref< IInputSourceData > >& getSourceData() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::map< std::wstring, Ref< IInputSourceData > > m_sourceData;
};

	}
}

