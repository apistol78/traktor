#ifndef traktor_flash_ActionFunctionNative_H
#define traktor_flash_ActionFunctionNative_H

#include "Core/Log/Log.h"
#include "Core/Meta/Traits.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/ActionValueArray.h"

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

/*! \brief Native call arguments.
 * \ingroup Flash
 */
struct CallArgs
{
	ActionContext* context;
	ActionObject* self;
	ActionObject* super;
	ActionValueArray args;	// \fixme Reference to instaed of copy
	ActionValue ret;
};

/*! \brief Native function wrapper.
 * \ingroup Flash
 */
struct T_DLLCLASS INativeFunction : public Object
{
	virtual void call(CallArgs& ca) = 0;
};


template < typename Type, bool IsTypePtr = IsPointer< Type >::value >
struct ActionValueCast { };

template < >
struct ActionValueCast< ActionValue, false >
{
	static ActionValue set(ActionContext* cx, const ActionValue& v) { return v; }
	static ActionValue get(ActionContext* cx, const ActionValue& v) { return v; }
};

template < >
struct ActionValueCast< const ActionValue&, false >
{
	static ActionValue set(ActionContext* cx, const ActionValue& v) { return v; }
	static ActionValue get(ActionContext* cx, const ActionValue& v) { return v; }
};

template < >
struct ActionValueCast< bool, false >
{
	static ActionValue set(ActionContext* cx, bool v) { return ActionValue(v); }
	static bool get(ActionContext* cx, const ActionValue& av) { return av.getBoolean(); }
};

template < >
struct ActionValueCast< int32_t, false >
{
	static ActionValue set(ActionContext* cx, int32_t v) { return ActionValue(avm_number_t(v)); }
	static int32_t get(ActionContext* cx, const ActionValue& av) { return int32_t(av.getNumber()); }
};

template < >
struct ActionValueCast< uint32_t, false >
{
	static ActionValue set(ActionContext* cx, uint32_t v) { return ActionValue(avm_number_t(v)); }
	static uint32_t get(ActionContext* cx, const ActionValue& av) { return uint32_t(av.getNumber()); }
};

template < >
struct ActionValueCast< float, false >
{
	static ActionValue set(ActionContext* cx, float v) { return ActionValue(avm_number_t(v)); }
	static float get(ActionContext* cx, const ActionValue& av) { return float(av.getNumber()); }
};

template < >
struct ActionValueCast< double, false >
{
	static ActionValue set(ActionContext* cx, double v) { return ActionValue(avm_number_t(v)); }
	static double get(ActionContext* cx, const ActionValue& av) { return double(av.getNumber()); }
};

template < >
struct ActionValueCast< std::string, false >
{
	static ActionValue set(ActionContext* cx, const std::string& v) { return ActionValue(v); }
	static std::string get(ActionContext* cx, const ActionValue& av) { return av.getString(); }
};

template < >
struct ActionValueCast< const std::string&, false >
{
	static ActionValue set(ActionContext* cx, const std::string& v) { return ActionValue(v); }
	static std::string get(ActionContext* cx, const ActionValue& av) { return av.getString(); }
};

template < >
struct ActionValueCast< std::wstring, false >
{
	static ActionValue set(ActionContext* cx, const std::wstring& v) { return ActionValue(v); }
	static std::wstring get(ActionContext* cx, const ActionValue& av) { return av.getWideString(); }
};

template < >
struct ActionValueCast< const std::wstring&, false >
{
	static ActionValue set(ActionContext* cx, const std::wstring& v) { return ActionValue(v); }
	static std::wstring get(ActionContext* cx, const ActionValue& av) { return av.getWideString(); }
};

template < >
struct ActionValueCast< Ref< ActionObject >, false >
{
	static ActionValue set(ActionContext* cx, const Ref< ActionObject >& v) { return ActionValue(v); }
	static ActionObject* get(ActionContext* cx, const ActionValue& value) { return value.getObject(); }
};

template < >
struct ActionValueCast< ActionObject*, true >
{
	static ActionValue set(ActionContext* cx, ActionObject* v) { return ActionValue(v); }
	static ActionObject* get(ActionContext* cx, const ActionValue& value) { return value.getObject(); }
};

