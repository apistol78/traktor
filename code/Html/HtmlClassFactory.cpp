/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
//#include "Core/Class/IRuntimeDelegate.h"
//#include "Core/Class/IRuntimeObject.h"
#include "Html/HtmlClassFactory.h"
#include "Html/Attribute.h"
#include "Html/Comment.h"
#include "Html/Document.h"
#include "Html/Element.h"
#include "Html/Text.h"

namespace traktor
{
	namespace html
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.html.HtmlClassFactory", 0, HtmlClassFactory, IRuntimeClassFactory)

void HtmlClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< Attribute > > classAttribute = new AutoRuntimeClass< Attribute >();
	registrar->registerClass(classAttribute);

	Ref< AutoRuntimeClass< Comment > > classComment = new AutoRuntimeClass< Comment >();
	registrar->registerClass(classComment);

	Ref< AutoRuntimeClass< Document > > classDocument = new AutoRuntimeClass< Document >();
	registrar->registerClass(classDocument);

	Ref< AutoRuntimeClass< Element > > classElement = new AutoRuntimeClass< Element >();
	registrar->registerClass(classElement);

	Ref< AutoRuntimeClass< Text > > classText = new AutoRuntimeClass< Text >();
	registrar->registerClass(classText);
}

	}
}
