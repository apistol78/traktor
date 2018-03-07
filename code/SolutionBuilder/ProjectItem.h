/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ProjectItem_H
#define ProjectItem_H

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

	virtual void serialize(ISerializer& s) T_OVERRIDE;

private:
	RefArray< ProjectItem > m_items;
};

	}
}

#endif	// ProjectItem_H
