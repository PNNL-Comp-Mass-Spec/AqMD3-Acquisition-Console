#ifndef ENCODED_RESULT_H
#define ENCODED_RESULT_H

class EncodedResult {
public:
	std::vector<int32_t> const encoded_spectra;
	int32_t non_zero_count;
	int32_t scan;
	int32_t frame;

	struct ScanData {
		int64_t const tic;
		int64_t const bpi;
		double const bpi_mz;
		int32_t const index_max_intensity;

		ScanData(int64_t tic, int64_t bpi, double bpi_mz, int32_t index_max_intensity)
			: tic(tic)
			, bpi(bpi)
			, bpi_mz(bpi_mz)
			, index_max_intensity(index_max_intensity)
		{}
	};

	EncodedResult(int32_t frame, int32_t scan, int32_t non_zero_count, std::vector<int32_t> encoded_spectra)
		: encoded_spectra(encoded_spectra)
		, frame(frame)
		, scan(scan)
		, non_zero_count(non_zero_count)
	{}

	ScanData get_characteristics() const;
};

#endif // !ENCODED_RESULT_H