#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Sound/Player/ISoundHandle.h"
#include "Spark/Canvas.h"
#include "Spark/Character.h"
#include "Spark/CharacterData.h"
#include "Spark/Context.h"
#include "Spark/IComponent.h"
#include "Spark/IComponentData.h"
#include "Spark/Shape.h"
#include "Spark/SparkClassFactory.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteData.h"
#include "Spark/Text.h"
#include "Spark/TextData.h"
#include "Spark/Script/ScriptComponent.h"
#include "Spark/Script/ScriptComponentData.h"
#include "Spark/Sound/SoundComponent.h"
#include "Spark/Sound/SoundComponentData.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

void Canvas_quadricTo_4(Canvas* self, float x1, float y1, float x, float y)
{
	self->quadricTo(x1, y1, x, y);
}

void Canvas_quadricTo_2(Canvas* self, float x, float y)
{
	self->quadricTo(x, y);
}

void Canvas_cubicTo_6(Canvas* self, float x1, float y1, float x2, float y2, float x, float y)
{
	self->cubicTo(x1, y1, x2, y2, x, y);
}

void Canvas_cubicTo_4(Canvas* self, float x2, float y2, float x, float y)
{
	self->cubicTo(x2, y2, x, y);
}

void Character_setPosition(Character* self, float x, float y)
{
	self->setPosition(Vector2(x, y));
}

void Character_setScale(Character* self, float x, float y)
{
	self->setScale(Vector2(x, y));
}

Shape* Sprite_getShape(Sprite* self)
{
	return self->getShape().getResource();
}

void Sprite_remove(Sprite* self, const Any& a)
{
	if (a.isObject())
		self->remove(mandatory_non_null_type_cast< Character* >(a.getObject()));
	else if (a.isInteger())
		self->remove(a.getIntegerUnsafe());
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.SparkClassFactory", 0, SparkClassFactory, IRuntimeClassFactory)

void SparkClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< Context > > classContext = new AutoRuntimeClass< Context >();
	registrar->registerClass(classContext);

	Ref< AutoRuntimeClass< IComponentData > > classIComponentData = new AutoRuntimeClass< IComponentData >();
	registrar->registerClass(classIComponentData);

	Ref< AutoRuntimeClass< IComponent > > classIComponent = new AutoRuntimeClass< IComponent >();
	registrar->registerClass(classIComponent);

	Ref< AutoRuntimeClass< ScriptComponentData > > classScriptComponentData = new AutoRuntimeClass< ScriptComponentData >();
	registrar->registerClass(classScriptComponentData);

	Ref< AutoRuntimeClass< ScriptComponent > > classScriptComponent = new AutoRuntimeClass< ScriptComponent >();
	registrar->registerClass(classScriptComponent);

	Ref< AutoRuntimeClass< SoundComponentData > > classSoundComponentData = new AutoRuntimeClass< SoundComponentData >();
	registrar->registerClass(classSoundComponentData);

	Ref< AutoRuntimeClass< SoundComponent > > classSoundComponent = new AutoRuntimeClass< SoundComponent >();
	classSoundComponent->addMethod("play", &SoundComponent::play);
	registrar->registerClass(classSoundComponent);

	Ref< AutoRuntimeClass< CharacterData > > classCharacterData = new AutoRuntimeClass< CharacterData >();
	registrar->registerClass(classCharacterData);

	Ref< AutoRuntimeClass< Character > > classCharacter = new AutoRuntimeClass< Character >();
	classCharacter->addMethod("setTransform", &Character::setTransform);
	classCharacter->addMethod("getTransform", &Character::getTransform);
	classCharacter->addMethod("setPosition", &Character::setPosition);
	classCharacter->addMethod("setPosition", &Character_setPosition);
	classCharacter->addMethod("getPosition", &Character::getPosition);
	classCharacter->addMethod("setScale", &Character::setScale);
	classCharacter->addMethod("setScale", &Character_setScale);
	classCharacter->addMethod("getScale", &Character::getScale);
	classCharacter->addMethod("setRotation", &Character::setRotation);
	classCharacter->addMethod("getRotation", &Character::getRotation);
	classCharacter->addMethod("setVisible", &Character::setVisible);
	classCharacter->addMethod("getVisible", &Character::getVisible);
	classCharacter->addMethod("getBounds", &Character::getBounds);
	registrar->registerClass(classCharacter);

	Ref< AutoRuntimeClass< Shape > > classShape = new AutoRuntimeClass< Shape >();
	registrar->registerClass(classShape);

	Ref< AutoRuntimeClass< SpriteData > > classSpriteData = new AutoRuntimeClass< SpriteData >();
	registrar->registerClass(classSpriteData);

	Ref< AutoRuntimeClass< Sprite > > classSprite = new AutoRuntimeClass< Sprite >();
	classSprite->addMethod("getContext", &Sprite::getContext);
	classSprite->addMethod("setShape", &Sprite::setShape);
	classSprite->addMethod("getShape", &Sprite_getShape);
	classSprite->addMethod("create", &Sprite::create);
	classSprite->addMethod("place", &Sprite::place);
	classSprite->addMethod("remove", &Sprite_remove);
	classSprite->addMethod("setAlpha", &Sprite::setAlpha);
	classSprite->addMethod("getAlpha", &Sprite::getAlpha);
	classSprite->addMethod("setComponent", &Sprite::setComponent);
	classSprite->addMethod("getComponent", &Sprite::getComponent);
	registrar->registerClass(classSprite);

	Ref< AutoRuntimeClass< TextData > > classTextData = new AutoRuntimeClass< TextData >();
	registrar->registerClass(classTextData);

	Ref< AutoRuntimeClass< Text > > classText = new AutoRuntimeClass< Text >();
	classText->addMethod("setText", &Text::setText);
	classText->addMethod("getText", &Text::getText);
	classText->addMethod("setHeight", &Text::setHeight);
	classText->addMethod("getHeight", &Text::getHeight);
	classText->addMethod("setBounds", &Text::setBounds);
	classText->addMethod("setAlpha", &Text::setAlpha);
	classText->addMethod("getAlpha", &Text::getAlpha);
	registrar->registerClass(classText);

	Ref< AutoRuntimeClass< Canvas > > classCanvas = new AutoRuntimeClass< Canvas >();
	classCanvas->addConstructor();
	classCanvas->addMethod("clear", &Canvas::clear);
	classCanvas->addMethod("moveTo", &Canvas::moveTo);
	classCanvas->addMethod("lineTo", &Canvas::lineTo);
	classCanvas->addMethod("quadricTo", &Canvas_quadricTo_4);
	classCanvas->addMethod("quadricTo", &Canvas_quadricTo_2);
	classCanvas->addMethod("cubicTo", &Canvas_cubicTo_6);
	classCanvas->addMethod("cubicTo", &Canvas_cubicTo_4);
	classCanvas->addMethod("close", &Canvas::close);
	classCanvas->addMethod("rect", &Canvas::rect);
	classCanvas->addMethod("circle", &Canvas::circle);
	classCanvas->addMethod("fill", &Canvas::fill);
	classCanvas->addMethod("stroke", &Canvas::stroke);
	classCanvas->addMethod("createShape", &Canvas::createShape);
	registrar->registerClass(classCanvas);
}

	}
}
