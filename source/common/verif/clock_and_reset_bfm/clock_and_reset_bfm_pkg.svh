// clock_and_reset Verification IP Package

package clock_and_reset_bfm_pkg;
    // Mandatory UVM
    import uvm_pkg::*;
    `include "uvm_macros.svh"

    `include "uvm/clock_and_reset_bfm_cfg.sv"
    `include "uvm/clock_and_reset_bfm_driver.sv"    // Toggles signals
    `include "uvm/clock_and_reset_bfm_agent.sv"     // Collection of all of the above

endpackage