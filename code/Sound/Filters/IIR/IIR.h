/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include <cmath>

#ifndef M_PI
#	define M_PI 3.14159265358979323846
#endif

#define FILTER_TYPE_LOWPASS 0
#define FILTER_TYPE_HIGHPASS 1

namespace traktor::sound
{

class PolarBiquadSection;
class PolarBiquadsCascade;

class BiquadSection
{
public:
	double b0;
	double b1;
	double b2;
	double a1;
	double a2;

	BiquadSection();
	BiquadSection(const BiquadSection& init);
	BiquadSection(const PolarBiquadSection& init);

	void clear();

	void resetTaps();

	BiquadSection& operator=(const BiquadSection& init);
	BiquadSection& operator=(const PolarBiquadSection& init);

	void computeOutput(double* buffer, const int numSamples);

private:
	double w0;
	double w11;
	double w12;

	void copy(const BiquadSection& init);

	void copy(const PolarBiquadSection& init);
};

class BiquadsCascade
{
public:
	BiquadsCascade();
	BiquadsCascade(const BiquadsCascade& init);
	BiquadsCascade(const PolarBiquadsCascade& init);
	~BiquadsCascade();

	bool setSize(int numberBiquadSections);

	int getSize();

	void release();

	// It modifies the samples in buffer, the operation is in place (that is the output is written in the same buffer)
	void computeOutput(double* buffer, const int numSamples);

	BiquadsCascade& operator=(const BiquadsCascade& init);
	BiquadsCascade& operator=(const PolarBiquadsCascade& init);
	BiquadSection& operator[](long index);
	const BiquadSection& operator[](long index) const;

private:
	friend class PolarBiquadsCascade;

	BiquadSection* biquadSection;
	int size;

	bool copy(const BiquadsCascade& init);
	bool copy(const PolarBiquadsCascade& init);
};

class PolarBiquadSection
{
public:
	PolarBiquadSection();
	PolarBiquadSection(const PolarBiquadSection& init);
	PolarBiquadSection(const BiquadSection& init);

	void clear();

	double zeroR; // alpha
	double zeroAngle; // alpha
	double poleR; // beta
	double poleAngle; // beta
	double k;

	PolarBiquadSection& operator=(const PolarBiquadSection& init);
	PolarBiquadSection& operator=(const BiquadSection& init);

	void getPoles(double& out_reP1, double& out_imP1, double& out_reP2, double& out_imP2);

	void getZeros(double& out_reZ1, double& out_imZ1, double& out_reZ2, double& out_imZ2);

private:
	void copy(const PolarBiquadSection& init);

	void copy(const BiquadSection& init);
};

class PolarBiquadsCascade
{
public:
	PolarBiquadsCascade();
	PolarBiquadsCascade(const PolarBiquadsCascade& init);
	PolarBiquadsCascade(const BiquadsCascade& init);

	~PolarBiquadsCascade();

	bool setSize(int numberBiquadSections);

	int getSize() const;

	void release();

	double getMagnitude(double freq_rads);

	double getGroupDelay(double freq_rads);

	PolarBiquadsCascade& operator=(const PolarBiquadsCascade& init);

	PolarBiquadsCascade& operator=(const BiquadsCascade& init);

	PolarBiquadSection& operator[](long index);

	const PolarBiquadSection& operator[](long index) const;

private:
	friend class BiquadsCascade;

	PolarBiquadSection* biquadSection;
	int size;

	bool copy(const PolarBiquadsCascade& init);
	
	bool copy(const BiquadsCascade& init);
};

class BiquadFilter
{
public:
	bool createLowPass(double cutFreq_rads, double passBand_dB, double stopFreq_rads, double stopBand_dB);

	bool createHighPass(double cutFreq_rads, double passBand_dB, double stopFreq_rads, double stopBand_dB);

	PolarBiquadsCascade biquadsCascade;

protected:
	//
	// input:  A*s*s + B*s + C
	// output:  a  + b * z^-1 + c * z^-2
	// filter_type: FILTER_TYPE_LOWPASS s = 2*(1 - z^-1)/(1 + z^-1), FILTER_TYPE_HIGHPASS s = 2*(1 + z^-1)/(1 - z^-1)
	static void bilinearTransformation(int filter_type, double A, double B, double C, double& out_a, double& out_b, double& out_c);

	virtual bool create(int filter_type, double cutFreq_rads, double passBand_dB, double stopFreq_rads, double stopBand_dB) = 0;
};

class ButterworthFilter: public BiquadFilter
{
private:
	virtual bool create(int filter_type, double cutFreq_rads, double passBand_dB, double stopFreq_rads, double stopBand_dB) override final;
};

class ChebyshevFilter: public BiquadFilter
{
private:
	virtual bool create(int filter_type, double cutFreq_rads, double passBand_dB, double stopFreq_rads, double stopBand_dB) override final;
};

class EllipticFilter: public BiquadFilter
{
private:
	virtual bool create(int filter_type, double cutFreq_rads, double passBand_dB, double stopFreq_rads, double stopBand_dB) override final;
};

}
