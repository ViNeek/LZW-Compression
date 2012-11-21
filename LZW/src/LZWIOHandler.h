#ifndef _LZW_IOHANDLER_H_
#define _LZW_IOHANDLER_H_

#define COMPRESSED_INPUT	0
#define UNCOMPRESSED_INPUT	1

#include "LZWHash.h"

#define COMPRESSION_ID		1
#define DECOMPRESSION_ID	2

/* File Input Handler
 * Class representing the input of LZW
 * abstarcting the way data is read for
 * compressed or uncompressed sources
 */

typedef struct _LZWInput		LZWInput;
typedef BYTE					*(*ReadFunc)( LZWInput * );

LZWInput*		input_new(const char *src, ReadFunc rf);
BYTE			*input_pull(LZWInput *in);
void			input_free(LZWInput *in);
BOOL			input_end(LZWInput *in);
void			input_next(LZWInput *in);
void			input_check(LZWInput *in, uint_32 code);

BYTE			*readUncompressedData(LZWInput *ptr);
BYTE			*readCompressedData(LZWInput *ptr);

/* File Output Handler
 * Class representing the output of LZW
 * abstarcting the way data is writeen to
 * compressed or uncompressed destinations
 */

typedef struct _LZWOutput		LZWOutput;
typedef void					(*WriteFunc)( LZWOutput *, const void *);

LZWOutput*			output_new(WriteFunc wf);
void				output_push(LZWOutput *out, void *raw);
void				output_free(LZWOutput *out);
void				output_dump(LZWOutput *out, const char *dst, uint_32 opType);
void				output_check(LZWOutput *out, uint_32 code);

void			writeUncompressedData(LZWOutput *out, const void *raw);
void			writeCompressedData(LZWOutput *out, const void *raw);
/* Read Functions
 * Depending on whether the input is a compressed 
 * or uncompressed file, the correct
 * Read Function is used
 */


/* Read Functions
 * Depending on whether the output is a compressed 
 * or uncompressed file, the correct
 * Write Function is used
 */

#endif //_LZW_IOHANDLER_H_