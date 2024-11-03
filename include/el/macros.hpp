/*
ELEKTRON © 2024 - now
Written by melektron
www.elektron.work
03.11.24, 20:34
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

Little utility macros that really should be part of the C++ and/or C standard library.
*/

#pragma once

// https://stackoverflow.com/a/2653351
#define STR(x) #x       // stringify
#define XSTR(x) STR(x)  // stringify expansion

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
