#pragma once
#ifndef JJSON_INCLUDE_JJSON_COMMON_H
#define JJSON_INCLUDE_JJSON_COMMON_H

// defines possible JSON types
// JJSON_null: null type
// JJSON_string: string type
// JJSON_number: number type 
// JJSON_object: associative array type
// JJSON_array:  array type
// JJSON_bool:   boolean type
enum JJSON_types { JJSON_null, JJSON_bool, JJSON_number, JJSON_string, JJSON_object, JJSON_array, JJSON_type_cnt };

// represents a JSON value of any type (as in JJSON_types)
typedef struct JJSON_JSON_t_ *JJSON_JSON;
typedef const struct JJSON_Value_t_ *JJSON_Value;

/// <summary>
/// user defined callback to get a character from stream
/// </summary>
/// <param name="user"> passed is equal to 'user' passed to JJSON_ReadStream</param>
/// <returns>character or EOF on error (after returning EOF the function is never called again)</returns>
typedef int (*JJSON_GetcFn)(void *user);

#endif // ndef JJSON_INCLUDE_JJSON_COMMON_H