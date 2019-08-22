#ifndef UIMFWRITER_H
#define UIMFWRITER_H

#include <sqlite3.h>
#include <memory>
#include <string>

class UimfWriter
{
	private:
		sqlite3 *db;

	public:
		UimfWriter() {};
		~UimfWriter() {};

		static std::shared_ptr<UimfWriter> Instance(std::string filepath);
};


#endif // !UIMFWRITER_H
