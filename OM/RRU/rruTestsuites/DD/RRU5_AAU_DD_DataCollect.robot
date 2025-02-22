# encoding utf-8
'''
*************************************************************
*** COPYRIGHT DaTang Mobile Communications Equipment CO.,LTD
*************************************************************
*文件名称：RRU12_AAU_DD_DataCollect.robot
*功能描述：5G3 AAU DD时域抓数
*使用方法：
1.抓数时在osp对应aiu面板上操作，需注意打印的返回信息，如下图红线标注
注：以下命令所描述的天线1-4对应射频板天线0，16,  32，48，
-----------------------------------------------------------------------------------
*修改记录：                                                                       |
*##修改日期    |    修改人    |    修改描述    |                                  |
*2018-11-21        zhaobaoxin       创建文件                                      |
*2018-11-22        zhaobaoxin       添加时域上下行抓数用例                        |
----------------------------------------------------------------------------------|
*************************************************************
______________________________________________________________________________________
*用例记录：（记录用例测试出的问题，引用关键字遇到的异常问题等等）                    |
**                                                                                   |
**                                                                                   |
_____________________________________________________________________________________|

************************************************用例文件头结束**************************************************
'''
from pysnmp.smi.rfc1902 import ObjectIdentity

*** Keywords ***

*** Settings ***
Resource        ../../rruResources/DD/ddResources.robot

*** Variables ***
${link_direction_up}    ${0x00030000}
${link_direction_down}    ${0x00010000}
${FILE_PRO2DDR0_BIN}                PRO2DDR0.bin
${AIU_address}                     172.27.45.250
*** test case ***
Data Collect Test
    Data Collect In Time Piling    ${link_direction_up}
    Data Collect In Time Piling    ${link_direction_down}
