set(CMAKE_C_STANDARD 11)

file(COPY bin/img DESTINATION ../bin)

add_executable(lab1
        src/client1.c
        src/client2.c
        src/server1.c
        src/server2.c
        src/libpalindrome.c
        src/libpalindrome.h
        src/client3.c
        src/server3.c
        src/server4.c
        src/server5.c
        )
