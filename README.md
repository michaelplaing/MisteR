# MisteR

MisteR provides a Redis module (mister.so) that implements MQTT over RESP (REdis Serialization Protocol),
hence MR or MisteR. It's very incomplete.

It may provide a thin library over the Redis hiredis library to make it easy to test and use MisteR module.

It may provide an MQTT reverse proxy utilizing said library to complete a useful MQTT server implmentation.

MisteR is currently a playground for me to experiment with and to rebuild my skills in C development.

## License

It's licensed under the [MIT License](./COPYING).<sup><b>1</b></sup>

## Building

This project uses cmake.

Configuration, build, and installation are standard, e.g. for Debian:

```bash

sudo apt-get update
sudo apt-get install libhiredis-dev libev-dev
```

...or for Mac:

```zsh

brew install hiredis libev

# Create a build directory and cd into it:
```zsh

mkdir -p build ; cd build

# cmake & make
cmake .. && make
```
