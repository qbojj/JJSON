// JJSON.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"
#include "JJSON.h"
#include "JJSON.hpp"
#include <charconv>
	

using namespace std;

/*
typedef unique_ptr<JJSON_Value_t_> ValuePtr;
typedef unordered_map<string, ValuePtr> ValueMap;
typedef vector<ValuePtr> ValueArray;

struct JJSON_Value_t_
{
	virtual ~JJSON_Value_t_() {};

	JJSON_types type = JJSON_null;
};

struct JJSON_Value_Bool : JJSON_Value_t_
{
	JJSON_Value_Bool( bool v = false ) { type = JJSON_bool; value = v; };
	bool value;
};
struct JJSON_Value_Number : JJSON_Value_t_
{
	JJSON_Value_Number( double v = 0.0 ) { type = JJSON_number; value = v; };
	double value;
};
struct JJSON_Value_String : JJSON_Value_t_
{
	JJSON_Value_String( const char *v = "" ) { type = JJSON_string; value = v; };
	JJSON_Value_String( string &&v ) { type = JJSON_string; value = move(v); };
	string value;
};
struct JJSON_Value_Object : JJSON_Value_t_
{
	JJSON_Value_Object( ValueMap &&v ) { type = JJSON_object; value = move(v); };
	ValueMap value;
};
struct JJSON_Value_Array : JJSON_Value_t_
{
	JJSON_Value_Array( ValueArray &&v ) { type = JJSON_array; value = move(v); };
	ValueArray value;
};
*/

struct JJSON_JSON_t_;

struct JJSON_Value_t_
{
	JJSON_types type;
	JJSON_JSON json;

	union
	{
		size_t ValueIdx;
		bool Bool;
		double Number;
	};

	JJSON_Value_t_() { type = JJSON_null; };
	JJSON_Value_t_( bool b ) { type = JJSON_bool; Bool = b; }
	JJSON_Value_t_( double v )  { type = JJSON_number; Number = v; }
	JJSON_Value_t_( JJSON_types t, size_t idx ) { type = t; ValueIdx = idx; }
};

typedef vector< JJSON_Value_t_ > ValueArray;
typedef unordered_map< string, JJSON_Value_t_ > ValueMap;
struct JJSON_JSON_t_
{
	vector<string> strings;
	vector<ValueArray> arrays;
	vector<ValueMap> objects;

	JJSON_Value_t_ mainValue;
};

extern "C" {
	JJSON_types JJSON_GetValueType( JJSON_Value Value )
	{
		return Value ? Value->type : JJSON_null;
	}

	const char *JJSON_GetString( JJSON_Value String )
	{
		if( !String || String->type != JJSON_string ) return NULL;
		return String->json->strings[String->ValueIdx].c_str();//((JJSON_Value_String *)String)->value.c_str();
	}
	double JJSON_GetNumber( JJSON_Value Number )
	{
		if( !Number || Number->type != JJSON_number ) return 0.0;
		return Number->Number;//((JJSON_Value_Number *)Number)->value;
	}
	bool JJSON_GetBool( JJSON_Value Bool )
	{
		if( !Bool || Bool->type != JJSON_bool ) return false;
		return Bool->Bool;//((JJSON_Value_Bool *)Bool)->value;
	}

	JJSON_Value JJSON_Object_Get( JJSON_Value Object, const char *key )
	{
		if( !Object || Object->type != JJSON_object ) return NULL;
		const ValueMap &v = Object->json->objects[Object->ValueIdx];//((JJSON_Value_Object *)Object)->value;

		ValueMap::const_iterator loc = v.find( key );
		return loc != v.end() ? &loc->second : NULL;//loc->second.get() : NULL;
	}

	size_t JJSON_Array_Size( JJSON_Value Array )
	{
		if( !Array || Array->type != JJSON_array ) return 0;
		return Array->json->arrays[Array->ValueIdx].size();//(int)((JJSON_Value_Array *)Array)->value.size();
	}

	JJSON_Value JJSON_Array_At( JJSON_Value Array, size_t index )
	{
		if( !Array || Array->type != JJSON_array ) return NULL;
		const ValueArray &arr = Array->json->arrays[Array->ValueIdx];//((JJSON_Value_Array *)Array)->value;

		return index >= 0 && index < arr.size() ? &arr[index] : NULL;
	}
} // extern "C"

#define TMP_BUFFER_SIZE 1024
struct JJSON_Stream
{
	JJSON_GetcFn stream;
	void *user;

	int last = 0; // for ungetc
	bool ungot = false;

	char TmpBuffer[TMP_BUFFER_SIZE];

	int getc()
	{
		if( last == EOF ) return EOF;

		if( ungot )
		{
			ungot = false;
			return last;
		}

		return last = stream( user );
	}

