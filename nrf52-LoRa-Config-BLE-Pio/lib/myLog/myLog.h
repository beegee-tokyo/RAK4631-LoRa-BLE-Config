#include <Arduino.h>

#define MYLOG_LOG_LEVEL_NONE (0)
#define MYLOG_LOG_LEVEL_ERROR (1)
#define MYLOG_LOG_LEVEL_WARN (2)
#define MYLOG_LOG_LEVEL_INFO (3)
#define MYLOG_LOG_LEVEL_DEBUG (4)
#define MYLOG_LOG_LEVEL_VERBOSE (5)

#ifndef MYLOG_LOG_LEVEL
#ifdef CFG_DEBUG
#define MYLOG_LOG_LEVEL CFG_DEBUG
#if (CFG_DEBUG != 0) && (CFG_DEBUG != 1)
#undef MYLOG_LOG_LEVEL
#define MYLOG_LOG_LEVEL 5
#endif
#endif
#endif

const char *pathToFileNameNRF(const char *path);

#if __cplusplus
#ifndef PRINTF
#define PRINTF ::printf
#endif
#else
#ifndef PRINTF
#define PRINTF printf
#endif
#endif

#if MYLOG_LOG_LEVEL >= MYLOG_LOG_LEVEL_VERBOSE
#define myLog_v(...)                         \
	do                                       \
	{                                        \
		PRINTF("[V]");                       \
		PRINTF("[");                         \
		PRINTF(pathToFileNameNRF(__FILE__)); \
		PRINTF(":%d", __LINE__);             \
		PRINTF("]");                         \
		PRINTF(__FUNCTION__);                \
		PRINTF(": ");                        \
		PRINTF(__VA_ARGS__);                 \
		PRINTF("\n");                        \
	} while (0)
#else
#define myLog_v(format, ...)
#endif

#if MYLOG_LOG_LEVEL >= MYLOG_LOG_LEVEL_DEBUG
#define myLog_d(...)                         \
	do                                       \
	{                                        \
		PRINTF("[D]");                       \
		PRINTF("[");                         \
		PRINTF(pathToFileNameNRF(__FILE__)); \
		PRINTF(":%d", __LINE__);             \
		PRINTF("]");                         \
		PRINTF(__FUNCTION__);                \
		PRINTF(": ");                        \
		PRINTF(__VA_ARGS__);                 \
		PRINTF("\n");                        \
	} while (0)
#else
#define myLog_d(format, ...)
#endif

#if MYLOG_LOG_LEVEL >= MYLOG_LOG_LEVEL_INFO
#define myLog_i(...)                         \
	do                                       \
	{                                        \
		PRINTF("[I]");                       \
		PRINTF("[");                         \
		PRINTF(pathToFileNameNRF(__FILE__)); \
		PRINTF(":%d", __LINE__);             \
		PRINTF("]");                         \
		PRINTF(__FUNCTION__);                \
		PRINTF(": ");                        \
		PRINTF(__VA_ARGS__);                 \
		PRINTF("\n");                        \
	} while (0)
#else
#define myLog_i(format, ...)
#endif

#if MYLOG_LOG_LEVEL >= MYLOG_LOG_LEVEL_WARN
#define myLog_w(...)                         \
	do                                       \
	{                                        \
		PRINTF("[W]");                       \
		PRINTF("[");                         \
		PRINTF(pathToFileNameNRF(__FILE__)); \
		PRINTF(":%d", __LINE__);             \
		PRINTF("]");                         \
		PRINTF(__FUNCTION__);                \
		PRINTF(": ");                        \
		PRINTF(__VA_ARGS__);                 \
		PRINTF("\n");                        \
	} while (0)
#else
#define myLog_w(format, ...)
#endif

#if MYLOG_LOG_LEVEL >= MYLOG_LOG_LEVEL_ERROR
#define myLog_e(...)                         \
	do                                       \
	{                                        \
		PRINTF("[E]");                       \
		PRINTF("[");                         \
		PRINTF(pathToFileNameNRF(__FILE__)); \
		PRINTF(":%d", __LINE__);             \
		PRINTF("]");                         \
		PRINTF(__FUNCTION__);                \
		PRINTF(": ");                        \
		PRINTF(__VA_ARGS__);                 \
		PRINTF("\n");                        \
	} while (0)
#else
#define myLog_e(format, ...)
#endif

#if MYLOG_LOG_LEVEL == MYLOG_LOG_LEVEL_NONE
#define myLog_n(format, ...)
#endif
