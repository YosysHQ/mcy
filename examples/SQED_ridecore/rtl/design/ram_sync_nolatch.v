`include "constants.vh"
`default_nettype none
module ram_sync_nolatch_2r1w #(
			       parameter BRAM_ADDR_WIDTH = `ADDR_LEN,
			       parameter BRAM_DATA_WIDTH = `DATA_LEN,
			       parameter DATA_DEPTH      = 32
			       ) 
   (
    input wire 			      clk,
    input wire [BRAM_ADDR_WIDTH-1:0]  raddr1,
    input wire [BRAM_ADDR_WIDTH-1:0]  raddr2,
    output wire [BRAM_DATA_WIDTH-1:0] rdata1,
    output wire [BRAM_DATA_WIDTH-1:0] rdata2,
    input wire [BRAM_ADDR_WIDTH-1:0]  waddr,
    input wire [BRAM_DATA_WIDTH-1:0]  wdata,
    input wire 			      we
    );
   
   reg [BRAM_DATA_WIDTH-1:0] 				       mem [0:DATA_DEPTH-1];

   integer 						       i;
   
   assign rdata1 = mem[raddr1];
   assign rdata2 = mem[raddr2];
   
   always @ (posedge clk) begin
      if (we)
	mem[waddr] <= wdata;
   end
endmodule // ram_sync_nolatch_2r1w

module ram_sync_nolatch_2r2w #(
			       parameter BRAM_ADDR_WIDTH = `ADDR_LEN,
			       parameter BRAM_DATA_WIDTH = `DATA_LEN,
			       parameter DATA_DEPTH      = 32
			       ) 
   (
    input wire 			      clk,
    input wire [BRAM_ADDR_WIDTH-1:0]  raddr1,
    input wire [BRAM_ADDR_WIDTH-1:0]  raddr2,
    output wire [BRAM_DATA_WIDTH-1:0] rdata1,
    output wire [BRAM_DATA_WIDTH-1:0] rdata2,
    input wire [BRAM_ADDR_WIDTH-1:0]  waddr1,
    input wire [BRAM_ADDR_WIDTH-1:0]  waddr2,
    input wire [BRAM_DATA_WIDTH-1:0]  wdata1,
    input wire [BRAM_DATA_WIDTH-1:0]  wdata2,
    input wire 			      we1,
    input wire 			      we2
    );
   
   reg [BRAM_DATA_WIDTH-1:0] 				       mem [0:DATA_DEPTH-1];

   assign rdata1 = mem[raddr1];
   assign rdata2 = mem[raddr2];
   
   always @ (posedge clk) begin
      if (we1)
	mem[waddr1] <= wdata1;
      if (we2)
	mem[waddr2] <= wdata2;
   end
endmodule // ram_sync_nolatch_2r2w

/*
 module ram_sync_nolatch_4r1w(
 input wire 			       clk,
 input wire [BRAM_ADDR_WIDTH-1:0]       raddr1,
 input wire [BRAM_ADDR_WIDTH-1:0]       raddr2,
 input wire [BRAM_ADDR_WIDTH-1:0]       raddr3,
 input wire [BRAM_ADDR_WIDTH-1:0]       raddr4,
 output wire [BRAM_DATA_WIDTH-1:0] rdata1,
 output wire [BRAM_DATA_WIDTH-1:0] rdata2,
 output wire [BRAM_DATA_WIDTH-1:0] rdata3,
 output wire [BRAM_DATA_WIDTH-1:0] rdata4,
 input wire [BRAM_ADDR_WIDTH-1:0]       waddr,
 input wire [BRAM_DATA_WIDTH-1:0]       wdata,
 input wire 			       we
 );
 parameter BRAM_ADDR_WIDTH = `ADDR_LEN;
 parameter BRAM_DATA_WIDTH = `DATA_LEN;
 parameter DATA_DEPTH      = 32;

 ram_sync_nolatch_2r1w 
 #(BRAM_ADDR_WIDTH, BRAM_DATA_WIDTH, DATA_DEPTH) 
 mem0(
 .clk(clk),
 .raddr1(raddr1),
 .raddr2(raddr2),
 .rdata1(rdata1),
 .rdata2(rdata2),
 .waddr(waddr),
 .wdata(wdata),
 .we(we)
 );

 ram_sync_nolatch_2r1w 
 #(BRAM_ADDR_WIDTH, BRAM_DATA_WIDTH, DATA_DEPTH) 
 mem1(
 .clk(clk),
 .raddr1(raddr3),
 .raddr2(raddr4),
 .rdata1(rdata3),
 .rdata2(rdata4),
 .waddr(waddr),
 .wdata(wdata),
 .we(we)
 );
 
 endmodule //ram_sync_nolatch_4r1w
 */