	void ungetc() { if( ungot ) throw exception(); ungot = true; }
};

static JJSON_Value_t_ GetValue( JJSON_Stream *stream, JJSON_JSON json );

static bool IsWhite( unsigned char c )
{
	return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}
static void SkipWhiteSpaces( JJSON_Stream *stream )
{
	while( IsWhite( (unsigned char)stream->getc() ) );
	stream->ungetc();
}

static string GetString( JJSON_Stream *stream );
static ValueMap GetObject( JJSON_Stream *stream, JJSON_JSON json );
static ValueArray GetArray( JJSON_Stream *stream, JJSON_JSON json );
static double GetNumber( JJSON_Stream *stream );

static JJSON_Value_t_ GetValue( JJSON_Stream *stream, JJSON_JSON json )
{
	SkipWhiteSpaces( stream );

	unsigned char c = stream->getc();

	JJSON_Value_t_ res;
	res.type = JJSON_type_cnt;

	switch( c )
	{
	case 'n': // null
		if( stream->getc() == 'u' &&
			stream->getc() == 'l' &&
			stream->getc() == 'l' ) res = JJSON_Value_t_();//make_unique<JJSON_Value_t_>();
		break;
	case 't': // true
		if( stream->getc() == 'r' &&
			stream->getc() == 'u' &&
			stream->getc() == 'e' ) res = JJSON_Value_t_( true );//make_unique<JJSON_Value_Bool>( JJSON_Value_Bool( true ) );
		break;
	case 'f': // false
		if( stream->getc() == 'a' &&
			stream->getc() == 'l' &&
			stream->getc() == 's' &&
			stream->getc() == 'e' ) res = JJSON_Value_t_( false );//make_unique<JJSON_Value_Bool>( JJSON_Value_Bool( false ) );
		break;
	case '\"': // string
		stream->ungetc();
		{
			string s = GetString( stream );

			res = JJSON_Value_t_( JJSON_string, json->strings.size() );//make_unique<JJSON_Value_String>( GetString( stream ) );
			json->strings.push_back( move( s ) );
		}
		break;

	case '{': // object
		stream->ungetc();
		{
			ValueMap v = GetObject( stream, json );

			res = JJSON_Value_t_( JJSON_object, json->objects.size() );
			json->objects.push_back( move( v ) );
		}
		break;
	case '[': // array
		stream->ungetc();
		{
			ValueArray a = GetArray( stream, json );

			res = JJSON_Value_t_( JJSON_array, json->arrays.size() );
			json->arrays.push_back( move( a ) );
		}

		break;

	default:
		if( c == '-' || (c >= '0' && c <= '9') ) // number
		{
			stream->ungetc();
			res = JJSON_Value_t_( GetNumber( stream ) );
		}

		break;
	}

	if( res.type == JJSON_type_cnt ) throw exception(); // couldn't create value

	res.json = json;
	SkipWhiteSpaces( stream );
	return res;
}

static string GetString( JJSON_Stream *stream )
{
	if( stream->getc() != '\"' ) throw exception();

	string res;
	while( 1 )
	{
		int c = stream->getc();
		if( c == EOF ) throw exception();
		if( c == '\"' ) return res;

		if( c != '\\' )
		{
			res.push_back( (unsigned char)c );
			continue;
		}

		unsigned char v;

		c = stream->getc();
		if( c == EOF ) throw exception();
		switch( c )
		{
		case '\"': v = '\"'; break;
		case '\\': v = '\\'; break;
		case '/': v = '/'; break;
		case 'b': v = '\b'; break;
		case 'f': v = '\f'; break;
		case 'n': v = '\n'; break;
		case 'r': v = '\r'; break;
		case 't': v = '\t'; break;
		case 'u':
		{
			unsigned short codepoint = 0;
			for( int i = 0; i < 4; i++ )
			{
				int q = stream->getc();
				if( q == EOF || !( 
					( q >= '0' && q <= '9' ) || 
					( q >= 'A' && q <= 'F' ) || 
					( q >= 'a' && q <= 'f' )
					) ) throw exception();
				codepoint = (codepoint << 4) | q;
			}

			// unicode to utf-8
			if( codepoint <= 0x7f )
				res.append( 1, static_cast<char>(codepoint) );
			else if( codepoint <= 0x7ff )
			{
				res.append( 1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)) );
				res.append( 1, static_cast<char>(0x80 | (codepoint & 0x3f)) );
			}
			else //if( codepoint <= 0xffff ) // always true for short
			{
				res.append( 1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)) );
				res.append( 1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)) );
				res.append( 1, static_cast<char>(0x80 | (codepoint & 0x3f)) );
			}
			//else
			//{
			//	res.append( 1, static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)) );
			//	res.append( 1, static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)) );
			//	res.append( 1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)) );
			//	res.append( 1, static_cast<char>(0x80 | (codepoint & 0x3f)) );
			//}

			break;
		}

		default: throw exception();
		}

		res.push_back( v );
	}
}

