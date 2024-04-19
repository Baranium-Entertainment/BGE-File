/**
 * @file BgeConfig.hpp
 * @author GAMINGNOOBdev (https://github.com/GAMINGNOOBdev)
 * @brief A somewhat nice and easy config file reader utility in a single header for C++
 * @note This header does depend on BgeFile.hpp
 * @version 1.0
 * @date 2024-04-19
 * 
 * @copyright Copyright (c) GAMINGNOOBdev 2024
 */

#ifndef __BGECONFIG_HPP_
#define __BGECONFIG_HPP_ 1

#include <BgeFile.hpp>
#include <algorithm>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

#ifndef _WIN32
#   include <iomanip>
#   include <sstream>
#else
#   include <sstream>
#endif

/**
 * Type of a configuration properties' value
*/
enum class BgePropertyValueType : uint8_t
{
    UNKNOWN,
    STRING,
    FLOAT,
    BOOL,
    INT,
};

/**
 * Property of a configuration file
*/
struct BgeConfigProperty
{
    // Name of the property
    std::string Name;

    // Type of this properties's value
    BgePropertyValueType Type;

    // String value of this property
    std::string StrValue;

    // Integer value of this property
    int IntValue;

    // Floating-point value of this property
    float FloatValue;

    // Boolean value of this property
    bool BoolValue;

public:
    /**
     * @brief Construct a new `BgeConfigProperty` object
     */
    BgeConfigProperty()
    {
        Type = BgePropertyValueType::UNKNOWN;
        Name = "";
        StrValue = "";
        IntValue = 0;
        FloatValue = 0;
        BoolValue = false;
    }

    /**
     * @brief Construct a new `BgeConfigProperty` object
     * 
     * @param type Property type
     * @param name Property name
     * @param strValue String value
     * @param intValue Integer value
     * @param floatValue Float value
     * @param boolValue Boolean value
     */
    BgeConfigProperty(BgePropertyValueType type, std::string name = "", std::string strValue = "", int intValue = 0, float floatValue = 0.f, bool boolValue = false)
    {
        Type = type;
        Name = name;
        StrValue = strValue;
        IntValue = intValue;
        FloatValue = floatValue;
        BoolValue = boolValue;
    }

    /**
     * @brief Save this property to a file
     * 
     * @param file The file to which we save
     */
    void Save(BgeFile& file)
    {
        std::string output = std::string(Name).append(" = ");
        switch(Type)
        {
            case BgePropertyValueType::INT:
                output.append(std::to_string(IntValue));
                break;

            case BgePropertyValueType::FLOAT:
                output.append(std::to_string(FloatValue));
                break;
            
            case BgePropertyValueType::BOOL:
            {
                BoolValue ? output.append("true") : output.append("false");
                break;
            }

            default:
            case BgePropertyValueType::UNKNOWN:
            case BgePropertyValueType::STRING:
                output.append(StrValue);
                break;
        }

        file.WriteLine(output);
    }

    /**
     * @brief Compare if two `BgeConfigProperty` objects are equal
     */
    bool operator==(BgeConfigProperty& other)
    {
        return Type == other.Type && Name == other.Name; // it is enough if the name and type are the same
    }

    /**
     * @brief Compare if two `BgeConfigProperty` objects are not equal
     */
    bool operator!=(BgeConfigProperty& other)
    {
        return !operator==(other);
    }
};

struct BgeConfigSection;
using BgeConfigPropertyList = std::vector<BgeConfigProperty>;
using BgeConfigSectionList = std::vector<BgeConfigSection>;

struct BgeConfigSection
{
    // Name of the section
    std::string Name;

public:
    /**
     * @brief Construct a new empty `BgeConfigSection` object
     */
    BgeConfigSection()
        : Name(), mProperties(), mNestedSections()
    {
    }

    /**
     * @brief Construct a new `BgeConfigSection` object
     * 
     * @param name The section name
     */
    BgeConfigSection(std::string name)
        : Name(name)
    {
    }

    /**
     * @brief Save this section to a file
     * 
     * @param file The file to which we save
     * @param sectionPrefix The prefix that should be added to the full section name
     */
    void Save(BgeFile& file, std::string sectionPrefix = "")
    {
        std::string sectionName = "[";
        if (!sectionPrefix.empty())
            sectionName.append(sectionPrefix).append(".");
        sectionName.append(Name).append("]");

        file.WriteLine(sectionName);
        for (auto& property : mProperties)
            property.Save(file);
        file.WriteLine();

        std::string newSectionPrefix = std::string(sectionPrefix);
        if (!newSectionPrefix.empty())
            newSectionPrefix.append(".");
        newSectionPrefix.append(Name);

        for (auto& subSection : mNestedSections)
            subSection.Save(file, newSectionPrefix);
    }

