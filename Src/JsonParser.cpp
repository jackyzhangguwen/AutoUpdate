#include "JsonParser.h"
#include <sstream>

#pragma comment(lib, ".\\lib\\jsoncpp\\lib_json.lib")

CParseJson::CParseJson()
	: m_bSucceess(false)
{

}

bool CParseJson::ParseJson(const string& _strJson)
{
	Reader reader;
	if (false == (m_bSucceess = reader.parse(_strJson, m_jsonObj))) {
		m_strError = reader.getFormatedErrorMessages();
	}
	return m_bSucceess;
}

bool CParseJson::GetString(const string& _strKey, string& _strValue)
{
	Value value;
	if (_GetValue(_strKey, String, value)) {
		_strValue = value.asString();
		return true;
	}
	return false;
}

bool CParseJson::GetInt(const string& _strKey, int& _iValue)
{
	Value value;
	if (_GetValue(_strKey, Int, value)) {
		_iValue = value.asInt();
		return true;
	}
	return false;
}

bool CParseJson::GetBool(const string& _strKey, bool& _bValue)
{
	Value value;
	if (_GetValue(_strKey, Bool, value)) {
		_bValue = value.asBool();
		return true;
	}
	return false;
}

bool CParseJson::GetDouble(const string& _strKey, double& _dValue)
{
	Value value;
	if (_GetValue(_strKey, Double, value)) {
		_dValue = value.asDouble();
		return true;
	}
	return false;
}

bool CParseJson::GetArray(const string& _strKey, Value& _aryValue)
{
	return _GetValue(_strKey, Array, _aryValue);
}

bool CParseJson::GetObjectValue(const string& _strKey, Value& _objValue)
{
	return _GetValue(_strKey, Object, _objValue);
}

const char* CParseJson::GetLastError()
{
	return m_strError.c_str();
}

bool CParseJson::_GetValue(const string& _strKey, int _iType, Value& _Value)
{
	if (false == m_bSucceess) { return false; }
	if (false == m_jsonObj.isMember(_strKey)) {
		ostringstream stream;
		stream << "json object not found key:" << _strKey;
		m_strError = stream.str();
		return false;
	}
	_Value = m_jsonObj[_strKey];
	string sErrorType;
	// 类型是否正确
	bool bType = false;
	switch (_iType) {
	case Int:
		if (false == (bType = _Value.isInt())) {
			sErrorType = "Int";
		}
		break;
	case String:
		if (false == (bType = _Value.isString())) {
			sErrorType = "String";
		}
		break;
	case Bool:
		if (false == (bType = _Value.isBool())) {
			sErrorType = "Bool";
		}
		break;
	case Double:
		if (false == (bType = _Value.isDouble())) {
			sErrorType = "Double";
		}
		break;
	case Array:
		if (false == (bType = _Value.isArray())) {
			sErrorType = "Array";
		}
		break;
	case Object:
		if (false == (bType = _Value.isObject())) {
			sErrorType = "Object";
		}
		break;
	default:
		bType = false;
		break;
	}
	if (false == bType)	{
		ostringstream stream;
		stream << "the key:" << _strKey << " is not " << sErrorType << " type";
		m_strError = stream.str();
	}
	return bType;
}
