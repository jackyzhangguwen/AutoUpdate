#pragma once

#include "./Include/jsoncpp/json.h"

using namespace Json;
using namespace std;

class CParseJson
{
public:
	CParseJson();
	bool ParseJson(const string& _strJson);
	bool GetString(const string& _strKey, string& _strValue);
	bool GetInt(const string& _strKey, int& _iValue);
	bool GetBool(const string& _strKey, bool& _bValue);
	bool GetDouble(const string& _strKey, double& _dValue);
	bool GetArray(const string& _strKey, Value& _aryValue);
	bool GetObjectValue(const string& _strKey, Value& _objValue);
	const char* GetLastError();
private:
	bool _GetValue(const string& _strKey, int _iType, Value& _Value);
private:
	/**
	* @brief The Type enum
	* 字段类型枚举
	*/
	enum Type {
		Int = 0x0,			// 整型
		Bool = 0x1,			// bool值
		Double = 0x2,		// 浮点型
		String = 0x3,		// 字符串
		Array = 0x4,		// json数组
		Object = 0x5,		// json对象
		Undefined = 0x80	// 无定义
	};
	Value m_jsonObj;		// 解析后的json对象
	string m_strError;		// 解析过程出现的错误信息
	bool m_bSucceess;		// 解析是否成功标志位
};
