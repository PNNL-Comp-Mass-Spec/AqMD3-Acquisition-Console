#include "../../include/massspec/toftiminginformation.h"


namespace AqirisDigitizer
{
    TofTimingInformation TofTimingInformation::create_timing_information(const SA220 *digitizer, double sample_rate, double post_trigger_delay_seconds, double trigger_rearm_time_seconds)
    {
        auto samples_per_trigger = TofTimingInformation::get_trigger_time_stamp_average(digitizer, 20);
        auto t = TofTimingInformation::get_optimal_record_size(digitizer, samples_per_trigger, post_trigger_delay_seconds, sample_rate, trigger_rearm_time_seconds);

        return TofTimingInformation(samples_per_trigger, std::get<1>(t), std::get<0>(t), std::get<2>(t), 
            post_trigger_delay_seconds, trigger_rearm_time_seconds);
    }

    uint64_t TofTimingInformation::get_trigger_time_stamp_average(const SA220 *digitizer, int triggers)
    {
        uint64_t record_size = 1024;
        digitizer->set_record_size(record_size);
        auto dig_context = digitizer->configure_cst(digitizer->channel_1, std::make_shared<AcquisitionBufferPool>(triggers, record_size, 10, 10));

        dig_context->start();
        AcquiredData result = dig_context->acquire(triggers, std::chrono::milliseconds(80));
        dig_context->stop();

        uint64_t total = 0;

        for (int i = 0; i < result.stamps.size() - 1; i++)
        {
            total += result.stamps[i + 1].timestamp - result.stamps[i].timestamp;
        }

        total = ((total / (result.stamps.size() - 1)));

        return total;
    }

    std::tuple<uint64_t, uint64_t, uint64_t> TofTimingInformation::get_optimal_record_size(const SA220 *digitizer, uint64_t pusher_pulse_pulse_width_samples, double post_trigger_delay_s, double sample_rate, double trig_rearm_s)
    {
        uint64_t actual_trigger_width_samples = uint64_t(double(pusher_pulse_pulse_width_samples) * (sample_rate / digitizer->max_sample_rate));
        uint64_t trig_rearm_samples = uint64_t(trig_rearm_s * sample_rate);
        uint64_t delay_samples = uint64_t(post_trigger_delay_s * sample_rate);

        auto record_size_samples = actual_trigger_width_samples - delay_samples - trig_rearm_samples;
        if (record_size_samples % 32 != 0)
            record_size_samples = (record_size_samples / 32) * 32;

        return std::make_tuple(delay_samples, record_size_samples, trig_rearm_samples);
    }
}