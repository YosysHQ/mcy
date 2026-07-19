module properties #(
    parameter REG_SEL = 5,
    parameter DATA_LEN = 32
)
(
    input clk,
    input rst,
    input [DATA_LEN-1:0] reg_val0,
    input [DATA_LEN-1:0] reg_val1,
    input [DATA_LEN-1:0] reg_val2,
    input [DATA_LEN-1:0] reg_val3,
    input [DATA_LEN-1:0] reg_val4,
    input [DATA_LEN-1:0] reg_val5,
    input [DATA_LEN-1:0] reg_val6,
    input [DATA_LEN-1:0] reg_val7,
    input [DATA_LEN-1:0] reg_val8,
    input [DATA_LEN-1:0] reg_val9,
    input [DATA_LEN-1:0] reg_val10,
    input [DATA_LEN-1:0] reg_val11,
    input [DATA_LEN-1:0] reg_val12,
    input [DATA_LEN-1:0] reg_val13,
    input [DATA_LEN-1:0] reg_val14,
    input [DATA_LEN-1:0] reg_val15,
    input [DATA_LEN-1:0] reg_val16,
    input [DATA_LEN-1:0] reg_val17,
    input [DATA_LEN-1:0] reg_val18,
    input [DATA_LEN-1:0] reg_val19,
    input [DATA_LEN-1:0] reg_val20,
    input [DATA_LEN-1:0] reg_val21,
    input [DATA_LEN-1:0] reg_val22,
    input [DATA_LEN-1:0] reg_val23,
    input [DATA_LEN-1:0] reg_val24,
    input [DATA_LEN-1:0] reg_val25,
    input [DATA_LEN-1:0] reg_val26,
    input [DATA_LEN-1:0] reg_val27,
    input [DATA_LEN-1:0] reg_val28,
    input [DATA_LEN-1:0] reg_val29,
    input [DATA_LEN-1:0] reg_val30,
    input [DATA_LEN-1:0] reg_val31,
    input arfwe1,
    input arfwe2,
    input [REG_SEL-1:0] dstarf1,
    input [REG_SEL-1:0] dstarf2
);

   // embed the assumption that there's no reset into the generated BTOR2
   // we will run the reset sequence in Yosys before generating the BTOR2
   // it will complain about the violated assumption, but should work
   // fine otherwise
   // this is a pos-edge reset, so we assume it is 0.
    always @* begin
        no_reset: assume(rst == 1'b0);
    end

    // Insert the ready logic: tracks number of committed instructions 
    (* keep *) wire qed_ready;
    (* keep *) reg [15:0] num_orig_insts;
    (* keep *) reg [15:0] num_dup_insts;
    (* keep *) integer timecount; 
    (* keep *) wire timeout; //if timeout, force to quit
    wire [1:0] num_orig_commits;
    wire [1:0] num_dup_commits;

    // Instruction with destination register as 5'b0 is a NOP so ignore those
   assign num_orig_commits = ((arfwe1 == 1)&&(dstarf1 < 16)&&(dstarf1 != 5'b0)
			      &&(arfwe2 == 1)&&(dstarf2 < 16)&&(dstarf2 != 5'b0)) ? 2'b10 :
			     ((((arfwe1 == 1)&&(dstarf1 < 16)&&(dstarf1 != 5'b0)
			       &&((arfwe2 != 1)||(dstarf2 >= 16)||(dstarf2 == 5'b0)))
			      ||((arfwe2 == 1)&&(dstarf2 < 16)&&(dstarf2 != 5'b0)
				 &&((arfwe1 != 1)||(dstarf1 >= 16)||(dstarf1 == 5'b0)))) ? 2'b01 : 2'b00) ;


   // When destination register is 5'b0, it remains the same for both original and duplicate
   assign num_dup_commits = ((arfwe1 == 1)&&(dstarf1 >= 16)
			      &&(arfwe2 == 1)&&(dstarf2 >= 16)) ? 2'b10 :
			     ((((arfwe1 == 1)&&(dstarf1 >= 16)
			       &&((arfwe2 != 1)||(dstarf2 < 16)))
			      ||((arfwe2 == 1)&&(dstarf2 >= 16)
				 &&((arfwe1 != 1)||(dstarf1 < 16)))) ? 2'b01 : 2'b00) ;

    always @(posedge clk) begin
		if (rst) begin
	   		num_orig_insts <= 16'b0;
	   		num_dup_insts <= 16'b0;
		end else begin
	   		num_orig_insts <= num_orig_insts + {14'b0,num_orig_commits};
	   		num_dup_insts <= num_dup_insts + {14'b0,num_dup_commits};
		end
    end

    always @(posedge clk) begin
        if(rst) begin
            timecount <= 0;
        end 
        else begin
            timecount <= timecount + 1;
        end
    end

    assign timeout = (timecount >= 12);
    //assign qed_ready = timeout; // For debug
    assign qed_ready = (num_orig_insts == num_dup_insts);

    always @(posedge clk) begin
        if (qed_ready) begin
            qed_consistency: assert property ( (reg_val0 == 0) &&
                                 (reg_val1 == reg_val17) && (reg_val2 == reg_val18) &&
	                             (reg_val3 == reg_val19) && (reg_val4 == reg_val20) &&
							     (reg_val5 == reg_val21) && (reg_val6 == reg_val22) &&
							     (reg_val7 == reg_val23) && (reg_val8 == reg_val24) &&
							     (reg_val9 == reg_val25) && (reg_val10 == reg_val26) &&
							     (reg_val11 == reg_val27) && (reg_val12 == reg_val28) &&
                                 (reg_val13 == reg_val29) && (reg_val14 == reg_val30) &&
                                 (reg_val15 == reg_val31) 
	   		                 );
            
	    end
    end

    
endmodule

`default_nettype wire