'''
OpenOCD telnet-protocol python wrapper.

Written by Artamonov Dmitry <screwer@gmail.com>

This program is free software. It comes without any warranty, to the extent permitted by applicable law.
You can redistribute it and/or modify it under the terms of the WTFPL, Version 2, as published by Sam Hocevar.
See http://www.wtfpl.net/ for more details
'''
#-------------------------------------------------------------------------------------------------

import binascii
import struct

#-------------------------------------------------------------------------------------------------

def ReverseInt16(i):
   i = (i & 0x5555) << 1 | (i >> 1) & 0x5555
   i = (i & 0x3333) << 2 | (i >> 2) & 0x3333
   i = (i & 0x0F0F) << 4 | (i >> 4) & 0x0F0F
   i = (i & 0x00FF) << 8 | (i >> 8)
   return i


class CRC16:

    def __init__(self, MsbFirst = True):
        self.MsbFirst = MsbFirst

    def GenTable(self, poly):
        self.Table = [0] * 256
        self.GenTableMsbFirst(ReverseInt16(poly)) if self.MsbFirst else self.GenTableLsbFirst(poly)

    def Calculate(self, Data, InitialCrcValue = 0):
        return self.CalculateMsbFirst(Data, InitialCrcValue) if self.MsbFirst else self.CalculateLsbFirst(Data, InitialCrcValue)

    def CalculateMsbFirst(self, Data, InitialCrcValue):
        crc = InitialCrcValue
        for c in Data:
            crc = ((crc << 8) & 0xFF00) ^ (self.Table[(crc >> 8) ^ (ord(c) & 0xFF)] & 0xFFFF)
        return crc

    def CalculateLsbFirst(self, Data, InitialCrcValue):
        crc = InitialCrcValue
        for c in Data:
          crc = (crc >> 8) ^ (self.Table[(crc & 0xFF) ^ (ord[c] & 0xFF)] & 0xFFFF)
        return crc

    def GenTableMsbFirst(self, poly):
        for x in xrange(0, 256):
            w = x << 8
            for i in xrange(0, 8):
                w = (w << 1) ^ poly if w & 0x8000 else w << 1
            self.Table[x] = w & 0xFFFF

    def GenTableLsbFirst(self, poly):
        for x in xrange(0, 256):
            w = x
            for i in xrange(0, 8):
                w = (w >> 1) ^ poly if w & 1 else w >> 1
            self.Table[x] = w & 0xFFFF

#-------------------------------------------------------------------------------------------------

def Str2Bin(s):
    return binascii.unhexlify(''.join(s.split('-')))

#-------------------------------------------------------------------------------------------------

CrcPermutation = [
     1,  2,  3,  4, 31,  5,  6,  7,
     8,  9, 10, 11, 12,  0, 13, 25,
    14, 15, 16, 17, 18, 19, 20, 21,
    22, 23, 24, 26, 27, 28, 29, 30,
]

def CrcPermutate(Table):
    Result = []
    for i in xrange(0, 32):
        Idx = CrcPermutation[i] << 3
        Result += Table[Idx:Idx+8]
    return Result

ManyXorValue = 0x9AEF
XorCodeCrc16 = 0x5E31

Glyph_800F4BB = Str2Bin('00-04-04-44-c4-4f-44-44-c4-24-24-2f-b4-24-04-04-00-40-44-24-24-15-0c-04-fe-04-0c-15-24-24-44-40')
Glyph_800F4FB = Str2Bin('00-10-60-02-8c-00-00-fe-92-92-92-92-92-fe-00-00-00-04-04-7e-01-40-7e-42-42-7e-42-7e-42-42-7e-40')

#-------------------------------------------------------------------------------------------------

DesignID = Str2Bin('56-FF-76-06-51-80-48-54-38-18-10-87') 

#-------------------------------------------------------------------------------------------------

def CalcRegCode():

    Crc16_CCIT = CRC16()
    Crc16_CCIT.GenTable(0x8408) # 0x8408 - poly for CCIT
    
    Crc16_RG = CRC16()
    Crc16_RG.Table = CrcPermutate(Crc16_CCIT.Table)

    #
    # Get activation ID's from Chip serial
    #
    ID1 = Crc16_CCIT.Calculate(DesignID[0:6])
    ID2 = Crc16_CCIT.Calculate(DesignID[6:12])
    #
    # Or specify it manually
    #
    '''
    ID1 = 0xDC90    # RG1: 4D6E
    ID2 = 0x5A92    # RG2: A789

    ID1 = 0xB0B3    # RG1: C650
    ID2 = 0x0FAD    # RG2: 401A

    ID1 = 0x3251    # RG1: 8BE3
    ID2 = 0x1351    # RG2: 29E1

    ID1 = 0xB57E    # RG1: 9FDB
    ID2 = 0x43AB    # RG2: 19D6
    
    ID1 = 0xC9E9    # RG1: B04F
    ID2 = 0x0D09    # RG2: 503A
    '''

    print('ID1: %04X' % ID1)
    print('ID2: %04X' % ID2)

    ID = struct.pack('<HH', ID1, ID2)

    CrcInitialValue = ManyXorValue ^ XorCodeCrc16
    RG1 = Crc16_RG.Calculate(Glyph_800F4BB[:-4] + ID, CrcInitialValue)
    RG2 = Crc16_RG.Calculate(Glyph_800F4FB[:-4] + ID, CrcInitialValue)

    print('RG1: %04X' % RG1)
    print('RG2: %04X' % RG2)
    pass

#-------------------------------------------------------------------------------------------------

CalcRegCode()

 