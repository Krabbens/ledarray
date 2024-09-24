#ifndef __INC_CHIPSETS_H
#define __INC_CHIPSETS_H

#include "FastLED.h"
#include "pixeltypes.h"
#include "five_bit_hd_gamma.h"

/// @file chipsets.h
/// Contains the bulk of the definitions for the various LED chipsets supported.

FASTLED_NAMESPACE_BEGIN

/// @defgroup Chipsets LED Chipset Controllers
/// Implementations of ::CLEDController classes for various led chipsets.
///
/// @{

#if defined(ARDUINO) //&& defined(SoftwareSerial_h)


#if defined(SoftwareSerial_h) || defined(__SoftwareSerial_h)
#include <SoftwareSerial.h>

#define HAS_PIXIE

/// Adafruit Pixie controller class
/// @tparam DATA_PIN the pin to write data out on
/// @tparam RGB_ORDER the RGB ordering for the LED data
template<uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class PixieController : public CPixelLEDController<RGB_ORDER> {
	SoftwareSerial Serial;
	CMinWait<2000> mWait;

public:
	PixieController() : Serial(-1, DATA_PIN) {}

protected:
	/// Initialize the controller
	virtual void init() {
		Serial.begin(115200);
		mWait.mark();
	}

	/// @copydoc CPixelLEDController::showPixels()
	virtual void showPixels(PixelController<RGB_ORDER> & pixels) {
		mWait.wait();
		while(pixels.has(1)) {
			uint8_t r = pixels.loadAndScale0();
			Serial.write(r);
			uint8_t g = pixels.loadAndScale1();
			Serial.write(g);
			uint8_t b = pixels.loadAndScale2();
			Serial.write(b);
			pixels.advanceData();
			pixels.stepDithering();
		}
		mWait.mark();
	}

};

// template<SoftwareSerial & STREAM, EOrder RGB_ORDER = RGB>
// class PixieController : public PixieBaseController<STREAM, RGB_ORDER> {
// public:
// 	virtual void init() {
// 		STREAM.begin(115200);
// 	}
// };
#endif
#endif

/// @defgroup ClockedChipsets Clocked Chipsets
/// Nominally SPI based, these chipsets have a data and a clock line.
/// @{


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// APA102 definition - takes data/clock/select pin values (N.B. should take an SPI definition?)
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// APA102 controller class.
/// @tparam DATA_PIN the data pin for these LEDs
/// @tparam CLOCK_PIN the clock pin for these LEDs
/// @tparam RGB_ORDER the RGB ordering for these LEDs
/// @tparam SPI_SPEED the clock divider used for these LEDs.  Set using the ::DATA_RATE_MHZ / ::DATA_RATE_KHZ macros.  Defaults to ::DATA_RATE_MHZ(12)
template <
	uint8_t DATA_PIN, uint8_t CLOCK_PIN,
	EOrder RGB_ORDER = RGB,
	uint32_t SPI_SPEED = DATA_RATE_MHZ(12),
	uint8_t BUS = 0,
	FiveBitGammaCorrectionMode GAMMA_CORRECTION_MODE = kFiveBitGammaCorrectionMode_Null,
	uint32_t START_FRAME = 0x00000000,
	uint32_t END_FRAME = 0xFF000000
>
class APA102Controller : public CPixelLEDController<RGB_ORDER> {
	typedef SPIOutput<DATA_PIN, CLOCK_PIN, SPI_SPEED, BUS> SPI;
	SPI mSPI;

	void startBoundary() {
		mSPI.writeWord(START_FRAME >> 16);
		mSPI.writeWord(START_FRAME & 0xFFFF);
	}
	void endBoundary(int nLeds) {
		int nDWords = (nLeds/32);
		const uint8_t b0 = uint8_t(END_FRAME >> 24 & 0x000000ff);
		const uint8_t b1 = uint8_t(END_FRAME >> 16 & 0x000000ff);
		const uint8_t b2 = uint8_t(END_FRAME >>  8 & 0x000000ff);
		const uint8_t b3 = uint8_t(END_FRAME >>  0 & 0x000000ff);
		do {
			mSPI.writeByte(b0);
			mSPI.writeByte(b1);
			mSPI.writeByte(b2);
			mSPI.writeByte(b3);
		} while(nDWords--);
	}

