/*
 *	string.h
 *	bedreamer@163.com
 */
#ifndef _STD_STRING_INCLUDE_H_
#define _STD_STRING_INCLUDE_H_

#ifndef _STDDEF_
	#include <stddef.h>
#endif // _STDDEF_

extern int 
	strcpy(
	char */*desnation*/,		
	const char */* source string*/);
extern size_t 
	strncpy(
	char */*desnation*/,
	const char */*source string*/,
	size_t/*max string length*/);
extern int 
	strcmp(
	const char */*string 1*/,
	const char */*string 2*/);
extern int 
	strncmp(
	const char */*string 1*/,
	const char */*string 2*/,
	size_t/*max string length*/);
extern size_t 
	strcat(
	char */*desnation*/,
	const char */*string to add*/);
extern size_t 
	strlen(
	const char*/*string to measure*/);
extern size_t 
	strnlen(
	const char*/*string to measure*/,
	size_t/*max strin length*/);
extern const char *
	strstr(
	const char */*string 1*/,
	const char */*string 2*/);
extern size_t 
	tolower(
	char */*string to convert*/);
extern size_t 
	tonlower(
	char */*string to convert*/,
	size_t/*max string length*/);
extern size_t 
	toupper(
	char */*string to convert*/);
extern size_t 
	tonupper(
	char */*string to convert*/,
	size_t/*max string length*/);
extern size_t 
	memcpy(
	void */*desnation*/,
	const void */*source*/,
	size_t/*length to copy in byte*/);
extern size_t 
	memmove(
	void */*desnation*/,
	const void */*start address*/,
	size_t/*length to move in byte*/);
extern size_t 
	memset(
	void */*denation*/,
	int/*fill data*/,
	size_t/*size of memory to set.*/);

#endif // _STD_STRING_INCLUDE_H_