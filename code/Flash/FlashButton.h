/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_FlashButton_H
#define traktor_flash_FlashButton_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix33.h"
#include "Flash/ColorTransform.h"
#include "Flash/FlashCharacter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class IActionVMImage;

/*! \brief Flash button character.
 * \ingroup Flash
 */
class T_DLLCLASS FlashButton : public FlashCharacter
{
	T_RTTI_CLASS;

public:
	enum ConditionMasks
	{
		CmIdleToOverDown = 1 << 0,
		CmOutDownToIdle = 1 << 1,
		CmOutDownToOverDown = 1 << 2,
		CmOverDownToOutDown = 1 << 3,
		CmOverDownToOverUp = 1 << 4,
		CmOverUpToOverDown = 1 << 5,
		CmOverUpToIdle = 1 << 6,
		CmIdleToOverUp = 1 << 7,
		CmOverDownToIdle = 1 << 8
	};

	enum StateMasks
	{
		SmHitTest = 1 << 0,
		SmDown = 1 << 1,
		SmOver = 1 << 2,
		SmUp = 1 << 3
	};

	struct ButtonLayer
	{
		uint8_t state;
		uint16_t characterId;
		uint16_t placeDepth;
		Matrix33 placeMatrix;
		ColorTransform cxform;

		ButtonLayer()
		:	state(0)
		,	characterId(0)
		,	placeDepth(0)
		,	placeMatrix(Matrix33::identity())
		{
		}

		void serialize(ISerializer& s);
	};

	struct ButtonCondition
	{
		uint8_t key;
		uint16_t mask;
		Ref< const IActionVMImage > script;

		ButtonCondition()
		:	key(0)
		,	mask(0)
		{
		}

		void serialize(ISerializer& s);
	};

	typedef AlignedVector< ButtonLayer > button_layers_t;
	typedef AlignedVector< ButtonCondition > button_conditions_t;

	FlashButton();

	FlashButton(uint16_t id);

	/*! \brief Add button layer.
	 *
	 * \param layer Button layer description.
	 */
	void addButtonLayer(const ButtonLayer& layer);

	/*! \brief Get button layers.
	 *
	 * \return Button layers.
	 */
	const button_layers_t& getButtonLayers() const;

	/*! \brief Add button condition script.
	 *
	 * \param condition Condition script.
	 */
	void addButtonCondition(const ButtonCondition& condition);

	/*! \brief Get button condition scripts.
	 *
	 * \return Condition scripts.
	 */
	const button_conditions_t& getButtonConditions() const;

	virtual Ref< FlashCharacterInstance > createInstance(
		ActionContext* context,
		FlashDictionary* dictionary,
		FlashCharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform,
		const ActionObject* initObject,
		const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	button_layers_t m_layers;
	button_conditions_t m_conditions;
};

	}
}

#endif	// traktor_flash_FlashButton_H
