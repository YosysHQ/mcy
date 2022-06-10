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

module testbench (
	input [63:0] din_data_a,
	input [63:0] din_data_b,
	input [ 2:0] din_func,
	input [ 5:0] shamt
);
	wire [63:0] dout_data_a;
	wire [63:0] dout_data_b;
	wire [63:0] any_onehot = 1 << shamt;

	bitcnt A (
		.din_data  (din_data_a),
		.din_func  (din_func),
		.dout_data (dout_data_a)
	);

	bitcnt B (
		.din_data  (din_data_b),
		.din_func  (din_func),
		.dout_data (dout_data_b)
	);

	always @* begin
		case (din_func)
			3'b 000: begin
				// 64-bit count leading zeros
				assume (din_data_a != 0);
				assume (din_data_a >> 1 == din_data_b);
				assert (dout_data_a + 1 == dout_data_b);
				if (din_data_a[63]) assert(dout_data_a == 0);
			end
			3'b 001: begin
				// 32-bit count leading zeros
				assume (din_data_a[31:0] != 0);
				assume (din_data_a[31:0] >> 1 == din_data_b[31:0]);
				assert (dout_data_a + 1 == dout_data_b);
				if (din_data_a[31]) assert(dout_data_a == 0);
			end
			3'b 010: begin
				// 64-bit count trailing zeros
				assume (din_data_a != 0);
				assume (din_data_a << 1 == din_data_b);
				assert (dout_data_a + 1 == dout_data_b);
				if (din_data_a[0]) assert(dout_data_a == 0);
			end
			3'b 011: begin
				// 32-bit count trailing zeros
				assume (din_data_a[31:0] != 0);
				assume (din_data_a[31:0] << 1 == din_data_b[31:0]);
				assert (dout_data_a + 1 == dout_data_b);
				if (din_data_a[0]) assert(dout_data_a == 0);
			end
			3'b 100: begin
				// 64-bit population count
				assume (din_data_a != din_data_b);
				assume ((din_data_a | any_onehot) == din_data_b);
				assert (dout_data_a + 1 == dout_data_b);
				if (din_data_a == 0) assert(dout_data_a == 0);
			end
			3'b 101: begin
				// 32-bit population count
				assume (din_data_a[31:0] != din_data_b[31:0]);
				assume ((din_data_a[31:0] | any_onehot[31:0]) == din_data_b[31:0]);
				assert (dout_data_a + 1 == dout_data_b);
				if (din_data_a[31:0] == 0) assert(dout_data_a == 0);
			end
		endcase
	end
endmodule
