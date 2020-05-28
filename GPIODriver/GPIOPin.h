#pragma once
#ifndef GPIOPIN_H_
#define GPIOPIN_H_

#include "GPIOEventPackageQueue.h"

#include "..\mpack\src\mpack\mpack-platform.h"
#include "..\mpack\src\mpack\mpack.h"


#include <map> 

using namespace Platform;
using namespace std;

namespace GPIODriver
{
	enum GPIOTyp {
		undef		= -1,
		input		= 0,
		output		= 1,
		HC_SR04		= 3,
		BME280		= 4,
		BME680		= 5,
		PWM_MT3620	= 7,
		ADC_MT3620	= 8,
		PWM			= 10,
		inputShutdown = 11,
		PWM9685 = 12
	};


	class GPIOPin {
	public:
		GPIOPin(GPIOEventPackageQueue* pGPIOEventPackageQueue);

		virtual ~GPIOPin();

		double getPinValue();
		double getSetValue();
		int getPinNumber();
	
		bool getActivateProcessing();
		bool getActivateOutputProcessing();

		GPIODriver::GPIOTyp	getGPIOTyp(); 
		std::string & getPinName() { return m_PinName; };

		void setSetValue(double Value);
		void setPinValue(double Value);
		void setPulseTimeinms(double Value);

		virtual void setActivateProcessing(bool Activate);

		virtual void setActivateOutputProcessing(bool Activate);
		virtual Platform::String^ GetGPIOPinCmd();

		virtual Platform::String^ GetGPIOPinClientSendCmd();

		virtual UINT64 getTimeinNanos();

		virtual void setTimeinNanos(UINT64 nanos);


		virtual bool setOutputToInitialValue();
	
		virtual bool doProcessing();

		virtual bool doClientProcessing(); // Processs Client 

		virtual void DiableValueChanged();

		virtual bool doPack(char* outbuf, size_t* outbuflen);
		virtual bool doparse(mpack_reader_t* reader);

		virtual Windows::Storage::Streams::IBuffer^ GetGPIOPinClientSendCmdBuf(bool doMpack);
	protected:
		void UnLock();
		void Lock();

	protected:
		std::string m_PinName;
		GPIODriver::GPIOTyp	 m_Typ;
		bool m_ActivateProcessing;		// Activate processing commands
		bool m_ActivateOutputProcessing;		// Activate processing commands
		double m_PinValue;		// Value
		double m_InitValue;		// Initialisierungs Value
		double m_SetValue;		// für Output und PWM-Output
		int m_PinNumber;
		Windows::Foundation::EventRegistrationToken m_valueChangedToken;
		double m_PulseTimeinms;		// Pulse Time in ms
		bool m_PulseTimeActive;		// Pulse-Timing aktiv
		UINT64 m_ActMeasTime;
		Windows::System::Threading::ThreadPoolTimer ^m_Timer;
		UINT64 m_ActTimeinNanos;
	//	UINT64 m_ime;
#if !GPIOCLIENT_USING
		Windows::Devices::Gpio::GpioPin^ m_Pin;
#else

#endif


	protected:
		CRITICAL_SECTION m_CritLock;
		GPIODriver::GPIOEventPackageQueue* m_pGPIOEventPackageQueue;
	};
	
	class GPIOInputPin :public GPIOPin {
	public:
		GPIOInputPin(GPIOEventPackageQueue* pGPIOEventPackageQueue,int PinNo, double InitPinValue);
		virtual ~GPIOInputPin();

		virtual Platform::String^ GetGPIOPinCmd();
		virtual bool Init(Windows::Devices::Gpio::GpioController ^ GPIOController);

		virtual void OnValueChanged(LONGLONG TimeinYsec, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args);


	private:
			static void OnValueInputChanged(Windows::Devices::Gpio::GpioPin ^sender, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args);
	};


	class GPIOInputPinShutDown :public GPIOInputPin {
	public:
		GPIOInputPinShutDown(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue);
		virtual ~GPIOInputPinShutDown();

//		virtual Platform::String^ GetGPIOPinCmd();

