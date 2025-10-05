/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 * Modified by Mickael Roy and Prunellie Tchakam
 */

#include "interrupts.hpp"

#include <iostream>
using namespace std;

#define ISR_DELAY 40
#define CONTEXT_SAVE_TIME 30

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

  //
  /**
   * \brief Simulates one trace file line execution and logs it.
   *
   * Lambda function that logs the execution of a trace file. It passes by reference execution and clock
   *
   * @param duration Time needed to comlete the action
   * @param action Descriptive string that documents what the OS is doing.
   * @param incr_isr_total_delays Set to true when calling the function if duration == ISR_DELAY constant.
   * @return a vector of strings (the parsed vector table)
   *
   */
  auto simcycles = [&](int duration, const std::string &action, bool incr_isr_total_delays = false) {
    // log -> current clock, duration of interrupt, action performed
    execution += std::to_string(clock) + ", " + std::to_string(duration) + ", " + action + "\n";

    // add duration of interrupt to current clock
    clock += duration;

    // Optionally increment isr_total_delays to track the time already spent on an isr.
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
      continue;
    }

    // Interrupt routine
    auto [executiontemp, tempclock] = intr_boilerplate(clock, duration_intr, CONTEXT_SAVE_TIME, vectors);
    execution += executiontemp;
    clock = tempclock;

    // Error checking to ensure there is a device associated with the interrupt
    if (duration_intr < 0 || duration_intr >= static_cast<int>(delays.size())) {
      simcycles(1, "ERROR: invalid device id " + std::to_string(duration_intr));
      simcycles(1, "IRET");
      continue;
    }

    if (activity == "SYSCALL") {

      // Time taken to run the ISR
      int firststep = std::min(ISR_DELAY, delays[duration_intr] / 2);

      // Time taken to transfer data from device to memory
      int secondstep = std::min(ISR_DELAY, delays[duration_intr] - firststep);

      // Remaining time to complete ISR tasks
      int laststep = delays[duration_intr] - firststep - secondstep;

      if (firststep > 0)
        simcycles(firststep, "SYSCALL: run the ISR (device driver)", true);
      if (secondstep > 0)
        simcycles(secondstep, "transfer data from device to memory", true);
      if (laststep > 0)
        simcycles(laststep, "Remaining ISR tasks");

      // Return to user mode after interrupt
      simcycles(1, "IRET");
      continue;
    }

    if (activity == "END_IO") {

      // END_IO Simulation
      int firststep = std::min(ISR_DELAY, delays[duration_intr]);

      // Remaining time to complete ISR tasks
      int laststep = delays[duration_intr] - firststep;

      if (firststep > 0)
        simcycles(firststep, "END_IO: Store information in memory", true);

      if (laststep > 0)
        simcycles(laststep, "check device status");

      // Return to user mode after interrupt
      simcycles(1, "IRET");
    }
    /************************************************************************/
  }

  // simulate CPU speeds and log results at the top of output file
  double totaltime = static_cast<double>(clock) / CPU_SPEED;
  execution = "Total time taken to execute trace file: " + std::to_string(totaltime) + " seconds\n" + execution;

  // add final line to logging file.
  execution += std::to_string(clock) + ", NULL, NO MORE INSTRUCTIONS CPU SLEEP";
  input_file.close();

  write_output(execution);

  return 0;
}