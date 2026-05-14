`default_nettype none
module count_ones #(
  parameter int DATA_WIDTH = 32
)(
  input  wire  [          DATA_WIDTH-1:0] a_i,
  output logic [$clog2(DATA_WIDTH+1)-1:0] res_o
);

  always_comb begin
    res_o = '0;
    for (int i = 0; i < DATA_WIDTH; i++) begin
      /* verilator lint_off WIDTHEXPAND */
      res_o += a_i[i];
      /* verilator lint_on WIDTHEXPAND */
    end
  end

endmodule
`default_nettype wire
