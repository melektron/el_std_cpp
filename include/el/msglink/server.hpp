/*
ELEKTRON © 2022
Written by Matteo Reiter
www.elektron.work
26.11.22, 18:25
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

Preprocessor definitions for detecting C++ verisons
and enabeling supported features.

Currentyl this might not work for all compilers and it might not work at all.
In order to bypass all version checking, just 
#define __EL_ENABLE_CXX11
#define __EL_ENABLE_CXX17
to enable library features for versions not detected using the __cplusplus definition.
*/