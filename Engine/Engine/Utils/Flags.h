#pragma once

#define CREATE_FLAG_OF_ENUM(enumName) \
inline enumName operator~(enumName a)\
{\
	return (enumName)~(int)a;\
}\
inline enumName operator|(enumName a, enumName b)\
{\
	return (enumName)((int)a | (int)b);\
}\
inline enumName operator&(enumName a, enumName b)\
{\
	return (enumName)((int)a & (int)b);\
}\
inline enumName operator^(enumName a, enumName b)\
{\
	return (enumName)((int)a ^ (int)b);\
}\
inline enumName& operator|=(enumName& a, enumName b)\
{\
	return (enumName&)((int&)a |= (int)b);\
}\
inline enumName& operator&=(enumName& a, enumName b)\
{\
	return (enumName&)((int&)a &= (int)b);\
}\
inline enumName& operator^=(enumName& a, enumName b)\
{\
	return (enumName&)((int&)a ^= (int)b);\
}