# el-std

An extension to the C++ standard library adding useful features and utility classes I commonly use.

# __Disclaimer__

This is by no means a completed or production ready library. Whenever I have the need for a new feature I may add it or change old behaviour (altough avoiding doing so as it may break my own code). Currently, there are no implemented features, just a list of what I commonly use. I will collect my code and organize it in this repository when I have time and motivation to do so.

## Why el-std?

First of all, __elstd is NOT a optimized and extended replacement of the C++ standard library__. Instead, elstd is an additional C++ library that adds standard features that I frequently need or that I feel are missing from the C++ stdlib. Some features are just for convenience, some to make code shorter and more readable and some add completely new functionality.

## Compatability

Not all features of elstd are compatible with all compilers, OSes and platforms. Some features are explicitly aimed at Windows. However, many features just use the C++ stdlib and can be used with every platform and compiler. 

<br>

Every feature can be explicitly activated or deactivated to ajust for any requirements. I have not decided how that will be implemented jet.

## Feature list

None

## Planned features

 * Standardized logging functionality (LOGE, LOGD, LOGI, LOGW, LOGC)
 * retcode - a standardized return code enumeration

