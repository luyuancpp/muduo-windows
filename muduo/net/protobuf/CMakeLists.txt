
if(MSVC)
LINK_DIRECTORIES("${protobuf_BINARY_DIR}/Debug")
LINK_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/Debug")
endif (MSVC)

add_library(muduo_protobuf_codec ProtobufCodecLite.cc)
if (NOT MSVC)
set_target_properties(muduo_protobuf_codec PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
endif ()
if (TARGET protobuf::libprotobuf)
	set(MUDUO_PROTOBUF_LIB protobuf::libprotobuf)
elseif(TARGET libprotobuf)
	set(MUDUO_PROTOBUF_LIB libprotobuf)
elseif(TARGET protobuf)
	set(MUDUO_PROTOBUF_LIB protobuf)
else()
	message(FATAL_ERROR "No protobuf library target found")
endif()

if (TARGET zlibstatic)
	set(MUDUO_ZLIB_LIB zlibstatic)
elseif(TARGET zlib)
	set(MUDUO_ZLIB_LIB zlib)
elseif(TARGET ZLIB::ZLIB)
	set(MUDUO_ZLIB_LIB ZLIB::ZLIB)
else()
	message(FATAL_ERROR "No zlib library target found")
endif()

target_link_libraries(muduo_protobuf_codec muduo_net ${MUDUO_PROTOBUF_LIB} ${MUDUO_ZLIB_LIB})


install(TARGETS muduo_protobuf_codec DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/muduo/net/protobuf)

