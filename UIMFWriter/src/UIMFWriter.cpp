#include "UIMFWriter/UIMFWriter.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <sqlite3.h>

#include <iostream>
#include <fstream>

void UimfWriter::write_frame(UimfFrame& frame)
{
	try
	{
		int const extra = 128;
		std::cout << "starting transaction" << std::endl;
		SQLite::Transaction transaction(db);
		char *statement = new char[insert_scan_statement_size_bytes + extra];

		for (int i = 0; i < frame.get_data().size(); i++)
		{
			for (auto& er : *(frame.get_data()[i]))
			{
				if (er.encoded_spectra.size() > 1)
				{
					int count = sprintf(statement,
						insert_scan_statement.c_str(),
						er.frame,
						er.scan,
						er.non_zero_count,
						er.bpi,
						er.bpi_mz,
						er.tic);

					auto compressed = er.get_compressed_spectra();

					SQLite::Statement statement(db, statement);
					statement.bind(1, compressed.data, compressed.bytes);
					statement.exec();
				}
			}
		}

		transaction.commit();
	}
	catch (...)
	{
		std::cout << "error writing to UIMF file" << std::endl;
	}

}
