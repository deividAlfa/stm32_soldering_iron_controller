'''
https://habr.com/ru/articles/31180/
https://habr.com/ru/companies/slurm/articles/746622/


https://radiokot.ru/forum/viewtopic.php?f=2&t=154278&start=4140

$ python3 stm32-ss-keygen.py translate 56-FF-76-06-51-80-48-54-38-18-10-87
ID1: DC90
ID2: 5A92
$ python3 stm32-ss-keygen.py generate -p t12 0xDC90 0x5A92
ID1: DC90
ID2: 5A92
RG1: 4D6E
RG2: A789
$ python3 stm32-ss-keygen.py generate -p combo 0xDC90 0x5A92             
ID1: DC90
ID2: 5A92
RG1: 74E6
RG2: 5D03
$ python3 stm32-ss-keygen.py generate -p rework 0xDC90 0x5A92
ID1: DC90
ID2: 5A92
RG1: 8A90
RG2: 7954


STM32 Soldering Station Firmware Registration Handler.

Loosely based on code by Dmitry Artamonov <screwer@gmail.com>.
REF: https://web.archive.org/web/20220328075324/http://t12.omegahg.com/keygen.py

This program is free software. It comes without any warranty, to the extent permitted by applicable law.
You can redistribute it and/or modify it under the terms of the WTFPL, Version 2, as published by Sam Hocevar.
See http://www.wtfpl.net/ for more details

T12/Desolderer sample data:

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

ID1 = 0x8CB7    # RG1: 92F0
ID2 = 0x1B90    # RG2: 0114

Combo sample data:

ID1 = 0x45D1    # RG1: E68F
ID2 = 0x6C6D    # RG2: FECC

SMD Rework Station sample data:

ID1 = 0xE784    # RG1: B968
ID2 = 0xE7F4    # RG2: 4AAC

ID1 = 0x0130    # RG1: 5A09
ID2 = 0x2028    # RG2: 9AFC

ID1 = 0xD06F    # RG1: C8A1
ID2 = 0xDCF4    # RG2: 1754

generate -h 0xDC90 0x5A92
'''

import binascii
import struct
import argparse
import functools


class CRC16:
    CCIT_POLY = 0x8408

    def __init__(self, Extra=0):
        self.Extra = Extra

    def ReverseInt16(self, i):
        i = (i & 0x5555) << 1 | (i >> 1) & 0x5555
        i = (i & 0x3333) << 2 | (i >> 2) & 0x3333
        i = (i & 0x0F0F) << 4 | (i >> 4) & 0x0F0F
        i = (i & 0x00FF) << 8 | (i >> 8)
        return i

    def GenTable(self, poly):
        self.Table = [0] * 256
        poly = self.ReverseInt16(poly)

        for x in range(0, 256):
            w = x << 8
            for _ in range(0, 8):
                w = (w << 1) ^ poly if w & 0x8000 else w << 1
            self.Table[x] = w & 0xFFFF

    def Calculate(self, Data, InitialCrcValue=0):
        crc = InitialCrcValue
        for c in Data:
            crc = ((crc << 8) & 0xFF00) ^ (self.Table[((crc + self.Extra) >> 8) ^ (c & 0xFF)] & 0xFFFF)
        return crc

    def Permutate(self, CrcPermutation):
        Result = []
        for i in range(0, 32):
            Idx = CrcPermutation[i] << 3
            Result += self.Table[Idx : Idx + 8]
        self.Table = Result


ProductKeys = {
    "t12": {
        "ivec": b"\xB5\x06\x46\x42\xF2\xD3\x60\x81\xEA\x00\x04\x43\xF2\xF2\x60\x44\x40\x0D\x46\x40\xF2\x91\x60\x44\x40\x44\xF6\x82",
        "xor": 0x9ACF ^ 0x20,
        "permutation": [
            1,
            2,
            3,
            4,
            31,
            5,
            6,
            7,
            8,
            9,
            10,
            11,
            12,
            0,
            13,
            25,
            14,
            15,
            16,
            17,
            18,
            19,
            20,
            21,
            22,
            23,
            24,
            26,
            27,
            28,
            29,
            30,
        ],
        # These are part of Chinese 1bpp glyphs, but it does not matter for the registration purpose.
        "prefix_id1": b"\x00\x04\x04\x44\xc4\x4f\x44\x44\xc4\x24\x24\x2f\xb4\x24\x04\x04\x00\x40\x44\x24\x24\x15\x0c\x04\xfe\x04\x0c\x15",
        "prefix_id2": b"\x00\x10\x60\x02\x8c\x00\x00\xfe\x92\x92\x92\x92\x92\xfe\x00\x00\x00\x04\x04\x7e\x01\x40\x7e\x42\x42\x7e\x42\x7e",
    },
    "combo": {
        "ivec": b"\xB5\x06\x46\x41\xF2\x91\x60\x81\xEA\x00\x02\x46\xF6\x17\x60\x42\x40\x41\xF2\x11\x60\x42",
        "xor": 0x2A92 ^ 0x20,
        "permutation": [
            1,
            2,
            3,
            4,
            31,
            5,
            6,
            7,
            20,
            8,
            9,
            10,
            11,
            12,
            0,
            13,
            24,
            25,
            14,
            15,
            16,
            17,
            18,
            19,
            21,
            22,
            23,
            26,
            27,
            28,
            29,
            30,
        ],
        "prefix_id1": b"\x04\x04\x44\xc4\x4f\x44\x44\xc4\x24\x24\x2f\xb4\x24\x04\x04\x00\x40\x44\x24\x24\x15\x0c\x04\xfe\x04\x0c\x15\x24",
        "prefix_id2": b"\x10\x60\x02\x8c\x00\x00\xfe\x92\x92\x92\x92\x92\xfe\x00\x00\x00\x04\x04\x7e\x01\x40\x7e\x42\x42\x7e\x42\x7e\x42",
    },
    "rework": {
        "prefix_id1": b"\x00\x00\xF8\x49\x4A\x4C\x48\xF8\x48\x4C\x4A\x49\xF8\x00\x00\x00\x10\x10\x13\x12\x12\x12\x12\xFF\x12\x12\x12\x12",
        "prefix_id2": b"\x10\x13\x12\x12\x12\x12\xFF\x12\x12\x12\x12\x13\x10\x10\x00\x10\x60\x02\x8C\x00\x00\xFE\x92\x92\x92\x92\x92\xFE",
        "crc_extra": 0xB,
    },
}


