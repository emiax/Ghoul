/*****************************************************************************************
 *                                                                                       *
 * GHOUL                                                                                 *
 * General Helpful Open Utility Library                                                  *
 *                                                                                       *
 * Copyright (c) 2012-2014                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include <boost/any.hpp>
#include <map>
#include <string>
#include <vector>

namespace ghoul {

/**
 * The Dictionary is a class to generically store arbitrary items associated with and
 * accessible using an <code>std::string</code>%s. It has the abilitiy to store and
 * retrieve these items by unique <code>std::string</code>-typed keys. The items that can
 * be stored have to be compatible with the <code>boost::any</code> type. This means that
 * there is no automatic type conversion performed and value have to be retrieved with
 * the same type they were constructed with.
 * Values can be added using the <code>std::initializer_list</code> constructor or can be
 * added using the #setValue method. #hasKey will check if the Dictionary has any kind of
 * value for a provided key, regardless of its type. #hasValue will perform the same check
 * but will only return <code>true</code> if the stored type agrees with the template
 * parameter. #type returns the <code>std::type_info</code> for the item stored at a given
 * location, or will return <code>typeid(void)</code> if no such key exists. All methods
 * accept recursively defined keys which use <code>.</code> as a separator. The key
 * <code>a.b</code> will first search for a Dictionary entry at the key <code>a</code> and
 * then try to find a key <code>b</code> in this second Dictionary and checks, sets, or
 * gets the corresponding value. The single exception to this is the #setValue method,
 * which has an additional parameter that controls if each individual level of the
 * Dictionary is created on-the-fly or not. None of the method will throw an exception of
 * their own, but might pass exceptions from the unterlying systems, for example
 * <code>boost::any</code>.
 */
class Dictionary : private std::map<std::string, boost::any> {
public:
	/**
	 * Creates an empty Dictionary
	 */
	Dictionary();

	/**
	 * Creates a Dictionary out of the provided <code>std::initializer_list</code>. This
	 * initializer list can be, for example, of the format
	 * <code>{ { "a", 1 }, { "b", 2 } }</code> and it will add all of the
	 * <code>std::pair</code>%s provided to the Dictionary.
     * \param l The <code>std::initializer_list</code> that contains all of the values
     * that should be added to the Dictionary
	 */
	Dictionary(const std::initializer_list<std::pair<std::string, boost::any>>& l);
    
    /**
     * Returns all of the keys that are stored in the dictionary at a given
     * <code>location</code>. This location specifier can be recursive to inspect the keys
     * at deeper levels.
     * \param location The location for which all keys should be returned
     * \return A list of all keys that are stored in the Dictionary for the provided
     * location
     */
	const std::vector<std::string> keys(const std::string& location = "") const;

    /**
     * Returns <code>true</code> if there is a specific key in the Dictionary, regardless
     * of its type. <code>key</code> can be a nested location to search for keys at deeper
     * levels.
     * \param key The key that should be checked for existence
     * \return <code>true</code> if the provided key exists in the Dictionary,
     * <code>false</code> otherwise
     */
	bool hasKey(const std::string& key) const;

    /**
     * Adds the <code>value</code> for a given location at <code>key</code>. If a value
     * already exists at that key, the old value is overwritten, regardless of its
     * previous type and without any feedback. The <code>key</code> parameter can be
     * specified as a nested location to add values to deeper levels of the Dictionary.
     * If <code>createIntermediate</code> is <code>true</code> all intermediate
     * levels in the Dictionary are created automatically; if it is <code>false</code>,
     * this function will fail and return <code>false</code> if a non-existing level is
     * encountered.
     * \param key The key at which the <code>value</code> will be entered. This can be a
     * nested location to insert value are deeper levels. Depending on the value of the
     * <code>createIntermediate</code> parameter, all intermediate levels already have to
     * exist in the Dictionary or will be created on-the-fly.
     * \param value The value that will be added into the Dictionary. If the type supports
     * it, the value will only be moved rather than copied.
     * \param createIntermediate If <code>true</code> all intermediate levels in the
     * Dictionary will be automatically created along the way, if the provided
     * <code>key</code> contains a nested location. If <code>false</code> the method will
     * fail and return <code>false</code> if a missing intermediate level is encountered.
     * \return <code>true</code> if the value was stored successfully, <code>false</code>
     * otherwise
     */
    template <typename T>
    bool setValue(const std::string& key, T&& value, bool createIntermediate = false);

