cmake_minimum_required(VERSION 3.25)
project(OneTwoFiveA_Pitchator VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(PITCHATOR_BUILD_VST3 "Compile native Steinberg VST3" ON)
enable_testing()
add_executable(PitchEngineTests tests/PitchEngineTests.cpp)
add_executable(PitchSchedulerTests tests/PitchSchedulerTests.cpp)
add_executable(CurveMailboxTests tests/CurveMailboxTests.cpp)
add_executable(PitchatorUIContractTests tests/UIContractTests.cpp)
target_compile_definitions(PitchatorUIContractTests PRIVATE
 PITCHATOR_UIDESC_PATH="${CMAKE_SOURCE_DIR}/src/resource/pitchator.uidesc"
 PITCHATOR_SRC_PATH="${CMAKE_SOURCE_DIR}/src/PitchatorVST.cpp"
 PITCHATOR_GRAPH_PATH="${CMAKE_SOURCE_DIR}/src/PitchCurveView.cpp")
add_test(NAME PitchEngineTests COMMAND PitchEngineTests)
add_test(NAME PitchSchedulerTests COMMAND PitchSchedulerTests)
if(PITCHATOR_BUILD_VST3)
 set(SMTG_RUN_VST_VALIDATOR OFF CACHE BOOL "" FORCE)
 set(SMTG_CREATE_PLUGIN_LINK OFF CACHE BOOL "" FORCE)
 set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "" FORCE)
 set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES ON CACHE BOOL "" FORCE)
 set(SMTG_ENABLE_VSTGUI_SUPPORT ON CACHE BOOL "" FORCE)
 include(FetchContent)
 FetchContent_Declare(vst3sdk
  GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
  GIT_TAG v3.8.1_build_84)
 FetchContent_MakeAvailable(vst3sdk)
 smtg_add_vst3plugin(Pitchator src/PitchatorVST.cpp src/PitchatorVST.h
  src/PitchCurveView.cpp src/PitchCurveView.h src/CurveMailbox.h
  src/PitchEngine.h src/PitchScheduler.h)
 target_link_libraries(Pitchator PRIVATE sdk vstgui_support)
 smtg_target_add_plugin_resources(Pitchator
  RESOURCES src/resource/pitchator.uidesc
            src/resource/125A_Logo_Master_FINAL.svg)
 target_compile_features(Pitchator PRIVATE cxx_std_17)
 if(WIN32)
  target_sources(Pitchator PRIVATE ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/dllmain.cpp)
 endif()
 smtg_target_configure_version_file(Pitchator)
endif()
