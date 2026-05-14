`default_nettype none
/* TMDS Encoder for DVI Output */

module tmds_encoder(
  input  wire         clk_i,
  input  wire         resetn_i,

  input  wire  [7:0] data_i,
  input  wire        ctrl0_i,
  input  wire        ctrl1_i,
  input  wire        data_enable_i,

  output logic [9:0] data_o,
  /* This should only be consumed in sim */
  output logic       valid_o
);

  logic signed [4:0] disparity_r;
  logic signed [4:0] disparity_c;

  logic [3:0] num_data_ones;
  logic [3:0] num_q_ones;
  logic [3:0] num_q_zeroes_c;

  logic signed [4:0] num_q_ones_signed_c;
  logic signed [4:0] num_q_zeroes_signed_c;

  logic [8:0] q_m_c;
  logic [9:0] q_o_c;

  count_ones #(.DATA_WIDTH(8)) count_ones_data_inst (
    .a_i   (data_i),
    .res_o (num_data_ones)
  );

  count_ones #(.DATA_WIDTH(8)) count_ones_q_inst (
    .a_i(q_m_c[7:0]),
    .res_o(num_q_ones)
  );

  assign num_q_zeroes_c = 8 - num_q_ones;
  assign num_q_ones_signed_c = $signed({1'b0, num_q_ones});
  assign num_q_zeroes_signed_c = $signed({1'b0, num_q_zeroes_c});

  always_ff @(posedge clk_i) begin
    if (~resetn_i) begin
      disparity_r <= '0;
      valid_o     <= 1'b0;
    end else begin
      disparity_r <= disparity_c;
      data_o      <= q_o_c;
      valid_o     <= 1'b1;
    end
  end

  always_comb begin
    if (num_data_ones > 4'd4 || (num_data_ones == 4'd4 && data_i[0] == 1'b0)) begin
      q_m_c[0] = data_i[0];
      q_m_c[1] = ~(q_m_c[0] ^ data_i[1]);
      q_m_c[2] = ~(q_m_c[1] ^ data_i[2]);
      q_m_c[3] = ~(q_m_c[2] ^ data_i[3]);
      q_m_c[4] = ~(q_m_c[3] ^ data_i[4]);
      q_m_c[5] = ~(q_m_c[4] ^ data_i[5]);
      q_m_c[6] = ~(q_m_c[5] ^ data_i[6]);
      q_m_c[7] = ~(q_m_c[6] ^ data_i[7]);
      q_m_c[8] = 1'b0;
    end else begin
      q_m_c[0] = data_i[0];
      q_m_c[1] = q_m_c[0] ^ data_i[1];
      q_m_c[2] = q_m_c[1] ^ data_i[2];
      q_m_c[3] = q_m_c[2] ^ data_i[3];
      q_m_c[4] = q_m_c[3] ^ data_i[4];
      q_m_c[5] = q_m_c[4] ^ data_i[5];
      q_m_c[6] = q_m_c[5] ^ data_i[6];
      q_m_c[7] = q_m_c[6] ^ data_i[7];
      q_m_c[8] = 1'b1;
    end

    if (data_enable_i) begin
      if (disparity_r == 0 || num_q_ones == 4'd4) begin
        q_o_c[9] = ~q_m_c[8];
        q_o_c[8] = q_m_c[8];
        q_o_c[7:0] = (q_m_c[8]) ? q_m_c[7:0] : ~q_m_c[7:0];
        if (!q_m_c[8]) begin
          disparity_c = disparity_r + (num_q_zeroes_signed_c - num_q_ones_signed_c);
        end else begin
          disparity_c = disparity_r + (num_q_ones_signed_c - num_q_zeroes_signed_c);
        end
      end else begin
        if ((disparity_r > 0 && num_q_ones > 4'd4) || (disparity_r < 0 && num_q_ones < 4'd4)) begin
          q_o_c[9] = 1'b1;
          q_o_c[8] = q_m_c[8];
          q_o_c[7:0] = ~q_m_c[7:0];
          disparity_c = disparity_r + (q_m_c[8] ? 5'sd2 : 5'sd0) + (num_q_zeroes_signed_c - num_q_ones_signed_c);
        end else begin
          q_o_c[9] = 1'b0;
          q_o_c[8] = q_m_c[8];
          q_o_c[7:0] = q_m_c[7:0];
          disparity_c = disparity_r - (~q_m_c[8] ? 5'sd2 : 5'sd0) + (num_q_ones_signed_c - num_q_zeroes_signed_c);
        end
      end
    end else begin
      disparity_c = '0;
      unique case ({ctrl1_i, ctrl0_i})
        2'b00: q_o_c = 10'b1101010100;
        2'b01: q_o_c = 10'b0010101011;
        2'b10: q_o_c = 10'b0101010100;
        2'b11: q_o_c = 10'b1010101011;
      endcase
    end
  end
endmodule
`default_nettype wire
