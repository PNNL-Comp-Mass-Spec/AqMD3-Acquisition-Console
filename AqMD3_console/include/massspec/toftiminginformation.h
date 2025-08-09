#pragma once
#include <libaqmd3/sa220.h>
#include <iostream>
#include <tuple>


namespace AqirisDigitizer
{
	class TofTimingInformation
	{
    private:
            uint64_t samples_per_trigger;
            uint64_t record_size;
            uint64_t post_trigger_delay_samples;
            uint64_t trigger_rearm_samples;
            double post_trigger_delay_seconds;
            double trigger_rearm_time_seconds;

    public:
            TofTimingInformation(uint64_t samples_per_trigger, uint64_t record_size, uint64_t post_trigger_delay_samples, uint64_t trigger_rearm_samples,
                double post_trigger_delay_seconds,
                double trigger_rearm_time_seconds)
                : post_trigger_delay_samples(post_trigger_delay_samples)
                , record_size(record_size)
                , samples_per_trigger(samples_per_trigger)
                , trigger_rearm_samples(trigger_rearm_samples)
                , post_trigger_delay_seconds(post_trigger_delay_seconds)
                , trigger_rearm_time_seconds(trigger_rearm_time_seconds)
            {}

            TofTimingInformation() = default;

        static TofTimingInformation create_timing_information(const SA220 *digitizer, double sample_rate, double post_trigger_delay_seconds, double trigger_rearm_time_seconds);
        static std::tuple<uint64_t, uint64_t, uint64_t> get_optimal_record_size(const SA220 *digitizer, uint64_t pusher_pulse_pulse_width_samples, double post_trigger_delay_s, double sample_rate, double trig_rearm_s);
        static uint64_t get_trigger_time_stamp_average(const SA220 *digitizer, int triggers);
        uint64_t get_record_size() const { return record_size; }
        uint64_t get_post_trigger_delay_samples() const { return post_trigger_delay_samples; }
        uint64_t get_samples_per_trigger() const { return samples_per_trigger; }
        uint64_t get_trigger_rearm_samples() const { return trigger_rearm_samples; }
    };
}