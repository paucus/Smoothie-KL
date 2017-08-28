#include "LCDUtils.h"

#include "LCD4DModule.h"
#include "utils.h"

void draw_lcd_images( lcd_image_enum_t firstImage, lcd_image_enum_t lastImage, word handle){

	for (int i = firstImage; i <= lastImage; i++) {
		THELCD->lcd->img_Enable(handle, i);
		THELCD->lcd->img_ClearAttributes(handle, i, I_TOUCH_DISABLE);
		THELCD->lcd->img_SetWord(handle, i, IMAGE_INDEX, 0);
		THELCD->lcd->img_Show(handle, i);
	}

}

void draw_lcd_images( lcd_image_enum_t firstImage, lcd_image_enum_t lastImage, word handle, const std::initializer_list<int>& disabled_btns){

	for (int i = firstImage; i <= lastImage; i++) {
		THELCD->lcd->img_Enable(handle, i);
		THELCD->lcd->img_ClearAttributes(handle, i, I_TOUCH_DISABLE);
		THELCD->lcd->img_SetWord(handle, i, IMAGE_INDEX, is_in<int>(i, disabled_btns) ? 2 : 0 );
		THELCD->lcd->img_Show(handle, i);
	}

}

void draw_lcd_images_touch( lcd_image_enum_t firstImage, lcd_image_enum_t lastImage, word handle, int touched_btn) {
	for (int i = firstImage; i <= lastImage; i++) {
		THELCD->lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == touched_btn) ? 1 : 0 );
		THELCD->lcd->img_Show(handle, i);
	}
}
void draw_lcd_images_touch( lcd_image_enum_t firstImage, lcd_image_enum_t lastImage, word handle, int touched_btn, const std::initializer_list<int>& disabled_btns) {
	for (int i = firstImage; i <= lastImage; i++) {
		THELCD->lcd->img_SetWord(handle, i, IMAGE_INDEX, is_in<int>(i, disabled_btns) ? 2 : (i == touched_btn) ? 1 : 0 );
		THELCD->lcd->img_Show(handle, i);
	}
}
