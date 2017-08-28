// SPE Version 1.2 added peekM and pokeM
// SPE Version 1.1 added bus_ and pin_ functions
/*
 4DSerial - Library for 4D Systems Serial Environment.
 Released into the public domain.
 */

#ifndef Picaso_Serial_4DLib_h
#define Picaso_Serial_4DLib_h

// BEGIN MODIF arduino_picaso_lib
/*
 #if (ARDUINO >= 100)
 #include "Arduino.h" // for Arduino 1.0
 #else
 #include "WProgram.h" // for Arduino 23
 #endif
 */
#include "smoothie_arduino.h"
// END MODIF arduino_picaso_lib
//MODIF reset
#include "DigitalInOut.h"
//END MODIF reset

#include "Picaso_Const4D.h"	         	// Defines for 4dgl constants, generated by conversion of 4DGL constants to target language
#include "SerialConsoleStream.h"

typedef void (*Tcallback4D)(int, unsigned char);

class Picaso_Serial_4DLib {
public:
	Picaso_Serial_4DLib(SerialConsoleStream* virtualPort, PinName resetPin);
	Tcallback4D Callback4D;

	//MODIF reset
	void reset_display();
	//END MODIF reset

	//Compound 4D Routines
	word bus_In();
	void bus_Out(word Bits);
	word bus_Read();
	void bus_Set(word IOMap);
	void bus_Write(word Bits);
	word charheight(char TestChar);
	word charwidth(char TestChar);
	word file_Close(word Handle);
	word file_Count(char * Filename);
	word file_Dir(char * Filename);
	// BEGIN MODIF const
	word file_Erase(const char * Filename);
	// END MODIF const
	word file_Error();
	word file_Exec(char * Filename, word ArgCount, t4DWordArray Args);
	// BEGIN MODIF const
	word file_Exists(const char * Filename);
	// END MODIF const
	word file_FindFirst(char * Filename);
	word file_FindNext();
	char file_GetC(word Handle);
	word file_GetS(char * StringIn, word Size, word Handle);
	word file_GetW(word Handle);
	word file_Image(word X, word Y, word Handle);
	word file_Index(word Handle, word HiSize, word LoSize, word Recordnum);
	word file_LoadFunction(char * Filename);
	// BEGIN MODIF const
	word file_LoadImageControl(const char * Datname, const char * GCIName, word Mode);
	// END MODIF const
	word file_Mount();
	// BEGIN MODIF const
	word file_Open(const char * Filename, char Mode);
	// END MODIF const
	word file_PlayWAV(char * Filename);
	word file_PutC(char Character, word Handle);
	word file_PutS(char * StringOut, word Handle);
	word file_PutW(word Word, word Handle);
	word file_Read(t4DByteArray Data, word Size, word Handle);
	word file_Rewind(word Handle);
	word file_Run(char * Filename, word ArgCount, t4DWordArray Args);
	word file_ScreenCapture(word X, word Y, word Width, word Height, word Handle);
		word file_Seek(word  Handle, word  HiWord, word  LoWord);
		word file_Size(word  Handle, word *  HiWord, word *  LoWord);
		word file_Tell(word  Handle, word *  HiWord, word *  LoWord);
		void file_Unmount();
		word file_Write(word  Size, t4DByteArray  Source, word  Handle);
		word gfx_BevelShadow(word  Value);
		word gfx_BevelWidth(word  Value);
		word gfx_BGcolour(word  Color);
		void gfx_Button(word  Up, word  x, word  y, word  buttonColour, word  txtColour, word  font, word  txtWidth, word  txtHeight, char *   text);
		void gfx_ChangeColour(word  OldColor, word  NewColor);
		void gfx_Circle(word  X, word  Y, word  Radius, word  Color);
		void gfx_CircleFilled(word  X, word  Y, word  Radius, word  Color);
		void gfx_Clipping(word  OnOff);
		void gfx_ClipWindow(word  X1, word  Y1, word  X2, word  Y2);
		void gfx_Cls();
		word gfx_Contrast(word  Contrast);
		void gfx_Ellipse(word  X, word  Y, word  Xrad, word  Yrad, word  Color);
		void gfx_EllipseFilled(word  X, word  Y, word  Xrad, word  Yrad, word  Color);
		word gfx_FrameDelay(word  Msec);
		word gfx_Get(word  Mode);
		word gfx_GetPixel(word  X, word  Y);
		void gfx_Line(word  X1, word  Y1, word  X2, word  Y2, word  Color);
		word gfx_LinePattern(word  Pattern);
		void gfx_LineTo(word  X, word  Y);
		void gfx_MoveTo(word  X, word  Y);
		word gfx_Orbit(word  Angle, word  Distance, word *  Xdest, word *  Ydest);
		word gfx_OutlineColour(word  Color);
		void gfx_Panel(word  Raised, word  X, word  Y, word  Width, word  Height, word  Color);
		void gfx_Polygon(word  n, t4DWordArray  Xvalues, t4DWordArray  Yvalues, word  Color);
		void gfx_PolygonFilled(word  n, t4DWordArray  Xvalues, t4DWordArray  Yvalues, word  Color);
		void gfx_Polyline(word  n, t4DWordArray  Xvalues, t4DWordArray  Yvalues, word  Color);
		void gfx_PutPixel(word  X, word  Y, word  Color);
		void gfx_Rectangle(word  X1, word  Y1, word  X2, word  Y2, word  Color);
		void gfx_RectangleFilled(word  X1, word  Y1, word  X2, word  Y2, word  Color);
		void gfx_ScreenCopyPaste(word  Xs, word  Ys, word  Xd, word  Yd, word  Width, word  Height);
		word gfx_ScreenMode(word  ScreenMode);
		void gfx_Set(word  Func, word  Value);
		void gfx_SetClipRegion();
		word gfx_Slider(word  Mode, word  X1, word  Y1, word  X2, word  Y2, word  Color, word  Scale, word  Value);
		word gfx_Transparency(word  OnOff);
		word gfx_TransparentColour(word  Color);
		void gfx_Triangle(word  X1, word  Y1, word  X2, word  Y2, word  X3, word  Y3, word  Color);
		void gfx_TriangleFilled(word  X1, word  Y1, word  X2, word  Y2, word  X3, word  Y3, word  Color);
		word img_ClearAttributes(word  Handle, word  Index, word  Value);
		word img_Darken(word  Handle, word  Index);
		word img_Disable(word  Handle, word  Index);
		word img_Enable(word  Handle, word  Index);
		word img_GetWord(word  Handle, word  Index, word  Offset );
		word img_Lighten(word  Handle, word  Index);
		word img_SetAttributes(word  Handle, word  Index, word  Value);
		word img_SetPosition(word  Handle, word  Index, word  Xpos, word  Ypos);
		word img_SetWord(word  Handle, word  Index, word  Offset , word  Word);
		word img_Show(word  Handle, word  Index);
		word img_Touched(word  Handle, word  Index);
		word media_Flush();
		void media_Image(word  X, word  Y);
		word media_Init();
		word media_RdSector(t4DSector  SectorIn);
		word media_ReadByte();
		word media_ReadWord();
		void media_SetAdd(word  HiWord, word  LoWord);
		void media_SetSector(word  HiWord, word  LoWord);
		void media_Video(word  X, word  Y);
		void media_VideoFrame(word  X, word  Y, word  Framenumber);
		word media_WriteByte(word  Byte);
		word media_WriteWord(word  Word);
		word media_WrSector(t4DSector  SectorOut);
		word mem_Free(word  Handle);
		word mem_Heap();
		word pin_HI(word Pin);
		word peekM(word  Address);
		word pin_LO(word Pin);
		word pin_Read(word Pin);
		word pin_Set(word Mode, word Pin);
        void putCH(word  WordChar);
		void pokeM(word  Address, word  WordValue) ;
		// BEGIN MODIF const
		word putstr(const char *  InString);
		// END MODIF const
		// BEGIN MODIF wordwrap
		word putstr(const char * InString, word from, word to);
		// END MODIF wordwrap
		void snd_BufSize(word  Bufsize);
		void snd_Continue();
		void snd_Pause();
		word snd_Pitch(word  Pitch);
		word snd_Playing();
		void snd_Stop();
		void snd_Volume(word  Volume);
		word sys_Sleep(word  Units);
		void touch_DetectRegion(word  X1, word  Y1, word  X2, word  Y2);
		word touch_Get(word  Mode);
		void touch_Set(word  Mode);
		word txt_Attributes(word  Attribs);
		word txt_BGcolour(word  Color);
		word txt_Bold(word  Bold);
		word txt_FGcolour(word  Color);
		word txt_FontID(word  FontNumber);
		word txt_Height(word  Multiplier);
		word txt_Inverse(word  Inverse);
		word txt_Italic(word  Italic);
		void txt_MoveCursor(word  Line, word  Column);
		word txt_Opacity(word  TransparentOpaque);
		void txt_Set(word  Func, word  Value);
		word txt_Underline(word  Underline);
		word txt_Width(word  Multiplier);
		word txt_Wrap(word  Position);
		word txt_Xgap(word  Pixels);
		word txt_Ygap(word  Pixels);
		word file_CallFunction(word  Handle, word  ArgCount, t4DWordArray  Args);
		word sys_GetModel(char *  ModelStr);
		word sys_GetVersion();
		word sys_GetPmmC();
		word writeString(word  Handle, char *  StringOut);
		word readString(word  Handle, char *  StringIn);
		void blitComtoDisplay(word  X, word  Y, word  Width, word  Height, t4DByteArray  Pixels);
		word file_FindFirstRet(char *  Filename, char *  StringIn);
		word file_FindNextRet(char *  StringIn);
		void setbaudWait(word  Newrate);
		void GetAck(void);
		
