// Copyright (c) Stanford University
//
// This source code is patent protected and being made available under the
// terms explained in the ../LICENSE-Academic and ../LICENSE-GOV files.
//
// Author: Mario J Srouji
// Email: msrouji@stanford.edu

module inst_constraints (clk,
                        instruction);
    
    input clk;
    input [31:0] instruction;
    
    wire [6:0] funct7;
    wire [2:0] funct3;
    wire [4:0] rd;
    wire [4:0] rs1;
    wire [4:0] rs2;
    wire [6:0] opcode;
    wire [4:0] shamt;
    wire [11:0] imm12;
    wire [6:0] imm7;
    wire [4:0] imm5;
    
    wire FORMAT_R;
    wire ALLOWED_R;
    wire ADD;
    wire SUB;
    wire SLL;
    wire SLT;
    wire SLTU;
    wire XOR;
    wire SRL;
    wire SRA;
    wire OR;
    wire AND;
    wire MUL;
    wire MULH;
    wire MULHSU;
    wire MULHU;
    
    wire FORMAT_I;
    wire ALLOWED_I;
    wire ADDI;
    wire SLTI;
    wire SLTIU;
    wire XORI;
    wire ORI;
    wire ANDI;
    wire SLLI;
    wire SRLI;
    wire SRAI;
    
    wire FORMAT_LW;
    wire ALLOWED_LW;
    wire LW;
    
    wire FORMAT_SW;
    wire ALLOWED_SW;
    wire SW;
    
    wire ALLOWED_NOP;
    wire NOP;
    
    assign funct7 = instruction[31:25];
    assign funct3 = instruction[14:12];
    assign rd     = instruction[11:7];
    assign rs1    = instruction[19:15];
    assign rs2    = instruction[24:20];
    assign opcode = instruction[6:0];
    assign shamt  = instruction[24:20];
    assign imm12  = instruction[31:20];
    assign imm7   = instruction[31:25];
    assign imm5   = instruction[11:7];
    
    assign FORMAT_R  = (rs2 < 16) && (rs1 < 16) && (rd < 16);
    assign ADD       = FORMAT_R && (funct3 == 3'b000) && (funct7 == 7'b0000000) && (opcode == 7'b0110011);
    assign SUB       = FORMAT_R && (funct3 == 3'b000) && (funct7 == 7'b0100000) && (opcode == 7'b0110011);
    assign SLL       = FORMAT_R && (funct3 == 3'b001) && (funct7 == 7'b0000000) && (opcode == 7'b0110011);
    assign SLT       = FORMAT_R && (funct3 == 3'b010) && (funct7 == 7'b0000000) && (opcode == 7'b0110011);
    assign SLTU      = FORMAT_R && (funct3 == 3'b011) && (funct7 == 7'b0000000) && (opcode == 7'b0110011);
    assign XOR       = FORMAT_R && (funct3 == 3'b100) && (funct7 == 7'b0000000) && (opcode == 7'b0110011);
    assign SRL       = FORMAT_R && (funct3 == 3'b101) && (funct7 == 7'b0000000) && (opcode == 7'b0110011);
    assign SRA       = FORMAT_R && (funct3 == 3'b101) && (funct7 == 7'b0100000) && (opcode == 7'b0110011);
    assign OR        = FORMAT_R && (funct3 == 3'b110) && (funct7 == 7'b0000000) && (opcode == 7'b0110011);
    assign AND       = FORMAT_R && (funct3 == 3'b111) && (funct7 == 7'b0000000) && (opcode == 7'b0110011);
    assign MUL       = FORMAT_R && (funct3 == 3'b000) && (funct7 == 7'b0000001) && (opcode == 7'b0110011);
    assign MULH      = FORMAT_R && (funct3 == 3'b001) && (funct7 == 7'b0000001) && (opcode == 7'b0110011);
    assign MULHSU    = FORMAT_R && (funct3 == 3'b010) && (funct7 == 7'b0000001) && (opcode == 7'b0110011);
    assign MULHU     = FORMAT_R && (funct3 == 3'b011) && (funct7 == 7'b0000001) && (opcode == 7'b0110011);
    assign ALLOWED_R = ADD || SUB || SLL || SLT || SLTU || XOR || SRL || SRA || OR || AND || MUL || MULH || MULHSU || MULHU;
    
    assign FORMAT_I  = (rs1 < 16) && (rd < 16);
    assign ADDI      = FORMAT_I && (funct3 == 3'b000) && (opcode == 7'b0010011);
    assign SLTI      = FORMAT_I && (funct3 == 3'b010) && (opcode == 7'b0010011);
    assign SLTIU     = FORMAT_I && (funct3 == 3'b011) && (opcode == 7'b0010011);
    assign XORI      = FORMAT_I && (funct3 == 3'b100) && (opcode == 7'b0010011);
    assign ORI       = FORMAT_I && (funct3 == 3'b110) && (opcode == 7'b0010011);
    assign ANDI      = FORMAT_I && (funct3 == 3'b111) && (opcode == 7'b0010011);
    assign SLLI      = FORMAT_I && (funct3 == 3'b001) && (funct7 == 7'b0000000) && (opcode == 7'b0010011);
    assign SRLI      = FORMAT_I && (funct3 == 3'b101) && (funct7 == 7'b0000000) && (opcode == 7'b0010011);
    assign SRAI      = FORMAT_I && (funct3 == 3'b101) && (funct7 == 7'b0100000) && (opcode == 7'b0010011);
    assign ALLOWED_I = ADDI || SLTI || SLTIU || XORI || ORI || ANDI || SLLI || SRLI || SRAI;
    
    assign FORMAT_LW  = (instruction[31:30] == 00) && (rs1 < 16) && (rd < 16);
    assign LW         = FORMAT_LW && (rs1 == 5'b00000) && (opcode == 7'b0000011) && (funct3 == 3'b010);
    assign ALLOWED_LW = LW;
    
    assign FORMAT_SW  = (instruction[31:30] == 00) && (rs2 < 16) && (rs1 < 16);
    assign SW         = FORMAT_SW && (rs1 == 5'b00000) && (opcode == 7'b0100011) && (funct3 == 3'b010);
    assign ALLOWED_SW = SW;
    
    assign NOP         = (opcode == 7'b1111111);
    assign ALLOWED_NOP = NOP;
    
    `ifndef VERILATOR
    always @(posedge clk) begin
        assume (ALLOWED_R || ALLOWED_I || ALLOWED_LW || ALLOWED_SW || ALLOWED_NOP);
    end
    `endif
    
endmodule
