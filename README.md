# MisteR

MisteR provides a Redis module (mister.so) that implements MQTT over RESP (REdis Serialization Protocol),
hence MR or MisteR. It's very incomplete.

It may provide a thin library over the Redis hiredis library to make it easy to test and use MisteR module.

It may provide an MQTT reverse proxy utilizing said library to complete a useful MQTT server implmentation.

MisteR is currently a playground for me to experiment with and to rebuild my skills in C development.

## License

It's licensed under the [MIT License](./COPYING).<sup><b>1</b></sup>

## Building

This project uses:
 - [autoconf](https://www.gnu.org/software/autoconf/) for configuration
 - [automake](https://www.gnu.org/software/automake/) for makefile generation
 - [libtool](https://www.gnu.org/software/libtool/) to make linking easier
 - Future: [pomd4c](https://github.com/andrew-canaday/pomd4c) for documentation generation
 - [ymo_assert](https://github.com/andrew-canaday/ymo_assert) (included here) for the check targets

Configuration, build, and installation follows the classic pattern:

```bash
# NOTE: this assumes you're in the source root directory.

# prereqs for VSCode Dev Container C++:
sudo apt-get update
sudo apt-get install autoconf libtool libhiredis-dev libev-dev uuid-dev libjudy-dev

# Generate the configure script and ancillary build files:
./autogen.sh

# Create a build directory and cd into it:
mkdir -p ./build ; cd ./build

# configure & make
../configure --prefix=/my/installation/prefix && make
```