		//4D Global Variables Used
		int Error4D;  				// Error indicator,  used and set by Intrinsic routines
		unsigned char Error4D_Inv;	// Error byte returned from com port, onl set if error = Err_Invalid
	//	int Error_Abort4D;  		// if true routines will abort when detecting an error
		unsigned long TimeLimit4D;	// time limit in ms for total serial command duration, 2000 (2 seconds) should be adequate for most commands
									// assuming a reasonable baud rate AND low latency AND 0 for the Serial Delay Parameter
									// temporary increase might be required for very long (bitmap write, large image file opens)
									// or indeterminate (eg file_exec, file_run, file_callFunction) commands
		
	private:
                SerialConsoleStream * _virtualPort;
        		//MODIF reset
                mbed::DigitalInOut*	 reset_pin;
        		//END MODIF reset


		//Intrinsic 4D Routines
		// BEGIN MODIF thumbnails
		void WriteChars(const char * charsout);
		// END MODIF thumbnails
		void WriteBytes(char * Source, int Size);
		void WriteWords(word * Source, int Size);
		void getbytes(char * data, int size);
		word GetWord(void);
		void getString(char * outStr, int strLen);
		word GetAckResp(void);
		word GetAckRes2Words(word * word1, word * word2);
		void GetAck2Words(word * word1, word * word2);
		word GetAckResSector(t4DSector Sector);
		word GetAckResStr(char * OutStr);
		word GetAckResData(t4DByteArray OutData, word size);
		void SetThisBaudrate(int Newrate);
};
 
#endif
