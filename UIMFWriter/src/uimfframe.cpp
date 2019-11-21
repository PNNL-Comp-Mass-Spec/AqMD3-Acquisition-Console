#include "..\include\UIMFWriter\uimfframe.h"

#include <iostream>

void UimfFrame::append_encoded_results(std::shared_ptr<std::vector<EncodedResult>> results)
{
	data.push_back(results);
}

std::shared_ptr<std::vector<EncodedResult>> UimfFrame::append_and_return_excess(const std::shared_ptr<std::vector<EncodedResult>> results)
{
	auto count = get_encoded_result_count();
	auto diff = frame_length - count;

	if (results->size() <= diff)
	{
		data.push_back(results);

		return std::make_shared<std::vector<EncodedResult>>();
	}
	else if (diff < results->size() && diff != 0)
	{
		auto item_count_to_return = results->size() - diff;
		auto to_return = std::make_shared<std::vector<EncodedResult>>();
		auto to_add = std::make_shared<std::vector<EncodedResult>>();

		to_return->reserve(item_count_to_return);
		to_return->reserve(diff);
		std::copy(std::begin(*results), std::begin(*results) + diff, std::back_inserter(*to_add));
		std::copy(std::begin(*results) + diff, std::end(*results), std::back_inserter(*to_return));
		data.push_back(to_add);
		
		return to_return;
	}	

	return results;
}

int UimfFrame::get_encoded_result_count() const
{
	int count = 0;

	for (const auto& datum : data)
	{
		count += datum->size();
	}

	return count;
}

double UimfFrame::get_frame_duration_seconds(double ts_sample_period_s) const
{
	if (data.empty())
		return 0.0;

	if (data.size() == 1)
		return ((data[0])->back().timestamp - (data[0])->front().timestamp) * ts_sample_period_s;

	auto ts_first = (data.front())->front().timestamp;
	auto ts_last = (data.back())->back().timestamp;

	return (ts_last - ts_first) * ts_sample_period_s;
}

