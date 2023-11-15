#ifndef JSON_PB_CONVERTER_H
#define JSON_PB_CONVERTER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "eap_os_adapter.h"
#include <google/protobuf/util/json_util.h>
#include "ProductHwCfg.pb.h"
#include "ProductSwCfg.pb.h"
#include "ProductDebugCfg.pb.h"

/**
 * @brief load product cfg 
 *		the product configs includes  PRODUCT_HW_CFG_JSON_FILE,PRODUCT_SW_CFG_JSON_FILE ,
 * @param [in] 	filename    file
 *
 * @return 
 * @retval 1  		file exist
 *         0        file not exist.
 *
 * @see 	linux_app/src/eap/alink_pb/*.proto
 * @note 
 * @warning   Notice: Can only be called by SysBase constructor. 
 */
void ProductCfgLoad(char *std_path, char *debug_path);


/**
 * @brief get ProductHwCfg Obj
 *		
 * @param [in] 	filename    file
 *
 * @return ProductHwCfg&
 *
 * @see 	linux_app/src/eap/alink_pb/ProductHwCfg.proto
 * @note 
 * @warning    
 */
ProductHwCfg& ProductHwCfgGet();



/**
 * @brief get ProductSwCfg Obj
 *		
 * @param [in] 	filename    file
 *
 * @return ProductSwCfg&
 *
 * @see 	linux_app/src/eap/alink_pb/ProductSwCfg.proto
 * @note 
 * @warning   
 */
ProductSwCfg& ProductSwCfgGet();

/**
 * @brief get ProductDebugCfg Obj
 *		
 * @param [in] 	filename    file
 *
 * @return ProductDebugCfg&
 *
 * @see 	linux_app/src/eap/alink_pb/ProductDebugCfg.proto
 * @note 
 * @warning   
 */
ProductDebugCfg& ProductDebugCfgGet();


#ifdef __cplusplus
extern "C" {
#endif

// C function declaration for converting JSON string to Protobuf message
int json_string_to_message(const char* json_string, void* message);

// C function declaration for converting Protobuf message to JSON string
int message_to_json_string(const void* message, char** json_string);


#ifdef __cplusplus
}
#endif

#endif // JSON_PB_CONVERTER_H