	inline void writeLed(uint8_t brightness, uint8_t b0, uint8_t b1, uint8_t b2) __attribute__((always_inline)) {
#ifdef FASTLED_SPI_BYTE_ONLY
		mSPI.writeByte(0xE0 | brightness);
		mSPI.writeByte(b0);
		mSPI.writeByte(b1);
		mSPI.writeByte(b2);
#else
		// uint16_t b = 0xE000 | (brightness << 8) | (uint16_t)b0;
		// mSPI.writeWord(b);
		// uint16_t w = b1 << 8;
		// w |= b2;
		// mSPI.writeWord(w);
		uint32_t b = 0xE0000000 | (brightness << 24) | (uint32_t)b0 << 16 | (uint32_t)b1 << 8 | (uint32_t)b2;
		mSPI.writeFullPixel(b);
#endif
	}

	inline void write2Bytes(uint8_t b1, uint8_t b2) __attribute__((always_inline)) {
#ifdef FASTLED_SPI_BYTE_ONLY
		mSPI.writeByte(b1);
		mSPI.writeByte(b2);
#else
		mSPI.writeWord(uint16_t(b1) << 8 | b2);
#endif
	}

public:
	APA102Controller() {}

	virtual void init() {
		mSPI.init();
	}

protected:
	/// @copydoc CPixelLEDController::showPixels()
	virtual void showPixels(PixelController<RGB_ORDER> & pixels) {
		switch (GAMMA_CORRECTION_MODE) {
			case kFiveBitGammaCorrectionMode_Null: {
				showPixelsDefault(pixels);
				break;
			}
			case kFiveBitGammaCorrectionMode_BitShift: {
				showPixelsGammaBitShift(pixels);
				break;
			}
		}
	}

	virtual void selectSPI() {
		mSPI.select();
	}

	virtual void releaseSPI() {
		mSPI.release();
	}

private:

	static inline void getGlobalBrightnessAndScalingFactors(
		    PixelController<RGB_ORDER>& pixels,
		    uint8_t* out_s0, uint8_t* out_s1, uint8_t* out_s2, uint8_t* out_brightness) {
		uint8_t s0 = pixels.getScale0();
		uint8_t s1 = pixels.getScale1();
		uint8_t s2 = pixels.getScale2();
#if FASTLED_USE_GLOBAL_BRIGHTNESS == 1
		const uint16_t maxBrightness = 0x1F;
		uint16_t brightness = ((((uint16_t)max(max(s0, s1), s2) + 1) * maxBrightness - 1) >> 8) + 1;
		s0 = (maxBrightness * s0 + (brightness >> 1)) / brightness;
		s1 = (maxBrightness * s1 + (brightness >> 1)) / brightness;
		s2 = (maxBrightness * s2 + (brightness >> 1)) / brightness;
#else
		const uint8_t brightness = 0x1F;
#endif
		*out_s0 = s0;
		*out_s1 = s1;
		*out_s2 = s2;
		*out_brightness = static_cast<uint8_t>(brightness);
	}

	// Legacy showPixels implementation.
	inline void showPixelsDefault(PixelController<RGB_ORDER> & pixels) {
		int32_t start = micros();
		// mSPI.select();
		uint8_t s0, s1, s2, global_brightness;
		getGlobalBrightnessAndScalingFactors(pixels, &s0, &s1, &s2, &global_brightness);
		startBoundary();
		uint8_t p[pixels.size() * 4];
		int i = 0;
		while (pixels.has(1)) {
			uint8_t r = pixels.loadAndScale0(0, s0);
			uint8_t g = pixels.loadAndScale1(0, s1);
			uint8_t b = pixels.loadAndScale2(0, s2);
			//writeLed(global_brightness, r, g, b);
			p[i++] = 0xE0 | global_brightness;
			p[i++] = b;
			p[i++] = g;
			p[i++] = r;
			pixels.stepDithering();
			pixels.advanceData();
		}
		mSPI.writeAllData(p, pixels.size() * 4);
		endBoundary(pixels.size());

		mSPI.waitFully();
		//mSPI.release();
		//Serial.write(" Time: ");
		//Serial.print(micros() - start);
	}

