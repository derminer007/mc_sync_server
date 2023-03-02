#pragma once
#include <exception>

class networkIOException : public std::exception
{
private:
	const char* message;
public:
	networkIOException(const char* err_mess)
	{
		message = err_mess;
	}
	virtual const char* what() const override
	{
		return message;
	}
};

class fileIOException : public std::exception
{
private:
	const char* message;
public:
	fileIOException(const char* err_mess)
	{
		message = err_mess;
	}
	virtual const char* what() const override
	{
		return message;
	}
};