    /**
     * Returns the value stored at location with a given <code>key</code>. This key can be
     * nested and will automatically be decomposed by the method to traverse to deeper
     * levels of the Dictionary. If the Dictionary has a value at the provided key and the
     * type agrees with the template parameter, the value will be copied into the provided
     * reference <code>value</code> and the method returns <code>true</code>. If either
     * the key does not exist or the type does not agree, <code>false</code> is returned
     * and the <code>value</code> is unchanged.
     * \tparam The type of the value that should be tested. The <code>typeid</code> of
     * this type has to be equal to the typeid of the value that is to be retrieved
     * \param key The, potentially nested, key for which the stored value should be
     * returned
     * \param value A reference to the value where the value will be copied to, if it
     * could be found and the types agree
     * \return <code>true</code> if the value was retrieved successfully,
     * <code>false</code> otherwise
     */
    template <typename T>
    bool getValue(const std::string& key, T& value) const;

    /**
     * Returns <code>true</code> if the Dictionary stores a value at the provided
     * <code>key</code> and the stored type agrees with the provided template parameter.
     * The key can be nested and refer to deeper levels of the Dictionary. If any of the
     * intermediate levels does not exists, <code>false</code> is returned.
     * \tparam The type of the value that should be tested. The <code>typeid</code> of
     * this type has to be equal to the typeid of the value that is to be checked
     * \param key The, potentially nested, key which should be checked for existence
     * \return <code>true</code> if the Dictionary contains a value at the specified
     * <code>key</code> with the correct type <code>T</code>. Will return
     * <code>false</code> otherwise
     */
	template <typename T>
	bool hasValue(const std::string& key) const;

    /** 
     * Returns the type of the value stored at the provided <code>key</code>. This key can
     * be nested and the method will automatically traverse the sub-Dictionaries. If any
     * of the nested levels or the <code>key</code> itself does not exist,
     * <code>typeid(void)</code> will be returned. 
     * \param The, potentially nested, key pointing to the value that type of which should
     * be returned
     * \return The <code>std::type_info</code> of the value stored at the provided
     * <code>key</code>, or <code>typeid(void)</code> if the key points to an invalid
     * location
     */
	const std::type_info& type(const std::string& key) const;

    /**
     * Returns the total number of keys stored in this Dictionary. This method will not
     * recurse into sub-Dictionaries, but will only return the top-level keys for the
     * Dictionary it is called on. This value will always be equal to the size of the
     * <code>std::vector</code> returned by #keys.
     * \return The number of keys stored in this Dictionary
     */
    size_t size() const;

    /**
     * Clears the Dictionary and leaves it in the same state as if it would just have been
     * created.
     */
    void clear();

protected:
    /**
     * Splits the provided <code>key</code> into a <code>first</code> part and the
     * <code>rest</code>. Provided a key <code>a.b.c</code>, <code>first</code> will be
     * assigned <code>a</code> and <code>rest</code> will be assigned <code>b.c</code>
     * after calling this method.
     * \param key The key that should be split
     * \param first The first part of the key
     * \param rest The rest of the key
     * \return <code>true</code> if there was a rest of the key, <code>false</code> if no
     * separator could be found
     */
	// returns if there exists a rest
	bool splitKey(const std::string& key, std::string& first, std::string& rest) const;
};

} // namespace ghoul

#include "dictionary.inl"

#endif
