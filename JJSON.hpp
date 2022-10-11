#pragma once
#ifndef JJSON_INCLUDE_JJSON_HPP
#define JJSON_INCLUDE_JJSON_HPP

#ifndef __cplusplus
#error This header is for C++, for pure C use "JJSON.h"
#endif

#include <exception>
#include "JJSONcommon.h"

struct JSON_Exception : std::exception
{
public:
	std::string message;
	const char *what() const noexcept { return message.c_str(); }
};

struct JSON_TypeException : JSON_Exception {};
struct JSON_OutOfRangeException : JSON_Exception {};
struct JSON_ParseException : JSON_Exception {};

struct JSONValue
{
protected:
	friend class JSON;

	JJSON_Value v;

	JSONValue() noexcept;
	JSONValue( JJSON_Value v_ ) noexcept;
	void AssertType( JJSON_types t ) const;
	JJSON_Value GetObjectInternal( const char *key ) const;

public:
	JJSON_types type() const noexcept;

	size_t size() const; // array
	JSONValue operator[]( size_t idx ) const; // array

	bool contains( const std::string &key ) const; // object
	JSONValue operator[]( const std::string &key ) const; // object

	bool GetBool() const;
	double GetNumber() const;
	const char *GetString() const;
};

class JSON : public JSONValue
{
public:
	JSON() noexcept;
	JSON( const JSON & ) = delete;
	JSON( JSON && ) noexcept;

	JSON &operator=( const JSON & ) = delete;
	JSON &operator=( JSON && ) noexcept;

	JSON( const char *path );
	JSON( JJSON_GetcFn InStream, void *user );

	~JSON() noexcept;
};

#endif // ndef JJSON_INCLUDE_JJSON_HPP