	inline void showPixelsGammaBitShift(PixelController<RGB_ORDER> & pixels) {
		mSPI.select();
		uint8_t r_scale = pixels.getScale0();
		uint8_t g_scale = pixels.getScale1();
		uint8_t b_scale = pixels.getScale2();
		startBoundary();
		while (pixels.has(1)) {
			// Load raw uncorrected r,g,b values.
			uint8_t r = pixels.loadAndScale0(0, 0xFF);
			uint8_t g = pixels.loadAndScale1(0, 0xFF);
			uint8_t b = pixels.loadAndScale2(0, 0xFF);
			uint8_t brightness = 0;
			if ((r | g | b) != 0) {
				// Compute the gamma correction for non black pixels.
				five_bit_hd_gamma_bitshift(
					r, g, b,
					r_scale, g_scale, b_scale,  // Post-gamma scale.
					&r, &g, &b, &brightness);
			}
			writeLed(brightness, r, g, b);
			pixels.stepDithering();
			pixels.advanceData();
		}
		endBoundary(pixels.size());
		mSPI.waitFully();
		mSPI.release();
	}
};

template <
	uint8_t DATA_PIN,
	uint8_t CLOCK_PIN,
	EOrder RGB_ORDER = RGB,
	uint32_t SPI_SPEED = DATA_RATE_MHZ(24),
	uint8_t BUS = 0
>
class APA102ControllerHD : public APA102Controller<
	DATA_PIN,
	CLOCK_PIN, 
	RGB_ORDER,
	SPI_SPEED,
	BUS,
	kFiveBitGammaCorrectionMode_BitShift,
	uint32_t(0x00000000),
	uint32_t(0x00000000)> {
public:
  APA102ControllerHD() = default;
  APA102ControllerHD(const APA102ControllerHD&) = delete;
};

template <
	uint8_t DATA_PIN,
	uint8_t CLOCK_PIN,
	EOrder RGB_ORDER = RGB,
	uint32_t SPI_SPEED = DATA_RATE_MHZ(5),
	uint8_t BUS = 0
>
class APA2020Controller : public APA102Controller<
	DATA_PIN,
	CLOCK_PIN, 
	RGB_ORDER,
	SPI_SPEED,
	BUS,
	kFiveBitGammaCorrectionMode_BitShift,
	uint32_t(0x00000000),
	uint32_t(0xFFFFFFFF)> {
public:
  APA2020Controller() = default;
  APA2020Controller(const APA2020Controller&) = delete;
};

/// SK9822 controller class. It's exactly the same as the APA102Controller protocol but with a different END_FRAME and default SPI_SPEED.
/// @tparam DATA_PIN the data pin for these LEDs
/// @tparam CLOCK_PIN the clock pin for these LEDs
/// @tparam RGB_ORDER the RGB ordering for these LEDs
/// @tparam SPI_SPEED the clock divider used for these LEDs.  Set using the ::DATA_RATE_MHZ / ::DATA_RATE_KHZ macros.  Defaults to ::DATA_RATE_MHZ(24)
template <
	uint8_t DATA_PIN,
	uint8_t CLOCK_PIN,
	EOrder RGB_ORDER = RGB,
	uint32_t SPI_SPEED = DATA_RATE_MHZ(24),
	uint8_t BUS = 0
>
class SK9822Controller : public APA102Controller<
	DATA_PIN,
	CLOCK_PIN,
	RGB_ORDER,
	SPI_SPEED,
	BUS,
	kFiveBitGammaCorrectionMode_Null,
	0x00000000,
	0x00000000
> {
};

