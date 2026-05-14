`default_nettype none
module bram_dp #(
  parameter int DATA_WIDTH = 32,
  parameter int DEPTH = 65536,
  parameter int ADDRESS_WIDTH = $clog2(DEPTH)
) (
  input  wire                      a_clk_i,
  input  wire                      a_wr_enable_i,
  input  wire  [ADDRESS_WIDTH-1:0] a_addr_i,
  input  wire  [   DATA_WIDTH-1:0] a_data_i,

  input  wire                      b_clk_i,
  input  wire                      b_rd_enable_i,
  input  wire  [ADDRESS_WIDTH-1:0] b_addr_i,
  output logic [   DATA_WIDTH-1:0] b_data_o
);

  logic [DATA_WIDTH-1:0] ram [DEPTH];

  always_ff @(posedge a_clk_i) begin
    if (a_wr_enable_i) begin
      ram[a_addr_i] <= a_data_i;
    end
  end

  always_ff @(posedge b_clk_i) begin
    if (b_rd_enable_i) begin
      b_data_o <= ram[b_addr_i];
    end
  end
endmodule
`default_nettype wire
