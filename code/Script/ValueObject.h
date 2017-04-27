/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_ValueObject_H
#define traktor_script_ValueObject_H

#include "Script/IValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class Variable;

/*! \brief
 * \ingroup Script
 */
class T_DLLCLASS ValueObject : public IValue
{
	T_RTTI_CLASS;

public:
	ValueObject();

	explicit ValueObject(uint32_t objectRef);

	explicit ValueObject(uint32_t objectRef, const std::wstring& valueOf);

	void setObjectRef(uint32_t objectRef);

	uint32_t getObjectRef() const;

	void setValueOf(const std::wstring& valueOf);

	const std::wstring& getValueOf() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	uint32_t m_objectRef;
	std::wstring m_valueOf;
};

	}
}

#endif	// traktor_script_ValueObject_H
