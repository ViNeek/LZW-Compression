/*

	Decoder Dictionary Structure

*/

#ifndef _LZW_DEC_DICTIONARY_H_
#define _LZW_DEC_DICTIONARY_H_

#include "LZWHash.h"

typedef struct _DecDictionary	DecDictionary;

DecDictionary *		decDict_new(int_32 size);
uint_32				decDict_insert(DecDictionary *hDict, const ulong_32 pindex, const BYTE sym);
BOOL				decDict_contains(DecDictionary *hDict, uint_32 code, ulong_32 *pindex, BYTE *sym);
ulong_32			decDict_get_index_at(DecDictionary *hDict, uint_32 code);
BYTE				decDict_get_sym_at(DecDictionary *hDict, uint_32 code);
BYTE				*decDict_get_at(DecDictionary *hDict, uint_32 code);
uint_32				decDict_size(DecDictionary *hDict);
void				decDict_insert_at(DecDictionary *hDict, int pos, const ulong_32 pindex, const BYTE sym);
void				decDict_free(DecDictionary *hDict);
void				decDict_print(DecDictionary *hDict);

#endif //_LZW_DEC_DICTIONARY_H_