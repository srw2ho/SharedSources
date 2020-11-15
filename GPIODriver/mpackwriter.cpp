#include "pch.h"
#include "GPIODriver.h"
#include "GPIOPin.h"


//#pragma once





namespace GPIODriver
{
	bool GPIOPin::doPack(char* outbuf, size_t* outbuflen) {
		Lock();

#if !GPIOCLIENT_USING
		if (this->m_Pin != nullptr) {
			if (this->getGPIOTyp() == GPIODriver::GPIOTyp::output) {
				this->m_PinValue = (this->m_Pin->Read() == Windows::Devices::Gpio::GpioPinValue::High) ? 1 : 0;
			}
			else if (this->getGPIOTyp() == GPIODriver::GPIOTyp::input) {
				this->m_PinValue = (m_Pin->Read() == Windows::Devices::Gpio::GpioPinValue::High) ? 1 : 0;
			}
		}

#endif
		// encode to memory buffer
		*outbuf = NULL;
		*(outbuflen) = 0;
		char* data;
		size_t datasize;
		mpack_writer_t writer;
		mpack_writer_init_growable(&writer, &data, &datasize);

		mpack_write_u8(&writer, this->m_Typ);
		mpack_write_u16(&writer, this->m_PinNumber);
		mpack_write_cstr(&writer, this->m_PinName.c_str());
		//   mpack_write_str(&writer, pin->m_PinName,strlen(pin->m_PinName) );

		mpack_write_bool(&writer, this->m_ActivateProcessing);
		mpack_write_bool(&writer, this->m_ActivateOutputProcessing);
		mpack_write_double(&writer, this->m_PinValue);
		mpack_write_double(&writer, this->m_InitValue);
		mpack_write_double(&writer, this->m_SetValue);
		mpack_write_double(&writer, this->m_PulseTimeinms);

		mpack_write_u64(&writer, this->m_ActTimeinNanos);

		// finish writing
		if (mpack_writer_destroy(&writer) != mpack_ok) {
			if (data) MPACK_FREE(data);
			UnLock();
			return false;
		}
		memcpy(outbuf, data, datasize);

		*(outbuflen) = datasize;

		if (data) MPACK_FREE(data);

		UnLock();
		return true;


	}

	bool GPIOPWMOutputPin::doPack(char* outbuf, size_t* outbuflen) {

		Lock();
		// encode to memory buffer
		*outbuf = NULL;
		*outbuflen = 0;
		char* data;
		size_t size;
		mpack_writer_t writer;

		size_t GPIOPinbuflen = 0;

		GPIOPin::doPack(outbuf, &GPIOPinbuflen);

		mpack_writer_init_growable(&writer, &data, &size);
		// doing


		mpack_write_uint(&writer, this->m_Frequency);


		// finish writing
		if (mpack_writer_destroy(&writer) != mpack_ok) {
			//    fprintf(stderr, "An error occurred encoding the data!\n");
			if (data) MPACK_FREE(data);
			UnLock();
			return false;
		}
		*outbuflen = GPIOPinbuflen + size;
		memcpy(&outbuf[GPIOPinbuflen], data, size);

		if (data) MPACK_FREE(data);

		UnLock();
		return true;


	}


	bool GPIOHCSR04::doPack(char* outbuf, size_t* outbuflen) {

		Lock();
		// encode to memory buffer
		*outbuf = NULL;
		*outbuflen = 0;
		char* data;
		size_t size;
		mpack_writer_t writer;

		size_t GPIOPinbuflen = 0;

		GPIOPin::doPack(outbuf, &GPIOPinbuflen);

		mpack_writer_init_growable(&writer, &data, &size);
		// doing


		mpack_write_uint(&writer, this->m_TriggerPinNumber);


		// finish writing
		if (mpack_writer_destroy(&writer) != mpack_ok) {
			if (data) MPACK_FREE(data);
			UnLock();
			return false;
		}
		*outbuflen = GPIOPinbuflen + size;
		memcpy(&outbuf[GPIOPinbuflen], data, size);

		if (data) MPACK_FREE(data);

		UnLock();
		return true;


	}

