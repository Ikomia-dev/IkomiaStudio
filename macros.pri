top_srcdir=$$PWD
top_builddir=$$shadowed($$PWD)

defineReplace(absolute_paths) {
    result =
    for(path, ARGS) {
        result += $$absolute_path($$path, $$_PRO_FILE_PWD_)
    }
    return($$result)
}

defineReplace(changePath) {
    var1 = $$1
    var2 = $$2
    var3 = $$3
    var4 = $$quote(install_name_tool -change $$var1$$var2 @executable_path/../Frameworks/$$var2 $$var3 $$escape_expand(\n\t))

    return($$var4)
}

defineReplace(changeOpencvPath) {
    var1 = $$1
    var2 = $$2
    var3 = $$3
    var4 = $$quote(install_name_tool -change @rpath/$$var1 @executable_path/../Frameworks/$$var2 $$var3 $$escape_expand(\n\t))

    return($$var4)
}

defineReplace(link_opencv) {
    # ----- WINDOWS -----
    # Main modules
    win32:libs += -lopencv_calib3d$${OPENCV_VERSION}
    win32:libs += -lopencv_core$${OPENCV_VERSION}
    win32:libs += -lopencv_dnn$${OPENCV_VERSION}
    win32:libs += -lopencv_features2d$${OPENCV_VERSION}
    win32:libs += -lopencv_flann$${OPENCV_VERSION}
    win32:libs += -lopencv_gapi$${OPENCV_VERSION}
    win32:libs += -lopencv_highgui$${OPENCV_VERSION}
    win32:libs += -lopencv_imgcodecs$${OPENCV_VERSION}
    win32:libs += -lopencv_imgproc$${OPENCV_VERSION}
    win32:libs += -lopencv_ml$${OPENCV_VERSION}
    win32:libs += -lopencv_objdetect$${OPENCV_VERSION}
    win32:libs += -lopencv_photo$${OPENCV_VERSION}
    win32:libs += -lopencv_stitching$${OPENCV_VERSION}
    win32:libs += -lopencv_video$${OPENCV_VERSION}
    win32:libs += -lopencv_videoio$${OPENCV_VERSION}
    # Extra modules
    win32:libs += -lopencv_aruco$${OPENCV_VERSION}
    win32:libs += -lopencv_bgsegm$${OPENCV_VERSION}
    win32:libs += -lopencv_bioinspired$${OPENCV_VERSION}
    win32:libs += -lopencv_ccalib$${OPENCV_VERSION}
    win32:libs += -lopencv_cudaarithm$${OPENCV_VERSION}
    win32:libs += -lopencv_cudabgsegm$${OPENCV_VERSION}
    win32:libs += -lopencv_cudafeatures2d$${OPENCV_VERSION}
    win32:libs += -lopencv_cudafilters$${OPENCV_VERSION}
    win32:libs += -lopencv_cudaimgproc$${OPENCV_VERSION}
    win32:libs += -lopencv_cudalegacy$${OPENCV_VERSION}
    win32:libs += -lopencv_cudaobjdetect$${OPENCV_VERSION}
    win32:libs += -lopencv_cudaoptflow$${OPENCV_VERSION}
    win32:libs += -lopencv_cudastereo$${OPENCV_VERSION}
    win32:libs += -lopencv_cudawarping$${OPENCV_VERSION}
    win32:libs += -lopencv_cudev$${OPENCV_VERSION}
    win32:libs += -lopencv_datasets$${OPENCV_VERSION}
    win32:libs += -lopencv_dnn_objdetect$${OPENCV_VERSION}
    win32:libs += -lopencv_dnn_superres$${OPENCV_VERSION}
    win32:libs += -lopencv_dpm$${OPENCV_VERSION}
    win32:libs += -lopencv_face$${OPENCV_VERSION}
    win32:libs += -lopencv_fuzzy$${OPENCV_VERSION}
    #win32:libs += -lopencv_hdf$${OPENCV_VERSION}
    win32:libs += -lopencv_hfs$${OPENCV_VERSION}
    win32:libs += -lopencv_img_hash$${OPENCV_VERSION}
    win32:libs += -lopencv_line_descriptor$${OPENCV_VERSION}
    win32:libs += -lopencv_optflow$${OPENCV_VERSION}
    win32:libs += -lopencv_phase_unwrapping$${OPENCV_VERSION}
    win32:libs += -lopencv_plot$${OPENCV_VERSION}
    win32:libs += -lopencv_quality$${OPENCV_VERSION}
    win32:libs += -lopencv_reg$${OPENCV_VERSION}
    win32:libs += -lopencv_rgbd$${OPENCV_VERSION}
    win32:libs += -lopencv_saliency$${OPENCV_VERSION}
    win32:libs += -lopencv_shape$${OPENCV_VERSION}
    win32:libs += -lopencv_stereo$${OPENCV_VERSION}
    win32:libs += -lopencv_structured_light$${OPENCV_VERSION}
    win32:libs += -lopencv_superres$${OPENCV_VERSION}
    win32:libs += -lopencv_surface_matching$${OPENCV_VERSION}
    win32:libs += -lopencv_text$${OPENCV_VERSION}
    win32:libs += -lopencv_tracking$${OPENCV_VERSION}
    win32:libs += -lopencv_videostab$${OPENCV_VERSION}
    #win32:libs += -lopencv_viz$${OPENCV_VERSION}
    win32:libs += -lopencv_xfeatures2d$${OPENCV_VERSION}
    win32:libs += -lopencv_ximgproc$${OPENCV_VERSION}
    win32:libs += -lopencv_xobjdetect$${OPENCV_VERSION}
    win32:libs += -lopencv_xphoto$${OPENCV_VERSION}

    # ----- LINUX & MAC OS -----
    # Main modules
    unix:libs += -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
    #Not used but necessary for python
    unix:libs += -lopencv_alphamat
    unix:libs += -lopencv_videoio
    unix:libs += -lopencv_highgui
    unix:libs += -lopencv_video
    unix:libs += -lopencv_calib3d
    unix:libs += -lopencv_features2d
    unix:libs += -lopencv_freetype
    unix:libs += -lopencv_objdetect
    unix:libs += -lopencv_dnn
    unix:libs += -lopencv_ml
    unix:libs += -lopencv_flann
    unix:libs += -lopencv_photo
    unix:libs += -lopencv_stitching
    unix:libs += -lopencv_gapi
    unix:libs += -lopencv_wechat_qrcode
    # Extra modules
    unix:libs += -lopencv_aruco
    unix:libs += -lopencv_bgsegm
    unix:libs += -lopencv_bioinspired
    unix:libs += -lopencv_ccalib
    unix:libs += -lopencv_cudaarithm
    unix:libs += -lopencv_cudabgsegm
    unix:libs += -lopencv_cudafeatures2d
    unix:libs += -lopencv_cudafilters
    unix:libs += -lopencv_cudaimgproc
    unix:libs += -lopencv_cudalegacy
    unix:libs += -lopencv_cudaobjdetect
    unix:libs += -lopencv_cudaoptflow
    unix:libs += -lopencv_cudastereo
    unix:libs += -lopencv_cudawarping
    unix:libs += -lopencv_cudev
    #unix:libs += -lopencv_cvv
    unix:libs += -lopencv_datasets
    unix:libs += -lopencv_dnn_objdetect
    unix:libs += -lopencv_dnn_superres
    unix:libs += -lopencv_dpm
    unix:libs += -lopencv_face
    unix:libs += -lopencv_fuzzy
    macx:libs += -lopencv_hdf
    unix:libs += -lopencv_hfs
    unix:libs += -lopencv_img_hash
    unix:libs += -lopencv_intensity_transform
    unix:libs += -lopencv_line_descriptor
    unix:libs += -lopencv_mcc
    unix:libs += -lopencv_optflow
    unix:libs += -lopencv_phase_unwrapping
    unix:libs += -lopencv_plot
    unix:libs += -lopencv_quality
    unix:libs += -lopencv_rapid
    unix:libs += -lopencv_reg
    unix:libs += -lopencv_rgbd
    unix:libs += -lopencv_saliency
    macx:libs += -lopencv_sfm
    unix:libs += -lopencv_shape
    unix:libs += -lopencv_stereo
    unix:libs += -lopencv_structured_light
    unix:libs += -lopencv_superres
    unix:libs += -lopencv_surface_matching
    unix:libs += -lopencv_text
    unix:libs += -lopencv_tracking
    unix:libs += -lopencv_videostab
    unix:libs += -lopencv_viz
    unix:libs += -lopencv_xfeatures2d
    unix:libs += -lopencv_ximgproc
    unix:libs += -lopencv_xobjdetect
    unix:libs += -lopencv_xphoto
    return($$libs)
}

