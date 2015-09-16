#include "Spark/Character.h"
#include "Spark/CharacterInstance.h"
#include "Spark/IComponent.h"
#include "Spark/IComponentInstance.h"
#include "Spark/SparkClassFactory.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"
#include "Spark/Text.h"
#include "Spark/TextInstance.h"
#include "Spark/Tween.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.SparkClassFactory", 0, SparkClassFactory, IRuntimeClassFactory)

void SparkClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< IComponent > > classComponent = new AutoRuntimeClass< IComponent >();
	registrar->registerClass(classComponent);

	Ref< AutoRuntimeClass< IComponentInstance > > classComponentInstance = new AutoRuntimeClass< IComponentInstance >();
	registrar->registerClass(classComponentInstance);

	Ref< AutoRuntimeClass< Character > > classCharacter = new AutoRuntimeClass< Character >();
	registrar->registerClass(classCharacter);

	Ref< AutoRuntimeClass< CharacterInstance > > classCharacterInstance = new AutoRuntimeClass< CharacterInstance >();
	classCharacterInstance->addMethod("setTransform", &CharacterInstance::setTransform);
	classCharacterInstance->addMethod("getTransform", &CharacterInstance::getTransform);
	classCharacterInstance->addMethod("setPosition", &CharacterInstance::setPosition);
	classCharacterInstance->addMethod("getPosition", &CharacterInstance::getPosition);
	classCharacterInstance->addMethod("setScale", &CharacterInstance::setScale);
	classCharacterInstance->addMethod("getScale", &CharacterInstance::getScale);
	classCharacterInstance->addMethod("setRotation", &CharacterInstance::setRotation);
	classCharacterInstance->addMethod("getRotation", &CharacterInstance::getRotation);
	classCharacterInstance->addMethod("addComponent", &CharacterInstance::addComponent);
	classCharacterInstance->addMethod("getComponents", &CharacterInstance::getComponents);
	registrar->registerClass(classCharacterInstance);

	Ref< AutoRuntimeClass< Sprite > > classSprite = new AutoRuntimeClass< Sprite >();
	registrar->registerClass(classSprite);

	Ref< AutoRuntimeClass< SpriteInstance > > classSpriteInstance = new AutoRuntimeClass< SpriteInstance >();
	classSpriteInstance->addMethod("create", &SpriteInstance::create);
	classSpriteInstance->addMethod("place", &SpriteInstance::place);
	classSpriteInstance->addMethod("remove", &SpriteInstance::remove);
	registrar->registerClass(classSpriteInstance);

	Ref< AutoRuntimeClass< Text > > classText = new AutoRuntimeClass< Text >();
	registrar->registerClass(classText);

	Ref< AutoRuntimeClass< TextInstance > > classTextInstance = new AutoRuntimeClass< TextInstance >();
	classTextInstance->addMethod("setText", &TextInstance::setText);
	classTextInstance->addMethod("getText", &TextInstance::getText);
	classTextInstance->addMethod("setHeight", &TextInstance::setHeight);
	classTextInstance->addMethod("getHeight", &TextInstance::getHeight);
	registrar->registerClass(classTextInstance);

	Ref< AutoRuntimeClass< Tween::IListener > > classTweenIListener = new AutoRuntimeClass< Tween::IListener >();
	registrar->registerClass(classTweenIListener);

	Ref< AutoRuntimeClass< Tween > > classTween = new AutoRuntimeClass< Tween >();
	classTween->addConstructor< float, float, float, Tween::IListener* >();
	registrar->registerClass(classTween);
}

	}
}