/// SK9822 controller class. It's exactly the same as the APA102Controller protocol but with a different END_FRAME and default SPI_SPEED.
/// @tparam DATA_PIN the data pin for these LEDs
/// @tparam CLOCK_PIN the clock pin for these LEDs
/// @tparam RGB_ORDER the RGB ordering for these LEDs
/// @tparam SPI_SPEED the clock divider used for these LEDs.  Set using the ::DATA_RATE_MHZ / ::DATA_RATE_KHZ macros.  Defaults to ::DATA_RATE_MHZ(24)
template <
	uint8_t DATA_PIN,
	uint8_t CLOCK_PIN,
	EOrder RGB_ORDER = RGB,
	uint32_t SPI_SPEED = DATA_RATE_MHZ(24),
	uint8_t BUS = 0
>
class SK9822ControllerHD : public APA102Controller<
	DATA_PIN,
	CLOCK_PIN,
	RGB_ORDER,
	SPI_SPEED,
	BUS,
	kFiveBitGammaCorrectionMode_BitShift,
	0x00000000,
	0x00000000
> {
};




/// @} ClockedChipsets


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Clockless template instantiations - see clockless.h for how the timing values are used
//

#ifdef FASTLED_HAS_CLOCKLESS
/// @defgroup ClocklessChipsets Clockless Chipsets
/// These chipsets have only a single data line. 
///
/// The clockless chipset controllers use the same base class
/// and the same protocol, but with varying timing periods.
///
/// These controllers have 3 control points in their cycle for each bit:
///   @code
///   At T=0        : the line is raised hi to start a bit
///   At T=T1       : the line is dropped low to transmit a zero bit
///   At T=T1+T2    : the line is dropped low to transmit a one bit
///   At T=T1+T2+T3 : the cycle is concluded (next bit can be sent)
///   @endcode
///
/// The units used for T1, T2, and T3 is nanoseconds.  
///
/// For 8MHz/16MHz/24MHz frequencies, these values are also guaranteed
/// to be integral multiples of an 8MHz clock (125ns increments).
///
/// @note The base class, ClocklessController, is platform-specific.
/// @{

// Allow clock that clockless controller is based on to have different
// frequency than the CPU.
#if !defined(CLOCKLESS_FREQUENCY)
    #define CLOCKLESS_FREQUENCY F_CPU
#endif

// We want to force all avr's to use the Trinket controller when running at 8Mhz, because even the 328's at 8Mhz
// need the more tightly defined timeframes.
#if defined(__LGT8F__) || (CLOCKLESS_FREQUENCY == 8000000 || CLOCKLESS_FREQUENCY == 16000000 || CLOCKLESS_FREQUENCY == 24000000) || defined(FASTLED_DOXYGEN) //  || CLOCKLESS_FREQUENCY == 48000000 || CLOCKLESS_FREQUENCY == 96000000) // 125ns/clock

/// Frequency multiplier for each clockless data interval.
/// @see Notes in @ref ClocklessChipsets
#define FMUL (CLOCKLESS_FREQUENCY/8000000)

/// GE8822 controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class GE8822Controller800Khz : public ClocklessController<DATA_PIN, 3 * FMUL, 5 * FMUL, 3 * FMUL, RGB_ORDER, 4> {};

/// LPD1886 controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class LPD1886Controller1250Khz : public ClocklessController<DATA_PIN, 2 * FMUL, 3 * FMUL, 2 * FMUL, RGB_ORDER, 4> {};

/// LPD1886 controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class LPD1886Controller1250Khz_8bit : public ClocklessController<DATA_PIN, 2 * FMUL, 3 * FMUL, 2 * FMUL, RGB_ORDER> {};

/// WS2812 controller class @ 800 KHz.
/// @tparam DATA_PIN the data pin for these LEDs
/// @tparam RGB_ORDER the RGB ordering for these LEDs
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class WS2812Controller800Khz : public ClocklessController<DATA_PIN, 2 * FMUL, 5 * FMUL, 3 * FMUL, RGB_ORDER> {};

/// WS2811 controller class @ 800 KHz.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class WS2811Controller800Khz : public ClocklessController<DATA_PIN, 3 * FMUL, 4 * FMUL, 3 * FMUL, RGB_ORDER> {};