static bool IsInNumber( int c )
{
	return c == '-' || (c >= '0' && c <= '9') || c == '.' || c == 'e' || c == 'E';
}

static double GetDoubleFromString( string_view v )
{
	double res;
	
	char *end; 
	res = strtod( v.data(), &end );
	//auto [end, ec] = from_chars( (const char *)v.data(), (const char *)v.data() + v.size(), res );
	if( end == v.data() /* error parsing string */ || end != v.data() + v.size() /* not everything was part of a number */ )
		throw exception();	

	return res;
}

static double GetNumber( JJSON_Stream *stream )
{
	int idx = 0;

	int c;
	while( (c = stream->getc()) != EOF && IsInNumber( c ) && idx < TMP_BUFFER_SIZE )
		stream->TmpBuffer[idx++] = (unsigned char)c;

	if( IsInNumber( c ) )
	{
		// there is more to this number => fallback to dynamic allocation

		vector<unsigned char> s( stream->TmpBuffer, stream->TmpBuffer + idx );
		do {
			s.push_back( (unsigned char)c );
		} while( (c = stream->getc()) != EOF && IsInNumber( c ) );

		stream->ungetc();
		string_view q;
		return GetDoubleFromString( { (char*)s.data(), s.size() } );
	}

	stream->ungetc();
	return GetDoubleFromString( { stream->TmpBuffer, (size_t)idx } );

	/*
	// TODO: implement this using Grisu2 for round trip

	bool invert = false;
	long double v = 0.L;

	int c = stream->getc();
	if( c == '-' ) invert = true;
	else if( c >= '0' && c <= '9' )
	{
		v = c - '0';
	}
	else throw exception();

	while( 1 )
	{
		c = stream->getc();
		if( c >= '0' && c <= '9' ) v = 10 * v + (c - '0');
		else break;
	}

	// check what stopped the loop
	if( c == '.' )
	{
		long double k = 0.1L;

		while( 1 )
		{
			c = stream->getc();
			if( c >= '0' && c <= '9' )
			{
				v += k * (c - '0');
				k *= 0.1L;
			}
			else break;
		}
	}

	if( invert ) v = -v;

	if( c == 'e' || c == 'E' )
	{
		bool ExpInv = false;
		int exp = 0;
		c = stream->getc();

		if( c == '-' ) ExpInv = true;

		if( c != '-' && c != '+' ) stream->ungetc();

		while( 1 )
		{
			c = stream->getc();
			if( c >= '0' && c <= '9' )
			{
				exp = 10 * exp + (c - '0');
			}
			else break;
		}

		if( ExpInv ) exp = -exp;

		v *= pow( 10.L , exp );
	}

	stream->ungetc();
	
	return (double)v;//ValuePtr( new JJSON_Value_Number( (double)v ) );
	*/
}

static ValueArray GetArray( JJSON_Stream *stream, JJSON_JSON json )
{
	if( stream->getc() != '[' ) throw exception();

	ValueArray res;

	SkipWhiteSpaces( stream );

	int c = stream->getc();
	if( c == EOF ) throw exception();

	while( c != ']' )
	{
		stream->ungetc();
		res.push_back( GetValue( stream, json ) );

		c = stream->getc();
		if( c == ',' ) SkipWhiteSpaces( stream ), c = stream->getc();
		if( c == EOF ) throw exception();
	}

	return res;//ValuePtr( new JJSON_Value_Array( move( res ) ) );
}

static ValueMap GetObject( JJSON_Stream *stream, JJSON_JSON json )
{
	if( stream->getc() != '{' ) throw exception();

	ValueMap res;

	SkipWhiteSpaces( stream );

	int c = stream->getc();
	if( c == EOF ) throw exception();

	while( c != '}' )
	{
		stream->ungetc();
		string key = GetString( stream );
		SkipWhiteSpaces( stream );
		if( stream->getc() != ':' ) throw exception();

		res[move( key )] = move( GetValue( stream, json ) );

		c = stream->getc();
		if( c == ',' ) SkipWhiteSpaces( stream ), c = stream->getc();
		if( c == EOF ) throw exception();
	}
	return res;//ValuePtr( new JJSON_Value_Object( move( res ) ) );
}

