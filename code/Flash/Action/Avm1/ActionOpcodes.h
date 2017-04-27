/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ActionOpcodes_H
#define traktor_flash_ActionOpcodes_H

namespace traktor
{
	namespace flash
	{

/*! \brief ActionScript opcodes.
 * \ingroup Flash
 */
enum ActionOpcode
{
	AopEnd					= 0x00,
	AopGotoFrame			= 0x81,
	AopGetUrl				= 0x83,
	AopNextFrame			= 0x04,
	AopPrevFrame			= 0x05,
	AopPlay					= 0x06,
	AopStop					= 0x07,
	AopToggleQuality		= 0x08,
	AopStopSounds			= 0x09,
	AopWaitForFrame			= 0x8a,
	AopSetTarget			= 0x8b,
	AopGotoLabel			= 0x8c,
	AopAdd					= 0x0a,
	AopSubtract				= 0x0b,
	AopMultiply				= 0x0c,
	AopDivide				= 0x0d,
	AopEqual				= 0x0e,
	AopLessThan				= 0x0f,
	AopLogicalAnd			= 0x10,
	AopLogicalOr			= 0x11,
	AopLogicalNot			= 0x12,
	AopStringEqual			= 0x13,
	AopStringLength			= 0x14,
	AopStringExtract		= 0x15,
	AopPop					= 0x17,
	AopInt					= 0x18,
	AopGetVariable			= 0x1c,
	AopSetVariable			= 0x1d,
	AopSetTargetExpression	= 0x20,
	AopStringConcat			= 0x21,
	AopGetProperty			= 0x22,
	AopSetProperty			= 0x23,
	AopCloneSprite			= 0x24,
	AopRemoveSprite			= 0x25,
	AopTrace				= 0x26,
	AopStartDragMovie		= 0x27,
	AopStopDragMovie		= 0x28,
	AopStringCompare		= 0x29,
	AopThrow				= 0x2a,
	AopCastOp				= 0x2b,
	AopImplementsOp			= 0x2c,
	AopRandom				= 0x30,
	AopMbLength				= 0x31,
	AopOrd					= 0x32,
	AopChr					= 0x33,
	AopGetTime				= 0x34,
	AopMbSubString			= 0x35,
	AopMbOrd				= 0x36,
	AopMbChr				= 0x37,
	AopWaitForFrameExpression	= 0x8d,
	AopPushData					= 0x96,
	AopBranchAlways				= 0x99,
	AopGetUrl2					= 0x9a,
	AopBranchIfTrue				= 0x9d,
	AopCallFrame				= 0x9e,
	AopGotoFrame2			= 0x9f,
	AopDelete				= 0x3a,
	AopDelete2				= 0x3b,
	AopDefineLocal			= 0x3c,
	AopCallFunction			= 0x3d,
	AopReturn				= 0x3e,
	AopModulo				= 0x3f,
	AopNew					= 0x40,
	AopDefineLocal2			= 0x41,
	AopInitArray			= 0x42,
	AopInitObject			= 0x43,
	AopTypeOf				= 0x44,
	AopTargetPath			= 0x45,
	AopEnumerate			= 0x46,
	AopNewAdd				= 0x47,
	AopNewLessThan			= 0x48,
	AopNewEquals			= 0x49,
	AopToNumber				= 0x4a,
	AopToString				= 0x4b,
	AopDup					= 0x4c,
	AopSwap					= 0x4d,
	AopGetMember			= 0x4e,
	AopSetMember			= 0x4f,
	AopIncrement			= 0x50,
	AopDecrement			= 0x51,
	AopCallMethod			= 0x52,
	AopNewMethod			= 0x53,
	AopInstanceOf			= 0x54,
	AopEnum2				= 0x55,
	AopBitwiseAnd			= 0x60,
	AopBitwiseOr			= 0x61,
	AopBitwiseXor			= 0x62,
	AopShiftLeft			= 0x63,
	AopShiftRight			= 0x64,
	AopShiftRight2			= 0x65,
	AopStrictEq				= 0x66,
	AopGreater				= 0x67,
	AopStringGreater		= 0x68,
	AopExtends				= 0x69,
	AopConstantPool			= 0x88,
	AopDefineFunction2		= 0x8e,
	AopTry					= 0x8f,
	AopWith					= 0x94,
	AopDefineFunction		= 0x9b,
	AopSetRegister			= 0x87,

	AopReserved				= 0xff
};

	}
}

#endif	// traktor_flash_ActionOpcodes_H
