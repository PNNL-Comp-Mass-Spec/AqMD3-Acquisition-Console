#ifndef ACQUISITION_CONTROL_H
#define ACQUISITION_CONTROL_H

#include <UIMFWriter/uimfframe.h>
#include <iostream>
#include <atomic>

// High level interface for managing acquisition.
class AcquisitionControl {
protected:
	enum class State : uint8_t
	{
		ACQUIRING,
		STOPPED,
		ERRORED,
		UNKNOWN
	};

	std::atomic<State> state;

public:
	AcquisitionControl()
		: state(State::UNKNOWN)
	{}

	virtual ~AcquisitionControl() = default;

	virtual void start(std::shared_ptr<UimfFrame> frame) = 0;
	virtual void start() = 0;
	virtual void stop() = 0;

	inline bool is_acquiring() { return state == State::ACQUIRING; }

};

#endif // !ACQUISITION_CONTROL_H
