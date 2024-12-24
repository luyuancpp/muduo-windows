
if(MSVC)
LINK_DIRECTORIES("${protobuf_BINARY_DIR}/Debug")
LINK_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/Debug")
endif (MSVC)

add_library(muduo_protobuf_codec ProtobufCodecLite.cc)

target_link_libraries(muduo_protobuf_codec absl::absl_check 
absl::absl_log 
absl::algorithm 
absl::base 
absl::bind_front 
absl::bits 
absl::btree 
absl::cleanup 
absl::cord 
absl::core_headers 
absl::debugging 
absl::die_if_null 
absl::dynamic_annotations 
absl::flags 
absl::flat_hash_map 
absl::flat_hash_set 
absl::function_ref 
absl::hash 
absl::layout 
absl::log_initialize 
absl::log_severity 
absl::memory 
absl::node_hash_map 
absl::node_hash_set 
absl::optional 
absl::span 
absl::status 
absl::statusor 
absl::strings 
absl::synchronization 
absl::time 
absl::type_traits 
absl::utility 
absl::variant 
libprotobuf-lite)

if (UNIX)
set_target_properties(muduo_protobuf_codec PROPERTIES COMPILE_FLAGS "-Wno-error=shadow")
endif ()
if (MSVC)
target_link_libraries(muduo_protobuf_codec muduo_net libprotobufd zlibstatic)
else(MSVC)
target_link_libraries(muduo_protobuf_codec muduo_net protobuf z)
endif ()

install(TARGETS muduo_protobuf_codec DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/muduo/net/protobuf)

