## Debian package demo

### Prerequisites

- waf [Waf The build system](https://waf.io/)
- git-python `pip install git-python`

### Documentation

[TODO]

### Build and run

This project uses waf to build and package deb.

```shell
waf distclean configure --prefix=./out/usr/local/debian-demo --mode=product build install && ./package.sh
```

Then you install with `dpkg` and run it using `systemctl`.

```shell
sudo dpkg -i debian-demod_{auto generated version}_amd64.deb
sudo systemctl start debian-demod.service
```

Also you can run the executable file `debian_demo` in `build` folder.

```shell
./build/debian-demo
```

Help info

```shell
./build/debian-demo -h
Debian Demo app usage:
Usage:
  debian-demo [OPTION...]

  -h, --help        show help information
  -v, --version     show version info
      --host arg    server run host - default 127.0.0.1
      --wsport arg  websocket server run port - default 8080
      --hsport arg  http server run port - default 80
```

### Third-Party Libraries

- **machinezone/IXWebSocket** - [https://github.com/machinezone/IXWebSocket](https://github.com/machinezone/IXWebSocket)
- **jarro2783/cxxopts** - [https://github.com/jarro2783/cxxopts](https://github.com/jarro2783/cxxopts)
- **nlohmann/json** - [https://github.com/nlohmann/json](https://github.com/nlohmann/json)
- **yhirose/cpp-httplib** - [https://github.com/yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib)

See `libs/LICENSES` for third party licenses
