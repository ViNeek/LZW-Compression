#include <stdio.h>
#include <stdlib.h>

#include "LZWIOHandler.h"
#include "LZWEnDictionary.h"
#include "LZWDecDictionary.h"

#ifdef _WIN32

double PCFreq = 0.0;
__int64 CounterStart = 0;

//Timer Functions for Win32
//For linux use time command
void StartCounter()
{
    LARGE_INTEGER li;

    if(!QueryPerformanceFrequency(&li))
		printf("QueryPerformanceFrequency failed!\n");

    PCFreq = (double)(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}

double GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);

	return (double)(li.QuadPart-CounterStart)/PCFreq;
}

#endif

static void show_usage();
static BOOL check_cmd(int argc, char **argv);
static void lzw_compress(const char *src, const char *dst);
static void lzw_decompress(const char *src, const char *dst);

int main(int argc, char **argv) {
	const char *		srcFile;
	const char *		dstFile;
	int_32				opID;

//Win32 DEBUG Init
#if defined _WIN32 && defined _DEBUG
	_CrtSetDbgFlag (
            _CRTDBG_ALLOC_MEM_DF |
            _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode (
			_CRT_ERROR,
            _CRTDBG_MODE_DEBUG);
#endif

	//Check command syntax
	if ( !check_cmd(argc, argv ) )
		return 1;

	//Compress or Decompress
	if ( strcmp(argv[1],"-c") == 0 ) {
		opID = COMPRESSION_ID;
	} else
		opID = DECOMPRESSION_ID;

	//Input File
	srcFile = argv[3];
	//Output File
	dstFile = argv[5];

	if (opID == COMPRESSION_ID )
		lzw_compress(srcFile, dstFile);
	else
		lzw_decompress(srcFile, dstFile);

//Win32 Debug Output
#if defined _WIN32 && defined _DEBUG
	//_CrtDumpMemoryLeaks();
	system("PAUSE");
#endif

}

static void lzw_compress(const char *src, const char *dst) {
	EnDictionary		*eDict;
	LZWInput			*in;
	LZWOutput			*out;
	ulong_32			pindex, poffset, pcode;
	ulong_32			count;
	int_32				i;
	BYTE				*current;
	SeqKey				ckey;
	BYTE				alpha[128];
	ulong_32			init_indexes[128];
	ulong_32			init_offsets[128];

	//Initialize Data Structures for compression
	eDict = enDict_new(26, MurmurHash3);
	in = input_new(src, readUncompressedData);
	out = output_new(writeCompressedData);

#ifdef _WIN32
	StartCounter();
#endif
	count = 1;

	for(i = 0; i < 128; i++) {
		alpha[i] = i;
	}

	for(i = 0; i < 128; i++) {
		ckey.data.prevIndex = -1;
		ckey.data.prevOffset = -1;
		ckey.data.sym = alpha[i];

		enDict_insert(eDict, &ckey, count++, &init_indexes[i], &init_offsets[i]);
	}

	current = input_pull(in);
	ckey.data.prevIndex = init_indexes[*current];
	ckey.data.prevOffset = init_offsets[*current];
	pcode = (*current) + 1;

	while ( !input_end( in ) ) {
		current = input_pull(in);
		ckey.data.sym = *current;
		if ( enDict_search(eDict, &ckey, &pcode, &pindex, &poffset) ) {
			ckey.data.prevIndex = pindex;
			ckey.data.prevOffset = poffset;
		} else {
			output_check(out, enDict_size(eDict));
			output_push(out, &pcode);
			enDict_insert_at(eDict, pindex, &ckey, count, &poffset);
			count++;
			ckey.data.prevIndex = init_indexes[*current];
			ckey.data.prevOffset = init_offsets[*current];
			pcode = (*current) + 1;
		}
	}

	output_push(out, &pcode);
	output_dump(out, dst, COMPRESSION_ID);

#ifdef _WIN32
	printf("Compressed in %.3f secs\n",GetCounter()/1000);
#endif

	/*For debugging perposes only
	* The OS is going to clean up as soon as we are done
	* No need to bother
	*/
	//enDict_free(eDict);
	//input_free(in);
	//output_free(out);
}

static void lzw_decompress(const char *src, const char *dst) {
	DecDictionary		*dDict;
	LZWInput			*in;
	LZWOutput			*out;
	ulong_32			pindex, pcode;
	ulong_32			count, code, tempCode;
	int_32				i;
	BYTE				sym;
	BYTE				*current;

	//Initialize Data Structures for decompression
	dDict = decDict_new(512);
	in = input_new(src, readCompressedData);
	out = output_new(writeUncompressedData);

#ifdef _WIN32
	StartCounter();
#endif

	count = 129;
	for(i = 0; i < 128; i++)
		decDict_insert(dDict, 0, i);

	if ( !input_end( in ) )
		code = *((uint_32 *)input_pull(in));

	output_push(out, decDict_get_at(dDict, code));

	pcode = code;
	while ( !input_end( in ) ) {
		input_check(in, count);
		count++;
		code = *((uint_32 *)input_pull(in));
		if ( decDict_contains(dDict, code, &pindex, &sym ) ) {
			current = decDict_get_at(dDict, code);
			output_push(out, current);
			decDict_insert(dDict, pcode, current[strlen((const char *)current) - 1]);
		} else {
			current = decDict_get_at(dDict, pcode);
			tempCode = decDict_insert(dDict, pcode, current[strlen((const char *)current) - 1]);
			output_push(out, decDict_get_at(dDict, tempCode));
		}
		pcode = code;
	}

	output_dump(out, dst, DECOMPRESSION_ID);

#ifdef _WIN32
	printf("Decompressed in %.3f secs\n",GetCounter()/1000);
#endif

	/*For debugging perposes only
	* The OS is going to clean up as soon as we are done
	* No need to bother
	*/
	//decDict_free(dDict);
	//input_free(in);
	//output_free(out);
}

static BOOL check_cmd(int argc, char **argv) {

	if ( argc != 6 ) {
		show_usage();
		system("PAUSE");
		return FALSE;
	}

	if ( strcmp(argv[1],"-d") && strcmp(argv[1],"-c")) {
		show_usage();
		system("PAUSE");
		return FALSE;
	} else if ( strcmp(argv[2],"-i") ) {
		show_usage();
		system("PAUSE");
		return FALSE;
	} else if ( strcmp(argv[4],"-o") ) {
		show_usage();
		system("PAUSE");
		return FALSE;
	}

	return TRUE;
}

static void show_usage() {
	printf("Usage: lzw -[c|d] -i InputFile -o OutputFile\n\n");
	printf("Options:\n\t-c\t\tCompress\n\t-d\t\tDecompress\n");
}
