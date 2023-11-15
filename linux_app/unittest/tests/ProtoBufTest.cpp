/*****************************************************************************
 * Copyright (c) 2023
 * Skyfend Technology Co., Ltd
 *
 * All rights reserved. Any unauthorized disclosure or publication of the
 * confidential and proprietary information to any other party will constitute
 * an infringement of copyright laws.
 *
 * $Id$
 * Author  : JiaWei, jiawei@autel.com
 * Created : 20 September 2023
 * Purpose : Unit test
 *
 * $Rev$
 * $URL$
 *
 * Notes:
 *
 *****************************************************************************
 */
#include "ProtoBufTest.hpp"

#include "gtest/gtest.h"
#include <strstream>
#include "../../src/eap/alink_pb/TracerUavMsg.pb.h"

void ProtoBufTest::SetUpTestSuite()
{
}

void ProtoBufTest::TearDownTestSuite()
{
}

void ProtoBufTest::SetUp()
{
}

void ProtoBufTest::TearDown()
{
}

#if 0
TEST_F_A(ProtoBufTest, AngleInfoPBTest, jiaweiA23411)
{
    protobuf::traceruav::AngleInfoPB angleData;
    const std::uint32_t maxLength = 200;
    std::uint8_t buffer[500] = {0};
    std::ostrstream err;
    bool result = false;

    angleData.set_roll(1.1);
    angleData.set_yaw(2.2);
    angleData.set_pitch(3.3);
    std::size_t dataLength = angleData.ByteSizeLong();

    // std::cout << "angleData: "<< angleData.roll() << ", " << angleData.yaw() << ", " << angleData.pitch() << ", angleData.ByteSizeLong()=" << angleData.ByteSizeLong() << std::endl;

    if (dataLength <= maxLength)
    {
       result = angleData.SerializeToArray(buffer, angleData.ByteSizeLong());
       EXPECT_TRUE(result);

       for (uint32_t pos = 0; pos < dataLength; pos++)
       {
           err << " " << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)(buffer[pos]);
       }
       // std::cout << "SerializeToArray : " << std::boolalpha << result << ", " << err.str() << std::endl;
    }

    protobuf::traceruav::AngleInfoPB angleData1;
    EXPECT_EQ(angleData1.ByteSizeLong(), (std::size_t)0);

    if (dataLength <= maxLength)
    {
       result = angleData1.ParseFromArray(buffer, dataLength);
       EXPECT_TRUE(result);
       // std::cout << "angleData1: "<< angleData1.roll() << ", " << angleData1.yaw() << ", " << angleData1.pitch() << ", angleData1.ByteSizeLong()=" << angleData1.ByteSizeLong() << std::endl;
    }

    EXPECT_EQ(angleData1.ByteSizeLong(), angleData.ByteSizeLong());
    EXPECT_EQ(angleData.roll(), angleData1.roll());
    EXPECT_EQ(angleData.yaw(), angleData1.yaw());
    EXPECT_EQ(angleData.pitch(), angleData1.pitch());
}

TEST_F_A(ProtoBufTest, FlyStatusPBTest, jiaweiA23411)
{
    protobuf::traceruav::FlyStatusPB aapsData;
    const std::uint32_t maxLength = 200;
    std::uint8_t buffer[500] = {0};
    std::ostrstream err;
    bool result = false;

    aapsData.set_yaw(0x12345671);
    aapsData.set_pitch(0x12345672);
    aapsData.set_roll(0x12345673);
    aapsData.set_wy(0x12345674);
    aapsData.set_wp(0x12345675);
    aapsData.set_wr(0x12345676);
    aapsData.set_longitude(0x1234567812345671);
    aapsData.set_latitude(0x1234567812345672);
    aapsData.set_altitude(0x1234567812345673);
    aapsData.set_vx(0x12345677);
    aapsData.set_vy(0x12345678);
    aapsData.set_vz(0x12345679);

    std::size_t dataLength = aapsData.ByteSizeLong();

    if (dataLength <= maxLength)
    {
       result = aapsData.SerializeToArray(buffer, dataLength);
       EXPECT_TRUE(result);

       for (uint32_t pos = 0; pos < dataLength; pos++)
       {
           err << " " << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)(buffer[pos]);
       }
       // std::cout << "SerializeToArray : " << std::boolalpha << result << ", " << err.str() << std::endl;
    }

    protobuf::traceruav::FlyStatusPB aapsData1;
    EXPECT_EQ(aapsData1.ByteSizeLong(), (std::size_t)0);

    if (dataLength <= maxLength)
    {
       result = aapsData1.ParseFromArray(buffer, dataLength);
       EXPECT_TRUE(result);
       // std::cout << "angleData1: "<< angleData1.roll() << ", " << angleData1.yaw() << ", " << angleData1.pitch() << ", angleData1.ByteSizeLong()=" << angleData1.ByteSizeLong() << std::endl;
    }

    EXPECT_EQ(aapsData1.ByteSizeLong(), aapsData.ByteSizeLong());
    EXPECT_EQ(aapsData1.longitude(), aapsData.longitude());
    EXPECT_EQ(aapsData1.vx(), aapsData.vx());
}

