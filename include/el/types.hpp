/*
ELEKTRON © 2022 - now
Written by melektron
www.elektron.work
07.10.22, 22:13
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

Additional typedefs and structure types used now and then.
*/

#pragma once

#include <stdint.h>
#include <string>
#include "strutil.hpp"

namespace el::types
{
    /**
     * @brief 24 Bit RGB color value (8 bits per color)
     */
    struct rgb24_t
    {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;

        rgb24_t() = default;

        /**
         * @brief Construct a new rgb24 t object from a 32 bit integer
         * where each of the lower 3 bytes corresponds to a color:
         * Hex: 0x--rrggbb
         *
         * @param _packet_color
         */
        rgb24_t(uint32_t _packet_color) noexcept
        {
            r = (_packet_color >> 16) & 0xFF;
            g = (_packet_color >> 8) & 0xFF;
            b = (_packet_color >> 0) & 0xFF;
        }

        /**
         * @brief Construct a new rgb24 t object from single r, g and b values
         * (8 bit per color)
         * @param _r red brightness
         * @param _g green brightness
         * @param _b blue brightness
         */
        rgb24_t(uint8_t _r, uint8_t _g, uint8_t _b)
            : r(_r), g(_g), b(_b)
        {
        }

        /**
         * @brief method to convert color to 24 (32) bit integer where each
         * of the lower 3 bytes represents a color:
         * Hex: 0x--rrggbb
         *
         * @return uint32_t packet color value
         */
        uint32_t to_packed() const noexcept
        {
            return r << 16 | g << 8 | b;
        }

        std::string to_string() const noexcept
        {
            return strutil::format("(r=%3d, g=%3d, b=%3d)", r, g, b);
        }

        /**
         * @brief calculates the brightness by adding r, g and b values.
         *
         * @return uint16_t calculated brightness value
         */
        uint16_t get_brightness() const noexcept
        {
            return r + g + b;
        }

        // == comparison operators == //

        friend bool operator==(const rgb24_t &_lhs, const rgb24_t &_rhs) noexcept
        {
            if (_lhs.r != _rhs.r)
                return false;
            if (_lhs.g != _rhs.g)
                return false;
            if (_lhs.b != _rhs.b)
                return false;
            return true;
        }
    };

    /**
     * @brief Floating point RGB color value (1 double per color)
     */
    struct rgbd_t
    {
        double r = 0;
        double g = 0;
        double b = 0;

        rgbd_t() = default;

        /**
         * @brief Construct a new rgbd_t object from single r, g and b values
         * (1 double per color)
         * @param _r red brightness
         * @param _g green brightness
         * @param _b blue brightness
         */
        rgbd_t(double _r, double _g, double _b)
            : r(_r), g(_g), b(_b)
        {
        }

        std::string to_string() const noexcept
        {
            return strutil::format("(r=%3lf, g=%3lf, b=%3lf)", r, g, b);
        }

        /**
         * @brief calculates the brightness by adding r, g and b values.
         *
         * @return double calculated brightness value
         */
        double get_brightness() const noexcept
        {
            return r + g + b;
        }

        // == comparison operators == //

        friend bool operator==(const rgbd_t &_lhs, const rgbd_t &_rhs) noexcept
        {
            if (_lhs.r != _rhs.r)
                return false;
            if (_lhs.g != _rhs.g)
                return false;
            if (_lhs.b != _rhs.b)
                return false;
            return true;
        }
    };

    // destructures RGB colors into three function parameters
    #define EL_RGB_DESTRUCTURE(c) (c).r, (c).g, (c).b

    using mac_t = uint64_t;
};