template < >
struct ActionValueCast< Ref< ActionFunction >, false >
{
	static ActionValue set(ActionContext* cx, const Ref< ActionFunction >& v) { return ActionValue(v); }
	static ActionFunction* get(ActionContext* cx, const ActionValue& value) { return value.getObject< ActionFunction >(); }
};

template < >
struct ActionValueCast< ActionFunction*, true >
{
	static ActionValue set(ActionContext* cx, ActionFunction* v) { return ActionValue(v); }
	static ActionFunction* get(ActionContext* cx, const ActionValue& value) { return value.getObject< ActionFunction >(); }
};

template < typename Type >
struct ActionValueCast< Ref< Type >, false >
{
	typedef Type base_t;

	static ActionValue set(ActionContext* cx, const Ref< base_t >& v) { return ActionValue(v ? v->getAsObject(cx) : 0); }
	static Ref< base_t > get(ActionContext* cx, const ActionValue& value) { return value.getObject()->getRelay< base_t >(); }
};

template < typename Type >
struct ActionValueCast< Type, true >
{
	typedef typename IsPointer< Type >::base_t base_t;

	static ActionValue set(ActionContext* cx, base_t* v) { return ActionValue(v ? v->getAsObject(cx) : 0); }
	static base_t* get(ActionContext* cx, const ActionValue& value) { return value.getObject()->getRelay< base_t >(); }
};

extern void T_DLLCLASS trapInvalidCast();

#if defined(_DEBUG)
#	define T_DEBUG_NULL(x) \
	if (!(x)) trapInvalidCast();
#else
#	define T_DEBUG_NULL(x)
#endif

template <
	typename SelfClassType
>
struct Ensure
{
	static SelfClassType* get(ActionObject* o)
	{
		SelfClassType* self = o->getRelay< SelfClassType >();
		T_DEBUG_NULL(self);
		return self;
	}
};

template < >
struct Ensure < ActionObject >
{
	static ActionObject* get(ActionObject* o)
	{
		T_DEBUG_NULL(o);
		return o;
	}
};

template < >
struct Ensure < const ActionObject >
{
	static ActionObject* get(ActionObject* o)
	{
		return o;
	}
};


template <
	typename CallClassType
>
struct MethodNativeFunction : public INativeFunction
{
	CallClassType* m_object;
	void (CallClassType::*m_method)(CallArgs&);

	virtual void call(CallArgs& ca)
	{
		(m_object->*m_method)(ca);
	}
};

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType
>
struct MethodNativeFunction_0 : public INativeFunction
{
	CallClassType* m_object;
	ReturnType (CallClassType::*m_method)(SelfClassType*) const;

	virtual void call(CallArgs& ca)
	{
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			ca.ret = ActionValueCast< ReturnType >::set(
				ca.context,
				(m_object->*m_method)(self)
			);
	}
};

template <
	typename CallClassType,
	typename SelfClassType
>
struct MethodNativeFunction_0 < CallClassType, SelfClassType, void > : public INativeFunction
{
	CallClassType* m_object;
	void (CallClassType::*m_method)(SelfClassType*) const;

	virtual void call(CallArgs& ca)
	{
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			(m_object->*m_method)(self);
	}
};

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type
>
struct MethodNativeFunction_1 : public INativeFunction
{
	CallClassType* m_object;
	ReturnType (CallClassType::*m_method)(SelfClassType*, Argument1Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 1);
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			ca.ret = ActionValueCast< ReturnType >::set(
				ca.context,
				(m_object->*m_method)(
					self,
					ActionValueCast< Argument1Type >::get(ca.context, ca.args[0])
				)
			);
	}
};

template <
	typename CallClassType,
	typename SelfClassType,
	typename Argument1Type
>
struct MethodNativeFunction_1 < CallClassType, SelfClassType, void, Argument1Type > : public INativeFunction
{
	CallClassType* m_object;
	void (CallClassType::*m_method)(SelfClassType*, Argument1Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 1);
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			(m_object->*m_method)(
				self,
				ActionValueCast< Argument1Type >::get(ca.context, ca.args[0])
			);
	}
};

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type
>
struct MethodNativeFunction_2 : public INativeFunction
{
	CallClassType* m_object;
	ReturnType (CallClassType::*m_method)(SelfClassType*, Argument1Type, Argument2Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 2);
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			ca.ret = ActionValueCast< ReturnType >::set(
				ca.context,
				(m_object->*m_method)(
					self,
					ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
					ActionValueCast< Argument2Type >::get(ca.context, ca.args[1])
				)
			);
	}
};

