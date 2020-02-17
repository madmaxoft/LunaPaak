#include "RegistryKey.h"
#include <vector>
#include "StringUtils.h"





////////////////////////////////////////////////////////////////////////////////
// RegistryKey::RegistryValueNotFoundException:

RegistryKey::RegistryValueNotFoundException::RegistryValueNotFoundException(const std::wstring & aSubkey, const std::wstring & aName):
	std::runtime_error(createErrorText(aSubkey, aName)),
	mSubkey(aSubkey),
	mName(aName)
{
}





std::string RegistryKey::RegistryValueNotFoundException::createErrorText(const std::wstring & aSubkey, const std::wstring & aName)
{
	std::wstring res;
	res.append(L"Value \"");
	res.append(aName);
	res.append(L"\" not found in subkey \"");
	res.append(aSubkey);
	res.append(L"\".");
	return toUtf8(res);
}





////////////////////////////////////////////////////////////////////////////////
// RegistryKey:

RegistryKey::~RegistryKey()
{
	RegCloseKey(mKey);
}





RegistryKey RegistryKey::openUserHive(const std::wstring & aPath)
{
	HKEY openedKey = 0;
	auto err = RegCreateKeyExW(HKEY_CURRENT_USER, aPath.c_str(), 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &openedKey, nullptr);
	if (err != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to create registry key");
	}
	return RegistryKey(openedKey);
}





RegistryKey RegistryKey::openUserHive()
{
	return RegistryKey(HKEY_CURRENT_USER);
}





void RegistryKey::setValueNone(const std::wstring & aName)
{
	auto err = RegSetValueExW(mKey, aName.c_str(), 0, REG_NONE, nullptr, 0);
	if (err != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to set registry value to none");
	}
}





void RegistryKey::setValue(const std::wstring & aName, const std::wstring & aValue)
{
	auto err = RegSetValueExW(mKey, aName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE *>(aValue.c_str()), static_cast<DWORD>(aValue.size() * 2 + 2));
	if (err != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to set registry default value");
	}
}





void RegistryKey::setDefaultValue(const std::wstring & aValue)
{
	auto err = RegSetValueExW(mKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE *>(aValue.c_str()), static_cast<DWORD>(aValue.size() * 2 + 2));
	if (err != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to set registry default value");
	}
}





std::wstring RegistryKey::getValueString(const std::wstring & aSubkey,const std::wstring & aName)
{
	DWORD dataSize = 0;
	DWORD allowedTypes = RRF_RT_REG_EXPAND_SZ | RRF_RT_REG_MULTI_SZ | RRF_RT_REG_SZ;
	auto err = RegGetValueW(mKey, aSubkey.c_str(), aName.c_str(), allowedTypes, nullptr, nullptr, &dataSize);
	if (err == ERROR_FILE_NOT_FOUND)
	{
		throw RegistryValueNotFoundException(aSubkey, aName);
	}
	if (err != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to query value size");
	}
	std::vector<wchar_t> value;
	value.resize((dataSize + 1) / 2);
	err = RegGetValueW(mKey, aSubkey.c_str(), aName.c_str(), allowedTypes, nullptr, &value[0], &dataSize);
	if (err != ERROR_SUCCESS)
	{
		throw std::runtime_error("Failed to read value");
	}
	value.resize((dataSize + 1) / 2);
	return std::wstring(&value[0], (dataSize - 1) / 2);
}





void RegistryKey::deleteKey(const std::wstring & aSubkey)
{
	auto err = RegDeleteTreeW(mKey, aSubkey.c_str());
	if ((err != ERROR_SUCCESS) && (err != ERROR_FILE_NOT_FOUND))
	{
		throw std::runtime_error("Failed to delete registry key");
	}
}





RegistryKey::RegistryKey(HKEY aKey):
	mKey(aKey)
{
}
