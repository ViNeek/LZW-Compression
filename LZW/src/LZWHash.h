#ifndef _LZW_HASH_H_
#define _LZW_HASH_H_

#ifdef _WIN32

//LZW types
#define uint_32		unsigned __int32
#define int_32		signed __int32
#define ulong_32	unsigned __int32
#define long_32		signed __int32

#else

#include <stdint.h>

#define uint_32		uint32_t
#define int_32		int32_t
#define ulong_32	uint32_t
#define long_32		int32_t

#endif

#define UNUSED_CODE				-1
#define UNUSED_KEY				0
#define BYTESIZE				sizeof(unsigned char)*8
#define NODE_BYTESIZE			6
#define HEADER_SIZE				sizeof(int)
#define	KEY_BYTESIZE			9
#define TABLE_HEADER_SIZE		sizeof(int_32)
#define DICT_ELEM_HEADER		2 * sizeof(int_32)
#define DICT_ELEM_SIZE			( 2 * sizeof(int_32) + 1 * sizeof(BYTE) + 1 * sizeof(int_32) )

//LZW types
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32

//#define _CRTDBG_ALLOC
//#define _CRTDBG_MAP_ALLOC
#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
//#include <crtdbg.h>

#else

#define BOOL	int
#define TRUE	1
#define FALSE	0

#endif

#define BYTE	unsigned char

typedef struct _CharSeq {
	BYTE		*seq;
	int			len;
} CharSeq;

typedef union _SeqKey {
	BYTE				raw[9];
	struct {
		ulong_32		prevIndex;
		int_32			prevOffset;
		BYTE			sym;
	} data;
} SeqKey;

ulong_32	sdbm( const void *cseq );
ulong_32	bwsh( const void *cseq );
ulong_32	ihash( const void *ptr );
ulong_32	djbh( const void *cseq );
ulong_32	MurmurHash3( const void *cseq );

void		setMurmurHash3Seed( uint_32 );

BOOL cseq_cmp(const CharSeq *cseq1, const CharSeq *cseq2);

int			is_big_endian();

#endif //_LZW_HASH_H_
