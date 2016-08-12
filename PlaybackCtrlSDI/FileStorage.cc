
#include "defs.h"
#include "utils.h"
#include "MyThread.h"
#include "FileStorage.h"

CFileStorage::CFileStorage() : m_writter_handle(NULL), m_reader_handle(NULL), m_written_bytes(0), m_has_reach_max_file_size(FALSE) {
}

CFileStorage::~CFileStorage() {
	CloseHandle(m_writter_handle);
	CloseHandle(m_reader_handle);
}

void CFileStorage::set_file_name(const std::string& file_name, const int64_t max_raw_file_size) {
	m_file_name.append(file_name);
	m_max_file_size = max_raw_file_size;
}

int64_t CFileStorage::write_file(char* buffer, int64_t buffer_len) {
	int64_t ret = -1;
	int64_t timestamp = 0;
	int64_t frame_counter = 0;
	memcpy(&frame_counter, &buffer[FRAME_SEQ_START], sizeof int64_t);
	memcpy(&timestamp, &buffer[FRAME_TIMESTAMP_START], sizeof int64_t);

	if (timestamp == 0x00) {
		fprintf(stdout, "%s Insert a lost frame %d\n", __FUNCTION__, frame_counter);
		m_frame_offset_map.insert(std::make_pair(frame_counter, -1));
		ret = 0;
	}
	else {
		ret = RingBuffer_write(m_written_queue, buffer, buffer_len);
		if (-1 == ret) {
			fprintf(stdout, "%s RingBuffer full,insert a fake frame %d\n", __FUNCTION__, frame_counter);
		}
		m_frame_offset_map.insert(std::make_pair(frame_counter, -2));
	}

	if (m_has_reach_max_file_size) {
		//fprintf(stdout, "%s delete %d\n", __FUNCTION__, m_frame_offset_map.begin()->first);
		m_frame_offset_map.erase(m_frame_offset_map.begin());
	}
	
	return ret;
}


void CFileStorage::run() {
	m_written_queue = RingBuffer_create(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h) * MAX_FPS * MAX_CACHE_IMAGE_TIME);
	std::vector<char> m_swap_buffer(GET_IMAGE_BUFFER_SIZE(g_cs.m_image_w, g_cs.m_image_h), 0x00);

	DWORD dw = 0;
	if (m_writter_handle == NULL || m_writter_handle == INVALID_HANDLE_VALUE) {
		//check sector size
		m_writter_handle = CreateFile(m_file_name.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);//aligin??
		if (m_writter_handle == INVALID_HANDLE_VALUE) {
			dw = GetLastError();
			fprintf(stdout, "CreateFile failed, errno:%lu!!!\n", dw);
			exit(-1);
		}
	}

	while (!m_exited) {
		if (-1 == RingBuffer_read(m_written_queue, m_swap_buffer.data(), m_swap_buffer.size())) {
			sleep(1);
			continue;
		}
		int ret = do_write_file(m_swap_buffer.data(), m_swap_buffer.size());
		if (m_swap_buffer.size() != ret) {
			fprintf(stdout, "%s do_write_file failed! ret %d\n", __FUNCTION__, ret);
		}
	}

	RingBuffer_destroy(m_written_queue);
}

//timestamp(int64_t) frame_no(int64_t) buffer
int64_t  CFileStorage::do_write_file(char* buffer, int64_t buffer_len) {
	DWORD dw = 0;

	DWORD nBytesWritten = 0;
	int64_t timestamp = 0;
	int64_t frame_counter = 0;
	memcpy(&frame_counter, &buffer[FRAME_SEQ_START], sizeof int64_t);
	memcpy(&timestamp, &buffer[FRAME_TIMESTAMP_START], sizeof int64_t);

	BOOL ret = FALSE;
	ret = WriteFile(m_writter_handle, buffer + 16, buffer_len - 16, &nBytesWritten, NULL);
	if (FALSE == ret) {
		dw = GetLastError();
		fprintf(stdout, "WriteFile failed, errno:%lu, frame_counter:%llu!!!\n", dw, frame_counter);
		//m_frame_offset_map.insert(std::make_pair(frame_counter, -2));
		return -2;
	}

	//fprintf(stdout, "WriteFile success, frame_counter:%llu!!!\n", frame_counter);
	m_frame_offset_map[frame_counter] = m_written_bytes;
	m_written_bytes += nBytesWritten;
	m_fps_counter.statistics(__FUNCTION__, FALSE);

	//printf("write_file %d bytes, seq:%lld\n", nBytesWritten, frame_counter);
	if (m_written_bytes >= m_max_file_size) {//FIXME: maybe has a bug
		m_has_reach_max_file_size = TRUE;
		SetFilePointer(m_writter_handle, 0, NULL, FILE_BEGIN);
		m_written_bytes = 0;
	}
	return buffer_len;
}

int64_t CFileStorage::read_file(char* buffer, int64_t buffer_len, int64_t frame_no) {
	DWORD dw = 0;
	if (INVALID_HANDLE_VALUE == m_reader_handle || NULL == m_reader_handle) {
		m_reader_handle = CreateFile(m_file_name.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
		if (m_reader_handle == INVALID_HANDLE_VALUE) {
			dw = GetLastError();
			fprintf(stderr, "fopen failed, errno:%lu!!!\n", dw);
			return -1;
		}
	}

	if (m_frame_offset_map.find(frame_no) == m_frame_offset_map.end()) {//The frame was overwritten!!!
		//fprintf(stderr, "Frame %lld was not found!!!\n", frame_no);
		fprintf(stdout, "%s map size(%lld),beg(seq:%lld,offset:%lld),end(seq:%lld,offset:%lld),Frame %lld was not found\n", __FUNCTION__,\
			m_frame_offset_map.size(), m_frame_offset_map.begin()->first, m_frame_offset_map.begin()->second, 
			m_frame_offset_map.rbegin()->first, m_frame_offset_map.rbegin()->second, frame_no);
		return -2;
	}
	else {
		int64_t	frame_offset = m_frame_offset_map[frame_no];//The frame was lost!!!
		if (frame_offset < 0) {//this frame is lost!!!
			return 0;
		}
		//fprintf(stdout, "%s frame_no:%lld, frame_offset:%lld\n", __FUNCTION__,frame_no, frame_offset);
		LARGE_INTEGER offset;
		offset.QuadPart = frame_offset;
		SetFilePointerEx(m_reader_handle, offset, NULL, FILE_BEGIN);
	}
	DWORD nBytesRead = 0;
	//BOOL ret = ReadFile(m_reader_handle, &buffer[sizeof int64_t + sizeof int64_t], buffer_len - sizeof int64_t - sizeof int64_t, &nBytesRead, NULL);
	BOOL ret = ReadFile(m_reader_handle, buffer + 16, buffer_len - 16, &nBytesRead, NULL);
	if (FALSE == ret) {
		dw = GetLastError();
		fprintf(stdout, "ReadFile failed, errno:%lu!!!\n", dw);
	}

	memcpy(&buffer[FRAME_SEQ_START], &frame_no, sizeof frame_no);
	return nBytesRead;
}

