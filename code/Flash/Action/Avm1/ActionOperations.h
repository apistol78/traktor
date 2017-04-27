/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ActionOperations_H
#define traktor_flash_ActionOperations_H

#include "Core/Config.h"
#include "Flash/Action/Avm1/ActionOpcodes.h"

namespace traktor
{

class OutputStream;
class Timer;

	namespace flash
	{

class ActionContext;
class ActionFrame;
class ActionObject;
class ActionVMImage1;
class FlashSpriteInstance;

struct PreparationState
{
	ActionVMImage1* image;
	uint8_t* pc;
	uint8_t* npc;
	uint8_t* data;
	uint16_t length;
};

struct ExecutionState
{
	const ActionVMImage1* image;
	ActionFrame* frame;
	const uint8_t* pc;
	const uint8_t* npc;
	const uint8_t* data;
	uint16_t length;
	const Timer* timer;

	// Scope instance.
	Ref< ActionObject > with;

	// Cached instances.
	ActionContext* context;
	ActionObject* self;
	ActionObject* global;
	FlashSpriteInstance* movieClip;

	// Trace instances.
	OutputStream* trace;
};

struct OperationInfo
{
	ActionOpcode op;
	const char* name;
	void (*prepare)(PreparationState& state);
	void (*execute)(ExecutionState& state);
};

extern const OperationInfo c_operationInfos[];

	}
}

#endif	// traktor_flash_ActionOperations_H