    /**
     * Get a specific property of this configuration file
     * @param name name of the desired property, not including the name of this section
     * @returns desired property, NULL if not found
    */
    BgeConfigProperty* Get(std::string name)
    {
        if (name.empty())
            return nullptr;

        size_t sectionDivider = name.find_first_of('.');
        if (sectionDivider == std::string::npos)
        {
            auto propertyIterator = GetPropertyIterator(name);
            return (propertyIterator == mProperties.end()) ? nullptr : &*propertyIterator;
        }

        std::string sectionName = name.substr(0, sectionDivider);
        std::string nextSectionName = name.substr(sectionDivider+1);

        if (!HasSubSection(sectionName))
            return nullptr;

        return GetSubSection(sectionName)->Get(nextSectionName);
    }

    /**
     * Get a specific section of this configuration file
     * @param name name of the desired section, not including the name of this section
     * @returns desired section, NULL if not found
    */
    BgeConfigSection* GetSubSection(std::string name)
    {
        if (name.empty())
            return nullptr;

        size_t sectionDivider = name.find_first_of('.');
        if (sectionDivider == std::string::npos)
        {
            auto sectionIterator = GetSectionIterator(name);
            return (sectionIterator == mNestedSections.end()) ? nullptr : &*sectionIterator;
        }

        std::string sectionName = name.substr(0, sectionDivider);
        std::string nextSectionName = name.substr(sectionDivider+1);

        if (!HasSubSection(sectionName))
            return nullptr;
        
        return GetSubSection(sectionName)->GetSubSection(nextSectionName);
    }

    /**
     * @brief Check if a property exists in this configuration file
     * 
     * @param name Name of the property, not including the name of this section
     * @returns `true` if the property exists, otherwise `false`
     */
    bool HasProperty(std::string name)
    {
        if (name.empty())
            return false;

        size_t sectionDivider = name.find_first_of('.');
        if (sectionDivider == std::string::npos)
            return GetPropertyIterator(name) != mProperties.end();

        std::string sectionName = name.substr(0, sectionDivider);
        std::string nextSectionName = name.substr(sectionDivider+1);

        if (!HasSubSection(sectionName))
            return false;
        
        return GetSubSection(sectionName)->HasProperty(nextSectionName);
    }

    /**
     * @brief Check if a sub-section exists in this configuration file
     * 
     * @param name Name of the sub-section, not including the name of this section
     * @returns `true` if the sub-section exists, otherwise `false`
     */
    bool HasSubSection(std::string name)
    {
        if (name.empty())
            return false;

        size_t sectionDivider = name.find_first_of('.');
        if (sectionDivider == std::string::npos)
            return GetSectionIterator(name) != mNestedSections.end();
        
        std::string sectionName = name.substr(0, sectionDivider);
        std::string nextSectionName = name.substr(sectionDivider+1);

        if (!HasSubSection(sectionName))
            return false;
        
        return GetSubSection(sectionName)->HasSubSection(nextSectionName);
    }

    /**
     * Add a configuration property with a specific type
     * @param name name of the property, not including the name of this section
     * @param property the property object, can should contain the name of the property itself, not the full property "path"
    */
    void AddProperty(std::string name, BgeConfigProperty& property)
    {
        if (name.empty())
            return;

        if (HasProperty(name))
            return;
        
        size_t sectionDivider = name.find_first_of('.');
        if (sectionDivider == std::string::npos)
        {
            property.Name = name;
            mProperties.push_back(property);
            return;
        }

        std::string sectionName = name.substr(0, sectionDivider);
        std::string nextSectionStr = name.substr(sectionDivider+1);

        BgeConfigSection* nextSection = nullptr;
        if (!HasSubSection(sectionName))
            nextSection = AddSubSection(sectionName);
        else
            nextSection = GetSubSection(sectionName);

        if (nextSection == nullptr)
            return;

        nextSection->AddProperty(nextSectionStr, property);
    }

    /**
     * Add a configuration sub-section
     * 
     * @param name name of the sub-section, not including the name of this section
     * 
     * @returns A reference to the section object for modification purposes
    */
    BgeConfigSection* AddSubSection(std::string name)
    {
        if (name.empty())
            return nullptr;

        if (HasSubSection(name))
            return GetSubSection(name);

        size_t sectionDivider = name.find_first_of('.');
        if (sectionDivider == std::string::npos)
        {
            BgeConfigSection section = BgeConfigSection(name);
            mNestedSections.push_back(section);
            return &mNestedSections.back();
        }

        std::string sectionName = name.substr(0, sectionDivider);
        std::string nextSectionName = name.substr(sectionDivider+1);

        return AddSubSection(sectionName)->AddSubSection(nextSectionName);
    }