extern "C" 
{
	JJSON_Value JJSON_JSON_GetValue( const JJSON_JSON json )
	{
		return json ? &json->mainValue : NULL;
	}

	static int JJSON_FileReadFn( void *user )
	{
		return getc( (FILE *)user );
	}

	JJSON_JSON JJSON_ReadFilePath( const char *path )
	{
		FILE *f = fopen( path, "r" );
		if( !f ) return NULL;

		JJSON_JSON result = JJSON_ReadStream( JJSON_FileReadFn, (void*)f );

		fclose( f );
		return result;
	}

	JJSON_JSON JJSON_ReadStream( JJSON_GetcFn InStream, void *user )
	{
		if( !InStream ) return NULL;

		JJSON_Stream stream;
		stream.stream = InStream;
		stream.user = user;

		JJSON_JSON json = NULL;

		try
		{
			json = new JJSON_JSON_t_; // throws on bad alloc

			//return GetValue( &stream ).release();
			json->mainValue = GetValue( &stream, json );

			if( json->mainValue.type != JJSON_object ) throw exception();

			return json;
		}
		catch( ... )
		{
			delete json;
			return NULL;
		}
	}

	void JJSON_Free( JJSON_JSON json )
	{
		delete json;
	}
}

/////////
// C++ //
/////////

// JJSON.hpp

//JSONValue::JSONValue( JJSON_Value v_, const JJSON_JSON json_ ) noexcept : v( v_ ), json( json_ ) {};
JJSON_types JSONValue::type() const noexcept { return JJSON_GetValueType( v ); }

JSONValue::JSONValue() noexcept
{
	v = NULL;
}

JSONValue::JSONValue( JJSON_Value v_ ) noexcept
{
	v = v_;
}

void JSONValue::AssertType( JJSON_types t ) const
{
	JJSON_types cur = type();
	if( cur != t )
	{
		const char *toStr[JJSON_type_cnt] =
		{
			"Null", "String", "Number", "Object", "Array", "Bool"
		};

		JSON_TypeException e;

		e.message = { string() + "JSON: object is of type '" + toStr[cur] + "',"
			"but expected '" + toStr[t] + '\'' };

		throw e;
	}
}

bool JSONValue::GetBool() const 
{ 
	AssertType( JJSON_bool ); 
	return JJSON_GetBool( v );
}

double JSONValue::GetNumber() const 
{ 
	AssertType( JJSON_number ); 
	return JJSON_GetNumber( v );
}

const char *JSONValue::GetString() const
{
	AssertType( JJSON_string ); 
	return JJSON_GetString( v );
}

size_t JSONValue::size() const
{
	AssertType( JJSON_array );
	return JJSON_Array_Size( v );
}

JSONValue JSONValue::operator[]( size_t idx ) const
{
	AssertType( JJSON_array );
	JJSON_Value q = JJSON_Array_At( v, idx );
	if( !q )
	{
		JSON_OutOfRangeException e;
		e.message = string() + "JSON: array index out of range";
		throw e;
	}

	return q;//{ q, json };
}

JJSON_Value JSONValue::GetObjectInternal( const char *key ) const
{
	AssertType( JJSON_object );
	return JJSON_Object_Get( v, key );
}

bool JSONValue::contains( const string &key ) const
{
	return GetObjectInternal( key.c_str() ) != NULL;
}

JSONValue JSONValue::operator[]( const string &key ) const
{
	JJSON_Value q = GetObjectInternal( key.c_str() );

	if( !q )
	{
		JSON_OutOfRangeException e;
		e.message = string() + "JSON: key '" + key + "' not in object";
		throw e;
	}

	return q;//{ q, json };
}

JSON::JSON() noexcept {}

JSON::JSON( JSON &&o ) noexcept
{
	v = o.v;
	o.v = NULL;
}

JSON &JSON::operator=( JSON &&o ) noexcept
{
	if( this != &o )
	{
		if( v ) JJSON_Free( v->json );
		v = o.v;
		o.v = NULL;
	}

	return *this;
}

JSON::JSON( const char *path )
{
	v = JJSON_JSON_GetValue( JJSON_ReadFilePath( path ) );

	if( !v )
	{
		JSON_ParseException e;
		e.message = string() + "couldn't parse file " + path + "as json";
		throw e;
	}
}

JSON::JSON( JJSON_GetcFn InStream, void *user )
{
	v = JJSON_JSON_GetValue( JJSON_ReadStream( InStream, user ) );

	if( !v )
	{
		JSON_ParseException e;
		e.message = "couldn't parse stream as json";
		throw e;
	}
}

JSON::~JSON() noexcept { if( v ) JJSON_Free( v->json ); v = NULL; }

//bool JSON::contains( const std::string &key ) const { return get().contains( key ); }
//JSONValue JSON::operator[]( const std::string &key ) const { return get()[ key ]; }
