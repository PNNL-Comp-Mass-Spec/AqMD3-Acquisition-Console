#include "../include/UIMFWriter/encodedresult.h"

EncodedResult::ScanData EncodedResult::get_characteristics() const
{
	int64_t tic = 0;
	int64_t bpi = 0;
	double bpi_mz = 0.0;
	int32_t index_max_intensity = 0;

	int32_t offset = 0;
	for (int i = 0; i < encoded_spectra.size(); i++)
	{
		int32_t data = encoded_spectra[i];

		if (data < 0)
		{
			offset += data * -1;
			continue;
		}

		tic += data;
		if (data > bpi)
		{
			bpi = data;
			index_max_intensity = offset;

			bpi_mz = offset; // ?
		}

		offset += 1;
	}

	return EncodedResult::ScanData(tic, bpi, bpi_mz, index_max_intensity);
}