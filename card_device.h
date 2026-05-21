#pragma once

#include <windows.h>

// DataHandle 程序接口
#ifdef __cplusplus
extern "C" {
#endif

typedef void *CARD_DEVICE;

/* ================== 生命周期 ================== */
CARD_DEVICE WINAPI APP_Create(void);
void WINAPI        APP_Destroy(CARD_DEVICE card_device);

/* ================== 读卡器初始化 ================== */
/// @brief 初始化读卡器
/// @param card_device 读卡器句柄
/// @param card_reader_type 读卡器类型
/// @param readers [in/out] 读卡器名称数组
/// @param readers_count [in/out] 实际返回数量
/// @return BOOL 成功返回TRUE，失败返回FALSE
BOOL WINAPI APP_Initialize(CARD_DEVICE card_device, int card_reader_type, const char **readers, int *readers_count);

/* ================== 协议 / 读卡器 ================== */
BOOL WINAPI APP_CardProtocol(CARD_DEVICE card_device, int card_protocol_type);
BOOL WINAPI APP_CardReader(CARD_DEVICE card_device, size_t card_reader_index);

/* ================== 回调 ================== */
typedef void (*CardCallback)(const char *data, int len, void *user);
BOOL WINAPI APP_CardCallback(CARD_DEVICE card_device, CardCallback callback, void *user_data);

/* ================== 卡操作 ================== */
BOOL WINAPI APP_CardReset(CARD_DEVICE card_device, BOOL cold, char *out_atr, int out_atr_len);
BOOL WINAPI APP_PersoDataFile(CARD_DEVICE card_device, const char *prd_file, BOOL has_ds);
BOOL WINAPI APP_PersoDataJson(CARD_DEVICE card_device, const char *prd_json, BOOL has_ds);
BOOL WINAPI APP_RunFile(CARD_DEVICE card_device, const char *script_file, BOOL convert);
BOOL WINAPI APP_RunCode(CARD_DEVICE card_device, const char *script_code, BOOL convert);
BOOL WINAPI APP_GetLastError(CARD_DEVICE card_device, char *out_error, int out_error_len);

/* ================== 脚本操作 ================== */
BOOL WINAPI APP_ScriptConvert(CARD_DEVICE card_device, const char *rd_script_code, char *if_script_code, size_t if_script_code_size);

#ifdef __cplusplus
}
#endif