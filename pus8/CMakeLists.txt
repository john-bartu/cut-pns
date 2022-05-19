find_package(OpenSSL REQUIRED)

link_libraries(crypto)

add_executable(cipher_ecb zad1/src/cipher_ecb.c)
add_executable(cipher_cbc zad2/src/cipher_cbc.c)
add_executable(cipher_bio zad3/src/cipher_bio.c)
add_executable(digest zad4/src/digest.c)
add_executable(mac zad5/src/mac.c)
add_executable(client_hmac zad6/src/client_hmac.c)
add_executable(server_hmac zad6/src/server_hmac.c)
add_executable(rsa zad7/src/rsa.c)
add_executable(client_rsa zad7/src/client_rsa.c)
add_executable(server_rsa zad7/src/server_rsa.c)