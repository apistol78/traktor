#include "Script/Delegate.h"
#include "Script/IScriptClass.h"
#include "Script/IScriptContext.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

class DelegateClass : public IScriptClass
{
public:
	virtual const TypeInfo& getExportType() const
	{
		return type_of< Delegate >();
	}

	virtual bool haveConstructor() const
	{
		return true;
	}

	virtual Ref< Object > construct(const InvokeParam& param, uint32_t argc, const Any* argv) const
	{
		return new Delegate(param.context, argv[0].getObject(), argv[1].getString());
	}

	virtual uint32_t getMethodCount() const
	{
		return 0;
	}

	virtual std::wstring getMethodName(uint32_t methodId) const
	{
		return L"";
	}

	virtual Any invoke(const InvokeParam& param, uint32_t methodId, uint32_t argc, const Any* argv) const
	{
		return Any();
	}

	virtual Any invokeUnknown(const InvokeParam& param, const std::wstring& methodName, uint32_t argc, const Any* argv) const
	{
		return Any();
	}

	virtual uint32_t getPropertyCount() const
	{
		return 0;
	}

	virtual std::wstring getPropertyName(uint32_t propertyId) const
	{
		return L"";
	}

	virtual Any getPropertyValue(const InvokeParam& param, uint32_t propertyId) const
	{
		return Any();
	}

	virtual void setPropertyValue(const InvokeParam& param, uint32_t propertyId, const Any& value) const
	{
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.Delegate", Delegate, Object)

Delegate::Delegate(IScriptContext* context, Object* object, const std::wstring& methodName)
:	m_context(context)
,	m_object(object)
,	m_methodName(methodName)
{
}

void Delegate::invoke(uint32_t argc, const Any* argv)
{
	m_context->executeMethod(m_object, m_methodName, argc, argv);
}

void registerDelegateClasses(IScriptManager* scriptManager)
{
	scriptManager->registerClass(new DelegateClass());
}

	}
}
