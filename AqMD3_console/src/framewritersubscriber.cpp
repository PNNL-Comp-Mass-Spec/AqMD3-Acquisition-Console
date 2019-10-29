#include "..\include\framewritersubscriber.h"

#include <iostream>

void FrameWriterSubscriber::execute()
{
	while (!items.empty())
	{
		auto frame = items.front();
		items.pop_front();

		UimfWriter writer(frame->file_name);
		std::cout << "start writing frame" << std::endl;
		try
		{
			writer.write_frame(*frame);
		}
		catch (...)
		{
			std::cout << "there was an error writing the UIMF frame" << std::endl;
		}
		std::cout << "end writing frame" << std::endl;
	}
}