		virtual void OnValueChanged(LONGLONG TimeinYsec, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args);
		virtual bool doProcessing();

	private:
	};

	class GPIOOutputPin :public GPIOPin {
	public:
		GPIOOutputPin(GPIOEventPackageQueue* pGPIOEventPackageQueue,int PinNo, double InitPinValue);
		virtual ~GPIOOutputPin();

		virtual Platform::String^ GetGPIOPinCmd();
		virtual bool Init(Windows::Devices::Gpio::GpioController ^ GPIOController);

		virtual void OnValueChanged(LONGLONG TimeinYsec, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args);


	protected:
		static void OnValueOutputChanged(Windows::Devices::Gpio::GpioPin ^sender, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args);

	};

	class GPIOPWMOutputPin :public GPIOPin {
		double m_Frequency;
	public:
		GPIOPWMOutputPin(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue);
		virtual ~GPIOPWMOutputPin();

		virtual Platform::String^ GetGPIOPinCmd();
		virtual Platform::String^ GetGPIOPinClientSendCmd();
		virtual bool Init(Windows::Devices::Pwm::PwmController^ PwmController, double frequency);
		virtual bool doProcessing();
		virtual bool setOutputToInitialValue();
		virtual void UpdatesetFrequency(Windows::Devices::Pwm::PwmController^ PwmController, double value);
		virtual void setFrequency(double value);
		virtual double getFrequency();

		virtual bool doPack(char* outbuf, size_t* outbuflen);
		virtual bool doparse(mpack_reader_t* reader);
	protected:

#if !GPIOCLIENT_USING
		Windows::Devices::Pwm::PwmPin^ m_PWMPin;
#else

#endif

	};

	class GPIOPWMPWM9685OutputPin :public GPIOPWMOutputPin {
	public:
		GPIOPWMPWM9685OutputPin(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue);
		virtual ~GPIOPWMPWM9685OutputPin();


	protected:

#if !GPIOCLIENT_USING

#else

#endif

	};

	class GPIOHCSR04 :public GPIOPin {
	public:
		GPIOHCSR04(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, int TriggerPin, double InitPinValue);
		virtual ~GPIOHCSR04();

		virtual Platform::String^ GetGPIOPinCmd();
		virtual Platform::String^ GetGPIOPinClientSendCmd();
		virtual bool Init(Windows::Devices::Gpio::GpioController ^ GPIOController);

		void setTriggerPinNumber (int pinNo);
		virtual bool doProcessing();
		virtual bool setOutputToInitialValue();
	//	virtual void OnValueChanged(LONGLONG TimeinYsec, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args);

		virtual bool doPack(char* outbuf, size_t* outbuflen);
		virtual bool doparse(mpack_reader_t* reader);
	
	protected:
		bool doMeasurement();

	//	void setMeasuremenFinished(double MeasurementTime);

	protected:
		int m_TriggerPinNumber;
		LONGLONG m_ActMeasTime;
//		static void OnValueHCSR04Changed(Windows::Devices::Gpio::GpioPin ^sender, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args);
	//	HANDLE m_hFinishedEvent;

#if !GPIOCLIENT_USING
		Windows::Devices::Gpio::GpioPin^ m_TriggerPin;
#else

#endif

	};

	class BME280Sensor :public GPIOPin {

	protected:

		double m_pressure;
		/*! Compensated temperature */
		double m_temperature;
		/*! Compensated humidity */
		double m_humidity;


	public:
		BME280Sensor(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo,  double InitPinValue);
		virtual ~BME280Sensor();

		virtual Platform::String^ GetGPIOPinCmd();
		virtual Platform::String^ GetGPIOPinClientSendCmd();
		virtual bool Init(Windows::Devices::Gpio::GpioController ^ GPIOController);

		virtual bool doProcessing();

	
		void setPressure(double Pressure) { m_pressure = Pressure; };
		void setTemperature(double Temperature) { m_temperature = Temperature; };
		void setHumidity(double Humidity) { m_humidity = Humidity; };

		virtual double getPressure() { return m_pressure; };
		virtual double getTemperature() { return m_temperature; };
		virtual double getHumidity() { return m_humidity; };

