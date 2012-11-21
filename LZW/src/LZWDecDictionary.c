#include "LZWDecDictionary.h"

struct _DecDictionary
{
	int_32				size;
	int_32				noccupied;
	uint_32				last_index;

	ulong_32			*prevIndexes;
	BYTE				*symbols;

	BYTE				*strBuffer;
	uint_32				strBufferSize;
};

DecDictionary *		decDict_new(int_32 size) {
	DecDictionary *hDict;

	hDict = (DecDictionary *)malloc( sizeof(DecDictionary) );

	hDict->noccupied = 1;

	if  ( size < 256 ) 
		hDict->size = 256;
	else
		hDict->size = size;

	hDict->prevIndexes = (ulong_32 *)malloc( sizeof(ulong_32) * size );
	hDict->symbols = (BYTE *)malloc( sizeof(BYTE) * size );

	hDict->strBufferSize = 1024;
	hDict->strBuffer = (BYTE *)malloc( sizeof(BYTE) * hDict->strBufferSize );
	memset(hDict->strBuffer, 0, sizeof(BYTE) * hDict->strBufferSize);

	memset(hDict->symbols, -1, sizeof(BYTE) * size);
	memset(hDict->prevIndexes, 0, sizeof(ulong_32) * size);

	return hDict;
}

uint_32				decDict_insert(DecDictionary *hDict, const ulong_32 pindex, const BYTE sym) {
	if ( hDict->noccupied == hDict->size ) {
		hDict->size <<= 1;
		hDict->prevIndexes = (ulong_32 *)realloc(hDict->prevIndexes, sizeof(ulong_32) * hDict->size );
		hDict->symbols = (BYTE *)realloc(hDict->symbols, sizeof(BYTE) * hDict->size );

		memset(hDict->symbols + (hDict->size >> 1), -1, sizeof(BYTE) * (hDict->size >> 1) );
		memset(hDict->prevIndexes + (hDict->size >> 1), 0, sizeof(ulong_32) * (hDict->size >> 1) );
	}
	
	hDict->prevIndexes[hDict->noccupied] = pindex;
	hDict->symbols[hDict->noccupied] = sym;

	return hDict->noccupied++;
}

uint_32 decDict_size(DecDictionary *hDict) {
	return hDict->noccupied;
}

void				decDict_insert_at(DecDictionary *hDict, int_32 pos, const ulong_32 pindex, const BYTE sym) {
	if ( hDict->noccupied == hDict->size ) {
		hDict->size <<= 1;
		hDict->prevIndexes = (ulong_32 *)realloc(hDict->prevIndexes, sizeof(ulong_32) * hDict->size );
		hDict->symbols = (BYTE *)realloc(hDict->symbols, sizeof(BYTE) * hDict->size );

		memset(hDict->symbols + (hDict->size >> 1), -1, sizeof(BYTE) * (hDict->size >> 1) );
		memset(hDict->prevIndexes + (hDict->size >> 1), 0, sizeof(ulong_32) * (hDict->size >> 1) );
	}
	
	hDict->prevIndexes[pos] = pindex;
	hDict->symbols[pos] = sym;

	hDict->noccupied++;
}

BOOL				decDict_contains(DecDictionary *hDict, uint_32 code, ulong_32 *pindex, BYTE *sym) {
	if ( hDict->noccupied <= code ) {

		return FALSE;
	}
	else {
		*pindex = hDict->prevIndexes[code];
		*sym = hDict->symbols[code];

		return TRUE;
	}
}

ulong_32			decDict_get_index_at(DecDictionary *hDict, uint_32 code) {
	return hDict->prevIndexes[code];
}

BYTE				decDict_get_sym_at(DecDictionary *hDict, uint_32 code) {
	return hDict->symbols[code];
}

BYTE				*decDict_get_at(DecDictionary *hDict, uint_32 code) {
	BYTE		c;
	ulong_32	index;
	int			strLen;

	index = decDict_get_index_at(hDict, code);
	c = decDict_get_sym_at(hDict, code);
	hDict->strBuffer[0] = c;
	strLen = 1;
	
	while ( index != 0 ) {
		if ( (strLen + 1) == hDict->strBufferSize ) {
			hDict->strBufferSize <<= 1;
			hDict->strBuffer = (BYTE *)realloc(hDict->strBuffer, sizeof(BYTE) * hDict->strBufferSize);
			
			memset(hDict->strBuffer + (hDict->strBufferSize >> 1), 0, sizeof(BYTE) * (hDict->strBufferSize >> 1) );
		}
		c = decDict_get_sym_at(hDict, index);
		index = decDict_get_index_at(hDict, index);
		hDict->strBuffer[strLen] = c;
		
		strLen++;
	}

	hDict->strBuffer[strLen] = 0;
	
	return hDict->strBuffer;
}

void				decDict_print(DecDictionary *hDict) {
	int i;
	for ( i = 0; i < hDict->noccupied; i++ ) {
		printf("Pos %d: %d%d\n",i, hDict->prevIndexes[i], hDict->symbols[i]);
	}
}

void				decDict_free(DecDictionary *hDict) {
	if ( hDict ) {
		if ( hDict->prevIndexes )
			free(hDict->prevIndexes);

		if ( hDict->symbols )
			free(hDict->symbols);

		if ( hDict->strBuffer )
			free(hDict->strBuffer);
	
		free( hDict );
	}
}
