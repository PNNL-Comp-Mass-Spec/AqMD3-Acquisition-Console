#ifndef ENCODED_RESULT_H
#define ENCODED_RESULT_H

#include <vector>

class EncodedResult {
public:
	uint64_t const timestamp;

	std::vector<int32_t> const encoded_spectra;
	int32_t const non_zero_count;
	int32_t const scan;
	int64_t const tic;
	int64_t const bpi;
	double const bpi_mz;
	int32_t const index_max_intensity;

	struct CompressedSpectra {
		char const *data;
		size_t const size;

		CompressedSpectra(char *data, size_t size)
			: data(data)
			, size(size)
		{}
		~CompressedSpectra()
		{
			delete[] data;
		}
	};

public:
	EncodedResult(int32_t scan, int32_t non_zero_count, std::vector<int32_t> encoded_spectra, int64_t tic, int64_t bpi, double bpi_mz, int32_t index_max_intensity, uint64_t timestamp)
	: encoded_spectra(std::move(encoded_spectra))
		, scan(scan)
		, non_zero_count(non_zero_count)
		, tic(tic)
		, bpi(bpi)
		, bpi_mz(bpi_mz)
		, index_max_intensity(index_max_intensity)
		, timestamp(timestamp)
	{}

	CompressedSpectra get_compressed_spectra() const;
};

#endif // !ENCODED_RESULT_H