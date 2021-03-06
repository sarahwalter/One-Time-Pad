# One-Time-Pad
Five small programs that encrypt and decrypt information using a one-time pad-like system.

otp_enc_d: runs in background as daemon. Upon execution, otp_enc_d outputs an error if it cannot be run due to a network error, such as the ports being unavailable. Its function is to perform the actual encoding. This program listens on a particular port/socket, assigned when it is first ran. When a connection is made, otp_enc_d calls accept() to generate the socket used for actual communication, and then use a separate process to handle the rest of the transaction, which will occur on the newly accepted socket.

otp_enc: This program connects to otp_enc_d, and asks it to perform a one-time pad style encryption as detailed above. By itself, otp_enc doesn’t do the encryption - otp_end_d does. plaintext is the name of a file in the current directory that contains the plaintext you wish to encrypt.

otp_dec_d: This program performs exactly like otp_enc_d, in syntax and usage. In this case, however, otp_dec_d decrypts ciphertext it is given, using the passed-in ciphertext and key. Thus, it returns plaintext again to otp_dec.

otp_dec: Similarly, this program connects to otp_dec_d and asks it to decrypt ciphertext using a passed-in ciphertext and key, and otherwise performs exactly like otp_enc, and is runnable in the same three ways. otp_dec is not able to connect to otp_enc_d, even if it tries to connect on the correct port, the programs reject each other.

keygen: This program creates a key file of specified length. The characters in the file generated will be any of the 27 allowed characters, generated using the standard UNIX randomization methods.
