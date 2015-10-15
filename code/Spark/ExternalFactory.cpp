#include "Resource/IResourceManager.h"
#include "Spark/CharacterInstance.h"
#include "Spark/Context.h"
#include "Spark/External.h"
#include "Spark/ExternalFactory.h"
#include "Spark/ICharacterBuilder.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ExternalFactory", ExternalFactory, ICharacterFactory)

TypeInfoSet ExternalFactory::getCharacterTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< External >());
	return typeSet;
}

Ref< CharacterInstance > ExternalFactory::create(const Context* context, const ICharacterBuilder* builder, const Character* character, const CharacterInstance* parent, const std::wstring& name) const
{
	const External* xtrnal = mandatory_non_null_type_cast< const External* >(character);

	resource::Proxy< Character > xtrnalCharacter;
	if (!context->getResourceManager()->bind(xtrnal->m_reference, xtrnalCharacter))
		return 0;

	Ref< CharacterInstance > xtrnalInstance = builder->create(context, xtrnalCharacter, parent, name);
	if (!xtrnalInstance)
		return 0;

	xtrnalInstance->setTransform(xtrnal->getTransform());
	return xtrnalInstance;
}

	}
}
