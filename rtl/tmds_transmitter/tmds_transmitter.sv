`default_nettype none
/* TMDS Transmitter
* It uses an AXI4 interface to read from the framebuffers (whichever one is not being written to at the moment)
* It reads 32 bits at a time from the framebuffer ([23:16] R, [15:8] G, [7:0] B)
* It encodes them using the tmds_encoder
* It outputs the encoded data to be put into the SERDES to be output on the wire
  * The SERDES is outside this module so it can be individually tested
* */
module tmds_transmitter (
  input wire clk_i,
  input wire resetn_i,

  output logic m_axi_awaddr,

);

endmodule
`default_nettype wire
