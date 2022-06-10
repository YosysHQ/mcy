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

module bitcnt (
	// data input
	input  [63:0] din_data,    // input value
	input  [ 2:0] din_func,    // function

	// data output
	output [63:0] dout_data    // output value
);
	// func[2:0]  Function
	// ---------  --------
	//  0  0  0   CLZ_64
	//  0  0  1   CLZ_32
	//  0  1  0   CTZ_64
	//  0  1  1   CTZ_32
	//  1  0  0   CNT_64
	//  1  0  1   CNT_32
	//  1  1  0   *unused*
	//  1  1  1   *unused*

	wire mode32 = din_func[0];
	wire revmode = !din_func[1];
	wire czmode = !din_func[2];

	integer i;
	reg [63:0] tmp;
	reg [7:0] cnt;

	always @* begin
		for (i = 0; i < 64; i = i+1)
			tmp[i] = (i < 32 && mode32) ? din_data[(63-i) % 32] : din_data[63-i];
		if (!revmode)
			tmp = din_data;
		if (mode32)
			tmp = tmp[31:0];
		if (czmode)
			tmp = (tmp-1) & ~tmp;

		cnt = 0;
		for (i = 0; i < 64; i = i+1)
			cnt = cnt + (tmp[i] && (i < 32 || !mode32));
	end 

	assign dout_data = cnt;
endmodule
