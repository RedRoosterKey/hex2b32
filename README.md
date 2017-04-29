# hex2b32
Simple terminal program using STDIN/STDOUT to convert hexadecimal to base32 (RFC 3548).

[Request for Comments: 3548 - Base 32 Encoding](https://tools.ietf.org/html/rfc3548#section-5)

# How to build
Go into the Release directory and run the following command:
```shell
make all && make test
```
This will create a program called hex2b32 and execute test.sh in the scripts directory.
If all the tests pass, you should be good to go.  A good start would be to read the help by running:
```shell
./hex2b32 --help
```
## Demo
![gif](https://github.com/RedRoosterKey/hex2b32/blob/master/tty.gif?raw=true)
