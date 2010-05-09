#pragma once
#include <stdexcept>
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ERROR_STD_PROLOG    "Exception thrown in function: " __FUNCSIG__ " (in " __FILE__ ", last modified on " __TIMESTAMP__ ", line " TOSTRING( __LINE__ ) ").\nMessage: "
#define CANNOTLOADSTRING	_T("Cannot load string.")