	bool BME280Sensor::doPack(char* outbuf, size_t* outbuflen) {

		Lock();
		// encode to memory buffer
		*outbuf = NULL;
		*outbuflen = 0;
		char* data;
		size_t size;
		mpack_writer_t writer;

		size_t GPIOPinbuflen = 0;

		GPIOPin::doPack(outbuf, &GPIOPinbuflen);

		mpack_writer_init_growable(&writer, &data, &size);
		// doing


		mpack_write_double(&writer, this->m_pressure);
		mpack_write_double(&writer, this->m_temperature);
		mpack_write_double(&writer, this->m_humidity);


		// finish writing
		if (mpack_writer_destroy(&writer) != mpack_ok) {
			if (data) MPACK_FREE(data);
			UnLock();
			return false;
		}
		*outbuflen = GPIOPinbuflen + size;
		memcpy(&outbuf[GPIOPinbuflen], data, size);

		if (data) MPACK_FREE(data);

		UnLock();
		return true;


	}


	bool BME680Sensor::doPack(char* outbuf, size_t* outbuflen) {

		Lock();
		// encode to memory buffer
		*outbuf = NULL;
		*outbuflen = 0;
		char* data;
		size_t size;
		mpack_writer_t writer;

		size_t GPIOPinbuflen = 0;

		GPIOPin::doPack(outbuf, &GPIOPinbuflen);

		mpack_writer_init_growable(&writer, &data, &size);
		// doing


		mpack_write_double(&writer, this->m_iaq);
		mpack_write_int(&writer, this->m_iaq_accuracy);

		mpack_write_double(&writer, this->m_temperature);
		mpack_write_double(&writer, this->m_humidity);
		mpack_write_double(&writer, this->m_pressure);

		mpack_write_double(&writer, this->m_raw_temperature);
		mpack_write_double(&writer, this->m_raw_humidity);
		mpack_write_double(&writer, this->m_gas);
		mpack_write_int(&writer, this->m_bsec_status);
		mpack_write_double(&writer, this->m_static_iaq);
		mpack_write_double(&writer, this->m_co2_equivalent);
		mpack_write_double(&writer, this->m_breath_voc_equivalent);

		// finish writing
		if (mpack_writer_destroy(&writer) != mpack_ok) {
			if (data) MPACK_FREE(data);
			UnLock();
			return false;
		}
		*outbuflen = GPIOPinbuflen + size;
		memcpy(&outbuf[GPIOPinbuflen], data, size);

		if (data) MPACK_FREE(data);

		UnLock();
		return true;


	}

	bool ADCMT3620::doPack(char* outbuf, size_t* outbuflen) {

		Lock();
		// encode to memory buffer
		*outbuf = NULL;
		*outbuflen = 0;
		//char* data;
		size_t size;
	//	mpack_writer_t writer;

		size_t GPIOPinbuflen = 0;

		GPIOPin::doPack(outbuf, &GPIOPinbuflen);

		size = 0;


		*outbuflen = GPIOPinbuflen + size;
	//	memcpy(&outbuf[GPIOPinbuflen], data, size);

		UnLock();
		return true;


	}


	bool PWMMT3620::doPack(char* outbuf, size_t* outbuflen) {

		Lock();
		// encode to memory buffer
		*outbuf = NULL;
		*outbuflen = 0;
		char* data;
		size_t size;
		mpack_writer_t writer;

		size_t GPIOPinbuflen = 0;

		GPIOPin::doPack(outbuf, &GPIOPinbuflen);

		mpack_writer_init_growable(&writer, &data, &size);
		// doing
		mpack_write_u16(&writer, this->m_ChannelNos);

	
		PWMChannel* pwmChannel = (PWMChannel*)&this->m_PWMChannels[0];


		for (size_t i = 0; i < this->m_ChannelNos; ++i) {
			mpack_start_bin(&writer, sizeof(UserPWMState));
			mpack_write_bytes(&writer,(const char*) &pwmChannel->m_UserPWMState, sizeof(UserPWMState));
			mpack_finish_bin(&writer);

			pwmChannel++;

		}

		mpack_finish_array(&writer);

		// finish writing
		if (mpack_writer_destroy(&writer) != mpack_ok) {
			if (data) MPACK_FREE(data);
			UnLock();
			return false;
		}
		*outbuflen = GPIOPinbuflen + size;
		memcpy(&outbuf[GPIOPinbuflen], data, size);

		if (data) MPACK_FREE(data);

		UnLock();
		return true;


	}
}