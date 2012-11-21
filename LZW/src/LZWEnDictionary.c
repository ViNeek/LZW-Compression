#include "LZWEnDictionary.h"

struct _EnDictionary
{
	int             maxsize;
	int				maxbitsize;

	int             noccupied;

	void			**buckets;

	HashFunc		hash;
};

ulong_32		enDict_hash(EnDictionary *hDict, const void *code) {
	return hDict->hash(code) % hDict->maxsize;
}

EnDictionary *			enDict_new(int maxbitsize, HashFunc hash_func) {
	EnDictionary		*hDict;
	
	hDict = (EnDictionary *)malloc( sizeof(EnDictionary) );

	hDict->maxbitsize = maxbitsize;
	hDict->maxsize = 1 << maxbitsize;
	hDict->noccupied = 0;
	hDict->hash = hash_func;

	hDict->buckets = (void **)malloc( sizeof( void * ) * hDict->maxsize );
	memset(hDict->buckets, 0, sizeof( void * ) * hDict->maxsize);

	return hDict;
}

void			enDict_insert(EnDictionary *hDict, const SeqKey *skey, uint_32 code, uint_32 *index, uint_32 *offset) {
	ulong_32	key;
	void		*bucket_ptr;
	void		*iterator;
	int_32		init_size, init_pop;
	int_32		current_pop;
	int_32		new_size;

	key = hDict->hash(skey->raw) % hDict->maxsize;
	bucket_ptr =  hDict->buckets[key];
	*index = key;

	if ( bucket_ptr ) {
		current_pop = *((int_32 *)bucket_ptr);
		*offset = current_pop;
		new_size = (current_pop + 1) * DICT_ELEM_SIZE + TABLE_HEADER_SIZE;
		
		hDict->buckets[key] = realloc(hDict->buckets[key], new_size );
		iterator = bucket_ptr = hDict->buckets[key];

		iterator = (BYTE *)bucket_ptr + current_pop * DICT_ELEM_SIZE + TABLE_HEADER_SIZE;

		current_pop++;
		memcpy( bucket_ptr, &current_pop, TABLE_HEADER_SIZE );

		memcpy( (BYTE *)iterator, skey->raw, KEY_BYTESIZE );
		memcpy( (BYTE *)iterator + KEY_BYTESIZE, &code, sizeof(int_32) );


	} else {
		init_pop = 1;
		*offset = init_pop - 1;
		init_size = init_pop * DICT_ELEM_SIZE + TABLE_HEADER_SIZE;
		
		hDict->buckets[key] = malloc( init_size );
		iterator = bucket_ptr = (int *)hDict->buckets[key];
		
		iterator = (BYTE *)bucket_ptr + TABLE_HEADER_SIZE;

		memcpy( bucket_ptr, &init_pop, TABLE_HEADER_SIZE );

		memcpy( (BYTE *)iterator, skey->raw, KEY_BYTESIZE );
		memcpy( (BYTE *)iterator + KEY_BYTESIZE, &code, sizeof(int_32) );
	}

	hDict->noccupied++;
}

