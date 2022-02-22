# MisteR

MisteR provides a library for validating, packing & unpacking MQTT5 packets.

MisteR is currently part of a playground for me to experiment with and to rebuild my skills in C development.

Progress will be slow as there are many rabbitholes to explore.

## License

It's licensed under the [MIT License](./COPYING).<sup><b>1</b></sup>

## Building

This project uses cmake. Other projects may include it as a subproject.

Configuration, build, and installation are standard, e.g. for Mac:

```zsh

brew install zlog
```

# Create a build directory and cd into it:
```zsh

mkdir -p build ; cd build

# cmake & ninja
cmake -G Ninja .. && ninja
```
