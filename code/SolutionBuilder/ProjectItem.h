#pragma once

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sb
	{

class T_DLLCLASS ProjectItem : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addItem(ProjectItem* item);

	void removeItem(ProjectItem* item);

	const RefArray< ProjectItem >& getItems() const;

	virtual void serialize(ISerializer& s) override;

private:
	RefArray< ProjectItem > m_items;
};

	}
}

