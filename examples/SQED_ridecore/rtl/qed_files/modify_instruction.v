// Copyright (c) Stanford University
// 
// This source code is patent protected and being made available under the
// terms explained in the ../LICENSE-Academic and ../LICENSE-GOV files.
//
// Author: Mario J Srouji
// Email: msrouji@stanford.edu

module modify_instruction (
// Outputs
qed_instruction,
// Inputs
qic_qimux_instruction,
funct7,
funct3,
rd,
rs1,
rs2,
opcode,
shamt,
imm12,
imm7,
imm5,
IS_R,
IS_I,
IS_LW,
IS_SW);

  input [31:0] qic_qimux_instruction;
  input [6:0] funct7;
  input [2:0] funct3;
  input [4:0] rd;
  input [4:0] rs1;
  input [4:0] rs2;
  input [6:0] opcode;
  input [4:0] shamt;
  input [11:0] imm12;
  input [6:0] imm7;
  input [4:0] imm5;
  input IS_R;
  input IS_I;
  input IS_LW;
  input IS_SW;

  output reg [31:0] qed_instruction;

  wire [31:0] INS_R;
  wire [31:0] INS_I;
  wire [31:0] INS_LW;
  wire [31:0] INS_SW;
  wire [31:0] INS_CONSTRAINT;

  wire [4:0] NEW_rd;
  wire [4:0] NEW_rs1;
  wire [4:0] NEW_rs2;
  wire [11:0] NEW_imm12;
  wire [6:0] NEW_imm7;

  assign NEW_rd = (rd == 5'b00000) ? rd : {1'b1, rd[3:0]};
  assign NEW_rs1 = (rs1 == 5'b00000) ? rs1 : {1'b1, rs1[3:0]};
  assign NEW_rs2 = (rs2 == 5'b00000) ? rs2 : {1'b1, rs2[3:0]};
  assign NEW_imm12 = {2'b01, imm12[9:0]};
  assign NEW_imm7 = {2'b01, imm7[4:0]};

  assign INS_R = {funct7, NEW_rs2, NEW_rs1, funct3, NEW_rd, opcode};
  assign INS_I = {imm12, NEW_rs1, funct3, NEW_rd, opcode};
  assign INS_LW = {NEW_imm12, NEW_rs1, funct3, NEW_rd, opcode};
  assign INS_SW = {NEW_imm7, NEW_rs2, NEW_rs1, funct3, imm5, opcode};

  assign qed_instruction = IS_R ? INS_R : (IS_I ? INS_I : (IS_LW ? INS_LW : (IS_SW ? INS_SW : qic_qimux_instruction)));

endmodule