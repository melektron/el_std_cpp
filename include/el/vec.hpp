/*
ELEKTRON © 2023
Written by Matteo Reiter
www.elektron.work
15.02.23, 12:22
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

mathematical Vector class(es) that can smoothyl transistion from polar
to cartesian coordinates and support common mathematical operations.
*/

#pragma once

#include <cmath>

namespace el
{

    // A cartesian coordinate (x and y values)
    struct cart_t
    {
        double x = 0, y = 0;
        cart_t() = default;
        cart_t(double _x, double _y) : x(_x), y(_y) {}
    };

    // A polar coordinate (r and phi)
    struct polar_t
    {
        double r = 0, phi = 0;
        polar_t() = default;
        polar_t(double _r, double _phi) : r(_r), phi(_phi) {}
    };

    /**
     * @brief A vector representing a location or distance that allows seamless
     * conversion between polar and cartesian coordinates
     *
     */
    struct vec2_t
    {
    protected:
        cart_t cartesian;
        polar_t polar;

        // updates contained polar value from cartesian value
        void update_polar_from_cart()
        {
            polar.r = std::sqrt(
                std::pow(cartesian.x, 2) + std::pow(cartesian.y, 2)
            );
            polar.phi = std::atan2(cartesian.y, cartesian.x);
        }
        // updates contained cartesian value from polar value
        void update_cart_from_polar()
        {
            cartesian.x = polar.r * std::cos(polar.phi);
            cartesian.y = polar.r * std::sin(polar.phi);
        }

    public:
        // default constructor init all to 0
        vec2_t() = default;
        // initialize from cartesian coordinate
        vec2_t(cart_t _value)
        : cartesian(_value)
        {
            update_polar_from_cart();
        }
        // initialize from polar coordinate
        vec2_t(polar_t _value)
        : polar(_value)
        {
            update_cart_from_polar();
        }
        // creates vec2_t from raw cartesian values
        static vec2_t from_xy(double _x, double _y)
        {
            return cart_t(_x, _y);
        }
        // creates vec2_t from raw polar values
        static vec2_t from_rphi(double _r, double _phi)
        {
            return polar_t(_r, _phi);
        }

        // might be required for inheritance
        virtual ~vec2_t() = default;

        // == raw value getters
        double get_x() const { return cartesian.x; }
        double get_y() const { return cartesian.y; }
        double get_r() const { return polar.r; }
        double get_phi() const { return polar.phi; }

        // == raw value setters
        void set_x(double _x)
        {
            cartesian.x = _x;
            update_polar_from_cart();
        }
        void set_y(double _y)
        {
            cartesian.y = _y;
            update_polar_from_cart();
        }
        void set_xy(double _x, double _y)
        {
            cartesian.x = _x;
            cartesian.y = _y;
            update_polar_from_cart();
        }
        void set_r(double _r)
        {
            polar.r = _r;
            update_cart_from_polar();
        }
        void set_phi(double _phi)
        {
            polar.phi = _phi;
            update_cart_from_polar();
        }
        void set_rphi(double _r, double _phi)
        {
            polar.r = _r;
            polar.phi = _phi;
            update_cart_from_polar();
        }

        // == coordinate setters
        void set_cart(const cart_t &_value)
        {
            cartesian = _value;
            update_polar_from_cart();
        }
        void set_polar(const polar_t &_value)
        {
            polar = _value;
            update_cart_from_polar();
        }

        // == conversions to coordinate types
        const cart_t &to_cart() const { return cartesian; }
        const polar_t &to_polar() const { return polar; }
        operator cart_t() const { return cartesian; }
        operator polar_t() const { return polar; }

        // == mathematical operations
        // = addition and subtraction
        // vector addition: separately adds x and y values
        vec2_t operator+(const vec2_t &_rhs) const
        {
            return cart_t(
                cartesian.x + _rhs.cartesian.x,
                cartesian.y + _rhs.cartesian.y
            );
        }
        vec2_t &operator+=(const vec2_t &_rhs)
        {
            cartesian.x += _rhs.cartesian.x;
            cartesian.y += _rhs.cartesian.y;
            update_polar_from_cart();
            return *this;
        }
        // scalar addition: increases the vector's length by the given number
        vec2_t operator+(double _rhs) const
        {
            return polar_t(
                polar.r + _rhs,
                polar.phi
            );
        }
        vec2_t &operator+=(double _rhs)
        {
            polar.r += _rhs;
            update_cart_from_polar();
            return *this;
        }
        // numeric addition: adds the length of the vector to the number
        friend double operator+(double _lhs, const vec2_t &_rhs)
        {
            return _lhs + _rhs.polar.r;
        }
        // vector subtraction: separately subtracts x and y values
        vec2_t operator-(const vec2_t &_rhs) const
        {
            return cart_t(
                cartesian.x - _rhs.cartesian.x,
                cartesian.y - _rhs.cartesian.y
            );
        }
        vec2_t &operator-=(const vec2_t &_rhs)
        {
            cartesian.x -= _rhs.cartesian.x;
            cartesian.y -= _rhs.cartesian.y;
            update_polar_from_cart();
            return *this;
        }
        // scalar subtraction: reduces the vector's length by the given number
        vec2_t operator-(double _rhs) const
        {
            return polar_t(
                polar.r - _rhs,
                polar.phi
            );
        }
        vec2_t &operator-=(double _rhs)
        {
            polar.r -= _rhs;
            update_cart_from_polar();
            return *this;
        }
        // numeric subtraction: subtracts the length of the vector from the number
        friend double operator-(double _lhs, const vec2_t &_rhs)
        {
            return _lhs - _rhs.polar.r;
        }
        // = multiplication and division
        // scalar multiplication: multiplies the length of the vector with the number
        vec2_t operator*(double _rhs) const
        {
            return polar_t(
                polar.r * _rhs,
                polar.phi
            );
        }
        vec2_t &operator*=(double _rhs)
        {
            polar.r *= _rhs;
            update_cart_from_polar();
            return *this;
        }
        friend vec2_t operator*(double _lhs, const vec2_t &_rhs)
        {
            return polar_t(
                _rhs.polar.r * _lhs,
                _rhs.polar.phi
            );
        }
        // scalar division: divides the length of the vector by the number
        vec2_t operator/(double _rhs) const
        {
            return polar_t(
                polar.r / _rhs,
                polar.phi
            );
        }
        vec2_t &operator/=(double _rhs)
        {
            polar.r /= _rhs;
            update_cart_from_polar();
            return *this;
        }
        // numeric division: divides the number by the length of the vector
        //friend double operator/(double _lhs, const vec2_t &_rhs);
    };

// macro to convert vector to a pair of arguments (x, y) for calls to external code that requires such
#define XY(vector) ((vector).get_x()), ((vector).get_y())

    /**
     * @brief a pair of Vec2s that can be used to represent lines,
     * rectangles, circles, ...
     * Usually, the first vector is the start point of a line or one corner
     * of a rectangle and the other vector defines the end point of the line or
     * the opposite corner of the rectangle. Sometimes, the second coordinate is
     * also used to specify the width and height of the rectangle or line.
     *
     */
    struct pair2_t
    {
        vec2_t first, second;
    };
};
