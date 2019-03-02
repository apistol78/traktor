#include "Resource/IResourceManager.h"
#include "Spark/Character.h"
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

Ref< Character > ExternalFactory::create(const Context* context, const ICharacterBuilder* builder, const CharacterData* characterData, const Character* parent, const std::wstring& name) const
{
	const ExternalData* xtrnalData = mandatory_non_null_type_cast< const ExternalData* >(characterData);

	resource::Proxy< CharacterData > xtrnalCharacter;
	if (!context->getResourceManager()->bind(xtrnalData->m_reference, xtrnalCharacter))
		return 0;

	Ref< Character > xtrnal = builder->create(context, xtrnalCharacter, parent, name);
	if (!xtrnal)
		return 0;

	xtrnal->setTransform(xtrnalData->getTransform());
	return xtrnal;
}

	}
}
