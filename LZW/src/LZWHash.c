#include "LZWHash.h"

#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE
#endif

/*
static inline FORCE_INLINE  rotl32 ( uint32_t x, int8_t r )
{
  return (x << r) | (x >> (32 - r));
}

static inline FORCE_INLINE uint64_t rotl64 ( uint64_t x, int8_t r )
{
  return (x << r) | (x >> (64 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)
*/

int is_big_endian(void)
{
    union {
        uint_32		i;
        char		c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1; 
}

ulong_32 ihash( const void *ptr ) {
	unsigned	h;
	char		c;

	h = *((unsigned *)ptr);
	c = *((char *)ptr + 4);

	h = (h+0x7ed55d16) + (h<<12);
	h = (h^0xc761c23c) ^ (h>>19);
	h = (h+0x165667b1) + (h<<5);
	h = (h+0xd3a2646c) ^ (h<<9);
	h = (h+0xfd7046c5) + (h<<3);
	h = (h^0xb55a4f09) ^ (h>>16);

	return h^c;
}

ulong_32 sdbm( const void *cseq ) {
	int c, i;
	ulong_32 hash = 0;

	i = 0;
	while ( i++ < ((CharSeq *)cseq)->len ) {
		c = ((CharSeq *)cseq)->seq[i];
		hash = c + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

ulong_32 bwsh( const void *cseq ) {
	int_32		c, i;
	ulong_32	hash = 65599;
	BYTE		*ptr;
	int			len;

	len = ((CharSeq *)cseq)->len;
	ptr = ((CharSeq *)cseq)->seq;
	i = 0;

	//printf("Hashing: ");
	while ( i < len ) {
		c = ptr[i++];
		//printf("%c",c);
		hash ^= ( (hash << 5) + (hash >> 2) + c );
	}
	//printf(" to %u\n", hash);
	return hash;
}

ulong_32	djbh( const void *cseq ) {
	int c, i;
	ulong_32 hash = 5381;
	BYTE *ptr;
	int len;

	len = ((CharSeq *)cseq)->len;
	ptr = ((CharSeq *)cseq)->seq;

	i = 0;
	while ( i < len ) {
		c = ptr[i++];
		hash = hash * 33 + c;
	}

	return hash;
}

ulong_32	murmur3h( const void *cseq ) {
	return -1;
}

BOOL cseq_cmp(const CharSeq *cseq1, const CharSeq *cseq2) {
	int i, len;

	len = cseq1->len;
	i = 0;
	if ( cseq1->len != cseq2->len )
		return FALSE;

	while(i < len)
		if( cseq1->seq[i] != cseq2->seq[i] )
			return FALSE;
		else
			i++;

	return TRUE;
	//return !memcmp(cseq1->seq, cseq2->seq, cseq1->len);
}

static uint_32 MurmurHash3Seed = 42;

void		setMurmurHash3Seed( uint_32 seed) {
	MurmurHash3Seed = seed;
}

#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE
#endif

static __inline FORCE_INLINE uint_32 rotl32 ( uint_32 x, BYTE r )
{
  return (x << r) | (x >> (32 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)

#define getblock(p, i) (p[i])

static __inline FORCE_INLINE uint_32 fmix32 ( uint_32 h )
{
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

ulong_32	MurmurHash3 ( const void *cseq )
{
	const SeqKey	*key = (SeqKey *)cseq;
	const BYTE		*data = key->raw;
	const int_32	len = KEY_BYTESIZE;
	const int_32	nblocks = len / 4;
	const BYTE		*tail;
	uint_32			k1;
	int_32			i;

	ulong_32 h1 = MurmurHash3Seed;

	ulong_32 c1 = 0xcc9e2d51;
	ulong_32 c2 = 0x1b873593;

	//----------
	// body

	const ulong_32 *blocks = (const ulong_32 *)((const BYTE *)data + nblocks*4);

	for(i = -nblocks; i; i++)
	{
		ulong_32 k1 = getblock(blocks,i);

		k1 *= c1;
		k1 = ROTL32(k1,15);
		k1 *= c2;
    
		h1 ^= k1;
		h1 = ROTL32(h1,13); 
		h1 = h1*5+0xe6546b64;
	}

	tail = (const BYTE *)data + nblocks*4;

	k1 = 0;

	switch(len & 3)
	{
		case 3: k1 ^= tail[2] << 16;
		case 2: k1 ^= tail[1] << 8;
		case 1: k1 ^= tail[0];
          
		k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
	};

	//----------
	// finalization

	h1 ^= len;

	h1 = fmix32(h1);

	//printf("Val %u\n", h1);
	return h1;
} 

/*
ulong_32	MurmurHash3 ( const void *cseq )
{
	const CharSeq	*key = (CharSeq *)cseq;
	const BYTE		*data = key->seq;
	const int_32	len = key->len;
	const int_32	nblocks = key->len / 4;
	const BYTE		*tail;
	uint_32			k1;
	int_32			i;

	ulong_32 h1 = MurmurHash3Seed;

	ulong_32 c1 = 0xcc9e2d51;
	ulong_32 c2 = 0x1b873593;

	//----------
	// body

	const ulong_32 *blocks = (const ulong_32 *)((const BYTE *)data + nblocks*4);

	for(i = -nblocks; i; i++)
	{
		ulong_32 k1 = getblock(blocks,i);

		k1 *= c1;
		k1 = ROTL32(k1,15);
		k1 *= c2;
    
		h1 ^= k1;
		h1 = ROTL32(h1,13); 
		h1 = h1*5+0xe6546b64;
	}

	tail = (const BYTE *)data + nblocks*4;

	k1 = 0;

	switch(len & 3)
	{
		case 3: k1 ^= tail[2] << 16;
		case 2: k1 ^= tail[1] << 8;
		case 1: k1 ^= tail[0];
          
		k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
	};

	//----------
	// finalization

	h1 ^= len;

	h1 = fmix32(h1);

	//printf("Val %u\n", h1);
	return h1;
} 
*/