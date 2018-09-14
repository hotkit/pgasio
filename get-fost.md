# get-fost

This helper script fetches the Fost 5 libraries needed to compile the `f5` examples together with the Boost cmake configuration required.

To use it simply run `./get-fost` from any build directory and it will download a suitable version of `f5-threading` and `fost-boost`. This is a header only library so no build step is needed, and the Boost dependency will be properly resolved when you use cmake.
