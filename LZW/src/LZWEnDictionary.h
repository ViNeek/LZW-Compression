#ifndef _LZW_EN_DICTIONARY_H_
#define _LZW_EN_DICTIONARY_H_

#include "LZWHash.h"

typedef ulong_32	(*HashFunc)(const void *);
typedef struct _EnDictionary		EnDictionary;

ulong_32			enDict_hash(EnDictionary *hDict, const void *code);
EnDictionary *		enDict_new(int maxbitsize, HashFunc hash_func);
void				enDict_insert(EnDictionary *hDict, const SeqKey *skey, uint_32 code, uint_32 *index, uint_32 *offset);
void				enDict_insert_at(EnDictionary *hDict, uint_32 index, const SeqKey *skey, uint_32 code, uint_32 *offset);
BOOL				enDict_search(EnDictionary *hDict, const SeqKey *skey, uint_32 *code, uint_32 *pindex, uint_32 *poffset);
void				enDict_free(EnDictionary *hDict);
void				enDict_print(EnDictionary *hDict);
uint_32				enDict_size(EnDictionary *hDict);

#endif //_LZW_HASH_TABLE_H_