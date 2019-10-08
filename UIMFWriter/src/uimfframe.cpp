#include "..\include\UIMFWriter\uimfframe.h"

void UimfFrame::append_encoded_results(std::vector<EncodedResult> results)
{
	std::move(std::begin(results), std::end(results), std::back_inserter(data));
}