# get-boost

This helper script fetches a Boost version for you which you can use to do some test builds and build some of the examples.

To use it simply run `./get-boost` from any directory and it will download Boost 1.62.0 and bulid release versions of the two libraries needed. The libraries will be in the `boost/release` and `boost/debug` folders and all of the Boost headers will be in `boost/include`.

To use it with the bjam build files included with pgasio do:

    cd pgasio # You must be in the pgasio root directory
    BOOST_BUILD_PATH=./boost_1_62_0/ ./boost_1_62_0/bjam test include=boost/include search=boost/lib

As well as the `test` build target you can also use `examples`:

    cd pgasio # You must be in the pgasio root directory
    BOOST_BUILD_PATH=./boost_1_62_0/ ./boost_1_62_0/bjam test examples include=boost/include debug search=boost/debug

