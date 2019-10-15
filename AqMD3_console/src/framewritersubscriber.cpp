#include "..\include\framewritersubscriber.h"

int FrameWriterSubscriber::execute()
{
	while (!frames.empty())
	{
		auto frame = frames.front();
		frames.pop_front();

		UimfWriter writer(frame->file_name);
		writer.write_frame(*frame);
	}
}
