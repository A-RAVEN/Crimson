#pragma once
#include <string>

class ShaderBindingBuilder
{
public:
	virtual void Int(std::string const& name);
	virtual void Int2(std::string const& name);
	virtual void Int3(std::string const& name);
	virtual void Int4(std::string const& name);
	virtual void IntArray(std::string const& name, int size);
	virtual void Int2Array(std::string const& name, int size);
	virtual void Int3Array(std::string const& name, int size);
	virtual void Int4Array(std::string const& name, int size);

	virtual void UInt(std::string const& name);
	virtual void UInt2(std::string const& name);
	virtual void UInt3(std::string const& name);
	virtual void UInt4(std::string const& name);
	virtual void UIntArray(std::string const& name, int size);
	virtual void UInt2Array(std::string const& name, int size);
	virtual void UInt3Array(std::string const& name, int size);
	virtual void UInt4Array(std::string const& name, int size);

	virtual void Float(std::string const& name);
	virtual void Float2(std::string const& name);
	virtual void Float3(std::string const& name);
	virtual void Float4(std::string const& name);
	virtual void FloatArray(std::string const& name, int size);
	virtual void Float2Array(std::string const& name, int size);
	virtual void Float3Array(std::string const& name, int size);
	virtual void Float4Array(std::string const& name, int size);

	virtual void Mat4(std::string const& name, int size);



};