template <
	typename CallClassType,
	typename SelfClassType,
	typename Argument1Type,
	typename Argument2Type
>
struct MethodNativeFunction_2 < CallClassType, SelfClassType, void, Argument1Type, Argument2Type > : public INativeFunction
{
	CallClassType* m_object;
	void (CallClassType::*m_method)(SelfClassType*, Argument1Type, Argument2Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 2);
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			(m_object->*m_method)(
				self,
				ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
				ActionValueCast< Argument2Type >::get(ca.context, ca.args[1])
			);
	}
};

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct MethodNativeFunction_3 : public INativeFunction
{
	CallClassType* m_object;
	ReturnType (CallClassType::*m_method)(SelfClassType*, Argument1Type, Argument2Type, Argument3Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 3);
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			ca.ret = ActionValueCast< ReturnType >::set(
				ca.context,
				(m_object->*m_method)(
					self,
					ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
					ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
					ActionValueCast< Argument3Type >::get(ca.context, ca.args[2])
				)
			);
	}
};

template <
	typename CallClassType,
	typename SelfClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct MethodNativeFunction_3 < CallClassType, SelfClassType, void, Argument1Type, Argument2Type, Argument3Type > : public INativeFunction
{
	CallClassType* m_object;
	void (CallClassType::*m_method)(SelfClassType*, Argument1Type, Argument2Type, Argument3Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 3);
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			(m_object->*m_method)(
				self,
				ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
				ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
				ActionValueCast< Argument3Type >::get(ca.context, ca.args[2])
			);
	}
};

//--

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type
>
struct MethodNativeFunction_4 : public INativeFunction
{
	CallClassType* m_object;
	ReturnType (CallClassType::*m_method)(SelfClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 4);
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			ca.ret = ActionValueCast< ReturnType >::set(
				ca.context,
				(m_object->*m_method)(
					self,
					ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
					ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
					ActionValueCast< Argument3Type >::get(ca.context, ca.args[2]),
					ActionValueCast< Argument4Type >::get(ca.context, ca.args[3])
				)
			);
	}
};

template <
	typename CallClassType,
	typename SelfClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type
>
struct MethodNativeFunction_4 < CallClassType, SelfClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type > : public INativeFunction
{
	CallClassType* m_object;
	void (CallClassType::*m_method)(SelfClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 4);
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			(m_object->*m_method)(
				self,
				ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
				ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
				ActionValueCast< Argument3Type >::get(ca.context, ca.args[2]),
				ActionValueCast< Argument4Type >::get(ca.context, ca.args[3])
			);
	}
};

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type
>
struct MethodNativeFunction_5 : public INativeFunction
{
	CallClassType* m_object;
	ReturnType (CallClassType::*m_method)(SelfClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 5);
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			ca.ret = ActionValueCast< ReturnType >::set(
				ca.context,
				(m_object->*m_method)(
					self,
					ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
					ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
					ActionValueCast< Argument3Type >::get(ca.context, ca.args[2]),
					ActionValueCast< Argument4Type >::get(ca.context, ca.args[3]),
					ActionValueCast< Argument5Type >::get(ca.context, ca.args[4])
				)
			);
	}
};

template <
	typename CallClassType,
	typename SelfClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type
>
struct MethodNativeFunction_5 < CallClassType, SelfClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type > : public INativeFunction
{
	CallClassType* m_object;
	void (CallClassType::*m_method)(SelfClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 5);
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			(m_object->*m_method)(
				self,
				ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
				ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
				ActionValueCast< Argument3Type >::get(ca.context, ca.args[2]),
				ActionValueCast< Argument4Type >::get(ca.context, ca.args[3]),
				ActionValueCast< Argument5Type >::get(ca.context, ca.args[4])
			);
	}
};

//--

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type
>
struct MethodNativeFunction_6 : public INativeFunction
{
	CallClassType* m_object;
	ReturnType (CallClassType::*m_method)(SelfClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 6);
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			ca.ret = ActionValueCast< ReturnType >::set(
				ca.context,
				(m_object->*m_method)(
					self,
					ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
					ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
					ActionValueCast< Argument3Type >::get(ca.context, ca.args[2]),
					ActionValueCast< Argument4Type >::get(ca.context, ca.args[3]),
					ActionValueCast< Argument5Type >::get(ca.context, ca.args[4]),
					ActionValueCast< Argument6Type >::get(ca.context, ca.args[5])
				)
			);
	}
};