    /**
     * @brief Compare if two `BgeConfigSection` objects are equal
     */
    bool operator==(BgeConfigSection& other)
    {
        return Name == other.Name && mProperties.size() == other.mProperties.size() && mNestedSections.size() == other.mNestedSections.size();
    }

    /**
     * @brief Compare if two `BgeConfigSection` objects are not equal
     */
    bool operator!=(BgeConfigSection& other)
    {
        return !operator==(other);
    }

private:
    BgeConfigPropertyList::iterator GetPropertyIterator(std::string name)
    {
        return std::find_if(mProperties.begin(), mProperties.end(), [name](BgeConfigProperty& other){ return name == other.Name; });
    }

    BgeConfigSectionList::iterator GetSectionIterator(std::string name)
    {
        return std::find_if(mNestedSections.begin(), mNestedSections.end(), [name](BgeConfigSection& other){ return name == other.Name; });
    }

private:
    BgeConfigSectionList mNestedSections;
    BgeConfigPropertyList mProperties;
};

/**
 * Configuration file reader
 * @note doesn't fully work for .ini like formats
*/
struct BgeConfig
{
    /**
     * Creates a new `BgeConfig` object
    */
    BgeConfig()
        : mProperties(), mSections()
    {
    }

    /**
     * Closes this `BgeConfig` object
     * @note this does NOT save the configuration automatically
    */
    void Close()
    {
        mProperties.clear();
        mSections.clear();
    }

    /**
     * Open and load a configuration file
     * @param path file path to the configuration file to load
    */
    void Open(std::string path)
    {
        BgeFile file = BgeFile(path, false);
        if (!file.Ready())
            return;

        Close();

        std::string line;
        std::string cleanLine;
        std::string sectionName;
        std::string split0, split1;
        BgeConfigSection* currentSection = nullptr;

        while (!file.EndOfFile())
        {
            line = file.ReadLine();

            // to ensure we are not trying to parse empty lines
            if (line.length() < 1)
                continue;

            // if the line starts with "//" it is a comment and should be ignored
            if (line.substr(0, 2) == "//")
                continue;

            // remove any whitespaces at the front or end
            cleanLine = StringTrimComment(line);
            cleanLine = StringTrimLeading(cleanLine);

            // check if this is a section name or a section end
            if (cleanLine.at(0) == '[' && cleanLine.at(cleanLine.length()-1) == ']')
            {
                sectionName = cleanLine.substr(1, cleanLine.length()-2);

                if (!HasSection(sectionName))
                    currentSection = AddSection(sectionName);
                else
                    currentSection = GetSection(sectionName);
                continue;
            }

            if (cleanLine == "[SECTIONEND]")
            {
                currentSection = nullptr;
                sectionName = "";
                continue;
            }

            // split line

            size_t equalSignIdx = cleanLine.find_last_of('=');
            if (equalSignIdx == std::string::npos)
                continue;

            split0 = StringTrimLeading(cleanLine.substr(0, equalSignIdx));
            split1 = StringTrimLeading(cleanLine.substr(equalSignIdx+1, cleanLine.length()-equalSignIdx-1));

            BgePropertyValueType estimateType = EstimateValueType(split1);
            BgeConfigProperty property = BgeConfigProperty();
            switch(estimateType)
            {
                case BgePropertyValueType::INT:
                    property = BgeConfigProperty(estimateType, split0, split1, std::stoi(split1));
                    break;

                case BgePropertyValueType::FLOAT:
                    property = BgeConfigProperty(estimateType, split0, split1, 0, std::stod(split1), false);
                    break;

                case BgePropertyValueType::BOOL:
                {
                    bool value = split1 == "true" || split1 == "True" || split1 == "1";
                    property = BgeConfigProperty(estimateType, split0, split1, 0, 0.0, value);
                    break;
                }

                default:
                case BgePropertyValueType::UNKNOWN:
                case BgePropertyValueType::STRING:
                    if (!split1.empty())
                    {
                        if (split1.at(0) == '\'' || split1.at(0) == '"')
                            split1 = split1.substr(1);

                        size_t split1EndIdx = split1.length() - 1;
                        if (split1.at(split1EndIdx) == '\'' || split1.at(split1EndIdx) == '"')
                            split1 = split1.substr(0, split1EndIdx);
                    }
                    property = BgeConfigProperty(estimateType, split0, split1);
                    break;
            }
            if (currentSection != nullptr)
                currentSection->AddProperty(property.Name, property);
            else
                AddProperty(property.Name, property);
        }

        file.Close();
    }

