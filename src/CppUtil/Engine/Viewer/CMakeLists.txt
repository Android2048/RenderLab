Ubpa_QtBegin()

#项目名，默认为目录名
Ubpa_GetDirName(DIRNAME)
set(targetName ${DIRNAME})
#多个源文件用 ';' 分隔
#如：set(sources "main.cpp;src_2.cpp;help.h")
file(GLOB sources
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

list(APPEND sources "${CMAKE_SOURCE_DIR}/include/CppUtil/Engine/Viewer.h")
list(APPEND sources "${CMAKE_SOURCE_DIR}/include/CppUtil/Engine/Roamer.h")
list(APPEND sources "${CMAKE_SOURCE_DIR}/include/CppUtil/Engine/Raster.h")
list(APPEND sources "${CMAKE_SOURCE_DIR}/include/CppUtil/Engine/ForwardRaster.h")
list(APPEND sources "${CMAKE_SOURCE_DIR}/include/CppUtil/Engine/DirectIllumRaster.h")
list(APPEND sources "${CMAKE_SOURCE_DIR}/include/CppUtil/Engine/DeferredRaster.h")
list(APPEND sources "${CMAKE_SOURCE_DIR}/include/CppUtil/Engine/WireframeRaster.h")
list(APPEND sources "${CMAKE_SOURCE_DIR}/include/CppUtil/Engine/ForwardNPR.h")
list(APPEND sources "${CMAKE_SOURCE_DIR}/include/CppUtil/Engine/LTCTex.h")

#多个库文件用 ';' 分隔，如果为空，则为 ""
#如：set(libs "targetName;xxx.lib")
list(APPEND libs "RawAPI_OGLW")

Ubpa_AddTarget(MODE ${mode} NAME ${targetName} SOURCES ${sources} LIBS ${libs})

Ubpa_QtEnd()
