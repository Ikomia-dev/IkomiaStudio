add_compile_definitions(
    CUDA11
    PY310
)

set(IK_CPU OFF)
set(CENTOS7 OFF)
set(PYTHON_VERSION_DOT 3.7)
set(PYTHON_VERSION_DOT_M 3.7m)
set(PYTHON_VERSION_NO_DOT 37)

if(WIN32)
    set(BOOST_VERSION 1_74)
    set(BOOST_VC_VERSION 142)
    set(MSVC_VERSION 16)
    set(CUDA_VERSION 11.1)
endif()
