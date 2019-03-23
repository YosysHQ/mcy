`timescale 1 ns / 1 ps

module testbench;
	reg clk;
	reg resetn = 0;
	wire trap;

	always #5 clk = (clk === 1'b0);

	integer timeout = 0;
	integer cycle_cnt = 0;
	integer char_cnt = 0;

	always @(posedge clk) begin
		cycle_cnt <= cycle_cnt + 1;

		if (cycle_cnt > 150000) begin
			$display("GLOBAL TIMEOUT");
			$finish;
		end

		if (timeout > 10000) begin
			$display("CONSOLE WATCHDOG TIMEOUT");
			$finish;
		end

		if (char_cnt > 2000) begin
			$display("CONSOLE WATCHDOG OVERFLOW");
			$finish;
		end
	end

	initial begin
		repeat (100) @(posedge clk);
		resetn <= 1;
	end

	initial begin
		if ($test$plusargs("vcd")) begin
			$dumpfile("testbench.vcd");
			$dumpvars(0, testbench);
		end
	end

	always @(posedge clk) begin
		if (resetn && trap) begin
			$display("TRAP in cycle %1d.", cycle_cnt);
			$finish;
		end
	end

	reg [31:0] memory [0:64*1024/4-1];

	initial begin
		$readmemh("../../sim_simple.hex", memory);
	end

	reg [31:0] irq;

        always @* begin
                irq = 0;
                irq[4] = &cycle_cnt[12:0];
                irq[5] = &cycle_cnt[15:0];
        end

	reg [7:0] mutsel;

	initial begin
		if (!$value$plusargs("mut=%d", mutsel)) begin 
			mutsel = 0;
		end
	end

	wire        mem_valid;
	wire        mem_instr;
	reg         mem_ready;
	wire [31:0] mem_addr;
	wire [31:0] mem_wdata;
	wire [3:0]  mem_wstrb;
	reg  [31:0] mem_rdata;

	always @(posedge clk) begin
		mem_ready <= 0;
		mem_rdata <= 0;
		timeout <= timeout + resetn;

		if (resetn && mem_valid && !mem_ready) begin
			mem_ready <= 1;
			if (mem_addr == 32'h1000_0000) begin
				timeout <= 0;
				char_cnt <= char_cnt + 1;
				$write("%c", mem_wdata[7:0]);
				$fflush;
			end else begin
				mem_rdata <= memory[mem_addr >> 2];
				if (mem_wstrb[0]) memory[mem_addr >> 2][ 7: 0] <= mem_wdata[ 7: 0];
				if (mem_wstrb[1]) memory[mem_addr >> 2][15: 8] <= mem_wdata[15: 8];
				if (mem_wstrb[2]) memory[mem_addr >> 2][23:16] <= mem_wdata[23:16];
				if (mem_wstrb[3]) memory[mem_addr >> 2][31:24] <= mem_wdata[31:24];
			end
		end
	end

	picorv32 uut (
		.clk(clk),
		.resetn(resetn),
		.trap(trap),
		.mutsel(mutsel),
		.irq(irq),

		.mem_valid (mem_valid),
		.mem_instr (mem_instr),
		.mem_ready (mem_ready),

		.mem_addr  (mem_addr ),
		.mem_wdata (mem_wdata),
		.mem_wstrb (mem_wstrb),
		.mem_rdata (mem_rdata)
	);
endmodule
