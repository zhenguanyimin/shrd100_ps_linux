#include "json_pb_converter.h"

#if 1
#define DEBUG_PRINTF(arg...) printf(arg)
#else
#define DEBUG_PRINTF(arg...) 
#endif


static ProductHwCfg _productHwCfg;
static ProductSwCfg _productSwCfg;
static ProductDebugCfg _productDebugCfg;
/*find filename from debug_path ,then std_path.*/
static std::string* _SelectProductCfgPath(char *std_path, char *debug_path, char *filename)
{
    std::string *path;
    if(!std_path || !filename)
        return NULL;
        
    char buf[512];
    if(debug_path){
        snprintf(buf,sizeof(buf),"%s/%s",debug_path,filename);
        if(eap_file_exists(buf)){
            DEBUG_PRINTF("[%s:%d],buf:%s\n",__FUNCTION__,__LINE__,buf);
            path = new std::string(buf);
            return path;
        }
    }

	if(std_path){     
		snprintf(buf,sizeof(buf),"%s/%s",std_path,filename);
		if(eap_file_exists(buf)){
			DEBUG_PRINTF("[%s:%d],buf:%s\n",__FUNCTION__,__LINE__,buf);
			path = new std::string(buf);
			return path;
		}
    }

	return NULL;
}

/*load json config file into Project cfg obj*/
static void _LoadProductCfg(const char *jsonfile,void *message)
{
	char *data = NULL;
	size_t data_len = 0;
	int ret;
	
	data = eap_file_load(jsonfile, &data_len);
	if(!data){
		printf("err,load %s fail\n",jsonfile);
	}else{
		ret = json_string_to_message(data, message);
		if(ret < 0){
			printf("err,convert json to message %s fail\n",jsonfile);
		}
	}
	if(data){
		free(data);
	}

}


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
void ProductCfgLoad(char *std_path, char *debug_path)
{
	
	printf("***std_path:%s, debug_path:%s\n",std_path?std_path:"", debug_path?debug_path:"");
	std::string *hwcfg = _SelectProductCfgPath(std_path,debug_path,PRODUCT_HW_CFG_JSON_FILE);
	std::string *swcfg = _SelectProductCfgPath(std_path,debug_path,PRODUCT_SW_CFG_JSON_FILE);
	std::string *debugcfg = _SelectProductCfgPath(std_path,debug_path,PRODUCT_DEBUG_CFG_JSON_FILE);

	if(hwcfg){
		printf("***found hwcfg: %s\n",hwcfg->c_str());
		_LoadProductCfg(hwcfg->c_str(),&_productHwCfg);
	}
	if(swcfg){
		printf("***found swcfg: %s\n",swcfg->c_str());
		_LoadProductCfg(swcfg->c_str(),&_productSwCfg);
	}
	if(debugcfg){
		printf("***found debugcfg: %s\n",debugcfg->c_str());
		_LoadProductCfg(debugcfg->c_str(),&_productDebugCfg);
	}
}

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
ProductHwCfg& ProductHwCfgGet()
{
	return _productHwCfg;
}

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
ProductSwCfg& ProductSwCfgGet()
{
	return _productSwCfg;
}

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
ProductDebugCfg& ProductDebugCfgGet()
{
	return _productDebugCfg;
}


#define CFG_UNITTEST 1
#if CFG_UNITTEST

#define PRODUCT_HW_CFG_JSON_FILE  	"product_hw_cfg.json"
#define PRODUCT_HW_CFG_PB_FILE  	"product_hw_cfg.bin"

#define PRODUCT_SW_CFG_JSON_FILE  	"product_sw_cfg.json"
#define PRODUCT_SW_CFG_PB_FILE  	"product_sw_cfg.bin"


class ProductHwCfgTest 
{
public:
	static void test();
};

void ProductHwCfgTest::test()
{
	char *json = NULL;
	int ret;
	ProductHwCfg cfg;
	cfg.set_producthwtype("SHRD101L");

	//test save message into json file
	ret = message_to_json_string(&cfg, &json);
	if(!ret){
		printf("ProductHwCfg: [ok],  %s\n",json);

		ret = eap_file_save(PRODUCT_HW_CFG_JSON_FILE,json,strlen(json));
		if(ret < 0){
			printf("err,save %s fail\n",PRODUCT_HW_CFG_JSON_FILE);
		}
		
		free(json);
	}else{
		printf("ProductHwCfg: [err]\n");
	}
	

	//test load json file into message.
	char *data = NULL;
	size_t data_len = 0;
	ProductHwCfg cfg_load;
	
	data = eap_file_load(PRODUCT_HW_CFG_JSON_FILE, &data_len);
	if(!data){
		printf("err,load %s fail\n",PRODUCT_HW_CFG_JSON_FILE);
	}else{
		ret = json_string_to_message(data, &cfg_load);
		if(ret < 0){
			printf("err,convert json to message %s fail\n",PRODUCT_HW_CFG_JSON_FILE);
		}
		printf("producthwtype:%s\n",cfg_load.producthwtype().c_str());
	}
	if(data){
		free(data);
	}
}