void				enDict_insert_at(EnDictionary *hDict, uint_32 index, const SeqKey *skey, uint_32 code, uint_32 *offset) {
	void		*bucket_ptr;
	void		*iterator;
	int_32		init_size, init_pop;
	int_32		current_pop;
	int_32		new_size;
	
	bucket_ptr = hDict->buckets[index];
	
	if ( bucket_ptr ) {
		current_pop = *((int_32 *)bucket_ptr);
		*offset = current_pop;
		new_size = (current_pop + 1) * DICT_ELEM_SIZE + TABLE_HEADER_SIZE;
		
		hDict->buckets[index] = realloc(hDict->buckets[index], new_size );
		iterator = bucket_ptr = hDict->buckets[index];

		iterator = (BYTE *)bucket_ptr + current_pop * DICT_ELEM_SIZE + TABLE_HEADER_SIZE;

		current_pop++;
		memcpy( bucket_ptr, &current_pop, TABLE_HEADER_SIZE );

		memcpy( (BYTE *)iterator, skey->raw, KEY_BYTESIZE );
		memcpy( (BYTE *)iterator + KEY_BYTESIZE, &code, sizeof(int_32) );
	} else {
		init_pop = 1;
		*offset = init_pop - 1;
		init_size = init_pop * DICT_ELEM_SIZE + TABLE_HEADER_SIZE;
		
		hDict->buckets[index] = malloc( init_size );
		iterator = bucket_ptr = (int *)hDict->buckets[index];
		
		iterator = (BYTE *)bucket_ptr + TABLE_HEADER_SIZE;

		memcpy( bucket_ptr, &init_pop, TABLE_HEADER_SIZE );

		memcpy( (BYTE *)iterator, skey->raw, KEY_BYTESIZE );
		memcpy( (BYTE *)iterator + KEY_BYTESIZE, &code, sizeof(int_32) );
	}

	hDict->noccupied++;
}

BOOL				enDict_search(EnDictionary *hDict, const SeqKey *skey, uint_32 *code, uint_32 *pindex, uint_32 *poffset) {
	ulong_32	key;
	void		*bucket_ptr;
	void		*iterator;
	int_32		current_pop;
	int_32		num_read;

	key = hDict->hash(skey->raw) % hDict->maxsize;
	iterator = bucket_ptr =  hDict->buckets[key];
	*pindex = key;

	if ( bucket_ptr ) {
		current_pop = *((int_32 *)bucket_ptr);
		num_read = 0;
		iterator = (BYTE *)bucket_ptr + TABLE_HEADER_SIZE;
		
		while ( num_read < current_pop ) {
			
			if ( memcmp(iterator, skey->raw, KEY_BYTESIZE) == 0 ) {
				memcpy( poffset, &num_read, sizeof(int_32) );
				memcpy( code, (BYTE *)iterator + KEY_BYTESIZE, sizeof(int_32) );

				return TRUE;
			}

			iterator = (BYTE *)iterator + DICT_ELEM_SIZE;
			num_read++;
		}

		return FALSE;
	}

	return FALSE;
}

uint_32				enDict_size(EnDictionary *hDict) {
	return hDict->noccupied;
}

void				enDict_free(EnDictionary *hDict) {
	int i;

	for( i = 0; i < hDict->maxsize; i++ ) {
		if ( hDict->buckets[i] ) {
			free(hDict->buckets[i]);
			hDict->buckets[i] = 0;
		}
	}

	free(hDict->buckets);
	free(hDict);
}

void				enDict_print(EnDictionary *hDict) {
	int			i;
	void		*iter;
	int			size, code;
	int			num_read;
	SeqKey		sk;
	FILE		*fp;

	fp = fopen("DenugTest.txt","w");
	fprintf(fp, "Population %d\n", hDict->noccupied);
	for( i = 0; i < hDict->maxsize; i++ ) {
		num_read = 0;
		size = 0;

		if ( hDict->buckets[i] ) {
			fprintf(fp, "Bucket %d: ", i);
			iter = hDict->buckets[i];
			size = *((int_32 *)iter);
			fprintf(fp, "%d ", size);

			iter = (BYTE *)iter + TABLE_HEADER_SIZE;
			while (num_read < size ) {
				memcpy(sk.raw, iter, KEY_BYTESIZE);
				memcpy(&code, (BYTE *)iter + KEY_BYTESIZE, sizeof(int_32));
				
				fprintf(fp, "%d|%d|%c|%d|%d", sk.data.prevIndex, sk.data.prevOffset, sk.data.sym, sk.data.sym, code);
				fprintf(fp, " ");
				
				num_read++;
				iter = (BYTE *)iter + DICT_ELEM_SIZE;
			}

			fprintf(fp, "\n");
		} else {
			fprintf(fp, "Bucket %d: NONE", i);

			fprintf(fp, "\n");
		}
	}

	fclose(fp);

}