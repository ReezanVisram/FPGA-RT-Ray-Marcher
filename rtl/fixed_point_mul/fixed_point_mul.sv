`default_nettype none

// Fixed point multiplier
// The INT_WIDTH includes 1 sign bit
module fixed_point_mul #(
  parameter int INT_WIDTH  = 16,
  parameter int FRAC_WIDTH = 16,
  parameter int DATA_WIDTH = INT_WIDTH + FRAC_WIDTH
) (
  input  wire  signed [DATA_WIDTH-1:0] a_i,
  input  wire  signed [DATA_WIDTH-1:0] b_i,
  output logic signed [DATA_WIDTH-1:0] res_o
);
  localparam int MAX_VAL = (1 << (DATA_WIDTH-1)) - 1;
  localparam int MIN_VAL = -(1 << (DATA_WIDTH-1));

  logic signed [2*DATA_WIDTH-1:0] temp1_c;
  logic signed [2*DATA_WIDTH-1:0] temp2_c;

  always_comb begin
    temp1_c = a_i * b_i + (1 << (FRAC_WIDTH - 1));
    temp2_c = temp1_c >>> FRAC_WIDTH;

    /* verilator lint_off WIDTHEXPAND */
    if (temp2_c > MAX_VAL) begin
      res_o = MAX_VAL;
    end else if (temp2_c < MIN_VAL) begin
      res_o = MIN_VAL;
    end else begin
      res_o = temp2_c[DATA_WIDTH-1:0];
    end
    /* verilator lint_on WIDTHEXPAND */
  end

endmodule

`default_nettype wire
