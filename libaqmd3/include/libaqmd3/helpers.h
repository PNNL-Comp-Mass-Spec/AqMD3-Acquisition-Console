#ifndef HELPERS_H
#define HELPERS_H

#include "../include/libaqmd3/helpers.h"
#include "../include/libaqmd3/digitizer.h"

#include <format>

template <typename T>
T check_and_throw_on_error(std::tuple<std::string, enum Digitizer::ErrorType, T> err)
{
	if (std::get<1>(err) == Digitizer::Error)
	{
		throw std::runtime_error(std::format("Error during call to digitizer. {}", std::get<0>(err)));
	}

	return std::get<2>(err);
}

void check_and_throw_on_error(std::pair<std::string, enum Digitizer::ErrorType> err);

#endif