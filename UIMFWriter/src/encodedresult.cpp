#include "../include/UIMFWriter/encodedresult.h"

#include <stdexcept>

extern "C" {
#include "../include/lzf/lzf.h"
}

EncodedResult::CompressedSpectra EncodedResult::get_compressed_spectra() const
{
	int encoded_size = encoded_spectra.size() * sizeof(int32_t);
	int processing_buffer_size = encoded_size * 2;
	char *lzf_buffer = new char[processing_buffer_size];

	unsigned int bytes = lzf_compress(encoded_spectra.data(), encoded_size, lzf_buffer, processing_buffer_size);
	lzf_buffer[bytes] = '\0';

#ifdef check_compress
	char *out_buf = new char[4096];
	unsigned int o_bytes = lzf_decompress(lzf_buffer, processing_buffer_size, out_buf, 4096);

	int32_t *rt = const_cast<int32_t *>(encoded_spectra.data());
	char *data_ptr = (char *)rt;

	assert(o_bytes == encoded_size);

	for (int i = 0; i < bytes; i++)
	{
		if (data_ptr[i] != out_buf[i])
		{
			throw std::runtime_error("decompress fail");
		}
	}
#endif

	return CompressedSpectra(lzf_buffer, bytes);
}
