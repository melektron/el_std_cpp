/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
05.11.23, 18:28
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

Codable helper classes and macros for defining structures
and classes that can be encoded to and/or decoded from json.

This functionality is based on Niels Lohmann's JSON for modern C++ library
and depends on it. It must be includable as follows:

#include <nlohmann/json.hpp>
*/

#pragma once

#include <functional>

#include <nlohmann/json.hpp>

#include <el/metaprog.hpp>


namespace el
{
    /**
     * @brief Class interface for 
     * decodable structures and classes.
     * 
     * This also provides the converter function from_json()
     * which allows integration with the nlohmann::json library's
     * builtin conversion system. This generates an operator allowing
     * the assignment of a json object to any decodable object.
     */
    class decodable
    {
    protected:
        virtual ~decodable() = default;

    public:
        /**
         * @brief function which decodes the decodable
         * object from json-encoded data
         *
         * @param _output the json instance to decode.
         * This can be a number, object, list or whatever json type
         * is used to represent this codable. Invalid type will throw
         * a decoder exception.
         */
        virtual void _el_codable_decode(const nlohmann::json &_input) = 0;

        /**
         * @brief function to convert this decodable from json using
         * the functionality provided by the nlohmann::json library
         *
         * @param _j_output json instance to decode
         * @param _t_input decodable to decode from json
         */
        friend void from_json(const nlohmann::json &_j_input, decodable &_t_output)
        {
            _t_output._el_codable_decode(_j_input);
        };
    };
    
    /**
     * @brief Class interface for 
     * encodable structures and classes.
     * 
     * This also provides the converter function to_json()
     * which allows integration with the nlohmann::json library's
     * builtin conversion system. This generates an operator allowing
     * the assignment of any decodable object to a json object.
     */
    class encodable
    {
    protected:
        virtual ~encodable() = default;

    public:

        /**
         * @brief function which encodes the encodable's
         * members to json.
         *
         * @param _output the json instance to save the json encoded object to.
         * This might be converted to number, object, list or whatever json type
         * is used to represent this encodable.
         */
        virtual void _el_codable_encode(nlohmann::json &_output) const = 0;

        /**
         * @brief function to convert this encodable to json using
         * the functionality provided by the nlohmann::json library
         *
         * @param _j_output json instance to encode to
         * @param _t_input encodable to encode
         */
        friend void to_json(nlohmann::json &_j_output, const encodable &_t_input)
        {
            _t_input._el_codable_encode(_j_output);
        }
    };

    /**
     * @brief Class interface for
     * codable structures and classes.
     * 
     * This combines encodable and decodable
     * for objects that need to be both en- and decoded.
     */
    class codable : public encodable, public decodable
    {
    protected:
        virtual ~codable() = default;
    };


/**
 * Automatic constructor generation
 * 
 */

// (private) generates a constructor argument for a structure member (of a codable)
#define __EL_CODABLE_CONSTRUCTOR_ARG(member) decltype(member) & _ ## member, 
// (private) generates a constructor initializer list entry for a member from the above defined argument
#define __EL_CODABLE_CONSTRUCTOR_INIT(member) member(_ ## member), 

// (public) generates a constructor for a given structure which initializes the given members
#define EL_CODABLE_GENERATE_CONSTRUCTORS(TypeName, ...)                                         \
    private:                                                                                    \
    inline int __el_codable_ctorgen_dummy;                                                      \
    public:                                                                                     \
    TypeName() = default;                                                                       \
    TypeName(EL_METAPROG_DO_FOR_EACH(__EL_CODABLE_CONSTRUCTOR_ARG, __VA_ARGS__) char __dummy = 0)  \
        : EL_METAPROG_DO_FOR_EACH(__EL_CODABLE_CONSTRUCTOR_INIT, __VA_ARGS__)                      \
          __el_codable_ctorgen_dummy(__dummy) /* dummy is because comma left by macro */        \
    {}

/**
 * Encoding/Decoding code generation
 * 
 */

// (private) generates code which uses a member's encoder function to add it to a json object
#define __EL_CODABLE_ENCODE_KEY(member) encode_ ## member (_output[#member]);
// (private) generates code which uses a member's decoder function to retrieve it's value from a json object
#define __EL_CODABLE_DECODE_KEY(member) decode_ ## member (_input.at(#member));

// (public) generates the declaration of the encoder method for a specific member
#define EL_ENCODER(member) void encode_ ## member (nlohmann::json &encoded_data) const
// (public) generates the declaration of the decoder method for a specific member
#define EL_DECODER(member) void decode_ ## member (const nlohmann::json &encoded_data)

// (private) generates the default encoder method for a member
#define __EL_CODABLE_DEFINE_DEFAULT_ENCODER(member)                                     \
    /* these dummy templates make this function less specialized than one without,      \
       so the user can manually define their encoder which will take precedence over    \
       this one */                                                                      \
    template <class _D = int>                                                           \
    EL_ENCODER(member)                                                                  \
    {                                                                                   \
        encoded_data = member;                                                          \
    }

// (private) generates the default decoder method for a member
#define __EL_CODABLE_DEFINE_DEFAULT_DECODER(member)                                     \
    /* these dummy templates make this function less specialized than one without,      \
       so the user can manually define their encoder which will take precedence over    \
       this one */                                                                      \
    template <class _D = int>                                                           \
    EL_DECODER(member)                                                                  \
    {                                                                                   \
        member = encoded_data;                                                          \
    }

// (private) generates the default encoder/decoder methods for a class member
#define __EL_CODABLE_DEFINE_DEFAULT_CONVERTERS(member)                                  \
    __EL_CODABLE_DEFINE_DEFAULT_ENCODER(member)                                         \
    __EL_CODABLE_DEFINE_DEFAULT_DECODER(member)

// (public) generates the methods necessary to make a structure encodable. 
// Only the provided members will be made encodable, the others will not be touched.
#define EL_DEFINE_ENCODABLE(Name, ...)                                                  \
                                                                                        \
    EL_METAPROG_DO_FOR_EACH(__EL_CODABLE_DEFINE_DEFAULT_ENCODER, __VA_ARGS__)           \
                                                                                        \
    virtual void _el_codable_encode(nlohmann::json &_output) const override             \
    {                                                                                   \
        EL_METAPROG_DO_FOR_EACH(__EL_CODABLE_ENCODE_KEY, __VA_ARGS__)                   \
    }

// (public) generates the methods necessary to make a structure decodable. 
// Only the provided members will be made decodable, the others will not be touched.
#define EL_DEFINE_DECODABLE(Name, ...)                                                  \
                                                                                        \
    EL_METAPROG_DO_FOR_EACH(__EL_CODABLE_DEFINE_DEFAULT_DECODER, __VA_ARGS__)           \
                                                                                        \
    virtual void _el_codable_decode(const nlohmann::json &_input) override              \
    {                                                                                   \
        EL_METAPROG_DO_FOR_EACH(__EL_CODABLE_DECODE_KEY, __VA_ARGS__)                   \
    }

// (public) generates the methods necessary to make a structure codable (encodable and decodable). 
// Only the provided members will be made encodable/decodable, the others will not be touched.
#define EL_DEFINE_CODABLE(Name, ...)                                                    \
    EL_DEFINE_ENCODABLE(Name, __VA_ARGS__)                                              \
    EL_DEFINE_DECODABLE(Name, __VA_ARGS__)                                              \

}   // namespace el