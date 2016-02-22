
#if DEBUG
#define test fprintf(stdout, "[TEST %d]\n", __LINE__);
#define debugm(M) fprintf(	stdout, "\n\n[DEBUG %d] %s\n\n", __LINE__, M);
#define debug(M) fprintf(	stdout, "[DEBUG %d] %s: %x %d\n", \
							__LINE__, #M, \
							(unsigned int)M, (unsigned int)M); \
							fflush(stdout);
#define debugs(M) fprintf(stdout, "[DEBUG %d] %s: %s\n", __LINE__, #M, M); \
							fflush(stdout);
#define debugl(M) fprintf(stdout, "[DEBUG %d] %s: %lld\n", __LINE__, #M, M); \
							fflush(stdout);
#define debugc(M) fprintf(stdout, "[DEBUG %d] %s: %c %x\n", \
							__LINE__, #M, M, M); \
							fflush(stdout);
#define debugp(M) fprintf(stdout, "[DEBUG %d] %s: %p\n", \
							__LINE__, #M, M); \
							fflush(stdout);
#define BYTETOBINARYFPRINT "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte) \
	(byte & 0x80 ? 1 : 0), \
	(byte & 0x40 ? 1 : 0), \
	(byte & 0x20 ? 1 : 0), \
	(byte & 0x10 ? 1 : 0), \
	(byte & 0x08 ? 1 : 0), \
	(byte & 0x04 ? 1 : 0), \
	(byte & 0x02 ? 1 : 0), \
	(byte & 0x01 ? 1 : 0)
#define debugb(M) fprintf(stdout, \
	"[DEBUG %d] %s: "BYTETOBINARYFPRINT" "BYTETOBINARYFPRINT"\n", \
	__LINE__, #M, BYTETOBINARY(*(((char*)M)+1)), BYTETOBINARY(*((char*)M))); \
	fflush(stdout);
#else
#define test
#define debugm(M)
#define debug(M)
#define debugs(M)
#define debugc(M)
#define debugp(M)
#define debugb(M)
#define debugl(M)
#endif