defineReplace(link_boost) {
    #Dynamic link to Boost
    win32:CONFIG(release, debug|release): libs += -lboost_filesystem-vc$${BOOST_VC_VERSION}-mt-x64-$${BOOST_VERSION} -lboost_system-vc$${BOOST_VC_VERSION}-mt-x64-$${BOOST_VERSION} -lboost_python$${PYTHON_VERSION_NO_DOT}-vc$${BOOST_VC_VERSION}-mt-x64-$${BOOST_VERSION}
    else:win32:CONFIG(debug, debug|release): libs += -lboost_filesystem-vc$${BOOST_VC_VERSION}-mt-gd-x64-$${BOOST_VERSION} -lboost_system-vc$${BOOST_VC_VERSION}-mt-gd-x64-$${BOOST_VERSION} -lboost_python$${PYTHON_VERSION_NO_DOT}-vc$${BOOST_VC_VERSION}-mt-gd-x64-$${BOOST_VERSION}
    centos7 {
        unix:!macx: libs += -lboost_filesystem -lboost_system -lboost_python$${PYTHON_VERSION_NO_DOT}
    } else:ubuntu_20_04 {
        unix:!macx: libs += -lboost_filesystem -lboost_system -lboost_python$${PYTHON_VERSION_NO_DOT}
    } else {
        unix:!macx: libs += -lboost_filesystem -lboost_system -lboost_python$${PYTHON_VERSION_NO_DOT}
    }
    macx:libs += -lboost_filesystem -lboost_system -lboost_python37
    return($$libs)
}

