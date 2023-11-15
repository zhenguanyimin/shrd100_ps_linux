#include "input.h"
#include "../../../srv/log/log.h"
#include "../../TracerUtils_c.h"

static const  INPUT_ARG* InputArg = NULL;

static const  INPUT_ARG InputArg_T2[] =
{
    {MIO_MUTE_SWITCH        , ENABLE, MIO, GPIO_DEVICE_ID, 0, 37},
    {MIO_SWITCH_MODE0       , ENABLE, MIO, GPIO_DEVICE_ID, 0, 38},
    {MIO_SWITCH_MODE1       , ENABLE, MIO, GPIO_DEVICE_ID, 0, 40},
    {MIO_SWITCH_MODE2       , ENABLE, MIO, GPIO_DEVICE_ID, 0, 43},
    {MIO_SWITCH_MODE3       , ENABLE, MIO, GPIO_DEVICE_ID, 0, 8},

    {EMIO_CONFIG_SW         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 43},
    {EMIO_GPS_SW            , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 61},
    {EMIO_TRIGGER           , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 67},
    {EMIO_VCC0_21V_OUT_INT  , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 62},
    {EMIO_FAN_SENSOR        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 54},
    //{EMIO_GYRO_INT2_DRDY    , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 11},// ??EMIO_DX_CTRL_OUT1
    //{EMIO_GYRO_INT1         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 10},// ?? EMIO_DX_CTRL_OUT0
    {EMIO_ACC_MAGINT        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 23},
    //      {EMIO_ACC_INT2_XL       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 22},
    //      {EMIO_ACC_INT1_XL       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 21},
    //      {EMIO_WIFI_NET_STATUS   , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 66},
    //      {EMIO_WIFI_WAKEUP_IN    , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 64},
    //      {EMIO_WIFI_RX1          , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 68},
    {EMIO_CTRL_OUT0         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 44},
    {EMIO_CTRL_OUT1         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 45},
    {EMIO_CTRL_OUT2         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 46},
    //      {EMIO_CTRL_OUT3         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 47},
    //      {EMIO_CTRL_OUT4         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 48},
    //      {EMIO_CTRL_OUT5         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 49},
    {EMIO_VCC28V_PG_1V8     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 35},
    {EMIO_VCC32V_PG_1V8     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 34},

    //End
    {INPUT_NAME_MAX     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, 0}

};

// TODO: GIPO configuration for T3 board
static const  INPUT_ARG InputArg_T3[] =
{
    {MIO_MUTE_SWITCH        , ENABLE, MIO, GPIO_DEVICE_ID, 0, 37},
    {MIO_SWITCH_MODE0       , ENABLE, MIO, GPIO_DEVICE_ID, 0, 38},
    {MIO_SWITCH_MODE1       , ENABLE, MIO, GPIO_DEVICE_ID, 0, 40},
    {MIO_SWITCH_MODE2       , ENABLE, MIO, GPIO_DEVICE_ID, 0, 43},
    {MIO_SWITCH_MODE3       , ENABLE, MIO, GPIO_DEVICE_ID, 0, 8},

    {EMIO_CONFIG_SW         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 43},
    {EMIO_GPS_SW            , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 61},
    {EMIO_TRIGGER           , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 67},
    {EMIO_VCC0_21V_OUT_INT  , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 62},
    {EMIO_FAN_SENSOR        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 54},
    // {EMIO_GYRO_INT2_DRDY    , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 11},
    // {EMIO_GYRO_INT1         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 10},
    {EMIO_ACC_MAGINT        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 23},
    //      {EMIO_ACC_INT2_XL       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 22},
    //      {EMIO_ACC_INT1_XL       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 21},
    //      {EMIO_WIFI_NET_STATUS   , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 66},
    //      {EMIO_WIFI_WAKEUP_IN    , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 64},
    //      {EMIO_WIFI_RX1          , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 68},
    {EMIO_CTRL_OUT0         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 44},
    {EMIO_CTRL_OUT1         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 45},
    {EMIO_CTRL_OUT2         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 46},
    //      {EMIO_CTRL_OUT3         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 47},
    //      {EMIO_CTRL_OUT4         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 48},
    //      {EMIO_CTRL_OUT5         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 49},
    {EMIO_VCC28V_PG_1V8     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 35},
    {EMIO_VCC32V_PG_1V8     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 34},


    /***For T3 board, begin ******************/
    {EMIO_ADL5904_QP       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 69}, //RSV_3V3_IO4
    {EMIO_ADL5904_QN       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 70}, //RSV_3V3_IO5

    // {EMIO_ADL5904_VRMS     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 100}, //Not used by software yet

    {MIO_PS_UART1_RX      , ENABLE, MIO, GPIO_DEVICE_ID, 0, 45}, 
    // {EMIO_PL_UART1_RX      , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 100}, //TODO ?

    /***For T3 board, end ******************/

    //End
    {INPUT_NAME_MAX     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, 0}

};