/// DP1903 controller class @ 800 KHz.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class DP1903Controller800Khz : public ClocklessController<DATA_PIN, 2 * FMUL, 8 * FMUL, 2 * FMUL, RGB_ORDER> {};

/// DP1903 controller class @ 400 KHz.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class DP1903Controller400Khz : public ClocklessController<DATA_PIN, 4 * FMUL, 16 * FMUL, 4 * FMUL, RGB_ORDER> {};

/// WS2813 controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>                                                             //not tested
class WS2813Controller : public ClocklessController<DATA_PIN, 3 * FMUL, 4 * FMUL, 3 * FMUL, RGB_ORDER> {};

/// WS2811 controller class @ 400 KHz.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class WS2811Controller400Khz : public ClocklessController<DATA_PIN, 4 * FMUL, 10 * FMUL, 6 * FMUL, RGB_ORDER> {};

/// SK6822 controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class SK6822Controller : public ClocklessController<DATA_PIN, 3 * FMUL, 8 * FMUL, 3 * FMUL, RGB_ORDER> {};

/// SM16703 controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class SM16703Controller : public ClocklessController<DATA_PIN, 3 * FMUL, 4 * FMUL, 3 * FMUL, RGB_ORDER> {};

/// SK6812 controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class SK6812Controller : public ClocklessController<DATA_PIN, 3 * FMUL, 3 * FMUL, 4 * FMUL, RGB_ORDER> {};

/// UCS1903 controller class @ 400 KHz.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class UCS1903Controller400Khz : public ClocklessController<DATA_PIN, 4 * FMUL, 12 * FMUL, 4 * FMUL, RGB_ORDER> {};

/// UCS1903B controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class UCS1903BController800Khz : public ClocklessController<DATA_PIN, 2 * FMUL, 4 * FMUL, 4 * FMUL, RGB_ORDER> {};

/// UCS1904 controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class UCS1904Controller800Khz : public ClocklessController<DATA_PIN, 3 * FMUL, 3 * FMUL, 4 * FMUL, RGB_ORDER> {};

/// UCS2903 controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class UCS2903Controller : public ClocklessController<DATA_PIN, 2 * FMUL, 6 * FMUL, 2 * FMUL, RGB_ORDER> {};

/// TM1809 controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class TM1809Controller800Khz : public ClocklessController<DATA_PIN, 2 * FMUL, 5 * FMUL, 3 * FMUL, RGB_ORDER> {};

/// TM1803 controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class TM1803Controller400Khz : public ClocklessController<DATA_PIN, 6 * FMUL, 9 * FMUL, 6 * FMUL, RGB_ORDER> {};

/// TM1829 controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class TM1829Controller800Khz : public ClocklessController<DATA_PIN, 2 * FMUL, 5 * FMUL, 3 * FMUL, RGB_ORDER, 0, true, 500> {};

/// GW6205 controller class @ 400 KHz.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class GW6205Controller400Khz : public ClocklessController<DATA_PIN, 6 * FMUL, 7 * FMUL, 6 * FMUL, RGB_ORDER, 4> {};

/// UCS1904 controller class @ 800 KHz.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class GW6205Controller800Khz : public ClocklessController<DATA_PIN, 2 * FMUL, 4 * FMUL, 4 * FMUL, RGB_ORDER, 4> {};

/// PL9823 controller class.
/// @copydetails WS2812Controller800Khz
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class PL9823Controller : public ClocklessController<DATA_PIN, 3 * FMUL, 8 * FMUL, 3 * FMUL, RGB_ORDER> {};

#else

/// Calculates the number of cycles for the clockless chipset (which may differ from CPU cycles)
/// @see ::NS()
#ifdef FASTLED_TEENSY4
// just use raw nanosecond values for the teensy4
#define C_NS(_NS) _NS
#else
#define C_NS(_NS) (((_NS * ((CLOCKLESS_FREQUENCY / 1000000L)) + 999)) / 1000)
#endif

// GE8822 - 350ns 660ns 350ns
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class GE8822Controller800Khz : public ClocklessController<DATA_PIN, C_NS(350), C_NS(660), C_NS(350), RGB_ORDER, 4> {};

