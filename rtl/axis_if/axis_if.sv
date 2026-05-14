`default_nettype none
interface axis_if #(
  parameter int TDATA_WIDTH = 64,
  parameter int TID_WIDTH   = 0,
  parameter int TDEST_WIDTH = 0,
  parameter int TUSER_WIDTH = 0
) (
  input wire aclk,
  input wire aresetn
);
  localparam int TID_W = (TID_WIDTH == 0) ? 1 : TID_WIDTH;
  localparam int TDEST_W = (TDEST_WIDTH == 0) ? 1 : TDEST_W;
  localparam int TUSER_W = (TUSER_WIDTH == 0) ? 1 : TUSER_W;

  if (TDATA_WIDTH % 8 != 0) begin : g_tdata_width_check
    $error("TDATA_WIDTH must be a multiple of 8, given = %d", TDATA_WIDTH);
  end

  logic                            tvalid;
  logic [(TDATA_WIDTH/8)-1:0][7:0] tdata;
  logic [(TDATA_WIDTH/8)-1:0]      tstrb;
  logic [(TDATA_WIDTH/8)-1:0]      tkeep;
  logic                            tlast;
  logic [          TID_W-1:0]      tid;
  logic [        TDEST_W-1:0]      tdest;
  logic [        TUSER_W-1:0]      tuser;
  logic                            tready;

  modport master (
    output tvalid,
    output tdata,
    output tstrb,
    output tkeep,
    output tlast,
    output tid,
    output tdest,
    output tuser,
    input tready
  );

  modport slave (
    input tvalid,
    input tdata,
    input tstrb,
    input tkeep,
    input tlast,
    input tid,
    input tdest,
    input tuser,
    output tready
  );
endinterface
`default_nettype wire