// 车载Tracer专用,按键板替换为功放板
static const  INPUT_ARG InputArg_T3_1[] =
{
    {EMIO_CONFIG_SW         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 43},
    {EMIO_GPS_SW            , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 61},
    {EMIO_TRIGGER           , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 67},
    {EMIO_VCC0_21V_OUT_INT  , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 62},
    {EMIO_FAN_SENSOR        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 54},
    // {EMIO_GYRO_INT2_DRDY    , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 11},
    // {EMIO_GYRO_INT1         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 10},
    {EMIO_ACC_MAGINT        , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 23},
    //      {EMIO_ACC_INT2_XL       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 22},
    //      {EMIO_ACC_INT1_XL       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 21},
    //      {EMIO_WIFI_NET_STATUS   , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 66},
    //      {EMIO_WIFI_WAKEUP_IN    , DISABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 64},
    //      {EMIO_WIFI_RX1          , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 68},
    {EMIO_CTRL_OUT0         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 44},
    {EMIO_CTRL_OUT1         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 45},
    {EMIO_CTRL_OUT2         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 46},
    //      {EMIO_CTRL_OUT3         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 47},
    //      {EMIO_CTRL_OUT4         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 48},
    //      {EMIO_CTRL_OUT5         , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 49},
    {EMIO_VCC28V_PG_1V8     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 35},
    {EMIO_VCC32V_PG_1V8     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 34},


    /***For T3 board, begin ******************/
    {EMIO_ADL5904_QP       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 69}, //RSV_3V3_IO4
    {EMIO_ADL5904_QN       , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 70}, //RSV_3V3_IO5

    // {EMIO_ADL5904_VRMS     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 100}, //Not used by software yet

    {MIO_PS_UART1_RX      , ENABLE, MIO, GPIO_DEVICE_ID, 0, 45}, 
    // {EMIO_PL_UART1_RX      , ENABLE, EMIO, GPIO_DEVICE_ID, 0, EMIO_OFFSET + 100}, //TODO ?

    /***For T3 board, end ******************/

    //End
    {INPUT_NAME_MAX     , ENABLE, EMIO, GPIO_DEVICE_ID, 0, 0}

};

#if 1 //Linux

int32_t InputInit(void)
{
    int32_t Status = 0;
    int pin;

    switch(get_device_type())
    {
        case BOARD_TYPE_T2:
            EAP_LOG_DEBUG("BOARD_TYPE_T2\n");
            InputArg = InputArg_T2;
            break;
        case BOARD_TYPE_T3:
            EAP_LOG_DEBUG("BOARD_TYPE_T3\n");
            InputArg = InputArg_T3;
            break;
        case BOARD_TYPE_T3_1:
            EAP_LOG_DEBUG("BOARD_TYPE_T3_1\n");
            InputArg = InputArg_T3_1;
            break;
        default:
            EAP_LOG_DEBUG("get board type err!\n");
            return -1;
    }

    for(int i = 0; i < 0xFFFF; ++i)
    {
        if(INPUT_NAME_MAX == InputArg[i].Name)
        {
            break;
        }

        if(InputArg[i].Enable == ENABLE)
        {
            if ((InputArg[i].GpioMode == MIO) || (InputArg[i].GpioMode == EMIO))
            {
                pin = InputArg[i].Pin + LINUX_GPIO_OFFSET;
                gpio_unexport(pin);
                gpio_export(pin);
                gpio_direction(pin, 0);
            }
        }
    }

    printf("%s():****Success to init input GPIO.\n", __FUNCTION__);
    return Status;
}

int32_t InputUninit(void)
{
    int32_t Status = 0;
    int pin;

    for(int i = 0; i < 0xFFFF; ++i)
    {
        if(INPUT_NAME_MAX == InputArg[i].Name)
        {
            break;
        }

        if(InputArg[i].Enable == ENABLE)
        {
            if ((InputArg[i].GpioMode == MIO) || (InputArg[i].GpioMode == EMIO))
            {
                pin = InputArg[i].Pin + LINUX_GPIO_OFFSET;
                gpio_unexport(pin);
            }
        }
    }

    printf("%s():****Success to uninstall input GPIO.\n", __FUNCTION__);
    return Status;
}

int GetGpioArrayIdndexByInputName(INPUT_NAME pName)
{
    int index = 0;
    if( INPUT_NAME_MAX <= pName)
    {
        return -1;
    }

    for(; index < 0xFFFF; ++index)
    {
        if(INPUT_NAME_MAX == InputArg[index].Name)
        {
            break;
        }
        if(pName == InputArg[index].Name)
        {
            return index;
        }
    }

    //not found
    return -1;
}