template <
	typename CallClassType,
	typename SelfClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type
>
struct MethodNativeFunction_6 < CallClassType, SelfClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type > : public INativeFunction
{
	CallClassType* m_object;
	void (CallClassType::*m_method)(SelfClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type) const;

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 6);
		SelfClassType* self = Ensure< SelfClassType >::get(ca.self);
		if (self)
			(m_object->*m_method)(
				self,
				ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
				ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
				ActionValueCast< Argument3Type >::get(ca.context, ca.args[2]),
				ActionValueCast< Argument4Type >::get(ca.context, ca.args[3]),
				ActionValueCast< Argument5Type >::get(ca.context, ca.args[4]),
				ActionValueCast< Argument6Type >::get(ca.context, ca.args[5])
			);
	}
};

//--

template <
	typename ClassType,
	typename ReturnType
>
struct MethodNativeFunction_self_0 : public INativeFunction
{
	ReturnType (ClassType::*m_method)();

	virtual void call(CallArgs& ca)
	{
		ClassType* self = Ensure< ClassType >::get(ca.self);
		if (self)
			ca.ret = ActionValueCast< ReturnType >::set(
				ca.context,
				(self->*m_method)()
			);
	}
};

template <
	typename ClassType
>
struct MethodNativeFunction_self_0 < ClassType, void > : public INativeFunction
{
	void (ClassType::*m_method)();

