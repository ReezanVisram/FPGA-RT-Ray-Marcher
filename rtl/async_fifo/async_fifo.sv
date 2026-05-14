`default_nettype none
/* ASYNC FIFO without First Word Fall Through */
/* Each reset should be synchronous to the clock domain it is associated with */
module async_fifo #(
  parameter int DATA_WIDTH = 32,
  parameter int DEPTH = 1024,
  parameter int ALMOST_FULL_THRESHOLD = 4,
  parameter int ALMOST_EMPTY_THRESHOLD = 4
) (
  // Write side
  input wire a_clk_i,
  input wire a_resetn_i,
  input wire [DATA_WIDTH-1:0] a_data_i,
  input wire a_push_i,
  output logic a_almost_full_o,
  output logic a_full_o,

  // Read side
  input wire b_clk_i,
  input wire b_resetn_i,
  output logic [DATA_WIDTH-1:0] b_data_o,
  input wire b_pop_i,
  output logic b_almost_empty_o,
  output logic b_empty_o
);

  if (!$onehot(DEPTH)) begin : g_depth_power_of_2_assertion
    $error("DEPTH must be a power-of-2, currently DEPTH=%d", DEPTH);
  end

  localparam int PTR_W = $clog2(DEPTH);

  /* Extra bit to detect wrapping and thus empty/full */
  logic [PTR_W:0] a_wr_ptr_gray_r;
  logic [PTR_W:0] b_rd_ptr_gray_r;

  (* ASYNC_REG = "true" *) logic [PTR_W:0] a_to_b_wr_ptr_gray_sync_r1, a_to_b_wr_ptr_gray_sync_r0;
  (* ASYNC_REG = "true" *) logic [PTR_W:0] b_to_a_rd_ptr_gray_sync_r1, b_to_a_rd_ptr_gray_sync_r0;

  logic [PTR_W:0] a_wr_ptr_bin_r;
  logic [PTR_W:0] a_wr_ptr_gray_next_c;
  logic [PTR_W:0] a_wr_ptr_bin_next_c;

  logic [PTR_W:0] b_rd_ptr_bin_r;
  logic [PTR_W:0] b_rd_ptr_gray_next_c;
  logic [PTR_W:0] b_rd_ptr_bin_next_c;

  /* Actual addresses used to address BRAM */
  logic [PTR_W-1:0] a_wr_addr_c;
  logic [PTR_W-1:0] b_rd_addr_c;

  logic a_full_c;
  logic b_empty_c;

  /* verilator lint_off UNUSEDSIGNAL */
  logic [PTR_W:0] b_wr_ptr_bin_sync_c;
  logic [PTR_W:0] a_rd_ptr_bin_sync_c;

  logic [PTR_W:0] a_wr_ptr_bin_almost_full_r;
  logic [PTR_W:0] b_rd_ptr_bin_almost_empty_r;
  /* verilator lint_on UNUSEDSIGNAL */

  logic a_almost_full_c;
  logic b_almost_empty_c;

  always_ff @(posedge a_clk_i) begin
    if (!a_resetn_i) begin
      a_wr_ptr_gray_r <= '0;
      a_wr_ptr_bin_r <= '0;
      b_to_a_rd_ptr_gray_sync_r0 <= '0;
      b_to_a_rd_ptr_gray_sync_r1 <= '0;
      a_full_o <= 1'b0;
      a_almost_full_o <= 1'b0;
      /* verilator lint_off WIDTHTRUNC */
      a_wr_ptr_bin_almost_full_r <= ALMOST_FULL_THRESHOLD;
      /* verilator lint_on WIDTHTRUNC */
    end else begin
      a_wr_ptr_gray_r <= a_wr_ptr_gray_next_c;
      a_wr_ptr_bin_r <= a_wr_ptr_bin_next_c;
      b_to_a_rd_ptr_gray_sync_r0 <= b_rd_ptr_gray_r;
      b_to_a_rd_ptr_gray_sync_r1 <= b_to_a_rd_ptr_gray_sync_r0;
      a_full_o <= a_full_c;
      a_almost_full_o <= a_almost_full_c | a_full_c;
      /* verilator lint_off WIDTHEXPAND */
      /* verilator lint_off WIDTHTRUNC */
      a_wr_ptr_bin_almost_full_r <= a_wr_ptr_bin_next_c + ALMOST_FULL_THRESHOLD;
      /* verilator lint_on WIDTHTRUNC */
      /* verilator lint_on WIDTHEXPAND */
    end
  end

  /* verilator lint_off WIDTHEXPAND */
  assign a_wr_addr_c = a_wr_ptr_bin_r[PTR_W-1:0];
  assign a_wr_ptr_bin_next_c = a_wr_ptr_bin_r + (a_push_i & ~a_full_o);
  assign a_wr_ptr_gray_next_c = (a_wr_ptr_bin_next_c >> 1) ^ (a_wr_ptr_bin_next_c);
  assign a_full_c = a_wr_ptr_gray_next_c == {~b_to_a_rd_ptr_gray_sync_r1[PTR_W:PTR_W-1], b_to_a_rd_ptr_gray_sync_r1[PTR_W-2:0]};
  assign a_rd_ptr_bin_sync_c = gray2bin(b_to_a_rd_ptr_gray_sync_r1);
  assign a_almost_full_c = (a_wr_ptr_bin_almost_full_r[PTR_W-1:0] - a_rd_ptr_bin_sync_c[PTR_W-1:0]) < ALMOST_FULL_THRESHOLD;
  /* verilator lint_on WIDTHEXPAND */

  always_ff @(posedge b_clk_i) begin
    if (!b_resetn_i) begin
      b_rd_ptr_gray_r <= '0;
      b_rd_ptr_bin_r <= '0;
      a_to_b_wr_ptr_gray_sync_r0 <= '0;
      a_to_b_wr_ptr_gray_sync_r1 <= '0;
      b_empty_o <= 1'b1;
      b_almost_empty_o <= 1'b1;
      /* verilator lint_off WIDTHTRUNC */
      b_rd_ptr_bin_almost_empty_r <= ALMOST_EMPTY_THRESHOLD;
      /* verilator lint_on WIDTHTRUNC */
    end else begin
      b_rd_ptr_gray_r <= b_rd_ptr_gray_next_c;
      b_rd_ptr_bin_r <= b_rd_ptr_bin_next_c;
      a_to_b_wr_ptr_gray_sync_r0 <= a_wr_ptr_gray_r;
      a_to_b_wr_ptr_gray_sync_r1 <= a_to_b_wr_ptr_gray_sync_r0;
      b_empty_o <= b_empty_c;
      b_almost_empty_o <= b_almost_empty_c | b_empty_c;
      /* verilator lint_off WIDTHEXPAND */
      /* verilator lint_off WIDTHTRUNC */
      b_rd_ptr_bin_almost_empty_r <= b_rd_ptr_bin_next_c + ALMOST_EMPTY_THRESHOLD;
      /* verilator lint_on WIDTHTRUNC */
      /* verilator lint_on WIDTHEXPAND */
    end
  end

  /* verilator lint_off WIDTHEXPAND */
  assign b_rd_addr_c = b_rd_ptr_bin_r[PTR_W-1:0];
  assign b_rd_ptr_bin_next_c = b_rd_ptr_bin_r + (b_pop_i & ~b_empty_o);
  assign b_rd_ptr_gray_next_c = (b_rd_ptr_bin_next_c >> 1) ^ b_rd_ptr_bin_next_c;
  assign b_empty_c = b_rd_ptr_gray_next_c == a_to_b_wr_ptr_gray_sync_r1;
  assign b_wr_ptr_bin_sync_c = gray2bin(a_to_b_wr_ptr_gray_sync_r1);
  assign b_almost_empty_c = (b_rd_ptr_bin_almost_empty_r[PTR_W-1:0] - b_wr_ptr_bin_sync_c[PTR_W-1:0]) < ALMOST_EMPTY_THRESHOLD;
  /* verilator lint_on WIDTHEXPAND */

  bram_dp #(
    .DATA_WIDTH(DATA_WIDTH),
    .DEPTH(DEPTH)
  ) fifo_mem (
    .a_clk_i ( a_clk_i ),
    .a_wr_enable_i ( a_push_i & ~a_full_o ),
    .a_addr_i ( a_wr_addr_c ),
    .a_data_i ( a_data_i ),

    .b_clk_i ( b_clk_i ),
    .b_rd_enable_i ( b_pop_i & ~b_empty_o ),
    .b_addr_i ( b_rd_addr_c ),
    .b_data_o ( b_data_o )
  );

  function automatic logic [PTR_W:0] gray2bin(input logic [PTR_W:0] g);
    logic [PTR_W:0] b;
    b[PTR_W] = g[PTR_W];
    for (int i = PTR_W-1; i >= 0; i--) begin
      b[i] = g[i] ^ b[i+1];
    end

    return b;
  endfunction
endmodule

`default_nettype wire
