#pragma once

#include <string>

#include <Windows.h>





/** RAII wrapper for a single Registry key object and API calls on it. */
class RegistryKey
{
public:

	/** Exception that is thrown when attempting to read a non-existent value, or a value in a non-existent key. */
	class RegistryValueNotFoundException:
		public std::runtime_error
	{
	public:
		const std::wstring mSubkey;
		const std::wstring mName;


		RegistryValueNotFoundException(const std::wstring & aSubkey, const std::wstring & aName);

	protected:

		/** Returns the text description of the error, to be used within std::runtime_error constructor.
		The returned string it UTF8-encoded. */
		static std::string createErrorText(const std::wstring & aSubkey, const std::wstring & aName);
	};


	~RegistryKey();

	/** Opens the specified key in the user hive.
	Creates the entire path up to the key.
	Throws a std::runtime_error descendant on failure. */
	static RegistryKey openUserHive(const std::wstring & aPath);

	/** Opens the root key of the user hive. */
	static RegistryKey openUserHive();

	/** Sets the specified value to REG_NONE. */
	void setValueNone(const std::wstring & aName);

	/** Sets the specified value in the opened key. */
	void setValue(const std::wstring & aName, const std::wstring & aValue);

	/** Sets the default value in the opened key. */
	void setDefaultValue(const std::wstring & aValue);

	/** Returns the specified value of the currently open key, if it is a string.
	Throws a RegistryValueNotFoundException if the key or value is not found.
	Throws an std::runtime_error on other failure. */
	std::wstring getValueString(const std::wstring & aSubkey, const std::wstring & aName);

	/** Deletes the specified subkey, including all its children. */
	void deleteKey(const std::wstring & aSubkey);


protected:

	/** The opened key. */
	HKEY mKey;

	RegistryKey(HKEY aKey);
};
