/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_Sprite_H
#define traktor_flash_Sprite_H

#include <map>
#include "Core/RefArray.h"
#include "Flash/Character.h"
#include "Flash/SwfTypes.h"

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

class ActionContext;
class Frame;
class IActionVM;
class IActionVMImage;

/*! \brief Flash sprite.
 * \ingroup Flash
 */
class T_DLLCLASS Sprite : public Character
{
	T_RTTI_CLASS;

public:
	Sprite();

	Sprite(uint16_t id, uint16_t frameRate);

	uint16_t getFrameRate() const;

	void addFrame(Frame* frame);

	uint32_t getFrameCount() const;

	Frame* getFrame(uint32_t frameId) const;

	int findFrame(const std::string& frameLabel) const;

	void addInitActionScript(const IActionVMImage* initActionScript);

	const RefArray< const IActionVMImage >& getInitActionScripts() const;

	void setScalingGrid(const Aabb2& scalingGrid);

	const Aabb2& getScalingGrid() const;

	virtual Ref< CharacterInstance > createInstance(
		ActionContext* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform,
		const ActionObject* initObject,
		const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	uint16_t m_frameRate;
	RefArray< Frame > m_frames;
	RefArray< const IActionVMImage > m_initActionScripts;
	Aabb2 m_scalingGrid;
};

	}
}

#endif	// traktor_flash_Sprite_H