module ram_sync_nolatch_4r2w #(
			       parameter BRAM_ADDR_WIDTH = `ADDR_LEN,
			       parameter BRAM_DATA_WIDTH = `DATA_LEN,
			       parameter DATA_DEPTH      = 32
			       ) 
   (
    input wire 			      clk,
    input wire [BRAM_ADDR_WIDTH-1:0]  raddr1,
    input wire [BRAM_ADDR_WIDTH-1:0]  raddr2,
    input wire [BRAM_ADDR_WIDTH-1:0]  raddr3,
    input wire [BRAM_ADDR_WIDTH-1:0]  raddr4,
    output wire [BRAM_DATA_WIDTH-1:0] rdata1,
    output wire [BRAM_DATA_WIDTH-1:0] rdata2,
    output wire [BRAM_DATA_WIDTH-1:0] rdata3,
    output wire [BRAM_DATA_WIDTH-1:0] rdata4,
    input wire [BRAM_ADDR_WIDTH-1:0]  waddr1,
    input wire [BRAM_ADDR_WIDTH-1:0]  waddr2,
    input wire [BRAM_DATA_WIDTH-1:0]  wdata1,
    input wire [BRAM_DATA_WIDTH-1:0]  wdata2,
    input wire 			      we1,
    input wire 			      we2,
    output wire [BRAM_DATA_WIDTH-1:0] mem0,
    output wire [BRAM_DATA_WIDTH-1:0] mem1,
    output wire [BRAM_DATA_WIDTH-1:0] mem2,
    output wire [BRAM_DATA_WIDTH-1:0] mem3,
    output wire [BRAM_DATA_WIDTH-1:0] mem4,
    output wire [BRAM_DATA_WIDTH-1:0] mem5,
    output wire [BRAM_DATA_WIDTH-1:0] mem6,
    output wire [BRAM_DATA_WIDTH-1:0] mem7,
    output wire [BRAM_DATA_WIDTH-1:0] mem8,
    output wire [BRAM_DATA_WIDTH-1:0] mem9,
    output wire [BRAM_DATA_WIDTH-1:0] mem10,
    output wire [BRAM_DATA_WIDTH-1:0] mem11,
    output wire [BRAM_DATA_WIDTH-1:0] mem12,
    output wire [BRAM_DATA_WIDTH-1:0] mem13,
    output wire [BRAM_DATA_WIDTH-1:0] mem14,
    output wire [BRAM_DATA_WIDTH-1:0] mem15,
    output wire [BRAM_DATA_WIDTH-1:0] mem16,
    output wire [BRAM_DATA_WIDTH-1:0] mem17,
    output wire [BRAM_DATA_WIDTH-1:0] mem18,
    output wire [BRAM_DATA_WIDTH-1:0] mem19,
    output wire [BRAM_DATA_WIDTH-1:0] mem20,
    output wire [BRAM_DATA_WIDTH-1:0] mem21,
    output wire [BRAM_DATA_WIDTH-1:0] mem22,
    output wire [BRAM_DATA_WIDTH-1:0] mem23,
    output wire [BRAM_DATA_WIDTH-1:0] mem24,
    output wire [BRAM_DATA_WIDTH-1:0] mem25,
    output wire [BRAM_DATA_WIDTH-1:0] mem26,
    output wire [BRAM_DATA_WIDTH-1:0] mem27,
    output wire [BRAM_DATA_WIDTH-1:0] mem28,
    output wire [BRAM_DATA_WIDTH-1:0] mem29,
    output wire [BRAM_DATA_WIDTH-1:0] mem30,
    output wire [BRAM_DATA_WIDTH-1:0] mem31
    );

   reg [BRAM_DATA_WIDTH-1:0] 				       mem [0:DATA_DEPTH-1];
   
   //Mutation Testing: The value of the next register read is corrupted to all 0's
   assign rdata1 = mem[raddr1];
   //assign rdata1 = BRAM_DATA_WIDTH'd0;
   assign rdata2 = mem[raddr2];
   assign rdata3 = mem[raddr3];
   assign rdata4 = mem[raddr4];
   
   always @ (posedge clk) begin
      if (we1)
	      mem[waddr1] <= wdata1;
      if (we2)
	      mem[waddr2] <= wdata2;
   end

   assign mem0 = mem[0];
   assign mem1 = mem[1];
   assign mem2 = mem[2];
   assign mem3 = mem[3];
   assign mem4 = mem[4];
   assign mem5 = mem[5];
   assign mem6 = mem[6];
   assign mem7 = mem[7];
   assign mem8 = mem[8];
   assign mem9 = mem[9];
   assign mem10 = mem[10];
   assign mem11 = mem[11];
   assign mem12 = mem[12];
   assign mem13 = mem[13];
   assign mem14 = mem[14];
   assign mem15 = mem[15];
   assign mem16 = mem[16];
   assign mem17 = mem[17];
   assign mem18 = mem[18];
   assign mem19 = mem[19];
   assign mem20 = mem[20];
   assign mem21 = mem[21];
   assign mem22 = mem[22];
   assign mem23 = mem[23];
   assign mem24 = mem[24];
   assign mem25 = mem[25];
   assign mem26 = mem[26];
   assign mem27 = mem[27];
   assign mem28 = mem[28];
   assign mem29 = mem[29];
   assign mem30 = mem[30];
   assign mem31 = mem[31];
