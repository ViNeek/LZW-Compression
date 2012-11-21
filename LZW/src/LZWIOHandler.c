#include "LZWIOHandler.h"

struct _LZWInput
{
	FILE			*fd;
	
	BYTE			*dataStream;

	BYTE			*current;

	uint_32			activeBitLength;
	uint_32			curBitIndex;

	uint_32			currentCodeVal;

	int             inputSize;
	int				type;

	ReadFunc		read;
};

struct _LZWOutput
{
	FILE			*fd;
	
	BYTE			*outBuffer;

	uint_32			activeBitLength;
	uint_32			curBitIndex;

	int_32			bufferSize;
	int_32			bufferOccupied;

	WriteFunc		write;
};

LZWInput*	input_new(const char *src, ReadFunc rf) {
	LZWInput	*in;

	in = (LZWInput *)malloc(sizeof(LZWInput));

	in->read = rf;

	in->fd = fopen(src,"rb");
	if ( in->fd == NULL )
	{
		printf("Error opening file %s\n",src);
		exit(1);
		return NULL;
	}

	fseek(in->fd,0,SEEK_END);
	in->inputSize = ftell(in->fd);
	fseek(in->fd,0,SEEK_SET);
	in->dataStream = (BYTE *)malloc((sizeof(BYTE)*in->inputSize)+1);
	fread(in->dataStream,sizeof(BYTE),in->inputSize,in->fd);
	in->dataStream[in->inputSize] = 0;

	in->current = &in->dataStream[0];

	in->activeBitLength = 9;
	in->curBitIndex = 0;

	in->currentCodeVal = 0;

	printf("File Size %d Bytes\n",in->inputSize);

	fclose(in->fd);

	return in;
}

void			input_check(LZWInput *in, uint_32 code) {
	if ( ( (1 << in->activeBitLength) - 1 ) == code ) {
		in->activeBitLength++;
	}
}

BOOL			input_end(LZWInput *in) {
	return ( ( in->current - in->dataStream ) >= in->inputSize );
}

BYTE			*input_pull(LZWInput *in) {
	return in->read(in);
}

BYTE			*readUncompressedData(LZWInput *in) {
	return in->current++;
}

BYTE			*readCompressedData(LZWInput *in) {
	//BYTE *seq = ptr->current;
	//printf("Truth %d %d\n", *seq, *((uint_32 *)seq));
	//ptr->current += 4;

	//return seq;
	
	uint_32 bits, iValue, bitsLeft;
    uint_32 resBitIndex, mask;
	
	in->currentCodeVal = 0;
	bits = in->activeBitLength;
	resBitIndex = 0;

    while(bits > 0)
    {
		iValue = *in->current;
        if(in->curBitIndex == 0)
        {
            if(bits < 8)
            {
                mask = ~(~0U << bits);
                in->currentCodeVal |= (iValue & mask) << resBitIndex;
                in->curBitIndex += bits;
                bits = 0;
            }
            else
            {
                in->currentCodeVal |= (iValue << resBitIndex);
                resBitIndex += 8;
                bits -= 8;
                in->current++;
            }
        }
        else
        {
            mask = ~(~0U << bits);
            in->currentCodeVal |= ((iValue >> in->curBitIndex) & mask) << resBitIndex;
            bitsLeft = 8 - in->curBitIndex;
            if(bits < bitsLeft)
            {
                in->curBitIndex += bits;
                bits = 0;
            }
            else
            {
                in->curBitIndex = 0;
                in->current++;
                bits -= bitsLeft;
                resBitIndex += bitsLeft;
            }
        }
    }
	
    return (BYTE *)&in->currentCodeVal;
}

void			input_free(LZWInput *in) {
	if( in ) {
		if ( in->dataStream ) {
			free( in->dataStream );
		}
		free( in );
	}
}

LZWOutput*	output_new(WriteFunc wf) {
	LZWOutput	*out;

	out = (LZWOutput *)malloc(sizeof(LZWOutput));

	out->write = wf;

	out->bufferSize = sizeof(BYTE) * 1024;
	out->bufferOccupied = 0;
	out->outBuffer = (BYTE *)malloc( out->bufferSize );
	memset(out->outBuffer, 0, out->bufferSize);

	out->activeBitLength = 9;
	out->curBitIndex = 0;

	return out;
}

