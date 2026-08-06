module miter #(
        parameter DATA_LEN = 32,
        parameter INST_LEN = 32,
        parameter REG_SEL = 5,
        parameter REG_NUM = 32
)
(
        input din_clk,
        input din_rst_n,
        input ref_din_exec_dup,
        input uut_din_exec_dup,
        input [INST_LEN-1:0] ref_din_instruction,
        input [INST_LEN-1:0] uut_din_instruction
);
        wire [DATA_LEN-1:0] ref_dout_gpr0;
        wire [DATA_LEN-1:0] ref_dout_gpr1;
        wire [DATA_LEN-1:0] ref_dout_gpr2;
        wire [DATA_LEN-1:0] ref_dout_gpr3;
        wire [DATA_LEN-1:0] ref_dout_gpr4;
        wire [DATA_LEN-1:0] ref_dout_gpr5;
        wire [DATA_LEN-1:0] ref_dout_gpr6;
        wire [DATA_LEN-1:0] ref_dout_gpr7;
        wire [DATA_LEN-1:0] ref_dout_gpr8;
        wire [DATA_LEN-1:0] ref_dout_gpr9;
        wire [DATA_LEN-1:0] ref_dout_gpr10;
        wire [DATA_LEN-1:0] ref_dout_gpr11;
        wire [DATA_LEN-1:0] ref_dout_gpr12;
        wire [DATA_LEN-1:0] ref_dout_gpr13;
        wire [DATA_LEN-1:0] ref_dout_gpr14;
        wire [DATA_LEN-1:0] ref_dout_gpr15;
        wire [DATA_LEN-1:0] ref_dout_gpr16;
        wire [DATA_LEN-1:0] ref_dout_gpr17;
        wire [DATA_LEN-1:0] ref_dout_gpr18;
        wire [DATA_LEN-1:0] ref_dout_gpr19;
        wire [DATA_LEN-1:0] ref_dout_gpr20;
        wire [DATA_LEN-1:0] ref_dout_gpr21;
        wire [DATA_LEN-1:0] ref_dout_gpr22;
        wire [DATA_LEN-1:0] ref_dout_gpr23;
        wire [DATA_LEN-1:0] ref_dout_gpr24;
        wire [DATA_LEN-1:0] ref_dout_gpr25;
        wire [DATA_LEN-1:0] ref_dout_gpr26;
        wire [DATA_LEN-1:0] ref_dout_gpr27;
        wire [DATA_LEN-1:0] ref_dout_gpr28;
        wire [DATA_LEN-1:0] ref_dout_gpr29;
        wire [DATA_LEN-1:0] ref_dout_gpr30;
        wire [DATA_LEN-1:0] ref_dout_gpr31;
        wire ref_dout_arfwe1;
        wire ref_dout_arfwe2;
        wire [REG_SEL-1:0] ref_dout_dstarf1;
        wire [REG_SEL-1:0] ref_dout_dstarf2;
        wire ref_dout_stall_IF; 

        wire [DATA_LEN-1:0] uut_dout_gpr0;
        wire [DATA_LEN-1:0] uut_dout_gpr1;
        wire [DATA_LEN-1:0] uut_dout_gpr2;
        wire [DATA_LEN-1:0] uut_dout_gpr3;
        wire [DATA_LEN-1:0] uut_dout_gpr4;
        wire [DATA_LEN-1:0] uut_dout_gpr5;
        wire [DATA_LEN-1:0] uut_dout_gpr6;
        wire [DATA_LEN-1:0] uut_dout_gpr7;
        wire [DATA_LEN-1:0] uut_dout_gpr8;
        wire [DATA_LEN-1:0] uut_dout_gpr9;
        wire [DATA_LEN-1:0] uut_dout_gpr10;
        wire [DATA_LEN-1:0] uut_dout_gpr11;
        wire [DATA_LEN-1:0] uut_dout_gpr12;
        wire [DATA_LEN-1:0] uut_dout_gpr13;
        wire [DATA_LEN-1:0] uut_dout_gpr14;
        wire [DATA_LEN-1:0] uut_dout_gpr15;
        wire [DATA_LEN-1:0] uut_dout_gpr16;
        wire [DATA_LEN-1:0] uut_dout_gpr17;
        wire [DATA_LEN-1:0] uut_dout_gpr18;
        wire [DATA_LEN-1:0] uut_dout_gpr19;
        wire [DATA_LEN-1:0] uut_dout_gpr20;
        wire [DATA_LEN-1:0] uut_dout_gpr21;
        wire [DATA_LEN-1:0] uut_dout_gpr22;
        wire [DATA_LEN-1:0] uut_dout_gpr23;
        wire [DATA_LEN-1:0] uut_dout_gpr24;
        wire [DATA_LEN-1:0] uut_dout_gpr25;
        wire [DATA_LEN-1:0] uut_dout_gpr26;
        wire [DATA_LEN-1:0] uut_dout_gpr27;
        wire [DATA_LEN-1:0] uut_dout_gpr28;
        wire [DATA_LEN-1:0] uut_dout_gpr29;
        wire [DATA_LEN-1:0] uut_dout_gpr30;
        wire [DATA_LEN-1:0] uut_dout_gpr31;
        wire uut_dout_arfwe1;
        wire uut_dout_arfwe2;
        wire [REG_SEL-1:0] uut_dout_dstarf1;
        wire [REG_SEL-1:0] uut_dout_dstarf2;
        wire uut_dout_stall_IF;

        //initial assume (din_rst_n == 1'b0);

        // Use the inst_constraint module to constrain instruction to be
        // a valid instruction from the ISA
        inst_constraints ic (
		.clk         (din_clk),
                .instruction (ref_din_instruction)
	);

        always @ (posedge din_clk) begin
                assume_same_instruction: assume property(ref_din_instruction == uut_din_instruction); 
        end        

        wire [INST_LEN-1:0] ref_qed_instruction;
	(* kepp *) wire ref_qed_vld_out;
        wire [INST_LEN-1:0] uut_qed_instruction;
	(* keep *) wire uut_qed_vld_out;

        // QED module: transforms the original instruction to duplicate instruction
	qed ref_qed (
		//Inputs
		.clk(din_clk),
		.rst(~din_rst_n),
		.ena(1'b1),
		.exec_dup(ref_din_exec_dup),
		.ifu_qed_instruction(ref_din_instruction),
		.stall_IF(ref_dout_stall_IF),

		//Outputs
		.qed_ifu_instruction(ref_qed_instruction),
		.vld_out(ref_qed_vld_out)
	);

        qed uut_qed (
		//Inputs
		.clk(din_clk),
		.rst(~din_rst_n),
		.ena(1'b1),
		.exec_dup(uut_din_exec_dup),
		.ifu_qed_instruction(uut_din_instruction),
		.stall_IF(uut_dout_stall_IF),

		//Outputs
		.qed_ifu_instruction(uut_qed_instruction),
		.vld_out(uut_qed_vld_out)
	);

        // disable mutation
        top_ridecore ref (
                .mutsel         (1'b0),
                .clk            (din_clk),
		.reset_x  	(din_rst_n),
		.qed_instruction(ref_qed_instruction),
		.qed_vld_out    (ref_qed_vld_out),

                .gpr0           (ref_dout_gpr0),
                .gpr1           (ref_dout_gpr1),
                .gpr2           (ref_dout_gpr2),
                .gpr3           (ref_dout_gpr3),
                .gpr4           (ref_dout_gpr4),
                .gpr5           (ref_dout_gpr5),
                .gpr6           (ref_dout_gpr6),
                .gpr7           (ref_dout_gpr7),
                .gpr8           (ref_dout_gpr8),
                .gpr9           (ref_dout_gpr9),
                .gpr10          (ref_dout_gpr10),
                .gpr11          (ref_dout_gpr11),
                .gpr12          (ref_dout_gpr12),
                .gpr13          (ref_dout_gpr13),
                .gpr14          (ref_dout_gpr14),
                .gpr15          (ref_dout_gpr15),
                .gpr16          (ref_dout_gpr16),
                .gpr17          (ref_dout_gpr17),
                .gpr18          (ref_dout_gpr18),
                .gpr19          (ref_dout_gpr19),
                .gpr20          (ref_dout_gpr20),
                .gpr21          (ref_dout_gpr21),
                .gpr22          (ref_dout_gpr22),
                .gpr23          (ref_dout_gpr23),
                .gpr24          (ref_dout_gpr24),
                .gpr25          (ref_dout_gpr25),
                .gpr26          (ref_dout_gpr26),
                .gpr27          (ref_dout_gpr27),
                .gpr28          (ref_dout_gpr28),
                .gpr29          (ref_dout_gpr29),
                .gpr30          (ref_dout_gpr30),
                .gpr31          (ref_dout_gpr31),

                .arfwe1         (ref_dout_arfwe1),
		.arfwe2         (ref_dout_arfwe2),
		.dstarf1        (ref_dout_dstarf1),
		.dstarf2        (ref_dout_dstarf2),
		.stall_IF       (ref_dout_stall_IF)

        );

        // enable mutation
        top_ridecore uut (
                .mutsel         (1'b1),
                .clk            (din_clk),          
                .reset_x  	(din_rst_n),
		.qed_instruction(uut_qed_instruction),
		.qed_vld_out    (uut_qed_vld_out),

                .gpr0           (uut_dout_gpr0),
                .gpr1           (uut_dout_gpr1),
                .gpr2           (uut_dout_gpr2),
                .gpr3           (uut_dout_gpr3),
                .gpr4           (uut_dout_gpr4),
                .gpr5           (uut_dout_gpr5),
                .gpr6           (uut_dout_gpr6),
                .gpr7           (uut_dout_gpr7),
                .gpr8           (uut_dout_gpr8),
                .gpr9           (uut_dout_gpr9),
                .gpr10          (uut_dout_gpr10),
                .gpr11          (uut_dout_gpr11),
                .gpr12          (uut_dout_gpr12),
                .gpr13          (uut_dout_gpr13),
                .gpr14          (uut_dout_gpr14),
                .gpr15          (uut_dout_gpr15),
                .gpr16          (uut_dout_gpr16),
                .gpr17          (uut_dout_gpr17),
                .gpr18          (uut_dout_gpr18),
                .gpr19          (uut_dout_gpr19),
                .gpr20          (uut_dout_gpr20),
                .gpr21          (uut_dout_gpr21),
                .gpr22          (uut_dout_gpr22),
                .gpr23          (uut_dout_gpr23),
                .gpr24          (uut_dout_gpr24),
                .gpr25          (uut_dout_gpr25),
                .gpr26          (uut_dout_gpr26),
                .gpr27          (uut_dout_gpr27),
                .gpr28          (uut_dout_gpr28),
                .gpr29          (uut_dout_gpr29),
                .gpr30          (uut_dout_gpr30),
                .gpr31          (uut_dout_gpr31),

                .arfwe1         (uut_dout_arfwe1),
		.arfwe2         (uut_dout_arfwe2),
		.dstarf1        (uut_dout_dstarf1),
		.dstarf2        (uut_dout_dstarf2),
		.stall_IF       (uut_dout_stall_IF)
        );

        reg [15:0] ref_dout_comnum;
        reg [15:0] uut_dout_comnum;
        wire [1:0] ref_dout_commits;
        wire [1:0] uut_dout_commits;

        assign ref_dout_commits = (ref_dout_arfwe1 && ref_dout_arfwe2) ? 2'b10: 
                                  (ref_dout_arfwe1 || ref_dout_arfwe2) ? 2'b01:
                                                                         2'b00;

        assign uut_dout_commits = (uut_dout_arfwe1 && uut_dout_arfwe2) ? 2'b10: 
                                  (uut_dout_arfwe1 || uut_dout_arfwe2) ? 2'b01:
                                                                         2'b00;                                                                 


        always @(posedge din_clk) begin
                if (~din_rst_n) begin
                        ref_dout_comnum <= 0;
                        uut_dout_comnum <= 0;
                end else begin
                        ref_dout_comnum <= ref_dout_comnum + {14'b0, ref_dout_commits};
                        uut_dout_comnum <= uut_dout_comnum + {14'b0, uut_dout_commits};
                end
        end

        always @(posedge din_clk) begin
                if (din_rst_n) begin
                   assume (ref_din_exec_dup == uut_din_exec_dup);
                   if(ref_dout_comnum == uut_dout_comnum) begin
                                assert (ref_dout_gpr0 == uut_dout_gpr0);
                                assert (ref_dout_gpr1 == uut_dout_gpr1);
                                assert (ref_dout_gpr2 == uut_dout_gpr2);
                                assert (ref_dout_gpr3 == uut_dout_gpr3);
                                assert (ref_dout_gpr4 == uut_dout_gpr4);
                                assert (ref_dout_gpr5 == uut_dout_gpr5);
                                assert (ref_dout_gpr6 == uut_dout_gpr6);
                                assert (ref_dout_gpr7 == uut_dout_gpr7);
                                assert (ref_dout_gpr8 == uut_dout_gpr8);
                                assert (ref_dout_gpr9 == uut_dout_gpr9);
                                assert (ref_dout_gpr10 == uut_dout_gpr10);
                                assert (ref_dout_gpr11 == uut_dout_gpr11);
                                assert (ref_dout_gpr12 == uut_dout_gpr12);
                                assert (ref_dout_gpr13 == uut_dout_gpr13);
                                assert (ref_dout_gpr14 == uut_dout_gpr14);
                                assert (ref_dout_gpr15 == uut_dout_gpr15);
                                assert (ref_dout_gpr16 == uut_dout_gpr16);
                                assert (ref_dout_gpr17 == uut_dout_gpr17);
                                assert (ref_dout_gpr18 == uut_dout_gpr18);
                                assert (ref_dout_gpr19 == uut_dout_gpr19);
                                assert (ref_dout_gpr20 == uut_dout_gpr20);
                                assert (ref_dout_gpr21 == uut_dout_gpr21);
                                assert (ref_dout_gpr22 == uut_dout_gpr22);
                                assert (ref_dout_gpr23 == uut_dout_gpr23);
                                assert (ref_dout_gpr24 == uut_dout_gpr24);
                                assert (ref_dout_gpr25 == uut_dout_gpr25);
                                assert (ref_dout_gpr26 == uut_dout_gpr26);
                                assert (ref_dout_gpr27 == uut_dout_gpr27);
                                assert (ref_dout_gpr28 == uut_dout_gpr28);
                                assert (ref_dout_gpr29 == uut_dout_gpr29);
                                assert (ref_dout_gpr30 == uut_dout_gpr30);
                                assert (ref_dout_gpr31 == uut_dout_gpr31);
                        end
                end
        end

        // For debug
        (* keep *) integer timecount; 
        (* keep *) wire timeout; //if timeout, force to quit
        always @(posedge din_clk) begin
                if(~din_rst_n) begin
                        timecount <= 0;
                end 
                else begin
                        timecount <= timecount + 1;
                end
        end

        assign timeout = (timecount > 8);
        
        // always @(posedge din_clk) begin
        //         if (timeout) begin
        //                 debug: assert property (1'b0);
        //         end
        // end


endmodule

`default_nettype wire