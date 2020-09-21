#pragma once

class IResource
{
public:
	IResource() {};
	virtual ~IResource() {};
	virtual bool Ready() { return true; }
	std::string const& GetName() { return m_Name; }
protected:
	std::string m_Name;
};