		virtual bool doPack(char* outbuf, size_t* outbuflen);
		virtual bool doparse(mpack_reader_t* reader);


	protected:



	};

	class BME680Sensor :public GPIOPin {

	protected:

		double m_iaq;
		int m_iaq_accuracy;
		double  m_temperature;
		double  m_humidity;
		double  m_pressure;
		double  m_raw_temperature;
		double  m_raw_humidity;
		double  m_gas;
		int		m_bsec_status;
		double  m_static_iaq;
		double  m_co2_equivalent;
		double  m_breath_voc_equivalent;

	public:
		BME680Sensor(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue);
		virtual ~BME680Sensor();

		//virtual Platform::String^ GetGPIOPinCmd();
		//virtual Platform::String^ GetGPIOPinClientSendCmd();
		virtual bool Init(Windows::Devices::Gpio::GpioController^ GPIOController);

		virtual bool doProcessing();



		virtual double getiaq() { return m_iaq; };
		virtual int  getiaqaccuracy() { return m_iaq_accuracy; };
		virtual double getPressure() { return m_pressure; };
		virtual double getTemperature() { return m_temperature; };
		virtual double getHumidity() { return m_humidity; };

		virtual double getraw_temperature() { return m_raw_temperature; };
		virtual double getraw_humidity() { return m_raw_humidity; };
		virtual double getgas() { return m_gas; };
		virtual int getbsec_status() { return m_bsec_status; };
		virtual double getstatic_iaq() { return m_static_iaq; };
		virtual double getco2_equivalent() { return m_co2_equivalent; };
		virtual double getbreath_voc_equivalent() { return m_breath_voc_equivalent; };

		virtual bool doPack(char* outbuf, size_t* outbuflen);
		virtual bool doparse(mpack_reader_t* reader);

	protected:



	};





	class ADCMT3620 :public GPIOPin {

	protected:


	public:
		ADCMT3620(GPIOEventPackageQueue* pGPIOEventPackageQueue, int PinNo, double InitPinValue);
		virtual ~ADCMT3620();

		//virtual Platform::String^ GetGPIOPinCmd();
		//virtual Platform::String^ GetGPIOPinClientSendCmd();
		virtual bool Init(Windows::Devices::Gpio::GpioController^ GPIOController);

		virtual bool doProcessing();


		virtual bool doPack(char* outbuf, size_t* outbuflen);
		virtual bool doparse(mpack_reader_t* reader);

	protected:



	};


	typedef struct {
		unsigned int m_fullCycleNs;
		unsigned int m_dutyCycleNs;
		unsigned int m_polarity;
		bool m_enabled;

	}UserPWMState;

	typedef struct {
		UserPWMState m_UserPWMState;

	}PWMChannel;

	class PWMMT3620 :public GPIOPin {

	protected:

		unsigned int m_ChannelNos;
		PWMChannel* m_PWMChannels;


	public:
		PWMMT3620(GPIOEventPackageQueue* pGPIOEventPackageQueue, unsigned int PWMController, unsigned int channelNo, unsigned int Polarity, unsigned int fullCycleNs, unsigned int dutyCycleNs);
		virtual ~PWMMT3620();

		//virtual Platform::String^ GetGPIOPinCmd();
		//virtual Platform::String^ GetGPIOPinClientSendCmd();
		virtual bool Init(Windows::Devices::Gpio::GpioController^ GPIOController);

		virtual bool doProcessing();

		bool setfullCycleNs(unsigned int chan, unsigned int fullCycleNs);
		unsigned int getfullCycleNs(unsigned int chan);
		bool setdutyCycleNs( unsigned int chan, unsigned int dutyCycleNs);
		unsigned int getdutyCycleNs( unsigned int chan);
		bool setenable( unsigned int chan, bool enable);
		bool getenable(unsigned int chan);
		unsigned int PWMMT3620::getChannelNos();

		virtual bool doPack(char* outbuf, size_t* outbuflen);
		virtual bool doparse(mpack_reader_t* reader);


	protected:



	};
}

#endif