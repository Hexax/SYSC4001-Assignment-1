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
#define CONTEXT_SAVE_TIME 10

int main(int argc, char **argv) {
  // vectors is a C++ std::vector of strings that contain the address of the ISR
  // delays  is a C++ std::vector of ints that contain the delays of each device
  // the index of these elemens is the device number, starting from 0
  auto [vectors, delays] = parse_args(argc, argv);
  std::ifstream input_file(argv[1]);

  std::string trace;     //!< string to store single line of trace file
  std::string execution; //!< string to accumulate the execution output

  /******************ADD YOUR VARIABLES HERE*************************/
  // tracks amount of time spent doing various actions
  int clock = 0;

  // variable used to help calculate remaining time for IO devices.
  int isr_total_delays = 0;

  auto simcycles = [&](int duration, const std::string &action, bool incr_isr_total_delays = false) {
    // log -> current clock, duration of interrupt, action performed
    execution += std::to_string(clock) + ", " + std::to_string(duration) + ", " + action + "\n";

    // add duration of interrupt to current clock
    clock += duration;

    if (incr_isr_total_delays) {
      isr_total_delays++;
    }
  };
  /******************************************************************/

  // parse each line of the input trace file
  while (std::getline(input_file, trace)) {
    auto [activity, duration_intr] = parse_trace(trace);

    // reset isr_total_delays each time there is a new action
    isr_total_delays = 0;
    /******************ADD YOUR SIMULATION CODE HERE*************************/

    // CPU simulation
    if (activity == "CPU") {

      simcycles(duration_intr, "CPU Burst");

      // System call Simulation
    } else {

      // Interrupt routine
      auto [executiontemp, tempclock] = intr_boilerplate(clock, duration_intr, CONTEXT_SAVE_TIME, vectors);
      execution += executiontemp;
      clock = tempclock;

      if (activity == "SYSCALL") {
        // Run the ISR
        simcycles(ISR_DELAY, "SYSCALL: run the ISR (device driver)", true);

        // transfer data from device to memory
        simcycles(ISR_DELAY, "transfer data from device to memory", true);

        if (delays[duration_intr] - (ISR_DELAY * isr_total_delays) > 0) {
          simcycles(delays[duration_intr] - (ISR_DELAY * isr_total_delays), "Remaining ISR tasks");
        }

        // Return to user mode after interrupt
        simcycles(1, "IRET");

        // END_IO Simulation
      } else if (activity == "END_IO") {

        simcycles(ISR_DELAY, "END_IO: run the ISR(device driver)", true);

        if (delays[duration_intr] - (ISR_DELAY * isr_total_delays) > 0) {
          simcycles(delays[duration_intr] - (ISR_DELAY * isr_total_delays), "check device status");
        }

        // Return to user mode after interrupt
        simcycles(1, "IRET");
      }
    }
    /************************************************************************/
  }

  input_file.close();

  write_output(execution);

  return 0;
}
