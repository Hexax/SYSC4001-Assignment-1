#ifndef INTERRUPTS_HPP_
#define INTERRUPTS_HPP_

#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <stdio.h>

#define ADDR_BASE 0
#define VECTOR_SIZE 2

#define CPU_SPEED 100
#define MEM_LIMIT 1

/**
 * \brief parse the CLI arguments
 *
 * This helper function parses command line arguments and checks for errors
 *
 * @param argc number of command line arguments
 * @param argv the command line arguments
 * @return a vector of strings (the parsed vector table)
 *
 */
std::tuple<std::vector<std::string>, std::vector<int>> parse_args(int argc, char **argv) {
  if (argc != 4) {
    std::cout << "ERROR!\nExpected 3 argument, received " << argc - 1 << std::endl;
    std::cout << "To run the program, do: ./interrupts <your_trace_file.txt> <your_vector_table.txt> <your_device_table.txt>" << std::endl;
    exit(1);
  }

  std::ifstream input_file;
  input_file.open(argv[1]);
  if (!input_file.is_open()) {
    std::cerr << "Error: Unable to open file: " << argv[1] << std::endl;
    exit(1);
  }

  std::ifstream input_vector_table;
  input_vector_table.open(argv[2]);
  if (!input_vector_table.is_open()) {
    std::cerr << "Error: Unable to open file: " << argv[2] << std::endl;
    exit(1);
  }

  std::string vector;
  std::vector<std::string> vectors;
  while (std::getline(input_vector_table, vector)) {
    vectors.push_back(vector);
  }
  input_vector_table.close();

  std::string duration;
  std::vector<int> delays;
  std::ifstream device_table;
  device_table.open(argv[3]);
  if (!device_table.is_open()) {
    std::cerr << "Error: Unable to open file: " << argv[3] << std::endl;
    exit(1);
  }
  while (std::getline(device_table, duration)) {
    delays.push_back(std::stoi(duration));
  }

  return {vectors, delays};
}

// Following function was taken from stackoverflow; helper function for splitting strings
std::vector<std::string> split_delim(std::string input, std::string delim) {
  std::vector<std::string> tokens;
  std::size_t pos = 0;
  std::string token;
  while ((pos = input.find(delim)) != std::string::npos) {
    token = input.substr(0, pos);
    tokens.push_back(token);
    input.erase(0, pos + delim.length());
  }
  tokens.push_back(input);

  return tokens;
}

std::tuple<std::string, int> parse_trace(std::string trace) {
  // split line by ','
  auto parts = split_delim(trace, ",");
  if (parts.size() < 2) {
    std::cerr << "Error: Malformed input line: " << trace << std::endl;
    return {"null", -1};
  }

  auto activity = parts[0];
  auto duration_intr = std::stoi(parts[1]);

  return {activity, duration_intr};
}

// Default interrupt boilerplate
std::pair<std::string, int> intr_boilerplate(int current_time, int intr_num, int context_save_time, std::vector<std::string> vectors) {

  std::string execution = "";

  execution += std::to_string(current_time) + ", " + std::to_string(1) + ", switch to kernel mode\n";
  current_time++;

  execution += std::to_string(current_time) + ", " + std::to_string(context_save_time) + ", context saved\n";
  current_time += context_save_time;

  char vector_address_c[10];
  sprintf(vector_address_c, "0x%04X", (ADDR_BASE + (intr_num * VECTOR_SIZE)));
  std::string vector_address(vector_address_c);

  execution += std::to_string(current_time) + ", " + std::to_string(1) + ", find vector " + std::to_string(intr_num) + " in memory position " + vector_address + "\n";
  current_time++;

  execution += std::to_string(current_time) + ", " + std::to_string(1) + ", load address " + vectors.at(intr_num) + " into the PC\n";
  current_time++;

  return std::make_pair(execution, current_time);
}

void write_output(std::string execution) {
  std::ofstream output_file("execution.txt");

  if (output_file.is_open()) {
    output_file << execution;
    output_file.close(); // Close the file when done
    std::cout << "File content overwritten successfully." << std::endl;
  } else {
    std::cerr << "Error opening file!" << std::endl;
  }

  std::cout << "Output generated in execution.txt" << std::endl;
}
#endif
