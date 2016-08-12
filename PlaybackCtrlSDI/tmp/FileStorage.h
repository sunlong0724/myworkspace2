#ifndef __SEND_DATA_IMPL_H__
#define __SEND_DATA_IMPL_H__

#include "FPSCounter.h"
#include "MyThread.h"

class CFileStorage : public CMyThread{
public:
	CFileStorage();
	~CFileStorage();
	static void set_file_name(const std::string& file_name, const int64_t max_raw_file_size);
	int64_t  write_file(char* buffer, int64_t buffer_len);
	int64_t read_file(char* buffer, int64_t buffer_len, int64_t frame_no);
protected:
	void run();
private:
	//timestamp(int64_t) frame_no(int64_t) buffer
	int64_t  do_write_file(char* buffer, int64_t buffer_len);

private:

	HANDLE						m_reader_handle;
	HANDLE						m_writter_handle;

	
	uint64_t					m_written_bytes;
	BOOL						m_has_reach_max_file_size;
	uint64_t					m_frame_counter;
public:
	static std::map<int64_t, int64_t>   m_frame_offset_map;//<frame_no,frame offset in file>
	static std::string			m_file_name;
	static long long			m_max_file_size;

	std::deque<std::vector<char>> m_h264_buffers;
	CFPSCounter					m_fps_counter;
};


#endif