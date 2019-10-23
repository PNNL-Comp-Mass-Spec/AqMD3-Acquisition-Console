#ifndef ACQUISITION_CONTROL_H
#define ACQUISITION_CONTROL_H

// High level interface for managing acquisition.
class AcquisitionControl {
public:
	virtual void start() = 0;
	virtual void stop() = 0;
};

#endif // !ACQUISITION_CONTROL_H
