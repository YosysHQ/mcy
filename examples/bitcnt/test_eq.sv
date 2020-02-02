/*
 *  Copyright (C) 2020  Claire Wolf <claire@symbioticeda.com>
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
		if (din_func[0]) begin
			assume (ref_din_data[31:0] == uut_din_data[31:0]);
			if (din_func[2:1] != 3) begin
				assert (ref_dout_data[31:0] == uut_dout_data[31:0]);
			end
		end else begin
			assume (ref_din_data == uut_din_data);
			if (din_func[2:1] != 3) begin
				assert (ref_dout_data == uut_dout_data);
			end
		end
	end
endmodule
