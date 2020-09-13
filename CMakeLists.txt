cmake_minimum_required(VERSION 3.6)
set(CROSS_TARGET_TRIPLET "arm-none-eabi-")
set(MCU "STM32F767ZI")

set(PRJ_NAME stm32f7x7-cmake)
set(MCU_FAMILY STM32F7)
set(MCU_LINE STM32F767ZI)
set(MCU_LINKER_SCRIPT STM32F767ZI.ld)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(GENLINK_CMD ./scripts/genlink.py ld/devices.data ${MCU})
set(GENLINK_DIR ${CMAKE_SOURCE_DIR}/lib/libopencm3/libopencm3)


project(${PRJ_NAME} C ASM)

# Compiler settings

# Prepend the cross-compiler prefix to the used tools.
set(CMAKE_C_COMPILER "${CROSS_TARGET_TRIPLET}gcc")
set(CMAKE_CXX_COMPILER "${CROSS_TARGET_TRIPLET}g++")
set(CMAKE_ASM_COMPILER "${CROSS_TARGET_TRIPLET}as")
SET(CMAKE_OBJCOPY "${CROSS_TARGET_TRIPLET}objcopy")
SET(CMAKE_SIZE "${CROSS_TARGET_TRIPLET}size")



# Be verbose when compiling.
set(WARNING_FLAGS "-Wextra -Wshadow -Wredundant-decls")
set(WARNING_CFLAGS "${WARNING_FLAGS} -Wimplicit-function-declaration -Wmissing-prototypes -Wstrict-prototypes")
set(WARNING_CXXFLAGS "${WARNING_FLAGS} -Weffc++")

# Set MCPU, MFPU from genlink
execute_process(COMMAND ${GENLINK_CMD} CPU WORKING_DIRECTORY ${GENLINK_DIR} OUTPUT_VARIABLE MCPU)
execute_process(COMMAND ${GENLINK_CMD} FPU COMMAND cut -d "-" -f1 COMMAND tr -d "\n" WORKING_DIRECTORY ${GENLINK_DIR} OUTPUT_VARIABLE FPU_ABI)
execute_process(COMMAND ${GENLINK_CMD} FPU COMMAND cut -d "-" -f2- COMMAND tr -d "\n" WORKING_DIRECTORY ${GENLINK_DIR} OUTPUT_VARIABLE MFPU)


# Flags which are required for gcc, g++ and as.
set(ARCH_FLAGS "-mthumb -mcpu=${MCPU}")
set(COMMON_COMPILE_FLAGS "${ARCH_FLAGS} -mfloat-abi=${FPU_ABI} -mfpu=${MFPU} -fno-common -ffunction-sections -fdata-sections")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_COMPILE_FLAGS} ${WARNING_CFLAGS} ${DEFINITIONS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_COMPILE_FLAGS} ${WARNING_CXXFLAGS} ${DEFINITIONS}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${COMMON_COMPILE_FLAGS}")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --static -nostartfiles -L ${CMAKE_SOURCE_DIR}/lib/libopencm3/libopencm3/lib -T ${CMAKE_SOURCE_DIR}/${MCU_LINKER_SCRIPT} ${ARCH_FLAGS} -Wl,--gc-sections -specs=nosys.specs -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group -Wl,--undefined=uxTopUsedPriority")

# The following is required to get rid of the (not supported) -rdynamic flag
# when linking the final binary.
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "" )
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "" )

execute_process(COMMAND ${GENLINK_CMD} DEFS WORKING_DIRECTORY ${GENLINK_DIR} OUTPUT_VARIABLE DEFS)
add_definitions(${DEFS})

add_library(libopencm3 STATIC IMPORTED)
set_target_properties(libopencm3 PROPERTIES
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libopencm3/libopencm3/lib/libopencm3_stm32f7.a"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/libopencm3/libopencm3/include"
)
add_custom_target(
  libopencm3-build
  COMMAND make lib/stm32/f7
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/libopencm3/libopencm3"
)

file(GLOB_RECURSE USER_SOURCES src/*.c)
file(GLOB FREERTOS_SOURCES lib/FreeRTOS-Kernel/src/*.c)
file(GLOB LWMEM_SOURCES lib/lwmem/src/*.c)

set(SOURCE_FILES ${USER_SOURCES} ${FREERTOS_SOURCES} ${LWMEM_SOURCES})

include_directories(include)
include_directories(lib/FreeRTOS-Kernel/FreeRTOS-Kernel/include)
include_directories(lib/FreeRTOS-Kernel/include)
include_directories(lib/lwmem/lwmem/lwmem/src/include)
include_directories(lib/lwmem/include)

add_executable(${PROJECT_NAME}.elf ${SOURCE_FILES})
add_dependencies(${PROJECT_NAME}.elf libopencm3-build)
target_link_libraries(${PROJECT_NAME}.elf libopencm3)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.map -Wl,--undefined=uxTopUsedPriority -Wl,--print-memory-usage")
set(HEX_FILE ${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_SOURCE_DIR}/build/${PROJECT_NAME}.bin)
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
        COMMENT "Building ${HEX_FILE} \nBuilding ${BIN_FILE}")