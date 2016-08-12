#ifndef __DEFS__H__
#define __DEFS__H__

#define FRAME_TIMESTAMP_START  (0)
#define FRAME_SEQ_START   (sizeof int64_t)
#define FRAME_DATA_START (sizeof int64_t+sizeof int64_t)

#define GET_IMAGE_BUFFER_SIZE(w,h)  0 //  (sizeof int64_t + sizeof int64_t + (w) * (h))

#define ONE_MB	(1024*1024)
#define ONE_GB  (1024 * ONE_MB)


#define PRINT_FRAME_INFO(A) 	do { \
				break;\
				int64_t seq = 0;\
				int64_t timestamp = 0;\
				memcpy(&seq, &A[FRAME_SEQ_START], sizeof seq);\
				memcpy(&timestamp, &A[FRAME_TIMESTAMP_START], sizeof timestamp);\
				fprintf(stdout, "%s seq:%lld,timestamp:%lld\n", __FUNCTION__, seq, timestamp);\
				}while(0)


#define MAX_IMAGE_WIDTH  1920
#define MAX_IMAGE_HEIGHT 1080
#define MAX_FPS			 25

#endif // !__DEFS__H_
