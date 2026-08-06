/*
 *  Copyright (C) 2025 Yufeng Li <crazybinary494@gmail.com>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

module testbench #(
	parameter DATA_LEN = 32,
    parameter INST_LEN = 32,
    parameter REG_SEL = 5,
    parameter REG_NUM = 32
)
(
	input din_clk,  
	input din_rst_n,
	input [INST_LEN-1:0] din_instruction,
	input din_exec_dup
);
	wire [DATA_LEN-1:0] dout_gpr0;
	wire [DATA_LEN-1:0] dout_gpr1;
	wire [DATA_LEN-1:0] dout_gpr2;
	wire [DATA_LEN-1:0] dout_gpr3;
	wire [DATA_LEN-1:0] dout_gpr4;
	wire [DATA_LEN-1:0] dout_gpr5;
	wire [DATA_LEN-1:0] dout_gpr6;
	wire [DATA_LEN-1:0] dout_gpr7;
	wire [DATA_LEN-1:0] dout_gpr8;
	wire [DATA_LEN-1:0] dout_gpr9;
	wire [DATA_LEN-1:0] dout_gpr10;
	wire [DATA_LEN-1:0] dout_gpr11;
	wire [DATA_LEN-1:0] dout_gpr12;
	wire [DATA_LEN-1:0] dout_gpr13;
	wire [DATA_LEN-1:0] dout_gpr14;
	wire [DATA_LEN-1:0] dout_gpr15;
	wire [DATA_LEN-1:0] dout_gpr16;
	wire [DATA_LEN-1:0] dout_gpr17;
	wire [DATA_LEN-1:0] dout_gpr18;
	wire [DATA_LEN-1:0] dout_gpr19;
	wire [DATA_LEN-1:0] dout_gpr20;
	wire [DATA_LEN-1:0] dout_gpr21;
	wire [DATA_LEN-1:0] dout_gpr22;
	wire [DATA_LEN-1:0] dout_gpr23;
	wire [DATA_LEN-1:0] dout_gpr24;
	wire [DATA_LEN-1:0] dout_gpr25;
	wire [DATA_LEN-1:0] dout_gpr26;
	wire [DATA_LEN-1:0] dout_gpr27;
	wire [DATA_LEN-1:0] dout_gpr28;
	wire [DATA_LEN-1:0] dout_gpr29;
	wire [DATA_LEN-1:0] dout_gpr30;
	wire [DATA_LEN-1:0] dout_gpr31;
	wire dout_arfwe1;
	wire dout_arfwe2;
	wire [REG_SEL-1:0] dout_dstarf1;
	wire [REG_SEL-1:0] dout_dstarf2;
	wire dout_stall_IF;

	// Use the inst_constraint module to constrain instruction to be
    // a valid instruction from the ISA
    inst_constraints ic (
		.clk         (din_clk),
        .instruction (din_instruction)
	);

	wire [INST_LEN-1:0] qed_instruction;
	wire qed_vld_out;

	// QED module: transforms the original instruction 
	qed qed0 (
		// Inputs
		.clk(din_clk),
		.rst(~din_rst_n),
		.ena(1'b1),
		.exec_dup(din_exec_dup),
		.ifu_qed_instruction(din_instruction),
		.stall_IF(dout_stall_IF),

		// Outputs
		.qed_ifu_instruction(qed_instruction),
		.vld_out(qed_vld_out)
	);

	top_ridecore duv (
		.clk         (din_clk),
		.reset_x  	 (din_rst_n),
		.qed_instruction (qed_instruction),
		.qed_vld_out (qed_vld_out),

		.gpr0        (dout_gpr0),
		.gpr1        (dout_gpr1),
		.gpr2        (dout_gpr2),
		.gpr3        (dout_gpr3),
		.gpr4        (dout_gpr4),
		.gpr5        (dout_gpr5),
		.gpr6        (dout_gpr6),
		.gpr7        (dout_gpr7),
		.gpr8        (dout_gpr8),
		.gpr9        (dout_gpr9),
		.gpr10       (dout_gpr10),
		.gpr11       (dout_gpr11),
		.gpr12       (dout_gpr12),
		.gpr13       (dout_gpr13),
		.gpr14       (dout_gpr14),
		.gpr15       (dout_gpr15),
		.gpr16       (dout_gpr16),
		.gpr17       (dout_gpr17),
		.gpr18       (dout_gpr18),
		.gpr19       (dout_gpr19),
		.gpr20       (dout_gpr20),
		.gpr21       (dout_gpr21),
		.gpr22       (dout_gpr22),
		.gpr23       (dout_gpr23),
		.gpr24       (dout_gpr24),
		.gpr25       (dout_gpr25),
		.gpr26       (dout_gpr26),
		.gpr27       (dout_gpr27),
		.gpr28       (dout_gpr28),
		.gpr29       (dout_gpr29),
		.gpr30       (dout_gpr30),
		.gpr31       (dout_gpr31),
		.arfwe1      (dout_arfwe1),
		.arfwe2      (dout_arfwe2),
		.dstarf1     (dout_dstarf1),
		.dstarf2     (dout_dstarf2),
		.stall_IF    (dout_stall_IF)
	);

	properties p (
		.clk (din_clk),
		.rst (~din_rst_n),
		.reg_val0 (dout_gpr0),
		.reg_val1 (dout_gpr1),
		.reg_val2 (dout_gpr2),
		.reg_val3 (dout_gpr3),
		.reg_val4 (dout_gpr4),
		.reg_val5 (dout_gpr5),
		.reg_val6 (dout_gpr6),
		.reg_val7 (dout_gpr7),
		.reg_val8 (dout_gpr8),
		.reg_val9 (dout_gpr9),
		.reg_val10 (dout_gpr10),
		.reg_val11 (dout_gpr11),
		.reg_val12 (dout_gpr12),
		.reg_val13 (dout_gpr13),
		.reg_val14 (dout_gpr14),
		.reg_val15 (dout_gpr15),
		.reg_val16 (dout_gpr16),
		.reg_val17 (dout_gpr17),
		.reg_val18 (dout_gpr18),
		.reg_val19 (dout_gpr19),
		.reg_val20 (dout_gpr20),
		.reg_val21 (dout_gpr21),
		.reg_val22 (dout_gpr22),
		.reg_val23 (dout_gpr23),
		.reg_val24 (dout_gpr24),
		.reg_val25 (dout_gpr25),
		.reg_val26 (dout_gpr26),
		.reg_val27 (dout_gpr27),
		.reg_val28 (dout_gpr28),
		.reg_val29 (dout_gpr29),
		.reg_val30 (dout_gpr30),
		.reg_val31 (dout_gpr31),
		.arfwe1 (dout_arfwe1),
		.arfwe2 (dout_arfwe2),
		.dstarf1 (dout_dstarf1),
		.dstarf2 (dout_dstarf2)
	);


endmodule

`default_nettype wire