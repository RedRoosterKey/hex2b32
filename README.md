# hex2b32
Simple terminal program using STDIN/STDOUT to convert hexadecimal to base32 (RFC 3548).

https://tools.ietf.org/html/rfc3548

# How to build
Go into the Release directory and run the following command:
```shell
make all && make test
```
This will create a program called hex2b32 and test it with the scripts in the test directory.
If all the tests pass, you should be good to go.  A good start would be to read the help by running:
```shell
./hex2b32 --help
```
