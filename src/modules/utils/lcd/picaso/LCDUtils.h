#ifndef LCDUTILS_H_
#define LCDUTILS_H_

#include "ImagesConstants.h"
#include "Picaso_Serial_4DLib.h"
#include <initializer_list>

void draw_lcd_images (lcd_image_enum_t firstImage, lcd_image_enum_t lastImage, word handle);
void draw_lcd_images (lcd_image_enum_t firstImage, lcd_image_enum_t lastImage, word handle, const std::initializer_list<int>& disabled_btns);

void draw_lcd_images_touch( lcd_image_enum_t firstImage, lcd_image_enum_t lastImage, word handle, int touched_btn);
void draw_lcd_images_touch( lcd_image_enum_t firstImage, lcd_image_enum_t lastImage, word handle, int touched_btn, const std::initializer_list<int>& disabled_btns);

#endif // LCDUTILS_H_

