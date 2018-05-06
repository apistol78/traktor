/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ActionObjectRelay_H
#define traktor_flash_ActionObjectRelay_H

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Flash/Collectable.h"

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

class ActionContext;
class ActionObject;
class ActionValue;

class T_DLLCLASS ActionObjectRelay : public Collectable
{
	T_RTTI_CLASS;

public:
	virtual void setAsObject(ActionObject* asObject);

	virtual ActionObject* getAsObject(ActionContext* context);

	virtual bool enumerateMembers(AlignedVector< uint32_t >& outMemberNames) const;

	virtual bool setMember(ActionContext* context, uint32_t memberName, const ActionValue& memberValue);

	virtual bool getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue);

	ActionObject* getAsObject() const { return m_asObject; }

protected:
	ActionObjectRelay(const char* const prototype);

	virtual void trace(visitor_t visitor) const T_OVERRIDE;

	virtual void dereference() T_OVERRIDE;

private:
	const char* const m_prototype;
	mutable Ref< ActionObject > m_asObject;
};

	}
}

#endif	// traktor_flash_ActionObjectRelay_H
