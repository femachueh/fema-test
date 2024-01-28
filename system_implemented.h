/**
 * @file system_implemented.h
 * @author John Lin (John@brytoncorp.net)
 * 
 * @brief 
 * 
 * @version 0.0.1
 * @date 2020-11-18
 */

/**< module_name */
// LEV_Driver
// LEV_Statistic
// Sensor_I2C
// gSensor_Drv
// LightSensor_Drv
// GPS
// mapParser
// LCD
// Barometer
// hvService

#ifndef SYSTEM_IMPLEMENTED_H
#define SYSTEM_IMPLEMENTED_H

#ifndef SYS_INS
#define SYS_INS extern
#endif

/**< Module Log Level, name : (module_name)_LogLevel */
#define LEV_Driver_LogLevel         LOG_LEVEL_INFO
#define LEV_Statistic_LogLevel      LOG_LEVEL_INFO
#define Sensor_I2C_LogLevel         LOG_LEVEL_WARNING
#define gSensor_Drv_LogLevel        LOG_LEVEL_WARNING
#define LightSensor_Drv_LogLevel    LOG_LEVEL_INFO
#define GPS_LogLevel                LOG_LEVEL_INFO
#define mapParser_LogLevel          LOG_LEVEL_INFO
#define LCD_LogLevel                LOG_LEVEL_INFO
#define Barometer_LogLevel          LOG_LEVEL_INFO
#define hvService_LogLevel          LOG_LEVEL_INFO
#define routeData_LogLevel          LOG_LEVEL_INFO
#define routeUtil_LogLevel          LOG_LEVEL_INFO
#define Arch_LogLevel               LOG_LEVEL_INFO
#define Compression_LogLevel        LOG_LEVEL_INFO

/**< Module Block Level, name : (module_name)_BlockLevel */
#define LEV_Driver_BlockLevel       BLOCK_LEVEL_INCORRECT
#define LEV_Statistic_BlockLevel    BLOCK_LEVEL_INCORRECT
#define Sensor_I2C_BlockLevel       BLOCK_LEVEL_CRITICAL
#define gSensor_Drv_BlockLevel      BLOCK_LEVEL_CRITICAL
#define LightSensor_Drv_BlockLevel  BLOCK_LEVEL_CRITICAL
#define GPS_BlockLevel              BLOCK_LEVEL_ABNORMAL
#define mapParser_BlockLevel        BLOCK_LEVEL_ABNORMAL
#define LCD_BlockLevel              BLOCK_LEVEL_ABNORMAL
#define Barometer_BlockLevel        BLOCK_LEVEL_ABNORMAL
#define hvService_BlockLevel        BLOCK_LEVEL_INCORRECT
#define routeData_BlockLevel        BLOCK_LEVEL_INCORRECT
#define routeUtil_BlockLevel        BLOCK_LEVEL_INCORRECT
#define Arch_BlockLevel             BLOCK_LEVEL_INCORRECT
#define Compression_BlockLevel      BLOCK_LEVEL_INCORRECT

/**< Module WatchDog feature, name : (module_name)_WatchDogOn, (module_name)_WatchDogInstance, (module_name)_WatchDogTimeout, (module_name)_WatchDogMessage */
#define ADC_WatchDogOn       1
SYS_INS WatchDogInstance ADC_WatchDogInstance;
#define ADC_WatchDogTimeout  600000
#define ADC_WatchDogMessage  "ADC long term detect fail"

/**< Module Performance feature, name : (module_name)_PerformanceOn, (module_name)_PerformanceInstanceNum, (module_name)_PerformanceInstances[] */
#define mapParser_PerformanceOn             1
#define mapParser_PerformanceInstanceNum    1
SYS_INS PerformanceInstance mapParser_PerformanceInstances[mapParser_PerformanceInstanceNum];

#define EW_PerformanceOn            1
#define EW_PerformanceInstanceNum   1
SYS_INS PerformanceInstance EW_PerformanceInstances[EW_PerformanceInstanceNum];

#define Sleep_PerformanceOn             1
#define Sleep_PerformanceInstanceNum    1
SYS_INS PerformanceInstance Sleep_PerformanceInstances[Sleep_PerformanceInstanceNum];

#define routeData_PerformanceOn             1
#define routeData_PerformanceInstanceNum    1
SYS_INS PerformanceInstance routeData_PerformanceInstances[routeData_PerformanceInstanceNum];

#define test1_PerformanceOn             1
#define test1_PerformanceInstanceNum    1
SYS_INS PerformanceInstance test1_PerformanceInstances[test1_PerformanceInstanceNum];

#define test2_PerformanceOn             1
#define test2_PerformanceInstanceNum    1
SYS_INS PerformanceInstance test2_PerformanceInstances[test2_PerformanceInstanceNum];

#define test3_PerformanceOn             1
#define test3_PerformanceInstanceNum    1
SYS_INS PerformanceInstance test3_PerformanceInstances[test3_PerformanceInstanceNum];

#endif // SYSTEM_IMPLEMENTED_H