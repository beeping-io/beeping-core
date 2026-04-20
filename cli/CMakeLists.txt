add_executable(beeping_core_cli
  beeping_cli_main.cpp
  decode_cmd.cpp
  wav_reader.cpp
)

set_target_properties(beeping_core_cli PROPERTIES
  OUTPUT_NAME "beeping-core"
)

target_link_libraries(beeping_core_cli PRIVATE BeepingCore)
target_compile_features(beeping_core_cli PRIVATE cxx_std_20)

install(TARGETS beeping_core_cli
  RUNTIME DESTINATION bin
)
