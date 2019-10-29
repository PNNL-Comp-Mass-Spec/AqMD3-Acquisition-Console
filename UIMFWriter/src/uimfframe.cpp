#include "..\include\UIMFWriter\uimfframe.h"

void UimfFrame::append_encoded_results(std::shared_ptr<std::vector<EncodedResult>> results)
{
	data.push_back(results);
}
