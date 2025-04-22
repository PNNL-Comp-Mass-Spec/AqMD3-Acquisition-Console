#ifndef UIMF_ACQUISITION_RECORD
#define UIMF_ACQUISITION_RECORD

#include <libaqmd3/acquireddata.h>
#include <UIMFWriter/encodedresult.h>
#include <UIMFWriter/uimfframe.h>
#include <UIMFWriter/uimfframeparameters.h>

class UimfAcquisitionRecord
{
	UimfFrameParameters const parameters;
	AcquiredData const acquiredData;
	uint64_t const start_scan_index;

public:
	UimfAcquisitionRecord(UimfFrameParameters parameters, AcquiredData acquiredData, uint64_t start_scan_index)
		: parameters(parameters)
		, acquiredData(acquiredData)
		, start_scan_index(start_scan_index)
	{}

private:
	// Processes the raw sample data and returns a vector of processed results equivalent to a row of scan data in a .uimf file. 
	// Should not be called if the data was acquired without data-reduction enabled.
	std::vector<EncodedResult> process() const;

public:
	std::shared_ptr<UimfFrame> to_frame() const;
};
#endif !UIMF_ACQUISITION_RECORD