// GW6205@400khz - 800ns, 800ns, 800ns
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class GW6205Controller400Khz : public ClocklessController<DATA_PIN, C_NS(800), C_NS(800), C_NS(800), RGB_ORDER, 4> {};

// GW6205@400khz - 400ns, 400ns, 400ns
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class GW6205Controller800Khz : public ClocklessController<DATA_PIN, C_NS(400), C_NS(400), C_NS(400), RGB_ORDER, 4> {};

// UCS1903 - 500ns, 1500ns, 500ns
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class UCS1903Controller400Khz : public ClocklessController<DATA_PIN, C_NS(500), C_NS(1500), C_NS(500), RGB_ORDER> {};

// UCS1903B - 400ns, 450ns, 450ns
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class UCS1903BController800Khz : public ClocklessController<DATA_PIN, C_NS(400), C_NS(450), C_NS(450), RGB_ORDER> {};

// UCS1904 - 400ns, 400ns, 450ns
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class UCS1904Controller800Khz : public ClocklessController<DATA_PIN, C_NS(400), C_NS(400), C_NS(450), RGB_ORDER> {};

// UCS2903 - 250ns, 750ns, 250ns
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class UCS2903Controller : public ClocklessController<DATA_PIN, C_NS(250), C_NS(750), C_NS(250), RGB_ORDER> {};

// TM1809 - 350ns, 350ns, 550ns
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class TM1809Controller800Khz : public ClocklessController<DATA_PIN, C_NS(350), C_NS(350), C_NS(450), RGB_ORDER> {};

// WS2811 - 320ns, 320ns, 640ns
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class WS2811Controller800Khz : public ClocklessController<DATA_PIN, C_NS(320), C_NS(320), C_NS(640), RGB_ORDER> {};

// WS2813 - 320ns, 320ns, 640ns
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class WS2813Controller : public ClocklessController<DATA_PIN, C_NS(320), C_NS(320), C_NS(640), RGB_ORDER> {};

// WS2812 - 250ns, 625ns, 375ns
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class WS2812Controller800Khz : public ClocklessController<DATA_PIN, C_NS(250), C_NS(625), C_NS(375), RGB_ORDER> {};

// WS2811@400khz - 800ns, 800ns, 900ns
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class WS2811Controller400Khz : public ClocklessController<DATA_PIN, C_NS(800), C_NS(800), C_NS(900), RGB_ORDER> {};

// 750NS, 750NS, 750NS
template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class TM1803Controller400Khz : public ClocklessController<DATA_PIN, C_NS(700), C_NS(1100), C_NS(700), RGB_ORDER> {};

template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class TM1829Controller800Khz : public ClocklessController<DATA_PIN, C_NS(340), C_NS(340), C_NS(550), RGB_ORDER, 0, true, 500> {};

template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class TM1829Controller1600Khz : public ClocklessController<DATA_PIN, C_NS(100), C_NS(300), C_NS(200), RGB_ORDER, 0, true, 500> {};

template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class LPD1886Controller1250Khz : public ClocklessController<DATA_PIN, C_NS(200), C_NS(400), C_NS(200), RGB_ORDER, 4> {};

template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class LPD1886Controller1250Khz_8bit : public ClocklessController<DATA_PIN, C_NS(200), C_NS(400), C_NS(200), RGB_ORDER> {};


template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class SK6822Controller : public ClocklessController<DATA_PIN, C_NS(375), C_NS(1000), C_NS(375), RGB_ORDER> {};

template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class SK6812Controller : public ClocklessController<DATA_PIN, C_NS(300), C_NS(300), C_NS(600), RGB_ORDER> {};

template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class SM16703Controller : public ClocklessController<DATA_PIN, C_NS(300), C_NS(600), C_NS(300), RGB_ORDER> {};

template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB>
class PL9823Controller : public ClocklessController<DATA_PIN, C_NS(350), C_NS(1010), C_NS(350), RGB_ORDER> {};
#endif
/// @} ClocklessChipsets

#endif
/// @} Chipsets

FASTLED_NAMESPACE_END

#endif
