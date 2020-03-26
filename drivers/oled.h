/*
 * @Description: olcd 相关函数
 * @Author: chenxi
 * @Date: 2020-02-10 12:15:34
 * @LastEditTime : 2020-02-10 16:56:42
 * @LastEditors  : chenxi
 */

#ifndef __DRV_OLED_H
#define __DRV_OLED_H

#include "../user/DataType.h"
#include "..//applications//olcdfont.h"

#define I2C_OLED_Device "/dev/i2c-2" // OLED 使用的 I2C
#define I2C_OLED_Addr   0x3C           // 默认 OLED 地址

#define OLED_CMD 0  //写命令
#define OLED_DATA 1 //写数据

// OLED控制用函数
void OLED_WR_Byte(uint8 dat, uint8 cmd);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Refresh_Gram(void);

// OLED应用函数
int oledSetup(void);
void OLED_Clear(void);

void OLED_DrawPoint(uint8 x, uint8 y, uint8 t);
void OLED_Fill(uint8 x1, uint8 y1, uint8 x2, uint8 y2, uint8 dot);
void OLED_ShowChar(uint8 x, uint8 y, uint8 chr, uint8 size, uint8 mode);
void OLED_ShowNum(uint8 x, uint8 y, uint32 num, uint8 len, uint8 size);
void OLED_ShowString(uint8 x, uint8 y, const uint8 *p, uint8 size);
void OLED_ShowPicture(uint8 x, uint8 y, const uint8 *p, uint8 p_w, uint8 p_h);

void OLED_DrawBMP(uint8 x, uint8 y, uint8 chr, uint8 size, uint8 mode);

#endif
