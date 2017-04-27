/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ActionDictionary_H
#define traktor_flash_ActionDictionary_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Flash/Action/ActionValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief ActionScript constant string dictionary.
 * \ingroup Flash
 */
class T_DLLCLASS ActionDictionary : public Object
{
	T_RTTI_CLASS;

public:
	ActionDictionary();

	Ref< ActionDictionary > clone() const;

	void add(const ActionValue& value);

	const ActionValue& get(uint16_t index) const
	{
		T_ASSERT (index < m_table.size());
		return m_table[index];
	}

	const AlignedVector< ActionValue >& getTable() const
	{
		return m_table;
	}

private:
	AlignedVector< ActionValue > m_table;

	ActionDictionary(const AlignedVector< ActionValue >& table);
};

	}
}

#endif	// traktor_flash_ActionDictionary_H
