#ifndef COMPRESSION_PPMD_STREAM_H
#define COMPRESSION_PPMD_STREAM_H

namespace compression { namespace ppmd  { 

class stream {
private:
	size_t				m_buffer_size;
	u8				*m_buffer;
	u8				*m_pointer;

public:
	inline			stream		(const void *buffer, const size_t &buffer_size);
	inline	void	put_char	(const u8 &object);
	inline	int		get_char	();
	inline	void	rewind		();
	inline	u8		*buffer		() const;
	inline	u32		tell		() const;
};

} // namespace ppmd
} // namespace compression

#include "compression_ppmd_stream_inline.h"

#endif // COMPRESSION_PPMD_STREAM_H