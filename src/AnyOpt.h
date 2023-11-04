#pragma once
#ifndef ANY_OPT_HEADER
#define ANY_OPT_HEADER

#include "base.h"
#include "BigNumber.h"
#include "Evalvisitor.h"
#include <iostream>
#include <string>
#include <unordered_map>
using std::string;

void testVar(std::any &any);

Int toInt(std::any const &a);
double toFloat(std::any const &a);
string toStr(std::any const &a);
bool toBool(std::any const &a);

std::any operator-(std::any const &a);
void setAnyNegative(std::any &a);

std::any operator+(std::any const &a, std::any const &b);
std::any operator-(std::any const &a, std::any const &b);
std::any operator*(std::any const &a, std::any const &b);
std::any DivFloat(std::any const &a, std::any const &b);
std::any DivInt(std::any const &a, std::any const &b);
std::any operator%(std::any const &a, std::any const &b);

std::any& operator+=(std::any &a, std::any const &b);
std::any& operator-=(std::any &a, std::any const &b);
std::any& operator*=(std::any &a, std::any const &b);
std::any& operator%=(std::any &a, std::any const &b);

bool operator<(std::any const &a, std::any const &b);
bool operator>(std::any const &a, std::any const &b);
bool operator==(std::any const &a, std::any const &b);
bool operator>=(std::any const &a, std::any const &b);
bool operator<=(std::any const &a, std::any const &b);
bool operator!=(std::any const &a, std::any const &b);

string operator*(string x, int b);

std::ostream &operator<<(std::ostream &, std::any const &);

#endif
