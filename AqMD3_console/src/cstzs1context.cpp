#include "../include/cstzs1context.h"

#include <vector>
#include <tuple>
#include <exception>
#include <fstream>
#include <iostream>
using std::cout;
using std::cerr;

//#define bad_gates
//#define print_acq
//#define variable_m_t_a

AcquiredData CstZm1Context::acquire(std::chrono::milliseconds timeoutMs)
{
	int markers_to_acquire = min_target_records;
	int active_multiplier = 1;

	int trig_count = 0;
	int gate_count = 0;
	uint64_t to_acquire = 0;
	std::vector<AcquiredData::TriggerData> stamps;

	std::shared_ptr<AcquisitionBuffer> samples_buffer = get_buffer();

#ifdef print_acq
	std::cout << "\nSTART ACQUIRE\n";
#endif

	ViInt64 first_element_markers;
	ViInt64 available_elements_markers = 0;
	ViInt64 actual_elements_markers = markers_buffer.get_unprocessed();

	// sanity check
	if (actual_elements_markers % 16 != 0)
		throw std::string("actual_elements_markers % 16 != 0 at start of acquire");

	auto finish = std::chrono::high_resolution_clock::now() + timeoutMs;
	while (trig_count <= triggers_per_read)
	{

#ifdef print_acq
		auto t1_process = std::chrono::high_resolution_clock::now();
#endif

		for (int i = 0; i < actual_elements_markers / 16; i++)
		{
			int32_t *seg = markers_buffer.get_raw_unprocessed();
			uint32_t header = seg[0];

			switch (header & 0x000000FF)
			{
			case 0x01:
			{
				++trig_count;
				if (trig_count >= triggers_per_read + 1)
					goto process;

				uint64_t low = seg[1];
				uint64_t high = seg[2];
				uint64_t timestampLow = (low >> 8) & 0x0000000000ffffffL;
				uint64_t timestampHigh = uint64_t(high) << 24;
				stamps.emplace_back(timestampHigh | timestampLow, header >> 8, (-1 * (seg[1] & 0x000000ff))/256);
				markers_buffer.advance_processed(16);

				break;
			}
			case 0x04:
			case 0x0a:
			{
				int block_total = 0;
				for (int i = 0; i < 4; i++)
				{
					int32_t *l_ptr = seg + (i * 4);
					if ((*l_ptr & 0x000000FF) == 0x04)
					{
						uint32_t s_lo = l_ptr[0];
						uint32_t s_hi = l_ptr[1];
						uint32_t e_lo = l_ptr[2];
						uint32_t e_hi = l_ptr[3];

						uint64_t start_block_index = (uint64_t(s_hi & 0xffffff) << 8) | ((s_lo >> 24) & 0xff);
						uint32_t start_block_sample_indx = (s_hi >> 24) & 0xff;
						uint64_t end_block_index = (uint64_t(e_hi & 0xffffff) << 8) | ((e_lo >> 24) & 0xff);
						uint32_t end_block_sample_indx = (e_hi >> 24) & 0xff;

						uint64_t to_acquire_memory_blocks_f = (end_block_index - start_block_index) * 4;

#ifdef bad_gates
						if (to_acquire_memory_blocks_f == 4)
						{
							cerr << "gate block difference == 1\n";
							auto samp_b = (start_block_index - 1) * 8 + start_block_sample_indx;
							auto samp_e = (end_block_index - 1) * 8 - (8 - end_block_sample_indx);

							print = true;
							if (samp_e <= samp_b)
							{
								cerr << "stop index <= start index -> stop: " << samp_e << " start: " << samp_b << endl;
								//continue;
							}
						}
#endif

						if (to_acquire_memory_blocks_f % 16 != 0)
							to_acquire_memory_blocks_f = ((to_acquire_memory_blocks_f / 16) + 1) * 16;

						if (stamps.size() == 0)
						{
							std::cerr << "\tNo elements in stamps - discarding acquired elements." << std::endl;
							markers_buffer.advance_processed(16);
							break;
						}

						stamps.back().gate_data.emplace_back(
							start_block_index,
							start_block_sample_indx,
							end_block_index,
							end_block_sample_indx,
							to_acquire_memory_blocks_f);

						++gate_count;

						to_acquire += to_acquire_memory_blocks_f;
					}
					else if ((*l_ptr & 0x000000FF) == 0x0a)
					{
						uint32_t r_lo = l_ptr[0];
						uint32_t r_hi = l_ptr[1];

						uint64_t record_block_index = (uint64_t(r_hi & 0xffffff) << 8) | ((r_lo >> 24) & 0xff);
						uint32_t record_block_sample_indx = (r_hi >> 24) & 0xff;
					}
				}

				markers_buffer.advance_processed(16);
				break;
			}
			case 0x08:	// Should never get here
			{
				std::cerr << "\tERROR -- DUMMY GATE\n";
				throw std::string("dummy gate error");
			}
			default:
				std::cerr << "\t(default) header: " << (header & 0x000000FF) << std::endl;
				throw std::string("unexpected header");
				break;
			}
		}

#ifdef print_acq
		auto t2_process = std::chrono::high_resolution_clock::now();
		auto dur_process = std::chrono::duration_cast<std::chrono::milliseconds>(t2_process - t1_process);
		std::cout << "\tTIME TO PROCESS MARKERS: " << dur_process.count() << "\n";
#endif

		if (trig_count <= triggers_per_read)
		{
			markers_buffer.reset();
#ifdef variable_m_t_a
			int next_markers_to_acquire = markers_to_acquire;
#endif		
#ifdef print_acq
			auto t1_markers = std::chrono::high_resolution_clock::now();
#endif
			do
			{
				first_element_markers = 0;
				available_elements_markers = 0;
				actual_elements_markers = 0;

#ifdef variable_m_t_a	
				markers_to_acquire = next_markers_to_acquire;
#endif	
#ifdef print_acq
				auto t1_loop_markers = std::chrono::high_resolution_clock::now();
#endif

				AqMD3_StreamFetchDataInt32(
					session,
					markers_channel.c_str(),
					markers_to_acquire,
					markers_buffer.get_size(),
					(ViInt32 *)markers_buffer.get_raw_unaquired(),
					&available_elements_markers, &actual_elements_markers, &first_element_markers);

#ifdef print_acq
				if (actual_elements_markers != 0 && actual_elements_markers < markers_to_acquire)
				{
					std::cout << "\tMARKERS: actual_elements_markers != 0 && actual_elements_markers < markers_to_acquire\n";
					std::cout << "\tFIRST ACQ: " << actual_elements_markers << " offset: "  << first_element_markers <<"\n";
				}

				auto t2_loop_markers = std::chrono::high_resolution_clock::now();
				auto dur_loop_markers = std::chrono::duration_cast<std::chrono::milliseconds>(t2_loop_markers - t1_loop_markers);
				std::cout << "\tTIME TO LOOP ACQUIRE MARKERS: " << dur_loop_markers.count() << "\n";
#endif

				if (std::chrono::high_resolution_clock::now() > finish)
				{
					std::cerr << "timeout conditions:\n\n";
					std::cerr << "\tmarkers_buffer.get_size(): " << markers_buffer.get_size() << "\n";
					std::cerr << "\tmarkers_buffer.get_acquired(): " << markers_buffer.get_acquired() << "\n";

					std::cerr << "\tmarkers_to_acquire: " << markers_to_acquire << "\n";
					std::cerr << "\tavailable_elements_markers: " << available_elements_markers << "\n";
					std::cerr << "\tactual_elements_markers: " << actual_elements_markers << "\n\n";

					std::cerr << "\ttrig_count: " << trig_count << "\n";

					throw std::string("timeout in acquisition, processing");
				}

#ifdef variable_m_t_a
				if (available_elements_markers < markers_to_acquire && active_multiplier > multiplier_min)
				{
					next_markers_to_acquire = min_target_records * --active_multiplier;
				}
#endif
			} while (actual_elements_markers == 0);

#ifdef print_acq
			auto t2_markers = std::chrono::high_resolution_clock::now();
			auto dur_markers = std::chrono::duration_cast<std::chrono::milliseconds>(t2_markers - t1_markers);
			std::cout << "\tTIME TO ACQUIRE MARKERS: " << dur_markers.count() << "\n";
			
			std::cout
				<< "markers_to_acquire: " << markers_to_acquire << "\n"
				<< "AVAILABLE_ELEMENTS -- MARKERS: " << available_elements_markers << "\n"
				<< "actual_elements_markers: " << actual_elements_markers << "\n"
				;
#endif

#ifdef variable_m_t_a
			if (available_elements_markers > markers_to_acquire && active_multiplier < multiplier_max)
			{
				markers_to_acquire = min_target_records * ++active_multiplier;
			}
#endif

			markers_buffer.advance_offset(first_element_markers);
			markers_buffer.advance_acquired(actual_elements_markers);
		}
	}

process:
	ViInt64 first_element_samples;
	ViInt64 actual_elements_samples = 0;
	ViInt64 available_elements_samples = 0;

#ifdef print_acq
	std::cout << "\tTO ACQUIRE: " << to_acquire << std::endl;	
	auto t1_samples = std::chrono::high_resolution_clock::now();
#endif // print_acq

	do
	{
		// Should hopefully only need no more than two calls to FetchData. If that's not the case, this will need to be handled differently.
		int actual_acquire = to_acquire - actual_elements_samples;

		AqMD3_StreamFetchDataInt32(
			session,
			samples_channel.c_str(),
			actual_acquire,
			samples_buffer->get_size(),
			(ViInt32 *)samples_buffer->get_raw_unaquired(),
			&available_elements_samples, &actual_elements_samples, &first_element_samples);

#ifdef print_acq
		if (actual_elements_samples != 0 && actual_elements_samples < to_acquire)
		{
			std::cout << "\tSAMPLES: actual_elements_samples != 0 && actual_elements_samples < to_acquire\n";
		}
#endif // print_acq

		samples_buffer->advance_offset(first_element_samples);
		samples_buffer->advance_acquired(actual_elements_samples);
	} while (samples_buffer->get_acquired() < to_acquire);

#ifdef print_acq
	auto t2_samples = std::chrono::high_resolution_clock::now();
	auto dur_samples = std::chrono::duration_cast<std::chrono::milliseconds>(t2_samples - t1_samples);
	std::cout << "\tTIME TO ACQUIRE SAMPLES: " << dur_samples.count() << "\n";
	std::cout << "END ACQUIRE. AVAILABLE_ELEMENTS -- MARKERS: " << available_elements_markers << "\n";
#endif

	//std::cout
		//<< "to_acquire: " << to_acquire << "\n"
		//<< "\tAVAILABLE_ELEMENTS -- SAMPLES: " << available_elements_samples << "\n"
		//<< "actual_elements_samples: " << actual_elements_samples << "\n"
		//;

	return AcquiredData(stamps, samples_buffer, samples_per_trigger);

}