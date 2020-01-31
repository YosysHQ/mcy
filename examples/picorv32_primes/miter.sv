module miter (
	input        clk,
	input        resetn,
	input        mem_ready,
	input [31:0] mem_rdata,
	input [31:0] irq
);
	wire        ref_trap;
	wire        ref_mem_valid;
	wire        ref_mem_instr;
	wire [31:0] ref_mem_addr;
	wire [31:0] ref_mem_wdata;
	wire [3:0]  ref_mem_wstrb;

	picorv32 ref (
		.mutsel    (8'd 0),

		.clk       (clk      ),
		.resetn    (resetn   ),
		.irq       (irq      ),
		.mem_ready (mem_ready),
		.mem_rdata (mem_rdata),

		.trap      (ref_trap     ),
		.mem_valid (ref_mem_valid),
		.mem_instr (ref_mem_instr),
		.mem_addr  (ref_mem_addr ),
		.mem_wdata (ref_mem_wdata),
		.mem_wstrb (ref_mem_wstrb)
	);

	wire        uut_trap;
	wire        uut_mem_valid;
	wire        uut_mem_instr;
	wire [31:0] uut_mem_addr;
	wire [31:0] uut_mem_wdata;
	wire [3:0]  uut_mem_wstrb;

	picorv32 uut (
		.mutsel    (8'd `mutidx),

		.clk       (clk      ),
		.resetn    (resetn   ),
		.irq       (irq      ),
		.mem_ready (mem_ready),
		.mem_rdata (mem_rdata),

		.trap      (uut_trap     ),
		.mem_valid (uut_mem_valid),
		.mem_instr (uut_mem_instr),
		.mem_addr  (uut_mem_addr ),
		.mem_wdata (uut_mem_wdata),
		.mem_wstrb (uut_mem_wstrb)
	);

	initial assume(!resetn);

	always @* begin
		if (resetn) begin
			assert (ref_trap == uut_trap);
			assert (ref_mem_valid == uut_mem_valid);
			if (ref_mem_valid) begin
				assert (ref_mem_instr == uut_mem_instr);
				assert (ref_mem_addr  == uut_mem_addr);
				assert (ref_mem_wstrb == uut_mem_wstrb);
				if (ref_mem_wstrb[0]) assert (ref_mem_wdata[ 7: 0] == uut_mem_wdata[ 7: 0]);
				if (ref_mem_wstrb[1]) assert (ref_mem_wdata[15: 8] == uut_mem_wdata[15: 8]);
				if (ref_mem_wstrb[2]) assert (ref_mem_wdata[23:16] == uut_mem_wdata[23:16]);
				if (ref_mem_wstrb[3]) assert (ref_mem_wdata[31:24] == uut_mem_wdata[31:24]);
			end
		end
	end
endmodule
