# hdf5.uws
Header only C++ server component and middleware for [https://github.com/uNetworking/uWebSockets](https://github.com/uNetworking/uWebSockets) for moving datasets and images directly to web apps

The client API HDF5Interface.js works the same as the one in [hdf5.ws](https://github.com/HDF-NI/hdf5.ws)

### Installing

Currently using yarn bacause it will clone git repositories lacking a package.json

```
yarn config set hdf5_home_linux <your native hdf path>
yarn install
```
First time need to build openssl and [urlcpp](https://github.com/rimmartin/). Note: this fork
of urlcpp is independent from boost libraries and will gain more speedups using c++2a

```
yarn ssl
yarn urlcpp

```

Also need LD_LIBRARY_PATH set to include native hdf5 and the liburlcpp.so

### App usage

```
    #include "h5.h"
    #include "h5images.h"

    pmc::h5 h5{.currentH5Path={"./newone.h5"}};
    h5.open();
    pmc::h5images<true> h5images{.h5=h5, .port={8888}};
    h5images.make.h5=h5;
    h5images.read.h5=h5;
    h5images.readRegion.h5=h5;
    h5images.readMosaic.h5=h5;


```


### Testing - Current Status

The tests are performed by cypress.

```
yarn test
```
