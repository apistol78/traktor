#ifndef traktor_render_Nodes_H
#define traktor_render_Nodes_H

#include <string>
#include "Core/Guid.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Vector4.h"
#include "Render/Types.h"
#include "Render/Shader/ImmutableNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace render
	{

/*! \ingroup Render */
//@{

/*! \brief Absolute value. */
class T_DLLCLASS Abs : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Abs();
};

/*! \brief Add two values. */
class T_DLLCLASS Add : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Add();
};

/*! \brief Arcus cosine. */
class T_DLLCLASS ArcusCos : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	ArcusCos();
};

/*! \brief Absolute tangent. */
class T_DLLCLASS ArcusTan : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	ArcusTan();
};

/*! \brief Static branch permutation. */
class T_DLLCLASS Branch : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Branch(const std::wstring& parameterName = L"");

	void setParameterName(const std::wstring& parameterName);

	const std::wstring& getParameterName() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_parameterName;
};

/*! \brief Clamp value to given range. */
class T_DLLCLASS Clamp : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Clamp(float min = 0.0f, float max = 1.0f);

	void setMin(float min);

	float getMin() const;

	void setMax(float max);

	float getMax() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	float m_min;
	float m_max;
};

/*! \brief Color constant. */
class T_DLLCLASS Color : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Color(const traktor::Color4ub& color = traktor::Color4ub(0, 0, 0, 0));

	void setColor(const traktor::Color4ub& color);

	const traktor::Color4ub& getColor() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	traktor::Color4ub m_color;
};

/*! \brief Comment node. */
class T_DLLCLASS Comment : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Comment();
};

/*! \brief Conditional selection. */
class T_DLLCLASS Conditional : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	enum Branch
	{
		BrAuto,
		BrStatic,
		BrDynamic
	};

	enum Operator
	{
		CoLess,
		CoLessEqual,
		CoEqual,
		CoNotEqual,
		CoGreater,
		CoGreaterEqual
	};

	Conditional();

	void setBranch(Branch branch);

	Branch getBranch() const;

	void setOperator(Operator op);

	Operator getOperator() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Branch m_branch;
	Operator m_operator;
};

/*! \brief Connected. */
class T_DLLCLASS Connected : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Connected();
};

/*! \brief Cosine. */
class T_DLLCLASS Cos : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Cos();
};

/*! \brief Cross product. */
class T_DLLCLASS Cross : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Cross();
};

/*! \brief Partial derivative. */
class T_DLLCLASS Derivative : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	enum Axis
	{
		DaX,
		DaY
	};

	Derivative();

	Axis getAxis() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Axis m_axis;
};

/*! \brief Discard fragment. */
class T_DLLCLASS Discard : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	enum Operator
	{
		CoLess,
		CoLessEqual,
		CoEqual,
		CoNotEqual,
		CoGreater,
		CoGreaterEqual
	};

	Discard();

	void setOperator(Operator op);

	Operator getOperator() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Operator m_operator;
};

/*! \brief Divide. */
class T_DLLCLASS Div : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Div();
};

/*! \brief Dot product. */
class T_DLLCLASS Dot : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Dot();
};

/*! \brief Exponential. */
class T_DLLCLASS Exp : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Exp();
};

/*! \brief Fractional part. */
class T_DLLCLASS Fraction : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Fraction();
};

/*! \brief Fragment position. */
class T_DLLCLASS FragmentPosition : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	FragmentPosition();
};

/*! \brief Emit true if primitive is seen from the front. */
class T_DLLCLASS FrontFace : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	FrontFace();
};

/*! \brief Fetch value from array uniform. */
class T_DLLCLASS IndexedUniform : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	IndexedUniform(
		const std::wstring& parameterName = L"",
		ParameterType type = PtScalar,
		UpdateFrequency frequency = UfFrame,
		int32_t length = 0
	);

	void setParameterName(const std::wstring& parameterName);

	const std::wstring& getParameterName() const;

	void setParameterType(ParameterType type);

	ParameterType getParameterType() const;

	void setFrequency(UpdateFrequency frequency);
	
	UpdateFrequency getFrequency() const;

	void setLength(int32_t length);

	int32_t getLength() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_parameterName;
	ParameterType m_type;
	UpdateFrequency m_frequency;
	int32_t m_length;
};

/*! \brief Fragment named input. */
class T_DLLCLASS InputPort : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	InputPort(const std::wstring& name = L"", bool connectable = true, bool optional = false, float defaultValue = 0.0f);

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setConnectable(bool connectable);

	bool isConnectable() const;

	void setOptional(bool optional);

	bool isOptional() const;

	void setDefaultValue(float defaultValue);

	float getDefaultValue() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_name;
	bool m_connectable;
	bool m_optional;
	float m_defaultValue;
};

