ifneq ($(words $(CURDIR)), 1)
	$(error Unsupported: GNU Make cannot build in directories containing spaces, build elsewhere: '$(CURDIR)')
endif

ifeq ($(VERILATOR_ROOT),)
	VERILATOR = verilator
else
	export VERILATOR_ROOT
	VERILATOR = $(VERILATOR_ROOT)/bin/verilator
endif

MODULE ?= top
RTL_FILE ?= rtl/$(MODULE)/$(MODULE).sv
TB_FILE ?= rtl/$(MODULE)/$(MODULE)_tb.cpp

OBJ_DIR = obj_dir
RTL_DIR = rtl

RTL_SRCS = 

VERILATOR_FLAGS += -cc --exe
VERILATOR_FLAGS += --build
VERILATOR_FLAGS += -j 0
VERILATOR_FLAGS += -x-assign fast
VERILATOR_FLAGS += -Wall
VERILATOR_FLAGS += --trace-vcd
VERILATOR_FLAGS += -O2

CXXFLAGS = -O2

$(OBJ_DIR)/V$(MODULE): $(RTL_FILE) $(RTL_SRCS) $(TB_FILE)
	$(VERILATOR) $(VERILATOR_FLAGS) \
		-CFLAGS "$(CXXFLAGS)" \
		$(RTL_FILE) $(RTL_SRCS) \
		$(TB_FILE)

.PHONY: sim
sim: $(OBJ_DIR)/V$(MODULE)
	./$(OBJ_DIR)/V$(MODULE)

.PHONY: wave
wave:
	gtwkave waveform.vcd &

.PHONY: clean
clean:
	rm -rf obj_dir *.vcd *.fst
