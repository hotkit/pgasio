# get-fost

This helper script fetches the Fost 5 libraries needed to compile the `f5` examples.

To use it simply run `./get-fost` from any directory it will download a suitable version of `f5-threading`. This is a header only library so no build step is needed. You should use `get-boost` as well because you will need Boost too.

    BOOST_BUILD_PATH=./boost_1_62_0/ ./boost_1_62_0/bjam threading=multi test examples examples/f5 include=boost/include include=f5-threading/include debug search=boost/debug
