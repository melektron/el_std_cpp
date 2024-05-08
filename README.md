# __Important notice__

If you are looking for msglink development, take a look [here](https://github.com/melektron/el-std/tree/msglink_dev/include/el/msglink) in the [msglink_dev](https://github.com/melektron/el-std/tree/msglink_dev) branch, where the C++ msglink implementation is developed.

---

# el-std

An header-only extension to the C++ standard library adding useful features and utility classes I commonly use.

# __Disclaimer__

This is by no means a completed or production ready library. Whenever I have the need for a new feature I may add it or change old behaviour (altough avoiding doing so as it may break my own code). 

## Why el-std?

First of all, __elstd is NOT a optimized and extended replacement of the C++ standard library__. Instead, el-std is an additional C++ library that adds standard features that I frequently need or that I feel are missing from the C++ stdlib. Some features are just for convenience, some to make code shorter and more readable and some add completely new functionality.

## Compatability

Not all features of elstd are compatible with all compilers, OSes and platforms. Some features are explicitly aimed at Windows. However, many features just use the C++ stdlib and can be used with every platform and compiler. It should also be noted, that feature compatability is mostly not explicitly tested on all operating systems before it is added. I might test a feature on Linux or Windows if that is where I currently need it and I will fix compatability issues whenever I need to use a feature on a specific platform and it doesn't work. If you find an issue and know how to fix it, feel free to submit a pull request and I will look into it as soon as possible.

<br>

Features are separated into their own headers and can be included independently. However, it is to be noted that many headers depend on other headers internally.

## Feature list

 * namespace __```"el::types"```__ (el/types.hpp)
    * struct  __```"rgb24_t"```__ (RGB color type)
 * namespace __```"el::strutil"```__ (el/strutil.hpp)
    * function __```"format"```__ (Format string creator function)
 * class __```"el::universal"```__ (el/universal.hpp) (universal data container)

## Planned features

 * Standardized logging functionality (LOGE, LOGD, LOGI, LOGW, LOGC)
 * retcode - a standardized return code enumeration