endmodule // ram_sync_nolatch_4r2w

/*
 module ram_sync_nolatch_6r2w(
 input wire 			       clk,
 input wire [BRAM_ADDR_WIDTH-1:0]       raddr1,
 input wire [BRAM_ADDR_WIDTH-1:0]       raddr2,
 input wire [BRAM_ADDR_WIDTH-1:0]       raddr3,
 input wire [BRAM_ADDR_WIDTH-1:0]       raddr4,
 input wire [BRAM_ADDR_WIDTH-1:0]       raddr5,
 input wire [BRAM_ADDR_WIDTH-1:0]       raddr6,
 output wire [BRAM_DATA_WIDTH-1:0] rdata1,
 output wire [BRAM_DATA_WIDTH-1:0] rdata2,
 output wire [BRAM_DATA_WIDTH-1:0] rdata3,
 output wire [BRAM_DATA_WIDTH-1:0] rdata4,
 output wire [BRAM_DATA_WIDTH-1:0] rdata5,
 output wire [BRAM_DATA_WIDTH-1:0] rdata6,
 input wire [BRAM_ADDR_WIDTH-1:0]       waddr1,
 input wire [BRAM_ADDR_WIDTH-1:0]       waddr2,
 input wire [BRAM_DATA_WIDTH-1:0]       wdata1,
 input wire [BRAM_DATA_WIDTH-1:0]       wdata2,
 input wire 			       we1,
 input wire 			       we2
 );
 parameter BRAM_ADDR_WIDTH = `ADDR_LEN;
 parameter BRAM_DATA_WIDTH = `DATA_LEN;
 parameter DATA_DEPTH      = 32;

 ram_sync_nolatch_2r2w 
 #(BRAM_ADDR_WIDTH, BRAM_DATA_WIDTH, DATA_DEPTH) 
 mem0(
 .clk(clk),
 .raddr1(raddr1),
 .raddr2(raddr2),
 .rdata1(rdata1),
 .rdata2(rdata2),
 .waddr1(waddr1),
 .waddr2(waddr2),
 .wdata1(wdata1),
 .wdata2(wdata2),
 .we1(we1),
 .we2(we2)
 );

 ram_sync_nolatch_2r2w 
 #(BRAM_ADDR_WIDTH, BRAM_DATA_WIDTH, DATA_DEPTH) 
 mem1(
 .clk(clk),
 .raddr1(raddr3),
 .raddr2(raddr4),
 .rdata1(rdata3),
 .rdata2(rdata4),
 .waddr1(waddr1),
 .waddr2(waddr2),
 .wdata1(wdata1),
 .wdata2(wdata2),
 .we1(we1),
 .we2(we2)
 );

 ram_sync_nolatch_2r2w 
 #(BRAM_ADDR_WIDTH, BRAM_DATA_WIDTH, DATA_DEPTH) 
 mem2(
 .clk(clk),
 .raddr1(raddr5),
 .raddr2(raddr6),
 .rdata1(rdata5),
 .rdata2(rdata6),
 .waddr1(waddr1),
 .waddr2(waddr2),
 .wdata1(wdata1),
 .wdata2(wdata2),
 .we1(we1),
 .we2(we2)
 );
 
 endmodule // ram_sync_nolatch_6r2w
 */
`default_nettype wire