/*! \brief Instance index input. */
class T_DLLCLASS Instance : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Instance();
};

/*! \brief Interpolator, pass value from vertex to pixel shader. */
class T_DLLCLASS Interpolator : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Interpolator();
};

/*! \brief Iterate. */
class T_DLLCLASS Iterate : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Iterate(int32_t from = 0, int32_t to = 0);

	void setFrom(int32_t from);

	int32_t getFrom() const;

	void setTo(int32_t to);

	int32_t getTo() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	int32_t m_from;
	int32_t m_to;
};

/*! \brief 2-D iterate. */
class T_DLLCLASS Iterate2d : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Iterate2d(int32_t fromX = 0, int32_t toX = 0, int32_t fromY = 0, int32_t toY = 0);

	void setFromX(int32_t fromX);

	int32_t getFromX() const;

	void setToX(int32_t toX);

	int32_t getToX() const;

	void setFromY(int32_t fromY);

	int32_t getFromY() const;

	void setToY(int32_t toY);

	int32_t getToY() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	int32_t m_fromX;
	int32_t m_toX;
	int32_t m_fromY;
	int32_t m_toY;
};

/*! \brief Length of geometrical vector. */
class T_DLLCLASS Length : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Length();
};

/*! \brief Linear interpolate. */
class T_DLLCLASS Lerp : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Lerp();
};

/*! \brief Natural logarithm. */
class T_DLLCLASS Log : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	enum Base
	{
		LbTwo,
		LbTen,
		LbNatural
	};

	Log(Base base = LbTen);

	Base getBase() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Base m_base;
};

/*! \brief Build matrix. */
class T_DLLCLASS MatrixIn : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	MatrixIn();
};

/*! \brief Decompose matrix. */
class T_DLLCLASS MatrixOut : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	MatrixOut();
};

/*! \brief Maximum of two values. */
class T_DLLCLASS Max : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Max();
};

/*! \brief Minimum of two values. */
class T_DLLCLASS Min : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Min();
};

/*! \brief Mix in scalars into vector. */
class T_DLLCLASS MixIn : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	MixIn();
};

/*! \brief Extract scalars from vector. */
class T_DLLCLASS MixOut : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	MixOut();
};

/*! \brief Multiply. */
class T_DLLCLASS Mul : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Mul();
};

/*! \brief Multiply then add. */
class T_DLLCLASS MulAdd : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	MulAdd();
};

/*! \brief Negate value. */
class T_DLLCLASS Neg : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Neg();
};

/*! \brief Normalize vector. */
class T_DLLCLASS Normalize : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Normalize();
};

/*! \brief Shader fragment named output. */
class T_DLLCLASS OutputPort : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	OutputPort(const std::wstring& name = L"");

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_name;
};

/*! \brief Evaluate polynomial. */
class T_DLLCLASS Polynomial : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Polynomial();
};

/*! \brief Raise to power. */
class T_DLLCLASS Pow : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Pow();
};

/*! \brief Pixel output. */
class T_DLLCLASS PixelOutput : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	PixelOutput();

	void setTechnique(const std::wstring& technique);

	const std::wstring& getTechnique() const;

	void setPriority(uint32_t priority);

	uint32_t getPriority() const;

	void setRenderState(const RenderState& state);

	const RenderState& getRenderState() const;

	void setRegisterCount(uint32_t registerCount);

	uint32_t getRegisterCount() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;
	
	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_technique;
	uint32_t m_priority;
	RenderState m_renderState;
	uint32_t m_registerCount;
};

/*! \brief Conditional based on render platform. */
class T_DLLCLASS Platform : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Platform();
};

/*! \brief Reflect vector. */
class T_DLLCLASS Reflect : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Reflect();
};

/*! \brief Reciprocal square root. */
class T_DLLCLASS RecipSqrt : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	RecipSqrt();
};

/*! \brief Repeat until condition. */
class T_DLLCLASS Repeat : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Repeat();
};

/*! \brief Round. */
class T_DLLCLASS Round : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Round();
};

/*! \brief Texture sampler. */
class T_DLLCLASS Sampler : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Sampler();

	Sampler(const SamplerState& state);

	void setSamplerState(const SamplerState& state);

	const SamplerState& getSamplerState() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	SamplerState m_state;
};

/*! \brief Scalar constant. */
class T_DLLCLASS Scalar : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Scalar(float value = 0.0f);

	void set(float value);

	float get() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	float m_value;
};

/*! \brief Sign. */
class T_DLLCLASS Sign : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Sign();
};

/*! \brief Sine. */
class T_DLLCLASS Sin : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Sin();
};

/*! \brief Square root of value. */
class T_DLLCLASS Sqrt : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Sqrt();
};

