#include "Resource/IResourceManager.h"
#include "Spark/CharacterInstance.h"
#include "Spark/Context.h"
#include "Spark/ExternalData.h"
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
	typeSet.insert(&type_of< ExternalData >());
	return typeSet;
}

Ref< CharacterInstance > ExternalFactory::create(const Context* context, const ICharacterBuilder* builder, const CharacterData* character, const CharacterInstance* parent, const std::wstring& name) const
{
	const ExternalData* xtrnal = mandatory_non_null_type_cast< const ExternalData* >(character);

	resource::Proxy< CharacterData > xtrnalCharacter;
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
