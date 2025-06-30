// Copyright (c) Stanford University
//
// This source code is patent protected and being made available under the
// terms explained in the ../LICENSE-Academic and ../LICENSE-GOV files.

module qed_instruction_mux (
  // Outputs
  qed_ifu_instruction,
  // Inputs
  ifu_qed_instruction, 
  qed_instruction, 
  ena, 
  exec_dup
  );

  input [31:0] ifu_qed_instruction;
  input [31:0] qed_instruction;
  input exec_dup;
  input ena;

  output [31:0] qed_ifu_instruction;

  assign qed_ifu_instruction = ena ? ((exec_dup == 1'b0) ? ifu_qed_instruction : qed_instruction) : ifu_qed_instruction;

endmodule 

