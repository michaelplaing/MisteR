# MisteR
MisteR provides a library for validating, packing & unpacking MQTT5 packets.

MisteR is currently part of a playground for me to experiment with and to rebuild my skills in C development. I am not interested in supporting older standards or existing code.

Progress will be slow as there are many rabbitholes to explore.

Once the basic packet types (CONNECT, CONNACK, SUBSCRIBE, SUBACK, PUBLISH, and PUBACK) can be handled, I may switch gears and work on [mr_redis](https://github.com/michaelplaing/mr_redis) to create a very basic MQTT pubsub server and test client. This will probably provoke some backfilling in this project.

## Implementation
The project is coded in C and built with cmake. I use zlog for logging and catch2 for testing. I'm still developing a scheme for error handling.

The heavy work (packing & unpacking) is done in packet.c, which deals with data types and processes them in accordance with metadata provided by each packet-type specific module. Effectively, packet.c provides the parser and packer for all packet types and does data type validation and some cross-validation.

The packet-specific modules, e.g. connect.c, each have an ordered table of metadata describing their respective fields. These modules each provide get, set & reset functions for each field as appropriate. The also do range validation and additional cross-validation.

There are a few miscellaneous modules which may be renamed and/or consolidated, e.g. memory.c and util.c.

The public header is mister.h. It has the public function prototypes, enums and typedefs needed by a library client. mister_internal.h declares internally used functions, enums & typedefs.
## Building
Other projects will include this project as a subproject, e.g. [mr_redis](https://github.com/michaelplaing/mr_redis). The CMakelists.txt reflects this approach and, in general, I try to use modern cmake methods throughout.

Configuration, build, and installation are standard, e.g. for Mac:
```zsh
brew install zlog catch2
brew install doxygen graphviz # to build docs
```

Create a build directory, cd into it, then run cmake and ninja:
```zsh
mkdir -p build ; cd build
cmake -G Ninja .. && ninja
```
## Installation
To be done.
## Documentation
The documentation is currently present but limited.
## Testing
There is a testing module for each packet type. I am still exploring testing but currently you will see "happy" and "unhappy" tests where I try to model normal processing and validation transgressions respectively.
