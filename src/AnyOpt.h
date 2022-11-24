#pragma once
#ifndef ANY_OPT_HEADER
#define ANY_OPT_HEADER

#include "BigNumber.h"
#include "Evalvisitor.h"
#include <iostream>
#include <string>
#include <unordered_map>
using std::string;

void testVar(antlrcpp::Any &any);

Int toInt(antlrcpp::Any const &a);
double toFloat(antlrcpp::Any const &a);
string toStr(antlrcpp::Any const &a);
bool toBool(antlrcpp::Any const &a);

antlrcpp::Any operator-(antlrcpp::Any const &a);
void setAnyNegative(antlrcpp::Any &a);

antlrcpp::Any operator+(antlrcpp::Any const &a, antlrcpp::Any const &b);
antlrcpp::Any operator-(antlrcpp::Any const &a, antlrcpp::Any const &b);
antlrcpp::Any operator*(antlrcpp::Any const &a, antlrcpp::Any const &b);
antlrcpp::Any DivFloat(antlrcpp::Any const &a, antlrcpp::Any const &b);
antlrcpp::Any DivInt(antlrcpp::Any const &a, antlrcpp::Any const &b);
antlrcpp::Any operator%(antlrcpp::Any const &a, antlrcpp::Any const &b);

antlrcpp::Any& operator+=(antlrcpp::Any &a, antlrcpp::Any const &b);
antlrcpp::Any& operator-=(antlrcpp::Any &a, antlrcpp::Any const &b);
antlrcpp::Any& operator*=(antlrcpp::Any &a, antlrcpp::Any const &b);
antlrcpp::Any& operator%=(antlrcpp::Any &a, antlrcpp::Any const &b);

bool operator<(antlrcpp::Any const &a, antlrcpp::Any const &b);
bool operator>(antlrcpp::Any const &a, antlrcpp::Any const &b);
bool operator==(antlrcpp::Any const &a, antlrcpp::Any const &b);
bool operator>=(antlrcpp::Any const &a, antlrcpp::Any const &b);
bool operator<=(antlrcpp::Any const &a, antlrcpp::Any const &b);
bool operator!=(antlrcpp::Any const &a, antlrcpp::Any const &b);

string operator*(string x, int b);

std::ostream &operator<<(std::ostream &, antlrcpp::Any const &);

#endif