defineReplace(link_vtk) {
    win32:libs += -lvtkCommonCore-$${VTK_VERSION} -lvtkCommonDataModel-$${VTK_VERSION} -lvtkCommonExecutionModel-$${VTK_VERSION} -lvtkCommonMath-$${VTK_VERSION} -lvtkCommonSystem-$${VTK_VERSION} -lvtkCommonTransforms-$${VTK_VERSION}
    win32:libs += -lvtkFiltersCore-$${VTK_VERSION} -lvtkFiltersGeneral-$${VTK_VERSION} -lvtkFiltersImaging-$${VTK_VERSION} -lvtkImagingColor-$${VTK_VERSION} -lvtkImagingCore-$${VTK_VERSION}
    win32:libs += -lvtkImagingGeneral-$${VTK_VERSION} -lvtkImagingMath-$${VTK_VERSION} -lvtkImagingSources-$${VTK_VERSION} -lvtkInteractionImage-$${VTK_VERSION} -lvtkInteractionStyle-$${VTK_VERSION} -lvtkIOCore-$${VTK_VERSION} -lvtkIOImage-$${VTK_VERSION}
    win32:libs += -lvtkRenderingContext2D-$${VTK_VERSION} -lvtkRenderingCore-$${VTK_VERSION} -lvtkRenderingImage-$${VTK_VERSION} -lvtkRenderingOpenGL2-$${VTK_VERSION} -lvtksys-$${VTK_VERSION} -lvtkViewsCore-$${VTK_VERSION}
    win32:libs += -lvtkDICOM-$${VTK_VERSION}

    unix:!macx:libs += -lvtkCommonCore-$${VTK_VERSION} -lvtkCommonDataModel-$${VTK_VERSION} -lvtkCommonExecutionModel-$${VTK_VERSION} -lvtkCommonMath-$${VTK_VERSION} -lvtkCommonSystem-$${VTK_VERSION} -lvtkCommonTransforms-$${VTK_VERSION}
    unix:!macx:libs += -lvtkFiltersCore-$${VTK_VERSION} -lvtkFiltersGeneral-$${VTK_VERSION} -lvtkFiltersImaging-$${VTK_VERSION} -lvtkImagingColor-$${VTK_VERSION} -lvtkImagingCore-$${VTK_VERSION}
    unix:!macx:libs += -lvtkImagingGeneral-$${VTK_VERSION} -lvtkImagingMath-$${VTK_VERSION} -lvtkImagingSources-$${VTK_VERSION} -lvtkInteractionImage-$${VTK_VERSION} -lvtkInteractionStyle-$${VTK_VERSION} -lvtkIOCore-$${VTK_VERSION} -lvtkIOImage-$${VTK_VERSION}
    unix:!macx:libs += -lvtkRenderingContext2D-$${VTK_VERSION} -lvtkRenderingCore-$${VTK_VERSION} -lvtkRenderingImage-$${VTK_VERSION} -lvtkRenderingOpenGL2-$${VTK_VERSION} -lvtksys-$${VTK_VERSION} -lvtkViewsCore-$${VTK_VERSION}
    unix:!macx:libs += -lvtkDICOM-$${VTK_VERSION}
    return($$libs)
}

