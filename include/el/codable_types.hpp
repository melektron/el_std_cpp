/*
ELEKTRON © 2024 - now
Written by melektron
www.elektron.work
11.01.24, 09:28
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

Additional codable support for standard and extended types that are commonly
used.

This functionality is based on Niels Lohmann's JSON for modern C++ library
and depends on it. It must be includable as follows:

#include <nlohmann/json.hpp>
*/

#pragma once

#include <optional>
#include <nlohmann/json.hpp>

namespace el::codable_types
{
    /**
     * Codables use the "decode_from_object" and "encode_to_object" methods that can be overloaded
     * to support various type conversions.
     *
     * Unlike the functionality provided by nlohmann's JSON library ("to_json" and "from_json"),
     * these functions get the entire context of the containing object. This allows them to be
     * more flexible, such as coding optionals.
     * 
     * The namespace is called "el::codable_types" and not just "el" because this namespace may be extended
     * by the user of the library to support conversion of custom types.
     */


    /**
     * @brief object key decoder using nlohmann's generic decoding
     * mechanism. This enables the decoding of any C++ object
     * supported by nlohmann JSON and any types made decodable using
     * this mechanism.
     * 
     * @tparam _T datatype to decode from the object
     * @param _object json object to decode from
     * @param _key the key in the above object to decode
     * @param _out_data destination of decoded data
     */
    template<typename _T>
    void decode_from_object(
        const nlohmann::json &_object,
        const std::string &_key,
        _T &_out_data
    )
    {
        // this uses the from_json() functions to decode
        _out_data = _object.at(_key).get<_T>();
    }
    
    /**
     * @brief object key encoder using nlohmann's generic encoding
     * mechanism. This enables the encoding of any C++ object
     * supported by nlohmann JSON and any types made encodable using
     * this mechanism.
     * 
     * @tparam _T datatype to encode to the object
     * @param _object json object to store encoded output in
     * @param _key the key in the above object to store the encoded output
     * @param _in_data data to encode
     */
    template<typename _T>
    void encode_to_object(
        nlohmann::json &_object,
        const std::string &_key,
        const _T &_in_data
    )
    {
        _object[_key] = _in_data;
    }

    /**
     * @brief object key decoder overload for optional types.
     * This enables the decoding std::optionals from json, for any
     * contained type that can be decoded.
     * If key is not found in object, optional will be reset (nullptr). 
     * If key is found, optional is assigned the decoded value.
     * 
     * @tparam _T datatype contained in optional
     * @param _object json object to decode from
     * @param _key the key in the above object to decode
     * @param _out_data destination optional of decoded data
     */
    template<typename _OT>
    void decode_from_object(
        const nlohmann::json &_object,
        const std::string &_key,
        std::optional<_OT> &_out_data
    )
    {
        if (!_object.contains(_key))
            _out_data.reset();
        else
            _out_data = _object.at(_key).get<_OT>();
    }
    
    /**
     * @brief object key encoder overload for optional types.
     * This enables the encoding std::optionals to json, for any
     * contained type that can be encoded.
     * If optional is empty (nullptr), the key will not be added
     * to output object (if it's already ther, it's untouched).
     * If the optional contains a value, it is encoded and added to
     * the json object.
     * 
     * @tparam _T datatype contained in optional
     * @param _object json object to store encoded output in
     * @param _key the key in the above object to store the encoded output
     * @param _in_data optional data to encode
     */
    template<typename _OT>
    void encode_to_object(
        nlohmann::json &_object,
        const std::string &_key,
        const std::optional<_OT> &_in_data
    )
    {
        if (_in_data.has_value())
            _object[_key] = *_in_data;
    }

} // namespace codable_types


NLOHMANN_JSON_NAMESPACE_BEGIN
/**
 * @brief (de)serializer for std::optional
 * See https://json.nlohmann.me/features/arbitrary_types/#how-do-i-convert-third-party-types
 * for explanation why namespace is needed.
 * This has been replaced by the custom encoder/decoder method supporting more functionality.
 * @tparam _T contained type
 */
//template <typename _T>
//struct adl_serializer<std::optional<_T>>
//{
//    /**
//     * @brief nlohmann JSON decoder for optionals. Expects JSON null for 
//     * empty optional case and the value otherwise.
//     * 
//     * @param _j_input input json data
//     * @param _t_output decoded optional
//     */
//    static void from_json(const nlohmann::json &_j_input, std::optional<_T> &_t_output)
//    {
//        if (_j_input.is_null())
//            _t_output.reset();
//        else
//            _t_output = _j_input.get<_T>();
//    };
//
//    /**
//     * @brief nlohmann JSON encoder for optionals. Emits JSON null for
//     * empty optional case and the encoded value otherwise.
//     * 
//     * @tparam _T contained type
//     * @param _j_output output json data
//     * @param _t_input optional to encode
//     */
//    static void to_json(nlohmann::json &_j_output, const std::optional<_T> &_t_input)
//    {
//        if (!_t_input.has_value())
//            _j_output = nullptr;
//        else
//            _j_output = *_t_input;
//    }
//
//};
NLOHMANN_JSON_NAMESPACE_END