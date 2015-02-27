set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CROSSCOMPILING 1)

# Get some tools we need.
find_program(AVR_CC avr-gcc)
find_program(AVR_CXX avr-g++)
find_program(AVR_OBJCOPY avr-objcopy)
find_program(AVR_OBJDUMP avr-objdump)
find_program(AVR_SIZE avr-size)
find_program(AVR_AVRDUDE avrdude)

# Configure compilers.
set(CMAKE_C_COMPILER   ${AVR_CC})
set(CMAKE_CXX_COMPILER ${AVR_CXX})

# Compiler and linker flags.
set(AVR_COMPILE_FLAGS "-Wall -Wl,--relax -g -Os -mcall-prologues -fno-exceptions")
set(AVR_COMPILE_FLAGS "${AVR_COMPILE_FLAGS} -ffunction-sections -fdata-sections")
set(AVR_COMPILE_FLAGS "${AVR_COMPILE_FLAGS} -mmcu=${AVR_MCU} -DF_CPU=${AVR_F_CPU}UL")
set(AVR_LINK_FLAGS "-Wl,--gc-sections -Wl,--relax -lm -mmcu=${AVR_MCU}")

# Upload flags
set(AVR_AVRDUDE_TYPE "arduino")
set(AVR_AVRDUDE_PORT "/dev/ttyUSB0")
set(AVR_AVRDUDE_BAUD 57600)
set(AVR_AVRDUDE_ARGS "-V")


function(avr_add_firmware TARGET)
	set(elf_file ${TARGET}.elf)
	set(hex_file ${TARGET}.hex)
	set(map_file ${TARGET}.map)
	set(lst_file ${TARGET}.lst)
	set(eeprom_file ${TARGET}-eeprom.hex)

	add_executable(${elf_file} EXCLUDE_FROM_ALL ${TARGET}.cpp ${ARGN})
	set_target_properties(
		${elf_file}
		PROPERTIES
		COMPILE_FLAGS "${AVR_COMPILE_FLAGS}"
		LINK_FLAGS "${AVR_LINK_FLAGS} -Wl,-Map,${map_file}"
	)

	add_custom_command(
		OUTPUT ${hex_file}
		COMMAND
			${AVR_OBJCOPY} -j .text -j .data -O ihex ${elf_file} ${hex_file}
		COMMAND
			${AVR_SIZE} ${elf_file}
		DEPENDS ${elf_file}
	)

	add_custom_command(
		OUTPUT ${eeprom_file}
		COMMAND
			${AVR_OBJCOPY} -j .eeprom --change-section-lma .eeprom=0 
				-O ihex ${elf_file} ${eeprom_file}
		DEPENDS ${elf_file}
	)

	add_custom_target(
		${TARGET}
		ALL
		DEPENDS ${hex_file} ${eeprom_file}
	)

	set(avrdude_args -c ${AVR_AVRDUDE_TYPE} -P ${AVR_AVRDUDE_PORT} -p ${AVR_MCU})
	set(avrdude_args ${avrdude_args} -b ${AVR_AVRDUDE_BAUD} -D ${AVRDUDE_OPTIONS})

	add_custom_target(
		upload
		${AVR_AVRDUDE} ${avrdude_args} -U flash:w:${hex_file}
		DEPENDS ${hex_file}
	)

	add_custom_target(
		upload_eeprom
		${AVR_AVRDUDE} ${avrdude_args} -U eeprom:w:${eeprom_file}
		DEPENDS ${eeprom_file}
	)

	add_custom_target(
		disasm
		${AVR_OBJDUMP} -h -S ${elf_file} > ${lst_file}
		DEPENDS ${elf_file}
	)
endfunction()

function(avr_add_library LIB SRC)
	add_library(${LIB} ${SRC})
	set_target_properties(${LIB} PROPERTIES COMPILE_FLAGS "${AVR_COMPILE_FLAGS}")
endfunction()

function(avr_add_libraries_directory LIBDIR)
	get_filename_component(LIBNAME ${LIBDIR} NAME)
	add_subdirectory(${LIBDIR} ${CMAKE_CURRENT_BINARY_DIR}/${LIBNAME})
	include_directories(${LIBDIR})
endfunction()

function(avr_link_library TARGET LIB)
	target_link_libraries(${TARGET}.elf ${LIB})
endfunction()