uint8_t Input_GetValue(INPUT_NAME name)
{
    uint8_t i = (uint8_t)name;
    uint8_t status = 0;
    int pin;

    if(InputArg[i].Enable == ENABLE)
    {
        if ((InputArg[i].GpioMode == MIO) || (InputArg[i].GpioMode == EMIO))
        {
            pin = InputArg[i].Pin + LINUX_GPIO_OFFSET;
            status = (uint8_t)gpio_read(pin);
        }
    }

    return status;
}

// TODO: Need remove 
#if 0
int32_t InputInit(void)
{
	int32_t Status = 0;
	int pin;

	for(uint16_t i = 0; i < INPUT_NAME_MAX; i++)
	{
		if(InputArg[i].Enable == ENABLE)
		{
			if ((InputArg[i].GpioMode == MIO) || (InputArg[i].GpioMode == EMIO))
			{
				pin = InputArg[i].Pin + LINUX_GPIO_OFFSET;
				gpio_unexport(pin);
				gpio_export(pin);
				gpio_direction(pin, 0);
			}
		}
	}

	return Status;
}

int32_t InputUninit(void)
{
	int32_t Status = 0;
	int pin;

	for(uint16_t i = 0; i < INPUT_NAME_MAX; i++)
	{
		if(InputArg[i].Enable == ENABLE)
		{
			if ((InputArg[i].GpioMode == MIO) || (InputArg[i].GpioMode == EMIO))
			{
				pin = InputArg[i].Pin + LINUX_GPIO_OFFSET;
				gpio_unexport(pin);
			}
		}
	}

	return Status;
}

uint8_t Input_GetValue(INPUT_NAME name)
{
	uint8_t i = (uint8_t)name;
	uint8_t status = 0;
	int pin;

	if(InputArg[i].Enable == ENABLE)
	{
		if ((InputArg[i].GpioMode == MIO) || (InputArg[i].GpioMode == EMIO))
		{
			pin = InputArg[i].Pin + LINUX_GPIO_OFFSET;
			status = (uint8_t)gpio_read(pin);
		}
	}

	return status;
}

#endif // end for if 0

#else
/**
 * The output pin is initialized
 */
int32_t InputInit(void)
{
	int32_t Status = 0;
	uint32_t GpioVal;
	XGpioPs *pPsGpio;
	XGpio *pPlGpio;

	for(uint16_t i = 0; i < INPUT_NAME_MAX; i++)
	{
		if(InputArg[i].Enable == ENABLE)
		{
			if ((InputArg[i].GpioMode == MIO) || (InputArg[i].GpioMode == EMIO))
			{
				pPsGpio = GetPsGpioDev();
				XGpioPs_SetDirectionPin(pPsGpio, InputArg[i].Pin, INPUT);
			}
			else if (InputArg[i].GpioMode == AXI_GPIO)
			{
				pPlGpio = GetPlGpioDev(InputArg[i].DeviceId);
				if (pPlGpio->IsReady == 0)
				{
					Status = XGpio_Initialize(pPlGpio, InputArg[i].DeviceId);
					if (Status != XST_SUCCESS)
					{
						LOG_ERROR("AXI GPIO config failed!\r\n");
						break;
					}
				}
				GpioVal = XGpio_GetDataDirection(pPlGpio, InputArg[i].AxiGpioChl);
				GpioVal = GpioVal | (1 << InputArg[i].Pin);
				XGpio_SetDataDirection(pPlGpio, InputArg[i].AxiGpioChl, GpioVal);//channel x xbit output
			}
			else if (InputArg[i].GpioMode == AXI_BRAM)
			{
			}
		}
		uint8_t aa = Input_GetValue(i);
		uint16_t ii = 0;
	}

	return Status;
}





/**
 * 读取引脚电平
 * @param name 引脚名字
 * @return	1: 高电平     0: 低电平
 */
uint8_t Input_GetValue(INPUT_NAME name)
{
//	return (uint8_t)axi_read_data(name);
	uint32_t GpioVal;
	uint8_t i = (uint8_t)name;
	uint8_t status;

	XGpioPs *pPsGpio;
	XGpio *pPlGpio;

	if(InputArg[i].Enable == ENABLE)
	{
		if ((InputArg[i].GpioMode == MIO) || (InputArg[i].GpioMode == EMIO))
		{
			pPsGpio = GetPsGpioDev();
			status = (uint8_t)XGpioPs_ReadPin(pPsGpio, InputArg[i].Pin);
		}
		else if (InputArg[i].GpioMode == AXI_GPIO)
		{
			pPlGpio = GetPlGpioDev(InputArg[i].DeviceId);
			GpioVal = XGpio_DiscreteRead(pPlGpio, InputArg[i].AxiGpioChl);
			GpioVal = GpioVal & (1 << InputArg[i].Pin);
			status = (GpioVal == 0);
		}
		else if (InputArg[i].GpioMode == AXI_BRAM)
		{
			status = (uint8_t)axi_read_data(InputArg[i].Pin);
		}
	}

	return status;
}
#endif
