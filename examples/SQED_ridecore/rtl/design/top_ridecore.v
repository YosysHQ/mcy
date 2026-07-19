`include "define.v"
`include "constants.vh"

module top_ridecore
  (
//   input 	    CLK_P,
//   input 	    CLK_N,
//   input 	    RST_X_IN,
//   output 	    TXD,
//   input 	    RXD,
//   output reg [7:0] LED
   input clk,
   input reset_x,

// EDIT
   input [`INSN_LEN-1:0] qed_instruction,
   input qed_vld_out,

   output [`DATA_LEN-1:0] gpr0,
   output [`DATA_LEN-1:0] gpr1,
   output [`DATA_LEN-1:0] gpr2,
   output [`DATA_LEN-1:0] gpr3,
   output [`DATA_LEN-1:0] gpr4,
   output [`DATA_LEN-1:0] gpr5,
   output [`DATA_LEN-1:0] gpr6,
   output [`DATA_LEN-1:0] gpr7,
   output [`DATA_LEN-1:0] gpr8,
   output [`DATA_LEN-1:0] gpr9,
   output [`DATA_LEN-1:0] gpr10,
   output [`DATA_LEN-1:0] gpr11,
   output [`DATA_LEN-1:0] gpr12,
   output [`DATA_LEN-1:0] gpr13,
   output [`DATA_LEN-1:0] gpr14,
   output [`DATA_LEN-1:0] gpr15,
   output [`DATA_LEN-1:0] gpr16,
   output [`DATA_LEN-1:0] gpr17,
   output [`DATA_LEN-1:0] gpr18,
   output [`DATA_LEN-1:0] gpr19,
   output [`DATA_LEN-1:0] gpr20,
   output [`DATA_LEN-1:0] gpr21,
   output [`DATA_LEN-1:0] gpr22,
   output [`DATA_LEN-1:0] gpr23,
   output [`DATA_LEN-1:0] gpr24,
   output [`DATA_LEN-1:0] gpr25,
   output [`DATA_LEN-1:0] gpr26,
   output [`DATA_LEN-1:0] gpr27,
   output [`DATA_LEN-1:0] gpr28,
   output [`DATA_LEN-1:0] gpr29,
   output [`DATA_LEN-1:0] gpr30,
   output [`DATA_LEN-1:0] gpr31,
   output wire arfwe1,
   output wire arfwe2,
   output wire [`REG_SEL-1:0] dstarf1,
   output wire [`REG_SEL-1:0] dstarf2,
   output wire stall_IF
// EDIT: END
   );


   //Active Low SW
//   wire 	    clk;
//   wire 	    reset_x;

   (* keep *) wire [`ADDR_LEN-1:0] pc;
   (* keep *) wire [4*`INSN_LEN-1:0] idata;
   (* keep *) wire [8:0] 		  imem_addr;
   (* keep *) wire [`DATA_LEN-1:0]   dmem_data;
   (* keep *) wire [`DATA_LEN-1:0]   dmem_wdata;
   (* keep *) wire [`ADDR_LEN-1:0]   dmem_addr;
   (* keep *) wire 		  dmem_we;
   (* keep *) wire [`DATA_LEN-1:0]   dmem_wdata_core;
   (* keep *) wire [`ADDR_LEN-1:0]   dmem_addr_core;
   (* keep *) wire 		  dmem_we_core;

   wire 		  utx_we;
   wire 		  finish_we;
   wire 		  ready_tx;
   wire 		  loaded;
   
   reg 			  prog_loading;
   wire [4*`INSN_LEN-1:0] prog_loaddata = 0;
   wire [`ADDR_LEN-1:0]   prog_loadaddr = 0;
   wire 		  prog_dmem_we = 0;
   wire 		  prog_imem_we = 0;

/*   
   assign utx_we = (dmem_we_core && (dmem_addr_core == 32'h0)) ? 1'b1 : 1'b0;
   assign finish_we = (dmem_we_core && (dmem_addr_core == 32'h8)) ? 1'b1 : 1'b0;
   
   always @ (posedge clk) begin
      if (!reset_x) begin
	 LED <= 0;
      end else if (utx_we) begin
	 LED <= {LED[7], dmem_wdata[6:0]};
      end else if (finish_we) begin
	 LED <= {1'b1, LED[6:0]};
      end
   end
*/
   always @ (posedge clk) begin
      if (!reset_x) begin
	      prog_loading <= 1'b1;
      end else begin
	      prog_loading <= 0;
      end
   end
/*   
   GEN_MMCM_DS genmmcmds
     (
      .CLK_P(CLK_P), 
      .CLK_N(CLK_N), 
      .RST_X_I(~RST_X_IN), 
      .CLK_O(clk), 
      .RST_X_O(reset_x)
      );
*/
   // EDIT: wire up the instruction to the new qed_instruction port
   pipeline pipe
     (
      .qed_instruction(qed_instruction),
      .qed_vld_out(qed_vld_out),
      .clk(clk),
      .reset(~reset_x),
      .pc(pc),
      .idata(idata),
      .dmem_wdata(dmem_wdata_core),
      .dmem_addr(dmem_addr_core),
      .dmem_we(dmem_we_core),
      .dmem_data(dmem_data),

      .gpr0_o(gpr0),
      .gpr1_o(gpr1),
      .gpr2_o(gpr2),
      .gpr3_o(gpr3),
      .gpr4_o(gpr4),
      .gpr5_o(gpr5),
      .gpr6_o(gpr6),
      .gpr7_o(gpr7),
      .gpr8_o(gpr8),
      .gpr9_o(gpr9),
      .gpr10_o(gpr10),
      .gpr11_o(gpr11),
      .gpr12_o(gpr12),
      .gpr13_o(gpr13),
      .gpr14_o(gpr14),
      .gpr15_o(gpr15),
      .gpr16_o(gpr16),
      .gpr17_o(gpr17),
      .gpr18_o(gpr18),
      .gpr19_o(gpr19),
      .gpr20_o(gpr20),
      .gpr21_o(gpr21),
      .gpr22_o(gpr22),
      .gpr23_o(gpr23),
      .gpr24_o(gpr24),
      .gpr25_o(gpr25),
      .gpr26_o(gpr26),
      .gpr27_o(gpr27),
      .gpr28_o(gpr28),
      .gpr29_o(gpr29),
      .gpr30_o(gpr30),
      .gpr31_o(gpr31),
      .arfwe1_o(arfwe1),
      .arfwe2_o(arfwe2),
      .dstarf1_o(dstarf1),
      .dstarf2_o(dstarf2),
      .stall_IF_o(stall_IF)
      // EDIT: END
      );

   assign dmem_addr = prog_loading ? prog_loadaddr : dmem_addr_core;
   assign dmem_we = prog_loading ? prog_dmem_we : dmem_we_core;
   assign dmem_wdata = prog_loading ? prog_loaddata[127:96] : dmem_wdata_core;
   dmem datamemory(
		   .clk(clk),
		   .addr({2'b0, dmem_addr[`ADDR_LEN-1:2]}),
		   .wdata(dmem_wdata),
		   .we(dmem_we),
		   .rdata(dmem_data)
		   );

   assign imem_addr = prog_loading ? prog_loadaddr[12:4] : pc[12:4];
   imem_ld instmemory(
            // EDIT: changed clk polarity
		      //.clk(~clk),
		      .clk(clk),
            // EDIT: END
            .addr(imem_addr),
		      .rdata(idata),
		      .wdata(prog_loaddata),
		      .we(prog_imem_we)
		      );
/*   
   SingleUartTx sutx
     (
      .CLK(clk),
      .RST_X(reset_x),
      .TXD(TXD),
      .ERR(),
      .DT01(dmem_wdata[7:0]),
      .WE01(utx_we)
      );

   PLOADER loader
     (
      .CLK(clk),
      .RST_X(reset_x),
      .RXD(RXD),
      .ADDR(prog_loadaddr),
      .DATA(prog_loaddata),
      .WE_32(prog_dmem_we),
      .WE_128(prog_imem_we),
      .DONE(loaded)
      );
*/   
endmodule // top

   