defineReplace(link_utils) {
    #Dynamic link to Utils
    win32: libs += -L$$IKOMIA_LIB_PATH -likUtils
    else:unix: libs += -L$$IKOMIA_LIB_PATH -likUtils
    return($$libs)
}

defineReplace(link_core) {
    #Dynamic link to Core
    win32: libs += -L$$IKOMIA_LIB_PATH -likCore
    else:unix: libs += -L$$IKOMIA_LIB_PATH -likCore
    return($$libs)
}

defineReplace(link_dataprocess) {
    #Dynamic link to DataProcess
    win32: libs += -L$$IKOMIA_LIB_PATH -likDataProcess
    else:unix: libs += -L$$IKOMIA_LIB_PATH -likDataProcess
    return($$libs)
}

defineReplace(link_dataio) {
    #Dynamic link to DataIO
    win32: libs += -L$$IKOMIA_LIB_PATH -likDataIO
    else:unix: libs += -L$$IKOMIA_LIB_PATH -likDataIO
    return($$libs)
}

defineReplace(link_datamanagement) {
    #Dynamic link to DataManagement
    win32: libs += -L$$IKOMIA_LIB_PATH -likDataManagement
    else:unix: libs += -L$$IKOMIA_LIB_PATH -likDataManagement
    return($$libs)
}

defineReplace(link_dataplot) {
    #Dynamic link to DataPlot
    win32: libs += -L$$IKOMIA_LIB_PATH -likDataPlot
    else:unix: libs += -L$$IKOMIA_LIB_PATH -likDataPlot
    return($$libs)
}

defineReplace(link_volumerender) {
    #Dynamic link to VolumeRender
    win32: libs += -L$$IKOMIA_LIB_PATH -likVolumeRender
    else:unix: libs += -L$$IKOMIA_LIB_PATH -likVolumeRender
    return($$libs)
}

defineReplace(link_qwt) {
    # Dynamic link to QWT
    win32:CONFIG(release, debug|release): libs += -L$$IKOMIA_EXT_LIB_PATH/qwt/build/lib/ -lqwt
    else:win32:CONFIG(debug, debug|release): libs += -L$$IKOMIA_EXT_LIB_PATH/qwt/build/lib/ -lqwtd
    else:unix:!macx: libs += -L$$IKOMIA_EXT_LIB_PATH/qwt/build/lib/ -lqwt
    else:macx: libs += -F$$IKOMIA_EXT_LIB_PATH/qwt/build/lib/ -framework qwt
    return($$libs)
}

defineReplace(link_python) {
    win32:libs += -lpython$${PYTHON_VERSION_NO_DOT}
    macx:libs += -lpython$${PYTHON_VERSION_DOT}
    unix:!macx: libs += -lpython$${PYTHON_VERSION_DOT}
    return($$libs)
}
