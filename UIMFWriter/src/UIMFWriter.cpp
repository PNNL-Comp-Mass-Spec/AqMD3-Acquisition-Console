#include "UIMFWriter/UIMFWriter.h"
#include <iostream>


std::shared_ptr<UimfWriter> UimfWriter::Instance(std::string filepath) {
	std::shared_ptr<UimfWriter> returnable(new UimfWriter());
	
	return std::move(returnable);
}
