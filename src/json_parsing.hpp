#ifndef JSON_PARSING_INCLUDED_
#define JSON_PARSING_INCLUDED_

#include "VectorN.hpp"
#include "Material.hpp"
#include "Process.hpp"
#include "BoardLayer.hpp"
#include "Board.hpp"

void ParseMaterial(const char *filename, std::vector<Material> &materials);
double UnitsToInches(const char *value_units);
void ParseProcess(const char *filename, std::vector<Process> &processes,
                  std::vector<Material> const& materials);
void ParseBoard(const char *filename, Board &board,
                std::vector<Process> const& processes);

#endif // #ifndef JSON_PARSING_INCLUDED_

