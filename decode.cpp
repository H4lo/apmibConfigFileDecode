#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <endian.h>
#include <fcntl.h>

#define N		 4096	/* size of ring buffer */
#define F		   18	/* upper limit for match_length */
#define THRESHOLD	2   /* encode string into position and length if match_length is greater than this */
static unsigned char *text_buf;	/* ring buffer of size N, with extra F-1 bytes to facilitate string comparison */
#define LZSS_TYPE	unsigned short
#define NIL			N	/* index for root of binary search trees */
struct lzss_buffer {
	unsigned char	text_buf[N + F - 1];
	LZSS_TYPE	lson[N + 1];
	LZSS_TYPE	rson[N + 257];
	LZSS_TYPE	dad[N + 1];
};
static LZSS_TYPE		match_position, match_length;  /* of longest match.  These are set by the InsertNode() procedure. */
static LZSS_TYPE		*lson, *rson, *dad;  /* left & right children & parents -- These constitute binary search trees. */


typedef struct compress_mib_header {
	unsigned char signature[6];
	unsigned short compRate;
	unsigned int compLen;
} COMPRESS_MIB_HEADER_T;

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

int Decode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput)	/* Just the reverse of Encode(). */
{
	
	int  i, j, k, r, c;
	unsigned int  flags;
	unsigned int ulPos=0;
	unsigned int ulExpLen=0;

	if ((text_buf = (unsigned char *)malloc( N + F - 1 )) == 0) {
		//fprintf(stderr, "fail to get mem %s:%d\n", __FUNCTION__, __LINE__);
		return 0;
	}
	
	for (i = 0; i < N - F; i++)
		text_buf[i] = ' ';

        
	r = N - F;
	flags = 0;
	while(1) {
		if (((flags >>= 1) & 256) == 0) {
			c = ucInput[ulPos++];
			if (ulPos>inLen)
				break;
			flags = c | 0xff00;		/* uses higher byte cleverly */
		}							/* to count eight */
		if (flags & 1) {
			c = ucInput[ulPos++];
			if ( ulPos > inLen )
				break;
			ucOutput[ulExpLen++] = c;
			text_buf[r++] = c;
			r &= (N - 1);
		} else {
			i = ucInput[ulPos++];
			if ( ulPos > inLen ) break;
			j = ucInput[ulPos++];
			if ( ulPos > inLen ) break;
			
			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + THRESHOLD;
			for (k = 0; k <= j; k++) {
				c = text_buf[(i + k) & (N - 1)];
				ucOutput[ulExpLen++] = c;
				text_buf[r++] = c;
				r &= (N - 1);
			}
		}
	}

	free(text_buf);
	return ulExpLen;

}


int main(int argc,char **argv,char **env){
	if(argc<3){
		puts("[+] Usage: ./decode_apmib_config INPUT_FILE OUTPUT_FILE");
		exit(0);
	}

	struct stat sb;
	char *read_buf = (char *)calloc(1,0x100000);
	char *expFile = (char *)calloc(1,0x100000);

	int infile = open(argv[1],O_RDONLY);
	int outfile = open(argv[2],O_CREAT|O_RDWR,S_IRWXU);
	if(infile == -1 || outfile == -1){
		fprintf(stderr,"read file error!");
		exit(-1);
	}
		
	fstat(infile,&sb);
	read(infile,read_buf,sb.st_size);

	//printf("Input file size: %ld\n",sb.st_size);
	Decode((char *)((char *)read_buf+0xc),sb.st_size,expFile);
	
	write(outfile,expFile,0x4000);

	char *buf = malloc(0x200);
	sprintf(buf,"TMP=`strings %s | head -n +4 | tail -n1`;python -c \"t='$TMP'.replace('@','');print(t);\"",argv[2]);
	system(buf);

	free(read_buf);	
	free(expFile);
	free(buf);
	
	return 0;
}
