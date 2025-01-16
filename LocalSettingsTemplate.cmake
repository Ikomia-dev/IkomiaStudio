add_compile_definitions(
    CUDA12
    PY311
)

set(OPENCV_GPU ON)
set(CENTOS7 OFF)
set(PYTHON_VERSION_DOT 3.11)
set(PYTHON_VERSION_DOT_M 3.11)
set(PYTHON_VERSION_NO_DOT 311)

if(WIN32)
    set(BOOST_VERSION 1_83)
    set(BOOST_VC_VERSION 142)
    set(MSVC_VERSION 16)
    set(CUDA_VERSION 12.1)
endif()
