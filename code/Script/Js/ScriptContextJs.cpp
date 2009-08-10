#include "Script/Js/ScriptContextJs.h"
#include "Script/IScriptClass.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

struct FunctionData
{
	Ref< ScriptContextJs > scriptContext;
	Ref< IScriptClass > scriptClass;
	uint32_t methodId;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptContextJs", ScriptContextJs, IScriptContext)

ScriptContextJs::ScriptContextJs()
{
}

ScriptContextJs::~ScriptContextJs()
{
	m_context.Dispose();
}

bool ScriptContextJs::create(const RefArray< IScriptClass >& registeredClasses)
{
	m_context = v8::Context::New();
	if (m_context.IsEmpty())
		return false;

	v8::Context::Scope contextScope(m_context);
	v8::HandleScope handleScope;

	// Setup all registered classes.
	for (RefArray< IScriptClass >::const_iterator i = registeredClasses.begin(); i != registeredClasses.end(); ++i)
	{
		IScriptClass* scriptClass = *i;
		T_ASSERT (scriptClass);

		const Type& exportType = scriptClass->getExportType();

		v8::Local< v8::FunctionTemplate > functionTemplate = v8::FunctionTemplate::New();

		for (uint32_t j = 0; j < scriptClass->getMethodCount(); ++j)
		{
			FunctionData* functionData = new FunctionData();
			functionData->scriptContext = this;
			functionData->scriptClass = scriptClass;
			functionData->methodId = j;

			v8::Local< v8::FunctionTemplate > methodTemplate = v8::FunctionTemplate::New(
				&ScriptContextJs::invokeMethod,
				v8::External::New(functionData)
			);

			functionTemplate->PrototypeTemplate()->Set(
				createString(scriptClass->getMethodName(j)),
				methodTemplate
			);
		}

		const Type* superType = scriptClass->getExportType().getSuper();
		T_ASSERT (superType);

		v8::Local< v8::Object > globalObject = m_context->Global();

		for (std::vector< RegisteredClass >::const_iterator j = m_classRegistry.begin(); j != m_classRegistry.end(); ++j)
		{
			if (superType == &j->scriptClass->getExportType())
			{
				functionTemplate->Inherit(j->functionTemplate);
				break;
			}
		}

		// Ensure we have an internal field available for C++ object pointer.
		v8::Local< v8::ObjectTemplate > objectTemplate = functionTemplate->InstanceTemplate();
		objectTemplate->SetInternalFieldCount(1);

		v8::Local< v8::Function > function = functionTemplate->GetFunction();

		globalObject->Set(
			createString(exportType.getName()),
			function->NewInstance()
		);

		RegisteredClass registeredClass =
		{ 
			scriptClass,
			v8::Persistent< v8::FunctionTemplate >::New(functionTemplate),
			v8::Persistent< v8::Function >::New(function)
		};
		m_classRegistry.push_back(registeredClass);
	}

	return true;
}

void ScriptContextJs::setGlobal(const std::wstring& globalName, const Any& globalValue)
{
	v8::Context::Scope contextScope(m_context);
	v8::HandleScope handleScope;

	m_context->Global()->Set(
		createString(globalName),
		toValue(globalValue)
	);
}

Any ScriptContextJs::getGlobal(const std::wstring& globalName)
{
	v8::Context::Scope contextScope(m_context);
	v8::HandleScope handleScope;

	v8::Local< v8::Value > value = m_context->Global()->Get(createString(globalName));
	return fromValue(value);
}

bool ScriptContextJs::executeScript(const std::wstring& script, bool compileOnly, IErrorCallback* errorCallback)
{
	v8::Context::Scope contextScope(m_context);
	v8::HandleScope handleScope;
	v8::TryCatch trycatch;

	v8::Local< v8::String > str = v8::Local< v8::String >::New(createString(script));
	
	v8::Local< v8::Script > obj = v8::Script::Compile(str);
	if (obj.IsEmpty())
	{
		v8::Handle< v8::Value > exception = trycatch.Exception();
		v8::String::AsciiValue xs(exception);
		if (errorCallback)
			errorCallback->syntaxError(0, mbstows(*xs));
		else
			log::error << L"Unhandled JavaScript exception occurred \"" << mbstows(*xs) << L"\"" << Endl;
		return false;
	}

	if (compileOnly)
		return true;

	v8::Local< v8::Value > result = obj->Run();
	if (result.IsEmpty())
	{
		v8::Handle< v8::Value > exception = trycatch.Exception();
		v8::String::AsciiValue xs(exception);
		if (errorCallback)
			errorCallback->otherError(mbstows(*xs));
		else
			log::error << L"Unhandled JavaScript exception occurred \"" << mbstows(*xs) << L"\"" << Endl;
		return false;
	}

	return true;
}

bool ScriptContextJs::haveFunction(const std::wstring& functionName) const
{
	v8::Context::Scope contextScope(m_context);
	v8::HandleScope handleScope;

	return m_context->Global()->Has(createString(functionName));
}

Any ScriptContextJs::executeFunction(const std::wstring& functionName, const std::vector< Any >& arguments)
{
	v8::Context::Scope contextScope(m_context);
	v8::HandleScope handleScope;
	v8::TryCatch trycatch;

	v8::Local< v8::Value > value = m_context->Global()->Get(createString(functionName));
	if (value.IsEmpty())
		return Any();

	v8::Local< v8::Function > function = v8::Local< v8::Function >::Cast(value);
	if (function.IsEmpty())
		return Any();

	std::vector< v8::Local< v8::Value > > argv(arguments.size());
	for (uint32_t i = 0; i < uint32_t(arguments.size()); ++i)
		argv[i] = v8::Local< v8::Value >::New(toValue(arguments[i]));

	v8::Local< v8::Value > result = function->Call(
		m_context->Global(),
		argv.size(),
		&argv[0]
	);

	if (result.IsEmpty())
	{
		v8::Handle< v8::Value > exception = trycatch.Exception();
		v8::String::AsciiValue xs(exception);
		log::error << L"Unhandled JavaScript exception occurred \"" << mbstows(*xs) << L"\"" << Endl;
		return Any();
	}

	return fromValue(result);
}

Any ScriptContextJs::executeMethod(Object* self, const std::wstring& methodName, const std::vector< Any >& arguments)
{
	v8::Context::Scope contextScope(m_context);
	v8::HandleScope handleScope;
	v8::TryCatch trycatch;

	v8::Local< v8::Value > value = m_context->Global()->Get(createString(methodName));
	if (value.IsEmpty())
		return Any();

	v8::Local< v8::Function > function = v8::Local< v8::Function >::Cast(value);
	if (function.IsEmpty())
		return Any();

	std::vector< v8::Local< v8::Value > > argv(arguments.size());
	for (uint32_t i = 0; i < uint32_t(arguments.size()); ++i)
		argv[i] = v8::Local< v8::Value >::New(toValue(arguments[i]));

	v8::Local< v8::Object > recv = v8::Local< v8::Object >::New(createObject(self));

	v8::Local< v8::Value > result = function->Call(
		recv,
		argv.size(),
		&argv[0]
	);

	if (result.IsEmpty())
	{
		v8::Handle< v8::Value > exception = trycatch.Exception();
		v8::String::AsciiValue xs(exception);
		log::error << L"Unhandled JavaScript exception occurred \"" << mbstows(*xs) << L"\"" << Endl;
		return Any();
	}

	return fromValue(result);
}

v8::Handle< v8::Value > ScriptContextJs::invokeMethod(const v8::Arguments& arguments)
{
	v8::Local< v8::Object > vthis = arguments.This();
	if (vthis->InternalFieldCount() <= 0)
		return v8::Undefined();

	v8::Local< v8::External > objectX = v8::Local< v8::External >::Cast(vthis->GetInternalField(0));
	Ref< Object >* objectRef = static_cast< Ref< Object >* >(objectX->Value());
	Ref< Object > object = *objectRef;

	v8::Local< v8::External > functionDataX = v8::Local< v8::External >::Cast(arguments.Data());
	FunctionData* functionData = static_cast< FunctionData* >(functionDataX->Value());

	std::vector< Any > args(arguments.Length());
	for (int i = 0; i < arguments.Length(); ++i)
		args[i] = functionData->scriptContext->fromValue(arguments[i]);

	Any result = functionData->scriptClass->invoke(
		object,
		functionData->methodId,
		args
	);

	return functionData->scriptContext->toValue(result);
}

void ScriptContextJs::weakHandleCallback(v8::Persistent< v8::Value > object, void* parameter)
{
	Ref< Object >* objectRef = reinterpret_cast< Ref< Object >* >(parameter);
	T_ASSERT (objectRef);

	delete objectRef;
}

v8::Handle< v8::String > ScriptContextJs::createString(const std::wstring& s) const
{
	return v8::String::New((const uint16_t*)s.c_str());
}

v8::Handle< v8::Object > ScriptContextJs::createObject(Object* object) const
{
	if (object)
	{
		const Type* objectType = &object->getType();
		for (std::vector< RegisteredClass >::const_iterator i = m_classRegistry.begin(); i != m_classRegistry.end(); ++i)
		{
			if (&i->scriptClass->getExportType() == objectType)
			{
				Ref< Object >* objectRef = new Ref< Object >(object);

				v8::Persistent< v8::Object > instanceHandle(i->function->NewInstance());
				instanceHandle.MakeWeak(objectRef, &ScriptContextJs::weakHandleCallback);
				instanceHandle->SetInternalField(0, v8::External::New(objectRef));

				return instanceHandle;
			}
		}
	}
	return v8::Object::New();
}

v8::Handle< v8::Value > ScriptContextJs::toValue(const Any& value) const
{
	if (value.isBoolean())
	{
		if (value.getBoolean())
			return v8::True();
		else
			return v8::False();
	}
	if (value.isInteger())
		return v8::Int32::New(value.getInteger());
	if (value.isFloat())
		return v8::Number::New(value.getFloat());
	if (value.isString())
		return createString(value.getString());
	if (value.isObject())
		return createObject(value.getObject());	

	return v8::Undefined();
}

Any ScriptContextJs::fromValue(v8::Handle< v8::Value > value) const
{
	if (value.IsEmpty())
		return Any();
	if (value->IsTrue())
		return Any(true);
	if (value->IsFalse())
		return Any(false);
	if (value->IsString())
	{
		v8::String::AsciiValue str(value);
		return Any(mbstows(*str));
	}
	if (value->IsInt32())
		return Any(int32_t(value->ToInt32()->Value()));
	if (value->IsNumber())
		return Any(float(value->ToNumber()->Value()));
	
	return Any();
}

	}
}
