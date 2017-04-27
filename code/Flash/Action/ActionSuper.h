/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ActionSuper_H
#define traktor_flash_ActionSuper_H

#include "Flash/Action/ActionFunction.h"

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

/*! \brief Class wrapper to access parent class members.
 * \ingroup Flash
 *
 * \note Super is accessed through
 * "(object).__proto__.__constructor__" and
 * "(object).__proto__.__proto__"
 * which are relationships that AopExtends prepares.
 */
class T_DLLCLASS ActionSuper : public ActionFunction
{
	T_RTTI_CLASS;

public:
	ActionSuper(ActionContext* context, ActionObject* object, ActionObject* superPrototype, ActionFunction* superClass);

	virtual ActionObject* get__proto__() T_OVERRIDE;

	virtual void setMember(uint32_t memberName, const ActionValue& memberValue) T_OVERRIDE;

	virtual bool getMember(uint32_t memberName, ActionValue& outMemberValue) T_OVERRIDE;

	virtual bool getPropertyGet(uint32_t propertyName, Ref< ActionFunction >& outPropertyGet) T_OVERRIDE;

	virtual bool getPropertySet(uint32_t propertyName, Ref< ActionFunction >& outPropertySet) T_OVERRIDE;

	virtual ActionValue valueOf() T_OVERRIDE;

	virtual ActionValue toString() T_OVERRIDE;

	virtual Ref< ActionObject > getSuper() T_OVERRIDE;

	virtual ActionValue call(ActionObject* self, ActionObject* super, const ActionValueArray& args) T_OVERRIDE;

	ActionObject* getObject() { return m_object; }

protected:
	virtual void trace(visitor_t visitor) const T_OVERRIDE;

	virtual void dereference() T_OVERRIDE;

private:
	Ref< ActionObject > m_object;
	Ref< ActionFunction > m_superClass;
	Ref< ActionObject > m_superPrototype;
};

	}
}

#endif	// traktor_flash_ActionSuper_H
