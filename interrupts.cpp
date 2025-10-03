/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include "interrupts.hpp"

#include <iostream>
using namespace std;

#define ISR_DELAY 40

int main(int argc, char **argv) {
  // vectors is a C++ std::vector of strings that contain the address of the ISR
  // delays  is a C++ std::vector of ints that contain the delays of each device
  // the index of these elemens is the device number, starting from 0
  auto [vectors, delays] = parse_args(argc, argv);
  std::ifstream input_file(argv[1]);

  std::string trace;     //!< string to store single line of trace file
  std::string execution; //!< string to accumulate the execution output

  /******************ADD YOUR VARIABLES HERE*************************/
  int clock = 0;

  /******************************************************************/

  // parse each line of the input trace file
  while (std::getline(input_file, trace)) {
    auto [activity, duration_intr] = parse_trace(trace);
    int isr_total_delays = 0;
    /******************ADD YOUR SIMULATION CODE HERE*************************/

    if (activity == "CPU") {
      execution += std::to_string(clock) + ", " + std::to_string(duration_intr) + ", CPU Burst\n";
      clock += duration_intr;
    } else if (activity == "SYSCALL") {
      auto [executiontemp, tempclock] = intr_boilerplate(clock, duration_intr, 10, vectors);
      execution += executiontemp;
      clock = tempclock;

      // Check for hardware Interrupts
      if (duration_intr < 20) {
        // Run the ISR
        execution += std::to_string(clock) + ", " + std::to_string(ISR_DELAY) + ", SYSCALL: run the ISR (device driver)\n";
        clock += ISR_DELAY;
        isr_total_delays++;
        // transfer data from device to memory
        execution += std::to_string(clock) + ", " + std::to_string(ISR_DELAY) + ", transfer data from device to memory\n";
        clock += ISR_DELAY;
        isr_total_delays++;
      }
      if (delays[duration_intr] - (ISR_DELAY * isr_total_delays) > 0) {
        execution += std::to_string(clock) + ", " + std::to_string(delays[duration_intr] - (ISR_DELAY * isr_total_delays)) + ", Remaining ISR tasks\n";
        clock += delays[duration_intr] - (ISR_DELAY * isr_total_delays);
      }
    } else if (activity == "END_IO") {
      execution += std::to_string(clock) + ", " + std::to_string(ISR_DELAY) + ", ENDIO: run the ISR(device driver)\n";
      clock += ISR_DELAY;
      isr_total_delays++;

      execution += std::to_string(clock) + ", " + std::to_string(delays[duration_intr] - (ISR_DELAY * isr_total_delays)) + ", check device status\n";
      clock += delays[duration_intr] - (ISR_DELAY * isr_total_delays);
    }
    /************************************************************************/
  }

  for (size_t i = 0; i < delays.size(); i++) {
    std::cout << "Delay total for ISR: " << i << ": " << delays[i] << "\n";
  }
  input_file.close();

  write_output(execution);

  return 0;
}