class ProductSwCfgTest 
{
public:
	static void test();
};

void ProductSwCfgTest::test()
{
	char *json = NULL;
	int ret;
	ProductSwCfg cfg;
	cfg.set_vertype("SHRD101L");
	cfg.set_issupportdroneid(true);
	cfg.set_issupportremoteid(true);
	cfg.set_issupportspectrum(true);
	cfg.set_issupportspectrumorient(true);
	
	//test save message into json file
	ret = message_to_json_string(&cfg, &json);
	if(!ret){
		printf("ProductSwCfg: [ok],  %s\n",json);

		ret = eap_file_save(PRODUCT_SW_CFG_JSON_FILE,json,strlen(json));
		if(ret < 0){
			printf("err,save %s fail\n",PRODUCT_SW_CFG_JSON_FILE);
		}
		
		free(json);
	}else{
		printf("ProductSwCfg: [err]\n");
	}
	

	//test load json file into message.
	char *data = NULL;
	size_t data_len = 0;
	ProductSwCfg cfg_load;
	
	data = eap_file_load(PRODUCT_SW_CFG_JSON_FILE, &data_len);
	if(!data){
		printf("err,load %s fail\n",PRODUCT_SW_CFG_JSON_FILE);
	}else{
		ret = json_string_to_message(data, &cfg_load);
		if(ret < 0){
			printf("err,convert json to message %s fail\n",PRODUCT_SW_CFG_JSON_FILE);
		}
		printf("vertype:%s,issupportdroneid:%d,issupportremoteid:%d,issupportspectrum:%d,issupportspectrumorient:%d\n",
			cfg_load.vertype().c_str(),cfg_load.issupportdroneid(),cfg_load.issupportremoteid(),
			cfg_load.issupportspectrum(),cfg_load.issupportspectrumorient());
	}
	if(data){
		free(data);
	}
}


#endif



#ifdef __cplusplus
extern "C" {
#endif

// C function implementation for converting JSON string to Protobuf message
int json_string_to_message(const char* json_string, void* message) {
    if (json_string == NULL || message == NULL) {
        return -1;
    }

    // Convert the JSON string to Protobuf message
    google::protobuf::util::JsonParseOptions options;
    if (!google::protobuf::util::JsonStringToMessage(json_string, static_cast<google::protobuf::Message*>(message), options).ok()) {
        return -1;
    }
    return 0;
}

// C function implementation for converting Protobuf message to JSON string
int message_to_json_string(const void* message, char** json_string) {
    if (message == NULL || json_string == NULL) {
        return -1;
    }

    // Convert the Protobuf message to JSON string
    std::string json;
    google::protobuf::util::JsonPrintOptions options;
    if (!google::protobuf::util::MessageToJsonString(*static_cast<const google::protobuf::Message*>(message), &json, options).ok()) {
        return -1;
    }

    // Allocate memory for the JSON string and copy the data
    *json_string = (char*)malloc(json.size() + 1);
    if (*json_string == NULL) {
        return -1;
    }
    strcpy(*json_string, json.c_str());

    return 0;
	
}

#if CFG_UNITTEST
void product_hw_cfg_test()
{
	ProductHwCfgTest::test();
}

void product_sw_cfg_test()
{
	ProductSwCfgTest::test();
}	

void project_cfg_dump()
{
	int ret;
	char *json = NULL;
 	ProductHwCfg &hw = ProductHwCfgGet();
 	ProductSwCfg &sw = ProductSwCfgGet();
 	ProductDebugCfg &debug = ProductDebugCfgGet();	

	ret = message_to_json_string(&hw, &json);
	if(!ret){
		printf("ProductHwCfg: [ok],  %s\n",json);	
		printf("producthwtype:%s\n",hw.producthwtype().c_str());
	}else{
		printf("ProductHwCfg: [err]\n");
	}

	ret = message_to_json_string(&sw, &json);
	if(!ret){
		printf("ProductSwCfg: [ok],  %s\n",json);	
		printf("vertype:%s,issupportdroneid:%d,issupportremoteid:%d,issupportspectrum:%d,issupportspectrumorient:%d\n",
			sw.vertype().c_str(),sw.issupportdroneid(),sw.issupportremoteid(),
			sw.issupportspectrum(),sw.issupportspectrumorient());
		
	}else{
		printf("ProductSwCfg: [err]\n");
	}

	ret = message_to_json_string(&debug, &json);
	if(!ret){
		printf("ProductDebugCfg: [ok],  %s\n",json);	
		printf("flag_debug_disable_buzz_motor:%d\n",debug.flag_debug_disable_buzz_motor());
	}else{
		printf("ProductDebugCfg: [err]\n");
	}
}
#endif


#ifdef __cplusplus
}
#endif


