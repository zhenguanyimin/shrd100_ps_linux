/* LzmaUtil.c -- Test application for LZMA compression
2010-09-20 : Igor Pavlov : Public domain */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Alloc.h"
#include "7zFile.h"
#include "7zVersion.h"
#include "LzmaDec.h"
#include "LzmaEnc.h"

const char *kCantReadMessage = "Can not read input file";
const char *kCantWriteMessage = "Can not write output file";
const char *kCantAllocateMessage = "Can not allocate memory";
const char *kDataErrorMessage = "Data error";

static void *SzAlloc(void *p, size_t size) { p = p; return MyAlloc(size); }
static void SzFree(void *p, void *address) { p = p; MyFree(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

void PrintHelp(char *buffer)
{
  strcat(buffer, "\nLZMA Utility " MY_VERSION_COPYRIGHT_DATE "\n"
      "\nUsage:  lzma <e|d> inputFile outputFile\n"
             "  e: encode file\n"
             "  d: decode file\n");
}

int PrintError(char *buffer, const char *message)
{
  strcat(buffer, "\nError: ");
  strcat(buffer, message);
  strcat(buffer, "\n");
  return 1;
}

int PrintErrorNumber(char *buffer, SRes val)
{
  sprintf(buffer + strlen(buffer), "\nError code: %x\n", (unsigned)val);
  return 1;
}

int PrintUserError(char *buffer)
{
  return PrintError(buffer, "Incorrect command");
}

#define IN_BUF_SIZE (1 << 16)
#define OUT_BUF_SIZE (1 << 16)

static SRes Decode2(CLzmaDec *state, ISeqOutStream *outStream, ISeqInStream *inStream,
    UInt64 unpackSize)
{
  int thereIsSize = (unpackSize != (UInt64)(Int64)-1);
  Byte inBuf[IN_BUF_SIZE];
  Byte outBuf[OUT_BUF_SIZE];
  size_t inPos = 0, inSize = 0, outPos = 0;
  LzmaDec_Init(state);
  for (;;)
  {
    if (inPos == inSize)
    {
      inSize = IN_BUF_SIZE;
      RINOK(inStream->Read(inStream, inBuf, &inSize));
      inPos = 0;
    }
    {
      SRes res;
      SizeT inProcessed = inSize - inPos;
      SizeT outProcessed = OUT_BUF_SIZE - outPos;
      ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
      ELzmaStatus status;
      if (thereIsSize && outProcessed > unpackSize)
      {
        outProcessed = (SizeT)unpackSize;
        finishMode = LZMA_FINISH_END;
      }
      
      res = LzmaDec_DecodeToBuf(state, outBuf + outPos, &outProcessed,
        inBuf + inPos, &inProcessed, finishMode, &status);
      inPos += inProcessed;
      outPos += outProcessed;
      unpackSize -= outProcessed;
      
      if (outStream)
        if (outStream->Write(outStream, outBuf, outPos) != outPos)
          return SZ_ERROR_WRITE;
        
      outPos = 0;
      
      if (res != SZ_OK || thereIsSize && unpackSize == 0)
        return res;
      
      if (inProcessed == 0 && outProcessed == 0)
      {
        if (thereIsSize || status != LZMA_STATUS_FINISHED_WITH_MARK)
          return SZ_ERROR_DATA;
        return res;
      }
    }
  }
}

static SRes Decode(ISeqOutStream *outStream, ISeqInStream *inStream)
{
  UInt64 unpackSize;
  int i;
  SRes res = 0;

  CLzmaDec state;

  /* header: 5 bytes of LZMA properties and 8 bytes of uncompressed size */
  unsigned char header[LZMA_PROPS_SIZE + 8];

  /* Read and parse header */

  RINOK(SeqInStream_Read(inStream, header, sizeof(header)));

  unpackSize = 0;
  for (i = 0; i < 8; i++)
    unpackSize += (UInt64)header[LZMA_PROPS_SIZE + i] << (i * 8);

  LzmaDec_Construct(&state);
  RINOK(LzmaDec_Allocate(&state, header, LZMA_PROPS_SIZE, &g_Alloc));
  res = Decode2(&state, outStream, inStream, unpackSize);
  LzmaDec_Free(&state, &g_Alloc);
  return res;
}

static SRes Encode(ISeqOutStream *outStream, ISeqInStream *inStream, UInt64 fileSize)
{
  CLzmaEncHandle enc;
  SRes res;
  CLzmaEncProps props;

  enc = LzmaEnc_Create(&g_Alloc);
  if (enc == 0)
    return SZ_ERROR_MEM;

  LzmaEncProps_Init(&props);
  res = LzmaEnc_SetProps(enc, &props);

  if (res == SZ_OK)
  {
    Byte header[LZMA_PROPS_SIZE + 8];
    size_t headerSize = LZMA_PROPS_SIZE;
    int i;

    res = LzmaEnc_WriteProperties(enc, header, &headerSize);
    for (i = 0; i < 8; i++)
      header[headerSize++] = (Byte)(fileSize >> (8 * i));
    if (outStream->Write(outStream, header, headerSize) != headerSize)
      res = SZ_ERROR_WRITE;
    else
    {
      if (res == SZ_OK)
        res = LzmaEnc_Encode(enc, outStream, inStream, NULL, &g_Alloc, &g_Alloc);
    }
  }
  LzmaEnc_Destroy(enc, &g_Alloc, &g_Alloc);
  return res;
}

void lzmaPrintError(const char *pcErrorMsg)
{
    printf("\nLzma Error: %s\n", pcErrorMsg);
}

void lzmaPrintErrorNumber(SRes res)
{
    if (res == SZ_ERROR_MEM)
    {
        lzmaPrintError("Can not allocate memory");
    }
    else if (res == SZ_ERROR_DATA)
    {
        lzmaPrintError("Data error");
    }
    else if (res == SZ_ERROR_WRITE)
    {
        lzmaPrintError("Can not write output file");
    }
    else if (res == SZ_ERROR_READ)
    {
        lzmaPrintError("Can not read input file");
    }
    else
    {
        printf("\nLzma Error code : 0x%x\n", (unsigned)res);
    }
}

int lzmaCompress(const Byte ucEncodeMode, const char *pcInputFile, const char *pcOutputFile)
{
    int res = SZ_OK;
    CFileSeqInStream inStream;
    CFileOutStream outStream;

    FileSeqInStream_CreateVTable(&inStream);
    File_Construct(&inStream.file);

    FileOutStream_CreateVTable(&outStream);
    File_Construct(&outStream.file);

    res = InFile_Open(&inStream.file, pcInputFile);
    if (SZ_OK != res)
    {
        lzmaPrintError("Can not open input file");
        return res;
    }
    res = OutFile_Open(&outStream.file, pcOutputFile);
    if (SZ_OK != res)
    {
        lzmaPrintError("Can not open output file");
        File_Close(&inStream.file);
        return res;
    }
    if (ucEncodeMode)
    {
		UInt64 fileSize;
		File_GetLength(&inStream.file, &fileSize);
		res = Encode(&outStream.s, &inStream.s, fileSize);
    }
    else
    {
		res = Decode(&outStream.s, &inStream.s);
    }
    File_Close(&outStream.file);
    File_Close(&inStream.file);

    if (res != SZ_OK)
    {
        lzmaPrintErrorNumber(res);
    }
    return res;
}