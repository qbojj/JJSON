#pragma once

#ifndef JJSON_INCLUDE_JJSON_H
#define JJSON_INCLUDE_JJSON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "JJSONcommon.h"

	JJSON_types JJSON_GetValueType( JJSON_Value Value );
	const char *JJSON_GetString( JJSON_Value String );
	double JJSON_GetNumber( JJSON_Value Number );
	bool JJSON_GetBool( JJSON_Value Bool );

	JJSON_Value JJSON_Object_Get( JJSON_Value Object, const char *key );

	size_t JJSON_Array_Size( JJSON_Value Array );
	JJSON_Value JJSON_Array_At( JJSON_Value Array, size_t index );

	JJSON_Value JJSON_JSON_GetValue( const JJSON_JSON json );

	/// <summary>
	/// create JSON object from stream
	/// </summary>
	/// <param name="InStream">stream to read from</param>
	/// <param name="user">JJSON_GetcFn 'user' argument</param>
	/// <returns>json object or NULL on error</returns>
	JJSON_JSON JJSON_ReadStream( JJSON_GetcFn InStream, void *user );

	/// <summary>
	/// create JSON object form file
	/// </summary>
	/// <param name="path">path to file to read from</param>
	/// <returns>json object or NULL on error</returns>
	JJSON_JSON JJSON_ReadFilePath( const char *path );

	void JJSON_Free( JJSON_JSON json );

#ifdef __cplusplus
} // extern "C"
#endif

#endif //def JJSON_INCLUDE_JJSON_H