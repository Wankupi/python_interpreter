#pragma once
#ifndef PYTHON_BASE_H
#define PYTHON_BASE_H

#include <any>

template <typename T>
bool is(std::any const &a) {
	return std::any_cast<T>(&a);
}

template <typename T>
T &as(std::any &a) {
	return std::any_cast<T>(a);
}

template <typename T>
T const &as(std::any const &a) {
	return std::any_cast<T>(a);
}

bool isNull(std::any const &a) {
	return !a.has_value();
}

#endif