	virtual void call(CallArgs& ca)
	{
		ClassType* self = Ensure< ClassType >::get(ca.self);
		if (self)
			(self->*m_method)();
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type
>
struct MethodNativeFunction_self_1 : public INativeFunction
{
	ReturnType (ClassType::*m_method)(Argument1Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 1);
		ClassType* self = Ensure< ClassType >::get(ca.self);
		if (self)
			ca.ret = ActionValueCast< ReturnType >::set(
				ca.context,
				(self->*m_method)(
					ActionValueCast< Argument1Type >::get(ca.context, ca.args[0])
				)
			);
	}
};

template <
	typename ClassType,
	typename Argument1Type
>
struct MethodNativeFunction_self_1 < ClassType, void, Argument1Type > : public INativeFunction
{
	void (ClassType::*m_method)(Argument1Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 1);
		ClassType* self = Ensure< ClassType >::get(ca.self);
		if (self)
			(self->*m_method)(
				ActionValueCast< Argument1Type >::get(ca.context, ca.args[0])
			);
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type
>
struct MethodNativeFunction_self_2 : public INativeFunction
{
	ReturnType (ClassType::*m_method)(Argument1Type, Argument2Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 2);
		ClassType* self = Ensure< ClassType >::get(ca.self);
		if (self)
			ca.ret = ActionValueCast< ReturnType >::set(
				ca.context,
				(self->*m_method)(
					ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
					ActionValueCast< Argument2Type >::get(ca.context, ca.args[1])
				)
			);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type
>
struct MethodNativeFunction_self_2 < ClassType, void, Argument1Type, Argument2Type > : public INativeFunction
{
	void (ClassType::*m_method)(Argument1Type, Argument2Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 2);
		ClassType* self = Ensure< ClassType >::get(ca.self);
		if (self)
			(self->*m_method)(
				ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
				ActionValueCast< Argument2Type >::get(ca.context, ca.args[1])
			);
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct MethodNativeFunction_self_3 : public INativeFunction
{
	ReturnType (ClassType::*m_method)(Argument1Type, Argument2Type, Argument3Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 3);
		ClassType* self = Ensure< ClassType >::get(ca.self);
		if (self)
			ca.ret = ActionValueCast< ReturnType >::set(
				ca.context,
				(self->*m_method)(
					ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
					ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
					ActionValueCast< Argument3Type >::get(ca.context, ca.args[2])
				)
			);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
struct MethodNativeFunction_self_3 < ClassType, void, Argument1Type, Argument2Type, Argument3Type > : public INativeFunction
{
	void (ClassType::*m_method)(Argument1Type, Argument2Type, Argument3Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 3);
		ClassType* self = Ensure< ClassType >::get(ca.self);
		if (self)
			(self->*m_method)(
				ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
				ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
				ActionValueCast< Argument3Type >::get(ca.context, ca.args[2])
			);
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type
>
struct MethodNativeFunction_self_5 : public INativeFunction
{
	ReturnType (ClassType::*m_method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 5);
		ClassType* self = Ensure< ClassType >::get(ca.self);
		if (self)
			ca.ret = ActionValueCast< ReturnType >::set(
				ca.context,
				(self->*m_method)(
					ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
					ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
					ActionValueCast< Argument3Type >::get(ca.context, ca.args[2]),
					ActionValueCast< Argument4Type >::get(ca.context, ca.args[3]),
					ActionValueCast< Argument5Type >::get(ca.context, ca.args[4])
				)
			);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type
>
struct MethodNativeFunction_self_5 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type > : public INativeFunction
{
	void (ClassType::*m_method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 5);
		ClassType* self = Ensure< ClassType >::get(ca.self);
		if (self)
			(self->*m_method)(
				ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
				ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
				ActionValueCast< Argument3Type >::get(ca.context, ca.args[2]),
				ActionValueCast< Argument4Type >::get(ca.context, ca.args[3]),
				ActionValueCast< Argument5Type >::get(ca.context, ca.args[4])
			);
	}
};

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type
>
struct MethodNativeFunction_self_6 : public INativeFunction
{
	ReturnType (ClassType::*m_method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 6);
		ClassType* self = Ensure< ClassType >::get(ca.self);
		if (self)
			ca.ret = ActionValueCast< ReturnType >::set(
				ca.context,
				(self->*m_method)(
					ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
					ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
					ActionValueCast< Argument3Type >::get(ca.context, ca.args[2]),
					ActionValueCast< Argument4Type >::get(ca.context, ca.args[3]),
					ActionValueCast< Argument5Type >::get(ca.context, ca.args[4]),
					ActionValueCast< Argument6Type >::get(ca.context, ca.args[5])
				)
			);
	}
};

template <
	typename ClassType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type
>
struct MethodNativeFunction_self_6 < ClassType, void, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type > : public INativeFunction
{
	void (ClassType::*m_method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type);

	virtual void call(CallArgs& ca)
	{
		T_ASSERT (ca.args.size() >= 6);
		ClassType* self = Ensure< ClassType >::get(ca.self);
		if (self)
			(self->*m_method)(
				ActionValueCast< Argument1Type >::get(ca.context, ca.args[0]),
				ActionValueCast< Argument2Type >::get(ca.context, ca.args[1]),
				ActionValueCast< Argument3Type >::get(ca.context, ca.args[2]),
				ActionValueCast< Argument4Type >::get(ca.context, ca.args[3]),
				ActionValueCast< Argument5Type >::get(ca.context, ca.args[4]),
				ActionValueCast< Argument6Type >::get(ca.context, ca.args[5])
			);
	}
};

/*! \brief ActionScript native function.
 * \ingroup Flash
 */
class T_DLLCLASS ActionFunctionNative : public ActionFunction
{
	T_RTTI_CLASS;

public:
	ActionFunctionNative(ActionContext* context, INativeFunction* nativeFunction);

	virtual ActionValue call(ActionObject* self, ActionObject* super, const ActionValueArray& args);

private:
	Ref< INativeFunction > m_nativeFunction;
};

// \group Prototype methods
// \{

template <
	typename CallClassType
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, CallClassType* object, void (CallClassType::*method)(CallArgs&))
{
	typedef MethodNativeFunction< CallClassType > NF;
	Ref< NF > nf = new NF();
	nf->m_object = object;
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, CallClassType* object, ReturnType (CallClassType::*method)(SelfClassType*) const)
{
	typedef MethodNativeFunction_0< CallClassType, SelfClassType, ReturnType > NF;
	Ref< NF > nf = new NF();
	nf->m_object = object;
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, CallClassType* object, ReturnType (CallClassType::*method)(SelfClassType*, Argument1Type) const)
{
	typedef MethodNativeFunction_1< CallClassType, SelfClassType, ReturnType, Argument1Type > NF;
	Ref< NF > nf = new NF();
	nf->m_object = object;
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, CallClassType* object, ReturnType (CallClassType::*method)(SelfClassType*, Argument1Type, Argument2Type) const)
{
	typedef MethodNativeFunction_2< CallClassType, SelfClassType, ReturnType, Argument1Type, Argument2Type > NF;
	Ref< NF > nf = new NF();
	nf->m_object = object;
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, CallClassType* object, ReturnType (CallClassType::*method)(SelfClassType*, Argument1Type, Argument2Type, Argument3Type) const)
{
	typedef MethodNativeFunction_3< CallClassType, SelfClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type > NF;
	Ref< NF > nf = new NF();
	nf->m_object = object;
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, CallClassType* object, ReturnType (CallClassType::*method)(SelfClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type) const)
{
	typedef MethodNativeFunction_4< CallClassType, SelfClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type > NF;
	Ref< NF > nf = new NF();
	nf->m_object = object;
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, CallClassType* object, ReturnType (CallClassType::*method)(SelfClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type) const)
{
	typedef MethodNativeFunction_5< CallClassType, SelfClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type > NF;
	Ref< NF > nf = new NF();
	nf->m_object = object;
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}

template <
	typename CallClassType,
	typename SelfClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, CallClassType* object, ReturnType (CallClassType::*method)(SelfClassType*, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type) const)
{
	typedef MethodNativeFunction_6< CallClassType, SelfClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type > NF;
	Ref< NF > nf = new NF();
	nf->m_object = object;
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}

// \}


// \group Self methods
// \{

template <
	typename ClassType,
	typename ReturnType
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, ReturnType (ClassType::*method)())
{
	typedef MethodNativeFunction_self_0< ClassType, ReturnType > NF;
	Ref< NF > nf = new NF();
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, ReturnType (ClassType::*method)(Argument1Type))
{
	typedef MethodNativeFunction_self_1< ClassType, ReturnType, Argument1Type > NF;
	Ref< NF > nf = new NF();
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, ReturnType (ClassType::*method)(Argument1Type, Argument2Type))
{
	typedef MethodNativeFunction_self_2< ClassType, ReturnType, Argument1Type, Argument2Type > NF;
	Ref< NF > nf = new NF();
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type))
{
	typedef MethodNativeFunction_self_3< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type > NF;
	Ref< NF > nf = new NF();
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type))
{
	typedef MethodNativeFunction_self_5< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type > NF;
	Ref< NF > nf = new NF();
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}