    /**
     * Saves this configuration to a desired path
     * @param path output file path
    */
    void Save(std::string path)
    {
        BgeFile file = BgeFile(path, true);
        if (!file.Ready())
            return;
        
        for (auto& property : mProperties)
            property.Save(file);

        if (mProperties.size())
            file.WriteLine();

        for (auto& section : mSections)
            section.Save(file);

        file.Close();
    }

    /**
     * Adds a configuration property with a specific type
     * @param name name of the property
     * @param property the property object
    */
    void AddProperty(std::string name, BgeConfigProperty& property)
    {
        if (name.empty())
            return;

        if (HasProperty(name))
            return;
        
        size_t sectionDivider = name.find_first_of('.');
        if (sectionDivider == std::string::npos)
        {
            property.Name = name;
            mProperties.push_back(property);
            return;
        }

        std::string sectionName = name.substr(0, sectionDivider);
        std::string nextSectionStr = name.substr(sectionDivider+1);

        BgeConfigSection* nextSection = nullptr;
        if (!HasSection(sectionName))
            nextSection = AddSection(sectionName);
        else
            nextSection = GetSection(sectionName);

        if (nextSection == nullptr)
            return;

        nextSection->AddProperty(nextSectionStr, property);
    }

    /**
     * Add a configuration sub-section
     * @param name name of the sub-section, not including the name of this section
    */
    BgeConfigSection* AddSection(std::string name)
    {
        if (name.empty())
            return nullptr;

        if (HasSection(name))
            return GetSection(name);

        size_t sectionDivider = name.find_first_of('.');
        if (sectionDivider == std::string::npos)
        {
            BgeConfigSection section = BgeConfigSection(name);
            mSections.push_back(section);
            return &mSections.back();
        }

        std::string sectionName = name.substr(0, sectionDivider);
        std::string nextSectionName = name.substr(sectionDivider+1);

        return AddSection(sectionName)->AddSubSection(nextSectionName);
    }

    /**
     * @brief Check if a property exists in this configuration file
     * 
     * @param name Name of the property
     * @returns `true` if the property exists, otherwise `false`
     */
    bool HasProperty(std::string name)
    {
        if (name.empty())
            return false;

        size_t sectionDivider = name.find_first_of('.');
        if (sectionDivider == std::string::npos)
            return GetPropertyIterator(name) != mProperties.end();

        std::string sectionName = name.substr(0, sectionDivider);
        std::string nextSectionName = name.substr(sectionDivider+1);

        if (!HasSection(sectionName))
            return false;
        
        return GetSection(sectionName)->HasProperty(nextSectionName);
    }

    /**
     * @brief Check if a section exists in this configuration file
     * 
     * @param name Name of the section
     * @returns `true` if the section exists, otherwise `false`
     */
    bool HasSection(std::string name)
    {
        if (name.empty())
            return false;

        size_t sectionDivider = name.find_first_of('.');
        if (sectionDivider == std::string::npos)
            return GetSectionIterator(name) != mSections.end();
        
        std::string sectionName = name.substr(0, sectionDivider);
        std::string nextSectionName = name.substr(sectionDivider+1);

        if (!HasSection(sectionName))
            return false;
        
        return GetSection(sectionName)->HasSubSection(nextSectionName);
    }

    /**
     * Gets a specific property of this configuration file
     * @param name name of the desired property
     * @returns desired property, NULL if not found
    */
    BgeConfigProperty* Get(std::string name)
    {
        if (name.empty())
            return nullptr;

        size_t sectionDivider = name.find_first_of('.');
        if (sectionDivider == std::string::npos)
        {
            auto propertyIterator = GetPropertyIterator(name);
            return (propertyIterator == mProperties.end()) ? nullptr : &*propertyIterator;
        }

        std::string sectionName = name.substr(0, sectionDivider);
        std::string nextSectionName = name.substr(sectionDivider+1);

        if (!HasSection(sectionName))
            return nullptr;

        return GetSection(sectionName)->Get(nextSectionName);
    }

