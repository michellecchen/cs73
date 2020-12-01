# CMake generated Testfile for 
# Source directory: /Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/modules/features2d
# Build directory: /Users/tanli/dev/cs73/panorama-opencv/opencv/build_opencv/modules/features2d
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(opencv_test_features2d "/Users/tanli/dev/cs73/panorama-opencv/opencv/build_opencv/bin/opencv_test_features2d" "--gtest_output=xml:opencv_test_features2d.xml")
set_tests_properties(opencv_test_features2d PROPERTIES  LABELS "Main;opencv_features2d;Accuracy" WORKING_DIRECTORY "/Users/tanli/dev/cs73/panorama-opencv/opencv/build_opencv/test-reports/accuracy" _BACKTRACE_TRIPLES "/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/cmake/OpenCVUtils.cmake;1640;add_test;/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/cmake/OpenCVModule.cmake;1282;ocv_add_test_from_target;/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/cmake/OpenCVModule.cmake;1073;ocv_add_accuracy_tests;/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/modules/features2d/CMakeLists.txt;6;ocv_define_module;/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/modules/features2d/CMakeLists.txt;0;")
add_test(opencv_perf_features2d "/Users/tanli/dev/cs73/panorama-opencv/opencv/build_opencv/bin/opencv_perf_features2d" "--gtest_output=xml:opencv_perf_features2d.xml")
set_tests_properties(opencv_perf_features2d PROPERTIES  LABELS "Main;opencv_features2d;Performance" WORKING_DIRECTORY "/Users/tanli/dev/cs73/panorama-opencv/opencv/build_opencv/test-reports/performance" _BACKTRACE_TRIPLES "/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/cmake/OpenCVUtils.cmake;1640;add_test;/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/cmake/OpenCVModule.cmake;1193;ocv_add_test_from_target;/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/cmake/OpenCVModule.cmake;1074;ocv_add_perf_tests;/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/modules/features2d/CMakeLists.txt;6;ocv_define_module;/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/modules/features2d/CMakeLists.txt;0;")
add_test(opencv_sanity_features2d "/Users/tanli/dev/cs73/panorama-opencv/opencv/build_opencv/bin/opencv_perf_features2d" "--gtest_output=xml:opencv_perf_features2d.xml" "--perf_min_samples=1" "--perf_force_samples=1" "--perf_verify_sanity")
set_tests_properties(opencv_sanity_features2d PROPERTIES  LABELS "Main;opencv_features2d;Sanity" WORKING_DIRECTORY "/Users/tanli/dev/cs73/panorama-opencv/opencv/build_opencv/test-reports/sanity" _BACKTRACE_TRIPLES "/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/cmake/OpenCVUtils.cmake;1640;add_test;/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/cmake/OpenCVModule.cmake;1194;ocv_add_test_from_target;/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/cmake/OpenCVModule.cmake;1074;ocv_add_perf_tests;/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/modules/features2d/CMakeLists.txt;6;ocv_define_module;/Users/tanli/dev/cs73/panorama-opencv/opencv/opencv/modules/features2d/CMakeLists.txt;0;")