TEST_F_A(ProtoBufTest, TracerUavMessagePBTest, jiaweiA23411)
{
    const std::uint32_t maxLength = 200;
    std::uint8_t buffer[500] = {0};
    std::ostrstream err;
    bool result = false;

    protobuf::traceruav::MeasureInfoPb meaInfoData;
    protobuf::traceruav::TracerUavMessagePB uavMsgData;

    meaInfoData.set_workstatus(2);
    meaInfoData.set_tarnum(3);

    protobuf::traceruav::MeasureInfoPb_MeasureInfoData *meaData = meaInfoData.add_meainfodata();
    meaData->set_tnum(0x12345671);
    meaData->set_tfreq(0x12345672);
    meaData->set_tamp(0x12345673);
    meaData->set_tmfw(0x12345674);
    meaData->set_tmfy(0x12345675);
    meaData->set_tffw(0x12345676);
    meaData->set_tffy(0x12345677);

    protobuf::traceruav::MeasureInfoPb_MeasureInfoData *meaData2 = meaInfoData.add_meainfodata();
    meaData2->set_tnum(0x99123450);
    meaData2->set_tfreq(0x99123451);
    meaData2->set_tamp(0x99123452);
    meaData2->set_tmfw(0x99123453);
    meaData2->set_tmfy(0x99123454);
    meaData2->set_tffw(0x99123455);
    meaData2->set_tffy(0x99123456);

    uavMsgData.mutable_measureinfo()->CopyFrom(meaInfoData);

    std::size_t dataLength = uavMsgData.ByteSizeLong();
    // std::cout << "uavMsgData dataLength : " << dataLength << std::endl;

    if (dataLength <= maxLength)
    {
    	result = uavMsgData.SerializeToArray(buffer, dataLength);
        EXPECT_TRUE(result);

    	for (uint32_t pos = 0; pos < dataLength; pos++)
    	{
    		err << " " << std::hex << std::setw(2) << std::setfill('0') << (uint16_t)(buffer[pos]);
    	}
    	// std::cout << "SerializeToArray : " << std::boolalpha << result << ", " << err.str() << std::endl;
    }

    
    protobuf::traceruav::TracerUavMessagePB uavMsgData1;
    result = uavMsgData1.ParseFromArray(buffer, dataLength);
    EXPECT_TRUE(result);

    const protobuf::traceruav::MeasureInfoPb& meaInfoData1 = uavMsgData1.measureinfo();

    const protobuf::traceruav::MeasureInfoPb_MeasureInfoData& data1 = meaInfoData1.meainfodata(0);
    const protobuf::traceruav::MeasureInfoPb_MeasureInfoData& data2 = meaInfoData1.meainfodata(1);

    // std::cout << "size=" << (std::uint32_t)meaInfoData1.meainfodata_size() << std::endl;

    EXPECT_EQ(meaInfoData1.meainfodata_size(), 2);
    EXPECT_EQ(data1.tnum(), meaData->tnum());
    EXPECT_EQ(data2.tamp(), meaData2->tamp());
}
#endif
