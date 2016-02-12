#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Json/JsonArray.h"
#include "Json/JsonClassFactory.h"
#include "Json/JsonDocument.h"
#include "Json/JsonMember.h"
#include "Json/JsonObject.h"

namespace traktor
{
	namespace json
	{
		namespace
		{

const Any& JsonArray_get_1(JsonArray* self, uint32_t index)
{
	return self->get(index);
}

//const AlignedVector< Any >& JsonArray_get_0(JsonArray* self)
//{
//	return self->get();
//}

JsonMember* JsonObject_get_1(JsonObject* self, uint32_t index)
{
	return self->get(index);
}

const RefArray< JsonMember >& JsonObject_get_0(JsonObject* self)
{
	return self->get();
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.json.JsonClassFactory", 0, JsonClassFactory, IRuntimeClassFactory)

void JsonClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< JsonArray > > classJsonArray = new AutoRuntimeClass< JsonArray >();
	classJsonArray->addConstructor();
	classJsonArray->addMethod("push", &JsonArray::push);
	classJsonArray->addMethod("empty", &JsonArray::empty);
	classJsonArray->addMethod("front", &JsonArray::front);
	classJsonArray->addMethod("back", &JsonArray::back);
	classJsonArray->addMethod("size", &JsonArray::size);
	classJsonArray->addMethod("get", &JsonArray_get_1);
	//classJsonArray->addMethod("get", &JsonArray_get_0);
	registrar->registerClass(classJsonArray);

	Ref< AutoRuntimeClass< JsonDocument > > classJsonDocument = new AutoRuntimeClass< JsonDocument >();
	classJsonDocument->addConstructor();
	classJsonDocument->addMethod("loadFromFile", &JsonDocument::loadFromFile);
	classJsonDocument->addMethod("loadFromStream", &JsonDocument::loadFromStream);
	classJsonDocument->addMethod("loadFromText", &JsonDocument::loadFromText);
	classJsonDocument->addMethod("saveToFile", &JsonDocument::saveToFile);
	classJsonDocument->addMethod("saveToStream", &JsonDocument::saveToStream);
	registrar->registerClass(classJsonDocument);

	Ref< AutoRuntimeClass< JsonMember > > classJsonMember = new AutoRuntimeClass< JsonMember >();
	classJsonMember->addConstructor();
	classJsonMember->addConstructor< const std::wstring&, const Any& >();
	classJsonMember->addMethod("getName", &JsonMember::getName);
	classJsonMember->addMethod("setValue", &JsonMember::setValue);
	classJsonMember->addMethod("getValue", &JsonMember::getValue);
	registrar->registerClass(classJsonMember);

	Ref< AutoRuntimeClass< JsonObject > > classJsonObject = new AutoRuntimeClass< JsonObject >();
	classJsonObject->addConstructor();
	classJsonObject->addMethod("push", &JsonObject::push);
	classJsonObject->addMethod("empty", &JsonObject::empty);
	classJsonObject->addMethod("front", &JsonObject::front);
	classJsonObject->addMethod("back", &JsonObject::back);
	classJsonObject->addMethod("size", &JsonObject::size);
	classJsonObject->addMethod("getMember", &JsonObject::getMember);
	classJsonObject->addMethod("setMemberValue", &JsonObject::setMemberValue);
	classJsonObject->addMethod("getMemberValue", &JsonObject::getMemberValue);
	classJsonObject->addMethod("getValue", &JsonObject::getValue);
	classJsonObject->addMethod("get", &JsonObject_get_1);
	classJsonObject->addMethod("get", &JsonObject_get_0);
	registrar->registerClass(classJsonObject);
}

	}
}