def CalcRegCode(product, id1, id2):
    print(f'ID1: {id1:04X}')
    print(f'ID2: {id2:04X}')

    ID = struct.pack('<HH', id1, id2)

    context = ProductKeys[product]

    if 'permutation' in context:
        Crc16_CCIT = CRC16()
        Crc16_CCIT.GenTable(CRC16.CCIT_POLY)

        Crc16_RG = CRC16()
        Crc16_RG.GenTable(CRC16.CCIT_POLY)
        Crc16_RG.Permutate(context['permutation'])

        XorCodeCrc16 = Crc16_CCIT.Calculate(context['ivec'])

        CrcInitialValue = XorCodeCrc16 ^ context['xor']

        # print(f'CRC: {CrcInitialValue:04X}')

        RG1 = Crc16_RG.Calculate(context['prefix_id1'] + ID, CrcInitialValue)
        RG2 = Crc16_RG.Calculate(context['prefix_id2'] + ID, CrcInitialValue)
    else:
        Crc16_CCIT = CRC16(Extra=context['crc_extra'])
        Crc16_CCIT.GenTable(CRC16.CCIT_POLY)

        RG1 = Crc16_CCIT.Calculate(context['prefix_id1'] + ID)
        RG2 = Crc16_CCIT.Calculate(context['prefix_id2'] + ID)

    print(f'RG1: {RG1:04X}')
    print(f'RG2: {RG2:04X}')


def TranslateCode(mcu):
    # '56-FF-76-06-51-80-48-54-38-18-10-87'

    DesignID = binascii.unhexlify(''.join(mcu.split('-')))


    if len(DesignID) != 12:
        print("ERROR: Design ID length must be 12 bytes!")
        return -1

    Crc16_CCIT = CRC16()
    Crc16_CCIT.GenTable(CRC16.CCIT_POLY)

    id1 = Crc16_CCIT.Calculate(DesignID[0:6])
    id2 = Crc16_CCIT.Calculate(DesignID[6:12])

    print(f'ID1: {id1:04X}')
    print(f'ID2: {id2:04X}')


def main():
    while True :
        x=input('Что будем делать?\n1 - Генерировать\n2 - Транслировать\n3 - Выйти\n')
        try:
            iMission=int(x, 10) #число
            if iMission==1:
               sMission="Generate"
               while True:
                    x=input('Какой тип станции?\n1 - Паяльник Т12\n2 - Комбо\n3 - Переделка\n')
                    try:
                         iMission=int(x, 10) #число
                         if iMission==1:
                               sType="t12"
                         elif iMission==2:
                               sType="combo"
                         elif iMission==3:
                               sType="rework"
                         else:
                            raise Exception()
                         break
                    except:
                        print("Неверный ввод типа станции. Только цифры \'1\', \'2\', \'3\'")
               while True:
                    sID1=input('ID1:\n')
                    try:
                        xID1=int(sID1,16)
                    except:
                        print(f'Неверный ID1 - {sID1}')
                    sID2=input('ID2:\n')
                    try:
                        xID2=int(sID2,16)
                        CalcRegCode( sType   ,xID1, xID2)
                        break
                    except:
                        print(f'Неверный ID2 - {sID2}')                
 
            elif iMission==2:
               sMission="Translate"
               while True:
                   sID=input('ID процессора\nДлина идентификатора должна составлять 12 байт\nНапример (дефис не обязателен):\n56-FF-76-06-51-80-48-54-38-18-10-87\n')
                   try:
                     xxx=  TranslateCode(sID)
                     break
                   except ValueError as e:
                            print(e)
            elif iMission==3:
               break
            else:
                raise Exception()
            break
        except:
            print("Неверный ввод. Только цифры \'1\', \'2\', \'3\'")
    input("Нажмите Enter для завершения")
main()
