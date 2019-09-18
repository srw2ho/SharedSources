#pragma once
#ifndef GPIOPIN_H_
#define GPIOPIN_H_

#include "GPIOEventPackageQueue.h"

#include <map> 

using namespace Platform;
using namespace std;

namespace GPIODriver
{
	enum GPIOTyp {
		undef		= -1,
		input		= 0,
		output		= 1,
		PWM			= 2,
		HC_SR04		= 3,
		BME280		= 4,
		inputShutdown = 5,
		PWM9685 = 6,
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
		virtual void OnValueChanged(LONGLONG TimeinYsec, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args);

	
	protected:
		bool doMeasurement();

		void setMeasuremenFinished(double MeasurementTime);

	protected:
		int m_TriggerPinNumber;
		LONGLONG m_ActMeasTime;
		static void OnValueHCSR04Changed(Windows::Devices::Gpio::GpioPin ^sender, Windows::Devices::Gpio::GpioPinValueChangedEventArgs ^args);
		HANDLE m_hFinishedEvent;

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




	protected:



	};

}

#endif