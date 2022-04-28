set(CMAKE_C_STANDARD 11)


add_executable(4_1_server zad1/src/server1.c)
add_executable(4_1_client zad1/src/client1.c)
add_executable(4_2_server zad2/src/server2.c)
add_executable(4_2_client zad2/src/client2.c)
add_executable(4_3_server zad3/src/server3.c)
add_executable(4_3_client zad3/src/client3.c)

find_library( SCTP_LIBRARY sctp )

target_link_libraries( 4_2_server ${SCTP_LIBRARY} )
target_link_libraries( 4_2_client ${SCTP_LIBRARY} )
target_link_libraries( 4_3_server ${SCTP_LIBRARY} )
target_link_libraries( 4_3_client ${SCTP_LIBRARY} )