/*
 *  Copyright (C) 2020  Claire Wolf <claire@yosyshq.com>
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

module miter (
	input [63:0] ref_din_data,
	input [63:0] uut_din_data,
	input [ 2:0] din_func
);
	wire [63:0] ref_dout_data;
	wire [63:0] uut_dout_data;

	bitcnt ref (
		.mutsel    (1'b 0),
		.din_data  (ref_din_data),
		.din_func  (din_func),
		.dout_data (ref_dout_data)
	);

	bitcnt uut (
		.mutsel    (1'b 1),
		.din_data  (uut_din_data),
		.din_func  (din_func),
		.dout_data (uut_dout_data)
	);

	always @* begin
		casez (din_func)
			3'b11z: begin
				// unused opcode: don't check anything
			end
			3'bzz1: begin
				// 32-bit opcodes, only constrain lower 32 input bits and check all 64 output bits
				assume (ref_din_data[31:0] == uut_din_data[31:0]);
				assert (ref_dout_data == uut_dout_data);
			end
			3'bzz0: begin
				// 64-bit opcodes, constrain all 64 input bits and check all 64 output bits
				assume (ref_din_data == uut_din_data);
				assert (ref_dout_data == uut_dout_data);
			end
		endcase
	end
endmodule