template <
	typename ClassType,
	typename ReturnType,
	typename Argument1Type,
	typename Argument2Type,
	typename Argument3Type,
	typename Argument4Type,
	typename Argument5Type,
	typename Argument6Type
>
Ref< ActionFunctionNative > createNativeFunction(ActionContext* context, ReturnType (ClassType::*method)(Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type))
{
	typedef MethodNativeFunction_self_6< ClassType, ReturnType, Argument1Type, Argument2Type, Argument3Type, Argument4Type, Argument5Type, Argument6Type > NF;
	Ref< NF > nf = new NF();
	nf->m_method = method;
	return new ActionFunctionNative(context, nf);
}


// \}

// \group Polymorphic functions
// \{

struct PolymorphicNativeFunction : public INativeFunction
{
	Ref< ActionFunctionNative > m_fnptr[5];

	PolymorphicNativeFunction(
		ActionFunctionNative* fnptr_0,
		ActionFunctionNative* fnptr_1 = 0,
		ActionFunctionNative* fnptr_2 = 0,
		ActionFunctionNative* fnptr_3 = 0,
		ActionFunctionNative* fnptr_4 = 0
	)
	{
		m_fnptr[0] = fnptr_0;
		m_fnptr[1] = fnptr_1;
		m_fnptr[2] = fnptr_2;
		m_fnptr[3] = fnptr_3;
		m_fnptr[4] = fnptr_4;
	}

	virtual void call(CallArgs& ca)
	{
		ActionFunctionNative* fn = m_fnptr[ca.args.size()];
		if (fn)
			ca.ret = fn->call(ca.self, ca.super, ca.args);
		else
		{
			T_IF_VERBOSE(
				log::warning << L"Trying to call polymorphic native function with " << ca.args.size() << L" argument(s)" << Endl;
			)
		}
	}
};

Ref< ActionFunctionNative > T_DLLCLASS createPolymorphicFunction(
	ActionContext* context,
	ActionFunctionNative* fnptr_0,
	ActionFunctionNative* fnptr_1 = 0,
	ActionFunctionNative* fnptr_2 = 0,
	ActionFunctionNative* fnptr_3 = 0,
	ActionFunctionNative* fnptr_4 = 0
);

// \}

	}
}

#endif	// traktor_flash_ActionFunctionNative_H