/*! \brief Output states. */
class T_DLLCLASS State : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	State();

	void setPriority(uint32_t priority);

	uint32_t getPriority() const;

	void setRenderState(const RenderState& renderState);

	const RenderState& getRenderState() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	uint32_t m_priority;
	RenderState m_renderState;
};

/*! \brief Step function. */
class T_DLLCLASS Step : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Step();
};

/*! \brief Subtract. */
class T_DLLCLASS Sub : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Sub();
};

/*! \brief Summarize function. */
class T_DLLCLASS Sum : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Sum(int32_t from = 0, int32_t to = 0);

	void setFrom(int32_t from);

	int32_t getFrom() const;

	void setTo(int32_t to);

	int32_t getTo() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	int32_t m_from;
	int32_t m_to;
};

/*! \brief Conditional switch. */
class T_DLLCLASS Switch : public Node
{
	T_RTTI_CLASS;

public:
	enum Branch
	{
		BrAuto,
		BrStatic,
		BrDynamic
	};

	Switch();

	void setBranch(Branch branch);

	Branch getBranch() const;

	void addCase(int32_t value);

	const std::vector< int32_t >& getCases() const;

	virtual int getInputPinCount() const T_OVERRIDE T_FINAL;

	virtual const InputPin* getInputPin(int index) const T_OVERRIDE T_FINAL;

	virtual int getOutputPinCount() const T_OVERRIDE T_FINAL;

	virtual const OutputPin* getOutputPin(int index) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Branch m_branch;
	std::vector< int32_t > m_cases;
	std::vector< InputPin* > m_inputPins;
	OutputPin* m_outputPin;
};

/*! \brief Swizzle elements in a vector. */
class T_DLLCLASS Swizzle : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Swizzle(const std::wstring& swizzle = L"xyzw");

	void set(const std::wstring& swizzle);
	
	const std::wstring& get() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_swizzle;
};

/*! \brief Tangent. */
class T_DLLCLASS Tan : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Tan();
};

/*! \brief Target size. */
class T_DLLCLASS TargetSize : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	TargetSize();
};

/*! \brief Texture constant. */
class T_DLLCLASS Texture : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Texture(const Guid& external = Guid(), ParameterType type = PtTexture2D);

	void setExternal(const Guid& external);

	const Guid& getExternal() const;

	void setParameterType(ParameterType type);

	ParameterType getParameterType() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Guid m_external;
	ParameterType m_type;
};


/*! \brief Texture size. */
class T_DLLCLASS TextureSize : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	TextureSize();
};

/*! \brief Transform vector by matrix. */
class T_DLLCLASS Transform : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Transform();
};

/*! \brief Transpose matrix. */
class T_DLLCLASS Transpose : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Transpose();
};

/*! \brief Truncate. */
class T_DLLCLASS Truncate : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Truncate();
};

/*! \brief Type switch. */
class T_DLLCLASS Type : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Type();
};

/*! \brief Fetch parameter value. */
class T_DLLCLASS Uniform : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Uniform(
		const std::wstring& parameterName = L"",
		ParameterType type = PtScalar,
		UpdateFrequency frequency = UfFrame
	);

	void setParameterName(const std::wstring& parameterName);

	const std::wstring& getParameterName() const;

	void setParameterType(ParameterType type);

	ParameterType getParameterType() const;
	
	void setFrequency(UpdateFrequency frequency);
	
	UpdateFrequency getFrequency() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_parameterName;
	ParameterType m_type;
	UpdateFrequency m_frequency;
};

/*! \brief Variable */
class T_DLLCLASS Variable : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Variable();

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_name;
};

/*! \brief Vector constant. */
class T_DLLCLASS Vector : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Vector(const Vector4& value = Vector4(0, 0, 0, 0));

	void set(const Vector4& value);

	const Vector4& get() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Vector4 m_value;
};

/*! \brief Fetch value from vertex. */
class T_DLLCLASS VertexInput : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	VertexInput(const std::wstring& name = L"", DataUsage usage = DuPosition, DataType type = DtFloat4, int index = 0);

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setDataUsage(DataUsage usage);

	DataUsage getDataUsage() const;

	void setDataType(DataType type);

	DataType getDataType() const;

	void setIndex(int32_t index);

	int32_t getIndex() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_name;
	DataUsage m_usage;
	DataType m_type;
	int32_t m_index;
};

/*! \brief Output vertex's clip position. */
class T_DLLCLASS VertexOutput : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	VertexOutput();

	void setTechnique(const std::wstring& technique);

	const std::wstring& getTechnique() const;

	virtual std::wstring getInformation() const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_technique;
};

//@}

	}
}

#endif	// traktor_render_Nodes_H
