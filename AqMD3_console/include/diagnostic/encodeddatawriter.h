#ifndef ENCODED_DATA_WRITER_H
#define ENCODED_DATA_WRITER_H

#include "../framesubscriber.h"
#include "../acquireddata.h"
#include <UIMFWriter/encodedresult.h>

#include <string>
#include <fstream>
#include <vector>

class EncodedDataWriter : public FrameSubscriber<std::shared_ptr<std::vector<EncodedResult>>>
{
private:
	std::ofstream out_file_stream;
	std::string out_file;

public:
	EncodedDataWriter(std::string out_file)
		: out_file_stream(out_file)
		, out_file(out_file)
	{}
	~EncodedDataWriter()
	{
		out_file_stream.close();
	}

private:
	void on_notify() override;
	void on_completed() override
	{
		std::cout << "ENCODED DATA WRITER EXITING" << std::endl;
	}
};

void EncodedDataWriter::on_notify()
{
	if (!out_file_stream.is_open())
		out_file_stream.open(out_file);

	while (!items.empty())
	{
		auto ad = items.front();
		items.pop_front();

		for (int i = 0; i < ad->size(); i++)
		{
			//out_file_stream << "FRAME " << ad->at(i).frame << std::endl;
			out_file_stream << "NON-ZERO COUNT " << ad->at(i).non_zero_count << std::endl;
			out_file_stream << "DATA [" << std::endl;
			for (int j = 0; j < ad->at(i).encoded_spectra.size(); j++)
			{
				out_file_stream << ad->at(i).encoded_spectra[j] << ", ";
			}
			out_file_stream << "]"<< std::endl;
			out_file_stream << endl;
		}

	}
}

#endif
