# (c) Copyright 2013 - 2016 Xilinx, Inc. All rights reserved. 
# 
# This file contains confidential and proprietary information of Xilinx, Inc. and
# is protected under U.S. and international copyright and other intellectual
# property laws.
# 
# DISCLAIMER 
# This disclaimer is not a license and does not grant any rights to the materials
# distributed herewith. Except as otherwise provided in a valid license issued to
# you by Xilinx, and to the maximum extent permitted by applicable law: (1) THESE
# MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS, AND XILINX HEREBY
# DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY,
# INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, OR
# FITNESS FOR ANY PARTICULAR PURPOSE; and (2) Xilinx shall not be liable (whether
# in contract or tort, including negligence, or under any other theory of
# liability) for any loss or damage of any kind or nature related to, arising
# under or in connection with these materials, including for any direct, or any
# indirect, special, incidental, or consequential loss or damage (including loss
# of data, profits, goodwill, or any type of loss or damage suffered as a result
# of any action brought by a third party) even if such damage or loss was
# reasonably foreseeable or Xilinx had been advised of the possibility of the
# same.
# 
# CRITICAL APPLICATIONS
# Xilinx products are not designed or intended to be fail-safe, or for use in any
# application requiring fail-safe performance, such as life-support or safety
# devices or systems, Class III medical devices, nuclear facilities, applications
# related to the deployment of airbags, or any other applications that could lead
# to death, personal injury, or severe property or environmental damage
# (individually and collectively, "Critical Applications"). Customer assumes the
# sole risk and liability of any use of Xilinx products in Critical Applications,
# subject only to applicable laws and regulations governing limitations on product
# liability.
# 
# THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT
# ALL TIMES. 

APPSOURCES = imgio.cpp test-c.cpp optflow_hls.cpp
EXECUTABLE = optflow4k_fio.elf

ifndef TARGET_BOARD
TARGET_BOARD = zcu102
endif

SDSFLAGS = -sds-pf ${TARGET_BOARD} \
	-sds-hw flowWrap optflow_hls.cpp -clkid 3  -sds-end -dmclkid 3 -impl-strategy Performance_Explore


CC = sdscc ${SDSFLAGS}
CXX = sds++ ${SDSFLAGS}

CFLAGS = -O3 -c -Wall
LFLAGS = -O3  

CPP_OBJECTS := $(APPSOURCES:.cpp=.o)
PLATFORM_OBJECTS := $(PLATFORM_SOURCES:.c=.o)

.PHONY: all

all: ${EXECUTABLE}


${EXECUTABLE}: ${CPP_OBJECTS} ${PLATFORM_OBJECTS}
	${CXX} ${CPP_OBJECTS} ${PLATFORM_OBJECTS} -o $@ $(LFLAGS)

%.o: %.cpp
	${CXX} ${CFLAGS} $< -o $@

clean:
	${RM} ${EXECUTABLE} ${CPP_OBJECTS} ${PLATFORM_OBJECTS}

ultraclean:
	${RM} -rf _sds sd_card ${EXECUTABLE} ${CPP_OBJECTS} ${PLATFORM_OBJECTS}


