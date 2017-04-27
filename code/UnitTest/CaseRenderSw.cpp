/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "UnitTest/CaseRenderSw.h"
/*
#include "Render/Sw/Core/x86/JitX86.h"
#include "Render/Sw/Core/InterpreterFixed.h"
#include "Render/Sw/Core/Interpreter.h"
#include "Render/Sw/Core/IntrProgram.h"
*/
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Math/RandomGeometry.h"
#include "Core/Log/Log.h"

#pragma optimize("", off)

namespace traktor
{
	namespace
	{

bool equalFuzzy(const Vector4& a, const Vector4& b)
{
	const Scalar c_threshold(0.001f);
	Vector4 r = (a - b).absolute();
	return
		(r.x() <= c_threshold) &&
		(r.y() <= c_threshold) &&
		(r.z() <= c_threshold) &&
		(r.w() <= c_threshold);
}

	}

void CaseRenderSw::run()
{
/*
#if !defined(__GNUC__) && !defined(_WIN64)
	render::Interpreter interpreter;
	render::InterpreterFixed interpreterFixed;
	render::JitX86 jit;

	const Vector4 T_ALIGN16 inputUniforms[] =
	{
		Vector4(-1.0f, 0.0f, 0.0f, 0.0f),
		Vector4(0.0f, -1.0f, 0.0f, 0.0f),
		Vector4(0.0f, 0.0f, -1.0f, 0.0f),
		Vector4(0.0f, 0.0f, 0.0f,- 1.0f),

		Vector4(1.0f, 2.0f, 3.0f, 4.0f),
		Vector4(5.0f, 6.0f, 7.0f, 8.0f),
		Vector4(9.0f, 10.0f, 11.0f, 12.0f),
		Vector4(13.0f, 14.0f, 15.0f, 16.0f)
	};
	const Vector4 T_ALIGN16 inputVaryings[] =
	{
		Vector4(1.0f, 2.0f, 3.0f, 4.0f),
		Vector4(4.0f, 4.0f, 4.0f, 4.0f)
	};

	const Vector4 targetSize(11.0f, 22.0f, 33.0f, 44.0f);

	Vector4 T_ALIGN16 outputVarying1[8];
	Vector4 T_ALIGN16 outputVarying2[8];
	Vector4 T_ALIGN16 outputVarying3[8];

	// Fetch constant + store varying
	log::info << L"Fetch constant" << Endl;
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 0, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(1.0f, 2.0f, 3.0f, 4.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Fetch target size + store varying
	log::info << L"Fetch target size" << Endl;
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addInstruction(render::Instruction(render::OpFetchTargetSize, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 0, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(11.0f, 22.0f, 33.0f, 44.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Fetch varying
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addInstruction(render::Instruction(render::OpFetchVarying, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 0, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(1.0f, 2.0f, 3.0f, 4.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Fetch indexed uniform
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchIndexedUniform, 1, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchIndexedUniform, 2, 4, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 1, 2, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(0.0f, -1.0f, 0.0f, 0.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[1], Vector4(5.0f, 6.0f, 7.0f, 8.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[1], outputVarying2[1], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[1], outputVarying3[1], equalFuzzy);
	}

	// Fetch indexed uniform (2)
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchIndexedUniform, 1, 0, 4, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 1, 2, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 2, 3, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 3, 4, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(1.0f, 2.0f, 3.0f, 4.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[1], Vector4(5.0f, 6.0f, 7.0f, 8.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[2], Vector4(9.0f, 10.0f, 11.0f, 12.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[3], Vector4(13.0f, 14.0f, 15.0f, 16.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[1], outputVarying2[1], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[2], outputVarying2[2], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[3], outputVarying2[3], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[1], outputVarying3[1], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[2], outputVarying3[2], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[3], outputVarying3[3], equalFuzzy);
	}

	// Abs
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, -1.0f, 2.0f, -2.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpAbs, 1, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(1.0f, 1.0f, 2.0f, 2.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Increment
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, -1.0f, 2.0f, -2.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpIncrement, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 0, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(2.0f, 0.0f, 3.0f, -1.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Decrement
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, -1.0f, 2.0f, -2.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpDecrement, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 0, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(0.0f, -2.0f, 1.0f, -3.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Add
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, -1.0f, 2.0f, -2.0f));
		program.addConstant(Vector4(1.0f, 2.0f, -1.0f, -2.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpAdd, 2, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 2, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(2.0f, 1.0f, 1.0f, -4.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Div
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, -1.0f, 2.0f, -2.0f));
		program.addConstant(Vector4(1.0f, 2.0f, -1.0f, -2.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpDiv, 2, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 2, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(1.0f, -0.5f, -2.0f, 1.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Mul
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, -1.0f, 2.0f, -2.0f));
		program.addConstant(Vector4(1.0f, 2.0f, -1.0f, -2.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpMul, 2, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 2, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(1.0f, -2.0f, -2.0f, 4.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// MulAdd
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, -1.0f, 2.0f, -2.0f));
		program.addConstant(Vector4(1.0f, 2.0f, -1.0f, -2.0f));
		program.addConstant(Vector4(2.0f, 3.0f, 4.0f, 5.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 2, 2, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpMulAdd, 3, 0, 1, 2, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 3, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(3.0f, 1.0f, 2.0f, 9.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Log

	// Log2

	// Log10

	// Exp

	// Fraction
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.1f, -1.1f, 2.2f, -2.2f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFraction, 1, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(0.1f, -0.1f, 0.2f, -0.2f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Neg
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, -1.0f, 2.0f, -2.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpNeg, 1, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(-1.0f, 1.0f, -2.0f, 2.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Pow
	log::info << L"Pow" << Endl;
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(2.0f, 2.0f, 2.0f, 2.0f));
		program.addConstant(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpPow, 2, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 2, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(2.0f, 4.0f, 8.0f, 16.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Sqrt
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 4.0f, 9.0f, 16.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpSqrt, 1, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(1.0f, 2.0f, 3.0f, 4.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Sub
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, -1.0f, 2.0f, -2.0f));
		program.addConstant(Vector4(1.0f, 2.0f, -1.0f, -2.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpSub, 2, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 2, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(0.0f, -3.0f, 3.0f, 0.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Acos
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(0.0f, 1.0f, -1.0f, 0.5f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpAcos, 1, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		const Vector4 result(acosf(0.0f), acosf(1.0f), acosf(-1.0f), acosf(0.5f));
		CASE_ASSERT_COMPARE(outputVarying1[0], result, equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);

		log::info << L"outputVarying1[0] " << outputVarying1[0] << Endl;
		log::info << L"outputVarying2[0] " << outputVarying2[0] << Endl;
	}

	// Atan

	// Cos
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(0.0f, PI / 2.0f, -PI / 2.0f, PI));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpCos, 1, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		const Vector4 result(cosf(0.0f), cosf(PI / 2.0f), cosf(-PI / 2.0f), cosf(PI));
		CASE_ASSERT_COMPARE(outputVarying1[0], result, equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);

		log::info << L"outputVarying1[0] " << outputVarying1[0] << Endl;
		log::info << L"outputVarying2[0] " << outputVarying2[0] << Endl;
	}

	// Sin
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(0.0f, PI / 2.0f, -PI / 2.0f, PI));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpSin, 1, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		const Vector4 result(sinf(0.0f), sinf(PI / 2.0f), sinf(-PI / 2.0f), sinf(PI));
		CASE_ASSERT_COMPARE(outputVarying1[0], result, equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);

		log::info << L"outputVarying1[0] " << outputVarying1[0] << Endl;
		log::info << L"outputVarying2[0] " << outputVarying2[0] << Endl;
	}

	// Tan

	// Cross
	RandomGeometry rg;
	for (int32_t i = 0; i < 10; ++i)
	{
		log::info << L"Cross " << i << Endl;

		memset(outputVarying1, 0, sizeof(outputVarying1));
		memset(outputVarying2, 0, sizeof(outputVarying2));
		memset(outputVarying3, 0, sizeof(outputVarying3));
		{
			Vector4 axis1 = rg.nextUnit();
			Vector4 axis2 = rg.nextUnit();

			//axis1.w = rg.nextFloat() * 100.0f - 50.0f;
			//axis2.w = rg.nextFloat() * 100.0f - 50.0f;

			render::IntrProgram program;
			program.addConstant(axis1);
			program.addConstant(axis2);
			program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
			program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
			program.addInstruction(render::Instruction(render::OpCross, 2, 0, 1, 0, 0));
			program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 2, 0, 0, 0));

			render::Processor::image_t image1 = interpreter.compile(program);
			render::Processor::image_t image2 = interpreterFixed.compile(program);
			render::Processor::image_t image3 = jit.compile(program);

			interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
			interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
			jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

			Vector4 result = cross(axis1, axis2);
			CASE_ASSERT_COMPARE(outputVarying1[0], result, equalFuzzy);
			CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
			CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
		}
	}

	// Dot3
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{	
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
		program.addConstant(Vector4(5.0f, 6.0f, 7.0f, 8.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpDot3, 2, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 2, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(38.0f, 38.0f, 38.0f, 38.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Dot4
	log::info << L"Dot4" << Endl;
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
		program.addConstant(Vector4(5.0f, 6.0f, 7.0f, 8.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpDot4, 2, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 2, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(70.0f, 70.0f, 70.0f, 70.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Length
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpLength, 1, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		const float length = Vector4(1.0f, 2.0f, 3.0f, 4.0f).length();
		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(length, length, length, length), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Normalize
	log::info << L"Normalize" << Endl;
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpNormalize, 1, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		const Vector4 result = Vector4(1.0f, 2.0f, 3.0f, 4.0f).normalized();
		CASE_ASSERT_COMPARE(outputVarying1[0], result, equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);

		log::info << outputVarying1[0] << Endl;
		log::info << outputVarying2[0] << Endl;
	}

	// Transform
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchUniform, 1, 0, 4, 0, 0));
		program.addInstruction(render::Instruction(render::OpTransform, 5, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 5, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(-1.0f, -2.0f, -3.0f, -4.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Transpose
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addInstruction(render::Instruction(render::OpFetchUniform, 0, 4, 4, 0, 0));
		program.addInstruction(render::Instruction(render::OpTranspose, 4, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 4, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 1, 5, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 2, 6, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 3, 7, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(1.0f, 5.0f, 9.0f, 13.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[1], Vector4(2.0f, 6.0f, 10.0f, 14.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[2], Vector4(3.0f, 7.0f, 11.0f, 15.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[3], Vector4(4.0f, 8.0f, 12.0f, 16.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[1], outputVarying2[1], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[2], outputVarying2[2], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[3], outputVarying2[3], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[1], outputVarying3[1], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[2], outputVarying3[2], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[3], outputVarying3[3], equalFuzzy);

		log::info << L"outputVarying1[2] : " << outputVarying1[2] << Endl;
		log::info << L"outputVarying2[2] : " << outputVarying2[2] << Endl;
	}

	// Clamp
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(0.5f, 1.5f, -0.5f, -1.5f));
		program.addConstant(Vector4(0.1f, 0.1f, 0.1f, 0.1f));
		program.addConstant(Vector4(0.9f, 0.9f, 0.9f, 0.9f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpClamp, 1, 0, 1, 2, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(0.5f, 0.9f, 0.1f, 0.1f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);

		log::info << L"outputVarying1[0] : " << outputVarying1[0] << Endl;
		log::info << L"outputVarying2[0] : " << outputVarying2[0] << Endl;
	}

	// Lerp
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
		program.addConstant(Vector4(2.0f, 3.0f, 4.0f, 5.0f));
		program.addConstant(Vector4(0.25f, 0.0f, 0.0f, 0.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 2, 2, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpLerp, 3, 2, 0, 1, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 3, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		const Vector4 result = lerp(Vector4(1.0f, 2.0f, 3.0f, 4.0f), Vector4(2.0f, 3.0f, 4.0f, 5.0f), Scalar(0.25f));
		CASE_ASSERT_COMPARE(outputVarying1[0], result, equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// MixIn
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 5.0f, 5.0f, 5.0f));
		program.addConstant(Vector4(2.0f, 5.0f, 5.0f, 5.0f));
		program.addConstant(Vector4(3.0f, 5.0f, 5.0f, 5.0f));
		program.addConstant(Vector4(4.0f, 5.0f, 5.0f, 5.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 2, 2, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 3, 3, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpMixIn, 4, 0, 1, 2, 3));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 4, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(1.0f, 2.0f, 3.0f, 4.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Max
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 2.0f, -1.0f, -2.0f));
		program.addConstant(Vector4(2.0f, 1.0f, -2.0f, -1.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpMax, 2, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 2, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(2.0f, 2.0f, -1.0f, -1.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Sampler

	// Swizzle
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		#define SWIZZLE_MASK(x, y, z, w) ( ((x) << 6) | ((y) << 4) | ((z) << 2) | (w) )

		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpSwizzle, 1, SWIZZLE_MASK(2, 0, 3, 1), 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(3.0f, 1.0f, 4.0f, 2.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Set
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpSet, 0, 1 | 4, 2, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 0, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(1.0f, 0.0f, 1.0f, 4.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// ExpandWithZero
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpExpandWithZero, 1, 0, 4 | 8, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(1.0f, 2.0f, 0.0f, 0.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Compare
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 2.0f, 3.0f, 4.0f));
		program.addConstant(Vector4(1.0f, 3.0f, 2.0f, 1.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpCompareGreaterEqual, 2, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 2, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpCompareGreater, 2, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 1, 2, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpCompareEqual, 2, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 2, 2, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpCompareNotEqual, 2, 0, 1, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 3, 2, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		//CASE_ASSERT_NOT_EQUAL(*(uint32_t*)&outputVarying1[0].x, 0);
		//CASE_ASSERT_COMPARE(*(uint32_t*)&outputVarying1[1].x, 0, equalFuzzy);
		//CASE_ASSERT_NOT_EQUAL(*(uint32_t*)&outputVarying1[2].x, 0);
		//CASE_ASSERT_COMPARE(*(uint32_t*)&outputVarying1[3].x, 0, equalFuzzy);

		//CASE_ASSERT_NOT_EQUAL(*(uint32_t*)&outputVarying2[0].x, 0);
		//CASE_ASSERT_COMPARE(*(uint32_t*)&outputVarying2[1].x, 0, equalFuzzy);
		//CASE_ASSERT_NOT_EQUAL(*(uint32_t*)&outputVarying2[2].x, 0);
		//CASE_ASSERT_COMPARE(*(uint32_t*)&outputVarying2[3].x, 0, equalFuzzy);

		//CASE_ASSERT_NOT_EQUAL(*(uint32_t*)&outputVarying3[0].x, 0);
		//CASE_ASSERT_COMPARE(*(uint32_t*)&outputVarying3[1].x, 0, equalFuzzy);
		//CASE_ASSERT_NOT_EQUAL(*(uint32_t*)&outputVarying3[2].x, 0);
		//CASE_ASSERT_COMPARE(*(uint32_t*)&outputVarying3[3].x, 0, equalFuzzy);
	}

	// JumpIfZero
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		program.addConstant(Vector4(4.0f, 4.0f, 4.0f, 4.0f));
		program.addConstant(Vector4(2.0f, 2.0f, 2.0f, 2.0f));
		program.addConstant(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpJumpIfZero, 0, 2));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 2, 0, 0, 0));	// Skipped
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpJumpIfZero, 1, 2));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 1, 3, 0, 0, 0));	// Not skipped
		program.addInstruction(render::Instruction(render::OpStoreVarying, 1, 1, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(4.0f, 4.0f, 4.0f, 4.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[1], Vector4(1.0f, 1.0f, 1.0f, 1.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[1], outputVarying2[1], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[1], outputVarying3[1], equalFuzzy);
	}

	// Jump (2)
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(10.0f, 10.0f, 10.0f, 10.0f));
		
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpJumpIfZero, 0, 3));
		program.addInstruction(render::Instruction(render::OpDecrement, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpJump, 0, -2));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 0, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(0.0f, 0.0f, 0.0f, 0.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Jump (3)
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpDecrement, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpJumpIfZero, 0, -1));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 0, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(-1.0f, -1.0f, -1.0f, -1.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}

	// Jump
	memset(outputVarying1, 0, sizeof(outputVarying1));
	memset(outputVarying2, 0, sizeof(outputVarying2));
	memset(outputVarying3, 0, sizeof(outputVarying3));
	{
		render::IntrProgram program;
		program.addConstant(Vector4(4.0f, 4.0f, 4.0f, 4.0f));
		program.addConstant(Vector4(2.0f, 2.0f, 2.0f, 2.0f));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 0, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpJump, 0, 2));
		program.addInstruction(render::Instruction(render::OpFetchConstant, 0, 1, 0, 0, 0));
		program.addInstruction(render::Instruction(render::OpStoreVarying, 0, 0, 0, 0, 0));

		render::Processor::image_t image1 = interpreter.compile(program);
		render::Processor::image_t image2 = interpreterFixed.compile(program);
		render::Processor::image_t image3 = jit.compile(program);

		interpreter.execute(image1, inputUniforms, inputVaryings, targetSize, 0, outputVarying1);
		interpreterFixed.execute(image2, inputUniforms, inputVaryings, targetSize, 0, outputVarying2);
		jit.execute(image3, inputUniforms, inputVaryings, targetSize, 0, outputVarying3);

		CASE_ASSERT_COMPARE(outputVarying1[0], Vector4(4.0f, 4.0f, 4.0f, 4.0f), equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying2[0], equalFuzzy);
		CASE_ASSERT_COMPARE(outputVarying1[0], outputVarying3[0], equalFuzzy);
	}
#endif
*/
}

}
