#include "json_parsing.hpp"
#include <iostream>
#include <vector>

int main (int argc, char** argv) {
  const char *filename = "material.json";

  std::vector<Material> materials;
  ParseMaterial(filename, materials);
  
  std::cout << materials.size() << std::endl;

  filename = "process.json";
  std::vector<Process> processes;
  ParseProcess(filename, processes, materials);

  std::cout << processes.size() << std::endl;


  Board board;
  filename = "soroban_clock.json";
  ParseBoard(filename, board, processes);
}

