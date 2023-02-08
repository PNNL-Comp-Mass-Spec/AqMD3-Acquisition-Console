#ifndef UIMF_FRAME_WRITER_SUBSCRIBER_H
#define UIMF_FRAME_WRITER_SUBSCRIBER_H
#include "subscriber.h"
#include <UIMFWriter/uimfframe.h>
#include "definitions.h"

class UimfFrameWriterSubscriber : public Subscriber<std::shared_ptr<UimfFrame>> {
private:
	std::deque<std::shared_ptr<UimfFrame>> frames;
	bool write_timestamps;

public:
	UimfFrameWriterSubscriber(bool write_timestamps)
		: Subscriber(true)
		, write_timestamps(write_timestamps)
	{}

private:
	void on_notify() override;
	void on_completed() override;
};

#endif // !UIMF_FRAME_WRITER_SUBSCRIBER_H
