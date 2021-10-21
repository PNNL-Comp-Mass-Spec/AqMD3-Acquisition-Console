#ifndef UIMF_DECOMPRESSOR_SUBSCRIBER_H
#define UIMF_DECOMPRESSOR_SUBSCRIBER_H

#include "../subscriber.h"
#include <libaqmd3/acquireddata.h>
#include <UIMFWriter/uimfframe.h>

extern "C" {
#include <lzf/lzf.h>
}

#include <string>
#include <fstream>

class UimfFrameDecompressorSubscriber : public Subscriber<std::shared_ptr<UimfFrame>>
{
private:
	std::ofstream out_file_stream;
	std::string out_file;

public:
	UimfFrameDecompressorSubscriber(std::string out_file)
		: out_file_stream(out_file)
		, out_file(out_file)
	{}
	~UimfFrameDecompressorSubscriber()
	{
		out_file_stream.close();
	}

private:
	void on_notify() override;
	void on_completed() override {};
};

static bool check_comp_decomp(EncodedResult& er, std::ofstream& stream)
{
	auto cs = er.get_compressed_spectra();

	size_t out_buf_size = cs.size * 3;
	char *out_buf = new char[out_buf_size];
	unsigned int o_bytes = lzf_decompress(cs.data, cs.size, out_buf, out_buf_size);

	int32_t *rt = const_cast<int32_t *>(er.encoded_spectra.data());
	char *data_ptr = (char *)rt;

	//if (o_bytes != cs.size)
	//{
	//	stream << "data mismatch in size after decompression\n";
	//	stream << "decompressed bytes: " << o_bytes << " pre-compression bytes: " << cs.size << std::endl;
	//}

	bool is_same = true;

	for (int i = 0; i < o_bytes; i++)
	{
		is_same = data_ptr[i] == out_buf[i];
		if (!is_same)
			break;
	}
	
	delete[] out_buf;
	return is_same;
}

void UimfFrameDecompressorSubscriber::on_notify()
{
	if (!out_file_stream.is_open())
		out_file_stream.open(out_file);

	while (!items.empty())
	{
		auto frame = items.front();
		items.pop_front();

		out_file_stream << "frame->file_name: " << frame->file_name << std::endl;
		out_file_stream << "frame->frame_length: " << frame->frame_length << std::endl;
		out_file_stream << "frame->frame_number: " << frame->frame_number << std::endl;
		out_file_stream << "frame->time_created: " << frame->time_created << std::endl;
		out_file_stream << "frame->start_trigger: " << frame->start_trigger << std::endl;
		out_file_stream << "frame->offset_bins: " << frame->offset_bins << std::endl;
		out_file_stream << "frame->nbr_samples: " << frame->nbr_samples << std::endl;

		auto cs_count = frame->get_encoded_results_count();
		out_file_stream << "get_encoded_results_count: " << cs_count << std::endl;

		int c_dc_false = 0;
		auto er = frame->get_data();
		for (const auto& segs : er)
		{
			for (auto& seg : *segs)
			{
				if (!check_comp_decomp(seg, out_file_stream))
					c_dc_false++;
			}
		}

		out_file_stream << "compression/decompression mismatch count: " << c_dc_false << std::endl;
	}
}


#endif // !UIMF_DECOMPRESSOR_SUBSCRIBER_H