#include "Core/Class/PolymorphicDispatch.h"
#include "Core/Class/RuntimeClass.h"

namespace traktor
{

void RuntimeClass::addConstant(const char* const name, const Any& value)
{
	ConstInfo ci;
	ci.name = name;
	ci.value = value;
	m_consts.push_back(ci);
}

const IRuntimeDispatch* RuntimeClass::getConstructorDispatch() const
{
	return m_constructor;
}

uint32_t RuntimeClass::getConstantCount() const
{
	return uint32_t(m_consts.size());
}

std::string RuntimeClass::getConstantName(uint32_t constId) const
{
	return m_consts[constId].name;
}

Any RuntimeClass::getConstantValue(uint32_t constId) const
{
	return m_consts[constId].value;
}

uint32_t RuntimeClass::getMethodCount() const
{
	return uint32_t(m_methods.size());
}

std::string RuntimeClass::getMethodName(uint32_t methodId) const
{
	return m_methods[methodId].name;
}

const IRuntimeDispatch* RuntimeClass::getMethodDispatch(uint32_t methodId) const
{
	return m_methods[methodId].dispatch;
}

uint32_t RuntimeClass::getStaticMethodCount() const
{
	return uint32_t(m_staticMethods.size());
}

std::string RuntimeClass::getStaticMethodName(uint32_t methodId) const
{
	return m_staticMethods[methodId].name;
}

const IRuntimeDispatch* RuntimeClass::getStaticMethodDispatch(uint32_t methodId) const
{
	return m_staticMethods[methodId].dispatch;
}

uint32_t RuntimeClass::getPropertiesCount() const
{
	return uint32_t(m_properties.size());
}

std::string RuntimeClass::getPropertyName(uint32_t propertyId) const
{
	return m_properties[propertyId].name;
}

const IRuntimeDispatch* RuntimeClass::getPropertyGetDispatch(uint32_t propertyId) const
{
	return m_properties[propertyId].getter;
}

const IRuntimeDispatch* RuntimeClass::getPropertySetDispatch(uint32_t propertyId) const
{
	return m_properties[propertyId].setter;
}

const IRuntimeDispatch* RuntimeClass::getOperatorDispatch(OperatorType op) const
{
	return m_operators[op];
}

const IRuntimeDispatch* RuntimeClass::getUnknownDispatch() const
{
	return m_unknown;
}

void RuntimeClass::addConstructor(uint32_t argc, IRuntimeDispatch* constructor)
{
	if (m_constructor)
	{
		if (!is_a< PolymorphicDispatch >(m_constructor))
		{
			Ref< PolymorphicDispatch > pd = new PolymorphicDispatch();
			pd->set(m_constructorArgc, m_constructor);
			m_constructor = pd;
		}
		mandatory_non_null_type_cast< PolymorphicDispatch* >(m_constructor)->set(argc, constructor);
		return;
	}

	m_constructor = constructor;
	m_constructorArgc = argc;
}

void RuntimeClass::addMethod(const char* const methodName, uint32_t argc, IRuntimeDispatch* method)
{
	for (typename AlignedVector< MethodInfo >::iterator i = m_methods.begin(); i != m_methods.end(); ++i)
	{
		if (i->name == methodName)
		{
			if (!is_a< PolymorphicDispatch >(i->dispatch))
			{
				Ref< PolymorphicDispatch > pd = new PolymorphicDispatch();
				pd->set(i->argc, i->dispatch);
				i->dispatch = pd;
			}
			mandatory_non_null_type_cast< PolymorphicDispatch* >(i->dispatch)->set(argc, method);
			return;
		}
	}

	MethodInfo& m = m_methods.push_back();
	m.name = methodName;
	m.argc = argc;
	m.dispatch = method;
}

void RuntimeClass::addStaticMethod(const char* const methodName, uint32_t argc, IRuntimeDispatch* method)
{
	for (typename AlignedVector< MethodInfo >::iterator i = m_staticMethods.begin(); i != m_staticMethods.end(); ++i)
	{
		if (i->name == methodName)
		{
			if (!is_a< PolymorphicDispatch >(i->dispatch))
			{
				Ref< PolymorphicDispatch > pd = new PolymorphicDispatch();
				pd->set(i->argc, i->dispatch);
				i->dispatch = pd;
			}
			mandatory_non_null_type_cast< PolymorphicDispatch* >(i->dispatch)->set(argc, method);
			return;
		}
	}

	MethodInfo& m = m_staticMethods.push_back();
	m.name = methodName;
	m.argc = argc;
	m.dispatch = method;
}

void RuntimeClass::addProperty(const char* const propertyName, const std::wstring& signature, IRuntimeDispatch* setter, IRuntimeDispatch* getter)
{
	for (typename AlignedVector< PropertyInfo >::iterator i = m_properties.begin(); i != m_properties.end(); ++i)
	{
		if (i->name == propertyName)
		{
			i->setter = setter;
			i->getter = getter;
			return;
		}
	}

	PropertyInfo& p = m_properties.push_back();
	p.name = propertyName;
	p.setter = setter;
	p.getter = getter;
}

}