    /**
     * Gets a specific section of this configuration file
     * @param name name of the desired section
     * @returns desired section, NULL if not found
    */
    BgeConfigSection* GetSection(std::string name)
    {
        if (name.empty())
            return nullptr;

        size_t sectionDivider = name.find_first_of('.');
        if (sectionDivider == std::string::npos)
        {
            auto sectionIterator = GetSectionIterator(name);
            return (sectionIterator == mSections.end()) ? nullptr : &*sectionIterator;
        }

        std::string sectionName = name.substr(0, sectionDivider);
        std::string nextSectionName = name.substr(sectionDivider+1);

        if (!HasSection(sectionName))
            return nullptr;

        return GetSection(sectionName)->GetSubSection(nextSectionName);
    }

    /**
     * Estimates the type a given string could have
     * 
     * @param value value in string format
     *   
     * @returns the type of `value`
    */
    static BgePropertyValueType EstimateValueType(std::string value)
    {
        if (value.empty())
            return BgePropertyValueType::UNKNOWN;

        if (StringIsBool(value))
            return BgePropertyValueType::BOOL;

        if (StringIsNumber(value))
            return BgePropertyValueType::INT;

        if (StringIsFloat(value))
            return BgePropertyValueType::FLOAT;

        return BgePropertyValueType::STRING;
    }

private:
    BgeConfigPropertyList::iterator GetPropertyIterator(std::string name)
    {
        return std::find_if(mProperties.begin(), mProperties.end(), [name](BgeConfigProperty& other){ return name == other.Name; });
    }

    BgeConfigSectionList::iterator GetSectionIterator(std::string name)
    {
        return std::find_if(mSections.begin(), mSections.end(), [name](BgeConfigSection& other){ return name == other.Name; });
    }

    /**
     * Checks if given string is a number
     * 
     * @param[in] str input string
     * 
     * @returns true if it is a number, false otherwise
    */
    static bool StringIsNumber(std::string str)
    {
        int index = 0;
        char chr = str.at(index);
        if (chr == '+' || chr == '-')
            index++;

        for (; index < str.length(); index++)
        {
            chr = str.at(index);

            if (chr < '0' || chr > '9')
                return false;
        }

        return true;
    }

    /**
     * Checks if given string is a floating point number
     * 
     * @param[in] str input string
     * 
     * @returns true if it is a floating point number, false otherwise
    */
    static bool StringIsFloat(std::string str)
    {
        int index = 0;
        char chr = str.at(index);
        if (chr == '+' || chr == '-')
            index++;

        bool hasDot = false;

        for (; index < str.length(); index++)
        {
            chr = str.at(index);

            if (chr == '.')
            {
                if (hasDot)
                    return false;

                hasDot = true;
                continue;
            }

            if (chr < '0' || chr > '9')
                return false;
        }

        return true;
    }

    /**
     * Checks if given string is a bool
     * 
     * @param[in] str input string
     * 
     * @returns true if it is a bool, false otherwise
    */
    static bool StringIsBool(std::string str)
    {
        // yes this is a cheap and stupid solution but it works
        return str == "True"  ||
            str == "true"  ||
            str == "False" ||
            str == "false";
    }

    /**
     * Removes all whitespaces from the start and end of a string
     * 
     * @param[in] str input string
     * 
     * @returns cleared string without whitespaces at the start and end
    */
    static std::string StringTrimLeading(std::string str)
    {
        if (str.find_first_not_of(" \n\t\r\f\v") == std::string::npos)
            return "";

        int beginIdx = 0;
        int endIdx = 0;

        for (beginIdx = 0; beginIdx < str.length() && (str.at(beginIdx) == '\t' || str.at(beginIdx) == '\n' || str.at(beginIdx) == ' '); beginIdx++);
        for (endIdx = str.length()-1; endIdx >= 0 && (str.at(endIdx) == '\t' || str.at(endIdx) == '\n' || str.at(endIdx) == ' '); endIdx--);

        if (beginIdx == 0 && endIdx == str.length()-1)
            return str;

        return str.substr(beginIdx, endIdx - beginIdx + 1);
    }

    /**
     * Removes all "single-lined comments" ( "//" ) from a string
     * 
     * @param[in] str input string
     * 
     * @returns cleared string without "comments"
    */
    static std::string StringTrimComment(std::string str)
    {
        std::stringstream res = std::stringstream();
        for (size_t i = 0; i < str.size(); i++)
        {
            if (i < str.size()-1)
                if (str.at(i) == '/' && str.at(i+1) == '/')
                    break;
            
            res << str.at(i);
        }
        return res.str();
    }

private:
    BgeConfigPropertyList mProperties;
    BgeConfigSectionList mSections;
};

#endif