void		output_push(LZWOutput *out, void *raw) {
	out->write(out, raw);
}

void			writeUncompressedData(LZWOutput *out, const void *raw) {
	char *str;
	int_32 strLen;
	int i;

	str = (char *)raw;
	strLen = strlen(str);

	if ( out->bufferOccupied + strLen >= out->bufferSize ) {
		out->bufferSize = out->bufferSize * 2 + strLen;
		out->outBuffer = (BYTE *)realloc(out->outBuffer, out->bufferSize);

		memset(out->outBuffer + (out->bufferSize >> 1), 0, sizeof(BYTE) * (out->bufferSize >> 1) );
	}
	
	for(i = 0; i < strLen; i++){
		out->outBuffer[out->bufferOccupied + i] = str[strLen - (i+1)];
	}

	out->bufferOccupied += strLen;
	out->outBuffer[out->bufferOccupied] = 0;

}

void			writeCompressedData(LZWOutput *out, const void *raw) {
	/*uint_32	*iterator;
	uint_32 value;
	int_32	bits_left;
	
	if ( ptr->bufferOccupied == ptr->bufferSize ) {
		//printf("Doing it %d %d\n", ptr->bufferSize << 1, ptr->bufferOccupied);
		ptr->bufferSize <<= 1;
		ptr->outBuffer = (BYTE *)realloc(ptr->outBuffer, ptr->bufferSize);
	}
	
	iterator = (uint_32 *)((BYTE *)ptr->outBuffer + ptr->bufferOccupied);
	*iterator = *((uint_32 *)raw);
	ptr->bufferOccupied += 4;*/
	
	BYTE		*iterator;
	uint_32		value;
	uint_32		bits_left;
	BYTE		b;
	uint_32		bitsWritten;
	
	if ( out->bufferOccupied + 8 >= out->bufferSize ) {
		out->bufferSize <<= 1;
		out->outBuffer = (BYTE *)realloc(out->outBuffer, out->bufferSize);

		memset(out->outBuffer + (out->bufferSize >> 1), 0, sizeof(BYTE) * (out->bufferSize >> 1) );
	}

	bits_left = out->activeBitLength;
	value = *((uint_32 *)raw);
	
	while(bits_left > 0)
    {
        if(out->curBitIndex == 0)
        {
			iterator = (BYTE *)out->outBuffer + out->bufferOccupied;
			*iterator = (BYTE)value;
            if(bits_left < 8)
            {
                out->curBitIndex += bits_left;
                bits_left = 0;
            }
            else
            {
                bits_left -= 8;
                value >>= 8;
				out->bufferOccupied++;
            }
        }
        else
        {
            b = (BYTE)(value << out->curBitIndex);
			iterator = (BYTE *)out->outBuffer + out->bufferOccupied;
			*iterator |= b;
            bitsWritten = 8 - out->curBitIndex;
            if(bits_left < bitsWritten)
            {
                out->curBitIndex += bits_left;
                bits_left = 0;
            }
            else
            {
                bits_left -= bitsWritten;
                value >>= bitsWritten;
                out->curBitIndex = 0;
				out->bufferOccupied++;
            }
        }
    }
}

void		output_dump(LZWOutput *out, const char *dst, uint_32 opType) {
	int *iterator;
	int i, max;

	if ( opType == COMPRESSION_ID ) {
		iterator = (int *)out->outBuffer;
		i = 0;
		max = out->bufferOccupied / 4;

		out->fd = fopen(dst,"wb");

		if ( out->fd == NULL )
		{
			printf("Error opening file %s\n",dst);
			exit(1);
		}

		fwrite(out->outBuffer, out->bufferOccupied, 1, out->fd);
	} else {
		out->fd = fopen(dst,"wb");

		if ( out->fd == NULL )
		{
			printf("Error opening file %s\n",dst);
			exit(1);
		}

		fwrite(out->outBuffer, out->bufferOccupied, 1, out->fd);
	}
	
	fclose(out->fd);
}

void		output_check(LZWOutput *out, uint_32 code) {
	if ( ( (1 << out->activeBitLength) - 1 ) == code ) {
		out->activeBitLength++;
	}
}

void		output_free(LZWOutput *out) {
	if ( out->outBuffer )
		free(out->outBuffer);

	free(out);
}
